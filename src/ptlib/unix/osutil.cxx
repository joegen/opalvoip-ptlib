/*
 * OSUTIL.CXX
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 */

#define _OSUTIL_CXX

#pragma implementation "timer.h"
#pragma implementation "pdirect.h"
#pragma implementation "file.h"
#pragma implementation "sfile.h"
#pragma implementation "textfile.h"
#pragma implementation "ptime.h"
#pragma implementation "timeint.h"
#pragma implementation "filepath.h"
#pragma implementation "lists.h"
#pragma implementation "pstring.h"
#pragma implementation "dict.h"
#pragma implementation "array.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <time.h>
#include <sys/time.h>

#ifdef P_HPUX9
#include <langinfo.h>
#else
#include <localeinfo.h>
#endif

#include <ctype.h>

#include "ptlib.h"

#if ! defined (P_USE_INLINES)

#include "../../common/osutil.inl"
#include "ptlib.inl"

#endif

#define	LINE_SIZE_STEP	100


static PString CanonicaliseDirectory (const PString & path)

{
  int l;
  PString canonical_path;
  PString slash("/");

  // if the path does not start with a slash, then the current directory
  // must be prepended
  if (path.IsEmpty() || path[0] != '/') 
    PAssertOS (getcwd(canonical_path.GetPointer(PATH_MAX), PATH_MAX));

  // if the path doesn't end in a slash, add one
  if (canonical_path[canonical_path.GetLength()-1] != '/')
    canonical_path += slash;

  char * ptr = (char *)path;
  char * end;

  for (;;) {
    // ignore slashes
    while (*ptr == '/' && *ptr != '\0')
      ptr++;

    // finished if end of string
    if (*ptr == '\0')
      break;

    // collect non-slash characters
    end = ptr;
    while (*end != '/' && *end != '\0')
      end++;

    // make a string out of the element
    PString element(ptr, end - ptr);
    
    if (element == "..") {
      PINDEX last_char = canonical_path.GetLength()-1;
      if (last_char > 0)
        canonical_path = canonical_path.Left(canonical_path.FindLast('/', last_char-1)+1);
    } else if (element == "." || element == "") {
    } else {
      canonical_path += element;
      canonical_path += slash;
    }
    ptr = end;
  }

  return canonical_path;
}

static PString CanonicaliseFilename(const PString & filename)

{
  PINDEX p;
  PString dirname;

  // if there is a slash in the string, extract the dirname
  if ((p = filename.FindLast('/')) != P_MAX_INDEX) {
    dirname = filename(0,p);
    while (filename[p] == '/')
      p++;
  } else
    p = 0;

  return CanonicaliseDirectory(dirname) + filename(p, P_MAX_INDEX);
}


///////////////////////////////////////////////////////////////////////////////
//
// timer

PTimeInterval PTimer::Tick()

{
  struct timeval tv;

  gettimeofday (&tv, NULL);

  return ((tv.tv_sec * 1000L)+(tv.tv_usec / 1000L)) & 0x7fffffff;
}



///////////////////////////////////////////////////////////////////////////////
//
// PDirectory
//

void PDirectory::CopyContents(const PDirectory & d)
{
  entryInfo = d.entryInfo;
  directory = NULL;
  entry     = NULL;
}

void PDirectory::Construct ()

{
  entryInfo = NULL;
  directory = NULL;
  entry     = NULL;

  PString::operator =(CanonicaliseDirectory(*this));
}

BOOL PDirectory::Open(int ScanMask)

{
  scanMask = ScanMask;

  if ((directory = opendir((const char *)*this)) == NULL)
    return FALSE;

  entryInfo = new PFileInfo;

  return Next();
}


