/*
 * http.cxx
 *
 * HTTP ancestor class and common classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: http.cxx,v $
 * Revision 1.64  2001/11/09 05:46:14  robertj
 * Removed double slash on sip URL.
 * Fixed extra : if have username but no password.
 * Added h323: scheme
 *
 * Revision 1.63  2001/11/08 00:32:49  robertj
 * Added parsing of ';' based parameter fields into string dictionary if there are multiple parameters, with '=' values.
 *
 * Revision 1.62  2001/10/31 01:33:07  robertj
 * Added extra const for constant HTTP tag name strings.
 *
 * Revision 1.61  2001/10/03 00:26:34  robertj
 * Upgraded client to HTTP/1.1 and for chunked mode entity bodies.
 *
 * Revision 1.60  2001/09/28 00:45:42  robertj
 * Broke out internal static function for unstranslating URL strings.
 *
 * Revision 1.59  2001/07/16 00:43:06  craigs
 * Added ability to parse other transport URLs
 *
 * Revision 1.58  2000/05/02 08:29:07  craigs
 * Removed "memory leaks" caused by brain-dead GNU linker
 *
 * Revision 1.57  1999/05/11 12:24:18  robertj
 * Fixed URL parser so leading blanks are ignored.
 *
 * Revision 1.56  1999/05/04 15:26:01  robertj
 * Improved HTTP/1.1 compatibility (pass through user commands).
 * Fixed problems with quicktime installer.
 *
 * Revision 1.55  1999/04/21 01:56:13  robertj
 * Fixed problem with escape codes greater that %80
 *
 * Revision 1.54  1999/01/16 12:45:54  robertj
 * Added RTSP schemes to URL's
 *
 * Revision 1.53  1998/11/30 05:38:15  robertj
 * Moved PURL::Open() code to .cxx file to avoid linking unused code.
 *
 * Revision 1.52  1998/11/30 04:51:53  robertj
 * New directory structure
 *
 * Revision 1.51  1998/09/23 06:22:07  robertj
 * Added open source copyright license.
 *
 * Revision 1.50  1998/02/03 10:02:34  robertj
 * Added ability to get scheme, host and port from URL as a string.
 *
 * Revision 1.49  1998/02/03 06:27:26  robertj
 * Fixed URL encoding to be closer to RFC
 *
 * Revision 1.48  1998/01/26 02:49:16  robertj
 * GNU support.
 *
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

#ifdef __GNUC__
#pragma implementation "http.h"
#pragma implementation "url.h"
#endif

#include <ptlib.h>
#include <ptlib/sockets.h>
#include <ptclib/http.h>
#include <ptclib/url.h>

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
#define DEFAULT_RTSP_PORT	554
#define DEFAULT_RTSPU_PORT	554
#define DEFAULT_PROSPERO_PORT	1525
#define	DEFAULT_H323_PORT       1720
#define	DEFAULT_SIP_PORT        5060

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
  { "rtsp",      HostPort, TRUE, DEFAULT_RTSP_PORT },
  { "rtspu",     HostPort, TRUE, DEFAULT_RTSPU_PORT },

  { "ftp",       UserPasswordHostPort, TRUE, DEFAULT_FTP_PORT },
  { "telnet",    UserPasswordHostPort, TRUE, DEFAULT_TELNET_PORT },
  { "file",      HostOnly,             TRUE },
  { "mailto",    Other, FALSE},
  { "news",      Other, FALSE},
  { "h323",      UserPasswordHostPort, FALSE, DEFAULT_H323_PORT },
  { "sip",       UserPasswordHostPort, FALSE, DEFAULT_SIP_PORT },
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

  PString safeChars = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789$-_.+!*'(),";
  switch (type) {
    case LoginTranslation :
      safeChars += ";?&=";
      break;

    case PathTranslation :
      safeChars += ":@&=";
      break;

    case QueryTranslation :
      safeChars += ":@";
  }
  PINDEX pos = (PINDEX)-1;
  while ((pos += 1+strspn(&xlat[pos+1], safeChars)) < xlat.GetLength())
    xlat.Splice(psprintf("%%%02X", (BYTE)xlat[pos]), pos, 1);

  return xlat;
}


PString PURL::UntranslateString(const PString & str, TranslationType type)
{
  PString xlat = str;
  xlat.MakeUnique();

  PINDEX pos;
  if (type == PURL::QueryTranslation) {
    pos = (PINDEX)-1;
    while ((pos = xlat.Find('+', pos+1)) != P_MAX_INDEX)
      xlat[pos] = ' ';
  }

  pos = (PINDEX)-1;
  while ((pos = xlat.Find('%', pos+1)) != P_MAX_INDEX) {
    int digit1 = xlat[pos+1];
    int digit2 = xlat[pos+2];
    if (isxdigit(digit1) && isxdigit(digit2)) {
      xlat[pos] = (char)(
            (isdigit(digit2) ? (digit2-'0') : (toupper(digit2)-'A'+10)) +
           ((isdigit(digit1) ? (digit1-'0') : (toupper(digit1)-'A'+10)) << 4));
      xlat.Delete(pos+1, 2);
    }
  }

  return xlat;
}


static void SplitVars(const PString & str, PStringToString & vars, char sep1, char sep2)
{
  PINDEX sep1prev = 0;
  do {
    PINDEX sep1next = str.Find(sep1, sep1prev);
    if (sep1next == P_MAX_INDEX)
      sep1next--; // Implicit assumption string is not a couple of gigabytes long ...

    PINDEX sep2pos = str.Find(sep2, sep1prev);
    if (sep2pos > sep1next)
      sep2pos = sep1next;

    PCaselessString key = PURL::UntranslateString(str(sep1prev, sep2pos-1), PURL::QueryTranslation);
    if (!key) {
      PString data = PURL::UntranslateString(str(sep2pos+1, sep1next-1), PURL::QueryTranslation);

      if (vars.Contains(key))
        vars.SetAt(key, vars[key] + ',' + data);
      else
        vars.SetAt(key, data);
    }

    sep1prev = sep1next+1;
  } while (sep1prev != P_MAX_INDEX);
}


void PURL::SplitQueryVars(const PString & queryStr, PStringToString & queryVars)
{
  SplitVars(queryStr, queryVars, '&', '=');
}


void PURL::Parse(const char * cstr)
{
  hostname = PCaselessString();
  pathStr = username = password = parameters = fragment = queryStr = PString();
  path.SetSize(0);
  queryVars.RemoveAll();
  port = 0;

  // copy the string so we can take bits off it
  while (isspace(*cstr))
    cstr++;
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
        url.Delete(0, pos+1);
        break;
      }
    }
  }

  // if there is no scheme, then default to http for the local
  // on the default port
  if (scheme.IsEmpty()) {
    scheme   = "http";
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
      pos = url.FindOneOf("/;?#");
      PString uphp = url.Left(pos);
      if (pos != P_MAX_INDEX)
        url.Delete(0, pos);
      else
        url = PString();

      // if the URL is of type HostOnly, then this is the hostname
      if (schemeInfo.type == HostOnly) {
        hostname = UntranslateString(uphp, LoginTranslation);
      } 

      // if the URL is of type UserPasswordHostPort, then parse it
      if (schemeInfo.type == UserPasswordHostPort) {

        // extract username and password
        PINDEX pos2 = uphp.Find('@');
        if (pos2 != P_MAX_INDEX && pos2 > 0) {
          PINDEX pos3 = uphp.Find(":");
          // if no password...
          if (pos3 > pos2)
            username = UntranslateString(uphp(0, pos2-1), LoginTranslation);
          else {
            username = UntranslateString(uphp(0, pos3-1), LoginTranslation);
            password = UntranslateString(uphp(pos3+1, pos2-1), LoginTranslation);
          }
          uphp.Delete(0, pos2+1);
        }
      }

      // determine if the URL has a port number
      if (schemeInfo.type == HostPort ||
          schemeInfo.type == UserPasswordHostPort) {
        pos = uphp.Find(":");
        if (pos == P_MAX_INDEX) {
          hostname = UntranslateString(uphp, LoginTranslation);
          port = schemeInfo.defaultPort;
        } else {
          hostname = UntranslateString(uphp.Left(pos), LoginTranslation);
          port = (WORD)uphp(pos+1, P_MAX_INDEX).AsInteger();
        }
        if (hostname.IsEmpty())
          hostname = PIPSocket::GetHostName();
      }
    }
  }

  // chop off any trailing query
  pos = url.Find('?');
  if (pos != P_MAX_INDEX /* && pos > 0 */) {
    queryStr = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
    SplitQueryVars(queryStr, queryVars);
  }

  // chop off any trailing parameters
  pos = url.Find(';');
  if (pos != P_MAX_INDEX /* && pos > 0 */) {
    PString paramStr = url(pos+1, P_MAX_INDEX);
    SplitVars(paramStr, paramVars, ';', '=');
    parameters = UntranslateString(paramStr, PathTranslation);
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing fragment
  pos = url.Find('#');
  if (pos != P_MAX_INDEX /* && pos > 0 */) {
    fragment = UntranslateString(url(pos+1, P_MAX_INDEX), PathTranslation);
    url.Delete(pos, P_MAX_INDEX);
  }

  // the hierarchy is what is left
  pathStr = url;
  path = url.Tokenise("/", TRUE);
  if (path.GetSize() > 0 && path[0].IsEmpty()) 
    path.RemoveAt(0);
  for (pos = 0; pos < path.GetSize(); pos++) {
    path[pos] = UntranslateString(path[pos], PathTranslation);
    if (pos > 0 && path[pos] == ".." && path[pos-1] != "..") {
      path.RemoveAt(pos--);
      path.RemoveAt(pos--);
    }
  }
}


