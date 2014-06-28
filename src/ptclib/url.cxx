/*
 * url.cxx
 *
 * URL parsing classes.
 *
 * Portable Tools Library
 *
 * Copyright (c) 1993-2008 Equivalence Pty. Ltd.
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

#ifdef __GNUC__
#pragma implementation "url.h"
#endif

#include <ptlib.h>

#if P_URL

#include <ptclib/url.h>

#include <ptlib/sockets.h>
#include <ptclib/cypher.h>
#include <ctype.h>

#if defined(_WIN32) && !defined(_WIN32_WCE)
#include <shellapi.h>
#ifdef _MSC_VER
#pragma comment(lib,"shell32.lib")
#endif
#endif


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

#define DEFAULT_FTP_PORT      21
#define DEFAULT_TELNET_PORT   23
#define DEFAULT_GOPHER_PORT   70
#define DEFAULT_HTTP_PORT     80
#define DEFAULT_NNTP_PORT     119
#define DEFAULT_WAIS_PORT     210
#define DEFAULT_HTTPS_PORT    443
#define DEFAULT_RTSP_PORT     554
#define DEFAULT_RTSPU_PORT    554
#define DEFAULT_PROSPERO_PORT 1525
#define DEFAULT_H323_PORT     1720
#define DEFAULT_H323S_PORT    1300
#define DEFAULT_H323RAS_PORT  1719
#define DEFAULT_MSRP_PORT     2855
#define DEFAULT_RTMP_PORT     1935
#define DEFAULT_SIP_PORT      5060
#define DEFAULT_SIPS_PORT     5061


//                 schemeName,user,  passwd,host,  defUser,defhost,query, params,frags, path,  rel,   port
PURL_LEGACY_SCHEME(http,      true,  true,  true,  false,  true,   true,  true,  true,  true,  true,  DEFAULT_HTTP_PORT )
PURL_LEGACY_SCHEME(file,      false, false, true,  false,  true,   false, false, false, true,  false, 0)
PURL_LEGACY_SCHEME(https,     false, false, true,  false,  true,   true,  true,  true,  true,  true,  DEFAULT_HTTPS_PORT)
PURL_LEGACY_SCHEME(gopher,    false, false, true,  false,  true,   false, false, false, true,  false, DEFAULT_GOPHER_PORT)
PURL_LEGACY_SCHEME(wais,      false, false, true,  false,  false,  false, false, false, true,  false, DEFAULT_WAIS_PORT)
PURL_LEGACY_SCHEME(nntp,      false, false, true,  false,  true,   false, false, false, true,  false, DEFAULT_NNTP_PORT)
PURL_LEGACY_SCHEME(prospero,  false, false, true,  false,  true,   false, false, false, true,  false, DEFAULT_PROSPERO_PORT)
PURL_LEGACY_SCHEME(rtsp,      false, false, true,  false,  true,   true,  false, false, true,  false, DEFAULT_RTSP_PORT)
PURL_LEGACY_SCHEME(rtspu,     false, false, true,  false,  true,   false, false, false, true,  false, DEFAULT_RTSPU_PORT)
PURL_LEGACY_SCHEME(ftp,       true,  true,  true,  false,  true,   false, false, false, true,  false, DEFAULT_FTP_PORT)
PURL_LEGACY_SCHEME(telnet,    true,  true,  true,  false,  true,   false, false, false, false, false, DEFAULT_TELNET_PORT)
PURL_LEGACY_SCHEME(mailto,    false, false, false, true,   false,  true,  false, false, false, false, 0)
PURL_LEGACY_SCHEME(news,      false, false, false, false,  true,   false, false, false, false, false, 0)
PURL_LEGACY_SCHEME(h323,      true,  false, true,  true,   false,  false, true,  false, false, false, DEFAULT_H323_PORT)
PURL_LEGACY_SCHEME(h323s,     true,  false, true,  true,   false,  false, true,  false, false, false, DEFAULT_H323S_PORT)
PURL_LEGACY_SCHEME(rtmp,      false, false, true,  false,  false,  false, false, false, true,  false, DEFAULT_RTMP_PORT)
PURL_LEGACY_SCHEME(sip,       true,  true,  true,  false,  false,  true,  true,  false, false, false, DEFAULT_SIP_PORT)
PURL_LEGACY_SCHEME(sips,      true,  true,  true,  false,  false,  true,  true,  false, false, false, DEFAULT_SIPS_PORT)
PURL_LEGACY_SCHEME(fax,       false, false, false, true,   false,  false, true,  false, false, false, 0)
PURL_LEGACY_SCHEME(msrp,      false, false, true,  false,  false,  true,  true,  false, true,  false, DEFAULT_MSRP_PORT)

#define DEFAULT_SCHEME "http"
#define FILE_SCHEME    "file"

//////////////////////////////////////////////////////////////////////////////
// PURL

PURL::PURL()
  : schemeInfo(NULL)
  , port(0)
  , portSupplied (false)
  , relativePath(false)
{
}


PURL::PURL(const char * str, const char * defaultScheme)
{
  InternalParse(str, defaultScheme);
}


PURL::PURL(const PString & str, const char * defaultScheme)
{
  InternalParse((const char *)str, defaultScheme);
}


PURL::PURL(const PFilePath & filePath)
  : schemeInfo(PURLSchemeFactory::CreateInstance(FILE_SCHEME))
  , scheme(FILE_SCHEME)
  , port(0)
  , portSupplied(false)
  , relativePath(false)
{
  PStringArray pathArray = filePath.GetDirectory().GetPath();
  if (pathArray.IsEmpty())
    return;

  if (pathArray[0].GetLength() == 2 && pathArray[0][1] == ':')
    pathArray[0][1] = '|';

  pathArray.AppendString(filePath.GetFileName());

  SetPath(pathArray);
}


PObject::Comparison PURL::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PURL), PInvalidCast);
  return urlString.Compare(((const PURL &)obj).urlString);
}

PURL::PURL(const PURL & other)
{
  CopyContents(other);
}

PURL & PURL::operator=(const PURL & other)
{
  CopyContents(other);
  return *this;
}

void PURL::CopyContents(const PURL & other)
{
  schemeInfo   = other.schemeInfo;
  urlString    = other.urlString;
  scheme       = other.scheme;
  username     = other.username;
  password     = other.password;
  hostname     = other.hostname;
  port         = other.port;
  portSupplied = other.portSupplied;
  relativePath = other.relativePath;
  path         = other.path;
  fragment     = other.fragment;

  paramVars    = other.paramVars;
  paramVars.MakeUnique();

  queryVars    = other.queryVars;
  queryVars.MakeUnique();

  m_contents   = other.m_contents;
}

PINDEX PURL::HashFunction() const
{
  return urlString.HashFunction();
}


void PURL::PrintOn(ostream & stream) const
{
  stream << urlString;
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

  /* Characters sets are from RFC2396.
     The EBNF defines lowalpha, upalpha, digit and mark which are always
     allowed. The reserved list consisting of ";/?:@&=+$," may or may not be
     allowed depending on the syntatic element being encoded.
   */
  PString safeChars = "abcdefghijklmnopqrstuvwxyz"  // lowalpha
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"  // upalpha
                      "0123456789"                  // digit
                      "-_.!~*'()";                  // mark
  switch (type) {
    case LoginTranslation :
      safeChars += ";&=+$,";  // Section 3.2.2
      break;

    case PathTranslation :
      safeChars += ":@&=$,|";   // Section 3.3
      break;

    case ParameterTranslation :
      /* By strict RFC2396/3.3 this should be as for PathTranslation, but many
         URI schemes have parameters of the form key=value so we don't allow
         '=' character in the allowed set. Also, including one of "@,|" is
         incompatible with some schemes, leave those out too. */
      safeChars += ":&+$";
      break;

    case QuotedParameterTranslation :
      safeChars += "[]/:@&=+$,|";
      return str.FindSpan(safeChars) != P_MAX_INDEX ? str.ToLiteral() : str;

    default :
      break;    // Section 3.4, no reserved characters may be used
  }
  
  for (PINDEX pos = 0; (pos = xlat.FindSpan(safeChars, pos)) != P_MAX_INDEX; ++pos) {
    char buf[10];
    sprintf(buf, "%%%02X", (BYTE)xlat[pos]);
    xlat.Splice(buf, pos, 1);
  }

  return xlat;
}


