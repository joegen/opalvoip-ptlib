/*
 * $Id: osutil.inl,v 1.32 1995/12/10 11:32:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: osutil.inl,v $
 * Revision 1.32  1995/12/10 11:32:44  robertj
 * Added extra user information to processes and applications.
 *
 * Revision 1.31  1995/08/12 22:30:05  robertj
 * Work around for  GNU bug: can't have private copy constructor with multiple inheritance.
 *
 * Revision 1.30  1995/07/31 12:15:44  robertj
 * Removed PContainer from PChannel ancestor.
 *
 * Revision 1.29  1995/04/22 00:49:19  robertj
 * Fixed missing common construct code call in edit box constructor.
 *
 * Revision 1.28  1995/03/12 04:41:16  robertj
 * Moved GetHandle() function from PFile to PChannel.
 *
 * Revision 1.27  1995/01/27  11:11:19  robertj
 * Changed single string default constructor to be section name not file name.
 *
 * Revision 1.26  1995/01/18  09:00:40  robertj
 * Added notifiers to timers.
 *
 * Revision 1.25  1995/01/15  04:51:09  robertj
 * Mac compatibility.
 * Added structure function to structured files.
 *
 * Revision 1.24  1995/01/11  09:45:02  robertj
 * Documentation and normalisation.
 *
 * Revision 1.23  1995/01/09  12:34:25  robertj
 * Removed unnecesary return value from I/O functions.
 *
 * Revision 1.22  1994/10/24  00:07:01  robertj
 * Changed PFilePath and PDirectory so descends from either PString or
 *     PCaselessString depending on the platform.
 *
 * Revision 1.21  1994/10/23  04:49:00  robertj
 * Chnaged PDirectory to descend of PString.
 * Added PDirectory Exists() function.
 * Implemented PPipeChannel.
 *
 * Revision 1.20  1994/09/25  10:41:19  robertj
 * Moved PFile::DestroyContents() to cxx file.
 * Added PTextFile constructors for DOS/NT platforms.
 * Added Pipe channel.
 *
 * Revision 1.19  1994/08/21  23:43:02  robertj
 * Added "remove on close" feature for temporary files.
 * Added "force" option to Remove/Rename etc to override write protection.
 * Removed default argument when of PString type (MSC crashes).
 *
 * Revision 1.18  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.17  1994/07/21  12:33:49  robertj
 * Moved cooperative threads to common.
 *
 * Revision 1.16  1994/07/17  10:46:06  robertj
 * Moved file handle to PChannel.
 *
 * Revision 1.15  1994/07/02  03:03:49  robertj
 * Time interval and timer redesign.
 *
 * Revision 1.14  1994/06/25  11:55:15  robertj
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

PINLINE PObject * PTimeInterval::Clone() const
  { return PNEW PTimeInterval(milliseconds); }

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


PINLINE BOOL PTimeInterval::operator==(const PTimeInterval & t) const
  { return milliseconds == t.milliseconds; }

PINLINE BOOL PTimeInterval::operator!=(const PTimeInterval & t) const
  { return milliseconds != t.milliseconds; }

PINLINE BOOL PTimeInterval::operator> (const PTimeInterval & t) const
  { return milliseconds > t.milliseconds; }

PINLINE BOOL PTimeInterval::operator>=(const PTimeInterval & t) const
  { return milliseconds >= t.milliseconds; }

PINLINE BOOL PTimeInterval::operator< (const PTimeInterval & t) const
  { return milliseconds < t.milliseconds; }

PINLINE BOOL PTimeInterval::operator<=(const PTimeInterval & t) const
  { return milliseconds <= t.milliseconds; }

PINLINE BOOL PTimeInterval::operator==(long msecs) const
  { return milliseconds == msecs; }

PINLINE BOOL PTimeInterval::operator!=(long msecs) const
  { return milliseconds != msecs; }

PINLINE BOOL PTimeInterval::operator> (long msecs) const
  { return milliseconds > msecs; }

PINLINE BOOL PTimeInterval::operator>=(long msecs) const
  { return milliseconds >= msecs; }

PINLINE BOOL PTimeInterval::operator< (long msecs) const
  { return milliseconds < msecs; }

PINLINE BOOL PTimeInterval::operator<=(long msecs) const
  { return milliseconds <= msecs; }


///////////////////////////////////////////////////////////////////////////////
// PTime

PINLINE PTime::PTime(time_t t)
  : theTime(t) { }

PINLINE PObject * PTime::Clone() const
  { return PNEW PTime(theTime); }

PINLINE void PTime::PrintOn(ostream & strm) const
  { strm << AsString(ShortDateTime); }

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

PINLINE PString PTime::AsString(const PString & format) const
  { return AsString((const char *)format); }


///////////////////////////////////////////////////////////////////////////////
// PTimerList

PINLINE PTimerList::PTimerList()
  : PAbstractList() { DisallowDeleteObjects(); }


///////////////////////////////////////////////////////////////////////////////
// PTimer

PINLINE BOOL PTimer::IsRunning() const
  { return state == Running; }

PINLINE BOOL PTimer::IsPaused() const
  { return state == Paused; }

PINLINE const PNotifier & PTimer::GetNotifier() const
  { return callback; }

PINLINE void PTimer::SetNotifier(const PNotifier & func)
  { callback = func; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PChannelStreamBuffer::PChannelStreamBuffer(const PChannelStreamBuffer & sbuf)
  : channel(sbuf.channel) { }

PINLINE PChannelStreamBuffer &
          PChannelStreamBuffer::operator=(const PChannelStreamBuffer & sbuf)
  { channel = sbuf.channel; return *this; }

PINLINE PChannel::PChannel(const PChannel &)
  { PAssertAlways("Cannot copy channels"); }

PINLINE PChannel & PChannel::operator=(const PChannel &)
  { PAssertAlways("Cannot assign channels"); return *this; }

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

PINLINE int PChannel::GetHandle() const
  { PAssert(os_handle >= 0, PChannelNotOpen); return os_handle; }

PINLINE PChannel::Errors PChannel::GetErrorCode() const
  { return lastError; }

PINLINE int PChannel::GetErrorNumber() const
  { return osError; }

PINLINE void PChannel::AbortCommandString()
  { abortCommandString = TRUE; }


///////////////////////////////////////////////////////////////////////////////
// PDirectory

PINLINE PDirectory::PDirectory()
  : PFILE_PATH_STRING(".") { Construct(); }
  
PINLINE PDirectory::PDirectory(const PString & pathname)
  : PFILE_PATH_STRING(pathname) { Construct(); }
  

PINLINE void PDirectory::DestroyContents()
  { Close(); PFILE_PATH_STRING::DestroyContents(); }

PINLINE void PDirectory::CloneContents(const PDirectory * d)
  { CopyContents(*d); }

PINLINE BOOL PDirectory::Exists() const
  { return Exists(*this); }

PINLINE BOOL PDirectory::Change() const
  { return Change(*this); }

PINLINE BOOL PDirectory::Create(int perm) const
  { return Create(*this, perm); }

PINLINE BOOL PDirectory::Remove()
  { Close(); return Remove(*this); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PFilePath::PFilePath()
  { }

PINLINE PFilePath::PFilePath(const PFilePath & path)
  : PFILE_PATH_STRING(path) { }

PINLINE PFilePath & PFilePath::operator=(const char * cstr)
  { return operator=(PString(cstr)); }

PINLINE PFilePath & PFilePath::operator=(const PFilePath & path)
  { PFILE_PATH_STRING::operator=(path); return *this; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PFile::PFile()
  { os_handle = -1; removeOnClose = FALSE; }

PINLINE PFile::PFile(OpenMode mode, int opts)
  { os_handle = -1; removeOnClose = FALSE; Open(mode, opts); }

PINLINE PFile::PFile(const PFilePath & name, OpenMode mode, int opts)
  { os_handle = -1; removeOnClose = FALSE; Open(name, mode, opts); }

PINLINE PObject::Comparison PFile::Compare(const PObject & obj) const
  { return path.Compare(((const PFile &)obj).path); }

PINLINE BOOL PFile::Exists() const
  { return Exists(path); }

PINLINE BOOL PFile::Access(OpenMode mode)
  { return ConvertOSError(Access(path, mode) ? 0 : -1); }

PINLINE BOOL PFile::Remove(BOOL force)
  { Close(); return ConvertOSError(Remove(path, force) ? 0 : -1); }

PINLINE BOOL PFile::Copy(const PFilePath & newname, BOOL force)
  { return ConvertOSError(Copy(path, newname, force) ? 0 : -1); }

PINLINE BOOL PFile::GetInfo(PFileInfo & info)
  { return ConvertOSError(GetInfo(path, info) ? 0 : -1); }

PINLINE BOOL PFile::SetPermissions(int permissions)
  { return ConvertOSError(SetPermissions(path, permissions) ? 0 : -1); }


PINLINE const PFilePath & PFile::GetFilePath() const
  { return path; }
      

PINLINE PString PFile::GetName() const
  { return path; }

PINLINE off_t PFile::GetPosition() const
  { return _lseek(GetHandle(), 0, SEEK_CUR); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PTextFile::PTextFile()
  { }

PINLINE PTextFile::PTextFile(OpenMode mode, int opts)
  { Open(mode, opts); }

PINLINE PTextFile::PTextFile(const PFilePath & name, OpenMode mode, int opts)
  { Open(name, mode, opts); }


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

PINLINE void PStructuredFile::SetStructure(Element * struc, PINDEX num)
  { structure = struc; numElements = num; }


///////////////////////////////////////////////////////////////////////////////
// PPipeChannel

PINLINE PPipeChannel::PPipeChannel(const PString & subProgram,
                                                OpenMode mode, BOOL searchPath)
  { Construct(subProgram, NULL, mode, searchPath); }

PINLINE PPipeChannel::PPipeChannel(const PString & subProgram,
                const char * const * arguments, OpenMode mode, BOOL searchPath)
  { Construct(subProgram, arguments, mode, searchPath); }


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

PINLINE PConfig::PConfig(Source src)
  : defaultSection("Options") { Construct(src); }

PINLINE PConfig::PConfig(const PString & section, Source src)
  : defaultSection(section) { Construct(src); }

PINLINE PConfig::PConfig(const PFilePath & filename, const PString & section)
  : defaultSection(section) { Construct(filename); }

PINLINE void PConfig::SetDefaultSection(const PString & section)
  { defaultSection = section; }

PINLINE PString PConfig::GetDefaultSection() const
  { return defaultSection; }

PINLINE PStringList PConfig::GetKeys() const
  { return GetKeys(defaultSection); }

PINLINE void PConfig::DeleteSection()
  { DeleteSection(defaultSection); }

PINLINE void PConfig::DeleteKey(const PString & key)
  { DeleteKey(defaultSection, key); }

PINLINE PString PConfig::GetString(const PString & key)
  { return GetString(defaultSection, key, PString()); }

PINLINE PString PConfig::GetString(const PString & key, const PString & dflt)
  { return GetString(defaultSection, key, dflt); }

PINLINE void PConfig::SetString(const PString & key, const PString & value)
  { SetString(defaultSection, key, value); }

PINLINE BOOL PConfig::GetBoolean(const PString & key, BOOL dflt)
  { return GetBoolean(defaultSection, key, dflt); }

PINLINE void PConfig::SetBoolean(const PString & key, BOOL value)
  { SetBoolean(defaultSection, key, value); }

PINLINE long PConfig::GetInteger(const PString & key, long dflt)
  { return GetInteger(defaultSection, key, dflt); }

PINLINE void PConfig::SetInteger(const PString & key, long value)
  { SetInteger(defaultSection, key, value); }

PINLINE double PConfig::GetReal(const PString & key, double dflt)
  { return GetReal(defaultSection, key, dflt); }

PINLINE void PConfig::SetReal(const PString & key, double value)
  { SetReal(defaultSection, key, value); }


#endif


///////////////////////////////////////////////////////////////////////////////
// PArgList

PINLINE void PArgList::Parse(const PString & theArgumentSpec)
  { Parse((const char *)theArgumentSpec); }

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
// PThread

#ifndef P_PLATFORM_HAS_THREADS

PINLINE PThread * PThread::Current()
  { return PProcess::Current()->currentThread; }

PINLINE BOOL PThread::IsTerminated() const
  { return status == Terminated; }

PINLINE void PThread::Resume()
  { Suspend(FALSE); }

PINLINE BOOL PThread::IsSuspended() const
  { return suspendCount > 0; }

PINLINE void PThread::SetPriority(Priority priorityLevel)
  { basePriority = priorityLevel; }

PINLINE PThread::Priority PThread::GetPriority() const
  { return basePriority; }

#endif


///////////////////////////////////////////////////////////////////////////////
// PProcess

PINLINE PArgList & PProcess::GetArguments()
  { return arguments; }

PINLINE const PString & PProcess::GetManufacturer() const
  { return manufacturer; }

PINLINE const PString & PProcess::GetName() const
  { return productName; }

PINLINE const PString & PProcess::GetVersion() const
  { return version; }

PINLINE const PFilePath & PProcess::GetFile() const
  { return executableFile; }

PINLINE PTimerList * PProcess::GetTimerList()
  { return &timers; }

PINLINE void PProcess::SetTerminationValue(int value)
  { terminationValue = value; }

PINLINE int PProcess::GetTerminationValue() const
  { return terminationValue; }



// End Of File ///////////////////////////////////////////////////////////////
