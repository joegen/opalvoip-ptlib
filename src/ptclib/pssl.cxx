/*
 * pssl.cxx
 *
 * SSL implementation for PTLib using the SSLeay package
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
 * Portions bsed upon the file crypto/buffer/bss_sock.c 
 * Original copyright notice appears below
 *
 * $Id: pssl.cxx,v 1.25 2001/09/10 02:51:23 robertj Exp $
 * $Log: pssl.cxx,v $
 * Revision 1.25  2001/09/10 02:51:23  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.24  2001/06/01 00:53:59  robertj
 * Added certificate constructor that takes a PBYTEArray
 *
 * Revision 1.23  2001/05/31 07:04:11  craigs
 * Changed random seed function
 *
 * Revision 1.22  2001/05/29 03:33:54  craigs
 * Added options to be compatible with OpenSSL 0.9.6
 *
 * Revision 1.21  2001/05/16 06:31:37  robertj
 * Fixed GNU C++ compatibility
 *
 * Revision 1.20  2001/05/16 06:02:37  craigs
 * Changed to allow detection of non-SSL connection to SecureHTTPServiceProcess
 *
 * Revision 1.19  2001/05/09 07:00:22  robertj
 * Removed clearing of lock callbacks in context destructor, should not!
 *
 * Revision 1.18  2001/02/16 07:13:41  robertj
 * Fixed bug in PSSLChannel error detection, thinks a zero byte write is error.
 *
 * Revision 1.17  2000/11/27 06:46:16  robertj
 * Added asserts with SSL error message text.
 *
 * Revision 1.16  2000/11/14 08:33:16  robertj
 * Added certificate and private key classes.
 *
 * Revision 1.15  2000/11/03 10:00:43  robertj
 * Fixed initialisation of SSL, needed random number seed for some modes.
 *
 * Revision 1.14  2000/09/01 03:32:46  robertj
 * Fixed assert on setting directories for CAs.
 *
 * Revision 1.13  2000/09/01 02:06:00  craigs
 * Changed to OpenSSL_add_ssl_algorthms to fix link problem on some machines
 *
 * Revision 1.12  2000/08/25 13:56:46  robertj
 * Fixed some GNU warnings
 *
 * Revision 1.11  2000/08/25 08:11:02  robertj
 * Fixed OpenSSL support so can operate as a server channel.
 *
 * Revision 1.10  2000/08/04 12:52:18  robertj
 * SSL changes, added error functions, removed need to have openssl include directory in app.
 *
 * Revision 1.9  2000/01/10 02:24:09  craigs
 * Updated for new OpenSSL
 *
 * Revision 1.8  1998/12/04 13:04:18  craigs
 * Changed for SSLeay 0.9
 *
 * Revision 1.7  1998/09/23 06:22:35  robertj
 * Added open source copyright license.
 *
 * Revision 1.6  1998/01/26 02:50:17  robertj
 * GNU Support
 *
 * Revision 1.5  1997/05/04 02:50:54  craigs
 * Added support for client and server sertificates
 *
 * Revision 1.1  1996/11/15 07:38:34  craigs
 * Initial revision
 *
 */