PString PURL::AsString(UrlFormat fmt) const
{
  PStringStream str;

  if (fmt == FullURL || fmt == HostPortOnly) {

    // if the scheme is empty, assume http
    if (!scheme) {
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
          if (!username) {
            str << TranslateString(username, LoginTranslation);
            if (!password)
              str << ':' << TranslateString(password, LoginTranslation);
            str << '@';
          }
        }

        if (schemeInfo.type == HostPort || schemeInfo.type == UserPasswordHostPort) {
          if (hostname.IsEmpty())
            str = PString();
          else {
            str << hostname;
            if (port != schemeInfo.defaultPort)
              str << ':' << port;
          }
        }
      }
    }
    if (fmt == HostPortOnly)
      return str;
  }

  PINDEX count = path.GetSize();
  if (count > 0) {
    str << '/';
    for (PINDEX i = 0; i < count; i++) {
      str << TranslateString(path[i], PathTranslation);
      if (i < count-1)
        str << '/';
    }
  }

  if (fmt == FullURL || fmt == URIOnly) {
    if (!parameters)
      str << ";" << TranslateString(parameters, PathTranslation);

    if (!queryStr)
      str << "?" << queryStr;

    if (!fragment)
      str << "#" << TranslateString(fragment, PathTranslation);
  }

  return str;
}


