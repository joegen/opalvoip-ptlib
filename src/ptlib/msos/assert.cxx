/*
 * $Id: assert.cxx,v 1.2 1994/06/25 12:13:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: assert.cxx,v $
 * Revision 1.2  1994/06/25 12:13:01  robertj
 * Synchronisation.
 *
// Revision 1.1  1994/04/01  14:39:35  robertj
// Initial revision
//
 */

#include "ptlib.h"


void PAssertFunc(const char * file, int line, const char * msg)
{
  for (;;) {
    cerr << "Assertion fail: File " << file << ", Line " << line << endl;
    if (msg != NULL)
      cerr << msg << endl;
    cerr << "<A>bort, <B>reak, <I>gnore? ";
    cerr.flush();
    switch (getchar()) {
      case 'A' :
      case 'a' :
      case EOF :
        cerr << "Aborted\n";
        _exit(100);
        
      case 'B' :
      case 'b' :
        cerr << "Break\n";
        __asm int 3;
        
      case 'I' :
      case 'i' :
        cerr << "Ignored\n";
        return;
    }
  }
}


// End Of File ///////////////////////////////////////////////////////////////
