/*
 * $Id: socket.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */

#ifndef _PSOCKET


#include "::common:socket.h"
  public:
    PSocket();
      // create an unattached socket
    ~PSocket();
      // close a socket

    virtual BOOL Read(void * buf, PINDEX len);
    virtual BOOL Write(const void * buf, PINDEX len);
    virtual BOOL Close();

  protected:
    BOOL ConvertOSError(int error);
    BOOL _WaitForData(BOOL reading);
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
