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

#pragma message("SSL support (via OpenSSL) enabled")

#define USE_SOCKETS

extern "C" {

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/x509v3.h>

#ifdef P_SSL_AES
  #include <openssl/aes.h>
#endif
};


#ifdef _MSC_VER
  #pragma comment(lib, P_SSL_LIB1)
  #pragma comment(lib, P_SSL_LIB2)
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
  : m_pkey(NULL)
{
}


PSSLPrivateKey::PSSLPrivateKey(unsigned modulus,
                               void (*callback)(int,int,void *),
                               void *cb_arg)
  : m_pkey(NULL)
{
  Create(modulus, callback, cb_arg);
}


PSSLPrivateKey::PSSLPrivateKey(const PFilePath & keyFile, PSSLFileTypes fileType)
  : m_pkey(NULL)
{
  Load(keyFile, fileType);
}


PSSLPrivateKey::PSSLPrivateKey(const BYTE * keyData, PINDEX keySize)
  : m_pkey(NULL)
{
  SetData(PBYTEArray(keyData, keySize, false));
}


PSSLPrivateKey::PSSLPrivateKey(const PBYTEArray & keyData)
  : m_pkey(NULL)
{
  SetData(keyData);
}


PSSLPrivateKey::PSSLPrivateKey(const PSSLPrivateKey & privKey)
{
  SetData(privKey.GetData());
}


PSSLPrivateKey::PSSLPrivateKey(evp_pkey_st * privKey, bool duplicate)
{
  if (privKey == NULL || !duplicate)
    m_pkey = privKey;
  else {
    m_pkey = privKey;
    PBYTEArray data = GetData();
    m_pkey = NULL;
    SetData(data);
  }
}


PSSLPrivateKey & PSSLPrivateKey::operator=(const PSSLPrivateKey & privKey)
{
  if (this != &privKey) {
    FreePrivateKey();
    m_pkey = privKey.m_pkey;
  }
  return *this;
}


PSSLPrivateKey & PSSLPrivateKey::operator=(evp_pkey_st * privKey)
{
  if (m_pkey != privKey) {
    FreePrivateKey();
    m_pkey = privKey;
  }
  return *this;
}


PSSLPrivateKey::~PSSLPrivateKey()
{
  FreePrivateKey();
}


void PSSLPrivateKey::FreePrivateKey()
{
  if (m_pkey != NULL) {
    EVP_PKEY_free(m_pkey);
    m_pkey = NULL;
  }
}


void PSSLPrivateKey::Attach(evp_pkey_st * key)
{
  FreePrivateKey();
  m_pkey = key;
}


PBoolean PSSLPrivateKey::Create(unsigned modulus,
                            void (*callback)(int,int,void *),
                            void *cb_arg)
{
  FreePrivateKey();

  if (!PAssert(modulus >= 384, PInvalidParameter))
    return false;

  m_pkey = EVP_PKEY_new();
  if (m_pkey == NULL)
    return false;

  if (EVP_PKEY_assign_RSA(m_pkey, RSA_generate_key(modulus, 0x10001, callback, cb_arg)))
    return true;

  FreePrivateKey();
  return false;
}


bool PSSLPrivateKey::SetData(const PBYTEArray & keyData)
{
  FreePrivateKey();

  const BYTE * keyPtr = keyData;
#if P_SSL_USE_CONST
  m_pkey = d2i_AutoPrivateKey(NULL, &keyPtr, keyData.GetSize());
#else
  m_pkey = d2i_AutoPrivateKey(NULL, (BYTE **)&keyPtr, keyData.GetSize());
#endif

  return m_pkey != NULL;
}


PBYTEArray PSSLPrivateKey::GetData() const
{
  PBYTEArray data;

  if (m_pkey != NULL) {
    BYTE * keyPtr = data.GetPointer(i2d_PrivateKey(m_pkey, NULL));
    i2d_PrivateKey(m_pkey, &keyPtr);
  }

  return data;
}


PString PSSLPrivateKey::AsString() const
{
  return PBase64::Encode(GetData());
}


bool PSSLPrivateKey::Parse(const PString & keyStr)
{
  PBYTEArray keyData;
  return PBase64::Decode(keyStr, keyData) && SetData(keyData);
}


