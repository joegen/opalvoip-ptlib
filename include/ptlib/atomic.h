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
 */

#ifndef PTLIB_CRITICALSECTION_H
#define PTLIB_CRITICALSECTION_H

#if P_STD_ATOMIC

#include <atomic>

using std::atomic;
#define PAtomicEnum std::atomic

#else

#ifdef P_ATOMICITY_HEADER
  #include P_ATOMICITY_HEADER
#endif

template <typename Type> struct atomic
{
  __inline atomic() { this->ConstructMutex(); }
  __inline atomic(Type value) : m_storage(value) { this->ConstructMutex(); }
  __inline atomic(const atomic & other) : m_storage(other.m_storage) { this->ConstructMutex(); }
  __inline ~atomic() { this->DestroyMutex(); }
  __inline atomic & operator=(const atomic & other) { this->Lock(); other.Lock(); this->m_storage = other.m_storage; other.Unlock(); this->Unlock(); return *this; }
  __inline operator Type() const { this->Lock(); Type value = this->m_storage; this->Unlock(); return value; }
  __inline Type load() const { this->Lock(); Type value = this->m_storage; this->Unlock(); return value; } \
  __inline void store(Type value) { this->Lock(); this->m_storage = value; this->Unlock(); } \
  __inline Type exchange(Type value) { this->Lock(); Type previous = this->m_storage; this->m_storage = value; this->Unlock(); return previous; }
  bool compare_exchange_strong(Type & comp, Type value)
  {
      this->Lock();
      if (this->m_storage == comp) {
        this->m_storage = value;
        this->Unlock();
        return true;
      }
      else {
        comp = this->m_storage;
        this->Unlock();
        return false;
      }
  }

private:
  volatile Type m_storage;

  // This is declared before any PMutex classes, have to do it manually.
#if _WIN32
  mutable CRITICAL_SECTION m_mutex;
  __inline void ConstructMutex() { InitializeCriticalSection(&this->m_mutex); }
  __inline void DestroyMutex()   { DeleteCriticalSection(&this->m_mutex); }
  __inline void Lock() const     { EnterCriticalSection(&this->m_mutex); }
  __inline void Unlock() const   { LeaveCriticalSection(&this->m_mutex); }
#else
  mutable pthread_mutex_t m_mutex;
  __inline void ConstructMutex() { pthread_mutex_init(&this->m_mutex, NULL); }
  __inline void DestroyMutex()   { pthread_mutex_destroy(&this->m_mutex); }
  __inline void Lock() const     { pthread_mutex_lock(&this->m_mutex); }
  __inline void Unlock() const   { pthread_mutex_unlock(&this->m_mutex); }
#endif
};

#define P_DEFINE_ATOMIC_FUNCTIONS(Type,Exch,FetchAdd,AddFetch,CompExch) \
    __inline atomic() : m_storage() { } \
    __inline atomic(Type value) : m_storage(value) { } \
    __inline atomic(const atomic & other) : m_storage((Type)AddFetch(const_cast<Type *>(&other.m_storage), 0)) { } \
    __inline atomic & operator=(const atomic & other) { store(other.load()); return *this; } \
    __inline atomic & operator=(Type other) { store(other); return *this; } \
    __inline operator Type() const { return (Type)AddFetch(const_cast<Type *>(&m_storage), 0); } \
    __inline bool compare_exchange_strong(Type & comp, Type value) { return CompExch(&m_storage, comp, value); } \
    __inline void store(Type value) { exchange(value); } \
    __inline Type load() const { return (Type)AddFetch(const_cast<Type *>(&m_storage), 0); } \
    __inline Type exchange(Type value) { return (Type)Exch(&m_storage, value); } \
    __inline Type operator++()         { return (Type)AddFetch(&m_storage,  1); } \
    __inline Type operator++(int)      { return (Type)FetchAdd(&m_storage,  1); } \
    __inline Type operator+=(Type i)   { return (Type)AddFetch(&m_storage,  i); } \
    __inline Type operator--()         { return (Type)AddFetch(&m_storage, -1); } \
    __inline Type operator--(int)      { return (Type)FetchAdd(&m_storage, -1); } \
    __inline Type operator-=(Type i)   { return (Type)AddFetch(&m_storage, -i); } \

#define P_DEFINE_ATOMIC_INT_CLASS(Type,Exch,FetchAdd,AddFetch,CompExch) \
  template <> struct atomic<Type> { \
    P_DEFINE_ATOMIC_FUNCTIONS(Type,Exch,FetchAdd,AddFetch,CompExch) \
    private: volatile Type m_storage; \
  }

