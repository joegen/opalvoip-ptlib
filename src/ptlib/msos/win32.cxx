/*
 * win32.cxx
 *
 * Miscellaneous implementation of classes for Win32
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
 * $Log: win32.cxx,v $
 * Revision 1.80  1999/01/30 14:28:25  robertj
 * Added GetOSConfigDir() function.
 *
 * Revision 1.79  1999/01/16 02:00:29  robertj
 * Added hardware description funtion.
 *
 * Revision 1.78  1998/12/04 10:10:47  robertj
 * Added virtual for determining if process is a service. Fixes linkage problem.
 *
 * Revision 1.77  1998/11/30 07:31:18  robertj
 * New directory structure
 * Fission of file into pipe.cxx, winserial.cxx and wincfg.cxx
 *
 * Revision 1.76  1998/11/26 10:35:08  robertj
 * Improved support of FAT32 and large NTFS volumes in GetFreeSpace().
 *
 * Revision 1.75  1998/11/20 03:17:19  robertj
 * Added thread WaitForTermination() function.
 *
 * Revision 1.74  1998/11/19 05:19:53  robertj
 * Bullet proofed WaitForMultipleObjects under 95.
 *
 * Revision 1.73  1998/11/02 10:07:20  robertj
 * Added capability of pip output to go to stdout/stderr.
 *
 * Revision 1.72  1998/10/31 12:50:47  robertj
 * Removed ability to start threads immediately, race condition with vtable (Main() function).
 * Rearranged PPipChannel functions to help with multi-platform-ness.
 *
 * Revision 1.71  1998/10/29 11:29:20  robertj
 * Added ability to set environment in sub-process.
 *
 * Revision 1.70  1998/10/28 00:59:12  robertj
 * Fixed problem when reading standard error from pipe channel, no terminating null on string.
 *
 * Revision 1.69  1998/10/26 09:11:31  robertj
 * Added ability to separate out stdout from stderr on pipe channels.
 *
 * Revision 1.68  1998/10/15 02:20:26  robertj
 * Added message for connection aborted error.
 *
 * Revision 1.67  1998/10/13 14:13:36  robertj
 * Removed uneeded heap allocation.
 *
 * Revision 1.66  1998/09/24 03:30:59  robertj
 * Added open software license.
 *
 * Revision 1.65  1998/09/18 13:56:20  robertj
 * Added support of REG_BINARY registry types in PConfig class.
 *
 * Revision 1.64  1998/08/20 06:05:28  robertj
 * Allowed Win32 class to be used in other compilation modules
 *
 * Revision 1.63  1998/04/01 01:52:42  robertj
 * Fixed problem with NoAutoDelete threads.
 *
 * Revision 1.62  1998/03/29 06:16:56  robertj
 * Rearranged initialisation sequence so PProcess descendent constructors can do "things".
 *
 * Revision 1.61  1998/03/27 10:52:39  robertj
 * Fixed crash bug in win95 OSR2 GetVolumeSpace().
 * Fixed error 87 problem with threads.
 * Fixed GetVolumeSpace() when UNC used.
 *
 * Revision 1.60  1998/03/20 03:19:49  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.59  1998/03/17 10:17:09  robertj
 * Fixed problem with viewing registry entries where the section ends with a \.
 *
 * Revision 1.58  1998/03/09 11:17:38  robertj
 * FAT32 compatibility
 *
 * Revision 1.57  1998/03/05 12:48:37  robertj
 * Fixed bug in get free space on volume.
 * Added cluster size.
 * MemCheck fixes.
 *
 * Revision 1.56  1998/02/16 00:10:45  robertj
 * Added function to open a URL in a browser.
 * Added functions to validate characters in a filename.
 *
 * Revision 1.55  1998/01/26 00:57:09  robertj
 * Fixed uninitialised source in PConfig when getting environment.
 *
 * Revision 1.54  1997/08/28 12:50:21  robertj
 * Fixed race condition in cleaning up threads on application termination.
 *
 * Revision 1.53  1997/08/21 13:27:41  robertj
 * Attempt to fix very slight possibility of endless loop in housekeeping thread.
 *
 * Revision 1.52  1997/08/21 12:44:56  robertj
 * Removed extension from DLL "short" name.
 *
 * Revision 1.51  1997/08/07 11:57:42  robertj
 * Added ability to get registry data from other applications and anywhere in system registry.
 *
 * Revision 1.50  1997/08/04 10:38:43  robertj
 * Fixed infamous error 87 assert failure in housekeeping thread.
 *
 * Revision 1.49  1997/07/14 11:47:22  robertj
 * Added "const" to numerous variables.
 *
 * Revision 1.48  1997/06/16 13:15:53  robertj
 * Added function to get a dyna-link libraries name.
 *
 * Revision 1.47  1997/06/08 04:42:41  robertj
 * Added DLL file extension string function.
 *
 * Revision 1.46  1997/03/28 04:36:30  robertj
 * Added assert for error in thread cleanup wait.
 *
 * Revision 1.45  1997/02/05 11:50:58  robertj
 * Changed current process function to return reference and validate objects descendancy.
 *
 * Revision 1.44  1997/01/12 04:24:16  robertj
 * Added function to get disk size and free space.
 *
 * Revision 1.43  1997/01/01 11:17:06  robertj
 * Added implementation for PPipeChannel::GetReturnCode and PPipeChannel::IsRunning
 *
 * Revision 1.44  1996/12/29 13:05:03  robertj
 * Added wait and abort for pipe channel commands.
 * Added setting of error codes on status error.
 *
 * Revision 1.43  1996/12/29 02:53:13  craigs
 * Added implementation for PPipeChannel::GetReturnCode and
 *   PPipeChannel::IsRunning
 *
 * Revision 1.42  1996/12/17 13:13:05  robertj
 * Fixed win95 support for registry security code,
 *
 * Revision 1.41  1996/12/17 11:00:28  robertj
 * Fixed register entry security access control lists.
 *
 * Revision 1.40  1996/11/16 10:52:48  robertj
 * Fixed bug in PPipeChannel test for open channel, win95 support.
 * Put time back to C function as run time library bug fixed now.
 *
 * Revision 1.39  1996/11/04 03:36:31  robertj
 * Added extra error message for UDP packet truncated.
 *
 * Revision 1.38  1996/10/26 01:42:51  robertj
 * Added more translations for winsock error codes to standard error codes.
 *
 * Revision 1.37  1996/10/14 03:11:25  robertj
 * Changed registry key so when reading only opens in ReadOnly mode.
 *
 * Revision 1.36  1996/10/08 13:03:47  robertj
 * Added new error messages.
 *
 * Revision 1.35  1996/08/08 10:03:43  robertj
 * Fixed static error text returned when no osError value.
 *
 * Revision 1.34  1996/07/27 04:05:31  robertj
 * Created static version of ConvertOSError().
 * Created static version of GetErrorText().
 * Changed thread creation to use C library function instead of direct WIN32.
 * Fixed bug in auto-deleting the housekeeping thread.
 *
 * Revision 1.33  1996/07/20 05:34:05  robertj
 * Fixed order of registry section tree traversal so can delete whole trees.
 *
 * Revision 1.32  1996/06/28 13:24:33  robertj
 * Fixed enumeration of sections to recurse into registry tree.
 *
 * Revision 1.31  1996/06/17 11:38:58  robertj
 * Fixed memory leak on termination of threads.
 *
 * Revision 1.30  1996/06/13 13:32:13  robertj
 * Rewrite of auto-delete threads, fixes Windows95 total crash.
 *
 * Revision 1.29  1996/06/10 09:54:35  robertj
 * Fixed Win95 compatibility for semaphores.
 *
 * Revision 1.28  1996/05/30 11:48:51  robertj
 * Fixed error on socket timeout to return "Timed Out".
 *
 * Revision 1.27  1996/05/23 10:05:36  robertj
 * Fixed bug in PConfig::GetBoolean().
 * Changed PTimeInterval millisecond access function so can get int64.
 * Moved service process code into separate module.
 *
 * Revision 1.26  1996/04/29 12:23:22  robertj
 * Fixed ability to access GDI stuff from subthreads.
 * Added function to return process ID.
 *
 * Revision 1.25  1996/04/17 12:09:30  robertj
 * Added service dependencies.
 * Started win95 support.
 *
 * Revision 1.24  1996/04/09 03:33:58  robertj
 * Fixed bug in incorrect report of timeout on socket read.
 *
 * Revision 1.23  1996/04/01 13:33:19  robertj
 * Fixed bug in install of service, incorrectly required installation before install.
 *
 * Revision 1.22  1996/03/31 09:10:33  robertj
 * Added use of "CurrentVersion" key in registry.
 * Added version display to service process.
 * Added another socket error text message.
 *
 * Revision 1.21  1996/03/12 11:31:39  robertj
 * Moved PProcess destructor to platform dependent code.
 * Fixed bug in deleting Event Viewer registry entry for service process.
 *
 * Revision 1.20  1996/03/10 13:16:49  robertj
 * Implemented system version functions.
 *
 * Revision 1.19  1996/03/04 13:07:33  robertj
 * Allowed for auto deletion of threads on termination.
 *
 * Revision 1.18  1996/02/25 11:15:29  robertj
 * Added platform dependent Construct function to PProcess.
 *
 * Revision 1.17  1996/02/25 03:12:48  robertj
 * Added consts to all GetXxxx functions in PConfig.
 * Fixed bug in PTime::GetTimeZone(), incorrect sign!
 * Fixed problem with PConfig get functions and their WIN32 types should be
 *    able to interchange strings and numbers.
 *
 * Revision 1.16  1996/02/19 13:53:21  robertj
 * Fixed error reporting for winsock classes.
 *
 * Revision 1.15  1996/02/15 14:54:06  robertj
 * Compensated for C library bug in time().
 *
 * Revision 1.14  1996/02/08 12:30:41  robertj
 * Time zone changes.
 * Added OS identification strings to PProcess.
 *
 * Revision 1.13  1996/01/28 02:56:56  robertj
 * Fixed bug in PFilePath functions for if path ends in a directory separator.
 * Made sure all directory separators are correct character in normalised path.
 *
 * Revision 1.12  1996/01/23 13:25:21  robertj
 * Added time zones.
 * Fixed bug if daylight savings indication.
 *
 * Revision 1.11  1996/01/02 12:58:33  robertj
 * Fixed copy of directories.
 * Changed process construction mechanism.
 * Made service process "common".
 *
 * Revision 1.10  1995/12/10 12:05:48  robertj
 * Changes to main() startup mechanism to support Mac.
 * Moved error code for specific WIN32 and MS-DOS versions.
 * Added WIN32 registry support for PConfig objects.
 * Added asserts in WIN32 semaphores.
 *
 * Revision 1.9  1995/11/21 11:53:24  robertj
 * Added timeout on semaphore wait.
 *
 * Revision 1.8  1995/10/14 15:13:04  robertj
 * Fixed bug in WIN32 service command line parameters.
 *
 * Revision 1.7  1995/08/24 12:42:33  robertj
 * Changed PChannel so not a PContainer.
 * Rewrote PSerialChannel::Read yet again so can break out of I/O.
 *
 * Revision 1.6  1995/07/02 01:26:52  robertj
 * Changed thread internal variables.
 * Added service process support for NT.
 *
 * Revision 1.5  1995/06/17 01:03:08  robertj
 * Added NT service process type.
 *
 * Revision 1.4  1995/06/04 12:48:52  robertj
 * Fixed bug in directory path creation.
 * Fixed bug in comms channel open error.
 *
 * Revision 1.3  1995/04/25 11:33:54  robertj
 * Fixed Borland compiler warnings.
 *
 * Revision 1.2  1995/03/22 13:56:18  robertj
 * Fixed directory handle value check for closing directory.
 *
// Revision 1.1  1995/03/14  12:45:20  robertj
// Initial revision
//
 */

