/*
 * $Id: ptlib.cxx,v 1.2 1994/06/25 12:13:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: ptlib.cxx,v $
 * Revision 1.2  1994/06/25 12:13:01  robertj
 * Synchronisation.
 *
// Revision 1.1  1994/04/01  14:39:35  robertj
// Initial revision
//
 */

#include "ptlib.h"

#include <fcntl.h>
#include <errno.h>
#include <dos.h>
#include <sys\stat.h>
#include <malloc.h>

#ifndef P_USE_INLINES
#include <ptlib.inl>
#endif


///////////////////////////////////////////////////////////////////////////////
// PChannel

PString PChannel::GetErrorText() const
{
  if (osError == 0)
    return PString();

  if (osError > 0 && osError < _sys_nerr)
    return _sys_errlist[osError];

  return psprintf("OS error %d", osError);
}


///////////////////////////////////////////////////////////////////////////////
// Directories

void PDirectory::CopyContents(const PDirectory & dir)
{
  path = dir.path;
  scanMask = dir.scanMask;
  fileinfo = dir.fileinfo;
}


static PString FixPath(const PString & path, BOOL isDirectory)
{
  PString curdir;
  PAssert(getcwd(curdir.GetPointer(P_MAX_PATH),
                                   P_MAX_PATH) != NULL, POperatingSystemError);

  PString fullpath;

  PINDEX offset;
  if (path.GetLength() < 2 || path[1] != ':') {
    fullpath = curdir(0,1);
    offset = 0;
  }
  else {
    fullpath = path(0,1).ToUpper();
    offset = 2;
  }

  char slash = path[offset];
  if (slash != '\\' && slash != '/') {
    if (fullpath[0] == curdir[0])
      fullpath += curdir(2, P_MAX_INDEX);
    else if (_chdrive(fullpath[0]-'A'+1) == 0) {
      PString otherdir;
      PAssert(getcwd(otherdir.GetPointer(P_MAX_PATH),
                                   P_MAX_PATH) != NULL, POperatingSystemError);
      fullpath += otherdir(2, P_MAX_INDEX);
      _chdrive(curdir[0]-'A'+1);  // Put drive back
    }
    slash = fullpath[fullpath.GetLength()-1];
    if (slash != '\\' && slash != '/')
      fullpath += "\\";
  }

  fullpath += path(offset, P_MAX_INDEX);

  slash = fullpath[fullpath.GetLength()-1];
  if (isDirectory && slash != '\\' && slash != '/')
    fullpath += "\\";

  int pos;
  while ((pos = fullpath.Find('/')) != P_MAX_INDEX)
    fullpath[pos] = '\\';

  while ((pos = fullpath.Find("\\.\\")) != P_MAX_INDEX)
    fullpath = fullpath(0, pos) + fullpath(pos+3, P_MAX_INDEX);

  while ((pos = fullpath.Find("\\..\\")) != P_MAX_INDEX)
    fullpath = fullpath(0, fullpath.FindLast('\\', pos-1)) +
                                                  fullpath(pos+4, P_MAX_INDEX);

  return fullpath.ToUpper();
}


void PDirectory::Construct()
{
  path = FixPath(path, TRUE);
}


BOOL PDirectory::Change(const PString & p)
{
  if (p[1] == ':') {
    if (_chdrive(toupper(p[0])-'A'+1) < 0)
      return FALSE;
  }
  PINDEX len = p.GetLength();
  if (p[len-1] == '\\')
    len--;
  return chdir(p(0, len-1)) == 0;
}


BOOL PDirectory::Filtered()
{
  if (strcmp(fileinfo.name, ".") == 0)
    return TRUE;
  if (strcmp(fileinfo.name, "..") == 0)
    return TRUE;
  if (scanMask == PAllPermissions)
    return FALSE;

  PFileInfo inf;
  PAssert(PFile::GetInfo(path+fileinfo.name, inf), POperatingSystemError);
  return (inf.type&scanMask) == 0;
}


