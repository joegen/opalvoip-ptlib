/*
 * $Id: ptlib.inl,v 1.8 1994/12/21 11:55:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993, Equivalence
 *
 * $Log: ptlib.inl,v $
 * Revision 1.8  1994/12/21 11:55:09  robertj
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



///////////////////////////////////////////////////////////////////////////////
// PTimer

#ifdef _WINDOWS

PINLINE PTimeInterval PTimer::Tick()
  { return GetTickCount()&0x7fffffff; }

PINLINE unsigned PTimer::Resolution()
  { return 55; }

#else

PINLINE PTimeInterval PTimer::Tick()
  { return clock()*CLOCKS_PER_SEC/1000; }

PINLINE unsigned PTimer::Resolution()
  { return 1000/CLOCKS_PER_SEC; }

#endif


///////////////////////////////////////////////////////////////////////////////
// PDirectory

PINLINE BOOL PDirectory::Exists(const PString & p)
  { return _access(p+".", 0) == 0; }

PINLINE BOOL PDirectory::Create(const PString & p, int)
  { return mkdir(p) == 0; }

PINLINE BOOL PDirectory::Remove(const PString & p)
  { return rmdir(p) == 0; }


PINLINE BOOL PDirectory::Restart(int scanMask)
  { return Open(scanMask); }



///////////////////////////////////////////////////////////////////////////////
// PFilePath

PINLINE PCaselessString PFilePath::GetVolume() const
  { return Left(Find(':')+1); }

PINLINE PCaselessString PFilePath::GetPath() const
  { return operator()(Find(':')+1, FindLast('\\')); }

PINLINE PCaselessString PFilePath::GetTitle() const
  { return operator()(FindLast('\\')+1, FindLast('.')-1); }

PINLINE PCaselessString PFilePath::GetType() const
  { return operator()(FindLast('.'), P_MAX_INDEX); }

PINLINE PCaselessString PFilePath::GetFileName() const
  { return operator()(FindLast('\\')+1, P_MAX_INDEX); }


///////////////////////////////////////////////////////////////////////////////
// PFile

PINLINE BOOL PFile::Exists(const PString & name)
  { return _access(name, 0) == 0; }


///////////////////////////////////////////////////////////////////////////////
// PPipeChannel

PINLINE BOOL PPipeChannel::CanReadAndWrite()
  { return FALSE; }


///////////////////////////////////////////////////////////////////////////////
// PThread

PINLINE BOOL PThread::IsOnlyThread() const
  { return link == this; }

PINLINE void PThread::AllocateStack(PINDEX stackSize)
  { stackBase = (char NEAR *)_nmalloc(stackSize); }

PINLINE void PThread::ClearBlock()
  { isBlocked = NULL; }

PINLINE BOOL PThread::IsNoLongerBlocked()
  { return !isBlocked(blocker); }


///////////////////////////////////////////////////////////////////////////////
// PProcess

PINLINE PProcess::~PProcess()
  { }

PINLINE void PProcess::OperatingSystemYield()
  { }


// End Of File ///////////////////////////////////////////////////////////////
