/*
 * main.h
 *
 * PWLib application header file for XMPP Console
 *
 * Copyright 2004 Reitek S.p.A.
 *
 * Copied by Derek Smithies, 1)removed all the wxwidget stuff.
 *                           2)turned into a console application.
 *
 */

#ifndef _XMPPConsole_MAIN_H
#define _XMPPConsole_MAIN_H

#include <ptlib.h>
#include <ptlib/pprocess.h>

#if P_EXPAT

#include <ptlib/notifier_ext.h>
#include <ptclib/xmpp_c2s.h>
#include <ptclib/xmpp_roster.h>
#include <ptclib/xmpp_muc.h>


class XMPPFrame : public PObject
{
  PCLASSINFO(XMPPFrame, PObject);
public:
  XMPPFrame();
  ~XMPPFrame();

  void ConnectNow() { OnConnect(); }
  void DisconnectNow() { OnQuit(); }

  PDECLARE_NOTIFIER(PTimer, XMPPFrame, OnReadyForUse);
  PTimer onReadyForUseTimer;

  bool    LocalPartyIsEmpty() { return localParty.IsEmpty(); }
  bool    OtherPartyIsEmpty() { return otherParty.IsEmpty(); }
  PString GetOtherParty() { return otherParty; }
  PString GetLocalParty() { return localParty; }
  bool    IsConnected() {  return isReadyForUse; }

  bool Send(XMPP::Stanza * stanza) { return m_Client->Send(stanza); }

  void OnConnect();
  virtual void OnDisconnect();
  virtual void OnQuit();

  // PTLib events
  PDECLARE_NOTIFIER(XMPP::Message, XMPPFrame, OnError);
  PDECLARE_NOTIFIER(XMPP::Message, XMPPFrame, OnMessage);
  PDECLARE_NOTIFIER(XMPP::Presence, XMPPFrame, OnPresence);
  PDECLARE_NOTIFIER(XMPP::IQ, XMPPFrame, OnIQ);
  PDECLARE_NOTIFIER(XMPP::Roster, XMPPFrame, OnRosterChanged);
  PDECLARE_NOTIFIER(XMPP::C2S::StreamHandler, XMPPFrame, OnSessionEstablished);
  PDECLARE_NOTIFIER(XMPP::C2S::StreamHandler, XMPPFrame, OnSessionReleased);

  void ReportRoster();

  PStringArray & GetAvailableNodes();

private:
  XMPP::Roster * m_Roster;
  XMPP::C2S::StreamHandler * m_Client;

  PStringArray availableNodes;
  PMutex       availableLock;

  PString otherParty;
  PString localParty;
  bool    isReadyForUse;
};


class UserInterface: public PThread
{
  PCLASSINFO(UserInterface, PThread);
   
public:
  UserInterface(XMPPFrame & _frame)
    : PThread(1000, NoAutoDeleteThread),  frame(_frame)
    { Resume(); }
   
  void Main();
     
 protected:

  void ProcessDirectedMessage(PString & message);

  void SendThisMessageTo(PString & _message, PString subject, PString dest);

  void SendThisMessage(PString & _message);

  void ReportAvailableNodes();

  XMPPFrame &frame;

  PTime startTime;
};

#endif

class XMPPConsole :public PProcess
{
  PCLASSINFO(XMPPConsole, PProcess);

public:
  XMPPConsole();

  void Main();

  PString GetPassword() { return password; }
  PString GetUserName() { return userName; }
  PString GetServer()   { return server; }

  static XMPPConsole & Current() { return (XMPPConsole &)PProcess::Current(); }

protected:
  PString password;
  PString userName;
  PString server;
};


#endif  // _XMPPConsole_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
