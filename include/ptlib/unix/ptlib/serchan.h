/*
 * $Id: serchan.h,v 1.2 1995/12/08 13:14:52 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
 * Revision 1.2  1995/12/08 13:14:52  craigs
 * Added new function
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 */

#ifndef _PSERIALCHANNEL

#pragma interface

#include <termio.h>

#include "../../common/serchan.h"
  public:
    BOOL Close();

  private:
    struct termio oldTermio;
    struct termio Termio;

    DWORD  baudRate;
    BYTE   dataBits;
    Parity parityBits;
    BYTE   stopBits;
};

#endif