PBoolean PSSLPrivateKey::Load(const PFilePath & keyFile, PSSLFileTypes fileType)
{
  FreePrivateKey();

  PSSL_BIO in;
  if (!in.OpenRead(keyFile)) {
    PTRACE(2, "SSL\tCould not open private key file \"" << keyFile << '"');
    return false;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      m_pkey = d2i_PrivateKey_bio(in, NULL);
      if (m_pkey != NULL)
        break;

      PTRACE(2, "SSL\tInvalid ASN.1 private key file \"" << keyFile << '"');
      return false;

    case PSSLFileTypePEM :
      m_pkey = PEM_read_bio_PrivateKey(in, NULL, NULL, NULL);
      if (m_pkey != NULL)
        break;

      PTRACE(2, "SSL\tInvalid PEM private key file \"" << keyFile << '"');
      return false;

    default :
      m_pkey = PEM_read_bio_PrivateKey(in, NULL, NULL, NULL);
      if (m_pkey != NULL)
        break;

      m_pkey = d2i_PrivateKey_bio(in, NULL);
      if (m_pkey != NULL)
        break;

      PTRACE(2, "SSL\tInvalid private key file \"" << keyFile << '"');
      return false;
  }

  PTRACE(4, "SSL\tLoaded private key file \"" << keyFile << '"');
  return true;
}


