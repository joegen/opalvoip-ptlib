/*
 * xmpp_c2s.cxx
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
 * $Log: xmpp_c2s.cxx,v $
 * Revision 1.1  2004/04/22 12:31:00  rjongbloed
 * Added PNotifier extensions and XMPP (Jabber) support,
 *   thanks to Federico Pinna and Reitek S.p.A.
 *
 *
 */

#ifdef __GNUC__
#pragma implementation "xmpp_c2s.h"
#endif

#include <ptlib.h>
#include <ptclib/xmpp_c2s.h>

#if P_EXPAT


XMPP_C2S_TCPTransport::XMPP_C2S_TCPTransport(const PString& hostname, WORD port)
  : m_Hostname(hostname),
    m_Port(port)
{
}


XMPP_C2S_TCPTransport::~XMPP_C2S_TCPTransport()
{
  Close();
}


BOOL XMPP_C2S_TCPTransport::Open()
{
  if (IsOpen())
    Close();

  PTCPSocket * s = new PTCPSocket(m_Hostname, m_Port);
  return PIndirectChannel::Open(s);
}


BOOL XMPP_C2S_TCPTransport::Close()
{
  return PIndirectChannel::Close();
}

///////////////////////////////////////////////////////

XMPP_C2S::XMPP_C2S(const PString& uid, const PString& server,
                   const PString& resource, const PString& pwd)
  : m_UserID(uid), m_Server(server), m_Resource(resource), m_Password(pwd),
    m_SASL("xmpp", m_UserID + PString("@") + m_Server, m_UserID, m_Password),
    m_HasBind(FALSE), m_HasSession(FALSE),
    m_State(XMPP_C2S::Null)
{
}


XMPP_C2S::~XMPP_C2S()
{
}


void XMPP_C2S::OnOpen(XMPPStream& stream, INT extra)
{
  PString streamOn(PString::Printf, "<?xml version='1.0' encoding='UTF-8' ?>"
    "<stream:stream to='%s' xmlns='jabber:client' "
    "xmlns:stream='http://etherx.jabber.org/streams' version='1.0'>", (const char *)m_Server);

  stream.Reset();
  stream.Write((const char *)streamOn, streamOn.GetLength());

  XMPPStreamHandler::OnOpen(stream, extra);
}


void XMPP_C2S::OnClose(XMPPStream& stream, INT extra)
{
  m_State = XMPP_C2S::Null;
  m_HasBind = FALSE;
  m_HasSession = FALSE;
  PString streamOff("</stream:stream>");

  stream.Write((const char *)streamOff, streamOff.GetLength());

  XMPPStreamHandler::OnClose(stream, extra);
}


void XMPP_C2S::OnElement(PXML& pdu)
{
  switch(m_State)
  {
  case Null:
    HandleNullState(pdu);
    break;

  case TLSStarted:
    HandleTLSStartedState(pdu);
    break;

  case SASLStarted:
    HandleSASLStartedState(pdu);
    break;

  case StreamSent:
    HandleStreamSentState(pdu);
    break;

  case BindSent:
    HandleBindSentState(pdu);
    break;

  case SessionSent:
    HandleSessionSentState(pdu);
    break;

  case Established:
    HandleEstablishedState(pdu);
    break;

  default:
    // Error
    PAssertAlways(PLogicError);
  }
}


void XMPP_C2S::HandleNullState(PXML& pdu)
{
  if (pdu.GetRootElement()->GetName() != "stream:features")
  {
    Stop();
    return;
  }

  /* This is what we are kind of expecting (more or less)
  <stream:features>
  <starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'>
  <required/>
  </starttls>
  <mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>
  <mechanism>DIGEST-MD5</mechanism>
  <mechanism>PLAIN</mechanism>
  </mechanisms>
  </stream:features>
  */

  PXMLElement * starttls = pdu.GetRootElement()->GetElement("starttls");
  PXMLElement * mechList = pdu.GetRootElement()->GetElement("mechanisms");
  PStringSet ourMechSet;

  // We might have to negotiate the TLS first, but we set up the SASL phase now

  if (!mechList || !m_SASL.Init(m_Server, ourMechSet))
  {
    // SASL initialisation failed, goodbye...
    Stop();
    return;
  }

  PXMLElement * mech;
  PINDEX i = 0;

  while ((mech = mechList->GetElement("mechanism", i++)) != 0)
  {
    if (ourMechSet.Contains(m_Mechanism = mech->GetData())) // Hit
      break;
  }

  if (!mech) // No shared mechanism found
  {
    Stop();
    return;
  }

  if (starttls && /* m_UseTLS && */ starttls->GetElement("required") != 0)
  {
    // we must start the TLS nogotiation...
    m_State = XMPP_C2S::TLSStarted;
  }
  else
  {
    // Go with SASL!
    PString output;

    if (!m_SASL.Start(m_Mechanism, output))
    {
      Stop();
      return;
    }

    PString auth("<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='");
    auth += m_Mechanism;

    if (output.IsEmpty())
      auth += "'/>";
    else
    {
      auth += "'>";
      auth += output;
      auth += "</auth>";
    }

    m_Stream->Write((const char *)auth, auth.GetLength());

    m_State = XMPP_C2S::SASLStarted;
  }
}