/* crypto/buffer/bss_sock.c */
/* Copyright (C) 1995-1996 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifdef __GNUC__
#pragma implementation "pssl.h"
#endif

#include <ptlib.h>

#include <ptclib/pssl.h>


#define USE_SOCKETS

extern "C" {

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>

};

PARRAY(PSSLMutexArrayBase, PMutex);
class PSSLMutexArray : public PSSLMutexArrayBase
{
    PCLASSINFO(PSSLMutexArray, PSSLMutexArrayBase);
  public:
    PSSLMutexArray();
};


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PSSLMutexArray::PSSLMutexArray()
{
  // Initialise all of the mutexes for multithreaded operation.
  SetSize(CRYPTO_num_locks());
  for (PINDEX i = 0; i < GetSize(); i++)
    SetAt(i, new PMutex);
}


///////////////////////////////////////////////////////////////////////////////

PSSLCertificate::PSSLCertificate()
{
  certificate = NULL;
}


PSSLCertificate::PSSLCertificate(const PFilePath & certFile, int fileType)
{
  certificate = NULL;

  BIO *in = BIO_new(BIO_s_file_internal());
  if (in == NULL) {
    SSLerr(SSL_F_SSL_USE_PRIVATEKEY_FILE,ERR_R_BUF_LIB);
    return;
  }

  if (BIO_read_filename(in, (char *)(const char *)certFile) <= 0)
    SSLerr(SSL_F_SSL_USE_PRIVATEKEY_FILE,ERR_R_SYS_LIB);
  else {
    if (fileType < 0)
      fileType = certFile.GetType() == ".pem" ? SSL_FILETYPE_PEM : SSL_FILETYPE_ASN1;

    switch (fileType) {
      case SSL_FILETYPE_ASN1 :
        certificate = d2i_X509_bio(in, NULL);
        if (certificate == NULL)
	  SSLerr(SSL_F_SSL_CTX_USE_CERTIFICATE_FILE, ERR_R_ASN1_LIB);
        break;

      case SSL_FILETYPE_PEM :
        certificate = PEM_read_bio_X509(in, NULL, NULL, NULL);
        if (certificate == NULL)
	  SSLerr(SSL_F_SSL_CTX_USE_CERTIFICATE_FILE, ERR_R_PEM_LIB);
        break;

      default :
	SSLerr(SSL_F_SSL_CTX_USE_CERTIFICATE_FILE,SSL_R_BAD_SSL_FILETYPE);
    }
  }

  BIO_free(in);
}


PSSLCertificate::PSSLCertificate(const BYTE * certData, PINDEX certSize)
{
  certificate = d2i_X509(NULL, (unsigned char **)&certData, certSize);
}


PSSLCertificate::PSSLCertificate(const PBYTEArray & certData)
{
  const BYTE * certPtr = certData;
  certificate = d2i_X509(NULL, (unsigned char **)&certPtr, certData.GetSize());
}


PSSLCertificate::PSSLCertificate(const PSSLCertificate & cert)
{
  if (cert.certificate == NULL)
    certificate = NULL;
  else
    certificate = X509_dup(cert.certificate);
}


PSSLCertificate & PSSLCertificate::operator=(const PSSLCertificate & cert)
{
  if (certificate != NULL)
    X509_free(certificate);
  if (cert.certificate == NULL)
    certificate = NULL;
  else
    certificate = X509_dup(cert.certificate);

  return *this;
}


PSSLCertificate::~PSSLCertificate()
{
  if (certificate != NULL)
    X509_free(certificate);
}


///////////////////////////////////////////////////////////////////////////////

PSSLPrivateKey::PSSLPrivateKey()
{
  key = NULL;
}


PSSLPrivateKey::PSSLPrivateKey(const PFilePath & keyFile)
{
  key = NULL;

  BIO *in = BIO_new(BIO_s_file_internal());
  if (in == NULL) {
    SSLerr(SSL_F_SSL_USE_PRIVATEKEY_FILE,ERR_R_BUF_LIB);
    return;
  }

  if (BIO_read_filename(in, (char *)(const char *)keyFile) <= 0)
    SSLerr(SSL_F_SSL_USE_PRIVATEKEY_FILE,ERR_R_SYS_LIB);
  else
    key = PEM_read_bio_PrivateKey(in, NULL, NULL, NULL);

  BIO_free(in);
}


PSSLPrivateKey::PSSLPrivateKey(const BYTE * keyData, PINDEX keySize, BYTE /*unused*/)
{
  key = d2i_AutoPrivateKey(NULL, (BYTE **)&keyData, keySize);
}


PSSLPrivateKey::PSSLPrivateKey(const PSSLPrivateKey & privKey)
{
  key = privKey.key;
}


PSSLPrivateKey & PSSLPrivateKey::operator=(const PSSLPrivateKey & privKey)
{
  if (key != NULL)
    EVP_PKEY_free(key);

  key = privKey.key;

  return *this;
}


PSSLPrivateKey::~PSSLPrivateKey()
{
  if (key != NULL)
    EVP_PKEY_free(key);
}


///////////////////////////////////////////////////////////////////////////////

static void LockingCallback(int mode, int n, const char * /*file*/, int /*line*/)
{
  static PSSLMutexArray mutexes;
  if ((mode & CRYPTO_LOCK) != 0)
    mutexes[n].Wait();
  else
    mutexes[n].Signal();
}


static int VerifyCallBack(int ok, X509_STORE_CTX * ctx)
{
  X509 * err_cert = X509_STORE_CTX_get_current_cert(ctx);
  //int err         = X509_STORE_CTX_get_error(ctx);

  // get the subject name, just for verification
  char buf[256];
  X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

  PTRACE(1, "Verify callback depth "
         << X509_STORE_CTX_get_error_depth(ctx)
         << " : cert name = " << buf);

  return ok;
}


static void PSSLAssert(const char * msg)
{
  char buf[256];
  strcpy(buf, msg);
  ERR_error_string(ERR_peek_error(), &buf[strlen(msg)]);
  PTRACE(1, "SSL\t" << buf);
  PAssertAlways(buf);
}


