/*
 * $Id: sockets.cxx,v 1.3 1994/11/28 12:38:49 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
 * Revision 1.3  1994/11/28 12:38:49  robertj
 * Added DONT and WONT states.
 *
 * Revision 1.2  1994/08/21  23:43:02  robertj
 * Some implementation.
 *
 * Revision 1.1  1994/08/01  03:39:05  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>


//////////////////////////////////////////////////////////////////////////////
// PSocket

BOOL PSocket::Open (const PString &, WORD)
{
  PAssertAlways(PLogicError);
  return FALSE;
}


BOOL PSocket::Accept (const PString &)
{
  PAssertAlways(PLogicError);
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PIPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

BOOL PIPSocket::LookupHost(const PString & host_address, sockaddr_in * address)
{
  struct hostent *host_info;

  // lookup the host address using inet_addr, assuming it is a "." address
  if ((address->sin_addr.s_addr = inet_addr((const char *)host_address)) != -1)
    address->sin_family = AF_INET;

  // otherwise lookup the name as a host name
  else if ((host_info = gethostbyname ((const char *)host_address)) != 0) {
    address->sin_family = host_info->h_addrtype;
    memcpy(&address->sin_addr, host_info->h_addr, host_info->h_length);
  }

  // otherwise we don't know about the host at all!
  else {
    lastError = NotFound;
    return FALSE;
  }

  return TRUE;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

BOOL PTCPSocket::Open (const PString & host_address, u_short port)
{
  sockaddr_in address;

  // attempt to lookup the host name
  if (!LookupHost(host_address, &address))
    return FALSE;

  // set the port
  address.sin_port = ::htons(port);

  // attempt to create a socket
  if (!ConvertOSError(os_handle = ::socket(AF_INET, SOCK_STREAM, 0)))
    return FALSE;

  // attempt to connect
  if (!ConvertOSError(::connect(os_handle, (struct sockaddr *)&address, sizeof(address)))) {
    Close();
    return FALSE;
  }

  return TRUE;
}


#endif


//////////////////////////////////////////////////////////////////////////////
// PTelnetSocket

PTelnetSocket::PTelnetSocket()
{
  Construct();
}


PTelnetSocket::PTelnetSocket(const PString & address, WORD port)
{
  Construct();
  Open(address, port);
}


void PTelnetSocket::Construct()
{
 state = StateNormal;
 debug = TRUE;
}


BOOL PTelnetSocket::Open(const PString & address, WORD port)
{
  return PTCPSocket::Open(address, port);
}


BOOL PTelnetSocket::Read(void * data, PINDEX bytesToRead)
{
  PBYTEArray buffer(bytesToRead);
  PINDEX charsLeft = bytesToRead;
  BYTE * dst = (BYTE *)data;

  while (charsLeft > 0) {
    BYTE * src = buffer.GetPointer(charsLeft);
    if (!PTCPSocket::Read(src, charsLeft)) {
      lastReadCount = bytesToRead - charsLeft;
      return lastReadCount > 0;
    }

    while (lastReadCount > 0) {
      switch (state) {
        case StateIAC:
          switch (*src) {
            case IAC:
              state = StateNormal;
              return IAC;

            case DO:
              state = StateDo;
              break;

            case DONT:
              state = StateDont;
              break;

            case WILL:
              state = StateWill;
              break;

            case WONT:
              state = StateWont;
              break;

            default:
              if (OnUnknownCommand(*src))
                state = StateNormal;
              else
                state = StateUnknownCommand;
              break;
          }
          break;

        case StateDo:
          OnDo(*src);
          state = StateNormal;
          break;

        case StateDont:
          OnDont(*src);
          state = StateNormal;
          break;

        case StateWill:
          OnWill(*src);
          state = StateNormal;
          break;

        case StateWont:
          OnWont(*src);
          state = StateNormal;
          break;

        case StateUnknownCommand:
          if (OnUnknownCommand(*src))
            state = StateNormal;
          else
            state = StateUnknownCommand;
          break;

        case StateNormal:
        default:
          if (*src == IAC)
            state = StateIAC;
          else {
            *dst++ = *src;
            charsLeft--;
          }
          break;
      }
      lastReadCount--;
      src++;
    }
  }
  lastReadCount = bytesToRead;
  return TRUE;
}


#define IMPLEMENT_SEND_CMD(n,c) \
  void PTelnetSocket::Send ##n## (BYTE code) \
  { BYTE cmd[3] = {IAC, c}; cmd[2] = code; PTCPSocket::Write(cmd, 3);  }

IMPLEMENT_SEND_CMD(Will, WILL)
IMPLEMENT_SEND_CMD(Wont, WONT)
IMPLEMENT_SEND_CMD(Do,   DO)
IMPLEMENT_SEND_CMD(Dont, DONT)


void PTelnetSocket::OnDo(BYTE code)
{
  if (debug)
    PError << "DO " << (int)code << endl;
  SendWont(code);
}


void PTelnetSocket::OnDont(BYTE code)
{
  if (debug)
    PError << "DONT " << (int)code << endl;
}


void PTelnetSocket::OnWill(BYTE code)
{
  if (debug)
    PError << "WILL " << (int)code << endl;
  SendDont(code);
}


void PTelnetSocket::OnWont(BYTE code)
{
  if (debug)
    PError << "WONT " << (int)code << endl;
}


BOOL PTelnetSocket::OnUnknownCommand(BYTE code)
{
  if (debug)
    PError << "unknown telnet command " << (int)code << endl;
  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