PBoolean PSSLPrivateKey::Save(const PFilePath & keyFile, PBoolean append, PSSLFileTypes fileType)
{
  if (m_pkey == NULL)
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
      if (i2d_PrivateKey_bio(out, m_pkey))
        return true;
      break;

    case PSSLFileTypePEM :
      if (PEM_write_bio_PrivateKey(out, m_pkey, NULL, NULL, 0, 0, NULL))
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
  : m_certificate(NULL)
{
}


PSSLCertificate::PSSLCertificate(const PFilePath & certFile, PSSLFileTypes fileType)
  : m_certificate(NULL)
{
  Load(certFile, fileType);
}


PSSLCertificate::PSSLCertificate(const BYTE * certData, PINDEX certSize)
  : m_certificate(NULL)
{
  SetData(PBYTEArray(certData, certSize, false));
}


PSSLCertificate::PSSLCertificate(const PBYTEArray & certData)
  : m_certificate(NULL)
{
  SetData(certData);
}


PSSLCertificate::PSSLCertificate(const PString & certStr)
  : m_certificate(NULL)
{
  Parse(certStr);
}


PSSLCertificate::PSSLCertificate(const PSSLCertificate & cert)
{
  if (cert.m_certificate == NULL)
    m_certificate = NULL;
  else
    m_certificate = X509_dup(cert.m_certificate);
}


PSSLCertificate::PSSLCertificate(x509_st * cert, bool duplicate)
{
  if (cert == NULL)
    m_certificate = NULL;
  else if (duplicate)
    m_certificate = X509_dup(cert);
  else
    m_certificate = cert;
}


PSSLCertificate & PSSLCertificate::operator=(const PSSLCertificate & cert)
{
  if (this != &cert) {
    FreeCertificate();

    if (cert.m_certificate != NULL)
      m_certificate = X509_dup(cert.m_certificate);
  }
  return *this;
}


PSSLCertificate & PSSLCertificate::operator=(x509_st * cert)
{
  if (m_certificate !=  cert) {
    FreeCertificate();

    if (cert != NULL)
      m_certificate = X509_dup(cert);
  }
  return *this;
}


PSSLCertificate::~PSSLCertificate()
{
  FreeCertificate();
}


void PSSLCertificate::FreeCertificate()
{
  if (m_certificate != NULL) {
    X509_free(m_certificate);
    m_certificate = NULL;
  }
}


void PSSLCertificate::Attach(x509_st * cert)
{
  if (m_certificate != cert) {
    FreeCertificate();
    m_certificate = cert;
  }
}


PBoolean PSSLCertificate::CreateRoot(const PString & subject,
                                 const PSSLPrivateKey & privateKey)
{
  FreeCertificate();

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

  m_certificate = X509_new();
  if (m_certificate == NULL)
    return false;

  if (X509_set_version(m_certificate, 2)) {
    /* Set version to V3 */
    ASN1_INTEGER_set(X509_get_serialNumber(m_certificate), 0L);

    X509_NAME * name = X509_NAME_new();
    for (POrdinalToString::iterator it = info.begin(); it != info.end(); ++it)
      X509_NAME_add_entry_by_NID(name,
                                 it->first,
                                 MBSTRING_ASC,
                                 (unsigned char *)(const char *)it->second,
                                 -1,-1, 0);
    X509_set_issuer_name(m_certificate, name);
    X509_set_subject_name(m_certificate, name);
    X509_NAME_free(name);

    X509_gmtime_adj(X509_get_notBefore(m_certificate), 0);
    X509_gmtime_adj(X509_get_notAfter(m_certificate), (long)60*60*24*365*5);

    X509_PUBKEY * pubkey = X509_PUBKEY_new();
    if (pubkey != NULL) {
      X509_PUBKEY_set(&pubkey, privateKey);
      EVP_PKEY * pkey = X509_PUBKEY_get(pubkey);
      X509_set_pubkey(m_certificate, pkey);
      EVP_PKEY_free(pkey);
      X509_PUBKEY_free(pubkey);

      if (X509_sign(m_certificate, privateKey, EVP_md5()) > 0)
        return true;
    }
  }

  FreeCertificate();
  return false;
}


bool PSSLCertificate::SetData(const PBYTEArray & certData)
{
  FreeCertificate();

  const BYTE * certPtr = certData;
#if P_SSL_USE_CONST
  m_certificate = d2i_X509(NULL, &certPtr, certData.GetSize());
#else
  m_certificate = d2i_X509(NULL, (unsigned char **)&certPtr, certData.GetSize());
#endif
  return m_certificate != NULL;
}


PBYTEArray PSSLCertificate::GetData() const
{
  PBYTEArray data;

  if (m_certificate != NULL) {
    BYTE * certPtr = data.GetPointer(i2d_X509(m_certificate, NULL));
    i2d_X509(m_certificate, &certPtr);
  }

  return data;
}


PString PSSLCertificate::AsString() const
{
  return PBase64::Encode(GetData());
}


bool PSSLCertificate::Parse(const PString & certStr)
{
  PBYTEArray certData;
  return PBase64::Decode(certStr, certData) && SetData(certData);
}


PBoolean PSSLCertificate::Load(const PFilePath & certFile, PSSLFileTypes fileType)
{
  FreeCertificate();

  PSSL_BIO in;
  if (!in.OpenRead(certFile)) {
    PTRACE(2, "SSL\tCould not open certificate file \"" << certFile << '"');
    return false;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      m_certificate = d2i_X509_bio(in, NULL);
      if (m_certificate != NULL)
        break;

      PTRACE(2, "SSL\tInvalid ASN.1 certificate file \"" << certFile << '"');
      return false;

    case PSSLFileTypePEM :
      m_certificate = PEM_read_bio_X509(in, NULL, NULL, NULL);
      if (m_certificate != NULL)
        break;

      PTRACE(2, "SSL\tInvalid PEM certificate file \"" << certFile << '"');
      return false;

    default :
      m_certificate = PEM_read_bio_X509(in, NULL, NULL, NULL);
      if (m_certificate != NULL)
        break;

      m_certificate = d2i_X509_bio(in, NULL);
      if (m_certificate != NULL)
        break;

      PTRACE(2, "SSL\tInvalid certificate file \"" << certFile << '"');
      return false;
  }

  PTRACE(4, "SSL\tLoaded certificate file \"" << certFile << '"');
  return true;
}


PBoolean PSSLCertificate::Save(const PFilePath & certFile, PBoolean append, PSSLFileTypes fileType)
{
  if (m_certificate == NULL)
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
      if (i2d_X509_bio(out, m_certificate))
        return true;
      break;

    case PSSLFileTypePEM :
      if (PEM_write_bio_X509(out, m_certificate))
        return true;
      break;

    default :
      PAssertAlways(PInvalidParameter);
      return false;
  }

  PTRACE(2, "SSL\tError writing certificate file \"" << certFile << '"');
  return false;
}


bool PSSLCertificate::GetSubjectName(X509_Name & name) const
{
  if (m_certificate == NULL)
    return false;

  name = X509_Name(X509_get_subject_name(m_certificate));
  return name.IsValid();
}


PString PSSLCertificate::GetSubjectName() const
{
  X509_Name name;
  return GetSubjectName(name) ? name.AsString() : PString::Empty();
}


bool PSSLCertificate::GetIssuerName(X509_Name & name) const
{
  if (m_certificate == NULL)
    return false;

  name = X509_Name(X509_get_subject_name(m_certificate));
  return name.IsValid();
}


