/*
 * $Id: ptlib.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: ptlib.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <ptlib.h>

#ifndef P_USE_INLINES
#include <osutil.inl>
#include <ptlib.inl>
#endif

#include <errno.h>


PProcess * PSTATIC PProcessInstance;
ostream * PSTATIC PErrorStream = &cerr;



///////////////////////////////////////////////////////////////////////////////
// PTime

PString PTime::GetTimeSeparator()
{
  return "";
}


BOOL PTime::GetTimeAMPM()
{
  return FALSE;
}


PString PTime::GetTimeAM()
{
  return "am";
}


PString PTime::GetTimePM()
{
  return "pm";
}


PString PTime::GetDayName(Weekdays dayOfWeek, BOOL abbreviated)
{
  static const char * const weekdays[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };
  static const char * const abbrev_weekdays[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  return (abbreviated ? abbrev_weekdays : weekdays)[dayOfWeek];
}


PString PTime::GetDateSeparator()
{
  return "-";
}


PString PTime::GetMonthName(Months month, BOOL abbreviated)
{
  static const char * const months[] = { "",
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
  };
  static const char * const abbrev_months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  return (abbreviated ? abbrev_months : months)[month];
}


PTime::DateOrder PTime::GetDateOrder()
{
  return DayMonthYear;
}


///////////////////////////////////////////////////////////////////////////////
// Directories

void PDirectory::Construct()
{
  PString::operator=(CreateFullPath(*this, TRUE));
}


void PDirectory::CopyContents(const PDirectory & dir)
{
}


BOOL PDirectory::Open(int newScanMask)
{
  return Filtered() ? Next() : TRUE;
}


BOOL PDirectory::Next()
{
  do {
  } while (Filtered());

  return TRUE;
}


PCaselessString PDirectory::GetEntryName() const
{
  return "";
}


BOOL PDirectory::IsSubDir() const
{
  return FALSE;
}


void PDirectory::Close()
{
}


PCaselessString PDirectory::GetVolume() const
{
  return PCaselessString();
}


PString PDirectory::CreateFullPath(const PString & path, BOOL isDirectory)
{
  return path;
}


PDirectory PDirectory::GetParent() const
{
  if (IsRoot())
    return *this;
  
  return *this + "::";
}


BOOL PDirectory::Change(const PString & p)
{
  return FALSE;
}


BOOL PDirectory::Filtered()
{
  return FALSE;
}


BOOL PDirectory::IsRoot() const
{
  return FALSE;
}


BOOL PDirectory::GetInfo(PFileInfo & info) const
{
  return PFile::GetInfo(*this + GetEntryName(), info);
}

BOOL PDirectory::Exists(const PString & p)
  { return FALSE; }

BOOL PDirectory::Create(const PString & p, int)
  { return FALSE; }

BOOL PDirectory::Remove(const PString & p)
  { return FALSE; }



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
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


PString PChannel::GetErrorText() const
{
  if (osError == 0)
    return PString();

  return psprintf("OS error %u", osError);
}


BOOL PChannel::ConvertOSError(int error)
{
  if (error >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

  osError = errno;
  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;
    case 1 :
      lastError = NotFound;
      break;
    case 2 :
      lastError = FileExists;
      break;
    case 3 :
      lastError = AccessDenied;
      break;
    case 4 :
      lastError = NoMemory;
      break;
    case 5 :
      lastError = DiskFull;
      break;
    case 6 :
      lastError = BadParameter;
      break;
    case 7 :
      lastError = NotOpen;
      break;
    default :
      lastError = Miscellaneous;
  }

  return FALSE;
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
    *this = PDirectory();
  }
  if (prefix != NULL)
    *this += prefix;
  else
    *this += "PW";
  *this += "XXXXXX";
}


PFilePath & PFilePath::operator=(const PString & str)
{
  PCaselessString::operator=(PDirectory::CreateFullPath(str, FALSE));
  return *this;
}


PCaselessString PFilePath::GetVolume() const
{
  PINDEX colon = Find(':');
  if (colon == P_MAX_INDEX)
    return PCaselessString();
  return Left(colon+1);
}


PCaselessString PFilePath::GetPath() const
{
  PINDEX last = FindLast(':');
  if (last == P_MAX_INDEX)
    return PCaselessString();

  PINDEX colon = Find(':');
  if (colon == P_MAX_INDEX)
    colon = 0;
  else
    colon++;

  return operator()(colon, last);
}


PCaselessString PFilePath::GetFileName() const
{
  PINDEX colon = FindLast(':');
  if (colon == P_MAX_INDEX)
    colon = 0;
  else
    colon++;

  return Mid(colon);
}


PCaselessString PFilePath::GetTitle() const
{
  return GetFileName();
}


PCaselessString PFilePath::GetType() const
{
  return PCaselessString();
}


void PFilePath::SetType(const PCaselessString & type)
{
}


///////////////////////////////////////////////////////////////////////////////
// PFile

BOOL PFile::Exists(const PFilePath & name)
  { return FALSE; }


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
      accmode = 2;
      break;

    case WriteOnly :
      accmode = 4;
      break;

    default :
      accmode = 6;
  }

  return FALSE;
}


BOOL PFile::Remove(const PFilePath & name, BOOL force)
{
    return FALSE;
}


BOOL PFile::Rename(const PFilePath & oldname, const PString & newname, BOOL force)
{
  return FALSE;
}


BOOL PFile::Move(const PFilePath & oldname, const PFilePath & newname, BOOL force)
{
  return Copy(oldname, newname, force) && Remove(oldname);
}


BOOL PFile::GetInfo(const PFilePath & name, PFileInfo & info)
{
  return FALSE;
}


BOOL PFile::SetPermissions(const PFilePath & name, int permissions)
{
  return FALSE;
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

  if ((opts&Temporary) != 0)
    removeOnClose = TRUE;

  return ConvertOSError(os_handle = _open(path, oflags));
}


BOOL PFile::SetLength(off_t len)
{
  return FALSE;
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
// PSerialChannel

void PSerialChannel::Construct()
{
}


PString PSerialChannel::GetName() const
{
  return PString();
}


BOOL PSerialChannel::Read(void * buf, PINDEX len)
{
  return FALSE;
}


BOOL PSerialChannel::Write(const void * buf, PINDEX len)
{
  return FALSE;
}


BOOL PSerialChannel::Close()
{
  if (!IsOpen())
    return FALSE;

  os_handle = -1;
  return TRUE;
}


BOOL PSerialChannel::Open(const PString & port, DWORD speed, BYTE data,
       Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  Close();

  os_handle = -1;
  return FALSE;
}


BOOL PSerialChannel::SetSpeed(DWORD speed)
{
  return FALSE;
}


DWORD PSerialChannel::GetSpeed() const
{
  return 0;
}


BOOL PSerialChannel::SetDataBits(BYTE data)
{
  return FALSE;
}


BYTE PSerialChannel::GetDataBits() const
{
  return 8;
}


BOOL PSerialChannel::SetParity(Parity parity)
{
  return FALSE;
}


PSerialChannel::Parity PSerialChannel::GetParity() const
{
  return EvenParity;
}


BOOL PSerialChannel::SetStopBits(BYTE stop)
{
  return FALSE;
}


BYTE PSerialChannel::GetStopBits() const
{
  return 2;
}


BOOL PSerialChannel::SetInputFlowControl(FlowControl flowControl)
{
  return FALSE;
}


PSerialChannel::FlowControl PSerialChannel::GetInputFlowControl() const
{
  return RtsCts;
}


BOOL PSerialChannel::SetOutputFlowControl(FlowControl flowControl)
{
  return FALSE;
}


PSerialChannel::FlowControl PSerialChannel::GetOutputFlowControl() const
{
  return RtsCts;
}


void PSerialChannel::SetDTR(BOOL state)
{
  if (!IsOpen())
    return;

}


void PSerialChannel::SetRTS(BOOL state)
{
  if (!IsOpen())
    return;

}


void PSerialChannel::SetBreak(BOOL state)
{
  if (!IsOpen())
    return;

  int s = state;
}


BOOL PSerialChannel::GetCTS()
{
  if (!IsOpen())
    return FALSE;

  return FALSE;
}


BOOL PSerialChannel::GetDSR()
{
  if (!IsOpen())
    return FALSE;

  return FALSE;
}


BOOL PSerialChannel::GetDCD()
{
  if (!IsOpen())
    return FALSE;

  return FALSE;
}


BOOL PSerialChannel::GetRing()
{
  if (!IsOpen())
    return FALSE;

  return FALSE;
}


PStringList PSerialChannel::GetPortNames()
{
  PStringList ports;
  return ports;
}


///////////////////////////////////////////////////////////////////////////////
// PPipeChannel

void PPipeChannel::Construct(const PString & subProgram,
                const char * const * arguments, OpenMode mode, BOOL searchPath)
{
}


PPipeChannel::~PPipeChannel()
{
  Close();
}


BOOL PPipeChannel::Read(void * buffer, PINDEX amount)
{
  return FALSE;
}
      

BOOL PPipeChannel::Write(const void * buffer, PINDEX amount)
{
  return FALSE;
}


BOOL PPipeChannel::Close()
{
  return FALSE;
}


BOOL PPipeChannel::Execute()
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// Configuration files

void PConfig::Construct(Source src)
{
  switch (src) {
    case Application :
      PFilePath appFile = PProcess::Current()->GetFile();
  }
}


void PConfig::Construct(const PFilePath & file)
{
}


PStringList PConfig::GetSections()
{
  PStringList sections;

  return sections;
}


PStringList PConfig::GetKeys(const PString &) const
{
  PStringList keys;
  PAssertAlways(PUnimplementedFunction);

  return keys;
}


void PConfig::DeleteSection(const PString &)
{
  PAssertAlways(PUnimplementedFunction);
}


void PConfig::DeleteKey(const PString &, const PString & key)
{
  PAssertAlways(PUnimplementedFunction);
}


PString PConfig::GetString(const PString &,
                                          const PString & key, const PString & dflt)
{
  PString str;

  PAssertAlways(PUnimplementedFunction);
  return str;
}


void PConfig::SetString(const PString &, const PString & key, const PString & value)
{
  PAssertAlways(PUnimplementedFunction);
}



///////////////////////////////////////////////////////////////////////////////
// PThread

PThread::~PThread()
{
  Terminate();
  free(stackBase);   // Give stack back to the near heap
}


void PThread::Block(BlockFunction isBlockFun, PObject * obj)
{
  isBlocked = isBlockFun;
  blocker = obj;
  status = BlockedIO;
  Yield();
}


void PThread::SwitchContext(PThread * from)
{
  if (setjmp(from->context) != 0) // Are being reactivated from previous yield
    return;

  if (status == Starting) {
    if (setjmp(context) != 0) {
      status = Running;
      Main();
      Terminate(); // Never returns from here
    }
//    context[7] = (int)stackTop-16;  // Change the stack pointer in jmp_buf
  }

  longjmp(context, TRUE);
  PAssertAlways("longjmp failed"); // Should never get here
}



///////////////////////////////////////////////////////////////////////////////
// PProcess

void PProcess::OperatingSystemYield()
{
}


PString PProcess::GetUserName() const
{
  PAssertAlways(PUnimplementedFunction);
  return "";
}


int PProcess::_main(int argc, char ** argv, char **)
{
  PreInitialise(argc, argv);

  Main();

  return terminationValue;
}



///////////////////////////////////////////////////////////////////////////////
// PDynaLink

PDynaLink::PDynaLink()
{
  PAssertAlways(PUnimplementedFunction);
}


PDynaLink::PDynaLink(const PString &)
{
  PAssertAlways(PUnimplementedFunction);
}


PDynaLink::~PDynaLink()
{
}


BOOL PDynaLink::Open(const PString & name)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


void PDynaLink::Close()
{
}


BOOL PDynaLink::IsLoaded() const
{
  return FALSE;
}


BOOL PDynaLink::GetFunction(PINDEX index, Function & func)
{
  return FALSE;
}


BOOL PDynaLink::GetFunction(const PString & name, Function & func)
{
  return FALSE;
}



// End Of File ///////////////////////////////////////////////////////////////
