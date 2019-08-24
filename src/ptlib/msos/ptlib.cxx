/*
 * ptlib.cxx
 *
 * General implementation of classes for Microsoft operating systems.
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
 */

#include <ptlib.h>

#include <ptlib/pprocess.h>

#if !P_USE_INLINES
#include <ptlib/osutil.inl>
#include <ptlib/msos/ptlib/ptlib.inl>
#endif

#include <share.h>
#ifdef _WIN32
#include <WERAPI.H>
#pragma comment(lib,"wer")
#endif


#define PTraceModule() "PTLib"


ostream & operator<<(ostream & s, PInt64 v)
{
  char buffer[25];

  if ((s.flags()&ios::hex) != 0)
    return s << _ui64toa(v, buffer, 16);

  if ((s.flags()&ios::oct) != 0)
    return s << _ui64toa(v, buffer, 8);

  if (v < 0) {
    s << '-';
    v = -v;
  }

  return s << _i64toa(v, buffer, 10);
}


ostream & operator<<(ostream & s, PUInt64 v)
{
  char buffer[25];
  return s << _ui64toa(v, buffer, (s.flags()&ios::oct) ? 8 : ((s.flags()&ios::hex) ? 16 : 10));
}


const PINDEX MaxDigits = (64+2)/3+1; // Maximum is 22 digit octal number, plus sign

static void GetDigits(PBoolean sign, istream & s, char * buffer)
{
  PINDEX count = 0;

  while (isspace(s.peek()))
    s.get();

  if (s.peek() == '+')
    s.get(); // Skip leading '+'
  else if (sign && s.peek() == '-')
    s.get(buffer[count++]);

  if ((s.flags()&ios::oct) != 0) {
    while (isdigit(s.peek()) && s.peek() < '8' && count < MaxDigits)
      s.get(buffer[count++]);
  }
  else if ((s.flags()&ios::hex) != 0) {
    while (isxdigit(s.peek()) && count < MaxDigits)
      s.get(buffer[count++]);
  }
  else {
    while (isdigit(s.peek()) && count < MaxDigits)
      s.get(buffer[count++]);
  }

  buffer[count] = '\0';

  if (count > (buffer[0] == '-' ? 1 : 0))
    return;

  s.clear(ios::failbit);
}


istream & operator>>(istream & s, PInt64 & v)
{
  char b[MaxDigits+1];
  GetDigits(true, s, b);
  v = _atoi64(b);
  return s;
}


istream & operator>>(istream & s, PUInt64 & v)
{
  char b[MaxDigits+1];
  GetDigits(false, s, b);
  v = _atoi64(b);
  return s;
}


///////////////////////////////////////////////////////////////////////////////
// PTime

struct tm * PTime::os_localtime(const time_t * clock, struct tm * tb)
{
  if (::localtime_s(tb, clock) == 0)
    return tb;

  memset(tb, 0, sizeof(*tb));
  return tb;
}


struct tm * PTime::os_gmtime(const time_t * clock, struct tm * tb)
{
  if (::gmtime_s(tb, clock) == 0)
    return tb;

  memset(tb, 0, sizeof(*tb));
  return tb;
}


///////////////////////////////////////////////////////////////////////////////
// PChannel

void PChannel::Construct()
{
}


PBoolean PChannel::Read(void *, PINDEX)
{
  PAssertAlways(PUnimplementedFunction);
  return false;
}


PBoolean PChannel::Write(const void *, PINDEX)
{
  PAssertAlways(PUnimplementedFunction);
  return false;
}


PBoolean PChannel::Close()
{
  return false;
}