BOOL PDirectory::Next()
{
  if (directory == NULL)
    return FALSE;

  do {
    do {
      if ((entry = readdir(directory)) == NULL)
        return FALSE;
    } while (strcmp(entry->d_name, "." ) == 0 ||
             strcmp(entry->d_name, "..") == 0);

    PAssert(PFile::GetInfo(*this+entry->d_name, *entryInfo), POperatingSystemError);
    if (scanMask == PFileInfo::AllPermissions)
      return TRUE;
  } while ((entryInfo->type & scanMask) == 0);

  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
// PFile
//

void PFile::SetFilePath(const PString & newName)
{
  PINDEX p;

  if ((p = newName.FindLast('/')) == P_MAX_INDEX) 
    path = CanonicaliseDirectory("") + newName;
  else
    path = CanonicaliseDirectory(newName(0,p)) + newName(p+1, P_MAX_INDEX);
}


BOOL PFile::Open(OpenMode mode, int opt)

{
  Close();
  clear();

  if (path.IsEmpty()) {
    char * tmp = tempnam(NULL, "PWL");
    PAssert(tmp != NULL, POperatingSystemError);
    path = PString(tmp);
    free(tmp);
  }

  int oflags = 0;
  switch (mode) {
    case ReadOnly :
      oflags |= O_RDONLY;
      if (opt == ModeDefault)
        opt = MustExist;
      break;
    case WriteOnly :
      oflags |= O_WRONLY;
      if (opt == ModeDefault)
        opt = Create|Truncate;
      break;
    default :
      oflags |= O_RDWR;
      if (opt == ModeDefault)
        opt = Create;
  }
  if ((opt&Create) != 0)
    oflags |= O_CREAT;
  if ((opt&Exclusive) != 0)
    oflags |= O_EXCL;
  if ((opt&Truncate) != 0)
    oflags |= O_TRUNC;

  removeOnClose = opt & Temporary;

  return ConvertOSError(os_handle = open(path, oflags, S_IREAD|S_IWRITE));
}


BOOL PFile::Access(const PString & name, OpenMode mode)
{
  int accmode;

  switch (mode) {
    case ReadOnly :
      accmode = 2;
      break;

    case WriteOnly :
      accmode = 4;
      break;

    default :
      accmode = 6;
  }

  return access(name, accmode) == 0;
}


BOOL PFile::GetInfo(const PFilePath & name, PFileInfo & status)

{
  struct stat s;

  if (stat(name, &s) != 0) {
    status.type = PFileInfo::UnknownFileType;
    return TRUE;
  }

  status.created     = s.st_ctime;
  status.modified    = s.st_mtime;
  status.accessed    = s.st_atime;
  status.size        = s.st_size;
  status.permissions = s.st_mode & PFileInfo::AllPermissions;

  switch (s.st_mode & S_IFMT) {
    case S_IFLNK :
      status.type = PFileInfo::SymbolicLink;
      break;
    case S_IFREG :
      status.type = PFileInfo::RegularFile;
      break;
    case S_IFDIR :
      status.type = PFileInfo::SubDirectory;
      break;
    case S_IFIFO :
      status.type = PFileInfo::Fifo;
      break;
    case S_IFCHR :
      status.type = PFileInfo::CharDevice;
      break;
    case S_IFBLK :
      status.type = PFileInfo::BlockDevice;
      break;
    case S_IFSOCK :
      status.type = PFileInfo::SocketDevice;
      break;
    default:
      status.type = PFileInfo::UnknownFileType;
      break;
  }

  return TRUE;
}


BOOL PFile::SetPermissions(const PFilePath & name, int permissions)

{
  mode_t mode = 0;

  if (permissions & PFileInfo::WorldExecute)
    mode |= S_IXOTH;
  if (permissions & PFileInfo::WorldWrite)
    mode |= S_IWOTH;
  if (permissions & PFileInfo::WorldRead)
    mode |= S_IROTH;
  if (permissions & PFileInfo::GroupExecute)
    mode |= S_IXGRP;
  if (permissions & PFileInfo::GroupWrite)
    mode |= S_IWGRP;
  if (permissions & PFileInfo::GroupRead)
    mode |= S_IRGRP;
  if (permissions & PFileInfo::UserExecute)
    mode |= S_IXUSR;
  if (permissions & PFileInfo::UserWrite)
    mode |= S_IWUSR;
  if (permissions & PFileInfo::UserRead)
    mode |= S_IRUSR;

  return chmod ((const char *)name, mode) == 0;
}

void PFile::CopyContents(const PFile & f)
{
  path      = f.path;
  os_handle = f.os_handle;
  osError   = 0;
}

///////////////////////////////////////////////////////////////////////////////
// PTextFile

BOOL PTextFile::WriteLine (const PString & line)

{
  if (!Write((const char *)line, line.GetLength()))
    return FALSE;

  char ch = '\n';
  return Write(&ch, 1);
}


BOOL PTextFile::ReadLine (PString & line)

{
  int len    = 0;
  int ch;
  char * base, * ptr;

  while (1) {
    len += LINE_SIZE_STEP;
    ptr = base = line.GetPointer(len);
    while ((ptr - base) < LINE_SIZE_STEP-1) {
      if ((ch = ReadChar()) < 0) {
        ConvertOSError(errno);
        return FALSE;
      }
      if (ch == '\n') {
        *ptr = '\0';
        line.MakeMinimumSize();
        return TRUE;
      }
      *ptr++ = ch;
    }
  } 
}

///////////////////////////////////////////////////////////////////////////////
// PFilePath

PFilePath::PFilePath(const PString & str)
  : PString(CanonicaliseFilename(str))
{
}


PFilePath::PFilePath(const char * cstr)
  : PString(CanonicaliseFilename(cstr))
{
}


PFilePath::PFilePath(const char * prefix, const char * dir)
  : PString()
{
  if (prefix == NULL)
    prefix = "tmp";
  
  char * n;
  if (dir == NULL) {
    n = tempnam(NULL, prefix);
    *this = CanonicaliseFilename(n);
    free (n);
  } else {
    PDirectory s(dir);
    PString p = PString(prefix) + "XXXXXX";
    mktemp(p.GetPointer());
    *this = s + p;
  }
}


PFilePath & PFilePath::operator=(const PString & str)
{
  PString::operator=(CanonicaliseFilename(str));
  return *this;
}


PString PFilePath::GetPath() const

{
  int i;

  PAssert((i = FindLast('/')) != P_MAX_INDEX, PInvalidArrayIndex);
  return Left(i+1);
}


PString PFilePath::GetTitle() const

{
  PString fn(GetFileName());
  return fn(0, fn.FindLast('.')-1);
}


PString PFilePath::GetType() const

{
  int p = FindLast('.');
  int l = (p == P_MAX_INDEX) ? 0 : (GetLength() - p);

  if (p < 0 || l > 4 || l < 2)
    return PString("");
  else
    return Right(GetLength()-p);
}


PString PFilePath::GetFileName() const

{
  int i;

  if ((i = FindLast('/')) == P_MAX_INDEX)
    return *this;
  else
    return Right(GetLength()-i-1);
}


//////////////////////////////////////////////////////
//
//  PTime
//

PString PTime::GetTimeSeparator()
{
#ifdef P_HPUX9
  char * p = nl_langinfo(T_FMT);
#else
  char * p = _time_info->time; 
#endif
  char buffer[2];
  while (*p == '%' || isalpha(*p))
    p++;
  buffer[0] = *p;
  buffer[1] = '\0';
  return PString(buffer);
}

PTime::DateOrder PTime::GetDateOrder()
{
#ifdef P_HPUX9
  char * p = nl_langinfo(D_FMT);
#else
  char * p = _time_info->date; 
#endif

  while (*p == '%')
    p++;
  switch (tolower(*p)) {
    case 'd':
      return DayMonthYear;
    case 'y':
      return YearMonthDay;
    case 'm':
    default:
      break;
  }
  return MonthDayYear;
}

PString PTime::GetDateSeparator()
{
#ifdef P_HPUX9
  char * p = nl_langinfo(D_FMT);
#else
  char * p = _time_info->date; 
#endif

  char buffer[2];
  while (*p == '%' || isalpha(*p))
    p++;
  buffer[0] = *p;
  buffer[1] = '\0';
  return PString(buffer);
}

PString PTime::GetDayName(PTime::Weekdays day, BOOL abbreviated)

{
#ifdef P_HPUX9
  return PString(
     abbreviated ? nl_langinfo(ABDAY_1+(int)day) :
                   nl_langinfo(DAY_1+(int)day)
                );
#else
  return abbreviated ? PString(_time_info->abbrev_wkday[(int)day]) :
                       PString(_time_info->full_wkday[(int)day]);
#endif
}

PString PTime::GetMonthName(PTime::Months month, BOOL abbreviated) 
{
#ifdef P_HPUX9
  return PString(
     abbreviated ? nl_langinfo(ABMON_1+(int)month-1) :
                   nl_langinfo(MON_1+(int)month-1)
                );
#else
  return abbreviated ? PString(_time_info->abbrev_month[(int)month-1]) :
                       PString(_time_info->full_month[(int)month-1]);
#endif
}

// End Of File ///////////////////////////////////////////////////////////////
