/*
 * $Id: httpsrvr.cxx,v 1.4 1996/12/12 09:24:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: httpsrvr.cxx,v $
 * Revision 1.4  1996/12/12 09:24:16  robertj
 * Persistent proxy connection support (work in progress).
 *
 * Revision 1.3  1996/11/10 21:09:33  robertj
 * Removed redundent GetSocket() call.
 * Added flush of stream after processing request, important on persistent connections.
 *
 * Revision 1.2  1996/10/26 03:31:05  robertj
 * Changed OnError so can pass in full HTML page as parameter.
 *
 * Revision 1.1  1996/09/14 13:02:18  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>
#include <http.h>
#include <ctype.h>


// undefine to remove support for persistant connections
#define HAS_PERSISTANCE

// define to enable work-around for Netscape persistant connection bug
// set to lifetime of suspect sockets (in seconds)
#define STRANGE_NETSCAPE_BUG	3

// maximum lifetime (in seconds) of persistant connections
#define	MAX_LIFETIME		30

// maximum lifetime (in transactions) of persistant connections
#define MAX_TRANSACTIONS	10

// maximum delay between characters whilst reading a line of text
#define	READLINE_TIMEOUT	30


static const PCaselessString ContentLengthStr   = "Content-Length";
static const PCaselessString ContentTypeStr     = "Content-Type";
static const PCaselessString DateStr            = "Date";
static const PCaselessString MIMEVersionStr     = "MIME-Version";
static const PCaselessString ServerStr          = "Server";
static const PCaselessString ExpiresStr         = "Expires";
static const PCaselessString WWWAuthenticateStr = "WWW-Authenticate";
static const PCaselessString IfModifiedSinceStr = "If-Modified-Since";
static const PCaselessString ConnectionStr      = "Connection";
static const PCaselessString KeepAliveStr       = "Keep-Alive";
static const PCaselessString ProxyConnectionStr = "Proxy-Connection";
static const PCaselessString UserAgentStr       = "User-Agent";


//////////////////////////////////////////////////////////////////////////////
// PHTTPSpace

PHTTPSpace::PHTTPSpace()
{
  resource = NULL;
}


PHTTPSpace::PHTTPSpace(const PString & nam, PHTTPSpace * parentNode)
  : name(nam)
{
  parent = parentNode;
  resource = NULL;
}


PHTTPSpace::~PHTTPSpace()
{
  delete resource;
}


PObject::Comparison PHTTPSpace::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PHTTPSpace::Class()), PInvalidCast);
  return name.Compare(((const PHTTPSpace &)obj).name);
}


BOOL PHTTPSpace::AddResource(PHTTPResource * res, AddOptions overwrite)
{
  PAssert(res != NULL, PInvalidParameter);
  const PStringArray & path = res->GetURL().GetPath();
  PHTTPSpace * node = this;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    if (path[i].IsEmpty())
      break;

    if (node->resource != NULL)
      return FALSE;   // Already a resource in tree in partial path

    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      pos = node->children.Append(PNEW PHTTPSpace(path[i], node));

    node = &node->children[pos];
  }

  if (!node->children.IsEmpty())
    return FALSE;   // Already a resource in tree further down path.

  if (overwrite == ErrorOnExist && node->resource != NULL)
    return FALSE;   // Already a resource in tree at leaf

  delete node->resource;
  node->resource = res;

  return TRUE;
}


BOOL PHTTPSpace::DelResource(const PURL & url)
{
  const PStringArray & path = url.GetPath();
  PHTTPSpace * node = this;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    if (path[i].IsEmpty())
      break;

    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      return FALSE;

    node = &node->children[pos];

    if (node->resource != NULL)
      return FALSE;
  }

  if (!node->children.IsEmpty())
    return FALSE;   // Still a resource in tree further down path.

  do {
    PHTTPSpace * par = node->parent;
    par->children.Remove(node);
    node = par;
  } while (node != NULL && node->children.IsEmpty());

  return TRUE;
}


static const char * const HTMLIndexFiles[] = {
  "Welcome.html", "welcome.html", "index.html",
  "Welcome.htm",  "welcome.htm",  "index.htm"
};

PHTTPResource * PHTTPSpace::FindResource(const PURL & url)
{
  const PStringArray & path = url.GetPath();

  PHTTPSpace * node = this;
  PINDEX i;
  for (i = 0; i < path.GetSize(); i++) {
    if (path[i].IsEmpty())
      break;

    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      return NULL;

    node = &node->children[pos];

    if (node->resource != NULL)
      return node->resource;
  }

  for (i = 0; i < PARRAYSIZE(HTMLIndexFiles); i++) {
    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(HTMLIndexFiles[i]));
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
  SetReadLineTimeout(PTimeInterval(0, READLINE_TIMEOUT));
}


BOOL PHTTPServer::ProcessCommand()
{
  PString args;
  PINDEX cmd;

  // if this is not the first command received by this socket, then set
  // the read timeout appropriately.
  if (transactionCount > 0) 
    SetReadTimeout(nextTimeout);

  // this will only return false upon timeout or completely invalid command
  if (!ReadCommand(cmd, args))
    return FALSE;

  PStringArray tokens = args.Tokenise(" \t", FALSE);

  // if no tokens, error
  if (tokens.IsEmpty()) {
    OnError(BadRequest, args, PHTTPConnectionInfo());
    return FALSE;
  }

  // if only one argument, then it must be a version 0.9 simple request
  if (tokens.GetSize() == 1) {
    majorVersion = 0;
    minorVersion = 9;
  }
  else { // otherwise, attempt to extract a version number
    PString verStr = tokens[1];
    PINDEX dotPos = verStr.Find('.');
    if (dotPos == P_MAX_INDEX
                      || verStr.GetLength() < 8 || verStr.Left(5) != "HTTP/") {
      OnError(BadRequest, "Malformed version number " + verStr, PHTTPConnectionInfo());
      return FALSE;
    }

    // should actually check if the text contains only digits, but the
    // chances of matching everything else and it not being a valid number
    // are pretty small, so don't bother
    majorVersion = (int)verStr(5, dotPos-1).AsInteger();
    minorVersion = (int)verStr(dotPos+1, P_MAX_INDEX).AsInteger();
  }

  // now that we've decided we did receive a HTTP request, increment the
  // count of transactions
  transactionCount++;
  nextTimeout.SetInterval(MAX_LIFETIME*1000);

  // If the protocol is version 1.0 or greater, there is MIME info, and the
  // prescence of a an entity body is signalled by the inclusion of
  // Content-Length header. If the protocol is less than version 1.0, then 
  // there is no entity body!
  PHTTPConnectionInfo connectInfo;
  PMIMEInfo mimeInfo;
  long contentLength = 0;

  if (majorVersion > 0) {

    // at this stage we should be ready to collect the MIME info
    // until an empty line is received, or EOF
    mimeInfo.Read(*this);

    // build our connection info
    connectInfo.Construct(mimeInfo, majorVersion, minorVersion);
//    if (connectInfo.IsPersistant()) {
//      if (connectInfo.IsProxyConnection())
//        PError << "Server: Persistant proxy connection received" << endl;
//      else
//        PError << "Server: Persistant direct connection received" << endl;
//    }

    // if the client specified a persistant connection, then use the
    // ContentLength field. If there is no content length field, then
    // assume a ContentLength of zero and close the connection.
    // The spec actually says to read until end of file in this case,
    // but Netscape hangs if this is done.
    // If the client didn't specify a persistant connection, then use the
    // ContentLength if there is one or read until end of file if there isn't
    if (!connectInfo.IsPersistant())
      contentLength = mimeInfo.GetInteger(ContentLengthStr, 0);
    else {
      contentLength = mimeInfo.GetInteger(ContentLengthStr, -1);
      if (contentLength < 0) {
//        PError << "Server: persistant connection has no content length" << endl;
        contentLength = 0;
        mimeInfo.SetAt(ContentLengthStr, "0");
      }
    }
  }

  connectInfo.SetEntityBodyLength(contentLength);

  // get the user agent for various foul purposes...
  userAgent = mimeInfo(UserAgentStr);

  PIPSocket * socket = GetSocket();
  WORD myPort = (WORD)(socket != NULL ? socket->GetPort() : 80);

  // the URL that comes with Connect requests is not quite kosher, so 
  // mangle it into a proper URL and do NOT close the connection.
  // for all other commands, close the read connection if not persistant
  PURL url;
  if (cmd == CONNECT) 
    url = "https://" + tokens[0];
  else {
    url = tokens[0];
    if (url.GetPort() == 0)
      url.SetPort(myPort);
  }

  // If the incoming URL is of a proxy type then call OnProxy() which will
  // probably just go OnError(). Even if a full URL is provided in the
  // command we should check to see if it is a local server request and process
  // it anyway even though we are not a proxy. The usage of GetHostName()
  // below are to catch every way of specifying the host (name, alias, any of
  // several IP numbers etc).
  if (url.GetScheme() != "http" ||
      (url.GetPort() != 0 && url.GetPort() != myPort) ||
      !PIPSocket::IsLocalHost(url.GetHostName()))
    return OnProxy((Commands)cmd, url, mimeInfo, connectInfo) && connectInfo.IsPersistant();

  PString entityBody = ReadEntityBody(connectInfo);

  // Handle the local request
  PStringToString postData;
  BOOL persist = TRUE;
  switch (cmd) {
    case GET :
      persist = OnGET(url, mimeInfo, connectInfo);
      break;

    case HEAD :
      persist = OnHEAD(url, mimeInfo, connectInfo);
      break;

    case POST :
      PURL::SplitQueryVars(entityBody, postData);
      persist = OnPOST(url, mimeInfo, postData, connectInfo);
      break;

    case P_MAX_INDEX:
    default:
      persist = OnUnknown(args, connectInfo);
  }

  flush();

  // if the function just indicated that the connection is to persist,
  // and so did the client, then return TRUE. Note that all of the OnXXXX
  // routines above must make sure that their return value is FALSE if
  // if there was no ContentLength field in the response. This ensures that
  // we always close the socket so the client will get the correct end of file
  if (persist &&
      connectInfo.IsPersistant() &&
      transactionCount < MAX_TRANSACTIONS)
    return TRUE;

//  if (connectInfo.IsPersistant())
//    PError << "Server: connection persistance end" << endl;

  // close the output stream now and return FALSE
  Shutdown(ShutdownWrite);
  return FALSE;
}


PString PHTTPServer::ReadEntityBody(const PHTTPConnectionInfo & connectInfo)
{
  if (connectInfo.GetMajorVersion() < 1)
    return PString();

  PString entityBody;
  long contentLength = connectInfo.GetEntityBodyLength();
  // a content length of > 0 means read explicit length
  // a content length of < 0 means read until EOF
  // a content length of 0 means read nothing
  int count = 0;
  if (contentLength > 0) {
    entityBody = ReadString((PINDEX)contentLength);
  } else if (contentLength < 0) {
    while (Read(entityBody.GetPointer(count+1000)+count, 1000))
      count += GetLastReadCount();
    entityBody.SetSize(count+1);
  }

  // close the connection, if not persistant
  if (!connectInfo.IsPersistant()) {
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


BOOL PHTTPServer::OnGET(const PURL & url,
                   const PMIMEInfo & info,
         const PHTTPConnectionInfo & connectInfo)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL) 
    return OnError(NotFound, url.AsString(), connectInfo);
  else 
    return resource->OnGET(*this, url, info, connectInfo);
}


BOOL PHTTPServer::OnHEAD(const PURL & url,
                    const PMIMEInfo & info,
          const PHTTPConnectionInfo & connectInfo)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL) 
    return OnError(NotFound, url.AsString(), connectInfo);
  else
    return resource->OnHEAD(*this, url, info, connectInfo);
}


BOOL PHTTPServer::OnPOST(const PURL & url,
                    const PMIMEInfo & info,
              const PStringToString & data,
          const PHTTPConnectionInfo & connectInfo)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL) 
    return OnError(NotFound, url.AsString(), connectInfo);
  else
    return resource->OnPOST(*this, url, info, data, connectInfo);
}


BOOL PHTTPServer::OnProxy(Commands cmd,
                          const PURL &,
                          const PMIMEInfo &,
                          const PHTTPConnectionInfo & connectInfo)
{
  return OnError(BadGateway, "Proxy not implemented.", connectInfo) && cmd != CONNECT;
}


struct httpStatusCodeStruct {
  const char * text;
  int  code;
  BOOL allowedBody;
  int  majorVersion;
  int  minorVersion;
};

static const httpStatusCodeStruct * GetStatusCodeStruct(int code)
{
  static const httpStatusCodeStruct httpStatusDefn[] = {
    // First entry MUST be InternalServerError
    { "Internal Server Error",         PHTTP::InternalServerError, 1 },
    { "OK",                            PHTTP::OK, 1 },
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


void PHTTPServer::StartResponse(StatusCode code,
                                PMIMEInfo & headers,
                                long bodySize)
{
  if (majorVersion < 1) 
    return;

  const httpStatusCodeStruct * statusInfo = GetStatusCodeStruct(code);

  // output the command line
  const char * crlf = ""; //(transactionCount > 1) ? "\r\n" : "";
  WriteString(psprintf("%sHTTP/%u.%u %03u %s\r\n",
              crlf, majorVersion, minorVersion, statusInfo->code, statusInfo->text));

  // output the headers
  if (bodySize >= 0 && !headers.Contains(ContentLengthStr))
    headers.SetAt(ContentLengthStr, PString(PString::Unsigned, (PINDEX)bodySize));
  headers.Write(*this);

#ifdef STRANGE_NETSCAPE_BUG
  // The following is a work around for a strange bug in Netscape where it
  // locks up when a persistent connection is made and data less than 1k
  // (including MIME headers) is sent. Go figure....
  if (bodySize < 1024 && userAgent.Find("Mozilla/2.0") != P_MAX_INDEX)
    nextTimeout.SetInterval(STRANGE_NETSCAPE_BUG*1000);
#endif
}


void PHTTPServer::SetDefaultMIMEInfo(PMIMEInfo & info,
                     const PHTTPConnectionInfo & connectInfo)
{
  PTime now;
  info.SetAt(DateStr, now.AsString(PTime::RFC1123, PTime::GMT));
  info.SetAt(MIMEVersionStr, "1.0");
  info.SetAt(ServerStr, GetServerName());

  if (connectInfo.IsPersistant()) {
    if (connectInfo.IsProxyConnection())
//{      PError << "Server: setting proxy persistant response" << endl;
      info.SetAt(ProxyConnectionStr, KeepAliveStr);
//    }
    else
//{      PError << "Server: setting direct persistant response" << endl;
      info.SetAt(ConnectionStr, KeepAliveStr);
//    }
  }
}



BOOL PHTTPServer::OnUnknown(const PCaselessString & cmd, 
                        const PHTTPConnectionInfo & connectInfo)
{
  return OnError(NotImplemented, cmd, connectInfo);
}


BOOL PHTTPServer::OnError(StatusCode code,
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
    return statusInfo->code == OK;
  }

  PString reply;
  if (extra.Find("<BODY>") != P_MAX_INDEX)
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
  }

  headers.SetAt(ContentTypeStr, "text/html");
  StartResponse(code, headers, reply.GetLength());
  WriteString(reply);
  return statusInfo->code == OK;
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


BOOL PHTTPAuthority::IsActive() const
{
  return TRUE;
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
  return PNEW PHTTPSimpleAuth(realm, username, password);
}


BOOL PHTTPSimpleAuth::IsActive() const
{
  return !username || !password;
}


PString PHTTPSimpleAuth::GetRealm(const PHTTPRequest &) const
{
  return realm;
}


BOOL PHTTPSimpleAuth::Validate(const PHTTPRequest &,
                               const PString & authInfo) const
{
  PString user, pass;
  DecodeBasicAuthority(authInfo, user, pass);
  return username == user && password == pass;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPRequest

PHTTPRequest::PHTTPRequest(const PURL & u, const PMIMEInfo & iM)
  : url(u), inMIME(iM)
{
  code        = PHTTP::OK;
  contentSize = 0;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPConnectionInfo

PHTTPConnectionInfo::PHTTPConnectionInfo()
{
  majorVersion      = 0;
  minorVersion      = 9;

  isPersistant      = FALSE;
  isProxyConnection = FALSE;
}


void PHTTPConnectionInfo::Construct(const PMIMEInfo & mimeInfo,
                                    int major, int minor)
{
  majorVersion      = major;
  minorVersion      = minor;

  isPersistant      = FALSE;

#ifndef HAS_PERSISTANCE
  isProxyConnection = FALSE;
#else
  PString str;
  // check for Proxy-Connection and Connection strings
  isProxyConnection = mimeInfo.HasKey(ProxyConnectionStr);
  if (isProxyConnection)
    str = mimeInfo[ProxyConnectionStr];
  else if (mimeInfo.HasKey(ConnectionStr))
    str = mimeInfo[ConnectionStr];

  // get any connection options
  if (!str) {
    PStringArray tokens = str.Tokenise(", ", FALSE);
    for (int z = 0; !isPersistant && z < tokens.GetSize(); z++)
      isPersistant = isPersistant || (tokens[z] *= KeepAliveStr);
  }
#endif
}


void PHTTPConnectionInfo::SetPersistance(BOOL newPersist)
{
#ifdef HAS_PERSISTANCE
  isPersistant = newPersist;
#else
  isPersistant = FALSE;
#endif
}


BOOL PHTTPConnectionInfo::IsCompatible(int major, int minor) const
{
  if (minor == 0 && major == 0)
    return TRUE;
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


BOOL PHTTPResource::OnGET(PHTTPServer & server,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(server, url, info, connectInfo, TRUE);
}

BOOL PHTTPResource::OnHEAD(PHTTPServer & server,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(server, url, info, connectInfo, FALSE);
}

BOOL PHTTPResource::OnGETOrHEAD(PHTTPServer & server,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo,
                                   BOOL isGET)
{
  if (isGET && info.Contains(IfModifiedSinceStr) &&
                           !IsModifiedSince(PTime(info[IfModifiedSinceStr]))) 
    return server.OnError(PHTTP::NotModified, url.AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(url, info);

  BOOL retVal = TRUE;
  if (CheckAuthority(server, *request, connectInfo)) {
    retVal = FALSE;
    server.SetDefaultMIMEInfo(request->outMIME, connectInfo);

    PTime expiryDate;
    if (GetExpirationDate(expiryDate))
      request->outMIME.SetAt(ExpiresStr,
                              expiryDate.AsString(PTime::RFC1123, PTime::GMT));

    if (!LoadHeaders(*request)) 
      retVal = server.OnError(request->code, url.AsString(), connectInfo);
    else if (!isGET)
      retVal = request->outMIME.Contains(ContentLengthStr);
    else {
      hitCount++;
      retVal = OnGETData(server, url, connectInfo, *request);
    }
  }

  delete request;
  return retVal;
}

BOOL PHTTPResource::OnGETData(PHTTPServer & server,
                               const PURL & /*url*/,
                const PHTTPConnectionInfo & /*connectInfo*/,
                             PHTTPRequest & request)
{
  if (!request.outMIME.Contains(ContentTypeStr) && !contentType)
    request.outMIME.SetAt(ContentTypeStr, contentType);

  PCharArray data;
  if (LoadData(request, data)) {
    server.StartResponse(request.code, request.outMIME, request.contentSize);
    do {
      server.Write(data, data.GetSize());
      data.SetSize(0);
    } while (LoadData(request, data));
  }
  else
    server.StartResponse(request.code, request.outMIME, data.GetSize());

  server.Write(data, data.GetSize());

  return request.outMIME.Contains(ContentLengthStr);
}

