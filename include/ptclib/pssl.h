/*
 * pssl.h
 *
 * Secure Sockets Layer channel interface class.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pssl.h,v $
 * Revision 1.4  1998/09/23 06:19:50  robertj
 * Added open source copyright license.
 *
 * Revision 1.3  1997/05/04 02:49:52  craigs
 * Added support for client and server certificates
 *
 * Revision 1.1  1996/11/15 07:37:48  craigs
 * Initial revision
 *
 */

#ifndef _PHTTPS
#define _PHTTPS

#ifdef __GNUC__
#pragma interface
#endif

#include <sockets.h>
#include <http.h>

extern "C" {
#include <ssl.h>
#include <crypto.h>
};

PDECLARE_CLASS(PSSLChannel, PIndirectChannel)
  public:
    enum {
      CertificateOK,
      UnknownCertificate,
      UnknownPrivateKey,
      PrivateKeyMismatch,
    };

    enum {
      VerifyNone,
      VerifyPeer,
      VerifyPeerMandatory,
    };

    PSSLChannel();
    ~PSSLChannel();

    static BOOL SetCAPath(const PDirectory & caPath);
    static BOOL SetCAFile(const PFilePath & caFile);
    static BOOL SetCAPathAndFile(const PDirectory & caPath, const PFilePath & caFile);

    void SetVerifyMode(int mode);

    int SetClientCertificate(const PString & certFile);
    int SetClientCertificate(const PString & certFile, const PString & keyFile);


    // Normal socket read & write functions
    BOOL   Read(void * buf, PINDEX len);
    BOOL   Write(const void * buf, PINDEX len);

    BOOL   RawRead(void * buf, PINDEX len);
    PINDEX RawGetLastReadCount() const;
    BOOL   RawWrite(const void * buf, PINDEX len);
    PINDEX RawGetLastWriteCount() const;

    BOOL Accept(PChannel & channel);
    BOOL Connect(PChannel & channel);
    BOOL Shutdown(ShutdownValue) { return TRUE; }

  protected:
    int  Set_SSL_Fd();
    int  SetClientCertificate(const char * certFile, const char * keyFile);

    static BOOL SetCAPathAndFile(const char * caPath, const char * caFile);
    static void Cleanup();


    SSL * ssl;
    static SSL_CTX * context;
    static PSemaphore semaphores[CRYPTO_NUM_LOCKS];
    static PSemaphore initFlag;
};

#endif
