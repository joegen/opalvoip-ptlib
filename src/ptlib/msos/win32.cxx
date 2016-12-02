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
#include <ptlib/msos/ptlib/pt_atl.h>

#include <process.h>
#include <errors.h>
#include <shlobj.h>
#include <Psapi.h>

#ifdef _MSC_VER
  #ifndef _WIN32_WCE
    #pragma comment(lib, "mpr.lib")
    #pragma comment(lib, "Shell32.lib")
  #endif

  #ifdef P_WIN_COM
    #pragma comment(lib, "ole32.lib")
  #endif

  #if P_DIRECTSHOW
    #pragma comment(lib, "quartz.lib")
  #endif

  #pragma comment(lib, "Psapi.lib")
#endif

#if P_VERSION_HELPERS
  #include <versionhelpers.h>
#endif

#define new PNEW


///////////////////////////////////////////////////////////////////////////////
// PTime

void PTime::SetCurrentTime()
{
  FILETIME timestamp;

#ifndef _WIN32_WCE
  GetSystemTimeAsFileTime(&timestamp);
#else
  SYSTEMTIME SystemTime;
  GetSystemTime(&SystemTime);
  SystemTimeToFileTime(&SystemTime, &timestamp);
#endif

  SetFromFileTime(timestamp);
}


PTime::PTime(const FILETIME & timestamp)
{
  SetFromFileTime(timestamp);
}


// Magic constant to convert epoch from 1601 to 1970
static const ULONGLONG Win332FileTimeDelta = ((PInt64)369*365+(369/4)-3)*24*60*60U*1000000;

void PTime::SetFromFileTime(const FILETIME & timestamp)
{
  ULARGE_INTEGER i;
  i.HighPart = timestamp.dwHighDateTime;
  i.LowPart = timestamp.dwLowDateTime;

  m_microSecondsSinceEpoch.store(i.QuadPart/10 - Win332FileTimeDelta);
}

void PTime::SetToFileTime(FILETIME & timestamp) const
{
    ULARGE_INTEGER i;
    i.QuadPart = (m_microSecondsSinceEpoch.load()+Win332FileTimeDelta)*10;
    timestamp.dwHighDateTime = i.HighPart;
    timestamp.dwLowDateTime = i.LowPart;
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
  char str[100];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_STIME, str, sizeof(str));
  return str;
}


bool PTime::GetTimeAMPM()
{
  char str[2];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_ITIME, str, sizeof(str));
  return str[0] == '0';
}


PString PTime::GetTimeAM()
{
  char str[100];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_S1159, str, sizeof(str));
  return str;
}


PString PTime::GetTimePM()
{
  char str[100];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_S2359, str, sizeof(str));
  return str;
}


PString PTime::GetDayName(Weekdays dayOfWeek, NameType type)
{
  char str[100];
  // Of course Sunday is 6 and Monday is 1...
  PWIN32GetLocaleInfo(GetUserDefaultLCID(),
                      (dayOfWeek+6)%7 + (type == Abbreviated ? LOCALE_SABBREVDAYNAME1 : LOCALE_SDAYNAME1),
                      str, sizeof(str));
  return str;
}


PString PTime::GetDateSeparator()
{
  char str[100];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_SDATE, str, sizeof(str));
  return str;
}


PString PTime::GetMonthName(Months month, NameType type)
{
  char str[100];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(),
                      month-1 + (type == Abbreviated ? LOCALE_SABBREVMONTHNAME1 : LOCALE_SMONTHNAME1),
                      str, sizeof(str));
  return str;
}


PTime::DateOrder PTime::GetDateOrder()
{
  char str[2];
  PWIN32GetLocaleInfo(GetUserDefaultLCID(), LOCALE_IDATE, str, sizeof(str));
  return (DateOrder)(str[0] - '0');
}


bool PTime::IsDaylightSavings()
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
  return PTimeInterval::MicroSeconds(count.QuadPart*1000/divisor);
}


unsigned PTimer::Resolution()
{
  LARGE_INTEGER frequency;
  if (QueryPerformanceFrequency(&frequency) && frequency.QuadPart >= 1000)
    return 1;

#ifndef _WIN32_WCE
  DWORD timeAdjustment;
  DWORD timeIncrement;
  BOOL timeAdjustmentDisabled;
  if (GetSystemTimeAdjustment(&timeAdjustment, &timeIncrement, &timeAdjustmentDisabled))
    return timeIncrement/10000;
#endif

  return 55;
}


///////////////////////////////////////////////////////////////////////////////
// Directories

void PDirectory::Construct()
{
  hFindFile = INVALID_HANDLE_VALUE;
  fileinfo.cFileName[0] = '\0';
  PCaselessString::AssignContents(PFilePath::Canonicalise(*this, true));
}


void PDirectory::CopyContents(const PDirectory & dir)
{
  m_scanMask  = dir.m_scanMask;
  hFindFile = INVALID_HANDLE_VALUE;
  fileinfo  = dir.fileinfo;
}


bool PDirectory::Open(PFileInfo::FileTypes newScanMask)
{
  m_scanMask = newScanMask;
  PVarString wildcard = *this + "*.*";

  hFindFile = FindFirstFile(wildcard, &fileinfo);
  if (hFindFile == INVALID_HANDLE_VALUE) {
    PTRACE_IF(2, GetLastError() != ERROR_PATH_NOT_FOUND && GetLastError() != ERROR_DIRECTORY,
              "PTLib", "Could not open directory \"" << *this << "\", error=" << GetLastError());
    return false;
  }

  return InternalEntryCheck() || Next();
}


bool PDirectory::Next()
{
  if (hFindFile == INVALID_HANDLE_VALUE)
    return false;

  do {
    if (!FindNextFile(hFindFile, &fileinfo))
      return false;
  } while (!InternalEntryCheck());

  return true;
}


PCaselessString PDirectory::GetEntryName() const
{
  return fileinfo.cFileName;
}


