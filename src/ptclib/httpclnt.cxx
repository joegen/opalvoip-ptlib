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
 */

#include <ptlib.h>

#if P_HTTP

#include <ptlib/sockets.h>
#include <ptclib/http.h>
#include <ptclib/guid.h>

#if P_SSL
#include <ptclib/pssl.h>
#endif

#include <ctype.h>


#define PTraceModule() "HTTP"


////////////////////////////////////////////////////////////////////////////////////

PFACTORY_CREATE(PHTTPClientAuthenticationFactory, PHTTPClientBasicAuthentication, "basic");
PFACTORY_CREATE(PHTTPClientAuthenticationFactory, PHTTPClientDigestAuthentication, "digest");

static const char * const AlgorithmNames[PHTTPClientDigestAuthentication::NumAlgorithms] = {
  "MD5"
};

#define new PNEW


static __inline bool IsOK(int response) { return (response/100) == 2; }

#if PTRACING
PINDEX PHTTPClient::MaxTraceContentSize = 1000;
#endif


//////////////////////////////////////////////////////////////////////////////

struct PHTTPClient_DummyProcessor : public PHTTPContentProcessor
{
  BYTE m_body[4096];

  PHTTPClient_DummyProcessor(bool reader)
    : PHTTPContentProcessor(reader)
  {
  }


  virtual void * GetBuffer(PINDEX & size)
  {
    if (m_reader) {
      size = sizeof(m_body);
      return m_body;
    }

    size = 0;
    return NULL;
  }
};


struct PHTTPClient_StringReader : public PHTTPContentProcessor
{
  PString & m_body;

  PHTTPClient_StringReader(PString & body)
    : PHTTPContentProcessor(true)
    , m_body(body)
  {
  }

  virtual void * GetBuffer(PINDEX & size)
  {
    PINDEX oldLength = m_body.GetLength();
    char * ptr = m_body.GetPointerAndSetLength(oldLength + size);
    return ptr != NULL ? ptr + oldLength : NULL;
  }
};


struct PHTTPClient_StringWriter : public PHTTPContentProcessor
{
  PString m_body;
  bool    m_written;

  PHTTPClient_StringWriter(const PString & body)
    : PHTTPContentProcessor(false)
    , m_body(body)
    , m_written(false)
  {
  }

  virtual void * GetBuffer(PINDEX & size)
  {
    if (m_written)
      return NULL;

    m_written = true;
    size = m_body.GetLength();
    return m_body.PCharArray::GetPointer();
  }

  virtual void Reset()
  {
    m_written = false;
  }
};


struct PHTTPClient_BinaryReader : public PHTTPContentProcessor
{
  PBYTEArray & m_body;

  PHTTPClient_BinaryReader(PBYTEArray & body)
    : PHTTPContentProcessor(true)
    , m_body(body)
  {
  }

  virtual void * GetBuffer(PINDEX & size)
  {
    PINDEX oldSize = m_body.GetSize();
    BYTE * ptr = m_body.GetPointer(oldSize+size);
    return ptr != NULL ? ptr+oldSize : NULL;
  }
};


struct PHTTPClient_BinaryWriter : public PHTTPContentProcessor
{
  PBYTEArray m_body;
  bool       m_written;

  PHTTPClient_BinaryWriter(const PBYTEArray & body)
    : PHTTPContentProcessor(false)
    , m_body(body)
    , m_written(false)
  {
  }

  virtual void * GetBuffer(PINDEX & size)
  {
    if (m_written)
      return NULL;

    m_written = true;
    size = m_body.GetLength();
    return m_body.GetPointer();
  }

  virtual void Reset()
  {
    m_written = false;
  }
};


struct PHTTPClient_FileWriter : public PHTTPContentProcessor
{
  PFile   m_file;
  uint8_t m_buffer[4096];

  PHTTPClient_FileWriter(const PFilePath & path)
    : PHTTPContentProcessor(false)
    , m_file(path, PFile::ReadOnly)
  {
  }

  virtual void * GetBuffer(PINDEX & size)
  {
    if (!m_file.IsOpen())
      return NULL;

    off_t left = m_file.GetLength() - m_file.GetPosition();
    size = left > (off_t)sizeof(m_buffer) ? (PINDEX)sizeof(m_buffer) : (PINDEX)left;
    if (m_file.Read(m_buffer, size))
      return m_buffer;

    return NULL;
  }

