/*
 * $Id: osutil.inl,v 1.14 1994/06/25 11:55:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: osutil.inl,v $
 * Revision 1.14  1994/06/25 11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.13  1994/04/20  12:17:44  robertj
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

PINLINE long PTimeInterval::GetMilliseconds() const
  { return milliseconds; }

PINLINE long PTimeInterval::GetSeconds() const
  { return milliseconds/1000; }

PINLINE long PTimeInterval::GetMinutes() const
  { return milliseconds/60000; }

PINLINE int PTimeInterval::GetHours() const
  { return (int)(milliseconds/3600000); }

PINLINE int PTimeInterval::GetDays() const
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

PINLINE PTime::PTime(const PTime & t)
  : theTime(t.theTime) { }

PINLINE PTime & PTime::operator=(const PTime & t)
  { theTime = t.theTime; return *this; }

PINLINE PObject * PTime::Clone() const
  { return new PTime(theTime); }

PINLINE ostream & PTime::PrintOn(ostream & strm) const
  { return strm << AsString(); }

PINLINE int PTime::GetSecond() const
  { return localtime(&theTime)->tm_sec; }

PINLINE int PTime::GetMinute() const
  { return localtime(&theTime)->tm_min; }

PINLINE int PTime::GetHour() const
  { return localtime(&theTime)->tm_hour; }

PINLINE int PTime::GetDay() const
  { return localtime(&theTime)->tm_mday; }

PINLINE PTime::Months PTime::GetMonth() const
  { return (Months)(localtime(&theTime)->tm_mon+January); }

PINLINE int PTime::GetYear() const
  { return localtime(&theTime)->tm_year+1900; }

PINLINE PTime::Weekdays PTime::GetDayOfWeek() const
  { return (Weekdays)localtime(&theTime)->tm_wday; }

PINLINE int PTime::GetDayOfYear() const
  { return localtime(&theTime)->tm_yday; }

PINLINE BOOL PTime::IsDaylightSavings() const
  { return localtime(&theTime)->tm_isdst; }


PINLINE PTime PTime::operator+(const PTimeInterval & t) const
  { return PTime(theTime + t.GetSeconds()); }

PINLINE PTime & PTime::operator+=(const PTimeInterval & t)
  { theTime += t.GetSeconds(); return *this; }

PINLINE PTimeInterval PTime::operator-(const PTime & t) const
  { return PTimeInterval(0, (int)(theTime - t.theTime)); }

PINLINE PTime PTime::operator-(const PTimeInterval & t) const
  { return PTime(theTime - t.GetSeconds()); }

PINLINE PTime & PTime::operator-=(const PTimeInterval & t)
  { theTime -= t.GetSeconds(); return *this; }


///////////////////////////////////////////////////////////////////////////////
// PTimerList

PINLINE PTimerList::PTimerList()
  : PAbstractSortedList() { DisallowDeleteObjects(); }


///////////////////////////////////////////////////////////////////////////////
// PTimer

PINLINE BOOL PTimer::IsRunning() const
  { return state == Running; }

PINLINE BOOL PTimer::IsPaused() const
  { return state == Paused; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PChannelStreamBuffer::PChannelStreamBuffer(const PChannelStreamBuffer & sbuf)
  : channel(sbuf.channel) { setb(buffer, &buffer[sizeof(buffer)-1]); }

PINLINE PChannelStreamBuffer &
          PChannelStreamBuffer::operator=(const PChannelStreamBuffer & sbuf)
  { channel = sbuf.channel; return *this; }

PINLINE BOOL PChannel::SetSize(PINDEX newSize)
  { return newSize == 1; }

PINLINE void PChannel::SetReadTimeout(PTimeInterval time)
  { readTimeout = time; }

PINLINE PTimeInterval PChannel::GetReadTimeout() const
  { return readTimeout; }

PINLINE PINDEX PChannel::GetLastReadCount() const
  { return lastReadCount; }

PINLINE void PChannel::SetWriteTimeout(PTimeInterval time)
  { writeTimeout = time; }

PINLINE PTimeInterval PChannel::GetWriteTimeout() const
  { return writeTimeout; }

PINLINE PINDEX PChannel::GetLastWriteCount() const
  { return lastWriteCount; }

PINLINE BOOL PChannel::WriteString(const PString & str)
  { return Write((const char *)str, str.GetLength()); }

PINLINE PChannel::Errors PChannel::GetErrorCode() const
  { return lastError; }

PINLINE int PChannel::GetErrorNumber() const
  { return osError; }


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

PINLINE PString PFile::GetName() const
  { return path; }

PINLINE int PFile::GetHandle() const
  { PAssert(os_handle >= 0, PFileNotOpen); return os_handle; }

PINLINE off_t PFile::GetPosition() const
  { return _lseek(GetHandle(), 0, SEEK_CUR); }

PINLINE BOOL PFile::IsEndOfFile() const
  { rdbuf()->sync(); return GetPosition() >= GetLength(); }


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
// PSerialChannel

#ifdef _PSERIALCHANNEL

PINLINE void PSerialChannel::ClearDTR()
  { SetDTR(FALSE); }

PINLINE void PSerialChannel::ClearRTS()
  { SetRTS(FALSE); }

PINLINE void PSerialChannel::ClearBreak()
  { SetBreak(FALSE); }


#endif


///////////////////////////////////////////////////////////////////////////////
// PModem

#ifdef _PMODEM

PINLINE void PModem::SetInitString(const PString & str)
  { initCmd = str; }

PINLINE PString PModem::GetInitString() const
  { return initCmd; }

PINLINE void PModem::SetDeinitString(const PString & str)
  { deinitCmd = str; }

PINLINE PString PModem::GetDeinitString() const
  { return deinitCmd; }

PINLINE void PModem::SetPreDialString(const PString & str)
  { preDialCmd = str; }

PINLINE PString PModem::GetPreDialString() const
  { return preDialCmd; }

PINLINE void PModem::SetPostDialString(const PString & str)
  { postDialCmd = str; }

PINLINE PString PModem::GetPostDialString() const
  { return postDialCmd; }

PINLINE void PModem::SetBusyString(const PString & str)
  { busyReply = str; }

PINLINE PString PModem::GetBusyString() const
  { return busyReply; }

PINLINE void PModem::SetNoCarrierString(const PString & str)
  { noCarrierReply = str; }

PINLINE PString PModem::GetNoCarrierString() const
  { return noCarrierReply; }

PINLINE void PModem::SetConnectString(const PString & str)
  { connectReply = str; }

PINLINE PString PModem::GetConnectString() const
  { return connectReply; }

PINLINE void PModem::SetHangUpString(const PString & str)
  { hangUpCmd = str; }

PINLINE PString PModem::GetHangUpString() const
  { return hangUpCmd; }

PINLINE PModem::Status PModem::GetStatus() const
  { return status; }


#endif


///////////////////////////////////////////////////////////////////////////////
// PConfig

#ifdef _PCONFIG

PINLINE void PConfig::SetDefaultSection(const char * section)
  { defaultSection = section; }

PINLINE PString PConfig::GetDefaultSection() const
  { return defaultSection; }

PINLINE void PConfig::DeleteKey(const char * key)
  { DeleteKey(defaultSection, key); }

PINLINE PString PConfig::GetString(const char * key, const char * dflt)
  { return GetString(defaultSection, key, dflt); }

PINLINE void PConfig::SetString(const char * key, const char * value)
  { SetString(defaultSection, key, value); }

PINLINE BOOL PConfig::GetBoolean(const char * key, BOOL dflt)
  { return GetBoolean(defaultSection, key, dflt); }

PINLINE void PConfig::SetBoolean(const char * key, BOOL value)
  { SetBoolean(defaultSection, key, value); }

PINLINE long PConfig::GetInteger(const char * key, long dflt)
  { return GetInteger(defaultSection, key, dflt); }

PINLINE void PConfig::SetInteger(const char * key, long value)
  { SetInteger(defaultSection, key, value); }

PINLINE double PConfig::GetReal(const char * key, double dflt)
  { return GetReal(defaultSection, key, dflt); }

PINLINE void PConfig::SetReal(const char * key, double value)
  { SetReal(defaultSection, key, value); }


#endif


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
// PProcess

PINLINE PThread::~PThread()
  { Terminate(); }


///////////////////////////////////////////////////////////////////////////////
// PProcess

PINLINE PArgList & PProcess::GetArguments()
  { return arguments; }

PINLINE PString PProcess::GetName() const
  { return executableName; }

PINLINE const PFilePath & PProcess::GetFile() const
  { return executableFile; }

PINLINE PTimerList * PProcess::GetTimerList()
  { return &timers; }


// End Of File ///////////////////////////////////////////////////////////////
