/*
 * httpsrvr.cxx
 *
 * HTTP server classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#ifdef P_HTTP

#include <ptlib/sockets.h>
#include <ptclib/http.h>
#include <ptclib/random.h>
#include <ctype.h>

#define new PNEW
#define PTraceModule() "HTTPServer"


// define to enable work-around for Netscape persistent connection bug
// set to lifetime of suspect sockets (in seconds)
#define STRANGE_NETSCAPE_BUG  3

// maximum delay between characters whilst reading a line of text
static const PTimeInterval ReadLineTimeout(0, 30);

#define DEFAULT_PERSIST_TIMEOUT 30
#define DEFAULT_PERSIST_TRANSATIONS 10

//  filename to use for directory access directives
static const char * accessFilename = "_access";

static const PConstString WebSocketGUID("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");


//////////////////////////////////////////////////////////////////////////////
// PHTTPSpace

PHTTPSpace::PHTTPSpace()
{
  mutex = new PReadWriteMutex;
  root = new Node(PString(), NULL);
}


void PHTTPSpace::DestroyContents()
{
  delete mutex;
  delete root;
}


void PHTTPSpace::CloneContents(const PHTTPSpace * c)
{
  mutex = new PReadWriteMutex;
  root = new Node(*c->root);
}


void PHTTPSpace::CopyContents(const PHTTPSpace & c)
{
  mutex = c.mutex;
  root = c.root;
}


PHTTPSpace::Node::Node(const PString & nam, Node * parentNode)
  : PString(nam)
{
  parent = parentNode;
  resource = NULL;
}


PHTTPSpace::Node::~Node()
{
  delete resource;
}


PBoolean PHTTPSpace::AddResource(PHTTPResource * res, AddOptions overwrite)
{
  PAssert(res != NULL, PInvalidParameter);
  const PStringArray & path = res->GetURL().GetPath();
  Node * node = root;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    if (path[i].IsEmpty())
      break;

    if (node->resource != NULL) {
      delete res;
      return false;   // Already a resource in tree in partial path
    }

    PINDEX pos = node->children.GetValuesIndex(path[i]);
    if (pos == P_MAX_INDEX)
      pos = node->children.Append(new Node(path[i], node));

    node = &node->children[pos];
  }

  if (!node->children.IsEmpty()) {
    delete res;
    return false;   // Already a resource in tree further down path.
  }

  if (overwrite == ErrorOnExist && node->resource != NULL) {
    delete res;
    return false;   // Already a resource in tree at leaf
  }

  delete node->resource;
  node->resource = res;

  return true;
}


PBoolean PHTTPSpace::DelResource(const PURL & url)
{
  const PStringArray & path = url.GetPath();
  Node * node = root;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    if (path[i].IsEmpty())
      break;

    PINDEX pos = node->children.GetValuesIndex(path[i]);
    if (pos == P_MAX_INDEX)
      return false;

    node = &node->children[pos];

    // If have resource and not last node, then trying to remove something
    // further down the tree than a leaf node.
    if (node->resource != NULL && i < (path.GetSize()-1))
      return false;
  }

  if (!node->children.IsEmpty())
    return false;   // Still a resource in tree further down path.

  if (node->parent != NULL) {
    do {
      Node * par = node->parent;
      par->children.Remove(node);
      node = par;
    } while (node->parent != NULL && node->children.IsEmpty());
  }

  return true;
}


static const char * const HTMLIndexFiles[] = {
  "Welcome.html", "welcome.html", "index.html",
  "Welcome.htm",  "welcome.htm",  "index.htm"
};

PHTTPResource * PHTTPSpace::FindResource(const PURL & url)
{
  const PStringArray & path = url.GetPath();

  Node * node = root;
  PINDEX i;
  for (i = 0; i < path.GetSize(); i++) {
    if (path[i].IsEmpty())
      break;

    PINDEX pos = node->children.GetValuesIndex(path[i]);
    if (pos == P_MAX_INDEX)
      return NULL;

    node = &node->children[pos];

    if (node->resource != NULL)
      return node->resource;
  }

  for (i = 0; i < PARRAYSIZE(HTMLIndexFiles); i++) {
    PINDEX pos = node->children.GetValuesIndex(PString(HTMLIndexFiles[i]));
    if (pos != P_MAX_INDEX)
      return node->children[pos].resource;
  }

  return NULL;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPServer

PHTTPServer::PHTTPServer()
{
  Construct();
}


PHTTPServer::PHTTPServer(const PHTTPSpace & space)
  : urlSpace(space)
{
  Construct();
}


void PHTTPServer::Construct()
{
  transactionCount = 0;
  SetReadLineTimeout(ReadLineTimeout);
}


PBoolean PHTTPServer::ProcessCommand()
{
  PString args;
  PINDEX cmd;

  // if this is not the first command received by this socket, then set
  // the read timeout appropriately.
  if (transactionCount > 0) 
    SetReadTimeout(nextTimeout);

  // this will only return false upon timeout or completely invalid command
  if (!ReadCommand(cmd, args))
    return false;

  connectInfo.commandCode = (Commands)cmd;
  if (cmd < NumCommands)
    connectInfo.commandName = commandNames[cmd];
  else {
    PINDEX spacePos = args.Find(' ');
    connectInfo.commandName = args.Left(spacePos);
    args = args.Mid(spacePos);
  }

  // if no tokens, error
  if (args.IsEmpty()) {
    OnError(BadRequest, args, connectInfo);
    return false;
  }

  if (!connectInfo.Initialise(*this, args))
    return false;

  // now that we've decided we did receive a HTTP request, increment the
  // count of transactions
  transactionCount++;
  nextTimeout = connectInfo.GetPersistenceTimeout();

  PIPSocket * socket = GetSocket();
  WORD myPort = (WORD)(socket != NULL ? socket->GetPort() : 80);

  // the URL that comes with Connect requests is not quite kosher, so 
  // mangle it into a proper URL and do NOT close the connection.
  // for all other commands, close the read connection if not persistent
  if (cmd == CONNECT) 
    connectInfo.url.Parse("https://" + args);
  else {
    connectInfo.url.Parse(args, "http");
    if (connectInfo.url.GetPort() == 0)
      connectInfo.url.SetPort(myPort);
  }

  bool persist;
  
  // make sure the form info is reset for each new operation
  connectInfo.ResetMultipartFormInfo();

  PTRACE(5, "Transaction " << connectInfo.GetCommandName() << " \"" << args << "\" url=" << connectInfo.GetURL());

  if (connectInfo.IsWebSocket()) {
    if (!OnWebSocket(connectInfo))
      return false;
    persist = true;
  }
  else {
    // If the incoming URL is of a proxy type then call OnProxy() which will
    // probably just go OnError(). Even if a full URL is provided in the
    // command we should check to see if it is a local server request and process
    // it anyway even though we are not a proxy. The usage of GetHostName()
    // below are to catch every way of specifying the host (name, alias, any of
    // several IP numbers etc).
    const PURL & url = connectInfo.GetURL();
    if (url.GetScheme() != "http" ||
        (url.GetPort() != 0 && url.GetPort() != myPort) ||
        (!url.GetHostName() && !PIPSocket::IsLocalHost(url.GetHostName())))
      persist = OnProxy(connectInfo);
    else {
      connectInfo.entityBody = ReadEntityBody();
      persist = OnCommand(cmd, url, args, connectInfo);
    }
  }

  flush();

  // if the function just indicated that the connection is to persist,
  // and so did the client, then return true. Note that all of the OnXXXX
  // routines above must make sure that their return value is false if
  // if there was no ContentLength field in the response. This ensures that
  // we always close the socket so the client will get the correct end of file
  if (persist && connectInfo.IsPersistent()) {
    unsigned max = connectInfo.GetPersistenceMaximumTransations();
    if (max == 0 || transactionCount < max)
      return true;
  }

  PTRACE(5, "Connection end: " << connectInfo.IsPersistent());

  // close the output stream now and return false
  Shutdown(ShutdownWrite);
  return false;
}


bool PHTTPServer::OnCommand(PINDEX cmd, const PURL &, const PString & args, PHTTPConnectionInfo & connectInfo)
{
  bool persist = false;

  // Handle the local request
  switch (cmd) {
    case GET :
      connectInfo.DecodeMultipartFormInfo();
      persist = OnGET(connectInfo);
      break;

    case HEAD :
      persist = OnHEAD(connectInfo);
      break;

    case POST :
      connectInfo.DecodeMultipartFormInfo();
      persist = OnPOST(connectInfo);
      break;

    case P_MAX_INDEX:
    default:
      persist = OnUnknown(args, connectInfo);
  }

  return persist;
}


bool PHTTPServer::OnWebSocket(PHTTPConnectionInfo & connectInfo)
{
#if P_SSL
  std::map<PString, WebSocketNotifier>::iterator notifier;

  const PMIMEInfo & mime = connectInfo.GetMIME();

  PStringArray protocols = mime(WebSocketProtocolTag()).Tokenise(", \t\r\n", false);
  for (PINDEX i = 0; ; ++i) {
    if (i >= protocols.GetSize())
      return OnError(NotFound, "Unsupported WebSocket protocol", connectInfo);

    if ((notifier = m_webSocketNotifiers.find(protocols[i])) != m_webSocketNotifiers.end())
      break;
  }

  PMIMEInfo reply;
  reply.SetAt(ConnectionTag(), UpgradeTag());
  reply.SetAt(UpgradeTag(), WebSocketTag());
  reply.SetAt(WebSocketProtocolTag(), notifier->first);
  reply.SetAt(WebSocketAcceptTag(), PMessageDigestSHA1::Encode(mime(WebSocketKeyTag()) + WebSocketGUID));

  StartResponse(SwitchingProtocols, reply, -1);
  flush();

  if (!notifier->second.IsNULL()) {
    notifier->second(*this, connectInfo);
    return !connectInfo.IsWebSocket();
  }

  urlSpace.StartRead();
  PHTTPResource * resource = urlSpace.FindResource(connectInfo.GetURL());
  bool persist = resource != NULL && resource->OnWebSocket(*this, connectInfo);
  urlSpace.EndRead();

  return persist;
#else
  PTRACE(2, "WebSocket refused due to no SSL");
  return OnError(NotFound, "WebSocket unsupported (No SSL)", connectInfo);
#endif
}


void PHTTPServer::SetWebSocketNotifier(const PString & protocol, const WebSocketNotifier & notifier)
{
  m_webSocketNotifiers[protocol] = notifier;
}


void PHTTPServer::ClearWebSocketNotifier(const PString & protocol)
{
  std::map<PString, WebSocketNotifier>::iterator it = m_webSocketNotifiers.find(protocol);
  if (it != m_webSocketNotifiers.end())
    m_webSocketNotifiers.erase(it);
}


PString PHTTPServer::ReadEntityBody()
{
  if (connectInfo.GetMajorVersion() < 1)
    return PString();

  PString entityBody;
  long contentLength = connectInfo.GetEntityBodyLength();
  // a content length of > 0 means read explicit length
  // a content length of < 0 means read until EOF
  // a content length of 0 means read nothing
  if (contentLength > 0)
    entityBody = ReadString((PINDEX)contentLength);
  else if (contentLength == -2)
    ReadLine(entityBody, false);
  else if (contentLength < 0)
    entityBody = ReadString(P_MAX_INDEX);

  // close the connection, if not persistent
  if (!connectInfo.IsPersistent()) {
    PIPSocket * socket = GetSocket();
    if (socket != NULL)
      socket->Shutdown(PIPSocket::ShutdownRead);
  }

  return entityBody;
}


PString PHTTPServer::GetServerName() const
{
  return "PWLib-HTTP-Server/1.0 PWLib/1.0";
}


void PHTTPServer::SetURLSpace(const PHTTPSpace & space)
{
  urlSpace = space;
}


bool PHTTPServer::OnGET(const PHTTPConnectionInfo & conInfo)
{
  urlSpace.StartRead();
  PHTTPResource * resource = urlSpace.FindResource(conInfo.GetURL());
  if (resource == NULL) {
    urlSpace.EndRead();
    return OnError(NotFound, conInfo.GetURL().AsString(), connectInfo);
  }

  bool retval = resource->OnGET(*this, connectInfo);
  urlSpace.EndRead();
  return retval;
}


bool PHTTPServer::OnHEAD(const PHTTPConnectionInfo & conInfo)
{
  urlSpace.StartRead();
  PHTTPResource * resource = urlSpace.FindResource(conInfo.GetURL());
  if (resource == NULL) {
    urlSpace.EndRead();
    return OnError(NotFound, conInfo.GetURL().AsString(), connectInfo);
  }

  bool retval = resource->OnHEAD(*this, connectInfo);
  urlSpace.EndRead();
  return retval;
}


bool PHTTPServer::OnPOST(const PHTTPConnectionInfo & conInfo)
{
  urlSpace.StartRead();
  PHTTPResource * resource = urlSpace.FindResource(conInfo.GetURL());
  if (resource == NULL) {
    urlSpace.EndRead();
    return OnError(NotFound, conInfo.GetURL().AsString(), connectInfo);
  }

  bool retval = resource->OnPOST(*this, connectInfo);
  urlSpace.EndRead();
  return retval;
}


PBoolean PHTTPServer::OnProxy(const PHTTPConnectionInfo & connectInfo)
{
  return OnError(BadGateway, "Proxy not implemented.", connectInfo) &&
         connectInfo.GetCommandCode() != CONNECT;
}


struct httpStatusCodeStruct {
  const char * text;
  int  code;
  PBoolean allowedBody;
  int  majorVersion;
  int  minorVersion;
};

static const httpStatusCodeStruct * GetStatusCodeStruct(int code)
{
  static const httpStatusCodeStruct httpStatusDefn[] = {
    // First entry MUST be InternalServerError
    { "Internal Server Error",         PHTTP::InternalServerError, 1 },
    { "OK",                            PHTTP::RequestOK, 1 },
    { "Unauthorised",                  PHTTP::UnAuthorised, 1 },
    { "Forbidden",                     PHTTP::Forbidden, 1 },
    { "Not Found",                     PHTTP::NotFound, 1 },
    { "Not Modified",                  PHTTP::NotModified },
    { "No Content",                    PHTTP::NoContent },
    { "Bad Gateway",                   PHTTP::BadGateway, 1 },
    { "Bad Request",                   PHTTP::BadRequest, 1 },
    { "Continue",                      PHTTP::Continue, 1, 1, 1 },
    { "Switching Protocols",           PHTTP::SwitchingProtocols, 1, 1, 1 },
    { "Created",                       PHTTP::Created, 1 },
    { "Accepted",                      PHTTP::Accepted, 1 },
    { "Non-Authoritative Information", PHTTP::NonAuthoritativeInformation, 1, 1, 1 },
    { "Reset Content",                 PHTTP::ResetContent, 0, 1, 1 },
    { "Partial Content",               PHTTP::PartialContent, 1, 1, 1 },
    { "Multiple Choices",              PHTTP::MultipleChoices, 1, 1, 1 },
    { "Moved Permanently",             PHTTP::MovedPermanently, 1 },
    { "Moved Temporarily",             PHTTP::MovedTemporarily, 1 },
    { "See Other",                     PHTTP::SeeOther, 1, 1, 1 },
    { "Use Proxy",                     PHTTP::UseProxy, 1, 1, 1 },
    { "Payment Required",              PHTTP::PaymentRequired, 1, 1, 1 },
    { "Method Not Allowed",            PHTTP::MethodNotAllowed, 1, 1, 1 },
    { "None Acceptable",               PHTTP::NoneAcceptable, 1, 1, 1 },
    { "Proxy Authetication Required",  PHTTP::ProxyAuthenticationRequired, 1, 1, 1 },
    { "Request Timeout",               PHTTP::RequestTimeout, 1, 1, 1 },
    { "Conflict",                      PHTTP::Conflict, 1, 1, 1 },
    { "Gone",                          PHTTP::Gone, 1, 1, 1 },
    { "Length Required",               PHTTP::LengthRequired, 1, 1, 1 },
    { "Unless True",                   PHTTP::UnlessTrue, 1, 1, 1 },
    { "Not Implemented",               PHTTP::NotImplemented, 1 },
    { "Service Unavailable",           PHTTP::ServiceUnavailable, 1, 1, 1 },
    { "Gateway Timeout",               PHTTP::GatewayTimeout, 1, 1, 1 }
  };

  // make sure the error code is valid
  for (PINDEX i = 0; i < PARRAYSIZE(httpStatusDefn); i++)
    if (code == httpStatusDefn[i].code)
      return &httpStatusDefn[i];

  return httpStatusDefn;
}


PBoolean PHTTPServer::StartResponse(StatusCode code,
                                PMIMEInfo & headers,
                                long bodySize)
{
  if (connectInfo.majorVersion < 1) 
    return false;

  httpStatusCodeStruct dummyInfo;
  const httpStatusCodeStruct * statusInfo;
  if (connectInfo.commandCode < NumCommands)
    statusInfo = GetStatusCodeStruct(code);
  else {
    dummyInfo.text = "";
    dummyInfo.code = code;
    dummyInfo.allowedBody = true;
    dummyInfo.majorVersion = connectInfo.majorVersion;
    dummyInfo.minorVersion = connectInfo.minorVersion;
    statusInfo = &dummyInfo;
  }

  // output the command line
  *this << "HTTP/" << connectInfo.majorVersion << '.' << connectInfo.minorVersion
        << ' ' << statusInfo->code << ' ' << statusInfo->text << "\r\n";

  PBoolean chunked = false;

  // If do not have user set content length, decide if we should add one
  if (!headers.Contains(ContentLengthTag())) {
    if (connectInfo.minorVersion < 1) {
      // v1.0 client, don't put in ContentLength if the bodySize is zero because
      // that can be confused by some browsers as meaning there is no body length.
      if (bodySize > 0)
        headers.SetAt(ContentLengthTag(), bodySize);
    }
    else {
      // v1.1 or later, see if will use chunked output
      chunked = (PINDEX)bodySize == P_MAX_INDEX;
      if (chunked)
        headers.SetAt(TransferEncodingTag(), ChunkedTag());
      else if ((bodySize >= 0) && ((PINDEX)bodySize < P_MAX_INDEX))
        headers.SetAt(ContentLengthTag(), bodySize);
    }
  }

  *this << setfill('\r') << headers;

#ifdef STRANGE_NETSCAPE_BUG
  // The following is a work around for a strange bug in Netscape where it
  // locks up when a persistent connection is made and data less than 1k
  // (including MIME headers) is sent. Go figure....
  if (bodySize < 1024 && connectInfo.GetMIME()(UserAgentTag()).Find("Mozilla/2.0") != P_MAX_INDEX)
    nextTimeout.SetInterval(STRANGE_NETSCAPE_BUG*1000);
#endif

  return chunked;
}


void PHTTPServer::SetDefaultMIMEInfo(PMIMEInfo & info,
                     const PHTTPConnectionInfo & connectInfo)
{
  if (!info.Contains(DateTag))
    info.SetAt(DateTag, PTime().AsString(PTime::RFC1123, PTime::GMT));

  if (!info.Contains(MIMEVersionTag))
    info.SetAt(MIMEVersionTag, "1.0");

  if (!info.Contains(ServerTag))
    info.SetAt(ServerTag, GetServerName());

  if (connectInfo.IsPersistent()) {
    if (connectInfo.IsProxyConnection()) {
      PTRACE(5, "Setting proxy persistent response");
      info.SetAt(ProxyConnectionTag, KeepAliveTag());
    }
    else {
      PTRACE(5, "Setting direct persistent response");
      info.SetAt(ConnectionTag, KeepAliveTag());
    }
  }
}



PBoolean PHTTPServer::OnUnknown(const PCaselessString & cmd, 
                        const PHTTPConnectionInfo & connectInfo)
{
  return OnError(NotImplemented, cmd, connectInfo);
}


PBoolean PHTTPServer::OnError(StatusCode code,
             const PCaselessString & extra,
         const PHTTPConnectionInfo & connectInfo)
{
  const httpStatusCodeStruct * statusInfo = GetStatusCodeStruct(code);

  if (!connectInfo.IsCompatible(statusInfo->majorVersion, statusInfo->minorVersion))
    statusInfo = GetStatusCodeStruct((code/100)*100);

  PMIMEInfo headers;
  SetDefaultMIMEInfo(headers, connectInfo);

  if (!statusInfo->allowedBody) {
    StartResponse(code, headers, 0);
    return statusInfo->code == RequestOK;
  }

  PString reply;
  if (extra.Find("<body") != P_MAX_INDEX)
    reply = extra;
  else {
    PHTML html;
    html << PHTML::Title()
         << statusInfo->code
         << ' '
         << statusInfo->text
         << PHTML::Body()
         << PHTML::Heading(1)
         << statusInfo->code
         << ' '
         << statusInfo->text
         << PHTML::Heading(1)
         << extra
         << PHTML::Body();
    reply = html;
  }

  headers.SetAt(ContentTypeTag(), "text/html");
  StartResponse(code, headers, reply.GetLength());
  WriteString(reply);
  return statusInfo->code == RequestOK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPListener

PHTTPListener::PHTTPListener(unsigned maxWorkers)
  : m_listenerPort(80)
  , m_listenerThread(NULL)
  , m_threadPool(maxWorkers, 0, "HTTP-Service")
{
}


bool PHTTPListener::ListenForHTTP(WORD port, PSocket::Reusability reuse)
{
  return ListenForHTTP(PString::Empty(), port, reuse);
}


bool PHTTPListener::ListenForHTTP(const PString & interfaces, WORD port, PSocket::Reusability reuse)
{
  if (port == 0) {
    PAssertAlways(PInvalidParameter);
    return false;
  }

  if (m_listenerInterfaces == interfaces && m_listenerPort == port)
    return true;

  ShutdownListeners();
  m_listenerInterfaces = interfaces;
  m_listenerPort = port;

  PStringArray ifaces = interfaces.Tokenise(',');
  if (ifaces.IsEmpty()) {
    ifaces.AppendString("0.0.0.0");
#if P_HAS_IPV6
    ifaces.AppendString("[::]");
#endif
  }

  bool atLeastOne = false;
  for (PINDEX i = 0; i < ifaces.GetSize(); ++i) {
    PIPSocket::Address binding(ifaces[i]);
    if (binding.IsValid()) {
      PTCPSocket * listener = new PTCPSocket(port);
      if (listener->Listen(binding, 5, 0, reuse)) {
        PTRACE(3, "Listening for HTTP on " << listener->GetLocalAddress());
        m_httpListeningSockets.Append(listener);
        atLeastOne = true;
      }
      else {
        PTRACE(2, "Listen on port " << binding << ':' << listener->GetPort() << " failed: " << listener->GetErrorText());
        delete listener;
      }
    }
    else {
      PTRACE(2, "Invalid interface address \"" << ifaces[i] << '"');
    }
  }

  if (atLeastOne)
    m_listenerThread = new PThreadObj<PHTTPListener>(*this, &PHTTPListener::ListenMain, false, "HTTP-Listen");

  return atLeastOne;
}


void PHTTPListener::ShutdownListeners()
{
  PTRACE_IF(3, !m_httpListeningSockets.IsEmpty(),
            "Closing listener socket on port " << m_httpListeningSockets.front().GetPort());

  for (PSocketList::iterator it = m_httpListeningSockets.begin(); it != m_httpListeningSockets.end(); ++it)
    it->Close();

  if (m_listenerThread != NULL) {
    PAssert(m_listenerThread->WaitForTermination(10000), "HTTP service listener did not terminate promptly");
    delete m_listenerThread;
    m_listenerThread = NULL;
  }

  m_httpListeningSockets.RemoveAll();

  m_threadPool.Shutdown();
}


void PHTTPListener::ListenMain()
{
  while (IsListening()) {
    PSocket::SelectList listeners;
    for (PSocketList::iterator it = m_httpListeningSockets.begin(); it != m_httpListeningSockets.end(); ++it)
      listeners += *it;

    PChannel::Errors error = PSocket::Select(listeners);
    if (error == PChannel::NoError) {
      // get a socket(s) when a client connects
      for (PSocket::SelectList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
        PTCPSocket * socket = new PTCPSocket;
        if (socket->Accept(*it)) {
          PTRACE(5, "Queuing thread pool work for: local=" << socket->GetLocalAddress() << ", peer=" << socket->GetPeerAddress());
          m_threadPool.AddWork(new Worker(*this, socket));
        }
        else {
          if (socket->GetErrorCode() != PChannel::Interrupted) {
            PTRACE(2, "Accept failed for HTTP: " << socket->GetErrorText());
          }
          delete socket;
        }
      }
    }
    else if (error != PChannel::Interrupted) {
      PTRACE(2, "Select failed for HTTP: " << PSocket::GetErrorText(error));
    }
  }
}


PChannel * PHTTPListener::CreateChannelForHTTP(PChannel * channel)
{
  return channel;
}


PHTTPServer * PHTTPListener::CreateServerForHTTP()
{
  return new PHTTPServer(m_httpNameSpace);
}


void PHTTPListener::OnHTTPStarted(PHTTPServer & /*server*/)
{
}


