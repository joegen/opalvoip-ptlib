/*
 * OSUTIL.INL
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 */

#ifdef	P_HPUX9
#include <langinfo.h>
#else
#include <localeinfo.h>
#endif

PINLINE void PThread::AllocateStack(PINDEX stackSize)
  { stackBase = (char *)malloc(5*stackSize); }

PINLINE void PThread::ClearBlock()
  { blockHandle = -1; }

///////////////////////////////////////////////////////////////////////////////

PINLINE PProcess * PProcess::Current()
  { return PProcessInstance; }

PINLINE char ** PProcess::GetEnvironment()
  { return envp; }

///////////////////////////////////////////////////////////////////////////////

PINLINE unsigned PTimer::Resolution()
  { return (unsigned)(1000/CLOCKS_PER_SEC); }

///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PTime::GetTimeAMPM()
#ifdef P_HPUX9
  { return strstr(nl_langinfo[T_FMT], "%p") =! NULL; }
#else
  { return strchr(_time_info->time, 'H') == NULL; }
#endif

PINLINE PString PTime::GetTimeAM()
#ifdef P_HPUX9
  { return PString(nl_langinfo[T_AM]); }
#else
  { return PString(_time_info->ampm[0]); }
#endif

PINLINE PString PTime::GetTimePM()
#ifdef P_HPUX9
  { return PString(nl_langinfo[T_PM]); }
#else
  { return PString(_time_info->ampm[1]); }
#endif

///////////////////////////////////////////////////////////////////////////////

PINLINE PTimeInterval & PTimeInterval::operator =(const PTimeInterval & timeInterval)
  { milliseconds = timeInterval.milliseconds; return *this; }

///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PDirectory::IsSubDir() const
  { return (entryInfo == NULL) ? FALSE : (entryInfo->type == PFileInfo::SubDirectory); }

PINLINE BOOL PDirectory::Change(const PString & p)
  { return chdir(p) == 0; }

PINLINE BOOL PDirectory::Create(const PString & p, int perm)
  { return mkdir(p, perm) == 0; }

PINLINE BOOL PDirectory::Remove(const PString & p)
  { return rmdir(p) == 0; }

PINLINE BOOL PDirectory::Restart(int newScanMask)
  { scanMask = newScanMask; if (directory != NULL) rewinddir(directory); return TRUE; }

PINLINE PString PDirectory::GetEntryName() const
  { return (entry == NULL) ? PString() : PString((const char *)entry->d_name); }

PINLINE void PDirectory::Close()
  { if (directory != NULL) PAssert(closedir(directory) == 0, POperatingSystemError);
    if (entryInfo != NULL) delete entryInfo; }

PINLINE BOOL PDirectory::Exists(const PString & p)
  { return access((const char *)p, 0) == 0; }

///////////////////////////////////////////////////////////////////////////////

PINLINE PString PFilePath::GetVolume() const
  { return PString(""); }

///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PFile::Rename(const PString & oldname, const PString & newname, BOOL force)
  { return rename(oldname,newname) == 0; }

PINLINE BOOL PFile::Exists(const PString & name)
  { return access(name, 0) == 0; }

PINLINE BOOL PFile::Remove(const PString & name, BOOL force)
  { return unlink(name) == 0; }

///////////////////////////////////////////////////////////////////////////////

PINLINE void PSerialChannel::Construct()
  { }

PINLINE DWORD PSerialChannel::GetSpeed() const
  { return baudRate; }

PINLINE BYTE PSerialChannel::GetDataBits() const
  { return dataBits; }

PINLINE PSerialChannel::Parity PSerialChannel::GetParity() const
  { return parityBits; }

PINLINE BYTE PSerialChannel::GetStopBits() const
  { return stopBits; }

///////////////////////////////////////////////////////////////////////////////

PINLINE void PChannel::Construct()
  { os_handle = -1; }

PINLINE PString PChannel::GetName() const
  { return channelName; }

///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PPipeChannel::CanReadAndWrite()
  { return TRUE; }

// End Of File ///////////////////////////////////////////////////////////////