bool PDirectory::IsSubDir() const
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


PFilePathString PFilePath::Canonicalise(const PFilePathString & path, bool isDirectory)
{
  if (path.IsEmpty())
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
  DWORD len = (PINDEX)GetFullPathName(partialpath, 0, NULL, &dummy);
  if (len-- == 0)
     return PString::Empty();
   PString fullpath;
   GetFullPathName(partialpath, len+1, fullpath.GetPointerAndSetLength(len), &dummy);
#endif
  if (isDirectory && len > 0 && fullpath[len-1] != PDIR_SEPARATOR)
    fullpath += PDIR_SEPARATOR;
  PINDEX pos = 0;
  while ((pos = fullpath.Find('/', pos)) != P_MAX_INDEX)
    fullpath[pos] = PDIR_SEPARATOR;
  return fullpath;
}


typedef PBoolean (WINAPI *GetDiskFreeSpaceExType)(LPCTSTR lpDirectoryName,
                                              PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                                              PULARGE_INTEGER lpTotalNumberOfBytes,
                                              PULARGE_INTEGER lpTotalNumberOfFreeBytes);


bool PDirectory::GetVolumeSpace(PInt64 & total, PInt64 & free, DWORD & clusterSize) const
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
    return false;

#ifndef _WIN32_WCE
  PBoolean needTotalAndFree = true;

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
      needTotalAndFree = false;
    }
  }

  clusterSize = 0;
  char fsName[100];
  if (GetVolumeInformation(root, NULL, 0, NULL, NULL, NULL, fsName, sizeof(fsName))) {
    if (strcasecmp(fsName, "FAT32") == 0) {
      clusterSize = 4096; // Cannot use GetDiskFreeSpace() results for FAT32
      if (!needTotalAndFree)
        return true;
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
      return false;

    PString drive = "A:";
    while (WNetAddConnection(root, NULL, drive) != NO_ERROR) {
      if (::GetLastError() != ERROR_ALREADY_ASSIGNED)
        return false;
      drive[0]++;
    }
    PBoolean ok = GetDiskFreeSpace(drive+'\\',
                               &sectorsPerCluster,
                               &bytesPerSector,
                               &numberOfFreeClusters,
                               &totalNumberOfClusters);
    WNetCancelConnection(drive, true);
    if (!ok)
      return false;
  }

  if (needTotalAndFree) {
    free = numberOfFreeClusters*sectorsPerCluster*bytesPerSector;
    total = totalNumberOfClusters*sectorsPerCluster*bytesPerSector;
  }

  if (clusterSize == 0)
    clusterSize = bytesPerSector*sectorsPerCluster;

  return true;
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
    return true;
  }
  return false;
#else
  return false;
#endif
}


///////////////////////////////////////////////////////////////////////////////
// PFilePath

static char const IllegalFilenameCharacters[] =
  "\\/:*?\"<>|"
  "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\0x10"
  "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f";

PBoolean PFilePath::IsValid(char c)
{
  return strchr(IllegalFilenameCharacters, c) == NULL;
}


PBoolean PFilePath::IsValid(const PString & str)
{
  return str != "." && str != ".." &&
         str.FindOneOf(IllegalFilenameCharacters) == P_MAX_INDEX;
}


bool PFilePath::IsAbsolutePath(const PString & path)
{
  return path.GetLength() > 2 && (path[1] == ':' || (path[0] == '\\' && path[1] == '\\'));
}


///////////////////////////////////////////////////////////////////////////////
// PFile

bool PFile::Touch(const PFilePath & name, const PTime & accessTime, const PTime & modTime)
{
  PWin32Handle hFile(::CreateFile(name,FILE_WRITE_ATTRIBUTES,0,NULL,0,0,NULL));
  if (!hFile.IsValid())
    return false;

  FILETIME now;
  GetSystemTimeAsFileTime(&now);

  FILETIME acc;
  if (accessTime.IsValid())
    accessTime.SetToFileTime(acc);
  else
    acc = now;

  FILETIME mod;
  if (modTime.IsValid())
    modTime.SetToFileTime(mod);
  else
    mod = now;

  return ::SetFileTime(hFile, NULL, &acc, &mod);
}


///////////////////////////////////////////////////////////////////////////////
// PChannel

HANDLE PChannel::GetAsyncReadHandle() const
{
  return INVALID_HANDLE_VALUE;
}


HANDLE PChannel::GetAsyncWriteHandle() const
{
  return INVALID_HANDLE_VALUE;
}


static VOID CALLBACK StaticOnIOComplete(DWORD dwErrorCode,
                                        DWORD dwNumberOfBytesTransfered,
                                        LPOVERLAPPED lpOverlapped)
{
  ((PChannel::AsyncContext *)lpOverlapped)->OnIOComplete(dwNumberOfBytesTransfered, dwErrorCode);
}


void PChannel::AsyncContext::SetOffset(off_t offset)
{
  Offset = (DWORD)offset;
#if P_64BIT
  OffsetHigh = (DWORD)((int64_t)offset>>32);
#endif
}


bool PChannel::AsyncContext::Initialise(PChannel * channel, CompletionFunction onComplete)
{
  if (m_channel != NULL)
    return false;

  m_channel = channel;
  m_onComplete = onComplete;
  return true;
}


bool PChannel::ReadAsync(AsyncContext & context)
{
  if (CheckNotOpen())
    return false;

  HANDLE handle = GetAsyncReadHandle();
  if (handle == INVALID_HANDLE_VALUE)
    return SetErrorValues(ProtocolFailure, EFAULT);

  if (!PAssert(context.Initialise(this, &PChannel::OnReadComplete),
               "Multiple async read with same context!"))
    return SetErrorValues(ProtocolFailure, EINVAL);

  return ConvertOSError(ReadFileEx(handle,
                                   context.m_buffer,
                                   context.m_length,
                                   &context,
                                   StaticOnIOComplete) && GetLastError() == 0 ? 0 : -2, LastReadError);

}


