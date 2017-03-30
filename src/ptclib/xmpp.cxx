/*
 * xmpp.cxx
 *
 * Extensible Messaging and Presence Protocol (XMPP) Core
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
 */

#ifdef __GNUC__
#pragma implementation "xmpp.h"
#endif

#include <ptlib.h>
#include <ptclib/xmpp.h>

#define new PNEW


#if P_EXPAT

///////////////////////////////////////////////////////

const PCaselessString & XMPP::LanguageTag()       { static const PConstCaselessString s("xml:lang"); return s; }
const PCaselessString & XMPP::NamespaceTag()      { static const PConstCaselessString s("xmlns");    return s; }
const PCaselessString & XMPP::MessageStanzaTag()  { static const PConstCaselessString s("message");  return s; }
const PCaselessString & XMPP::PresenceStanzaTag() { static const PConstCaselessString s("presence"); return s; }
const PCaselessString & XMPP::IQStanzaTag()       { static const PConstCaselessString s("iq");       return s; }
const PCaselessString & XMPP::IQQueryTag()        { static const PConstCaselessString s("query");    return s; }

///////////////////////////////////////////////////////

XMPP::JID::JID(const char * jid)
{
  ParseJID(jid);
}


XMPP::JID::JID(const PString& jid)
{
  ParseJID(jid);
}


XMPP::JID::JID(const PString& user, const PString& server, const PString& resource)
  : m_User(user), m_Server(server), m_Resource(resource), m_IsDirty(true)
{
  BuildJID();
}


PObject::Comparison XMPP::JID::Compare(const PObject & obj) const
{
  if (m_IsDirty)
    BuildJID();

  if (PIsDescendant(&obj, XMPP::JID))
    return m_JID.Compare((const PString&)((const XMPP::JID&)obj));
  else if (PIsDescendant(&obj, PString))
    return m_JID.Compare((const PString&)obj);

  PAssertAlways(PInvalidCast);
  return PObject::LessThan;
}


XMPP::JID& XMPP::JID::operator=(const PString & jid)
{
  ParseJID(jid);
  return *this;
}


XMPP::JID::operator const PString&() const
{
  if (m_IsDirty)
    BuildJID();

  return m_JID;
}


void XMPP::JID::SetUser(const PString& user)
{
  m_IsDirty = true;
  m_User = user;
}


void XMPP::JID::SetServer(const PString& server)
{
  m_IsDirty = true;
  m_Server = server;
}


void XMPP::JID::SetResource(const PString& resource)
{
  m_IsDirty = true;
  m_Resource = resource;
}


void XMPP::JID::PrintOn(ostream & strm) const
{
  strm << m_JID;
}


void XMPP::JID::ParseJID(const PString& jid)
{
  m_User[0] = m_Server[0] = m_Resource[0] = 0;

  PINDEX i = jid.Find('@');

  if (i == (jid.GetLength() - 1))
    return;
  else if (i == P_MAX_INDEX)
    SetServer(jid);
  else {
    SetUser(jid.Left(i));
    SetServer(jid.Mid(i + 1));
  }

  i = m_Server.Find('/');

  if (i != P_MAX_INDEX && i != 0) {
    SetResource(m_Server.Mid(i + 1));
    SetServer(m_Server.Left(i));
  }

  BuildJID();
}


void XMPP::JID::BuildJID() const
{
  if (m_User.IsEmpty())
    m_JID = m_Server;
  else
    m_JID = m_User + "@" + m_Server;

  if (!m_Resource.IsEmpty())
    m_JID += "/" + m_Resource;

  m_IsDirty = false;
}

///////////////////////////////////////////////////////

PObject::Comparison XMPP::BareJID::Compare(const PObject & obj) const
{
  if (m_IsDirty)
    BuildJID();

  XMPP::BareJID that;

  if (PIsDescendant(&obj, XMPP::JID))
    that = (const PString&)((const XMPP::JID&)obj);
  else if (PIsDescendant(&obj, PString))
    that = (const PString&)obj;
  else {
    PAssertAlways(PInvalidCast);
    return PObject::LessThan;
  }

  return m_JID.Compare(that.m_JID);
}


