/*
 * $Id: osutil.inl,v 1.13 1994/04/20 12:17:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: osutil.inl,v $
 * Revision 1.13  1994/04/20 12:17:44  robertj
 * assert stuff
 *
 * Revision 1.12  1994/04/01  14:06:48  robertj
 * Text file streams.
 *
 * Revision 1.11  1994/03/07  07:45:40  robertj
 * Major upgrade
 *
 * Revision 1.10  1994/01/13  03:14:51  robertj
 * Added AsString() function to convert a time to a string.
 *
 * Revision 1.9  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.8  1993/12/31  06:47:59  robertj
 * Made inlines optional for debugging purposes.
 *
 * Revision 1.7  1993/08/31  03:38:02  robertj
 * Changed PFile::Status to PFile::Info due to X-Windows compatibility.
 * Added copy constructor and assignement operator due to G++ wierdness.
 *
 * Revision 1.6  1993/08/27  18:17:47  robertj
 * Moved a lot of code from MS-DOS platform specific to common files.
 *
 * Revision 1.5  1993/08/21  04:40:19  robertj
 * Added Copy() function.
 *
 * Revision 1.4  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */



///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

PINLINE PTimeInterval::PTimeInterval(const PTimeInterval & ti)
  : milliseconds(ti.milliseconds) { }

PINLINE PTimeInterval & PTimeInterval::operator=(const PTimeInterval & ti)
  { milliseconds = ti.milliseconds; return *this; }

PINLINE PObject * PTimeInterval::Clone() const
  { return new PTimeInterval(milliseconds); }

PINLINE long PTimeInterval::Milliseconds() const
  { return milliseconds; }

PINLINE long PTimeInterval::Seconds() const
  { return milliseconds/1000; }

PINLINE long PTimeInterval::Minutes() const
  { return milliseconds/60000; }

PINLINE int PTimeInterval::Hours() const
  { return (int)(milliseconds/3600000); }

PINLINE int PTimeInterval::Days() const
  { return (int)(milliseconds/86400000); }


PINLINE PTimeInterval PTimeInterval::operator+(const PTimeInterval & t) const
  { return PTimeInterval(milliseconds + t.milliseconds); }

PINLINE PTimeInterval & PTimeInterval::operator+=(const PTimeInterval & t)
  { milliseconds += t.milliseconds; return *this; }

PINLINE PTimeInterval PTimeInterval::operator-(const PTimeInterval & t) const
  { return PTimeInterval(milliseconds - t.milliseconds); }

PINLINE PTimeInterval & PTimeInterval::operator-=(const PTimeInterval & t)
  { milliseconds -= t.milliseconds; return *this; }



///////////////////////////////////////////////////////////////////////////////
// PTime

PINLINE PTime::PTime(time_t t)
  : theTime(t) { }

PINLINE PObject * PTime::Clone() const
  { return new PTime(theTime); }

PINLINE ostream & PTime::PrintOn(ostream & strm) const
  { return strm << AsString(); }

PINLINE PString PTime::AsString() const
  { return ctime(&theTime); }

PINLINE int PTime::GetSecond() const
  { return localtime(&theTime)->tm_sec; }

PINLINE int PTime::GetMinute() const
  { return localtime(&theTime)->tm_min; }

PINLINE int PTime::GetHour() const
  { return localtime(&theTime)->tm_hour; }

PINLINE int PTime::GetDay() const
  { return localtime(&theTime)->tm_mday; }

PINLINE int PTime::GetMonth() const
  { return localtime(&theTime)->tm_mon+1; }

PINLINE int PTime::GetYear() const
  { return localtime(&theTime)->tm_year+1900; }

PINLINE int PTime::GetDayOfWeek() const
  { return localtime(&theTime)->tm_wday; }

PINLINE int PTime::GetDayOfYear() const
  { return localtime(&theTime)->tm_yday; }

PINLINE BOOL PTime::IsDaylightSavings() const
  { return localtime(&theTime)->tm_isdst; }


PINLINE PTime PTime::operator+(const PTimeInterval & t) const
  { return PTime(theTime + t.Seconds()); }

PINLINE PTime & PTime::operator+=(const PTimeInterval & t)
  { theTime += t.Seconds(); return *this; }

PINLINE PTimeInterval PTime::operator-(const PTime & t) const
  { return PTimeInterval(0, (int)(theTime - t.theTime)); }

PINLINE PTime PTime::operator-(const PTimeInterval & t) const
  { return PTime(theTime - t.Seconds()); }

PINLINE PTime & PTime::operator-=(const PTimeInterval & t)
  { theTime -= t.Seconds(); return *this; }


///////////////////////////////////////////////////////////////////////////////
// PTimerList

