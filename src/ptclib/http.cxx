/*
 * $Id: http.cxx,v 1.35 1996/08/19 13:42:40 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: http.cxx,v $
 * Revision 1.35  1996/08/19 13:42:40  robertj
 * Fixed errors in URL parsing and display.
 * Fixed "Forbidden" problem out of HTTP authorisation system.
 * Fixed authorisation so if have no user/password on basic authentication, does not require it.
 *
 * Revision 1.34  1996/07/27 04:13:47  robertj
 * Fixed use of HTTP proxy on non-persistent connections.
 *
 * Revision 1.33  1996/07/15 10:37:20  robertj
 * Improved proxy "self" detection (especially localhost).
 *
 * Revision 1.32  1996/06/28 13:20:24  robertj
 * Modified HTTPAuthority so gets PHTTPReqest (mainly for URL) passed in.
 * Moved HTTP form resource to another compilation module.
 * Fixed memory leak in POST command.
 *
 * Revision 1.31  1996/06/10 10:00:00  robertj
 * Added global function for query parameters parsing.
 *
 * Revision 1.30  1996/06/07 13:52:23  robertj
 * Added PUT to HTTP proxy FTP. Necessitating redisign of entity body processing.
 *
 * Revision 1.29  1996/06/05 12:33:04  robertj
 * Fixed bug in parsing URL with no path, is NOT absolute!
 *
 * Revision 1.28  1996/05/30 10:07:26  robertj
 * Fixed bug in version number checking of return code compatibility.
 *
 * Revision 1.27  1996/05/26 03:46:42  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.26  1996/05/23 10:02:13  robertj
 * Added common function for GET and HEAD commands.
 * Fixed status codes to be the actual status code instead of sequential enum.
 * This fixed some problems with proxy pass through of status codes.
 * Fixed bug in URL parsing of username and passwords.
 *
 * Revision 1.19.1.1  1996/04/17 11:08:22  craigs
 * New version by craig pending confirmation by robert
 *
 * Revision 1.19  1996/04/05 01:46:30  robertj
 * Assured PSocket::Write always writes the number of bytes specified, no longer need write loops.
 * Added workaraound for NT Netscape Navigator bug with persistent connections.
 *
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

// RFC 1738
// http://host:port/path...
// https://host:port/path....
// gopher://host:port
// wais://host:port
// nntp://host:port
// prospero://host:port
// ftp://user:password@host:port/path...
// telnet://user:password@host:port
// file://hostname/path...

// mailto:user@hostname
// news:string

#define DEFAULT_FTP_PORT	21
#define DEFAULT_TELNET_PORT	23
#define DEFAULT_GOPHER_PORT	70
#define DEFAULT_HTTP_PORT	80
#define DEFAULT_NNTP_PORT	119
#define DEFAULT_WAIS_PORT	210
#define DEFAULT_HTTPS_PORT	443
#define DEFAULT_PROSPERO_PORT	1525

enum SchemeFormat {
  HostPort = 1,
  UserPasswordHostPort = 2,
  HostOnly = 3,
  Other = 4
};

class schemeStruct {
  public:
    char * name;
    int    type;
    BOOL   hasDoubleSlash;
    WORD   defaultPort;
};

static schemeStruct schemeInfo[] = {
  { "http",      HostPort, TRUE, DEFAULT_HTTP_PORT },
  { "https",     HostPort, TRUE, DEFAULT_HTTPS_PORT },
  { "gopher",    HostPort, TRUE, DEFAULT_GOPHER_PORT },
  { "wais",      HostPort, TRUE, DEFAULT_WAIS_PORT },
  { "nntp",      HostPort, TRUE, DEFAULT_NNTP_PORT },
  { "prospero",  HostPort, TRUE, DEFAULT_PROSPERO_PORT },

  { "ftp",       UserPasswordHostPort, TRUE, DEFAULT_FTP_PORT },
  { "telnet",    UserPasswordHostPort, TRUE, DEFAULT_TELNET_PORT },
  { "file",      HostOnly,             TRUE },
  { "mailto",    Other, FALSE},
  { "news",      Other, FALSE},
  { NULL }
};

static schemeStruct defaultSchemeInfo = { "other", Other, FALSE};

static schemeStruct * GetSchemeInfo(const PString & scheme)
{
  PINDEX i;
  for (i = 0; schemeInfo[i].name != NULL; i++)
    if (scheme *= schemeInfo[i].name)
      return &schemeInfo[i];
  return &defaultSchemeInfo;
}

//////////////////////////////////////////////////////////////////////////////
// PURL

PURL::PURL()
{
  scheme = "http";
  port   = 0;
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


void PURL::SplitQueryVars(const PString & queryStr, PStringToString & queryVars)
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
  hostname = PCaselessString();
  pathStr = username = password = parameters = fragment = queryStr = PString();
  path.SetSize(0);
  queryVars.RemoveAll();
  port = 0;

  // copy the string so we can take bits off it
  PString url = cstr;

  PINDEX pos;

  static PString reservedChars = "=;/#?";

  // determine if the URL has a scheme
  scheme = "";
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

  // if there is no scheme, then default to http for the local
  // on the default port
  if (scheme.IsEmpty()) {
    scheme   = "http";
    port     = 0;
    hostname = PIPSocket::GetHostName();
    if (url.GetLength() > 2 && url[0] == '/' && url[1] == '/') 
      url.Delete(0, 2);
  } else {

    // get information which tells us how to parse URL for this
    // particular scheme
    schemeStruct * schemeInfo = GetSchemeInfo(scheme);
    
    // if the URL should have leading slash, then remove it if it has one
    if (schemeInfo->hasDoubleSlash &&
      url.GetLength() > 2 && url[0] == '/' && url[1] == '/') 
    url.Delete(0, 2);

    // if the rest of the URL isn't a path, then we are finished!
    if (schemeInfo->type == Other) {
      pathStr = url;
      return;
    }

    // parse user/password/host/port
    if (schemeInfo->type == HostPort ||
        schemeInfo->type == UserPasswordHostPort ||
        schemeInfo->type == HostOnly) {
      pos = url.Find('/');
      PString uphp = url.Left(pos);
      if (pos != P_MAX_INDEX)
        url.Delete(0, pos);
      else
        url = PString();

      // if the URL is of type HostOnly, then this is the hostname
      if (schemeInfo->type == HostOnly) {
        hostname = uphp;
        UnmangleString(hostname);
      } 

      // if the URL is of type UserPasswordHostPort, then parse it
      if (schemeInfo->type == UserPasswordHostPort) {

        // extract username and password
        PINDEX pos2 = uphp.Find('@');
        if (pos2 != P_MAX_INDEX && pos2 > 0) {
          PINDEX pos3 = uphp.Find(":");
          // if no password...
          if (pos3 > pos2)
            username = uphp(0, pos2-1);
          else {
            username = uphp(0, pos3-1);
            password = uphp(pos3+1, pos2-1);
            UnmangleString(password);
          }
          UnmangleString(username);
          uphp.Delete(0, pos2+1);
        }
      }

      // determine if the URL has a port number
      if (schemeInfo->type == HostPort ||
          schemeInfo->type == UserPasswordHostPort) {
        pos = uphp.Find(":");
        if (pos == P_MAX_INDEX) {
          hostname = uphp;
          port = schemeInfo->defaultPort;
        } else {
          hostname = uphp.Left(pos);
          port = (WORD)uphp(pos+1, P_MAX_INDEX).AsInteger();
        }
        UnmangleString(hostname);
        if (hostname.IsEmpty())
          hostname = PIPSocket::GetHostName();
      }
    }
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
    SplitQueryVars(queryStr, queryVars);
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
  path = url.Tokenise("/", FALSE);
  if (path.GetSize() > 0 && path[0].IsEmpty()) 
    path.RemoveAt(0);
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

    // if the scheme is empty, assume http
    if (scheme.IsEmpty())
      str << "http://";
    else {
      str << scheme << ':';
      schemeStruct * schemeInfo = GetSchemeInfo(scheme);

      if (schemeInfo->hasDoubleSlash)
        str << "//";

      if (schemeInfo->type == Other) 
        str << pathStr;
      else {
        if (schemeInfo->type == HostOnly)
          str << hostname;

        if (schemeInfo->type == UserPasswordHostPort) {
          if (!username.IsEmpty() || !password.IsEmpty())
            str << username
                << ':'
                << password;
            str << '@';
        }

        if (schemeInfo->type == HostPort ||
            schemeInfo->type == UserPasswordHostPort) {
          str << hostname;
          if (port != schemeInfo->defaultPort)
            str << ':'
                << port;
        }
      }
    }
  }

  PINDEX count = path.GetSize();
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
static const PCaselessString UserAgentStr       = "User-Agent";

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
  ConstructServer();
}


PHTTPSocket::PHTTPSocket(PSocket & socket, const PHTTPSpace & space)
  : PApplicationSocket(NumCommands, HTTPCommands, socket),
    urlSpace(space)
{
  ConstructServer();
}

void PHTTPSocket::ConstructServer()
{
  transactionCount = 0;
  SetReadLineTimeout(PTimeInterval(0, READLINE_TIMEOUT));
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
  PString http = ReadString(5);
  if (http != "HTTP/") {
    lastResponseCode = PHTTPSocket::OK;
    lastResponseInfo = "HTTP/0.9";
    return TRUE;
  }

  UnRead(http);

  if (PApplicationSocket::ReadResponse())
    return replyMIME.Read(*this);
  return FALSE;
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
  if (endVer == P_MAX_INDEX) {
    lastResponseInfo = "Bad response";
    lastResponseCode = PHTTPSocket::InternalServerError;
    return 0;
  }

  lastResponseInfo = line.Left(endVer);
  PINDEX endCode = line.Find(' ', endVer+1);
  lastResponseCode = line(endVer+1,endCode-1).AsInteger();
  if (lastResponseCode == 0)
    lastResponseCode = PHTTPSocket::InternalServerError;
  lastResponseInfo &= line.Mid(endCode);
  return 0;
}


BOOL PHTTPSocket::ProcessCommand()
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
        contentLength = 0;
        connectInfo.SetPersistance(FALSE);
      }
    }
  }

  connectInfo.SetEntityBodyLength(contentLength);

  // get the user agent for various foul purposes...
  userAgent = mimeInfo(UserAgentStr);

  // the URL that comes with Connect requests is not quite kosher, so 
  // mangle it into a proper URL and do NOT close the connection.
  // for all other commands, close the read connection if not persistant
  PURL url;
  if (cmd == CONNECT) 
    url = "https://" + tokens[0];
  else {
    url = tokens[0];
    if (url.GetPort() == 0)
      url.SetPort(GetPort());
  }

  // If the incoming URL is of a proxy type then call OnProxy() which will
  // probably just go OnError(). Even if a full URL is provided in the
  // command we should check to see if it is a local server request and process
  // it anyway even though we are not a proxy. The usage of GetHostName()
  // below are to catch every way of specifying the host (name, alias, any of
  // several IP numbers etc).
  BOOL doProxy;
  if (url.GetScheme() != "http")
    doProxy = TRUE;
  else if (url.GetPort() == 0 || url.GetPort() == GetPort())
    doProxy = FALSE;
  else if (url.GetHostName().IsEmpty() || (url.GetHostName() *= "localhost"))
    doProxy = FALSE;
  else {
    Address urlAddress;
    if (!PIPSocket::GetHostAddress(url.GetHostName(), urlAddress))
      doProxy = TRUE;
    else if (Address(127,0,0,1) == urlAddress)
      doProxy = FALSE;
    else {
      Address myAddress;
      GetLocalAddress(myAddress);
      if (myAddress == urlAddress)
        doProxy = FALSE;
      else
        doProxy = PIPSocket::GetHostName() !=
                                    PIPSocket::GetHostName(url.GetHostName());
    }
  }

  if (doProxy)
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

  // if the function just indicated that the connection is to persist,
  // and so did the client, then return TRUE. Note that all of the OnXXXX
  // routines above must make sure that their return value is FALSE if
  // if there was no ContentLength field in the response. This ensures that
  // we always close the socket so the client will get the correct end of file
  if (persist &&
      connectInfo.IsPersistant() &&
      transactionCount < MAX_TRANSACTIONS)
    return TRUE;

  // close the output stream now and return FALSE
  Shutdown(ShutdownWrite);
  return FALSE;
}


PString PHTTPSocket::ReadEntityBody(const PHTTPConnectionInfo & connectInfo)
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
  if (!connectInfo.IsPersistant())
    Shutdown(ShutdownRead);

  return entityBody;
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


BOOL PHTTPSocket::OnProxy(Commands cmd,
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
    { "Internal Server Error",         PHTTPSocket::InternalServerError, 1 },
    { "OK",                            PHTTPSocket::OK, 1 },
    { "Unauthorised",                  PHTTPSocket::UnAuthorised, 1 },
    { "Forbidden",                     PHTTPSocket::Forbidden, 1 },
    { "Not Found",                     PHTTPSocket::NotFound, 1 },
    { "Not Modified",                  PHTTPSocket::NotModified },
    { "No Content",                    PHTTPSocket::NoContent },
    { "Bad Gateway",                   PHTTPSocket::BadGateway, 1 },
    { "Bad Request",                   PHTTPSocket::BadRequest, 1 },
    { "Continue",                      PHTTPSocket::Continue, 1, 1, 1 },
    { "Switching Protocols",           PHTTPSocket::SwitchingProtocols, 1, 1, 1 },
    { "Created",                       PHTTPSocket::Created, 1 },
    { "Accepted",                      PHTTPSocket::Accepted, 1 },
    { "Non-Authoritative Information", PHTTPSocket::NonAuthoritativeInformation, 1, 1, 1 },
    { "Reset Content",                 PHTTPSocket::ResetContent, 0, 1, 1 },
    { "Partial Content",               PHTTPSocket::PartialContent, 1, 1, 1 },
    { "Multiple Choices",              PHTTPSocket::MultipleChoices, 1, 1, 1 },
    { "Moved Permanently",             PHTTPSocket::MovedPermanently, 1 },
    { "Moved Temporarily",             PHTTPSocket::MovedTemporarily, 1 },
    { "See Other",                     PHTTPSocket::SeeOther, 1, 1, 1 },
    { "Use Proxy",                     PHTTPSocket::UseProxy, 1, 1, 1 },
    { "Payment Required",              PHTTPSocket::PaymentRequired, 1, 1, 1 },
    { "Method Not Allowed",            PHTTPSocket::MethodNotAllowed, 1, 1, 1 },
    { "None Acceptable",               PHTTPSocket::NoneAcceptable, 1, 1, 1 },
    { "Proxy Authetication Required",  PHTTPSocket::ProxyAuthenticationRequired, 1, 1, 1 },
    { "Request Timeout",               PHTTPSocket::RequestTimeout, 1, 1, 1 },
    { "Conflict",                      PHTTPSocket::Conflict, 1, 1, 1 },
    { "Gone",                          PHTTPSocket::Gone, 1, 1, 1 },
    { "Length Required",               PHTTPSocket::LengthRequired, 1, 1, 1 },
    { "Unless True",                   PHTTPSocket::UnlessTrue, 1, 1, 1 },
    { "Not Implemented",               PHTTPSocket::NotImplemented, 1 },
    { "Service Unavailable",           PHTTPSocket::ServiceUnavailable, 1, 1, 1 },
    { "Gateway Timeout",               PHTTPSocket::GatewayTimeout, 1, 1, 1 }
  };

  // make sure the error code is valid
  for (PINDEX i = 0; i < PARRAYSIZE(httpStatusDefn); i++)
    if (code == httpStatusDefn[i].code)
      return &httpStatusDefn[i];

  return httpStatusDefn;
}


void PHTTPSocket::StartResponse(StatusCode code,
                                PMIMEInfo & headers,
                                long bodySize)
{
  if (majorVersion < 1) 
    return;

  const httpStatusCodeStruct * statusInfo = GetStatusCodeStruct(code);

  // output the command line
  WriteString(psprintf("HTTP/%u.%u %03u %s\r\n",
              majorVersion, minorVersion, statusInfo->code, statusInfo->text));

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


void PHTTPSocket::SetDefaultMIMEInfo(PMIMEInfo & info,
                     const PHTTPConnectionInfo & connectInfo)
{
  PTime now;
  info.SetAt(DateStr, now.AsString(PTime::RFC1123, PTime::GMT));
  info.SetAt(MIMEVersionStr, "1.0");
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


BOOL PHTTPSocket::OnError(StatusCode code,
                     const PString & extra,
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
  PAssert(!realm.IsEmpty(), "Must have a realm!");
}


PObject * PHTTPSimpleAuth::Clone() const
{
  return PNEW PHTTPSimpleAuth(realm, username, password);
}


BOOL PHTTPSimpleAuth::IsActive() const
{
  return !username.IsEmpty() || !password.IsEmpty();
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
  code        = PHTTPSocket::OK;
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
  if (!str.IsEmpty()) {
    PStringArray tokens = str.Tokenise(", ", FALSE);
    isPersistant = tokens.GetStringsIndex(KeepAliveStr) != P_MAX_INDEX;
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


BOOL PHTTPResource::OnGET(PHTTPSocket & socket,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(socket, url, info, connectInfo, TRUE);
}

BOOL PHTTPResource::OnHEAD(PHTTPSocket & socket,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo)
{
  return OnGETOrHEAD(socket, url, info, connectInfo, FALSE);
}

BOOL PHTTPResource::OnGETOrHEAD(PHTTPSocket & socket,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo,
                                   BOOL isGET)
{
  if (isGET && info.Contains(IfModifiedSinceStr) &&
                           !IsModifiedSince(PTime(info[IfModifiedSinceStr]))) 
    return socket.OnError(PHTTPSocket::NotModified, url.AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(url, info);

  BOOL retVal = TRUE;
  if (CheckAuthority(socket, *request, connectInfo)) {
    retVal = FALSE;
    socket.SetDefaultMIMEInfo(request->outMIME, connectInfo);

    PTime expiryDate;
    if (GetExpirationDate(expiryDate))
      request->outMIME.SetAt(ExpiresStr,
                              expiryDate.AsString(PTime::RFC1123, PTime::GMT));

    if (!LoadHeaders(*request)) 
      retVal = socket.OnError(request->code, url.AsString(), connectInfo);
    else if (!isGET)
      retVal = request->outMIME.Contains(ContentLengthStr);
    else {
      hitCount++;
      retVal = OnGETData(socket, url, connectInfo, *request);
    }
  }

  delete request;
  return retVal;
}

BOOL PHTTPResource::OnGETData(PHTTPSocket & socket,
                               const PURL & /*url*/,
                const PHTTPConnectionInfo & /*connectInfo*/,
                             PHTTPRequest & request)
{
  if (!request.outMIME.Contains(ContentTypeStr) && !contentType.IsEmpty())
    request.outMIME.SetAt(ContentTypeStr, contentType);

  PCharArray data;
  if (LoadData(request, data)) {
    socket.StartResponse(request.code, request.outMIME, request.contentSize);
    do {
      socket.Write(data, data.GetSize());
      data.SetSize(0);
    } while (LoadData(request, data));
  }
  else
    socket.StartResponse(request.code, request.outMIME, data.GetSize());

  socket.Write(data, data.GetSize());

  return request.outMIME.Contains(ContentLengthStr);
}

