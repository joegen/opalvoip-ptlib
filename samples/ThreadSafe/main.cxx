/*
 * main.cxx
 *
 * PWLib application source file for ThreadSafe
 *
 * Main program entry point.
 *
 * Copyright 2002 Equivalence
 *
 * $Log: main.cxx,v $
 * Revision 1.2  2002/05/02 00:30:26  robertj
 * Added dump of thread times during start up.
 *
 * Revision 1.1  2002/05/01 04:16:44  robertj
 * Added thread safe collection classes.
 *
 */

#include <ptlib.h>
#include "main.h"

#include <ptclib/random.h>


PCREATE_PROCESS(ThreadSafe);


ThreadSafe::ThreadSafe()
  : PProcess("Equivalence", "ThreadSafe", 1, 0, AlphaCode, 1)
{
  threadCount = 99;
  totalObjects = 0;
  currentObjects = 0;
}


void ThreadSafe::Main()
{
  PArgList & args = GetArguments();
  if (args.GetCount() > 0)
    threadCount = args[0].AsUnsigned();

  cout << "Starting " << threadCount << " threads." << endl;

  for (PINDEX i = 0; i < threadCount; i++) {
    PTimeInterval duration = PRandom::Number()%540000 + 60000;
    cout << setw(4) << (i+1) << '=' << duration;
    if (i%5 == 4)
      cout << '\n';
    PThread::Create(PCREATE_NOTIFIER(TestThread), duration.GetMilliSeconds());
  }
  cout << endl;

  startTick = PTimer::Tick();
  while (threadCount > 0) {
    Test();
    Sleep(5000);
  }

  Test();
  sorted.RemoveAll();
  unsorted.RemoveAll();
  sparse.RemoveAll();
  Test();
}


void ThreadSafe::Test()
{
  sorted.DeleteObjectsToBeRemoved();
  unsorted.DeleteObjectsToBeRemoved();
  sparse.DeleteObjectsToBeRemoved();

  cout << setprecision(0) << setw(5) << (PTimer::Tick()-startTick)
       << " Threads=" << threadCount
       << ", Unsorted=" << unsorted.GetSize()
       << ", Sorted=" << sorted.GetSize()
       << ", Dictionary=" << sparse.GetSize()
       << ", Objects=";

  mutexObjects.Wait();
  cout << currentObjects << '/' << totalObjects;
  mutexObjects.Signal();

  cout << endl;
}


void ThreadSafe::TestThread(PThread &, INT duration)
{
  PRandom random;
  PSafePtr<TestObject> ptr;

  PTimer timeout = duration;

  while (timeout.IsRunning()) {
    switch (random%15) {
      case 0 :
        if (random%(unsorted.GetSize()+1) == 0)
          unsorted.Append(new TestObject(*this, random));
        break;

      case 1 :
        if (random%(sorted.GetSize()+1) == 0)
          sorted.Append(new TestObject(*this, random));
        break;

      case 2 :
        sparse.SetAt(random%20, new TestObject(*this, random));
        break;

      case 3 :
        for (ptr = unsorted.GetWithLock(0, PSafeReference); ptr != NULL; ++ptr) {
          if (random%50 == 0)
            unsorted.Remove(ptr);
        }
        break;

      case 4 :
        for (ptr = sorted.GetWithLock(0, PSafeReference); ptr != NULL; ++ptr) {
          if (random%50 == 0)
            sorted.Remove(ptr);
        }
        break;

      case 5 :
        sparse.RemoveAt(random%20);
        break;

      case 6 :
        for (ptr = unsorted; ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 7 :
        for (ptr = sorted; ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 8 :
        for (ptr = sparse; ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 9 :
        for (ptr = unsorted.GetWithLock(0, PSafeReadOnly); ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 10 :
        for (ptr = sorted.GetWithLock(0, PSafeReadOnly); ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 11 :
        for (ptr = sparse.GetWithLock(0, PSafeReadOnly); ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 12 :
        for (ptr = unsorted.GetWithLock(0, PSafeReference); ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 13 :
        for (ptr = sorted.GetWithLock(0, PSafeReference); ptr != NULL; ++ptr)
          Sleep(random%50);
        break;

      case 14 :
        for (ptr = sparse.GetWithLock(0, PSafeReference); ptr != NULL; ++ptr)
          Sleep(random%50);
        break;
    }
    Sleep(random%500);
  }

  threadCount--;
}


TestObject::TestObject(ThreadSafe & proc, unsigned val)
  : process(proc)
{
  value = val;

  process.mutexObjects.Wait();
  process.totalObjects++;
  process.currentObjects++;
  process.mutexObjects.Signal();
}


TestObject::~TestObject()
{
  process.mutexObjects.Wait();
  process.currentObjects--;
  process.mutexObjects.Signal();
}


PObject::Comparison TestObject::Compare(const PObject & obj)
{
  PAssert(obj.IsDescendant(Class()), PInvalidCast);
  unsigned othervalue = ((const TestObject &)obj).value;
  if (value < othervalue)
    return LessThan;
  if (value > othervalue)
    return GreaterThan;
  return EqualTo;
}


void TestObject::PrintOn(ostream & strm) const
{
  strm << value;
}


// End of File ///////////////////////////////////////////////////////////////