void PHTTPListener::OnHTTPEnded(PHTTPServer & /*server*/)
{
}


PHTTPListener::Worker::~Worker()
{
  delete m_socket;
}


void PHTTPListener::Worker::Work()
{
  if (PAssertNULL(m_socket) == NULL)
    return;

#ifdef SO_LINGER
  const linger ling = { 1, 5 };
  m_socket->SetOption(SO_LINGER, &ling, sizeof(ling));
#endif

#if PTRACING
  PStringStream socketInfo;
  socketInfo << ": local=" << m_socket->GetLocalAddress() << ", peer=" << m_socket->GetPeerAddress();
#endif
  PTRACE(5, "Processing thread pool work for" << socketInfo);

  std::auto_ptr<PHTTPServer> server(m_listener.CreateServerForHTTP());
  if (server.get() == NULL) {
    PTRACE(2, "Creation failed" << socketInfo);
    return;
  }

  PChannel * channel = m_listener.CreateChannelForHTTP(m_socket);
  if (channel == NULL) {
    PTRACE(2, "Indirect channel creation failed" << socketInfo);
    return;
  }

  m_socket = NULL; // Is now auto-deleted by server.

  if (!server->Open(channel)) {
    PTRACE(2, "Open failed" << socketInfo);
    return;
  }

  PTRACE(5, "Started" << socketInfo);
  m_listener.OnHTTPStarted(*server);

  // process requests
  while (server->ProcessCommand())
    ;

  m_listener.OnHTTPEnded(*server);
  PTRACE(5, "Ended" << socketInfo);
}