FILE * PChannel::FDOpen(const char * /*mode*/)
{
  return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// Directories

PDirectory PDirectory::GetParent() const
{
  if (IsRoot())
    return *this;
  
  return *this + "..";
}


bool PDirectory::Change(const PString & p)
{
  PDirectory d = p;

  if (d[0] != '\\')
    if (_chdrive(toupper(d[0])-'A'+1) != 0)
      return false;

  return _chdir(d + ".") == 0;
}


bool PDirectory::InternalEntryCheck()
{
#ifdef UNICODE
  PString name(fileinfo.cFileName);
#else
  char * name = fileinfo.cFileName;
#endif // UNICODE
  if (strcmp(name, ".") == 0)
    return m_scanMask & PFileInfo::CurrentDirectory;
  if (strcmp(name, "..") == 0)
    return m_scanMask & PFileInfo::ParentDirectory;
  if (!PFilePath::IsValid(name))
    return false;

  PFileInfo info;
  if (!PFile::GetInfo(*this + name, info))
    return false; // Probably means file disappeared between dir open and here

  return info.type & m_scanMask;
}


bool PDirectory::IsRoot() const
{
  if ((*this)[1] == ':')
    return GetLength() == 3;

  PINDEX pos = FindOneOf("/\\", 2);
  return pos == GetLength();
}


PDirectory PDirectory::GetRoot() const
{
  if ((*this)[1] == ':')
    return Left(3);

  return Left(FindOneOf("/\\", 2));
}


PStringArray PDirectory::GetPath() const
{
  PStringArray path;

  if (IsEmpty())
    return path;

  if ((*this)[1] == ':')
    path = Tokenise("/\\", false);
  else {
    path = Mid(2).Tokenise("/\\", false);
    path[0].Splice("\\\\", 0);
  }

  PINDEX last = path.GetSize()-1;
  while (path[last].IsEmpty())
    path.SetSize(last--);

  return path;
}


bool PDirectory::GetInfo(PFileInfo & info) const
{
  return PFile::GetInfo(*this + GetEntryName(), info);
}


bool PDirectory::Exists(const PString & path)
{
  if (_access(path, 0) != 0)
    return false;

  // Could be a file, so actually need to try and open it
  PDirectory dir(path);
  return dir.Open(PFileInfo::AllFiles);
}


///////////////////////////////////////////////////////////////////////////////
// File Path

static PINDEX GetVolumeSubStringLength(const PString & path)
{
  if (path[1] == ':')
    return 2;

  if (path[0] == '\\' && path[1] == '\\') {
    PINDEX backslash = path.Find('\\', 2);
    if (backslash != P_MAX_INDEX) {
      backslash = path.Find('\\', backslash+1);
      if (backslash != P_MAX_INDEX)
        return backslash;
    }
  }

  PINDEX backslash = path.Find('\\');
  if (backslash != P_MAX_INDEX)
    return backslash;

  return 0;
}


PCaselessString PFilePath::GetVolume() const
{
  return Left(GetVolumeSubStringLength(*this));
}


PCaselessString PFilePath::GetPath() const
{
  return operator()(GetVolumeSubStringLength(*this), FindLast('\\', GetLength()-2));
}


///////////////////////////////////////////////////////////////////////////////
// PFile

bool PFile::Access(const PFilePath & name, OpenMode mode)
{
  int accmode;

  switch (mode) {
    case ReadOnly :
#ifndef R_OK
#define R_OK 4
#endif
      accmode = R_OK;
      break;

    case WriteOnly :
#ifndef W_OK
#define W_OK 2
#endif
      accmode = W_OK;
      break;

    default :
      accmode = R_OK|W_OK;
  }

  return _access(name, accmode) == 0;
}


PBoolean PFile::Remove(const PString & name, PBoolean force)
{
  if (remove(name) == 0)
    return true;
  if (!force || errno != EACCES)
    return false;
  if (_chmod(name, _S_IWRITE) != 0)
    return false;
  return remove(name) == 0;
}


#if defined(_WIN32)

static void TwiddleBits(PFileInfo::Permissions newPermissions,
                        PFileInfo::Permissions & oldPermissions,
                        PFileInfo::Permissions permission,
                        DWORD & mask,
                        DWORD access)
{
  if (newPermissions == PFileInfo::NoPermissions) {
    if ((mask&access) == access)
      oldPermissions |= permission;
  }
  else {
    if (newPermissions & permission)
      mask |= access;
    else
      mask &= ~access;
  }
}


static PFileInfo::Permissions FileSecurityPermissions(const PFilePath & filename, PFileInfo::Permissions newPermissions)
{
  // All of the following is to support cygwin style permissions

  PBYTEArray storage(sizeof(SECURITY_DESCRIPTOR));
  SECURITY_DESCRIPTOR * descriptor = (SECURITY_DESCRIPTOR *)storage.GetPointer();
  DWORD lengthNeeded = 0;

  if (!GetFileSecurity(filename,
                       DACL_SECURITY_INFORMATION,
                       descriptor,
                       storage.GetSize(),
                       &lengthNeeded)) {
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || lengthNeeded == 0)
      return PFileInfo::NoPermissions;

    descriptor = (SECURITY_DESCRIPTOR *)storage.GetPointer(lengthNeeded);
    if (!GetFileSecurity(filename,
                         DACL_SECURITY_INFORMATION,
                         descriptor,
                         storage.GetSize(),
                         &lengthNeeded))
                         return PFileInfo::NoPermissions;
  }

  BOOL daclPresent, daclDefaulted;
  PACL dacl;
  if (!GetSecurityDescriptorDacl(descriptor, &daclPresent, &dacl, &daclDefaulted))
    return PFileInfo::NoPermissions;

  if (!daclPresent || daclDefaulted || dacl == NULL)
    return PFileInfo::NoPermissions;


  ACL_SIZE_INFORMATION aclSize;
  if (!GetAclInformation(dacl, &aclSize, sizeof(aclSize), AclSizeInformation))
    return PFileInfo::NoPermissions;

  PFileInfo::Permissions oldPermissions;
  int cygwinMask = 0;

  for (DWORD aceIndex = 0; aceIndex< aclSize.AceCount; aceIndex++) {
    LPVOID acePtr;
    GetAce(dacl, aceIndex, &acePtr);
    ACE_HEADER * aceHdr = (ACE_HEADER *)acePtr;
    if (aceHdr->AceType == ACCESS_ALLOWED_ACE_TYPE) {
      ACCESS_ALLOWED_ACE * ace = (ACCESS_ALLOWED_ACE *)acePtr;
      PString account, domain;
      DWORD accountLen = 1000;
      DWORD domainLen = 1000;
      SID_NAME_USE usage;
      if (LookupAccountSid(NULL, &ace->SidStart,
                           account.GetPointerAndSetLength(accountLen-1), &accountLen,
                           domain.GetPointerAndSetLength(domainLen-1), &domainLen,
                           &usage)) {
        account.MakeMinimumSize(accountLen);
        domain.MakeMinimumSize(domainLen);
        if (account *= "None") {
          cygwinMask |= 2;
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::WorldRead,
                      ace->Mask, FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA);
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::WorldWrite,
                      ace->Mask, FILE_WRITE_DATA|FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA);
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::WorldExecute,
                      ace->Mask, FILE_EXECUTE);
        }
        else if (account *= "EVERYONE") {
          cygwinMask |= 1;
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::GroupRead,
                      ace->Mask, FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA);
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::GroupWrite,
                      ace->Mask, FILE_WRITE_DATA|FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA);
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::GroupExecute,
                      ace->Mask, FILE_EXECUTE);
        }
        else if (account == PProcess::Current().GetUserName()) {
          cygwinMask |= 4;
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::UserRead,
                      ace->Mask, FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_READ_EA);
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::UserWrite,
                      ace->Mask, FILE_WRITE_DATA|FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_EA);
          TwiddleBits(newPermissions, oldPermissions, PFileInfo::UserExecute,
                      ace->Mask, FILE_EXECUTE);
        }
      }
    }
  }

  // Only do it if have the three ACE entries as per cygwin
  if (cygwinMask != 7)
    return PFileInfo::NoPermissions;

  if (newPermissions != -1)
    SetFileSecurity(filename, DACL_SECURITY_INFORMATION, descriptor);

  return oldPermissions;
}

