/*
 * pssl.cxx
 *
 * SSL implementation for PTLib using the SSLeay package
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
 * Portions bsed upon the file crypto/buffer/bss_sock.c 
 * Original copyright notice appears below
 *
 * $Id: pssl.cxx 19752 2008-03-15 15:30:24Z hfriederich $
 * $Revision$
 * $Author$
 * $Date$
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
#include <ptclib/mime.h>

#include <ptbuildopts.h>

#if P_SSL

#define USE_SOCKETS

extern "C" {

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

};


#ifdef _MSC_VER
  #pragma comment(lib, P_SSL_LIB1)
  #pragma comment(lib, P_SSL_LIB2)
  #pragma message("SSL support (via OpenSSL) enabled")
#endif


// On Windows, use a define from the header to guess the API type
#ifdef _WIN32
#ifdef SSL_OP_NO_QUERY_MTU
#define P_SSL_USE_CONST 1
#endif
#endif


class PSSLInitialiser : public PProcessStartup
{
  PCLASSINFO(PSSLInitialiser, PProcessStartup)
  public:
    virtual void OnStartup();
    virtual void OnShutdown();
    void LockingCallback(int mode, int n);

    PFACTORY_GET_SINGLETON(PProcessStartupFactory, PSSLInitialiser);

  private:
    vector<PMutex> mutexes;
};

PFACTORY_CREATE_SINGLETON(PProcessStartupFactory, PSSLInitialiser);


///////////////////////////////////////////////////////////////////////////////

class PSSL_BIO
{
  public:
    PSSL_BIO(BIO_METHOD *method = BIO_s_file_internal())
      { bio = BIO_new(method); }

    ~PSSL_BIO()
      { BIO_free(bio); }

    operator BIO*() const
      { return bio; }

    bool OpenRead(const PFilePath & filename)
      { return BIO_read_filename(bio, (char *)(const char *)filename) > 0; }

    bool OpenWrite(const PFilePath & filename)
      { return BIO_write_filename(bio, (char *)(const char *)filename) > 0; }

    bool OpenAppend(const PFilePath & filename)
      { return BIO_append_filename(bio, (char *)(const char *)filename) > 0; }

  protected:
    BIO * bio;
};


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PSSLPrivateKey::PSSLPrivateKey()
{
  key = NULL;
}


PSSLPrivateKey::PSSLPrivateKey(unsigned modulus,
                               void (*callback)(int,int,void *),
                               void *cb_arg)
{
  key = NULL;
  Create(modulus, callback, cb_arg);
}


PSSLPrivateKey::PSSLPrivateKey(const PFilePath & keyFile, PSSLFileTypes fileType)
{
  key = NULL;
  Load(keyFile, fileType);
}


PSSLPrivateKey::PSSLPrivateKey(const BYTE * keyData, PINDEX keySize)
{
#if P_SSL_USE_CONST
  key = d2i_AutoPrivateKey(NULL, &keyData, keySize);
#else
  key = d2i_AutoPrivateKey(NULL, (BYTE **)&keyData, keySize);
#endif
}


PSSLPrivateKey::PSSLPrivateKey(const PBYTEArray & keyData)
{
  const BYTE * keyPtr = keyData;
#if P_SSL_USE_CONST
  key = d2i_AutoPrivateKey(NULL, &keyPtr, keyData.GetSize());
#else
  key = d2i_AutoPrivateKey(NULL, (BYTE **)&keyPtr, keyData.GetSize());
#endif
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


PBoolean PSSLPrivateKey::Create(unsigned modulus,
                            void (*callback)(int,int,void *),
                            void *cb_arg)
{
  if (key != NULL) {
    EVP_PKEY_free(key);
    key = NULL;
  }

  if (!PAssert(modulus >= 384, PInvalidParameter))
    return false;

  key = EVP_PKEY_new();
  if (key == NULL)
    return false;

  if (EVP_PKEY_assign_RSA(key, RSA_generate_key(modulus, 0x10001, callback, cb_arg)))
    return true;

  EVP_PKEY_free(key);
  key = NULL;
  return false;
}


PBYTEArray PSSLPrivateKey::GetData() const
{
  PBYTEArray data;

  if (key != NULL) {
    BYTE * keyPtr = data.GetPointer(i2d_PrivateKey(key, NULL));
    i2d_PrivateKey(key, &keyPtr);
  }

  return data;
}


PString PSSLPrivateKey::AsString() const
{
  return PBase64::Encode(GetData());
}


PBoolean PSSLPrivateKey::Load(const PFilePath & keyFile, PSSLFileTypes fileType)
{
  if (key != NULL) {
    EVP_PKEY_free(key);
    key = NULL;
  }

  PSSL_BIO in;
  if (!in.OpenRead(keyFile)) {
    PTRACE(2, "SSL\tCould not open private key file \"" << keyFile << '"');
    return false;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      key = d2i_PrivateKey_bio(in, NULL);
      if (key != NULL)
        break;

      PTRACE(2, "SSL\tInvalid ASN.1 private key file \"" << keyFile << '"');
      return false;

    case PSSLFileTypePEM :
      key = PEM_read_bio_PrivateKey(in, NULL, NULL, NULL);
      if (key != NULL)
        break;

      PTRACE(2, "SSL\tInvalid PEM private key file \"" << keyFile << '"');
      return false;

    default :
      key = PEM_read_bio_PrivateKey(in, NULL, NULL, NULL);
      if (key != NULL)
        break;

      key = d2i_PrivateKey_bio(in, NULL);
      if (key != NULL)
        break;

      PTRACE(2, "SSL\tInvalid private key file \"" << keyFile << '"');
      return false;
  }

  PTRACE(4, "SSL\tLoaded private key file \"" << keyFile << '"');
  return true;
}


PBoolean PSSLPrivateKey::Save(const PFilePath & keyFile, PBoolean append, PSSLFileTypes fileType)
{
  if (key == NULL)
    return false;

  PSSL_BIO out;
  if (!(append ? out.OpenAppend(keyFile) : out.OpenWrite(keyFile))) {
    PTRACE(2, "SSL\tCould not " << (append ? "append to" : "create") << " private key file \"" << keyFile << '"');
    return false;
  }

  if (fileType == PSSLFileTypeDEFAULT)
    fileType = keyFile.GetType() == ".der" ? PSSLFileTypeASN1 : PSSLFileTypePEM;

  switch (fileType) {
    case PSSLFileTypeASN1 :
      if (i2d_PrivateKey_bio(out, key))
        return true;
      break;

    case PSSLFileTypePEM :
      if (PEM_write_bio_PrivateKey(out, key, NULL, NULL, 0, 0, NULL))
        return true;
      break;

    default :
      PAssertAlways(PInvalidParameter);
      return false;
  }

  PTRACE(2, "SSL\tError writing certificate file \"" << keyFile << '"');
  return false;
}


///////////////////////////////////////////////////////////////////////////////

PSSLCertificate::PSSLCertificate()
{
  certificate = NULL;
}


PSSLCertificate::PSSLCertificate(const PFilePath & certFile, PSSLFileTypes fileType)
{
  certificate = NULL;
  Load(certFile, fileType);
}


PSSLCertificate::PSSLCertificate(const BYTE * certData, PINDEX certSize)
{
#if P_SSL_USE_CONST
  certificate = d2i_X509(NULL, &certData, certSize);
#else
  certificate = d2i_X509(NULL, (unsigned char **)&certData, certSize);
#endif
}


PSSLCertificate::PSSLCertificate(const PBYTEArray & certData)
{
  const BYTE * certPtr = certData;
#if P_SSL_USE_CONST
  certificate = d2i_X509(NULL, &certPtr, certData.GetSize());
#else
  certificate = d2i_X509(NULL, (unsigned char **)&certPtr, certData.GetSize());
#endif
}


PSSLCertificate::PSSLCertificate(const PString & certStr)
{
  PBYTEArray certData;
  PBase64::Decode(certStr, certData);
  if (certData.GetSize() > 0) {
    const BYTE * certPtr = certData;
#if P_SSL_USE_CONST
    certificate = d2i_X509(NULL, &certPtr, certData.GetSize());
#else
    certificate = d2i_X509(NULL, (unsigned char **)&certPtr, certData.GetSize());
#endif
  }
  else
    certificate = NULL;
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


PBoolean PSSLCertificate::CreateRoot(const PString & subject,
                                 const PSSLPrivateKey & privateKey)
{
  if (certificate != NULL) {
    X509_free(certificate);
    certificate = NULL;
  }

  if (privateKey == NULL)
    return false;

  POrdinalToString info;
  PStringArray fields = subject.Tokenise('/', false);
  PINDEX i;
  for (i = 0; i < fields.GetSize(); i++) {
    PString field = fields[i];
    PINDEX equals = field.Find('=');
    if (equals != P_MAX_INDEX) {
      int nid = OBJ_txt2nid((char *)(const char *)field.Left(equals));
      if (nid != NID_undef)
        info.SetAt(nid, field.Mid(equals+1));
    }
  }
  if (info.IsEmpty())
    return false;

  certificate = X509_new();
  if (certificate == NULL)
    return false;

  if (X509_set_version(certificate, 2)) {
    /* Set version to V3 */
    ASN1_INTEGER_set(X509_get_serialNumber(certificate), 0L);

    X509_NAME * name = X509_NAME_new();
    for (POrdinalToString::iterator it = info.begin(); it != info.end(); ++it)
      X509_NAME_add_entry_by_NID(name,
                                 it->first,
                                 MBSTRING_ASC,
                                 (unsigned char *)(const char *)it->second,
                                 -1,-1, 0);
    X509_set_issuer_name(certificate, name);
    X509_set_subject_name(certificate, name);
    X509_NAME_free(name);

    X509_gmtime_adj(X509_get_notBefore(certificate), 0);
    X509_gmtime_adj(X509_get_notAfter(certificate), (long)60*60*24*365*5);

    X509_PUBKEY * pubkey = X509_PUBKEY_new();
    if (pubkey != NULL) {
      X509_PUBKEY_set(&pubkey, privateKey);
      EVP_PKEY * pkey = X509_PUBKEY_get(pubkey);
      X509_set_pubkey(certificate, pkey);
      EVP_PKEY_free(pkey);
      X509_PUBKEY_free(pubkey);

      if (X509_sign(certificate, privateKey, EVP_md5()) > 0)
        return true;
    }
  }

  X509_free(certificate);
  certificate = NULL;
  return false;
}


