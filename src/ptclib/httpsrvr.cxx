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
  mutex = new PReadWriteMutex("HTTP Space");
  root = new Node(PString(), NULL);
}


void PHTTPSpace::DestroyContents()
{
  delete mutex;
  delete root;
}


void PHTTPSpace::CloneContents(const PHTTPSpace * c)
{
  mutex = new PReadWriteMutex("HTTP Space");
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
  : m_urlSpace(space)
{
  Construct();
}


void PHTTPServer::Construct()
{
  m_transactionCount = 0;
  SetReadLineTimeout(ReadLineTimeout);
}


PBoolean PHTTPServer::ProcessCommand()
{
  PString args;
  PINDEX cmd;

  // if this is not the first command received by this socket, then set
  // the read timeout appropriately.
  if (m_transactionCount > 0) 
    SetReadTimeout(m_nextTimeout);

  // this will only return false upon timeout or completely invalid command
  if (!ReadCommand(cmd, args))
    return false;

  PTime now;
  PTRACE(5, "Delay waiting for next command: " << now - m_lastCommandTime);
  m_lastCommandTime = now;

  m_connectInfo.commandCode = (Commands)cmd;
  if (cmd < NumCommands)
    m_connectInfo.commandName = commandNames[cmd];
  else {
    PINDEX spacePos = args.Find(' ');
    m_connectInfo.commandName = args.Left(spacePos);
    args = args.Mid(spacePos);
  }

  // if no tokens, error
  if (args.IsEmpty()) {
    OnError(BadRequest, args, m_connectInfo);
    return false;
  }

  if (!m_connectInfo.Initialise(*this, args))
    return false;

  // now that we've decided we did receive a HTTP request, increment the
  // count of transactions
  m_transactionCount++;
  m_nextTimeout = m_connectInfo.GetPersistenceTimeout();

  PIPSocket * socket = GetSocket();
  WORD myPort = (WORD)(socket != NULL ? socket->GetPort() : 80);

  // the URL that comes with Connect requests is not quite kosher, so 
  // mangle it into a proper URL and do NOT close the connection.
  // for all other commands, close the read connection if not persistent
  if (cmd == CONNECT) 
    m_connectInfo.url.Parse("https://" + args);
  else {
    m_connectInfo.url.Parse(args, "http");
    if (m_connectInfo.url.GetPort() == 0)
      m_connectInfo.url.SetPort(myPort);
  }

  bool persist;
  
  // make sure the form info is reset for each new operation
  m_connectInfo.ResetMultipartFormInfo();

  PTRACE(5, "Transaction: " << m_connectInfo.GetCommandName() << " \"" << args << "\","
            " url=" << m_connectInfo.GetURL() << ","
            " persist=" << std::boolalpha << m_connectInfo.IsPersistent() << ","
            " proxy=" << m_connectInfo.IsProxyConnection() << ","
            " websocket=" << m_connectInfo.IsWebSocket());

  if (m_connectInfo.IsWebSocket()) {
    if (!OnWebSocket(m_connectInfo))
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
    const PURL & url = m_connectInfo.GetURL();
    if (url.GetScheme() != "http" ||
        (url.GetPort() != 0 && url.GetPort() != myPort) ||
        (!url.GetHostName().IsEmpty() && !PIPSocket::IsLocalHost(url.GetHostName())))
      persist = OnProxy(m_connectInfo);
    else {
      m_connectInfo.entityBody = ReadEntityBody();
      persist = OnCommand(cmd, url, args, m_connectInfo);
    }
  }

  flush();

  // if the function just indicated that the connection is to persist,
  // and so did the client, then return true. Note that all of the OnXXXX
  // routines above must make sure that their return value is false if
  // if there was no ContentLength field in the response. This ensures that
  // we always close the socket so the client will get the correct end of file
  if (persist && m_connectInfo.IsPersistent()) {
    unsigned max = m_connectInfo.GetPersistenceMaximumTransations();
    if (max == 0 || m_transactionCount < max) {
      PTRACE(5, "Connection persisting: transactions=" << m_transactionCount << ", max=" << max);
      return true;
    }
  }

  PTRACE(5, "Connection persistence ended: requested=" << std::boolalpha << m_connectInfo.IsPersistent() << ", persist=" << persist);

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

    default:
      persist = OnUnknown(args, connectInfo);
  }

  return persist;
}


