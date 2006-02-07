/*
 * main.h
 *
 * PWLib application header file for safetest
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: main.h,v $
 * Revision 1.2  2006/02/07 02:02:00  dereksmithies
 * use a more sane method to keep track of the number of running DelayThread instances.
 *
 * Revision 1.1  2006/02/07 01:02:56  dereksmithies
 * Initial release of code to test the PSafeDictionary structure in pwlib.
 * Thanks to Indranet Technologies for supporting this work.
 *
 *
 *
 */

#ifndef _SafeTest_MAIN_H
#define _SafeTest_MAIN_H
#include <ptlib/safecoll.h>

class SafeTest;

/**This class is a simple simple thread that just creates, waits a
   period of time, and exits.It is designed to test the PwLib methods
   for reporting the status of a thread. This class will be created
   over and over- millions of times is possible if left long
   enough. */
class DelayThread : public PSafeObject
{
  PCLASSINFO(DelayThread, PSafeObject);
  
public:
  DelayThread(SafeTest & _safeTest, PINDEX _delay);
    
  ~DelayThread();

  /**Last thing the thread which runs in this class does, and initiates the close down */
  void Release();

  PString GetId() { return id; }

  virtual void PrintOn(ostream & strm) const;
 protected:

#ifdef DOC_PLUS_PLUS
  /**This method is where the delay is done */
    virtual void DelayThreadMain(PThread &, INT);
#else
    PDECLARE_NOTIFIER(PThread, DelayThread, DelayThreadMain);
#endif

#ifdef DOC_PLUS_PLUS
  /**This contains the 1 notifier that is used when closing down this
     instance of DelayThread class. It is called by a custom
     thread, which deletes this class  */
    virtual void OnReleaseThreadMain(PThread &, INT);
#else
    PDECLARE_NOTIFIER(PThread, DelayThread, OnReleaseThreadMain);
#endif
    
    SafeTest & safeTest;

    PINDEX delay;

    PString id;
};

////////////////////////////////////////////////////////////////////////////////
/**This thread handles the Users console requests to query the status of 
   the launcher thread. It provides a means for the user to close down this
   program - without having to use Ctrl-C*/
class UserInterfaceThread : public PThread
{
  PCLASSINFO(UserInterfaceThread, PThread);
  
public:
  UserInterfaceThread(SafeTest &_safeTest)
    : PThread(10000, NoAutoDeleteThread), safeTest(_safeTest)
    { }

  void Main();
    
 protected:
  SafeTest & safeTest;
};

///////////////////////////////////////////////////////////////////////////////
/**This thread launches multiple instances of the BusyWaitThread. Each
   thread launched is busy monitored for termination. When the thread
   terminates, the thread is deleted, and a new one is created. This
   process repeats until segfault or termination by the user */
class LauncherThread : public PThread
{
  PCLASSINFO(LauncherThread, PThread);
  
public:
  LauncherThread(SafeTest &_safe_test);
  
  void Main();
    
  PINDEX GetIteration() { return iteration; }

  virtual void Terminate() { keepGoing = FALSE; }

  PTimeInterval GetElapsedTime() { return PTime() - startTime; }

 protected:
  SafeTest & safeTest;

  PINDEX iteration;
  PTime startTime;
  BOOL  keepGoing;
};

////////////////////////////////////////////////////////////////////////////////


class SafeTest : public PProcess
{
  PCLASSINFO(SafeTest, PProcess)

  public:
    SafeTest();
    virtual void Main();

    PINDEX Delay()    { return delay; }

    PINDEX ActiveCount() { return activeCount; }
    /**Callback for removing a DelayThread from the list of active
       delaythreads */
    void OnReleased(DelayThread & delayThread);
                                                                                
    /**Append this DelayThread to delayThreadsActive, cause it is a valid
       and running DelayThread */
    void AppendRunning(PSafePtr<DelayThread> delayThread, PString id);

    /**Number of active DelayThread s*/
    PINDEX CurrentSize() { return currentSize; }

 protected:

    /**The thread safe list of DelayThread s that we manage */
    class DelayThreadsDict : public PSafeDictionary<PString, DelayThread>
    {
      /**One function that we have to define for this usage, and that
         is the deletion of existing methods. PWLib will have removed
         the object to be deleted from the list. We simply need to
         delete it. */
        virtual void DeleteObject(PObject * object) const;
    } delayThreadsActive;

 /**The thread which runs to look for DelayThread s to delete */
    PThread    *garbageCollector;
 
    /**The flag to say when we exit */
    BOOL exitNow;
     
    /**The method which does the garbage collection */
    void CollectGarbage();
 
#ifdef DOC_PLUS_PLUS
    /**This contains the 1 second delay loop, and calls to
       CollectGarbage */
    virtual void GarbageMain(PThread &, INT);
#else
    PDECLARE_NOTIFIER(PThread, SafeTest, GarbageMain);
#endif

    /**The delay each thread has to wait for */
    PINDEX delay;

    /**The number of threads that can be active */
    PINDEX activeCount;

    /**The number of entries in the dictionary */
    PAtomicInteger currentSize;
};



#endif  // _SafeTest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