  virtual void Reset()
  {
    m_file.Open();
  }
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPClient

PHTTPClient::PHTTPClient(const PString & userAgent)
  : m_userAgentName(userAgent)
  , m_persist(true)
  , m_authentication(NULL)
{
}


PHTTPClient::~PHTTPClient()
{
  delete m_authentication;
}


int PHTTPClient::ExecuteCommand(Commands cmd,
                                const PURL & url,
                                PMIMEInfo & outMIME,
                                const PString & dataBody,
                                PMIMEInfo & replyMime)
{
  PHTTPClient_StringWriter processor(dataBody);
  return ExecuteCommand(cmd, url, outMIME, processor, replyMime);
}


int PHTTPClient::ExecuteCommand(Commands cmd,
                                const PURL & url,
                                PMIMEInfo & outMIME,
                                const PBYTEArray & dataBody,
                                PMIMEInfo & replyMIME)
{
  PHTTPClient_BinaryWriter processor(dataBody);
  return ExecuteCommand(cmd, url, outMIME, processor, replyMIME);
}


int PHTTPClient::ExecuteCommand(Commands cmd,
                                const PURL & url,
                                PMIMEInfo & outMIME,
                                ContentProcessor & processor,
                                PMIMEInfo & replyMIME)
{
  if (!outMIME.Contains(DateTag()))
    outMIME.SetAt(DateTag(), PTime().AsString());

  if (!m_userAgentName && !outMIME.Contains(UserAgentTag()))
    outMIME.SetAt(UserAgentTag(), m_userAgentName);

  if (m_persist)
    outMIME.SetAt(ConnectionTag(), KeepAliveTag());

  bool needAuthentication = true;
  PURL adjustableURL = url;
  for (int retry = 3; retry > 0; --retry) {
    if (!ConnectURL(adjustableURL))
      break;

    // Have connection, so fill in the required MIME fields
    if (!outMIME.Contains(HostTag)) {
      if (url.GetHostName().IsEmpty())
        outMIME.SetAt(HostTag, "localhost");
      else
        outMIME.SetAt(HostTag, url.GetHostPort());
    }

    if (!WriteCommand(cmd, url.AsString(PURL::RelativeOnly), outMIME, processor)) {
      lastResponseCode = TransportWriteError;
      lastResponseInfo = GetErrorText(LastWriteError);
      lastResponseInfo.sprintf(" (errno=%i)", GetErrorNumber(LastWriteError));
      break;
    }

    // If not persisting need to shut down write so other end stops reading
    if (!m_persist)
      Shutdown(ShutdownWrite);

    // Await a response, if all OK exit loop
    if (ReadResponse(replyMIME) && (lastResponseCode != Continue || ReadResponse(replyMIME))) {
      if (IsOK(lastResponseCode))
        return lastResponseCode;

      switch (lastResponseCode) {
        case MovedPermanently:
        case MovedTemporarily:
          adjustableURL = replyMIME("Location");
          if (adjustableURL.IsEmpty())
            continue;
          break;

        case UnAuthorised:
          if (needAuthentication && replyMIME.Contains("WWW-Authenticate") && !(m_userName.IsEmpty() && m_password.IsEmpty())) {
            // authenticate 
            PString errorMsg;
            PHTTPClientAuthentication * newAuth = PHTTPClientAuthentication::ParseAuthenticationRequired(false, replyMIME, errorMsg);
            if (newAuth != NULL) {
              newAuth->SetUsername(m_userName);
              newAuth->SetPassword(m_password);

              delete m_authentication;
              m_authentication = newAuth;
              needAuthentication = false;
              continue;
            }

            lastResponseInfo += " - " + errorMsg;
          }
          // Do next case

        default:
          break;
      }

      retry = 0; // No more retries
    }
    else {
      // If not persisting, we have no oppurtunity to write again, just error out
      if (!m_persist)
        break;

      // ... we close the channel and allow ConnectURL() to reopen it.
      Close();
    }
  }

  PTRACE_IF(3, !IsOK(lastResponseCode), "Error " << lastResponseCode << ' ' << lastResponseInfo);
  return lastResponseCode;
}


bool PHTTPClient::WriteCommand(Commands cmd,
                        const PString & url,
                            PMIMEInfo & outMIME,
                     ContentProcessor & processor)
{
  if (!outMIME.Contains(ContentLengthTag())) {
    processor.Reset();
    uint64_t total = 0;
    PINDEX len;
    while (processor.GetBuffer(len) != NULL)
      total += len;
    outMIME.SetVar(ContentLengthTag(), total);
  }

  PString cmdName = commandNames[cmd];

  if (m_authentication != NULL) {
    PHTTPClientAuthenticator auth(cmdName, url, outMIME, processor);
    m_authentication->Authorise(auth);
  }

#if PTRACING
  if (PTrace::CanTrace(3)) {
    ostream & strm = PTRACE_BEGIN(3);
    strm << "Sending ";
    if (PTrace::CanTrace(4))
      strm << '\n';
    strm << cmdName << ' ';
    if (url.IsEmpty())
      strm << '/';
    else
      strm << url;
    if (PTrace::CanTrace(4))
      strm << '\n' << outMIME;
    strm << PTrace::End;
  }
#endif

  *this << cmdName << ' ' << (url.IsEmpty() ? "/" : (const char*)url) << " HTTP/1.1\r\n"
        << setfill('\r') << outMIME;

  processor.Reset();
  const void * data;
  PINDEX len = 0;
  while ((data = processor.GetBuffer(len)) != NULL) {
    if (!Write(data, len))
      return false;
  }

  return true;
}


bool PHTTPClient::ReadResponse(PMIMEInfo & replyMIME)
{
  PString http = ReadString(7);
  if (!http) {
    UnRead(http);

    if (http.Find("HTTP/") == P_MAX_INDEX) {
      lastResponseCode = PHTTP::RequestOK;
      lastResponseInfo = "HTTP/0.9";
      PTRACE(3, "Read HTTP/0.9 OK");
      return true;
    }

    if (http[0] == '\n')
      ReadString(1);
    else if (http[0] == '\r' &&  http[1] == '\n')
      ReadString(2);

    if (PHTTP::ReadResponse()) {
      bool readOK = replyMIME.Read(*this);

      PString body;
      if (lastResponseCode >= 300) {
#if PTRACING
        if (replyMIME.GetVar(ContentLengthTag(), numeric_limits<int64_t>::max()) <= (int64_t)MaxTraceContentSize)
          ReadContentBody(replyMIME, body);
        else
#endif
          ReadContentBody(replyMIME); // Waste body
      }

#if PTRACING
      if (PTrace::CanTrace(3)) {
        ostream & strm = PTRACE_BEGIN(3);
        strm << "Response ";
        if (PTrace::CanTrace(4))
          strm << '\n';
        strm << lastResponseCode << ' ' << lastResponseInfo;
        if (PTrace::CanTrace(4)) {
          strm << '\n' << replyMIME;
          if (!body.IsEmpty())
            strm << body;
        }
        strm << PTrace::End;
      }
#endif

      if (!body.IsEmpty())
        lastResponseInfo += '\n' + body;

      if (readOK)
        return true;
    }
  }
 
  lastResponseCode = TransportReadError;
  if (GetErrorCode(LastReadError) != NoError) {
    lastResponseInfo = GetErrorText(LastReadError);
    lastResponseInfo.sprintf(" (errno=%i)", GetErrorNumber(LastReadError));
  }
  else {
    lastResponseInfo = "Premature shutdown";
    SetErrorValues(ProtocolFailure, 0, LastReadError);
  }

  return false;
}


bool PHTTPClient::ReadContentBody(PMIMEInfo & replyMIME)
{
  PHTTPClient_DummyProcessor processor(true);
  return ReadContentBody(replyMIME, processor);
}


bool PHTTPClient::ReadContentBody(PMIMEInfo & replyMIME, PString & body)
{
  PHTTPClient_StringReader processor(body);
  return ReadContentBody(replyMIME, processor);
}


bool PHTTPClient::ReadContentBody(PMIMEInfo & replyMIME, PBYTEArray & body)
{
  PHTTPClient_BinaryReader processor(body);
  return ReadContentBody(replyMIME, processor);
}


bool PHTTPClient::ReadContentBody(PMIMEInfo & replyMIME, ContentProcessor & processor)
{
  PCaselessString encoding = replyMIME(TransferEncodingTag());

  if (encoding != ChunkedTag()) {
    if (replyMIME.Contains(ContentLengthTag())) {
      PINDEX length = replyMIME.GetInteger(ContentLengthTag());

      PINDEX size = length;
      void * ptr = processor.GetBuffer(size);
      if (ptr == NULL) {
        lastResponseCode = ContentProcessorError;
        lastResponseInfo = "No buffer from HTTP content processor";
        return false;
      }

      if (length == size)
        return ReadBlock(ptr, length);

      while (length > 0 && Read(ptr, PMIN(length, size))) {
        if (!processor.Process(ptr, GetLastReadCount())) {
          lastResponseCode = ContentProcessorError;
          lastResponseInfo = "Content processing error";
          return false;
        }
        length -= GetLastReadCount();
      }
      return true;
    }

    if (!(encoding.IsEmpty())) {
      lastResponseCode = UnknownTransferEncoding;
      lastResponseInfo = "Unknown Transfer-Encoding extension";
      return false;
    }

    PINDEX size = 8192;
    void * ptr = processor.GetBuffer(size);
    if (ptr == NULL) {
      lastResponseCode = ContentProcessorError;
      lastResponseInfo = "No buffer from HTTP content processor";
      return false;
    }

    // Must be raw, read to end file variety
    while (Read(ptr, size)) {
      if (!processor.Process(ptr, GetLastReadCount())) {
        lastResponseCode = ContentProcessorError;
        lastResponseInfo = "Content processing error";
        return false;
      }
    }

    return GetErrorCode(LastReadError) == NoError;
  }

  // HTTP1.1 chunked format
  for (;;) {
    // Read chunk length line
    PString chunkLengthLine;
    if (!ReadLine(chunkLengthLine))
      return false;

    // A zero length chunk is end of output
    PINDEX chunkLength = chunkLengthLine.AsUnsigned(16);
    if (chunkLength == 0)
      break;

    PINDEX size = chunkLength;
    void * ptr = processor.GetBuffer(size);
    if (ptr == NULL) {
      lastResponseCode = ContentProcessorError;
      lastResponseInfo = "No buffer from HTTP content processor";
      return false;
    }

    if (chunkLength == size) {
      if (!ReadBlock(ptr, chunkLength))
        return false;
    }
    else {
      // Read the chunk
      while (chunkLength > 0 && Read(ptr, PMIN(chunkLength, size))) {
        if (!processor.Process(ptr, GetLastReadCount())) {
          lastResponseCode = ContentProcessorError;
          lastResponseInfo = "Content processing error";
          return false;
        }
        chunkLength -= GetLastReadCount();
      }
    }

    // Read the trailing CRLF
    if (!ReadLine(chunkLengthLine))
      return false;
  }

  // Read the footer
  PString footer;
  do {
    if (!ReadLine(footer))
      return false;
  } while (replyMIME.AddMIME(footer));

  return true;
}


static bool CheckContentType(const PMIMEInfo & replyMIME, const PString & requiredContentType)
{
  PCaselessString actualContentType = replyMIME(PHTTPClient::ContentTypeTag());
  if (requiredContentType.IsEmpty() || actualContentType.IsEmpty() ||
      actualContentType.NumCompare(requiredContentType, requiredContentType.Find(';')) == PObject::EqualTo)
    return true;

  PTRACE(2, "Incorrect Content-Type for document: expecting "
         << requiredContentType << ", got " << actualContentType);
  return false;
}


bool PHTTPClient::GetTextDocument(const PURL & url,
                                     PString & document,
                               const PString & requiredContentType)
{
  PMIMEInfo outMIME, replyMIME;
  if (!GetDocument(url, outMIME, replyMIME))
    return false;

  if (!CheckContentType(replyMIME, requiredContentType)) {
    ReadContentBody(replyMIME); // Waste body
    return false;
  }

  if (!ReadContentBody(replyMIME, document)) {
    PTRACE(2, "Read of body failed");
    return false;
  }

  PTRACE_IF(4, !document.IsEmpty(), "Received body:\n"
            << document.Left(MaxTraceContentSize) << (document.GetLength() > MaxTraceContentSize ? "\n...." : ""));
  return true;
}


bool PHTTPClient::GetBinaryDocument(const PURL & url,
                                    PBYTEArray & document,
                                    const PString & requiredContentType)
{
  PMIMEInfo outMIME, replyMIME;
  if (!GetDocument(url, outMIME, replyMIME))
    return false;

  if (!CheckContentType(replyMIME, requiredContentType)) {
    ReadContentBody(replyMIME); // Waste body
    return false;
  }

  if (!ReadContentBody(replyMIME, document)) {
    PTRACE(2, "Read of body failed");
    return false;
  }

  PTRACE_IF(4, !document.IsEmpty(), "Received " << document.GetSize() << " byte body\n");
  return true;
}


bool PHTTPClient::GetDocument(const PURL & url, ContentProcessor & processor)
{
  PMIMEInfo outMIME, replyMIME;
  return IsOK(ExecuteCommand(GET, url, outMIME, PString::Empty(), replyMIME)) &&
         ReadContentBody(replyMIME, processor);
}


bool PHTTPClient::GetDocument(const PURL & url, PMIMEInfo & replyMIME)
{
  PMIMEInfo outMIME;
  return IsOK(ExecuteCommand(GET, url, outMIME, PString::Empty(), replyMIME));
}


bool PHTTPClient::GetDocument(const PURL & url, PMIMEInfo & outMIME, PMIMEInfo & replyMIME)
{
  return IsOK(ExecuteCommand(GET, url, outMIME, PString::Empty(), replyMIME));
}


bool PHTTPClient::GetHeader(const PURL & url, PMIMEInfo & replyMIME)
{
  PMIMEInfo outMIME;
  return IsOK(ExecuteCommand(HEAD, url, outMIME, PString::Empty(), replyMIME));
}


bool PHTTPClient::GetHeader(const PURL & url, PMIMEInfo & outMIME, PMIMEInfo & replyMIME)
{
  return IsOK(ExecuteCommand(HEAD, url, outMIME, PString::Empty(), replyMIME));
}


bool PHTTPClient::PostData(const PURL & url, const PStringToString & data)
{
  PStringStream entityBody;
  PURL::OutputVars(entityBody, data, '\0', '&', '=', PURL::QueryTranslation);
  entityBody << "\r\n"; // Add CRLF for compatibility with some CGI servers.

  PMIMEInfo outMIME;
  return PostData(url, outMIME, entityBody);
}


bool PHTTPClient::PostData(const PURL & url, PMIMEInfo & outMIME, const PString & data)
{
  PMIMEInfo replyMIME;
  return PostData(url, outMIME, data, replyMIME) && ReadContentBody(replyMIME);
}


bool PHTTPClient::PostData(const PURL & url,
                            PMIMEInfo & outMIME,
                        const PString & data,
                            PMIMEInfo & replyMIME)
{
  if (!outMIME.Contains(ContentTypeTag()))
    outMIME.SetAt(ContentTypeTag(), "application/x-www-form-urlencoded");

  return IsOK(ExecuteCommand(POST, url, outMIME, data, replyMIME));
}


bool PHTTPClient::PostData(const PURL & url,
                            PMIMEInfo & outMIME,
                        const PString & data,
                            PMIMEInfo & replyMIME,
                              PString & body)
{
  return PostData(url, outMIME, data, replyMIME) && ReadContentBody(replyMIME, body);
}


bool PHTTPClient::PutDocument(const PURL & url, const PString & data, const PString & contentType, const PMIMEInfo & mime)
{
  PMIMEInfo outMIME(mime), replyMIME;
  outMIME.SetAt(ContentTypeTag(), contentType);
  return IsOK(ExecuteCommand(PUT, url, outMIME, data, replyMIME));
}


bool PHTTPClient::PutDocument(const PURL & url, const PBYTEArray & data, const PString & contentType, const PMIMEInfo & mime)
{
  PMIMEInfo outMIME(mime), replyMIME;
  outMIME.SetAt(ContentTypeTag(), contentType);
  return IsOK(ExecuteCommand(PUT, url, outMIME, data, replyMIME));
}


bool PHTTPClient::PutDocument(const PURL & url, const PFilePath & path, const PString & contentType, const PMIMEInfo & mime)
{
  PMIMEInfo outMIME(mime), replyMIME;
  outMIME.SetAt(ContentTypeTag(), contentType.IsEmpty() ? PMIMEInfo::GetContentType(path.GetType()) : contentType);
  PHTTPClient_FileWriter processor(path);
  return IsOK(ExecuteCommand(PUT, url, outMIME, processor, replyMIME));
}


bool PHTTPClient::DeleteDocument(const PURL & url)
{
  PMIMEInfo outMIME, replyMIME;
  return IsOK(ExecuteCommand(DELETE, url, outMIME, PString::Empty(), replyMIME));
}


bool PHTTPClient::ConnectURL(const PURL & url)
{
  if (IsOpen())
    return true;

  PString host = url.GetHostName();

  // Is not open or other end shut down, restablish connection
  if (host.IsEmpty()) {
    lastResponseCode = BadRequest;
    lastResponseInfo = "No host specified";
    return SetErrorValues(ProtocolFailure, 0, LastReadError);
  }

#if P_SSL
  if (url.GetScheme() == "https") {
    PSSLChannel * ssl = NULL;
    PSSLContext::Method method = PSSLContext::HighestTLS;
    for (;;) {
      PTCPSocket * tcp = new PTCPSocket(url.GetPort());
      tcp->SetReadTimeout(readTimeout);
      if (!tcp->Connect(host)) {
        lastResponseCode = TransportConnectError;
        lastResponseInfo = tcp->GetErrorText();
        lastResponseInfo.sprintf(" (errno=%i)", tcp->GetErrorNumber());
        delete tcp;
        return false;
      }

      PSSLContext * context = new PSSLContext(method);
      if (!context->SetCredentials(m_authority, m_certificate, m_privateKey)) {
        lastResponseCode = TransportConnectError;
        lastResponseInfo = "Could not set certificates";
        delete context;
        delete tcp;
        return false;
      }

      ssl = new PSSLChannel(context, true);
      if (ssl->Connect(tcp))
        break;

      if ((unsigned)ssl->GetErrorNumber() != 0x9408f10b || method <= PSSLContext::BeginMethod) {
        lastResponseCode = TransportConnectError;
        lastResponseInfo = ssl->GetErrorText();
        delete ssl;
        return false;
      }

      delete ssl;
      --method;
    }

    if (!Open(ssl)) {
      lastResponseCode = TransportConnectError;
      lastResponseInfo = GetErrorText();
      return false;
    }
  }
  else
#endif

  if (!Connect(host, url.GetPort())) {
    lastResponseCode = TransportConnectError;
    lastResponseInfo = GetErrorText();
    lastResponseInfo.sprintf(" (errno=%i)", GetErrorNumber());
    return false;
  }

  PTRACE(5, "Connected to " << host);
  return true;
}


void PHTTPClient::SetAuthenticationInfo(const PString & userName,const PString & password)
{
  m_userName = userName;
  m_password = password;
}


#if P_SSL
void PHTTPClient::SetSSLCredentials(const PString & authority, const PString & certificate, const PString & privateKey)
{
  m_authority = authority;
  m_certificate = certificate;
  m_privateKey = privateKey;
}
#endif

////////////////////////////////////////////////////////////////////////////////////

PHTTPClientAuthentication::PHTTPClientAuthentication()
{
  isProxy = false;
}


PObject::Comparison PHTTPClientAuthentication::Compare(const PObject & other) const
{
  const PHTTPClientAuthentication * otherAuth = dynamic_cast<const PHTTPClientAuthentication *>(&other);
  if (otherAuth == NULL)
    return LessThan;

  Comparison result = GetUsername().Compare(otherAuth->GetUsername());
  if (result != EqualTo)
    return result;

  return GetPassword().Compare(otherAuth->GetPassword());
}


PString PHTTPClientAuthentication::GetAuthParam(const PString & auth, const char * name) const
{
  PString value;

  PINDEX pos = auth.Find(name);
  if (pos != P_MAX_INDEX)  {
    pos += (int)strlen(name);
    while (isspace(auth[pos]) || (auth[pos] == ','))
      pos++;
    if (auth[pos] == '=') {
      pos++;
      while (isspace(auth[pos]))
        pos++;
      if (auth[pos] == '"') {
        pos++;
        value = auth(pos, auth.Find('"', pos)-1);
      }
      else {
        PINDEX base = pos;
        while (auth[pos] != '\0' && !isspace(auth[pos]) && (auth[pos] != ','))
          pos++;
        value = auth(base, pos-1);
      }
    }
  }

  return value;
}

PString PHTTPClientAuthentication::AsHex(PMessageDigest5::Code & digest) const
{
  PStringStream out;
  out << hex << setfill('0');
  for (PINDEX i = 0; i < 16; i++)
    out << setw(2) << (unsigned)((BYTE *)&digest)[i];
  return out;
}

PString PHTTPClientAuthentication::AsHex(const PBYTEArray & data) const
{
  PStringStream out;
  out << hex << setfill('0');
  for (PINDEX i = 0; i < data.GetSize(); i++)
    out << setw(2) << (unsigned)data[i];
  return out;
}

////////////////////////////////////////////////////////////////////////////////////

PHTTPClientBasicAuthentication::PHTTPClientBasicAuthentication()
{
}


PObject::Comparison PHTTPClientBasicAuthentication::Compare(const PObject & other) const
{
  const PHTTPClientBasicAuthentication * otherAuth = dynamic_cast<const PHTTPClientBasicAuthentication *>(&other);
  if (otherAuth == NULL)
    return LessThan;

  return PHTTPClientAuthentication::Compare(other);
}

PBoolean PHTTPClientBasicAuthentication::Parse(const PString & /*auth*/, PBoolean /*proxy*/)
{
  return true;
}

PBoolean PHTTPClientBasicAuthentication::Authorise(AuthObject & authObject) const
{
  PBase64 digestor;
  digestor.StartEncoding();
  digestor.ProcessEncoding(username + ":" + password);
  PString result = digestor.CompleteEncoding();

  PStringStream auth;
  auth << "Basic " << result;

  authObject.GetMIME().SetAt(isProxy ? "Proxy-Authorization" : "Authorization", auth);
  return true;
}

////////////////////////////////////////////////////////////////////////////////////

PHTTPClientDigestAuthentication::PHTTPClientDigestAuthentication()
  : algorithm(NumAlgorithms)
  , qopAuth(false)
  , qopAuthInt(false)
  , stale(false)
  , nonceCount(0)
{
}

PHTTPClientDigestAuthentication & PHTTPClientDigestAuthentication::operator =(const PHTTPClientDigestAuthentication & auth)
{
  isProxy   = auth.isProxy;
  authRealm = auth.authRealm;
  username  = auth.username;
  password  = auth.password;
  nonce     = auth.nonce;
  opaque    = auth.opaque;
          
  qopAuth    = auth.qopAuth;
  qopAuthInt = auth.qopAuthInt;
  stale      = auth.stale;
  cnonce     = auth.cnonce;
  nonceCount = (uint32_t)auth.nonceCount;

  return *this;
}

PObject::Comparison PHTTPClientDigestAuthentication::Compare(const PObject & other) const
{
  const PHTTPClientDigestAuthentication * otherAuth = dynamic_cast<const PHTTPClientDigestAuthentication *>(&other);
  if (otherAuth == NULL)
    return LessThan;

  if (stale || otherAuth->stale)
    return LessThan;

  if (algorithm < otherAuth->algorithm)
    return LessThan;
  if (algorithm > otherAuth->algorithm)
    return GreaterThan;

  Comparison result = authRealm.Compare(otherAuth->authRealm);
  if (result != EqualTo)
    return result;

  return PHTTPClientAuthentication::Compare(other);
}

PBoolean PHTTPClientDigestAuthentication::Parse(const PString & p_auth, PBoolean proxy)
{
  PCaselessString auth = p_auth;

  authRealm.MakeEmpty();
  nonce.MakeEmpty();
  opaque.MakeEmpty();
  algorithm = NumAlgorithms;

  qopAuth = qopAuthInt = false;
  cnonce.MakeEmpty();
  nonceCount = 1;

  if (auth.Find("digest") == P_MAX_INDEX) {
    PTRACE(1, "Digest auth does not contian digest keyword");
    return false;
  }

  algorithm = Algorithm_MD5;  // default
  PCaselessString str = GetAuthParam(auth, "algorithm");
  if (!str.IsEmpty()) {
    while (str != AlgorithmNames[algorithm]) {
      algorithm = (Algorithm)(algorithm+1);
      if (algorithm >= PHTTPClientDigestAuthentication::NumAlgorithms) {
        PTRACE(1, "Unknown digest algorithm " << str);
        return false;
      }
    }
  }

  authRealm = GetAuthParam(auth, "realm");
  if (authRealm.IsEmpty()) {
    PTRACE(1, "No realm in authentication");
    return false;
  }

  nonce = GetAuthParam(auth, "nonce");
  if (nonce.IsEmpty()) {
    PTRACE(1, "No nonce in authentication");
    return false;
  }

  opaque = GetAuthParam(auth, "opaque");
  if (!opaque.IsEmpty()) {
    PTRACE(2, "Authentication contains opaque data");
  }

  PString qopStr = GetAuthParam(auth, "qop");
  if (!qopStr.IsEmpty()) {
    PTRACE(3, "Authentication contains qop-options " << qopStr);
    PStringList options = qopStr.Tokenise(',', true);
    qopAuth    = options.GetStringsIndex("auth") != P_MAX_INDEX;
    qopAuthInt = options.GetStringsIndex("auth-int") != P_MAX_INDEX;
    cnonce = PGloballyUniqueID().AsString();
  }

  PCaselessString staleStr = GetAuthParam(auth, "stale");
  PTRACE_IF(3, !staleStr.IsEmpty(), "Authentication contains stale flag \"" << staleStr << '"');
  stale = staleStr.Find("true") != P_MAX_INDEX;

  isProxy = proxy;
  return true;
}


PBoolean PHTTPClientDigestAuthentication::Authorise(AuthObject & authObject) const
{
  PTRACE(3, "Adding authentication information");

  PMessageDigest5 digestor;
  PMessageDigest5::Code a1, a2, entityBodyCode, response;

  PString uriText = authObject.GetURI();
  PINDEX pos = uriText.Find(";");
  if (pos != P_MAX_INDEX)
    uriText = uriText.Left(pos);

  digestor.Start();
  digestor.Process(username);
  digestor.Process(":");
  digestor.Process(authRealm);
  digestor.Process(":");
  digestor.Process(password);
  digestor.Complete(a1);

  if (qopAuthInt) {
    digestor.Start();
    PHTTPContentProcessor * processor = authObject.GetContentProcessor();
    if (processor == NULL)
      digestor.Process(authObject.GetEntityBody());
    else {
      processor->Reset();
      void * data;
      PINDEX len;
      while ((data = processor->GetBuffer(len)) != NULL)
        digestor.Process(data, len);
    }
    digestor.Complete(entityBodyCode);
  }

  digestor.Start();
  digestor.Process(authObject.GetMethod());
  digestor.Process(":");
  digestor.Process(uriText);
  if (qopAuthInt) {
    digestor.Process(":");
    digestor.Process(AsHex(entityBodyCode));
  }
  digestor.Complete(a2);

  PStringStream auth;
  auth << "Digest "
          "username=\"" << username << "\", "
          "realm=\"" << authRealm << "\", "
          "nonce=\"" << nonce << "\", "
          "uri=\"" << uriText << "\", "
          "algorithm=" << AlgorithmNames[algorithm];

  digestor.Start();
  digestor.Process(AsHex(a1));
  digestor.Process(":");
  digestor.Process(nonce);
  digestor.Process(":");

  if (qopAuthInt || qopAuth) {
    PString nc(psprintf("%08x", (unsigned int)nonceCount));
    ++nonceCount;
    PString qop;
    if (qopAuthInt)
      qop = "auth-int";
    else
      qop = "auth";
    digestor.Process(nc);
    digestor.Process(":");
    digestor.Process(cnonce);
    digestor.Process(":");
    digestor.Process(qop);
    digestor.Process(":");
    digestor.Process(AsHex(a2));
    digestor.Complete(response);
    auth << ", "
         << "response=\"" << AsHex(response) << "\", "
         << "cnonce=\"" << cnonce << "\", "
         << "nc=" << nc << ", "
         << "qop=" << qop;
  }
  else {
    digestor.Process(AsHex(a2));
    digestor.Complete(response);
    auth << ", response=\"" << AsHex(response) << "\"";
  }

  if (!opaque.IsEmpty())
    auth << ", opaque=\"" << opaque << "\"";

  authObject.GetMIME().SetAt(isProxy ? "Proxy-Authorization" : "Authorization", auth);
  return true;
}


static void AuthError(PString & errorMsg, const char * errText, const PString & scheme)
{
  if (!errorMsg.IsEmpty())
    errorMsg += ", ";
  errorMsg += errText;
  errorMsg += " scheme \"";
  errorMsg += scheme;
  errorMsg += '"';
}


PHTTPClientAuthentication * PHTTPClientAuthentication::ParseAuthenticationRequired(bool isProxy, const PMIMEInfo & replyMIME, PString & errorMsg)
{
  PStringArray lines = replyMIME(isProxy ? "Proxy-Authenticate" : "WWW-Authenticate").Lines();

  // find authentication
  for (PINDEX i = 0; i < lines.GetSize(); ++i) {
    PString line = lines[i];
    PString scheme = line.Left(line.Find(' ')).Trim().ToLower();
    PHTTPClientAuthentication * newAuth = PHTTPClientAuthenticationFactory::CreateInstance(scheme);
    if (newAuth == NULL)
      AuthError(errorMsg, "Unknown authentication", scheme);
    else {
      // parse the new authentication scheme
      if (newAuth->Parse(line, isProxy))
        return newAuth;

      delete newAuth;
      AuthError(errorMsg, "Failed to parse authentication for", scheme);
    }
  }

  return NULL;
}


////////////////////////////////////////////////////////////////////////////////////

PHTTPClientAuthenticator::PHTTPClientAuthenticator(const PString & method, const PString & uri, PMIMEInfo & mime, PHTTPContentProcessor & processor)
  : m_method(method)
  , m_uri(uri)
  , m_mime(mime)
  , m_contentProcessor(processor)
{
}

PMIMEInfo & PHTTPClientAuthenticator::GetMIME()
{
  return m_mime;
}

PString PHTTPClientAuthenticator::GetURI()
{
  return m_uri;
}

PHTTPContentProcessor * PHTTPClientAuthenticator::GetContentProcessor()
{
  return &m_contentProcessor;
}

PString PHTTPClientAuthenticator::GetMethod()
{
  return m_method;
}

////////////////////////////////////////////////////////////////////////////////////

#undef new

class PURL_HttpLoader : public PURLLoader
{
    PCLASSINFO(PURL_HttpLoader, PURLLoader);
  public:
    virtual bool Load(PString & str, const PURL & url, const PURL::LoadParams & params) const
    {
      PHTTPClient http;
      http.SetPersistent(false);
      http.SetReadTimeout(params.m_timeout);
      http.SetAuthenticationInfo(params.m_username, params.m_password);
#if P_SSL
      http.SetSSLCredentials(params.m_authority, params.m_certificate, params.m_privateKey);
#endif
      return http.GetTextDocument(url, str, params.m_requiredContentType);
    }

