/*
 * OSUTIL.INL
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 */

#if defined(P_HPUX9)
#include <langinfo.h>
#elif defined(P_SUN4)
#warning No locale info header
#else
#include <localeinfo.h>
#endif

PINLINE PProcess * PProcess::Current()
  { return PProcessInstance; }

PINLINE char ** PProcess::GetEnvp() const
  { return envp; }

PINLINE char ** PProcess::GetArgv() const
  { return argv; }

PINLINE int PProcess::GetArgc() const
  { return argc; }

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
#if defined(P_HPUX9)
  { return strstr(nl_langinfo(T_FMT), "%p") != NULL; }
#elif defined(P_SUN4)
#warning No AMPM flag
  { return FALSE; }
#else
  { return strchr(_time_info->time, 'H') == NULL; }
#endif

PINLINE PString PTime::GetTimeAM()
#if defined(P_HPUX9)
  { return PString(nl_langinfo(AM_STR)); }
#elif defined(P_SUN4)
#warning No AM string
  { return "am"; }
#else
  { return PString(_time_info->ampm[0]); }
#endif

PINLINE PString PTime::GetTimePM()
#if defined(P_HPUX9)
  { return PString(nl_langinfo(PM_STR)); }
#elif defined(P_SUN4)
#warning No PM string
  { return "pm"; }
#else
  { return PString(_time_info->ampm[1]); }
#endif

///////////////////////////////////////////////////////////////////////////////

PINLINE PTimeInterval & PTimeInterval::operator =(const PTimeInterval & timeInterval)
  { milliseconds = timeInterval.milliseconds; return *this; }

PINLINE PTimeInterval::PTimeInterval(const PTimeInterval & copy)
  { milliseconds = copy.milliseconds; }

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

PINLINE BOOL PFile::Remove(const PFilePath & name, BOOL force)
  { return unlink(name) == 0; }

///////////////////////////////////////////////////////////////////////////////

PINLINE void PChannel::Construct()
  { os_handle = -1; }

PINLINE PString PChannel::GetName() const
  { return channelName; }

// End Of File ///////////////////////////////////////////////////////////////