PBYTEArray PSSLCertificate::GetData() const
{
  PBYTEArray data;

  if (certificate != NULL) {
    BYTE * certPtr = data.GetPointer(i2d_X509(certificate, NULL));
    i2d_X509(certificate, &certPtr);
  }

  return data;
}


PString PSSLCertificate::AsString() const
{
  return PBase64::Encode(GetData());
}


PBoolean PSSLCertificate::Load(const PFilePath & certFile, PSSLFileTypes fileType)
{
  if (certificate != NULL) {
    X509_free(certificate);
    certificate = NULL;
  }

  PSSL_BIO in;
  if (!in.OpenRead(certFile)) {
    PTRACE(2, "SSL\tCould not open certificate file \"" << certFile << '"');
    return false;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      certificate = d2i_X509_bio(in, NULL);
      if (certificate != NULL)
        break;

      PTRACE(2, "SSL\tInvalid ASN.1 certificate file \"" << certFile << '"');
      return false;

    case PSSLFileTypePEM :
      certificate = PEM_read_bio_X509(in, NULL, NULL, NULL);
      if (certificate != NULL)
        break;

      PTRACE(2, "SSL\tInvalid PEM certificate file \"" << certFile << '"');
      return false;

    default :
      certificate = PEM_read_bio_X509(in, NULL, NULL, NULL);
      if (certificate != NULL)
        break;

      certificate = d2i_X509_bio(in, NULL);
      if (certificate != NULL)
        break;

      PTRACE(2, "SSL\tInvalid certificate file \"" << certFile << '"');
      return false;
  }

  PTRACE(4, "SSL\tLoaded certificate file \"" << certFile << '"');
  return true;
}