//////////////////////////////////////////////////////////////////////////////
// PHTTPSimpleAuth

void PHTTPAuthority::DecodeBasicAuthority(const PString & authInfo,
                                          PString & username,
                                          PString & password)
{
  PString decoded;
  if (authInfo(0, 5) *= "Basic ")
    decoded = PBase64::Decode(authInfo(6, P_MAX_INDEX));
  else
    decoded = PBase64::Decode(authInfo);

  PINDEX colonPos = decoded.Find(':');
  if (colonPos == P_MAX_INDEX) {
    username = decoded;
    password = PString();
  }
  else {
    username = decoded.Left(colonPos).Trim();
    password = decoded.Mid(colonPos+1).Trim();
  }
}


PBoolean PHTTPAuthority::IsActive() const
{
  return true;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSimpleAuth

PHTTPSimpleAuth::PHTTPSimpleAuth(const PString & realm_,
                                 const PString & username_,
                                 const PString & password_)
  : realm(realm_), username(username_), password(password_)
{
  PAssert(!realm, "Must have a realm!");
}


PObject * PHTTPSimpleAuth::Clone() const
{
  return new PHTTPSimpleAuth(realm, username, password);
}


PBoolean PHTTPSimpleAuth::IsActive() const
{
  return !username || !password;
}


PString PHTTPSimpleAuth::GetRealm(const PHTTPRequest &) const
{
  return realm;
}


PBoolean PHTTPSimpleAuth::Validate(const PHTTPRequest &,
                               const PString & authInfo) const
{
  PString user, pass;
  DecodeBasicAuthority(authInfo, user, pass);
  return username == user && password == pass;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPMultiSimpAuth

PHTTPMultiSimpAuth::PHTTPMultiSimpAuth(const PString & realm_)
  : realm(realm_)
{
  PAssert(!realm, "Must have a realm!");
}


PHTTPMultiSimpAuth::PHTTPMultiSimpAuth(const PString & realm_,
                                       const PStringToString & users_)
  : realm(realm_), users(users_)
{
  PAssert(!realm, "Must have a realm!");
}


PObject * PHTTPMultiSimpAuth::Clone() const
{
  return new PHTTPMultiSimpAuth(realm, users);
}


PBoolean PHTTPMultiSimpAuth::IsActive() const
{
  return !users.IsEmpty();
}


PString PHTTPMultiSimpAuth::GetRealm(const PHTTPRequest &) const
{
  return realm;
}


PBoolean PHTTPMultiSimpAuth::Validate(const PHTTPRequest &,
                                  const PString & authInfo) const
{
  PString user, pass;
  DecodeBasicAuthority(authInfo, user, pass);
  return users.Contains(user) && users[user] == pass;
}


void PHTTPMultiSimpAuth::AddUser(const PString & username, const PString & password)
{
  users.SetAt(username, password);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPRequest

PHTTPRequest::PHTTPRequest(const PURL & _url,
                      const PMIMEInfo & _mime,
                 const PMultiPartList & _multipartFormInfo,
                        PHTTPResource * resource,
                          PHTTPServer & _server)
  : server(_server)
  , url(_url)
  , inMIME(_mime)
  , multipartFormInfo(_multipartFormInfo)
  , code(PHTTP::RequestOK)
  , contentSize(P_MAX_INDEX)
  , origin(0)
  , localAddr(0)
  , localPort(0)
  , m_resource(resource)
{
  PIPSocket * socket = server.GetSocket();
  if (socket != NULL) {
    socket->GetPeerAddress(origin);
    socket->GetLocalAddress(localAddr, localPort);
  }
}


//////////////////////////////////////////////////////////////////////////////
// PWebSocket

#if P_SSL

PWebSocket::PWebSocket()
  : m_client(false)
  , m_fragmentingWrite(false)
  , m_binaryWrite(false)
  , m_remainingPayload(0)
  , m_currentMask(-1)
  , m_fragmentedRead(false)
  , m_recursiveRead(false)
{
}


PBoolean PWebSocket::Read(void * buf, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  if (m_recursiveRead)
    return PIndirectChannel::Read(buf, len);

  bool ok = false;
  m_recursiveRead = true;

  while (m_remainingPayload == 0) {
    OpCodes  opCode;
    if (!ReadHeader(opCode, m_fragmentedRead, m_remainingPayload, m_currentMask))
      goto badRead;

    switch (opCode) {
      case Ping :
        WriteHeader(Pong, false, 0, -1);
        break;

      case ConnectionClose :
        lastReadCount = 0;
        return false;

      default:
        break;
    }
  }

  if (len > (PINDEX)m_remainingPayload)
    len = (PINDEX)m_remainingPayload;

  if (!PIndirectChannel::ReadBlock(buf, len))
    goto badRead;

  if (m_currentMask >= 0) {
    BYTE * ptr = (BYTE *)buf;
    PINDEX count = GetLastReadCount();
    while (count >= 4) {
      *(uint32_t *)ptr ^= m_currentMask;
      count -= 4;
      ptr += 4;
    }
    switch (count) {
      case 1 :
        *(BYTE *)ptr ^= m_currentMask;
        break;
      case 2:
        *(uint16_t *)ptr ^= m_currentMask;
        break;
      case 3:
        *(uint16_t *)ptr ^= m_currentMask;
        *(BYTE *)(ptr+2) ^= m_currentMask>>16;
        break;
    }
  }

  m_remainingPayload -= GetLastReadCount();
  ok = true;

badRead:
  m_recursiveRead = false;
  return ok;
}


bool PWebSocket::ReadMessage(PBYTEArray & msg)
{
  if (!PAssert(m_remainingPayload == 0, "Cannot call ReadMessage whan have partial frames unread."))
    return false;

  PINDEX totalSize = 0;
  do {
    static const PINDEX chunkSize = 10000;
    if (!Read(msg.GetPointer(totalSize + chunkSize) + totalSize, chunkSize))
      return false;
    totalSize += GetLastReadCount();
  } while (!IsMessageComplete());

  msg.SetSize(totalSize);

  return true;
}


PBoolean PWebSocket::Write(const void * buf, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  if (!m_client)
    return WriteHeader(m_binaryWrite ? BinaryFrame : TextFrame, m_fragmentingWrite, len, -1) && PIndirectChannel::Write(buf, len);

  uint32_t mask = PRandom::Number();
  if (!WriteHeader(m_binaryWrite ? BinaryFrame : TextFrame, m_fragmentingWrite, len, mask))
    return false;

  const uint32_t * ptr = (const uint32_t *)buf;
  while (len > 65536) {
    if (!WriteMasked(ptr, 65536, mask))
      return false;
    ptr += 16384;
    len -= 65536;
  }

  return WriteMasked(ptr, len, mask);
}


bool PWebSocket::WriteMasked(const uint32_t * data, PINDEX len, uint32_t mask)
{
  uint32_t buffer[16384];
  PINDEX i = (len+3) / 4;
  while (i-- > 0)
    buffer[i] = data[i] ^ mask;

  return PIndirectChannel::Write(buffer, len);
}


bool PWebSocket::Connect(const PStringArray & protocols, PString * selectedProtocol)
{
  PHTTPClient http;
  if (!http.Open(*this))
    return false;

  PString key = PBase64::Encode("What is in here?");
  PMIMEInfo outMIME, replyMIME;
  outMIME.SetAt(PHTTP::ConnectionTag(), PHTTP::UpgradeTag());
  outMIME.SetAt(PHTTP::UpgradeTag(), PHTTP::WebSocketTag());
  outMIME.SetAt(PHTTP::WebSocketVersionTag(), "13");
  outMIME.SetAt(PHTTP::WebSocketProtocolTag(), PSTRSTRM(std::setfill(',') << protocols));
  outMIME.SetAt(PHTTP::WebSocketKeyTag(), key);

  if (!http.GetDocument(PURL(), outMIME, replyMIME))
    return false;

  if (replyMIME(PHTTP::WebSocketAcceptTag()) != PMessageDigestSHA1::Encode(key + WebSocketGUID)) {
    PTRACE(2, "WebSocket reply accept is unacceptable.");
    return false;
  }

  PString protocol = outMIME(PHTTP::WebSocketProtocolTag());
  if (protocols.GetValuesIndex(protocol) == P_MAX_INDEX) {
    PTRACE(2, "WebSocket selected a protocol we did not offer.");
    return false;
  }

  if (selectedProtocol != NULL)
    * selectedProtocol = protocol;

  PTRACE(3, "WebSocket started for protocol: " << protocol);
  m_client = true;
  return true;
}


bool PWebSocket::ReadHeader(OpCodes  & opCode,
                            bool     & fragment,
                            uint64_t & payloadLength,
                            int64_t  & masking)
{
  BYTE header1;
  if (!Read(&header1, 1))
    return false;

  fragment = (header1 & 0x80) == 0;
  opCode = (OpCodes)(header1 & 0xf);

  PTimeInterval oldTimeout = GetReadTimeout();
  bool ok = false;

  BYTE header2;
  if (!ReadBlock(&header2, 1))
    goto badHeader;

  switch (header2 & 0x7f) {
    case 126 :
    {
      PUInt16b len16;
      if (!ReadBlock(&len16, 2))
        goto badHeader;
      payloadLength = len16;
      break;
    }

    case 127 :
    {
      PUInt64b len64;
      if (!ReadBlock(&len64, 8))
        goto badHeader;
      payloadLength = len64;
      break;
    }

    default :
      payloadLength = header2 & 0x7f;
  }

  if ((header2 & 0x80) == 0)
    masking = -1;
  else {
    uint32_t mask32;
    if (!ReadBlock(&mask32, 4))
      goto badHeader;

    masking = mask32;
  }

  ok = true;

badHeader:
  SetReadTimeout(oldTimeout);
  return ok;
}


bool PWebSocket::WriteHeader(OpCodes  opCode,
                             bool     fragment,
                             uint64_t payloadLength,
                             int64_t  masking)
{
  BYTE header[14];
  PINDEX len = 2;

  header[0] = (BYTE)opCode;
  if (!fragment)
    header[0] |= 0x80;

  if (payloadLength < 126)
    header[1] = (BYTE)payloadLength;
  else if (payloadLength < 65536) {
    header[1] = 126;
    *(PUInt16b *)&header[len] = (uint16_t)payloadLength;
    len += 2;
  }
  else {
    header[1] = 127;
    *(PUInt64b *)&header[len] = payloadLength;
    len += 8;
  }

  if (masking >= 0) {
    header[1] |= 0x80;
    *(PUInt32b *)&header[len] = (uint32_t)masking;
    len += 4;
  }

  return PIndirectChannel::Write(header, len);
}

#endif //P_SSL


//////////////////////////////////////////////////////////////////////////////
// PHTTPConnectionInfo

PHTTPConnectionInfo::PHTTPConnectionInfo()
  : persistenceTimeout(0, DEFAULT_PERSIST_TIMEOUT) // maximum lifetime (in seconds) of persistent connections
{
  // maximum lifetime (in transactions) of persistent connections
  persistenceMaximum = DEFAULT_PERSIST_TRANSATIONS;

  commandCode       = PHTTP::NumCommands;

  majorVersion      = 0;
  minorVersion      = 9;

  isPersistent      = false;
  wasPersistent     = false;
  isProxyConnection = false;
  m_isWebSocket     = false;

  entityBodyLength  = -1;
}


PBoolean PHTTPConnectionInfo::Initialise(PHTTPServer & server, PString & args)
{
  // if only one argument, then it must be a version 0.9 simple request
  PINDEX lastSpacePos = args.FindLast(' ');
  if (lastSpacePos == P_MAX_INDEX || strncasecmp(&args[lastSpacePos+1], "HTTP/", 5) != 0) {
    majorVersion = 0;
    minorVersion = 9;
    return true;
  }

  // otherwise, attempt to extract a version number
  PINDEX dotPos = args.Find('.', lastSpacePos+6);
  if (dotPos == 0 || dotPos == P_MAX_INDEX) {
    server.OnError(PHTTP::BadRequest, "Malformed version number: " + args, *this);
    return false;
  }

  // should actually check if the text contains only digits, but the
  // chances of matching everything else and it not being a valid number
  // are pretty small, so don't bother
  majorVersion = atoi(&args[lastSpacePos+6]);
  minorVersion = atoi(&args[dotPos+1]);
  args.Delete(lastSpacePos, P_MAX_INDEX);

  // build our connection info reading MIME info until an empty line is
  // received, or EOF
  if (!mimeInfo.Read(server))
    return false;

  wasPersistent = isPersistent;
  isPersistent = false;

  // check for Proxy-Connection and Connection strings
  PString str = mimeInfo(PHTTP::ProxyConnectionTag());
  isProxyConnection = !str.IsEmpty();
  if (!isProxyConnection)
    str = mimeInfo(PHTTP::ConnectionTag());

  // get any connection options
  if (!str) {
    PStringArray tokens = str.Tokenise(", \t\r\n", false);
    for (PINDEX i = 0; i < tokens.GetSize(); i++) {
      PCaselessString token(tokens[i]);
      if (token == PHTTP::KeepAliveTag())
        isPersistent = true;
      else if (token == PHTTP::UpgradeTag()) {
        if (PHTTP::WebSocketTag() != mimeInfo(PHTTP::UpgradeTag()) || mimeInfo(PHTTP::WebSocketVersionTag()) != "13")
          return server.OnError(PHTTP::MethodNotAllowed, "Cannot upgrade to protocol or version", *this);
        m_isWebSocket = true;
      }
    }
  }

  // If the protocol is version 1.0 or greater, there is MIME info, and the
  // prescence of a an entity body is signalled by the inclusion of
  // Content-Length header. If the protocol is less than version 1.0, then 
  // there is no entity body!

  // if the client specified a persistent connection, then use the
  // ContentLength field. If there is no content length field, then
  // assume a ContentLength of zero and close the connection.
  // The spec actually says to read until end of file in this case,
  // but Netscape hangs if this is done.
  // If the client didn't specify a persistent connection, then use the
  // ContentLength if there is one or read until end of file if there isn't
  if (!isPersistent)
    entityBodyLength = mimeInfo.GetInteger(PHTTP::ContentLengthTag, (commandCode == PHTTP::POST) ? -2 : 0);
  else {
    entityBodyLength = mimeInfo.GetInteger(PHTTP::ContentLengthTag, -1);
    if (entityBodyLength < 0) {
      PTRACE(5, "Persistent connection has no content length");
      entityBodyLength = 0;
      mimeInfo.SetAt(PHTTP::ContentLengthTag, "0");
    }
  }

  return true;
}


void PHTTPConnectionInfo::SetMIME(const PString & tag, const PString & value)
{
  mimeInfo.MakeUnique();
  mimeInfo.SetAt(tag, value);
}


PBoolean PHTTPConnectionInfo::IsCompatible(int major, int minor) const
{
  if (minor == 0 && major == 0)
    return true;
  else
    return (majorVersion > major) ||
           ((majorVersion == major) && (minorVersion >= minor));
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPResource

PHTTPResource::PHTTPResource(const PURL & url)
  : baseURL(url)
{
  authority = NULL;
  hitCount = 0;
}


PHTTPResource::PHTTPResource(const PURL & url, const PHTTPAuthority & auth)
  : baseURL(url)
{
  authority = (PHTTPAuthority *)auth.Clone();
  hitCount = 0;
}


PHTTPResource::PHTTPResource(const PURL & url, const PString & type)
  : baseURL(url), contentType(type)
{
  authority = NULL;
  hitCount = 0;
}


PHTTPResource::PHTTPResource(const PURL & url,
                             const PString & type,
                             const PHTTPAuthority & auth)
  : baseURL(url), contentType(type)
{
  authority = (PHTTPAuthority *)auth.Clone();
  hitCount = 0;
}


PHTTPResource::~PHTTPResource()
{
  delete authority;
}


bool PHTTPResource::OnWebSocket(PHTTPServer &, PHTTPConnectionInfo &)
{
  return false;
}


bool PHTTPResource::OnGET(PHTTPServer & server, const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(server, connectInfo.GetURL(), connectInfo.GetMIME(), connectInfo, true);
}


PBoolean PHTTPResource::OnGET(PHTTPServer & server,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(server, url, info, connectInfo, true);
}


bool PHTTPResource::OnHEAD(PHTTPServer & server, const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(server, connectInfo.GetURL(), connectInfo.GetMIME(), connectInfo, false);
}


PBoolean PHTTPResource::OnHEAD(PHTTPServer & server,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(server, url, info, connectInfo, false);
}


PBoolean PHTTPResource::OnGETOrHEAD(PHTTPServer & server,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo,
                                   PBoolean isGET)
{
  // Nede to split songle if into 2 so the Tornado compiler won't end with
  // 'internal compiler error'
  if (isGET && info.Contains(PHTTP::IfModifiedSinceTag()))
    if (!IsModifiedSince(PTime(info[PHTTP::IfModifiedSinceTag()])))
      return server.OnError(PHTTP::NotModified, url.AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(url,
                                         info,
                                         connectInfo.GetMultipartFormInfo(),
                                         server);

  PBoolean retVal = true;
  if (CheckAuthority(server, *request, connectInfo)) {
    retVal = false;
    server.SetDefaultMIMEInfo(request->outMIME, connectInfo);

    PTime expiryDate(0);
    if (GetExpirationDate(expiryDate))
      request->outMIME.Set(PHTTP::ExpiresTag, expiryDate.AsString(PTime::RFC1123, PTime::GMT));

    if (!LoadHeaders(*request)) 
      retVal = server.OnError(request->code, url.AsString(), connectInfo);
    else if (!isGET)
      retVal = request->outMIME.Contains(PHTTP::ContentLengthTag());
    else {
      hitCount++;
      retVal = OnGETData(server, url, connectInfo, *request);
    }
  }

  delete request;
  return retVal;
}


PBoolean PHTTPResource::OnGETData(PHTTPServer & /*server*/,
                               const PURL & /*url*/,
                const PHTTPConnectionInfo & /*connectInfo*/,
                             PHTTPRequest & request)
{
  SendData(request);
  return request.outMIME.Contains(PHTTP::ContentLengthTag()) ||
         request.outMIME.Contains(PHTTP::TransferEncodingTag());
}


bool PHTTPResource::OnPOST(PHTTPServer & server, const PHTTPConnectionInfo & connectInfo)
{
  PStringToString postData;
  static const PConstCaselessString UrlEncoded("application/x-www-form-urlencoded");
  if (UrlEncoded == connectInfo.GetMIME().Get(PHTTP::ContentTypeTag(), UrlEncoded))
    PURL::SplitQueryVars(connectInfo.GetEntityBody(), postData);
  return OnPOST(server, connectInfo.GetURL(), connectInfo.GetMIME(), postData, connectInfo);
}


PBoolean PHTTPResource::OnPOST(PHTTPServer & server,
                            const PURL & url,
                       const PMIMEInfo & info,
                 const PStringToString & data,
             const PHTTPConnectionInfo & connectInfo)
{
  PHTTPRequest * request = CreateRequest(url,
                                         info,
                                         connectInfo.GetMultipartFormInfo(),
                                         server);

  request->entityBody = connectInfo.GetEntityBody();

  PBoolean persist = true;
  if (CheckAuthority(server, *request, connectInfo)) {
    server.SetDefaultMIMEInfo(request->outMIME, connectInfo);
    persist = OnPOSTData(*request, data);
    if (request->code != PHTTP::RequestOK)
      persist = server.OnError(request->code, "", connectInfo) && persist;
  }

  delete request;
  return persist;
}


PBoolean PHTTPResource::OnPOSTData(PHTTPRequest & request,
                               const PStringToString & data)
{
  PHTML msg;
  PBoolean persist = Post(request, data, msg);

  if (msg.Is(PHTML::InBody))
    msg << PHTML::Body();

  if (request.code != PHTTP::RequestOK)
    return persist;

  if (msg.IsEmpty())
    msg << PHTML::Title()    << (unsigned)PHTTP::RequestOK << " OK" << PHTML::Body()
        << PHTML::Heading(1) << (unsigned)PHTTP::RequestOK << " OK" << PHTML::Heading(1)
        << PHTML::Body();

  request.outMIME.SetAt(PHTTP::ContentTypeTag(), "text/html");

  PINDEX len = msg.GetLength();
  request.server.StartResponse(request.code, request.outMIME, len);
  return request.server.Write((const char *)msg, len) && persist;
}


PBoolean PHTTPResource::CheckAuthority(PHTTPServer & server,
                            const PHTTPRequest & request,
                     const PHTTPConnectionInfo & connectInfo)
{
  if (authority == NULL)
    return true;

  return CheckAuthority(*authority, server, request, connectInfo);
}
    
    
PBoolean PHTTPResource::CheckAuthority(PHTTPAuthority & authority,
                                      PHTTPServer & server,
                               const PHTTPRequest & request,
                        const PHTTPConnectionInfo & connectInfo)
{
  if (!authority.IsActive())
    return true;


  // if this is an authorisation request...
  if (request.inMIME.Contains(PHTTP::AuthorizationTag()) &&
      authority.Validate(request, request.inMIME[PHTTP::AuthorizationTag()]))
    return true;

  // it must be a request for authorisation
  PMIMEInfo headers;
  server.SetDefaultMIMEInfo(headers, connectInfo);
  headers.SetAt(PHTTP::WWWAuthenticateTag(),
                       "Basic realm=\"" + authority.GetRealm(request) + "\"");
  headers.SetAt(PHTTP::ContentTypeTag(), "text/html");

  const httpStatusCodeStruct * statusInfo =
                               GetStatusCodeStruct(PHTTP::UnAuthorised);

  PHTML reply;
  reply << PHTML::Title()
        << statusInfo->code
        << ' '
        << statusInfo->text
        << PHTML::Body()
        << PHTML::Heading(1)
        << statusInfo->code
        << ' '
        << statusInfo->text
        << PHTML::Heading(1)
        << "Your request cannot be authorised because it requires authentication."
        << PHTML::Paragraph()
        << "This may be because you entered an incorrect username or password, "
        << "or because your browser is not performing Basic authentication."
        << PHTML::Body();

  server.StartResponse(PHTTP::UnAuthorised, headers, reply.GetLength());
  server.WriteString(reply);

  return false;
}


void PHTTPResource::SetAuthority(const PHTTPAuthority & auth)
{
  delete authority;
  authority = (PHTTPAuthority *)auth.Clone();
}


void PHTTPResource::ClearAuthority()
{
  delete authority;
  authority = NULL;
}


PBoolean PHTTPResource::IsModifiedSince(const PTime &)
{
  return true;
}


PBoolean PHTTPResource::GetExpirationDate(PTime &)
{
  return false;
}


PHTTPRequest * PHTTPResource::CreateRequest(const PURL & url,
                                            const PMIMEInfo & inMIME,
                                            const PMultiPartList & multipartFormInfo,
                                            PHTTPServer & socket)
{
  return new PHTTPRequest(url, inMIME, multipartFormInfo, this, socket);
}


static void WriteChunkedDataToServer(PHTTPServer & server, PCharArray & data)
{
  if (data.GetSize() == 0)
    return;

  server << data.GetSize() << "\r\n";
  server.Write(data, data.GetSize());
  server << "\r\n";
  data.SetSize(0);
}


PBoolean PHTTPResource::LoadHeaders(PHTTPRequest & request)
{
	request.code = PHTTP::MethodNotAllowed;
	return false;
}


void PHTTPResource::SendData(PHTTPRequest & request)
{
  if (!request.outMIME.Contains(PHTTP::ContentTypeTag) && !contentType)
    request.outMIME.SetAt(PHTTP::ContentTypeTag, contentType);

  PCharArray data;
  if (LoadData(request, data)) {
    if (request.server.StartResponse(request.code, request.outMIME, request.contentSize)) {
      // Chunked transfer encoding
      request.outMIME.RemoveAll();
      do {
        WriteChunkedDataToServer(request.server, data);
      } while (LoadData(request, data));
      WriteChunkedDataToServer(request.server, data);
      request.server << "0\r\n" << request.outMIME;
    }
    else {
      do {
        request.server.Write(data, data.GetSize());
        data.SetSize(0);
      } while (LoadData(request, data));
      request.server.Write(data, data.GetSize());
    }
  }
  else {
    request.server.StartResponse(request.code, request.outMIME, data.GetSize());
    request.server.write(data, data.GetSize());
  }
}


PBoolean PHTTPResource::LoadData(PHTTPRequest & request, PCharArray & data)
{
  PString text = LoadText(request);
  OnLoadedText(request, text);
  if(data.SetSize(text.GetLength()))
    memcpy(data.GetPointer(), (const char *)text, text.GetLength()); // Lose the trailing '\0'
  return false;
}


PString PHTTPResource::LoadText(PHTTPRequest &)
{
  PAssertAlways(PUnimplementedFunction);
  return PString();
}


void PHTTPResource::OnLoadedText(PHTTPRequest &, PString &)
{
  // Do nothing
}


PBoolean PHTTPResource::Post(PHTTPRequest & request,
                         const PStringToString &,
                         PHTML & msg)
{
  request.code = PHTTP::MethodNotAllowed;
  msg = "Error in POST";
  msg << "Post to this resource is not allowed" << PHTML::Body();
  return true;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPString

PHTTPString::PHTTPString(const PURL & url)
  : PHTTPResource(url, "text/html")
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, "text/html", auth)
{
}


PHTTPString::PHTTPString(const PURL & url, const PString & str)
  : PHTTPResource(url, "text/html"), string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PString & type)
  : PHTTPResource(url, type), string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, "text/html", auth), string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PString & type,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth), string(str)
{
}


PBoolean PHTTPString::LoadHeaders(PHTTPRequest & request)
{
  request.contentSize = string.GetLength();
  return true;
}


PString PHTTPString::LoadText(PHTTPRequest &)
{
  return string;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

PHTTPFile::PHTTPFile(const PURL & url, int)
  : PHTTPResource(url)
{
}


PHTTPFile::PHTTPFile(const PString & filename)
  : PHTTPResource(filename, PMIMEInfo::GetContentType(PFilePath(filename).GetType())),
    filePath(filename)
{
}


PHTTPFile::PHTTPFile(const PString & filename, const PHTTPAuthority & auth)
  : PHTTPResource(filename, auth), filePath(filename)
{
}


PHTTPFile::PHTTPFile(const PURL & url, const PFilePath & path)
  : PHTTPResource(url, PMIMEInfo::GetContentType(path.GetType())),
    filePath(path)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PString & type)
  : PHTTPResource(url, type), filePath(path)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, PMIMEInfo::GetContentType(path.GetType()), auth),
    filePath(path)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PString & type,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth), filePath(path)
{
}