#endif


bool PFile::GetInfo(const PFilePath & name, PFileInfo & info)
{
  if (name.IsEmpty())
    return false;

  PString fn = name;
  PINDEX pos = fn.GetLength()-1;
  while (PDirectory::IsSeparator(fn[pos]))
    pos--;
  fn.Delete(pos+1, P_MAX_INDEX);

  struct stat s;
  if (stat(fn, &s) != 0)
    return false;

  info.created =  (s.st_ctime < 0) ? 0 : s.st_ctime;
  info.modified = (s.st_mtime < 0) ? 0 : s.st_mtime;
  info.accessed = (s.st_atime < 0) ? 0 : s.st_atime;
  info.size = s.st_size;

#if defined(_WIN32)
  info.permissions = FileSecurityPermissions(name, PFileInfo::NoPermissions);
  if (info.permissions == PFileInfo::NoPermissions)
#endif
  {
    if ((s.st_mode&S_IREAD) != 0)
      info.permissions |= PFileInfo::UserRead|PFileInfo::GroupRead|PFileInfo::WorldRead;
    if ((s.st_mode&S_IWRITE) != 0)
      info.permissions |= PFileInfo::UserWrite|PFileInfo::GroupWrite|PFileInfo::WorldWrite;
    if ((s.st_mode&S_IEXEC) != 0)
      info.permissions |= PFileInfo::UserExecute|PFileInfo::GroupExecute|PFileInfo::WorldExecute;
  }

  switch (s.st_mode & S_IFMT) {
    case S_IFREG :
      info.type = PFileInfo::RegularFile;
      break;

    case S_IFDIR :
      info.type = PFileInfo::SubDirectory;
      break;

    default:
      info.type = PFileInfo::UnknownFileType;
      break;
  }

#if defined(_WIN32)
  info.hidden = (GetFileAttributes(name) & FILE_ATTRIBUTE_HIDDEN) != 0;
#else
  unsigned int attr;
  _dos_getfileattr(name, &attr);
  info.hidden = (attr & _A_HIDDEN) != 0;
#endif

  return true;
}


