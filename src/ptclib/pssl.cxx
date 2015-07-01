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

#if P_SSL

#define USE_SOCKETS

extern "C" {

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/x509v3.h>
#include <openssl/evp.h>

#ifdef P_SSL_AES
  #include <openssl/aes.h>
#endif
};

#if (OPENSSL_VERSION_NUMBER < 0x00906000)
  #error OpenSSL too old!
#endif

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

#define PTraceModule() "SSL"


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


static PString PSSLError(unsigned long err = ERR_peek_error())
{
  char buf[200];
  ERR_error_string_n(err, buf, sizeof(buf));
  if (buf[0] == '\0')
    sprintf(buf, "code=%lu", err);
  return buf;
}

#define PSSLAssert(prefix) PAssertAlways(prefix + PSSLError())


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

static int PasswordCallback(char *buf, int size, int rwflag, void *userdata)
{
  if (!PAssert(userdata != NULL, PLogicError))
    return 0;

  PSSLPasswordNotifier & notifier = *reinterpret_cast<PSSLPasswordNotifier *>(userdata);
  if (!PAssert(!notifier.IsNULL(), PLogicError))
    return 0;

  PString password;
  notifier(password, rwflag != 0);

  int len = password.GetLength()+1;
  if (len > size)
    len = size;
  memcpy(buf, password.GetPointer(), len); // Include '\0'
  return len-1;
}


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


PBoolean PSSLPrivateKey::Load(const PFilePath & keyFile, PSSLFileTypes fileType, const PSSLPasswordNotifier & notifier)
{
  FreePrivateKey();

  PSSL_BIO in;
  if (!in.OpenRead(keyFile)) {
    PTRACE(2, "Could not open private key file \"" << keyFile << '"');
    return false;
  }

  pem_password_cb *cb;
  void *ud;
  if (notifier.IsNULL()) {
    cb = NULL;
    ud = NULL;
  }
  else {
    cb = PasswordCallback;
    ud = (void *)&notifier;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      m_pkey = d2i_PrivateKey_bio(in, NULL);
      if (m_pkey != NULL)
        break;

      PTRACE(2, "Invalid ASN.1 private key file \"" << keyFile << '"');
      return false;

    case PSSLFileTypePEM :
      m_pkey = PEM_read_bio_PrivateKey(in, NULL, cb, ud);
      if (m_pkey != NULL)
        break;

      PTRACE(2, "Invalid PEM private key file \"" << keyFile << '"');
      return false;

    default :
      m_pkey = PEM_read_bio_PrivateKey(in, NULL, cb, ud);
      if (m_pkey != NULL)
        break;

      m_pkey = d2i_PrivateKey_bio(in, NULL);
      if (m_pkey != NULL)
        break;

      PTRACE(2, "Invalid private key file \"" << keyFile << '"');
      return false;
  }

  PTRACE(4, "Loaded private key file \"" << keyFile << '"');
  return true;
}


PBoolean PSSLPrivateKey::Save(const PFilePath & keyFile, PBoolean append, PSSLFileTypes fileType)
{
  if (m_pkey == NULL)
    return false;

  PSSL_BIO out;
  if (!(append ? out.OpenAppend(keyFile) : out.OpenWrite(keyFile))) {
    PTRACE(2, "Could not " << (append ? "append to" : "create") << " private key file \"" << keyFile << '"');
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

  PTRACE(2, "Error writing certificate file \"" << keyFile << '"');
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
    PTRACE(2, "Could not open certificate file \"" << certFile << '"');
    return false;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      m_certificate = d2i_X509_bio(in, NULL);
      if (m_certificate != NULL)
        break;

      PTRACE(2, "Invalid ASN.1 certificate file \"" << certFile << '"');
      return false;

    case PSSLFileTypePEM :
      m_certificate = PEM_read_bio_X509(in, NULL, NULL, NULL);
      if (m_certificate != NULL)
        break;

      PTRACE(2, "Invalid PEM certificate file \"" << certFile << '"');
      return false;

    default :
      m_certificate = PEM_read_bio_X509(in, NULL, NULL, NULL);
      if (m_certificate != NULL)
        break;

      m_certificate = d2i_X509_bio(in, NULL);
      if (m_certificate != NULL)
        break;

      PTRACE(2, "Invalid certificate file \"" << certFile << '"');
      return false;
  }

  PTRACE(4, "Loaded certificate file \"" << certFile << '"');
  return true;
}


