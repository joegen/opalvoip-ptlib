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
 * Revision 1.1  1996/01/26 11:09:15  craigs
 * Initial revision
 *
 */

#include <ptlib.h>

#include <ctype.h>
#include <signal.h>

void PAssertFunc (const char * file, int line, const char * msg)

{
  for(;;) {
    PError << "Assertion fail: File " << file << ", Line " << line << endl;
    if (msg != NULL)
      PError << msg << endl;
    PError << "\n<A>bort, <C>ore dump, <I>gnore? ";
    PError.flush();
    switch (tolower(getchar())) {
      case 'a' :
        exit (1);

      case 'c' :
        kill(getpid(), SIGABRT);

      case 'i' :
        return;
    }
  }
}

// End Of File ///////////////////////////////////////////////////////////////
