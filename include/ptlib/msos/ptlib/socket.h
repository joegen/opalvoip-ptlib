/*
 * $Id: socket.h,v 1.4 1994/12/12 10:10:17 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.4  1994/12/12 10:10:17  robertj
 * Changed so can compile if no winsock available.
 *
 * Revision 1.3  1994/10/30  11:24:22  robertj
 * Fixed DOS version of header.
 *
 * Revision 1.2  1994/10/23  05:36:51  robertj
 * Sockets implementation.
 *
 * Revision 1.1  1994/08/22  00:18:02  robertj
 * Initial revision
 *
 * Revision 1.1  1994/07/27  06:00:10  robertj
 * Initial revision
 *
 */

#ifndef _PSOCKET


#if defined(_WINDOWS) && defined(PHAS_WINSOCK)
#include <winsock.h>
#define P_HAS_BERKELEY_SOCKETS
#endif


#include "../../common/socket.h"
  public:
    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function may block until the
      // requested number of characters were read or the read timeout was
      // reached. The return value indicates that at least one character was
      // read from the channel.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters are written or the write timeout is
      // reached. The return value is TRUE if at least len bytes were written
      // to the channel.

    virtual BOOL Close();
      // Close the socket.

  protected:
      BOOL ConvertOSError(int error);
      // Convert an operating system error into platform independent error.
      // This will set the lastError and osError member variables for access
      // by GetErrorCode() and GetErrorNumber(). Returns TRUE if there was
      // no error.


  private:
#ifdef P_HAS_BERKELEY_SOCKETS
    static BOOL WinSockStarted;
#endif
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