XMPP::BareJID& XMPP::BareJID::operator=(const PString & jid)
{
  ParseJID(jid);
  return *this;
}

///////////////////////////////////////////////////////

XMPP::Stream::Stream(XMPP::Transport * transport)
  : m_Parser(new PXMLStreamParser(m_Document))
{
  if (transport)
    Open(transport);
}


XMPP::Stream::~Stream()
{
  delete m_Parser;
  Close();
}


PBoolean XMPP::Stream::Close()
{
  if (IsOpen()) {
    OnClose();
    return PIndirectChannel::Close();
  }

  return false;
}


PBoolean XMPP::Stream::Write(const void * buf, PINDEX len)
{
  PTRACE(5, "XMPP\tSND: " << (const char *)buf);
  return PIndirectChannel::Write(buf, len);
}


PBoolean XMPP::Stream::Write(const PString& data)
{
  return Write((const char *)data, data.GetLength());
}


#if 0
PBoolean XMPP::Stream::Write(const PXMLElement & pdu)
{
  PStringStream os;
  pdu.Output(os, m_Document, 0);
  return Write(os);
}
#endif


PBoolean XMPP::Stream::Write(const PXML & pdu)
{
  if (!pdu.IsLoaded())
    return false;

  PStringStream os;
  pdu.GetRootElement()->Output(os, pdu, 0);
  return Write(os);
}


PXMLElement * XMPP::Stream::Read()
{
  return m_Parser->Read(this);
}


void XMPP::Stream::Reset()
{
  delete m_Parser;
  m_Parser = new PXMLStreamParser(m_Document);
}

///////////////////////////////////////////////////////

XMPP::BaseStreamHandler::BaseStreamHandler()
  : PThread(0x1000, NoAutoDeleteThread, NormalPriority, "XMPP"),
    m_Stream(NULL),
    m_AutoReconnect(true),
    m_ReconnectTimeout(1000)
{
}


XMPP::BaseStreamHandler::~BaseStreamHandler()
{
  Stop();
}


PBoolean XMPP::BaseStreamHandler::Start(XMPP::Transport * transport)
{
  if (m_Stream != NULL)
    Stop();

  m_Stream = new XMPP::Stream();
  m_Stream->OpenHandlers().Add(PCREATE_NOTIFIER(OnOpen));
  m_Stream->CloseHandlers().Add(PCREATE_NOTIFIER(OnClose));

  if (!transport->IsOpen() && !transport->Open())
    return false;

  if (m_Stream->Open(transport))
  {
    if (IsSuspended())
      Resume();
    else
      Restart();
    return true;
  }

  return false;
}


PBoolean XMPP::BaseStreamHandler::Stop(const PString& _error)
{
  if (m_Stream == NULL)
    return false;

  if (!_error.IsEmpty())
  {
    PString error = "<stream:error><";
    error += _error;
    error += " xmlns='urn:ietf:params:xml:ns:xmpp-streams'/></stream:error>";
    m_Stream->Write((const char *)error, error.GetLength());
  }

  m_Stream->Close();

  if (PThread::Current() != this)
    WaitForTermination(10000);

  delete m_Stream;
  m_Stream = NULL;

  return false;
}


void XMPP::BaseStreamHandler::OnOpen(XMPP::Stream&, P_INT_PTR)
{
}


void XMPP::BaseStreamHandler::OnClose(XMPP::Stream&, P_INT_PTR)
{
}


void XMPP::BaseStreamHandler::SetAutoReconnect(PBoolean b, long t)
{
  m_AutoReconnect = b;
  m_ReconnectTimeout = t;
}


PBoolean XMPP::BaseStreamHandler::Write(const void * buf, PINDEX len)
{
  if (m_Stream == NULL)
    return false;

  return m_Stream->Write(buf, len);
}


PBoolean XMPP::BaseStreamHandler::Write(const PString& data)
{
  if (m_Stream == NULL)
    return false;

  return m_Stream->Write(data);
}


PBoolean XMPP::BaseStreamHandler::Write(const PXML & pdu)
{
  if (m_Stream == NULL)
    return false;

  return m_Stream->Write(pdu);
}


