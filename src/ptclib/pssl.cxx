/*
 * SSL implementation for PTLib using the SSLeay package
 * Copyright (C) 1996 by Equivalence
 *
 * Portions bsed upon the file crypto/buffer/bss_sock.c 
 * Original copyright notice appears below
 *
 * $Id: pssl.cxx,v 1.1 1996/11/15 07:38:34 craigs Exp $
 * $Log: pssl.cxx,v $
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
#include "pssl.h"
#include <buffer.h>

SSL_CTX * PSSLChannel::sslContext = NULL;
int       PSSLChannel::sslContextRefCount = 0;

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

PSSLChannel::PSSLChannel()
{
  int s;

  if (sslContext == NULL) {
    sslContext         = SSL_CTX_new();
    sslContextRefCount = 1;

#if 0

    s = SSL_load_verify_locations(sslContext, NULL, ".:../../ssl/certs");
    PError << "SSL_load_verify_locations returned " << s << endl;

    s = SSL_set_default_verify_paths(sslContext);
    PError << "SSL_set_default_verify_paths " << s << endl;

    int verify = SSL_VERIFY_NONE;
    SSL_CTX_set_default_verify(sslContext, verify, (int (*)())verify_callback);

#endif

    // specify certificate 
    s = SSL_CTX_use_certificate_file(sslContext, "server.pem", SSL_FILETYPE_PEM);
    PError << "SSL_CTX_use_certificate_file returned " << s << endl;

    // specify private key 
    s = SSL_CTX_use_RSAPrivateKey_file(sslContext, "server.pem", SSL_FILETYPE_PEM);
    PError << "SSL_CTX_use_RSAPrivateKey_file returned " << s << endl;
  }

  // create an SSL context
  ssl = SSL_new(sslContext);
  SSL_clear(ssl);
}

int PSSLChannel::Accept(PChannel & channel)
{
  if (!Open(channel))
    return FALSE;

  // set the "fd" to use with this SSL context
  Set_SSL_Fd();

  // connect the SSL layer
  return SSL_accept(ssl) == 1;
}

PSSLChannel::~PSSLChannel()
{
  // free the SSL context
  SSL_free(ssl);
  sslContextRefCount--;
}

BOOL PSSLChannel::Read(void * buf, PINDEX len)
{
  int r = SSL_read(ssl, (char *)buf, len);
  lastReadCount = 0;
  if (!ConvertOSError(r))
    return FALSE;
  lastReadCount = r;
  return TRUE;
}

BOOL PSSLChannel::Write(const void * buf, PINDEX len)
{
  lastWriteCount = 0;
  while (len > 0) {
    int sendResult = SSL_write(ssl, ((const char *)buf)+lastWriteCount, len);
    if (!ConvertOSError(sendResult))
      return FALSE;
    lastWriteCount += sendResult;
    len -= sendResult;
  }
  return TRUE;
}

BOOL PSSLChannel::RawRead(void * buf, PINDEX len)
{
  return readChannel->Read(buf, len);
}

PINDEX PSSLChannel::RawGetLastReadCount() const
{
  return readChannel->GetLastReadCount();
}

BOOL PSSLChannel::RawWrite(const void * buf, PINDEX len)
{
  return writeChannel->Write(buf, len);
}

PINDEX PSSLChannel::RawGetLastWriteCount() const
{
  return writeChannel->GetLastWriteCount();
}

//////////////////////////////////////////////////////////////////////////

#define	PSSLSOCKET(bio)			((PSSLChannel *)(bio->num))

extern "C" {

#include <stdio.h>
#include <errno.h>
#define USE_SOCKETS
#include "cryptlib.h"
#include "buffer.h"

static int  Psock_write(BIO *h,char *buf,int num);
static int  Psock_read(BIO *h,char *buf,int size);
static int  Psock_puts(BIO *h,char *str);
static long Psock_ctrl(BIO *h,int cmd,long arg1,char *arg2);
static int  Psock_new(BIO *h);
static int  Psock_free(BIO *data);
static int  Psock_should_retry(int s);

typedef int (*ifptr)();
typedef long (*lfptr)();

static BIO_METHOD methods_Psock =
{
  BIO_TYPE_MEM,"ptlib socket",
  (ifptr)Psock_write,
  (ifptr)Psock_read,
  (ifptr)Psock_puts,
  NULL, /* sock_gets, */
  (lfptr)Psock_ctrl,
  (ifptr)Psock_new,
  (ifptr)Psock_free
};

};

