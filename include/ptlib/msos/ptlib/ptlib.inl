/*
 * ptlib.inl
 *
 * Non-GUI classes inline function implementation.
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
 * $Log: ptlib.inl,v $
 * Revision 1.18  1998/11/14 23:37:06  robertj
 * Fixed file path directory extraction, not able to return root directory
 *
 * Revision 1.17  1998/09/24 03:30:19  robertj
 * Added open software license.
 *
 * Revision 1.16  1996/08/20 12:10:36  robertj
 * Fixed bug in timers wrapping unexpectedly and producing fast timeout.
 *
 * Revision 1.15  1996/07/20 05:32:26  robertj
 * MSVC 4.1 compatibility.
 *
 * Revision 1.14  1996/05/15 10:23:25  robertj
 * Changed millisecond access functions to get 64 bit integer.
 *
 * Revision 1.13  1996/03/31 09:08:23  robertj
 * Added mutex to thread dictionary access.
 *
 * Revision 1.12  1996/03/04 12:38:56  robertj
 * Moved calculation of stackTop to platform dependent code.
 *
 * Revision 1.11  1995/12/10 11:48:27  robertj
 * Fixed bug in application shutdown of child threads.
 *
 * Revision 1.10  1995/04/22 00:52:55  robertj
 * Added GetDirectory() function to PFilePath.
 *
 * Revision 1.9  1995/03/12 04:59:58  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.8  1994/12/21  11:55:09  robertj
 * Fixed file paths returning correct string type.
 *
 * Revision 1.7  1994/10/23  05:38:57  robertj
 * PipeChannel implementation.
 * Added directory exists function.
 *
 * Revision 1.6  1994/08/22  00:18:02  robertj
 * Renamed CheckBlock() to IsNoLongerBlocked()
 *
 * Revision 1.5  1994/07/27  06:00:10  robertj
 * Backup
 *
 * Revision 1.4  1994/07/21  12:35:18  robertj
 * *** empty log message ***
 *
 * Revision 1.3  1994/07/02  03:18:09  robertj
 * Multi-threading support.
 * Fixed bug in time intervals being signed.
 *
 * Revision 1.2  1994/06/25  12:13:01  robertj
 * Synchronisation.
 *
 * Revision 1.1  1994/04/01  14:38:42  robertj
 * Initial revision
 */

#include <direct.h>


///////////////////////////////////////////////////////////////////////////////
// PTimer

#if defined(_WINDOWS) || defined(_WIN32)

PINLINE PTimeInterval PTimer::Tick()
  { return (int)(GetTickCount()&0x7fffffff); }

PINLINE unsigned PTimer::Resolution()
#if defined(_WIN32)
  { return 1; }
#else
  { return 55; }
#endif

#elif CLOCKS_PER_SEC==1000

PINLINE PTimeInterval PTimer::Tick()
  { return clock(); }

PINLINE unsigned PTimer::Resolution()
  { return 1; }

#else

PINLINE PTimeInterval PTimer::Tick()
  { return (PInt64)clock()*CLOCKS_PER_SEC/1000; }

PINLINE unsigned PTimer::Resolution()
  { return 1000/CLOCKS_PER_SEC; }

#endif


///////////////////////////////////////////////////////////////////////////////
// PDirectory

PINLINE BOOL PDirectory::IsSeparator(char c)
  { return c == ':' || c == '/' || c == '\\'; }

PINLINE BOOL PDirectory::Exists(const PString & p)
  { return _access(p+".", 0) == 0; }

PINLINE BOOL PDirectory::Create(const PString & p, int)
  { return _mkdir(p) == 0; }

PINLINE BOOL PDirectory::Remove(const PString & p)
  { return _rmdir(p) == 0; }


PINLINE BOOL PDirectory::Restart(int scanMask)
  { return Open(scanMask); }



///////////////////////////////////////////////////////////////////////////////
// PFile

PINLINE BOOL PFile::Exists(const PFilePath & name)
  { return _access(name, 0) == 0; }


///////////////////////////////////////////////////////////////////////////////
// PPipeChannel

PINLINE BOOL PPipeChannel::CanReadAndWrite()
#if defined(_WIN32)
  { return TRUE; }
#else
  { return FALSE; }
#endif


///////////////////////////////////////////////////////////////////////////////
// PThread

#if defined(_WIN32)

PINLINE void PThread::Sleep(const PTimeInterval & delay)
  { ::Sleep(delay.GetInterval()); }

#else

PINLINE BOOL PThread::IsOnlyThread() const
  { return link == this; }

PINLINE void PThread::AllocateStack(PINDEX stackSize)
  { stackTop = (stackBase = (char NEAR *)_nmalloc(stackSize)) + stackSize; }

PINLINE void PThread::ClearBlock()
  { isBlocked = NULL; }

PINLINE BOOL PThread::IsNoLongerBlocked()
  { return !isBlocked(blocker); }

#endif


// End Of File ///////////////////////////////////////////////////////////////