    virtual bool Load(PBYTEArray & data, const PURL & url, const PURL::LoadParams & params) const
    {
      PHTTPClient http;
      http.SetPersistent(false);
      http.SetReadTimeout(params.m_timeout);
      http.SetAuthenticationInfo(params.m_username, params.m_password);
#if P_SSL
      http.SetSSLCredentials(params.m_authority, params.m_certificate, params.m_privateKey);
#endif
      PMIMEInfo outMIME = params.m_customOptions, replyMIME;
      if (!http.GetDocument(url, outMIME, replyMIME))
        return false;

      PCaselessString actualContentType = replyMIME(PHTTP::ContentTypeTag());
      if (!params.m_requiredContentType.IsEmpty() && !actualContentType.IsEmpty() &&
            actualContentType.NumCompare(params.m_requiredContentType, params.m_requiredContentType.Find(';')) != EqualTo) {
        PTRACE(2, "Incorrect Content-Type for document: expecting " << params.m_requiredContentType << ", got " << actualContentType);
        return false;
      }

      return http.ReadContentBody(replyMIME, data);
    }
};

PFACTORY_CREATE(PURLLoaderFactory, PURL_HttpLoader, "http", true);
#if P_SSL
PFACTORY_SYNONYM(PURLLoaderFactory, PURL_HttpLoader, https, "https");
#endif

#endif // P_HTTP


// End Of File ///////////////////////////////////////////////////////////////
