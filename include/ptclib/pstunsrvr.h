/*
 * pstunsrvr.h
 *
 * STUN server
 *
 * Portable Tools Library
 *
 * Copyright (c) 2011 Post Increment.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Tools Library.
 *
 * The Initial Developer of the Original Code is Post Increment.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PSTUNSRVR_H
#define PTLIB_PSTUNSRVR_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_STUNSRVR

#include <ptclib/pstun.h>


class PSTUNServer : public PObject, public PSTUN
{
  PCLASSINFO(PSTUNServer, PObject)
  public:
    PSTUNServer();
    
    bool Open(WORD port = DefaultPort);
    bool Open(PUDPSocket * socket1, PUDPSocket * socket2 = NULL);

    bool IsOpen() const;

    bool Close();

    struct SocketInfo {
      SocketInfo(PUDPSocket * socket = NULL);

      PUDPSocket * m_socket;
      PIPSocketAddressAndPort m_socketAddress;
      PIPSocketAddressAndPort m_alternateAddressAndPort;

      PUDPSocket * m_alternatePortSocket;
      PUDPSocket * m_alternateAddressSocket;
      PUDPSocket * m_alternateAddressAndPortSocket;
    };

    virtual bool Read(
      PSTUNMessage & message, 
      SocketInfo & socketInfo
    );

    virtual bool Process();

    virtual bool OnReceiveMessage(
      const PSTUNMessage & message,
      const SocketInfo & socketInfo
      );

    virtual bool OnBindingRequest(
      const PSTUNMessage & request,
      const SocketInfo & socketInfo
    );

    virtual void OnBindingResponse(
      const PSTUNMessage & request,
      PSTUNMessage & response
    );

    virtual bool OnUnknownRequest(
      const PSTUNMessage & request,
      const SocketInfo & socketInfo
    );

  protected:
    void PopulateInfo(PUDPSocket * socket, const PIPSocket::Address & alternateAddress, WORD alternatePort, 
             PUDPSocket * alternatePortSocket, PUDPSocket * alternateAddressSocket, PUDPSocket * alternateAddressAndPortSocket);

    SocketInfo * CreateAndAddSocket(const PIPSocket::Address & addess, WORD port);

    typedef std::map<PUDPSocket *, SocketInfo> SocketToSocketInfoMap;
    SocketToSocketInfoMap m_socketToSocketInfoMap;
    PSocket::SelectList   m_sockets;
    PSocket::SelectList   m_selectList;

    bool m_autoDelete;

    PTRACE_THROTTLE(m_throttleReceivedPacket, 3, 30000, 5);
};


#endif // P_STUNSRVR

#endif // PTLIB_PSTUNSRVR_H