bool PFile::SetPermissions(const PFilePath & name, PFileInfo::Permissions permissions)
{
#if defined(_WIN32)
  FileSecurityPermissions(name, permissions);
#endif

  return _chmod(name, permissions&(_S_IWRITE|_S_IREAD)) == 0;
}


PBoolean PFile::IsTextFile() const
{
  return false;
}


bool PFile::InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions)
{
  Close();
  clear();

  if (m_path.IsEmpty())
    m_path = PFilePath("PTL", NULL);

  int oflags = IsTextFile() ? _O_TEXT : _O_BINARY;
  switch (mode) {
    case ReadOnly :
      oflags |= O_RDONLY;
      if (opts == ModeDefault)
        opts = MustExist;
      break;

    case WriteOnly :
      oflags |= O_WRONLY;
      if (opts == ModeDefault)
        opts = Create|Truncate;
      break;

    case ReadWrite :
      oflags |= O_RDWR;
      if (opts == ModeDefault)
        opts = Create;
      break;

    default :
      PAssertAlways(PInvalidParameter);
  }

  if (opts & Create)
    oflags |= O_CREAT;
  if (opts & Exclusive)
    oflags |= O_EXCL;
  if (opts & Truncate)
    oflags |= O_TRUNC;

  if (opts != ModeDefault)
    m_removeOnClose = opts & Temporary;

  int sflags = _SH_DENYNO;
  if (opts & DenySharedRead)
    sflags = _SH_DENYRD;
  else if (opts & DenySharedWrite)
    sflags = _SH_DENYWR;
  else if (opts & (DenySharedRead|DenySharedWrite))
    sflags = _SH_DENYWR;

  os_handle = _sopen(m_path, oflags, sflags, permissions.AsBits());

  // As ConvertOSError tests for < 0 and some return values _sopen may be
  // negative, only pass -1 through.
  return ConvertOSError(os_handle == -1 ? -1 : 0);
}


PBoolean PFile::SetLength(off_t len)
{
  return ConvertOSError(_chsize(GetOSHandleAsInt(), len));
}

FILE * PFile::FDOpen(const char * mode)
{
  FILE * h = _fdopen((int)os_handle, mode);
  if (h != NULL)
    os_handle = -1;
  return h;
}


///////////////////////////////////////////////////////////////////////////////
// PTextFile

