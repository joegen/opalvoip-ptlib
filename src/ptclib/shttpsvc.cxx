/*
 * shttpsvc.cxx
 *
 * Class for secure service applications using HTTPS as the user interface.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: shttpsvc.cxx,v $
 * Revision 1.8  2001/12/13 09:19:32  robertj
 * Added ability to create HTTP server certificate if one does not exist.
 *
 * Revision 1.7  2001/09/10 02:51:23  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.6  2001/08/28 06:44:45  craigs
 * Added ability to override PHTTPServer creation
 *
 * Revision 1.5  2001/05/24 01:01:28  robertj
 * Fixed GNU C++ warning
 *
 * Revision 1.4  2001/05/16 06:02:37  craigs
 * Changed to allow detection of non-SSL connection to SecureHTTPServiceProcess
 *
 * Revision 1.3  2001/05/07 23:27:06  robertj
 * Added SO_LINGER setting to HTTP sockets to help with clearing up sockets
 *   when the application exits, which prevents new run of app as "port in use".
 *
 * Revision 1.2  2001/03/27 03:55:48  craigs
 * Added hack to allow secure servers to act as non-secure servers
 *
 * Revision 1.1  2001/02/15 02:41:14  robertj
 * Added class to do secure HTTP based service process.
 *
 */

#include <ptlib.h>
#include <ptclib/shttpsvc.h>

class HTTP_PSSLChannel : public PSSLChannel
{
  PCLASSINFO(HTTP_PSSLChannel, PSSLChannel);

  public:
    HTTP_PSSLChannel(PSecureHTTPServiceProcess * svc, PSSLContext * context = NULL);

    virtual BOOL RawSSLRead(void * buf, PINDEX & len);

  protected:
    enum { PreRead_Size = 4 };

    PSecureHTTPServiceProcess * svc;
    PINDEX preReadLen;
    char preRead[PreRead_Size];
};

#define new PNEW

#ifdef _DEBUG
BOOL PSecureHTTPServiceProcess::secureServerHack = FALSE;
#endif

PSecureHTTPServiceProcess::PSecureHTTPServiceProcess(const Info & inf)
  : PHTTPServiceProcess(inf)
{
  sslContext = new PSSLContext;
}


PSecureHTTPServiceProcess::~PSecureHTTPServiceProcess()
{
  delete sslContext;
}


PHTTPServer * PSecureHTTPServiceProcess::CreateHTTPServer(PTCPSocket & socket)
{
#ifdef _DEBUG
  if (secureServerHack)
    return PHTTPServiceProcess::CreateHTTPServer(socket);
#endif

#ifdef SO_LINGER
  const linger ling = { 1, 5 };
  socket.SetOption(SO_LINGER, &ling, sizeof(ling));
#endif

  PSSLChannel * ssl = new HTTP_PSSLChannel(this, sslContext);

  if (!ssl->Accept(socket)) {
    PSYSTEMLOG(Error, "HTTPS\tAccept failed: " << ssl->GetErrorText());
    delete ssl;
    return NULL;
  }

  PHTTPServer * server = OnCreateHTTPServer(httpNameSpace);

  server->GetConnectionInfo().SetPersistenceMaximumTransations(0);
  if (server->Open(ssl))
    return server;

  delete server;
  return NULL;
}


BOOL PSecureHTTPServiceProcess::SetServerCertificate(const PFilePath & certificateFile,
                                                     BOOL create,
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
      PTRACE(0, "MTGW\tCould not create certificate");
      return FALSE;
    }
    certificate.Save(certificateFile);
    key.Save(certificateFile, TRUE);
  }

  return sslContext->UseCertificate(certificateFile) &&
         sslContext->UsePrivateKey(certificateFile);
}

BOOL PSecureHTTPServiceProcess::OnDetectedNonSSLConnection(PChannel * chan, const PString & line)
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
      url = addr.AsString() + ":" + PString(PString::Unsigned, httpListeningSocket->GetPort());
    }
  }

  PString str = CreateNonSSLMessage(PString("http://") + url);
  
  chan->WriteString(str);
  chan->Close();

  return FALSE; 
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


BOOL HTTP_PSSLChannel::RawSSLRead(void * buf, PINDEX & len)
{ 
  if (preReadLen == 0)
    return PSSLChannel::RawSSLRead(buf, len); 

  if (preReadLen == P_MAX_INDEX) {
    PChannel * chan = GetReadChannel();

    // read some bytes from the channel
    preReadLen = 0;
    while (preReadLen < PreRead_Size) {
      BOOL b = chan->Read(preRead + preReadLen, PreRead_Size - preReadLen); 
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
        return FALSE;
    }
  }

  // copy some bytes to the returned buffer, but no more than the buffer will allow
  len = PMIN(len, preReadLen);
  memcpy(buf, preRead, len);
  preReadLen -= len;
  return TRUE;
}



// End Of File ///////////////////////////////////////////////////////////////