PString PURL::UntranslateString(const PString & str, TranslationType type)
{
  PString xlat = str;
  xlat.MakeUnique();

  PINDEX pos;
  if (type == PURL::QueryTranslation) {
    /* Even though RFC2396 never mentions this, RFC1630 does. */
    for (pos = 0; (pos = xlat.Find('+', pos)) != P_MAX_INDEX; ++pos)
      xlat[pos] = ' ';
  }

  for (pos = 0; (pos = xlat.Find('%', pos)) != P_MAX_INDEX; ++pos) {
    int digit1 = xlat[pos+1];
    int digit2 = xlat[pos+2];
    if (isxdigit(digit1) && isxdigit(digit2)) {
      char buf[2];
      buf[0] = (char)(
            (isdigit(digit2) ? (digit2-'0') : (toupper(digit2)-'A'+10)) +
           ((isdigit(digit1) ? (digit1-'0') : (toupper(digit1)-'A'+10)) << 4));
      buf[1] = '\0';
      xlat.Splice(buf, pos, 3);
    }
  }

  return xlat;
}


void PURL::SplitVars(const PString & str, PStringToString & vars, char sep1, char sep2, TranslationType type)
{
  vars.RemoveAll();

  PINDEX sep1prev = 0;
  do {
    PINDEX sep1next = str.Find(sep1, sep1prev);
    if (sep1next == P_MAX_INDEX)
      sep1next--; // Implicit assumption string is not a couple of gigabytes long ...

    PCaselessString key, data;

    PINDEX sep2pos = str.Find(sep2, sep1prev);
    if (sep2pos > sep1next) {
      if (sep1next > 0)
        key = str(sep1prev, sep1next-1);
    }
    else {
      if (sep2pos > 0)
        key = str(sep1prev, sep2pos-1);
      if (type != QuotedParameterTranslation)
        data = str(sep2pos+1, sep1next-1);
      else {
        while (isspace(str[++sep2pos]))
          ;
        if (str[sep2pos] != '"')
          data = str(sep2pos, sep1next-1);
        else {
          // find the end quote
          PINDEX endQuote = sep2pos+1;
          do {
            endQuote = str.Find('"', endQuote+1);
            if (endQuote == P_MAX_INDEX) {
              PTRACE2(1, NULL, "URI\tNo closing double quote in parameter: " << str);
              endQuote = str.GetLength()-1;
              break;
            }
          } while (str[endQuote-1] == '\\');

          data = PString(PString::Literal, str(sep2pos, endQuote));

          if (sep1next < endQuote) {
            sep1next = str.Find(sep1, endQuote);
            if (sep1next == P_MAX_INDEX)
              sep1next--; // Implicit assumption string is not a couple of gigabytes long ...
          }
        }
      }
    }

    key = PURL::UntranslateString(key, type);
    if (!key) {
      data = PURL::UntranslateString(data, type);
      if (vars.Contains(key))
        vars.SetAt(key, vars[key] + '\n' + data);
      else
        vars.SetAt(key, data);
    }

    sep1prev = sep1next+1;
  } while (sep1prev != P_MAX_INDEX);
}


