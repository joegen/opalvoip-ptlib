/*
 * mutex.h
 *
 * Mutual exclusion thread synchronisation class.
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
 * basis, WITHOUT WARRANTY OF ANY KIND, eitF ANY KIND, either express or implied. See
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
 * $Revision$
 * $Author$
 * $Date$
 */

///////////////////////////////////////////////////////////////////////////////
// PMutex

#if defined(P_PTHREADS) || defined(VX_TASKS)
    virtual ~PTimedMutex();
    mutable pthread_mutex_t mutex;
#endif

#if defined(P_PTHREADS) || defined(__BEOS__) || defined(P_MAC_MPTHREADS) || defined(VX_TASKS)
    virtual void Wait();
    virtual BOOL Wait(const PTimeInterval & timeout);
    virtual void Signal();
    virtual BOOL WillBlock() const;

  protected:

#  if defined(P_PTHREADS) && !defined(VX_TASKS)
#    if P_HAS_RECURSIVE_MUTEX == 0
       mutable PAtomicInteger lockCount;
#    endif
#  endif

#endif


// End Of File ////////////////////////////////////////////////////////////////
