/*
 * sound.h
 *
 * Sound interface class.
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
 * $Log: sound.h,v $
 * Revision 1.11  1998/09/23 06:21:27  robertj
 * Added open source copyright license.
 *
 * Revision 1.10  1995/06/17 11:13:26  robertj
 * Documentation update.
 *
 * Revision 1.9  1995/03/14 12:42:40  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.8  1995/01/16  09:42:05  robertj
 * Documentation.
 *
 * Revision 1.7  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.6  1994/08/22  00:46:48  robertj
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

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PSound, PObject)
/* A class representing a sound. A sound is a highly platform dependent entity
   that is abstracted for use here. Very little manipulation of the sounds are
   possible. The class is provided mainly for the playback of sound files on
   the system.

   The most common sound to use is the static function <A>Beep()</A> which
   emits the system standard "warning" or "attention" sound.
 */

  public:
    PSound();
    // Create an empty sound.


    static void Beep();
    // Play the "standard" warning beep for the platform.

    
// Class declaration continued in platform specific header file ///////////////
