/*
 * $Id: ptlib.inl,v 1.1 1994/04/01 14:38:42 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993, Equivalence
 *
 * $Log: ptlib.inl,v $
 * Revision 1.1  1994/04/01 14:38:42  robertj
 * Initial revision
 *
 */


///////////////////////////////////////////////////////////////////////////////
// PTimer

PINLINE PMilliseconds PTimer::Tick()
  { return clock()*CLOCKS_PER_SEC/1000; }

PINLINE unsigned PTimer::Resolution()
  { return 1000/CLOCKS_PER_SEC; }



///////////////////////////////////////////////////////////////////////////////
// PDirectory

PINLINE BOOL PDirectory::Create(const PString & p, int)
  { return mkdir(p) == 0; }

PINLINE BOOL PDirectory::Remove(const PString & p)
  { return rmdir(p) == 0; }


PINLINE BOOL PDirectory::Restart(int scanMask)
  { return Open(scanMask); }



///////////////////////////////////////////////////////////////////////////////
// PFile

PINLINE PString PFile::GetVolume() const
  { return fullname(0, fullname.Find(':')); }

PINLINE PString PFile::GetPath() const
  { return fullname(fullname.Find(':')+1, fullname.FindLast('\\')); }

PINLINE PString PFile::GetTitle() const
  { return fullname(fullname.FindLast('\\')+1, fullname.FindLast('.')-1); }

PINLINE PString PFile::GetType() const
  { return fullname(fullname.FindLast('.'), P_MAX_INDEX); }

PINLINE PString PFile::GetFileName() const
  { return fullname(fullname.FindLast('\\')+1, P_MAX_INDEX); }

PINLINE BOOL PFile::Exists(const PString & name)
  { return _access(name, 0) == 0; }

PINLINE BOOL PFile::Remove(const PString & name)
  { return unlink(name) == 0; }

PINLINE BOOL PFile::Rename(const PString & oldname, const PString & newname)
  { return rename(oldname, newname) == 0; }


// End Of File ///////////////////////////////////////////////////////////////