BOOL PHTTPResource::OnPOST(PHTTPSocket & socket,
                            const PURL & url,
                       const PMIMEInfo & info,
                 const PStringToString & data,
             const PHTTPConnectionInfo & connectInfo)
{
  PHTTPRequest * request = CreateRequest(url, info);

  BOOL persist = TRUE;
  if (CheckAuthority(socket, *request, connectInfo)) {
    PHTML msg;
    persist = Post(*request, data, msg);

    if (msg.IsEmpty())
      persist = socket.OnError(request->code, "", connectInfo) && persist;
    else {
      if (msg.Is(PHTML::InBody))
        msg << PHTML::Body();

      request->outMIME.SetAt(ContentTypeStr, "text/html");

      PINDEX len = msg.GetLength();
      socket.StartResponse(request->code, request->outMIME, len);
      persist = socket.Write((const char *)msg, len) && persist;
    }
  }

  delete request;
  return persist;
}


BOOL PHTTPResource::CheckAuthority(PHTTPSocket & socket,
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
  socket.SetDefaultMIMEInfo(headers, connectInfo);
  headers.SetAt(WWWAuthenticateStr,
                       "Basic realm=\"" + authority->GetRealm(request) + "\"");
  headers.SetAt(ContentTypeStr, "text/html");

  const httpStatusCodeStruct * statusInfo =
                               GetStatusCodeStruct(PHTTPSocket::UnAuthorised);

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

  socket.StartResponse(PHTTPSocket::UnAuthorised, headers, reply.GetLength());
  socket.WriteString(reply);

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
  PINDEX i;
  for (i = baseURL.GetPath().GetSize(); i < path.GetSize()-1; i++)
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


// End Of File ///////////////////////////////////////////////////////////////
