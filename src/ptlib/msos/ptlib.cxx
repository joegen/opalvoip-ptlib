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
 *
 * $Id$
 */

#include <ptlib.h>

#include <ptlib/pprocess.h>

#if !P_USE_INLINES
#include <ptlib/osutil.inl>
#include <ptlib/msos/ptlib/ptlib.inl>
#endif

#ifndef _WIN32_WCE
#include <share.h>
#endif


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


const int MaxDigits = (64+2)/3+1; // Maximum is 22 digit octal number, plus sign

static void GetDigits(BOOL sign, istream & s, char * buffer)
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
  GetDigits(TRUE, s, b);
  v = _atoi64(b);
  return s;
}


istream & operator>>(istream & s, PUInt64 & v)
{
  char b[MaxDigits+1];
  GetDigits(FALSE, s, b);
  v = _atoi64(b);
  return s;
}


PInt64 PString::AsInt64(unsigned base) const
{
  if (base == 10)
    return _atoi64(theArray);

  PAssert(base >= 2 && base <= 36, PInvalidParameter);

  PInt64 total = 0;
  const char * ptr = theArray;

  while (isspace(*ptr))
    ptr++;

  BOOL negative = *ptr == '-';
  if (*ptr == '-' || *ptr == '+')
    ptr++;

  for (;;) {
    unsigned c = *ptr++;
    if (c < '0')
      break;

    if (c <= '9')
      c -= '0';
    else
      c = toupper(c) - 'A' + 10;

    if (c >= base)
      break;

    total = base * total + c;
  }

  if (negative)
    return -total;
  else
    return total;
}


PUInt64 PString::AsUnsigned64(unsigned base) const
{
  PAssert(base >= 2 && base <= 36, PInvalidParameter);

  PUInt64 total = 0;
  const char * ptr = theArray;

  while (isspace(*ptr))
    ptr++;

  for (;;) {
    unsigned c = *ptr++;
    if (c < '0')
      break;

    if (c <= '9')
      c -= '0';
    else
      c = toupper(c) - 'A' + 10;

    if (c >= base)
      break;

    total = base * total + c;
  }

  return total;
}


///////////////////////////////////////////////////////////////////////////////
// PTime

struct tm * PTime::os_localtime(const time_t * clock, struct tm * tb)
{
  struct tm * tp = ::localtime(clock);
  if (tp != NULL)
    return tp;

  memset(tb, 0, sizeof(*tb));
  return tb;
}


struct tm * PTime::os_gmtime(const time_t * clock, struct tm * tb)
{
  struct tm * tp = ::gmtime(clock);
  if (tp != NULL)
    return tp;

  memset(tb, 0, sizeof(*tb));
  return tb;
}


///////////////////////////////////////////////////////////////////////////////
// PChannel

void PChannel::Construct()
{
}


PString PChannel::GetName() const
{
  PAssertAlways(PUnimplementedFunction);
  return PString();
}


BOOL PChannel::Read(void *, PINDEX)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


BOOL PChannel::Write(const void *, PINDEX)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


BOOL PChannel::Close()
{
  return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Directories

PDirectory PDirectory::GetParent() const
{
  if (IsRoot())
    return *this;
  
  return *this + "..";
}


BOOL PDirectory::Change(const PString & p)
{
  PDirectory d = p;

  if (d[0] != '\\')
    if (_chdrive(toupper(d[0])-'A'+1) != 0)
      return FALSE;

  return _chdir(d + ".") == 0;
}


BOOL PDirectory::Filtered()
{
#if defined(_WIN32)
#ifdef _WIN32_WCE
  USES_CONVERSION;
  char * name = T2A(fileinfo.cFileName);
#else
  char * name = fileinfo.cFileName;
#endif // _WIN32_WCE
#else
  char * name = fileinfo.name;
#endif
  if (strcmp(name, ".") == 0)
    return TRUE;
  if (strcmp(name, "..") == 0)
    return TRUE;
  if (scanMask == PFileInfo::AllPermissions)
    return FALSE;

  PFileInfo inf;
  PAssert(PFile::GetInfo(*this+name, inf), POperatingSystemError);
  return (inf.type&scanMask) == 0;
}


BOOL PDirectory::IsRoot() const
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
    path = Tokenise("/\\", FALSE);
  else {
    path = Mid(2).Tokenise("/\\", FALSE);
    path[0].Splice("\\\\", 0);
  }

  PINDEX last = path.GetSize()-1;
  while (path[last].IsEmpty())
    path.SetSize(last--);

  return path;
}


