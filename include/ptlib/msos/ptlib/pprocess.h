/*
 * $Id: pprocess.h,v 1.16 1997/02/05 11:49:37 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.16  1997/02/05 11:49:37  robertj
 * Changed current process function to return reference and validate objects descendancy.
 *
 * Revision 1.15  1996/10/08 13:05:18  robertj
 * Rewrite to use standard window isntead of console window.
 *
 * Revision 1.14  1996/08/17 10:00:34  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.13  1996/08/08 10:09:08  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.12  1996/07/27 04:09:23  robertj
 * Changed thread creation to use C library function instead of direct WIN32.
 *
 * Revision 1.11  1996/06/13 13:32:09  robertj
 * Rewrite of auto-delete threads, fixes Windows95 total crash.
 *
 * Revision 1.10  1996/05/23 10:02:41  robertj
 * Changed process.h to pprocess.h to avoid name conflict.
 *
 * Revision 1.9  1996/03/31 09:08:04  robertj
 * Added mutex to thread dictionary access.
 *
 * Revision 1.8  1996/03/12 11:31:06  robertj
 * Moved PProcess destructor to platform dependent code.
 *
 * Revision 1.7  1995/12/10 11:48:08  robertj
 * Fixed bug in application shutdown of child threads.
 *
 * Revision 1.6  1995/04/25 11:17:11  robertj
 * Fixes for DLL use in WIN32.
 *
 * Revision 1.5  1995/03/12 04:59:57  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.4  1994/07/27  06:00:10  robertj
 * Backup
 *
 * Revision 1.3  1994/07/21  12:35:18  robertj
 * *** empty log message ***
 *
 * Revision 1.2  1994/07/02  03:18:09  robertj
 * Prevent WinMain in pure MSDOS versions.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 *
 */


#ifndef _PPROCESS


///////////////////////////////////////////////////////////////////////////////
// PProcess

#include "../../common/ptlib/pprocess.h"
  public:
    ~PProcess();

#if defined(P_PLATFORM_HAS_THREADS)

    void SignalTimerChange();
    // Signal to the timer thread that a change was made.

  private:
    PDICTIONARY(ThreadDict, POrdinalKey, PThread);
    ThreadDict activeThreads;
    PSemaphore threadMutex;

    PDECLARE_CLASS(HouseKeepingThread, PThread)
        public:
        HouseKeepingThread();
        void Main();
        PSemaphore semaphore;
    };
    HouseKeepingThread * houseKeeper;
    // Thread for doing timers, thread clean up etc.

  friend PThread * PThread::Current();
  friend void HouseKeepingThread::Main();
  friend UINT __stdcall PThread::MainFunction(void * thread);
  friend class PServiceProcess;

#endif

#if defined(_WINDOWS) || defined(_WIN32)
  friend int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
#endif
};


#ifdef PMAKEDLL
extern __declspec(dllexport) PProcess * PProcessInstance;
#elif PUSEDLL
extern __declspec(dllimport) PProcess * PProcessInstance;
#else
extern PProcess * PProcessInstance;
#endif


inline PProcess & PProcess::Current()
  { return *PProcessInstance; }


#endif
