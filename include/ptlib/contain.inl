/*
 * $Id: contain.inl,v 1.4 1993/07/16 14:40:55 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: contain.inl,v $
 * Revision 1.4  1993/07/16 14:40:55  robertj
 * Added PString constructor for individual characters.
 * Added string to C style literal format.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


///////////////////////////////////////////////////////////////////////////////

inline BOOL PObject::operator==(const PObject & obj) const
  { return Compare(obj) == EqualTo; }

inline BOOL PObject::operator!=(const PObject & obj) const
  { return Compare(obj) != EqualTo; }

inline BOOL PObject::operator<(const PObject & obj) const
  { return Compare(obj) == LessThan; }

inline BOOL PObject::operator>(const PObject & obj) const
  { return Compare(obj) == GreaterThan; }

inline BOOL PObject::operator<=(const PObject & obj) const
  { return Compare(obj) != GreaterThan; }

inline BOOL PObject::operator>=(const PObject & obj) const
  { return Compare(obj) != LessThan; }

inline ostream & operator<<(ostream &strm, const PObject & obj)
  { return obj.PrintOn(strm); }

inline istream & operator>>(istream &strm, PObject & obj)
  { return obj.ReadFrom(strm); }



///////////////////////////////////////////////////////////////////////////////

inline PContainer::PContainer(PINDEX initialSize )
  : size(initialSize), referenceCount(new unsigned(1)) { }


///////////////////////////////////////////////////////////////////////////////

inline PAbstractArray::~PAbstractArray()
  { DestroyContents(); }

inline BOOL PAbstractArray::MakeUnique()
  { return *referenceCount == 1 || SetSize(size); }


///////////////////////////////////////////////////////////////////////////////

inline PString::PString()
  : PCharArray(1) { }

inline PString::PString(const PString & str)
  : PCharArray(str) { }

inline PString::PString(const char * cstr)
  : PCharArray(PAssertNULL(cstr), strlen(cstr)+1) { }

inline PString::PString(const char * cstr, PINDEX len)
  : PCharArray(len+1) { memcpy(theArray, PAssertNULL(cstr), len); }

inline PString::PString(char c)
  : PCharArray(2) { *theArray = c; }

inline PObject::Comparison PString::CompareString(const char * cstr) const
  { return (Comparison)strcmp(theArray,PAssertNULL(cstr)); }

inline BOOL PString::MakeMinimumSize()
  { return SetSize(strlen(theArray)+1); }

inline PINDEX PString::Length() const
  { return strlen(theArray); }

inline PString & PString::operator=(const PString & str)
  { PCharArray::operator=(str); return *this; }

inline PString PString::operator+(const PString & str) const
  { return operator+((const char *)str); }

inline PString operator+(const char * cstr, const PString & str)
  { return PString(cstr) + str; }
  
inline PString & PString::operator+=(const PString & str)
  { return operator+=((const char *)str); }

inline BOOL PString::operator==(const PObject & obj) const
  { return PObject::operator==(obj); }

inline BOOL PString::operator!=(const PObject & obj) const
  { return PObject::operator!=(obj); }

inline BOOL PString::operator<(const PObject & obj) const
  { return PObject::operator<(obj); }

inline BOOL PString::operator>(const PObject & obj) const
  { return PObject::operator>(obj); }

inline BOOL PString::operator<=(const PObject & obj) const
  { return PObject::operator<=(obj); }

inline BOOL PString::operator>=(const PObject & obj) const
  { return PObject::operator>=(obj); }

inline BOOL PString::operator==(const char * cstr) const
  { return CompareString(cstr) == EqualTo; }

inline BOOL PString::operator!=(const char * cstr) const
  { return CompareString(cstr) != EqualTo; }

inline BOOL PString::operator<(const char * cstr) const
  { return CompareString(cstr) == LessThan; }

inline BOOL PString::operator>(const char * cstr) const
  { return CompareString(cstr) == GreaterThan; }

inline BOOL PString::operator<=(const char * cstr) const
  { return CompareString(cstr) != GreaterThan; }

inline BOOL PString::operator>=(const char * cstr) const
  { return CompareString(cstr) != LessThan; }

inline PString::operator const unsigned char *() const
  { return (const unsigned char *)theArray; }
  

///////////////////////////////////////////////////////////////////////////////

inline PCaselessString::PCaselessString()
  : PString() { }

inline PCaselessString::PCaselessString(const char * cstr)
  : PString(cstr) { }

inline PCaselessString::PCaselessString(const PString & str)
  : PString(str) { }

inline PCaselessString & PCaselessString::operator=(const PString & str)
  { PString::operator=(str); return *this; }


///////////////////////////////////////////////////////////////////////////////

inline PCollection::PCollection(PINDEX initialSize)
  : PContainer(initialSize), deleteObjects(TRUE) { }

inline PCollection::PCollection(const PCollection & coll)
  : PContainer(coll), deleteObjects(coll.deleteObjects) { }

inline PCollection & PCollection::operator=(const PCollection & coll)
  { PContainer::operator=(coll);
                            deleteObjects = coll.deleteObjects; return *this; }

inline void PCollection::DeleteObjects(BOOL yes)
  { deleteObjects = yes; }

inline void PCollection::NoDeleteObjects()
  { DeleteObjects(FALSE); }


///////////////////////////////////////////////////////////////////////////////

inline PArrayObjects::PArrayObjects(PINDEX initialSize)
  : theArray(initialSize) { }

inline PArrayObjects::PArrayObjects(const PArrayObjects & arr)
  : PCollection(arr), theArray(arr.theArray) { }

inline PArrayObjects::~PArrayObjects()
  { DestroyContents(); }


///////////////////////////////////////////////////////////////////////////////

inline PAbstractList::PAbstractList()
  : PCollection(0), head(NULL), tail(NULL), lastElement(NULL), lastIndex(0) { }

inline PAbstractList::~PAbstractList()
 { DestroyContents(); }

inline PListElement::PListElement(PObject * theData)
  : next(NULL), prev(NULL), data(theData) { }


///////////////////////////////////////////////////////////////////////////////

inline PAbstractSortedList::PAbstractSortedList()
  : PCollection(), root(NULL), lastElement(NULL), lastIndex(0) { }

inline PAbstractSortedList::~PAbstractSortedList()
  { DestroyContents(); }

inline PSortedListElement::PSortedListElement(PObject * theData)
  : parent(NULL), left(NULL), right(NULL),
                               colour(Black), subTreeSize(1), data(theData) { }

inline void PSortedListElement::MakeBlack()
  { colour = Black; }

inline void PSortedListElement::MakeRed()
  { colour = Red; }

inline BOOL PSortedListElement::IsBlack()
  { return colour == Black; }

inline BOOL PSortedListElement::IsLeftBlack()
  { return left == NULL || left->colour == Black; }

inline BOOL PSortedListElement::IsRightBlack()
  { return right == NULL || right->colour == Black; }

inline BOOL PSortedListElement::LeftTreeSize()
  { return left != NULL ? left->subTreeSize : 0; }

inline BOOL PSortedListElement::RightTreeSize()
  { return right != NULL ? right->subTreeSize : 0; }


///////////////////////////////////////////////////////////////////////////////

inline PScalarKey::PScalarKey(PINDEX newKey)
  : theKey(newKey) { }

inline PScalarKey::operator PINDEX() const
  { return theKey; }


///////////////////////////////////////////////////////////////////////////////

inline PHashTable::~PHashTable()
  { DestroyContents(); }

inline PObject & PHashTable::AbstractGetDataAt(PINDEX index)
  { return *(PObject *)(SetLastElementAt(index) ? lastElement->data : NULL); }

inline const PObject & PHashTable::AbstractGetKeyAt(PINDEX index)
  { return *(PObject *)(SetLastElementAt(index) ? lastElement->key : NULL); }


///////////////////////////////////////////////////////////////////////////////

inline PAbstractSet::PAbstractSet()
  { }
  
inline PAbstractSet::PAbstractSet(const PAbstractSet & set)
  : PHashTable(set) { }
  
inline PAbstractSet::PAbstractSet(const PAbstractSet * set)
  : PHashTable(set) { }

inline BOOL PAbstractSet::Contains(const PObject & key)
  { return GetElementAt(key) != NULL; }


///////////////////////////////////////////////////////////////////////////////

inline PAbstractDictionary::PAbstractDictionary()
  { }
  
inline 
     PAbstractDictionary::PAbstractDictionary(const PAbstractDictionary & dict)
  : PHashTable(dict) { }
  
inline 
     PAbstractDictionary::PAbstractDictionary(const PAbstractDictionary * dict)
  : PHashTable(dict) { }

inline PObject * PAbstractDictionary::GetAt(PINDEX index) const
  { return ((PAbstractDictionary *)this)->GetAt(index); }

inline PObject * PAbstractDictionary::GetAt(const PObject & key) const
  { return ((PAbstractDictionary *)this)->GetAt(key); }



// End Of File ///////////////////////////////////////////////////////////////