PSSLContext::PSSLContext(const void * sessionId, PINDEX idSize)
{
  PINDEX i;

  static PMutex InitialisationMutex;
  InitialisationMutex.Wait();

  static BOOL needInitialisation = TRUE;
  if (needInitialisation) {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // Seed the random number generator
    BYTE seed[128];
    for (i = 0; i < sizeof(seed); i++)
      seed[i] = (BYTE)rand();
    RAND_seed(seed, sizeof(seed));

    // set up multithread stuff
    CRYPTO_set_locking_callback(LockingCallback);

    needInitialisation = FALSE;
  }

  InitialisationMutex.Signal();

  // create the new SSL context
  SSL_METHOD * meth = SSLv23_method();
  context  = SSL_CTX_new(meth);
  if (context == NULL)
    PSSLAssert("Error creating context: ");

  // Shutdown
  SSL_CTX_set_quiet_shutdown(context, 1);

  // Set default locations
  if (!SSL_CTX_load_verify_locations(context, NULL, ".") ||
      !SSL_CTX_set_default_verify_paths(context))
    PSSLAssert("Cannot set CAfile and path: ");

  if (sessionId != NULL) {
    if (idSize == 0)
      idSize = ::strlen((const char *)sessionId)+1;
    SSL_CTX_set_session_id_context(context, (const BYTE *)sessionId, idSize);
    SSL_CTX_sess_set_cache_size(context, 128);
  }

  // set default verify mode
  SSL_CTX_set_verify(context, SSL_VERIFY_NONE, VerifyCallBack);
}


PSSLContext::~PSSLContext()
{
  SSL_CTX_free(context);
}


BOOL PSSLContext::SetCAPath(const PDirectory & caPath)
{
  PString path = caPath.Left(caPath.GetLength()-1);
  if (!SSL_CTX_load_verify_locations(context, NULL, path))
    return FALSE;

  return SSL_CTX_set_default_verify_paths(context);
}


BOOL PSSLContext::SetCAFile(const PFilePath & caFile)
{
  if (!SSL_CTX_load_verify_locations(context, caFile, NULL))
    return FALSE;

  return SSL_CTX_set_default_verify_paths(context);
}


BOOL PSSLContext::UseCertificate(const PSSLCertificate & certificate)
{
  return SSL_CTX_use_certificate(context, certificate) > 0;
}


BOOL PSSLContext::UsePrivateKey(const PSSLPrivateKey & key)
{
  if (SSL_CTX_use_PrivateKey(context, key) <= 0)
    return FALSE;

  return SSL_CTX_check_private_key(context);
}


BOOL PSSLContext::SetCipherList(const PString & ciphers)
{
  if (ciphers.IsEmpty())
    return FALSE;

  return SSL_CTX_set_cipher_list(context, (char *)(const char *)ciphers);
}


/////////////////////////////////////////////////////////////////////////
//
//  SSLChannel
//

PSSLChannel::PSSLChannel(PSSLContext * ctx, BOOL autoDel)
{
  if (ctx != NULL) {
    context = ctx;
    autoDeleteContext = autoDel;
  }
  else {
    context = new PSSLContext;
    autoDeleteContext = TRUE;
  }

  ssl = SSL_new(*context);
  if (ssl == NULL)
    PSSLAssert("Error creating channel: ");
}


PSSLChannel::PSSLChannel(PSSLContext & ctx)
{
  context = &ctx;
  autoDeleteContext = FALSE;

  ssl = SSL_new(*context);
}


PSSLChannel::~PSSLChannel()
{
  // free the SSL connection
  if (ssl != NULL)
    SSL_free(ssl);

  if (autoDeleteContext)
    delete context;
}


BOOL PSSLChannel::Read(void * buf, PINDEX len)
{
  flush();

  channelPointerMutex.StartRead();

  BOOL returnValue = FALSE;
  if (readChannel == NULL)
    SetErrorValues(NotOpen, EBADF, LastReadError);
  else if (readTimeout == 0 && SSL_pending(ssl) == 0)
    SetErrorValues(Timeout, ETIMEDOUT, LastReadError);
  else {
    readChannel->SetReadTimeout(readTimeout);

    int readResult = SSL_read(ssl, (char *)buf, len);
    lastReadCount = readResult;
    returnValue = readResult >= 0;
    if (!returnValue && GetErrorCode(LastReadError) == NoError)
      ConvertOSError(-1, LastReadError);
  }

  channelPointerMutex.EndRead();

  return returnValue;
}