#if P_SSL
bool PHTTPServer::OnWebSocket(PHTTPConnectionInfo & connectInfo)
{
  const PMIMEInfo & mime = connectInfo.GetMIME();
  PString key = mime(WebSocketKeyTag());
  PString supportedGlobally;

  PStringArray protocols = mime(WebSocketProtocolTag()).Tokenise(", \t\r\n", false);
  for (PINDEX i = 0; i < protocols.GetSize(); ++i) {
    PString protocol = protocols[i];
    std::map<PString, WebSocketNotifier>::iterator notifier = m_webSocketNotifiers.find(protocol);
    if (notifier != m_webSocketNotifiers.end()) {
      if (notifier->second.IsNULL()) {
        supportedGlobally = protocol;
        break;
      }

      SwitchToWebSocket(protocol, key);
      notifier->second(*this, connectInfo);
      return !connectInfo.IsWebSocket();
    }
  }

  bool persist = false;
 
  m_urlSpace.StartRead();

  PHTTPResource * resource = m_urlSpace.FindResource(connectInfo.GetURL());
  if (resource == NULL) {
    if (!m_urlSpace.IsEmpty() || supportedGlobally.IsEmpty())
      persist = OnError(NotFound, connectInfo.GetURL().AsString(), connectInfo);
    else
      SwitchToWebSocket(supportedGlobally, key);
  }
  else if (!supportedGlobally.IsEmpty()) {
    SwitchToWebSocket(supportedGlobally, key);
    persist = resource->OnWebSocket(*this, connectInfo);
  }
  else {
    bool found = false;
    for (PINDEX i = 0; i < protocols.GetSize(); ++i) {
      PString protocol = protocols[i];
      if (resource->SupportsWebSocketProtocol(protocol)) {
        SwitchToWebSocket(protocol, key);
        persist = resource->OnWebSocket(*this, connectInfo);
        found = true;
        break;
      }
    }
    if (!found) {
      PTRACE(3, "Unsupported WebSocket protocols: " << setfill(',') << protocols);
      persist = OnError(NotFound, "Unsupported WebSocket protocol", connectInfo);
    }
  }

  m_urlSpace.EndRead();

  return persist;
}


void PHTTPServer::SwitchToWebSocket(const PString & protocol, const PString & key)
{
  PTRACE(4, "Switching to WebSocket protocol \"" << protocol << '"');
  PMIMEInfo reply;
  reply.SetAt(ConnectionTag(), UpgradeTag());
  reply.SetAt(UpgradeTag(), WebSocketTag());
  reply.SetAt(WebSocketProtocolTag(), protocol);
  reply.SetAt(WebSocketAcceptTag(), PMessageDigestSHA1::Encode(key + WebSocketGUID));

  StartResponse(SwitchingProtocols, reply, -1);
  flush();
}
#else
bool PHTTPServer::OnWebSocket(PHTTPConnectionInfo & connectInfo)
{
  PTRACE(2, "WebSocket refused due to no SSL");
  return OnError(NotFound, "WebSocket unsupported (No SSL)", connectInfo);
}
#endif // P_SSL


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
  if (m_connectInfo.GetMajorVersion() < 1)
    return PString();

  PString entityBody;
  long contentLength = m_connectInfo.GetEntityBodyLength();
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
  if (!m_connectInfo.IsPersistent()) {
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
  m_urlSpace = space;
}


bool PHTTPServer::OnGET(const PHTTPConnectionInfo & conInfo)
{
  m_urlSpace.StartRead();
  PHTTPResource * resource = m_urlSpace.FindResource(conInfo.GetURL());
  if (resource == NULL) {
    m_urlSpace.EndRead();
    return OnError(NotFound, conInfo.GetURL().AsString(), m_connectInfo);
  }

  bool retval = resource->OnGET(*this, m_connectInfo);
  m_urlSpace.EndRead();
  return retval;
}


bool PHTTPServer::OnHEAD(const PHTTPConnectionInfo & conInfo)
{
  m_urlSpace.StartRead();
  PHTTPResource * resource = m_urlSpace.FindResource(conInfo.GetURL());
  if (resource == NULL) {
    m_urlSpace.EndRead();
    return OnError(NotFound, conInfo.GetURL().AsString(), m_connectInfo);
  }

  bool retval = resource->OnHEAD(*this, m_connectInfo);
  m_urlSpace.EndRead();
  return retval;
}