void XMPP::BaseStreamHandler::OnElement(PXMLElement & pdu)
{
  m_ElementHandlers(*m_Stream, pdu);
}


void XMPP::BaseStreamHandler::Main()
{
  PXMLElement * pdu;

  for (;;)
  {
    if (!m_Stream || !m_Stream->IsOpen())
      break;

    pdu = m_Stream->Read();

    if (pdu != NULL)
    {
      PTRACE(5, "XMPP", "RCV: " << *pdu);
      OnElement(*pdu);
    }
    else if (m_Stream->GetErrorCode() != PChannel::Timeout)
      break;

    delete pdu;
  }
}

///////////////////////////////////////////////////////

const PCaselessString & XMPP::Stanza::IDTag()   { static const PConstCaselessString s("id");   return s; }
const PCaselessString & XMPP::Stanza::FromTag() { static const PConstCaselessString s("from"); return s; }
const PCaselessString & XMPP::Stanza::ToTag()   { static const PConstCaselessString s("to");   return s; }

void XMPP::Stanza::SetID(const PString& id)
{ 
  if (!id.IsEmpty() && PAssertNULL(m_rootElement) != NULL)
    m_rootElement->SetAttribute(XMPP::Stanza::IDTag(), id);
}

void XMPP::Stanza::SetFrom(const PString& from)
{
  if (!from.IsEmpty() && PAssertNULL(m_rootElement) != NULL)
    m_rootElement->SetAttribute(XMPP::Stanza::FromTag(), from);
}

void XMPP::Stanza::SetTo(const PString& to)
{ 
  if (!to.IsEmpty() && PAssertNULL(m_rootElement) != NULL)
    m_rootElement->SetAttribute(XMPP::Stanza::ToTag(), to);
}

PString XMPP::Stanza::GetID() const
{
  return PAssertNULL(m_rootElement) == NULL ? PString::Empty() : m_rootElement->GetAttribute(XMPP::Stanza::IDTag());
}


PString XMPP::Stanza::GetFrom() const
{
  return PAssertNULL(m_rootElement) == NULL ? PString::Empty() : m_rootElement->GetAttribute(XMPP::Stanza::FromTag());
}


PString XMPP::Stanza::GetTo() const
{
  return PAssertNULL(m_rootElement) == NULL ? PString::Empty() : m_rootElement->GetAttribute(XMPP::Stanza::ToTag());
}


PXMLElement * XMPP::Stanza::GetElement(const PString& name, PINDEX i)
{
  return PXML::GetElement(name, i);
}


static atomic<uint32_t> s_id;

PString XMPP::Stanza::GenerateID()
{
  return psprintf("pdu_%d", ++s_id);
}


///////////////////////////////////////////////////////

const PCaselessString & XMPP::Message::TypeTag()    { static const PConstCaselessString s("type");    return s; }
const PCaselessString & XMPP::Message::SubjectTag() { static const PConstCaselessString s("subject"); return s; }
const PCaselessString & XMPP::Message::BodyTag()    { static const PConstCaselessString s("body");    return s; }
const PCaselessString & XMPP::Message::ThreadTag()  { static const PConstCaselessString s("thread");  return s; }

XMPP::Message::Message()
{
  SetRootElement(XMPP::MessageStanzaTag());
  m_rootElement->SetAttribute(XMPP::Message::TypeTag(), "normal");
  SetID(XMPP::Stanza::GenerateID());
}


XMPP::Message::Message(PXMLElement & pdu)
{
  SetRootElement(new PXMLRootElement(*this, pdu));
}


PBoolean XMPP::Message::IsValid() const
{
  return GetDocumentType() == XMPP::MessageStanzaTag();
}


XMPP::Message::MessageType XMPP::Message::GetType(PString * typeName) const
{
  if (PAssertNULL(m_rootElement) == NULL)
    return XMPP::Message::Unknown;

  PString t = m_rootElement->GetAttribute(XMPP::Message::TypeTag());

  if (typeName != NULL)
    *typeName = t;

  if (t *= "normal")
    return XMPP::Message::Normal;
  else if (t *= "chat")
    return XMPP::Message::Chat;
  else if (t *= "error")
    return XMPP::Message::Error;
  else if (t *= "groupchat")
    return XMPP::Message::GroupChat;
  else if (t *= "headline")
    return XMPP::Message::HeadLine;
  else
    return XMPP::Message::Unknown;
}


