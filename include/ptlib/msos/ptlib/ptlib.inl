/*
 * $Id: ptlib.inl,v 1.11 1995/12/10 11:48:27 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993, Equivalence
 *
 * $Log: ptlib.inl,v $
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

#ifdef _WINDOWS

PINLINE PTimeInterval PTimer::Tick()
  { return GetTickCount()&0x7fffffff; }

PINLINE unsigned PTimer::Resolution()
#if defined(_WIN32)
  { return 1; }
#else
  { return 55; }
#endif

#else

PINLINE PTimeInterval PTimer::Tick()
  { return clock()*CLOCKS_PER_SEC/1000; }

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
// PFilePath

PINLINE PDirectory PFilePath::GetDirectory() const
  { return Left(FindLast('\\')); }


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

PINLINE PThread * PThread::Current()
  { return(PThread*)PProcess::Current()->threads.GetAt(GetCurrentThreadId()); }

PINLINE void PThread::Sleep(const PTimeInterval & delay)
  { ::Sleep(delay.GetMilliseconds()); }

#else

PINLINE BOOL PThread::IsOnlyThread() const
  { return link == this; }

PINLINE void PThread::AllocateStack(PINDEX stackSize)
  { stackBase = (char NEAR *)_nmalloc(stackSize); }

PINLINE void PThread::ClearBlock()
  { isBlocked = NULL; }

PINLINE BOOL PThread::IsNoLongerBlocked()
  { return !isBlocked(blocker); }

#endif


// End Of File ///////////////////////////////////////////////////////////////
