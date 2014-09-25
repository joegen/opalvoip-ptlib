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

#ifndef PTLIB_CRITICALSECTION_H
#define PTLIB_CRITICALSECTION_H

#if P_STD_ATOMIC

#include <atomic>

#else

#ifdef P_ATOMICITY_HEADER
  #include P_ATOMICITY_HEADER
#endif

#define P_DEFINE_ATOMIC_FUNCTIONS1(T,Exch) \
    __inline atomic() { } \
    __inline atomic(T value) : m_storage(value) { } \
    __inline atomic(const atomic & other) : m_storage(other.m_storage) { } \
    __inline atomic & operator=(const atomic & other) { m_storage = other.m_storage; return *this; } \
    __inline operator T() const { return m_storage; } \
    __inline T exchange(T value) { return (T)(Exch); }

#define P_DEFINE_ATOMIC_FUNCTIONS2(T, Exch, PreInc, PostInc, PreDec, PostDec) \
    P_DEFINE_ATOMIC_FUNCTIONS1(T,Exch) \
    __inline T operator++(int) { return (T)(PreInc);  } \
    __inline T operator++()    { return (T)(PostInc); } \
    __inline T operator--(int) { return (T)(PreDec);  } \
    __inline T operator--()    { return (T)(PostDec); } \


template <typename T> struct atomic
{
  P_DEFINE_ATOMIC_FUNCTIONS1(T, m_storage = value)
private:
  volatile T m_storage;
#if _WIN32
  CRITICAL_SECTION m_mutex;
#else
  pthread_mutex_t m_mutex;
#endif
};

#define P_DEFINE_ATOMIC_BASE_CLASS(T, Exch) \
  template <> struct atomic<T *> { \
    P_DEFINE_ATOMIC_FUNCTIONS1(T *, Exch) \
    private: T * m_storage; \
  }

#define P_DEFINE_ATOMIC_INT_CLASS(T, Exch, PreInc, PostInc, PreDec, PostDec) \
  template <> struct atomic<T> { \
    P_DEFINE_ATOMIC_FUNCTIONS2(T, Exch, PreInc, PostInc, PreDec, PostDec) \
    private: T m_storage; \
  }

#define P_DEFINE_ATOMIC_PTR_CLASS(Exch) \
  template <typename T> struct atomic<T *> { \
    P_DEFINE_ATOMIC_FUNCTIONS1(T *, Exch) \
    private: T * m_storage; \
  }

#if defined(_WIN32)

  #define P_DEFINE_ATOMIC_INT_CLASS_WIN32(T1, T2, Exch, PreInc, PreDec, Add) \
    P_DEFINE_ATOMIC_INT_CLASS(T1, Exch(reinterpret_cast<T2 *>(&m_storage), value), \
                                PreInc(reinterpret_cast<T2 *>(&m_storage)), \
                                   Add(reinterpret_cast<T2 *>(&m_storage), 1), \
                                PreDec(reinterpret_cast<T2 *>(&m_storage)), \
                                   Add(reinterpret_cast<T2 *>(&m_storage), -1))

  P_DEFINE_ATOMIC_INT_CLASS_WIN32(  int8_t, SHORT,  _InterlockedExchange16, _InterlockedIncrement16, _InterlockedDecrement16, _InterlockedExchangeAdd16);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32( uint8_t, SHORT,  _InterlockedExchange16, _InterlockedIncrement16, _InterlockedDecrement16, _InterlockedExchangeAdd16);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32( int16_t, SHORT,  _InterlockedExchange16, _InterlockedIncrement16, _InterlockedDecrement16, _InterlockedExchangeAdd16);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(uint16_t, SHORT,  _InterlockedExchange16, _InterlockedIncrement16, _InterlockedDecrement16, _InterlockedExchangeAdd16);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32( int32_t, long,   _InterlockedExchange,   _InterlockedIncrement,   _InterlockedDecrement,   _InterlockedAdd  );
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(uint32_t, long,   _InterlockedExchange,   _InterlockedIncrement,   _InterlockedDecrement,   _InterlockedAdd  );
  P_DEFINE_ATOMIC_INT_CLASS_WIN32( int64_t, LONG64, _InterlockedExchange64, _InterlockedIncrement64, _InterlockedDecrement64, _InterlockedExchangeAdd64);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(uint64_t, LONG64, _InterlockedExchange64, _InterlockedIncrement64, _InterlockedDecrement64, _InterlockedExchangeAdd64);
  P_DEFINE_ATOMIC_PTR_CLASS(_InterlockedExchangePointer(reinterpret_cast<PVOID *>(&m_storage), value));

