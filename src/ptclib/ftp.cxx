/*
 * $Id: ftp.cxx,v 1.10 1997/07/14 11:47:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: ftp.cxx,v $
 * Revision 1.10  1997/07/14 11:47:09  robertj
 * Added "const" to numerous variables.
 *
 * Revision 1.9  1996/09/14 13:09:26  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.8  1996/05/30 10:04:46  robertj
 * Fixed bug in breaking accept within FTP constructor returning wrong error code.
 *
 * Revision 1.7  1996/05/26 03:46:36  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.6  1996/05/23 09:56:27  robertj
 * Changed FTP so can do passive/active mode on all data transfers.
 *
 * Revision 1.5  1996/03/31 09:01:20  robertj
 * More FTP client implementation.
 *
 * Revision 1.4  1996/03/26 00:50:30  robertj
 * FTP Client Implementation.
 *
 * Revision 1.3  1996/03/18 13:33:15  robertj
 * Fixed incompatibilities to GNU compiler where PINDEX != int.
 *
 * Revision 1.2  1996/03/16 04:51:12  robertj
 * Changed lastResponseCode to an integer.
 *
 * Revision 1.1  1996/03/04 12:12:51  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>
#include <ftp.h>


/////////////////////////////////////////////////////////
//  File Transfer Protocol

static const char * const FTPCommands[PFTP::NumCommands] = 
{
  "USER", "PASS", "ACCT", "CWD", "CDUP", "SMNT", "QUIT", "REIN", "PORT", "PASV",
  "TYPE", "STRU", "MODE", "RETR", "STOR", "STOU", "APPE", "ALLO", "REST", "RNFR",
  "RNTO", "ABOR", "DELE", "RMD", "MKD", "PWD", "LIST", "NLST", "SITE", "SYST",
  "STAT", "HELP", "NOOP"
};

PFTP::PFTP()
  : PInternetProtocol("ftp 21", NumCommands, FTPCommands)
{
}


BOOL PFTP::SendPORT(const PIPSocket::Address & addr, WORD port)
{
  PString str(PString::Printf,
              "%i,%i,%i,%i,%i,%i",
              addr.Byte1(),
              addr.Byte2(),
              addr.Byte3(),
              addr.Byte4(),
              port/256,
              port%256);
  return ExecuteCommand(PORT, str)/100 == 2;
}


// End of File ///////////////////////////////////////////////////////////////