bool PChannel::WriteAsync(AsyncContext & context)
{
  if (CheckNotOpen())
    return false;

  HANDLE handle = GetAsyncWriteHandle();
  if (handle == INVALID_HANDLE_VALUE)
    return SetErrorValues(ProtocolFailure, EFAULT);

  if (!PAssert(context.Initialise(this, &PChannel::OnWriteComplete),
               "Multiple async write with same context!"))
    return SetErrorValues(ProtocolFailure, EINVAL);

  return ConvertOSError(WriteFileEx(handle,
                                    context.m_buffer,
                                    context.m_length,
                                    &context,
                                    StaticOnIOComplete) ? 0 : -2, LastWriteError);
}


PString PChannel::GetErrorText(Errors lastError, int osError)
{
  if (osError == 0) {
    if (lastError == NoError)
      return PString();

    static int const errors[NumNormalisedErrors] = {
      0, ENOENT, EEXIST, ENOSPC, EACCES, EBUSY, EINVAL, ENOMEM, EBADF, EAGAIN, ECANCELED,
      WSAEMSGSIZE|PWIN32ErrorFlag, EIO, 0x1000000|PWIN32ErrorFlag
    };
    osError = errors[lastError];
  }
#ifndef _WIN32_WCE
  if (osError > 0 && osError < _sys_nerr && _sys_errlist[osError][0] != '\0')
    return _sys_errlist[osError];
#endif

  static const struct {
    int id1;
    int id2;
    const char * msg;
  } win32_errlist[] = {
    { PWIN32ErrorFlag|ERROR_FILE_NOT_FOUND,     ENOENT,                "File not found" },
    { PWIN32ErrorFlag|ERROR_PATH_NOT_FOUND,     ENOTDIR,               "Path not found" },
    { PWIN32ErrorFlag|ERROR_ACCESS_DENIED,      EACCES,                "Access denied" },
    { PWIN32ErrorFlag|ERROR_NOT_ENOUGH_MEMORY,  ENOMEM,                "Not enough memory" },
    { PWIN32ErrorFlag|ERROR_INVALID_FUNCTION,   EINVAL,                "Invalid function" },
    { PWIN32ErrorFlag|WSAEADDRINUSE,            EADDRINUSE,            "Address in use" },
    { PWIN32ErrorFlag|WSAEADDRNOTAVAIL,         EADDRNOTAVAIL,         "Address type not available" },
    { PWIN32ErrorFlag|WSAENETDOWN,              ENETDOWN,              "Network subsystem failed" },
    { PWIN32ErrorFlag|WSAEISCONN,               EISCONN,               "Socket is already connected" },
    { PWIN32ErrorFlag|WSAENETUNREACH,           ENETUNREACH,           "Network unreachable" },
    { PWIN32ErrorFlag|WSAEHOSTUNREACH,          EHOSTUNREACH,          "Host unreachable" },
    { PWIN32ErrorFlag|WSAECONNREFUSED,          ECONNREFUSED,          "Connection refused" },
    { PWIN32ErrorFlag|WSAEINVAL,                EINVAL,                "Invalid operation" },
    { PWIN32ErrorFlag|WSAENOTCONN,              ENOTCONN,              "Socket not connected" },
    { PWIN32ErrorFlag|WSAECONNABORTED,          ECONNABORTED,          "Connection aborted" },
    { PWIN32ErrorFlag|WSAECONNRESET,            ECONNRESET,            "Connection reset" },
    { PWIN32ErrorFlag|WSAESHUTDOWN,             -1,                    "Connection shutdown" },
    { PWIN32ErrorFlag|WSAENOTSOCK,              ENOTSOCK,              "Socket closed or invalid" },
    { PWIN32ErrorFlag|WSAETIMEDOUT,             ETIMEDOUT,             "Timed out" },
    { PWIN32ErrorFlag|WSAEMSGSIZE,              EMSGSIZE,              "Message larger than buffer" },
    { PWIN32ErrorFlag|WSAEWOULDBLOCK,           EWOULDBLOCK,           "Would block" },
    { 0x1000000,                                -1,                    "High level protocol failure" }
  };

  for (PINDEX i = 0; i < PARRAYSIZE(win32_errlist); i++) {
    if (win32_errlist[i].id1 == osError || win32_errlist[i].id2 == osError)
      return win32_errlist[i].msg;
  }

  if ((osError & PWIN32ErrorFlag) == 0)
    return psprintf("C runtime error %u", osError);
  else
    return psprintf("WIN32 error %u", osError & ~PWIN32ErrorFlag);
}


PBoolean PChannel::ConvertOSError(P_INT_PTR libcReturnValue, ErrorGroup group)
{
  int osError = GetErrorNumber(group);

  switch (libcReturnValue) {
    case -1 :
      if ((osError = os_errno()) != 0)
        break;
      // Do next case

    case -2 :
      osError = ::GetLastError();
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
        default :
          osError |= PWIN32ErrorFlag;
      }
      break;

    case -3 :
      break;

    default :
      osError = 0;
  }

  Errors lastError;
  switch (osError) {
    case 0 :
      lastError = NoError;
      break;
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
    case ENOTSOCK :
      lastError = NotOpen;
      break;
    case EAGAIN :
    case ETIMEDOUT :
    case EWOULDBLOCK :
      lastError = Timeout;
      break;
    case EMSGSIZE :
      lastError = BufferTooSmall;
      break;
    case EINTR :
    case ECANCELED :
      lastError = Interrupted;
      break;
    default :
      lastError = Miscellaneous;
  }

  return SetErrorValues(lastError, osError, group);
}


///////////////////////////////////////////////////////////////////////////////
// PWin32Overlapped

