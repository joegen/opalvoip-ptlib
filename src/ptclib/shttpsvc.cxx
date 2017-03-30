/*
 * shttpsvc.cxx
 *
 * Class for secure service applications using HTTPS as the user interface.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2001-2002 Equivalence Pty. Ltd.
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
#pragma implementation "shttpsvc.h"
#endif

#include <ptlib.h>

#ifdef P_HTTPSVC

#include <ptclib/shttpsvc.h>

#ifdef P_SSL

class HTTP_PSSLChannel : public PSSLChannel
{
  PCLASSINFO(HTTP_PSSLChannel, PSSLChannel);

  public:
    HTTP_PSSLChannel(PSecureHTTPServiceProcess * svc, PSSLContext * context = NULL);

    virtual int BioRead(char * out, int outl);

  protected:
    enum { PreRead_Size = 4 };

    PSecureHTTPServiceProcess * svc;
    PINDEX preReadLen;
    char preRead[PreRead_Size];
};

#define new PNEW


PSecureHTTPServiceProcess::PSecureHTTPServiceProcess(const Info & inf)
  : PHTTPServiceProcess(inf)
  , m_sslContext(NULL)
{
}


PSecureHTTPServiceProcess::~PSecureHTTPServiceProcess()
{
  delete m_sslContext;
}


void PSecureHTTPServiceProcess::DisableSSL()
{
  delete m_sslContext;
  m_sslContext = NULL;
}


PSecureHTTPServiceProcess::Params::Params(const char * configPageName, const char * section)
  : PHTTPServiceProcess::Params(configPageName, section)
  , m_certificateFileKey("HTTP Certificate")
  , m_createCertificateKey("Create HTTP Certificate")
{
}


bool PSecureHTTPServiceProcess::InitialiseBase(PHTTPServiceProcess::Params & params)
{
  if (!PHTTPServiceProcess::InitialiseBase(params))
    return false;

  if (params.m_configPage == NULL)
    return true;

  Params * secure = dynamic_cast<Params *>(&params);
  if (secure == NULL || secure->m_certificateFileKey == NULL)
    return true;

  // SSL certificate file.
  PString certificateFile = params.m_configPage->AddStringField(secure->m_certificateFileKey, 250, PString::Empty(),
                                                                "Certificate for HTTPS user interface, if empty HTTP is used.", 1, 50);
  if (certificateFile.IsEmpty()) {
    DisableSSL();
    return true;
  }

  bool create = true;
  if (secure->m_createCertificateKey != NULL)
    create = params.m_configPage->AddBooleanField(secure->m_createCertificateKey, true, "Automatically create certificate file if does not exist");

  if (SetServerCertificate(certificateFile, create))
    return true;

  PSYSTEMLOG(Fatal, "Could not load certificate \"" << certificateFile << '"');
  return false;
}


PHTTPServer * PSecureHTTPServiceProcess::CreateHTTPServer(PTCPSocket & socket)
{
  if (m_sslContext == NULL)
    return PHTTPServiceProcess::CreateHTTPServer(socket);

#ifdef SO_LINGER
  const linger ling = { 1, 5 };
  socket.SetOption(SO_LINGER, &ling, sizeof(ling));
#endif

  PSSLChannel * ssl = new HTTP_PSSLChannel(this, m_sslContext);

  if (!ssl->Accept(socket)) {
    PSYSTEMLOG(Error, "Accept failed: " << ssl->GetErrorText());
    delete ssl;
    return NULL;
  }

  PHTTPServer * server = OnCreateHTTPServer(m_httpNameSpace);

  server->GetConnectionInfo().SetPersistenceMaximumTransations(0);
  if (server->Open(ssl))
    return server;

  delete server;
  return NULL;
}


bool PSecureHTTPServiceProcess::SetServerCertificate(const PFilePath & certificateFile,
                                                     bool create,
                                                     const char * dn)
{
  if (m_sslContext == NULL)
    m_sslContext = new PSSLContext;

  if (create && !PFile::Exists(certificateFile)) {
    PSSLPrivateKey key(1024);
    PSSLCertificate certificate;
    PStringStream name;
    if (dn != NULL)
      name << dn;
    else {
      name << "/O=" << GetManufacturer()
           << "/CN=" << GetName() << '@' << PIPSocket::GetHostName();
    }
    if (!certificate.CreateRoot(name, key)) {
      PSYSTEMLOG(Fatal, "Could not create certificate for name \"" << name << '"');
      return false;
    }
    certificate.Save(certificateFile);
    key.Save(certificateFile, true);
  }

  if (m_sslContext->UseCertificate(certificateFile) && m_sslContext->UsePrivateKey(certificateFile))
    return true;

  DisableSSL();
  return false;
}

PBoolean PSecureHTTPServiceProcess::OnDetectedNonSSLConnection(PChannel * chan, const PString & line)
{ 
  // get the MIME info
  PMIMEInfo mime(*chan);

  PString url;

  // get the host field
  PString host = mime("host");
  if (!host.IsEmpty()) {

    // parse the command
    PINDEX pos = line.Find(' ');
    if (pos != P_MAX_INDEX) {
      PString str = line.Mid(pos).Trim();
      pos = str.FindLast(' ');
      if (pos != P_MAX_INDEX)
        url = host + str.Left(pos);
    }
  }

  // no URL was available, return something!
  if (url.IsEmpty()) {
    if (!host.IsEmpty())
      url = host;
    else {
      PIPSocket::Address addr;
      PIPSocket::GetHostAddress(addr);
      url = addr.AsString() + ":" + PString(PString::Unsigned, m_httpListeningSockets.front().GetPort());
    }
  }

  PString str = CreateNonSSLMessage(PString("http://") + url);
  
  chan->WriteString(str);
  chan->Close();

  return false; 
}

PString PSecureHTTPServiceProcess::CreateNonSSLMessage(const PString & url)
{
  PString newUrl = url;
  if (url.Left(5) == "http:")
    newUrl = PString("https:") + newUrl.Mid(5);
  return CreateRedirectMessage(newUrl);
}

PString PSecureHTTPServiceProcess::CreateRedirectMessage(const PString & url)
{
  return PString("HTTP/1.1 301 Moved Permanently\r\n") +
                 "Location: " + url + "\r\n" +
                 "\r\n";
}

HTTP_PSSLChannel::HTTP_PSSLChannel(PSecureHTTPServiceProcess * _svc, PSSLContext * context)
  : PSSLChannel(context), svc(_svc)
{
  preReadLen = P_MAX_INDEX;
}


int HTTP_PSSLChannel::BioRead(char * buf, int len)
{ 
  if (preReadLen == 0)
    return PSSLChannel::BioRead(buf, len); 

  if (preReadLen == P_MAX_INDEX) {
    PChannel * chan = GetReadChannel();

    // read some bytes from the channel
    preReadLen = 0;
    while (preReadLen < PreRead_Size) {
      PBoolean b = chan->Read(preRead + preReadLen, PreRead_Size - preReadLen); 
      if (!b)
        break;
      preReadLen += chan->GetLastReadCount();
    }

    // see if these bytes correspond to a GET or POST
    if (
         (preReadLen == PreRead_Size) &&
         ((strncmp(preRead, "GET", 3) == 0) || (strncmp(preRead, "POST", 4) == 0))
        ) {

      // read in the rest of the line
      PString line(preRead, 4);
      int ch;
      while (((ch = chan->ReadChar()) > 0) && (ch != '\n'))
        line += (char)ch;

      if (!svc->OnDetectedNonSSLConnection(chan, line))
        return -1;
    }
  }

  // copy some bytes to the returned buffer, but no more than the buffer will allow
  len = std::min(len, (int)preReadLen);
  memcpy(buf, preRead, len);
  preReadLen -= len;
  return len;
}

#endif //P_SSL

#endif // P_HTTPSVC

// End Of File ///////////////////////////////////////////////////////////////
