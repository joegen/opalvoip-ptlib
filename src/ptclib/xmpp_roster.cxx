/*
 * xmpp_roster.cxx
 *
 * Extensible Messaging and Presence Protocol (XMPP) IM
 * Roster management classes
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
 * $Log: xmpp_roster.cxx,v $
 * Revision 1.1  2004/04/26 01:51:58  rjongbloed
 * More implementation of XMPP, thanks a lot to Federico Pinna & Reitek S.p.A.
 *
 *
 */

#ifdef __GNUC__
#pragma implementation "xmpp_roster.h"
#endif

#include <ptlib.h>
#include <ptclib/xmpp_roster.h>

#if P_EXPAT

XMPP::Roster::Item::Item(PXMLElement * item)
  : m_IsDirty(FALSE)
{
  if (item != NULL)
    operator=(*item);
}


XMPP::Roster::Item::Item(PXMLElement& item)
  : m_IsDirty(FALSE)
{
  operator=(item);
}


XMPP::Roster::Item::Item(const JID& jid, ItemType type, const PString& group, const PString& name)
  : m_JID(jid),
    m_IsDirty(TRUE)
{
  SetType(type);
  AddGroup(group);
  SetName(name.IsEmpty() ? m_JID.GetUser() : name);
}


void XMPP::Roster::Item::AddGroup(const PString& group, BOOL dirty)
{
  if (group.IsEmpty())
    return;

  if (!m_Groups.Contains(group) && dirty)
    SetDirty();

  m_Groups.Include(group);
}


void XMPP::Roster::Item::RemoveGroup(const PString& group, BOOL dirty)
{
  if (m_Groups.Contains(group) && dirty)
    SetDirty();

  m_Groups.Exclude(group);
}


XMPP::Roster::Item& XMPP::Roster::Item::operator=(const PXMLElement& item)
{
  SetJID(item.GetAttribute("jid"));
  SetName(item.GetAttribute("name"));
  if (m_Name.IsEmpty())
    SetName(m_JID.GetUser());

  PCaselessString type = item.GetAttribute("subscription");

  if (type.IsEmpty() || type == "none")
    SetType(XMPP::Roster::None);
  else if (type == "to")
    SetType(XMPP::Roster::To);
  else if (type == "from")
    SetType(XMPP::Roster::From);
  else if (type == "both")
    SetType(XMPP::Roster::Both);
  else
    SetType(XMPP::Roster::Unknown);

  PINDEX i = 0;
  PXMLElement * group;

  while ((group = item.GetElement("group", i)) != 0) {
    i = item.FindObject(group) + 1;
    AddGroup(group->GetData());
  }

  return *this;
}


PXMLElement * XMPP::Roster::Item::AsXML(PXMLElement * parent) const
{
  if (parent == NULL)
    return NULL;

  PXMLElement * item = parent->AddChild(new PXMLElement(parent, "item"));
  item->SetAttribute("jid", GetJID());
  item->SetAttribute("name", GetName());

  PString s;

  switch (m_Type) {
    case XMPP::Roster::None:
      s = "none";
      break;
    case XMPP::Roster::To:
      s = "to";
      break;
    case XMPP::Roster::From:
      s = "from";
      break;
    case XMPP::Roster::Both:
      s = "both";
      break;
  }

  if (!s.IsEmpty())
    item->SetAttribute("subscrition", s);

  for (PINDEX i = 0, max = m_Groups.GetSize() ; i < max ; i++) {
    PXMLElement * group = item->AddChild(new PXMLElement(item, "group"));
    group->AddChild(new PXMLData(group, m_Groups.GetKeyAt(i)));
  }

  return item;
}

///////////////////////////////////////////////////////

XMPP::Roster::Roster(XMPP::C2S::StreamHandler * handler)
  : m_Handler(NULL)
{
  if (handler != NULL)
    Attach(handler);
}


XMPP::Roster::~Roster()
{
}


XMPP::Roster::Item * XMPP::Roster::FindItem(const PString& jid)
{
  for (PINDEX i = 0, max = m_Items.GetSize() ; i < max ; i++) {
    if (m_Items[i].GetJID() == jid)
      return &(m_Items[i]);
  }

  return NULL;
}