BOOL PSSLChannel::Write(const void * buf, PINDEX len)
{
  flush();

  channelPointerMutex.StartRead();

  BOOL returnValue;
  if (writeChannel == NULL) {
    SetErrorValues(NotOpen, EBADF, LastWriteError);
    returnValue = FALSE;
  }
  else {
    writeChannel->SetWriteTimeout(writeTimeout);

    int writeResult = SSL_write(ssl, (const char *)buf, len);
    lastWriteCount = writeResult;
    returnValue = writeResult >= 0;
    if (!returnValue && GetErrorCode(LastWriteError) == NoError)
      ConvertOSError(-1, LastWriteError);
  }

  channelPointerMutex.EndRead();

  return returnValue;
}


BOOL PSSLChannel::Close()
{
  BOOL ok = SSL_shutdown(ssl);
  return PIndirectChannel::Close() && ok;
}


BOOL PSSLChannel::ConvertOSError(int error, ErrorGroup group)
{
  Errors lastError = NoError;
  DWORD osError = 0;
  if (SSL_get_error(ssl, error) != SSL_ERROR_NONE && (osError = ERR_peek_error()) != 0) {
    osError |= 0x80000000;
    lastError = Miscellaneous;
  }

  return SetErrorValues(lastError, osError, group);
}


PString PSSLChannel::GetErrorText(ErrorGroup group) const
{
  if ((lastErrorNumber[group]&0x80000000) == 0)
    return PIndirectChannel::GetErrorText(group);

  char buf[200];
  return ERR_error_string(lastErrorNumber[group]&0x7fffffff, buf);
}


BOOL PSSLChannel::Accept()
{
  if (IsOpen())
    return ConvertOSError(SSL_accept(ssl));
  return FALSE;
}


BOOL PSSLChannel::Accept(PChannel & channel)
{
  if (Open(channel))
    return ConvertOSError(SSL_accept(ssl));
  return FALSE;
}


BOOL PSSLChannel::Accept(PChannel * channel, BOOL autoDelete)
{
  if (Open(channel, autoDelete))
    return ConvertOSError(SSL_accept(ssl));
  return FALSE;
}


BOOL PSSLChannel::Connect()
{
  if (IsOpen())
    return ConvertOSError(SSL_connect(ssl));
  return FALSE;
}


BOOL PSSLChannel::Connect(PChannel & channel)
{
  if (Open(channel))
    return ConvertOSError(SSL_connect(ssl));
  return FALSE;
}


BOOL PSSLChannel::Connect(PChannel * channel, BOOL autoDelete)
{
  if (Open(channel, autoDelete))
    return ConvertOSError(SSL_connect(ssl));
  return FALSE;
}


BOOL PSSLChannel::UseCertificate(const PSSLCertificate & certificate)
{
  return SSL_use_certificate(ssl, certificate);
}


void PSSLChannel::SetVerifyMode(VerifyMode mode)
{
  int verify;

  switch (mode) {
    default :
    case VerifyNone:
      verify = SSL_VERIFY_NONE;
      break;

    case VerifyPeer:
      verify = SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE;
      break;

    case VerifyPeerMandatory:
      verify = SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
  }

  SSL_set_verify(ssl, verify, VerifyCallBack);
}


