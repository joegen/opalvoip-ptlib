/*
 * $Id: dossock.cxx,v 1.2 1995/02/05 00:53:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: dossock.cxx,v $
 * Revision 1.2  1995/02/05 00:53:15  robertj
 * Commonised out of band stuff.
 *
// Revision 1.1  1994/10/30  12:06:54  robertj
// Initial revision
//
// Revision 1.1  1994/10/23  05:42:49  robertj
// Initial revision
//
// Revision 1.1  1994/08/22  00:18:02  robertj
// Initial revision
//
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
