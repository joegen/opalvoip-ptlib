/*
 * $Id: socket.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: socket.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>


//////////////////////////////////////////////////////////////////////////////
// PSocket

PSocket::PSocket()
{
}


BOOL PSocket::Read(void * buf, PINDEX len)
{
  return TRUE;
}


BOOL PSocket::Write(const void * buf, PINDEX len)
{
  return TRUE;
}


BOOL PSocket::Close()
{
  if (IsOpen())
    return FALSE;
//  return ConvertOSError(closesocket(os_handle));
  return FALSE;
}


BOOL PSocket::ConvertOSError(int error)
{
  if (error >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

//  osError = WSAGetLastError();
  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;
    default :
      lastError = Miscellaneous;
      osError |= 0x20000000;
  }
  return FALSE;
}


// End Of File ///////////////////////////////////////////////////////////////
