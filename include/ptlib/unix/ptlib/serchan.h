/*
 * $Id: serchan.h,v 1.3 1996/05/02 11:55:01 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
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

#include <sys/ioctl.h>

#ifdef P_SUN4
#undef NL0
#undef NL1
#undef CR0
#undef CR1
#undef CR2
#undef CR3
#undef TAB0
#undef TAB1
#undef TAB2
#undef XTABS
#undef BS0
#undef BS1
#undef FF0
#undef FF1
#undef ECHO
#undef NOFLSH
#undef TOSTOP
#undef FLUSHO
#undef PENDIN
#endif

#include <sys/termios.h>

#include "../../common/serchan.h"
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