PBoolean PSSLCertificate::Save(const PFilePath & certFile, PBoolean append, PSSLFileTypes fileType)
{
  if (certificate == NULL)
    return false;

  PSSL_BIO out;
  if (!(append ? out.OpenAppend(certFile) : out.OpenWrite(certFile))) {
    PTRACE(2, "SSL\tCould not " << (append ? "append to" : "create") << " certificate file \"" << certFile << '"');
    return false;
  }

  if (fileType == PSSLFileTypeDEFAULT)
    fileType = certFile.GetType() == ".der" ? PSSLFileTypeASN1 : PSSLFileTypePEM;

  switch (fileType) {
    case PSSLFileTypeASN1 :
      if (i2d_X509_bio(out, certificate))
        return true;
      break;

    case PSSLFileTypePEM :
      if (PEM_write_bio_X509(out, certificate))
        return true;
      break;

    default :
      PAssertAlways(PInvalidParameter);
      return false;
  }

  PTRACE(2, "SSL\tError writing certificate file \"" << certFile << '"');
  return false;
}


///////////////////////////////////////////////////////////////////////////////

PSSLDiffieHellman::PSSLDiffieHellman()
{
  dh = NULL;
}


PSSLDiffieHellman::PSSLDiffieHellman(const PFilePath & dhFile,
                                     PSSLFileTypes fileType)
{
  dh = NULL;
  Load(dhFile, fileType);
}


