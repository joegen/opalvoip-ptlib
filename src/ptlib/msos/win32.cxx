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
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>

#include <ptlib/msos/ptlib/debstrm.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef __MINGW32__
#include <process.h>
#endif

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
  #include <process.h>
  #pragma comment(lib, "mpr.lib")
#endif

#if defined(_WIN32_DCOM) 
  #include <objbase.h>
  #ifdef _MSC_VER
    #pragma comment(lib, _OLE_LIB)
  #endif
#endif

#include "../common/pglobalstatic.cxx"


#define new PNEW


///////////////////////////////////////////////////////////////////////////////
// PTime

PTime::PTime()
{
  // Magic constant to convert epoch from 1601 to 1970
  static const PInt64 delta = ((PInt64)369*365+(369/4)-3)*24*60*60U;
  static const PInt64 scale = 10000000;

  PInt64 timestamp;

#ifndef _WIN32_WCE
  GetSystemTimeAsFileTime((LPFILETIME)&timestamp);
#else
  SYSTEMTIME SystemTime;
  GetSystemTime(&SystemTime);
  SystemTimeToFileTime(&SystemTime, (LPFILETIME)&timestamp);
#endif

  theTime = (time_t)(timestamp/scale - delta);
  microseconds = (long)(timestamp%scale/10);
}

#ifdef UNICODE
static void PWIN32GetLocaleInfo(LCID Locale,LCTYPE LCType,LPSTR lpLCData,int cchData)
{
  TCHAR* pw = new TCHAR[cchData+1];
  GetLocaleInfo(Locale,LCType,pw,cchData);
  lpLCData[0]=0;
  WideCharToMultiByte(GetACP(), 0, pw, -1, lpLCData, cchData, NULL, NULL);
}
#else

#define PWIN32GetLocaleInfo GetLocaleInfo

#endif



PString PTime::GetTimeSeparator()
{
  PString str;
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_STIME, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


BOOL PTime::GetTimeAMPM()
{
  char str[2];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_ITIME, str, sizeof(str));
  return str[0] == '0';
}


