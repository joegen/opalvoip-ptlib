/*
 * Assert.cxx
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: assert.cxx,v $
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
