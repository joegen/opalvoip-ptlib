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
 */

#ifdef __GNUC__
#pragma implementation "url.h"
#endif

#include <ptlib.h>

#if P_URL

#include <ptclib/url.h>

#include <ptlib/sockets.h>
#include <ptclib/cypher.h>
#include <ptclib/mime.h>
#include <ctype.h>

#if defined(_WIN32)
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
PURL_LEGACY_SCHEME(ws,        false, false, true,  false,  true,   true,  false, false, true,  false, DEFAULT_HTTP_PORT)
PURL_LEGACY_SCHEME(wss,       false, false, true,  false,  true,   true,  false, false, true,  false, DEFAULT_HTTPS_PORT)

#define DEFAULT_SCHEME "http"
#define FILE_SCHEME    "file"

//////////////////////////////////////////////////////////////////////////////
// PURL

PURL::PURL()
  : m_schemeInfo(NULL)
  , m_port(0)
  , m_portSupplied (false)
  , m_relativePath(false)
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
  : m_schemeInfo(PURLSchemeFactory::CreateInstance(FILE_SCHEME))
  , m_scheme(FILE_SCHEME)
  , m_port(0)
  , m_portSupplied(false)
  , m_relativePath(false)
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
  return m_urlString.Compare(((const PURL &)obj).m_urlString);
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
  m_schemeInfo   = other.m_schemeInfo;
  m_urlString    = other.m_urlString;
  m_scheme       = other.m_scheme;
  m_username     = other.m_username;
  m_password     = other.m_password;
  m_hostname     = other.m_hostname;
  m_port         = other.m_port;
  m_portSupplied = other.m_portSupplied;
  m_relativePath = other.m_relativePath;
  m_path         = other.m_path;
  m_fragment     = other.m_fragment;

  m_paramVars    = other.m_paramVars;
  m_paramVars.MakeUnique();

  m_queryVars    = other.m_queryVars;
  m_queryVars.MakeUnique();

  m_contents   = other.m_contents;
}

PINDEX PURL::HashFunction() const
{
  return m_urlString.HashFunction();
}


