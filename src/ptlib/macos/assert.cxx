/*
 * $Id: assert.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: assert.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <svcproc.h>

#include <errno.h>


///////////////////////////////////////////////////////////////////////////////
// PProcess

void PAssertFunc(const char * file, int line, const char * msg)
{
  int err = errno;

  for (;;) {
    fprintf(stderr, "Assertion fail: File %s, Line %u\n", file, line);
    if (msg != NULL)
      fprintf(stderr, "%s - Error code=%u\n", msg, err);
    fputs("<A>bort, <B>reak, <I>gnore? ", stderr);
    switch (getchar()) {
      case 'A' :
      case 'a' :
      case EOF :
        fputs("Aborted\n", stderr);
        _exit(100);
        
      case 'B' :
      case 'b' :
        fputs("Break\n", stderr);
        Debugger();
        return;

      case 'I' :
      case 'i' :
        fputs("Ignored\n", stderr);
        return;
    }
  }

#if 0
  SetCursor(&qd.arrow);
  PString message;
  message.sprintf("Assert failed: file %s line %d", file, line);
  Str255 dummy;
  dummy[0] = '\0';
  ParamText(message.ToPascal(), dummy, dummy, dummy);
  Alert(10000, nil);
  ExitToShell();
#endif
}


// End Of File ///////////////////////////////////////////////////////////////
