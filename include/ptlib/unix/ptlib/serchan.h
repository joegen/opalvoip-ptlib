/*
 * $Id: serchan.h,v 1.6 1996/08/03 12:08:19 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
 * Revision 1.6  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.5  1996/05/03 13:12:07  craigs
 * More Sun4 fixes
 *
 * Revision 1.4  1996/05/02 12:01:47  craigs
 * More Sun4 fixed
 *
 * Revision 1.3  1996/05/02 11:55:01  craigs
 * Fixed problem with compiling on Sun4
 *
 * Revision 1.2  1995/12/08 13:14:52  craigs
 * Added new function
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 */

#ifndef _PSERIALCHANNEL

#pragma interface

#include "../../common/ptlib/serchan.h"
  public:
    BOOL Close();

  private:
    struct termios oldTermio;
    struct termios Termio;

    DWORD  baudRate;
    BYTE   dataBits;
    Parity parityBits;
    BYTE   stopBits;
};

#endif
