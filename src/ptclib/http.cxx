/*
 * $Id: http.cxx,v 1.7 1996/02/03 11:33:19 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: http.cxx,v $
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


//////////////////////////////////////////////////////////////////////////////
// PURL

PURL::PURL()
{
  port = 80;
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


void PURL::Parse(const char * cstr)
{
  scheme = hostname = PCaselessString();
  pathStr = username = password = parameters = fragment = queryStr = PString();
  path.SetSize(0);
  queryVars.RemoveAll();
  port = 80;
  absolutePath = TRUE;

  // copy the string so we can take bits off it
  PString url = cstr;

  static PString reservedChars = "=;/#?";
  PINDEX pos;

  pos = (PINDEX)-1;
  while ((pos = url.Find('%', pos+1)) != P_MAX_INDEX) {
    int digit1 = url[pos+1];
    int digit2 = url[pos+2];
    if (isxdigit(digit1) && isxdigit(digit2)) {
      url[pos] = (char)(
            (isdigit(digit2) ? (digit2-'0') : (toupper(digit2)-'A'+10)) +
           ((isdigit(digit1) ? (digit1-'0') : (toupper(digit1)-'A'+10)) << 4));
      url.Delete(pos+1, 2);
    }
  }

  // determine if the URL has a scheme
  if (isalpha(url[0])) {
    for (pos = 0; url[pos] != '\0' &&
                          reservedChars.Find(url[pos]) == P_MAX_INDEX; pos++) {
      if (url[pos] == ':') {
        scheme = url.Left(pos);
        url.Delete(0, pos+1);
        break;
      }
    }
  }

  // determine if the URL is absolute or relative - only absolute
  // URLs can have a username/password string
  if (url.GetLength() > 2 && url[0] == '/' && url[1] == '/') {
    // extract username and password
    PINDEX pos2 = url.Find('@');
    if (pos2 != P_MAX_INDEX && pos2 > 0) {
      pos = url.Find(":");

      // if no password...
      if (pos > pos2)
        username = url(2, pos2-1);
      else {
        username = url(2, pos-1);
        password = url(pos+1, pos2-1);
      }
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
    url.Delete(0, pos+1);
  }

  // chop off any trailing fragment
  pos = url.FindLast('#');
  if (pos != P_MAX_INDEX && pos > 0) {
    fragment = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing query
  pos = url.FindLast('?');
  if (pos != P_MAX_INDEX && pos > 0) {
    queryStr = url(pos+1, P_MAX_INDEX);
    PStringArray tokens = queryStr.Tokenise("&=", TRUE);
    for (PINDEX i = 0; i < tokens.GetSize(); i += 2) {
      PCaselessString key = tokens[i];
      if (queryVars.GetAt(key) != NULL) 
        queryVars.SetAt(key, queryVars[key] + "," + tokens[i+1]);
      else
        queryVars.SetAt(key, tokens[i+1]);
    }
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing parameters
  pos = url.FindLast(';');
  if (pos != P_MAX_INDEX && pos > 0) {
    parameters = url(pos+1, P_MAX_INDEX);
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
                                           !hostname.IsEmpty() || port != 80) {
      str << "//";
      if (!username.IsEmpty() || !password.IsEmpty())
        str << username << ':' << password << '@';
      if (hostname.IsEmpty())
        str << "localhost";
      else
        str << hostname;
      if (port != 80)
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

  if (fmt == FullURL) {
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


PHTTPSpace::PHTTPSpace(const PString & nam)
  : name(nam)
{
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


BOOL PHTTPSpace::AddResource(PHTTPResource * res)
{
  PAssert(res != NULL, PInvalidParameter);
  const PStringArray & path = res->GetURL().GetPath();
  PHTTPSpace * node = this;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      pos = node->children.Append(PNEW PHTTPSpace(path[i]));
    node = &node->children[pos];
    if (node->resource != NULL)
      return FALSE;   // Already a resource in tree in partial path
  }
  if (!node->children.IsEmpty())
    return FALSE;   // Already a resource in tree further down path.
  node->resource = res;
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
  "GET", "HEAD", "POST"
};

static const PString ContentLengthStr = "Content-Length";
static const PString ContentTypeStr = "Content-Type";


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


BOOL PHTTPSocket::GetDocument(const PString & url)
{
  WriteCommand(GET, url);
  return FALSE;
}


BOOL PHTTPSocket::GetHeader(const PString & url)
{
  WriteCommand(HEAD, url);
  return FALSE;
}


BOOL PHTTPSocket::PostData(const PString & url, const PStringToString & data)
{
  WriteCommand(POST, url);
  for (PINDEX i = 0; i < data.GetSize(); i++)
    WriteString(data.GetKeyAt(i) + " = " + data.GetDataAt(i));
  return FALSE;
}


BOOL PHTTPSocket::ProcessCommand()
{
  PString args;
  PINDEX cmd;
  if (!ReadCommand(cmd, args))
    return FALSE;

  if (cmd == P_MAX_INDEX)   // Unknown command
    return OnUnknown(args);

  PStringArray tokens = args.Tokenise(" \t", FALSE);

  // if no tokens, error
  if (tokens.IsEmpty()) {
    OnError(BadRequest, args);
    return FALSE;
  }

  PURL url = tokens[0];

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
      OnError(BadRequest, "Malformed version number " + verStr);
      return FALSE;
    }

    // should actually check if the text contains only digits, but the
    // chances of matching everything else and it not being a valid number
    // are pretty small, so don't bother
    majorVersion = (int)verStr(5, dotPos-1).AsInteger();
    minorVersion = (int)verStr(dotPos+1, P_MAX_INDEX).AsInteger();
  }

  // If the protocol is version 1.0 or greater, there is MIME info and the
  // prescence of a an entity body is signalled by the inclusion of
  // Content-Length header. If the protocol is less than version 1.0, then the
  // entity body is all remaining bytes until EOF
  PMIMEInfo mimeInfo;
  PString entityBody;
  if (majorVersion >= 1) {
    // at this stage we should be ready to collect the MIME info
    // until an empty line is received, or EOF
    mimeInfo.Read(*this);

    // if there was a Content-Length header, then it gives the exact
    // length of the entity body. Otherwise, read the entity-body until EOF
    long contentLength = mimeInfo.GetInteger(ContentLengthStr, 0);
    if (contentLength > 0) {
      entityBody = ReadString((PINDEX)contentLength);
      if (GetLastReadCount() != contentLength) {
        OnError(BadRequest, "incomplete entity-body received");
        return FALSE;
      }
    }
  }
  else {
    int count = 0;
    while (Read(entityBody.GetPointer(count+100)+count, 100))
      count += GetLastReadCount();
  }

  switch (cmd) {
    case GET :
      OnGET(url, mimeInfo);
      break;

    case HEAD :
      OnHEAD(url, mimeInfo);
      break;

    case POST :
      PStringToString postData;
      // break the string into string/value pairs separated by &
      PStringArray tokens = entityBody.Tokenise("&=", TRUE);
      for (PINDEX i = 0; i < tokens.GetSize(); i += 2) {
        PCaselessString key = tokens[i];
        if (postData.GetAt(key) != NULL) 
          postData.SetAt(key, postData[key] + "," + tokens[i+1]);
        else
          postData.SetAt(key, tokens[i+1]);
      }
      OnPOST(url, mimeInfo, postData);
      break;
  }

  return TRUE;
}


PString PHTTPSocket::GetServerName() const
{
  return "PWLib-HTTP-Server/1.0 PWLib/1.0";
}


void PHTTPSocket::OnGET(const PURL & url, const PMIMEInfo & info)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnGET(*this, url, info);
}


void PHTTPSocket::OnHEAD(const PURL & url, const PMIMEInfo & info)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnHEAD(*this, url, info);
}


void PHTTPSocket::OnPOST(const PURL & url,
                         const PMIMEInfo & info,
                         const PStringToString & data)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnPOST(*this, url, info, data);
}


BOOL PHTTPSocket::OnUnknown(const PCaselessString & command)
{
  OnError(BadRequest, command);
  return FALSE;
}


static struct httpStatusCodeStruct {
  char *  text;
  int     code;
  BOOL    allowedBody;
} httpStatusDefn[PHTTPSocket::NumStatusCodes] = {
  { "Information",           100, 0 },
  { "OK",                    200, 1 },
  { "Created",               201, 1 },
  { "Accepted",              202, 1 },
  { "No Content",            204, 0 },
  { "Moved Permanently",     301, 1 },
  { "Moved Temporarily",     302, 1 },
  { "Not Modified",          304, 0 },
  { "Bad Request",           400, 1 },
  { "Unauthorised",          401, 0 },
  { "Forbidden",             403, 1 },
  { "Not Found",             404, 1 },
  { "Internal Server Error", 500, 1 },
  { "Not Implemented",       501, 1 },
  { "Bad Gateway",           502, 1 },
};

const char RFC1123TimeFormat[] = "www, dd MMM yyyy hh:mm:ssg";

void PHTTPSocket::StartResponse(StatusCode code,
                                PMIMEInfo & headers,
                                PINDEX bodySize)
{
  if (majorVersion < 1)
    return;

  httpStatusCodeStruct * statusInfo = httpStatusDefn+code;
  *this << "HTTP/" << majorVersion << '.' << minorVersion << ' '
        << statusInfo->code <<  statusInfo->text << "\r\n";

  headers.SetAt(ContentLengthStr, PString(PString::Unsigned, bodySize));
  headers.Write(*this);
}


void PHTTPSocket::SetDefaultMIMEInfo(PMIMEInfo & info)
{
  PTime now;
  info.SetAt("Date", now.AsString(RFC1123TimeFormat)+" GMT");
  info.SetAt("MIME-Version", "1.0");
  info.SetAt("Server", GetServerName());
}


void PHTTPSocket::OnError(StatusCode code, const PString & extra)
{
  httpStatusCodeStruct * statusInfo = httpStatusDefn+code;

  PMIMEInfo headers;
  SetDefaultMIMEInfo(headers);

  if (!statusInfo->allowedBody) {
    StartResponse(code, headers, 0);
    return;
  }

#if !defined(_MSC_VER) || _MSC_VER > 800
  PHTML reply;
  reply << PHTML::Title()
        << statusInfo->code
        << " "
        << statusInfo->text
        << PHTML::Body()
        << PHTML::Heading(1)
        << statusInfo->code
        << " "
        << statusInfo->text
        << PHTML::Heading(1)
        << extra
        << PHTML::Body();
#else
  PStringStream reply;
  reply << "<TITLE>"
        << statusInfo->code
        << " "
        << statusInfo->text
        << "</TITLE><BODY><H1>"
        << statusInfo->code
        << " "
        << statusInfo->text
        << "</H1>"
        << extra
        << "</BODY>";
#endif

  headers.SetAt(ContentTypeStr, "text/html");
  StartResponse(code, headers, reply.GetLength());
  WriteString(reply);
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
  if (authInfo(0, 5) == PCaselessString("Basic "))
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
// PHTTPResource

PHTTPResource::PHTTPResource(const PURL & url, const PString & type)
  : baseURL(url), contentType(type)
{
  authority = NULL;
}


PHTTPResource::PHTTPResource(const PURL & url,
                             const PString & type,
                             const PHTTPAuthority & auth)
  : baseURL(url), contentType(type)
{
  authority = (PHTTPAuthority *)auth.Clone();
}


PHTTPResource::~PHTTPResource()
{
  delete authority;
}


void PHTTPResource::OnGET(PHTTPSocket & socket,
                          const PURL & url,
                          const PMIMEInfo & info)
{
  if (CheckAuthority(socket, info)) {
    PString modSinceDate = info.GetString("If-Modified-Since", "");
    if (!modSinceDate.IsEmpty() && IsModifiedSince(PTime(modSinceDate)))
      socket.OnError(PHTTPSocket::NotModified, url.AsString());
    else {
      PString data;
      PMIMEInfo outMIME;
      socket.SetDefaultMIMEInfo(outMIME);
      PTime expiryDate;
      if (GetExpirationDate(expiryDate))
        outMIME.SetAt("Expires", expiryDate.AsString(RFC1123TimeFormat));
      outMIME.SetAt(ContentTypeStr, contentType);
      PHTTPSocket::StatusCode code = OnLoadData(url, info, data, outMIME);
      if (code != PHTTPSocket::OK)
        socket.OnError(code, url.AsString());
      else {
        socket.StartResponse(code, outMIME, data.GetSize());
        socket.Write((const char *)data, data.GetSize());
      }
    }
  }
}


void PHTTPResource::OnHEAD(PHTTPSocket & socket,
                                const PURL & url,
                                const PMIMEInfo & info)
{
  if (CheckAuthority(socket, info)) {
    PString data;
    PMIMEInfo outMIME;
    socket.SetDefaultMIMEInfo(outMIME);
    PTime expiryDate;
    if (GetExpirationDate(expiryDate))
      outMIME.SetAt("Expires", expiryDate.AsString(RFC1123TimeFormat));
    outMIME.SetAt(ContentTypeStr, contentType);
    PHTTPSocket::StatusCode code = OnLoadHead(url, info, data, outMIME);
    if (code != PHTTPSocket::OK)
      socket.OnError(code, url.AsString());
    else {
      socket.StartResponse(code, outMIME, data.GetSize());
      socket.Write((const char *)data, data.GetSize());
    }
  }
}


void PHTTPResource::OnPOST(PHTTPSocket & socket,
                                const PURL & url,
                                const PMIMEInfo & info,
                                const PStringToString & data)
{
  if (CheckAuthority(socket, info)) {
    Post(url, info, data);
    socket.OnError(PHTTPSocket::OK, "");
  }
}


BOOL PHTTPResource::CheckAuthority(PHTTPSocket & socket,
                                   const PMIMEInfo & info)
{
  if (authority == NULL)
    return TRUE;

  // if this is an authorisation request...
  PString authInfo = info.GetString("Authorization", "");
  if (authInfo.IsEmpty()) {
    // it must be a request for authorisation
    PMIMEInfo reply;
    socket.SetDefaultMIMEInfo(reply);
    reply.SetAt("WWW-Authenticate",
                              "Basic realm=\"" + authority->GetRealm() + "\"");
    socket.StartResponse(PHTTPSocket::UnAuthorised, reply, 0);
    return FALSE;
  }

  if (authority->Validate(authInfo))
    return TRUE;

  socket.OnError(PHTTPSocket::Forbidden, "");
  return FALSE;
}


BOOL PHTTPResource::IsModifiedSince(const PTime &)
{
  return TRUE;
}


BOOL PHTTPResource::GetExpirationDate(PTime &)
{
  return FALSE;
}


PHTTPSocket::StatusCode PHTTPResource::OnLoadHead(const PURL & url,
                                                  const PMIMEInfo & inMIME,
                                                  PString & data,
                                                  PMIMEInfo & outMIME)
{
  return OnLoadData(url, inMIME, data, outMIME);
}


PHTTPSocket::StatusCode PHTTPResource::Post(const PURL &,
                                            const PMIMEInfo &,
                                            const PStringToString &)
{
  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPString

PHTTPString::PHTTPString(const PURL & url)
  : PHTTPResource(url, "text/html")
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


PHTTPSocket::StatusCode PHTTPString::OnLoadData(const PURL &,
                                                const PMIMEInfo &,
                                                PString & data,
                                                PMIMEInfo &)
{
  data = string;
  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

PHTTPFile::PHTTPFile(const PString & filename)
  : PHTTPResource(filename, ""),
    filePath(filename)
{
  SetContentType(PMIMEInfo::GetContentType(filePath.GetType()));
}


PHTTPFile::PHTTPFile(const PURL & url, const PFilePath & file)
  : PHTTPResource(url, PMIMEInfo::GetContentType(file.GetType())),
    filePath(file)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & file,
                     const PString & type)
  : PHTTPResource(url, type), filePath(file)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & file,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, PMIMEInfo::GetContentType(file.GetType()), auth),
    filePath(file)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & file,
                     const PString & type,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth), filePath(file)
{
}


PHTTPSocket::StatusCode PHTTPFile::OnLoadData(const PURL & url,
                                              const PMIMEInfo &,
                                              PString & data,
                                              PMIMEInfo &)
{
  if (baseURL != url)
    return PHTTPSocket::NotFound;

  PFile file;
  if (!file.Open(filePath, PFile::ReadOnly))
    return PHTTPSocket::NotFound;

  PINDEX count = (PINDEX)file.GetLength();
  file.Read(data.GetPointer(count), count);

  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

PHTTPDirectory::PHTTPDirectory(const PURL & url, const PDirectory & dir)
  : PHTTPResource(url, PString()), basePath(dir)
{
}


PHTTPDirectory::PHTTPDirectory(const PURL & url,
                               const PDirectory & dir,
                               const PHTTPAuthority & auth)
  : PHTTPResource(url, PString(), auth), basePath(dir)
{
}


PHTTPSocket::StatusCode PHTTPDirectory::OnLoadData(const PURL & url,
                                                   const PMIMEInfo &,
                                                   PString & data,
                                                   PMIMEInfo & outMIME)
{
  const PStringArray & path = url.GetPath();
  PFilePath realPath = basePath;
  for (PINDEX i = baseURL.GetPath().GetSize(); i < path.GetSize()-1; i++)
    realPath += path[i] + PDIR_SEPARATOR;

  if (i < path.GetSize())
    realPath += path[i];

  // See if its a normal file
  PFileInfo info;
  if (!PFile::GetInfo(realPath, info))
    return PHTTPSocket::NotFound;

  // Noew try and open it
  PFile file;
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(realPath, PFile::ReadOnly))
      return PHTTPSocket::NotFound;
  }
  else {
    for (i = 0; i < PARRAYSIZE(HTMLIndexFiles); i++)
      if (file.Open(realPath +
                          PDIR_SEPARATOR + HTMLIndexFiles[i], PFile::ReadOnly))
        break;
  }

  if (file.IsOpen()) {
    PINDEX count = (PINDEX)file.GetLength();
    file.Read(data.GetPointer(count), count);
    outMIME.SetAt(ContentTypeStr,
                      PMIMEInfo::GetContentType(file.GetFilePath().GetType()));

  }
#if _MSC_VER > 800
  else {
    outMIME.SetAt(ContentTypeStr, "text/html");
    PHTML reply = "Directory of " + url.AsString();
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
              << PHTML::Anchor(realPath.GetFileName()+'/'+dir.GetEntryName())
              << dir.GetEntryName()
              << PHTML::Anchor()
              << PHTML::BreakLine();
      } while (dir.Next());
    }
    reply << PHTML::Body();
    data = reply;
  }
#endif

  return PHTTPSocket::OK;
}



//////////////////////////////////////////////////////////////////////////////
// PHTTPPassThrough

PHTTPPassThrough::PHTTPPassThrough(const PURL & localURL,
                                   const PURL & remURL)
  : PHTTPResource(localURL, PString()), remoteURL(remURL)
{
}


void PHTTPPassThrough::OnGET(PHTTPSocket & socket,
                             const PURL & url,
                             const PMIMEInfo & info)
{
  const PStringArray & path = url.GetPath();
  PString newURL = remoteURL.AsString();
  for (PINDEX i = baseURL.GetPath().GetSize(); i < path.GetSize()-1; i++)
    newURL += '/' + path[i];

  // More to come
}


// End Of File ///////////////////////////////////////////////////////////////
