/*
 * $Id: sound.h,v 1.6 1994/08/22 00:46:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: sound.h,v $
 * Revision 1.6  1994/08/22 00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.5  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.4  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.3  1993/09/29  03:06:30  robertj
 * Added unix compatibility to Beep()
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


#define _PSOUND

#ifdef __GNU__
#pragma interface
#endif


PDECLARE_CLASS(PSound, PObject)
  // A class representing a sound.

  public:
    PSound();
      // Create an empty sound


    static void Beep();
      // Play the "standard" warning beep for the platform.

    
// Class declaration continued in platform specific header file ///////////////