#elif defined(P_ATOMICITY_BUILTIN)

  #define P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(T) \
    P_DEFINE_ATOMIC_INT_CLASS(T, \
                              __sync_lock_test_and_set(&m_storage, value), \
                              __sync_fetch_and_add(&m_storage, 1), \
                              __sync_add_and_fetch(&m_storage, 1), \
                              __sync_fetch_and_sub(&m_storage, 1), \
                              __sync_sub_and_fetch(&m_storage, 1))
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(  int8_t);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN( uint8_t);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN( int16_t);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(uint16_t);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN( int32_t);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(uint32_t);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN( int64_t);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(uint64_t);
  P_DEFINE_ATOMIC_PTR_CLASS(__sync_lock_test_and_set(&m_storage, value));
  
#elif defined(_STLP_INTERNAL_THREADS_H) && defined(_STLP_ATOMIC_INCREMENT) && defined(_STLP_ATOMIC_DECREMENT)

  P_DEFINE_ATOMIC_INT_CLASS(__stl_atomic_t, \
                            _STLP_ATOMIC_EXCHANGE( &m_storage, value), \
                            _STLP_ATOMIC_INCREMENT(&m_storage, 1)-1, \
                            _STLP_ATOMIC_INCREMENT(&m_storage, 1), \
                            _STLP_ATOMIC_DECREMENT(&m_storage, 1)+1, \
                            _STLP_ATOMIC_DECREMENT(&m_storage, 1));
  P_DEFINE_ATOMIC_BASE_CLASS(bool, _STLP_ATOMIC_EXCHANGE(&m_storage, value));
  #if !defined(P_64BIT)
    P_DEFINE_ATOMIC_PTR_CLASS(_STLP_ATOMIC_EXCHANGE(&m_storage, value));
  #endif

#elif defined(SOLARIS) && !defined(__GNUC__)

  P_DEFINE_ATOMIC_INT_CLASS(__stl_atomic_t, \
                            atomic_swap_32(  &m_storage, value), \
                            atomic_add_32_nv(&m_storage,  1)-1, \
                            atomic_add_32_nv(&m_storage,  1), \
                            atomic_add_32_nv(&m_storage, -1)+1, \
                            atomic_add_32_nv(&m_storage, -1))
  P_DEFINE_ATOMIC_BASE_CLASS(bool, atomic_swap_32(&m_storage, value))
  #if !defined(P_64BIT)
    P_DEFINE_ATOMIC_PTR_CLASS(atomic_swap_32(&m_storage, value));
  #endif

#elif defined(P_ATOMICITY_HEADER)

  #define EXCHANGE_AND_ADD P_ATOMICITY_NAMESPACE __exchange_and_add
  P_DEFINE_ATOMIC_INT_CLASS(_Atomic_word, \
                            m_storage = value, \
                            EXCHANGE_AND_ADD(&m_storage,  1), \
                            EXCHANGE_AND_ADD(&m_storage,  1)+1, \
                            EXCHANGE_AND_ADD(&m_storage, -1), \
                            EXCHANGE_AND_ADD(&m_storage, -1)-1)
  P_DEFINE_ATOMIC_BASE_CLASS(bool, (_Atomic_word previous = EXCHANGE_AND_ADD(&m_value, value?1:-1), m_value = value?1:0))

#endif


#endif // P_STD_ATOMIC

// For backward compatibility
class PAtomicInteger : public atomic<long>
{
  public:
    typedef long IntegerType;
    explicit PAtomicInteger(IntegerType value = 0) : atomic<IntegerType>(value) { }
    __inline PAtomicInteger & operator=(IntegerType value) { atomic<IntegerType>::operator=(value); return *this; }
    void SetValue(IntegerType value) { atomic<IntegerType>::operator=(value); }
    __inline bool IsZero() const { return static_cast<IntegerType>(*this) == 0; }
};

// For backward compatibility
class PAtomicBoolean : public atomic<bool>
{
  public:
    typedef long IntegerType;
    explicit PAtomicBoolean(bool value = false) : atomic<bool>(value) { }
    __inline PAtomicBoolean & operator=(bool value) { atomic<bool>::operator=(value); return *this; }
    bool TestAndSet(bool value) { return exchange(value); }
};

#endif // PTLIB_CRITICALSECTION_H


// End Of File ///////////////////////////////////////////////////////////////
