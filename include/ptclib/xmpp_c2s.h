/*
 * xmpp_c2s.h
 *
 * Extensible Messaging and Presence Protocol (XMPP) Core
 * Client to Server communication classes
 *
 * Portable Windows Library
 *
 * Copyright (c) 2004 Reitek S.p.A.
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
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: xmpp_c2s.h,v $
 * Revision 1.1  2004/04/22 12:31:00  rjongbloed
 * Added PNotifier extensions and XMPP (Jabber) support,
 *   thanks to Federico Pinna and Reitek S.p.A.
 *
 *
 */

#if P_EXPAT
#ifndef _XMPP_C2S
#define _XMPP_C2S

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/xmpp.h>
#include <ptlib/sockets.h>

///////////////////////////////////////////////////////

/** XMPP client to server TCP transport
 */
class XMPP_C2S_TCPTransport : public XMPPTransport
{
    PCLASSINFO(XMPP_C2S_TCPTransport, XMPPTransport);

  public:
    XMPP_C2S_TCPTransport(const PString& hostname, WORD port = 5222);
    ~XMPP_C2S_TCPTransport();

    const PString&  GetServerHost() const   { return m_Hostname; }
    WORD            GetServerPort() const   { return m_Port; }

    virtual BOOL Open();
    virtual BOOL Close();

  protected:
    const PString&  m_Hostname;
    const WORD      m_Port;
    PTCPSocket *    m_Socket;
};

///////////////////////////////////////////////////////

/** This class handles the client side of a C2S (Client to Server)
    XMPP stream.
 */
class XMPP_C2S : public XMPPStreamHandler
{
    PCLASSINFO(XMPP_C2S, XMPPStreamHandler);

  public:
    XMPP_C2S(const PString& uid,
      const PString& server,
      const PString& resource,
      const PString& pwd);

    ~XMPP_C2S();

    /** These notifier lists are fired when a XMPP stanza or a
    stream error is received. For the notifier lists to be fired
    the stream must be already in the established state (i.e.
    after the bind and the session state)
    */
    PNotifierList&  ErrorHandlers()     { return m_ErrorHandlers; }
    PNotifierList&  MessageHandlers()   { return m_MessageHandlers; }
    PNotifierList&  PresenceHandlers()  { return m_PresenceHandlers; }
    PNotifierList&  IQHandlers()        { return m_IQHandlers; }

  protected:
    const PString       m_UserID;
    const PString       m_Server;
    PString             m_Resource; // the resource can change: the server can force one
    const PString       m_Password;
    PSASLClient         m_SASL;
    PString             m_Mechanism;
    BOOL                m_HasBind;
    BOOL                m_HasSession;

    PNotifierList       m_ErrorHandlers;
    PNotifierList       m_MessageHandlers;
    PNotifierList       m_PresenceHandlers;
    PNotifierList       m_IQHandlers;

    enum C2S_StreamState
    {
      Null,
      TLSStarted,
      SASLStarted,
      StreamSent,
      BindSent,
      SessionSent,
      Established
    };

    C2S_StreamState m_State;

    virtual void    OnOpen(XMPPStream& stream, INT);
    virtual void    OnClose(XMPPStream& stream, INT);

    virtual void    OnElement(PXML& pdu);
    virtual void    OnError(PXML& pdu);
    virtual void    OnMessage(PXML& pdu);
    virtual void    OnPresence(PXML& pdu);
    virtual void    OnIQ(PXML& pdu);

    // State handlers
    virtual void    HandleNullState(PXML& pdu);
    virtual void    HandleTLSStartedState(PXML& pdu);
    virtual void    HandleSASLStartedState(PXML& pdu);
    virtual void    HandleStreamSentState(PXML& pdu);
    virtual void    HandleBindSentState(PXML& pdu);
    virtual void    HandleSessionSentState(PXML& pdu);
    virtual void    HandleEstablishedState(PXML& pdu);
  };

  #endif  // _XMPP_C2S
#endif  // P_EXPAT

// End of File ///////////////////////////////////////////////////////////////

