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
#pragma implementation "ptime.h"
#pragma implementation "timeint.h"
#pragma implementation "filepath.h"
#pragma implementation "lists.h"
#pragma implementation "pstring.h"
#pragma implementation "dict.h"
#pragma implementation "array.h"
#pragma implementation "object.h"
#pragma implementation "contain.h"

#include <ptlib.h>

#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>

#if defined(P_LINUX)

#if (__GNUC_MINOR__ < 7)
#include <localeinfo.h>
#else
#define P_USE_LANGINFO
#endif

#elif defined(P_HPUX9) 
#define P_USE_LANGINFO

#elif defined(P_SOLARIS) 
#define P_USE_LANGINFO
#include <sys/timeb.h>

#elif defined(P_SUN4)
#include <sys/timeb.h>

#endif

#ifdef P_USE_LANGINFO
#include <langinfo.h>
#endif

#define	LINE_SIZE_STEP	100

#define	DEFAULT_FILE_MODE	(S_IRUSR|S_IWUSR|S_IROTH|S_IRGRP)

#ifndef P_USE_INLINES
#include "../../common/ptlib/osutil.inl"
#include "ptlib.inl"
#endif

#ifdef P_SUN4
extern "C" {
int on_exit(void (*f)(void), caddr_t);
int atexit(void (*f)(void))
{
  return on_exit(f, 0);
}
static char *tzname[2] = { "STD", "DST" };
};
#endif

static PString CanonicaliseDirectory (const PString & path)

{
  PString canonical_path;
  PString slash("/");

  // if the path does not start with a slash, then the current directory
  // must be prepended
  if (path.IsEmpty() || path[0] != '/') 
    PAssertOS (getcwd(canonical_path.GetPointer(P_MAX_PATH), P_MAX_PATH));

  // if the path doesn't end in a slash, add one
  if (canonical_path[canonical_path.GetLength()-1] != '/')
    canonical_path += slash;

  const char * ptr = path;
  const char * end;

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
  ::gettimeofday (&tv, NULL);
  return (PInt64)(tv.tv_sec) * 1000 + tv.tv_usec/1000L;
}



///////////////////////////////////////////////////////////////////////////////
//
// PDirectory
//

void PDirectory::CopyContents(const PDirectory & d)
{
  if (d.entryInfo == NULL)
    entryInfo = NULL;
  else {
    entryInfo  = PNEW PFileInfo;
    *entryInfo = *d.entryInfo;
  }
  directory  = NULL;
  entry      = NULL;
}

