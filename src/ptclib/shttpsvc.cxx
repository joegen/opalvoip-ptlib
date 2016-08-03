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
 *
 * $Revision$
 * $Author$
 * $Date$
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
    PSecureHTTPServiceProcess * m_serviceProcess;
    PCaselessString             m_preReadData;
    enum {
      e_Starting,
      e_IsSSL,
      e_Finished }
    m_preReadState;
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
  PSYSTEMLOG(Info, "SSL disabled");
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


PChannel * PSecureHTTPServiceProcess::CreateChannelForHTTP(PChannel * channel)
{
  if (m_sslContext == NULL)
    return PHTTPServiceProcess::CreateChannelForHTTP(channel);

  PSSLChannel * ssl = new HTTP_PSSLChannel(this, m_sslContext);
  if (ssl->Accept(channel))
    return ssl;

  PSYSTEMLOG(Error, "Accept failed: " << ssl->GetErrorText());
  ssl->Detach();
  delete ssl;
  return NULL;
}


void PSecureHTTPServiceProcess::OnHTTPStarted(PHTTPServer & server)
{
  if (m_sslContext != NULL)
    server.GetConnectionInfo().SetPersistenceMaximumTransations(0);
}


bool PSecureHTTPServiceProcess::SetServerCertificate(const PFilePath & certificateFile,
                                                     bool create,
                                                     const char * dn)
{
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

  return SetServerCredentials(certificateFile, certificateFile, PString::Empty());
}


bool PSecureHTTPServiceProcess::SetServerCredentials(const PString & cert, const PString & key, const PString & ca)
{
  if (m_sslContext == NULL)
    m_sslContext = new PSSLContext(PSSLContext::TLSv1);

  if (m_sslContext->SetCredentials(ca, cert, key))
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

  url.Splice("https://", 0);
  PSYSTEMLOG(Info, "Detected non-SSL connection, host=\"" << host << "\", redirecting to " << url);

  PString str = CreateNonSSLMessage(url);
  
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
  static const char * CRLF = "\r\n";
  return PSTRSTRM("HTTP/1.1 301 Moved Permanently" << CRLF <<
                  PHTTP::LocationTag() << ": " << url << CRLF <<
                  PHTTP::ContentLengthTag() << ": 0" << CRLF <<
                  CRLF);
}

HTTP_PSSLChannel::HTTP_PSSLChannel(PSecureHTTPServiceProcess * svc, PSSLContext * context)
  : PSSLChannel(context)
  , m_serviceProcess(svc)
  , m_preReadState(e_Starting)
{
}


int HTTP_PSSLChannel::BioRead(char * buf, int len)
{
  if (m_preReadState == e_Finished)
    return PSSLChannel::BioRead(buf, len);

  if (m_preReadState == e_Starting) {
    PChannel * chan = GetReadChannel();

    // read first line from the channel
    for (;;) {
      int c = chan->ReadChar();
      if (c < 0)
        return -1;

      m_preReadData += (char)c;

      if (c == '\n' && m_preReadData.Find("HTTP/1") != P_MAX_INDEX) {
        if (m_serviceProcess->OnDetectedNonSSLConnection(chan, m_preReadData))
          chan->Close();
        return -1;
      }

      if (c == '\r' || !iscntrl(c))
        continue;

      if (!m_preReadData.IsEmpty()) {
        m_preReadState = e_IsSSL;
        break;
      }

      m_preReadState = e_Finished;
      *buf = (char)c;
      len = 1;
      return len;
    }
  }

  // copy pre-read bytes to the supplied buffer, but no more than the buffer will allow
  if (len < (int)m_preReadData.GetLength()) {
    memcpy(buf, m_preReadData.GetPointer(), len);
    m_preReadData.Delete(0, len);
  }
  else {
    len = m_preReadData.GetLength();
    memcpy(buf, m_preReadData.GetPointer(), len);
    m_preReadState = e_Finished;
  }
  return len;
}

#endif //P_SSL

#endif // P_HTTPSVC

// End Of File ///////////////////////////////////////////////////////////////