PString XMPP::Message::GetLanguage() const
{
  return PAssertNULL(m_rootElement) == NULL ? PString::Empty() : m_rootElement->GetAttribute(XMPP::LanguageTag());
}


PXMLElement * XMPP::Message::GetSubjectElement(const PString& lang)
{
  PXMLElement * dfltSubj = NULL;
  PINDEX i = 0;
  PXMLElement * subj;
  PString l;

  while ((subj = GetElement(XMPP::Message::SubjectTag(), i++)) != NULL) {
    l = subj->GetAttribute(XMPP::LanguageTag());

    if (l == lang)
      return subj;
    else if (l.IsEmpty() && dfltSubj == NULL)
      dfltSubj = subj;
  }

  return dfltSubj;
}


PString XMPP::Message::GetSubject(const PString& lang)
{
  PXMLElement * elem = GetSubjectElement(lang);
  return elem != NULL ? elem->GetData() : PString::Empty();
}


PXMLElement * XMPP::Message::GetBodyElement(const PString& lang)
{
  if (PAssertNULL(m_rootElement) == NULL)
    return NULL;

  PXMLElement * dfltBody = NULL;
  PINDEX i = 0;
  PXMLElement * body;
  PString l;

  while ((body = m_rootElement->GetElement(XMPP::Message::BodyTag(), i++)) != NULL) {
    l = body->GetAttribute(XMPP::LanguageTag());

    if (l == lang)
      return body;
    else if (l.IsEmpty() && dfltBody == NULL)
      dfltBody = body;
  }

  return dfltBody;
}


PString XMPP::Message::GetBody(const PString& lang)
{
  PXMLElement * elem = GetBodyElement(lang);
  return elem != NULL ? elem->GetData() : PString::Empty();
}


PString XMPP::Message::GetThread()
{
  if (PAssertNULL(m_rootElement) == NULL)
    return PString::Empty();

  PXMLElement * elem = m_rootElement->GetElement(XMPP::Message::ThreadTag());
  return elem != NULL ? elem->GetData() : PString::Empty();
}


void XMPP::Message::SetType(MessageType type)
{
  switch (type) {
  case XMPP::Message::Normal:
    SetType("normal");
    break;
  case XMPP::Message::Chat:
    SetType("chat");
    break;
  case XMPP::Message::Error:
    SetType("error");
    break;
  case XMPP::Message::GroupChat:
    SetType("groupchat");
    break;
  case XMPP::Message::HeadLine:
    SetType("headline");
    break;
  default :
    break;
  }
}


void XMPP::Message::SetType(const PString& type)
{
  if (PAssertNULL(m_rootElement) != NULL)
    m_rootElement->SetAttribute(XMPP::Message::TypeTag(), type);
}


void XMPP::Message::SetLanguage(const PString& lang)
{
  if (PAssertNULL(m_rootElement) != NULL)
    m_rootElement->SetAttribute(XMPP::LanguageTag(), lang);
}


void XMPP::Message::SetSubject(const PString& subj, const PString& lang)
{
  if (PAssertNULL(m_rootElement) != NULL)
    return;

  PXMLElement * elem = GetSubjectElement(lang);

  if (elem == NULL) {
    elem = m_rootElement->AddElement(XMPP::Message::SubjectTag());

    if (!lang.IsEmpty())
      elem->SetAttribute(XMPP::LanguageTag(), lang);
  }
  elem->AddData(subj);
}


void XMPP::Message::SetBody(const PString& body, const PString& lang)
{
  if (PAssertNULL(m_rootElement) != NULL)
    return;

  PXMLElement * elem = GetBodyElement(lang);

  if (elem == NULL) {
    elem = m_rootElement->AddElement(XMPP::Message::BodyTag());

    if (!lang.IsEmpty())
      elem->SetAttribute(XMPP::LanguageTag(), lang);
  }

  elem->AddData(body);
}