void PDirectory::Close()
{
  if (directory != NULL)
    PAssert(closedir(directory) == 0, POperatingSystemError);
  directory = NULL;
  if (entryInfo != NULL)
    delete entryInfo;
  entryInfo = NULL;
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
  if (directory != NULL)
    Close();

  scanMask = ScanMask;

  if ((directory = opendir((const char *)*this)) == NULL)
    return FALSE;

  entryInfo = PNEW PFileInfo;

  if (Next())
    return TRUE;
  Close();
  return FALSE;
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

BOOL PDirectory::GetInfo(PFileInfo & info) const
{
  info = *entryInfo;
  return TRUE;
}

BOOL PDirectory::Create(const PString & p, int perm)
{
  PAssert(!p.IsEmpty(), "attempt to create dir with empty name");
  PString str = p.Left(p.GetLength()-1);
  return mkdir(str, perm) == 0;
}

BOOL PDirectory::Remove(const PString & p)
{
  PAssert(!p.IsEmpty(), "attempt to remove dir with empty name");
  PString str = p.Left(p.GetLength()-1);
  return rmdir(str) == 0;
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

  if (!ConvertOSError(os_handle = ::open(path, oflags, DEFAULT_FILE_MODE)))
    return FALSE;

  return ConvertOSError(::fcntl(os_handle, F_SETFD, 1));
}


BOOL PFile::Rename(const PFilePath & oldname, const PString & newname, BOOL force)
{
  if (newname.Find('/') != P_MAX_INDEX) {
    errno = EINVAL;
    return FALSE;
  }

  if (rename(oldname, oldname.GetPath() + newname) == 0)
    return TRUE;

  if (!force || errno == ENOENT || !Exists(newname))
    return FALSE;

  if (!Remove(newname, TRUE))
    return FALSE;

  return rename(oldname, oldname.GetPath() + newname) == 0;
}


BOOL PFile::Move(const PFilePath & oldname, const PFilePath & newname, BOOL force)
{
  PFilePath from = oldname.GetDirectory() + oldname.GetFileName();
  PFilePath to = newname.GetDirectory() + newname.GetFileName();

  if (rename(from, to) == 0)
    return TRUE;

  if (errno == EXDEV)
    return Copy(from, to, force) && Remove(from);

  if (force && errno == EEXIST)
    if (Remove(to, TRUE))
      if (rename(from, to) == 0)
	return TRUE;

  return FALSE;
}


BOOL PFile::Access(const PFilePath & name, OpenMode mode)
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
  if (lstat(name, &s) != 0) {
    status.type = PFileInfo::UnknownFileType;
    return TRUE;
  }

  status.type = (PFileInfo::FileTypes)0;

  if (S_ISLNK(s.st_mode)) {
    status.type = PFileInfo::SymbolicLink;
    if (stat(name, &s) != 0) 
      return TRUE;
  } 

  status.created     = s.st_ctime;
  status.modified    = s.st_mtime;
  status.accessed    = s.st_atime;
  status.size        = s.st_size;
  status.permissions = s.st_mode & PFileInfo::AllPermissions;

  if (S_ISREG(s.st_mode))
    status.type = PFileInfo::RegularFile;

  else if (S_ISDIR(s.st_mode))
    status.type = PFileInfo::SubDirectory;

  else if (S_ISFIFO(s.st_mode))
    status.type = PFileInfo::Fifo;

  else if (S_ISCHR(s.st_mode))
    status.type = PFileInfo::CharDevice;

  else if (S_ISBLK(s.st_mode))
    status.type = PFileInfo::BlockDevice;

  else if (S_ISSOCK(s.st_mode))
    status.type = PFileInfo::SocketDevice;

  if (status.type == 0)
    status.type = PFileInfo::UnknownFileType;

  return TRUE;
}


BOOL PFile::SetPermissions(const PFilePath & name, int permissions)

{
  mode_t mode = 0;

    mode |= S_IROTH;
    mode |= S_IRGRP;

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
    PString p = s + prefix + "XXXXXX";
    if (mktemp(p.GetPointer()) == NULL) {
      char extra = 'a';
      do 
        PString p = s + prefix + extra++ + "XXXXXX";
      while (mktemp(p.GetPointer()) == NULL && extra <= 'z');
    }
    *this = PString(p);
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

  if (p < 0 || l < 2)
    return PString("");
  else
    return (*this)(p, P_MAX_INDEX);
}


PString PFilePath::GetFileName() const

{
  int i;

  if ((i = FindLast('/')) == P_MAX_INDEX)
    return *this;
  else
    return Right(GetLength()-i-1);
}


PDirectory PFilePath::GetDirectory() const
{
  int i;

  if ((i = FindLast('/')) == P_MAX_INDEX)
    return "./";
  else
    return Left(i);
}


//////////////////////////////////////////////////////
//
//  PTime
//

PString PTime::GetTimeSeparator()
{
#if defined(P_LINUX) || defined(P_HPUX9) || defined(P_SOLARIS)
#  if defined(P_USE_LANGINFO)
     char * p = nl_langinfo(T_FMT);
#  elif defined(P_LINUX)
     char * p = _time_info->time; 
#  endif
  char buffer[2];
  while (*p == '%' || isalpha(*p))
    p++;
  buffer[0] = *p;
  buffer[1] = '\0';
  return PString(buffer);
#elif defined(P_SUN4)
  return PString(":");
#else

#warning Using default time separator
  return PString(":");
#endif
}