PSSLDiffieHellman::PSSLDiffieHellman(const BYTE * pData, PINDEX pSize,
                                     const BYTE * gData, PINDEX gSize)
{
  dh = DH_new();
  if (dh == NULL)
    return;

  dh->p = BN_bin2bn(pData, pSize, NULL);
  dh->g = BN_bin2bn(gData, gSize, NULL);
  if (dh->p != NULL && dh->g != NULL)
    return;

  DH_free(dh);
  dh = NULL;
}


PSSLDiffieHellman::PSSLDiffieHellman(const PSSLDiffieHellman & diffie)
{
  dh = diffie.dh;
}


PSSLDiffieHellman & PSSLDiffieHellman::operator=(const PSSLDiffieHellman & diffie)
{
  if (dh != NULL)
    DH_free(dh);
  dh = diffie.dh;
  return *this;
}


PSSLDiffieHellman::~PSSLDiffieHellman()
{
  if (dh != NULL)
    DH_free(dh);
}

#ifdef P_d2i_DHparams_bio_OLD
// 2/21/04 Yuri Kiryanov - fix for compiler choke on BeOS for usage of
// SSL function d2i_DHparams_bio below in PSSLDiffieHellman::Load
// 5/26/06 Hannes Friederich - Mac OS X seems to need that fix too...
// 3/15/08 Hannes Friederich - Mac OS X 10.5 (Darwin 9.X) no longer needs this
#undef  d2i_DHparams_bio
#define d2i_DHparams_bio(bp,x) \
 (DH *)ASN1_d2i_bio( \
         (char *(*)(...))(void *)DH_new, \
         (char *(*)(...))(void *)d2i_DHparams, \
         (bp), \
         (unsigned char **)(x) \
)
#endif

PBoolean PSSLDiffieHellman::Load(const PFilePath & dhFile, PSSLFileTypes fileType)
{
  if (dh != NULL) {
    DH_free(dh);
    dh = NULL;
  }

  PSSL_BIO in;
  if (!in.OpenRead(dhFile)) {
    PTRACE(2, "SSL\tCould not open DH file \"" << dhFile << '"');
    return false;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      dh = d2i_DHparams_bio(in, NULL);
      if (dh != NULL)
        break;

      PTRACE(2, "SSL\tInvalid ASN.1 DH file \"" << dhFile << '"');
      return false;

    case PSSLFileTypePEM :
      dh = PEM_read_bio_DHparams(in, NULL, NULL, NULL);
      if (dh != NULL)
        break;

      PTRACE(2, "SSL\tInvalid PEM DH file \"" << dhFile << '"');
      return false;

    default :
      dh = PEM_read_bio_DHparams(in, NULL, NULL, NULL);
      if (dh != NULL)
        break;

      dh = d2i_DHparams_bio(in, NULL);
      if (dh != NULL)
        break;

      PTRACE(2, "SSL\tInvalid DH file \"" << dhFile << '"');
      return false;
  }

  PTRACE(4, "SSL\tLoaded DH file \"" << dhFile << '"');
  return false;
}


///////////////////////////////////////////////////////////////////////////////

static void LockingCallback(int mode, int n, const char * /*file*/, int /*line*/)
{
  PSSLInitialiser::GetInstance().LockingCallback(mode, n);
}


void PSSLInitialiser::OnStartup()
{
  SSL_library_init();
  SSL_load_error_strings();

  // Seed the random number generator
  BYTE seed[128];
  for (size_t i = 0; i < sizeof(seed); i++)
    seed[i] = (BYTE)rand();
  RAND_seed(seed, sizeof(seed));

  // set up multithread stuff
  mutexes.resize(CRYPTO_num_locks());
  CRYPTO_set_locking_callback(::LockingCallback);
}


