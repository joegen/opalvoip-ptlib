/*
 * main.h
 *
 * PWLib application header file for ThreadSafe
 *
 * Copyright 2002 Equivalence
 *
 * $Log: main.h,v $
 * Revision 1.4  2002/12/11 03:38:35  robertj
 * Added more tests
 *
 * Revision 1.3  2002/12/02 01:08:29  robertj
 * Updated to latest safe collection classes, thanks Vladimir Nesic
 *
 * Revision 1.2  2002/05/02 00:30:03  robertj
 * Allowed for non-template containers
 *
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

    ThreadSafe & process;
    unsigned value;
};

PLIST(TestList, TestObject);
PSORTED_LIST(TestSortedList, TestObject);
PDICTIONARY(TestDict, POrdinalKey, TestObject);


class ThreadSafe : public PProcess
{
  PCLASSINFO(ThreadSafe, PProcess)

  public:
    ThreadSafe();
    ~ThreadSafe();
    void Main();

  private:
    void Test1(PArgList & args);
    void Test1Output();
    PDECLARE_NOTIFIER(PThread, ThreadSafe, Test1Thread);

    void Test2(PArgList & args);
    PDECLARE_NOTIFIER(PThread, ThreadSafe, Test2Thread1);
    PDECLARE_NOTIFIER(PThread, ThreadSafe, Test2Thread2);

    void Test3(PArgList & args);
    PDECLARE_NOTIFIER(PThread, ThreadSafe, Test3Thread1);
    PDECLARE_NOTIFIER(PThread, ThreadSafe, Test3Thread2);

    PSafeList<TestObject> unsorted;
    PSafeList<TestObject> sorted;
    PSafeDictionary<POrdinalKey, TestObject> sparse;

    PINDEX        threadCount;
    PTimeInterval startTick;
    PMutex        mutexObjects;
    unsigned      totalObjects;
    unsigned      currentObjects;

  friend class TestObject;
};


#endif  // _ThreadSafe_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