static PString From_ASN1_STRING(ASN1_STRING * asn)
{
  PString str;
  if (asn != NULL) {
    unsigned char * utf8;
    int len = ASN1_STRING_to_UTF8(&utf8, asn);
    str = PString((const char *)utf8, len);
    OPENSSL_free(utf8);
  }
  return str;
}


PString PSSLCertificate::GetSubjectAltName() const
{
  if (m_certificate == NULL)
    return PString::Empty();

  const GENERAL_NAMES * sANs = (const GENERAL_NAMES *)X509_get_ext_d2i(m_certificate, NID_subject_alt_name, 0, 0);
  if (sANs == NULL)
    return PString::Empty();
 
  int numAN = sk_GENERAL_NAME_num(sANs);
  for (int i = 0; i < numAN; ++i) {
    GENERAL_NAME * sAN = sk_GENERAL_NAME_value(sANs, i);
    // we only care about DNS entries
    if (sAN->type == GEN_DNS)
      return From_ASN1_STRING(sAN->d.dNSName);
  }

  return PString::Empty();
}


PObject::Comparison PSSLCertificate::X509_Name::Compare(const PObject & other) const
{
  int cmp = X509_NAME_cmp(m_name, dynamic_cast<const X509_Name &>(other).m_name);
  if (cmp < 0)
    return LessThan;
  if (cmp > 0)
    return GreaterThan;
  return EqualTo;
}


void PSSLCertificate::X509_Name::PrintOn(ostream & strm) const
{
  strm << AsString();
}


PString PSSLCertificate::X509_Name::GetCommonName() const
{
  return GetNID(NID_commonName);
}


PString PSSLCertificate::X509_Name::GetNID(int id) const
{
  if (m_name != NULL) {
    X509_NAME_ENTRY * entry = X509_NAME_get_entry(m_name, X509_NAME_get_index_by_NID(m_name, id, -1));
    if (entry != NULL)
      return From_ASN1_STRING(X509_NAME_ENTRY_get_data(entry));
  }

  return PString::Empty();
}