void PSSLInitialiser::OnShutdown()
{
  CRYPTO_set_locking_callback(NULL);
  ERR_free_strings();
}


void PSSLInitialiser::LockingCallback(int mode, int n)
{
  if ((mode & CRYPTO_LOCK) != 0)
    mutexes[n].Wait();
  else
    mutexes[n].Signal();
}


static void InfoCallback(const SSL *ssl, int where, int ret)
{
#if PTRACING
  static const unsigned Level = 4;
  if (PTrace::GetLevel() <= Level) {
    ostream & trace = PTrace::Begin(Level, __FILE__, __LINE__);
    trace << "SSL\t";

    if (where & SSL_CB_ALERT) {
      trace << "Alert "
            << ((where & SSL_CB_READ) ? "read" : "write")
            << ' ' << SSL_alert_type_string_long(ret)
            << ": " << SSL_alert_desc_string_long(ret);
    }
    else {
      if (where & SSL_ST_CONNECT)
        trace << "Connect";
      else if (where & SSL_ST_ACCEPT)
        trace << "Accept";
      else
        trace << "General";

      trace << ": ";

      if (where & SSL_CB_EXIT) {
        if (ret == 0)
          trace << "failed in ";
        else if (ret < 0)
          trace << "error in ";
      }

      trace << SSL_state_string_long(ssl);
    }
    trace << PTrace::End;
  }
#endif // PTRACING
}