void PURL::OutputVars(ostream & strm,
                      const PStringToString & vars,
                      char sep0,
                      char sep1,
                      char sep2,
                      TranslationType type)
{
  bool outputSeperator = false;
  for (PStringOptions::const_iterator it = vars.begin(); it != vars.end(); ++it) {
    if (outputSeperator)
      strm << sep1;
    else {
      if (sep0 != '\0')
        strm << sep0;
      outputSeperator = true;
    }

    PString key  = TranslateString(it->first,  type);
    PString data = TranslateString(it->second, type);

    if (key.IsEmpty())
      strm << data;
    else if (data.IsEmpty())
      strm << key;
    else
      strm << key << sep2 << data;
  }
}


PCaselessString PURL::ExtractScheme(const char * cstr)
{
  // Character set as per RFC2396
  //    scheme        = alpha *( alpha | digit | "+" | "-" | "." )
  if (cstr == NULL)
    return PString::Empty();

  while (((*cstr & 0x80) == 0x00) && isspace(*cstr))
    ++cstr;

  if (!isalpha(*cstr))
    return PString::Empty();

  const char * ptr = cstr+1;
  while (isalnum(*ptr) || *ptr == '+' || *ptr == '-' || *ptr == '.')
    ++ptr;

  if (*ptr != ':')
    return PString::Empty();

  return PString(cstr, ptr-cstr);
}


PBoolean PURL::InternalParse(const char * cstr, const char * defaultScheme)
{
  scheme.MakeEmpty();
  username.MakeEmpty();
  password.MakeEmpty();
  hostname.MakeEmpty();
  port = 0;
  portSupplied = false;
  relativePath = false;
  path.SetSize(0);
  paramVars.RemoveAll();
  fragment.MakeEmpty();
  queryVars.RemoveAll();
  m_contents.MakeEmpty();

  // get information which tells us how to parse URL for this
  // particular scheme
  schemeInfo = NULL;

  if (cstr == NULL)
    return false;

  // copy the string so we can take bits off it
  while (((*cstr & 0x80) == 0x00) && isspace(*cstr))
    cstr++;

  // if string is empty, return
  if (*cstr == '\0')
    return false;

  scheme = ExtractScheme(cstr);
  if (!scheme.IsEmpty()) {
    // get the scheme information
    schemeInfo = PURLSchemeFactory::CreateInstance(scheme);
    if (schemeInfo != NULL)
      cstr = strchr(cstr, ':')+1;
  }

  // if we could not match a scheme, then use the specified default scheme
  if (schemeInfo == NULL && defaultScheme != NULL && *defaultScheme != '\0') {
    scheme = defaultScheme;
    schemeInfo = PURLSchemeFactory::CreateInstance(defaultScheme);
    PAssert(schemeInfo != NULL, "Default scheme " + scheme + " not available");
  }

  // if that still fails, then there is nowehere to go
  if (schemeInfo == NULL)
    return false;

  // Now parse using the the scheme info.
  return schemeInfo->Parse(cstr, *this) && !IsEmpty();
}


