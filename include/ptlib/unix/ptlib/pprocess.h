/*
 * pprocess.h
 *
 * Operating System process (running program) class.
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
 * Revision 1.20  2001/03/20 06:44:25  robertj
 * Lots of changes to fix the problems with terminating threads that are I/O
 *   blocked, especially when doing orderly shutdown of service via SIGTERM.
 *
 * Revision 1.19  2001/03/14 01:16:11  robertj
 * Fixed signals processing, now uses housekeeping thread to handle signals
 *   synchronously. This also fixes issues with stopping PServiceProcess.
 *
 * Revision 1.18  1999/03/02 05:41:58  robertj
 * More BeOS changes
 *
 * Revision 1.17  1998/12/15 12:43:43  robertj
 * Fixed signal handling so can now ^C a pthread version.
 *
 * Revision 1.16  1998/11/30 22:06:59  robertj
 * New directory structure.
 *
 * Revision 1.15  1998/10/18 14:29:42  robertj
 * Renamed argv/argc to eliminate accidental usage.
 *
 * Revision 1.14  1998/10/16 11:27:58  robertj
 * Added access to argc/argv.
 *
 * Revision 1.13  1998/09/24 04:11:46  robertj
 * Added open software license.
 *
 * Revision 1.12  1998/05/30 13:30:44  robertj
 * Fixed shutdown problems with PConfig caching.
 *
 * Revision 1.11  1998/03/29 10:42:52  craigs
 * Made PConfig thread safe
 *
 * Revision 1.10  1998/03/26 04:55:53  robertj
 * Added PMutex and PSyncPoint
 *
 * Revision 1.9  1998/01/04 08:13:32  craigs
 * Removed extern reference to PProcessInstance
 *
 * Revision 1.8  1998/01/03 23:06:32  craigs
 * Added PThread support
 *
 * Revision 1.7  1996/10/31 10:28:38  craigs
 * Removed PXOnSigxxx routines
 *
 * Revision 1.6  1996/09/21 05:42:12  craigs
 * Changes for new common files, PConfig changes and signal handling
 *
 * Revision 1.5  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.4  1996/06/10 11:03:23  craigs
 * Changed include name
 *
 * Revision 1.3  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.2  1996/01/26 11:06:31  craigs
 * Added signal handlers
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PPROCESS

#pragma interface

#include <ptlib/syncpoint.h>

PDICTIONARY(PXFdDict,    POrdinalKey, PThread);


///////////////////////////////////////////////////////////////////////////////
// PProcess

#include "../../pprocess.h"
  public:
    friend class PApplication;
    friend class PServiceProcess;
    friend void PXSignalHandler(int);
    friend class PHouseKeepingThread;

    ~PProcess();

    PDirectory PXGetHomeDir ();
    char ** PXGetArgv() const { return p_argv; }
    int     PXGetArgc() const { return p_argc; }
    char ** PXGetEnvp() const { return p_envp; }

    friend void PXSigHandler(int);
    virtual void PXOnSignal(int);
    virtual void PXOnAsyncSignal(int);
    void         PXCheckSignals();

    static void PXShowSystemWarning(PINDEX code);
    static void PXShowSystemWarning(PINDEX code, const PString & str);

  protected:
    void         CommonConstruct();
    void         CommonDestruct();

    virtual void _PXShowSystemWarning(PINDEX code, const PString & str);
    int pxSignals;

  protected:
    void CreateConfigFilesDictionary();
    PAbstractDictionary * configFiles;


#ifdef P_PTHREADS

  public:
    void SignalTimerChange();

  protected:
    PDICTIONARY(ThreadDict, POrdinalKey, PThread);
    ThreadDict activeThreads;
    PMutex     threadMutex;
    int        timerChangePipe[2];
    PHouseKeepingThread * housekeepingThread;

#elif defined(BE_THREADS)

  public:
    void SignalTimerChange();

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
    friend class HouseKeepingThread;
    HouseKeepingThread * houseKeeper;
    // Thread for doing timers, thread clean up etc.

  friend PThread * PThread::Current();
  friend int32 PThread::ThreadFunction(void * thread);

#else

  public:
    void PXAbortIOBlock(int fd);
  protected:
    PXFdDict     ioBlocks[3];
#endif
};

#endif