BOOL PDirectory::Open(int newScanMask)
{
  scanMask = newScanMask;
  if (_dos_findfirst(path+"*.*", 0xff, &fileinfo) != 0)
    return FALSE;

  return Filtered() ? Next() : TRUE;
}


BOOL PDirectory::Next()
{
  do {
    if (_dos_findnext(&fileinfo) != 0)
      return FALSE;
  } while (Filtered());

  return TRUE;
}


PString PDirectory::GetEntryName() const
{
  return fileinfo.name;
}


BOOL PDirectory::IsSubDir() const
{
  return (fileinfo.attrib&_A_SUBDIR) != 0;
}


void PDirectory::Close()
{
  /* do nothing */
}


///////////////////////////////////////////////////////////////////////////////
// File Path

PFilePath::PFilePath(const PString & str)
  : PString(FixPath(str, FALSE))
{
}


PFilePath::PFilePath(const char * cstr)
  : PString(FixPath(cstr, FALSE))
{
}


PFilePath & PFilePath::operator=(const PString & str)
{
  PString::operator=(FixPath(str, FALSE));
  return *this;
}


void PFilePath::SetType(const PString & type)
{
  *this = operator()(0, FindLast('.')-1) + type;
}


///////////////////////////////////////////////////////////////////////////////
// PFile

void PFile::SetFilePath(const PString & newName)
{
  path = FixPath(newName, FALSE);
}


void PFile::CopyContents(const PFile & f)
{
  path = f.path;
  os_handle = f.os_handle;
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


BOOL PFile::GetInfo(const PFilePath & name, PFileInfo & info)
{
  struct stat s;
  if (stat(name, &s) != 0)
    return FALSE;

  info.created =  (s.st_ctime < 0) ? 0 : s.st_ctime;
  info.modified = (s.st_mtime < 0) ? 0 : s.st_mtime;
  info.accessed = (s.st_atime < 0) ? 0 : s.st_atime;
  info.size = s.st_size;

  info.permissions = 0;
  if ((s.st_mode&S_IREAD) != 0)
    info.permissions |= PUserRead|PGroupRead|PWorldRead;
  if ((s.st_mode&S_IWRITE) != 0)
    info.permissions |= PUserWrite|PGroupWrite|PWorldWrite;
  if ((s.st_mode&S_IEXEC) != 0)
    info.permissions |= PUserExecute|PGroupExecute|PWorldExecute;

  switch (s.st_mode & S_IFMT) {
    case S_IFREG :
      info.type = PRegularFile;
      break;

    case S_IFDIR :
      info.type = PSubDirectory;
      break;

    default:
      info.type = PUnknownFileType;
      break;
  }

  unsigned int attr;
  _dos_getfileattr(name, &attr);
  info.hidden = (attr & _A_HIDDEN) != 0;

  return TRUE;
}


BOOL PFile::IsTextFile() const
{
  return FALSE;
}


BOOL PFile::Open(OpenMode mode, int opts)
{
  Close();
  clear();

  if (path.IsEmpty()) {
    char * tmp = tempnam("C:\\", "PWL");
    path = PString(PAssertNULL(tmp));
    free(tmp);
  }

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
    default :
      oflags |= O_RDWR;
      if (opts == ModeDefault)
        opts = Create;
  }

  if ((opts&Create) != 0)
    oflags |= O_CREAT;
  if ((opts&Exclusive) != 0)
    oflags |= O_EXCL;
  if ((opts&Truncate) != 0)
    oflags |= O_TRUNC;

  BOOL ok = (os_handle = _open(path, oflags, S_IREAD|S_IWRITE)) >= 0;
  osError = ok ? 0 : errno;
  switch (osError) {
    case 0 :
      lastError = NoError;
    case ENOENT :
      lastError = NotFound;
    case EEXIST :
      lastError = FileExists;
    case EACCES :
      lastError = AccessDenied;
    case ENOMEM :
      lastError = NoMemory;
    default :
      lastError = Miscellaneous;
  }
  return ok;
}