BOOL PDirectory::GetInfo(PFileInfo & info) const
{
  return PFile::GetInfo(*this + GetEntryName(), info);
}


///////////////////////////////////////////////////////////////////////////////
// File Path

PFilePath::PFilePath(const PString & str)
  : PCaselessString(PDirectory::CreateFullPath(str, FALSE))
{
}


PFilePath::PFilePath(const char * cstr)
  : PCaselessString(PDirectory::CreateFullPath(cstr, FALSE))
{
}


PFilePath::PFilePath(const char * prefix, const char * dir)
{
  if (dir != NULL) {
    PDirectory tmpdir(dir);
    operator=(tmpdir);
  }
  else {
    PConfig cfg(PConfig::Environment);
    PString path = cfg.GetString("TMPDIR");
    if (path.IsEmpty()) {
      path = cfg.GetString("TMP");
      if (path.IsEmpty())
        path = cfg.GetString("TEMP");
    }
    if (path.IsEmpty() || path[path.GetLength()-1] != '\\')
      path += '\\';
    *this = path;
  }
  if (prefix != NULL)
    *this += prefix;
  else
    *this += "PW";
  *this += "XXXXXX";
  PAssert(_mktemp(GetPointer()) != NULL, "Could not make temporary file");
}


void PFilePath::AssignContents(const PContainer & cont)
{
  PCaselessString::AssignContents(cont);
  PCaselessString::AssignContents(PDirectory::CreateFullPath(*this, FALSE));
}


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


PDirectory PFilePath::GetDirectory() const
{
  PINDEX backslash = FindLast('\\');
  if (backslash != P_MAX_INDEX)
    return Left(backslash+1);

  return PCaselessString();
}


PCaselessString PFilePath::GetPath() const
{
  return operator()(GetVolumeSubStringLength(*this), FindLast('\\', GetLength()-2));
}


PCaselessString PFilePath::GetFileName() const
{
  PINDEX backslash = FindLast('\\', GetLength()-2);
  if (backslash == P_MAX_INDEX)
    backslash = 0;
  else
    backslash++;

  return Mid(backslash);
}


PCaselessString PFilePath::GetTitle() const
{
  PINDEX backslash = FindLast('\\', GetLength()-2);
  if (backslash == P_MAX_INDEX)
    backslash = 0;
  else
    backslash++;

  PINDEX last_dot = FindLast('.');
  if (last_dot < backslash)
    last_dot = P_MAX_INDEX;

  return operator()(backslash, last_dot-1);
}


PCaselessString PFilePath::GetType() const
{
  PINDEX slash = FindLast('\\');
  if (slash == P_MAX_INDEX)
    slash = 0;
  PINDEX dot = FindLast('.');
  if (dot < slash)
    return PCaselessString();
  return operator()(dot, P_MAX_INDEX);
}


void PFilePath::SetType(const PCaselessString & type)
{
  PINDEX dot = Find('.', FindLast('\\'));
  if (dot != P_MAX_INDEX)
    Splice(type, dot, GetLength()-dot);
  else
    *this += type;
}


///////////////////////////////////////////////////////////////////////////////
// PFile

void PFile::SetFilePath(const PString & newName)
{
  if (!IsOpen())
    path = newName;
}


BOOL PFile::Access(const PFilePath & name, OpenMode mode)
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


BOOL PFile::Remove(const PFilePath & name, BOOL force)
{
  if (remove(name) == 0)
    return TRUE;
  if (!force || errno != EACCES)
    return FALSE;
  if (_chmod(name, _S_IWRITE) != 0)
    return FALSE;
  return remove(name) == 0;
}


BOOL PFile::Rename(const PFilePath & oldname, const PString & newname, BOOL force)
{
  if (newname.FindOneOf(":\\/") != P_MAX_INDEX) {
#ifdef _WIN32_WCE
    set_errno(EINVAL);
#else
    errno = EINVAL;
#endif // _WIN32_WCE
    return FALSE;
  }
  PString fullname = oldname.GetDirectory() + newname;
  if (rename(oldname, fullname) == 0)
    return TRUE;
  if (!force || errno == ENOENT || !Exists(fullname))
    return FALSE;
  if (!Remove(fullname, TRUE))
    return FALSE;
  return rename(oldname, fullname) == 0;
}