PTime::DateOrder PTime::GetDateOrder()
{
#if defined(P_LINUX) || defined(P_HPUX9) || defined(P_SOLARIS)
#  if defined(P_USE_LANGINFO)
     char * p = nl_langinfo(D_FMT);
#  elif defined(P_LINUX)
     char * p = _time_info->date; 
#  endif

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

#elif defined(P_SUN4)
  return DayMonthYear;

#else
#warning Using default date order
  return DayMonthYear;
#endif
}

PString PTime::GetDateSeparator()
{
#if defined(P_SUN4)
  return PString("/");
#elif defined(P_LINUX) || defined(P_HPUX9) || defined(P_SOLARIS)
#  if defined(P_USE_LANGINFO)
     char * p = nl_langinfo(D_FMT);
#  elif defined(P_LINUX)
     char * p = _time_info->date; 
#  endif

  char buffer[2];
  while (*p == '%' || isalpha(*p))
    p++;
  buffer[0] = *p;
  buffer[1] = '\0';
  return PString(buffer);
#elif defined(P_SUN4)
  return PString("/");
#else
#warning No date separator
  return PString("/");
#endif
}

PString PTime::GetDayName(PTime::Weekdays day, NameType type)

{
#if defined(P_USE_LANGINFO)
  return PString(
     (type == Abbreviated) ? nl_langinfo((nl_item)(ABDAY_1+(int)day)) :
                   nl_langinfo((nl_item)(DAY_1+(int)day))
                );

#elif defined(P_LINUX)
  return (type == Abbreviated) ? PString(_time_info->abbrev_wkday[(int)day]) :
                       PString(_time_info->full_wkday[(int)day]);

#else
#if ! defined(P_SUN4)
#warning Using default day names
#endif
  static char *defaultNames[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
    "Saturday"
  };

  static char *defaultAbbrev[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  return (type == Abbreviated) ? PString(defaultNames[(int)day]) :
                       PString(defaultAbbrev[(int)day]);
#endif
}

PString PTime::GetMonthName(PTime::Months month, NameType type) 
{
#if defined(P_USE_LANGINFO)
  return PString(
     (type == Abbreviated) ? nl_langinfo((nl_item)(ABMON_1+(int)month-1)) :
                   nl_langinfo((nl_item)(MON_1+(int)month-1))
                );
#elif defined(P_LINUX)
  return (type == Abbreviated) ? PString(_time_info->abbrev_month[(int)month-1]) :
                       PString(_time_info->full_month[(int)month-1]);
#else
#if ! defined(P_SUN4)
#warning Using default monthnames
#endif
  static char *defaultNames[] = {
  "January", "February", "March", "April", "May", "June", "July", "August",
  "September", "October", "November", "December" };

  static char *defaultAbbrev[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
  "Sep", "Oct", "Nov", "Dec" };

  return (type == Abbreviated) ? PString(defaultNames[(int)month-1]) :
                       PString(defaultAbbrev[(int)month-1]);
#endif
}


BOOL PTime::IsDaylightSavings()
{
  time_t theTime = ::time(NULL);
  return ::localtime(&theTime)->tm_isdst != 0;
}

int PTime::GetTimeZone(PTime::TimeZoneType type) 
{
#if defined(P_LINUX) || defined(P_SOLARIS)
  long tz = -::timezone/60;
  if (type == StandardTime)
    return tz;
  else
    return tz + ::daylight*60;
#elif defined(P_SUN4) 
  struct timeb tb;
  ftime(&tb);
  if (type == StandardTime || tb.dstflag == 0)
    return -tb.timezone;
  else
    return -tb.timezone + 60;
#else
#warning No timezone information
  return 0; 
#endif
}

PString PTime::GetTimeZoneString(PTime::TimeZoneType type) 
{
#if defined(P_LINUX) || defined(P_SUN4) || defined(P_SOLARIS)
  const char * str = (type == StandardTime) ? ::tzname[0] : ::tzname[1]; 
  if (str != NULL)
    return PString();
  else
    return PString(str);
#else
#warning No timezone name information
  return PString(); 
#endif
}


// End Of File ///////////////////////////////////////////////////////////////