BOOL XMPP::Roster::SetItem(Item * item, BOOL localOnly)
{
  if (item == NULL)
    return FALSE;

  if (localOnly) {
    Item * existingItem = FindItem(item->GetJID());

    if (existingItem != NULL)
      m_Items.Remove(existingItem);

    return m_Items.Append(item);
  }

  PXMLElement * query = new PXMLElement(0, XMPP::IQQuery);
  query->SetAttribute(XMPP::Namespace, "jabber:iq:roster");
  item->AsXML(query);

  XMPP::IQ iq(XMPP::IQ::Set, query);
  return m_Handler->Write(iq);
}


BOOL XMPP::Roster::RemoveItem(const PString& jid, BOOL localOnly)
{
  Item * item = FindItem(jid);

  if (item == NULL)
    return FALSE;

  if (localOnly) {
    m_Items.Remove(item);
    return TRUE;
  }

  PXMLElement * query = new PXMLElement(0, XMPP::IQQuery);
  query->SetAttribute(XMPP::Namespace, "jabber:iq:roster");
  PXMLElement * _item = item->AsXML(query);
  _item->SetAttribute("subscription", "remove");

  XMPP::IQ iq(XMPP::IQ::Set, query);
  return m_Handler->Write(iq);
}


BOOL XMPP::Roster::RemoveItem(Item * item, BOOL localOnly)
{
  if (item == NULL)
    return FALSE;

  return RemoveItem(item->GetJID(), localOnly);
}


void XMPP::Roster::Attach(XMPP::C2S::StreamHandler * handler)
{
  if (m_Handler != NULL)
    Detach();

  if (handler == NULL)
    return;

  m_Handler = handler;
  m_Handler->SessionEstablishedHandlers().Add(new PCREATE_NOTIFIER(OnSessionEstablished));
  m_Handler->SessionReleasedHandlers().Add(new PCREATE_NOTIFIER(OnSessionReleased));
  m_Handler->IQQueryHandlers("jabber:iq:roster").Add(new PCREATE_NOTIFIER(OnIQ));
}


void XMPP::Roster::Detach()
{
  m_Items.RemoveAll();

  if (m_Handler != NULL) {
    m_Handler->SessionEstablishedHandlers().RemoveTarget(this);
    m_Handler->SessionReleasedHandlers().RemoveTarget(this);
    m_Handler->IQQueryHandlers("jabber:iq:roster").RemoveTarget(this);
    m_Handler = 0;
  }
}


void XMPP::Roster::OnSessionEstablished(XMPP::C2S::StreamHandler&, INT)
{
  if (m_Handler == NULL)
    return;

  XMPP::Presence pre;
  m_Handler->Write(pre);

  PXMLElement * query = new PXMLElement(0, XMPP::IQQuery);
  query->SetAttribute(XMPP::Namespace, "jabber:iq:roster");
  XMPP::IQ iq(XMPP::IQ::Get, query);

  m_Handler->Write(iq);
}


void XMPP::Roster::OnSessionReleased(XMPP::C2S::StreamHandler&, INT)
{
  Detach();
}


void XMPP::Roster::OnIQ(XMPP::IQ& iq, INT)
{
  PXMLElement * query = iq.GetElement(XMPP::IQQuery);

  PAssertNULL(query);

  PINDEX i = 0;
  PXMLElement * item;

  while ((item = query->GetElement("item", i)) != 0) {
    i = query->FindObject(item) + 1;

    if (item->GetAttribute("subscription") == "remove")
      RemoveItem(item->GetAttribute("jid"), TRUE);
    else
      SetItem(new XMPP::Roster::Item(item), TRUE);
  }

  if (iq.GetType() == XMPP::IQ::Set) {
    iq.SetProcessed();
    
    if (!iq.GetID().IsEmpty())
      m_Handler->Send(iq.BuildResult());
  }
}


#endif // P_EXPAT


// End of File ///////////////////////////////////////////////////////////////


