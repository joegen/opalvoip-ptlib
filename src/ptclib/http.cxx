/*
 * $Id: http.cxx,v 1.18 1996/03/31 09:05:07 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: http.cxx,v $
 * Revision 1.18  1996/03/31 09:05:07  robertj
 * HTTP 1.1 upgrade.
 *
 * Revision 1.17  1996/03/17 05:48:07  robertj
 * Fixed host name print out of URLs.
 * Added hit count to PHTTPResource.
 *
 * Revision 1.16  1996/03/16 05:00:26  robertj
 * Added ParseReponse() for splitting reponse line into code and info.
 * Added client side support for HTTP socket.
 * Added hooks for proxy support in HTTP socket.
 * Added translation type to TranslateString() to accommodate query variables.
 * Defaulted scheme field in URL to "http".
 * Inhibited output of port field on string conversion of URL according to scheme.
 *
 * Revision 1.15  1996/03/11 10:29:50  robertj
 * Fixed bug in help image HTML.
 *
 * Revision 1.14  1996/03/10 13:15:24  robertj
 * Redesign to make resources thread safe.
 *
 * Revision 1.13  1996/03/02 03:27:37  robertj
 * Added function to translate a string to a form suitable for inclusion in a URL.
 * Added radio button and selection boxes to HTTP form resource.
 * Fixed bug in URL parsing, losing first / if hostname specified.
 *
 * Revision 1.12  1996/02/25 11:14:24  robertj
 * Radio button support for forms.
 *
 * Revision 1.11  1996/02/25 03:10:34  robertj
 * Removed pass through HTTP resource.
 * Fixed PHTTPConfig resource to use correct name for config key.
 *
 * Revision 1.10  1996/02/19 13:48:28  robertj
 * Put multiple uses of literal strings into const variables.
 * Fixed URL parsing so that the unmangling of strings occurs correctly.
 * Moved nested classes from PHTTPForm.
 * Added overwrite option to AddResource().
 * Added get/set string to PHTTPString resource.
 *
 * Revision 1.9  1996/02/13 13:09:17  robertj
 * Added extra parameters to callback function in PHTTPResources, required
 *   by descendants to make informed decisions on data being loaded.
 *
 * Revision 1.8  1996/02/08 12:26:29  robertj
 * Redesign of resource callback mechanism.
 * Added new resource types for HTML data entry forms.
 *
 * Revision 1.7  1996/02/03 11:33:19  robertj
 * Changed RadCmd() so can distinguish between I/O error and unknown command.
 *
 * Revision 1.6  1996/02/03 11:11:49  robertj
 * Numerous bug fixes.
 * Added expiry date and ismodifiedsince support.
 *
 * Revision 1.5  1996/01/30 23:32:40  robertj
 * Added single .
 *
 * Revision 1.4  1996/01/28 14:19:09  robertj
 * Split HTML into separate source file.
 * Beginning of pass through resource type.
 * Changed PCharArray in OnLoadData to PString for convenience in mangling data.
 * Made PHTTPSpace return standard page on selection of partial path.
 *
 * Revision 1.3  1996/01/28 02:49:16  robertj
 * Further implementation.
 *
 * Revision 1.2  1996/01/26 02:24:30  robertj
 * Further implemetation.
 *
 * Revision 1.1  1996/01/23 13:04:32  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <html.h>
#include <http.h>
#include <ctype.h>

#define DEFAULT_FTP_PORT	21
#define DEFAULT_TELNET_PORT	23
#define DEFAULT_GOPHER_PORT	70
#define DEFAULT_HTTP_PORT	80
#define DEFAULT_NNTP_PORT	119
#define DEFAULT_WAIS_PORT	210
#define DEFAULT_HTTPS_PORT	443
#define DEFAULT_PROSPERO_PORT	1525


//////////////////////////////////////////////////////////////////////////////
// PURL

PURL::PURL()
{
  port = DEFAULT_HTTP_PORT;
}


PURL::PURL(const char * str)
{
  Parse(str);
}


PURL::PURL(const PString & str)
{
  Parse(str);
}


PObject::Comparison PURL::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PURL::Class()), PInvalidCast);
  const PURL & other = (const PURL &)obj;
  Comparison c = scheme.Compare(other.scheme);
  if (c == EqualTo) {
    c = username.Compare(other.username);
    if (c == EqualTo) {
      c = password.Compare(other.password);
      if (c == EqualTo) {
        c = hostname.Compare(other.hostname);
        if (c == EqualTo) {
          c = pathStr.Compare(other.pathStr);
          if (c == EqualTo) {
            c = parameters.Compare(other.parameters);
            if (c == EqualTo) {
              c = fragment.Compare(other.fragment);
              if (c == EqualTo)
                c = queryStr.Compare(other.queryStr);
            }
          }
        }
      }
    }
  }
  return c;
}


void PURL::PrintOn(ostream & stream) const
{
  stream << AsString(FullURL);
}


void PURL::ReadFrom(istream & stream)
{
  PString s;
  stream >> s;
  Parse(s);
}


PString PURL::TranslateString(const PString & str, TranslationType type)
{
  PString xlat = str;

  PString unsafeChars = ";/?:@=&#%+";
  if (type != QueryTranslation)
    unsafeChars += ' ';

  PINDEX pos = (PINDEX)-1;
  while ((pos = xlat.FindOneOf(unsafeChars, pos+1)) != P_MAX_INDEX)
    xlat.Splice(psprintf("%%%02X", xlat[pos]), pos, 1);

  if (type == QueryTranslation) {
    pos = (PINDEX)-1;
    while ((pos = xlat.Find(' ', pos+1)) != P_MAX_INDEX)
      xlat[pos] = '+';
  }

  return xlat;
}


static void UnmangleString(PString & str)
{
  PINDEX pos = (PINDEX)-1;
  while ((pos = str.Find('+', pos+1)) != P_MAX_INDEX)
    str[pos] = ' ';

  pos = (PINDEX)-1;
  while ((pos = str.Find('%', pos+1)) != P_MAX_INDEX) {
    int digit1 = str[pos+1];
    int digit2 = str[pos+2];
    if (isxdigit(digit1) && isxdigit(digit2)) {
      str[pos] = (char)(
            (isdigit(digit2) ? (digit2-'0') : (toupper(digit2)-'A'+10)) +
           ((isdigit(digit1) ? (digit1-'0') : (toupper(digit1)-'A'+10)) << 4));
      str.Delete(pos+1, 2);
    }
  }
}


static void SplitVars(const PString & queryStr, PStringToString & queryVars)
{
  PStringArray tokens = queryStr.Tokenise("&=", TRUE);
  for (PINDEX i = 0; i < tokens.GetSize(); i += 2) {
    PCaselessString key = tokens[i];
    UnmangleString(key);
    PString data = tokens[i+1];
    UnmangleString(data);
    if (queryVars.Contains(key))
      queryVars.SetAt(key, queryVars[key] + ',' + data);
    else
      queryVars.SetAt(key, data);
  }
}


void PURL::Parse(const char * cstr)
{
  scheme = "http";
  hostname = PCaselessString();
  pathStr = username = password = parameters = fragment = queryStr = PString();
  path.SetSize(0);
  queryVars.RemoveAll();
  port = 0;
  absolutePath = TRUE;

  // copy the string so we can take bits off it
  PString url = cstr;

  PINDEX pos;

  static PString reservedChars = "=;/#?";

  // determine if the URL has a scheme
  if (isalpha(url[0])) {
    for (pos = 0; url[pos] != '\0' &&
                          reservedChars.Find(url[pos]) == P_MAX_INDEX; pos++) {
      if (url[pos] == ':') {
        scheme = url.Left(pos);
        UnmangleString(scheme);
        url.Delete(0, pos+1);
        break;
      }
    }
  }

  // determine if the URL is absolute or relative - only absolute
  // URLs can have a username/password string
  if (url.GetLength() > 2 && url[0] == '/' && url[1] == '/') {

    // remove the leading //
    url.Delete(0, 2);

    // extract username and password
    PINDEX pos2 = url.Find('@');
    if (pos2 != P_MAX_INDEX && pos2 > 0) {
      pos = url.Find(":");

      // if no password...
      if (pos > pos2)
        username = url(0, pos2-1);
      else {
        username = url(0, pos-1);
        password = url(pos+1, pos2-1);
      }
      UnmangleString(username);
      UnmangleString(password);
      url.Delete(0, pos2+1);
    }

    // determine if the URL has a port number
    for (pos = 0; url[pos] != '\0'; pos++)
      if (reservedChars.Find(url[pos]) != P_MAX_INDEX)
        break;

    pos2 = url.Find(":");
    if (pos2 >= pos) 
      hostname = url.Left(pos);
    else {
      hostname = url.Left(pos2);
      port = (WORD)url(pos2+1, pos).AsInteger();
    }
    UnmangleString(hostname);
    if (hostname.IsEmpty())
      hostname = PIPSocket::GetHostName();
    url.Delete(0, pos);
  }

  // chop off any trailing fragment
  pos = url.FindLast('#');
  if (pos != P_MAX_INDEX && pos > 0) {
    fragment = url(pos+1, P_MAX_INDEX);
    UnmangleString(fragment);
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing query
  pos = url.FindLast('?');
  if (pos != P_MAX_INDEX && pos > 0) {
    queryStr = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
    SplitVars(queryStr, queryVars);
  }

  // chop off any trailing parameters
  pos = url.FindLast(';');
  if (pos != P_MAX_INDEX && pos > 0) {
    parameters = url(pos+1, P_MAX_INDEX);
    UnmangleString(parameters);
    url.Delete(pos, P_MAX_INDEX);
  }

  // the hierarchy is what is left
  pathStr = url;
  path = url.Tokenise('/', FALSE);
  absolutePath = path[0].IsEmpty();
  if (absolutePath)
    path.RemoveAt(0);
  if (path.GetSize() > 0 && path[path.GetSize()-1].IsEmpty())
    path.RemoveAt(path.GetSize()-1);
  for (pos = 0; pos < path.GetSize(); pos++) {
    UnmangleString(path[pos]);
    if (pos > 0 && path[pos] == ".." && path[pos-1] != "..") {
      path.RemoveAt(pos--);
      path.RemoveAt(pos--);
    }
  }
}


PString PURL::AsString(UrlFormat fmt) const
{
  PStringStream str;

  if (fmt == FullURL) {
    if (!scheme.IsEmpty())
      str << scheme << ':';
    if (!username.IsEmpty() || !password.IsEmpty() ||
                                             !hostname.IsEmpty() || port != 0) {
      str << "//" << username;
      if (!password.IsEmpty())
        str << ':' << password;
      if (!username.IsEmpty() || !password.IsEmpty())
        str << '@';
      if (hostname.IsEmpty())
        str << PIPSocket::GetHostName();
      else
        str << hostname;
      // default the port as required
      if (port != 0 &&
          !(scheme == "http" &&     port == DEFAULT_HTTP_PORT) &&
          !(scheme == "https" &&    port == DEFAULT_HTTPS_PORT) &&
          !(scheme == "ftp" &&      port == DEFAULT_FTP_PORT) &&
          !(scheme == "gopher" &&   port == DEFAULT_GOPHER_PORT) &&
          !(scheme == "nntp" &&     port == DEFAULT_NNTP_PORT) &&
          !(scheme == "telnet" &&   port == DEFAULT_TELNET_PORT) &&
          !(scheme == "wais" &&     port == DEFAULT_WAIS_PORT) &&
          !(scheme == "prospero" && port == DEFAULT_PROSPERO_PORT))
        str << ':' << port;
    }
  }

  PINDEX count = path.GetSize();
  if (absolutePath)
    str << '/';
  for (PINDEX i = 0; i < count; i++) {
    str << path[i];
    if (i < count-1)
      str << '/';
  }

  if (fmt == FullURL || fmt == URIOnly) {
    if (!parameters.IsEmpty())
      str << ";" << parameters;

    if (!queryStr.IsEmpty())
      str << "?" << queryStr;

    if (!fragment.IsEmpty())
      str << "#" << fragment;
  }

  return str;
}


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
  for (PINDEX i = 0; i < path.GetSize(); i++) {
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
// PHTTPSocket

static char const * HTTPCommands[PHTTPSocket::NumCommands] = {
  // HTTP 1.0 commands
  "GET", "HEAD", "POST",

  // HTTP 1.1 commands
  "PUT",  "PATCH", "COPY",    "MOVE",   "DELETE",
  "LINK", "UNLINK", "TRACE", "WRAPPED", "OPTIONS",
  "CONNECT"
};

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

PHTTPSocket::PHTTPSocket(WORD port)
  : PApplicationSocket(NumCommands, HTTPCommands, port)
{
}

PHTTPSocket::PHTTPSocket(const PString & address, WORD port)
  : PApplicationSocket(NumCommands, HTTPCommands, address, port)
{
}


PHTTPSocket::PHTTPSocket(const PString & address, const PString & service)
  : PApplicationSocket(NumCommands, HTTPCommands, address, service)
{
}


PHTTPSocket::PHTTPSocket(PSocket & socket)
  : PApplicationSocket(NumCommands, HTTPCommands, socket)
{
}


PHTTPSocket::PHTTPSocket(PSocket & socket, const PHTTPSpace & space)
  : PApplicationSocket(NumCommands, HTTPCommands, socket),
    urlSpace(space)
{
}


int PHTTPSocket::ExecuteCommand(Commands cmd,
                                const PString & url,
                                const PMIMEInfo & outMIME,
                                const PString & dataBody,
                                PMIMEInfo & replyMime)
{
  if (!WriteCommand(cmd, url, outMIME, dataBody))
    return -1;

  if (!ReadResponse(replyMime))
    return -1;

  return lastResponseCode;
}

BOOL PHTTPSocket::WriteCommand(Commands cmd,
                               const PString & url,
                               const PMIMEInfo & outMIME,
                               const PString & dataBody)
{
  return PApplicationSocket::WriteCommand(cmd, url & "HTTP/1.0") &&
      outMIME.Write(*this) &&
      WriteString(dataBody);
}

BOOL PHTTPSocket::ReadResponse(PMIMEInfo & replyMIME)
{
  if (!PApplicationSocket::ReadResponse())
    return FALSE;

  if (lastResponseInfo.Left(8) == "HTTP/0.9")
    return TRUE;

  return replyMIME.Read(*this);
}


BOOL PHTTPSocket::GetDocument(const PURL & url,
                              const PMIMEInfo & outMIME,
                              PMIMEInfo & replyMIME)
{
  return ExecuteCommand(GET, url.AsString(PURL::URIOnly), outMIME, PString(), replyMIME);
}


BOOL PHTTPSocket::GetHeader(const PURL & url,
                            const PMIMEInfo & outMIME,
                            PMIMEInfo & replyMIME)
{
  return ExecuteCommand(HEAD, url.AsString(PURL::URIOnly), outMIME, PString(), replyMIME);
}


BOOL PHTTPSocket::PostData(const PURL & url,
                           const PMIMEInfo & outMIME,
                           const PStringToString & data,
                           PMIMEInfo & replyMIME)
{
  PStringStream body;
  body << data;
  return ExecuteCommand(HEAD, url.AsString(PURL::URIOnly), outMIME, body, replyMIME);
}


PINDEX PHTTPSocket::ParseResponse(const PString & line)
{
  PINDEX endVer = line.Find(' ');
  if (endVer == P_MAX_INDEX || line.Left(endVer) != "HTTP/1.0") {
    UnRead(line);
    lastResponseCode = 200;
    lastResponseInfo = "HTTP/0.9";
    return 0;
  }

  lastResponseInfo = line.Left(endVer);
  PINDEX endCode = line.Find(' ', endVer+1);
  lastResponseCode = line(endVer+1,endCode-1).AsInteger();
  lastResponseInfo &= line.Mid(endCode);
  return 0;
}


BOOL PHTTPSocket::ProcessCommand()
{
  PString args;
  PINDEX cmd;
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

  // If the protocol is version 1.0 or greater, there is MIME info, and the
  // prescence of a an entity body is signalled by the inclusion of
  // Content-Length header. If the protocol is less than version 1.0, then 
  // there is no entity body!
  PHTTPConnectionInfo connectInfo;
  PMIMEInfo mimeInfo;
  PString entityBody;
  long contentLength = 0;

  if (majorVersion > 0) {

    // at this stage we should be ready to collect the MIME info
    // until an empty line is received, or EOF
    mimeInfo.Read(*this);

    // build our connection info
    connectInfo.Construct(mimeInfo, majorVersion, minorVersion);

    // if the client specified a persistant connection, then use the
    // ContentLength field. If there is no content length field, then
    // assume 0. This isn't what the spec says, but it's what Netscape
    // does so we don't really have a choice
    // If the client didn't specify a persistant connection, then use the
    // ContentLength if there is one or read until end of file if there isn't
    if (connectInfo.IsPersistant())
      contentLength = mimeInfo.GetInteger(ContentLengthStr, 0);
    else 
      contentLength = mimeInfo.GetInteger(ContentLengthStr, -1);

    // a content length of > 0 means read explicit length
    // a content length of < 0 means read until EOF
    int count = 0;
    if (contentLength > 0) {
      entityBody = ReadString((PINDEX)contentLength);
    } else if (contentLength < 0) {
      while (Read(entityBody.GetPointer(count+1000)+count, 1000))
        count += GetLastReadCount();
      entityBody.SetSize(count+1);
    }
  }

  // always shutdown the incoming stream after we have read it
  if (!connectInfo.IsPersistant())
    Shutdown(ShutdownRead);

  // the URL that comes with Connect requests is not quite kosher, so 
  // mangle it into a proper URL
  PURL url = tokens[0];
  if (cmd == CONNECT) 
    url = "https://" + tokens[0];

  // If the incoming URL is of a proxy type then call OnProxy() which will
  // probably just go OnError(). Even if a full URL is provided in the
  // command we should check to see if it is a local server request and process
  // it anyway even though we are not a proxy. The usage of GetHostName()
  // below are to catch every way of specifying the host (name, alias, any of
  // several IP numbers etc).
  if (connectInfo.IsProxyConnection() ||
      url.GetScheme() != "http" ||
      (url.GetPort() != 0 && url.GetPort() != GetPort()) ||
      (!url.GetHostName().IsEmpty() &&
         PIPSocket::GetHostName(url.GetHostName()) != PIPSocket::GetHostName())) {
    return OnProxy((Commands)cmd, url, mimeInfo, entityBody, connectInfo) && connectInfo.IsPersistant();
  }

  // Handle the local request
  PStringToString postData;
  BOOL persist = TRUE;
  switch (cmd) {
    case GET :
      persist = OnGET(url, mimeInfo, connectInfo);
//      Shutdown(ShutdownWrite);
      break;

    case HEAD :
      persist = OnHEAD(url, mimeInfo, connectInfo);
      break;

    case POST :
      SplitVars(entityBody, postData);
      persist = OnPOST(url, mimeInfo, postData, connectInfo);
      break;

    case P_MAX_INDEX:
    default:
      OnUnknown(args, connectInfo);
      return connectInfo.IsPersistant();
  }

  // if the function just indicated that the connection is to persist,
  // and so did the client, then return TRUE. Note that all of the OnXXXX
  // routines above must make sure that their return value is FALSE if
  // if there was no ContentLength field in the response. This ensures that
  // we always close the socket so the client will get the correct end of file
  if (persist && connectInfo.IsPersistant())
    return TRUE;

  // close the output stream now and return FALSE
  Shutdown(ShutdownWrite);
  return FALSE;
}


PString PHTTPSocket::GetServerName() const
{
  return "PWLib-HTTP-Server/1.0 PWLib/1.0";
}


BOOL PHTTPSocket::OnGET(const PURL & url,
                   const PMIMEInfo & info,
         const PHTTPConnectionInfo & connectInfo)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL) 
    return OnError(NotFound, url.AsString(), connectInfo);
  else 
    return resource->OnGET(*this, url, info, connectInfo);
}


BOOL PHTTPSocket::OnHEAD(const PURL & url,
                    const PMIMEInfo & info,
          const PHTTPConnectionInfo & connectInfo)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL) 
    return OnError(NotFound, url.AsString(), connectInfo);
  else
    return resource->OnHEAD(*this, url, info, connectInfo);
}


BOOL PHTTPSocket::OnPOST(const PURL & url,
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


BOOL PHTTPSocket::OnProxy(Commands,
                          const PURL &,
                          const PMIMEInfo &,
                          const PString &, 
                          const PHTTPConnectionInfo & connectInfo)
{
  return OnError(BadGateway, "Proxy not implemented.", connectInfo);
}

static struct httpStatusCodeStruct {
  char *  text;
  int     code;
  BOOL    allowedBody;
  int     majorVersion;
  int     minorVersion;
} httpStatusDefn[PHTTPSocket::NumStatusCodes-1000] = {
  { "Continue",                      100, 1, 1, 1 },
  { "Switching Protocols",           101, 1, 1, 1 },
  { "OK",                            200, 1 },
  { "Created",                       201, 1 },
  { "Accepted",                      202, 1 },
  { "Non-Authoritative Information", 203, 1, 1, 1 },
  { "No Content",                    204, 0 },
  { "Reset Content",                 205, 0, 1, 1 },
  { "Partial Content",               206, 1, 1, 1 },
  { "Multiple Choices",              300, 1, 1, 1 },
  { "Moved Permanently",             301, 1 },
  { "Moved Temporarily",             302, 1 },
  { "See Other",                     303, 1, 1, 1 },
  { "Not Modified",                  304, 0 },
  { "Use Proxy",                     305, 1, 1, 1 },
  { "Bad Request",                   400, 1 },
  { "Unauthorised",                  401, 1 },
  { "Payment Required",              402, 1, 1, 1 },
  { "Forbidden",                     403, 1 },
  { "Not Found",                     404, 1 },
  { "Method Not Allowed",            405, 1, 1, 1 },
  { "None Acceptable",               406, 1, 1, 1 },
  { "Proxy Authetication Required",  407, 1, 1, 1 },
  { "Request Timeout",               408, 1, 1, 1 },
  { "Conflict",                      409, 1, 1, 1 },
  { "Gone",                          410, 1, 1, 1 },
  { "Length Required",               411, 1, 1, 1 },
  { "Unless True",                   412, 1, 1, 1 },
  { "Internal Server Error",         500, 1 },
  { "Not Implemented",               501, 1 },
  { "Bad Gateway",                   502, 1 },
  { "Service Unavailable",           503, 1, 1, 1 },
  { "Gateway Timeout",               504, 1, 1, 1 }
};

void PHTTPSocket::StartResponse(StatusCode code,
                                PMIMEInfo & headers,
                                long bodySize)
{
  if (majorVersion < 1)
    return;

  // make sure the error code is valid for the protocol version
  for (PINDEX i = 0; code < 1000 && i < PHTTPSocket::NumStatusCodes; i++)
    if (code == httpStatusDefn[i].code)
      code = (StatusCode)(1000+i);
  if (code < 1000 || code >= PHTTPSocket::NumStatusCodes+1000)
    code = InternalServerError;
  httpStatusCodeStruct * statusInfo = httpStatusDefn+code-1000;

  // output the command line
  *this << "HTTP/" << majorVersion << '.' << minorVersion << ' '
        << statusInfo->code << ' ' << statusInfo->text << "\r\n";

  if (bodySize >= 0 && !headers.Contains(ContentLengthStr))
    headers.SetAt(ContentLengthStr, PString(PString::Unsigned, (PINDEX)bodySize));
  headers.Write(*this);
}


void PHTTPSocket::SetDefaultMIMEInfo(PMIMEInfo & info,
                     const PHTTPConnectionInfo & connectInfo)
{
  PTime now;
  info.SetAt(DateStr, now.AsString(PTime::RFC1123, PTime::GMT));
  info.SetAt(MIMEVersionStr, psprintf("%i.%i",
             connectInfo.GetMajorVersion(), connectInfo.GetMinorVersion()));
  info.SetAt(ServerStr, GetServerName());
  if (connectInfo.IsPersistant()) {
    if (connectInfo.IsProxyConnection())
      info.SetAt(ProxyConnectionStr, KeepAliveStr);
    else
      info.SetAt(ConnectionStr, KeepAliveStr);
  }
}



BOOL PHTTPSocket::OnUnknown(const PCaselessString & cmd, 
                        const PHTTPConnectionInfo & connectInfo)
{
  return OnError(NotImplemented, cmd, connectInfo);
}

static compatibleErrors[] = { PHTTPSocket::Continue,
                              PHTTPSocket::OK,
                              PHTTPSocket::MultipleChoices,
                              PHTTPSocket::BadRequest,
                              PHTTPSocket::InternalServerError
                            };

BOOL PHTTPSocket::OnError(StatusCode code,
                     const PString & extra,
         const PHTTPConnectionInfo & connectInfo)
{
  httpStatusCodeStruct * statusInfo = httpStatusDefn+code-1000;

  if (!connectInfo.IsCompatible(statusInfo->majorVersion, statusInfo->minorVersion)) 
    statusInfo = httpStatusDefn + compatibleErrors[(statusInfo->code/100)-1] - 1000;

  PMIMEInfo headers;
  SetDefaultMIMEInfo(headers, connectInfo);

  if (!statusInfo->allowedBody) {
    StartResponse(code, headers, 0);
    return TRUE;
  }

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
        << extra
        << PHTML::Body();

  headers.SetAt(ContentTypeStr, "text/html");
  StartResponse(code, headers, reply.GetLength());
  WriteString(reply);
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSimpleAuth

PHTTPSimpleAuth::PHTTPSimpleAuth(const PString & realm_,
                                 const PString & username_,
                                 const PString & password_)
  : realm(realm_), username(username_), password(password_)
{
  PAssert(!realm.IsEmpty(), "Must have a realm!");
}


PObject * PHTTPSimpleAuth::Clone() const
{
  return PNEW PHTTPSimpleAuth(realm, username, password);
}


PString PHTTPSimpleAuth::GetRealm() const
{
  return realm;
}


BOOL PHTTPSimpleAuth::Validate(const PString & authInfo) const
{
  PString decoded;
  if (authInfo(0, 5) *= "Basic ")
    decoded = PBase64::Decode(authInfo(6, P_MAX_INDEX));
  else
    decoded = PBase64::Decode(authInfo);

  PINDEX colonPos = decoded.Find(':');
  if (colonPos == P_MAX_INDEX) 
    return FALSE;

  return username == decoded.Left(colonPos).Trim() &&
         password == decoded(colonPos+1, P_MAX_INDEX).Trim();
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPRequest

PHTTPRequest::PHTTPRequest(const PURL & u, const PMIMEInfo & iM)
  : url(u), inMIME(iM)
{
  code        = PHTTPSocket::OK;
  contentSize = 0;
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


BOOL PHTTPResource::OnGET(PHTTPSocket & socket,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo)
{
  if (!CheckAuthority(socket, info, connectInfo))
    return TRUE;

  if (info.Contains(IfModifiedSinceStr) &&
                           !IsModifiedSince(PTime(info[IfModifiedSinceStr]))) 
    return socket.OnError(PHTTPSocket::NotModified, url.AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(url, info);
  socket.SetDefaultMIMEInfo(request->outMIME, connectInfo);

  PTime expiryDate;
  if (GetExpirationDate(expiryDate))
    request->outMIME.SetAt(ExpiresStr, expiryDate.AsString(PTime::RFC1123, PTime::GMT));

  if (!LoadHeaders(*request)) {
    BOOL stat = socket.OnError(request->code, url.AsString(), connectInfo);
    delete request;
    return stat;
  }

  hitCount++;

  if (!request->outMIME.Contains(ContentTypeStr) && !contentType.IsEmpty())
    request->outMIME.SetAt(ContentTypeStr, contentType);

  PCharArray data;
  char * ptr;
  PINDEX len, slen;
  if (LoadData(*request, data)) {
    socket.StartResponse(request->code,request->outMIME,request->contentSize);
    do {
      slen = data.GetSize();
      len = 0;
      ptr = data.GetPointer();
      while (len < slen && socket.Write(ptr+len, slen - len))
        len += socket.GetLastWriteCount();
      data.SetSize(0);
    } while (LoadData(*request, data));
  }
  else
    socket.StartResponse(request->code, request->outMIME, data.GetSize());

  if (data.GetSize() > 0) {
    slen = data.GetSize();
    len = 0;
    ptr = data.GetPointer();
    while (len < slen && socket.Write(ptr+len, slen - len))
      len += socket.GetLastWriteCount();
  }

  BOOL retVal = request->outMIME.Contains(ContentLengthStr);
  delete request;
  return retVal;
}


BOOL PHTTPResource::OnHEAD(PHTTPSocket & socket,
                            const PURL & url,
                       const PMIMEInfo & info,
             const PHTTPConnectionInfo & connectInfo)
{
  if (!CheckAuthority(socket, info, connectInfo))
    return TRUE;

  PHTTPRequest * request = CreateRequest(url, info);
  socket.SetDefaultMIMEInfo(request->outMIME, connectInfo);

  PTime expiryDate;
  if (GetExpirationDate(expiryDate))
    request->outMIME.SetAt(ExpiresStr, expiryDate.AsString(PTime::RFC1123, PTime::GMT));

  if (LoadHeaders(*request)) {
    if (!request->outMIME.Contains(ContentTypeStr) && !contentType.IsEmpty())
      request->outMIME.SetAt(ContentTypeStr, contentType);
    socket.StartResponse(request->code,request->outMIME,request->contentSize);
  }
  else
    return socket.OnError(request->code, url.AsString(), connectInfo);

  BOOL retVal = request->outMIME.Contains(ContentLengthStr);
  delete request;
  return retVal;
}


BOOL PHTTPResource::OnPOST(PHTTPSocket & socket,
                            const PURL & url,
                       const PMIMEInfo & info,
                 const PStringToString & data,
             const PHTTPConnectionInfo & connectInfo)
{
  if (!CheckAuthority(socket, info, connectInfo)) 
    return TRUE;

  PHTTPRequest * request = CreateRequest(url, info);
  PHTML msg;
  Post(*request, data, msg);
  if (msg.IsEmpty())
    return socket.OnError(request->code, "", connectInfo);

  request->outMIME.SetAt(ContentTypeStr, "text/html");
  if (msg.Is(PHTML::InBody))
    msg << PHTML::Body();
  PINDEX len = msg.GetLength();
  socket.StartResponse(request->code, request->outMIME, len);
  socket.Write((const char *)msg, len);

  BOOL persist = request->outMIME.Contains(ContentLengthStr);
  delete request;
  return persist;
}


BOOL PHTTPResource::CheckAuthority(PHTTPSocket & socket,
                               const PMIMEInfo & info,
                     const PHTTPConnectionInfo & connectInfo)
{
  if (authority == NULL)
    return TRUE;

  // if this is an authorisation request...
  if (info.Contains(PString("Authorization"))) {
    if (authority->Validate(info["Authorization"]))
      return TRUE;

    socket.OnError(PHTTPSocket::Forbidden, "", connectInfo);
  }
  else {
    // it must be a request for authorisation
    PMIMEInfo reply;
    socket.SetDefaultMIMEInfo(reply, connectInfo);
    reply.SetAt(WWWAuthenticateStr,
                              "Basic realm=\"" + authority->GetRealm() + "\"");
    socket.StartResponse(PHTTPSocket::UnAuthorised, reply, 0);
  }
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
  request.code = PHTTPSocket::MethodNotAllowed;
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
    request.code = PHTTPSocket::NotFound;
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
  for (PINDEX i = baseURL.GetPath().GetSize(); i < path.GetSize()-1; i++)
    realPath += path[i] + PDIR_SEPARATOR;

  if (i < path.GetSize())
    realPath += path[i];

  // See if its a normal file
  PFileInfo info;
  if (!PFile::GetInfo(realPath, info)) {
    request.code = PHTTPSocket::NotFound;
    return FALSE;
  }

  // Now try and open it
  PFile & file = ((PHTTPDirRequest&)request).file;
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(realPath, PFile::ReadOnly)) {
      request.code = PHTTPSocket::NotFound;
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


//////////////////////////////////////////////////////////////////////////////
// PHTTPForm

PHTTPField::PHTTPField(const char * nam, const char * titl, const char * hlp)
  : name(nam),
    title(titl != NULL ? titl : nam),
    help(hlp != NULL ? hlp : "")
{
  notInHTML = TRUE;
}

PObject::Comparison PHTTPField::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PHTTPField::Class()), PInvalidCast);
  return name.Compare(((const PHTTPField &)obj).name);
}


void PHTTPField::SetHelp(const PString & hotLinkURL,
                         const PString & linkText)
{
  help = "<A HREF=\"" + hotLinkURL + "\">" + linkText + "</A>\r\n";
}


void PHTTPField::SetHelp(const PString & hotLinkURL,
                         const PString & imageURL,
                         const PString & imageText)
{
  help = "<A HREF=\"" + hotLinkURL + "\"><IMG SRC=\"" +
             imageURL + "\" ALT=\"" + imageText + "\" ALIGN=absmiddle></A>\r\n";
}


BOOL PHTTPField::Validated(const PString &, PStringStream &) const
{
  return TRUE;
}


PHTTPStringField::PHTTPStringField(const char * name,
                                   PINDEX siz,
                                   const char * initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help), value(initVal != NULL ? initVal : "")
{
  size = siz;
}


PHTTPStringField::PHTTPStringField(const char * name,
                                   const char * title,
                                   PINDEX siz,
                                   const char * initVal,
                                   const char * help)
  : PHTTPField(name, title, help), value(initVal != NULL ? initVal : "")
{
  size = siz;
}


void PHTTPStringField::GetHTML(PHTML & html)
{
  html << PHTML::InputText(name, size, value);
  notInHTML = FALSE;
}


void PHTTPStringField::SetValue(const PString & val)
{
  value = val;
}


PString PHTTPStringField::GetValue() const
{
  return value;
}


PHTTPPasswordField::PHTTPPasswordField(const char * name,
                                       PINDEX siz,
                                       const char * initVal,
                                       const char * help)
  : PHTTPStringField(name, siz, initVal, help)
{
}


PHTTPPasswordField::PHTTPPasswordField(const char * name,
                                       const char * title,
                                       PINDEX siz,
                                       const char * initVal,
                                       const char * help)
  : PHTTPStringField(name, title, siz, initVal, help)
{
}


void PHTTPPasswordField::GetHTML(PHTML & html)
{
  html << PHTML::InputPassword(name, size, value);
  notInHTML = FALSE;
}


PHTTPIntegerField::PHTTPIntegerField(const char * nam,
                                     int lo, int hig,
                                     int initVal,
                                     const char * unit,
                                     const char * help)
  : PHTTPField(nam, NULL, help), units(unit != NULL ? unit : "")
{
  low = lo;
  high = hig;
  value = initVal;
}

PHTTPIntegerField::PHTTPIntegerField(const char * nam,
                                     const char * titl,
                                     int lo, int hig,
                                     int initVal,
                                     const char * unit,
                                     const char * help)
  : PHTTPField(nam, titl, help), units(unit != NULL ? unit : "")
{
  low = lo;
  high = hig;
  value = initVal;
}


BOOL PHTTPIntegerField::Validated(const PString & newVal,
                                                     PStringStream & msg) const
{
  int val = newVal.AsInteger();
  if (val >= low && val <= high)
    return TRUE;

  msg << "The field \"" << name << "\" should be between "
      << low << " and " << high << ".<BR>";
  return FALSE;
}


void PHTTPIntegerField::GetHTML(PHTML & html)
{
  html << PHTML::InputRange(name, low, high, value) << "  " << units;
  notInHTML = FALSE;
}


void PHTTPIntegerField::SetValue(const PString & val)
{
  value = val.AsInteger();
}


PString PHTTPIntegerField::GetValue() const
{
  return PString(PString::Signed, value);
}


PHTTPBooleanField::PHTTPBooleanField(const char * name,
                                     BOOL initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help)
{
  value = initVal;
}

PHTTPBooleanField::PHTTPBooleanField(const char * name,
                                     const char * title,
                                     BOOL initVal,
                                     const char * help)
  : PHTTPField(name, title, help)
{
  value = initVal;
}


void PHTTPBooleanField::GetHTML(PHTML & html)
{
  html << PHTML::CheckBox(name, value ? PHTML::Checked : PHTML::UnChecked);
  notInHTML = FALSE;
}


void PHTTPBooleanField::SetValue(const PString & val)
{
  value = val[0] != 'F';
}


PString PHTTPBooleanField::GetValue() const
{
  return (value ? "T" : "F");
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const PStringArray & valueArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(valueArray),
    titles(valueArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const PStringArray & valueArray,
                                 const PStringArray & titleArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(valueArray),
    titles(titleArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(count, valueStrings),
    titles(count, valueStrings),
    value(valueStrings[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 const char * const * titleStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(count, valueStrings),
    titles(count, titleStrings),
    value(valueStrings[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 const PStringArray & valueArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(valueArray),
    titles(valueArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 const PStringArray & valueArray,
                                 const PStringArray & titleArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(valueArray),
    titles(titleArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(count, valueStrings),
    titles(count, valueStrings),
    value(valueStrings[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 const char * const * titleStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(count, valueStrings),
    titles(count, titleStrings),
    value(valueStrings[initVal])
{
}


void PHTTPRadioField::GetHTML(PHTML & html)
{
  for (PINDEX i = 0; i < values.GetSize(); i++)
    html << PHTML::RadioButton(name, values[i],
                        values[i] == value ? PHTML::Checked : PHTML::UnChecked)
         << titles[i]
         << PHTML::BreakLine();
  notInHTML = FALSE;
}


PString PHTTPRadioField::GetValue() const
{
  return value;
}


void PHTTPRadioField::SetValue(const PString & val)
{
  value = val;
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const PStringArray & valueArray,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help),
    values(valueArray),
    value(valueArray[initVal])
{
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   PINDEX count,
                                   const char * const * valueStrings,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help),
    values(count, valueStrings),
    value(valueStrings[initVal])
{
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const char * title,
                                   const PStringArray & valueArray,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, title, help),
    values(valueArray),
    value(valueArray[initVal])
{
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const char * title,
                                   PINDEX count,
                                   const char * const * valueStrings,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, title, help),
    values(count, valueStrings),
    value(valueStrings[initVal])
{
}


void PHTTPSelectField::GetHTML(PHTML & html)
{
  html << PHTML::Select(name);
  for (PINDEX i = 0; i < values.GetSize(); i++)
    html << PHTML::Option(
                    values[i] == value ? PHTML::Selected : PHTML::NotSelected)
         << values[i];
  html << PHTML::Select();
  notInHTML = FALSE;
}


PString PHTTPSelectField::GetValue() const
{
  return value;
}


void PHTTPSelectField::SetValue(const PString & val)
{
  value = val;
}



PHTTPForm::PHTTPForm(const PURL & url)
  : PHTTPString(url)
{
}

PHTTPForm::PHTTPForm(const PURL & url, const PHTTPAuthority & auth)
  : PHTTPString(url, auth)
{
}

PHTTPForm::PHTTPForm(const PURL & url, const PString & html)
  : PHTTPString(url, html)
{
}

PHTTPForm::PHTTPForm(const PURL & url,
                     const PString & html,
                     const PHTTPAuthority & auth)
  : PHTTPString(url, html, auth)
{
}


PHTTPField * PHTTPForm::Add(PHTTPField * fld)
{
  PAssertNULL(fld);
  PAssert(!fieldNames[fld->GetName()], "Field already on form!");
  fieldNames += fld->GetName();
  fields.Append(fld);
  return fld;
}


void PHTTPForm::BuildHTML(const char * heading)
{
  PHTML html = heading;
  BuildHTML(html);
}


void PHTTPForm::BuildHTML(const PString & heading)
{
  PHTML html = heading;
  BuildHTML(html);
}


void PHTTPForm::BuildHTML(PHTML & html, BuildOptions option)
{
  if (!html.Is(PHTML::InForm))
    html << PHTML::Form("POST");

  html << PHTML::Table();
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    if (fields[fld].NotYetInHTML()) {
      html << PHTML::TableRow()
           << PHTML::TableData("align=right")
           << fields[fld].GetTitle()
           << PHTML::TableData("align=left");
      fields[fld].GetHTML(html);
      html << PHTML::TableData()
           << fields[fld].GetHelp();
    }
  }
  html << PHTML::Table();
  if (option != InsertIntoForm)
    html << PHTML::Paragraph()
         << ' ' << PHTML::SubmitButton("Accept")
         << ' ' << PHTML::ResetButton("Reset")
         << PHTML::Form();

  if (option == CompleteHTML) {
    html << PHTML::Body();
    string = html;
  }
}


BOOL PHTTPForm::Post(PHTTPRequest & request,
                     const PStringToString & data,
                     PHTML & msg)
{
  msg = "Error in Request";
  if (data.GetSize() == 0) {
    msg << "No parameters changed." << PHTML::Body();
    request.code = PHTTPSocket::NoContent;
    return TRUE;
  }

  BOOL good = TRUE;
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    const PCaselessString & name = field.GetName();
    if (data.Contains(name) && !field.Validated(data[name], msg))
      good = FALSE;
  }

  if (!good) {
    msg << PHTML::Body();
    request.code = PHTTPSocket::BadRequest;
    return TRUE;
  }

  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    const PCaselessString & name = field.GetName();
    if (data.Contains(name))
      field.SetValue(data[name]);
  }

  msg = "Accepted New Configuration";
  msg << PHTML::Body();
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPConfig

PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & sect)
  : PHTTPForm(url), section(sect)
{
}


PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & sect,
                         const PHTTPAuthority & auth)
  : PHTTPForm(url, auth), section(sect)
{
}


PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & html,
                         const PString & sect)
  : PHTTPForm(url, html), section(sect)
{
}


PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & html,
                         const PString & sect,
                         const PHTTPAuthority & auth)
  : PHTTPForm(url, html, auth), section(sect)
{
}


BOOL PHTTPConfig::Post(PHTTPRequest & request,
                       const PStringToString & data,
                       PHTML & msg)
{
  PHTTPForm::Post(request, data, msg);
  if (request.code != PHTTPSocket::OK)
    return TRUE;

  PConfig cfg(section);
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    const PCaselessString & name = field.GetName();
    if (data.Contains(name)) {
      if (&field == keyField) {
        PString key = field.GetValue();
        if (!key.IsEmpty())
          cfg.SetString(key, valField->GetValue());
      }
      else if (&field != valField)
        cfg.SetString(name, field.GetValue());
    }
  }
  return TRUE;
}


void PHTTPConfig::SetConfigValues()
{
  PConfig cfg(section);
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    field.SetValue(cfg.GetString(field.GetName(), field.GetValue()));
  }
}


void PHTTPConfig::AddNewKeyFields(PHTTPField * keyFld,
                                  PHTTPField * valFld)
{
  keyField = PAssertNULL(keyFld);
  Add(keyFld);
  valField = PAssertNULL(valFld);
  Add(valFld);
}

PHTTPConnectionInfo::PHTTPConnectionInfo()
{
  isPersistant      = FALSE;
  isProxyConnection = FALSE;
  majorVersion      = 0;
  minorVersion      = 9;
}

void PHTTPConnectionInfo::Construct(const PMIMEInfo & mimeInfo,
                                    int major, int minor)
{
  isPersistant      = FALSE;
  majorVersion      = major;
  minorVersion      = minor;

  PString str;

  // check for Proxy-Connection and Connection strings
  isProxyConnection = mimeInfo.HasKey(ProxyConnectionStr);
  if (isProxyConnection)
    str = mimeInfo[ProxyConnectionStr];
  else if (mimeInfo.HasKey(ConnectionStr))
    str = mimeInfo[ConnectionStr];

  // get any connection options
  if (!str.IsEmpty()) {
    PStringArray tokens = str.Tokenise(",", TRUE);
    isPersistant = tokens.GetStringsIndex(KeepAliveStr) != P_MAX_INDEX;
  }
}

void PHTTPConnectionInfo::SetPersistance(BOOL newPersist)
{
  isPersistant = newPersist;
}

BOOL PHTTPConnectionInfo::IsCompatible(int major, int minor) const
{
  if (minor == 0 && major == 0)
    return TRUE;
  else
    return (major > majorVersion) ||
           ((major == majorVersion) && (minor >= minorVersion));
}

// End Of File ///////////////////////////////////////////////////////////////
