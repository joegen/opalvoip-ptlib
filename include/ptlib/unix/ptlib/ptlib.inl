/*
 * osutil.inl
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
 * $Log: ptlib.inl,v $
 * Revision 1.18  1998/11/24 09:38:22  robertj
 * FreeBSD port.
 *
 * Revision 1.17  1998/11/10 12:59:18  robertj
 * Fixed strange problems with readdir_r usage.
 *
 * Revision 1.16  1998/10/18 10:02:47  robertj
 * Fixed program argument access functions.
 *
 * Revision 1.15  1998/09/24 04:11:49  robertj
 * Added open software license.
 *
 */

#if defined(P_LINUX)
#if (__GNUC_MINOR__ < 7)
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

PINLINE DWORD PProcess::GetProcessID() const
{
  return (DWORD)getpid();
}

///////////////////////////////////////////////////////////////////////////////

PINLINE unsigned PTimer::Resolution()
#if defined(P_SUN4)
  { return 1000; }
#else
  { return (unsigned)(1000/CLOCKS_PER_SEC); }
#endif

///////////////////////////////////////////////////////////////////////////////


PINLINE PTime::PTime()
{
  theTime = time(NULL);
}


///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PDirectory::IsRoot() const
  { return IsSeparator((*this)[0]) && ((*this)[1] == '\0'); }

PINLINE BOOL PDirectory::IsSeparator(char ch)
  { return ch == PDIR_SEPARATOR; }

PINLINE BOOL PDirectory::Change(const PString & p)
  { return chdir(p) == 0; }

PINLINE BOOL PDirectory::Exists(const PString & p)
  { return access((const char *)p, 0) == 0; }

///////////////////////////////////////////////////////////////////////////////

PINLINE PString PFilePath::GetVolume() const
  { return PString(""); }

///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PFile::Exists(const PFilePath & name)
  { return access(name, 0) == 0; }

PINLINE BOOL PFile::Remove(const PFilePath & name, BOOL)
  { return unlink(name) == 0; }

///////////////////////////////////////////////////////////////////////////////

PINLINE void PChannel::Construct()
  { os_handle = -1; }

PINLINE PString PChannel::GetName() const
  { return channelName; }

// End Of File ///////////////////////////////////////////////////////////////
