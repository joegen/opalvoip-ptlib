/*
 * $Id: http.cxx,v 1.12 1996/02/25 11:14:24 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: http.cxx,v $
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
    url.Delete(0, pos+1);
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
  "GET", "HEAD", "POST"
};

static const PCaselessString ContentLengthStr = "Content-Length";
static const PCaselessString ContentTypeStr = "Content-Type";
static const PCaselessString DateStr = "Date";
static const PCaselessString MIMEVersionStr = "MIME-Version";
static const PCaselessString ServerStr = "Server";
static const PCaselessString ExpiresStr = "Expires";
static const PCaselessString WWWAuthenticateStr = "WWW-Authenticate";
static const PCaselessString IfModifiedSinceStr = "If-Modified-Since";

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
      return OnGET(url, mimeInfo);

    case HEAD :
      return OnHEAD(url, mimeInfo);
  }

  // Must be a POST, break the string into string/value pairs separated by &
  PStringToString postData;
  SplitVars(entityBody, postData);
  return OnPOST(url, mimeInfo, postData);
}


PString PHTTPSocket::GetServerName() const
{
  return "PWLib-HTTP-Server/1.0 PWLib/1.0";
}


BOOL PHTTPSocket::OnGET(const PURL & url, const PMIMEInfo & info)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnGET(*this, url, info);
  return FALSE;
}


BOOL PHTTPSocket::OnHEAD(const PURL & url, const PMIMEInfo & info)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnHEAD(*this, url, info);
  return FALSE;
}


BOOL PHTTPSocket::OnPOST(const PURL & url,
                         const PMIMEInfo & info,
                         const PStringToString & data)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnPOST(*this, url, info, data);
  return FALSE;
}


BOOL PHTTPSocket::OnUnknown(const PCaselessString & command)
{
  return OnError(BadRequest, command);
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
  info.SetAt(DateStr, now.AsString(PTime::RFC1123));
  info.SetAt(MIMEVersionStr, "1.0");
  info.SetAt(ServerStr, GetServerName());
}


BOOL PHTTPSocket::OnError(StatusCode code, const PString & extra)
{
  httpStatusCodeStruct * statusInfo = httpStatusDefn+code;

  PMIMEInfo headers;
  SetDefaultMIMEInfo(headers);

  if (!statusInfo->allowedBody) {
    StartResponse(code, headers, 0);
    return FALSE;
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
  PINDEX len = reply.GetLength();
  StartResponse(code, headers, len);
  Write((const char *)reply, len);
  return FALSE;
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
// PHTTPResource

PHTTPResource::PHTTPResource(const PURL & url)
  : baseURL(url)
{
  authority = NULL;
}


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
  if (!CheckAuthority(socket, info))
    return;

  if (info.Contains(IfModifiedSinceStr) &&
                           !IsModifiedSince(PTime(info[IfModifiedSinceStr]))) {
    socket.OnError(PHTTPSocket::NotModified, url.AsString());
    return;
  }

  PMIMEInfo outMIME;
  socket.SetDefaultMIMEInfo(outMIME);

  PTime expiryDate;
  if (GetExpirationDate(expiryDate))
    outMIME.SetAt(ExpiresStr, expiryDate.AsString(PTime::RFC1123));

  PINDEX size;
  PHTTPSocket::StatusCode code = PHTTPSocket::OK;
  if (!LoadHeaders(url, info, code, outMIME, size)) {
    socket.OnError(code, url.AsString());
    return;
  }

  if (!outMIME.Contains(ContentTypeStr) && !contentType.IsEmpty())
    outMIME.SetAt(ContentTypeStr, contentType);

  socket.StartResponse(code, outMIME, size);
  PCharArray data;
  while (LoadData(data, outMIME, url, info)) {
    socket.Write(data, data.GetSize());
    data.SetSize(0);
  }
  if (data.GetSize() > 0)
    socket.Write(data, data.GetSize());
}


void PHTTPResource::OnHEAD(PHTTPSocket & socket,
                           const PURL & url,
                           const PMIMEInfo & info)
{
  if (!CheckAuthority(socket, info))
    return;

  PMIMEInfo outMIME;
  socket.SetDefaultMIMEInfo(outMIME);

  PTime expiryDate;
  if (GetExpirationDate(expiryDate))
    outMIME.SetAt(ExpiresStr, expiryDate.AsString(PTime::RFC1123));

  PINDEX size;
  PHTTPSocket::StatusCode code = PHTTPSocket::OK;
  if (!LoadHeaders(url, info, code, outMIME, size)) {
    socket.OnError(code, url.AsString());
    return;
  }

  if (!outMIME.Contains(ContentTypeStr) && !contentType.IsEmpty())
    outMIME.SetAt(ContentTypeStr, contentType);
  socket.StartResponse(code, outMIME, size);
}


void PHTTPResource::OnPOST(PHTTPSocket & socket,
                           const PURL & url,
                           const PMIMEInfo & info,
                           const PStringToString & data)
{
  if (CheckAuthority(socket, info)) {
    PStringStream msg;
    socket.OnError(Post(url, info, data, msg), msg);
  }
}


BOOL PHTTPResource::CheckAuthority(PHTTPSocket & socket,
                                   const PMIMEInfo & info)
{
  if (authority == NULL)
    return TRUE;

  // if this is an authorisation request...
  if (info.Contains(PString("Authorization"))) {
    if (authority->Validate(info["Authorization"]))
      return TRUE;

    socket.OnError(PHTTPSocket::Forbidden, "");
  }
  else {
    // it must be a request for authorisation
    PMIMEInfo reply;
    socket.SetDefaultMIMEInfo(reply);
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


BOOL PHTTPResource::LoadData(PCharArray & data,
                              PMIMEInfo & outMIME,
                             const PURL & url,
                        const PMIMEInfo & inMIME)
{
  PString text = LoadText();
  OnLoadedText(text, outMIME, url, inMIME);
  text.SetSize(text.GetLength());  // Lose the trailing '\0'
  data = text;
  return FALSE;
}


PString PHTTPResource::LoadText()
{
  PAssertAlways(PUnimplementedFunction);
  return PString();
}


void PHTTPResource::OnLoadedText(PString &,
                               PMIMEInfo &,
                              const PURL &,
                         const PMIMEInfo &)
{
  // Do nothing
}


PHTTPSocket::StatusCode PHTTPResource::Post(const PURL &,
                                            const PMIMEInfo &,
                                            const PStringToString &,
                                            PStringStream & msg)
{
  msg = "Post to this resource is not implemented!";
  return PHTTPSocket::Forbidden;
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


BOOL PHTTPString::LoadHeaders(const PURL &,
                              const PMIMEInfo &,
                              PHTTPSocket::StatusCode &,
                              PMIMEInfo &,
                              PINDEX & contentSize)
{
  contentSize = string.GetLength();
  return TRUE;
}


PString PHTTPString::LoadText()
{
  return string;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

PHTTPFile::PHTTPFile(const PURL & url)
  : PHTTPResource(url)
{
}


PHTTPFile::PHTTPFile(const PString & filename)
  : PHTTPResource(filename)
{
  file.SetFilePath(filename);
  SetContentType(PMIMEInfo::GetContentType(file.GetFilePath().GetType()));
}


PHTTPFile::PHTTPFile(const PURL & url, const PFilePath & path)
  : PHTTPResource(url)
{
  file.SetFilePath(path);
  SetContentType(PMIMEInfo::GetContentType(file.GetFilePath().GetType()));
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PString & type)
  : PHTTPResource(url, type)
{
  file.SetFilePath(path);
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, PString(), auth)
{
  file.SetFilePath(path);
  SetContentType(PMIMEInfo::GetContentType(file.GetFilePath().GetType()));
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & path,
                     const PString & type,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth)
{
  file.SetFilePath(path);
}


BOOL PHTTPFile::LoadHeaders(const PURL &,
                            const PMIMEInfo &,
                            PHTTPSocket::StatusCode & code,
                            PMIMEInfo &,
                            PINDEX & contentSize)
{
  if (!file.Open(PFile::ReadOnly)) {
    code = PHTTPSocket::NotFound;
    return FALSE;
  }

  contentSize = file.GetLength();
  return TRUE;
}


BOOL PHTTPFile::LoadData(PCharArray & data,
                          PMIMEInfo & outMIME,
                         const PURL & url,
                    const PMIMEInfo & inMIME)
{
  if (contentType(0, 4) == "text/")
    return PHTTPResource::LoadData(data, outMIME, url, inMIME);

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


PString PHTTPFile::LoadText()
{
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
  : PHTTPFile(url), basePath(dir)
{
}


PHTTPDirectory::PHTTPDirectory(const PURL & url,
                               const PDirectory & dir,
                               const PHTTPAuthority & auth)
  : PHTTPFile(url, PString(), auth), basePath(dir)
{
}


BOOL PHTTPDirectory::LoadHeaders(const PURL & url,
                                 const PMIMEInfo &,
                                 PHTTPSocket::StatusCode & code,
                                 PMIMEInfo &,
                                 PINDEX & contentSize)
{
  const PStringArray & path = url.GetPath();
  PFilePath realPath = basePath;
  for (PINDEX i = baseURL.GetPath().GetSize(); i < path.GetSize()-1; i++)
    realPath += path[i] + PDIR_SEPARATOR;

  if (i < path.GetSize())
    realPath += path[i];

  // See if its a normal file
  PFileInfo info;
  if (!PFile::GetInfo(realPath, info)) {
    code = PHTTPSocket::NotFound;
    return FALSE;
  }

  // Now try and open it
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(realPath, PFile::ReadOnly)) {
      code = PHTTPSocket::NotFound;
      return FALSE;
    }
  }
  else {
    for (i = 0; i < PARRAYSIZE(HTMLIndexFiles); i++)
      if (file.Open(realPath +
                          PDIR_SEPARATOR + HTMLIndexFiles[i], PFile::ReadOnly))
        break;
  }

  if (file.IsOpen()) {
    contentType = PMIMEInfo::GetContentType(file.GetFilePath().GetType());
    contentSize = file.GetLength();
    fakeIndex = "";
    return TRUE;
  }

  contentType = "text/html";
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
  fakeIndex = reply;

  return TRUE;
}


PString PHTTPDirectory::LoadText()
{
  if (fakeIndex.IsEmpty())
    return PHTTPFile::LoadText();

  return fakeIndex;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPForm

PHTTPField::PHTTPField(const char * nam, const char * titl)
  : name(nam), title(titl != NULL ? titl : nam)
{
  notInHTML = TRUE;
}

PObject::Comparison PHTTPField::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PHTTPField::Class()), PInvalidCast);
  return name.Compare(((const PHTTPField &)obj).name);
}


BOOL PHTTPField::Validated(const PString &, PStringStream &) const
{
  return TRUE;
}


PHTTPStringField::PHTTPStringField(const char * name,
                                   PINDEX siz,
                                   const char * initVal)
  : PHTTPField(name), value(initVal != NULL ? initVal : "")
{
  size = siz;
}


PHTTPStringField::PHTTPStringField(const char * name,
                                   const char * title,
                                   PINDEX siz,
                                   const char * initVal)
  : PHTTPField(name, title), value(initVal != NULL ? initVal : "")
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
                                       const char * initVal)
  : PHTTPStringField(name, siz, initVal)
{
}


PHTTPPasswordField::PHTTPPasswordField(const char * name,
                                       const char * title,
                                       PINDEX siz,
                                       const char * initVal)
  : PHTTPStringField(name, title, siz, initVal)
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
                                     const char * unit)
  : PHTTPField(nam), units(unit != NULL ? unit : "")
{
  low = lo;
  high = hig;
  value = initVal;
}

PHTTPIntegerField::PHTTPIntegerField(const char * nam,
                                     const char * titl,
                                     int lo, int hig,
                                     int initVal,
                                     const char * unit)
  : PHTTPField(nam, titl), units(unit != NULL ? unit : "")
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


PHTTPBooleanField::PHTTPBooleanField(const char * name, BOOL initVal)
  : PHTTPField(name)
{
  value = initVal;
}

PHTTPBooleanField::PHTTPBooleanField(const char * name,
                                     const char * title,
                                     BOOL initVal)
  : PHTTPField(name, title)
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
                                 PINDEX initVal)
  : PHTTPField(name),
    values(valueArray),
    titles(valueArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const PStringArray & valueArray,
                                 const PStringArray & titleArray,
                                 PINDEX initVal)
  : PHTTPField(name),
    values(valueArray),
    titles(titleArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 PINDEX initVal)
  : PHTTPField(name),
    values(count, valueStrings),
    titles(count, valueStrings),
    value(valueStrings[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 const char * const * titleStrings,
                                 PINDEX initVal)
  : PHTTPField(name),
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


void PHTTPForm::Add(PHTTPField * fld)
{
  PAssertNULL(fld);
  PAssert(!fieldNames[fld->GetName()], "Field already on form!");
  fieldNames += fld->GetName();
  fields.Append(fld);
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
    }
  }
  html << PHTML::Table();
  if (option != InsertIntoForm)
    html << PHTML::Paragraph()
         << "    " << PHTML::SubmitButton("Accept")
         << "    " << PHTML::ResetButton("Reset")
         << PHTML::Form();

  if (option == CompleteHTML) {
    html << PHTML::Body();
    string = html;
  }
}


PHTTPSocket::StatusCode PHTTPForm::Post(const PURL &,
          const PMIMEInfo &, const PStringToString & data, PStringStream & msg)
{
  if (data.GetSize() == 0) {
    msg = "No parameters changed.";
    return PHTTPSocket::NoContent;
  }

  BOOL good = TRUE;
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    const PCaselessString & name = field.GetName();
    if (data.Contains(name) && !field.Validated(data[name], msg))
      good = FALSE;
  }

  if (!good)
    return PHTTPSocket::BadRequest;

  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    const PCaselessString & name = field.GetName();
    if (data.Contains(name))
      field.SetValue(data[name]);
  }

  msg = "Accepted new configuration.";
  return PHTTPSocket::OK;
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


PHTTPSocket::StatusCode PHTTPConfig::Post(const PURL & url,
                                          const PMIMEInfo & info,
                                          const PStringToString & data,
                                          PStringStream & msg)
{
  PHTTPSocket::StatusCode code = PHTTPForm::Post(url, info, data, msg);
  if (code != PHTTPSocket::OK)
    return code;

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

  return PHTTPSocket::OK;
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


// End Of File ///////////////////////////////////////////////////////////////
