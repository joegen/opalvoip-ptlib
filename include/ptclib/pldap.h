/*
 * pldap.h
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
 * $Log: pldap.h,v $
 * Revision 1.1  2003/03/28 01:15:44  robertj
 * OpenLDAP support.
 *
 *
 */

#ifndef _PLDAP_H
#define _PLDAP_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


struct ldap;


/**This class will create an LDAP client to access a remote LDAP server.
  */
class PLDAPSession : public PObject
{
  PCLASSINFO(PLDAPSession, PObject)
  public:
    /**Create a LDAP client.
      */
    PLDAPSession();

    /**Close the sesison on destruction
      */
    ~PLDAPSession();

    /**Open the LDAP session to the specified server
      */
    BOOL Open(
      const PString & server,
      WORD port = 0
    );

    /**Close the LDAP session
      */
    BOOL Close();

    /**Determine of session is open.
      */
    BOOL IsOpen() const { return ldapSession != NULL; }

    /**Set LDAP option parameter (OpenLDAp specific values)
      */
    BOOL SetOption(
      int optcode,
      int value
    );

    /**Set LDAP option parameter (OpenLDAP specific values)
      */
    BOOL SetOption(
      int optcode,
      void * value
    );

    enum AuthenticationMethod {
      AuthSimple,
      AuthSASL,
      AuthKerberos
    };

    /**Bind to the remote LDAP server.
      */
    BOOL Bind(
      const PString & who = PString::Empty(),
      const PString & passwd = PString::Empty(),
      AuthenticationMethod authMethod = AuthSimple
    );

    enum SearchScope {
      ScopeBaseOnly,
      ScopeSingleLevel,
      ScopeSubTree,
      NumSearchScope
    };

    /**Search for specified information.
       Every match is returned.
      */
    PList<PStringToString> SearchAll(
      const PString & filter,
      const PStringArray & attributes = PStringList(),
      const PString & base = PString::Empty(),
      SearchScope scope = ScopeSubTree,
      const PTimeInterval & timeout = PMaxTimeInterval
    );


  protected:
    struct ldap * ldapSession;
    unsigned      protocolVersion;
};


#endif // _PLDAP_H


// End of file ////////////////////////////////////////////////////////////////
