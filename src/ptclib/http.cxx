/*
 * $Id: http.cxx,v 1.47 1997/11/10 12:40:20 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: http.cxx,v $
 * Revision 1.47  1997/11/10 12:40:20  robertj
 * Fixed illegal character set for URL's.
 *
 * Revision 1.46  1997/07/14 11:47:10  robertj
 * Added "const" to numerous variables.
 *
 * Revision 1.45  1997/07/12 09:45:01  robertj
 * Fixed bug when URL has + sign in somthing other than parameters.
 *
 * Revision 1.44  1997/06/06 08:54:47  robertj
 * Allowed username/password on http scheme URL.
 *
 * Revision 1.43  1997/04/06 07:46:09  robertj
 * Fixed bug where URL has more than special character ('?', '#' etc).
 *
 * Revision 1.42  1997/03/28 04:40:24  robertj
 * Added tags for cookies.
 *
 * Revision 1.41  1997/03/18 22:03:44  robertj
 * Fixed bug that incorrectly parses URL with double slashes.
 *
 * Revision 1.40  1997/02/14 13:55:44  robertj
 * Fixed bug in URL for reproducing fields with special characters, must be escaped and weren't.
 *
 * Revision 1.39  1997/01/12 04:15:21  robertj
 * Globalised MIME tag strings.
 *
 * Revision 1.38  1996/09/14 13:09:28  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.37  1996/08/25 09:37:41  robertj
 * Added function to detect "local" host name.
 * Fixed printing of trailing '/' in empty URL, is distinction between with and without.
 *
 * Revision 1.36  1996/08/22 13:22:26  robertj
 * Fixed bug in print of URLs, extra @ signs.
 *
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
#include <sockets.h>
#include <http.h>
#include <ctype.h>


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
    const char * name;
    int  type;
    BOOL hasDoubleSlash;
    WORD defaultPort;
};

static schemeStruct const schemeInfo[] = {
  { "http",      UserPasswordHostPort, TRUE, DEFAULT_HTTP_PORT },
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

static schemeStruct const defaultSchemeInfo = { "other", Other, FALSE};

static const schemeStruct & GetSchemeInfo(const PString & scheme)
{
  PINDEX i;
  for (i = 0; schemeInfo[i].name != NULL; i++)
    if (scheme *= schemeInfo[i].name)
      return schemeInfo[i];
  return defaultSchemeInfo;
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


PINDEX PURL::HashFunction() const
{
  return ((BYTE)toupper(scheme[0]) +
          (BYTE)toupper(username[0]) +
          (BYTE)toupper(password[0]) +
          (BYTE)toupper(hostname[0]) +
          (BYTE)toupper(pathStr[0]) +
          (BYTE)toupper(parameters[0]) +
          (BYTE)toupper(fragment[0]) +
          (BYTE)toupper(queryStr[0]))%41;
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

  if (type == QueryTranslation) {
    PINDEX space = (PINDEX)-1;
    while ((space = xlat.Find(' ', space+1)) != P_MAX_INDEX)
      xlat[space] = '+';
  }

  static const char safeChars[] =
        "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ$-_.!*'(),+";

  PINDEX pos = (PINDEX)-1;
  while ((pos += 1+strspn(&xlat[pos+1], safeChars)) < xlat.GetLength())
    xlat.Splice(psprintf("%%%02X", xlat[pos]), pos, 1);

  return xlat;
}


static void UnmangleString(PString & str, PURL::TranslationType type)
{
  PINDEX pos;
  if (type == PURL::QueryTranslation) {
    pos = (PINDEX)-1;
    while ((pos = str.Find('+', pos+1)) != P_MAX_INDEX)
      str[pos] = ' ';
  }

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
    UnmangleString(key, QueryTranslation);
    PString data = tokens[i+1];
    UnmangleString(data, QueryTranslation);
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

  static const PString reservedChars = "=;/#?";

  // determine if the URL has a scheme
  scheme = "";
  if (isalpha(url[0])) {
    for (pos = 0; url[pos] != '\0' &&
                          reservedChars.Find(url[pos]) == P_MAX_INDEX; pos++) {
      if (url[pos] == ':') {
        scheme = url.Left(pos);
        UnmangleString(scheme, NormalTranslation);
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
    const schemeStruct & schemeInfo = GetSchemeInfo(scheme);
    
    // if the URL should have leading slash, then remove it if it has one
    if (schemeInfo.hasDoubleSlash &&
      url.GetLength() > 2 && url[0] == '/' && url[1] == '/') 
    url.Delete(0, 2);

    // if the rest of the URL isn't a path, then we are finished!
    if (schemeInfo.type == Other) {
      pathStr = url;
      return;
    }

    // parse user/password/host/port
    if (schemeInfo.type == HostPort ||
        schemeInfo.type == UserPasswordHostPort ||
        schemeInfo.type == HostOnly) {
      pos = url.Find('/');
      PString uphp = url.Left(pos);
      if (pos != P_MAX_INDEX)
        url.Delete(0, pos);
      else
        url = PString();

      // if the URL is of type HostOnly, then this is the hostname
      if (schemeInfo.type == HostOnly) {
        hostname = uphp;
        UnmangleString(hostname, NormalTranslation);
      } 

      // if the URL is of type UserPasswordHostPort, then parse it
      if (schemeInfo.type == UserPasswordHostPort) {

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
            UnmangleString(password, NormalTranslation);
          }
          UnmangleString(username, NormalTranslation);
          uphp.Delete(0, pos2+1);
        }
      }

      // determine if the URL has a port number
      if (schemeInfo.type == HostPort ||
          schemeInfo.type == UserPasswordHostPort) {
        pos = uphp.Find(":");
        if (pos == P_MAX_INDEX) {
          hostname = uphp;
          port = schemeInfo.defaultPort;
        } else {
          hostname = uphp.Left(pos);
          port = (WORD)uphp(pos+1, P_MAX_INDEX).AsInteger();
        }
        UnmangleString(hostname, NormalTranslation);
        if (hostname.IsEmpty())
          hostname = PIPSocket::GetHostName();
      }
    }
  }

  // chop off any trailing fragment
  pos = url.Find('#');
  if (pos != P_MAX_INDEX && pos > 0) {
    fragment = url(pos+1, P_MAX_INDEX);
    UnmangleString(fragment, NormalTranslation);
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing query
  pos = url.Find('?');
  if (pos != P_MAX_INDEX && pos > 0) {
    queryStr = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
    SplitQueryVars(queryStr, queryVars);
  }

  // chop off any trailing parameters
  pos = url.Find(';');
  if (pos != P_MAX_INDEX && pos > 0) {
    parameters = url(pos+1, P_MAX_INDEX);
    UnmangleString(parameters, NormalTranslation);
    url.Delete(pos, P_MAX_INDEX);
  }

  // the hierarchy is what is left
  pathStr = url;
  path = url.Tokenise("/", TRUE);
  if (path.GetSize() > 0 && path[0].IsEmpty()) 
    path.RemoveAt(0);
  for (pos = 0; pos < path.GetSize(); pos++) {
    UnmangleString(path[pos], NormalTranslation);
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
      const schemeStruct & schemeInfo = GetSchemeInfo(scheme);

      if (schemeInfo.hasDoubleSlash)
        str << "//";

      if (schemeInfo.type == Other) 
        str << pathStr;
      else {
        if (schemeInfo.type == HostOnly)
          str << hostname;

        if (schemeInfo.type == UserPasswordHostPort) {
          if (!username || !password)
            str << TranslateString(username)
                << ':'
                << TranslateString(password)
                << '@';
        }

        if (schemeInfo.type == HostPort ||
            schemeInfo.type == UserPasswordHostPort) {
          str << hostname;
          if (port != schemeInfo.defaultPort)
            str << ':'
                << port;
        }
      }
    }
  }

  PINDEX count = path.GetSize();
  if (count > 0) {
    str << '/';
    for (PINDEX i = 0; i < count; i++) {
      str << TranslateString(path[i]);
      if (i < count-1)
        str << '/';
    }
  }

  if (fmt == FullURL || fmt == URIOnly) {
    if (!parameters)
      str << ";" << TranslateString(parameters);

    if (!queryStr)
      str << "?" << queryStr;

    if (!fragment)
      str << "#" << TranslateString(fragment);
  }

  return str;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTP

static char const * const HTTPCommands[PHTTP::NumCommands] = {
  // HTTP 1.0 commands
  "GET", "HEAD", "POST",

  // HTTP 1.1 commands
  "PUT",  "PATCH", "COPY",    "MOVE",   "DELETE",
  "LINK", "UNLINK", "TRACE", "WRAPPED", "OPTIONS",
  "CONNECT"
};


const PCaselessString PHTTP::AllowTag           = "Allow";
const PCaselessString PHTTP::AuthorizationTag   = "Authorization";
const PCaselessString PHTTP::ContentEncodingTag = "Content-Encoding";
const PCaselessString PHTTP::ContentLengthTag   = "Content-Length";
const PCaselessString PHTTP::ContentTypeTag     = "Content-Type";
const PCaselessString PHTTP::DateTag            = "Date";
const PCaselessString PHTTP::ExpiresTag         = "Expires";
const PCaselessString PHTTP::FromTag            = "From";
const PCaselessString PHTTP::IfModifiedSinceTag = "If-Modified-Since";
const PCaselessString PHTTP::LastModifiedTag    = "Last-Modified";
const PCaselessString PHTTP::LocationTag        = "Location";
const PCaselessString PHTTP::PragmaTag          = "Pragma";
const PCaselessString PHTTP::PragmaNoCacheTag   = "no-cache";
const PCaselessString PHTTP::RefererTag         = "Referer";
const PCaselessString PHTTP::ServerTag          = "Server";
const PCaselessString PHTTP::UserAgentTag       = "User-Agent";
const PCaselessString PHTTP::WWWAuthenticateTag = "WWW-Authenticate";
const PCaselessString PHTTP::MIMEVersionTag     = "MIME-Version";
const PCaselessString PHTTP::ConnectionTag      = "Connection";
const PCaselessString PHTTP::KeepAliveTag       = "Keep-Alive";
const PCaselessString PHTTP::ProxyConnectionTag = "Proxy-Connection";
const PCaselessString PHTTP::ProxyAuthorizationTag = "Proxy-Authorization";
const PCaselessString PHTTP::ProxyAuthenticateTag = "Proxy-Authenticate";
const PCaselessString PHTTP::ForwardedTag       = "Forwarded";
const PCaselessString PHTTP::SetCookieTag       = "Set-Cookie";
const PCaselessString PHTTP::CookieTag          = "Cookie";



PHTTP::PHTTP()
  : PInternetProtocol("www 80", NumCommands, HTTPCommands)
{
}


PINDEX PHTTP::ParseResponse(const PString & line)
{
  PINDEX endVer = line.Find(' ');
  if (endVer == P_MAX_INDEX) {
    lastResponseInfo = "Bad response";
    lastResponseCode = PHTTP::InternalServerError;
    return 0;
  }

  lastResponseInfo = line.Left(endVer);
  PINDEX endCode = line.Find(' ', endVer+1);
  lastResponseCode = line(endVer+1,endCode-1).AsInteger();
  if (lastResponseCode == 0)
    lastResponseCode = PHTTP::InternalServerError;
  lastResponseInfo &= line.Mid(endCode);
  return 0;
}


// End Of File ///////////////////////////////////////////////////////////////