void XMPP_C2S::HandleTLSStartedState(PXML& /*pdu*/)
{
  PAssertAlways(PUnimplementedFunction);
}


void XMPP_C2S::HandleSASLStartedState(PXML& pdu)
{
  PString name = pdu.GetRootElement()->GetName();

  if (name == "challenge")
  {
    PString input = pdu.GetRootElement()->GetData();
    PString output;

    if (m_SASL.Negotiate(input, output) == PSASLClient::Fail)
    {
      Stop();
      return;
    }

    PString response("<response xmlns='urn:ietf:params:xml:ns:xmpp-sasl'");
    if (output.IsEmpty())
      response += "/>";
    else
    {
      response += ">";
      response += output;
      response += "</response>";
    }
    m_Stream->Write((const char *)response, response.GetLength());
  }
  else if (name == "success")
  {
    m_SASL.End();
    OnOpen(*m_Stream, 0); // open the inner stream (i.e. reset the parser)

    //CABLONSKY
    /*
    m_Stream->Close();
    m_Stream->SetReadChannel(new PTextFile("c:\\tmp\\j2.txt", PFile::ReadOnly));
    m_Stream->SetWriteChannel(new PConsoleChannel(PConsoleChannel::StandardOutput));
    */
    //CABLONSKY

    m_State = XMPP_C2S::StreamSent;
  }
  else
  {
    Stop();
  }
}


void XMPP_C2S::HandleStreamSentState(PXML& pdu)
{
  if (pdu.GetRootElement()->GetName() != "stream:features")
  {
    Stop();
    return;
  }
  /*
  <stream:features>
  <bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>
  <session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>
  </stream:features>
  */

  m_HasBind = pdu.GetRootElement()->GetElement("bind") != 0;
  m_HasSession = pdu.GetRootElement()->GetElement("session") != 0;

  if (m_HasBind)
  {
    PString bind("<iq type='set' id='bind_1'>"
      "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'");

    if (m_Resource.IsEmpty())
      bind += "/></iq>";
    else
    {
      bind += "><resource>";
      bind += m_Resource;
      bind += "</resource></bind></iq>";
    }

    m_Stream->Write((const char *)bind, bind.GetLength());
    m_State = XMPP_C2S::BindSent;
  }
  else if (m_HasSession)
    HandleBindSentState(pdu);
  else
    m_State = XMPP_C2S::Established;
}


void XMPP_C2S::HandleBindSentState(PXML& pdu)
{
  if (m_State == XMPP_C2S::BindSent)
  {
    PXMLElement * elem = pdu.GetRootElement();

    if (elem->GetName() != "iq" || elem->GetAttribute("type") != "result")
    {
      Stop();
      return;
    }

    if ((elem = elem->GetElement("bind")) == 0 || (elem = elem->GetElement("jid")) == 0)
    {
      Stop();
      return;
    }

    PString jid = elem->GetData();
  }

  if (m_HasSession)
  {
    PString session = "<iq id='sess_1' type='set'><session xmlns='urn:ietf:params:xml:ns:xmpp-session'/></iq>";
    m_Stream->Write((const char *)session, session.GetLength());
    m_State = XMPP_C2S::SessionSent;
  }
  else
    m_State = XMPP_C2S::Established;
}


void XMPP_C2S::HandleSessionSentState(PXML& pdu)
{
  PXMLElement * elem = pdu.GetRootElement();

  if (elem->GetName() != "iq" || elem->GetAttribute("type") != "result")
  {
    Stop();
    return;
  }

  m_State = XMPP_C2S::Established;
}


void XMPP_C2S::HandleEstablishedState(PXML& pdu)
{
  PCaselessString name = pdu.GetRootElement()->GetName();

  if (name == "stream:error")
  {
    OnError(pdu);
    Stop();
  }
  else if (name == "message")
    OnMessage(pdu);
  else if (name == "presence")
    OnPresence(pdu);
  else if (name == "iq")
    OnIQ(pdu);
  else
    Stop("unsupported-stanza-type");
}


void XMPP_C2S::OnError(PXML& pdu)
{
  m_ErrorHandlers.Fire(pdu);
}


void XMPP_C2S::OnMessage(PXML& pdu)
{
  m_MessageHandlers.Fire(pdu);
}


void XMPP_C2S::OnPresence(PXML& pdu)
{
  m_PresenceHandlers.Fire(pdu);
}


void XMPP_C2S::OnIQ(PXML& pdu)
{
  m_IQHandlers.Fire(pdu);
}


#endif // P_EXPAT


// End of File ///////////////////////////////////////////////////////////////