bool PURL::LegacyParse(const char * cstr, const PURLLegacyScheme * schemeInfo)
{
  const PConstCaselessString str(cstr);
  PINDEX start = 0;
  PINDEX end = P_MAX_INDEX;
  PINDEX pos;

  // if the URL should have leading slash, then remove it if it has one
  if (schemeInfo != NULL && schemeInfo->hasHostPort && schemeInfo->hasPath) {
    if (str.GetLength() > 2 && str[0] == '/' && str[1] == '/')
      start = 2;
    else
      relativePath = true;
  }

  // parse user/password/host/port
  if (!relativePath && schemeInfo->hasHostPort) {
    PString endHostChars;
    if (schemeInfo->hasPath)
      endHostChars += '/';
    if (schemeInfo->hasQuery)
      endHostChars += '?';
    if (schemeInfo->hasParameters)
      endHostChars += ';';
    if (schemeInfo->hasFragments)
      endHostChars += '#';

    if (endHostChars.IsEmpty())
      pos = P_MAX_INDEX;
    else if (schemeInfo->hasUsername) {
      //';' showing in the username field should be valid.
      // Looking for ';' after the '@' for the parameters.
      PINDEX posAt = str.Find('@', start);
      if (posAt != P_MAX_INDEX)
        pos = str.FindOneOf(endHostChars, posAt);
      else 
        pos = str.FindOneOf(endHostChars, start);
    }
    else
      pos = str.FindOneOf(endHostChars, start);

    PString uphp = str(start, pos-1);
    if (pos != P_MAX_INDEX)
      start = pos;
    else
      start = P_MAX_INDEX;

    // if the URL is of type UserPasswordHostPort, then parse it
    if (schemeInfo->hasUsername) {
      // extract username and password
      PINDEX pos2 = uphp.Find('@');
      PINDEX pos3 = P_MAX_INDEX;
      if (schemeInfo->hasPassword)
        pos3 = uphp.Find(':');
      switch (pos2) {
        case 0 :
          uphp.Delete(0, 1);
          break;

        case P_MAX_INDEX :
          if (schemeInfo->defaultToUserIfNoAt) {
            if (pos3 == P_MAX_INDEX)
              username = UntranslateString(uphp, LoginTranslation);
            else {
              username = UntranslateString(uphp.Left(pos3), LoginTranslation);
              password = UntranslateString(uphp.Mid(pos3+1), LoginTranslation);
            }
            uphp.MakeEmpty();
          }
          break;

        default :
          if (pos3 > pos2)
            username = UntranslateString(uphp.Left(pos2), LoginTranslation);
          else {
            username = UntranslateString(uphp.Left(pos3), LoginTranslation);
            password = UntranslateString(uphp(pos3+1, pos2-1), LoginTranslation);
          }
          uphp.Delete(0, pos2+1);
      }
    }

    // if the URL does not have a port, then this is the hostname
    if (schemeInfo->defaultPort == 0)
      hostname = UntranslateString(uphp, LoginTranslation);
    else {
      // determine if the URL has a port number
      // Allow for [ipv6] form
      if (uphp[0] == '[' && (pos = uphp.Find(']')) != P_MAX_INDEX) {
        hostname = uphp.Left(pos+1); // No translation if inside []
        pos = uphp.Find(':', pos);
      }
      else {
        pos = uphp.Find(':');
        hostname = UntranslateString(uphp.Left(pos), LoginTranslation);
      }

      if (pos != P_MAX_INDEX) {
        port = (WORD)uphp.Mid(pos+1).AsUnsigned();
        portSupplied = true;
      }

      if (hostname.IsEmpty() && schemeInfo->defaultHostToLocal)
        hostname = PIPSocket::GetHostName();
    }
  }

  // chop off any trailing query
  if (schemeInfo->hasQuery && (pos = str.Find('?', start)) < end) {
    SplitQueryVars(str(pos+1, end), queryVars);
    end = pos-1;
  }

  // chop off any trailing parameters
  if (schemeInfo->hasParameters && (pos = str.Find(';', start)) < end) {
    SplitVars(str(pos+1, end), paramVars);
    end = pos-1;
  }

  // chop off any trailing fragment
  if (schemeInfo->hasFragments && (pos = str.Find('#', start)) < end) {
    fragment = UntranslateString(str(pos+1, end), PathTranslation);
    end = pos-1;
  }

  if (port == 0 && !relativePath) {
    // Yes another horrible, horrible special case!
    if (scheme == "h323" && paramVars("type") == "gk")
      port = DEFAULT_H323RAS_PORT;
    else
      port = schemeInfo->defaultPort;
  }

  if (schemeInfo->hasPath) {
    if (str[start] == '/')
      ++start;
    SetPathStr(str(start, end));   // the hierarchy is what is left
  }
  else {
    // if the rest of the URL isn't a path, then we are finished!
    m_contents = UntranslateString(str(start, end), PathTranslation);
    Recalculate();
  }

  return true;
}


