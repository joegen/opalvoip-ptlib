/*
 * pprocess.h
 *
 * Operating system process (running program) class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pprocess.h,v $
 * Revision 1.22  1998/09/24 03:30:16  robertj
 * Added open software license.
 *
 * Revision 1.21  1998/04/07 13:32:58  robertj
 * Changed startup code to support PApplication class.
 *
 * Revision 1.20  1998/04/01 01:53:24  robertj
 * Fixed problem with NoAutoDelete threads.
 *
 * Revision 1.19  1998/03/20 03:18:27  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.18  1997/07/08 13:01:12  robertj
 * DLL support.
 *
 * Revision 1.17  1997/04/27 05:50:25  robertj
 * DLL support.
 *
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

#include <syncpoint.h>

extern "C" int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);


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
    PMutex activeThreadMutex;
    PLIST(ThreadList, PThread);
    ThreadList autoDeleteThreads;
    PMutex deleteThreadMutex;

    PDECLARE_CLASS(HouseKeepingThread, PThread)
        public:
        HouseKeepingThread();
        void Main();
        PSyncPoint breakBlock;
    };
    HouseKeepingThread * houseKeeper;
    // Thread for doing timers, thread clean up etc.

  friend PThread * PThread::Current();
  friend void HouseKeepingThread::Main();
  friend UINT __stdcall PThread::MainFunction(void * thread);
  friend class PServiceProcess;
  friend class PApplication;
  friend int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

#endif
};


#endif