void PURL::PrintOn(ostream & stream) const
{
  stream << m_urlString;
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
      // If already starts and ends with quotes, assume it already formatted correctly.
      if ((str.GetLength() >= 2 && str[0] == '"' && str[str.GetLength()-1] == '"') ||
           str.FindSpan(safeChars) == P_MAX_INDEX)
        return str;

      return str.ToLiteral();

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

    key = PURL::UntranslateString(key.Trim(), type);
    if (!key.IsEmpty()) {
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

    if (type < ParameterTranslation) {
        PString data = TranslateString(it->second, type);
      if (key.IsEmpty())
        strm << data;
      else
        strm << key << sep2 << data;
    }
    else {
      PStringArray values = it->second.Lines();
      if (values.IsEmpty())
        strm << key;
      else {
        for (PINDEX i = 0; i < values.GetSize(); ++i) {
          PString data = TranslateString(values[i], type);

          if (i > 0)
            strm << sep1;
          if (key.IsEmpty())
            strm << data;
          else
            strm << key << sep2 << data;
        }
      }
    }
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
  m_scheme.MakeEmpty();
  m_username.MakeEmpty();
  m_password.MakeEmpty();
  m_hostname.MakeEmpty();
  m_port = 0;
  m_portSupplied = false;
  m_relativePath = false;
  m_path.SetSize(0);
  m_paramVars.RemoveAll();
  m_fragment.MakeEmpty();
  m_queryVars.RemoveAll();
  m_contents.MakeEmpty();

  // get information which tells us how to parse URL for this
  // particular scheme
  m_schemeInfo = NULL;

  if (cstr == NULL)
    return false;

  // copy the string so we can take bits off it
  while (((*cstr & 0x80) == 0x00) && isspace(*cstr))
    cstr++;

  // if string is empty, return
  if (*cstr == '\0')
    return false;

  m_scheme = ExtractScheme(cstr);
  if (!m_scheme.IsEmpty()) {
    // get the scheme information
    m_schemeInfo = PURLSchemeFactory::CreateInstance(m_scheme);
    if (m_schemeInfo != NULL)
      cstr = strchr(cstr, ':')+1;
  }

  // if we could not match a scheme, then use the specified default scheme
  if (m_schemeInfo == NULL && defaultScheme != NULL && *defaultScheme != '\0') {
    m_scheme = defaultScheme;
    m_schemeInfo = PURLSchemeFactory::CreateInstance(defaultScheme);
    PAssert(m_schemeInfo != NULL, "Default scheme " + m_scheme + " not available");
  }

  // if that still fails, then there is nowehere to go
  if (m_schemeInfo == NULL)
    return false;

  // Now parse using the the scheme info.
  return m_schemeInfo->Parse(cstr, *this) && !IsEmpty();
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
      m_relativePath = schemeInfo->relativeImpliesScheme || str[0] != '/';
  }

  // parse user/password/host/port
  if (!m_relativePath && schemeInfo->hasHostPort) {
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

    PString uphp;
    if (pos > start) {
      uphp = str(start, pos - 1);
      start = pos;
    }

    // if the URL is of type UserPasswordHostPort, then parse it
    if (schemeInfo->hasUsername) {
      // extract username and password
      PINDEX pos2 = uphp.Find('@');
      PINDEX pos3 = P_MAX_INDEX;
      if (schemeInfo->hasPassword)
        pos3 = uphp.Find(':');
      if (pos2 == 0)
        uphp.Delete(0, 1);
      else if (pos2 == P_MAX_INDEX) {
        if (schemeInfo->defaultToUserIfNoAt) {
          if (pos3 == P_MAX_INDEX)
            m_username = UntranslateString(uphp, LoginTranslation);
          else {
            m_username = UntranslateString(uphp.Left(pos3), LoginTranslation);
            m_password = UntranslateString(uphp.Mid(pos3+1), LoginTranslation);
          }
          uphp.MakeEmpty();
        }
      }
      else {
        if (pos3 > pos2)
          m_username = UntranslateString(uphp.Left(pos2), LoginTranslation);
        else {
          m_username = UntranslateString(uphp.Left(pos3), LoginTranslation);
          m_password = UntranslateString(uphp(pos3+1, pos2-1), LoginTranslation);
        }
        uphp.Delete(0, pos2+1);
      }
    }

    // if the URL does not have a port, then this is the hostname
    if (schemeInfo->defaultPort == 0)
      m_hostname = UntranslateString(uphp, LoginTranslation);
    else {
      // determine if the URL has a port number
      // Allow for [ipv6] form
      if (uphp[0] == '[' && (pos = uphp.Find(']')) != P_MAX_INDEX) {
        m_hostname = uphp.Left(pos+1); // No translation if inside []
        pos = uphp.Find(':', pos);
      }
      else {
        pos = uphp.Find(':');
        m_hostname = UntranslateString(uphp.Left(pos), LoginTranslation);
      }

      if (pos != P_MAX_INDEX) {
        m_port = (WORD)uphp.Mid(pos+1).AsUnsigned();
        m_portSupplied = true;
      }

      if (m_hostname.IsEmpty() && schemeInfo->defaultHostToLocal)
        m_hostname = PIPSocket::GetHostName();
    }
  }

  // chop off any trailing query
  if (schemeInfo->hasQuery && (pos = str.Find('?', start)) < end) {
    SplitQueryVars(str(pos+1, end), m_queryVars);
    end = pos-1;
  }

  // chop off any trailing parameters
  if (schemeInfo->hasParameters && (pos = str.Find(';', start)) < end) {
    SplitVars(str(pos+1, end), m_paramVars);
    end = pos-1;
  }

  // chop off any trailing fragment
  if (schemeInfo->hasFragments && (pos = str.Find('#', start)) < end) {
    m_fragment = UntranslateString(str(pos+1, end), PathTranslation);
    end = pos-1;
  }

  if (m_port == 0 && !m_relativePath) {
    // Yes another horrible, horrible special case!
    if (m_scheme == "h323" && m_paramVars("type") == "gk")
      m_port = DEFAULT_H323RAS_PORT;
    else
      m_port = schemeInfo->defaultPort;
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

  if (m_path.IsEmpty() || m_scheme != FILE_SCHEME || (!m_hostname.IsEmpty() && m_hostname != "localhost"))
    return PString::Empty();

  PStringStream str;

  if (m_path[0].GetLength() == 2 && m_path[0][1] == '|')
    str << m_path[0][0] << ':' << PDIR_SEPARATOR; // Special case for Windows paths with drive letter
  else {
    if (!m_relativePath)
      str << PDIR_SEPARATOR;
    str << m_path[0];
  }

  for (PINDEX i = 1; i < m_path.GetSize(); i++)
    str << PDIR_SEPARATOR << m_path[i];

  return str;
}


PString PURL::AsString(UrlFormat fmt) const
{
  if (fmt == FullURL)
    return m_urlString;

  if (m_scheme.IsEmpty() || m_schemeInfo == NULL)
    return PString::Empty();

  return m_schemeInfo->AsString(fmt, *this);
}


PString PURL::LegacyAsString(PURL::UrlFormat fmt, const PURLLegacyScheme * schemeInfo) const
{
  PStringStream str;

  if (fmt != RelativeOnly && !(m_relativePath && schemeInfo->relativeImpliesScheme))
    str << m_scheme << ':';

  if (fmt == LocationOnly && m_relativePath)
    return str;

  if (fmt != RelativeOnly && !m_relativePath) {
    if (schemeInfo->hasPath && schemeInfo->hasHostPort)
      str << "//";

    if (schemeInfo->hasUsername) {
      if (!m_username.IsEmpty()) {
        str << TranslateString(m_username, LoginTranslation);
        if (schemeInfo->hasPassword && !m_password.IsEmpty())
          str << ':' << TranslateString(m_password, LoginTranslation);
        if (schemeInfo->hasHostPort && !m_hostname.IsEmpty())
          str << '@';
      }
    }

    if (schemeInfo->hasHostPort) {
      if (m_hostname[0] == '[') // Should be IPv6 address
        str << m_hostname;
      else if (m_hostname.Find(':') != P_MAX_INDEX) // Assume it is an IPv6 address
        str << '[' << m_hostname << ']';
      else
        str << TranslateString(m_hostname, LoginTranslation);
    }

    if (schemeInfo->defaultPort != 0) {
      if (m_port != schemeInfo->defaultPort || m_portSupplied)
        str << ':' << m_port;
    }

    if (fmt == LocationOnly) {
      // Problem was fixed for handling legacy schema like tel URI.
      // LocationOnly format: if there is no default user and host fields, only the schema itself is being returned.
      // RelativeOnly only format: the pathStr will be retruned.
      // The Recalculate() will merge both LocationOnly and RelativeOnly formats for the completed uri string creation.
      if (schemeInfo->defaultToUserIfNoAt)
        return str;

      if (str.GetLength() > m_scheme.GetLength()+1)
        return str;

      // Cannot JUST have the scheme: ....
      return PString::Empty();
    }
  }

  // RelativeOnly and PathOnly
  if (schemeInfo->hasPath) {
    for (PINDEX i = 0; i < m_path.GetSize(); i++) {
      if (i > 0 || !m_relativePath)
        str << '/';
      str << TranslateString(m_path[i], PathTranslation);
    }
    if (!m_relativePath && str.IsEmpty())
      str << '/';
  }
  else
    str << TranslateString(m_contents, PathTranslation);

  if (fmt == FullURL || fmt == RelativeOnly) {
    if (!m_fragment.IsEmpty())
      str << "#" << TranslateString(m_fragment, PathTranslation);

    OutputVars(str, m_paramVars, ';', ';', '=', ParameterTranslation);
    OutputVars(str, m_queryVars, '?', '&', '=', QueryTranslation);
  }

  return str;
}


bool PURL::SetScheme(const PString & newScheme)
{
  const PURLScheme * newSchemeInfo = PURLSchemeFactory::CreateInstance(newScheme);
  if (newSchemeInfo == NULL)
    return false;

  m_scheme = newScheme;
  m_schemeInfo = newSchemeInfo;

  if (!m_portSupplied) {
    const PURLLegacyScheme * legacy = dynamic_cast<const PURLLegacyScheme *>(m_schemeInfo);
    if (legacy != NULL)
      m_port = legacy->defaultPort;
  }

  Recalculate();
  return true;
}


void PURL::SetUserName(const PString & u)
{
  m_username = u;
  Recalculate();
}


void PURL::SetPassword(const PString & p)
{
  m_password = p;
  Recalculate();
}


void PURL::SetHostName(const PString & h)
{
  m_hostname = h;
  Recalculate();
}


void PURL::SetPort(WORD newPort)
{
  if (newPort != 0) {
    m_port = newPort;
    m_portSupplied = true;
  }
  else {
    m_port = m_schemeInfo != NULL ? m_schemeInfo->GetDefaultPort() : 0;
    m_portSupplied = false;
  }
  Recalculate();
}


PString PURL::GetHostPort() const
{
  PStringStream strm;
  strm << m_hostname;
  if (m_portSupplied)
    strm << ':' << m_port;
  return strm;
}


void PURL::SetPathStr(const PString & pathStr)
{
  m_path = pathStr.Tokenise("/", true);

  if (m_path.GetSize() > 0 && m_path[0].IsEmpty()) 
    m_path.RemoveAt(0);

  for (PINDEX i = 0; i < m_path.GetSize(); i++) {
    m_path[i] = UntranslateString(m_path[i], PathTranslation);
    if (i > 0 && m_path[i] == ".." && m_path[i-1] != "..") {
      m_path.RemoveAt(i--);
      m_path.RemoveAt(i--);
    }
  }

  Recalculate();
}


PString PURL::GetPathStr() const
{
  PStringStream strm;
  for (PINDEX i = 0; i < m_path.GetSize(); i++) {
    if (i > 0 || !m_relativePath)
      strm << '/';
    strm << TranslateString(m_path[i], PathTranslation);
  }
  return strm;
}


void PURL::SetPath(const PStringArray & p)
{
  m_path = p;
  m_path.MakeUnique();
  Recalculate();
}


void PURL::AppendPath(const PString & segment)
{
  m_path.MakeUnique();
  m_path.AppendString(segment);
  Recalculate();
}


void PURL::ChangePath(const PString & segment, PINDEX idx)
{
  m_path.MakeUnique();

  if (m_path.IsEmpty()) {
    if (!segment.IsEmpty())
      m_path.AppendString(segment);
  }
  else {
    if (idx >= m_path.GetSize())
      idx = m_path.GetSize()-1;
    if (segment.IsEmpty())
      m_path.RemoveAt(idx);
    else
      m_path[idx] = segment;
  }

  Recalculate();
}


PString PURL::GetParameters() const
{
  PStringStream strm;
  OutputVars(strm, m_paramVars, '\0', ';', '=', ParameterTranslation);
  return strm;
}


void PURL::SetParameters(const PString & parameters)
{
  SplitVars(parameters, m_paramVars);
  Recalculate();
}


void PURL::SetParamVars(const PStringToString & p, bool merge)
{
  if (merge)
    m_paramVars.Merge(p, PStringToString::e_MergeIgnore);
  else {
    m_paramVars = p;
    m_paramVars.MakeUnique();
  }
  Recalculate();
}


void PURL::SetParamVar(const PString & key, const PString & data, bool emptyDataDeletes)
{
  if (emptyDataDeletes && data.IsEmpty())
    m_paramVars.RemoveAt(key);
  else
    m_paramVars.SetAt(key, data);
  Recalculate();
}


PString PURL::GetQuery() const
{
  PStringStream strm;
  OutputVars(strm, m_queryVars, '\0', '&', '=', QueryTranslation);
  return strm;
}


void PURL::SetQuery(const PString & queryStr)
{
  SplitQueryVars(queryStr, m_queryVars);
  Recalculate();
}


void PURL::SetQueryVars(const PStringToString & q)
{
  m_queryVars = q;
  Recalculate();
}


void PURL::SetQueryVar(const PString & key, const PString & data)
{
  if (data.IsEmpty())
    m_queryVars.RemoveAt(key);
  else
    m_queryVars.SetAt(key, data);
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
  if (m_schemeInfo != NULL)
    m_urlString = m_schemeInfo->AsString(FullURL, *this);
  else
    m_urlString.MakeEmpty();
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
      if (!hostname.IsEmpty())
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

      strm << "data:" + params("type", PMIMEInfo::TextPlain());

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
        if (!data.IsEmpty())
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
      if (!file.Open(url.AsFilePath(), PFile::ReadOnly))
        return false;
      str = file.ReadString(file.GetLength());
      return true;
    }

    virtual bool Load(PBYTEArray & data, const PURL & url, const PURL::LoadParams &) const
    {
      PFile file;
      if (!file.Open(url.AsFilePath(), PFile::ReadOnly))
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
