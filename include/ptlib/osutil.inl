/*
 * OSUTIL.INL
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 */


///////////////////////////////////////////////////////////////////////////////

inline PDirectory::PDirectory()
  : path(".") { Construct(); }
  
inline PDirectory::PDirectory(const PString & pathname)
  : path(pathname) { Construct(); }
  

inline PObject * PDirectory::Clone() const
  { return new PDirectory(path); }

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

inline BOOL PDirectory::Change()
  { return Change(path); }

inline BOOL PDirectory::Create(PPermissions perm)
  { return Create(path, perm); }

inline BOOL PDirectory::Remove()
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


inline BOOL PFile::Exists()
  { return Exists(fullname); }

inline BOOL PFile::Access(OpenMode mode)
  { return Access(fullname, mode); }

inline BOOL PFile::Remove()
  { return Remove(fullname); }

inline BOOL PFile::Rename(const PString & newname)
  { return Rename(fullname, newname); }

inline BOOL PFile::GetStatus(PStatus & status)
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
