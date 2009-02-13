/*
 * httpclnt.cxx
 *
 * HTTP Client class.
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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#if P_HTTP

#include <ptlib/sockets.h>
#include <ptclib/http.h>

#if P_SSL
#include <ptclib/pssl.h>
#endif

#include <ctype.h>


#define new PNEW


//////////////////////////////////////////////////////////////////////////////
// PHTTPClient

PHTTPClient::PHTTPClient()
{
}


PHTTPClient::PHTTPClient(const PString & userAgent)
  : userAgentName(userAgent)
{
}


int PHTTPClient::ExecuteCommand(Commands cmd,
                                const PURL & url,
                                PMIMEInfo & outMIME,
                                const PString & dataBody,
                                PMIMEInfo & replyMime,
                                PBoolean persist)
{
  return ExecuteCommand(commandNames[cmd], url, outMIME, dataBody, replyMime, persist);
}


int PHTTPClient::ExecuteCommand(const PString & cmdName,
                                const PURL & url,
                                PMIMEInfo & outMIME,
                                const PString & dataBody,
                                PMIMEInfo & replyMime,
                                PBoolean persist)
{
  if (!outMIME.Contains(DateTag()))
    outMIME.SetAt(DateTag(), PTime().AsString());

  if (!userAgentName && !outMIME.Contains(UserAgentTag()))
    outMIME.SetAt(UserAgentTag(), userAgentName);

  if (persist)
    outMIME.SetAt(ConnectionTag(), KeepAliveTag());

  for (PINDEX retry = 0; retry < 3; retry++) {
    if (!AssureConnect(url, outMIME))
      break;

    if (!WriteCommand(cmdName, url.AsString(PURL::URIOnly), outMIME, dataBody)) {
      lastResponseCode = -1;
      lastResponseInfo = GetErrorText(LastWriteError);
      break;
    }

    // If not persisting need to shut down write so other end stops reading
    if (!persist)
      Shutdown(ShutdownWrite);

    // Await a response, if all OK exit loop
    if (ReadResponse(replyMime)) {
      if (lastResponseCode != Continue)
        break;
      if (ReadResponse(replyMime))
        break;
    }

    // If not persisting, we have no oppurtunity to write again, just error out
    if (!persist)
      break;

    // If have had a failure to read a response but there was no error then
    // we have a shutdown socket probably due to a lack of persistence so ...
    if (GetErrorCode(LastReadError) != NoError)
      break;

    // ... we close the channel and allow AssureConnet() to reopen it.
    Close();
  }

  return lastResponseCode;
}


PBoolean PHTTPClient::WriteCommand(Commands cmd,
                               const PString & url,
                               PMIMEInfo & outMIME,
                               const PString & dataBody)
{
  return WriteCommand(commandNames[cmd], url, outMIME, dataBody);
}


PBoolean PHTTPClient::WriteCommand(const PString & cmdName,
                               const PString & url,
                               PMIMEInfo & outMIME,
                               const PString & dataBody)
{
  ostream & this_stream = *this;
  PINDEX len = dataBody.GetSize()-1;
  if (!outMIME.Contains(ContentLengthTag()))
    outMIME.SetInteger(ContentLengthTag(), len);

  if (cmdName.IsEmpty())
    this_stream << "GET";
  else
    this_stream << cmdName;

  this_stream << ' ' << (url.IsEmpty() ? "/" :  (const char*) url) << " HTTP/1.1\r\n"
              << setfill('\r') << outMIME;

  return Write((const char *)dataBody, len);
}


PBoolean PHTTPClient::ReadResponse(PMIMEInfo & replyMIME)
{
  PString http = ReadString(7);
  if (!http) {
    UnRead(http);

    if (http.Find("HTTP/") == P_MAX_INDEX) {
      lastResponseCode = PHTTP::RequestOK;
      lastResponseInfo = "HTTP/0.9";
      return PTrue;
    }

    if (http[0] == '\n')
      ReadString(1);
    else if (http[0] == '\r' &&  http[1] == '\n')
      ReadString(2);

    if (PHTTP::ReadResponse())
      if (replyMIME.Read(*this))
        return PTrue;
  }
 
  lastResponseCode = -1;
  if (GetErrorCode(LastReadError) != NoError)
    lastResponseInfo = GetErrorText(LastReadError);
  else {
    lastResponseInfo = "Premature shutdown";
    SetErrorValues(ProtocolFailure, 0, LastReadError);
  }

  return PFalse;
}


PBoolean PHTTPClient::ReadContentBody(PMIMEInfo & replyMIME, PString & body)
{
  PBoolean ok = InternalReadContentBody(replyMIME, body);
  body.SetSize(body.GetSize()+1);
  return ok;
}


PBoolean PHTTPClient::ReadContentBody(PMIMEInfo & replyMIME, PBYTEArray & body)
{
  return InternalReadContentBody(replyMIME, body);
}


PBoolean PHTTPClient::InternalReadContentBody(PMIMEInfo & replyMIME, PAbstractArray & body)
{
  PCaselessString encoding = replyMIME(TransferEncodingTag());

  if (encoding != ChunkedTag()) {
    if (replyMIME.Contains(ContentLengthTag())) {
      PINDEX length = replyMIME.GetInteger(ContentLengthTag());
      body.SetSize(length);
      return ReadBlock(body.GetPointer(), length);
    }

    if (!(encoding.IsEmpty())) {
      lastResponseCode = -1;
      lastResponseInfo = "Unknown Transfer-Encoding extension";
      return PFalse;
    }

    // Must be raw, read to end file variety
    static const PINDEX ChunkSize = 2048;
    PINDEX bytesRead = 0;
    while (ReadBlock((char *)body.GetPointer(bytesRead+ChunkSize)+bytesRead, ChunkSize))
      bytesRead += GetLastReadCount();

    body.SetSize(bytesRead + GetLastReadCount());
    return GetErrorCode(LastReadError) == NoError;
  }

  // HTTP1.1 chunked format
  PINDEX bytesRead = 0;
  for (;;) {
    // Read chunk length line
    PString chunkLengthLine;
    if (!ReadLine(chunkLengthLine))
      return PFalse;

    // A zero length chunk is end of output
    PINDEX chunkLength = chunkLengthLine.AsUnsigned(16);
    if (chunkLength == 0)
      break;

    // Read the chunk
    if (!ReadBlock((char *)body.GetPointer(bytesRead+chunkLength)+bytesRead, chunkLength))
      return PFalse;
    bytesRead+= chunkLength;

    // Read the trailing CRLF
    if (!ReadLine(chunkLengthLine))
      return PFalse;
  }

  // Read the footer
  PString footer;
  do {
    if (!ReadLine(footer))
      return PFalse;
  } while (replyMIME.AddMIME(footer));

  return PTrue;
}


PBoolean PHTTPClient::GetTextDocument(const PURL & url,
                                  PString & document,
                                  PBoolean persist)
{
  PMIMEInfo outMIME, replyMIME;
  if (!GetDocument(url, outMIME, replyMIME, persist))
    return PFalse;

  return ReadContentBody(replyMIME, document);
}


PBoolean PHTTPClient::GetDocument(const PURL & _url,
                              PMIMEInfo & _outMIME,
                              PMIMEInfo & replyMIME,
                              PBoolean persist)
{
  int count = 0;
  static const char locationTag[] = "Location";
  PURL url = _url;
  for (;;) {
    PMIMEInfo outMIME = _outMIME;
    replyMIME.RemoveAll();
    PString u = url.AsString();
    int code = ExecuteCommand(GET, url, outMIME, PString(), replyMIME, persist);
    switch (code) {
      case RequestOK:
        return PTrue;
      case MovedPermanently:
      case MovedTemporarily:
        {
          if (count > 10)
            return PFalse;
          PString str = replyMIME(locationTag);
          if (str.IsEmpty())
            return PFalse;
          PString doc;
          if (!ReadContentBody(replyMIME, doc))
            return PFalse;
          url = str;
          count++;
        }
        break;
      default:
        return PFalse;
    }
  }
}


PBoolean PHTTPClient::GetHeader(const PURL & url,
                            PMIMEInfo & outMIME,
                            PMIMEInfo & replyMIME,
                            PBoolean persist)
{
  return ExecuteCommand(HEAD, url, outMIME, PString(), replyMIME, persist) == RequestOK;
}


PBoolean PHTTPClient::PostData(const PURL & url,
                           PMIMEInfo & outMIME,
                           const PString & data,
                           PMIMEInfo & replyMIME,
                           PBoolean persist)
{
  PString dataBody = data;
  if (!outMIME.Contains(ContentTypeTag())) {
    outMIME.SetAt(ContentTypeTag(), "application/x-www-form-urlencoded");
    dataBody += "\r\n"; // Add CRLF for compatibility with some CGI servers.
  }

  return ExecuteCommand(POST, url, outMIME, data, replyMIME, persist) == RequestOK;
}


PBoolean PHTTPClient::PostData(const PURL & url,
                           PMIMEInfo & outMIME,
                           const PString & data,
                           PMIMEInfo & replyMIME,
                           PString & body,
                           PBoolean persist)
{
  if (!PostData(url, outMIME, data, replyMIME, persist))
    return PFalse;

  return ReadContentBody(replyMIME, body);
}


PBoolean PHTTPClient::AssureConnect(const PURL & url, PMIMEInfo & outMIME)
{
  PString host = url.GetHostName();

  // Is not open or other end shut down, restablish connection
  if (!IsOpen()) {
    if (host.IsEmpty()) {
      lastResponseCode = BadRequest;
      lastResponseInfo = "No host specified";
      return SetErrorValues(ProtocolFailure, 0, LastReadError);
    }

#if P_SSL
    if (url.GetScheme() == "https") {
      PTCPSocket * tcp = new PTCPSocket(url.GetPort());
      tcp->SetReadTimeout(readTimeout);
      if (!tcp->Connect(host)) {
        lastResponseCode = -2;
        lastResponseInfo = tcp->GetErrorText();
        delete tcp;
        return PFalse;
      }

      PSSLChannel * ssl = new PSSLChannel;
      if (!ssl->Connect(tcp)) {
        lastResponseCode = -2;
        lastResponseInfo = ssl->GetErrorText();
        delete ssl;
        return PFalse;
      }

      if (!Open(ssl)) {
        lastResponseCode = -2;
        lastResponseInfo = GetErrorText();
        return PFalse;
      }
    }
    else
#endif

    if (!Connect(host, url.GetPort())) {
      lastResponseCode = -2;
      lastResponseInfo = GetErrorText();
      return PFalse;
    }
  }

  // Have connection, so fill in the required MIME fields
  static char HostTag[] = "Host";
  if (!outMIME.Contains(HostTag)) {
    if (!host)
      outMIME.SetAt(HostTag, host);
    else {
      PIPSocket * sock = GetSocket();
      if (sock != NULL)
        outMIME.SetAt(HostTag, sock->GetHostName());
    }
  }

  return PTrue;
}

#endif // P_HTTP


// End Of File ///////////////////////////////////////////////////////////////