PBoolean PSSLCertificate::Save(const PFilePath & certFile, PBoolean append, PSSLFileTypes fileType)
{
  if (m_certificate == NULL)
    return false;

  PSSL_BIO out;
  if (!(append ? out.OpenAppend(certFile) : out.OpenWrite(certFile))) {
    PTRACE(2, "Could not " << (append ? "append to" : "create") << " certificate file \"" << certFile << '"');
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

  PTRACE(2, "Error writing certificate file \"" << certFile << '"');
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

static char const * const FingerprintHashNames[PSSLCertificateFingerprint::NumHashType] = { "md5", "sha-1", "sha-256", "sha-512" };

PSSLCertificateFingerprint::PSSLCertificateFingerprint()
  : m_hashAlogorithm(NumHashType)
{
}


PSSLCertificateFingerprint::PSSLCertificateFingerprint(const PString & str)
{
  FromString(str);
}


PSSLCertificateFingerprint::PSSLCertificateFingerprint(HashType type, const PSSLCertificate& cert)
  : m_hashAlogorithm(type)
{
  if (m_hashAlogorithm == NumHashType || !cert.IsValid())
    return;

  const EVP_MD* evp = 0;
  switch(m_hashAlogorithm)
  {
    case HashMd5:
      evp = EVP_md5();
      break;
    case HashSha1:
      evp = EVP_sha1();
      break;
    case HashSha256:
      evp = EVP_sha256();
      break;
    case HashSha512:
      evp = EVP_sha512();
      break;
    default:
      break;
  }

  unsigned len = 0;
  unsigned char fp[256];
  if(X509_digest(cert, evp, fp, &len) != 1 || len <= 0) {
    PTRACE(2, "X509_digest() failed: " << PSSLError());
    return;
  }

  for(unsigned i = 0; i < len; ++i)
    m_fingerprint.sprintf(i > 0 ? ":%.2X" : "%.2X", fp[i]);
}


PObject::Comparison PSSLCertificateFingerprint::Compare(const PObject & obj) const
{
  const PSSLCertificateFingerprint & other = dynamic_cast<const PSSLCertificateFingerprint &>(obj);
  if (m_hashAlogorithm < other.m_hashAlogorithm)
    return LessThan;
  if (m_hashAlogorithm > other.m_hashAlogorithm)
    return GreaterThan;
  return m_fingerprint.Compare(other.m_fingerprint);
}


bool PSSLCertificateFingerprint::IsValid() const
{
  return m_hashAlogorithm < NumHashType && !m_fingerprint.IsEmpty();
}


bool PSSLCertificateFingerprint::MatchForCertificate(const PSSLCertificate& cert) const
{
  return (*this == PSSLCertificateFingerprint(m_hashAlogorithm, cert));
}


PString PSSLCertificateFingerprint::AsString() const
{
  if (IsValid())
    return PSTRSTRM(FingerprintHashNames[m_hashAlogorithm] << ' ' << m_fingerprint);

  return PString::Empty();
}


bool PSSLCertificateFingerprint::FromString(const PString & str)
{
  PCaselessString hashType, fp;

  m_fingerprint.MakeEmpty();
  m_hashAlogorithm = NumHashType;

  if (!str.Split(' ', hashType, fp))
    return false;

  for (m_hashAlogorithm = HashMd5; m_hashAlogorithm < EndHashType; ++m_hashAlogorithm) {
    if (hashType == FingerprintHashNames[m_hashAlogorithm]) {
      m_fingerprint = fp.ToUpper();
      return true;
    }
  }

  return false;
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

PSSLCipherContext::PSSLCipherContext(bool encrypt)
  : m_padMode(PadPKCS)
  , m_context(new EVP_CIPHER_CTX)
{
  EVP_CIPHER_CTX_init(m_context);
  m_context->encrypt = encrypt;
  EVP_CIPHER_CTX_set_padding(m_context, 1);
}


PSSLCipherContext::~PSSLCipherContext()
{
  EVP_CIPHER_CTX_cleanup(m_context);
  delete m_context;
}


PString PSSLCipherContext::GetAlgorithm() const
{
  const EVP_CIPHER * cipher = EVP_CIPHER_CTX_cipher(m_context);
  if (cipher == NULL)
    return PString::Empty();
  return EVP_CIPHER_name(cipher);
}


bool PSSLCipherContext::SetAlgorithm(const PString & name)
{
  const EVP_CIPHER * cipher = EVP_get_cipherbyname(name);
  if (cipher == NULL) {
    cipher = EVP_get_cipherbyobj(OBJ_txt2obj(name, 1));
    if (cipher == NULL) {
      if (name == "1.3.14.3.2.6") // For some reason, not in the OpenSSL database.
        cipher = EVP_des_ecb();
      else {
        PTRACE(2, "Unknown cipher algorithm \"" << name << '"');
        return false;
      }
    }
  }

  if (EVP_CipherInit_ex(m_context, cipher, NULL, NULL, NULL, m_context->encrypt)) {
    PTRACE(4, "Set cipher algorithm \"" << GetAlgorithm() << "\" from \"" << name << '"');
    return true;
  }

  PTRACE(2, "Could not set cipher algorithm: " << PSSLError());
  return false;
}


bool PSSLCipherContext::SetKey(const BYTE * keyPtr, PINDEX keyLen)
{
  PTRACE(4, "Setting key: " << hex << fixed << setfill('0') << PBYTEArray(keyPtr, keyLen, false));

  if (keyLen < EVP_CIPHER_CTX_key_length(m_context)) {
    PTRACE(2, "Incorrect key length for encryption");
    return false;
  }

  if (EVP_CipherInit_ex(m_context, NULL, NULL, keyPtr, NULL, m_context->encrypt))
    return true;

  PTRACE(2, "Could not set key: " << PSSLError());
  return false;
}


bool PSSLCipherContext::SetPadding(PadMode pad)
{
  m_padMode = pad;
  return EVP_CIPHER_CTX_set_padding(m_context, pad != NoPadding) != 0;
}


bool PSSLCipherContext::SetIV(const BYTE * ivPtr, PINDEX ivLen)
{
  if (ivLen < EVP_CIPHER_CTX_iv_length(m_context)) {
    PTRACE(2, "Incorrect inital vector length for encryption");
    return false;
  }

  if (EVP_CipherInit_ex(m_context, NULL, NULL, NULL, ivPtr, m_context->encrypt))
    return true;

  PTRACE(2, "Could not set initial vector: " << PSSLError());
  return false;
}


bool PSSLCipherContext::Process(const PBYTEArray & in, PBYTEArray & out)
{
  if (!out.SetMinSize(GetBlockedDataSize(in.GetSize())))
    return false;

  PINDEX outLen = out.GetSize();
  if (!Process(in, in.GetSize(), out.GetPointer(), outLen))
    return false;

  return out.SetSize(outLen);
}


// ciphertext stealing code based on a OpenSSL patch by An-Cheng Huang
// Note: This ciphertext stealing implementation doesn't seem to always produce
// compatible results, avoid when encrypting:

static int EVP_CipherUpdate_cts(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl,
                      const unsigned char *in, int inl)
{
    int bl = ctx->cipher->block_size;
    int leftover = 0;
    OPENSSL_assert(bl <= (int)sizeof(ctx->buf));
    *outl = 0;

    if ((ctx->buf_len + inl) <= bl) {
        /* new plaintext is no more than 1 block */
        /* copy the in data into the buffer and return */
        memcpy(&(ctx->buf[ctx->buf_len]), in, inl);
        ctx->buf_len += inl;
        *outl = 0;
        return 1;
    }

    /* more than 1 block of new plaintext available */
    /* encrypt the previous plaintext, if any */
    if (ctx->final_used) {
        if (!(ctx->cipher->do_cipher(ctx, out, ctx->final, bl))) {
          return 0;
        }
        out += bl;
        *outl += bl;
        ctx->final_used = 0;
    }

    /* we already know ctx->buf_len + inl must be > bl */
    memcpy(&(ctx->buf[ctx->buf_len]), in, (bl - ctx->buf_len));
    in += (bl - ctx->buf_len);
    inl -= (bl - ctx->buf_len);
    ctx->buf_len = bl;

    if (inl <= bl) {
        memcpy(ctx->final, ctx->buf, bl);
        ctx->final_used = 1;
        memcpy(ctx->buf, in, inl);
        ctx->buf_len = inl;
        return 1;
    } else {
        if (!(ctx->cipher->do_cipher(ctx, out, ctx->buf, bl))) {
            return 0;
        }
        out += bl;
        *outl += bl;
        ctx->buf_len = 0;

        leftover = inl & ctx->block_mask;
        if (leftover) {
            inl -= (bl + leftover);
            memcpy(ctx->buf, &(in[(inl + bl)]), leftover);
            ctx->buf_len = leftover;
        } else {
            inl -= (2 * bl);
             memcpy(ctx->buf, &(in[(inl + bl)]), bl);
             ctx->buf_len = bl;
        }
        memcpy(ctx->final, &(in[inl]), bl);
        ctx->final_used = 1;
        if (!(ctx->cipher->do_cipher(ctx, out, in, inl))) {
            return 0;
        }
        out += inl;
        *outl += inl;
    }

    return 1;
}

static int EVP_EncryptFinal_cts(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl)
{
    unsigned char tmp[EVP_MAX_BLOCK_LENGTH];
    int bl = ctx->cipher->block_size;
    int leftover = 0;
    *outl = 0;

    if (!ctx->final_used) {
        PTRACE(1, "CTS Error: expecting previous ciphertext");
        return 0;
    }
    if (ctx->buf_len == 0) {
        PTRACE(1, "CTS Error: expecting previous plaintext");
        return 0;
    }

    /* handle leftover bytes */
    leftover = ctx->buf_len;

    switch (EVP_CIPHER_CTX_mode(ctx)) {
        case EVP_CIPH_ECB_MODE: {
            /* encrypt => C_{n} plus C' */
            if (!(ctx->cipher->do_cipher(ctx, tmp, ctx->final, bl))) {
                 return 0;
            }

            /* P_n plus C' */
            memcpy(&(ctx->buf[leftover]), &(tmp[leftover]), (bl - leftover));
            /* encrypt => C_{n-1} */
            if (!(ctx->cipher->do_cipher(ctx, out, ctx->buf, bl))) {
                return 0;
            }

            memcpy((out + bl), tmp, leftover);
            *outl += (bl + leftover);
            return 1;
        }
        case EVP_CIPH_CBC_MODE: {
            /* encrypt => C_{n} plus C' */
            if (!(ctx->cipher->do_cipher(ctx, tmp, ctx->final, bl))) {
                return 0;
            }

            /* P_n plus 0s */
            memset(&(ctx->buf[leftover]), 0, (bl - leftover));

            /* note that in cbc encryption, plaintext will be xor'ed with the previous
             * ciphertext, which is what we want.
             */
            /* encrypt => C_{n-1} */
            if (!(ctx->cipher->do_cipher(ctx, out, ctx->buf, bl))) {
                return 0;
            }

            memcpy((out + bl), tmp, leftover);
            *outl += (bl + leftover);
            return 1;
        }
        default:
            PTRACE(1, "CTS Error: unsupported mode");
            return 0;
    }
}

static int EVP_DecryptFinal_cts(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl)
{
    unsigned char tmp[EVP_MAX_BLOCK_LENGTH];
    int bl = ctx->cipher->block_size;
    int leftover = 0;
    *outl = 0;

    if (!ctx->final_used) {
        PTRACE(1, "CTS Error: expecting previous ciphertext");
        return 0;
    }
    if (ctx->buf_len == 0) {
        PTRACE(1, "CTS Error: expecting previous ciphertext");
        return 0;
    }

    /* handle leftover bytes */
    leftover = ctx->buf_len;

    switch (EVP_CIPHER_CTX_mode(ctx)) {
        case EVP_CIPH_ECB_MODE: {
            /* decrypt => P_n plus C' */
            if (!(ctx->cipher->do_cipher(ctx, tmp, ctx->final, bl))) {
                return 0;
            }

            /* C_n plus C' */
            memcpy(&(ctx->buf[leftover]), &(tmp[leftover]), (bl - leftover));
            /* decrypt => P_{n-1} */
            if (!(ctx->cipher->do_cipher(ctx, out, ctx->buf, bl))) {
                return 0;
            }

            memcpy((out + bl), tmp, leftover);
            *outl += (bl + leftover);
            return 1;
        }
        case EVP_CIPH_CBC_MODE: {
            int i = 0;
            unsigned char C_n_minus_2[EVP_MAX_BLOCK_LENGTH];

            memcpy(C_n_minus_2, ctx->iv, bl);

            /* C_n plus 0s in ctx->buf */
            memset(&(ctx->buf[leftover]), 0, (bl - leftover));

            /* ctx->final is C_{n-1} */
            /* decrypt => (P_n plus C')'' */
            if (!(ctx->cipher->do_cipher(ctx, tmp, ctx->final, bl))) {
                return 0;
            }
            /* XOR'ed with C_{n-2} => (P_n plus C')' */
            for (i = 0; i < bl; i++) {
                tmp[i] = tmp[i] ^ C_n_minus_2[i];
            }
            /* XOR'ed with (C_n plus 0s) => P_n plus C' */
            for (i = 0; i < bl; i++) {
                tmp[i] = tmp[i] ^ ctx->buf[i];
            }

            /* C_n plus C' in ctx->buf */
            memcpy(&(ctx->buf[leftover]), &(tmp[leftover]), (bl - leftover));
            /* decrypt => P_{n-1}'' */
            if (!(ctx->cipher->do_cipher(ctx, out, ctx->buf, bl))) {
                return 0;
            }
            /* XOR'ed with C_{n-1} => P_{n-1}' */
            for (i = 0; i < bl; i++) {
                out[i] = out[i] ^ ctx->final[i];
            }
            /* XOR'ed with C_{n-2} => P_{n-1} */
            for (i = 0; i < bl; i++) {
                out[i] = out[i] ^ C_n_minus_2[i];
            }

            memcpy((out + bl), tmp, leftover);
            *outl += (bl + leftover);
            return 1;
        }
        default:
            PTRACE(1, "CTS Error: unsupported mode");
            return 0;
    }
}


bool PSSLCipherContext::Process(const BYTE * inPtr, PINDEX inLen, BYTE * outPtr, PINDEX & outLen, bool partial)
{
  if (outLen < (m_context->encrypt ? GetBlockedDataSize(inLen) : inLen))
    return false;

  int len = -1;
  if (m_padMode == PadCipherStealing) {
    if (!EVP_CipherUpdate_cts(m_context, outPtr, &len, inPtr, inLen))
      return false;
  }
  else {
    if (!EVP_CipherUpdate(m_context, outPtr, &len, inPtr, inLen)) {
      PTRACE(2, "Could not update data: " << PSSLError());
      return false;
    }
  }

  outLen = len;

  if (partial)
    return true;

  switch (m_padMode) {
    case NoPadding :
      if (inLen != outLen) {
        PTRACE(2, "No padding selected and data required padding");
        return false;
      }
      len = 0;
      break;

    case PadPKCS :
      if (!EVP_CipherFinal(m_context, outPtr+len, &len)) {
        PTRACE(2, "Could not finalise data: " << PSSLError());
        return false;
      }
      break;

    // Polycom endpoints (eg. m100 and PVX) don't fill the pading properly, so we have to disable some checks
    case PadLoosePKCS :
      if (m_context->buf_len != 0 || m_context->final_used == 0) {
        PTRACE(2, "Decrypt error: wrong final block length:"
                       " buf_len=" << m_context->buf_len <<
                    " final_used=" << m_context->final_used);
        return false;
      }
      else {
        int b = m_context->cipher->block_size;
        int n = m_context->final[b-1];
        if (n == 0 || n > b) {
          PTRACE(1, "Decrypt error: bad decrypt");
          return false;
        }
        len = b - n;
        for (n = 0; n < len; n++)
          outPtr[outLen+n] = m_context->final[n];
      }
      break;

    case PadCipherStealing :
      if (!(m_context->encrypt ? EVP_EncryptFinal_cts(m_context, outPtr+len, &len)
                               : EVP_DecryptFinal_cts(m_context, outPtr+len, &len)))
        return false;
      break;
  }

  outLen += len;
  return true;
}


bool PSSLCipherContext::IsEncrypt() const
{
  return m_context->encrypt != 0;
}


PINDEX PSSLCipherContext::GetKeyLength() const
{
  return EVP_CIPHER_CTX_key_length(m_context);
}


PINDEX PSSLCipherContext::GetIVLength() const
{
  return EVP_CIPHER_CTX_iv_length(m_context);
}


PINDEX PSSLCipherContext::GetBlockSize() const
{
  return EVP_CIPHER_CTX_block_size(m_context);
}


PINDEX PSSLCipherContext::GetBlockedDataSize(PINDEX size) const
{
  PINDEX blockSize = EVP_CIPHER_CTX_block_size(m_context);
  return ((size+blockSize-1)/blockSize)*blockSize;
}


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
  : m_dh(NULL)
{
}


PSSLDiffieHellman::PSSLDiffieHellman(const PFilePath & dhFile, PSSLFileTypes fileType)
  : m_dh(NULL)
{
  Load(dhFile, fileType);
}


PSSLDiffieHellman::PSSLDiffieHellman(PINDEX numBits, const BYTE * pData, const BYTE * gData, const BYTE * pubKey)
  : m_dh(DH_new())
{
  if (PAssertNULL(m_dh) == NULL)
    return;

  PINDEX numBytes = numBits/8;
  if (Construct(pData, numBytes, gData, numBytes, pubKey, numBytes))
    return;

  DH_free(m_dh);
  m_dh = NULL;
}


PSSLDiffieHellman::PSSLDiffieHellman(const PBYTEArray & pData,
                                     const PBYTEArray & gData,
                                     const PBYTEArray & pubKey)
  : m_dh(DH_new())
{
  if (PAssertNULL(m_dh) == NULL)
    return;

  if (Construct(pData, pData.GetSize(), gData, gData.GetSize(), pubKey, pubKey.GetSize()))
    return;

  DH_free(m_dh);
  m_dh = NULL;
}


bool PSSLDiffieHellman::Construct(const BYTE * pData, PINDEX pSize,
                                  const BYTE * gData, PINDEX gSize,
                                  const BYTE * kData, PINDEX kSize)
{
  if (!PAssert(pSize >= 64 && pSize%4 == 0 && gSize <= pSize && kSize <= pSize, PInvalidParameter))
    return false;

  if ((m_dh->p = BN_bin2bn(pData, pSize, NULL)) == NULL)
    return false;

  if ((m_dh->g = BN_bin2bn(gData, gSize, NULL)) == NULL)
    return false;

  int err = -1;
  if (DH_check(m_dh, &err)) {
    switch (err) {
      case DH_CHECK_P_NOT_PRIME:
        PTRACE(2, "The p value is not prime");
        break;
      case DH_CHECK_P_NOT_SAFE_PRIME:
        PTRACE(2, "The p value is not a safe prime");
        break;
      case DH_UNABLE_TO_CHECK_GENERATOR:
        PTRACE(2, "Unable to check the generator value");
        break;
      case DH_NOT_SUITABLE_GENERATOR:
        PTRACE(4, "The g value is not a suitable generator");
        break;
      default :
        PTRACE(1, "Diffie-Hellman check failed: code=" << err);
        return false;
    }
  }

  if (kData == NULL) {
    if (DH_generate_key(m_dh))
      return true;

    PSSLAssert("DH key generate failed: ");
    return false;
  }

  if (!PAssert(kSize <= pSize && kSize%4 == 0, PInvalidParameter))
    return false;

  if ((m_dh->pub_key = BN_bin2bn(kData, kSize, NULL)) != NULL)
    return true;

  PSSLAssert("DH public key invalid: ");
  return false;
}


PSSLDiffieHellman::PSSLDiffieHellman(const PSSLDiffieHellman & diffie)
{
  m_dh = diffie.m_dh;
}


PSSLDiffieHellman & PSSLDiffieHellman::operator=(const PSSLDiffieHellman & diffie)
{
  if (m_dh != NULL)
    DH_free(m_dh);
  m_dh = diffie.m_dh;
  return *this;
}


PSSLDiffieHellman::~PSSLDiffieHellman()
{
  if (m_dh != NULL)
    DH_free(m_dh);
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
  if (m_dh != NULL) {
    DH_free(m_dh);
    m_dh = NULL;
  }

  PSSL_BIO in;
  if (!in.OpenRead(dhFile)) {
    PTRACE(2, "Could not open DH file \"" << dhFile << '"');
    return false;
  }

  switch (fileType) {
    case PSSLFileTypeASN1 :
      m_dh = d2i_DHparams_bio(in, NULL);
      if (m_dh != NULL)
        break;

      PTRACE(2, "Invalid ASN.1 DH file \"" << dhFile << '"');
      return false;

    case PSSLFileTypePEM :
      m_dh = PEM_read_bio_DHparams(in, NULL, NULL, NULL);
      if (m_dh != NULL)
        break;

      PTRACE(2, "Invalid PEM DH file \"" << dhFile << '"');
      return false;

    default :
      m_dh = PEM_read_bio_DHparams(in, NULL, NULL, NULL);
      if (m_dh != NULL)
        break;

      m_dh = d2i_DHparams_bio(in, NULL);
      if (m_dh != NULL)
        break;

      PTRACE(2, "Invalid DH file \"" << dhFile << '"');
      return false;
  }

  PTRACE(4, "Loaded DH file \"" << dhFile << '"');
  return false;
}


PINDEX PSSLDiffieHellman::GetNumBits() const
{
  return m_dh != NULL ? BN_num_bits(m_dh->p) : 0;
}


static PBYTEArray GetBigNum(BIGNUM * bn, PINDEX numBits)
{
  PBYTEArray bytes;

  if (bn == NULL)
    return bytes;

  PINDEX numBytes = numBits/8;
  if (BN_bn2bin(bn, bytes.GetPointer(numBytes) + numBytes - BN_num_bytes(bn)) > 0)
    return bytes;

  bytes.SetSize(0);
  return bytes;
}


PBYTEArray PSSLDiffieHellman::GetModulus() const
{
  return GetBigNum(m_dh != NULL ? m_dh->p : NULL, GetNumBits());
}


PBYTEArray PSSLDiffieHellman::GetGenerator() const
{
  return GetBigNum(m_dh != NULL ? m_dh->g : NULL, GetNumBits());
}


PBYTEArray PSSLDiffieHellman::GetHalfKey() const
{
  return GetBigNum(m_dh != NULL ? m_dh->pub_key : NULL, GetNumBits());
}


bool PSSLDiffieHellman::ComputeSessionKey(const PBYTEArray & otherHalf)
{
  if (m_dh == NULL)
    return false;

  BIGNUM * obn = BN_bin2bn(otherHalf, otherHalf.GetSize(), NULL);
  int result = DH_compute_key(m_sessionKey.GetPointer(DH_size(m_dh)), obn, m_dh);
  if (result > 0)
    m_sessionKey.SetSize(result);
  else {
    PTRACE(2, "Could not calculate session key: " << PSSLError());
    m_sessionKey.SetSize(0);
  }
  BN_free(obn);

  return !m_sessionKey.IsEmpty();
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


static void InfoCallback(const SSL * PTRACE_PARAM(ssl), int PTRACE_PARAM(location), int PTRACE_PARAM(ret))
{
#if PTRACING
  static const unsigned Level = 4;
  if (PTrace::GetLevel() >= Level) {
    ostream & trace = PTRACE_BEGIN(Level);

    if (location & SSL_CB_ALERT) {
      trace << "Alert "
            << ((location & SSL_CB_READ) ? "read" : "write")
            << ' ' << SSL_alert_type_string_long(ret)
            << ": " << SSL_alert_desc_string_long(ret);
    }
    else {
      if (location & SSL_ST_CONNECT)
        trace << "Connect";
      else if (location & SSL_ST_ACCEPT)
        trace << "Accept";
      else
        trace << "General";

      trace << ": ";

      if (location & SSL_CB_EXIT) {
        if (ret == 0)
          trace << "failed in ";
        else if (ret < 0)
          trace << "error in ";
      }

      trace << "state=" << SSL_state_string_long(ssl);
    }
    trace << PTrace::End;
  }
#endif // PTRACING
}


#if PTRACING
static void TraceVerifyCallback(int ok, X509_STORE_CTX * ctx)
{
  const unsigned Level = ok ? 5 : 2;
  if (PTrace::GetLevel() >= Level) {
    int err = X509_STORE_CTX_get_error(ctx);
    int depth = X509_STORE_CTX_get_error_depth(ctx);
    PSSLCertificate cert(X509_STORE_CTX_get_current_cert(ctx));
    PSSLCertificate::X509_Name issuer, subject;
    cert.GetIssuerName(issuer);
    cert.GetSubjectName(subject);

    PTRACE_BEGIN(Level)
      << "Verify callback: depth="
      << depth
      << ", err=" << err << " - " << X509_verify_cert_error_string(err)
      << "\n  Subject:\n" << subject.AsString(4)
      << "\n  Issuer:\n" << issuer.AsString(4)
      << PTrace::End;
  }
}
#else
  #define TraceVerifyCallback(ok, ctx)
#endif // PTRACING

static int VerifyCallback(int ok, X509_STORE_CTX * ctx)
{
  SSL * ssl = reinterpret_cast<SSL *>(X509_STORE_CTX_get_app_data(ctx));
  if (ssl != NULL) {
    PSSLChannel * channel = reinterpret_cast<PSSLChannel *>(SSL_get_app_data(ssl));
    if (channel != NULL)
      ok = channel->OnVerify(ok, X509_STORE_CTX_get_current_cert(ctx));
  }

  TraceVerifyCallback(ok, ctx);
  return ok;
}


///////////////////////////////////////////////////////////////////////////////

PSSLContext::PSSLContext(Method method, const void * sessionId, PINDEX idSize)
  : m_method(method)
{
  Construct(sessionId, idSize);
}


PSSLContext::PSSLContext(const void * sessionId, PINDEX idSize)
  : m_method(TLSv1_2)
{
  Construct(sessionId, idSize);
}

void PSSLContext::Construct(const void * sessionId, PINDEX idSize)
{
  // create the new SSL context
#if OPENSSL_VERSION_NUMBER > 0x0090819fL
  const
#endif
       SSL_METHOD * meth;

  switch (m_method) {
    case SSLv23:
      meth = SSLv23_method();
      break;
    case SSLv3:
      meth = SSLv3_method();
      break;
#if OPENSSL_VERSION_NUMBER > 0x10002000L
    case TLSv1:
      meth = TLSv1_method(); 
      break;
    case TLSv1_1 :
      meth = TLSv1_1_method(); 
      break;
    case TLSv1_2 :
      meth = TLSv1_2_method(); 
      break;
    case DTLSv1:
      meth = DTLSv1_method();
      break;
    case DTLSv1_2 :
      meth = DTLSv1_2_method(); 
      break;
    case DTLSv1_2_v1_0 :
      meth = DTLS_method(); 
      break;
#else
  #pragma message ("Using " OPENSSL_VERSION_TEXT " - TLS 1.1, 1.2 and DTLS 1.2 not available")
    case TLSv1:
    case TLSv1_1 :
    case TLSv1_2 :
      meth = TLSv1_method();
      break;
    case DTLSv1:
    case DTLSv1_2 :
    case DTLSv1_2_v1_0 :
      meth = DTLSv1_method();
      break;
#endif
    default :
      PAssertAlways("Unsupported TLS/DTLS version");
      m_context = NULL;
      return;
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

  PTRACE(4, "Constructed context: method=" << m_method << " ctx=" << m_context);
}


PSSLContext::~PSSLContext()
{
  PTRACE(4, "Destroyed context: method=" << m_method << " ctx=" << m_context);
  SSL_CTX_free(m_context);
}


bool PSSLContext::SetVerifyLocations(const PFilePath & caFile, const PDirectory & caDir)
{
  PString caPath = caDir.Left(caDir.GetLength()-1);
  if (SSL_CTX_load_verify_locations(m_context, caFile.IsEmpty() ? NULL : (const char *)caFile,
                                               caPath.IsEmpty() ? NULL : (const char *)caPath)) {
    PTRACE(4, "Set context " << m_context << " verify locations file=\"" << caFile << "\", dir=\"" << caDir << '"');
    return true;
  }

  PTRACE(2, "Could not set context " << m_context << " verify locations file=\"" << caFile << "\", dir=\"" << caDir << '"');
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
      PTRACE(2, "Could not find/parse certificate authority \"" << authority << '"');
      return false;
    }
    SetVerifyMode(VerifyPeerMandatory);
  }

  if (certificate.IsEmpty() && privateKey.IsEmpty())
    return true;

  PSSLCertificate cert;
  PSSLPrivateKey key;

  if (PFile::Exists(certificate) && !cert.Load(certificate)) {
    PTRACE(2, "Could not load certificate file \"" << certificate << '"');
    return false;
  }

  if (PFile::Exists(privateKey) && !key.Load(privateKey, PSSLFileTypeDEFAULT, m_passwordNotifier)) {
    PTRACE(2, "Could not load private key file \"" << privateKey << '"');
    return false;
  }

  // Can put the base64 directly into string, rather than file path
  if (!key.IsValid())
    key.Parse(privateKey);

  if (!cert.IsValid())
    cert.Parse(certificate);

  if (!cert.IsValid() || !key.IsValid()) {

    if (cert.IsValid() || key.IsValid()) {
      PTRACE(2, "Require both certificate and private key");
      return false;
    }

    if (!create) {
      PTRACE(2, "Require certificate and private key");
      return false;
    }

    PStringStream dn;
    dn << "/O=" << PProcess::Current().GetManufacturer()
       << "/CN=" << PIPSocket::GetHostName();

    if (!key.Create(2048)) {
      PTRACE(1, "Could not create private key");
      return false;
    }

    if (!cert.CreateRoot(dn, key)) {
      PTRACE(1, "Could not create certificate");
      return false;
    }

    if (!cert.Save(certificate))
      return false;

    PTRACE(2, "Created new certificate file \"" << certificate << '"');

    if (!key.Save(privateKey, true))
      return false;

    PTRACE(2, "Created new private key file \"" << privateKey << '"');
  }

  if (!UseCertificate(cert)) {
    PTRACE(1, "Could not use certificate " << cert);
    return false;
  }

  if (!UsePrivateKey(key)) {
    PTRACE(1, "Could not use private key " << key);
    return false;
  }

  return true;
}


void PSSLContext::SetPasswordNotifier(const PSSLPasswordNotifier & notifier)
{
  if (m_context == NULL)
    return;

  m_passwordNotifier = notifier;
  if (notifier.IsNULL())
    SSL_CTX_set_default_passwd_cb(m_context, NULL);
  else {
    SSL_CTX_set_default_passwd_cb(m_context, PasswordCallback);
    SSL_CTX_set_default_passwd_cb_userdata(m_context, &m_passwordNotifier);
  }
}


bool PSSLContext::SetExtension(const char * extension)
{
#if P_SSL_SRTP
  return SSL_CTX_set_tlsext_use_srtp(m_context, extension) == 0;
#else
  return false;
#endif
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
  if (m_ssl == NULL) {
    PSSLAssert("Error creating channel: ");
    return;
  }

  SSL_set_app_data(m_ssl, this);

  static BIO_METHOD BioMethods =
  {
    BIO_TYPE_SOCKET,
    "PTLib-PSSLChannel",
    BioWrite,
    BioRead,
    NULL,
    NULL,
    BioControl,
    NULL,
    BioClose,
    NULL
  };

  m_bio = BIO_new(&BioMethods);
  if (m_bio == NULL) {
    PSSLAssert("Error creating BIO: ");
    return;
  }

  m_bio->ptr = this;
  m_bio->init = 1;

  SSL_set_bio(m_ssl, m_bio, m_bio);

  PTRACE(4, "Constructed channel: ssl=" << m_ssl << " method=" << m_context->GetMethod() << " context=" << &*m_context);
}


PSSLChannel::~PSSLChannel()
{
  PTRACE(4, "Destroyed channel: ssl=" << m_ssl << " method=" << m_context->GetMethod() << " context=" << &*m_context);

  // free the SSL connection
  if (m_ssl != NULL)
    SSL_free(m_ssl);

  // The above free of SSL also frees the m_bio, no need to it here

  if (m_autoDeleteContext)
    delete m_context;
}


PBoolean PSSLChannel::Read(void * buf, PINDEX len)
{
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


int PSSLChannel::BioRead(bio_st * bio, char * buf, int len)
{
  return bio != NULL && bio->ptr != NULL ? reinterpret_cast<PSSLChannel *>(bio->ptr)->BioRead(buf, len) : -1;
}


int PSSLChannel::BioRead(char * buf, int len)
{
  BIO_clear_retry_flags(m_bio);

  // Skip over the polymorphic read, want to do real one
  if (PIndirectChannel::Read(buf, len))
    return GetLastReadCount();

  if (GetErrorCode(PChannel::LastReadError) == PChannel::Interrupted)
    BIO_set_retry_read(m_bio);

  return -1;
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


int PSSLChannel::BioWrite(bio_st * bio, const char * buf, int len)
{
  return bio != NULL && bio->ptr != NULL ? reinterpret_cast<PSSLChannel *>(bio->ptr)->BioWrite(buf, len) : -1;
}


int PSSLChannel::BioWrite(const char * buf, int len)
{
  BIO_clear_retry_flags(m_bio);

  // Skip over the polymorphic write, want to do real one
  if (PIndirectChannel::Write(buf, len))
    return GetLastWriteCount();

  if (GetErrorCode(PChannel::LastWriteError) == PChannel::Interrupted)
    BIO_set_retry_write(m_bio);

  return -1;
}


PBoolean PSSLChannel::Close()
{
  PBoolean ok = SSL_shutdown(m_ssl);
  return PIndirectChannel::Close() && ok;
}


int PSSLChannel::BioClose(bio_st * bio)
{
  return bio != NULL && bio->ptr != NULL ? reinterpret_cast<PSSLChannel *>(bio->ptr)->BioClose() : 0;
}


int PSSLChannel::BioClose()
{
  if (m_bio->shutdown) {
    if (m_bio->init) {
      Shutdown(PSocket::ShutdownReadAndWrite);
      Close();
    }
    m_bio->init  = 0;
    m_bio->flags = 0;
  }

  return 1;
}


PBoolean PSSLChannel::ConvertOSError(P_INT_PTR libcReturnValue, ErrorGroup group)
{
  Errors lastError = NoError;
  DWORD osError = 0;
  if (SSL_get_error(m_ssl, (int)libcReturnValue) != SSL_ERROR_NONE && (osError = ERR_peek_error()) != 0) {
    osError |= 0x80000000;
    lastError = AccessDenied;
  }

  return SetErrorValues(lastError, osError, group);
}


PString PSSLChannel::GetErrorText(ErrorGroup group) const
{
  if ((lastErrorNumber[group]&0x80000000) == 0)
    return PIndirectChannel::GetErrorText(group);

  return PSSLError(lastErrorNumber[group]&0x7fffffff);
}


PBoolean PSSLChannel::Accept()
{
  return IsOpen() && InternalAccept();
}


PBoolean PSSLChannel::Accept(PChannel & channel)
{
  return Open(channel) && InternalAccept();
}


PBoolean PSSLChannel::Accept(PChannel * channel, PBoolean autoDelete)
{
  return Open(channel, autoDelete) && InternalAccept();
}


bool PSSLChannel::InternalAccept()
{
  return ConvertOSError(SSL_accept(m_ssl));
}


PBoolean PSSLChannel::Connect()
{
  return IsOpen() && InternalConnect();
}


PBoolean PSSLChannel::Connect(PChannel & channel)
{
  return Open(channel) && InternalConnect();
}


PBoolean PSSLChannel::Connect(PChannel * channel, PBoolean autoDelete)
{
  return Open(channel, autoDelete) && InternalConnect();
}


bool PSSLChannel::InternalConnect()
{
  return ConvertOSError(SSL_connect(m_ssl));
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


void PSSLChannel::SetVerifyMode(VerifyMode mode, const VerifyNotifier & notifier)
{
  m_verifyNotifier = notifier;

  if (m_ssl != NULL)
    SSL_set_verify(m_ssl, VerifyModeBits[mode], VerifyCallback);
}


bool PSSLChannel::OnVerify(bool ok, const PSSLCertificate & peerCertificate)
{
  if (m_verifyNotifier.IsNULL())
    return ok;

  VerifyInfo info(ok, peerCertificate);
  m_verifyNotifier(*this, info);
  return info.m_ok;
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


long PSSLChannel::BioControl(bio_st * bio, int cmd, long num, void * ptr)
{
  return bio != NULL && bio->ptr != NULL ? reinterpret_cast<PSSLChannel *>(bio->ptr)->BioControl(cmd, num, ptr) : 0;
}


long PSSLChannel::BioControl(int cmd, long num, void * /*ptr*/)
{
  switch (cmd) {
    case BIO_CTRL_SET_CLOSE:
      m_bio->shutdown = (int)num;
      return 1;

    case BIO_CTRL_GET_CLOSE:
      return m_bio->shutdown;

    case BIO_CTRL_FLUSH:
      return 1;
  }

  // Other BIO commands, return 0
  return 0;
}


///////////////////////////////////////////////////////////////////////////////

PSSLChannelDTLS::PSSLChannelDTLS(PSSLContext * context, bool autoDeleteContext)
  : PSSLChannel(context, autoDeleteContext)
{
}


PSSLChannelDTLS::PSSLChannelDTLS(PSSLContext & context)
  : PSSLChannel(context)
{
}


PSSLChannelDTLS::~PSSLChannelDTLS()
{
}


bool PSSLChannelDTLS::ExecuteHandshake()
{
  PTRACE(5, "DTLS executing handshake.");

  SSL_set_mode(m_ssl, SSL_MODE_AUTO_RETRY);
  SSL_set_read_ahead(m_ssl, 1);
  SSL_CTX_set_read_ahead(*m_context, 1);

  int errorCode = SSL_do_handshake(m_ssl);
  if (errorCode == 1) {
    PTRACE(3, "DTLS handshake successful.");
    return true;
  }

  PTRACE_IF(2, IsOpen(), "DTLS handshake failed (" << errorCode <<") - " << PSSLError(SSL_get_error(m_ssl, errorCode)));
  return false;
}


bool PSSLChannelDTLS::IsServer() const
{
  return m_ssl->server;
}


PCaselessString PSSLChannelDTLS::GetSelectedProfile() const
{
#if P_SSL_SRTP
  SRTP_PROTECTION_PROFILE *p = SSL_get_selected_srtp_profile(m_ssl);
  if (p != NULL)
    return p->name;

  PTRACE(2, "SSL_get_selected_srtp_profile returned NULL: " << PSSLError());
#endif
  return PString::Empty();
}


PBYTEArray PSSLChannelDTLS::GetKeyMaterial(PINDEX materialSize, const char * name) const
{
#if P_SSL_SRTP
  if (PAssert(materialSize > 0 && name != NULL && *name != '\0', PInvalidParameter)) {
    PBYTEArray result;
    if (SSL_export_keying_material(m_ssl,
                                   result.GetPointer(materialSize), materialSize,
                                   name, strlen(name),
                                   NULL, 0, 0) == 1)
      return result;

    PTRACE(2, "SSL_export_keying_material failed: " << PSSLError());
  }
#endif

  return PBYTEArray();
}


bool PSSLChannelDTLS::InternalAccept()
{
  SSL_set_accept_state(*this);
  return true;
}


bool PSSLChannelDTLS::InternalConnect()
{
  SSL_set_connect_state(*this);
  return true;
}
#endif // P_SSL