PFilePath PURL::AsFilePath() const
{
  /* While it is never explicitly stated anywhere in RFC1798, there is an
     implication RFC 1808 that the path is absolute unless the relative
     path rules of that RFC apply. We follow that logic. */

  if (path.IsEmpty() || scheme != FILE_SCHEME || (!hostname.IsEmpty() && hostname != "localhost"))
    return PString::Empty();

  PStringStream str;

  if (path[0].GetLength() == 2 && path[0][1] == '|')
    str << path[0][0] << ':' << PDIR_SEPARATOR; // Special case for Windows paths with drive letter
  else {
    if (!relativePath)
      str << PDIR_SEPARATOR;
    str << path[0];
  }

  for (PINDEX i = 1; i < path.GetSize(); i++)
    str << PDIR_SEPARATOR << path[i];

  return str;
}


PString PURL::AsString(UrlFormat fmt) const
{
  if (fmt == FullURL)
    return urlString;

  if (scheme.IsEmpty() || schemeInfo == NULL)
    return PString::Empty();

  return schemeInfo->AsString(fmt, *this);
}


PString PURL::LegacyAsString(PURL::UrlFormat fmt, const PURLLegacyScheme * schemeInfo) const
{
  PStringStream str;

  if (fmt != RelativeOnly && !(relativePath && schemeInfo->relativeImpliesScheme))
    str << scheme << ':';

  if (fmt == LocationOnly && relativePath)
    return str;

  if (fmt != RelativeOnly && !relativePath) {
    if (schemeInfo->hasPath && schemeInfo->hasHostPort)
      str << "//";

    if (schemeInfo->hasUsername) {
      if (!username) {
        str << TranslateString(username, LoginTranslation);
        if (schemeInfo->hasPassword && !password)
          str << ':' << TranslateString(password, LoginTranslation);
        if (schemeInfo->hasHostPort && !hostname.IsEmpty())
          str << '@';
      }
    }

    if (schemeInfo->hasHostPort) {
      if (hostname[0] == '[') // Should be IPv6 address
        str << hostname;
      else if (hostname.Find(':') != P_MAX_INDEX) // Assume it is an IPv6 address
        str << '[' << hostname << ']';
      else
        str << TranslateString(hostname, LoginTranslation);
    }

    if (schemeInfo->defaultPort != 0) {
      if (port != schemeInfo->defaultPort || portSupplied)
        str << ':' << port;
    }

    if (fmt == LocationOnly) {
      // Problem was fixed for handling legacy schema like tel URI.
      // LocationOnly format: if there is no default user and host fields, only the schema itself is being returned.
      // RelativeOnly only format: the pathStr will be retruned.
      // The Recalculate() will merge both LocationOnly and RelativeOnly formats for the completed uri string creation.
      if (schemeInfo->defaultToUserIfNoAt)
        return str;

      if (str.GetLength() > scheme.GetLength()+1)
        return str;

      // Cannot JUST have the scheme: ....
      return PString::Empty();
    }
  }

  // RelativeOnly and PathOnly
  if (schemeInfo->hasPath) {
    for (PINDEX i = 0; i < path.GetSize(); i++) {
      if (i > 0 || !relativePath)
        str << '/';
      str << TranslateString(path[i], PathTranslation);
    }
    if (!relativePath && str.IsEmpty())
      str << '/';
  }
  else
    str << TranslateString(m_contents, PathTranslation);

  if (fmt == FullURL || fmt == RelativeOnly) {
    if (!fragment)
      str << "#" << TranslateString(fragment, PathTranslation);

    OutputVars(str, paramVars, ';', ';', '=', ParameterTranslation);
    OutputVars(str, queryVars, '?', '&', '=', QueryTranslation);
  }

  return str;
}


bool PURL::SetScheme(const PString & newScheme)
{
  const PURLScheme * newSchemeInfo = PURLSchemeFactory::CreateInstance(newScheme);
  if (newSchemeInfo == NULL)
    return false;

  scheme = newScheme;
  schemeInfo = newSchemeInfo;

  if (!portSupplied) {
    const PURLLegacyScheme * legacy = dynamic_cast<const PURLLegacyScheme *>(schemeInfo);
    if (legacy != NULL)
      port = legacy->defaultPort;
  }

  Recalculate();
  return true;
}


