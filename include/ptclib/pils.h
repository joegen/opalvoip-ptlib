/*
 * pils.h
 *
 * Microsoft Internet Location Server Protocol interface class.
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
 * $Log: pils.h,v $
 * Revision 1.1  2003/03/31 03:35:20  robertj
 * Major addition of LDAP functionality.
 * Added ILS specialisation of LDAP.
 *
 */

#ifndef _PILS_H
#define _PILS_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptlib/sockets.h>
#include <ptclib/pldap.h>


 /**This class will create an LDAP client to access a remote ILS server.
  */
class PILSSession : public PLDAPSession
{
  PCLASSINFO(PILSSession, PLDAPSession)
  public:
    /**Create an ILS client.
      */
    PILSSession();

    PLDAP_STRUCT_BEGIN(RTPerson)
       PLDAP_ATTR_SIMP(RTPerson, PString,     cn); // Must be non-empty
       PLDAP_ATTR_SIMP(RTPerson, PString,     c);
       PLDAP_ATTR_SIMP(RTPerson, PString,     o);
       PLDAP_ATTR_SIMP(RTPerson, PString,     surname);
       PLDAP_ATTR_SIMP(RTPerson, PString,     givenName);
       PLDAP_ATTR_SIMP(RTPerson, PString,     rfc822Mailbox); // Must be non-empty
       PLDAP_ATTR_SIMP(RTPerson, PString,     location);
       PLDAP_ATTR_SIMP(RTPerson, PString,     comment);
       PLDAP_ATTR_SIMP(RTPerson, PIPSocket::Address, sipAddress);
       PLDAP_ATTR_SIMP(RTPerson, PWORDArray,  sport);
       PLDAP_ATTR_INIT(RTPerson, unsigned,    sflags,       0);
       PLDAP_ATTR_INIT(RTPerson, unsigned,    ssecurity,    0);
       PLDAP_ATTR_INIT(RTPerson, unsigned,    smodop,       0);
       PLDAP_ATTR_INIT(RTPerson, unsigned,    sttl,         3600);
       PLDAP_ATTR_SIMP(RTPerson, PStringList, sprotid);
       PLDAP_ATTR_SIMP(RTPerson, PStringList, sprotmimetype);
       PLDAP_ATTR_INIT(RTPerson, PString,     sappid,       PProcess::Current().GetName()); // Must be non-empty
       PLDAP_ATTR_INIT(RTPerson, PString,     sappguid,     "none"); // Must be non-empty
       PLDAP_ATTR_SIMP(RTPerson, PStringList, smimetype);
       PLDAP_ATTR_INIT(RTPerson, BOOL,        ilsa32833566, 0); // 1=audio capable
       PLDAP_ATTR_INIT(RTPerson, BOOL,        ilsa32964638, 0); // 1=video capable
       PLDAP_ATTR_INIT(RTPerson, BOOL,        ilsa26214430, 0); // 1=in a call
       PLDAP_ATTR_INIT(RTPerson, unsigned,    ilsa26279966, 0); // unknown
       PLDAP_ATTR_INIT(RTPerson, unsigned,    ilsa39321630, 0); // 1 personal; 2 business user; 4 adults-only
       PLDAP_ATTR_INIT(RTPerson, time_t,      timestamp,    PTime().GetTimeInSeconds());

      public:
       PString GetDN() const;

    PLDAP_STRUCT_END();

    BOOL AddPerson(
      const RTPerson & person
    );

    BOOL ModifyPerson(
      const RTPerson & person
    );

    BOOL DeletePerson(
      const RTPerson & person
    );

    BOOL SearchPerson(
      const PString & canonicalName,
      RTPerson & person
    );

    PList<RTPerson> SearchPeople(
      const PString & filter
    );
};


#endif // _PILS_H


// End of file ////////////////////////////////////////////////////////////////