PWin32Overlapped::PWin32Overlapped()
{
  memset(this, 0, sizeof(*this));
  hEvent = CreateEvent(NULL, true, false, NULL);
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
  AttachThreadInput(thread->threadId, ((PThread&)process).threadId, true);
  AttachThreadInput(((PThread&)process).threadId, thread->threadId, true);
#endif
*/

  process.InternalThreadStarted(thread);

  process.OnThreadStart(*thread);
  thread->Main();
  process.OnThreadEnded(*thread);

  process.InternalThreadEnded(thread);
  return 0;
}


void PThread::Win32AttachThreadInput()
{
#ifndef _WIN32_WCE
  PProcess & process = PProcess::Current();
  ::AttachThreadInput(m_threadId, ((PThread&)process).m_threadId, true);
  ::AttachThreadInput(((PThread&)process).m_threadId, m_threadId, true);
#endif
}


PThread::PThread(bool isProcess)
  : m_type(isProcess ? e_IsProcess : e_IsExternal)
  , m_originalStackSize(0)
  , m_threadId(GetCurrentThreadId())
#if defined(P_WIN_COM)
  , m_comInitialised(false)
#endif
{
  if (isProcess) {
    m_threadHandle = GetCurrentThread();
    return;
  }

  m_threadHandle.Duplicate(GetCurrentThread());

  PProcess::Current().InternalThreadStarted(this);
}


PThread::PThread(PINDEX stackSize,
                 AutoDeleteFlag deletion,
                 Priority priorityLevel,
                 const PString & name)
  : m_type(deletion == AutoDeleteThread ? e_IsAutoDelete : e_IsManualDelete)
  , m_originalStackSize(std::max(stackSize, (PINDEX)65535))
  , m_threadName(name)
#if defined(P_WIN_COM)
  , m_comInitialised(false)
#endif
{
  PAssert(m_originalStackSize > 0, PInvalidParameter);

#ifndef _WIN32_WCE
  m_threadHandle = (HANDLE)_beginthreadex(NULL, m_originalStackSize, MainFunction, this, CREATE_SUSPENDED, &m_threadId);
#else
  m_threadHandle = CreateThread(NULL, m_originalStackSize, 
                       (LPTHREAD_START_ROUTINE)MainFunction, this, CREATE_SUSPENDED, (LPDWORD) &m_threadId);
#endif

  PAssertOS(m_threadHandle.IsValid());

  SetPriority(priorityLevel);
}


#ifdef P_WIN_COM
static atomic<bool> s_securityInitialised(false);

bool PThread::CoInitialise()
{
  if (m_comInitialised)
    return true;

  HRESULT result = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(result)) {
    PTRACE_IF(1, result != RPC_E_CHANGED_MODE, "PTLib", "Could not initialise COM: error=0x" << hex << result);
    return false;
  }

  if (!s_securityInitialised.exchange(true)) {
    result = ::CoInitializeSecurity(NULL, 
                                    -1,                          // COM authentication
                                    NULL,                        // Authentication services
                                    NULL,                        // Reserved
                                    RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
                                    RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
                                    NULL,                        // Authentication info
                                    EOAC_NONE,                   // Additional capabilities 
                                    NULL                         // Reserved
                                    );
    PTRACE_IF(2, result, "PTLib", "Could not initialise COM security: error=0x" << hex << result);
  }

  m_comInitialised = true;
  return true;
}


std::ostream & operator<<(std::ostream & strm, const PComResult & result)
{
  TCHAR msg[MAX_ERROR_TEXT_LEN+1];
  DWORD dwMsgLen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,
                                 result.m_result,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 msg, sizeof(msg),
                                 NULL);
  while (dwMsgLen > 0 && isspace(msg[dwMsgLen-1]))
    msg[--dwMsgLen] = '\0';
  if (dwMsgLen > 0)
    return strm << msg;

#if P_DIRECTSHOW
  dwMsgLen = AMGetErrorText(result.m_result, msg, sizeof(msg));
  while (dwMsgLen > 0 && isspace(msg[dwMsgLen-1]))
    msg[--dwMsgLen] = '\0';
  if (dwMsgLen > 0)
    return strm << msg;
#endif

  return strm << "0x" << hex << result.m_result << dec;
}


#if PTRACING
bool PComResult::Succeeded(HRESULT result, const char * func, const char * file, int line, HRESULT nomsg1, HRESULT nomsg2)
{
  if (Succeeded(result))
    return true;

  // Don't look at "Facility" code when filterring errors.
  if (nomsg1 >= 0 && LOWORD(result) == LOWORD(nomsg1))
    return false;

  if (nomsg2 >= 0 && LOWORD(result) == LOWORD(nomsg2))
    return false;

  static const int Level = 2;
  if (PTrace::CanTrace(Level))
    PTrace::Begin(Level, file, line) << "Function \"" << func << "\" failed, "
           "error=0x" << hex << result << dec << " : " << *this << PTrace::End;
  return false;
}
#endif // PTRACING


std::ostream & operator<<(std::ostream & strm, const PComVariant & var)
{
  switch (var.vt) {
    case VT_BSTR :
      return strm << PString(var.bstrVal);
    case VT_I2 :
      return strm << var.iVal;
    case VT_I4 :
      return strm << var.lVal;
    case VT_I8 :
      return strm << var.llVal;
    case VT_R4 :
      return strm << var.fltVal;
    case VT_R8 :
      return strm << var.dblVal;
  }
  return strm;
}

#endif // P_WIN_COM


__inline static ULONGLONG GetMillisecondFromFileTime(const FILETIME & ft)
{
  return (reinterpret_cast<const ULARGE_INTEGER *>(&ft)->QuadPart+9999)/10000;
}


struct PWindowsTimes
{
  FILETIME m_created;
  FILETIME m_exit;
  FILETIME m_kernel;
  FILETIME m_user;
  FILETIME m_idle;

  PWindowsTimes()
  {
    m_exit.dwHighDateTime = m_exit.dwLowDateTime = m_idle.dwHighDateTime = m_idle.dwLowDateTime = 0;
  }

