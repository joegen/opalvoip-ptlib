/*
 * $Id: sockets.cxx,v 1.1 1994/08/01 03:39:05 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
 * Revision 1.1  1994/08/01 03:39:05  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>


BOOL PSocket::Open (const PString & address, int portnum)
{
  PAssertAlways(PLogicError);
  return FALSE;
}


BOOL PSocket::Accept (const PString & address)
{
  PAssertAlways(PLogicError);
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PTCPSocket

BOOL PTCPSocket::Open (const PString host_address, int port)

{
  return FALSE;
}