BOOL PFile::Move(const PFilePath & oldname, const PFilePath & newname, BOOL force)
{
  if (rename(oldname, newname) == 0)
    return TRUE;
  if (errno == ENOENT)
    return FALSE;
  if (force && Exists(newname)) {
    if (!Remove(newname, TRUE))
      return FALSE;
    if (rename(oldname, newname) == 0)
      return TRUE;
  }
  return Copy(oldname, newname, force) && Remove(oldname);
}


#ifdef _WIN32_WCE

BOOL PFile::GetInfo(const PFilePath & name, PFileInfo & info)
{
  USES_CONVERSION;
  
  PString fn = name;
  PINDEX pos = fn.GetLength()-1;
  while (PDirectory::IsSeparator(fn[pos]))
    pos--;
  fn.Delete(pos+1, P_MAX_INDEX);
  
  HANDLE hFile = CreateFile(A2T((const char*)fn),0,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hFile==INVALID_HANDLE_VALUE) 
    return false;
  
  bool res=false;
  BY_HANDLE_FILE_INFORMATION FInfo;
  if (GetFileInformationByHandle(hFile,&FInfo))
  {
    info.created = FileTimeToTime(FInfo.ftCreationTime);
    info.modified = FileTimeToTime(FInfo.ftLastWriteTime);
    info.accessed = FileTimeToTime(FInfo.ftLastAccessTime);
    info.size = (__int64(FInfo.nFileSizeHigh)<<32)+__int64(FInfo.nFileSizeLow);
    
    info.permissions = PFileInfo::UserRead|PFileInfo::GroupRead|PFileInfo::WorldRead;
    
    if ((FInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)==0)
      info.permissions |= PFileInfo::UserWrite|PFileInfo::GroupWrite|PFileInfo::WorldWrite;
    
    if (FInfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
    {
      info.type = PFileInfo::SubDirectory;
      info.permissions |= PFileInfo::UserExecute|PFileInfo::GroupExecute|PFileInfo::WorldExecute;
    }
    else
    {
      info.type = PFileInfo::RegularFile;
    }
    info.hidden = (FInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0;
    res=true;
  }
  
  CloseHandle(hFile);
  return res;
}

#else // !_WIN32_WCE

#if defined(_WIN32)

static void TwiddleBits(int newPermissions,
                        int & oldPermissions,
                        int permission,
                        DWORD & mask,
                        DWORD access)
{
  if (newPermissions < 0) {
    if ((mask&access) == access)
      oldPermissions |= permission;
  }
  else {
    if ((newPermissions&permission) != 0)
      mask |= access;
    else
      mask &= ~access;
  }
}


static int FileSecurityPermissions(const PFilePath & filename, int newPermissions)
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
      return -1;

    descriptor = (SECURITY_DESCRIPTOR *)storage.GetPointer(lengthNeeded);
    if (!GetFileSecurity(filename,
                         DACL_SECURITY_INFORMATION,
                         descriptor,
                         storage.GetSize(),
                         &lengthNeeded))
      return -1;
  }

  BOOL daclPresent, daclDefaulted;
  PACL dacl;
  if (!GetSecurityDescriptorDacl(descriptor, &daclPresent, &dacl, &daclDefaulted))
    return -1;

  if (!daclPresent || daclDefaulted || dacl == NULL)
    return -1;


  ACL_SIZE_INFORMATION aclSize;
  if (!GetAclInformation(dacl, &aclSize, sizeof(aclSize), AclSizeInformation))
    return -1;

  int oldPermissions = 0;
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
                           account.GetPointer(1000), &accountLen,
                           domain.GetPointer(1000), &domainLen,
                           &usage)) {
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
    return -1;

  if (newPermissions != -1)
    SetFileSecurity(filename, DACL_SECURITY_INFORMATION, descriptor);

  return oldPermissions;
}

#endif