bool PHTTPServer::OnPOST(const PHTTPConnectionInfo & conInfo)
{
  m_urlSpace.StartRead();
  PHTTPResource * resource = m_urlSpace.FindResource(conInfo.GetURL());
  if (resource == NULL) {
    m_urlSpace.EndRead();
    return OnError(NotFound, conInfo.GetURL().AsString(), m_connectInfo);
  }

  bool retval = resource->OnPOST(*this, m_connectInfo);
  m_urlSpace.EndRead();
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


bool PHTTPServer::StartResponse(StatusCode code,
                                PMIMEInfo & headers,
                                long bodySize)
{
  if (m_connectInfo.majorVersion < 1) 
    return false;

  httpStatusCodeStruct dummyInfo;
  const httpStatusCodeStruct * statusInfo;
  if (m_connectInfo.commandCode < NumCommands)
    statusInfo = GetStatusCodeStruct(code);
  else {
    dummyInfo.text = "";
    dummyInfo.code = code;
    dummyInfo.allowedBody = true;
    dummyInfo.majorVersion = m_connectInfo.majorVersion;
    dummyInfo.minorVersion = m_connectInfo.minorVersion;
    statusInfo = &dummyInfo;
  }

  // output the command line
  *this << "HTTP/" << m_connectInfo.majorVersion << '.' << m_connectInfo.minorVersion
        << ' ' << statusInfo->code << ' ' << statusInfo->text << "\r\n";

  PBoolean chunked = false;

  // If do not have user set content length, decide if we should add one
  if (!headers.Contains(ContentLengthTag())) {
    if (m_connectInfo.minorVersion < 1) {
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
  if (bodySize < 1024 && m_connectInfo.GetMIME()(UserAgentTag()).Find("Mozilla/2.0") != P_MAX_INDEX)
    m_nextTimeout.SetInterval(STRANGE_NETSCAPE_BUG*1000);
#endif

  return chunked;
}


bool PHTTPServer::SendResponse(StatusCode code, const PString & body)
{
  PMIMEInfo headers;
  return SendResponse(code, headers, body);
}


bool PHTTPServer::SendResponse(StatusCode code, PMIMEInfo & headers, const PString & body)
{
  StartResponse(code, headers, body.GetLength());
  return WriteString(body);
}


void PHTTPServer::SetDefaultMIMEInfo(PMIMEInfo & info, const PHTTPConnectionInfo & connectInfo) const
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

  headers.SetAt(ContentTypeTag(), PMIMEInfo::TextHTML());
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


PHTTPListener::~PHTTPListener()
{
  ShutdownListeners();
}


bool PHTTPListener::ListenForHTTP(WORD port, PSocket::Reusability reuse, unsigned queueSize)
{
  return ListenForHTTP(PString::Empty(), port, reuse, queueSize);
}


bool PHTTPListener::ListenForHTTP(const PString & interfaces, WORD port, PSocket::Reusability reuse, unsigned queueSize)
{
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
      PTCPSocket * listener = new PTCPSocket(m_listenerPort);
      if (listener->Listen(binding, queueSize, 0, reuse)) {
        m_listenerPort = listener->GetPort();
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

  m_httpServersMutex.Wait();
  for (PList<PHTTPServer>::iterator it = m_httpServers.begin(); it != m_httpServers.end(); ++it)
    it->Close();
  m_httpServersMutex.Signal();

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


PHTTPListener::Worker::Worker(PHTTPListener & listener, PTCPSocket * socket)
  : m_listener(listener)
  , m_socket(socket)
  , m_httpServer(NULL)
{
}


PHTTPListener::Worker::~Worker()
{
  if (m_httpServer != NULL) {
    m_listener.m_httpServersMutex.Wait();
    m_listener.m_httpServers.Remove(m_httpServer); // And deletes it
    m_listener.m_httpServersMutex.Signal();
  }

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
  PTRACE(5, "Processing thread pool work for" << socketInfo << ", delay=" << m_queuedTime.GetElapsed());

  m_httpServer = m_listener.CreateServerForHTTP();
  if (m_httpServer == NULL) {
    PTRACE(2, "Creation failed" << socketInfo);
    return;
  }
  m_httpServer->SetServiceStartTime(m_queuedTime);
  m_listener.m_httpServers.Append(m_httpServer); // Deleted in this list

  PChannel * channel = m_listener.CreateChannelForHTTP(m_socket);
  if (channel == NULL) {
    PTRACE(2, "Indirect channel creation failed" << socketInfo);
    return;
  }
  m_socket = NULL; // Will be deleted via PIndirectChannel now

  if (!m_httpServer->Open(channel)) {
    PTRACE(2, "Open failed" << socketInfo);
    return;
  }

  PTRACE(5, "Started" << socketInfo);
  m_listener.OnHTTPStarted(*m_httpServer);

  // process requests
  while (m_httpServer->ProcessCommand()) {
    PTRACE(5, "Processed" << socketInfo << ", duration=" << m_httpServer->GetLastCommandTime().GetElapsed());
  }

  m_listener.OnHTTPEnded(*m_httpServer);
  PTRACE(5, "Ended" << socketInfo << ", duration=" << m_queuedTime.GetElapsed());
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

PHTTPSimpleAuth::PHTTPSimpleAuth(const PString & realm,
                                 const PString & username,
                                 const PString & password)
  : m_realm(realm)
  , m_username(username)
  , m_password(password)
{
  PAssert(!realm.IsEmpty(), "Must have a realm!");
}


PObject * PHTTPSimpleAuth::Clone() const
{
  return new PHTTPSimpleAuth(m_realm, m_username, m_password);
}


PBoolean PHTTPSimpleAuth::IsActive() const
{
  return !m_username.IsEmpty() || !m_password.IsEmpty();
}


PString PHTTPSimpleAuth::GetRealm(const PHTTPRequest &) const
{
  return m_realm;
}


PBoolean PHTTPSimpleAuth::Validate(const PHTTPRequest &,
                               const PString & authInfo) const
{
  PString user, pass;
  DecodeBasicAuthority(authInfo, user, pass);
  return m_username == user && m_password == pass;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPMultiSimpAuth

PHTTPMultiSimpAuth::PHTTPMultiSimpAuth(const PString & realm_)
  : m_realm(realm_)
{
  PAssert(!m_realm.IsEmpty(), "Must have a realm!");
}


PHTTPMultiSimpAuth::PHTTPMultiSimpAuth(const PString & realm, const PStringToString & users)
  : m_realm(realm)
  , m_users(users)
{
  PAssert(!realm.IsEmpty(), "Must have a realm!");
}


PObject * PHTTPMultiSimpAuth::Clone() const
{
  return new PHTTPMultiSimpAuth(m_realm, m_users);
}


PBoolean PHTTPMultiSimpAuth::IsActive() const
{
  return !m_users.IsEmpty();
}


PString PHTTPMultiSimpAuth::GetRealm(const PHTTPRequest &) const
{
  return m_realm;
}


PBoolean PHTTPMultiSimpAuth::Validate(const PHTTPRequest &,
                                  const PString & authInfo) const
{
  PString user, pass;
  DecodeBasicAuthority(authInfo, user, pass);
  return m_users.Contains(user) && m_users[user] == pass;
}


void PHTTPMultiSimpAuth::AddUser(const PString & username, const PString & password)
{
  m_users.SetAt(username, password);
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
        SetLastReadCount(0);
        return false;

      default:
        break;
    }
  }

  if (len > (PINDEX)m_remainingPayload)
    len = (PINDEX)m_remainingPayload;

  if (!ReadBlock(buf, len))
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
        m_currentMask = uint32_t(m_currentMask << 24) | (m_currentMask >> 8);
        break;
      case 2:
        *(uint16_t *)ptr ^= m_currentMask;
        m_currentMask = uint32_t(m_currentMask << 16) | (m_currentMask >> 16);
        break;
      case 3:
        *(uint16_t *)ptr ^= m_currentMask;
        *(BYTE *)(ptr+2) ^= m_currentMask>>16;
        m_currentMask = uint32_t(m_currentMask << 8) | (m_currentMask >> 24);
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
  if (!PAssert(m_remainingPayload == 0, "Cannot call ReadMessage when have partial frames unread."))
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


bool PWebSocket::ReadText(PString & msg)
{
  PBYTEArray data;
  if (!ReadMessage(data))
    return false;

  msg = PString(data);
  return true;
}


bool PWebSocket::IsMessageComplete() const
{
  return !m_fragmentedRead && m_remainingPayload == 0;
}


PBoolean PWebSocket::Write(const void * buf, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  // Make sure WriteHeader and WriteMasked/Write of body are atomic
  PWaitAndSignal lock(m_writeMutex);

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


void PWebSocket::SetSSLCredentials(const PString & authority, const PString & certificate, const PString & privateKey)
{
  m_authority = authority;
  m_certificate = certificate;
  m_privateKey = privateKey;
}


bool PWebSocket::Connect(const PURL & url, const PStringArray & protocols, PString * selectedProtocol)
{
  channelPointerMutex.StartWrite();

  // See if we already have a HTTP client in the chain
  PHTTPClient * http = FindChannel<PHTTPClient>();
  if (http == NULL) {
    http = new PHTTPClient;
    http->SetReadChannel(Detach(ShutdownRead));
    http->SetWriteChannel(Detach(ShutdownWrite));
    http->SetReadTimeout(GetReadTimeout()); // Set timeouts, as Open() copies form subchannel
    http->SetWriteTimeout(GetWriteTimeout());
    Open(http);
  }

  channelPointerMutex.EndWrite();

  http->SetSSLCredentials(m_authority, m_certificate, m_privateKey);

  // Before starting up, make sure underlying socket is closed, so reconnects
  PChannel * base = http->GetBaseReadChannel();
  if (base != NULL)
    base->Close();

  PString key = PBase64::Encode(PRandom::Octets(16));
  PMIMEInfo outMIME, replyMIME;
  outMIME.SetAt(PHTTP::ConnectionTag(), PHTTP::UpgradeTag());
  outMIME.SetAt(PHTTP::UpgradeTag(), PHTTP::WebSocketTag());
  outMIME.SetAt(PHTTP::WebSocketVersionTag(), "13");
  outMIME.SetAt(PHTTP::WebSocketProtocolTag(), PSTRSTRM(std::setfill(',') << protocols));
  outMIME.SetAt(PHTTP::WebSocketKeyTag(), key);

  int result = http->ExecuteCommand(PHTTP::GET, url, outMIME, PString::Empty(), replyMIME);
  if (result < 100 || result >= 300) {
    PTRACE(2, "WebSocket reply error: " << result << " - " << http->GetLastResponseInfo());
    SetErrorValues(ProtocolFailure, EPROTO);
    return false;
  }

  PMessageDigestSHA1::Result theirHash, ourHash;
  PBase64::Decode(replyMIME(PHTTP::WebSocketAcceptTag()), theirHash);
  PMessageDigestSHA1::Encode(key + WebSocketGUID, ourHash);

  if (!ourHash.ConstantTimeCompare(theirHash)) {
    PTRACE(2, "WebSocket reply Accept header is unacceptable: ours=" << ourHash << ", theirs=" << theirHash);
    SetErrorValues(ProtocolFailure, EPROTO);
    return false;
  }

  PString protocol = outMIME(PHTTP::WebSocketProtocolTag());
  if (protocols.GetValuesIndex(protocol) == P_MAX_INDEX) {
    PTRACE(2, "WebSocket selected a protocol we did not offer.");
    SetErrorValues(ProtocolFailure, EPROTO);
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
  SetReadTimeout(1000);
  bool ok = false;

  BYTE header2;
  if (!Read(&header2, 1))
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
  PUInt64b * pLen = (PUInt64b *)&header[2];
  PINDEX len = 2;

  header[0] = (BYTE)opCode;
  if (!fragment)
    header[0] |= 0x80;

  if (payloadLength < 126)
    header[1] = (BYTE)payloadLength;
  else if (payloadLength < 65536) {
    header[1] = 126;
    *(PUInt16b *)pLen = (uint16_t)payloadLength;
    len += 2;
  }
  else {
    header[1] = 127;
    *pLen = payloadLength;
    len += 8;
  }

  if (masking >= 0) {
    header[1] |= 0x80;
    *(uint32_t *)&header[len] = (uint32_t)masking;
    len += 4;
  }

  return PIndirectChannel::Write(header, len);
}

#endif //P_SSL


//////////////////////////////////////////////////////////////////////////////
// PHTTPConnectionInfo

PHTTPConnectionInfo::PHTTPConnectionInfo()
  : persistenceSeconds(DEFAULT_PERSIST_TIMEOUT) // maximum lifetime (in seconds) of persistent connections
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
  if (!str.IsEmpty()) {
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
  : m_baseURL(url)
  , m_authority(NULL)
  , m_hitCount(0)
{
}


PHTTPResource::PHTTPResource(const PURL & url, const PHTTPAuthority & auth)
  : m_baseURL(url)
  , m_authority(auth.CloneAs<PHTTPAuthority>())
  , m_hitCount(0)
{
}


PHTTPResource::PHTTPResource(const PURL & url, const PString & type)
  : m_baseURL(url)
  , m_contentType(type)
  , m_authority(NULL)
  , m_hitCount(0)
{
}


PHTTPResource::PHTTPResource(const PURL & url,
                             const PString & type,
                             const PHTTPAuthority & auth)
  : m_baseURL(url)
  , m_contentType(type)
  , m_authority(auth.CloneAs<PHTTPAuthority>())
  , m_hitCount(0)
{
}


PHTTPResource::~PHTTPResource()
{
  delete m_authority;
}


bool PHTTPResource::SupportsWebSocketProtocol(const PString &) const
{
  return false;
}


bool PHTTPResource::OnWebSocket(PHTTPServer &, PHTTPConnectionInfo &)
{
  return false;
}


bool PHTTPResource::OnGET(PHTTPServer & server, const PHTTPConnectionInfo & connectInfo)
{
  return InternalOnCommand(server, connectInfo, PHTTP::GET);
}


bool PHTTPResource::OnHEAD(PHTTPServer & server, const PHTTPConnectionInfo & connectInfo)
{
  return InternalOnCommand(server, connectInfo, PHTTP::HEAD);
}


bool PHTTPResource::InternalOnCommand(PHTTPServer & server,
                        const PHTTPConnectionInfo & connectInfo,
                                   PHTTP::Commands cmd)
{
  // Nede to split songle if into 2 so the Tornado compiler won't end with
  // 'internal compiler error'
  if (cmd == PHTTP::GET && connectInfo.GetMIME().Contains(PHTTP::IfModifiedSinceTag()))
    if (!IsModifiedSince(PTime(connectInfo.GetMIME()[PHTTP::IfModifiedSinceTag()])))
      return server.OnError(PHTTP::NotModified, connectInfo.GetURL().AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(connectInfo.GetURL(),
                                         connectInfo.GetMIME(),
                                         connectInfo.GetMultipartFormInfo(),
                                         server);
  request->entityBody = connectInfo.GetEntityBody();

  bool retVal = true;
  if (CheckAuthority(server, *request, connectInfo)) {
    retVal = false;
    server.SetDefaultMIMEInfo(request->outMIME, connectInfo);

    PTime expiryDate(0);
    if (GetExpirationDate(expiryDate))
      request->outMIME.Set(PHTTP::ExpiresTag, expiryDate.AsString(PTime::RFC1123, PTime::GMT));

    if (cmd == PHTTP::POST) {
      PStringToString postData;
      if (PHTTP::FormUrlEncoded() == connectInfo.GetMIME().Get(PHTTP::ContentTypeTag(), PHTTP::FormUrlEncoded()))
        PURL::SplitQueryVars(connectInfo.GetEntityBody(), postData);
      if (!OnPOSTData(*request, postData)) {
        if (request->code != PHTTP::RequestOK)
          server.OnError(request->code, "", connectInfo);
        retVal = false;
      }
    }
    if (!LoadHeaders(*request)) 
      retVal = server.OnError(request->code, connectInfo.GetURL().AsString(), connectInfo);
    else if (cmd != PHTTP::GET)
      retVal = request->outMIME.Contains(PHTTP::ContentLengthTag());
    else {
      m_hitCount++;
      retVal = OnGETData(server, connectInfo.GetURL(), connectInfo, *request);
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
  return InternalOnCommand(server, connectInfo, PHTTP::POST);
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

  request.outMIME.SetAt(PHTTP::ContentTypeTag(), PMIMEInfo::TextHTML());

  PINDEX len = msg.GetLength();
  request.server.StartResponse(request.code, request.outMIME, len);
  return request.server.Write((const char *)msg, len) && persist;
}


PBoolean PHTTPResource::CheckAuthority(PHTTPServer & server,
                            const PHTTPRequest & request,
                     const PHTTPConnectionInfo & connectInfo)
{
  if (m_authority == NULL)
    return true;

  return CheckAuthority(*m_authority, server, request, connectInfo);
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
  headers.SetAt(PHTTP::ContentTypeTag(), PMIMEInfo::TextHTML());

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
  delete m_authority;
  m_authority = (PHTTPAuthority *)auth.Clone();
}


void PHTTPResource::ClearAuthority()
{
  delete m_authority;
  m_authority = NULL;
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
  if (!request.outMIME.Contains(PHTTP::ContentTypeTag) && !m_contentType.IsEmpty())
    request.outMIME.SetAt(PHTTP::ContentTypeTag, m_contentType);

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
  : PHTTPResource(url, PMIMEInfo::TextHTML())
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, PMIMEInfo::TextHTML(), auth)
{
}


PHTTPString::PHTTPString(const PURL & url, const PString & str)
  : PHTTPResource(url, PMIMEInfo::TextHTML())
  , m_string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PString & type)
  : PHTTPResource(url, type)
  , m_string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, PMIMEInfo::TextHTML(), auth)
  , m_string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PString & type,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth)
  , m_string(str)
{
}


PBoolean PHTTPString::LoadHeaders(PHTTPRequest & request)
{
  request.contentSize = m_string.GetLength();
  return true;
}


PString PHTTPString::LoadText(PHTTPRequest &)
{
  return m_string;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

PHTTPFile::PHTTPFile(const PURL & url, int)
  : PHTTPResource(url)
{
}


PHTTPFile::PHTTPFile(const PString & filename)
  : PHTTPResource(filename, PMIMEInfo::GetContentType(PFilePath(filename).GetType()))
  , m_filePath(filename)
{
}


PHTTPFile::PHTTPFile(const PString & filename, const PHTTPAuthority & auth)
  : PHTTPResource(filename, auth)
  , m_filePath(filename)
{
}


PHTTPFile::PHTTPFile(const PURL & url, const PFilePath & path)
  : PHTTPResource(url, PMIMEInfo::GetContentType(path.GetType()))
  , m_filePath(path)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PString & type)
  : PHTTPResource(url, type)
  , m_filePath(path)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, PMIMEInfo::GetContentType(path.GetType()), auth)
  , m_filePath(path)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PString & type,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth)
  , m_filePath(path)
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
  PFile & file = ((PHTTPFileRequest&)request).m_file;

  if (!file.Open(m_filePath, PFile::ReadOnly)) {
    PTRACE(3, "Could not open \"" << m_filePath << "\" for URL " << request.url);
    request.code = PHTTP::NotFound;
    return false;
  }

  request.contentSize = file.GetLength();
  return true;
}


PBoolean PHTTPFile::LoadData(PHTTPRequest & request, PCharArray & data)
{
  PFile & file = ((PHTTPFileRequest&)request).m_file;

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
  PFile & file = ((PHTTPFileRequest&)request).m_file;
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
  PFile & file = ((PHTTPFileRequest&)request).m_file;

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
  : PHTTPFile(url, 0)
  , m_basePath(dir)
  , m_allowDirectoryListing(true)
{
}


PHTTPDirectory::PHTTPDirectory(const PURL & url,
                               const PDirectory & dir,
                               const PHTTPAuthority & auth)
  : PHTTPFile(url, PString(), auth)
  , m_basePath(dir)
  , m_allowDirectoryListing(true)
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
  request->m_realPath = m_basePath;
  PINDEX i;
  for (i = GetURL().GetPath().GetSize(); i < path.GetSize()-1; i++)
    request->m_realPath += path[i] + PDIR_SEPARATOR;

  // append the last path element
  if (i < path.GetSize())
    request->m_realPath += path[i];

  if (request->m_realPath.Find(m_basePath) != 0)
    request->m_realPath = m_basePath;

  return request;
}


void PHTTPDirectory::EnableAuthorisation(const PString & realm)
{
  m_authorisationRealm = realm;
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
    
  if (dir.IsRoot() || (dir == m_basePath))
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
  if (m_authorisationRealm.IsEmpty() ||
      !FindAuthorisations(((PHTTPDirRequest&)request).m_realPath.GetDirectory(), newRealm, authorisations) ||
      authorisations.GetSize() == 0)
    return true;

  PHTTPMultiSimpAuth authority(newRealm, authorisations);
  return PHTTPResource::CheckAuthority(authority, server, request, conInfo);
}

PBoolean PHTTPDirectory::LoadHeaders(PHTTPRequest & request)
{
  PFilePath & realPath = dynamic_cast<PHTTPDirRequest&>(request).m_realPath;
    
  // if not able to obtain resource information, then consider the resource "not found"
  PFileInfo info;
  if (!PFile::GetInfo(realPath, info)) {
    PTRACE(4, "Directory \"" << realPath << "\" does not exist for URL " << request.url);
    request.code = PHTTP::NotFound;
    return false;
  }

  // if the resource is a file, and the file can't be opened, then return "not found"
  PFile & file = ((PHTTPDirRequest&)request).m_file;
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(realPath, PFile::ReadOnly) ||
        (!m_authorisationRealm.IsEmpty() && realPath.GetFileName() == accessFilename)) {
      PTRACE(4, "No permission to access \"" << realPath << "\" for URL " << request.url);
      request.code = PHTTP::NotFound;
      return false;
    }
  } 

  // resource is a directory - if index files disabled, then return "not found"
  else if (!m_allowDirectoryListing) {
    PTRACE(4, "No directory listing allowed for \"" << realPath << "\" for URL " << request.url);
    request.code = PHTTP::NotFound;
    return false;
  }

  // else look for index files
  else {
    PINDEX i;
    for (i = 0; i < PARRAYSIZE(HTMLIndexFiles); i++)
      if (file.Open(realPath + PDIR_SEPARATOR + HTMLIndexFiles[i], PFile::ReadOnly))
        break;
  }

  // open the file and return information
  PString & fakeIndex = ((PHTTPDirRequest&)request).m_fakeIndex;
  if (file.IsOpen()) {
    PTRACE(4, "Delivering file \"" << file.GetFilePath() << "\" for URL " << request.url);
    request.outMIME.SetAt(PHTTP::ContentTypeTag(),
                          PMIMEInfo::GetContentType(file.GetFilePath().GetType()));
    request.contentSize = file.GetLength();
    fakeIndex = PString();
    return true;
  }

  // construct a directory listing
  request.outMIME.SetAt(PHTTP::ContentTypeTag(), PMIMEInfo::TextHTML());
  PHTML reply("Directory of " + request.url.AsString());

  PDirectory::Entries listing;
  PCaselessString sort = request.url.GetQueryVars().GetString("sort", "name");
  bool reversed = request.url.GetQueryVars().GetBoolean("reversed");

  PDirectory(realPath).GetEntries(listing, sort);
  if (reversed)
    std::reverse(listing.begin(), listing.end());

  reply << PHTML::TableStart(PHTML::Border1, PHTML::CellPad4)
          << PHTML::TableRow();

  static struct {
    const char * m_name;
    const char * m_title;
  } const Columns[] = {
    { "name",        "File Name" },
    { "type",        "Type" },
    { "size",        "Size" },
    { "modified",    "Modified" },
    { "permissions", "Permissions" }
  };
  for (PINDEX i = 0; i < PARRAYSIZE(Columns); ++i) {
    PStringStream url;
    url << m_baseURL << "?sort=" << Columns[i].m_name;
    if (sort == Columns[i].m_name)
      url << "&reversed=" << boolalpha << !reversed;
    reply << PHTML::TableHeader() << PHTML::HotLink(url) << Columns[i].m_title << PHTML::HotLink();
  }

  for (PDirectory::Entries::iterator it = listing.begin(); it != listing.end(); ++it) {
    PURL entryURL = request.url;
    entryURL.AppendPath(it->m_name);
    reply << PHTML::TableRow()
          << PHTML::TableData()
          << PHTML::HotLink(entryURL.AsString()) << it->m_name << PHTML::HotLink()
          << PHTML::TableData(PHTML::AlignCentre);
    if (it->type&PFileInfo::SymbolicLink)
      reply << "Link";
    else if (it->type&PFileInfo::RegularFile)
      reply << "File";
    else if (it->type&(PFileInfo::SubDirectory|PFileInfo::ParentDirectory|PFileInfo::CurrentDirectory))
      reply << "Directory";
    else if (it->type&(PFileInfo::CharDevice|PFileInfo::BlockDevice|PFileInfo::SocketDevice|PFileInfo::Fifo))
      reply << "Device";
    else
      reply << "Unknown";
    reply << PHTML::TableData(PHTML::AlignRight)
          << PString(::PString::ScaleSI, it->size) << 'B'
          << PHTML::TableData()
          << it->modified.AsString("hh:mm:ss dd-MMM-yyyy")
          << PHTML::TableData(PHTML::AlignCentre)
          << std::oct << '0' << it->permissions << std::dec;
  }
  reply << PHTML::TableEnd()
        << PHTML::Body();
  fakeIndex = reply;

  return true;
}


PString PHTTPDirectory::LoadText(PHTTPRequest & request)
{
  PString & fakeIndex = ((PHTTPDirRequest&)request).m_fakeIndex;
  if (fakeIndex.IsEmpty())
    return PHTTPFile::LoadText(request);

  return fakeIndex;
}

#endif // P_HTTP


// End Of File ///////////////////////////////////////////////////////////////
