/*
 * http.cxx
 *
 * HTTP ancestor class and common classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
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
#pragma implementation "http.h"
#endif

#include <ptlib.h>

#if P_HTTP

#include <ptlib/sockets.h>
#include <ptclib/http.h>
#include <ptclib/url.h>


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


const PCaselessString & PHTTP::HostTag             () { static const PConstCaselessString s("Host"); return s; }
const PCaselessString & PHTTP::AllowTag            () { static const PConstCaselessString s("Allow"); return s; }
const PCaselessString & PHTTP::AuthorizationTag    () { static const PConstCaselessString s("Authorization"); return s; }
const PCaselessString & PHTTP::ContentEncodingTag  () { static const PConstCaselessString s("Content-Encoding"); return s; }
const PCaselessString & PHTTP::ContentLengthTag    () { static const PConstCaselessString s("Content-Length"); return s; }
const PCaselessString & PHTTP::DateTag             () { static const PConstCaselessString s("Date"); return s; }
const PCaselessString & PHTTP::ExpiresTag          () { static const PConstCaselessString s("Expires"); return s; }
const PCaselessString & PHTTP::FromTag             () { static const PConstCaselessString s("From"); return s; }
const PCaselessString & PHTTP::IfModifiedSinceTag  () { static const PConstCaselessString s("If-Modified-Since"); return s; }
const PCaselessString & PHTTP::LastModifiedTag     () { static const PConstCaselessString s("Last-Modified"); return s; }
const PCaselessString & PHTTP::LocationTag         () { static const PConstCaselessString s("Location"); return s; }
const PCaselessString & PHTTP::PragmaTag           () { static const PConstCaselessString s("Pragma"); return s; }
const PCaselessString & PHTTP::PragmaNoCacheTag    () { static const PConstCaselessString s("no-cache"); return s; }
const PCaselessString & PHTTP::RefererTag          () { static const PConstCaselessString s("Referer"); return s; }
const PCaselessString & PHTTP::ServerTag           () { static const PConstCaselessString s("Server"); return s; }
const PCaselessString & PHTTP::UserAgentTag        () { static const PConstCaselessString s("User-Agent"); return s; }
const PCaselessString & PHTTP::WWWAuthenticateTag  () { static const PConstCaselessString s("WWW-Authenticate"); return s; }
const PCaselessString & PHTTP::MIMEVersionTag      () { static const PConstCaselessString s("MIME-Version"); return s; }
const PCaselessString & PHTTP::ConnectionTag       () { static const PConstCaselessString s("Connection"); return s; }
const PCaselessString & PHTTP::KeepAliveTag        () { static const PConstCaselessString s("Keep-Alive"); return s; }
const PCaselessString & PHTTP::UpgradeTag          () { static const PConstCaselessString s("Upgrade"); return s; }
const PCaselessString & PHTTP::WebSocketTag        () { static const PConstCaselessString s("websocket"); return s; }
const PCaselessString & PHTTP::WebSocketKeyTag     () { static const PConstCaselessString s("Sec-WebSocket-Key"); return s; }
const PCaselessString & PHTTP::WebSocketAcceptTag  () { static const PConstCaselessString s("Sec-WebSocket-Accept"); return s; }
const PCaselessString & PHTTP::WebSocketProtocolTag() { static const PConstCaselessString s("Sec-WebSocket-Protocol"); return s; }
const PCaselessString & PHTTP::WebSocketVersionTag () { static const PConstCaselessString s("Sec-WebSocket-Version"); return s; }
const PCaselessString & PHTTP::TransferEncodingTag () { static const PConstCaselessString s("Transfer-Encoding"); return s; }
const PCaselessString & PHTTP::ChunkedTag          () { static const PConstCaselessString s("chunked"); return s; }
const PCaselessString & PHTTP::ProxyConnectionTag  () { static const PConstCaselessString s("Proxy-Connection"); return s; }
const PCaselessString & PHTTP::ProxyAuthorizationTag(){ static const PConstCaselessString s("Proxy-Authorization"); return s; }
const PCaselessString & PHTTP::ProxyAuthenticateTag() { static const PConstCaselessString s("Proxy-Authenticate"); return s; }
const PCaselessString & PHTTP::ForwardedTag        () { static const PConstCaselessString s("Forwarded"); return s; }
const PCaselessString & PHTTP::SetCookieTag        () { static const PConstCaselessString s("Set-Cookie"); return s; }
const PCaselessString & PHTTP::CookieTag           () { static const PConstCaselessString s("Cookie"); return s; }
const PCaselessString & PHTTP::FormUrlEncoded      () { static const PConstCaselessString s("application/x-www-form-urlencoded"); return s; }
const PCaselessString & PHTTP::AllowHeaderTag      () { static const PConstCaselessString s("Access-Control-Allow-Headers"); return s; }
const PCaselessString & PHTTP::AllowOriginTag      () { static const PConstCaselessString s("Access-Control-Allow-Origin"); return s; }
const PCaselessString & PHTTP::AllowMethodTag      () { static const PConstCaselessString s("Access-Control-Allow-Methods"); return s; }
const PCaselessString & PHTTP::MaxAgeTAG           () { static const PConstCaselessString s("Access-Control-Max-Age"); return s; }



PHTTP::PHTTP()
  : PInternetProtocol("www 80", NumCommands, HTTPCommands)
{
}


PHTTP::PHTTP(const char * defaultServiceName)
  : PInternetProtocol(defaultServiceName, NumCommands, HTTPCommands)
{
}


PINDEX PHTTP::ParseResponse(const PString & line)
{
  PINDEX endVer = line.Find(' ');
  if (endVer == P_MAX_INDEX)
    return SetLastResponse(PHTTP::BadResponse, "Bad response");

  m_lastResponseInfo = line.Left(endVer);
  PINDEX endCode = line.Find(' ', endVer+1);
  m_lastResponseCode = line(endVer+1,endCode-1).AsInteger();
  if (m_lastResponseCode == 0)
    m_lastResponseCode = PHTTP::BadResponse;
  m_lastResponseInfo &= line.Mid(endCode);
  return 0;
}

#endif // P_HTTP


// End Of File ///////////////////////////////////////////////////////////////
