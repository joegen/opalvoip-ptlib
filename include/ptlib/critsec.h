/*
 * critsec.h
 *
 * Critical section mutex class.
 *
 * Portable Windows Library
 *
 * Copyright (C) 2004 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: critsec.h,v $
 * Revision 1.2  2004/04/11 03:20:41  csoutheren
 * Added Unix implementation of PCriticalSection
 *
 * Revision 1.1  2004/04/11 02:55:17  csoutheren
 * Added PCriticalSection for Windows
 * Added compile time option for PContainer to use critical sections to provide thread safety under some circumstances
 *
 */

#ifndef _PCRITICALSECTION
#define _PCRITICALSECTION

/** This class implements critical section mutexes using the most
  * efficient mechanism available on the host platform.
  */

class PCriticalSection : public PObject
{
  PCLASSINFO(PCriticalSection, PObject);

  public:
  /**@name Construction */
  //@{
    /**Create a new critical section object .
     */
    PINLINE PCriticalSection();

    /**Destroy the critical section object
     */
    PINLINE ~PCriticalSection();
  //@}

  /**@name Operations */
  //@{
    /** Enter the critical section by waiting for exclusive access.
     */
    PINLINE void Enter();

    /** Leave the critical section by unlocking the mutex
     */
    PINLINE void Leave();

  //@}

  private:
    PCriticalSection & operator=(const PCriticalSection &) { return *this; }

// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/critsec.h"
#else
#include "unix/ptlib/critsec.h"
#endif
};


class PEnterAndLeave {
  public:
    /**Create the critical section enter and leave instance.
       This will enter the critical section using the #Enter()# function
       before returning.
      */
    inline PEnterAndLeave(
      PCriticalSection & _critSec,   /// CriticalSection descendent to enter/leave.
      BOOL enter = TRUE                   /// Enter before returning.
    )
    : critSec(_critSec)
    { if (enter) critSec.Enter(); }

    /** Leave the critical section.
        This will execute the Leave() function on the critical section that was used
        in the construction of this instance.
     */
    inline ~PEnterAndLeave()
    { critSec.Leave(); }

  protected:
    PCriticalSection & critSec;
};


#endif

// End Of File ///////////////////////////////////////////////////////////////
