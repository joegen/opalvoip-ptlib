/*
 * $Id: contain.inl,v 1.14 1994/01/15 02:48:55 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: contain.inl,v $
 * Revision 1.14  1994/01/15 02:48:55  robertj
 * Rearranged PString assignment operator for NT portability.
 *
 * Revision 1.13  1994/01/13  08:42:29  robertj
 * Fixed missing copy constuctor and assignment operator for PString.
 *
 * Revision 1.12  1994/01/13  05:33:41  robertj
 * Added contructor to get caseless string from ordinary string.
 *
 * Revision 1.11  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.10  1993/12/31  06:48:46  robertj
 * Made inlines optional for debugging purposes.
 * Added PImgIcon class.
 *
 * Revision 1.9  1993/12/24  04:20:52  robertj
 * Mac CFront port.
 *
 * Revision 1.8  1993/12/22  05:54:08  robertj
 * Checked for severe out of memory condition in containers.
 *
 * Revision 1.7  1993/12/16  00:51:46  robertj
 * Made some container functions const.
 *
 * Revision 1.6  1993/12/15  21:10:10  robertj
 * Fixed reference system used by container classes.
 *
 * Revision 1.5  1993/08/27  18:17:47  robertj
 * Fixed bugs in PSortedList default size.
 *
 * Revision 1.4  1993/07/16  14:40:55  robertj
 * Added PString constructor for individual characters.
 * Added string to C style literal format.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


///////////////////////////////////////////////////////////////////////////////

BOOL PObject::IsClass(const char * clsName) const
  { return strcmp(clsName, GetClassName()) == 0; }

PINLINE BOOL PObject::operator==(const PObject & obj) const
  { return Compare(obj) == EqualTo; }

PINLINE BOOL PObject::operator!=(const PObject & obj) const
  { return Compare(obj) != EqualTo; }

PINLINE BOOL PObject::operator<(const PObject & obj) const
  { return Compare(obj) == LessThan; }

PINLINE BOOL PObject::operator>(const PObject & obj) const
  { return Compare(obj) == GreaterThan; }

PINLINE BOOL PObject::operator<=(const PObject & obj) const
  { return Compare(obj) != GreaterThan; }

PINLINE BOOL PObject::operator>=(const PObject & obj) const
  { return Compare(obj) != LessThan; }

PINLINE ostream & operator<<(ostream &strm, const PObject & obj)
  { return obj.PrintOn(strm); }

PINLINE istream & operator>>(istream &strm, PObject & obj)
  { return obj.ReadFrom(strm); }



///////////////////////////////////////////////////////////////////////////////

PINLINE PContainer::~PContainer()
  { Destruct(); }

PINLINE BOOL PContainer::IsUnique() const
  { return PAssertNULL(reference)->count <= 1; }

PINLINE void PContainer::CloneContents(const PContainer *)
  { }

PINLINE void PContainer::CopyContents(const PContainer &)
  { }


///////////////////////////////////////////////////////////////////////////////

PINLINE BOOL PAbstractArray::MakeUnique()
  { return IsUnique() || SetSize(GetSize()); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PString::PString()
  : PCharArray(1) { }

PINLINE PString::PString(const PString & str)
  : PCharArray(str) { }

PINLINE PString::PString(const char * cstr)
  : PCharArray(strlen(PAssertNULL(cstr))+1) { strcpy(theArray, cstr); }

PINLINE PString::PString(const char * cstr, PINDEX len)
  : PCharArray(len+1) { memcpy(theArray, PAssertNULL(cstr), len); }

PINLINE PString::PString(char c)
  : PCharArray(2) { *theArray = c; }

PINLINE PObject::Comparison PString::CompareString(const char * cstr) const
  { return (Comparison)strcmp(theArray,PAssertNULL(cstr)); }

PINLINE BOOL PString::MakeMinimumSize()
  { return SetSize(strlen(theArray)+1); }

PINLINE PINDEX PString::Length() const
  { return strlen(theArray); }

PINLINE PString & PString::operator=(const PString & str)
  { PCharArray::operator=(str); return *this; }

PINLINE PString PString::operator+(const PString & str) const
  { return operator+((const char *)str); }

PINLINE PString operator+(const char * cstr, const PString & str)
  { return PString(cstr) + str; }
  
PINLINE PString & PString::operator+=(const PString & str)
  { return operator+=((const char *)str); }

PINLINE BOOL PString::operator==(const PObject & obj) const
  { return PObject::operator==(obj); }

PINLINE BOOL PString::operator!=(const PObject & obj) const
  { return PObject::operator!=(obj); }

PINLINE BOOL PString::operator<(const PObject & obj) const
  { return PObject::operator<(obj); }

PINLINE BOOL PString::operator>(const PObject & obj) const
  { return PObject::operator>(obj); }

PINLINE BOOL PString::operator<=(const PObject & obj) const
  { return PObject::operator<=(obj); }

PINLINE BOOL PString::operator>=(const PObject & obj) const
  { return PObject::operator>=(obj); }

PINLINE BOOL PString::operator==(const char * cstr) const
  { return CompareString(cstr) == EqualTo; }

PINLINE BOOL PString::operator!=(const char * cstr) const
  { return CompareString(cstr) != EqualTo; }

PINLINE BOOL PString::operator<(const char * cstr) const
  { return CompareString(cstr) == LessThan; }

PINLINE BOOL PString::operator>(const char * cstr) const
  { return CompareString(cstr) == GreaterThan; }

PINLINE BOOL PString::operator<=(const char * cstr) const
  { return CompareString(cstr) != GreaterThan; }

PINLINE BOOL PString::operator>=(const char * cstr) const
  { return CompareString(cstr) != LessThan; }

PINLINE PString::operator const unsigned char *() const
  { return (const unsigned char *)theArray; }
  

///////////////////////////////////////////////////////////////////////////////

PINLINE PCaselessString::PCaselessString()
  : PString() { }

PINLINE PCaselessString::PCaselessString(const char * cstr)
  : PString(cstr) { }

PINLINE PCaselessString::PCaselessString(const PString & str)
  : PString(str) { }

PINLINE PCaselessString & PCaselessString::operator=(const PString & str)
  { PString::operator=(str); return *this; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PCollection::PCollection(PINDEX initialSize)
  : PContainer(initialSize) { }

PINLINE void PCollection::DeleteObjects(BOOL yes)
  { reference->deleteObjects = yes; }

PINLINE void PCollection::NoDeleteObjects()
  { DeleteObjects(FALSE); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PArrayObjects::PArrayObjects(PINDEX initialSize)
  : theArray(initialSize) { }

PINLINE void PArrayObjects::CopyContents(const PArrayObjects & array)
  { theArray = array.theArray; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PAbstractList::PAbstractList()
  : info(new ListInfo) { PAssertNULL(info); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PAbstractSortedList::PAbstractSortedList()
  : info(new SortedListInfo) { PAssertNULL(info); }

PINLINE void PSortedListElement::MakeBlack()
  { colour = Black; }

PINLINE void PSortedListElement::MakeRed()
  { colour = Red; }

PINLINE BOOL PSortedListElement::IsBlack()
  { return colour == Black; }

PINLINE BOOL PSortedListElement::IsLeftBlack()
  { return left == NULL || left->colour == Black; }

PINLINE BOOL PSortedListElement::IsRightBlack()
  { return right == NULL || right->colour == Black; }

PINLINE BOOL PSortedListElement::LeftTreeSize()
  { return left != NULL ? left->subTreeSize : 0; }

PINLINE BOOL PSortedListElement::RightTreeSize()
  { return right != NULL ? right->subTreeSize : 0; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PScalarKey::PScalarKey(PINDEX newKey)
  : theKey(newKey) { }

PINLINE PScalarKey::operator PINDEX() const
  { return theKey; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PObject & PHashTable::AbstractGetDataAt(PINDEX index) const
  { return *(PObject *)(hashTable->SetLastElementAt(index)
                                      ? hashTable->lastElement->data : NULL); }

PINLINE const PObject & PHashTable::AbstractGetKeyAt(PINDEX index) const
  { return *(PObject *)(hashTable->SetLastElementAt(index)
                                       ? hashTable->lastElement->key : NULL); }


///////////////////////////////////////////////////////////////////////////////

PINLINE PAbstractSet::PAbstractSet()
  { }
  
PINLINE BOOL PAbstractSet::Contains(const PObject & key)
  { return hashTable->GetElementAt(key) != NULL; }


///////////////////////////////////////////////////////////////////////////////

PINLINE PAbstractDictionary::PAbstractDictionary()
  { }
  


// End Of File ///////////////////////////////////////////////////////////////
