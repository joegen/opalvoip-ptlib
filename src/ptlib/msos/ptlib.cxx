/*
 * $Id: ptlib.cxx,v 1.1 1994/04/01 14:39:35 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: ptlib.cxx,v $
 * Revision 1.1  1994/04/01 14:39:35  robertj
 * Initial revision
 *
 */

#include "ptlib.h"

#include <fcntl.h>
#include <errno.h>
#include <dos.h>


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
  PAssert(getcwd(curdir.GetPointer(P_MAX_PATH), P_MAX_PATH) != NULL);

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
      PAssert(getcwd(otherdir.GetPointer(P_MAX_PATH), P_MAX_PATH) != NULL);
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

  PFile::Info inf;
  PAssert(PFile::GetInfo(path+fileinfo.name, inf));
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


PString PDirectory::Entry() const
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
// PFile

PFile::PFile()
{
  char * tmp = tempnam("C:\\", "PWL");
  if (tmp != NULL) {
    fullname = tmp;
    free(tmp);
  }
  Construct();
}


void PFile::SetName(const PString & newName)
{
  fullname = FixPath(newName, FALSE);
}


void PFile::CopyContents(const PFile & f)
{
  fullname = f.fullname;
  os_handle = f.os_handle;
  os_errno = f.os_errno;
}


void PFile::Construct()
{
  os_handle = -1;
  os_errno = 0;
  fullname = FixPath(fullname, FALSE);
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


BOOL PFile::GetInfo(const PString & name, Info & info)
{
  struct stat s;
  if (stat(name, &s) != 0)
    return FALSE;

  info.created =  (s.st_ctime < 0) ? 0 : s.st_ctime;
  info.modified = (s.st_mtime < 0) ? 0 : s.st_mtime;
  info.accessed = (s.st_atime < 0) ? 0 : s.st_atime;
  info.filesize = s.st_size;

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


BOOL PFile::Open(OpenMode mode, int opts)
{
  Close();

  int oflags = (int)(IsDescendant(PTextFile::Class()) ? _O_TEXT : _O_BINARY);
  switch (mode) {
    case ReadOnly :
      oflags |= O_RDONLY;
      break;
    case WriteOnly :
      oflags |= O_WRONLY;
      break;
    default :
      oflags |= O_RDWR;
  }

  if ((opts&Create) != 0)
    oflags |= O_CREAT;
  if ((opts&Exclusive) != 0)
    oflags |= O_EXCL;
  if ((opts&Truncate) != 0)
    oflags |= O_TRUNC;

  BOOL ok = (os_handle = open(fullname, oflags, S_IREAD|S_IWRITE)) >= 0;
  os_errno = ok ? 0 : errno;
  return ok;
}


BOOL PFile::SetLength(off_t len)
{
  BOOL ok = chsize(GetHandle(), len) == 0;
  os_errno = ok ? 0 : errno;
  return ok;
}


PFile::Errors PFile::GetErrorCode() const
{
  switch (os_errno) {
    case 0 :
      return NoError;

    case ENOENT :
      return FileNotFound;

    case EEXIST :
      return FileExists;

    case ENOSPC :
      return DiskFull;

    case EACCES :
      return AccessDenied;

    default :
      return Miscellaneous;
  }
}


PString PFile::GetErrorText() const
{
  if (os_errno > 0 && os_errno < _sys_nerr)
    return _sys_errlist[os_errno];
  return PString();
}


///////////////////////////////////////////////////////////////////////////////
// PTextApplication

int PTextApplication::Main(int argc, char ** argv)
{
  PreInitialise(argc, argv);

  if (Initialise()) {
    terminationValue = 0;
    MainBody();
  }

  return Termination();
}


///////////////////////////////////////////////////////////////////////////////

#if !defined(P_USE_INLINES)

#include "../../common/osutil.inl"
#include "ptlib.inl"

#endif


// End Of File ///////////////////////////////////////////////////////////////