#include <ptlib.h>

#include <process.h>

#define new PNEW


///////////////////////////////////////////////////////////////////////////////
// PTime

PTime::PTime()
{
  time(&theTime);
}


PString PTime::GetTimeSeparator()
{
  PString str;
  GetLocaleInfo(GetUserDefaultLCID(), LOCALE_STIME, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


BOOL PTime::GetTimeAMPM()
{
  char str[2];
  GetLocaleInfo(GetUserDefaultLCID(), LOCALE_ITIME, str, sizeof(str));
  return str[0] == '0';
}


PString PTime::GetTimeAM()
{
  PString str;
  GetLocaleInfo(GetUserDefaultLCID(), LOCALE_S1159, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetTimePM()
{
  PString str;
  GetLocaleInfo(GetUserDefaultLCID(), LOCALE_S2359, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetDayName(Weekdays dayOfWeek, NameType type)
{
  PString str;
  // Of course Sunday is 6 and Monday is 1...
  GetLocaleInfo(GetUserDefaultLCID(), (dayOfWeek+6)%7 +
          (type == Abbreviated ? LOCALE_SABBREVDAYNAME1 : LOCALE_SDAYNAME1),
          str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetDateSeparator()
{
  PString str;
  GetLocaleInfo(GetUserDefaultLCID(), LOCALE_SDATE, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetMonthName(Months month, NameType type)
{
  PString str;
  GetLocaleInfo(GetUserDefaultLCID(), month-1 +
      (type == Abbreviated ? LOCALE_SABBREVMONTHNAME1 : LOCALE_SMONTHNAME1),
      str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PTime::DateOrder PTime::GetDateOrder()
{
  char str[2];
  GetLocaleInfo(GetUserDefaultLCID(), LOCALE_IDATE, str, sizeof(str));
  return (DateOrder)(str[0] - '0');
}


BOOL PTime::IsDaylightSavings()
{
  TIME_ZONE_INFORMATION tz;
  DWORD result = GetTimeZoneInformation(&tz);
  PAssertOS(result != 0xffffffff);
  return result == TIME_ZONE_ID_DAYLIGHT;
}


int PTime::GetTimeZone(TimeZoneType type)
{
  TIME_ZONE_INFORMATION tz;
  PAssertOS(GetTimeZoneInformation(&tz) != 0xffffffff);
  if (type == DaylightSavings)
    tz.Bias += tz.DaylightBias;
  return -tz.Bias;
}


PString PTime::GetTimeZoneString(TimeZoneType type)
{
  TIME_ZONE_INFORMATION tz;
  PAssertOS(GetTimeZoneInformation(&tz) != 0xffffffff);
  return type == StandardTime ? tz.StandardName : tz.DaylightName;
}


///////////////////////////////////////////////////////////////////////////////
// Directories

void PDirectory::Construct()
{
  hFindFile = INVALID_HANDLE_VALUE;
  fileinfo.cFileName[0] = '\0';
  PString::operator=(CreateFullPath(*this, TRUE));
}


void PDirectory::CopyContents(const PDirectory & dir)
{
  scanMask  = dir.scanMask;
  hFindFile = dir.hFindFile;
  fileinfo  = dir.fileinfo;
}


BOOL PDirectory::Open(int newScanMask)
{
  scanMask = newScanMask;

  hFindFile = FindFirstFile(operator+("*.*"), &fileinfo);
  if (hFindFile == INVALID_HANDLE_VALUE)
    return FALSE;

  return Filtered() ? Next() : TRUE;
}


BOOL PDirectory::Next()
{
  if (hFindFile == INVALID_HANDLE_VALUE)
    return FALSE;

  do {
    if (!FindNextFile(hFindFile, &fileinfo))
      return FALSE;
  } while (Filtered());

  return TRUE;
}


PCaselessString PDirectory::GetEntryName() const
{
  return fileinfo.cFileName;
}


BOOL PDirectory::IsSubDir() const
{
  return (fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0;
}


PCaselessString PDirectory::GetVolume() const
{
  char volName[100];
  PAssertOS(GetVolumeInformation(NULL, volName, sizeof(volName), NULL, NULL, NULL, NULL, 0));
  return PCaselessString(volName);
}


void PDirectory::Close()
{
  if (hFindFile != INVALID_HANDLE_VALUE) {
    FindClose(hFindFile);
    hFindFile = INVALID_HANDLE_VALUE;
  }
}


PString PDirectory::CreateFullPath(const PString & path, BOOL isDirectory)
{
  PString partialpath = path;
  if (path.IsEmpty())
    partialpath = ".";

  LPSTR dummy;
  PString fullpath;
  PINDEX len = (PINDEX)GetFullPathName(partialpath,
                           _MAX_PATH, fullpath.GetPointer(_MAX_PATH), &dummy);
  if (isDirectory && len > 0 && fullpath[len-1] != PDIR_SEPARATOR)
    fullpath += PDIR_SEPARATOR;
  PINDEX pos = 0;
  while ((pos = fullpath.Find('/', pos)) != P_MAX_INDEX)
    fullpath[pos] = PDIR_SEPARATOR;
  return fullpath;
}


typedef BOOL (WINAPI *GetDiskFreeSpaceExType)(LPCTSTR lpDirectoryName,
                                              PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                                              PULARGE_INTEGER lpTotalNumberOfBytes,
                                              PULARGE_INTEGER lpTotalNumberOfFreeBytes);


BOOL PDirectory::GetVolumeSpace(PInt64 & total, PInt64 & free, DWORD & clusterSize) const
{
  clusterSize = 512;
  total = free = ULONG_MAX;

  PString root;
  if ((*this)[1] == ':')
    root = Left(3);
  else if (theArray[0] == '\\' && theArray[1] == '\\') {
    PINDEX backslash = Find('\\', 2);
    if (backslash != P_MAX_INDEX) {
      backslash = Find('\\', backslash+1);
      if (backslash != P_MAX_INDEX)
        root = Left(backslash+1);
    }
  }

  if (root.IsEmpty())
    return FALSE;

  BOOL needTotalAndFree = TRUE;

  static GetDiskFreeSpaceExType GetDiskFreeSpaceEx =
        (GetDiskFreeSpaceExType)GetProcAddress(LoadLibrary("KERNEL32.DLL"), "GetDiskFreeSpaceExA");
  if (GetDiskFreeSpaceEx != NULL) {
    ULARGE_INTEGER freeBytesAvailableToCaller;
    ULARGE_INTEGER totalNumberOfBytes; 
    ULARGE_INTEGER totalNumberOfFreeBytes;
    if (GetDiskFreeSpaceEx(root,
                           &freeBytesAvailableToCaller,
                           &totalNumberOfBytes,
                           &totalNumberOfFreeBytes)) {
      total = totalNumberOfBytes.QuadPart;
      free = totalNumberOfFreeBytes.QuadPart;
      needTotalAndFree = FALSE;
    }
  }

  clusterSize = 0;
  char fsName[100];
  if (GetVolumeInformation(root, NULL, 0, NULL, NULL, NULL, fsName, sizeof(fsName))) {
    if (stricmp(fsName, "FAT32") == 0) {
      clusterSize = 4096; // Cannot use GetDiskFreeSpace() results for FAT32
      if (!needTotalAndFree)
        return TRUE;
    }
  }

  DWORD sectorsPerCluster;      // address of sectors per cluster 
  DWORD bytesPerSector;         // address of bytes per sector 
  DWORD numberOfFreeClusters;   // address of number of free clusters  
  DWORD totalNumberOfClusters;  // address of total number of clusters 

  if (!GetDiskFreeSpace(root,
                        &sectorsPerCluster,
                        &bytesPerSector,
                        &numberOfFreeClusters,
                        &totalNumberOfClusters)) {
    if (root[0] != '\\' || ::GetLastError() != ERROR_NOT_SUPPORTED)
      return FALSE;

    PString drive = "A:";
    while (WNetAddConnection(root, NULL, drive) != NO_ERROR) {
      if (GetLastError() != ERROR_ALREADY_ASSIGNED)
        return FALSE;
      drive[0]++;
    }
    BOOL ok = GetDiskFreeSpace(drive+'\\',
                               &sectorsPerCluster,
                               &bytesPerSector,
                               &numberOfFreeClusters,
                               &totalNumberOfClusters);
    WNetCancelConnection(drive, TRUE);
    if (!ok)
      return FALSE;
  }

  if (needTotalAndFree) {
    free = numberOfFreeClusters*sectorsPerCluster*bytesPerSector;
    total = totalNumberOfClusters*sectorsPerCluster*bytesPerSector;
  }

  if (clusterSize == 0)
    clusterSize = bytesPerSector*sectorsPerCluster;

  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// PFilePath

static PString IllegalFilenameCharacters =
  "\\/:*?\"<>|"
  "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\0x10"
  "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f";

BOOL PFilePath::IsValid(char c)
{
  return IllegalFilenameCharacters.Find(c) == P_MAX_INDEX;
}


BOOL PFilePath::IsValid(const PString & str)
{
  return str != "." && str != ".." &&
         str.FindOneOf(IllegalFilenameCharacters) == P_MAX_INDEX;
}



///////////////////////////////////////////////////////////////////////////////
// PChannel

PString PChannel::GetErrorText() const
{
  return GetErrorText(lastError, osError);
}

PString PChannel::GetErrorText(Errors lastError, int osError)
{
  if (osError == 0) {
    static int const errors[Miscellaneous+1] = {
      0, ENOENT, EEXIST, ENOSPC, EACCES, 1000, EINVAL, ENOMEM, EBADF, EAGAIN, EINTR, 1001
    };
    if (osError == 0)
      return PString();
    osError = errors[lastError];
  }

  if (osError > 0 && osError < _sys_nerr && _sys_errlist[osError][0] != '\0')
    return _sys_errlist[osError];

  if ((osError & 0x40000000) == 0)
    return psprintf("C runtime error %u", osError);

  DWORD err = osError & 0x3fffffff;

  static const struct {
    DWORD id;
    const char * msg;
  } win32_errlist[] = {
    { ERROR_FILE_NOT_FOUND,     "File not found" },
    { ERROR_PATH_NOT_FOUND,     "Path not found" },
    { ERROR_ACCESS_DENIED,      "Access denied" },
    { ERROR_NOT_ENOUGH_MEMORY,  "Not enough memory" },
    { ERROR_INVALID_FUNCTION,   "Invalid function" },
    { WSAEADDRINUSE,            "Address in use" },
    { WSAENETDOWN,              "Network subsystem failed" },
    { WSAEISCONN,               "Socket is already connected" },
    { WSAENETUNREACH,           "Network unreachable" },
    { WSAEHOSTUNREACH,          "Host unreachable" },
    { WSAECONNREFUSED,          "Connection refused" },
    { WSAEINVAL,                "Invalid operation" },
    { WSAENOTCONN,              "Socket not connected" },
    { WSAECONNABORTED,          "Connection aborted" },
    { WSAECONNRESET,            "Connection reset" },
    { WSAESHUTDOWN,             "Connection shutdown" },
    { WSAENOTSOCK,              "Socket closed or invalid" },
    { WSAETIMEDOUT,             "Timed out" },
    { WSAEMSGSIZE,              "Message larger than buffer" },
    { WSAEWOULDBLOCK,           "Would block" },
    { 0x1000000,                "Unexpected error in protocol" }
  };

  for (PINDEX i = 0; i < PARRAYSIZE(win32_errlist); i++)
    if (win32_errlist[i].id == err)
      return win32_errlist[i].msg;

  return psprintf("WIN32 error %u", err);
}


BOOL PChannel::ConvertOSError(int error)
{
  return ConvertOSError(error, lastError, osError);
}


BOOL PChannel::ConvertOSError(int error, Errors & lastError, int & osError)
{
  if (error >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

  if (error != -2)
    osError = errno;
  else {
    osError = GetLastError();
    switch (osError) {
      case ERROR_INVALID_HANDLE :
      case WSAEBADF :
        osError = EBADF;
        break;
      case ERROR_INVALID_PARAMETER :
      case WSAEINVAL :
        osError = EINVAL;
        break;
      case ERROR_ACCESS_DENIED :
      case WSAEACCES :
        osError = EACCES;
        break;
      case ERROR_NOT_ENOUGH_MEMORY :
        osError = ENOMEM;
        break;
      case WSAEINTR :
        osError = EINTR;
        break;
      case WSAEMSGSIZE :
        osError |= 0x40000000;
        lastError = BufferTooSmall;
        return FALSE;
      case WSAEWOULDBLOCK :
      case WSAETIMEDOUT :
        osError |= 0x40000000;
        lastError = Timeout;
        return FALSE;
      default :
        osError |= 0x40000000;
    }
  }

  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;
    case ENOENT :
      lastError = NotFound;
      break;
    case EEXIST :
      lastError = FileExists;
      break;
    case EACCES :
      lastError = AccessDenied;
      break;
    case ENOMEM :
      lastError = NoMemory;
      break;
    case ENOSPC :
      lastError = DiskFull;
      break;
    case EINVAL :
      lastError = BadParameter;
      break;
    case EBADF :
      lastError = NotOpen;
      break;
    case EAGAIN :
      lastError = Timeout;
      break;
    case EINTR :
      lastError = Interrupted;
      break;
    default :
      lastError = Miscellaneous;
  }

  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PWin32Overlapped

PWin32Overlapped::PWin32Overlapped()
{
  memset(this, 0, sizeof(*this));
  hEvent = CreateEvent(0, TRUE, 0, NULL);
}

PWin32Overlapped::~PWin32Overlapped()
{
  if (hEvent != NULL)
    CloseHandle(hEvent);
}

void PWin32Overlapped::Reset()
{
  Offset = OffsetHigh = 0;
  if (hEvent != NULL)
    ResetEvent(hEvent);
}


///////////////////////////////////////////////////////////////////////////////
// Threads

UINT __stdcall PThread::MainFunction(void * threadPtr)
{
  PThread * thread = (PThread *)PAssertNULL(threadPtr);

  PProcess & process = PProcess::Current();

  AttachThreadInput(thread->threadId, ((PThread&)process).threadId, TRUE);
  AttachThreadInput(((PThread&)process).threadId, thread->threadId, TRUE);

  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt(thread->threadId, thread);
  process.activeThreadMutex.Signal();

  process.SignalTimerChange();

  thread->Main();

  return 0;
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel)
{
  PAssert(stackSize > 0, PInvalidParameter);
  originalStackSize = stackSize;

  autoDelete = deletion == AutoDeleteThread;

  threadHandle = (HANDLE)_beginthreadex(NULL, stackSize, MainFunction,
                                        this, CREATE_SUSPENDED, &threadId);
  PAssertOS(threadHandle != NULL);

  SetPriority(priorityLevel);

  if (autoDelete) {
    PProcess & process = PProcess::Current();
    process.deleteThreadMutex.Wait();
    process.autoDeleteThreads.Append(this);
    process.deleteThreadMutex.Signal();
  }
}


PThread::~PThread()
{
  if (originalStackSize <= 0)
    return;

  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt(threadId, NULL);
  process.activeThreadMutex.Signal();

  if (!IsTerminated())
    Terminate();

  if (threadHandle != NULL)
    CloseHandle(threadHandle);
}


void PThread::Restart()
{
  PAssert(IsTerminated(), "Cannot restart running thread");

  threadHandle = (HANDLE)_beginthreadex(NULL,
                         originalStackSize, MainFunction, this, 0, &threadId);
  PAssertOS(threadHandle != NULL);
}


void PThread::Terminate()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  PAssert(originalStackSize > 0, PLogicError);

  if (Current() == this)
    ExitThread(0);
  else
    TerminateThread(threadHandle, 1);
}


BOOL PThread::IsTerminated() const
{
  return WaitForTermination(0);
}


void PThread::WaitForTermination() const
{
  WaitForTermination(PMaxTimeInterval);
}


BOOL PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  if (threadHandle == NULL)
    return TRUE;

  DWORD result;
  PINDEX retries = 10;
  while ((result = WaitForSingleObject(threadHandle, maxWait.GetInterval())) != WAIT_TIMEOUT) {
    if (result == WAIT_OBJECT_0)
      return TRUE;

    if (::GetLastError() != ERROR_INVALID_HANDLE) {
      PAssertAlways(POperatingSystemError);
      return TRUE;
    }

    if (retries > 0)
      return TRUE;

    retries--;
  }

  return FALSE;
}


void PThread::Suspend(BOOL susp)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  if (susp)
    SuspendThread(threadHandle);
  else
    Resume();
}


void PThread::Resume()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  ResumeThread(threadHandle);
}


BOOL PThread::IsSuspended() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  SuspendThread(threadHandle);
  return ResumeThread(threadHandle) <= 1;
}


void PThread::SetPriority(Priority priorityLevel)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  static int const priorities[NumPriorities] = {
    THREAD_PRIORITY_LOWEST,
    THREAD_PRIORITY_BELOW_NORMAL,
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_ABOVE_NORMAL,
    THREAD_PRIORITY_HIGHEST
  };
  SetThreadPriority(threadHandle, priorities[priorityLevel]);
}


PThread::Priority PThread::GetPriority() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  switch (GetThreadPriority(threadHandle)) {
    case THREAD_PRIORITY_LOWEST :
      return LowestPriority;
    case THREAD_PRIORITY_BELOW_NORMAL :
      return LowPriority;
    case THREAD_PRIORITY_NORMAL :
      return NormalPriority;
    case THREAD_PRIORITY_ABOVE_NORMAL :
      return HighPriority;
    case THREAD_PRIORITY_HIGHEST :
      return HighestPriority;
  }
  PAssertAlways(POperatingSystemError);
  return LowestPriority;
}


void PThread::Yield()
{
  ::Sleep(0);
}


void PThread::InitialiseProcessThread()
{
  originalStackSize = 0;
  autoDelete = FALSE;
  threadHandle = GetCurrentThread();
  threadId = GetCurrentThreadId();
  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt(threadId, this);
}


PThread * PThread::Current()
{
  PProcess & process = PProcess::Current();
  process.activeThreadMutex.Wait();
  PThread * thread = process.activeThreads.GetAt(GetCurrentThreadId());
  process.activeThreadMutex.Signal();
  return thread;
}


///////////////////////////////////////////////////////////////////////////////
// PProcess::TimerThread

PProcess::HouseKeepingThread::HouseKeepingThread()
  : PThread(1000, NoAutoDeleteThread, LowPriority)
{
  Resume();
}


void PProcess::HouseKeepingThread::Main()
{
  PProcess & process = PProcess::Current();
  for (;;) {
    process.deleteThreadMutex.Wait();
    HANDLE handles[MAXIMUM_WAIT_OBJECTS];
    DWORD numHandles = 1;
    handles[0] = breakBlock.GetHandle();
    for (PINDEX i = 0; i < process.autoDeleteThreads.GetSize(); i++) {
      PThread & thread = process.autoDeleteThreads[i];
      if (thread.IsTerminated())
        process.autoDeleteThreads.RemoveAt(i--);
      else {
        handles[numHandles] = thread.GetHandle();
        if (handles[numHandles] != process.GetHandle()) {
          numHandles++;
          if (numHandles >= MAXIMUM_WAIT_OBJECTS)
            break;
        }
      }
    }
    process.deleteThreadMutex.Signal();

    PTimeInterval nextTimer = process.timers.Process();
    DWORD delay;
    if (nextTimer == PMaxTimeInterval)
      delay = INFINITE;
    else if (nextTimer > 10000)
      delay = 10000;
    else
      delay = nextTimer.GetInterval();

    DWORD result;
    PINDEX retries = 100;
    while ((result = WaitForMultipleObjects(numHandles, handles, FALSE, delay)) == WAIT_FAILED) {
      PAssertOS(::GetLastError() == ERROR_INVALID_HANDLE || retries > 0);
      retries--;
    }
  }
}


void PProcess::SignalTimerChange()
{
  if (houseKeeper == NULL)
    houseKeeper = new HouseKeepingThread;
  else
    houseKeeper->breakBlock.Signal();
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

PProcess::~PProcess()
{
  Sleep(100);  // Give threads time to die a natural death

  // Get rid of the house keeper (majordomocide)
  delete houseKeeper;

  // OK, if there are any left we get really insistent...
  activeThreadMutex.Wait();
  for (PINDEX i = 0; i < activeThreads.GetSize(); i++) {
    PThread & thread = activeThreads.GetDataAt(i);
    if (this != &thread && !thread.IsTerminated())
      TerminateThread(thread.GetHandle(), 1);  // With extreme prejudice
  }
  activeThreadMutex.Signal();

  deleteThreadMutex.Wait();
  autoDeleteThreads.RemoveAll();
  deleteThreadMutex.Signal();

#ifndef PMEMORY_CHECK
  extern void PWaitOnExitConsoleWindow();
  PWaitOnExitConsoleWindow();
#endif
}


PString PProcess::GetOSClass()
{
  return "Windows";
}


PString PProcess::GetOSName()
{
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);
  switch (info.dwPlatformId) {
    case VER_PLATFORM_WIN32s :
      return "32s";
    case 1 : //VER_PLATFORM_WIN32_WINDOWS :
      return "95";
    case VER_PLATFORM_WIN32_NT :
      return "NT";
  }
  return "?";
}


PString PProcess::GetOSHardware()
{
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  switch (info.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_INTEL :
      switch (info.dwProcessorType) {
        case PROCESSOR_INTEL_386 :
          return "i386";
        case PROCESSOR_INTEL_486 :
          return "i486";
        case PROCESSOR_INTEL_PENTIUM :
          return "i586";
      }
      return "iX86";

    case PROCESSOR_ARCHITECTURE_MIPS :
      return "mips";

    case PROCESSOR_ARCHITECTURE_ALPHA :
      return "alpha";

    case PROCESSOR_ARCHITECTURE_PPC :
      return "ppc";
  }
  return "?";
}


PString PProcess::GetOSVersion()
{
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);
  return psprintf("v%u.%u", info.dwMajorVersion, info.dwMinorVersion);
}


PDirectory PProcess::GetOSConfigDir()
{
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);

  char dir[_MAX_PATH];

  if (info.dwPlatformId != VER_PLATFORM_WIN32_NT) {
    PAssertOS(GetWindowsDirectory(dir, sizeof(dir)) != 0);
    return dir;
  }

  PAssertOS(GetSystemDirectory(dir, sizeof(dir)) != 0);
  PDirectory sysdir = dir;
  return sysdir + "drivers\\etc";
}


PString PProcess::GetUserName() const
{
  PString username;
  unsigned long size = 50;
  ::GetUserName(username.GetPointer((PINDEX)size), &size);
  username.MakeMinimumSize();
  return username;
}


DWORD PProcess::GetProcessID() const
{
  return GetCurrentProcessId();
}


BOOL PProcess::IsServiceProcess() const
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PSemaphore

PSemaphore::PSemaphore(HANDLE h)
{
  handle = h;
  PAssertOS(handle != NULL);
}


PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
{
  if (initial > maxCount)
    initial = maxCount;
  handle = CreateSemaphore(NULL, initial, maxCount, NULL);
  PAssertOS(handle != NULL);
}


PSemaphore::~PSemaphore()
{
  if (handle != NULL)
    PAssertOS(CloseHandle(handle));
}


void PSemaphore::Wait()
{
  PAssertOS(WaitForSingleObject(handle, INFINITE) != WAIT_FAILED);
}


BOOL PSemaphore::Wait(const PTimeInterval & timeout)
{
  DWORD result = WaitForSingleObject(handle, timeout.GetInterval());
  PAssertOS(result != WAIT_FAILED);
  return result != WAIT_TIMEOUT;
}


void PSemaphore::Signal()
{
  if (!ReleaseSemaphore(handle, 1, NULL))
    PAssertOS(GetLastError() != ERROR_INVALID_HANDLE);
  SetLastError(ERROR_SUCCESS);
}


BOOL PSemaphore::WillBlock() const
{
  DWORD result = WaitForSingleObject(handle, 0);
  PAssertOS(result != WAIT_FAILED);
  return result == WAIT_TIMEOUT;
}


///////////////////////////////////////////////////////////////////////////////
// PMutex

PMutex::PMutex()
  : PSemaphore(::CreateMutex(NULL, FALSE, NULL))
{
}


void PMutex::Signal()
{
  ::ReleaseMutex(handle);
}


///////////////////////////////////////////////////////////////////////////////
// PSyncPoint

PSyncPoint::PSyncPoint()
  : PSemaphore(::CreateEvent(NULL, FALSE, FALSE, NULL))
{
}


void PSyncPoint::Signal()
{
  ::SetEvent(handle);
}


///////////////////////////////////////////////////////////////////////////////
// PDynaLink

PDynaLink::PDynaLink()
{
  _hDLL = NULL;
}


PDynaLink::PDynaLink(const PString & name)
{
  Open(name);
}


PDynaLink::~PDynaLink()
{
  Close();
}


PString PDynaLink::GetExtension()
{
  return ".DLL";
}


BOOL PDynaLink::Open(const PString & name)
{
  _hDLL = LoadLibrary(name);
  return _hDLL != NULL;
}


void PDynaLink::Close()
{
  if (_hDLL != NULL)
    FreeLibrary(_hDLL);
}


BOOL PDynaLink::IsLoaded() const
{
  return _hDLL != NULL;
}


PString PDynaLink::GetName(BOOL full) const
{
  PFILE_PATH_STRING str;
  if (_hDLL != NULL) {
    GetModuleFileName(_hDLL, str.GetPointer(_MAX_PATH), _MAX_PATH-1);
    if (!full) {
      str.Delete(0, str.FindLast('\\')+1);
      PINDEX pos = str.Find(".DLL");
      if (pos != P_MAX_INDEX)
        str.Delete(pos, P_MAX_INDEX);
    }
  }
  str.MakeMinimumSize();
  return str;
}


BOOL PDynaLink::GetFunction(PINDEX index, Function & func)
{
  if (_hDLL == NULL)
    return FALSE;

  FARPROC p = GetProcAddress(_hDLL, (LPSTR)(DWORD)LOWORD(index));
  if (p == NULL)
    return FALSE;

  func = (Function)p;
  return TRUE;
}


BOOL PDynaLink::GetFunction(const PString & name, Function & func)
{
  if (_hDLL == NULL)
    return FALSE;

  FARPROC p = GetProcAddress(_hDLL, name);
  if (p == NULL)
    return FALSE;

  func = (Function)p;
  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
