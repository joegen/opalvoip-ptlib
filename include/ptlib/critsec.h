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
 * Revision 1.8  2004/05/12 04:36:13  csoutheren
 * Fixed problems with using sem_wait and friends on systems that do not
 * support atomic integers
 *
 * Revision 1.7  2004/04/21 11:22:56  csoutheren
 * Modified to work with gcc 3.4.0
 *
 * Revision 1.6  2004/04/14 06:58:00  csoutheren
 * Fixed PAtomicInteger and PSmartPointer to use real atomic operations
 *
 * Revision 1.5  2004/04/12 03:35:26  csoutheren
 * Fixed problems with non-recursuve mutexes and critical sections on
 * older compilers and libc
 *
 * Revision 1.4  2004/04/12 00:58:45  csoutheren
 * Fixed PAtomicInteger on Linux, and modified PMutex to use it
 *
 * Revision 1.3  2004/04/12 00:36:04  csoutheren
 * Added new class PAtomicInteger and added Windows implementation
 *
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

#if P_HAS_ATOMIC_INT
#if P_NEEDS_GNU_CXX_NAMESPACE
#define	EXCHANGE_AND_ADD(v,i)	__gnu_cxx::__exchange_and_add(v,i)
#else
#define	EXCHANGE_AND_ADD(v,i)	__exchange_and_add(v,i)
#endif
#endif

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


/** This class implements ain integer that can be atomically 
  * incremented and decremented
  */

class PAtomicInteger 
{
#ifdef _WIN32
    public:
      inline PAtomicInteger(long v = 0)
        : value(v) { }
      BOOL IsZero() const                 { return value == 0; }
      inline long operator++()            { return InterlockedIncrement(&value); }
      inline long operator--()            { return InterlockedDecrement(&value); }
      inline operator long () const       { return value; }
      inline void SetValue(long v)        { value = v; }
    protected:
      long value;
#elif P_HAS_ATOMIC_INT
    public:
      inline PAtomicInteger(int v = 0)
        : value(v) { }
      BOOL IsZero() const                { return value == 0; }
      inline int operator++()            { return EXCHANGE_AND_ADD(&value, 1) + 1; }
      inline int unsigned operator--()   { return EXCHANGE_AND_ADD(&value, -1) - 1; }
      inline operator int () const       { return value; }
      inline void SetValue(int v)        { value = v; }
    protected:
      _Atomic_word value;
#else 
    protected:
      PCriticalSection critSec;
    public:
      inline PAtomicInteger(int v = 0)
        : value(v) { }
      BOOL IsZero() const                { return value == 0; }
      inline int operator++()            { PEnterAndLeave m(critSec); value++; return value;}
      inline int operator--()            { PEnterAndLeave m(critSec); value--; return value;}
      inline operator int () const       { return value; }
      inline void SetValue(int v)        { value = v; }
   private:
      PAtomicInteger & operator=(const PAtomicInteger & ref) { value = (int)ref; return *this; }
    protected:
      int value;
#endif
};

#endif

// End Of File ///////////////////////////////////////////////////////////////
