/*
 * $Id: http.cxx,v 1.1 1996/01/23 13:04:32 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: http.cxx,v $
 * Revision 1.1  1996/01/23 13:04:32  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <http.h>


//////////////////////////////////////////////////////////////////////////////
// PURL

PURL::PURL()
  : hostname("localhost")
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
  const PURL & other = (const PURL &)obj;
  Comparison c = scheme.Compare(other.scheme);
  if (c == EqualTo) {
    c = username.Compare(other.username);
    if (c == EqualTo) {
      c = password.Compare(other.password);
      if (c == EqualTo) {
        c = hostname.Compare(other.hostname);
        if (c == EqualTo) {
          c = hierarchy.Compare(other.hierarchy);
          if (c == EqualTo) {
            c = parameters.Compare(other.parameters);
            if (c == EqualTo) {
              c = fragment.Compare(other.fragment);
              if (c == EqualTo)
                c = query.Compare(other.query);
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
  username = password = parameters = fragment = query = PString();
  hierarchy.SetSize(0);
  port = 80;

  // copy the string so we can take bits off it
  PString url = cstr;

  static PString reservedChars = "=;/#?";
  PINDEX pos;

  while ((pos = url.Find('%')) != P_MAX_INDEX) {
    int c = 0;
    PINDEX i = pos+1;
    while (isdigit(url[i]))
      c = c * 10 + url[i++];
    url[pos] = (char)c;
    if (i > pos+1)
      url.Delete(pos+1, i-pos-1);
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
    query = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing parameters
  pos = url.FindLast(';');
  if (pos != P_MAX_INDEX && pos > 0) {
    parameters = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
  }

  // the hierarchy is what is left
  for (pos = 0; url[pos] == '/'; pos++)
    ;
  if (pos > 0)
    url.Delete(0, pos);
  hierarchy = url.Tokenise('/', FALSE);
  for (pos = 1; pos < hierarchy.GetSize(); pos++) {
    if (pos > 0 && hierarchy[pos] == "..") {
      hierarchy.RemoveAt(pos--);
      hierarchy.RemoveAt(pos--);
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
      str << '//';
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

  PINDEX count = hierarchy.GetSize();
  for (PINDEX i = 0; i < count; i++) {
    str << hierarchy[i];
    if (i < count-1)
      str << '/';
  }

  if (fmt == FullURL) {
    if (!parameters.IsEmpty())
      str << ";" << parameters;

    if (!query.IsEmpty())
      str << "?" << query;

    if (!fragment.IsEmpty())
      str << "#" << fragment;
  }

  return str;
}


//////////////////////////////////////////////////////////////////////////////
// PHTSimpleAuth

PHTSimpleAuth::PHTSimpleAuth(const PString & realm_,
                               const PString & username_,
                               const PString & password_)
  : realm(realm_), username(username_), password(password_)
{
  PAssert(!realm.IsEmpty(), "Must have a realm!");
}


PObject * PHTSimpleAuth::Clone() const
{
  return PNEW PHTSimpleAuth(realm, username, password);
}


PString PHTSimpleAuth::GetRealm() const
{
  return realm;
}


BOOL PHTSimpleAuth::Validate(const PString & authInfo) const
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
  return name.Compare(((const PHTTPSpace &)obj).name);
}


BOOL PHTTPSpace::AddResource(PHTTPResource * res)
{
  PAssert(res != NULL, PInvalidParameter);
  const PStringArray & path = res->GetURL().GetHierarchy();
  PHTTPSpace * node = this;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      pos = node->children.Append(PNEW PHTTPSpace(path[i]));
    node = &children[pos];
    if (node->resource != NULL)
      return FALSE;   // Already a resource in tree in partial path
  }
  if (!node->children.IsEmpty())
    return FALSE;   // Already a resource in tree further down path.
  node->resource = res;
  return TRUE;
}


PHTTPResource * PHTTPSpace::FindResource(const PURL & url)
{
  const PStringArray & path = url.GetHierarchy();
  PHTTPSpace * node = this;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      return NULL;
    node = &children[pos];
    if (node->resource != NULL)
      return node->resource;
  }
  return NULL;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSocket

static char const * HTTPCommands[PHTTPSocket::NumCommands] = {
  "GET", "HEAD", "POST"
};

static char ContentLengthStr[] = "Content-Length";


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


BOOL PHTTPSocket::PostData(const PString & url, const PostDict & info)
{
  WriteCommand(POST, url);
  for (PINDEX i = 0; i < info.GetSize(); i++)
    WriteString(info.GetKeyAt(i) + " = " + info.GetDataAt(i));
  return FALSE;
}


PHTTPSocket::PostDict::PostDict(const PString & str)
{
  // break the string into string/value pairs separated by &
  PStringArray tokens = str.Tokenise("&=", TRUE);
  for (PINDEX i = 0; i < tokens.GetSize(); i += 2) {
    if (GetAt((PCaselessString)tokens[i]) != NULL) 
      SetAt(tokens[i], (*this)[tokens[i]] + "," + tokens[i+1]);
    else
      SetAt(tokens[i], tokens[i+1]);
  }
}


BOOL PHTTPSocket::ProcessCommand()
{
  PString args;
  PINDEX cmd = ReadCommand(args);
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
    majorVersion = verStr(5, dotPos-1).AsInteger();
    minorVersion = verStr(dotPos+1, P_MAX_INDEX).AsInteger();
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
      entityBody = ReadString(contentLength);
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
      OnPOST(url, mimeInfo, PostDict(entityBody));
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
                         const PostDict & data)
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
  { "Unauthorised",          401, 1 },
  { "Forbidden",             403, 1 },
  { "Not Found",             404, 1 },
  { "Internal Server Error", 500, 1 },
  { "Not Implemented",       501, 1 },
  { "Bad Gateway",           502, 1 },
};

void PHTTPSocket::OnError(PHTTPSocket::StatusCode code, const PString & reasonPhrase)
{
  httpStatusCodeStruct * statusInfo = httpStatusDefn+code;

  PMIMEInfo headers;
  PStringStream entityBody;
  if (statusInfo->allowedBody) {
    headers.SetAt("Content-Type", "text/html");
    entityBody << "<HEAD><TITLE>"
               << statusInfo->code
               << " "
               << statusInfo->text
               << "</TITLE></HEAD>\r\n"
               << "<BODY><H1>"
               << statusInfo->code
               << " "
               << statusInfo->text
               << "</H1>"
               << reasonPhrase << "\r\n"
               << "</BODY>\r\n";
  }

  SendResponse(code, headers, entityBody);
}


void PHTTPSocket::SendResponse(PHTTPSocket::StatusCode code, 
                               PMIMEInfo & headers,
                               const PString & entityBody)
{
  httpStatusCodeStruct * statusInfo = httpStatusDefn+code;

  if (majorVersion >= 1) {
    *this << "HTTP/" << majorVersion << '.' << minorVersion << ' '
          << statusInfo->code <<  statusInfo->text << "\r\n";

    PTime now;
    headers.SetAt("Date", now.AsString("www, dd MMM yyyy hh:mm:ssg")+" GMT");
    headers.SetAt("MIME-Version", "1.0");
    headers.SetAt("Server", GetServerName());
    headers.SetAt(ContentLengthStr,
                           PString(PString::Unsigned, entityBody.GetLength()));
    headers.Write(*this);
  }

  WriteString(entityBody);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPResource

PHTTPResource::PHTTPResource(const PURL & url)
  : baseURL(url)
{
  authority = NULL;
}


PHTTPResource::PHTTPResource(const PURL & url,
                                       const PHTTPAuthority & auth)
  : baseURL(url)
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
    PCharArray data;
    PMIMEInfo reply;
    PHTTPSocket::StatusCode code = GetData(url, info, data, reply);
    if (code != PHTTPSocket::OK)
      socket.OnError(code, url.AsString());
    else {
      socket.SendResponse(code, reply, "");
      socket.Write(data, data.GetSize());
    }
  }
}


void PHTTPResource::OnHEAD(PHTTPSocket & socket,
                                const PURL & url,
                                const PMIMEInfo & info)
{
  if (CheckAuthority(socket, info)) {
    PMIMEInfo reply;
    PCharArray data;
    PHTTPSocket::StatusCode code = GetHead(url, info, data, reply);
    if (code != PHTTPSocket::OK)
      socket.OnError(code, url.AsString());
    else {
      socket.SendResponse(code, reply, "");
      socket.Write(data, data.GetSize());
    }
  }
}


void PHTTPResource::OnPOST(PHTTPSocket & socket,
                                const PURL & url,
                                const PMIMEInfo & info,
                                const PHTTPSocket::PostDict & data)
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
    reply.SetAt("WWW-Authenticate",
                              "Basic realm=\"" + authority->GetRealm() + "\"");
    socket.SendResponse(PHTTPSocket::UnAuthorised, reply, "");
    return FALSE;
  }

  if (authority->Validate(authInfo))
    return TRUE;

  PMIMEInfo reply;
  socket.SendResponse(PHTTPSocket::Forbidden, reply, "");
  return FALSE;
}


PHTTPSocket::StatusCode PHTTPResource::GetHead(const PURL & url,
                                               const PMIMEInfo & inMIME,
                                               PCharArray & data,
                                               PMIMEInfo & outMIME)
{
  return GetData(url, inMIME, data, outMIME);
}


PHTTPSocket::StatusCode PHTTPResource::Post(const PURL &,
                                            const PMIMEInfo &,
                                            const PHTTPSocket::PostDict &)
{
  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

PHTTPFile::PHTTPFile(const PURL & url, const PFilePath & file)
  : PHTTPResource(url), path(file)
{
}


PHTTPSocket::StatusCode PHTTPFile::GetData(const PURL & url,
                                           const PMIMEInfo &,
                                           PCharArray & data,
                                           PMIMEInfo & outMIME)
{
  if (baseURL != url)
    return PHTTPSocket::NotFound;

  PFile file;
  if (!file.Open(path, PFile::ReadOnly))
    return PHTTPSocket::NotFound;

  PINDEX count = file.GetLength();
  file.Read(data.GetPointer(count), count);

  outMIME.SetAt("Content-Type", "text/html");

  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

PHTTPDirectory::PHTTPDirectory(const PURL & url, const PDirectory & dir)
  : PHTTPResource(url), path(dir)
{
}


PHTTPSocket::StatusCode PHTTPDirectory::GetData(const PURL & url,
                                                const PMIMEInfo &,
                                                PCharArray & data,
                                                PMIMEInfo & outMIME)
{
  const PStringArray & hierarchy = url.GetHierarchy();
  PFilePath newPath = path;
  for (PINDEX i = baseURL.GetHierarchy().GetSize();
                                                i < hierarchy.GetSize()-1; i++)
    newPath += hierarchy[i] + PDIR_SEPARATOR;

  if (i < hierarchy.GetSize())
    newPath += hierarchy[i];

  // See if its a normal file
  PFileInfo info;
  if (!PFile::GetInfo(newPath, info))
    return PHTTPSocket::NotFound;

  // Noew try and open it
  PFile file;
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(newPath, PFile::ReadOnly))
      return PHTTPSocket::NotFound;
  }
  else {
    static const char * const IndexFiles[] = {
      "Welcome.html", "welcome.html", "index.html"
    };
    for (i = 0; i < PARRAYSIZE(IndexFiles); i++)
      if (file.Open(newPath + PDIR_SEPARATOR + IndexFiles[i], PFile::ReadOnly))
        break;
  }

  if (file.IsOpen()) {
    PINDEX count = file.GetLength();
    file.Read(data.GetPointer(count), count);
  }

  outMIME.SetAt("Content-Type", "text/html");

  return PHTTPSocket::OK;
}


// End Of File ///////////////////////////////////////////////////////////////
