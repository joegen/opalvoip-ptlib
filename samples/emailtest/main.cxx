/*
 * main.cxx
 *
 * PWLib application source file for emailtest
 *
 * Main program entry point.
 *
 * Copyright (c) 2004 Post Increment
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
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptclib/inetmail.h>
#include <ptlib/pprocess.h>


class Test : public PProcess
{
  PCLASSINFO(Test, PProcess)

  public:
    Test();
    virtual void Main();
};

PCREATE_PROCESS(Test);

Test::Test()
  : PProcess("Post Increment", "emailtest", 1, 0, AlphaCode, 0)
{
}

void Test::Main()
{
  PArgList & args = GetArguments();

  if (!args.Parse("h-help.   Help\n"
                  "-host:    Server host/address\n"
                  "-port:    Server port\n"
                  "-user:    Username\n"
                  "-pass:    Password\n"
                  "-from:    From email address\n"
                  "-to:      To email addresses\n"
                  "-cc:      CC email addresses\n"
                  "-bcc:     BCC email addresses\n"
                  "-subject: Subject for email\n"
                  "-attach:  Attachments"
                  PTRACE_ARGLIST
  ) || args.HasOption('h')) {
    args.Usage(cerr, "[ options] <body>");
    return;
  }

  PTRACE_INITIALISE(args);

  PSMTP::Parameters params;
  params.m_hostname = args.GetOptionString("host");
  params.m_port = args.GetOptionAs<WORD>("port", 0);
  params.m_username = args.GetOptionString("user");
  params.m_password = args.GetOptionString("pass");
  params.m_from = args.GetOptionString("from");
  params.m_to = args.GetOptionString("to").Lines();
  params.m_cc = args.GetOptionString("cc").Lines();
  params.m_bcc = args.GetOptionString("bcc").Lines();
  params.m_subject = args.GetOptionString("subject");
  params.m_bodyText = args.GetParameters().ToString();
  params.m_attachments = args.GetOptionString("attach").Lines();

  PString error;
  if (PSMTP::SendMail(params, error))
    cout << "Send of email successful\n";
  else
    cerr << "Send of email failed: " << error << '\n';
}


// End of File ///////////////////////////////////////////////////////////////
