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
 * Revision 1.5  2004/05/02 08:58:15  csoutheren
 * Removed warnings when compling without SASL
 *
 * Revision 1.4  2004/04/28 11:26:43  csoutheren
 * Hopefully fixed SASL and SASL2 problems
 *
 * Revision 1.3  2004/04/26 01:51:58  rjongbloed
 * More implementation of XMPP, thanks a lot to Federico Pinna & Reitek S.p.A.
 *
 * Revision 1.2  2004/04/23 06:07:25  csoutheren
 * Added #if P_SASL to allow operation without SASL
 *
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

#if P_DNS
#include <ptclib/pdns.h>
#endif


// If DNS resolver is enabled, we look for a matching SRV record
XMPP::C2S::TCPTransport::TCPTransport(const PString& hostname)
  : m_Hostname(hostname),
    m_Port(5222)
{
#if P_DNS
  PDNS::SRVRecordList srvRecords;

  if (PDNS::GetSRVRecords(PString("_xmpp-client._tcp.") + hostname, srvRecords)) {
    PDNS::SRVRecord * rec = srvRecords.GetFirst();

    if (rec) {
      m_Hostname = rec->hostName;
      m_Port = rec->port;
    }
  }
#endif
}


// A port was specified, so well connect exactly where we're told
XMPP::C2S::TCPTransport::TCPTransport(const PString& hostname, WORD port)
  : m_Hostname(hostname),
    m_Port(port)
{
}


XMPP::C2S::TCPTransport::~TCPTransport()
{
  Close();
}


BOOL XMPP::C2S::TCPTransport::Open()
{
  if (IsOpen())
    Close();

  PTCPSocket * s = new PTCPSocket(m_Hostname, m_Port);
  return PIndirectChannel::Open(s);
}


BOOL XMPP::C2S::TCPTransport::Close()
{
  return PIndirectChannel::Close();
}

///////////////////////////////////////////////////////

XMPP::C2S::StreamHandler::StreamHandler(const JID& jid, const PString& pwd)
  : m_VersionMajor(1), m_VersionMinor(0),
    m_JID(jid), m_Password(pwd),
#if P_SASL2
    m_SASL("xmpp", m_JID.GetShortFormat(), m_JID.GetUser(), m_Password),
#endif
    m_HasBind(FALSE), m_HasSession(FALSE),
    m_State(XMPP::C2S::StreamHandler::Null)
{
  m_PendingIQs.DisallowDeleteObjects();
}


XMPP::C2S::StreamHandler::~StreamHandler()
{
  m_PendingIQsLock.Wait();
  while (m_PendingIQs.GetSize() > 0)
    delete m_PendingIQs.RemoveAt(0);
  m_PendingIQsLock.Signal();
}


BOOL XMPP::C2S::StreamHandler::Start(Transport * transport)
{
  if (!transport)
    transport = new XMPP::C2S::TCPTransport(m_JID.GetServer());

  return BaseStreamHandler::Start(transport);
}


BOOL XMPP::C2S::StreamHandler::Send(XMPP::Stanza * stanza)
{
  if (!stanza)
    return FALSE;

  if (PIsDescendant(stanza, XMPP::IQ)) {
    XMPP::IQ * iq = (XMPP::IQ *)stanza;

    if (iq->GetResponseHandlers().GetSize() > 0) {

      if (Write(*iq)) {
        m_PendingIQsLock.Wait();
        m_PendingIQs.Append(iq);
        m_PendingIQsLock.Signal();
        return TRUE;
      }
      else {
        delete iq;
        return FALSE;
      }
    }
  }
  
  BOOL res = Write(*stanza);
  delete stanza;
  return res;
}


void XMPP::C2S::StreamHandler::SetVersion(WORD major, WORD minor)
{
  m_VersionMajor = major;
  m_VersionMinor = minor;
}


void XMPP::C2S::StreamHandler::GetVersion(WORD& major, WORD& minor) const
{
  major = m_VersionMajor;
  minor = m_VersionMinor;
}


