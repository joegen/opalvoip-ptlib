/*
 * main.cxx
 *
 * PWLib application source file for LDAP Test
 *
 * Main program entry point.
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * $Log: main.cxx,v $
 * Revision 1.1  2003/03/28 01:15:44  robertj
 * OpenLDAP support.
 *
 */

#include "precompile.h"
#include "main.h"
#include "version.h"

#include <ptclib/pldap.h>


// Test command line:
//  search -h ils.seconix.com -x -b "objectClass=RTPerson" "cn=*" "*"



PCREATE_PROCESS(LDAPTest);



LDAPTest::LDAPTest()
  : PProcess("Equivalence", "LDAP Test", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}


void LDAPTest::Main()
{
  PArgList & args = GetArguments();
  args.Parse("h:p:x.b:s:", FALSE);

  if (args.GetCount() == 0) {
    Usage();
    return;
  }

  PLDAPSession ldap;
  if (!ldap.Open(args.GetOptionString('h'), (WORD)args.GetOptionString('p').AsUnsigned())) {
    cerr << "Could not open LDAP server at " << args[1];
    return;
  }

  if (args[0] == "search")
    Search(args, ldap);
  else
    cerr << "Invalid command: " << args[0];
}


void LDAPTest::Usage()
{
  cerr << "usage: " << GetFile().GetTitle() << " search [ args ]\n"
          "   General args:\n"
          "      -h host    LDAP server\n"
          "      -p port    port on LDAP server\n"
          "      -s scope   one of base, one, or sub (search scope)\n"
          "      -x         Simple authentication\n"
          "\n"
          "   search args:\n"
          "      filter attribute [ attribute ... ]\n";
}


void LDAPTest::Search(PArgList & args, PLDAPSession & ldap)
{
  if (args.GetCount() < 2) {
    Usage();
    return;
  }

  PList<PStringToString> data = ldap.SearchAll(args[1],
                                               args.GetParameters(2),
                                               args.GetOptionString('b'));
  for (PINDEX i = 0; i < data.GetSize(); i++)
    cout << data[i] << '\n';
}


// End of File ///////////////////////////////////////////////////////////////
