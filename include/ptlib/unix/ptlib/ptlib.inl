/*
 * OSUTIL.INL
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
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

PINLINE char ** PProcess::PXGetEnvp() const
  { return envp; }

#if 0
PINLINE char ** PProcess::GetArgv() const
  { return argv; }

PINLINE int PProcess::GetArgc() const
  { return argc; }
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

PINLINE BOOL PTime::GetTimeAMPM()
#if defined(P_USE_LANGINFO)
  { return strstr(nl_langinfo(T_FMT), "%p") != NULL; }
#elif defined(P_LINUX)
  { return strchr(_time_info->time, 'H') == NULL; }
#elif defined(P_SUN4)
  { return FALSE; }
#else
#warning No AMPM flag
  { return FALSE; }
#endif

PINLINE PString PTime::GetTimeAM()
#if defined(P_USE_LANGINFO)
  { return PString(nl_langinfo(AM_STR)); }
#elif defined(P_LINUX)
  { return PString(_time_info->ampm[0]); }
#elif defined(P_SUN4)
  { return "am"; }
#else
#warning No AM string
  { return "am"; }
#endif

PINLINE PString PTime::GetTimePM()
#if defined(P_USE_LANGINFO)
  { return PString(nl_langinfo(PM_STR)); }
#elif defined(P_LINUX)
  { return PString(_time_info->ampm[1]); }
#elif defined(P_SUN4)
  { return "pm"; }
#else
#warning No PM string
  { return "pm"; }
#endif

///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PDirectory::IsSubDir() const
  { return (entryInfo == NULL) ? FALSE : (entryInfo->type == PFileInfo::SubDirectory); }

PINLINE BOOL PDirectory::IsRoot() const
  { return IsSeparator((*this)[0]) && ((*this)[1] == '\0'); }

PINLINE BOOL PDirectory::IsSeparator(char ch)
  { return ch == PDIR_SEPARATOR; }

PINLINE BOOL PDirectory::Change(const PString & p)
  { return chdir(p) == 0; }

PINLINE BOOL PDirectory::Restart(int newScanMask)
  { scanMask = newScanMask; if (directory != NULL) rewinddir(directory); return TRUE; }

PINLINE PString PDirectory::GetEntryName() const
  { return (entry == NULL) ? PString() : PString((const char *)entry->d_name); }

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
