/*
 * $Id: mail.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Mail Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: mail.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <mail.h>


//////////////////////////////////////////////////////////////////////////////
// PMail

PMail::PMail()
{
  Construct();
}


PMail::PMail(const PString & username, const PString & password)
{
  Construct();
}


PMail::PMail(const PString & username,
             const PString & password,
             const PString & service)
{
  Construct();
}


PMail::~PMail()
{
  LogOff();
}


void PMail::Construct()
{
  loggedOn = FALSE;
}


BOOL PMail::LogOn(const PString & username, const PString & password)
{
  return FALSE;
}


BOOL PMail::LogOn(const PString & username,
                  const PString & password,
                  const PString & service)
{
  return FALSE;
}


BOOL PMail::LogOff()
{
  if (!loggedOn)
    return TRUE;

  lastError = 1;
  return FALSE;
}


BOOL PMail::IsLoggedOn() const
{
  return loggedOn;
}


BOOL PMail::SendNote(const PString & recipient,
                     const PString & subject,
                     const char * body)
{
  PStringList recipients;
  recipients.AppendString(recipient);
  return SendNote(recipients, subject, body);
}


BOOL PMail::SendNote(const PStringList & recipients,
                     const PString & subject,
                     const char * body)
{
  lastError = 1;
  return FALSE;
}


PStringArray PMail::GetMessageIDs(BOOL unreadOnly)
{
  PStringArray msgIDs;

  lastError = 1;
  return msgIDs;
}


BOOL PMail::GetMessageHeader(const PString & id,
                             Header & hdrInfo)
{
  lastError = 1;
  return FALSE;
}


BOOL PMail::GetMessageBody(const PString & id, PString & body, BOOL markAsRead)
{
  lastError = 1;
  return FALSE;
}


BOOL PMail::GetMessageAttachments(const PString & id,
                                  PStringArray & filenames,
                                  BOOL includeBody,
                                  BOOL markAsRead)
{
  filenames.SetSize(0);

  lastError = 1;
  return FALSE;
}


BOOL PMail::MarkMessageRead(const PString & id)
{
  lastError = 1;
  return FALSE;
}


BOOL PMail::DeleteMessage(const PString & id)
{
  lastError = 1;
  return FALSE;
}


PMail::LookUpResult PMail::LookUp(const PString & name, PString * fullName)
{
  lastError = 1;
  return LookUpError;
}


int PMail::GetErrorCode() const
{
  return (int)lastError;
}


PString PMail::GetErrorText() const
{
  return "No mail library loaded.";
}


//////////////////////////////////////////////////////////////////////////////
// PMail

PMailGUI::PMailGUI(PInteractor * parent)
{
  owner = parent;
  if (parent != NULL)
    LogOnGUI(parent);
}


PMailGUI::~PMailGUI()
{
  LogOff();
}


BOOL PMailGUI::LogOnGUI(PInteractor * parent)
{
  if (parent != NULL)
    owner = parent;
  if (owner == NULL)
    owner = PApplication::Current()->GetWindow();

  return FALSE;
}


BOOL PMailGUI::SendGUI()
{
  lastError = 1;
  return FALSE;
}


// End Of File ///////////////////////////////////////////////////////////////
