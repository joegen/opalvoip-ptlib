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
 * $Id: ptlib.inl 21219 2008-10-02 08:52:00Z rjongbloed $
 */


///////////////////////////////////////////////////////////////////////////////
// PTimer

#if !defined(_WIN32)

#if CLOCKS_PER_SEC==1000

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

#endif


///////////////////////////////////////////////////////////////////////////////
// PDirectory

PINLINE bool PDirectory::IsSeparator(char c)
  { return c == ':' || c == '/' || c == '\\'; }

PINLINE bool PDirectory::Remove(const PString & p)
  { return _rmdir(p) == 0; }


PINLINE bool PDirectory::Restart(PFileInfo::FileTypes scanMask)
  { return Open(scanMask); }



///////////////////////////////////////////////////////////////////////////////
// PFile

PINLINE bool PFile::Exists(const PFilePath & name)
  { return _access(name, 0) == 0; }

PINLINE bool PFile::Remove(const PFilePath & name, bool force)
  { return PFile::Remove((const PString &)name, force); }


///////////////////////////////////////////////////////////////////////////////
// PThread

PINLINE PThreadIdentifier PThread::GetCurrentThreadId()
  { return ::GetCurrentThreadId(); }

///////////////////////////////////////////////////////////////////////////////
// PCriticalSection

PINLINE PCriticalSection::PCriticalSection()
{
  ::InitializeCriticalSection(&criticalSection);
}

PINLINE PCriticalSection::PCriticalSection(const PCriticalSection &)
{
  ::InitializeCriticalSection(&criticalSection);
}

PINLINE PCriticalSection::~PCriticalSection()
{
  ::DeleteCriticalSection(&criticalSection);
}

PINLINE void PCriticalSection::Wait()
{
  ::EnterCriticalSection(&criticalSection);
}

PINLINE void PCriticalSection::Signal()
{
  ::LeaveCriticalSection(&criticalSection);
}

PINLINE bool PCriticalSection::Try()
{
  return TryEnterCriticalSection(&criticalSection) != 0;
}


// End Of File ///////////////////////////////////////////////////////////////
