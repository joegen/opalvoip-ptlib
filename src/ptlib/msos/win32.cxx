/*
 * $Id: win32.cxx,v 1.20 1996/03/10 13:16:49 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: win32.cxx,v $
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

#include <fcntl.h>
#include <errno.h>
#include <sys\stat.h>
#include <process.h>
#include <signal.h>


class RegistryKey
{
  public:
    RegistryKey(const PString & subkey, BOOL create = FALSE);
    ~RegistryKey();
    BOOL EnumKey(PINDEX idx, PString & str);
    BOOL EnumValue(PINDEX idx, PString & str);
    BOOL DeleteKey(const PString & subkey);
    BOOL DeleteValue(const PString & value);
    BOOL QueryValue(const PString & value, PString & str);
    BOOL QueryValue(const PString & value, DWORD & num);
    BOOL SetValue(const PString & value, const PString & str);
    BOOL SetValue(const PString & value, DWORD num);
  private:
    HKEY key;
};


RegistryKey::RegistryKey(const PString & subkey, BOOL create)
{
  if (RegOpenKeyEx(HKEY_CURRENT_USER,
                             subkey, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    return;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             subkey, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    return;

  if (create) {
    HKEY rootKey = HKEY_CURRENT_USER;
    if (PProcess::Current()->IsDescendant(PServiceProcess::Class()))
      rootKey = HKEY_LOCAL_MACHINE;
    DWORD disposition;
    if (RegCreateKeyEx(rootKey, subkey, 0, "", REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS, NULL, &key, &disposition) == ERROR_SUCCESS)
      return;
  }

  key = NULL;
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


BOOL RegistryKey::QueryValue(const PString & value, DWORD & num)
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


///////////////////////////////////////////////////////////////////////////////
// PTime

PTime::PTime()
{
  struct tm t;
  SYSTEMTIME st;
  GetLocalTime(&st);
  t.tm_sec   = st.wSecond;
  t.tm_min   = st.wMinute;
  t.tm_hour  = st.wHour;
  t.tm_mday  = st.wDay;
  t.tm_mon   = st.wMonth-1;
  t.tm_year  = st.wYear-1900;
  t.tm_isdst = IsDaylightSavings();
  theTime = mktime(&t);
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


///////////////////////////////////////////////////////////////////////////////
// PChannel

PString PChannel::GetErrorText() const
{
  if (osError == 0)
    return PString();

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
    { WSAECONNREFUSED,          "Connection refused" },
    { WSAEINVAL,                "Invalid operation" },
    { WSAENOTCONN,              "Socket not connected" },
    { WSAEWOULDBLOCK,           "Would block" }
  };

  for (PINDEX i = 0; i < PARRAYSIZE(win32_errlist); i++)
    if (win32_errlist[i].id == err)
      return win32_errlist[i].msg;

  return psprintf("WIN32 error %u", err);
}


BOOL PChannel::ConvertOSError(int error)
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
        osError = EBADF;
        break;
      case ERROR_INVALID_PARAMETER :
        osError = EINVAL;
        break;
      case ERROR_ACCESS_DENIED :
        osError = EACCES;
        break;
      case ERROR_NOT_ENOUGH_MEMORY :
        osError = ENOMEM;
        break;
      case WSAEINTR :
        osError = EINTR;
        break;
      case WSAEWOULDBLOCK :
        osError = EAGAIN;
        break;
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
  DWORD timeToGo = readTimeout.GetMilliseconds();
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
    cto.WriteTotalTimeoutConstant = writeTimeout.GetMilliseconds();
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
        location += proc->GetName() + PDIR_SEPARATOR;
        if (!proc->GetVersion(FALSE).IsEmpty())
          location += proc->GetVersion(FALSE) + PDIR_SEPARATOR;
      }
      break;
  }
}


void PConfig::Construct(const PFilePath & filename)
{
  location = filename;
  source = NumSources;
}


PStringList PConfig::GetSections() const
{
  PStringList sections;

  switch (source) {
    case Application : {
      RegistryKey registry = location;
      PString name;
      for (PINDEX idx = 0; registry.EnumKey(idx, name); idx++)
        sections.AppendString(name);
      break;
    }

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
      RegistryKey registry = location + section;
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
      RegistryKey registry = location;
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
      RegistryKey registry = location + section;
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
      RegistryKey registry = location + section;
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
      RegistryKey registry(location + section, TRUE);
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
    return str[0] == 'T' || str[0] == 'Y' || str.AsInteger() != 0;
  }

  PAssert(!section.IsEmpty(), PInvalidParameter);
  RegistryKey registry = location + section;

  DWORD value;
  if (!registry.QueryValue(key, value))
    return dflt;

  return value != 0;
}


void PConfig::SetBoolean(const PString & section, const PString & key, BOOL value)
{
  if (source != Application)
    SetString(section, key, value ? "True" : "False");
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    RegistryKey registry(location + section, TRUE);
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
  RegistryKey registry = location + section;

  DWORD value;
  if (!registry.QueryValue(key, value))
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
    RegistryKey registry(location + section, TRUE);
    registry.SetValue(key, value);
  }
}


///////////////////////////////////////////////////////////////////////////////
// Threads

DWORD EXPORTED PThread::MainFunction(LPVOID threadPtr)
{
  PProcess * process = PProcess::Current();
  PThread * thread = (PThread *)PAssertNULL(threadPtr);

  AttachThreadInput(thread->threadId, ((PThread*)process)->threadId, TRUE);
  process->threads.SetAt(thread->threadId, thread);

  thread->Main();

  process->threads.SetAt(thread->threadId, NULL);
  CloseHandle(thread->threadHandle);
  thread->threadHandle = NULL;

  if (thread->autoDelete)
    delete thread;

  return 0;
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 InitialSuspension start,
                 Priority priorityLevel)
{
  PAssert(stackSize > 0, PInvalidParameter);
  originalStackSize = stackSize;
  threadHandle = CreateThread(NULL,
                              stackSize,
                              MainFunction,
                              (LPVOID)this,
                              start == StartSuspended ? CREATE_SUSPENDED : 0,
                              &threadId);
  PAssertOS(threadHandle != NULL);
  SetPriority(priorityLevel);
  autoDelete = deletion == AutoDeleteThread;
}


PThread::~PThread()
{
  Terminate();
}


void PThread::Restart()
{
  if (threadHandle == NULL) {
    threadHandle = CreateThread(NULL,
                  originalStackSize, MainFunction, (LPVOID)this, 0, &threadId);
    PAssertOS(threadHandle != NULL);
  }
}


void PThread::Terminate()
{
  if (threadHandle != NULL && originalStackSize > 0) {
    if (Current() == this)
      ExitThread(0);
    else
      TerminateThread(threadHandle, 1);
    PProcess::Current()->threads.SetAt(threadId, NULL);
    CloseHandle(threadHandle);
    threadHandle = NULL;
    if (autoDelete)
      delete this;
  }
}


BOOL PThread::IsTerminated() const
{
  return threadHandle == NULL;
}


void PThread::Suspend(BOOL susp)
{
  PAssert(threadHandle != NULL, "Suspend on terminated thread");
  if (susp)
    SuspendThread(threadHandle);
  else
    Resume();
}


void PThread::Resume()
{
  PAssert(threadHandle != NULL, "Resume on terminated thread");
  ResumeThread(threadHandle);
}


BOOL PThread::IsSuspended() const
{
  SuspendThread(PAssertNULL(threadHandle));
  return ResumeThread(threadHandle) <= 1;
}


void PThread::SetPriority(Priority priorityLevel)
{
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
  ((PProcess *)this)->threads.DisallowDeleteObjects();
  ((PProcess *)this)->threads.SetAt(threadId, this);
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

void PProcess::Construct()
{
  timerThread = NULL;
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


///////////////////////////////////////////////////////////////////////////////
// PServiceProcess

PServiceProcess::PServiceProcess(const char * manuf, const char * name,
                           WORD major, WORD minor, CodeStatus stat, WORD build)
  : PProcess(manuf, name, major, minor, stat, build)
{
}


static void Control_C(int)
{
  PServiceProcess::Current()->OnStop();
  exit(1);
}


int PServiceProcess::_main(int argc, char ** argv, char **)
{
  PErrorStream = &cerr;
  PreInitialise(1, argv);

  if (argc > 1) {
    switch (ProcessCommand(argv[1])) {
      case CommandProcessed :
        return 0;
      case ProcessCommandError :
        return 1;
    }

    debugMode = TRUE;
    currentLogLevel = LogInfo;
    PError << "Service simulation started for \"" << GetName() << "\".\n"
              "Press Ctrl-C to terminate.\n" << endl;

    signal(SIGINT, Control_C);
    if (OnStart())
      Main();
    OnStop();
    return 0;
  }

  debugMode = FALSE;
  currentLogLevel = LogError;

  // SERVICE_STATUS members that don't change
  status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  status.dwServiceSpecificExitCode = 0;

  static SERVICE_TABLE_ENTRY dispatchTable[] = {
    { "", PServiceProcess::StaticMainEntry },
    { NULL, NULL }
  };

  dispatchTable[0].lpServiceName = (char *)(const char *)GetName();

  if (StartServiceCtrlDispatcher(dispatchTable))
    return GetTerminationValue();

  SystemLog(LogFatal, "StartServiceCtrlDispatcher failed.");
  PError << "Could not start service.\n";
  ProcessCommand("");
  return 1;
}


void PServiceProcess::StaticMainEntry(DWORD argc, LPTSTR * argv)
{
  Current()->MainEntry(argc, argv);
}


void PServiceProcess::MainEntry(DWORD argc, LPTSTR * argv)
{
  // register our service control handler:
  statusHandle = RegisterServiceCtrlHandler(GetName(), StaticControlEntry);
  if (statusHandle == NULL)
    return;

  // report the status to Service Control Manager.
  if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 1, 3000))
    return;

  // create the event object. The control handler function signals
  // this event when it receives the "stop" control code.
  terminationEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (terminationEvent == NULL)
    return;

  GetArguments().SetArgs(argc, argv);

  // start the thread that performs the work of the service.
  threadHandle = CreateThread(NULL,4096,StaticThreadEntry,NULL,0,&threadId);
  if (threadHandle != NULL)
    WaitForSingleObject(terminationEvent, INFINITE);  // Wait here for the end

  TerminateThread(threadHandle, 1);
  CloseHandle(terminationEvent);
  ReportStatus(SERVICE_STOPPED, 0);
}


DWORD EXPORTED PServiceProcess::StaticThreadEntry(LPVOID)
{
  Current()->ThreadEntry();
  return 0;
}


void PServiceProcess::ThreadEntry()
{
  if (OnStart()) {
    ReportStatus(SERVICE_RUNNING);
    Main();
    ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 1, 3000);
  }
  SetEvent(terminationEvent);
}


void PServiceProcess::StaticControlEntry(DWORD code)
{
  Current()->ControlEntry(code);
}


void PServiceProcess::ControlEntry(DWORD code)
{
  switch (code) {
    case SERVICE_CONTROL_PAUSE : // Pause the service if it is running.
      if (status.dwCurrentState != SERVICE_RUNNING)
        ReportStatus(status.dwCurrentState);
      else {
        if (OnPause())
          ReportStatus(SERVICE_PAUSED);
      }
      break;

    case SERVICE_CONTROL_CONTINUE : // Resume the paused service.
      if (status.dwCurrentState == SERVICE_PAUSED)
        OnContinue();
      ReportStatus(status.dwCurrentState);
      break;

    case SERVICE_CONTROL_STOP : // Stop the service.
      // Report the status, specifying the checkpoint and waithint, before
      // setting the termination event.
      ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 1, 3000);
      OnStop();
      SetEvent(terminationEvent);
      break;

    case SERVICE_CONTROL_INTERROGATE : // Update the service status.
    default :
      ReportStatus(status.dwCurrentState);
  }
}


BOOL PServiceProcess::ReportStatus(DWORD dwCurrentState,
                                   DWORD dwWin32ExitCode,
                                   DWORD dwCheckPoint,
                                   DWORD dwWaitHint)
{
  // Disable control requests until the service is started.
  if (dwCurrentState == SERVICE_START_PENDING)
    status.dwControlsAccepted = 0;
  else
    status.dwControlsAccepted =
                           SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;

  // These SERVICE_STATUS members are set from parameters.
  status.dwCurrentState = dwCurrentState;
  status.dwWin32ExitCode = dwWin32ExitCode;
  status.dwCheckPoint = dwCheckPoint;
  status.dwWaitHint = dwWaitHint;

  if (debugMode)
    return TRUE;

  // Report the status of the service to the service control manager.
  if (SetServiceStatus(statusHandle, &status))
    return TRUE;

  // If an error occurs, stop the service.
  SystemLog(LogError, "SetServiceStatus failed");
  return FALSE;
}


void PServiceProcess::SystemLog(SystemLogLevel level,const PString & msg)
{
  SystemLog(level, "%s", (const char *)msg);
}


void PServiceProcess::SystemLog(SystemLogLevel level, const char * fmt, ...)
{
  if (level > currentLogLevel)
    return;

  DWORD err = GetLastError();

  char msg[1000];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);

  if (debugMode) {
    static HANDLE mutex = CreateSemaphore(NULL, 1, 1, NULL);
    WaitForSingleObject(mutex, INFINITE);
    static const char * levelName[NumLogLevels] = {
      "Fatal error",
      "Error",
      "Warning",
      "Information"
    };
    PTime now;
    PError << now.AsString("yy/MM/dd hh:mm:ss ") << levelName[level];
    if (msg[0] != '\0')
      PError << ": " << msg;
    if (level != LogInfo && err != 0)
      PError << " - error = " << err;
    PError << endl;
    ReleaseSemaphore(mutex, 1, NULL);
  }
  else {
    // Use event logging to log the error.
    HANDLE hEventSource = RegisterEventSource(NULL, GetName());
    if (hEventSource == NULL)
      return;

    if (level != LogInfo && err != 0)
      sprintf(&msg[strlen(msg)], "\nError code = %d", err);

    LPCTSTR strings[2];
    strings[0] = msg;
    strings[1] = level != LogFatal ? "" : " Program aborted.";

    static const WORD levelType[NumLogLevels] = {
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_ERROR_TYPE,
      EVENTLOG_WARNING_TYPE,
      EVENTLOG_INFORMATION_TYPE
    };
    ReportEvent(hEventSource, // handle of event source
                levelType[level],     // event type
                0,                    // event category
                1,                    // event ID
                NULL,                 // current user's SID
                PARRAYSIZE(strings),  // number of strings
                0,                    // no bytes of raw data
                strings,              // array of error strings
                NULL);                // no raw data
    DeregisterEventSource(hEventSource);
  }
}


void PServiceProcess::OnStop()
{
}


BOOL PServiceProcess::OnPause()
{
  SuspendThread(threadHandle);
  return TRUE;
}


void PServiceProcess::OnContinue()
{
  ResumeThread(threadHandle);
}


class P_SC_HANDLE
{
  public:
    P_SC_HANDLE()
        { h = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); }
    P_SC_HANDLE(const P_SC_HANDLE & manager, PServiceProcess * svc)
        { h = OpenService(manager, svc->GetName(), SERVICE_ALL_ACCESS); }
    ~P_SC_HANDLE()
        { if (h != NULL) CloseServiceHandle(h); }
    operator SC_HANDLE() const
        { return h; }
    BOOL IsNULL()
        { return h == NULL; }
  private:
    SC_HANDLE h;
};


PServiceProcess::ProcessCommandResult
                             PServiceProcess::ProcessCommand(const char * cmd)
{
  static const char * commandNames[] = {
    "debug", "install", "remove", "start", "stop", "pause", "resume"
  };
  
  for (PINDEX cmdNum = 0; cmdNum < PARRAYSIZE(commandNames); cmdNum++)
    if (stricmp(cmd, commandNames[cmdNum]) == 0)
      break;

  if (cmdNum >= PARRAYSIZE(commandNames)) {
    if (*cmd != '\0')
      PError << "Unknown command \"" << cmd << "\".\n";
    PError << "usage: " << GetName() << " [ ";
    for (cmdNum = 0; cmdNum < PARRAYSIZE(commandNames)-1; cmdNum++)
      PError << commandNames[cmdNum] << " | ";
    PError << commandNames[cmdNum] << " ]" << endl;
    return ProcessCommandError;
  }

  if (cmdNum == 0) // Debug mode
    return DebugCommandMode;

  P_SC_HANDLE schSCManager;
  if (schSCManager.IsNULL()) {
    PError << "Could not open Service Manager." << endl;
    return ProcessCommandError;
  }

  P_SC_HANDLE schService(schSCManager, this);
  if (cmdNum != 1 && schService.IsNULL()) {
    PError << "Service is not installed." << endl;
    return ProcessCommandError;
  }

  SERVICE_STATUS status;

  BOOL good = FALSE;
  switch (cmdNum) {
    case 1 : // install
      if (!schService.IsNULL()) {
        PError << "Service is already installed." << endl;
        return ProcessCommandError;
      }
      else {
        SC_HANDLE newService = CreateService(
                          schSCManager,               // SCManager database
                          GetName(),                  // name of service
                          GetName(),                  // name to display
                          SERVICE_ALL_ACCESS,         // desired access
                          SERVICE_WIN32_OWN_PROCESS,  // service type
                          SERVICE_DEMAND_START,       // start type
                          SERVICE_ERROR_NORMAL,       // error control type
                          GetFile(),                  // service's binary
                          NULL,                       // no load ordering group
                          NULL,                       // no tag identifier
                          NULL,                       // no dependencies
                          NULL,                       // LocalSystem account
                          NULL);                      // no password
        good = newService != NULL;
        if (good) {
          CloseServiceHandle(newService);
          HKEY key;
          if (RegCreateKey(HKEY_LOCAL_MACHINE,
              "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" +
                                           GetName(), &key) == ERROR_SUCCESS) {
            RegSetValueEx(key, "EventMessageFile", 0, REG_EXPAND_SZ,
                   (LPBYTE)(const char *)GetFile(), GetFile().GetLength() + 1);
            DWORD dwData = EVENTLOG_ERROR_TYPE |
                             EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
            RegSetValueEx(key, "TypesSupported",
                                 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
            RegCloseKey(key);
          }
        }
      }
      break;

    case 2 : // remove
      good = DeleteService(schService);
      if (good) {
        PString name = "SYSTEM\\CurrentControlSet\\Services\\"
                                         "EventLog\\Application\\" + GetName();
        RegDeleteValue(HKEY_LOCAL_MACHINE, (char *)(const char *)name);
      }
      break;

    case 3 : // start
      good = StartService(schService, 0, NULL);
      break;

    case 4 : // stop
      good = ControlService(schService, SERVICE_CONTROL_STOP, &status);
      break;

    case 5 : // pause
      good = ControlService(schService, SERVICE_CONTROL_PAUSE, &status);
      break;

    case 6 : // resume
      good = ControlService(schService, SERVICE_CONTROL_CONTINUE, &status);
  }

  DWORD err = GetLastError();
  PError << "Service command \"" << commandNames[cmdNum] << "\" ";
  if (good) {
    PError << "successful." << endl;
    return CommandProcessed;
  }
  else {
    PError << "failed - error code = " << err << endl;
    return ProcessCommandError;
  }
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
  DWORD result = WaitForSingleObject(hSemaphore, timeout.GetMilliseconds());
  PAssertOS(result != WAIT_FAILED);
  return result != WAIT_TIMEOUT;
}


void PSemaphore::Signal()
{
  if (!ReleaseSemaphore(hSemaphore, 1, NULL))
    PAssertOS(GetLastError() == ERROR_TOO_MANY_POSTS);
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