BOOL PFile::GetInfo(const PFilePath & name, PFileInfo & info)
{
  if (name.IsEmpty())
    return FALSE;

  PString fn = name;
  PINDEX pos = fn.GetLength()-1;
  while (PDirectory::IsSeparator(fn[pos]))
    pos--;
  fn.Delete(pos+1, P_MAX_INDEX);

  struct stat s;
  if (stat(fn, &s) != 0)
    return FALSE;

  info.created =  (s.st_ctime < 0) ? 0 : s.st_ctime;
  info.modified = (s.st_mtime < 0) ? 0 : s.st_mtime;
  info.accessed = (s.st_atime < 0) ? 0 : s.st_atime;
  info.size = s.st_size;

#if defined(_WIN32)
  info.permissions = FileSecurityPermissions(name, -1);
  if (info.permissions < 0)
#endif
  {
    info.permissions = 0;
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

  return TRUE;
}

#endif // _WIN32_WCE

BOOL PFile::SetPermissions(const PFilePath & name, int permissions)
{
#if defined(_WIN32) && !defined(_WIN32_WCE)
  FileSecurityPermissions(name, permissions);
#endif

  return _chmod(name, permissions&(_S_IWRITE|_S_IREAD)) == 0;
}


BOOL PFile::IsTextFile() const
{
  return FALSE;
}


BOOL PFile::Open(OpenMode mode, int opts)
{
  Close();
  clear();

  if (path.IsEmpty())
    path = PFilePath("PWL", NULL);

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

  if ((opts&Create) != 0)
    oflags |= O_CREAT;
  if ((opts&Exclusive) != 0)
    oflags |= O_EXCL;
  if ((opts&Truncate) != 0)
    oflags |= O_TRUNC;

  if ((opts&Temporary) != 0)
    removeOnClose = TRUE;

  int sflags = _SH_DENYNO;
  if ((opts&DenySharedRead) == DenySharedRead)
    sflags = _SH_DENYRD;
  else if ((opts&DenySharedWrite) == DenySharedWrite)
    sflags = _SH_DENYWR;
  else if ((opts&(DenySharedRead|DenySharedWrite)) != 0)
    sflags = _SH_DENYWR;

  os_handle = _sopen(path, oflags, sflags, S_IREAD|S_IWRITE);

  // As ConvertOSError tests for < 0 and some return values _sopen may be
  // negative, only pass -1 through.
  return ConvertOSError(os_handle == -1 ? -1 : 0);
}


BOOL PFile::SetLength(off_t len)
{
  return ConvertOSError(_chsize(GetHandle(), len));
}


///////////////////////////////////////////////////////////////////////////////
// PTextFile

BOOL PTextFile::IsTextFile() const
{
  return TRUE;
}


BOOL PTextFile::ReadLine(PString & str)
{
  char * ptr = str.GetPointer(100);
  PINDEX len = 0;
  int c;
  while ((c = ReadChar()) >= 0 && c != '\n') {
    *ptr++ = (char)c;
    if (++len >= str.GetSize())
      ptr = str.GetPointer(len + 100) + len;
  }
  *ptr = '\0';
  PAssert(str.MakeMinimumSize(), POutOfMemory);
  return c >= 0 || len > 0;
}


BOOL PTextFile::WriteLine(const PString & str)
{
  return WriteString(str) && WriteChar('\n');
}


///////////////////////////////////////////////////////////////////////////////
// PConsoleChannel

PConsoleChannel::PConsoleChannel()
{
}


PConsoleChannel::PConsoleChannel(ConsoleType type)
{
  Open(type);
}


BOOL PConsoleChannel::Open(ConsoleType type)
{
  switch (type) {
    case StandardInput :
      os_handle = 0;
      return TRUE;

    case StandardOutput :
      os_handle = 1;
      return TRUE;

    case StandardError :
      os_handle = 2;
      return TRUE;
  }

  return FALSE;
}


PString PConsoleChannel::GetName() const
{
  return "\\\\.\\Console";
}


BOOL PConsoleChannel::Read(void * buffer, PINDEX length)
{
  flush();
  lastReadCount = _read(os_handle, buffer, length);
  return ConvertOSError(lastReadCount, LastReadError) && lastReadCount > 0;
}


BOOL PConsoleChannel::Write(const void * buffer, PINDEX length)
{
  flush();
  lastWriteCount = _write(os_handle, buffer, length);
  return ConvertOSError(lastWriteCount, LastWriteError) && lastWriteCount >= length;
}


BOOL PConsoleChannel::Close()
{
  os_handle = -1;
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

void PProcess::Construct()
{
  PSetErrorStream(&cerr);

#if !defined(_WIN32) && defined(_MSC_VER) && defined(_WINDOWS)
  _wsetscreenbuf(1, _WINBUFINF);
  _wsizeinfo ws;
  ws._version = _QWINVER;
  ws._type = _WINSIZEMAX;
  _wsetsize(1, &ws);
#endif

  houseKeeper = NULL;
}


BOOL PProcess::SetMaxHandles(int /*newLimit*/)
{
  // Not applicable
  return TRUE;
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