  bool FromThread(HANDLE handle)  { return GetThreadTimes(handle, &m_created, &m_exit, &m_kernel, &m_user); }
  bool FromProcess(HANDLE handle) { return GetProcessTimes(handle, &m_created, &m_exit, &m_kernel, &m_user); }
  bool FromSystem()               { return GetSystemTimes(&m_idle, &m_kernel, &m_user); }

  void ToTimes(PThread::Times & times)
  {
    times.m_kernel.SetInterval(GetMillisecondFromFileTime(m_kernel));
    times.m_user.SetInterval(GetMillisecondFromFileTime(m_user));
    if (m_idle.dwHighDateTime != 0 || m_idle.dwLowDateTime != 0)
      times.m_real.SetInterval(GetMillisecondFromFileTime(m_kernel) + GetMillisecondFromFileTime(m_user) + GetMillisecondFromFileTime(m_idle));
    else {
      if (m_exit.dwHighDateTime == 0 && m_exit.dwLowDateTime == 0)
        GetSystemTimeAsFileTime(&m_exit);
      times.m_real.SetInterval(GetMillisecondFromFileTime(m_exit) - GetMillisecondFromFileTime(m_created));
    }
  }
};

bool PThread::GetTimes(Times & times)
{
  // Do not use any PTLib functions in here as they could to a PTRACE, and this deadlock
  times.m_name = GetThreadName();
  times.m_uniqueId = times.m_threadId = m_threadId;

  PWindowsTimes wt;
  if (!wt.FromThread(GetHandle()))
    return false;

  wt.ToTimes(times);
  return true;
}


void PThread::InternalDestroy()
{
  if (m_type == e_IsProcess)
    m_threadHandle.Detach();
  else
    m_threadHandle.Close();

#if defined(P_WIN_COM)
  if (m_comInitialised)
    ::CoUninitialize();
#endif
}


void PThread::Restart()
{
  if (!PAssert(m_originalStackSize != 0, "Cannot restart process/external thread") ||
      !PAssert(IsTerminated(), "Cannot restart running thread"))
    return;

  InternalDestroy();

#ifndef _WIN32_WCE
  m_threadHandle = (HANDLE)_beginthreadex(NULL, m_originalStackSize, MainFunction, this, 0, &m_threadId);
#else
  m_threadHandle = CreateThread(NULL, m_originalStackSize, 
                                  (LPTHREAD_START_ROUTINE)MainFunction, this, 0, (LPDWORD)&m_threadId);
#endif
  PAssertOS(m_threadHandle.IsValid());
}


void PThread::Terminate()
{
  if (PAssert(m_type != e_IsProcess, "Cannot terminate the process!") &&
       m_threadHandle.IsValid() &&
      !m_threadHandle.Wait(0)) {
    PTRACE(2, "PTLib\tTerminating thread " << *this);
    TerminateThread(m_threadHandle, 1);
  }
}


PBoolean PThread::IsTerminated() const
{
  return m_threadHandle.Wait(0);
}


void PThread::WaitForTermination() const
{
  WaitForTermination(PMaxTimeInterval);
}


PBoolean PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  if (!m_threadHandle.IsValid())
    return true;

  if (GetThreadId() == GetCurrentThreadId()) {
    PTRACE(3, "PTLib\tWaitForTermination short circuited");
    return true;
  }

  return m_threadHandle.Wait(maxWait.GetInterval());
}


void PThread::Suspend(PBoolean susp)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  if (susp)
    SuspendThread(m_threadHandle);
  else
    Resume();
}


void PThread::Resume()
{
  PAssert(!IsTerminated(), "Operation on terminated thread");
  ResumeThread(m_threadHandle);
}


PBoolean PThread::IsSuspended() const
{
  if (GetThreadId() == GetCurrentThreadId())
    return false;

  SuspendThread(m_threadHandle);
  return ResumeThread(m_threadHandle) > 1;
}


void PThread::SetPriority(Priority priorityLevel)
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  static int const priorities[NumPriority] = {
    THREAD_PRIORITY_LOWEST,
    THREAD_PRIORITY_BELOW_NORMAL,
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_ABOVE_NORMAL,
    THREAD_PRIORITY_HIGHEST
  };
  SetThreadPriority(m_threadHandle, priorities[priorityLevel]);
}


PThread::Priority PThread::GetPriority() const
{
  PAssert(!IsTerminated(), "Operation on terminated thread");

  switch (GetThreadPriority(m_threadHandle)) {
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


void PThread::Sleep(const PTimeInterval & delay)
{
  ::SleepEx(delay.GetInterval(), TRUE);
}


void PThread::Yield()
{
  ::Sleep(0);
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

PProcess::~PProcess()
{
  // do whatever needs to shutdown
  PreShutdown();
  WaitOnExitConsoleWindow();
  PostShutdown();
}


PString PProcess::GetOSClass()
{
  return "Windows";
}


PString PProcess::GetOSName()
{
#if P_VERSION_HELPERS
  if (IsWindowsVersionOrGreater(10,0,0))
    return "10";
  if (IsWindows8Point1OrGreater())
    return "8.1";
  if (IsWindows8OrGreater())
    return "8";
  if (IsWindows7SP1OrGreater())
    return "7 sp1";
  if (IsWindows7OrGreater())
    return "7";
  if (IsWindowsVistaSP2OrGreater())
    return "Vista sp2";
  if (IsWindowsVistaSP1OrGreater())
    return "Vista sp1";
  if (IsWindowsVistaOrGreater())
    return "Vista";
  if (IsWindowsServer())
    return "Server 2003";
  if (IsWindowsXPSP3OrGreater())
    return "XP sp3";
  if (IsWindowsXPOrGreater())
    return "XP";
#else
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
          switch (info.dwMinorVersion) {
            case 0 :
              return "Vista";
            case 1 :
              return "7";
            case 2 :
              return info.dwBuildNumber < 9200 ? "8" : "8.1";
          }
      }
  }
#endif // P_VERSION_HELPERS
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

#ifdef PROCESSOR_ARCHITECTURE_AMD64
    case PROCESSOR_ARCHITECTURE_AMD64:
      return "AMD64";
#endif

  }
  return "?";
}