void PURL::SetUserName(const PString & u)
{
  username = u;
  Recalculate();
}


void PURL::SetPassword(const PString & p)
{
  password = p;
  Recalculate();
}


void PURL::SetHostName(const PString & h)
{
  hostname = h;
  Recalculate();
}


void PURL::SetPort(WORD newPort)
{
  if (newPort != 0) {
    port = newPort;
    portSupplied = true;
  }
  else {
    port = schemeInfo != NULL ? schemeInfo->GetDefaultPort() : 0;
    portSupplied = false;
  }
  Recalculate();
}


PString PURL::GetHostPort() const
{
  PStringStream strm;
  strm << hostname;
  if (portSupplied)
    strm << ':' << port;
  return strm;
}


void PURL::SetPathStr(const PString & pathStr)
{
  path = pathStr.Tokenise("/", true);

  if (path.GetSize() > 0 && path[0].IsEmpty()) 
    path.RemoveAt(0);

  for (PINDEX i = 0; i < path.GetSize(); i++) {
    path[i] = UntranslateString(path[i], PathTranslation);
    if (i > 0 && path[i] == ".." && path[i-1] != "..") {
      path.RemoveAt(i--);
      path.RemoveAt(i--);
    }
  }

  Recalculate();
}


PString PURL::GetPathStr() const
{
  PStringStream strm;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    if (i > 0 || !relativePath)
      strm << '/';
    strm << TranslateString(path[i], PathTranslation);
  }
  return strm;
}


void PURL::SetPath(const PStringArray & p)
{
  path = p;
  path.MakeUnique();
  Recalculate();
}


void PURL::AppendPath(const PString & segment)
{
  path.MakeUnique();
  path.AppendString(segment);
  Recalculate();
}


void PURL::ChangePath(const PString & segment, PINDEX idx)
{
  path.MakeUnique();

  if (path.IsEmpty()) {
    if (!segment.IsEmpty())
      path.AppendString(segment);
  }
  else {
    if (idx >= path.GetSize())
      idx = path.GetSize()-1;
    if (segment.IsEmpty())
      path.RemoveAt(idx);
    else
      path[idx] = segment;
  }

  Recalculate();
}


PString PURL::GetParameters() const
{
  PStringStream strm;
  OutputVars(strm, paramVars, '\0', ';', '=', ParameterTranslation);
  return strm;
}


void PURL::SetParameters(const PString & parameters)
{
  SplitVars(parameters, paramVars);
  Recalculate();
}


void PURL::SetParamVars(const PStringToString & p)
{
  paramVars = p;
  Recalculate();
}


void PURL::SetParamVar(const PString & key, const PString & data, bool emptyDataDeletes)
{
  if (emptyDataDeletes && data.IsEmpty())
    paramVars.RemoveAt(key);
  else
    paramVars.SetAt(key, data);
  Recalculate();
}


PString PURL::GetQuery() const
{
  PStringStream strm;
  OutputVars(strm, queryVars, '\0', '&', '=', QueryTranslation);
  return strm;
}


void PURL::SetQuery(const PString & queryStr)
{
  SplitQueryVars(queryStr, queryVars);
  Recalculate();
}


void PURL::SetQueryVars(const PStringToString & q)
{
  queryVars = q;
  Recalculate();
}


void PURL::SetQueryVar(const PString & key, const PString & data)
{
  if (data.IsEmpty())
    queryVars.RemoveAt(key);
  else
    queryVars.SetAt(key, data);
  Recalculate();
}


void PURL::SetContents(const PString & str)
{
  m_contents = str;
  Recalculate();
}


bool PURL::LoadResource(PString & str, const LoadParams & params) const
{
  PURLLoader * loader = PURLLoaderFactory::CreateInstance(GetScheme());
  return loader != NULL && loader->Load(str, *this, params);
}


bool PURL::LoadResource(PBYTEArray & data, const LoadParams & params) const
{
  PURLLoader * loader = PURLLoaderFactory::CreateInstance(GetScheme());
  return loader != NULL && loader->Load(data, *this, params);
}


bool PURL::OpenBrowser(const PString & url)
{
#ifdef _WIN32
  SHELLEXECUTEINFO sei;
  ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
  sei.cbSize = sizeof(SHELLEXECUTEINFO);
  sei.lpVerb = TEXT("open");
  PVarString file = url;
  sei.lpFile = file;

  if (ShellExecuteEx(&sei) != 0)
    return true;

  PVarString msg = "Unable to open page" & url;
  PVarString name = PProcess::Current().GetName();
  MessageBox(NULL, msg, name, MB_TASKMODAL);

#endif // WIN32
  return false;
}


