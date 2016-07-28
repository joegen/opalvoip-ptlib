/*
 * shttpsvc.h
 *
 * Class for secure service applications using HTTPS as the user interface.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2001 Equivalence Pty. Ltd.
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

#ifndef PTLIB_SHTTPSVC_H
#define PTLIB_SHTTPSVC_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptclib/httpsvc.h>
#include <ptclib/pssl.h>


/////////////////////////////////////////////////////////////////////

class PSecureHTTPServiceProcess : public PHTTPServiceProcess
{
  PCLASSINFO(PSecureHTTPServiceProcess, PHTTPServiceProcess)

  public:
    PSecureHTTPServiceProcess(const Info & inf);
    ~PSecureHTTPServiceProcess();

    struct Params : PHTTPServiceProcess::Params
    {
      Params(
        const char * configPageName,
        const char * section = GetDefaultSection()
      );

      const char * m_certificateFileKey;
      const char * m_createCertificateKey;
    };

    virtual bool InitialiseBase(
      PHTTPServiceProcess::Params & params
    );

    virtual PChannel * CreateChannelForHTTP(PChannel * channel);
    virtual void OnHTTPStarted(PHTTPServer & server);

    /** Set/Create the server certificate to use.
        Must be called before ListenForHTTP() or https will not be supported.
      */
    bool SetServerCertificate(
      const PFilePath & certFile,   ///< Combined certificate/private key file
      bool create = false,          ///< Flag indicating a self signed certificate should be generated if it does not exist.
      const char * dn = NULL        ///< Distinguished Name to use if creating new self signed certificate.
    );

    /** Set the server certificates to use.
        Must be called before ListenForHTTP() or https will not be supported.
      */
    bool SetServerCertificates(
      const PString & cert,   ///< Certificate file or text string
      const PString & key,    ///< Private key file or text string
      const PString & ca      ///< Certificate authority file, directory or text string
    );

    virtual PBoolean OnDetectedNonSSLConnection(PChannel * chan, const PString & line);

    virtual PString CreateNonSSLMessage(const PString & url);
    virtual PString CreateRedirectMessage(const PString & url);

    void DisableSSL();

  protected:
    PSSLContext * m_sslContext;
};


#endif // PTLIB_SHTTPSVC_H


// End Of File ///////////////////////////////////////////////////////////////
