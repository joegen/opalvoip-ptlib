/*
 * $Id: assert.cxx,v 1.1 1994/04/01 14:39:35 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: assert.cxx,v $
 * Revision 1.1  1994/04/01 14:39:35  robertj
 * Initial revision
 *
 */

#include "ptlib.h"

void PAssertFunc(const char * file, int line)
{
  for (;;) {
    cerr << "Assertion fail: File " << file << ", Line " << line << endl
         << "<A>bort, <B>reak, <I>gnore? ";
    cerr.flush();
    switch (getchar()) {
      case 'A' :
      case 'a' :
        cerr << "Aborted\n";
        exit(100);
        
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