void XMPP::Message::SetThread(const PString& thrd)
{
  if (PAssertNULL(m_rootElement) != NULL)
    return;

  PXMLElement * elem = m_rootElement->GetElement(XMPP::Message::ThreadTag());

  if (elem == NULL)
    elem = m_rootElement->AddElement(XMPP::Message::ThreadTag());

  elem->AddData(thrd);
}

///////////////////////////////////////////////////////

const PCaselessString & XMPP::Presence::TypeTag()    { static const PConstCaselessString s("type");     return s; }
const PCaselessString & XMPP::Presence::ShowTag()    { static const PConstCaselessString s("show");     return s; }
const PCaselessString & XMPP::Presence::StatusTag()  { static const PConstCaselessString s("status");   return s; }
const PCaselessString & XMPP::Presence::PriorityTag(){ static const PConstCaselessString s("priority"); return s; }

XMPP::Presence::Presence()
{
  SetRootElement(XMPP::PresenceStanzaTag());
  SetID(XMPP::Stanza::GenerateID());
}


XMPP::Presence::Presence(PXMLElement & pdu)
{
  SetRootElement(new PXMLRootElement(*this, pdu));
}


PBoolean XMPP::Presence::IsValid() const
{
  return GetDocumentType() == XMPP::PresenceStanzaTag();
}


XMPP::Presence::PresenceType XMPP::Presence::GetType(PString * typeName) const
{
  if (PAssertNULL(m_rootElement) != NULL)
    return XMPP::Presence::Error;

  PString t = m_rootElement->GetAttribute(XMPP::Presence::TypeTag());

  if (t.IsEmpty()) {
    if (typeName != NULL)
      *typeName = "available";

    return XMPP::Presence::Available;
  }
  else {
    if (typeName != NULL)
      *typeName = t;

    if (t *= "unavailable")
      return XMPP::Presence::Unavailable;
    else if (t *= "subscribe")
      return XMPP::Presence::Subscribe;
    else if (t *= "subscribed")
      return XMPP::Presence::Subscribed;
    else if (t *= "unsubscribe")
      return XMPP::Presence::Unsubscribe;
    else if (t *= "unsubscribed")
      return XMPP::Presence::Unsubscribed;
    else if (t *= "probe")
      return XMPP::Presence::Probe;
    else if (t *= "error")
      return XMPP::Presence::Error;
    else
      return XMPP::Presence::Unknown;
  }
}


XMPP::Presence::ShowType XMPP::Presence::GetShow(PString * showName) const
{
  if (PAssertNULL(m_rootElement) != NULL)
    return XMPP::Presence::Other;

  PXMLElement * elem = m_rootElement->GetElement(XMPP::Presence::ShowTag());

  if (elem == NULL) {
    if (showName != NULL)
      *showName = "online";

    return XMPP::Presence::Online;
  }
  
  PString s = elem->GetData();

  if (s.IsEmpty()) {
    if (showName != NULL)
      *showName = "online";

    return XMPP::Presence::Online;
  }
  else {
    if (showName != NULL)
      *showName = s;

    if (s *= "away")
      return XMPP::Presence::Away;
    else if (s *= "chat")
      return XMPP::Presence::Chat;
    else if (s *= "dnd")
      return XMPP::Presence::DND;
    else if (s *= "xa")
      return XMPP::Presence::XA;
    else
      return XMPP::Presence::Other;
  }
}


BYTE XMPP::Presence::GetPriority() const
{
  if (PAssertNULL(m_rootElement) != NULL)
    return 0;

  PXMLElement * elem = m_rootElement->GetElement(XMPP::Presence::PriorityTag());
  return elem == NULL ? (BYTE)0 : (BYTE)elem->GetData().AsInteger();
}