PHTTPFileRequest::PHTTPFileRequest(const PURL & url,
                              const PMIMEInfo & inMIME,
                         const PMultiPartList & multipartFormInfo,
                                PHTTPResource * resource,
                                  PHTTPServer & server)
  : PHTTPRequest(url, inMIME, multipartFormInfo, resource, server)
{
}


PHTTPRequest * PHTTPFile::CreateRequest(const PURL & url,
                                   const PMIMEInfo & inMIME,
                              const PMultiPartList & multipartFormInfo,
                                       PHTTPServer & server)
{
  return new PHTTPFileRequest(url, inMIME, multipartFormInfo, this, server);
}


PBoolean PHTTPFile::LoadHeaders(PHTTPRequest & request)
{
  PFile & file = ((PHTTPFileRequest&)request).file;

  if (!file.Open(filePath, PFile::ReadOnly)) {
    request.code = PHTTP::NotFound;
    return false;
  }

  request.contentSize = file.GetLength();
  return true;
}


PBoolean PHTTPFile::LoadData(PHTTPRequest & request, PCharArray & data)
{
  PFile & file = ((PHTTPFileRequest&)request).file;

  PString contentType = GetContentType();
  if (contentType.IsEmpty())
    contentType = PMIMEInfo::GetContentType(file.GetFilePath().GetType());

  if (contentType(0, 4) *= "text/")
    return PHTTPResource::LoadData(request, data);

  PAssert(file.IsOpen(), PLogicError);

  PINDEX count = file.GetLength() - file.GetPosition();
  if (count > 10000)
    count = 10000;

  if (count > 0)
    PAssert(file.Read(data.GetPointer(count), count), PLogicError);

  if (!file.IsEndOfFile())
    return true;

  file.Close();
  return false;
}


