/*
 * $Id
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log
 */


#define _PSOUND


DECLARE_CLASS(PSound, PObject)
  // A class representing a sound.

  public:
    PSound();


    static void Beep();
      // Play the "standard" warning beep for the platform.

    
// Class declaration continued in platform specific header file ///////////////