BOOL PSSLChannel::RawSSLRead(void * buf, PINDEX & len)
{
  if (!PIndirectChannel::Read(buf, len)) 
    return FALSE;

  len = GetLastReadCount();
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//  Low level interface to SSLEay routines
//


#define	PSSLSOCKET(bio)			((PSSLChannel *)(bio->num))

extern "C" {

#if (OPENSSL_VERSION_NUMBER < 0x00906000)

typedef int (*ifptr)();
typedef long (*lfptr)();

#endif

static int Psock_new(BIO * bio)
{
  bio->init     = 0;
  bio->num      = 0;    // this is really (PSSLChannel *), not int
  bio->ptr      = NULL;
  bio->flags    = 0;

  return(1);
}


static int Psock_free(BIO * bio)
{
  if (bio == NULL)
    return 0;

  if (bio->shutdown) {
    if (bio->init) {
      PSSLSOCKET(bio)->Shutdown(PSocket::ShutdownReadAndWrite);
      PSSLSOCKET(bio)->Close();
    }
    bio->init  = 0;
    bio->flags = 0;
  }
  return 1;
}


static long Psock_ctrl(BIO * bio, int cmd, long num, void * ptr)
{
  long ret = 1;
  int *ip;

  switch (cmd) {
    case BIO_C_SET_FD:
      Psock_free(bio);
      bio->num      = *((int *)ptr);
      bio->shutdown = (int)num;
      bio->init     = 1;
      break;

    case BIO_C_GET_FD:
      if (bio->init) {
	ip = (int *)ptr;
	if (ip != NULL)
          *ip = bio->num;
	ret = bio->num;
      }
      else
        ret = -1;
      break;

    case BIO_CTRL_GET_CLOSE:
      ret = bio->shutdown;
      break;

    case BIO_CTRL_SET_CLOSE:
      bio->shutdown = (int)num;
      break;

    case BIO_CTRL_DUP:
    case BIO_CTRL_FLUSH:
      return 1;

    // Other BIO commands, return 0
    default:
      return 0;
  }

  return ret;
}


static int Psock_read(BIO * bio, char * out, int outl)
{
  if (out == NULL)
    return 0;

  BIO_clear_retry_flags(bio);

  // Skip over the polymorphic read, want to do real one
  PINDEX len = outl;
  if (PSSLSOCKET(bio)->RawSSLRead(out, len))
    return len;

  switch (PSSLSOCKET(bio)->GetErrorCode(PChannel::LastReadError)) {
    case PChannel::Interrupted :
    case PChannel::Timeout :
      BIO_set_retry_read(bio);
      return -1;

    default :
      break;
  }

  return 0;
}


static int Psock_write(BIO * bio, const char * in, int inl)
{
  if (in == NULL)
    return 0;

  BIO_clear_retry_flags(bio);

  // Skip over the polymorphic write, want to do real one
  if (PSSLSOCKET(bio)->PIndirectChannel::Write(in, inl))
    return PSSLSOCKET(bio)->GetLastWriteCount();

  switch (PSSLSOCKET(bio)->GetErrorCode(PChannel::LastWriteError)) {
    case PChannel::Interrupted :
    case PChannel::Timeout :
      BIO_set_retry_write(bio);
      return -1;

    default :
      break;
  }

  return 0;
}


static int Psock_puts(BIO * bio, const char * str)
{
  int n,ret;

  n   = strlen(str);
  ret = Psock_write(bio,str,n);

  return ret;
}

};


static BIO_METHOD methods_Psock =
{
  BIO_TYPE_SOCKET,
  "ptlib socket",
#if (OPENSSL_VERSION_NUMBER < 0x00906000)
  (ifptr)Psock_write,
  (ifptr)Psock_read,
  (ifptr)Psock_puts,
  NULL,
  (lfptr)Psock_ctrl,
  (ifptr)Psock_new,
  (ifptr)Psock_free
#else
  Psock_write,
  Psock_read,
  Psock_puts,
  NULL,
  Psock_ctrl,
  Psock_new,
  Psock_free
#endif
};


BOOL PSSLChannel::OnOpen()
{
  BIO * bio = BIO_new(&methods_Psock);
  if (bio == NULL) {
    SSLerr(SSL_F_SSL_SET_FD,ERR_R_BUF_LIB);
    return FALSE;
  }

  SSL_set_bio(ssl, bio, bio);

  BIO_set_fd(bio, (int)this, BIO_NOCLOSE);

/*
  if (ssl->rbio != NULL)
    BIO_free((bio_st *)ssl->rbio);

  if ((ssl->wbio != NULL) && (ssl->wbio != ssl->rbio))
    BIO_free((bio_st *)ssl->wbio);

  ssl->rbio = bio;
  ssl->wbio = bio;
  ret = 1;
*/
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//  misc unused code
//

#if 0

extern "C" {

static verify_depth = 0;
static verify_error = VERIFY_OK;

// should be X509 * but we can just have them as char *. 
int verify_callback(int ok, X509 * xs, X509 * xi, int depth, int error)
{
  char *s;

  s = (char *)X509_NAME_oneline(X509_get_subject_name(xs));
  if (s == NULL) {
//    ERR_print_errors(bio_err);
    return(0);
  }
  PError << "depth= " << depth << " " << (char *)s << endl;
  free(s);
  if (error == VERIFY_ERR_UNABLE_TO_GET_ISSUER) {
    s=(char *)X509_NAME_oneline(X509_get_issuer_name(xs));
    if (s == NULL) {
      PError << "verify error" << endl;
      //ERR_print_errors(bio_err);
      return(0);
    }
    PError << "issuer = " << s << endl;
    free(s);
  }

  if (!ok) {
    PError << "verify error:num=" << error << " " <<
			X509_cert_verify_error_string(error) << endl;
    if (verify_depth <= depth) {
      ok=1;
      verify_error=VERIFY_OK;
    } else {
      ok=0;
      verify_error=error;
    }
  }
  PError << "verify return:" << ok << endl;
  return(ok);
}

};

#endif