BOOL PURL::OpenBrowser(const PString & url)
{
#ifdef WIN32
  if ((int)ShellExecute(NULL, "open", url, NULL, NULL, 0) > 32)
    return TRUE;

  MessageBox(NULL, "Unable to open page"&url, PProcess::Current().GetName(), MB_TASKMODAL);
#endif
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTP

static char const * const HTTPCommands[PHTTP::NumCommands] = {
  // HTTP 1.0 commands
  "GET", "HEAD", "POST",

  // HTTP 1.1 commands
  "PUT",  "DELETE", "TRACE", "OPTIONS",

  // HTTPS command
  "CONNECT"
};


const char * const PHTTP::AllowTag           = "Allow";
const char * const PHTTP::AuthorizationTag   = "Authorization";
const char * const PHTTP::ContentEncodingTag = "Content-Encoding";
const char * const PHTTP::ContentLengthTag   = "Content-Length";
const char * const PHTTP::ContentTypeTag     = "Content-Type";
const char * const PHTTP::DateTag            = "Date";
const char * const PHTTP::ExpiresTag         = "Expires";
const char * const PHTTP::FromTag            = "From";
const char * const PHTTP::IfModifiedSinceTag = "If-Modified-Since";
const char * const PHTTP::LastModifiedTag    = "Last-Modified";
const char * const PHTTP::LocationTag        = "Location";
const char * const PHTTP::PragmaTag          = "Pragma";
const char * const PHTTP::PragmaNoCacheTag   = "no-cache";
const char * const PHTTP::RefererTag         = "Referer";
const char * const PHTTP::ServerTag          = "Server";
const char * const PHTTP::UserAgentTag       = "User-Agent";
const char * const PHTTP::WWWAuthenticateTag = "WWW-Authenticate";
const char * const PHTTP::MIMEVersionTag     = "MIME-Version";
const char * const PHTTP::ConnectionTag      = "Connection";
const char * const PHTTP::KeepAliveTag       = "Keep-Alive";
const char * const PHTTP::TransferEncodingTag= "Transfer-Encoding";
const char * const PHTTP::ChunkedTag         = "chunked";
const char * const PHTTP::ProxyConnectionTag = "Proxy-Connection";
const char * const PHTTP::ProxyAuthorizationTag = "Proxy-Authorization";
const char * const PHTTP::ProxyAuthenticateTag = "Proxy-Authenticate";
const char * const PHTTP::ForwardedTag       = "Forwarded";
const char * const PHTTP::SetCookieTag       = "Set-Cookie";
const char * const PHTTP::CookieTag          = "Cookie";



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
