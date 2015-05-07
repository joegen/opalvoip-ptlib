/*
 * ptlib.inl
 *
 * Operating System classes inline function implementation
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
 * $Id: ptlib.inl 19008 2007-11-29 09:17:41Z rjongbloed $
 */

#if defined(P_LINUX) || defined(P_GNU_HURD)
#if (__GNUC_MINOR__ < 7 && __GNUC__ <= 2)
#include <localeinfo.h>
#else
#define P_USE_LANGINFO
#endif
#elif defined(P_HPUX9)
#define P_USE_LANGINFO
#elif defined(P_SUN4)
#endif

#ifdef P_USE_LANGINFO
#include <langinfo.h>
#endif

PINLINE PProcessIdentifier PProcess::GetCurrentProcessID()
{
#ifdef P_VXWORKS
  return PThread::Current().PX_threadId;
#else
  return getpid();
#endif // P_VXWORKS
}

///////////////////////////////////////////////////////////////////////////////

PINLINE unsigned PTimer::Resolution()
{
  return 1;
}

///////////////////////////////////////////////////////////////////////////////

PINLINE bool PDirectory::IsRoot() const
  { return IsSeparator((*this)[0]) && ((*this)[1] == '\0'); }

PINLINE PDirectory PDirectory::GetRoot() const
  { return PString(PDIR_SEPARATOR); }

PINLINE bool PDirectory::IsSeparator(char ch)
  { return ch == PDIR_SEPARATOR; }

PINLINE bool PDirectory::Change(const PString & p)
  { return chdir((char *)(const char *)p) == 0; }

///////////////////////////////////////////////////////////////////////////////

PINLINE PString PFilePath::GetVolume() const
  { return PString::Empty(); }

///////////////////////////////////////////////////////////////////////////////

PINLINE bool PFile::Remove(const PFilePath & name, bool)
  { return unlink((char *)(const char *)name) == 0; }

PINLINE bool PFile::Remove(const PString & name, bool)
  { return unlink((char *)(const char *)name) == 0; }

///////////////////////////////////////////////////////////////////////////////

#ifdef BE_THREADS
PINLINE PThreadIdentifier PThread::GetThreadId() const { return mId; }
#elif defined(P_PTHREADS)
PINLINE PThreadIdentifier PThread::GetCurrentThreadId() { return ::pthread_self(); }
#elif defined(VX_TASKS)
PINLINE PThreadIdentifier PThread::GetCurrentThreadId() { return ::taskIdSelf(); }
#else
PINLINE PThreadIdentifier PThread::GetCurrentThreadId() { return 0; }
#endif


///////////////////////////////////////////////////////////////////////////////
// PCriticalSection

#if defined(P_PTHREADS) || defined(VX_TASKS)

PINLINE PCriticalSection::PCriticalSection()
{
  PMutex::InitialiseRecursiveMutex(&m_mutex);
}

PINLINE PCriticalSection::PCriticalSection(const PCriticalSection &)
{
  PMutex::InitialiseRecursiveMutex(&m_mutex);
}

PINLINE PCriticalSection::~PCriticalSection()
{
  ::pthread_mutex_destroy(&m_mutex);
}

PINLINE void PCriticalSection::Wait()
{
  ::pthread_mutex_lock(&m_mutex);
}

PINLINE void PCriticalSection::Signal()
{
  ::pthread_mutex_unlock(&m_mutex);
}

PINLINE bool PCriticalSection::Try()
{
  return ::pthread_mutex_trylock(&m_mutex) != 0;
}

#endif


// End Of File ///////////////////////////////////////////////////////////////