PString PProcess::GetOSVersion()
{
#if P_VERSION_HELPERS
  if (IsWindowsVersionOrGreater(10,0,0))
    return "v10.0";
  if (IsWindows8Point1OrGreater())
    return "v6.3";
  if (IsWindows8OrGreater())
    return "v6.2";
  if (IsWindows7SP1OrGreater())
    return "v6.1.1";
  if (IsWindows7OrGreater())
    return "v6.1";
  if (IsWindowsVistaSP2OrGreater())
    return "v6.0.2";
  if (IsWindowsVistaSP1OrGreater())
    return "v6.0.1";
  if (IsWindowsVistaOrGreater())
    return "v6.0";
  if (IsWindowsServer())
    return "v5.2";
  if (IsWindowsXPSP3OrGreater())
    return "v5.1.3";
  if (IsWindowsXPOrGreater())
    return "v5.1";
  return "?";
#else
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);
  WORD wBuildNumber = (WORD)info.dwBuildNumber;
  return psprintf(wBuildNumber > 0 ? "v%u.%u.%u" : "v%u.%u",
                  info.dwMajorVersion, info.dwMinorVersion, wBuildNumber);
#endif // P_VERSION_HELPERS
}


bool PProcess::IsOSVersion(unsigned major, unsigned minor, unsigned build)
{
#if P_VERSION_HELPERS
  OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
  DWORDLONG        const dwlConditionMask = VerSetConditionMask(
    VerSetConditionMask(
    VerSetConditionMask(
    0, VER_MAJORVERSION, VER_GREATER_EQUAL),
    VER_MINORVERSION, VER_GREATER_EQUAL),
    VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

  osvi.dwMajorVersion = major;
  osvi.dwMinorVersion = minor;
  osvi.wServicePackMajor = (WORD)build;

  return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
#else
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);

  if (info.dwMajorVersion < major)
    return false;
  if (info.dwMajorVersion > major)
    return true;

  if (info.dwMinorVersion < minor)
    return false;
  if (info.dwMinorVersion > minor)
    return true;

  return info.dwBuildNumber >= build;
#endif // P_VERSION_HELPERS
}


PDirectory PProcess::GetOSConfigDir()
{
#ifdef _WIN32_WCE
  return PString("\\Windows");
#else
  char dir[_MAX_PATH];

  PAssertOS(GetSystemDirectory(dir, sizeof(dir)) != 0);
  PDirectory sysdir = dir;
  return sysdir;  //+ "drivers\\etc";
#endif
}

PString PProcess::GetUserName() const
{
#ifndef _WIN32_WCE
  char username[100];
  DWORD size = sizeof(username);
  return ::GetUserName(username, &size) ? PString(username, size-1) : PString::Empty();
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
  
  return wcsuser;
#endif
}


PBoolean PProcess::SetUserName(const PString & username, PBoolean)
{
  if (username.IsEmpty())
    return false;

  if (username == GetUserName())
    return true;

  PAssertAlways(PUnimplementedFunction);
  return false;
}


PDirectory PProcess::GetHomeDirectory() const
{
  const char * dir;
  if ((dir = getenv("HOME")) != NULL)
    return dir;

  TCHAR szPath[MAX_PATH];
  if (SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, szPath) == S_OK)
    return szPath;

  if ((dir = getenv("USERPROFILE")) != NULL)
    return dir;

  return ".";
}


PString PProcess::GetGroupName() const
{
  return "Users";
}


PBoolean PProcess::SetGroupName(const PString & groupname, PBoolean)
{
  if (groupname.IsEmpty())
    return false;

  PAssertAlways(PUnimplementedFunction);
  return false;
}


PProcessIdentifier PProcess::GetCurrentProcessID()
{
  return ::GetCurrentProcessId();
}


PBoolean PProcess::IsServiceProcess() const
{
  return false;
}


void PProcess::GetMemoryUsage(MemoryUsage & usage)
{
  PROCESS_MEMORY_COUNTERS info;
  info.cb = sizeof(info);
  if (GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) {
    usage.m_virtual = info.PeakWorkingSetSize;
    usage.m_resident = info.WorkingSetSize;
  }
}


bool PProcess::GetProcessTimes(Times & times) const
{
  times.m_name = GetName();

  PWindowsTimes wt;
  if (!wt.FromProcess(GetCurrentProcess()))
    return false;

  wt.ToTimes(times);
  return true;
}


bool PProcess::GetSystemTimes(Times & times)
{
  times.m_name = "SYSTEM";

  PWindowsTimes wt;
  if (!wt.FromSystem())
    return false;

  wt.ToTimes(times);
  return true;
}


unsigned PThread::GetNumProcessors()
{
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
}


#ifdef _WIN32_WCE

PBoolean PProcess::IsGUIProcess() const
{
  return true;
}

#else

static bool s_IsGUIProcess = true;
static atomic<bool> s_checkGUIProcess(false);

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

  s_IsGUIProcess = false;
  return FALSE;
}

PBoolean PProcess::IsGUIProcess() const
{
  if (!s_checkGUIProcess.exchange(true))
    EnumWindows(EnumWindowsProc, GetCurrentProcessId());
  return s_IsGUIProcess;
}

#endif // _WIN32_WCE


///////////////////////////////////////////////////////////////////////////////

void PProcess::HostSystemURLHandlerInfo::SetIcon(const PString & _icon)
{
  PString icon(_icon);
  PFilePath exe(PProcess::Current().GetFile());
  icon.Replace("%exe",  exe, true);
  icon.Replace("%base", exe.GetFileName(), true);
  iconFileName = icon;
}

