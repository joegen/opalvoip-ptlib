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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef _PCRITICALSECTION
#define _PCRITICALSECTION

#include <ptlib/psync.h>

#if defined(SOLARIS) && !defined(__GNUC__)
#include <atomic.h>
#endif

#if P_HAS_ATOMIC_INT

#if defined(__GNUC__)
#  if __GNUC__ >= 4 && __GNUC_MINOR__ >= 2
#     include <ext/atomicity.h>
#  else
#     include <bits/atomicity.h>
#  endif
#endif

#if P_NEEDS_GNU_CXX_NAMESPACE
#define EXCHANGE_AND_ADD(v,i)   __gnu_cxx::__exchange_and_add(v,i)
#else
#define EXCHANGE_AND_ADD(v,i)   __exchange_and_add(v,i)
#endif

#endif // P_HAS_ATOMIC_INT


/** This class implements critical section mutexes using the most
  * efficient mechanism available on the host platform.
  * For Windows, CriticalSection is used.
  * On other platforms, pthread_mutex_t is used
  */

#ifdef _WIN32

class PCriticalSection : public PSync
{
  PCLASSINFO(PCriticalSection, PSync);

  public:
  /**@name Construction */
  //@{
    /**Create a new critical section object .
     */
    PCriticalSection();

    /**Allow copy constructor, but it actually does not copy the critical section,
       it creates a brand new one as they cannot be shared in that way.
     */
    PCriticalSection(const PCriticalSection &);

    /**Destroy the critical section object
     */
    ~PCriticalSection();

    /**Assignment operator is allowed but does nothing. Overwriting the old critical
       section information would be very bad.
      */
    PCriticalSection & operator=(const PCriticalSection &) { return *this; }
  //@}

  /**@name Operations */
  //@{
    /** Create a new PCriticalSection
      */
    PObject * Clone() const
    {
      return new PCriticalSection();
    }

    /** Enter the critical section by waiting for exclusive access.
     */
    void Wait();
    inline void Enter() { Wait(); }

    /** Leave the critical section by unlocking the mutex
     */
    void Signal();
    inline void Leave() { Signal(); }

    /** Try to enter the critical section for exlusive access. Does not wait.
        @return true if cirical section entered, leave/Signal must be called.
      */
    bool Try();
  //@}


#include "msos/ptlib/critsec.h"

};

#endif

typedef PWaitAndSignal PEnterAndLeave;

/** This class implements an integer that can be atomically 
  * incremented and decremented in a thread-safe manner.
  * On Windows, the integer is of type long and this class is implemented using InterlockedIncrement
  * and InterlockedDecrement integer is of type long.
  * On Unix systems with GNU std++ support for EXCHANGE_AND_ADD, the integer is of type _Atomic_word (normally int)
  * On all other systems, this class is implemented using PCriticalSection and the integer is of type int.
  */

class PAtomicInteger 
{
#if defined(_WIN32) || defined(DOC_PLUS_PLUS)
    public:
      /** Create a PAtomicInteger with the specified initial value
        */
      inline PAtomicInteger(
        long v = 0                     ///< initial value
      )
        : value(v) { }

      /**
        * Test if an atomic integer has a zero value. Note that this
        * is a non-atomic test - use the return value of the operator++() or
        * operator--() tests to perform atomic operations
        *
        * @return PTrue if the integer has a value of zero
        */
      bool IsZero() const                 { return value == 0; }

      /**
        * atomically increment the integer value
        *
        * @return Returns the value of the integer after the increment
        */
      inline long operator++()            { return InterlockedIncrement(&value); }

      /**
        * atomically decrement the integer value
        *
        * @return Returns the value of the integer after the decrement
        */
      inline long operator--()            { return InterlockedDecrement(&value); }

      /**
        * @return Returns the value of the integer
        */
      inline operator long () const       { return value; }

      /**
        * Set the value of the integer
        */
      inline void SetValue(
        long v                          ///< value to set
      )
      { value = v; }
    protected:
      long value;
#elif defined(_STLP_INTERNAL_THREADS_H) && defined(_STLP_ATOMIC_INCREMENT) && defined(_STLP_ATOMIC_DECREMENT)
    public:
      inline PAtomicInteger(__stl_atomic_t v = 0)
        : value(v) { }
      inline bool IsZero() const         { return value == 0; }
      inline int operator++()            { return _STLP_ATOMIC_INCREMENT(&value); }
      inline int unsigned operator--()   { return _STLP_ATOMIC_DECREMENT(&value); }
      inline operator int () const       { return value; }
      inline void SetValue(int v)        { value = v; }
    protected:
      __stl_atomic_t value;
#elif defined(SOLARIS) && !defined(__GNUC__)
    public:
      inline PAtomicInteger(uint32_t v = 0)
      : value(v) { }
      inline bool IsZero() const         { return value == 0; }
      inline int operator++()            { return atomic_add_32_nv((&value), 1); }
      inline int unsigned operator--()   { return atomic_add_32_nv((&value), -1); }
      inline operator int () const       { return value; }
      inline void SetValue(int v)        { value = v; }
    protected:
       uint32_t value;
#elif defined(__GNUC__) && P_HAS_ATOMIC_INT
    public:
      inline PAtomicInteger(int v = 0)
        : value(v) { }
      inline bool IsZero() const         { return value == 0; }
      inline int operator++()            { return EXCHANGE_AND_ADD(&value, 1) + 1; }
      inline int unsigned operator--()   { return EXCHANGE_AND_ADD(&value, -1) - 1; }
      inline operator int () const       { return value; }
      inline void SetValue(int v)        { value = v; }
    protected:
      _Atomic_word value;
#else
    public:
      inline PAtomicInteger(int v = 0)
        : value(v)                       { pthread_mutex_init(&mutex, NULL); }
      inline ~PAtomicInteger()           { pthread_mutex_destroy(&mutex); }
      inline bool IsZero() const         { return value == 0; }
      inline int operator++()            { pthread_mutex_lock(&mutex); int retval = value++; pthread_mutex_unlock(&mutex); return retval; }
      inline int operator--()            { pthread_mutex_lock(&mutex); int retval = value--; pthread_mutex_unlock(&mutex); return retval; }
      inline operator int () const       { return value; }
      inline void SetValue(int v)        { pthread_mutex_lock(&mutex); value = v; pthread_mutex_unlock(&mutex); }
    protected:
      pthread_mutex_t mutex;
      int value;
#endif
    private:
      PAtomicInteger & operator=(const PAtomicInteger & ref) { value = (int)ref; return *this; }
};

#endif

// End Of File ///////////////////////////////////////////////////////////////