int PSSLChannel::Set_SSL_Fd()
{
  int ret=0;
  BIO * bio = NULL;

  bio = BIO_new(&methods_Psock);
  if (bio == NULL) {
    SSLerr(SSL_F_SSL_SET_FD,ERR_R_BUF_LIB);
    goto err;
  }

  BIO_set_fd(bio,this,BIO_NOCLOSE);
  if (ssl->rbio != NULL)
    BIO_free((bio_st *)ssl->rbio);

  if ((ssl->wbio != NULL) && (ssl->wbio != ssl->rbio))
    BIO_free((bio_st *)ssl->wbio);

  ssl->rbio = (char *)bio;
  ssl->wbio = (char *)bio;
  ret = 1;

err:
  return(ret);
}

static int Psock_new(BIO * bio)
{
  bio->init     = 0;
  bio->ptr      = NULL;
  bio->flags    = 0;
  bio->shutdown = 0;

  // these are really (PSSLChannel *), not ints
  bio->num      = 0;

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
	
static int Psock_read(BIO * bio, char * out, int outl)
{
  int ret = 0;

  if (out != NULL) {
    BOOL b = PSSLSOCKET(bio)->RawRead(out, outl);
    bio->flags &= ~(BIO_FLAGS_RW|BIO_FLAGS_SHOULD_RETRY);
    if (b)
      ret = PSSLSOCKET(bio)->RawGetLastReadCount();
    else {
      bio->flags |= BIO_FLAGS_READ;
      bio->flags |= (Psock_should_retry(ret) ? BIO_FLAGS_SHOULD_RETRY:0);
    }
  }
  return(ret);
}

static int Psock_write(BIO * bio, char * in, int inl)
{
  int ret = 0;

  if (in != NULL) {
    BOOL b = PSSLSOCKET(bio)->RawWrite(in, inl);
    bio->flags &= ~(BIO_FLAGS_RW|BIO_FLAGS_SHOULD_RETRY);
    if (b)
      ret = PSSLSOCKET(bio)->RawGetLastWriteCount();
    else {
      bio->flags |= BIO_FLAGS_WRITE;
      bio->flags |= (Psock_should_retry(ret)?BIO_FLAGS_SHOULD_RETRY:0);
    }
  }
  return(ret);
}

static long Psock_ctrl(BIO * bio, int cmd, long num, char * ptr)
{
  long ret = 1;
  int *ip;

  switch (cmd) {
    case BIO_CTRL_RESET:
      ret = 0;
      break;

    case BIO_CTRL_INFO:
      ret = 0;
      break;

    case BIO_CTRL_SET:
      Psock_free(bio);
      bio->num      = (int)ptr;
      bio->shutdown = (int)num;
      bio->init     = 1;
      break;

    case BIO_CTRL_GET:
      if (bio->init) {
        ip = (int *)ptr;
	if (ip == NULL)
	  ret = 0;
	else
	  *ip = bio->num;
      }
      break;

    case BIO_CTRL_GET_CLOSE:
      ret = bio->shutdown;
      break;

    case BIO_CTRL_SET_CLOSE:
      bio->shutdown=(int)num;
      break;

    case BIO_CTRL_PENDING:
      ret = 0;
      break;

    case BIO_CTRL_FLUSH:
      break;

    case BIO_CTRL_SHOULD_RETRY:
      ret = (long)BIO_should_retry(bio);
      break;

    case BIO_CTRL_RETRY_TYPE:
      ret = (long)BIO_retry_type(bio);
      break;

    default:
      ret = 0;
      break;
  }
  return ret;
}

static int Psock_puts(BIO * bio,char * str)
{
  int n,ret;

  n   = strlen(str);
  ret = Psock_write(bio,str,n);

  return ret;
}

static int Psock_should_retry(int i)
{
  if (i == PChannel::Interrupted)
    return 1;
  if (i == PChannel::Timeout)
    return 1;

  return 0;
}