PString PProcess::HostSystemURLHandlerInfo::GetIcon() const 
{
  return iconFileName;
}

void PProcess::HostSystemURLHandlerInfo::SetCommand(const PString & key, const PString & _cmd)
{
  PString cmd(_cmd);

  // do substitutions
  PFilePath exe(PProcess::Current().GetFile());
  cmd.Replace("%exe", "\"" + exe + "\"", true);
  cmd.Replace("%1",   "\"%1\"", true);

  // save command
  cmds.SetAt(key, cmd);
}

PString PProcess::HostSystemURLHandlerInfo::GetCommand(const PString & key) const
{
  return cmds(key);
}

bool PProcess::HostSystemURLHandlerInfo::GetFromSystem()
{
  if (type.IsEmpty())
    return false;

  // get icon file
  {
    RegistryKey key("HKEY_CLASSES_ROOT\\" + type + "\\DefaultIcon", RegistryKey::ReadOnly);
    key.QueryValue("", iconFileName);
  }

  // enumerate the commands
  {
    PString keyRoot("HKEY_CLASSES_ROOT\\" + type + "\\");
    RegistryKey key(keyRoot + "shell", RegistryKey::ReadOnly);
    PString str;
    for (PINDEX idx = 0; key.EnumKey(idx, str); ++idx) {
      RegistryKey cmd(keyRoot + "shell\\" + str + "\\command", RegistryKey::ReadOnly);
      PString value;
      if (cmd.QueryValue("", value)) 
        cmds.SetAt(str, value);
    }
  }

  return true;
}

bool PProcess::HostSystemURLHandlerInfo::CheckIfRegistered()
{
  // if no type information in system, definitely not registered
  HostSystemURLHandlerInfo currentInfo(type);
  if (!currentInfo.GetFromSystem()) 
    return false;

  // check icon file
  if (!iconFileName.IsEmpty() && !(iconFileName *= currentInfo.GetIcon()))
    return false;

  // check all of the commands
  return (currentInfo.cmds.GetSize() != 0) && (currentInfo.cmds == cmds);
}

