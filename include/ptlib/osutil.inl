/*
 * $Id: osutil.inl,v 1.7 1993/08/31 03:38:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: osutil.inl,v $
 * Revision 1.7  1993/08/31 03:38:02  robertj
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

inline PTimeInterval::PTimeInterval(const PTimeInterval & ti)
  : milliseconds(ti.milliseconds) { }

inline PTimeInterval & PTimeInterval::operator=(const PTimeInterval & ti)
  { milliseconds = ti.milliseconds; return *this; }

inline PObject * PTimeInterval::Clone() const
  { return new PTimeInterval(milliseconds); }

inline long PTimeInterval::Milliseconds() const
  { return milliseconds; }

inline long PTimeInterval::Seconds() const
  { return milliseconds/1000; }

inline long PTimeInterval::Minutes() const
  { return milliseconds/60000; }

inline int PTimeInterval::Hours() const
  { return (int)(milliseconds/3600000); }

inline int PTimeInterval::Days() const
  { return (int)(milliseconds/86400000); }


inline PTimeInterval PTimeInterval::operator+(const PTimeInterval & t) const
  { return PTimeInterval(milliseconds + t.milliseconds); }

inline PTimeInterval & PTimeInterval::operator+=(const PTimeInterval & t)
  { milliseconds += t.milliseconds; return *this; }

inline PTimeInterval PTimeInterval::operator-(const PTimeInterval & t) const
  { return PTimeInterval(milliseconds - t.milliseconds); }

inline PTimeInterval & PTimeInterval::operator-=(const PTimeInterval & t)
  { milliseconds -= t.milliseconds; return *this; }



///////////////////////////////////////////////////////////////////////////////
// PTime

inline PTime::PTime(time_t t)
  : theTime(t) { }

inline PObject * PTime::Clone() const
  { return new PTime(theTime); }

inline ostream & PTime::PrintOn(ostream & strm) const
  { return strm << ctime(&theTime); }

inline int PTime::GetSecond() const
  { return localtime(&theTime)->tm_sec; }

inline int PTime::GetMinute() const
  { return localtime(&theTime)->tm_min; }

inline int PTime::GetHour() const
  { return localtime(&theTime)->tm_hour; }

inline int PTime::GetDay() const
  { return localtime(&theTime)->tm_mday; }

inline int PTime::GetMonth() const
  { return localtime(&theTime)->tm_mon+1; }

inline int PTime::GetYear() const
  { return localtime(&theTime)->tm_year+1900; }

inline int PTime::GetDayOfWeek() const
  { return localtime(&theTime)->tm_wday; }

inline int PTime::GetDayOfYear() const
  { return localtime(&theTime)->tm_yday; }

inline BOOL PTime::IsDaylightSavings() const
  { return localtime(&theTime)->tm_isdst; }


inline PTime PTime::operator+(const PTimeInterval & t) const
  { return PTime(theTime + t.Seconds()); }

inline PTime & PTime::operator+=(const PTimeInterval & t)
  { theTime += t.Seconds(); return *this; }

inline PTimeInterval PTime::operator-(const PTime & t) const
  { return PTimeInterval(0, (int)(theTime - t.theTime)); }

inline PTime PTime::operator-(const PTimeInterval & t) const
  { return PTime(theTime - t.Seconds()); }

inline PTime & PTime::operator-=(const PTimeInterval & t)
  { theTime -= t.Seconds(); return *this; }


///////////////////////////////////////////////////////////////////////////////
// PDirectory

inline PDirectory::PDirectory()
  : path(".") { Construct(); }
  
inline PDirectory::PDirectory(const PString & pathname)
  : path(pathname) { Construct(); }
  

inline PObject::Comparison PDirectory::Compare(const PObject & obj) const
  { return path.Compare(((const PDirectory &)obj).path); }

inline ostream & PDirectory::PrintOn(ostream & strm) const
  { return strm << path; }

inline BOOL PDirectory::SetSize(PINDEX newSize)
  { return newSize == 1; }

inline void PDirectory::DestroyContents()
  { Close(); }


inline PString PDirectory::GetPath() const
  { return path; }

inline BOOL PDirectory::Change() const
  { return Change(path); }

inline BOOL PDirectory::Create(int perm) const
  { return Create(path, perm); }

inline BOOL PDirectory::Remove() const
  { return Remove(path); }


inline PDirectory::~PDirectory()
  { DestroyContents(); }


///////////////////////////////////////////////////////////////////////////////

inline PFile::PFile(const PString & name)
  : fullname(name) { Construct(); }

inline PObject::Comparison PFile::Compare(const PObject & obj) const
  { return fullname.Compare(((const PFile &)obj).fullname); }

inline BOOL PFile::SetSize(PINDEX newSize)
  { return newSize == 1; }

inline void PFile::DestroyContents()
  { Close(); }

inline PFile::~PFile()
  { DestroyContents(); }


inline BOOL PFile::Exists() const
  { return Exists(fullname); }

inline BOOL PFile::Access(OpenMode mode) const
  { return Access(fullname, mode); }

inline BOOL PFile::Remove() const
  { return Remove(fullname); }

inline BOOL PFile::Copy(const PString & newname)
  { return Copy(fullname, newname); }

inline BOOL PFile::GetInfo(Info & info) const
  { return GetInfo(fullname, info); }


inline PString PFile::GetFullName() const
  { return fullname; }
      

inline BOOL PFile::IsOpen()
  { return os_handle >= 0; }

inline int PFile::GetHandle() const
  { PAssert(os_handle >= 0); return os_handle; }

inline off_t PFile::GetPosition()
  { return lseek(GetHandle(), 0, SEEK_CUR); }

inline BOOL PFile::SetPosition(long pos, FilePositionOrigin origin)
  { return lseek(GetHandle(), pos, origin) == pos; }



///////////////////////////////////////////////////////////////////////////////


inline PTextFile::PTextFile()
  : PFile() { }

inline PTextFile::PTextFile(const PString & name)
  : PFile(name) { }

inline PTextFile::PTextFile(const PString & name, OpenMode mode, int opts)
  : PFile(name, mode, opts) { }


///////////////////////////////////////////////////////////////////////////////


inline PStructuredFile::PStructuredFile()
  : PFile(), structureSize(1) { }

inline PStructuredFile::PStructuredFile(const PString & name)
  : PFile(name), structureSize(1) { }

inline PStructuredFile::PStructuredFile(const PString & name,
                                                      OpenMode mode, int opts)
  : PFile(name, mode, opts), structureSize(1) { }


inline BOOL PStructuredFile::Read(void * buffer)
  { return PFile::Read(buffer, structureSize); }
      
inline BOOL PStructuredFile::Write(void * buffer)
  { return PFile::Write(buffer, structureSize); }

inline size_t PStructuredFile::GetStructureSize()
  { return structureSize; }

inline void PStructuredFile::SetStructureSize(size_t newSize)
  { structureSize = newSize; }



// End Of File ///////////////////////////////////////////////////////////////
