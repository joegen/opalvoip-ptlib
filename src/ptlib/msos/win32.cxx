/*
 * $Id: win32.cxx,v 1.44 1997/01/12 04:24:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: win32.cxx,v $
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
#include <svcproc.h>

#include <winuser.h>
#include <winnls.h>

#include <process.h>
#include <fcntl.h>
#include <errno.h>
#include <sys\stat.h>



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
  DWORD maxFilenameLen, fileSystemFlags;
  PAssertOS(GetVolumeInformation(NULL, volName, sizeof(volName), NULL,
                                 &maxFilenameLen, &fileSystemFlags, NULL, 0));
  return PCaselessString(volName);
}


void PDirectory::Close()
{
  if (hFindFile != INVALID_HANDLE_VALUE)
    FindClose(hFindFile);
}


PString PDirectory::CreateFullPath(const PString & path, BOOL isDirectory)
{
  LPSTR dummy;
  PString fullpath;
  PINDEX len = (PINDEX)GetFullPathName(path,
                           _MAX_PATH, fullpath.GetPointer(_MAX_PATH), &dummy);
  if (isDirectory && len > 0 && fullpath[len-1] != PDIR_SEPARATOR)
    fullpath += PDIR_SEPARATOR;
  PINDEX pos = 0;
  while ((pos = fullpath.Find('/', pos)) != P_MAX_INDEX)
    fullpath[pos] = PDIR_SEPARATOR;
  return fullpath;
}


BOOL PDirectory::GetVolumeSpace(PInt64 & total, PInt64 & free) const
{
  PString root;
  if ((*this)[1] == ':')
    root = Mid(3);
  else {
    PINDEX slash = FindOneOf("\\/", 2);
    if (slash != P_MAX_INDEX)
      root = Mid(slash+1);
    else
      root = *this;
  }

  DWORD sectorsPerCluster;      // address of sectors per cluster 
  DWORD bytesPerSector;         // address of bytes per sector 
  DWORD numberOfFreeClusters;   // address of number of free clusters  
  DWORD totalNumberOfClusters;  // address of total number of clusters 

  if (!GetDiskFreeSpace(root,
                        &sectorsPerCluster,
                        &bytesPerSector,
                        &numberOfFreeClusters,
                        &totalNumberOfClusters))
    return FALSE;

  PInt64 clusterSize = bytesPerSector*sectorsPerCluster;
  free = numberOfFreeClusters*clusterSize;
  total = totalNumberOfClusters*clusterSize;
  return TRUE;
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
    static int errors[Miscellaneous+1] = {
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
    { WSAECONNRESET,            "Connection reset" },
    { WSAESHUTDOWN,             "Connection shutdown" },
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
// PPipeChannel

void PPipeChannel::Construct(const PString & subProgram,
                const char * const * arguments, OpenMode mode, BOOL searchPath)
{
  STARTUPINFO startup;
  memset(&startup, 0, sizeof(startup));
  startup.cb = sizeof(startup);
  startup.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
  startup.wShowWindow = SW_HIDE;

  SECURITY_ATTRIBUTES security;
  security.nLength = sizeof(security);
  security.lpSecurityDescriptor = NULL;
  security.bInheritHandle = TRUE;

  if (mode == ReadOnly)
    hToChild = INVALID_HANDLE_VALUE;
  else {
    HANDLE writeEnd;
    PAssertOS(CreatePipe(&startup.hStdInput, &writeEnd, &security, 0));
    PAssertOS(DuplicateHandle(GetCurrentProcess(), writeEnd,
                              GetCurrentProcess(), &hToChild, 0, FALSE,
                              DUPLICATE_CLOSE_SOURCE|DUPLICATE_SAME_ACCESS));
  }

  if (mode == WriteOnly)
    hFromChild = NULL;
  else {
    PAssertOS(CreatePipe(&hFromChild, &startup.hStdOutput, &security, 0));
    startup.hStdError = startup.hStdOutput;
  }

  const char * prog = NULL;
  PString cmdLine;
  if (searchPath)
    cmdLine = subProgram + " ";
  else
    prog = subProgram;

  if (arguments != NULL) {
    while (*arguments != NULL) {
      cmdLine += *arguments;
      cmdLine += ' ';
      arguments++;
    }
  }

  if (ConvertOSError(CreateProcess(prog, cmdLine.GetPointer(),
                     NULL, NULL, TRUE, 0,
                     NULL, NULL, &startup, &info) ? 0 : -2))
    os_handle = info.dwProcessId;

  if (startup.hStdInput != NULL)
    CloseHandle(startup.hStdInput);
  if (startup.hStdOutput != NULL)
    CloseHandle(startup.hStdOutput);
}


PPipeChannel::~PPipeChannel()
{
  Close();
}


BOOL PPipeChannel::IsOpen() const
{
  return os_handle != (int)INVALID_HANDLE_VALUE;
}


int PPipeChannel::GetReturnCode() const
{
  DWORD code;
  if (GetExitCodeProcess(info.hProcess, &code) && (code != STILL_ACTIVE))
    return code;

  ((PPipeChannel*)this)->ConvertOSError(-2);
  return -1;
}

BOOL PPipeChannel::IsRunning() const
{
  DWORD code;
  return GetExitCodeProcess(info.hProcess, &code) && (code == STILL_ACTIVE);
}

int PPipeChannel::WaitForTermination()
{
  if (WaitForSingleObject(info.hProcess, INFINITE) == WAIT_OBJECT_0)
    return GetReturnCode();

  ConvertOSError(-2);
  return -1;
}

int PPipeChannel::WaitForTermination(const PTimeInterval & timeout)
{
  if (WaitForSingleObject(info.hProcess, timeout.GetInterval()) == WAIT_OBJECT_0)
    return GetReturnCode();

  ConvertOSError(-2);
  return -1;
}

BOOL PPipeChannel::Kill(int signal)
{
  return ConvertOSError(TerminateProcess(info.hProcess, signal) ? 0 : -2);
}

BOOL PPipeChannel::Read(void * buffer, PINDEX len)
{
  lastReadCount = 0;
  DWORD count;
  if (!ConvertOSError(ReadFile(hFromChild, buffer, len, &count, NULL) ? 0 :-2))
    return FALSE;
  lastReadCount = count;
  return lastReadCount > 0;
}
      

BOOL PPipeChannel::Write(const void * buffer, PINDEX len)
{
  lastWriteCount = 0;
  DWORD count;
  if (!ConvertOSError(WriteFile(hToChild, buffer, len, &count, NULL) ? 0 : -2))
    return FALSE;
  lastWriteCount = count;
  return lastWriteCount >= len;
}


BOOL PPipeChannel::Close()
{
  if (IsOpen()) {
    if (hToChild != INVALID_HANDLE_VALUE)
      CloseHandle(hToChild);
    if (hFromChild != INVALID_HANDLE_VALUE)
      CloseHandle(hFromChild);
    TerminateProcess(info.hProcess, 1);
    os_handle = -1;
  }
  return TRUE;
}


BOOL PPipeChannel::Execute()
{
  flush();
  clear();
  if (hToChild != INVALID_HANDLE_VALUE)
    CloseHandle(hToChild);
  hToChild = INVALID_HANDLE_VALUE;
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// PSerialChannel

void PSerialChannel::Construct()
{
  commsResource = INVALID_HANDLE_VALUE;

  char str[50];
  strcpy(str, "com1");
  GetProfileString("ports", str, "9600,n,8,1,x", &str[5], sizeof(str)-6);
  str[4] = ':';
  memset(&deviceControlBlock, 0, sizeof(deviceControlBlock));
  deviceControlBlock.DCBlength = sizeof(deviceControlBlock);
  BuildCommDCB(str, &deviceControlBlock);
}


PString PSerialChannel::GetName() const
{
  return portName;
}


class POVERLAPPED : public OVERLAPPED {
  public:
    POVERLAPPED()
      {
        memset(this, 0, sizeof(OVERLAPPED));
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
      }
    ~POVERLAPPED()
      { CloseHandle(hEvent); }
};


BOOL PSerialChannel::Read(void * buf, PINDEX len)
{
  lastReadCount = 0;

  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  COMMTIMEOUTS cto;
  PAssertOS(GetCommTimeouts(commsResource, &cto));
  cto.ReadIntervalTimeout = 0;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 0;
  cto.ReadIntervalTimeout = MAXDWORD; // Immediate timeout
  PAssertOS(SetCommTimeouts(commsResource, &cto));

  DWORD eventMask;
  PAssertOS(GetCommMask(commsResource, &eventMask));
  if (eventMask != (EV_RXCHAR|EV_TXEMPTY))
    PAssertOS(SetCommMask(commsResource, EV_RXCHAR|EV_TXEMPTY));

  POVERLAPPED overlap;
  DWORD timeToGo = readTimeout.GetInterval();
  DWORD bytesToGo = len;
  char * bufferPtr = (char *)buf;

  for (;;) {
    DWORD readCount = 0;
    if (!ReadFile(commsResource, bufferPtr, bytesToGo, &readCount, &overlap)) {
      if (GetLastError() != ERROR_IO_PENDING)
        return ConvertOSError(-2);
      if (!GetOverlappedResult(commsResource, &overlap, &readCount, FALSE))
        return ConvertOSError(-2);
    }

    bytesToGo -= readCount;
    bufferPtr += readCount;
    lastReadCount += readCount;
    if (lastReadCount >= len || timeToGo == 0)
      return lastReadCount > 0;

    if (!WaitCommEvent(commsResource, &eventMask, &overlap)) {
      if (GetLastError() != ERROR_IO_PENDING)
        return ConvertOSError(-2);
      if (WaitForSingleObject(overlap.hEvent, timeToGo) == WAIT_FAILED)
        return ConvertOSError(-2);
    }
  }
}


BOOL PSerialChannel::Write(const void * buf, PINDEX len)
{
  lastWriteCount = 0;

  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  COMMTIMEOUTS cto;
  PAssertOS(GetCommTimeouts(commsResource, &cto));
  cto.WriteTotalTimeoutMultiplier = 0;
  if (writeTimeout == PMaxTimeInterval)
    cto.WriteTotalTimeoutConstant = 0;
  else if (writeTimeout <= PTimeInterval(0))
    cto.WriteTotalTimeoutConstant = 1;
  else
    cto.WriteTotalTimeoutConstant = writeTimeout.GetInterval();
  PAssertOS(SetCommTimeouts(commsResource, &cto));

  OVERLAPPED overlap;
  memset(&overlap, 0, sizeof(overlap));
  overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (WriteFile(commsResource, buf, len, (LPDWORD)&lastWriteCount, &overlap)) {
    CloseHandle(overlap.hEvent);
    return lastWriteCount == len;
  }

  if (GetLastError() == ERROR_IO_PENDING)
    if (GetOverlappedResult(commsResource,
                                   &overlap, (LPDWORD)&lastWriteCount, TRUE)) {
      CloseHandle(overlap.hEvent);
      return lastWriteCount == len;
    }

  ConvertOSError(-2);
  CloseHandle(overlap.hEvent);

  return FALSE;
}


BOOL PSerialChannel::Close()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  CloseHandle(commsResource);
  commsResource = INVALID_HANDLE_VALUE;
  os_handle = -1;
  return ConvertOSError(-2);
}


BOOL PSerialChannel::SetCommsParam(DWORD speed, BYTE data, Parity parity,
                     BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  if (speed > 0)
    deviceControlBlock.BaudRate = speed;

  if (data > 0)
    deviceControlBlock.ByteSize = data;

  switch (parity) {
    case NoParity :
      deviceControlBlock.Parity = NOPARITY;
      break;
    case OddParity :
      deviceControlBlock.Parity = ODDPARITY;
      break;
    case EvenParity :
      deviceControlBlock.Parity = EVENPARITY;
      break;
    case MarkParity :
      deviceControlBlock.Parity = MARKPARITY;
      break;
    case SpaceParity :
      deviceControlBlock.Parity = SPACEPARITY;
      break;
  }

  switch (stop) {
    case 1 :
      deviceControlBlock.StopBits = ONESTOPBIT;
      break;
    case 2 :
      deviceControlBlock.StopBits = TWOSTOPBITS;
      break;
  }

  switch (inputFlow) {
    case NoFlowControl :
      deviceControlBlock.fRtsControl = RTS_CONTROL_DISABLE;
      deviceControlBlock.fInX = FALSE;
      break;
    case XonXoff :
      deviceControlBlock.fRtsControl = RTS_CONTROL_DISABLE;
      deviceControlBlock.fInX = TRUE;
      break;
    case RtsCts :
      deviceControlBlock.fRtsControl = RTS_CONTROL_HANDSHAKE;
      deviceControlBlock.fInX = FALSE;
      break;
  }

  switch (outputFlow) {
    case NoFlowControl :
      deviceControlBlock.fOutxCtsFlow = FALSE;
      deviceControlBlock.fOutxDsrFlow = FALSE;
      deviceControlBlock.fOutX = FALSE;
      break;
    case XonXoff :
      deviceControlBlock.fOutxCtsFlow = FALSE;
      deviceControlBlock.fOutxDsrFlow = FALSE;
      deviceControlBlock.fOutX = TRUE;
      break;
    case RtsCts :
      deviceControlBlock.fOutxCtsFlow = TRUE;
      deviceControlBlock.fOutxDsrFlow = FALSE;
      deviceControlBlock.fOutX = FALSE;
      break;
  }

  if (IsOpen())
   return ConvertOSError(SetCommState(commsResource,
                                                &deviceControlBlock) ? 0 : -2);

  osError = EBADF;
  lastError = NotOpen;
  return FALSE;
}


BOOL PSerialChannel::Open(const PString & port, DWORD speed, BYTE data,
               Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  Close();

  portName = port;
  if (portName.Find(PDIR_SEPARATOR) == P_MAX_INDEX)
    portName = "\\\\.\\" + port;
  commsResource = CreateFile(portName,
                             GENERIC_READ|GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL);
  if (commsResource == INVALID_HANDLE_VALUE)
    return ConvertOSError(-2);

  os_handle = 0;

  SetupComm(commsResource, 2048, 2048);

  if (SetCommsParam(speed, data, parity, stop, inputFlow, outputFlow))
    return TRUE;

  ConvertOSError(-2);
  CloseHandle(commsResource);
  os_handle = -1;
  return FALSE;
}


BOOL PSerialChannel::SetSpeed(DWORD speed)
{
  return SetCommsParam(speed,
                  0, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


DWORD PSerialChannel::GetSpeed() const
{
  return deviceControlBlock.BaudRate;
}


BOOL PSerialChannel::SetDataBits(BYTE data)
{
  return SetCommsParam(0,
               data, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


BYTE PSerialChannel::GetDataBits() const
{
  return deviceControlBlock.ByteSize;
}


BOOL PSerialChannel::SetParity(Parity parity)
{
  return SetCommsParam(0,0,parity,0,DefaultFlowControl,DefaultFlowControl);
}


PSerialChannel::Parity PSerialChannel::GetParity() const
{
  switch (deviceControlBlock.Parity) {
    case ODDPARITY :
      return OddParity;
    case EVENPARITY :
      return EvenParity;
    case MARKPARITY :
      return MarkParity;
    case SPACEPARITY :
      return SpaceParity;
  }
  return NoParity;
}


BOOL PSerialChannel::SetStopBits(BYTE stop)
{
  return SetCommsParam(0,
               0, DefaultParity, stop, DefaultFlowControl, DefaultFlowControl);
}


BYTE PSerialChannel::GetStopBits() const
{
  return (BYTE)(deviceControlBlock.StopBits == ONESTOPBIT ? 1 : 2);
}


BOOL PSerialChannel::SetInputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0,DefaultParity,0,flowControl,DefaultFlowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetInputFlowControl() const
{
  if (deviceControlBlock.fRtsControl == RTS_CONTROL_HANDSHAKE)
    return RtsCts;
  if (deviceControlBlock.fInX != 0)
    return XonXoff;
  return NoFlowControl;
}


BOOL PSerialChannel::SetOutputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0,DefaultParity,0,DefaultFlowControl,flowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetOutputFlowControl() const
{
  if (deviceControlBlock.fOutxCtsFlow != 0)
    return RtsCts;
  if (deviceControlBlock.fOutX != 0)
    return XonXoff;
  return NoFlowControl;
}


void PSerialChannel::SetDTR(BOOL state)
{
  if (IsOpen())
    PAssertOS(EscapeCommFunction(commsResource, state ? SETDTR : CLRDTR));
  else {
    osError = EBADF;
    lastError = NotOpen;
  }
}


void PSerialChannel::SetRTS(BOOL state)
{
  if (IsOpen())
    PAssertOS(EscapeCommFunction(commsResource, state ? SETRTS : CLRRTS));
  else {
    osError = EBADF;
    lastError = NotOpen;
  }
}


void PSerialChannel::SetBreak(BOOL state)
{
  if (IsOpen())
    if (state)
      PAssertOS(SetCommBreak(commsResource));
    else
      PAssertOS(ClearCommBreak(commsResource));
  else {
    osError = EBADF;
    lastError = NotOpen;
  }
}


BOOL PSerialChannel::GetCTS()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_CTS_ON) != 0;
}


BOOL PSerialChannel::GetDSR()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_DSR_ON) != 0;
}


BOOL PSerialChannel::GetDCD()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_RLSD_ON) != 0;
}


BOOL PSerialChannel::GetRing()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_RING_ON) != 0;
}


PStringList PSerialChannel::GetPortNames()
{
  static char buf[] = "\\\\.\\COM ";
  PStringList ports;
  for (char p = '1'; p <= '9'; p++) {
    buf[sizeof(buf)-2] = p;
    ports.AppendString(buf);
  }
  return ports;
}



///////////////////////////////////////////////////////////////////////////////
// Configuration files

class RegistryKey
{
  public:
    enum OpenMode {
      ReadOnly,
      ReadWrite,
      Create
    };
    RegistryKey(const PString & subkey, OpenMode mode);
    ~RegistryKey();
    BOOL EnumKey(PINDEX idx, PString & str);
    BOOL EnumValue(PINDEX idx, PString & str);
    BOOL DeleteKey(const PString & subkey);
    BOOL DeleteValue(const PString & value);
    BOOL QueryValue(const PString & value, PString & str);
    BOOL QueryValue(const PString & value, DWORD & num, BOOL boolean);
    BOOL SetValue(const PString & value, const PString & str);
    BOOL SetValue(const PString & value, DWORD num);
  private:
    HKEY key;
};


class SecurityID
{
  public:
    SecurityID(PSID_IDENTIFIER_AUTHORITY  pIdentifierAuthority,	// pointer to identifier authority
               BYTE nSubAuthorityCount,	// count of subauthorities
               DWORD dwSubAuthority0,	// subauthority 0
               DWORD dwSubAuthority1,	// subauthority 1
               DWORD dwSubAuthority2,	// subauthority 2
               DWORD dwSubAuthority3,	// subauthority 3
               DWORD dwSubAuthority4,	// subauthority 4
               DWORD dwSubAuthority5,	// subauthority 5
               DWORD dwSubAuthority6,	// subauthority 6
               DWORD dwSubAuthority7	// subauthority 7
              )
    {
      if (!AllocateAndInitializeSid(pIdentifierAuthority,	// pointer to identifier authority
                                    nSubAuthorityCount,	// count of subauthorities
                                    dwSubAuthority0,	// subauthority 0
                                    dwSubAuthority1,	// subauthority 1
                                    dwSubAuthority2,	// subauthority 2
                                    dwSubAuthority3,	// subauthority 3
                                    dwSubAuthority4,	// subauthority 4
                                    dwSubAuthority5,	// subauthority 5
                                    dwSubAuthority6,	// subauthority 6
                                    dwSubAuthority7,	// subauthority 7
                                    &sidptr))
        sidptr = NULL;
    }

    SecurityID(LPCTSTR lpSystemName,	// address of string for system name
               LPCTSTR lpAccountName,	// address of string for account name
               LPTSTR ReferencedDomainName,	// address of string for referenced domain 
               LPDWORD cbReferencedDomainName,	// address of size of domain string
               PSID_NAME_USE peUse 	// address of SID-type indicator
              )
    {
      DWORD len = 1024;
      sidptr = (PSID)LocalAlloc(LPTR, len);
      if (sidptr != NULL) {
        if (!LookupAccountName(lpSystemName,	// address of string for system name
                               lpAccountName,	// address of string for account name
                               sidptr,	// address of security identifier
                               &len,	// address of size of security identifier
                               ReferencedDomainName,	// address of string for referenced domain 
                               cbReferencedDomainName,	// address of size of domain string
                               peUse 	// address of SID-type indicator
                              )) {
          LocalFree(sidptr);
          sidptr = NULL;
        }
      }
    }
    ~SecurityID()
    {
      FreeSid(sidptr);
    }

    operator PSID() const
    {
      return sidptr;
    }

    DWORD GetLength() const
    {
      return GetLengthSid(sidptr);
    }

    BOOL IsValid() const
    {
      return sidptr != NULL && IsValidSid(sidptr);
    }

  private:
    PSID sidptr;
};


static DWORD SecureCreateKey(HKEY rootKey, const PString & subkey, HKEY & key)
{
  SECURITY_DESCRIPTOR secdesc;
  if (!InitializeSecurityDescriptor(&secdesc, SECURITY_DESCRIPTOR_REVISION))
    return GetLastError();

  static SID_IDENTIFIER_AUTHORITY siaNTAuthority = SECURITY_NT_AUTHORITY;
  SecurityID adminID(&siaNTAuthority, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS, 
                     0, 0, 0, 0, 0, 0);
  if (!adminID.IsValid())
    return GetLastError();

  static SID_IDENTIFIER_AUTHORITY siaSystemAuthority = SECURITY_NT_AUTHORITY;
  SecurityID systemID(&siaSystemAuthority, 1,
                      SECURITY_LOCAL_SYSTEM_RID,
                      0, 0, 0, 0, 0, 0, 0);
  if (!systemID.IsValid())
    return GetLastError();

  static SID_IDENTIFIER_AUTHORITY siaCreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
  SecurityID creatorID(&siaCreatorAuthority, 1,
                       SECURITY_CREATOR_OWNER_RID,
                       0, 0, 0, 0, 0, 0, 0);
  if (!creatorID.IsValid())
    return GetLastError();

  SID_NAME_USE snuType;
  char szDomain[100];
  DWORD cchDomainName = sizeof(szDomain);
  SecurityID userID(NULL, PProcess::Current()->GetUserName(),
                    szDomain, &cchDomainName, &snuType);
  if (!userID.IsValid())
    return GetLastError();

  DWORD acl_len = sizeof(ACL) + 4*sizeof(ACCESS_ALLOWED_ACE) +
                    adminID.GetLength() + creatorID.GetLength() +
                    systemID.GetLength() + userID.GetLength();
  PBYTEArray dacl_buf(acl_len);
  PACL dacl = (PACL)dacl_buf.GetPointer(acl_len);
  if (!InitializeAcl(dacl, acl_len, ACL_REVISION2))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, adminID))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, systemID))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, creatorID))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, userID))
    return GetLastError();

  if (!SetSecurityDescriptorDacl(&secdesc, TRUE, dacl, FALSE))
    return GetLastError();

  SECURITY_ATTRIBUTES secattr;
  secattr.nLength = sizeof(secattr);
  secattr.lpSecurityDescriptor = &secdesc;
  secattr.bInheritHandle = FALSE;

  DWORD disposition;

  return RegCreateKeyEx(rootKey, subkey, 0, "", REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, &secattr, &key, &disposition);
}


RegistryKey::RegistryKey(const PString & subkey, OpenMode mode)
{
  PProcess * proc = PProcess::Current();
  DWORD access = mode == ReadOnly ? KEY_READ : KEY_ALL_ACCESS;
  DWORD error;

  if (!proc->GetVersion(FALSE).IsEmpty()) {
    PString keyname = subkey;
    keyname.Replace("CurrentVersion", proc->GetVersion(FALSE));

    error = RegOpenKeyEx(HKEY_CURRENT_USER, keyname, 0, access, &key);
    if (error == ERROR_SUCCESS)
      return;

    PAssert(error != ERROR_ACCESS_DENIED, "Access denied accessing registry entry.");

    error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyname, 0, access, &key);
    if (error == ERROR_SUCCESS)
      return;

    PAssert(error != ERROR_ACCESS_DENIED, "Access denied accessing registry entry.");
  }

  error = RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, access, &key);
  if (error == ERROR_SUCCESS)
    return;

  PAssert(error != ERROR_ACCESS_DENIED, "Access denied accessing registry entry.");

  error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, access, &key);
  if (error == ERROR_SUCCESS)
    return;

  PAssert(error != ERROR_ACCESS_DENIED, "Access denied accessing registry entry.");

  key = NULL;
  if (mode != Create)
    return;

  HKEY rootKey = HKEY_CURRENT_USER;
  if (PProcess::Current()->IsDescendant(PServiceProcess::Class()))
    rootKey = HKEY_LOCAL_MACHINE;

  error = SecureCreateKey(rootKey, subkey, key);
  if (error != ERROR_SUCCESS) {
    DWORD disposition;
    error = RegCreateKeyEx(rootKey, subkey, 0, "", REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS, NULL, &key, &disposition);
    if (error != ERROR_SUCCESS)
      key = NULL;
  }
}


RegistryKey::~RegistryKey()
{
  if (key != NULL)
    RegCloseKey(key);
}


BOOL RegistryKey::EnumKey(PINDEX idx, PString & str)
{
  if (key == NULL)
    return FALSE;

  if (RegEnumKey(key, idx, str.GetPointer(MAX_PATH),MAX_PATH) != ERROR_SUCCESS)
    return FALSE;

  str.MakeMinimumSize();
  return TRUE;
}


BOOL RegistryKey::EnumValue(PINDEX idx, PString & str)
{
  if (key == NULL)
    return FALSE;

  DWORD sizeofname = MAX_PATH;
  if (RegEnumValue(key, idx, str.GetPointer(sizeofname),
                         &sizeofname, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    return FALSE;

  str.MakeMinimumSize();
  return TRUE;
}


BOOL RegistryKey::DeleteKey(const PString & subkey)
{
  if (key == NULL)
    return TRUE;

  return RegDeleteKey(key, subkey) == ERROR_SUCCESS;
}


BOOL RegistryKey::DeleteValue(const PString & value)
{
  if (key == NULL)
    return TRUE;

  return RegDeleteValue(key, (char *)(const char *)value) == ERROR_SUCCESS;
}


BOOL RegistryKey::QueryValue(const PString & value, PString & str)
{
  if (key == NULL)
    return FALSE;

  DWORD type, size;
  if (RegQueryValueEx(key, (char *)(const char *)value,
                                    NULL, &type, NULL, &size) != ERROR_SUCCESS)
    return FALSE;

  switch (type) {
    case REG_SZ :
      return RegQueryValueEx(key, (char *)(const char *)value, NULL,
                  &type, (LPBYTE)str.GetPointer(size), &size) == ERROR_SUCCESS;

    case REG_DWORD :
      DWORD num;
      size = sizeof(num);
      if (RegQueryValueEx(key, (char *)(const char *)value, NULL,
                                &type, (LPBYTE)&num, &size) == ERROR_SUCCESS) {
        str = PString(PString::Signed, num);
        return TRUE;
      }
  }
  return FALSE;
}


BOOL RegistryKey::QueryValue(const PString & value, DWORD & num, BOOL boolean)
{
  if (key == NULL)
    return FALSE;

  DWORD type, size;
  if (RegQueryValueEx(key, (char *)(const char *)value,
                                    NULL, &type, NULL, &size) != ERROR_SUCCESS)
    return FALSE;

  switch (type) {
    case REG_DWORD :
      return RegQueryValueEx(key, (char *)(const char *)value, NULL,
                                  &type, (LPBYTE)&num, &size) == ERROR_SUCCESS;

    case REG_SZ :
      PString str;
      DWORD size = 20;
      if (RegQueryValueEx(key, (char *)(const char *)value, NULL,
                &type, (LPBYTE)str.GetPointer(size), &size) == ERROR_SUCCESS) {
        num = str.AsInteger();
        if (num == 0 && boolean) {
          int c = toupper(str[0]);
          num = c == 'T' || c == 'Y';
        }
        return TRUE;
      }
  }

  return FALSE;
}


BOOL RegistryKey::SetValue(const PString & value, const PString & str)
{
  if (key == NULL)
    return FALSE;

  return RegSetValueEx(key, (char *)(const char *)value, 0, REG_SZ,
                (LPBYTE)(const char *)str, str.GetLength()+1) == ERROR_SUCCESS;

}


BOOL RegistryKey::SetValue(const PString & value, DWORD num)
{
  if (key == NULL)
    return FALSE;

  return RegSetValueEx(key, (char *)(const char *)value,
                     0, REG_DWORD, (LPBYTE)&num, sizeof(num)) == ERROR_SUCCESS;
}


void PConfig::Construct(Source src)
{
  source = src;

  switch (src) {
    case System :
      Construct("WIN.INI");
      break;

    case Application :
      PFilePath appFile = PProcess::Current()->GetFile();
      PFilePath cfgFile = appFile.GetVolume() +
                               appFile.GetPath() + appFile.GetTitle() + ".INI";
      if (PFile::Exists(cfgFile))
        Construct(cfgFile); // Make a file based config
      else {
        location = "SOFTWARE\\";
        PProcess * proc = PProcess::Current();
        if (!proc->GetManufacturer().IsEmpty())
          location += proc->GetManufacturer() + PDIR_SEPARATOR;
        else
          location += "PWLib\\";
        location += proc->GetName() + "\\CurrentVersion\\";
      }
      break;
  }
}


void PConfig::Construct(const PFilePath & filename)
{
  location = filename;
  source = NumSources;
}


static void RecurseRegistryKeys(const PString & location,
                                PINDEX baseLength,
                                PStringList &sections)
{
  RegistryKey registry(location, RegistryKey::ReadOnly);
  PString name;
  for (PINDEX idx = 0; registry.EnumKey(idx, name); idx++) {
    RecurseRegistryKeys(location + name + '\\', baseLength, sections);
    sections.AppendString(location.Mid(baseLength) + name);
  }
}


PStringList PConfig::GetSections() const
{
  PStringList sections;

  switch (source) {
    case Application :
      RecurseRegistryKeys(location, location.GetLength(), sections);
      break;

    case NumSources :
      PString buf;
      char * ptr = buf.GetPointer(10000);
      GetPrivateProfileString(NULL, NULL, "", ptr, 9999, location);
      while (*ptr != '\0') {
        sections.AppendString(ptr);
        ptr += strlen(ptr)+1;
      }
      break;
  }

  return sections;
}


PStringList PConfig::GetKeys(const PString & section) const
{
  PStringList keys;

  switch (source) {
    case Environment : {
      char ** ptr = _environ;
      while (*ptr != NULL) {
        PString buf = *ptr++;
        keys.AppendString(buf.Left(buf.Find('=')));
      }
      break;
    }

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::ReadOnly);
      PString name;
      for (PINDEX idx = 0; registry.EnumValue(idx, name); idx++)
        keys.AppendString(name);
      break;
    }

    case NumSources :
      PAssert(!section.IsEmpty(), PInvalidParameter);
      PString buf;
      char * ptr = buf.GetPointer(10000);
      GetPrivateProfileString(section, NULL, "", ptr, 9999, location);
      while (*ptr != '\0') {
        keys.AppendString(ptr);
        ptr += strlen(ptr)+1;
      }
  }

  return keys;
}


void PConfig::DeleteSection(const PString & section)
{
  switch (source) {
    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location, RegistryKey::ReadWrite);
      registry.DeleteKey(section);
      break;
    }

    case NumSources :
      PAssert(!section.IsEmpty(), PInvalidParameter);
      PAssertOS(WritePrivateProfileString(section, NULL, NULL, location));
  }
}


void PConfig::DeleteKey(const PString & section, const PString & key)
{
  PAssert(!key.IsEmpty(), PInvalidParameter);

  switch (source) {
    case Environment :
      PAssert(key.Find('=') == P_MAX_INDEX, PInvalidParameter);
      putenv(key + "=");
      break;

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::ReadWrite);
      registry.DeleteValue(key);
      break;
    }

    case NumSources :
      PAssert(!section.IsEmpty(), PInvalidParameter);
      PAssertOS(WritePrivateProfileString(section, key, NULL, location));
  }
}


PString PConfig::GetString(const PString & section,
                               const PString & key, const PString & dflt) const
{
  PAssert(!key.IsEmpty(), PInvalidParameter);

  PString str;

  switch (source) {
    case Environment : {
      PAssert(key.Find('=') == P_MAX_INDEX, PInvalidParameter);
      char * env = getenv(key);
      if (env != NULL)
        str = env;
      else
        str = dflt;
      break;
    }

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::ReadOnly);
      if (!registry.QueryValue(key, str))
        str = dflt;
      break;
    }

    case NumSources :
      PAssert(!section.IsEmpty(), PInvalidParameter);
      GetPrivateProfileString(section, key, dflt,
                                        str.GetPointer(1000), 999, location);
      str.MakeMinimumSize();
  }

  return str;
}


void PConfig::SetString(const PString & section,
                                    const PString & key, const PString & value)
{
  PAssert(!key.IsEmpty(), PInvalidParameter);

  switch (source) {
    case Environment :
      PAssert(key.Find('=') == P_MAX_INDEX, PInvalidParameter);
      putenv(key + "=" + value);
      break;

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::Create);
      registry.SetValue(key, value);
      break;
    }

    case NumSources :
      PAssert(!section.IsEmpty(), PInvalidParameter);
      PAssertOS(WritePrivateProfileString(section, key, value, location));
  }
}


BOOL PConfig::GetBoolean(const PString & section,
                                          const PString & key, BOOL dflt) const
{
  if (source != Application) {
    PString str = GetString(section, key, dflt ? "T" : "F").ToUpper();
    int c = toupper(str[0]);
    return c == 'T' || c == 'Y' || str.AsInteger() != 0;
  }

  PAssert(!section.IsEmpty(), PInvalidParameter);
  RegistryKey registry(location + section, RegistryKey::ReadOnly);

  DWORD value;
  if (!registry.QueryValue(key, value, TRUE))
    return dflt;

  return value != 0;
}


void PConfig::SetBoolean(const PString & section, const PString & key, BOOL value)
{
  if (source != Application)
    SetString(section, key, value ? "True" : "False");
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    RegistryKey registry(location + section, RegistryKey::Create);
    registry.SetValue(key, value ? 1 : 0);
  }
}


long PConfig::GetInteger(const PString & section,
                                          const PString & key, long dflt) const
{
  if (source != Application) {
    PString str(PString::Signed, dflt);
    return GetString(section, key, str).AsInteger();
  }

  PAssert(!section.IsEmpty(), PInvalidParameter);
  RegistryKey registry(location + section, RegistryKey::ReadOnly);

  DWORD value;
  if (!registry.QueryValue(key, value, FALSE))
    return dflt;

  return value;
}


void PConfig::SetInteger(const PString & section, const PString & key, long value)
{
  if (source != Application) {
    PString str(PString::Signed, value);
    SetString(section, key, str);
  }
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    RegistryKey registry(location + section, RegistryKey::Create);
    registry.SetValue(key, value);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Threads

UINT __stdcall PThread::MainFunction(void * threadPtr)
{
  PThread * thread = (PThread *)PAssertNULL(threadPtr);

  PProcess * process = PProcess::Current();

  AttachThreadInput(thread->threadId, ((PThread*)process)->threadId, TRUE);
  AttachThreadInput(((PThread*)process)->threadId, thread->threadId, TRUE);

  process->threadMutex.Wait();
  process->activeThreads.SetAt(thread->threadId, thread);
  process->threadMutex.Signal();

  process->SignalTimerChange();

  thread->Main();

  return 0;
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 InitialSuspension start,
                 Priority priorityLevel)
{
  PAssert(stackSize > 0, PInvalidParameter);
  autoDelete = deletion == AutoDeleteThread;
  originalStackSize = stackSize;
  threadHandle = (HANDLE)_beginthreadex(NULL, stackSize, MainFunction, this,
                   start == StartSuspended ? CREATE_SUSPENDED : 0, &threadId);
  PAssertOS(threadHandle != NULL);
  SetPriority(priorityLevel);
}


PThread::~PThread()
{
  if (originalStackSize > 0 && !IsTerminated())
    Terminate();
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
  return threadHandle == NULL ||
                         WaitForSingleObject(threadHandle, 0) == WAIT_OBJECT_0;
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

  static int priorities[NumPriorities] = {
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
  threadHandle = GetCurrentThread();
  threadId = GetCurrentThreadId();
  ((PProcess *)this)->activeThreads.DisallowDeleteObjects();
  ((PProcess *)this)->activeThreads.SetAt(threadId, this);
}


PThread * PThread::Current()
{
  PProcess * process = PProcess::Current();
  process->threadMutex.Wait();
  PThread * thread = process->activeThreads.GetAt(GetCurrentThreadId());
  process->threadMutex.Signal();
  return thread;
}


DWORD PThread::CleanUpOnTerminated()
{
  DWORD id = 0;
  if (IsTerminated()) {
    id = threadId;

    if (threadHandle != NULL) {
      CloseHandle(threadHandle);
      threadHandle = NULL;
    }

    if (autoDelete)
      delete this;
  }
  return id;
}


///////////////////////////////////////////////////////////////////////////////
// PProcess::TimerThread

PProcess::HouseKeepingThread::HouseKeepingThread()
  : PThread(1000, NoAutoDeleteThread, StartSuspended, LowPriority)
{
  Resume();
}


void PProcess::HouseKeepingThread::Main()
{
  PProcess & process = *PProcess::Current();
  for (;;) {
    process.threadMutex.Wait();
    HANDLE * handles = new HANDLE[process.activeThreads.GetSize()+1];
    DWORD numHandles = 1;
    handles[0] = semaphore.GetHandle();
    for (PINDEX i = 0; i < process.activeThreads.GetSize(); i++) {
      handles[numHandles] = process.activeThreads.GetDataAt(i).GetHandle();
      if (handles[numHandles] != process.GetHandle())
        numHandles++;
    }
    process.threadMutex.Signal();

    PTimeInterval nextTimer = process.timers.Process();
    DWORD delay;
    if (nextTimer == PMaxTimeInterval)
      delay = INFINITE;
    else if (nextTimer > 10000)
      delay = 10000;
    else
      delay = nextTimer.GetInterval();

    DWORD status = WaitForMultipleObjects(numHandles, handles, FALSE, delay);

    delete [] handles;

    if (status != WAIT_TIMEOUT && status != WAIT_FAILED
                                                  && status != WAIT_OBJECT_0) {
      process.threadMutex.Wait();
      for (PINDEX i = 0; i < process.activeThreads.GetSize(); i++) {
        DWORD id = process.activeThreads.GetDataAt(i).CleanUpOnTerminated();
        if (id != 0)
          process.activeThreads.SetAt(id, NULL);
      }
      process.threadMutex.Signal();
    }
  }
}


void PProcess::SignalTimerChange()
{
  if (houseKeeper == NULL)
    houseKeeper = PNEW HouseKeepingThread;
  else
    houseKeeper->semaphore.Signal();
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

void PProcess::Construct()
{
  houseKeeper = NULL;
}


PProcess::~PProcess()
{
  Sleep(1000);  // Give threads time to die a natural death

  threadMutex.Wait();
  for (PINDEX i = 0; i < activeThreads.GetSize(); i++) {
    PThread & thread = activeThreads.GetDataAt(i);
    if (this != &thread && !thread.IsTerminated())
      TerminateThread(thread.GetHandle(), 1);  // With extreme prejudice
    thread.CleanUpOnTerminated();
  }
  threadMutex.Signal();

  delete houseKeeper;

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


PString PProcess::GetOSVersion()
{
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);
  return psprintf("v%u.%u", info.dwMajorVersion, info.dwMinorVersion);
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


///////////////////////////////////////////////////////////////////////////////
// PSemaphore

PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
{
  if (initial > maxCount)
    initial = maxCount;
  hSemaphore = CreateSemaphore(NULL, initial, maxCount, NULL);
  PAssertOS(hSemaphore != NULL);
}


PSemaphore::~PSemaphore()
{
  PAssertOS(CloseHandle(hSemaphore));
}


void PSemaphore::Wait()
{
  PAssertOS(WaitForSingleObject(hSemaphore, INFINITE) != WAIT_FAILED);
}


BOOL PSemaphore::Wait(const PTimeInterval & timeout)
{
  DWORD result = WaitForSingleObject(hSemaphore, timeout.GetInterval());
  PAssertOS(result != WAIT_FAILED);
  return result != WAIT_TIMEOUT;
}


void PSemaphore::Signal()
{
  if (!ReleaseSemaphore(hSemaphore, 1, NULL))
    PAssertOS(GetLastError() != ERROR_INVALID_HANDLE);
  SetLastError(ERROR_SUCCESS);
}


BOOL PSemaphore::WillBlock() const
{
  DWORD result = WaitForSingleObject(hSemaphore, 0);
  PAssertOS(result != WAIT_FAILED);
  return result == WAIT_TIMEOUT;
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