PBoolean PTextFile::IsTextFile() const
{
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// PConsoleChannel

PConsoleChannel::PConsoleChannel()
  : m_lastMouseState(0)
{
}


PConsoleChannel::PConsoleChannel(ConsoleType type)
  : m_lastMouseState(0)
{
  Open(type);
}


PBoolean PConsoleChannel::Open(ConsoleType type)
{
  if (!PAssert(type >= StandardInput && type <= StandardError, PInvalidParameter))
    return false;

  static DWORD HandleNames[] = { STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE };
  if (!m_hConsole.Duplicate(GetStdHandle(HandleNames[type])))
    return ConvertOSError(-2);

  if (type == StandardInput)
    SetConsoleCP(CP_UTF8);
  else
    SetConsoleOutputCP(CP_UTF8);

  os_handle = type;
  return true;
}


PString PConsoleChannel::GetName() const
{
  return "\\\\.\\Console";
}


int PConsoleChannel::ReadChar()
{
  if (CheckNotOpen())
    return -1;

  {
    DWORD mode;
    if (!GetConsoleMode(m_hConsole, &mode))
      return -1;
    if ((mode & ENABLE_LINE_INPUT) != 0)
      return PChannel::ReadChar();
  }

  for (;;) {
    INPUT_RECORD input;
    DWORD numRead;
    if (!ReadConsoleInput(m_hConsole, &input, 1, &numRead))
      return ConvertOSError(-2, LastReadError);

    if (CheckNotOpen())
      return -1;

    if (numRead == 0)
      continue;

    switch (input.EventType) {
      case KEY_EVENT :
        if (input.Event.KeyEvent.bKeyDown)
          continue;

        switch (input.Event.KeyEvent.wVirtualKeyCode) {
          case VK_LEFT :
            return KeyLeft;
          case VK_RIGHT :
            return KeyRight;
          case VK_UP :
            return KeyUp;
          case VK_DOWN :
            return KeyDown;
          case VK_PRIOR :
            return KeyPageUp;
          case VK_NEXT :
            return KeyPageDown;
          case VK_HOME :
            return KeyHome;
          case VK_END :
            return KeyEnd;
          case VK_BACK :
            return KeyBackSpace;
          case VK_DELETE :
            return KeyDelete;
          case VK_INSERT :
            return KeyInsert;
        }

        if (input.Event.KeyEvent.wVirtualKeyCode >= VK_F1 && input.Event.KeyEvent.wVirtualKeyCode <= VK_F24)
          return input.Event.KeyEvent.wVirtualKeyCode - KeyF1;

        if (input.Event.KeyEvent.uChar.UnicodeChar != 0)
          return (input.Event.KeyEvent.uChar.UnicodeChar&0xffff);

        break;

      case MOUSE_EVENT :
        if (input.Event.MouseEvent.dwButtonState != m_lastMouseState) {
          int code = MouseEvent;

          DWORD buttons = m_lastMouseState|input.Event.MouseEvent.dwButtonState;
          static const DWORD Mask[] = { FROM_LEFT_1ST_BUTTON_PRESSED, RIGHTMOST_BUTTON_PRESSED, FROM_LEFT_2ND_BUTTON_PRESSED, FROM_LEFT_3RD_BUTTON_PRESSED };
          for (PINDEX btn = 0; btn < PARRAYSIZE(Mask); ++btn) {
            if (buttons&Mask[btn]) {
              code |= MouseButton1 << btn;
              if (input.Event.MouseEvent.dwButtonState&&Mask[btn])
                code |= (btn+1) << MouseClickShift;
            }
          }

          if (input.Event.MouseEvent.dwEventFlags&DOUBLE_CLICK)
            code |= MouseDoubleClick;

          code |= input.Event.MouseEvent.dwMousePosition.X << MouseColShift;
          code |= input.Event.MouseEvent.dwMousePosition.Y << MouseRowShift;

          m_lastMouseState = input.Event.MouseEvent.dwButtonState;
          return code;
        }
        break;
    }
  }
}


PBoolean PConsoleChannel::Read(void * buffer, PINDEX length)
{
  if (CheckNotOpen())
    return false;

  DWORD readBytes;
  if (ReadFile(m_hConsole, buffer, length, &readBytes, NULL))
    return SetLastReadCount(readBytes) > 0;

  return ConvertOSError(-2, LastReadError);
}


PBoolean PConsoleChannel::Write(const void * buffer, PINDEX length)
{
  if (CheckNotOpen())
    return false;

  flush();

  DWORD written;
  if (WriteFile(m_hConsole, buffer, length, &written, NULL))
    return SetLastWriteCount(written) >= length;

  return ConvertOSError(-2, LastWriteError);
}


PBoolean PConsoleChannel::Close()
{
  if (!IsOpen())
    return false;

  PTRACE(4, "PTLib", "Closing console channel.");
  os_handle = -1;

  INPUT_RECORD input;
  memset(&input, 0, sizeof(input));
  DWORD written = 0;
  if (!WriteConsoleInput(m_hConsole, &input, 1, &written) || written != 1) {
    PTRACE(2, "WriteConsoleInput failed: error=" << ::GetLastError());
  }

  m_hConsole.Close();
  return true;
}


bool PConsoleChannel::SetLocalEcho(bool localEcho)
{
  return InternalSetConsoleMode(ENABLE_ECHO_INPUT, localEcho);
}


bool PConsoleChannel::SetLineBuffered(bool lineBuffered)
{
  return InternalSetConsoleMode(ENABLE_LINE_INPUT, lineBuffered);
}


bool PConsoleChannel::GetTerminalSize(unsigned & rows, unsigned & columns)
{
  if (CheckNotOpen())
    return false;

  CONSOLE_SCREEN_BUFFER_INFO info;
  if (!GetConsoleScreenBufferInfo(m_hConsole, &info))
    return false;

  if (info.srWindow.Right <= info.srWindow.Left || info.srWindow.Bottom <= info.srWindow.Top)
    return false;

  rows = info.srWindow.Bottom - info.srWindow.Top + 1;
  columns = info.srWindow.Right - info.srWindow.Left + 1;
  return true;
}


bool PConsoleChannel::InternalSetConsoleMode(DWORD bit, bool on)
{
  if (CheckNotOpen())
    return false;

  if (os_handle != StandardInput)
    return SetErrorValues(Miscellaneous, EINVAL);

  DWORD mode;
  if (!GetConsoleMode(m_hConsole, &mode))
    return false;

  if (on)
    mode |= bit;
  else
    mode &= ~bit;

  return SetConsoleMode(m_hConsole, mode);
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

#ifdef _WIN32

LONG WINAPI MyExceptionHandler(_EXCEPTION_POINTERS * info)
{
  ostringstream str;
  str << "Unhandled exception: code=" << info->ExceptionRecord->ExceptionCode
      << ", when=" << PTime().AsString(PTime::LoggingFormat PTRACE_PARAM(, PTrace::GetTimeZone())) << ends;
  
  PAssertAlways(str.str().c_str());
  ExitProcess(1);
}

#else // _WIN32

PBoolean PProcess::IsGUIProcess() const
{
  return false;
}

#endif // _WIN32

static void StaticSignalHandler(int signal)
{
  PProcess::Current().AsynchronousRunTimeSignal(signal, 0);
}


PRunTimeSignalHandler PProcess::PlatformSetRunTimeSignalHandler(int signum)
{
  return ::signal(signum, StaticSignalHandler);
}


void PProcess::PlatformResetRunTimeSignalHandler(int signum, PRunTimeSignalHandler previous)
{
  ::signal(signum, previous);
}


#define SIG_NAME(s) { s, #s }
POrdinalToString::Initialiser const PProcess::InternalSigNames[] = {
  SIG_NAME(SIGABRT),
  SIG_NAME(SIGBREAK),
  SIG_NAME(SIGFPE),
  SIG_NAME(SIGILL),
  SIG_NAME(SIGINT),
  SIG_NAME(SIGSEGV),
  SIG_NAME(SIGTERM),
  { }
};


void PProcess::PlatformConstruct()
{
  m_waitOnExitConsoleWindow = true;
  PSetErrorStream(&cerr);

#if !defined(_WIN32) && defined(_MSC_VER) && defined(_WINDOWS)
  _wsetscreenbuf(1, _WINBUFINF);
  _wsizeinfo ws;
  ws._version = _QWINVER;
  ws._type = _WINSIZEMAX;
  _wsetsize(1, &ws);
#else
  extern LONG WINAPI PExceptionHandler(PEXCEPTION_POINTERS info);
  SetUnhandledExceptionFilter(PExceptionHandler);
  PTRACE_PARAM(HRESULT result =) WerAddExcludedApplication(m_executableFile.AsUCS2(), false);
  PTRACE_IF(1, result != S_OK, "PTLib", "Error excluding application from WER crash dialogs: err=" << result);
#endif
}


void PProcess::PlatformDestruct()
{
  WaitOnExitConsoleWindow();
}


PBoolean PProcess::SetMaxHandles(int /*newLimit*/)
{
  // Not applicable
  return true;
}


//////////////////////////////////////////////////////////////////////////////

#define INCLUDE_STUFF1(cls) \
  cls i##cls; \
  i##cls = i##cls

#define INCLUDE_STUFF2(cls) \
  INCLUDE_STUFF1(cls); \
  i##cls.GetPointer(); \
  i##cls.Attach(0, 0); \
  i##cls.SetAt(0, 0); \
  i##cls.GetAt(0); \
  i##cls[0]

void PDummyFunctionToMakeSureSymbolsAreInDEFFile()
{
  INCLUDE_STUFF2(PCharArray);
  INCLUDE_STUFF2(PShortArray);
  INCLUDE_STUFF2(PIntArray);
  INCLUDE_STUFF2(PLongArray);
  INCLUDE_STUFF2(PBYTEArray);
  INCLUDE_STUFF2(PWORDArray);
  INCLUDE_STUFF2(PUnsignedArray);
  INCLUDE_STUFF2(PDWORDArray);
  INCLUDE_STUFF1(PStringSet);
  INCLUDE_STUFF1(POrdinalToString);
  INCLUDE_STUFF1(PStringToOrdinal);
  INCLUDE_STUFF1(PStringToString);
}


// End Of File ///////////////////////////////////////////////////////////////
