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
 * Revision 1.9  2000/08/25 08:11:02  robertj
 * Fixed OpenSSL support so can operate as a server channel.
 *
 * Revision 1.8  2000/08/04 12:52:18  robertj
 * SSL changes, added error functions, removed need to have openssl include directory in app.
 *
 * Revision 1.7  2000/01/10 02:23:18  craigs
 * Update for new OpenSSL
 *
 * Revision 1.6  1999/02/16 08:07:10  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.5  1998/12/04 13:01:51  craigs
 * Changed for SSLeay 0.9
 *
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

#include <ptlib/sockets.h>


struct ssl_st;
struct ssl_ctx_st;


/**Context for SSL channels.
   This class embodies a common environment for all connections made via SSL
   using the PSSLChannel class. It includes such things as the version of SSL
   and certificates, CA's etc.
  */
class PSSLContext {
  public:
    /**Create a new context for SSL channels.
       An optional session ID may be provided in the context. This is used
       to identify sessions across multiple channels in this context. The
       session ID is a completely arbitrary block of data. If sessionId is
       non NULL and idSize is zero, then sessionId is assumed to be a pointer
       to a C string.
      */
    PSSLContext(
      const void * sessionId = NULL,  /// Pointer to session ID
      PINDEX idSize = 0               /// Size of session ID
    );

    /**Clean up the SSL context.
      */
    ~PSSLContext();

    /**Get the internal SSL context structure.
      */
    operator ssl_ctx_st *() const { return context; }

    /**Set the path to locate CA certificates.
      */
    BOOL SetCAPath(
      const PDirectory & caPath   /// Directory for CA certificates
    );

    /**Set the CA certificate file.
      */
    BOOL SetCAFile(
      const PFilePath & caFile    /// CA certificate file
    );

    /**Use the certificate specified.
       The type of the certificate (eg SSL_FILETYPE_PEM) can be specified
       explicitly, or if -1 it will be determined from the file extension.
      */
    BOOL UseCertificate(
      const PFilePath & certFile, /// Certificate file
      int fileType = -1           /// Type of certificate file
    );

    /**Use the private key file specified.
       The type of the key file (eg SSL_FILETYPE_PEM) can be specified
       explicitly, or if -1 it will be determined from the file extension.
      */
    BOOL UsePrivateKey(
      const PFilePath & keyFile,  /// Key file
      int fileType = -1           /// Type of key file
    );

    /**Set the available ciphers to those listed.
      */
    BOOL SetCipherList(
      const PString & ciphers   /// List of cipher names.
    );

  protected:
    ssl_ctx_st * context;
};


/**This class will start a secure SSL based channel.
  */
class PSSLChannel : public PIndirectChannel
{
  PCLASSINFO(PSSLChannel, PIndirectChannel)
  public:
    /**Create a new channel given the context.
       If no context is given a default one is created.
      */
    PSSLChannel(
      PSSLContext * context = NULL,   /// Context for SSL channel
      BOOL autoDeleteContext = FALSE  /// Flag for context to be automatically deleted.
    );
    PSSLChannel(
      PSSLContext & context           /// Context for SSL channel
    );

    /**Close and clear the SSL channel.
      */
    ~PSSLChannel();

    // Overrides from PChannel
    virtual BOOL Read(void * buf, PINDEX len);
    virtual BOOL Write(const void * buf, PINDEX len);
    virtual BOOL Close();
    virtual BOOL Shutdown(ShutdownValue) { return TRUE; }
    virtual PString GetErrorText() const;
    virtual BOOL ConvertOSError(int error);

    // New functions
    /**Accept a new inbound connection (server).
       This version expects that the indirect channel has already been opened
       using Open() beforehand.
      */
    BOOL Accept();

    /**Accept a new inbound connection (server).
      */
    BOOL Accept(
      PChannel & channel  /// Channel to attach to.
    );

    /**Accept a new inbound connection (server).
      */
    BOOL Accept(
      PChannel * channel,     /// Channel to attach to.
      BOOL autoDelete = TRUE  /// Flag for if channel should be automatically deleted.
    );


    /**Connect to remote server.
       This version expects that the indirect channel has already been opened
       using Open() beforehand.
      */
    BOOL Connect();

    /**Connect to remote server.
      */
    BOOL Connect(
      PChannel & channel  /// Channel to attach to.
    );

    /**Connect to remote server.
      */
    BOOL Connect(
      PChannel * channel,     /// Channel to attach to.
      BOOL autoDelete = TRUE  /// Flag for if channel should be automatically deleted.
    );

    enum CertificateStatus {
      CertificateOK,
      UnknownCertificate,
      UnknownPrivateKey,
      PrivateKeyMismatch,
    };

    CertificateStatus SetClientCertificate(const PString & certFile);
    CertificateStatus SetClientCertificate(const PString & certFile, const PString & keyFile);

    enum VerifyMode {
      VerifyNone,
      VerifyPeer,
      VerifyPeerMandatory,
    };

    void SetVerifyMode(VerifyMode mode);


  protected:
    /**This callback is executed when the Open() function is called with
       open channels. It may be used by descendent channels to do any
       handshaking required by the protocol that channel embodies.

       The default behaviour "connects" the channel to the OpenSSL library.

       @return
       Returns TRUE if the protocol handshaking is successful.
     */
    virtual BOOL OnOpen();

  protected:
    PSSLContext * context;
    BOOL          autoDeleteContext;
    ssl_st      * ssl;
};

#endif