#define P_DEFINE_ATOMIC_PTR_CLASS(Exch,FetchAdd,AddFetch,CompExch) \
  template <typename Type> struct atomic<Type *> { \
    P_DEFINE_ATOMIC_FUNCTIONS(Type*,Exch,FetchAdd,AddFetch,CompExch) \
    private: volatile Type * m_storage; \
  }

#if defined(_WIN32)

  #define P_Exchange8(storage, value)       _InterlockedExchange8         ((CHAR  *)(storage), value)
  #define P_CompExch8(storage,comp,value)  (_InterlockedCompareExchange8  ((CHAR  *)(storage), value, comp) == (CHAR)comp)
  #define P_FetchAdd8(storage, value)       _InterlockedExchangeAdd8      ((char  *)(storage), value)
  #define P_AddFetch8(storage, value)      (_InterlockedExchangeAdd8      ((char  *)(storage), value)+value)
  #define P_Exchange16(storage, value)      _InterlockedExchange16        ((SHORT *)(storage), value)
  #define P_CompExch16(storage,comp,value) (_InterlockedCompareExchange16 ((SHORT *)(storage), value, comp) == (SHORT)comp)
  #define P_FetchAdd16(storage, value)      _InterlockedExchangeAdd16     ((SHORT *)(storage), value)
  #define P_AddFetch16(storage, value)     (_InterlockedExchangeAdd16     ((SHORT *)(storage), value)+value)
  #define P_Exchange32(storage, value)      _InterlockedExchange          ((LONG  *)(storage), value)
  #define P_CompExch32(storage,comp,value) (_InterlockedCompareExchange   ((LONG  *)(storage), value, comp) == (LONG)comp)
  #define P_FetchAdd32(storage, value)      _InterlockedExchangeAdd       ((LONG  *)(storage), value)
  #define P_AddFetch32(storage, value)      _InterlockedAdd               ((LONG  *)(storage), value)
  #define P_Exchange64(storage, value)      _InterlockedExchange64        ((LONG64*)(storage), value)
  #define P_CompExch64(storage,comp,value) (_InterlockedCompareExchange64 ((LONG64*)(storage), value, comp) == (LONG64)comp)
  #define P_FetchAdd64(storage, value)      _InterlockedExchangeAdd64     ((LONG64*)(storage), value)
  #define P_AddFetch64(storage, value)      _InterlockedAdd64             ((LONG64*)(storage), value)
  #define P_ExchangePtr(storage, value)     _InterlockedExchangePointer   ((PVOID *)(storage), value)
  #ifdef _WIN64
    #define P_CompExchPtr(storage,comp,value) (_InterlockedCompareExchange64((LONG  *)(storage), value, comp) == comp)
    #define P_FetchAddPtr(storage, value)      _InterlockedExchangeAdd64    ((LONG64*)(storage), value)
    #define P_AddFetchPtr(storage, value)      _InterlockedAdd64            ((LONG64*)(storage), value)
  #else
    #define P_CompExchPtr(storage,comp,value) (_InterlockedCompareExchange  ((LONG  *)(storage), value, comp) == comp)
    #define P_FetchAddPtr(storage, value)      _InterlockedExchangeAdd      ((LONG  *)(storage), value)
    #define P_AddFetchPtr(storage, value)      _InterlockedAdd              ((LONG  *)(storage), value)
  #endif

  #define P_DEFINE_ATOMIC_INT_CLASS_WIN32(Type, Size) \
    P_DEFINE_ATOMIC_INT_CLASS(Type, P_Exchange##Size, P_FetchAdd##Size, P_AddFetch##Size, P_CompExch##Size)

  P_DEFINE_ATOMIC_INT_CLASS_WIN32(              bool, 8 );
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(  signed      char, 8 );
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(unsigned      char, 8 );
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(  signed     short, 16);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(unsigned     short, 16);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(  signed       int, 32);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(unsigned       int, 32);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(  signed      long, 32);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(unsigned      long, 32);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(  signed long long, 64);
  P_DEFINE_ATOMIC_INT_CLASS_WIN32(unsigned long long, 64);

  P_DEFINE_ATOMIC_PTR_CLASS(P_ExchangePtr, P_FetchAddPtr, P_AddFetchPtr, P_CompExchPtr);

#elif defined(P_ATOMICITY_BUILTIN)

  template <typename Type> static __inline bool p_compare_exchange_strong(volatile Type *ptr, Type & comp, Type newval)
  {
    Type oldval = __sync_val_compare_and_swap(ptr, comp, newval);
    if (oldval == comp)
      return true;
    comp = oldval;
    return false;
  }

  #define P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(Type) \
    P_DEFINE_ATOMIC_INT_CLASS(Type, __sync_lock_test_and_set, __sync_fetch_and_add, __sync_add_and_fetch, p_compare_exchange_strong)

  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(          bool);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(  signed  char);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(unsigned  char);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(  signed short);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(unsigned short);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(  signed   int);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(unsigned   int);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(  signed  long);
  P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(unsigned  long);
  #if HAVE_LONG_LONG_INT
    P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(signed long long);
  #endif
  #if HAVE_UNSIGNED_LONG_LONG_INT
    P_DEFINE_ATOMIC_INT_CLASS_BUILTIN(unsigned long long);
  #endif
  P_DEFINE_ATOMIC_PTR_CLASS(__sync_lock_test_and_set, __sync_fetch_and_add, __sync_add_and_fetch, __sync_bool_compare_and_swap);
  
#elif defined(SOLARIS) && !defined(__GNUC__)

  P_DEFINE_ATOMIC_INT_CLASS(          bool, atomic_swap_8,  atomic_add_8,  atomic_add_8_nv);
  P_DEFINE_ATOMIC_INT_CLASS(  signed  char, atomic_swap_8,  atomic_add_8,  atomic_add_8_nv);
  P_DEFINE_ATOMIC_INT_CLASS(unsigned  char, atomic_swap_8,  atomic_add_8,  atomic_add_8_nv);
  P_DEFINE_ATOMIC_INT_CLASS(  signed short, atomic_swap_16, atomic_add_16, atomic_add_16_nv);
  P_DEFINE_ATOMIC_INT_CLASS(unsigned short, atomic_swap_16, atomic_add_16, atomic_add_16_nv);
  P_DEFINE_ATOMIC_INT_CLASS(  signed   int, atomic_swap_32, atomic_add_32, atomic_add_32_nv);
  P_DEFINE_ATOMIC_INT_CLASS(unsigned   int, atomic_swap_32, atomic_add_32, atomic_add_32_nv);
  P_DEFINE_ATOMIC_INT_CLASS(  signed  long, atomic_swap_32, atomic_add_32, atomic_add_32_nv);
  P_DEFINE_ATOMIC_INT_CLASS(unsigned  long, atomic_swap_32, atomic_add_32, atomic_add_32_nv);
  #if HAVE_LONG_LONG_INT
    P_DEFINE_ATOMIC_INT_CLASS(signed long long, atomic_swap_64, atomic_add_64, atomic_add_64_nv);
  #endif
  #if HAVE_UNSIGNED_LONG_LONG_INT
    P_DEFINE_ATOMIC_INT_CLASS(unsigned long long, atomic_swap_64, atomic_add_64, atomic_add_64_nv);
  #endif
  #if defined(P_64BIT)
    P_DEFINE_ATOMIC_PTR_CLASS(atomic_swap_64, atomic_add_64, atomic_add_64_nv);
  #else
    P_DEFINE_ATOMIC_PTR_CLASS(atomic_swap_32, atomic_add_32, atomic_add_32_nv);
  #endif

#elif defined(P_ATOMICITY_HEADER)

  P_DEFINE_ATOMIC_INT_CLASS(_Atomic_word, m_storage = value, P_ATOMICITY_NAMESPACE __exchange_and_add)

#endif

  template <typename Enum> struct PAtomicEnum
  {
    __inline PAtomicEnum() { }
    __inline PAtomicEnum(Enum value) : m_enum(value) { }
    __inline PAtomicEnum(const PAtomicEnum & other) : m_enum(other.m_enum) { }
    __inline Enum operator=(const PAtomicEnum & other) { Enum value = other; this->store(value); return value; }
    __inline operator Enum() const { return this->load(); }
    __inline Enum load() const { return (Enum)this->m_enum.load(); }
    __inline void store(Enum value) { this->m_enum.store(value); }
    __inline Enum exchange(Enum value) { return (Enum)this->m_enum.exchange(value); }
    __inline bool compare_exchange_strong(Enum & comp, Enum value) { return this->m_enum.compare_exchange_strong((int &)comp, value); }
  private:
    atomic<int> m_enum;
  };

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
