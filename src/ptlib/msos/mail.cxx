/*
 * $Id: mail.cxx,v 1.2 1995/04/01 08:05:04 robertj Exp $
 *
 * Portable Windows Library
 *
 * Mail Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: mail.cxx,v $
 * Revision 1.2  1995/04/01 08:05:04  robertj
 * Added GUI support.
 *
 * Revision 1.1  1995/03/14 12:45:14  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <mail.h>

#include <xcmcext.h>
#include <xcmcmsxt.h>



//////////////////////////////////////////////////////////////////////////////
// PMailSession

PMailSession::PMailSession()
{
  Construct();
}


PMailSession::PMailSession(const PString & username, const PString & password)
{
  Construct();
  LogOn(username, password);
}


PMailSession::PMailSession(const PString & username,
                             const PString & password, const PString & service)
{
  Construct();
  LogOn(username, password, service);
}


PMailSession::~PMailSession()
{
  LogOff();
}


void PMailSession::Construct()
{
  loggedOn = FALSE;
  logOffHWND = NULL;
}


BOOL PMailSession::LogOn(const PString & username, const PString & password)
{
  return LogOn(username, password, PString());
}


BOOL PMailSession::LogOn(const PString & username,
                             const PString & password, const PString & service)
{
  return CMCLogOn(service, username, password, NULL);
}


BOOL PMailSession::CMCLogOn(const char * service,
                 const char * username, const char * password, CMC_ui_id ui_id)
{
  if (!cmc.IsLoaded()) {
    lastError = CMC_E_FAILURE;
    return FALSE;
  }

  if (!LogOff())
    return FALSE;
 
  logOffHWND = ui_id;

  CMC_X_COM_support support[2];
  support[0].item_code = CMC_XS_COM;
  support[0].flags = 0;
  support[1].item_code = CMC_XS_MS;
  support[1].flags = 0;
  CMC_extension extension;
  extension.item_code = CMC_X_COM_SUPPORT_EXT;
  extension.item_data = PARRAYSIZE(support);
  extension.item_reference = support;
  extension.extension_flags = CMC_EXT_LAST_ELEMENT;
  if ((lastError = cmc.logon((CMC_string)service,
                             (CMC_string)username,
                             (CMC_string)password,
                             NULL,
                             ui_id,
                             100,
                             ui_id == NULL ? 0 :
                                 (CMC_LOGON_UI_ALLOWED | CMC_ERROR_UI_ALLOWED),
                             &sessionId,
                             &extension)) != CMC_SUCCESS)
    return FALSE;

  loggedOn = TRUE;
  return TRUE;
}


BOOL PMailSession::LogOff()
{
  if (!cmc.IsLoaded()) {
    lastError = CMC_E_FAILURE;
    return FALSE;
  }

  if (!loggedOn)
    return TRUE;

  lastError = cmc.logoff(sessionId,
           logOffHWND,
           logOffHWND == NULL ? 0
                              : (CMC_LOGOFF_UI_ALLOWED | CMC_ERROR_UI_ALLOWED),
           NULL);

  switch (lastError) {
    case CMC_SUCCESS :
    case CMC_E_INVALID_SESSION_ID :
    case CMC_E_USER_NOT_LOGGED_ON :
      loggedOn = FALSE;
      return lastError == CMC_SUCCESS;
  }

  return FALSE;
}


BOOL PMailSession::IsLoggedOn() const
{
  return loggedOn;
}


BOOL PMailSession::SendNote(const PString & recipient,
                            const PString & subject,
                            const char * body)
{
  PStringList recipients;
  recipients[0] = recipient;
  return SendNote(recipients, subject, body);
}


BOOL PMailSession::SendNote(const PStringList & recipients,
                            const PString & subject,
                            const char * body)
{
  CMC_message msg;
  memset(&msg, 0, sizeof(msg));

  msg.recipients = new CMC_recipient[recipients.GetSize()];
  memset(msg.recipients, 0, recipients.GetSize()*sizeof(CMC_recipient));

  for (PINDEX i = 0 ; i < recipients.GetSize(); i++) {
    msg.recipients[i].role = i == 0 ? CMC_ROLE_TO : CMC_ROLE_CC;
    msg.recipients[i].name = (CMC_string)(const char *)recipients[i];
  }
  msg.recipients[recipients.GetSize()-1].recip_flags = CMC_RECIP_LAST_ELEMENT;

  msg.subject = (CMC_string)(const char *)subject;
  msg.text_note = (CMC_string)body;
  msg.message_flags = CMC_MSG_LAST_ELEMENT;

  lastError = cmc.send(sessionId, &msg, 0, NULL, NULL);

  delete [] msg.recipients;

  return lastError == CMC_SUCCESS;
}


PMailSession::LookUpResult
                 PMailSession::LookUp(const PString & name, PString * fullName)
{
  CMC_recipient recip_in;
  memset(&recip_in, 0, sizeof(recip_in));
  recip_in.name = (CMC_string)(const char *)name;
  recip_in.recip_flags = CMC_RECIP_LAST_ELEMENT;

  CMC_recipient * recip_out;
  CMC_uint32 count = 1;
  lastError = cmc.look_up(sessionId, &recip_in, 
                  CMC_LOOKUP_RESOLVE_IDENTITY, NULL, &count, &recip_out, NULL);

  switch (lastError) {
    case CMC_SUCCESS :
      if (fullName != NULL)
        *fullName = recip_out->name;
      cmc.free(recip_out);
      return ValidUser;

    case CMC_E_AMBIGUOUS_RECIPIENT :
      return AmbiguousUser;

    case CMC_E_RECIPIENT_NOT_FOUND :
      return UnknownUser;
  }
  
  return LookUpError;
}


int PMailSession::GetErrorCode() const
{
  return (int)lastError;
}


PString PMailSession::GetErrorText() const
{
  static const char * errMsg[] = {
    "CMC_SUCCESS",
    "CMC_E_AMBIGUOUS_RECIPIENT",
    "CMC_E_ATTACHMENT_NOT_FOUND",
    "CMC_E_ATTACHMENT_OPEN_FAILURE",
    "CMC_E_ATTACHMENT_READ_FAILURE",
    "CMC_E_ATTACHMENT_WRITE_FAILURE",
    "CMC_E_COUNTED_STRING_UNSUPPORTED",
    "CMC_E_DISK_FULL",
    "CMC_E_FAILURE",
    "CMC_E_INSUFFICIENT_MEMORY",
    "CMC_E_INVALID_CONFIGURATION",
    "CMC_E_INVALID_ENUM",
    "CMC_E_INVALID_FLAG",
    "CMC_E_INVALID_MEMORY",
    "CMC_E_INVALID_MESSAGE_PARAMETER",
    "CMC_E_INVALID_MESSAGE_REFERENCE",
    "CMC_E_INVALID_PARAMETER",
    "CMC_E_INVALID_SESSION_ID",
    "CMC_E_INVALID_UI_ID",
    "CMC_E_LOGON_FAILURE",
    "CMC_E_MESSAGE_IN_USE",
    "CMC_E_NOT_SUPPORTED",
    "CMC_E_PASSWORD_REQUIRED",
    "CMC_E_RECIPIENT_NOT_FOUND",
    "CMC_E_SERVICE_UNAVAILABLE",
    "CMC_E_TEXT_TOO_LARGE",
    "CMC_E_TOO_MANY_FILES",
    "CMC_E_TOO_MANY_RECIPIENTS",
    "CMC_E_UNABLE_TO_NOT_MARK_AS_READ",
    "CMC_E_UNRECOGNIZED_MESSAGE_TYPE",
    "CMC_E_UNSUPPORTED_ACTION",
    "CMC_E_UNSUPPORTED_CHARACTER_SET",
    "CMC_E_UNSUPPORTED_DATA_EXT",
    "CMC_E_UNSUPPORTED_FLAG",
    "CMC_E_UNSUPPORTED_FUNCTION_EXT",
    "CMC_E_UNSUPPORTED_VERSION",
    "CMC_E_USER_CANCEL",
    "CMC_E_USER_NOT_LOGGED_ON"
  };

  if (lastError < PARRAYSIZE(errMsg))
    return errMsg[lastError];
  return PString(PString::Printf, "CMC Error=%lu", lastError);
}


PMailSession::CMCDLL::CMCDLL()
  : PDynaLink("CMC.DLL")
{
  if (!GetFunction("cmc_logon", (Function &)logon) ||
      !GetFunction("cmc_logoff", (Function &)logoff) ||
      !GetFunction("cmc_free", (Function &)free) ||
      !GetFunction("cmc_query_configuration", (Function &)query_configuration) ||
      !GetFunction("cmc_look_up", (Function &)look_up) ||
      !GetFunction("cmc_list", (Function &)list) ||
      !GetFunction("cmc_send", (Function &)send) ||
      !GetFunction("cmc_read", (Function &)read) ||
      !GetFunction("cmc_act_on", (Function &)act_on))
    Close();
}


// End Of File ///////////////////////////////////////////////////////////////