void XMPP::C2S::StreamHandler::SetState(XMPP::C2S::StreamHandler::StreamState s)
{
  if (s == XMPP::C2S::StreamHandler::Null && m_State == XMPP::C2S::StreamHandler::Established)
    OnSessionReleased();
  else if (s == XMPP::C2S::StreamHandler::Established && m_State != XMPP::C2S::StreamHandler::Established)
    OnSessionEstablished();

  m_State = s;
}


PNotifierList& XMPP::C2S::StreamHandler::IQQueryHandlers(const PString& xml_namespace)
{
  if (!m_IQQueryHandlers.Contains(xml_namespace))
    m_IQQueryHandlers.SetAt(xml_namespace, new PNotifierList);

  return m_IQQueryHandlers[xml_namespace];
}


void XMPP::C2S::StreamHandler::OnOpen(XMPP::Stream& stream, INT extra)
{
  PString streamOn(PString::Printf, "<?xml version='1.0' encoding='UTF-8' ?>"
    "<stream:stream to='%s' xmlns='jabber:client' "
    "xmlns:stream='http://etherx.jabber.org/streams'", (const char *)m_JID.GetServer());

  if (m_VersionMajor >= 1)
    streamOn.sprintf(" version='%d.%d'>", (int)m_VersionMajor, (int)m_VersionMinor);
  else // old jabber protocol
    streamOn += ">";

  stream.Reset();
  stream.Write(streamOn);

  BaseStreamHandler::OnOpen(stream, extra);

  if (m_VersionMajor == 0)
    StartAuthNegotiation();
}


void XMPP::C2S::StreamHandler::OnClose(XMPP::Stream& stream, INT extra)
{
  SetState(XMPP::C2S::StreamHandler::Null);
  m_HasBind = FALSE;
  m_HasSession = FALSE;
  PString streamOff("</stream:stream>");

  stream.Write(streamOff);

  BaseStreamHandler::OnClose(stream, extra);
}


void XMPP::C2S::StreamHandler::StartAuthNegotiation()
{
#if P_SASL2
  // We have SASL, but we might have not found a mechanism in
  // common, or we are just supporting the old jabber protocol
  if (m_VersionMajor == 0 || m_Mechanism.IsEmpty())
#endif
  {
    // JEP-0078 Non SASL authentication
    PString auth(PString::Printf, "<iq type='get' to='%s' id='auth1'>"
                                     "<query xmlns='jabber:iq:auth'>"
                                        "<username>%s</username>"
                                     "</query></iq>",
                                     (const char *)m_JID.GetServer(), (const char *)m_JID.GetUser());

    m_Stream->Write(auth);
    SetState(XMPP::C2S::StreamHandler::NonSASLStarted);
  }
#if P_SASL2
  else {
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

    m_Stream->Write(auth);
    SetState(XMPP::C2S::StreamHandler::SASLStarted);
  }
#endif
}


void XMPP::C2S::StreamHandler::OnSessionEstablished()
{
  m_SessionEstablishedHandlers.Fire(*this);
}


void XMPP::C2S::StreamHandler::OnSessionReleased()
{
  m_SessionReleasedHandlers.Fire(*this);
}