PString PHTTPFile::LoadText(PHTTPRequest & request)
{
  PString text;
  PFile & file = ((PHTTPFileRequest&)request).file;
  if (PAssert(file.IsOpen(), PLogicError)) {
    text = file.ReadString(file.GetLength());
    PAssert(file.Close(), PLogicError);
  }
  return text;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPTailFile

PHTTPTailFile::PHTTPTailFile(const PString & filename)
  : PHTTPFile(filename)
{
}


PHTTPTailFile::PHTTPTailFile(const PString & filename,
                             const PHTTPAuthority & auth)
  : PHTTPFile(filename, auth)
{
}


PHTTPTailFile::PHTTPTailFile(const PURL & url,
                             const PFilePath & file)
  : PHTTPFile(url, file)
{
}


PHTTPTailFile::PHTTPTailFile(const PURL & url,
                             const PFilePath & file,
                             const PString & contentType)
  : PHTTPFile(url, file, contentType)
{
}


PHTTPTailFile::PHTTPTailFile(const PURL & url,
                             const PFilePath & file,
                             const PHTTPAuthority & auth)
  : PHTTPFile(url, file, auth)
{
}


PHTTPTailFile::PHTTPTailFile(const PURL & url,
                             const PFilePath & file,
                             const PString & contentType,
                             const PHTTPAuthority & auth)
  : PHTTPFile(url, file, contentType, auth)
{
}


PBoolean PHTTPTailFile::LoadHeaders(PHTTPRequest & request)
{
  if (!PHTTPFile::LoadHeaders(request))
    return false;

  request.contentSize = P_MAX_INDEX;
  return true;
}


PBoolean PHTTPTailFile::LoadData(PHTTPRequest & request, PCharArray & data)
{
  PFile & file = ((PHTTPFileRequest&)request).file;

  if (file.GetPosition() == 0)
    file.SetPosition(file.GetLength()-request.url.GetQueryVars()("offset", "10000").AsUnsigned());

  while (file.GetPosition() >= file.GetLength()) {
    if (!request.server.Write(NULL, 0))
      return false;
    PThread::Sleep(200);
  }

  PINDEX count = file.GetLength() - file.GetPosition();
  return file.Read(data.GetPointer(count), count);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

PHTTPDirectory::PHTTPDirectory(const PURL & url, const PDirectory & dir)
  : PHTTPFile(url, 0), basePath(dir), allowDirectoryListing(true)
{
}


PHTTPDirectory::PHTTPDirectory(const PURL & url,
                               const PDirectory & dir,
                               const PHTTPAuthority & auth)
  : PHTTPFile(url, PString(), auth), basePath(dir), allowDirectoryListing(true)
{
}


PHTTPDirRequest::PHTTPDirRequest(const PURL & url,
                            const PMIMEInfo & inMIME,
                       const PMultiPartList & multipartFormInfo,
                              PHTTPResource * resource,
                                PHTTPServer & server)
  : PHTTPFileRequest(url, inMIME, multipartFormInfo, resource, server)
{
}


PHTTPRequest * PHTTPDirectory::CreateRequest(const PURL & url,
                                        const PMIMEInfo & inMIME,
                                   const PMultiPartList & multipartFormInfo,
                                            PHTTPServer & socket)
{
  PHTTPDirRequest * request = new PHTTPDirRequest(url, inMIME, multipartFormInfo, this, socket);

  const PStringArray & path = url.GetPath();
  request->realPath = basePath;
  PINDEX i;
  for (i = GetURL().GetPath().GetSize(); i < path.GetSize()-1; i++)
    request->realPath += path[i] + PDIR_SEPARATOR;

  // append the last path element
  if (i < path.GetSize())
    request->realPath += path[i];

  if (request->realPath.Find(basePath) != 0)
    request->realPath = basePath;

  return request;
}


void PHTTPDirectory::EnableAuthorisation(const PString & realm)
{
  authorisationRealm = realm;
}


PBoolean PHTTPDirectory::FindAuthorisations(const PDirectory & dir, PString & realm, PStringToString & authorisations)
{
  PFilePath fn = dir + accessFilename;
  PTextFile file;
  PBoolean first = true;
  if (file.Open(fn, PFile::ReadOnly)) {
    PString line;
    while (file.ReadLine(line)) {
      if (first) {
        realm = line.Trim();
        first = false;
      } else {
        PStringArray tokens = line.Tokenise(':');
        if (tokens.GetSize() > 1)
          authorisations.SetAt(tokens[0].Trim(), tokens[1].Trim());
      }
    }
    return true;
  }
    
  if (dir.IsRoot() || (dir == basePath))
    return false;

  return FindAuthorisations(dir.GetParent(), realm, authorisations);
}

PBoolean PHTTPDirectory::CheckAuthority(PHTTPServer & server,
                             const PHTTPRequest & request,
                      const PHTTPConnectionInfo & conInfo)
{
  // if access control is enabled, then search parent directories for password files
  PStringToString authorisations;
  PString newRealm;
  if (authorisationRealm.IsEmpty() ||
      !FindAuthorisations(((PHTTPDirRequest&)request).realPath.GetDirectory(), newRealm, authorisations) ||
      authorisations.GetSize() == 0)
    return true;

  PHTTPMultiSimpAuth authority(newRealm, authorisations);
  return PHTTPResource::CheckAuthority(authority, server, request, conInfo);
}

PBoolean PHTTPDirectory::LoadHeaders(PHTTPRequest & request)
{
  PFilePath & realPath = ((PHTTPDirRequest&)request).realPath;
    
  // if not able to obtain resource information, then consider the resource "not found"
  PFileInfo info;
  if (!PFile::GetInfo(realPath, info)) {
    request.code = PHTTP::NotFound;
    return false;
  }

  // if the resource is a file, and the file can't be opened, then return "not found"
  PFile & file = ((PHTTPDirRequest&)request).file;
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(realPath, PFile::ReadOnly) ||
        (!authorisationRealm.IsEmpty() && realPath.GetFileName() == accessFilename)) {
      request.code = PHTTP::NotFound;
      return false;
    }
  } 

  // resource is a directory - if index files disabled, then return "not found"
  else if (!allowDirectoryListing) {
    request.code = PHTTP::NotFound;
    return false;
  }

  // else look for index files
  else {
    PINDEX i;
    for (i = 0; i < PARRAYSIZE(HTMLIndexFiles); i++)
      if (file.Open(realPath +
                          PDIR_SEPARATOR + HTMLIndexFiles[i], PFile::ReadOnly))
        break;
  }

  // open the file and return information
  PString & fakeIndex = ((PHTTPDirRequest&)request).fakeIndex;
  if (file.IsOpen()) {
    request.outMIME.SetAt(PHTTP::ContentTypeTag(),
                          PMIMEInfo::GetContentType(file.GetFilePath().GetType()));
    request.contentSize = file.GetLength();
    fakeIndex = PString();
    return true;
  }

  // construct a directory listing
  request.outMIME.SetAt(PHTTP::ContentTypeTag(), "text/html");
  PHTML reply("Directory of " + request.url.AsString());
  PDirectory dir = realPath;
  if (dir.Open()) {
    do {
      const char * imgName;
      if (dir.IsSubDir())
        imgName = "internal-gopher-menu";
      else if (PMIMEInfo::GetContentType(
                    PFilePath(dir.GetEntryName()).GetType())(0,4) == "text/")
        imgName = "internal-gopher-text";
      else
        imgName = "internal-gopher-unknown";
      reply << PHTML::Image(imgName) << ' '
            << PHTML::HotLink(realPath.GetFileName()+'/'+dir.GetEntryName())
            << dir.GetEntryName()
            << PHTML::HotLink()
            << PHTML::BreakLine();
    } while (dir.Next());
  }
  reply << PHTML::Body();
  fakeIndex = reply;

  return true;
}


PString PHTTPDirectory::LoadText(PHTTPRequest & request)
{
  PString & fakeIndex = ((PHTTPDirRequest&)request).fakeIndex;
  if (fakeIndex.IsEmpty())
    return PHTTPFile::LoadText(request);

  return fakeIndex;
}

#endif // P_HTTP


// End Of File ///////////////////////////////////////////////////////////////