void PURL::Recalculate()
{
  if (schemeInfo != NULL)
    urlString = schemeInfo->AsString(FullURL, *this);
  else
    urlString.MakeEmpty();
}


///////////////////////////////////////////////////////////////////////////////

// RFC3966 tel URI

class PURL_CalltoScheme : public PURLScheme
{
    PCLASSINFO(PURL_CalltoScheme, PURLScheme);
  public:
    virtual bool Parse(const char * cstr, PURL & url) const
    {
      const PConstCaselessString str(cstr);

      // Actually not part of MS spec, but a lot of people put in the // into
      // the URL, so we take it out of it is there.
      PINDEX start = 0;
      if (str.GetLength() > 2 && str[0] == '/' && str[1] == '/')
        start = 2;

      // For some bizarre reason callto uses + instead of ; for paramters
      PINDEX pos = str.Find('+', start);

      // We also check for phone numbers of the form +61243654666 still work
      if (pos != P_MAX_INDEX && isdigit(str[pos+1]))
        pos = str.Find('+', pos+1);

      PINDEX end = P_MAX_INDEX;
      if (pos != P_MAX_INDEX) {
        PStringToString paramVars;
        PURL::SplitVars(str(start, ++pos), paramVars, '+', '=');
        url.SetParamVars(paramVars);
        end = pos-1;
      }

      PString hostname = url.GetParamVars()("gateway");
      PString username;
      if (!hostname)
        username = PURL::UntranslateString(str(start, end), PURL::LoginTranslation);
      else {
        PCaselessString type = url.GetParamVars()("type");
        if (type == "directory") {
          pos = str.Find('/', start);
          if (pos == P_MAX_INDEX)
            username = PURL::UntranslateString(str(start, end), PURL::LoginTranslation);
          else {
            hostname = PURL::UntranslateString(str(start, pos), PURL::LoginTranslation);
            username = PURL::UntranslateString(str(pos+1, end), PURL::LoginTranslation);
          }
        }
        else {
          // Now look for an @ and split user and host
          pos = str.Find('@');
          if (pos != P_MAX_INDEX) {
            username = PURL::UntranslateString(str(start, pos), PURL::LoginTranslation);
            hostname = PURL::UntranslateString(str(pos+1, end), PURL::LoginTranslation);
          }
          else {
            if (type == "ip" || type == "host")
              hostname = PURL::UntranslateString(str(start, end), PURL::LoginTranslation);
            else
              username = PURL::UntranslateString(str(start, end), PURL::LoginTranslation);
          }
        }
      }

      // Allow for [ipv6] form
      pos = hostname.Find(']');
      if (pos == P_MAX_INDEX)
        pos = 0;
      pos = hostname.Find(':', pos);
      if (pos != P_MAX_INDEX) {
        url.SetPort((WORD)hostname.Mid(pos+1).AsUnsigned());
        hostname.Delete(pos, P_MAX_INDEX);
      }

      url.SetHostName(hostname);
      url.SetUserName(username);
      url.SetPassword(url.GetParamVars()("password"));
      return true;
    }

    virtual PString AsString(PURL::UrlFormat fmt, const PURL & url) const
    {
      if (fmt == PURL::LocationOnly)
        return PString::Empty();

      PStringStream strm;
      strm << "callto:" + url.GetUserName();
      PURL::OutputVars(strm, url.GetParamVars(), '+', '+', '=', PURL::ParameterTranslation);
      return strm;
    }
};

PFACTORY_CREATE(PURLSchemeFactory, PURL_CalltoScheme, "callto", true);


///////////////////////////////////////////////////////////////////////////////

// RFC3966 tel URI

class PURL_TelScheme : public PURLScheme
{
    PCLASSINFO(PURL_TelScheme, PURLScheme);
  public:
    virtual PBoolean Parse(const char * cstr, PURL & url) const
    {
      const PConstCaselessString str(cstr);

      PINDEX pos = str.FindSpan("0123456789*#", str[0] != '+' ? 0 : 1);
      if (pos == P_MAX_INDEX)
        url.SetUserName(str);
      else {
        if (str[pos] != ';')
          return false;

        url.SetUserName(str.Left(pos));

        PStringToString paramVars;
        PURL::SplitVars(str.Mid(pos+1), paramVars);
        url.SetParamVars(paramVars);

        PString phoneContext = paramVars("phone-context");
        if (phoneContext.IsEmpty()) {
          if (str[0] != '+')
            return false;
        }
        else if (phoneContext[0] != '+')
          url.SetHostName(phoneContext);
        else if (str[0] != '+')
          url.SetUserName(phoneContext+url.GetUserName());
        else
          return false;
      }

      return url.GetUserName() != "+";
    }