void XMPP::C2S::StreamHandler::OnElement(PXML& pdu)
{
  switch (m_State)
  {
  case Null:
    HandleNullState(pdu);
    break;

  case TLSStarted:
    HandleTLSStartedState(pdu);
    break;

#if P_SASL2
  case SASLStarted:
    HandleSASLStartedState(pdu);
    break;
#endif

  case NonSASLStarted:
    HandleNonSASLStartedState(pdu);
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


void XMPP::C2S::StreamHandler::HandleNullState(PXML& pdu)
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

//  PXMLElement * starttls = pdu.GetRootElement()->GetElement("starttls");
  PStringSet ourMechSet;

  // We might have to negotiate the TLS first, but we set up the SASL phase now

#if P_SASL2
  PXMLElement * mechList = pdu.GetRootElement()->GetElement("mechanisms");
  if (!mechList || !m_SASL.Init(m_JID.GetServer(), ourMechSet))
  {
    // SASL initialisation failed, goodbye...
    Stop();
    return;
  }

  PXMLElement * mech;
  PINDEX i = 0;

  while ((mech = mechList->GetElement("mechanism", i++)) != 0)
  {
    if (ourMechSet.Contains(mech->GetData())) // Hit
      break;
  }
  if (mech != 0)
    m_Mechanism = mech->GetData();
#endif

  // That's how it'll be once we support TLS
  /*if (starttls && (m_UseTLS || starttls->GetElement("required") != 0))
  {
    // we must start the TLS nogotiation...
    SetState(XMPP::C2S::StreamHandler::TLSStarted);
  }
  else*/
    StartAuthNegotiation();
}


void XMPP::C2S::StreamHandler::HandleTLSStartedState(PXML& /*pdu*/)
{
  PAssertAlways(PUnimplementedFunction);
}


#if P_SASL2
void XMPP::C2S::StreamHandler::HandleSASLStartedState(PXML& pdu)
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
    m_Stream->Write(response);
  }
  else if (name == "success")
  {
    m_SASL.End();
    OnOpen(*m_Stream, 0); // open the inner stream (i.e. reset the parser)
    SetState(XMPP::C2S::StreamHandler::StreamSent);
  }
  else
  {
    Stop();
  }
}
#endif


void XMPP::C2S::StreamHandler::HandleNonSASLStartedState(PXML& pdu)
{
  PXMLElement * elem = pdu.GetRootElement();

  if (elem->GetName() != "iq" || elem->GetAttribute("type") != "result") {
    Stop();
    return;
  }

  elem = elem->GetElement(XMPP::IQQuery);

  if (elem == 0) { // Authentication succeded
    SetState(XMPP::C2S::StreamHandler::Established);
  }
  else {

    PString auth(PString::Printf, "<iq type='set' to='%s' id='auth2'>"
                                     "<query xmlns='jabber:iq:auth'>", (const char *)m_JID.GetServer());

    PINDEX i = 0;
    PXMLElement * item;
    BOOL uid = FALSE, pwd = FALSE, digest = FALSE, res = FALSE;

    while ((item = (PXMLElement *)elem->GetElement(i++)) != 0) {
      PString name = item->GetName();

      if (name *= "username")
        uid = TRUE;
      else if (name *= "password")
        pwd = TRUE;
      else if (name *= "digest")
        digest = TRUE;
      else if (name *= "resource")
        res = TRUE;
    }

    if (uid)
      auth += "<username>" + m_JID.GetUser() + "</username>";

    if (res)
      auth += "<resource>" + m_JID.GetResource() + "</resource>";

/*
    if (digest) {
      // We need the stream id, an attribute of the root element
      PXMLElement * stream = m_Stream->GetParser()->GetXMLTree();
      PString id = stream ? stream->GetAttribute("id") : PString::Empty();

      PMessageDigest::Result bin_digest;
      PMessageDigestSHA1::Encode(id + m_Password, bin_digest);
      PString digest;

      const BYTE * data = bin_digest.GetPointer();

      for (PINDEX i = 0, max = bin_digest.GetSize(); i < max ; i += 4)
        digest.sprintf("%08x", *((unsigned *)&(data[i])));

      auth += "<digest>" + digest + "</digest>";
    }
    else*/ if (pwd)
      auth += "<password>" + m_Password + "</password>";

    auth += "</query></iq>";
    m_Stream->Write(auth);
  }
}


void XMPP::C2S::StreamHandler::HandleStreamSentState(PXML& pdu)
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

    if (m_JID.GetResource().IsEmpty())
      bind += "/></iq>";
    else
    {
      bind += "><resource>";
      bind += m_JID.GetResource();
      bind += "</resource></bind></iq>";
    }

    m_Stream->Write(bind);
    SetState(XMPP::C2S::StreamHandler::BindSent);
  }
  else if (m_HasSession)
    HandleBindSentState(pdu);
  else
    SetState(XMPP::C2S::StreamHandler::Established);
}


