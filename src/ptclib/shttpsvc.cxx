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
 * Revision 1.2  2001/03/27 03:55:48  craigs
 * Added hack to allow secure servers to act as non-secure servers
 *
 * Revision 1.1  2001/02/15 02:41:14  robertj
 * Added class to do secure HTTP based service process.
 *
 */

#include <ptlib.h>
#include <ptclib/shttpsvc.h>

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

  PSSLChannel * ssl = new PSSLChannel(sslContext);

  if (!ssl->Accept(socket)) {
    PSYSTEMLOG(Error, "HTTPS\tAccept failed: " << ssl->GetErrorText());
    delete ssl;
    return NULL;
  }

  PHTTPServer * server = new PHTTPServer(httpNameSpace);
  server->GetConnectionInfo().SetPersistenceMaximumTransations(0);
  if (server->Open(ssl))
    return server;

  delete server;
  return NULL;
}


BOOL PSecureHTTPServiceProcess::SetServerCertificate(const PFilePath & certificateFile)
{
  return sslContext->UseCertificate(certificateFile) &&
         sslContext->UsePrivateKey(certificateFile);
}


// End Of File ///////////////////////////////////////////////////////////////