PString PTime::GetTimeAM()
{
  PString str;
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_S1159, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetTimePM()
{
  PString str;
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_S2359, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetDayName(Weekdays dayOfWeek, NameType type)
{
  PString str;
  // Of course Sunday is 6 and Monday is 1...
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), (dayOfWeek+6)%7 +
          (type == Abbreviated ? LOCALE_SABBREVDAYNAME1 : LOCALE_SDAYNAME1),
          str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetDateSeparator()
{
  PString str;
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_SDATE, str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetMonthName(Months month, NameType type)
{
  PString str;
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), month-1 +
      (type == Abbreviated ? LOCALE_SABBREVMONTHNAME1 : LOCALE_SMONTHNAME1),
      str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PTime::DateOrder PTime::GetDateOrder()
{
  char str[2];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_IDATE, str, sizeof(str));
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
  return (const wchar_t *)(type == StandardTime ? tz.StandardName : tz.DaylightName);
}


///////////////////////////////////////////////////////////////////////////////
// PTimeInterval 

static unsigned GetDivisor()
{
  LARGE_INTEGER frequency;
  if (QueryPerformanceFrequency(&frequency))
    return (unsigned)frequency.QuadPart/1000;

  return 0;
}

PTimeInterval PTimer::Tick()
{
  static unsigned divisor = GetDivisor();

  if (divisor == 0)
    return (int)(GetTickCount()&0x7fffffff);
  
  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);
  return count.QuadPart/divisor;
}


unsigned PTimer::Resolution()
{
  LARGE_INTEGER frequency;
  if (QueryPerformanceFrequency(&frequency) && frequency.QuadPart >= 1000)
    return 1;

#ifndef _WIN32_WCE
  DWORD err = GetLastError();
  DWORD timeAdjustment;
  DWORD timeIncrement;
  BOOL timeAdjustmentDisabled;
  if (GetSystemTimeAdjustment(&timeAdjustment, &timeIncrement, &timeAdjustmentDisabled))
    return timeIncrement/10000;

  err = GetLastError();
#endif

  return 55;
}


///////////////////////////////////////////////////////////////////////////////
// Directories

void PDirectory::Construct()
{
  hFindFile = INVALID_HANDLE_VALUE;
  fileinfo.cFileName[0] = '\0';
  PCaselessString::AssignContents(CreateFullPath(*this, TRUE));
}


void PDirectory::CopyContents(const PDirectory & dir)
{
  scanMask  = dir.scanMask;
  hFindFile = INVALID_HANDLE_VALUE;
  fileinfo  = dir.fileinfo;
}


BOOL PDirectory::Open(int newScanMask)
{
  scanMask = newScanMask;
#ifdef UNICODE
  USES_CONVERSION;
  hFindFile = FindFirstFile(A2T(operator+("*.*")), &fileinfo);
#else
  hFindFile = FindFirstFile(operator+("*.*"), &fileinfo);
#endif
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
#ifdef _WIN32_WCE
  return PCaselessString("\\");
#else
  char volName[100];
  PAssertOS(GetVolumeInformation(NULL, volName, sizeof(volName), NULL, NULL, NULL, NULL, 0));
  return PCaselessString(volName);
#endif
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
  if (path.IsEmpty() && !isDirectory)
    return path;

#ifdef _WIN32_WCE //doesn't support Current Directory so the path suppose to be full
  PString fullpath=path;
  PINDEX len = fullpath.GetLength();

#else
  PString partialpath = path;

  // Look for special case of "\c:\" at start of string as some generalised
  // directory processing algorithms have a habit of adding a leading
  // PDIR_SEPARATOR as it would be for Unix.
  if (partialpath.NumCompare("\\\\\\") == EqualTo ||
        (partialpath.GetLength() > 3 &&
         partialpath[0] == PDIR_SEPARATOR &&
         partialpath[2] == ':'))
    partialpath.Delete(0, 1);

  LPSTR dummy;
  PString fullpath;
  PINDEX len = (PINDEX)GetFullPathName(partialpath,
                           _MAX_PATH, fullpath.GetPointer(_MAX_PATH), &dummy);
#endif
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

#ifndef _WIN32_WCE
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
    if (strcasecmp(fsName, "FAT32") == 0) {
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
                        &totalNumberOfClusters)) 
{
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
#elif _WIN32_WCE < 300
  USES_CONVERSION;
    ULARGE_INTEGER freeBytesAvailableToCaller;
    ULARGE_INTEGER totalNumberOfBytes; 
    ULARGE_INTEGER totalNumberOfFreeBytes;
    if (GetDiskFreeSpaceEx(A2T(root),
                           &freeBytesAvailableToCaller,
                           &totalNumberOfBytes,
                           &totalNumberOfFreeBytes)) 
  {
    total = totalNumberOfBytes.QuadPart;
    free = totalNumberOfFreeBytes.QuadPart;
    clusterSize = 512; //X3
    return TRUE;
  }
  return FALSE;
#else
  return FALSE;
#endif
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

PString PChannel::GetErrorText(Errors lastError, int osError)
{
  if (osError == 0) {
    if (lastError == NoError)
      return PString();

    static int const errors[NumNormalisedErrors] = {
      0, ENOENT, EEXIST, ENOSPC, EACCES, EBUSY, EINVAL, ENOMEM, EBADF, EAGAIN, EINTR,
      WSAEMSGSIZE|PWIN32ErrorFlag, EIO, 0x1000000|PWIN32ErrorFlag
    };
    osError = errors[lastError];
  }
#ifndef _WIN32_WCE
  if (osError > 0 && osError < _sys_nerr && _sys_errlist[osError][0] != '\0')
    return _sys_errlist[osError];
#endif
  if ((osError & PWIN32ErrorFlag) == 0)
    return psprintf("C runtime error %u", osError);

  DWORD err = osError & ~PWIN32ErrorFlag;

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
    { 0x1000000,                "High level protocol failure" }
  };

  for (PINDEX i = 0; i < PARRAYSIZE(win32_errlist); i++)
    if (win32_errlist[i].id == err)
      return win32_errlist[i].msg;

  return psprintf("WIN32 error %u", err);
}


BOOL PChannel::ConvertOSError(int status, Errors & lastError, int & osError)
{
  if (status >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

  if (status != -2)
    osError = errno;
  else {
    osError = GetLastError();
    switch (osError) {
      case ERROR_INVALID_HANDLE :
      case WSAEBADF :
      case WSAENOTSOCK :
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
        osError |= PWIN32ErrorFlag;
        lastError = BufferTooSmall;
        return FALSE;
      case WSAEWOULDBLOCK :
      case WSAETIMEDOUT :
        osError |= PWIN32ErrorFlag;
        lastError = Timeout;
        return FALSE;
      default :
        osError |= PWIN32ErrorFlag;
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
  hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
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
  thread->SetThreadName(thread->GetThreadName());

  PProcess & process = PProcess::Current();

/*
 * Removed this code because it causes a linear increase
 * in thread startup time when there are many (< 500) threads.
 * If this functionality is needed, call Win32AttachThreadInput
 * after the thread has been started
 *
#ifndef _WIN32_WCE
  AttachThreadInput(thread->threadId, ((PThread&)process).threadId, TRUE);
  AttachThreadInput(((PThread&)process).threadId, thread->threadId, TRUE);
#endif
*/

  process.activeThreadMutex.Wait();
  process.activeThreads.SetAt(thread->threadId, thread);
  process.activeThreadMutex.Signal();

  process.SignalTimerChange();

#if defined(_WIN32_DCOM)
  ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

  process.OnThreadStart(*thread);
  thread->Main();
  process.OnThreadEnded(*thread);

#if defined(_WIN32_DCOM)
  ::CoUninitialize();
#endif

  return 0;
}


void PThread::Win32AttachThreadInput()
{
#ifndef _WIN32_WCE
  PProcess & process = PProcess::Current();
  ::AttachThreadInput(threadId, ((PThread&)process).threadId, TRUE);
  ::AttachThreadInput(((PThread&)process).threadId, threadId, TRUE);
#endif
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel,
                 const PString & name)
  : threadName(name)
{
  PAssert(stackSize > 0, PInvalidParameter);
  originalStackSize = stackSize;

  autoDelete = deletion == AutoDeleteThread;

#ifndef _WIN32_WCE
  threadHandle = (HANDLE)_beginthreadex(NULL, stackSize, MainFunction,
                                        this, CREATE_SUSPENDED, &threadId);
#else
   threadHandle = CreateThread(NULL, stackSize, 
                               (LPTHREAD_START_ROUTINE)MainFunction,
                               this, CREATE_SUSPENDED, (LPDWORD) &threadId);
#endif

  PAssertOS(threadHandle != NULL);

  SetPriority(priorityLevel);

#if PTRACING
  traceBlockIndentLevel = 0;
#endif

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

#ifndef _WIN32_WCE
  threadHandle = (HANDLE)_beginthreadex(NULL,
                         originalStackSize, MainFunction, this, 0, &threadId);
#else
   threadHandle = CreateThread(NULL, originalStackSize, 
                (LPTHREAD_START_ROUTINE) MainFunction,
                  this, 0, (LPDWORD) &threadId);
#endif
  PAssertOS(threadHandle != NULL);
}


void PThread::Terminate()
{
  PAssert(originalStackSize > 0, PLogicError);

  if (Current() == this)
    ExitThread(0);
  else
    TerminateThread(threadHandle, 1);
}


BOOL PThread::IsTerminated() const
{
  if (this == PThread::Current())
    return FALSE;

  return WaitForTermination(0);
}


void PThread::WaitForTermination() const
{
  WaitForTermination(PMaxTimeInterval);
}


BOOL PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  if ((this == PThread::Current()) || threadHandle == NULL) {
    PTRACE(3, "PWLib\tWaitForTermination short circuited");
    return TRUE;
  }

  DWORD result;
  PINDEX retries = 10;
  while ((result = WaitForSingleObject(threadHandle, maxWait.GetInterval())) != WAIT_TIMEOUT) {
    if (result == WAIT_OBJECT_0)
      return TRUE;

    if (::GetLastError() != ERROR_INVALID_HANDLE) {
      PAssertAlways(POperatingSystemError);
      return TRUE;
    }

    if (retries == 0)
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
  SuspendThread(threadHandle);
  return ResumeThread(threadHandle) > 1;
}


void PThread::SetAutoDelete(AutoDeleteFlag deletion)
{
  PAssert(deletion != AutoDeleteThread || this != &PProcess::Current(), PLogicError);

  PProcess & process = PProcess::Current();

  if (autoDelete && deletion != AutoDeleteThread) {
    process.deleteThreadMutex.Wait();
    process.autoDeleteThreads.DisallowDeleteObjects();
    process.autoDeleteThreads.Remove(this);
    process.autoDeleteThreads.AllowDeleteObjects();
    process.deleteThreadMutex.Signal();
  }
  else if (!autoDelete && deletion == AutoDeleteThread) {
    process.deleteThreadMutex.Wait();
    process.autoDeleteThreads.Append(this);
    process.deleteThreadMutex.Signal();
  }

  autoDelete = deletion == AutoDeleteThread;
}

#if !defined(_WIN32_WCE) || (_WIN32_WCE < 300)
#define PTHREAD_PRIORITY_LOWEST THREAD_PRIORITY_LOWEST
#define PTHREAD_PRIORITY_BELOW_NORMAL THREAD_PRIORITY_BELOW_NORMAL
#define PTHREAD_PRIORITY_NORMAL THREAD_PRIORITY_NORMAL
#define PTHREAD_PRIORITY_ABOVE_NORMAL THREAD_PRIORITY_ABOVE_NORMAL
#define PTHREAD_PRIORITY_HIGHEST THREAD_PRIORITY_HIGHEST
#else
#define PTHREAD_PRIORITY_LOWEST 243
#define PTHREAD_PRIORITY_BELOW_NORMAL 245
#define PTHREAD_PRIORITY_NORMAL 247
#define PTHREAD_PRIORITY_ABOVE_NORMAL 249
#define PTHREAD_PRIORITY_HIGHEST 251
#endif

void PThread::SetPriority(Priority priorityLevel)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  static int const priorities[NumPriorities] = {
    PTHREAD_PRIORITY_LOWEST,
    PTHREAD_PRIORITY_BELOW_NORMAL,
    PTHREAD_PRIORITY_NORMAL,
    PTHREAD_PRIORITY_ABOVE_NORMAL,
    PTHREAD_PRIORITY_HIGHEST
  };
  SetThreadPriority(threadHandle, priorities[priorityLevel]);
}


PThread::Priority PThread::GetPriority() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  switch (GetThreadPriority(threadHandle)) {
    case PTHREAD_PRIORITY_LOWEST :
      return LowestPriority;
    case PTHREAD_PRIORITY_BELOW_NORMAL :
      return LowPriority;
    case PTHREAD_PRIORITY_NORMAL :
      return NormalPriority;
    case PTHREAD_PRIORITY_ABOVE_NORMAL :
      return HighPriority;
    case PTHREAD_PRIORITY_HIGHEST :
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
#if PTRACING
  traceBlockIndentLevel = 0;
#endif
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
  : PThread(1000, NoAutoDeleteThread, NormalPriority, "PWLib Housekeeper")
{
  Resume();
}


void PProcess::HouseKeepingThread::Main()
{
  PProcess & process = PProcess::Current();

  for (;;) {

    // collect a list of thread handles to check, and clean up 
    // handles for threads that disappeared without telling us
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

        // make sure we don't put invalid handles into the list
#ifndef _WIN32_WCE
        DWORD dwFlags;
        if (GetHandleInformation(handles[numHandles], &dwFlags) == 0) {
          PTRACE(2, "PWLib\tRefused to put invalid handle into wait list");
        }
        else
#endif
        // don't put the handle for the current process in the list
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
    else if (nextTimer > 1000)
      delay = 1000;
    else
      delay = nextTimer.GetInterval();

    DWORD result;
    int retries = 100;

    while ((result = WaitForMultipleObjects(numHandles, handles, FALSE, delay)) == WAIT_FAILED) {

      // if we get an invalid handle error, than assume this is because a thread ended between
      // creating the handle list and testing it. So, cleanup the list before calling 
      // WaitForMultipleObjects again
      if (::GetLastError() == ERROR_INVALID_HANDLE)
        break;

      // sometimes WaitForMultipleObjects fails. No idea why, so allow some retries
      else {
        retries--;
        if (retries <= 0)
          break;
      }
    }
  }
}


void PProcess::SignalTimerChange()
{
  deleteThreadMutex.Wait();
  if (houseKeeper == NULL)
    houseKeeper = new HouseKeepingThread;
  else
    houseKeeper->breakBlock.Signal();
  deleteThreadMutex.Signal();
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

PProcess::~PProcess()
{
  // do whatever needs to shutdown
  PreShutdown();

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

#if _DEBUG
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

#ifdef VER_PLATFORM_WIN32_CE
    case VER_PLATFORM_WIN32_CE :
      return "CE";
#endif

    case VER_PLATFORM_WIN32_WINDOWS :
      if (info.dwMinorVersion < 10)
        return "95";
      if (info.dwMinorVersion < 90)
        return "98";
      return "ME";

    case VER_PLATFORM_WIN32_NT :
      switch (info.dwMajorVersion) {
        case 4 :
          return "NT";
        case 5:
          switch (info.dwMinorVersion) {
            case 0 :
              return "2000";
            case 1 :
              return "XP";
          }
          return "Server 2003";

        case 6 :
          return "Vista";
      }
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
          return psprintf("i586 (Model=%u Stepping=%u)", info.wProcessorRevision>>8, info.wProcessorRevision&0xff);
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
  WORD wBuildNumber = (WORD)info.dwBuildNumber;
  return psprintf(wBuildNumber > 0 ? "v%u.%u.%u" : "v%u.%u",
                  info.dwMajorVersion, info.dwMinorVersion, wBuildNumber);
}


PDirectory PProcess::GetOSConfigDir()
{
#ifdef _WIN32_WCE
  return PString("\\Windows");
#else
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
  return sysdir;  //+ "drivers\\etc";
#endif
}

PString PProcess::GetUserName() const
{
  PString username;
#ifndef _WIN32_WCE
  unsigned long size = 50;
  ::GetUserName(username.GetPointer((PINDEX)size), &size);
#else
  TCHAR wcsuser[50] = {0};
  HKEY hKeyComm, hKeyIdent;
  RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Comm"), 0, 0, &hKeyComm);
  RegOpenKeyEx(hKeyComm, _T("Ident"), 0, 0, &hKeyIdent);

  DWORD dwType = REG_SZ; DWORD dw = 50;
  if( ERROR_SUCCESS != RegQueryValueEx(
    hKeyIdent, _T("Username"), NULL, &dwType, (LPBYTE) wcsuser, &dw) 
    || !*wcsuser )
  {
  RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Ident"), 0, 0, &hKeyIdent);
  dw = 50L;
  if( ERROR_SUCCESS == RegQueryValueEx( 
    hKeyIdent, _T("Name"), NULL, &dwType, (LPBYTE) wcsuser, &dw))
      wcscat( wcsuser, _T(" user") ); // like "Pocket_PC User"
  }
  
  USES_CONVERSION;
  username = T2A(wcsuser);
#endif
  username.MakeMinimumSize();
  return username;
}


BOOL PProcess::SetUserName(const PString & username, BOOL)
{
  if (username.IsEmpty())
    return FALSE;

  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


PString PProcess::GetGroupName() const
{
  return "Users";
}


BOOL PProcess::SetGroupName(const PString & groupname, BOOL)
{
  if (groupname.IsEmpty())
    return FALSE;

  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


DWORD PProcess::GetProcessID() const
{
  return GetCurrentProcessId();
}


BOOL PProcess::IsServiceProcess() const
{
  return FALSE;
}


#ifdef _WIN32_WCE

BOOL PProcess::IsGUIProcess() const
{
  return TRUE;
}

#else

static int IsGUIProcessStatus;

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM thisProcess)
{
  char wndClassName[100];
  GetClassName(hWnd, wndClassName, sizeof(wndClassName));
  if (strcmp(wndClassName, "ConsoleWindowClass") != 0)
    return TRUE;

  DWORD wndProcess;
  GetWindowThreadProcessId(hWnd, &wndProcess);
  if (wndProcess != (DWORD)thisProcess)
    return TRUE;

  IsGUIProcessStatus = -1;
  return FALSE;
}

BOOL PProcess::IsGUIProcess() const
{
  if (IsGUIProcessStatus == 0) {
    IsGUIProcessStatus = 1;
    EnumWindows(EnumWindowsProc, GetCurrentProcessId());
  }
  return IsGUIProcessStatus > 0;
}

#endif // _WIN32_WCE


///////////////////////////////////////////////////////////////////////////////
// PSemaphore

PSemaphore::PSemaphore(HANDLE h)
{
  handle = h;
  PAssertOS(handle != NULL);
}


PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
{
  initialVal  = initial;
  maxCountVal = maxCount;

  if (initial > maxCount)
    initial = maxCount;
  handle = CreateSemaphore(NULL, initial, maxCount, NULL);
  PAssertOS(handle != NULL);
}

PSemaphore::PSemaphore(const PSemaphore & sem)
{
  initialVal  = sem.GetInitialVal();
  maxCountVal = sem.GetMaxCountVal();

  if (initialVal > maxCountVal)
    initialVal = maxCountVal;
  handle = CreateSemaphore(NULL, initialVal, maxCountVal, NULL);
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
  PSemaphore * unconst = (PSemaphore *)this;

  if (!unconst->Wait(0))
    return TRUE;

  unconst->Signal();
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PTimedMutex

PTimedMutex::PTimedMutex()
  : PSemaphore(::CreateMutex(NULL, FALSE, NULL))
{
}

PTimedMutex::PTimedMutex(const PTimedMutex &)
  : PSemaphore(::CreateMutex(NULL, FALSE, NULL))
{
}

void PTimedMutex::Signal()
{
  PAssertOS(::ReleaseMutex(handle));
}

///////////////////////////////////////////////////////////////////////////////
// PSyncPoint
PSyncPoint::PSyncPoint(unsigned maxSignals)
  : PSemaphore(::CreateEvent(NULL, FALSE, maxSignals, NULL))
{
}

PSyncPoint::PSyncPoint()
  : PSemaphore(::CreateEvent(NULL, FALSE, FALSE, NULL))
{
}

PSyncPoint::PSyncPoint(const PSyncPoint &)
  : PSemaphore(::CreateEvent(NULL, FALSE, FALSE, NULL))
{
}

void PSyncPoint::Signal()
{
  PAssertOS(::SetEvent(handle));
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
#ifdef UNICODE
  USES_CONVERSION;
  _hDLL = LoadLibrary(A2T(name));
#else
  _hDLL = LoadLibrary(name);
#endif
  return _hDLL != NULL;
}


void PDynaLink::Close()
{
  if (_hDLL != NULL) {
    FreeLibrary(_hDLL);
    _hDLL = NULL;
  }
}


BOOL PDynaLink::IsLoaded() const
{
  return _hDLL != NULL;
}


PString PDynaLink::GetName(BOOL full) const
{
  PFilePathString str;
  if (_hDLL != NULL) 
{

#ifdef UNICODE
  TCHAR path[_MAX_PATH];
    GetModuleFileName(_hDLL, path, _MAX_PATH-1);
    str=PString(path);
#else
    GetModuleFileName(_hDLL, str.GetPointer(_MAX_PATH), _MAX_PATH-1);
#endif
    if (!full) 
    {
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

#ifndef _WIN32_WCE 
  FARPROC p = GetProcAddress(_hDLL, (LPSTR)(DWORD)LOWORD(index));
#else
 FARPROC p = GetProcAddress(_hDLL, (LPTSTR)(DWORD)LOWORD(index));
#endif
  if (p == NULL)
    return FALSE;

  func = (Function)p;
  return TRUE;
}


BOOL PDynaLink::GetFunction(const PString & name, Function & func)
{
  if (_hDLL == NULL)
    return FALSE;

#ifdef UNICODE
  USES_CONVERSION;
  FARPROC p = GetProcAddress(_hDLL, A2T(name));
#else
  FARPROC p = GetProcAddress(_hDLL, name);
#endif
  if (p == NULL)
    return FALSE;

  func = (Function)p;
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// PDebugStream

PDebugStream::PDebugStream()
  : ostream(&buffer)
{
}


PDebugStream::Buffer::Buffer()
{
  setg(buffer, buffer, &buffer[sizeof(buffer)-2]);
  setp(buffer, &buffer[sizeof(buffer)-2]);
}


int PDebugStream::Buffer::overflow(int c)
{
  int bufSize = pptr() - pbase();

  if (c != EOF) {
    *pptr() = (char)c;
    bufSize++;
  }

  if (bufSize != 0) {
    char * p = pbase();
    setp(p, epptr());
    p[bufSize] = '\0';

#ifdef UNICODE
    USES_CONVERSION;
    OutputDebugString(A2T(p));
#else
    OutputDebugString(p);
#endif
  }

  return 0;
}


int PDebugStream::Buffer::underflow()
{
  return EOF;
}


int PDebugStream::Buffer::sync()
{
  return overflow(EOF);
}

// End Of File ///////////////////////////////////////////////////////////////