BOOL PHTTPResource::OnPOST(PHTTPServer & server,
                            const PURL & url,
                       const PMIMEInfo & info,
                 const PStringToString & data,
             const PHTTPConnectionInfo & connectInfo)
{
  PHTTPRequest * request = CreateRequest(url, info);

  BOOL persist = TRUE;
  if (CheckAuthority(server, *request, connectInfo)) {
    PHTML msg;
    persist = Post(*request, data, msg);

    if (msg.IsEmpty())
      persist = server.OnError(request->code, "", connectInfo) && persist;
    else {
      if (msg.Is(PHTML::InBody))
        msg << PHTML::Body();

      request->outMIME.SetAt(ContentTypeStr, "text/html");

      PINDEX len = msg.GetLength();
      server.StartResponse(request->code, request->outMIME, len);
      persist = server.Write((const char *)msg, len) && persist;
    }
  }

  delete request;
  return persist;
}


BOOL PHTTPResource::CheckAuthority(PHTTPServer & server,
                            const PHTTPRequest & request,
                     const PHTTPConnectionInfo & connectInfo)
{
  if (authority == NULL || !authority->IsActive())
    return TRUE;

  // if this is an authorisation request...
  if (request.inMIME.Contains(PString("Authorization")) &&
      authority->Validate(request, request.inMIME["Authorization"]))
    return TRUE;

  // it must be a request for authorisation
  PMIMEInfo headers;
  server.SetDefaultMIMEInfo(headers, connectInfo);
  headers.SetAt(WWWAuthenticateStr,
                       "Basic realm=\"" + authority->GetRealm(request) + "\"");
  headers.SetAt(ContentTypeStr, "text/html");

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

  return FALSE;
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


BOOL PHTTPResource::IsModifiedSince(const PTime &)
{
  return TRUE;
}


BOOL PHTTPResource::GetExpirationDate(PTime &)
{
  return FALSE;
}


PHTTPRequest * PHTTPResource::CreateRequest(const PURL & url,
                                       const PMIMEInfo & inMIME)
{
  return PNEW PHTTPRequest(url, inMIME);
}


BOOL PHTTPResource::LoadData(PHTTPRequest & request, PCharArray & data)
{
  PString text = LoadText(request);
  OnLoadedText(request, text);
  text.SetSize(text.GetLength());  // Lose the trailing '\0'
  data = text;
  return FALSE;
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


BOOL PHTTPResource::Post(PHTTPRequest & request,
                         const PStringToString &,
                         PHTML & msg)
{
  request.code = PHTTP::MethodNotAllowed;
  msg = "Error in POST";
  msg << "Post to this resource is not allowed" << PHTML::Body();
  return TRUE;
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


BOOL PHTTPString::LoadHeaders(PHTTPRequest & request)
{
  request.contentSize = string.GetLength();
  return TRUE;
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
  : PHTTPResource(filename), filePath(filename)
{
  SetContentType(PMIMEInfo::GetContentType(filePath.GetType()));
}


PHTTPFile::PHTTPFile(const PString & filename, const PHTTPAuthority & auth)
  : PHTTPResource(filename, auth), filePath(filename)
{
}


PHTTPFile::PHTTPFile(const PURL & url, const PFilePath & path)
  : PHTTPResource(url), filePath(path)
{
  SetContentType(PMIMEInfo::GetContentType(path.GetType()));
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
  : PHTTPResource(url, PString(), auth), filePath(path)
{
  SetContentType(PMIMEInfo::GetContentType(filePath.GetType()));
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PString & type,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth), filePath(path)
{
}


PHTTPFileRequest::PHTTPFileRequest(const PURL & url,
                                   const PMIMEInfo & inMIME)
  : PHTTPRequest(url, inMIME)
{
}


PHTTPRequest * PHTTPFile::CreateRequest(const PURL & url,
                                   const PMIMEInfo & inMIME)
{
  return PNEW PHTTPFileRequest(url, inMIME);
}


BOOL PHTTPFile::LoadHeaders(PHTTPRequest & request)
{
  PFile & file = ((PHTTPFileRequest&)request).file;

  if (!file.Open(filePath, PFile::ReadOnly)) {
    request.code = PHTTP::NotFound;
    return FALSE;
  }

  request.contentSize = file.GetLength();
  return TRUE;
}


BOOL PHTTPFile::LoadData(PHTTPRequest & request, PCharArray & data)
{
  if (contentType(0, 4) == "text/")
    return PHTTPResource::LoadData(request, data);

  PFile & file = ((PHTTPFileRequest&)request).file;
  PAssert(file.IsOpen(), PLogicError);

  PINDEX count = file.GetLength() - file.GetPosition();
  if (count > 10000)
    count = 10000;

  PAssert(file.Read(data.GetPointer(count), count), PLogicError);

  if (!file.IsEndOfFile())
    return TRUE;

  file.Close();
  return FALSE;
}


PString PHTTPFile::LoadText(PHTTPRequest & request)
{
  PFile & file = ((PHTTPFileRequest&)request).file;
  PAssert(file.IsOpen(), PLogicError);
  PINDEX count = file.GetLength();
  PString text;
  PAssert(file.Read(text.GetPointer(count+1), count), PLogicError);
  PAssert(file.Close(), PLogicError);
  return text;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

PHTTPDirectory::PHTTPDirectory(const PURL & url, const PDirectory & dir)
  : PHTTPFile(url, 0), basePath(dir)
{
}


PHTTPDirectory::PHTTPDirectory(const PURL & url,
                               const PDirectory & dir,
                               const PHTTPAuthority & auth)
  : PHTTPFile(url, PString(), auth), basePath(dir)
{
}


PHTTPDirRequest::PHTTPDirRequest(const PURL & url,
                                   const PMIMEInfo & inMIME)
  : PHTTPFileRequest(url, inMIME)
{
}


PHTTPRequest * PHTTPDirectory::CreateRequest(const PURL & url,
                                        const PMIMEInfo & inMIME)
{
  return PNEW PHTTPDirRequest(url, inMIME);
}


BOOL PHTTPDirectory::LoadHeaders(PHTTPRequest & request)
{
  const PStringArray & path = request.url.GetPath();
  PFilePath realPath = basePath;
  PINDEX i;
  for (i = baseURL.GetPath().GetSize(); i < path.GetSize()-1; i++)
    realPath += path[i] + PDIR_SEPARATOR;

  if (i < path.GetSize())
    realPath += path[i];

  // See if its a normal file
  PFileInfo info;
  if (!PFile::GetInfo(realPath, info)) {
    request.code = PHTTP::NotFound;
    return FALSE;
  }

  // Now try and open it
  PFile & file = ((PHTTPDirRequest&)request).file;
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(realPath, PFile::ReadOnly)) {
      request.code = PHTTP::NotFound;
      return FALSE;
    }
  }
  else {
    for (i = 0; i < PARRAYSIZE(HTMLIndexFiles); i++)
      if (file.Open(realPath +
                          PDIR_SEPARATOR + HTMLIndexFiles[i], PFile::ReadOnly))
        break;
  }

  PString & fakeIndex = ((PHTTPDirRequest&)request).fakeIndex;
  if (file.IsOpen()) {
    contentType = PMIMEInfo::GetContentType(file.GetFilePath().GetType());
    request.contentSize = file.GetLength();
    fakeIndex = PString();
    return TRUE;
  }

  contentType = "text/html";
  PHTML reply = "Directory of " + request.url.AsString();
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

  return TRUE;
}


PString PHTTPDirectory::LoadText(PHTTPRequest & request)
{
  PString & fakeIndex = ((PHTTPDirRequest&)request).fakeIndex;
  if (fakeIndex.IsEmpty())
    return PHTTPFile::LoadText(request);

  return fakeIndex;
}


// End Of File ///////////////////////////////////////////////////////////////