PINLINE PTimerList::PTimerList()
  : PAbstractSortedList() { DisallowDeleteObjects(); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PChannelStreamBuffer::PChannelStreamBuffer(const PChannelStreamBuffer & sbuf)
  : channel(sbuf.channel) { setb(buffer, &buffer[sizeof(buffer)-1]); }

PINLINE PChannelStreamBuffer &
          PChannelStreamBuffer::operator=(const PChannelStreamBuffer & sbuf)
  { channel = sbuf.channel; return *this; }


///////////////////////////////////////////////////////////////////////////////
// PDirectory

PINLINE PDirectory::PDirectory()
  : path(".") { Construct(); }
  
PINLINE PDirectory::PDirectory(const PString & pathname)
  : path(pathname) { Construct(); }
  

PINLINE PObject::Comparison PDirectory::Compare(const PObject & obj) const
  { return path.Compare(((const PDirectory &)obj).path); }

PINLINE ostream & PDirectory::PrintOn(ostream & strm) const
  { return strm << path; }

PINLINE BOOL PDirectory::SetSize(PINDEX newSize)
  { return newSize == 1; }

PINLINE void PDirectory::DestroyContents()
  { Close(); }

PINLINE void PDirectory::CloneContents(const PDirectory * d)
  { CopyContents(*d); }

PINLINE PString PDirectory::GetPath() const
  { return path; }

PINLINE BOOL PDirectory::Change() const
  { return Change(path); }

PINLINE BOOL PDirectory::Create(int perm) const
  { return Create(path, perm); }

PINLINE BOOL PDirectory::Remove()
  { Close(); return Remove(path); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PFilePath::PFilePath()
  { }


///////////////////////////////////////////////////////////////////////////////

PINLINE PFile::PFile()
  { os_handle = -1; }

PINLINE PFile::PFile(OpenMode mode, int opts)
  { os_handle = -1; Open(mode, opts); }

PINLINE PFile::PFile(const PFilePath & name, OpenMode mode, int opts)
  { os_handle = -1; Open(name, mode, opts); }

PINLINE PObject::Comparison PFile::Compare(const PObject & obj) const
  { return path.Compare(((const PFile &)obj).path); }

PINLINE BOOL PFile::SetSize(PINDEX newSize)
  { return newSize == 1; }

PINLINE void PFile::DestroyContents()
  { Close(); }

PINLINE void PFile::CloneContents(const PFile * f)
  { CopyContents(*f); }

PINLINE BOOL PFile::Exists() const
  { return Exists(path); }

PINLINE BOOL PFile::Access(OpenMode mode) const
  { return Access(path, mode); }

PINLINE BOOL PFile::Remove()
  { Close(); return Remove(path); }

PINLINE BOOL PFile::Copy(const PString & newname)
  { return Copy(path, newname); }

PINLINE BOOL PFile::GetInfo(PFileInfo & info) const
  { return GetInfo(path, info); }


PINLINE const PFilePath & PFile::GetFilePath() const
  { return path; }
      

PINLINE BOOL PFile::IsOpen() const
  { return os_handle >= 0; }

PINLINE int PFile::GetHandle() const
  { PAssert(os_handle >= 0, PFileNotOpen); return os_handle; }

PINLINE BOOL PFile::WriteChar(char c)
  { return Write(&c, 1); }

PINLINE off_t PFile::GetPosition() const
  { return lseek(GetHandle(), 0, SEEK_CUR); }

PINLINE BOOL PFile::IsEndOfFile() const
  { return GetPosition() >= GetLength(); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PTextFile::PTextFile()
  { }

PINLINE PTextFile::PTextFile(OpenMode mode, int opts)
  : PFile(mode, opts) { }

PINLINE PTextFile::PTextFile(const PFilePath & name, OpenMode mode, int opts)
  : PFile(name, mode, opts) { }

PINLINE void PTextFile::DestroyContents()
  { PFile::DestroyContents(); }

PINLINE void PTextFile::CloneContents(const PTextFile *)
  { }

PINLINE void PTextFile::CopyContents(const PTextFile &)
  { }


///////////////////////////////////////////////////////////////////////////////


PINLINE PStructuredFile::PStructuredFile()
  { structureSize = 1; }

PINLINE PStructuredFile::PStructuredFile(OpenMode mode, int opts)
  : PFile(mode, opts) { structureSize = 1; }

PINLINE PStructuredFile::PStructuredFile(const PFilePath & name,
                                                      OpenMode mode, int opts)
  : PFile(name, mode, opts) { structureSize = 1; }


PINLINE size_t PStructuredFile::GetStructureSize()
  { return structureSize; }

PINLINE void PStructuredFile::SetStructureSize(size_t newSize)
  { structureSize = newSize; }


///////////////////////////////////////////////////////////////////////////////
// PArgList

PINLINE BOOL PArgList::HasOption(char option) const
  { return GetOptionCount(option) != 0; }

PINLINE BOOL PArgList::HasOption(const char * option) const
  { return GetOptionCount(option) != 0; }

PINLINE PINDEX PArgList::GetCount() const
  { return arg_count-shift; }

PINLINE PString PArgList::operator[](PINDEX num) const
  { return GetParameter(num); }

PINLINE void PArgList::operator<<(int sh)
  { Shift(sh); }

PINLINE void PArgList::operator>>(int sh)
  { Shift(-sh); }


///////////////////////////////////////////////////////////////////////////////
// PTextApplication

PINLINE const PArgList & PTextApplication::GetArguments() const
  { return arguments; }

PINLINE PString PTextApplication::GetAppName() const
  { return applicationName; }

PINLINE const PFilePath & PTextApplication::GetAppFile() const
  { return applicationFile; }

PINLINE void PTextApplication::AddTimer(PTimer * timer)
  { timers.Append(timer); }

PINLINE void PTextApplication::RemoveTimer(PTimer * timer)
  { timers.Remove(timer); }


// End Of File ///////////////////////////////////////////////////////////////