PXMLElement * XMPP::Presence::GetStatusElement(const PString& lang)
{
  if (PAssertNULL(m_rootElement) == NULL)
    return NULL;

  PXMLElement * dfltStatus = NULL;
  PINDEX i = 0;
  PXMLElement * status;
  PString l;

  while ((status = m_rootElement->GetElement(XMPP::Presence::StatusTag(), i++)) != NULL) {
    l = status->GetAttribute(XMPP::LanguageTag());

    if (l == lang)
      return status;
    else if (l.IsEmpty() && dfltStatus == NULL)
      dfltStatus = status;
  }

  return dfltStatus;
}


PString XMPP::Presence::GetStatus(const PString& lang)
{
  PXMLElement * elem = GetStatusElement(lang);
  return elem != NULL ? elem->GetData() : PString::Empty();
}


void XMPP::Presence::SetType(PresenceType type)
{
  switch (type) {
  case XMPP::Presence::Available:
    if (PAssertNULL(m_rootElement) != NULL)
      m_rootElement->SetAttribute(XMPP::Presence::TypeTag(), PString::Empty());
    break;
  case XMPP::Presence::Unavailable:
    SetType("unavailable");
    break;
  case XMPP::Presence::Subscribe:
    SetType("subscribe");
    break;
  case XMPP::Presence::Subscribed:
    SetType("subscribed");
    break;
  case XMPP::Presence::Unsubscribe:
    SetType("unsubscribe");
    break;
  case XMPP::Presence::Unsubscribed:
    SetType("unsubscribed");
    break;
  case XMPP::Presence::Probe:
    SetType("probe");
    break;
  case XMPP::Presence::Error:
    SetType("error");
    break;
  default :
    break;
  }
}


void XMPP::Presence::SetType(const PString& type)
{
  if (PAssertNULL(m_rootElement) != NULL)
    m_rootElement->SetAttribute(XMPP::Presence::TypeTag(), type);
}


void XMPP::Presence::SetShow(ShowType show)
{
  if (PAssertNULL(m_rootElement) != NULL)
    return;

  switch (show) {
  case XMPP::Presence::Online:
    {
      PXMLElement * elem = m_rootElement->GetElement(XMPP::Presence::ShowTag());
      if (elem)
        m_rootElement->RemoveSubObject(m_rootElement->FindObject(elem));
    }
    break;
  case XMPP::Presence::Away:
    SetType("away");
    break;
  case XMPP::Presence::Chat:
    SetType("chat");
    break;
  case XMPP::Presence::DND:
    SetType("dnd");
    break;
  case XMPP::Presence::XA:
    SetType("xa");
    break;
  default :
    break;
  }
}


void XMPP::Presence::SetShow(const PString& show)
{
  if (PAssertNULL(m_rootElement) != NULL)
    return;

  PXMLElement * elem = m_rootElement->GetElement(XMPP::Presence::ShowTag());

  if (elem == NULL)
    elem = m_rootElement->AddElement(XMPP::Presence::ShowTag());

  elem->AddData(show);
}


void XMPP::Presence::SetPriority(BYTE priority)
{
  if (PAssertNULL(m_rootElement) != NULL)
    return;

  PXMLElement * elem = m_rootElement->GetElement(XMPP::Presence::PriorityTag());

  if (elem == NULL)
    elem = m_rootElement->AddElement(XMPP::Presence::PriorityTag());

  elem->AddData(PString((PINDEX)priority));
}


void XMPP::Presence::SetStatus(const PString& status, const PString& lang)
{
  if (PAssertNULL(m_rootElement) != NULL)
    return;

  PXMLElement * elem = GetStatusElement(lang);

  if (elem == NULL) {
    elem = m_rootElement->AddElement(XMPP::Presence::StatusTag());

    if (!lang.IsEmpty())
      elem->SetAttribute(XMPP::LanguageTag(), lang);
  }
  elem->AddData(status);
}


///////////////////////////////////////////////////////

const PCaselessString & XMPP::IQ::TypeTag() { static const PConstCaselessString s("type"); return s; }


XMPP::IQ::IQ(XMPP::IQ::IQType type)
  : m_Processed(false),
    m_OriginalIQ(NULL)
{
  SetRootElement(XMPP::IQStanzaTag())->SetAttribute(XMPP::NamespaceTag(), "jabber:client");
  SetType(type);
  SetID(XMPP::Stanza::GenerateID());
}