static int VerifyCallback(int ok, X509_STORE_CTX * ctx)
{
#if PTRACING
  static const unsigned Level = 3;
  if (PTrace::GetLevel() <= Level) {
    X509 * err_cert = X509_STORE_CTX_get_current_cert(ctx);
    //int err         = X509_STORE_CTX_get_error(ctx);

    // get the subject name, just for verification
    char buf[256];
    X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

    ostream & trace = PTrace::Begin(Level, __FILE__, __LINE__);
    trace << "SSL\tVerify callback depth "
           << X509_STORE_CTX_get_error_depth(ctx)
           << " : cert name = " << buf
           << PTrace::End;
  }
#endif // PTRACING

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


///////////////////////////////////////////////////////////////////////////////

PSSLContext::PSSLContext(Method method, const void * sessionId, PINDEX idSize)
{
  Construct(method, sessionId, idSize);
}


PSSLContext::PSSLContext(const void * sessionId, PINDEX idSize)
{
  Construct(SSLv3, sessionId, idSize);
}

void PSSLContext::Construct(Method method, const void * sessionId, PINDEX idSize)
{
  // create the new SSL context
#if OPENSSL_VERSION_NUMBER >= 0x00909000L
  const
#endif
  SSL_METHOD * meth;

  switch (method) {
    case SSLv3:
      meth = SSLv3_method();
      break;
    case TLSv1:
      meth = TLSv1_method(); 
      break;
    case SSLv23:
    default:
      meth = SSLv23_method();
      break;
  }

  context  = SSL_CTX_new(meth);
  if (context == NULL)
    PSSLAssert("Error creating context: ");

  if (sessionId != NULL) {
    if (idSize == 0)
      idSize = ::strlen((const char *)sessionId)+1;
    SSL_CTX_set_session_id_context(context, (const BYTE *)sessionId, idSize);
    SSL_CTX_sess_set_cache_size(context, 128);
  }

  SSL_CTX_set_info_callback(context, InfoCallback);
  SSL_CTX_set_verify(context, SSL_VERIFY_NONE, VerifyCallback);
}


PSSLContext::~PSSLContext()
{
  SSL_CTX_free(context);
}


PBoolean PSSLContext::SetCAPath(const PDirectory & caPath)
{
  PString path = caPath.Left(caPath.GetLength()-1);
  if (!SSL_CTX_load_verify_locations(context, NULL, path))
    return false;

  return SSL_CTX_set_default_verify_paths(context);
}


PBoolean PSSLContext::AddCA(const PSSLCertificate & certificate)
{
  return SSL_CTX_add_client_CA(context, certificate);
}


PBoolean PSSLContext::UseCertificate(const PSSLCertificate & certificate)
{
  return SSL_CTX_use_certificate(context, certificate) > 0;
}


PBoolean PSSLContext::UsePrivateKey(const PSSLPrivateKey & key)
{
  if (SSL_CTX_use_PrivateKey(context, key) <= 0)
    return false;

  return SSL_CTX_check_private_key(context);
}


PBoolean PSSLContext::UseDiffieHellman(const PSSLDiffieHellman & dh)
{
  return SSL_CTX_set_tmp_dh(context, (dh_st *)dh) > 0;
}


PBoolean PSSLContext::SetCipherList(const PString & ciphers)
{
  if (ciphers.IsEmpty())
    return false;

  return SSL_CTX_set_cipher_list(context, (char *)(const char *)ciphers);
}


/////////////////////////////////////////////////////////////////////////
//
//  SSLChannel
//

PSSLChannel::PSSLChannel(PSSLContext * ctx, PBoolean autoDel)
{
  if (ctx != NULL)
    Construct(ctx, autoDel);
  else
    Construct(new PSSLContext, true);
}


PSSLChannel::PSSLChannel(PSSLContext & ctx)
{
  Construct(&ctx, false);
}


void PSSLChannel::Construct(PSSLContext * ctx, PBoolean autoDel)
{
  context = ctx;
  autoDeleteContext = autoDel;

  ssl = SSL_new(*context);
  if (ssl == NULL)
    PSSLAssert("Error creating channel: ");
  else {
    SSL_set_info_callback(ssl, InfoCallback);
    SSL_set_verify(ssl, SSL_VERIFY_NONE, VerifyCallback);
  }
}


PSSLChannel::~PSSLChannel()
{
  // free the SSL connection
  if (ssl != NULL)
    SSL_free(ssl);

  if (autoDeleteContext)
    delete context;
}


PBoolean PSSLChannel::Read(void * buf, PINDEX len)
{
  flush();

  channelPointerMutex.StartRead();

  lastReadCount = 0;

  PBoolean returnValue = false;
  if (readChannel == NULL)
    SetErrorValues(NotOpen, EBADF, LastReadError);
  else if (readTimeout == 0 && SSL_pending(ssl) == 0)
    SetErrorValues(Timeout, ETIMEDOUT, LastReadError);
  else {
    readChannel->SetReadTimeout(readTimeout);

    int readResult = SSL_read(ssl, (char *)buf, len);
    lastReadCount = readResult;
    returnValue = readResult > 0;
    if (readResult < 0 && GetErrorCode(LastReadError) == NoError)
      ConvertOSError(-1, LastReadError);
  }

  channelPointerMutex.EndRead();

  return returnValue;
}

PBoolean PSSLChannel::Write(const void * buf, PINDEX len)
{
  flush();

  channelPointerMutex.StartRead();

  lastWriteCount = 0;

  PBoolean returnValue;
  if (writeChannel == NULL) {
    SetErrorValues(NotOpen, EBADF, LastWriteError);
    returnValue = false;
  }
  else {
    writeChannel->SetWriteTimeout(writeTimeout);

    int writeResult = SSL_write(ssl, (const char *)buf, len);
    lastWriteCount = writeResult;
    returnValue = lastWriteCount >= len;
    if (writeResult < 0 && GetErrorCode(LastWriteError) == NoError)
      ConvertOSError(-1, LastWriteError);
  }

  channelPointerMutex.EndRead();

  return returnValue;
}


PBoolean PSSLChannel::Close()
{
  PBoolean ok = SSL_shutdown(ssl);
  return PIndirectChannel::Close() && ok;
}


PBoolean PSSLChannel::ConvertOSError(int error, ErrorGroup group)
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


PBoolean PSSLChannel::Accept()
{
  if (IsOpen())
    return ConvertOSError(SSL_accept(ssl));
  return false;
}


PBoolean PSSLChannel::Accept(PChannel & channel)
{
  if (Open(channel))
    return ConvertOSError(SSL_accept(ssl));
  return false;
}


PBoolean PSSLChannel::Accept(PChannel * channel, PBoolean autoDelete)
{
  if (Open(channel, autoDelete))
    return ConvertOSError(SSL_accept(ssl));
  return false;
}


PBoolean PSSLChannel::Connect()
{
  if (IsOpen())
    return ConvertOSError(SSL_connect(ssl));
  return false;
}


PBoolean PSSLChannel::Connect(PChannel & channel)
{
  if (Open(channel))
    return ConvertOSError(SSL_connect(ssl));
  return false;
}


PBoolean PSSLChannel::Connect(PChannel * channel, PBoolean autoDelete)
{
  if (Open(channel, autoDelete))
    return ConvertOSError(SSL_connect(ssl));
  return false;
}


PBoolean PSSLChannel::AddCA(const PSSLCertificate & certificate)
{
  return SSL_add_client_CA(ssl, certificate);
}


PBoolean PSSLChannel::UseCertificate(const PSSLCertificate & certificate)
{
  return SSL_use_certificate(ssl, certificate);
}


PBoolean PSSLChannel::UsePrivateKey(const PSSLPrivateKey & key)
{
  if (SSL_use_PrivateKey(ssl, key) <= 0)
    return false;

  return SSL_check_private_key(ssl);
}


PString PSSLChannel::GetCipherList() const
{
  PStringStream strm;
  int i = -1;
  const char * str;
  while ((str = SSL_get_cipher_list(ssl,++i)) != NULL) {
    if (i > 0)
      strm << ':';
    strm << str;
  }

  return strm;
}


void PSSLChannel::SetVerifyMode(VerifyMode mode)
{
  if (ssl == NULL)
    return;

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

  SSL_set_verify(ssl, verify, VerifyCallback);
}


PBoolean PSSLChannel::RawSSLRead(void * buf, PINDEX & len)
{
  if (!PIndirectChannel::Read(buf, len)) 
    return false;

  len = GetLastReadCount();
  return true;
}


//////////////////////////////////////////////////////////////////////////
//
//  Low level interface to SSLEay routines
//


#define PSSLCHANNEL(bio)      ((PSSLChannel *)(bio->ptr))

extern "C" {

#if (OPENSSL_VERSION_NUMBER < 0x00906000)

typedef int (*ifptr)();
typedef long (*lfptr)();

#endif

static int Psock_new(BIO * bio)
{
  bio->init     = 0;
  bio->num      = 0;
  bio->ptr      = NULL;    // this is really (PSSLChannel *)
  bio->flags    = 0;

  return(1);
}


static int Psock_free(BIO * bio)
{
  if (bio == NULL)
    return 0;

  if (bio->shutdown) {
    if (bio->init) {
      PSSLCHANNEL(bio)->Shutdown(PSocket::ShutdownReadAndWrite);
      PSSLCHANNEL(bio)->Close();
    }
    bio->init  = 0;
    bio->flags = 0;
  }
  return 1;
}


static long Psock_ctrl(BIO * bio, int cmd, long num, void * /*ptr*/)
{
  switch (cmd) {
    case BIO_CTRL_SET_CLOSE:
      bio->shutdown = (int)num;
      return 1;

    case BIO_CTRL_GET_CLOSE:
      return bio->shutdown;

    case BIO_CTRL_FLUSH:
      return 1;
  }

  // Other BIO commands, return 0
  return 0;
}


static int Psock_read(BIO * bio, char * out, int outl)
{
  if (out == NULL)
    return 0;

  BIO_clear_retry_flags(bio);

  // Skip over the polymorphic read, want to do real one
  PINDEX len = outl;
  if (PSSLCHANNEL(bio)->RawSSLRead(out, len))
    return len;

  switch (PSSLCHANNEL(bio)->GetErrorCode(PChannel::LastReadError)) {
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
  if (PSSLCHANNEL(bio)->PIndirectChannel::Write(in, inl))
    return PSSLCHANNEL(bio)->GetLastWriteCount();

  switch (PSSLCHANNEL(bio)->GetErrorCode(PChannel::LastWriteError)) {
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
  "PTLib-PSSLChannel",
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


PBoolean PSSLChannel::OnOpen()
{
  BIO * bio = BIO_new(&methods_Psock);
  if (bio == NULL) {
    PTRACE(2, "SSL\tCould not open BIO");
    return false;
  }

  // "Open" then bio
  bio->ptr  = this;
  bio->init = 1;

  SSL_set_bio(ssl, bio, bio);
  return true;
}


#else

  #ifdef _MSC_VER
    #pragma message("SSL support (via OpenSSL) DISABLED")
  #endif

#endif // P_SSL


// End of file ////////////////////////////////////////////////////////////////
