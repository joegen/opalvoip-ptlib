/*
 * semaphor.h
 *
 * Thread synchronisation semaphore class.
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
 * $Log: semaphor.h,v $
 * Revision 1.19  2003/09/17 01:18:03  csoutheren
 * Removed recursive include file system and removed all references
 * to deprecated coooperative threading support
 *
 * Revision 1.18  2002/10/10 04:43:44  robertj
 * VxWorks port, thanks Martijn Roest
 *
 * Revision 1.17  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.16  2002/06/09 16:36:34  rogerh
 * friend BOOL PThread::Terminate() should be type void (found by gcc 3.1)
 *
 * Revision 1.15  2002/01/23 04:26:36  craigs
 * Added copy constructors for PSemaphore, PMutex and PSyncPoint to allow
 * use of default copy constructors for objects containing instances of
 * these classes
 *
 * Revision 1.14  2001/09/20 05:38:25  robertj
 * Changed PSyncPoint to use pthread cond so timed wait blocks properly.
 * Also prevented semaphore from being created if subclass does not use it.
 *
 * Revision 1.13  2001/08/11 07:57:30  rogerh
 * Add Mac OS Carbon changes from John Woods <jfw@jfwhome.funhouse.com>
 *
 * Revision 1.12  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.11  2000/12/16 12:56:59  rogerh
 * BeOS changes, submitted by Yuri Kiryanov <openh323@kiryanov.com>
 *
 * Revision 1.10  1999/10/22 10:18:49  craigs
 * Changed semaphore.h inclusion to be dependent upon P_HAS_SEMAPHORE
 *
 * Revision 1.9  1999/09/23 06:52:16  robertj
 * Changed PSemaphore to use Posix semaphores.
 *
 * Revision 1.8  1999/03/02 05:41:58  robertj
 * More BeOS changes
 *
 * Revision 1.7  1999/01/09 03:35:56  robertj
 * Fixed problem with closing thread waiting on semaphore.
 *
 * Revision 1.6  1998/11/30 22:07:05  robertj
 * New directory structure.
 *
 * Revision 1.5  1998/09/24 04:11:52  robertj
 * Added open software license.
 *
 * Revision 1.4  1998/01/04 10:45:01  craigs
 * Added thread.h
 *
 * Revision 1.3  1998/01/03 23:06:32  craigs
 * Added PThread support
 *
 * Revision 1.2  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.1  1996/01/26 11:06:31  craigs
 * Initial revision
 *
 */

  public:
    unsigned GetInitial() const { return initialVar; }
    unsigned GetMaxCount() const     { return maxCountVar; }

  protected:
    unsigned initialVar;
    unsigned maxCountVar;

#if defined(P_MAC_MPTHREADS)
  protected:
    MPSemaphoreID semId;
#elif defined(P_PTHREADS)

    enum PXClass { PXSemaphore, PXMutex, PXSyncPoint } pxClass;
    PXClass GetSemClass() const { return pxClass; }

  protected:
    PSemaphore(PXClass);
    pthread_mutex_t mutex;
    pthread_cond_t  condVar;
    
#ifdef P_HAS_SEMAPHORES
    sem_t semId;
#else
    unsigned currentCount;
    unsigned maximumCount;
    unsigned queuedLocks;
  friend void PThread::Terminate();
#endif

#elif defined(BE_THREADS)

  public:
    PSemaphore( sem_id anId, int32 benaphoreCount);
  protected:
    sem_id semId;
    volatile int32 benaphoreCount;

#elif defined(VX_TASKS)

  public:
    PSemaphore( SEM_ID anId );
  protected:
    SEM_ID semId;

#else

  protected:
    PQUEUE(ThreadQueue, PThread);
    ThreadQueue waitQueue;

#endif

// End Of File ////////////////////////////////////////////////////////////////
