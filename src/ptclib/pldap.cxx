/*
 * pldap.cxx
 *
 * Lightweight Directory Access Protocol interface class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2003 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Id: pldap.cxx,v 1.2 2003/03/28 05:16:11 robertj Exp $
 */

#ifdef __GNUC__
#pragma implementation "pldap.h"
#endif

#include <ptlib.h>

#include <ptlib/sockets.h>
#include <ptclib/pldap.h>

#if P_LDAP

#include <ldap.h>


///////////////////////////////////////////////////////////////////////////////

PLDAPSession::PLDAPSession()
{
  ldapSession = NULL;
  protocolVersion = LDAP_VERSION3;
}


PLDAPSession::~PLDAPSession()
{
  Close();
}


BOOL PLDAPSession::Open(const PString & server, WORD port)
{
  Close();

  ldapSession = ldap_init(server, port);
  if (ldapSession == NULL)
    return FALSE;

  SetOption(LDAP_OPT_PROTOCOL_VERSION, protocolVersion);
  return TRUE;
}


BOOL PLDAPSession::Close()
{
  if (ldapSession == NULL)
    return FALSE;

  ldap_unbind(ldapSession);
  ldapSession = NULL;
  return TRUE;
}


BOOL PLDAPSession::SetOption(int optcode, int value)
{
  if (ldapSession == NULL)
    return FALSE;

  return ldap_set_option(ldapSession, optcode, &value);
}


BOOL PLDAPSession::SetOption(int optcode, void * value)
{
  if (ldapSession == NULL)
    return FALSE;

  return ldap_set_option(ldapSession, optcode, value);
}


BOOL PLDAPSession::Bind(const PString & who,
                        const PString & passwd,
                        AuthenticationMethod authMethod)
{
  if (ldapSession == NULL)
    return FALSE;

  const char * whoPtr;
  if (who.IsEmpty())
    whoPtr = NULL;
  else
    whoPtr = who;
  return ldap_bind_s(ldapSession, whoPtr, passwd, authMethod) == LDAP_SUCCESS;
}


PList<PStringToString> PLDAPSession::SearchAll(const PString & filter,
                                               const PStringArray & attributes,
                                               const PString & base,
                                               SearchScope scope,
                                               const PTimeInterval & timeout)
{
  PList<PStringToString> data;

  if (ldapSession == NULL)
    return data;

  PCharArray storage;
  char ** attribs = attributes.ToCharArray(&storage);

  static const int ScopeCode[NumSearchScope] = {
    LDAP_SCOPE_BASE, LDAP_SCOPE_ONELEVEL, LDAP_SCOPE_SUBTREE
  };

  if (ldap_search(ldapSession, base, ScopeCode[scope], filter, attribs, FALSE) < 0)
    return data;

  LDAPMessage * result;
  P_timeval tval = timeout;
  while (ldap_result(ldapSession, LDAP_RES_ANY, LDAP_MSG_ONE, NULL, &result) > 0) {

    LDAPMessage * msg = ldap_first_message(ldapSession, result);
    while (msg != NULL) {

      switch (ldap_msgtype(msg)) {
        case LDAP_RES_SEARCH_ENTRY :
        {
        PStringToString * entry = new PStringToString;

	char * dn = ldap_get_dn(ldapSession, msg);
        entry->SetAt("dn", dn);
        ldap_memfree(dn);

	BerElement * ber = NULL;
        char * attrib = ldap_first_attribute(ldapSession, msg, &ber);
        while (attrib != NULL) {

          struct berval ** bvals = ldap_get_values_len(ldapSession, msg, attrib);
          if (bvals != NULL) {
            PString value = (*entry)(attrib);

            for (PINDEX i = 0; bvals[i] != NULL; i++ ) {
              if (!value)
                value += '\n';
              value += PString(bvals[i]->bv_val, bvals[i]->bv_len);
            }
            ber_bvecfree(bvals);

            entry->SetAt(attrib, value);
          }

          ldap_memfree(attrib);
          attrib = ldap_next_attribute(ldapSession, msg, ber);
        }

        data.Append(entry);
        }
        break;

        case LDAP_RES_SEARCH_RESULT :
          return data;
      }

      msg = ldap_next_message(ldapSession, msg);
    }

    ldap_msgfree(result);
  }

  return data;
}


#endif // P_LDAP


// End of file ////////////////////////////////////////////////////////////////
