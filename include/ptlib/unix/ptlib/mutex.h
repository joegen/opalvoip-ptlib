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
 * $Log: mutex.h,v $
 * Revision 1.5  1999/03/05 07:03:27  robertj
 * Some more BeOS port changes.
 *
 * Revision 1.4  1999/01/09 03:35:09  robertj
 * Improved efficiency of mutex to use pthread functions directly.
 *
 * Revision 1.3  1998/11/30 22:06:51  robertj
 * New directory structure.
 *
 * Revision 1.2  1998/09/24 04:11:41  robertj
 * Added open software license.
 *
 * Revision 1.1  1998/03/24 07:31:04  robertj
 * Initial revision
 *
 */


#ifndef _PMUTEX


///////////////////////////////////////////////////////////////////////////////
// PMutex

#include "../../mutex.h"
#if defined(P_PTHREADS) || defined(BE_THREADS)
  public:
    virtual void Wait();
    virtual BOOL Wait(const PTimeInterval & timeout);
    virtual void Signal();
    virtual BOOL WillBlock() const;
#endif
#ifdef BE_THREADS
  protected:
  	int32 benaphoreCount;
#endif
};


#endif
