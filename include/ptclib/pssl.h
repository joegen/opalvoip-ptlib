/*
 * $Id: pssl.h,v 1.1 1997/01/12 13:23:35 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1996 Equivalence
 *
 * $Log: pssl.h,v $
 * Revision 1.1  1997/01/12 13:23:35  robertj
 * Initial revision
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
};

PDECLARE_CLASS(PSSLChannel, PIndirectChannel)
  public:
    PSSLChannel();
    ~PSSLChannel();

    int Accept(PChannel & channel);

    // Normal socket read & write functions
    BOOL   Read(void * buf, PINDEX len);
    BOOL   Write(const void * buf, PINDEX len);

    BOOL   RawRead(void * buf, PINDEX len);
    PINDEX RawGetLastReadCount() const;
    BOOL   RawWrite(const void * buf, PINDEX len);
    PINDEX RawGetLastWriteCount() const;

    BOOL Shutdown(ShutdownValue) { return TRUE; }

  protected:
    int Set_SSL_Fd();

    static SSL_CTX * sslContext;
    static int       sslContextRefCount;

    SSL * ssl;
};

#endif
