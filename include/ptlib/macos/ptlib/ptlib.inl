/*
 * $Id: ptlib.inl,v 1.2 1996/05/09 12:23:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993, Equivalence
 *
 * $Log: ptlib.inl,v $
 * Revision 1.2  1996/05/09 12:23:02  robertj
 * Further implementation.
 *
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */


///////////////////////////////////////////////////////////////////////////////
// PTimer

PINLINE PTimeInterval PTimer::Tick()
  { return (long)(clock()*CLOCKS_PER_SEC/1000); }

PINLINE unsigned PTimer::Resolution()
  { return 1000/CLOCKS_PER_SEC; }


///////////////////////////////////////////////////////////////////////////////
// PDirectory

PINLINE BOOL PDirectory::IsSeparator(char c)
  { return c == ':'; }

PINLINE BOOL PDirectory::Restart(int scanMask)
  { return Open(scanMask); }



///////////////////////////////////////////////////////////////////////////////
// PFilePath

PINLINE PDirectory PFilePath::GetDirectory() const
  { return Left(FindLast('\\')); }


///////////////////////////////////////////////////////////////////////////////
// PPipeChannel

PINLINE BOOL PPipeChannel::CanReadAndWrite()
  { return FALSE; }


///////////////////////////////////////////////////////////////////////////////
// PThread

PINLINE BOOL PThread::IsOnlyThread() const
  { return link == this; }

PINLINE void PThread::AllocateStack(PINDEX stackSize)
  { stackBase = (char *)malloc(stackSize); }

PINLINE void PThread::ClearBlock()
  { isBlocked = NULL; }

PINLINE BOOL PThread::IsNoLongerBlocked()
  { return !isBlocked(blocker); }


// End Of File ///////////////////////////////////////////////////////////////