bool PProcess::HostSystemURLHandlerInfo::Register()
{
  if (type.IsEmpty())
    return false;

  // delete any existing icon name
  {
    RegistryKey key("HKEY_CLASSES_ROOT\\" + type, RegistryKey::ReadOnly);
    key.DeleteKey("DefaultIcon");
  }

  // set icon file
  if (!iconFileName.IsEmpty()) {
    RegistryKey key("HKEY_CLASSES_ROOT\\" + type + "\\DefaultIcon", RegistryKey::Create);
    key.SetValue("", iconFileName);
  }

  // delete existing commands
  PString keyRoot("HKEY_CLASSES_ROOT\\" + type);
  {
    RegistryKey keyShell(keyRoot + "\\shell", RegistryKey::ReadOnly);
    PString str;
    for (PINDEX idx = 0; keyShell.EnumKey(idx, str); ++idx) {
      {
        RegistryKey key(keyRoot + "\\shell\\" + str, RegistryKey::ReadOnly);
        key.DeleteKey("command");
      }
      {
        RegistryKey key(keyRoot + "\\shell", RegistryKey::ReadOnly);
        key.DeleteKey(str);
      }
    }
  }

  // create new commands
  {
    RegistryKey key3(keyRoot, RegistryKey::Create);
    key3.SetValue("", type & "protocol");
    key3.SetValue("URL Protocol", "");

    RegistryKey key2(keyRoot + "\\shell",  RegistryKey::Create);

    for (PStringToString::iterator it = cmds.begin(); it != cmds.end(); ++it) {
      RegistryKey key1(keyRoot + "\\shell\\" + it->first,              RegistryKey::Create);
      RegistryKey key(keyRoot + "\\shell\\" + it->first + "\\command", RegistryKey::Create);
      key.SetValue("", it->second);
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// PSemaphore

void PSemaphore::Reset(unsigned initial, unsigned maximum)
{
  m_maximum = std::min(maximum, (unsigned)INT_MAX);
  m_initial = std::min(initial, m_maximum);

  m_handle.Close();
  m_handle = CreateSemaphore(NULL, m_initial, m_maximum, m_name.IsEmpty() ? NULL : m_name.GetPointer());
  PAssertOS(m_handle.IsValid());
}


PSemaphore::~PSemaphore()
{
}


void PSemaphore::Wait()
{
  m_handle.Wait(INFINITE);
}


PBoolean PSemaphore::Wait(const PTimeInterval & timeout)
{
  return m_handle.Wait(timeout.GetInterval());
}


void PSemaphore::Signal()
{
  if (PAssertOS(ReleaseSemaphore(m_handle, 1, NULL) || ::GetLastError() == ERROR_INVALID_HANDLE))
    SetLastError(ERROR_SUCCESS);
}


///////////////////////////////////////////////////////////////////////////////
// PTimedMutex

PTimedMutex::PTimedMutex(const char * name, unsigned line, unsigned timeout)
  : PMutexExcessiveLockInfo(name, line, timeout)
  , m_lockerId(PNullThreadIdentifier)
  , m_lastLockerId(PNullThreadIdentifier)
  , m_lastUniqueId(0)
  , m_lockCount(0)
  , m_handle(::CreateMutex(NULL, FALSE, NULL))
{
}


PTimedMutex::PTimedMutex(const PTimedMutex & other)
  : PMutexExcessiveLockInfo(other)
  , m_lockerId(PNullThreadIdentifier)
  , m_lastLockerId(PNullThreadIdentifier)
  , m_lastUniqueId(0)
  , m_lockCount(0)
  , m_handle(::CreateMutex(NULL, FALSE, NULL))
{
}


void PTimedMutex::Wait()
{
  if (!m_handle.Wait(m_excessiveLockTimeout)) {
    ExcessiveLockWait();
    m_handle.Wait(INFINITE);
    ExcessiveLockPhantom(*this);
  }

  if (m_lockCount++ == 0)
    m_lastUniqueId = m_lastLockerId = m_lockerId = ::GetCurrentThreadId();
}


PBoolean PTimedMutex::Wait(const PTimeInterval & timeout)
{
  if (!m_handle.Wait(timeout.GetInterval()))
    return false;

  if (m_lockCount++ == 0)
    m_lastUniqueId = m_lastLockerId = m_lockerId = ::GetCurrentThreadId();
  return true;
}


void PTimedMutex::Signal()
{
  if (--m_lockCount == 0)
    CommonSignal();

  PAssertOS(::ReleaseMutex(m_handle));
}


/////////////////////////////////////////////////////////////////////////////
// PSyncPoint

PSyncPoint::PSyncPoint()
  : m_handle(::CreateEvent(NULL, FALSE, FALSE, NULL))
{
}

PSyncPoint::PSyncPoint(const PSyncPoint &)
  : m_handle(::CreateEvent(NULL, FALSE, FALSE, NULL))
{
}

void PSyncPoint::Wait()
{
  m_handle.Wait(INFINITE);
}


PBoolean PSyncPoint::Wait(const PTimeInterval & timeout)
{
  return m_handle.Wait(timeout.GetInterval());
}


void PSyncPoint::Signal()
{
  PAssertOS(::SetEvent(m_handle));
}


void PWin32Handle::Close()
{
  if (IsValid()) {
    HANDLE h = m_handle; // This is a little more thread safe
    m_handle = NULL;
    PAssertOS(CloseHandle(h));
  }
}


HANDLE PWin32Handle::Detach()
{
  HANDLE h = m_handle;
  m_handle = NULL;
  return h;
}


HANDLE * PWin32Handle::GetPointer()
{
  Close();
  return &m_handle;
}


PWin32Handle & PWin32Handle::operator=(HANDLE h)
{
  Close();
  m_handle = h;
  return *this;
}


bool PWin32Handle::Wait(DWORD timeout) const
{
  int retry = 0;
  while (retry < 10) {
    DWORD tick = ::GetTickCount();
    switch (::WaitForSingleObjectEx(m_handle, timeout, TRUE)) {
      case WAIT_OBJECT_0 :
        return true;

      case WAIT_TIMEOUT :
        return false;

      case WAIT_IO_COMPLETION :
        timeout -= (::GetTickCount() - tick);
        break;

      default :
        if (::GetLastError() != ERROR_INVALID_HANDLE) {
          PAssertAlways(POperatingSystemError);
          return true;
        }
        ++retry;
    }
  }

  return false;
}


bool PWin32Handle::Duplicate(HANDLE h, DWORD flags)
{
  Close();
  return DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &m_handle, 0, 0, flags);
}


///////////////////////////////////////////////////////////////////////////////
// PDynaLink

PDynaLink::PDynaLink()
{
  m_hDLL = NULL;
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


PBoolean PDynaLink::Open(const PString & names)
{
  m_lastError.MakeEmpty();

  PStringArray filenames = names.Lines();
  for (PINDEX i = 0; i < filenames.GetSize(); ++i) {
    PVarString filename = filenames[i];
#ifndef _WIN32_WCE
    m_hDLL = LoadLibraryEx(filename, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
    m_hDLL = LoadLibrary(filename);
#endif
    if (m_hDLL != NULL)
      return true;
  }

  m_lastError.sprintf("0x%x", ::GetLastError());
  PTRACE(1, "DLL\tError loading DLL (" << setfill(',') << filenames << "), error=" << m_lastError);
  return false;
}


void PDynaLink::Close()
{
  if (m_hDLL != NULL) {
    FreeLibrary(m_hDLL);
    m_hDLL = NULL;
  }
}


PBoolean PDynaLink::IsLoaded() const
{
  return m_hDLL != NULL;
}


PString PDynaLink::GetName(PBoolean full) const
{
  PFilePathString str;
  if (m_hDLL != NULL) {
    TCHAR path[_MAX_PATH];
    GetModuleFileName(m_hDLL, path, _MAX_PATH-1);
    str = PString(path);
    if (!full) {
      str.Delete(0, str.FindLast('\\')+1);
      PINDEX pos = str.Find(".DLL");
      if (pos != P_MAX_INDEX)
        str.Delete(pos, P_MAX_INDEX);
    }
  }
  return str;
}


PBoolean PDynaLink::GetFunction(PINDEX index, Function & func, bool compulsory)
{
  m_lastError.MakeEmpty();
  func = NULL;

  if (m_hDLL == NULL)
    return false;

  LPCSTR ordinal = (LPCSTR)(DWORD_PTR)LOWORD(index);
  func = (Function)GetProcAddress(m_hDLL, ordinal);
  if (func != NULL)
    return true;

  if (compulsory)
    Close();

  m_lastError.sprintf("0x%x", ::GetLastError());
  return false;
}


PBoolean PDynaLink::GetFunction(const PString & name, Function & func, bool compulsory)
{
  m_lastError.MakeEmpty();
  func = NULL;

  if (m_hDLL == NULL)
    return false;

  PVarString funcname = name;
  func = (Function)GetProcAddress(m_hDLL, funcname);
  if (func != NULL)
    return true;

  if (compulsory)
    Close();

  m_lastError.sprintf("0x%x", ::GetLastError());
  return false;
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
  size_t bufSize = pptr() - pbase();

  if (c != EOF) {
    *pptr() = (char)c;
    bufSize++;
  }

  if (bufSize != 0) {
    char * p = pbase();
    setp(p, epptr());
    p[bufSize] = '\0';

#ifdef UNICODE
    // Note we do NOT use PWideString here as it could cause infinitely
    // recursive calls if there is an error!
    PINDEX length = strlen(p);
    wchar_t * unicode = new wchar_t[length+1];
    unicode[length] = 0;
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, p, length, unicode, length+1);
    OutputDebugString(unicode);
    delete [] unicode;
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
