/*
 * $Id: osutil.inl,v 1.4 1993/08/21 01:50:33 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: osutil.inl,v $
 * Revision 1.4  1993/08/21 01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


///////////////////////////////////////////////////////////////////////////////

inline PDirectory::PDirectory()
  : path(".") { Construct(); }
  
inline PDirectory::PDirectory(const PString & pathname)
  : path(pathname) { Construct(); }
  

inline PObject::Comparison PDirectory::Compare(const PObject & obj) const
  { return path.Compare(((const PDirectory &)obj).path); }

inline ostream & PDirectory::PrintOn(ostream & strm) const
  { return strm << path; }

inline istream & PDirectory::ReadFrom(istream & strm)
  { return strm >> path; }

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

inline PFile::PFile(const PString & name, OpenMode mode, int opts)
  : fullname(name) { Construct(); Open(mode, opts); }


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

inline BOOL PFile::Rename(const PString & newname)
  { if (!Rename(fullname, newname)) return FALSE;
    fullname = newname; return TRUE; }

inline BOOL PFile::GetStatus(Status & status) const
  { return GetStatus(fullname, status); }


inline PString PFile::GetFullName() const
  { return fullname; }
      


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