XMPP::IQ::IQ(PXMLElement & pdu)
  : m_Processed(false),
    m_OriginalIQ(NULL)
{
  SetRootElement(new PXMLRootElement(*this, pdu));
}


XMPP::IQ::~IQ()
{
  delete m_OriginalIQ;
}


PBoolean XMPP::IQ::IsValid() const
{
  if (GetDocumentType() != XMPP::IQStanzaTag())
    return false;

  PString s = GetRootElement()->GetAttribute(XMPP::IQ::TypeTag());

  if (s.IsEmpty() || (s != "get" && s != "set" && s != "result" && s != "error"))
    return false;

  /* Appartently when a server sends a set to us there's no id...
  s = elem->GetAttribute(XMPP::IQ::ID);
  return !s.IsEmpty();
  */
  return true;
}


XMPP::IQ::IQType XMPP::IQ::GetType(PString * typeName) const
{
  if (PAssertNULL(m_rootElement) != NULL)
    return XMPP::IQ::Unknown;

  PString t = m_rootElement->GetAttribute(XMPP::IQ::TypeTag());

  if (typeName != NULL)
    *typeName = t;

  if (t *= "get")
    return XMPP::IQ::Get;
  else if (t *= "set")
    return XMPP::IQ::Set;
  else if (t *= "result")
    return XMPP::IQ::Result;
  else if (t *= "error")
    return XMPP::IQ::Error;
  else
    return XMPP::IQ::Unknown;
}


PXMLElement * XMPP::IQ::GetBody()
{
  if (PAssertNULL(m_rootElement) != NULL)
    return NULL;

  return GetElement(0);
}


void XMPP::IQ::SetType(XMPP::IQ::IQType type)
{
  switch (type) {
  case XMPP::IQ::Get:
    SetType("get");
    break;
  case XMPP::IQ::Set:
    SetType("set");
    break;
  case XMPP::IQ::Result:
    SetType("result");
    break;
  case XMPP::IQ::Error:
    SetType("error");
    break;
  default :
    break;
  }
}


void XMPP::IQ::SetType(const PString& type)
{
  if (PAssertNULL(m_rootElement) != NULL)
    m_rootElement->SetAttribute(XMPP::IQ::TypeTag(), type);
}


void XMPP::IQ::SetBody(PXMLElement * body)
{
  if (PAssertNULL(m_rootElement) == NULL)
    return;

  while (m_rootElement->HasSubObjects())
    m_rootElement->RemoveSubObject(0);

  if (body != NULL)
    m_rootElement->AddSubObject(body);
}


void XMPP::IQ::SetOriginalMessage(IQ * iq)
{
  delete m_OriginalIQ;
  m_OriginalIQ = iq;
}


XMPP::IQ * XMPP::IQ::BuildResult() const
{
  IQType iq_type = GetType();

  if (iq_type != XMPP::IQ::Get && iq_type != XMPP::IQ::Set)
    return NULL;

  IQ * result = new IQ(XMPP::IQ::Result);
  result->SetID(GetID());
  result->SetTo(GetFrom());
  return result;
}


XMPP::IQ * XMPP::IQ::BuildError(const PString& type, const PString& code) const
{
  if (PAssertNULL(m_rootElement) != NULL)
    return NULL;

  IQType iq_type = GetType();

  if (iq_type != XMPP::IQ::Get && iq_type != XMPP::IQ::Set)
    return NULL;

  IQ * error = new IQ(XMPP::IQ::Error);
  error->SetID(GetID());
  error->SetTo(GetFrom());

  PXMLElement * body = error->GetRootElement()->AddElement("error");
  body->SetAttribute("type", type);
  PXMLElement * codeElem = body->AddElement(code);
  codeElem->SetAttribute(XMPP::NamespaceTag(), "urn:ietf:params:xml:ns:xmpp-stanzas");

  const PXMLElement * originalBody = m_rootElement->GetElement();
  if (originalBody != NULL)
    error->GetRootElement()->AddSubObject(dynamic_cast<PXMLObject *>(originalBody->Clone()));

  return error;
}