    virtual PString AsString(PURL::UrlFormat fmt, const PURL & url) const
    {
      if (fmt == PURL::LocationOnly)
        return PString::Empty();

      PStringStream strm;
      strm << "tel:" + url.GetUserName();
      PURL::OutputVars(strm, url.GetParamVars(), ';', ';', '=', PURL::ParameterTranslation);
      return strm;
    }
};

PFACTORY_CREATE(PURLSchemeFactory,PURL_TelScheme, "tel", true);


///////////////////////////////////////////////////////////////////////////////

// RFC2397 data URI

class PURL_DataScheme : public PURLScheme
{
    PCLASSINFO(PURL_DataScheme, PURLScheme);
  public:
    virtual bool Parse(const char * cstr, PURL & url) const
    {
      const PConstCaselessString str(cstr);

      PINDEX comma = str.Find(',');
      if (comma == P_MAX_INDEX)
        return false;

      PINDEX semi = str.Find(';');
      if (semi > comma)
        url.SetParamVar("type", str.Left(comma));
      else {
        url.SetParameters(str(semi, comma-1));
        url.SetParamVar("type", str.Left(semi));
      }

      url.SetContents(str.Mid(comma+1));

      return true;
    }

    virtual PString AsString(PURL::UrlFormat fmt, const PURL & purl) const
    {
      if (fmt == PURL::LocationOnly)
        return PString::Empty();

      const PStringToString & params = purl.GetParamVars();
      PStringStream strm;

      strm << "data:" + params("type", "text/plain");

      bool base64 = false;
      for (PStringOptions::const_iterator it = params.begin(); it != params.end(); ++it) {
        PCaselessString key = it->first;
        if (key == "type")
          continue;
        if (key == "base64") {
          base64 = true;
          continue;
        }

        strm << ';' << PURL::TranslateString(key, PURL::ParameterTranslation);

        PString data = it->second;
        if (!data)
          strm << '=' << PURL::TranslateString(data, PURL::ParameterTranslation);
      }

      // This must always be last according to EBNF
      if (base64)
        strm << ";base64";

      strm << ',' << PURL::TranslateString(purl.GetContents(), PURL::ParameterTranslation);

      return strm;
    }
};

PFACTORY_CREATE(PURLSchemeFactory, PURL_DataScheme, "data", true);


///////////////////////////////////////////////////////////////////////////////

class PURL_FileLoader : public PURLLoader
{
    PCLASSINFO(PURL_FileLoader, PURLLoader);
  public:
    virtual bool Load(PString & str, const PURL & url, const PURL::LoadParams &) const
    {
      PFile file;
      if (!file.Open(url.AsFilePath()))
        return false;
      str = file.ReadString(file.GetLength());
      return true;
    }

    virtual bool Load(PBYTEArray & data, const PURL & url, const PURL::LoadParams &) const
    {
      PFile file;
      if (!file.Open(url.AsFilePath()))
        return false;
      if (!data.SetSize(file.GetLength()))
        return false;
      return file.Read(data.GetPointer(), data.GetSize());
    }
};

PFACTORY_CREATE(PURLLoaderFactory, PURL_FileLoader, "file", true);


///////////////////////////////////////////////////////////////////////////////

class PURL_DataLoader : public PURLLoader
{
    PCLASSINFO(PURL_FileLoader, PURLLoader);
  public:
    virtual bool Load(PString & str, const PURL & url, const PURL::LoadParams & params) const
    {
      if (!params.m_requiredContentType.IsEmpty()) {
        PCaselessString actualContentType = url.GetParamVars()("type");
        if (!actualContentType.IsEmpty() && actualContentType != params.m_requiredContentType)
          return false;
      }

      str = url.GetContents();
      return true;
    }

    virtual bool Load(PBYTEArray & data, const PURL & url, const PURL::LoadParams & params) const
    {
      if (!params.m_requiredContentType.IsEmpty()) {
        PCaselessString actualContentType = url.GetParamVars()("type");
        if (!actualContentType.IsEmpty() && actualContentType != params.m_requiredContentType)
          return false;
      }

      if (url.GetParamVars().Contains("base64"))
        return PBase64::Decode(url.GetContents(), data);

      PString str = url.GetContents();
      PINDEX len = str.GetLength();
      if (!data.SetSize(len))
        return false;

      memcpy(data.GetPointer(), (const char *)str, len);
      return true;
    }
};

PFACTORY_CREATE(PURLLoaderFactory, PURL_DataLoader, "data", true);

#endif // P_URL


// End Of File ///////////////////////////////////////////////////////////////
