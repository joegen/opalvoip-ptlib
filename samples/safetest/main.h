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
 * Revision 1.7  2006/03/22 04:24:51  dereksmithies
 * Tidyups. Add Pragmas. make it slightly more friendly for 1 cpu boxes.
 *
 * Revision 1.6  2006/02/13 04:17:23  dereksmithies
 * Formatting fixes.
 *
 * Revision 1.5  2006/02/12 21:42:07  dereksmithies
 * Add lots of doxygen style comments, and an introductory page.
 *
 * Revision 1.4  2006/02/09 21:43:15  dereksmithies
 * Remove the notion of CleanerThread. This just confuses things.
 *
 * Revision 1.3  2006/02/09 21:07:23  dereksmithies
 * Add new (and temporary) thread to close down each DelayThread instance.
 * Now, it is less cpu intensive. No need for garbage thread to run.
 *
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


#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include  <ptclib/random.h>

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
  /**Create this class, so it runs for the specified delay period, The
     class is assigned a unique ID tag, based on the parameter
     idNumber */
  DelayThread(SafeTest & _safeTest, PINDEX _delay, PInt64 idNumber);
    
  /**Destroy this class. Includes a check to see if this class is
     still running*/
  ~DelayThread();

  /**Last thing the thread which runs in this class does, and
     initiates the close down */
  void Release();

  /**Report the id used by this class */
  PString GetId() { return id; }

  /**Pretty print the id of this class */
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
     instance of DelayThread class. It is called by a custom thread,
     which initiates the deletion of this Delaythread instance */
    virtual void OnReleaseThreadMain(PThread &, INT);
#else
    PDECLARE_NOTIFIER(PThread, DelayThread, OnReleaseThreadMain);
#endif
    /**Reference back to the class that knows everything, and holds the list
       of instances of this DelayThread class */
    SafeTest & safeTest;

    /**The time period (ms) we have to wait for in this thread */
    PINDEX delay;

    /**the label assigned to this instance of this DelayThread class */
    PString id;

    /**Flag to indicate we are still going */
    BOOL threadRunning;
};

////////////////////////////////////////////////////////////////////////////////
/**This thread handles the Users console requests to query the status of 
   the launcher thread. It provides a means for the user to close down this
   program - without having to use Ctrl-C*/
class UserInterfaceThread : public PThread
{
  PCLASSINFO(UserInterfaceThread, PThread);
  
public:
  /**Constructor */
  UserInterfaceThread(SafeTest &_safeTest)
    : PThread(10000, NoAutoDeleteThread), safeTest(_safeTest)
    { }

  /**Do the work of listening for user commands from the command line */
  void Main();
    
 protected:
  /**Reference to the class that holds the key data on everything */
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
  /**Create this thread. Note that the program only contains one
     instance of this thread */
  LauncherThread(SafeTest &_safe_test);
  
  /**Where all the work is done */
  void Main();
    
  /**Access function, which is callled by the UserInterfaceThread.  It
   reports the number of DelayThread instances that have been
   created.*/
  PInt64 GetIteration() { return iteration; }

  /**Cause this thread to stop work and end */
  virtual void Terminate() { keepGoing = FALSE; }

  /**Access function, which is callled by the UserInterfaceThread.
   It reports the time since this program stared.*/
  PTimeInterval GetElapsedTime() { return PTime() - startTime; }

 protected:
  /**Reference back to the master class */
  SafeTest & safeTest;

  /**Count on the number of DelayThread instances that have been
     created */
  PInt64          iteration;
  
  /**The time at which this program was started */
  PTime           startTime;

  /**A flag to indicate that we have to keep on doing our job, of
     launching DelayThread instances */
  BOOL            keepGoing;
};

////////////////////////////////////////////////////////////////////////////////

/**
   The core class that a)processes command line and b)launches
   relevant classes. This class contains a wrapper for the 
   \code int main(int argc, char **argv); \endcode function */
class SafeTest : public PProcess
{
  PCLASSINFO(SafeTest, PProcess)

  public:
  /**Constructor */
    SafeTest();

  /**Where execution first starts in this program, and it does all the
     command line processing */
    virtual void Main();

    /**Report the user specified delay, which is used in DelayThread
       instances. Units are in milliseconds */
    PINDEX Delay()    { return delay; }

    /**Report the user specified number of DelayThreads that can be in
       action. */
    PINDEX ActiveCount() { return activeCount; }

    /**Callback for removing a DelayThread from the list of active
       delaythreads */
    void OnReleased(DelayThread & delayThread);

    /**Append this DelayThread to delayThreadsActive, cause it is a valid
       and running DelayThread */
    void AppendRunning(PSafePtr<DelayThread> delayThread, PString id);

    /**Number of active DelayThread s*/
    PINDEX CurrentSize() { return currentSize; }

    /**Find a DelayThread instance witth the specified token.
 
       Note the caller of this function MUST call the DelayThread::Unlock()
       function if this function returns a non-NULL pointer. If it does not
       then a deadlock can occur.
      */
    PSafePtr<DelayThread> FindDelayThreadWithLock(
      const PString & token,  ///<  Token to identify connection
      PSafetyMode mode = PSafeReadWrite
    ) { return delayThreadsActive.FindWithLock(token, mode); }

    /**The method which does the garbage collection (i.e. removes old
       dead DelayThread instances)*/
    void CollectGarbage();

    /**Return a random number, of size 0 .. (delay/4), for use in
       making the delay threads random in duration. */
    PINDEX GetRandom() { return random.Generate() % (delay >> 2); }
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

    /**Create the Garbage collector thread */
    void MakeGarbageCollector();

 /**The thread which runs to look for DelayThread s to delete */
    PThread    *garbageCollector;
 
    /**The flag to say when we exit */
    BOOL exitNow;
     
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

    /**Random number generator, which is used to keep track of the
       variability in the delay period */
    PRandom random;
};



#endif  // _SafeTest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