void XMPP::C2S::StreamHandler::HandleBindSentState(PXML& pdu)
{
  if (m_State == XMPP::C2S::StreamHandler::BindSent)
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
    m_Stream->Write(session);
    SetState(XMPP::C2S::StreamHandler::SessionSent);
  }
  else
    SetState(XMPP::C2S::StreamHandler::Established);
}


void XMPP::C2S::StreamHandler::HandleSessionSentState(PXML& pdu)
{
  PXMLElement * elem = pdu.GetRootElement();

  if (elem->GetName() != "iq" || elem->GetAttribute("type") != "result")
  {
    Stop();
    return;
  }

  SetState(XMPP::C2S::StreamHandler::Established);
}


void XMPP::C2S::StreamHandler::HandleEstablishedState(PXML& pdu)
{
  PCaselessString name = pdu.GetRootElement()->GetName();

  if (name == "stream:error") {
    OnError(pdu);
    Stop();
  }
  else if (name == XMPP::MessageStanza) {
    XMPP::Message msg(pdu);

    if (msg.IsValid())
      OnMessage(msg);
    else
      Stop("bad-format");
  }
  else if (name == XMPP::PresenceStanza) {
    XMPP::Presence pre(pdu);

    if (pre.IsValid())
      OnPresence(pre);
    else
      Stop("bad-format");
  }
  else if (name == XMPP::IQStanza) {
    XMPP::IQ iq(pdu);

    if (iq.IsValid())
      OnIQ(iq);
    else
      Stop("bad-format");
  }
  else
    Stop("unsupported-stanza-type");
}


void XMPP::C2S::StreamHandler::OnError(PXML& pdu)
{
  m_ErrorHandlers.Fire(pdu);
}


void XMPP::C2S::StreamHandler::OnMessage(XMPP::Message& pdu)
{
  m_MessageHandlers.Fire(pdu);
}


void XMPP::C2S::StreamHandler::OnPresence(XMPP::Presence& pdu)
{
  m_PresenceHandlers.Fire(pdu);
}


void XMPP::C2S::StreamHandler::OnIQ(XMPP::IQ& pdu)
{
  XMPP::IQ::IQType type = pdu.GetType();
  XMPP::IQ * origMsg = 0;

  if (type == XMPP::IQ::Result || type == XMPP::IQ::Error) {
    PString id = pdu.GetID();

    m_PendingIQsLock.Wait();
    for (PINDEX i = 0, max = m_PendingIQs.GetSize() ; i < max ; i++)
      if (((XMPP::IQ&)(m_PendingIQs[i])).GetID() == id) {
        origMsg = (XMPP::IQ *)m_PendingIQs.RemoveAt(i);
        pdu.SetOriginalMessage(origMsg);
      }
    m_PendingIQsLock.Signal();
  }

  if (origMsg != 0)
    origMsg->GetResponseHandlers().Fire(pdu);

  // If it's a "query" iq, let's see if someone is registered to handled it
  PXMLElement * query = pdu.GetElement(XMPP::IQQuery);
  PString xmlns = query != 0 ? query->GetAttribute(XMPP::Namespace) : PString::Empty();

  if (!xmlns.IsEmpty() && m_IQQueryHandlers.Contains(xmlns))
    m_IQQueryHandlers[xmlns].Fire(pdu);

  // Now the "normal" handlers
  m_IQHandlers.Fire(pdu);

  // If it was a set or a get and nobody took care of it, we send and error back
  if ((type == XMPP::IQ::Set || type == XMPP::IQ::Get) && !pdu.HasBeenProcessed()) {
    XMPP::IQ * error = pdu.BuildError("cancel", "feature-not-implemented");
    Send(error);
  }
}


#endif // P_EXPAT

// End of File ///////////////////////////////////////////////////////////////