///////////////////////////////////////////////////////

XMPP::Disco::Item::Item(PXMLElement * item)
  : m_JID(item != 0 ? item->GetAttribute("jid") : PString::Empty()),
    m_Node(item != 0 ? item->GetAttribute("node") : PString::Empty())
{}


XMPP::Disco::Item::Item(const PString& jid, const PString& node)
  : m_JID(jid), m_Node(node)
{}


PXMLElement * XMPP::Disco::Item::AsXML(PXMLElement * parent) const
{
  if (parent == 0)
    return 0;

  PXMLElement * item = parent->AddElement("item");
  item->SetAttribute("jid", m_JID);
  if (!m_Node.IsEmpty())
    item->SetAttribute("node", m_Node);
  return item;
}


XMPP::Disco::ItemList::ItemList(PXMLElement * list)
{
  if (list == 0)
    return;

  PINDEX i = 0;
  PXMLElement * item;

  while ((item = list->GetElement("item", i++)) != 0)
    Append(new Item(item));
}


PXMLElement * XMPP::Disco::ItemList::AsXML(PXMLElement * parent) const
{
  if (parent == 0)
    return 0;

  PXMLElement * items = parent->AddElement("query");

  items->SetAttribute(XMPP::NamespaceTag(), "http://jabber.org/protocol/disco#items");

  for (const_iterator i = begin(); i != end(); i++)
    i->AsXML(items);

  return items;
}


XMPP::Disco::Identity::Identity(PXMLElement * identity)
  : m_Category(identity != 0 ? identity->GetAttribute("category") : PString::Empty()),
    m_Type(identity != 0 ? identity->GetAttribute("type") : PString::Empty()),
    m_Name(identity != 0 ? identity->GetAttribute("name") : PString::Empty())
{
}


XMPP::Disco::Identity::Identity(const PString& category, const PString& type, const PString& name)
  : m_Category(category), m_Type(type), m_Name(name)
{
}


PXMLElement * XMPP::Disco::Identity::AsXML(PXMLElement * parent) const
{
  if (parent == 0)
    return 0;

  PXMLElement * identity = parent->AddElement("identity");

  if (!m_Category.IsEmpty())
    identity->SetAttribute("category", m_Category);
  if (!m_Type.IsEmpty())
    identity->SetAttribute("type", m_Type);
  if (!m_Name.IsEmpty())
    identity->SetAttribute("name", m_Name);
  return identity;
}


XMPP::Disco::IdentityList::IdentityList(PXMLElement * list)
{
  if (list == 0)
    return;

  PINDEX i = 0;
  PXMLElement * identity;

  while ((identity = list->GetElement("identity", i++)) != 0)
    Append(new Identity(identity));
}


PXMLElement * XMPP::Disco::IdentityList::AsXML(PXMLElement * parent) const
{
  if (parent == 0)
    return 0;

  // Identity lists normally come as part of a full info, which we
  // assume here it's the parent
  for (const_iterator i = begin(); i != end(); i++)
    i->AsXML(parent);

  return parent;
}


XMPP::Disco::Info::Info(PXMLElement * info)
{
  if (info == 0)
    return;

  m_Identities = IdentityList(info);

  PINDEX i = 0;
  PXMLElement * feature;
  PString var;

  while ((feature = info->GetElement("feature", i++)) != 0) {
    var = feature->GetAttribute("var");
    if (!var.IsEmpty())
      m_Features.Include(var);
  }
}


PXMLElement * XMPP::Disco::Info::AsXML(PXMLElement * parent) const
{
  if (parent == 0)
    return 0;

  PXMLElement * info = parent->AddElement("query");

  info->SetAttribute(XMPP::NamespaceTag(), "http://jabber.org/protocol/disco#info");

  m_Identities.AsXML(info);

  for (PStringSet::const_iterator it = m_Features.begin(); it != m_Features.end(); ++it) {
    PXMLElement * feature = info->AddElement("feature");
    feature->SetAttribute("var", *it);
  }

  return info;
}

#endif // P_EXPAT

// End of File ///////////////////////////////////////////////////////////////



