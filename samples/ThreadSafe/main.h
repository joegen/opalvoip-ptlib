/*
 * main.h
 *
 * PWLib application header file for ThreadSafe
 *
 * Copyright 2002 Equivalence
 *
 * $Log: main.h,v $
 * Revision 1.1  2002/05/01 04:16:44  robertj
 * Added thread safe collection classes.
 *
 */

#ifndef _ThreadSafe_MAIN_H
#define _ThreadSafe_MAIN_H


#include <ptlib/safecoll.h>


class ThreadSafe;

class TestObject : public PSafeObject
{
    PCLASSINFO(TestObject, PSafeObject);
  public:
    TestObject(ThreadSafe & process, unsigned val);
    ~TestObject();

    Comparison Compare(const PObject & obj);
    void PrintOn(ostream & strm) const;

  private:
    ThreadSafe & process;
    unsigned value;
};


class ThreadSafe : public PProcess
{
  PCLASSINFO(ThreadSafe, PProcess)

  public:
    ThreadSafe();
    void Main();

  private:
    PDECLARE_NOTIFIER(PThread, ThreadSafe, TestThread);
    void Test();

    PSafeList<PList<TestObject>, TestObject> unsorted;
    PSafeList<PSortedList<TestObject>, TestObject> sorted;
    PSafeDictionary<PDictionary<POrdinalKey, TestObject>, POrdinalKey, TestObject> sparse;

    PINDEX        threadCount;
    PTimeInterval startTick;
    PMutex        mutexObjects;
    unsigned      totalObjects;
    unsigned      currentObjects;

  friend class TestObject;
};


#endif  // _ThreadSafe_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
