/*
 * assert.cxx
 *
 * Assert function implementation.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: assert.cxx,v $
 * Revision 1.3  1998/09/24 04:12:08  robertj
 * Added open software license.
 *
 * Revision 1.2  1998/06/17 14:47:47  robertj
 * Fixed continuous display of assert if input is from /dev/null
 *
 * Revision 1.1  1996/01/26 11:09:15  craigs
 * Initial revision
 *
 */

#include <ptlib.h>

#include <ctype.h>
#include <signal.h>

void PAssertFunc (const char * file, int line, const char * msg)

{
  PError << "Assertion fail: File " << file << ", Line " << line << endl;
  if (msg != NULL)
    PError << msg << endl;
  for(;;) {
    PError << "\n<A>bort, <C>ore dump, <I>gnore? " << flush;
    int c = getchar();
    switch (c) {
      case 'a' :
      case 'A' :
        PError << "\nAborting.\n";
        exit (1);

      case 'c' :
      case 'C' :
        PError << "\nDumping core.\n";
        kill(getpid(), SIGABRT);

      case 'i' :
      case 'I' :
      case EOF :
        PError << "\nIgnoring.\n";
        return;
    }
  }
}

// End Of File ///////////////////////////////////////////////////////////////
