/*
 * $Id: config.h,v 1.3 1994/01/03 04:42:23 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: config.h,v $
 * Revision 1.3  1994/01/03 04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


#define _PCONFIG


PDECLARE_CLASS(PConfig, PObject)
  // A class representing a configuration for the application

  public:
    PConfig();


// Class declaration continued in platform specific header file ///////////////