PString PSSLCertificate::X509_Name::AsString(int indent) const
{
  PString str;

  if (m_name == NULL)
    return str;

  BIO * bio = BIO_new(BIO_s_mem());
  if (bio == NULL)
    return str;

  X509_NAME_print_ex(bio, m_name, std::max(0, indent), indent < 0 ? XN_FLAG_ONELINE : XN_FLAG_MULTILINE);

  char * data;
  int len = BIO_get_mem_data(bio, &data);
  str = PString(data, len);

  (void)BIO_set_close(bio, BIO_CLOSE);
  BIO_free(bio);
  return str;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef P_SSL_AES
PAESContext::PAESContext()
  : m_key(new AES_KEY)
{
}


PAESContext::PAESContext(bool encrypt, const void * data, PINDEX numBits)
  : m_key(new AES_KEY)
{
  if (encrypt)
    SetEncrypt(data, numBits);
  else
    SetEncrypt(data, numBits);
}


PAESContext::~PAESContext()
{
  delete m_key;
}


void PAESContext::SetEncrypt(const void * data, PINDEX numBits)
{
  AES_set_encrypt_key((const unsigned char *)data, numBits, m_key);
}


void PAESContext::SetDecrypt(const void * data, PINDEX numBits)
{
  AES_set_decrypt_key((const unsigned char *)data, numBits, m_key);
}


void PAESContext::Encrypt(const void * in, void * out)
{
  AES_encrypt((const unsigned char *)in, (unsigned char *)out, m_key);
}


void PAESContext::Decrypt(const void * in, void * out)
{
  AES_decrypt((const unsigned char *)in, (unsigned char *)out, m_key);
}
#endif // P_SSL_AES


///////////////////////////////////////////////////////////////////////////////

PSHA1Context::PSHA1Context()
  : m_context(new SHA_CTX)
{
  SHA1_Init(m_context);
}


PSHA1Context::~PSHA1Context()
{
  delete m_context;
}


void PSHA1Context::Update(const void * data, PINDEX length)
{
  SHA1_Update(m_context, data, length);
}


void PSHA1Context::Finalise(BYTE * result)
{
  SHA1_Final(result, m_context);
}


void PSHA1Context::Process(const void * data, PINDEX length, Digest result)
{
  SHA1((const unsigned char *)data, length, result);
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


static void InfoCallback(const SSL * PTRACE_PARAM(ssl), int PTRACE_PARAM(where), int PTRACE_PARAM(ret))
{
#if PTRACING
  static const unsigned Level = 4;
  if (PTrace::GetLevel() >= Level) {
    ostream & trace = PTRACE_BEGIN(Level);
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


static int VerifyCallback(int ok, X509_STORE_CTX * PTRACE_PARAM(ctx))
{
#if PTRACING
  const unsigned Level = ok ? 5 : 2;
  if (PTrace::GetLevel() >= Level) {
    int err = X509_STORE_CTX_get_error(ctx);
    int depth = X509_STORE_CTX_get_error_depth(ctx);
    PSSLCertificate cert(X509_STORE_CTX_get_current_cert(ctx));
    PSSLCertificate::X509_Name issuer, subject;
    cert.GetIssuerName(issuer);
    cert.GetSubjectName(subject);

    PTRACE_BEGIN(Level)
        << "SSL\tVerify callback: depth="
        << depth
        << ", err=" << err << " - " << X509_verify_cert_error_string(err)
        << "\n  Subject:\n" << subject.AsString(4)
        << "\n  Issuer:\n" << issuer.AsString(4)
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
  Construct(TLSv1, sessionId, idSize);
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

  m_context = SSL_CTX_new(meth);
  if (m_context == NULL)
    PSSLAssert("Error creating context: ");

  if (sessionId != NULL) {
    if (idSize == 0)
      idSize = ::strlen((const char *)sessionId)+1;
    SSL_CTX_set_session_id_context(m_context, (const BYTE *)sessionId, idSize);
    SSL_CTX_sess_set_cache_size(m_context, 128);
  }

  SSL_CTX_set_info_callback(m_context, InfoCallback);
  SetVerifyMode(VerifyNone);
}


PSSLContext::~PSSLContext()
{
  SSL_CTX_free(m_context);
}


bool PSSLContext::SetVerifyLocations(const PFilePath & caFile, const PDirectory & caDir)
{
  PString caPath = caDir.Left(caDir.GetLength()-1);
  if (SSL_CTX_load_verify_locations(m_context, caFile.IsEmpty() ? NULL : (const char *)caFile,
                                               caPath.IsEmpty() ? NULL : (const char *)caPath)) {
    PTRACE(4, "SSL\tSet context " << m_context << " verify locations file=\"" << caFile << "\", dir=\"" << caDir << '"');
    return true;
  }

  PTRACE(2, "SSL\tCould not set context " << m_context << " verify locations file=\"" << caFile << "\", dir=\"" << caDir << '"');
  return SSL_CTX_set_default_verify_paths(m_context);
}


bool PSSLContext::SetVerifyCertificate(const PSSLCertificate & cert)
{
  if (m_context == NULL || !cert.IsValid())
    return false;

  X509_STORE * store = SSL_CTX_get_cert_store(m_context);
  if (store == NULL)
    return false;

  return X509_STORE_add_cert(store, cert);
}


static int VerifyModeBits[PSSLContext::EndVerifyMode] = {
  SSL_VERIFY_NONE,
  SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
  SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | SSL_VERIFY_FAIL_IF_NO_PEER_CERT
};


void PSSLContext::SetVerifyMode(VerifyMode mode, unsigned depth)
{
  if (m_context == NULL)
    return;

  SSL_CTX_set_verify(m_context, VerifyModeBits[mode], VerifyCallback);
  SSL_CTX_set_verify_depth(m_context, depth);
}


PSSLContext::VerifyMode PSSLContext::GetVerifyMode() const
{
  if (m_context == NULL)
    return VerifyNone;

  int v = SSL_CTX_get_verify_mode(m_context);
  if (v == SSL_VERIFY_NONE)
    return VerifyNone;
  if ((v&SSL_VERIFY_FAIL_IF_NO_PEER_CERT) == 0)
    return VerifyPeer;
  return VerifyPeerMandatory;
}


bool PSSLContext::AddClientCA(const PSSLCertificate & certificate)
{
  return SSL_CTX_add_client_CA(m_context, certificate);
}


bool PSSLContext::AddClientCA(const PList<PSSLCertificate> & certificates)
{
  for (PList<PSSLCertificate>::const_iterator it = certificates.begin(); it != certificates.end(); ++it) {
    if (!SSL_CTX_add_client_CA(m_context, *it))
      return false;
  }

  return true;
}


bool PSSLContext::UseCertificate(const PSSLCertificate & certificate)
{
  return SSL_CTX_use_certificate(m_context, certificate) > 0;
}


bool PSSLContext::UsePrivateKey(const PSSLPrivateKey & key)
{
  if (SSL_CTX_use_PrivateKey(m_context, key) <= 0)
    return false;

  return SSL_CTX_check_private_key(m_context);
}


bool PSSLContext::UseDiffieHellman(const PSSLDiffieHellman & dh)
{
  return SSL_CTX_set_tmp_dh(m_context, (dh_st *)dh) > 0;
}


bool PSSLContext::SetCipherList(const PString & ciphers)
{
  if (ciphers.IsEmpty())
    return false;

  return SSL_CTX_set_cipher_list(m_context, (char *)(const char *)ciphers);
}


bool PSSLContext::SetCredentials(const PString & authority,
                                 const PString & certificate,
                                 const PString & privateKey,
                                 bool create)
{
  if (!authority.IsEmpty()) {
    bool ok;
    if (PDirectory::Exists(authority))
      ok = SetVerifyLocations(PString::Empty(), authority);
    else if (PFile::Exists(authority))
      ok = SetVerifyLocations(authority, PString::Empty());
    else
      ok = SetVerifyCertificate(PSSLCertificate(authority));
    if (!ok) {
      PTRACE(2, "SSL\tCould not find/parse certificate authority \"" << authority << '"');
      return false;
    }
    SetVerifyMode(VerifyPeerMandatory);
  }

  if (certificate.IsEmpty() && privateKey.IsEmpty())
    return true;

  PSSLCertificate cert;
  PSSLPrivateKey key;

  if (PFile::Exists(certificate) && !cert.Load(certificate)) {
    PTRACE(2, "SSL\tCould not load certificate file \"" << certificate << '"');
    return false;
  }

  if (PFile::Exists(privateKey) && !key.Load(privateKey)) {
    PTRACE(2, "SSL\tCould not load private key file \"" << privateKey << '"');
    return false;
  }

  if (!key.IsValid() && !key.Parse(certificate)) {
    PTRACE(2, "SSL\tCould not parse certificate \"" << certificate << '"');
    return false;
  }

  if (!cert.IsValid() && !cert.Parse(privateKey)) {
    PTRACE(2, "SSL\tCould not parse private key \"" << privateKey << '"');
    return false;
  }

  if (!cert.IsValid() || !key.IsValid()) {

    if (cert.IsValid() || key.IsValid()) {
      PTRACE(2, "SSL\tRequire both certificate and private key");
      return false;
    }

    if (!create) {
      PTRACE(2, "SSL\tRequire certificate and private key");
      return false;
    }

    PStringStream dn;
    dn << "/O=" << PProcess::Current().GetManufacturer()
       << "/CN=" << PIPSocket::GetHostName();

    PSSLPrivateKey key(2048);
    PSSLCertificate root;
    if (!root.CreateRoot(dn, key)) {
      PTRACE(1, "SSL\tCould not create certificate");
      return false;
    }

    root.Save(certificate);
    PTRACE(2, "SSL\tCreated new certificate file \"" << certificate << '"');

    key.Save(privateKey, true);
    PTRACE(2, "SSL\tCreated new private key file \"" << privateKey << '"');
  }

  if (!UseCertificate(cert)) {
    PTRACE(1, "SSL\tCould not use certificate " << cert);
    return false;
  }

  if (!UsePrivateKey(key)) {
    PTRACE(1, "SSL\tCould not use private key " << key);
    return false;
  }

  return true;
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
  m_context = ctx;
  m_autoDeleteContext = autoDel;

  m_ssl = SSL_new(*m_context);
  if (m_ssl == NULL)
    PSSLAssert("Error creating channel: ");
}


PSSLChannel::~PSSLChannel()
{
  // free the SSL connection
  if (m_ssl != NULL)
    SSL_free(m_ssl);

  if (m_autoDeleteContext)
    delete m_context;
}


PBoolean PSSLChannel::Read(void * buf, PINDEX len)
{
  flush();

  channelPointerMutex.StartRead();

  lastReadCount = 0;

  PBoolean returnValue = false;
  if (readChannel == NULL)
    SetErrorValues(NotOpen, EBADF, LastReadError);
  else if (readTimeout == 0 && SSL_pending(m_ssl) == 0)
    SetErrorValues(Timeout, ETIMEDOUT, LastReadError);
  else {
    readChannel->SetReadTimeout(readTimeout);

    int readResult = SSL_read(m_ssl, (char *)buf, len);
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

    int writeResult = SSL_write(m_ssl, (const char *)buf, len);
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
  PBoolean ok = SSL_shutdown(m_ssl);
  return PIndirectChannel::Close() && ok;
}


PBoolean PSSLChannel::ConvertOSError(P_INT_PTR error, ErrorGroup group)
{
  Errors lastError = NoError;
  DWORD osError = 0;
  if (SSL_get_error(m_ssl, (int)error) != SSL_ERROR_NONE && (osError = ERR_peek_error()) != 0) {
    osError |= 0x80000000;
    lastError = AccessDenied;
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
    return ConvertOSError(SSL_accept(m_ssl));
  return false;
}


PBoolean PSSLChannel::Accept(PChannel & channel)
{
  if (Open(channel))
    return ConvertOSError(SSL_accept(m_ssl));
  return false;
}


PBoolean PSSLChannel::Accept(PChannel * channel, PBoolean autoDelete)
{
  if (Open(channel, autoDelete))
    return ConvertOSError(SSL_accept(m_ssl));
  return false;
}


PBoolean PSSLChannel::Connect()
{
  if (IsOpen())
    return ConvertOSError(SSL_connect(m_ssl));
  return false;
}


PBoolean PSSLChannel::Connect(PChannel & channel)
{
  if (Open(channel))
    return ConvertOSError(SSL_connect(m_ssl));
  return false;
}


PBoolean PSSLChannel::Connect(PChannel * channel, PBoolean autoDelete)
{
  if (Open(channel, autoDelete))
    return ConvertOSError(SSL_connect(m_ssl));
  return false;
}


PBoolean PSSLChannel::AddClientCA(const PSSLCertificate & certificate)
{
  return SSL_add_client_CA(m_ssl, certificate);
}


PBoolean PSSLChannel::AddClientCA(const PList<PSSLCertificate> & certificates)
{
  for (PList<PSSLCertificate>::const_iterator it = certificates.begin(); it != certificates.end(); ++it) {
    if (!SSL_add_client_CA(m_ssl, *it))
      return false;
  }

  return true;
}


PBoolean PSSLChannel::UseCertificate(const PSSLCertificate & certificate)
{
  return SSL_use_certificate(m_ssl, certificate);
}


PBoolean PSSLChannel::UsePrivateKey(const PSSLPrivateKey & key)
{
  if (SSL_use_PrivateKey(m_ssl, key) <= 0)
    return false;

  return SSL_check_private_key(m_ssl);
}


PString PSSLChannel::GetCipherList() const
{
  PStringStream strm;
  int i = -1;
  const char * str;
  while ((str = SSL_get_cipher_list(m_ssl,++i)) != NULL) {
    if (i > 0)
      strm << ':';
    strm << str;
  }

  return strm;
}


void PSSLChannel::SetVerifyMode(VerifyMode mode)
{
  if (m_ssl != NULL)
    SSL_set_verify(m_ssl, VerifyModeBits[mode], VerifyCallback);
}


bool PSSLChannel::GetPeerCertificate(PSSLCertificate & certificate, PString * error)
{
  long err = SSL_get_verify_result(m_ssl);
  certificate.Attach(SSL_get_peer_certificate(m_ssl));

  if (err == X509_V_OK && certificate.IsValid())
    return true;

  if (error != NULL) {
    if (err != X509_V_OK)
      *error = X509_verify_cert_error_string(err);
    else
      *error = "Peer did not offer certificate";
  }

  return (SSL_get_verify_mode(m_ssl)&SSL_VERIFY_FAIL_IF_NO_PEER_CERT) == 0;
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
      BIO_set_retry_read(bio);
    case PChannel::Timeout :
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
      BIO_set_retry_write(bio);
    case PChannel::Timeout :
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

  SSL_set_bio(m_ssl, bio, bio);
  return true;
}


#else
  #pragma message("SSL support (via OpenSSL) DISABLED")
#endif // P_SSL


// End of file ////////////////////////////////////////////////////////////////
