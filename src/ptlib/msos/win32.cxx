/*
 * $Id: win32.cxx,v 1.3 1995/04/25 11:33:54 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: win32.cxx,v $
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

#include <winuser.h>
#include <winnls.h>

#include <fcntl.h>
#include <errno.h>
#include <sys\stat.h>


///////////////////////////////////////////////////////////////////////////////
// PTime

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


PString PTime::GetDayName(Weekdays dayOfWeek, BOOL abbreviated)
{
  PString str;
  // Of course Sunday is 6 and Monday is 1...
  GetLocaleInfo(GetUserDefaultLCID(), (dayOfWeek+6)%7 +
                (abbreviated ? LOCALE_SABBREVDAYNAME1 : LOCALE_SDAYNAME1),
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


PString PTime::GetMonthName(Months month, BOOL abbreviated)
{
  PString str;
  GetLocaleInfo(GetUserDefaultLCID(), month-1 +
                (abbreviated ? LOCALE_SABBREVMONTHNAME1 : LOCALE_SMONTHNAME1),
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


///////////////////////////////////////////////////////////////////////////////
// Directories

void PDirectory::Construct()
{
  hFindFile = INVALID_HANDLE_VALUE;
  fileinfo.cFileName[0] = '\0';
  PString::operator=(CreateFullPath(*this, TRUE) + '\\');
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


PString PDirectory::CreateFullPath(const PString & path, BOOL)
{
  LPSTR dummy;
  PString fullpath;
  GetFullPathName(path, _MAX_PATH, fullpath.GetPointer(_MAX_PATH), &dummy);
  return fullpath;
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


void PPipeChannel::DestroyContents()
{
  Close();
  PChannel::DestroyContents();
}


void PPipeChannel::CloneContents(const PPipeChannel *)
{
  PAssertAlways("Cannot clone pipe");
}


void PPipeChannel::CopyContents(const PPipeChannel & chan)
{
  info = chan.info;
  hToChild = chan.hToChild;
  hFromChild = chan.hFromChild;
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
  BuildCommDCB(str, &deviceControlBlock);
}


void PSerialChannel::CopyContents(const PSerialChannel & chan)
{
  commsResource = chan.commsResource;
}


PString PSerialChannel::GetName() const
{
  return portName;
}


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
  cto.ReadIntervalTimeout = cto.ReadTotalTimeoutMultiplier = 0;
  if (readTimeout == PMaxTimeInterval)
    cto.ReadTotalTimeoutConstant = 0;
  else {
    cto.ReadTotalTimeoutConstant = readTimeout.GetMilliseconds();
    if (cto.ReadTotalTimeoutConstant == 0)
      cto.ReadIntervalTimeout = MAXDWORD; // Immediate timeout
  }
  PAssertOS(SetCommTimeouts(commsResource, &cto));

  OVERLAPPED overlap;
  memset(&overlap, 0, sizeof(overlap));
  overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (ReadFile(commsResource, buf, len, (LPDWORD)&lastReadCount, &overlap)) {
    CloseHandle(overlap.hEvent);
    return lastReadCount > 0;
  }

  if (GetLastError() == ERROR_IO_PENDING)
    if (GetOverlappedResult(commsResource,
                                   &overlap,
                                    (LPDWORD)&lastReadCount, TRUE)) {
      CloseHandle(overlap.hEvent);
      return lastReadCount > 0;
    }

  ConvertOSError(-2);
  CloseHandle(overlap.hEvent);

  return FALSE;
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
  overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
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
  if (IsOpen()) {
    deviceControlBlock.DCBlength = sizeof(deviceControlBlock);
    PAssertOS(GetCommState(commsResource, &deviceControlBlock));
  }

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
  if (portName.Find('\\') == P_MAX_INDEX)
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

  SetupComm(commsResource, 2048, 2048);

  if (SetCommsParam(speed, data, parity, stop, inputFlow, outputFlow)) {
    os_handle = 0;
    return TRUE;
  }

  ConvertOSError(-2);
  CloseHandle(commsResource);
  commsResource = INVALID_HANDLE_VALUE;
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
  switch (src) {
    case System :
      Construct("WIN.INI");
      break;

    case Application :
      PFilePath appFile = PProcess::Current()->GetFile();
      Construct(appFile.GetVolume() +
                              appFile.GetPath() + appFile.GetTitle() + ".INI");
      break;
  }
}


void PConfig::Construct(const PFilePath & filename)
{
  configFile = filename;
}


PStringList PConfig::GetSections()
{
  PStringList sections;

  if (!configFile.IsEmpty()) {
    PString buf;
    char * ptr = buf.GetPointer(10000);
    GetPrivateProfileString(NULL, NULL, "", ptr, 9999, configFile);
    while (*ptr != '\0') {
      sections.AppendString(ptr);
      ptr += strlen(ptr)+1;
    }
  }

  return sections;
}


PStringList PConfig::GetKeys(const PString & section) const
{
  PStringList keys;

  if (configFile.IsEmpty()) {
    char ** ptr = _environ;
    while (*ptr != NULL) {
      PString buf = *ptr++;
      keys.AppendString(buf.Left(buf.Find('=')));
    }
  }
  else {
    PString buf;
    char * ptr = buf.GetPointer(10000);
    GetPrivateProfileString(section, NULL, "", ptr, 9999, configFile);
    while (*ptr != '\0') {
      keys.AppendString(ptr);
      ptr += strlen(ptr)+1;
    }
  }

  return keys;
}


void PConfig::DeleteSection(const PString & section)
{
  if (configFile.IsEmpty())
    return;

  PAssert(!section.IsEmpty(), PInvalidParameter);
  PAssertOS(WritePrivateProfileString(section, NULL, NULL, configFile));
}


void PConfig::DeleteKey(const PString & section, const PString & key)
{
  PAssert(!key.IsEmpty(), PInvalidParameter);

  if (configFile.IsEmpty()) {
    PString str = key;
    PAssert(str.Find('=') == P_MAX_INDEX, PInvalidParameter);
    putenv(str + "=");
  }
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    PAssertOS(WritePrivateProfileString(section, key, NULL, configFile));
  }
}


PString PConfig::GetString(const PString & section,
                                     const PString & key, const PString & dflt)
{
  PString str;

  PAssert(!key.IsEmpty(), PInvalidParameter);

  if (configFile.IsEmpty()) {
    PAssert(key.Find('=') == P_MAX_INDEX, PInvalidParameter);
    char * env = getenv(key);
    if (env != NULL)
      str = env;
    else
      str = dflt;
  }
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    GetPrivateProfileString(section, key, dflt,
                                        str.GetPointer(1000), 999, configFile);
    str.MakeMinimumSize();
  }

  return str;
}


void PConfig::SetString(const PString & section,
                                    const PString & key, const PString & value)
{
  PAssert(!key.IsEmpty(), PInvalidParameter);

  if (configFile.IsEmpty()) {
    PString str = key;
    PAssert(str.Find('=') == P_MAX_INDEX, PInvalidParameter);
    putenv(str + "=" + value);
  }
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    PAssertOS(WritePrivateProfileString(section, key, value, configFile));
  }
}


///////////////////////////////////////////////////////////////////////////////
// Threads

PThread::ThreadDict PThread::threads;

DWORD EXPORTED PThread::MainFunction(LPVOID threadPtr)
{
  PThread * thread = (PThread *)PAssertNULL(threadPtr);

  AttachThreadInput(thread->id, ((PThread*)PProcess::Current())->id, TRUE);
  PThread::threads.SetAt(thread->id, thread);

  thread->Main();

  PThread::threads.SetAt(thread->id, NULL);
  thread->handle = NULL;
  return 0;
}


PThread::PThread(PINDEX stackSize, BOOL startSuspended, Priority priorityLevel)
{
  PAssert(stackSize > 0, PInvalidParameter);
  originalStackSize = stackSize;
  handle = CreateThread(NULL, stackSize, PThread::MainFunction,
                    (LPVOID)this, startSuspended ? CREATE_SUSPENDED : 0, &id);
  PAssertOS(handle != NULL);
  SetPriority(priorityLevel);
}


PThread::~PThread()
{
  Terminate();
}


void PThread::Restart()
{
  if (handle == NULL) {
    handle = CreateThread(NULL,
               originalStackSize, PThread::MainFunction, (LPVOID)this, 0, &id);
    PAssertOS(handle != NULL);
  }
}


void PThread::Terminate()
{
  if (handle != NULL && originalStackSize > 0) {
    if (Current() == this)
      ExitThread(0);
    else
      TerminateThread(handle, 1);
    threads.SetAt(id, NULL);
    handle = NULL;
  }
}


BOOL PThread::IsTerminated() const
{
  return handle == NULL;
}


void PThread::Suspend(BOOL susp)
{
  if (susp)
    SuspendThread(PAssertNULL(handle));
  else
    Resume();
}


void PThread::Resume()
{
  ResumeThread(PAssertNULL(handle));
}


BOOL PThread::IsSuspended() const
{
  SuspendThread(PAssertNULL(handle));
  return ResumeThread(handle) <= 1;
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
  SetThreadPriority(handle, priorities[priorityLevel]);
}


PThread::Priority PThread::GetPriority() const
{
  switch (GetThreadPriority(handle)) {
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
  handle = GetCurrentThread();
  id = GetCurrentThreadId();
  threads.DisallowDeleteObjects();
  threads.SetAt(id, this);
  originalStackSize = 0;
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

PString PProcess::GetUserName() const
{
  PString username;
  unsigned long size = 50;
  ::GetUserName(username.GetPointer((PINDEX)size), &size);
  username.MakeMinimumSize();
  return username;
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