BOOL PFile::SetLength(off_t len)
{
  BOOL ok = chsize(GetHandle(), len) == 0;
  osError = ok ? 0 : errno;
  switch (osError) {
    case 0 :
      lastError = NoError;
    case ENOSPC :
      lastError = DiskFull;
    case EACCES :
      lastError = AccessDenied;
    case ENOMEM :
      lastError = NoMemory;
    case EBADF :
      lastError = NotOpen;
    default :
      lastError = Miscellaneous;
  }
  return ok;
}


///////////////////////////////////////////////////////////////////////////////
// PTextFile

BOOL PTextFile::IsTextFile() const
{
  return TRUE;
}


BOOL PTextFile::Read(void * buf, PINDEX len)
{
  BOOL retVal = PFile::Read(buf, len);
  return retVal;
}


BOOL PTextFile::Write(const void * buf, PINDEX len)
{
  BOOL retVal = PFile::Write(buf, len);
  return retVal;
}


///////////////////////////////////////////////////////////////////////////////
// PTextFile

PThread::PThread(PINDEX stackSize, BOOL startSuspended, Priority priorityLevel)
{
  basePriority = priorityLevel;   // Threads user settable priority level
  dynamicPriority = 0;            // Allow immediate scheduling
  isBlocked = NULL;               // No I/O blocking function

  suspendCount = startSuspended ? 1 : 0;

  stackBase = (char NEAR *)_nmalloc(stackSize);
  PAssert(stackBase != NULL, "Insufficient near heap for thread");

  stackTop = stackBase + stackSize; // For stack checking code
  stackUsed = 0;

  PThread * current = Current();
  link = current->link;
  current->link = this;
  status = Starting;
  if (!startSuspended)
    SwitchContext(current);
}


void PThread::Terminate()
{
  if (link == this) // Is only thread, cannot terminate it.
    return;

  if (status == Terminated) // Is already terminated
    return;

  status = Terminating;

  // If current thread is self terminating then go to next thread
  if (Current() == this)
    Yield(); // Never returns from here
}


void PThread::Suspend(BOOL susp)
{
  // Suspend/Resume the thread
  if (susp)
    suspendCount++;
  else
    suspendCount--;

  // Suspending itself, yield to next thread
  if (IsSuspended() && Current() == this)
    Yield();
}


void PThread::Sleep(const PTimeInterval & time)
{
  wakeUpTime = PTimer::Tick() + time;

  // Current thread is going to sleep, so yield to other thread
  if (Current() == this)
    Yield();
}


void PThread::Yield()
{
  // Determine the next thread to schedule
  PThread * current = Current();
  PThread * prev = current; // Need the thread in the previous link
  for (PThread * next = current->link; next != current; next = next->link) {
    if (next->status == Terminating) {
      prev->link = next->link;   // Unlink it from the list
      _nfree(next->stackBase);   // Give stack back to the near heap
      next->status = Terminated;
      continue;
    }

    prev = next;

    if (next->suspendCount > 0)
      continue;

    if (PTimer::Tick() < next->wakeUpTime)
      continue;

    if (next->isBlocked != NULL) {
      if (next->isBlocked(next->blocker))
        continue;
      next->isBlocked = NULL;
      break;
    }

    if (next->dynamicPriority <= 0)
      break;

    next->dynamicPriority--;
  }

  switch (next->basePriority) {
    case LowPriority :
      next->dynamicPriority = 3;
      break;
    case NormalPriority :
      next->dynamicPriority = 1;
      break;
    case HighPriority :
      next->dynamicPriority = 0;
      break;
  }

  next->SwitchContext(current);
}


void PThread::Block(PThreadBlockFunction isBlockFun, PObject * obj)
{
  isBlocked = isBlockFun;
  blocker = obj;
  Yield();
}


void PThread::InitialiseProcessThread()
{
  basePriority = NormalPriority;  // User settable priority
  dynamicPriority = 0;            // Allow immediate scheduling
  suspendCount = 0;               // Not suspended (would not be a good idea)
  isBlocked = NULL;               // No I/O blocking function
  status = Running;               // Thread is already running
  link = this;                    // Circular list - is only one
  PProcess::Current()->currentThread = this;
}


// End Of File ///////////////////////////////////////////////////////////////
