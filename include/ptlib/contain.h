/*
 * $Id: contain.h,v 1.37 1994/11/28 12:33:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Container Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: contain.h,v $
 * Revision 1.37  1994/11/28 12:33:44  robertj
 * Added dummy parameter for cls* constructor in containers. This prevents some very
 * strange an undesirable default construction of clones.
 *
 * Revision 1.36  1994/10/30  11:50:09  robertj
 * Split into Object classes and Container classes.
 * Changed mechanism for doing notification callback functions.
 *
 * Revision 1.35  1994/10/23  04:40:50  robertj
 * Made container * constractor protected.
 * Shorted OS Error assert.
 * Added printf constructor to PString.
 *
 * Revision 1.34  1994/09/25  10:36:41  robertj
 * Improved const behavious of container class macros.
 *
 * Revision 1.33  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.32  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.31  1994/08/21  23:43:02  robertj
 * Changed parameter before variable argument list to NOT be a reference.
 * Added object serialisation classes.
 *
 * Revision 1.30  1994/08/04  11:51:39  robertj
 * Rewrite of memory check functions.
 *
 * Revision 1.29  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.28  1994/07/25  03:33:50  robertj
 * Extra memory tests.
 *
 * Revision 1.27  1994/07/17  10:46:06  robertj
 * Added functions to strings in containers.
 *
 * Revision 1.26  1994/07/02  03:03:49  robertj
 * Addition of container searching facilities.
 *
 * Revision 1.25  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.24  1994/04/20  12:17:44  robertj
 * Added code to assert
 *
 * Revision 1.23  1994/04/11  14:17:27  robertj
 * Made standard operators new and delete only declared for GNU C++
 *
 * Revision 1.22  1994/04/01  14:09:46  robertj
 * Removed PDECLARE_ABSTRACT_CONTAINER.
 * Added string stream class.
 * Added string containers.
 *
 * Revision 1.21  1994/03/07  07:38:19  robertj
 * Major enhancementsacross the board.
 *
 * Revision 1.20  1994/01/13  08:42:29  robertj
 * Fixed missing copy constuctor and assignment operator for PString.
 *
 * Revision 1.19  1994/01/13  05:33:41  robertj
 * Added contructor to get caseless string from ordinary string.
 *
 * Revision 1.18  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.17  1993/12/31  06:40:34  robertj
 * Made inlines optional for debugging purposes.
 * Added default to DeleteObjects() function.
 *
 * Revision 1.16  1993/12/24  04:20:52  robertj
 * Mac CFront port.
 *
 * Revision 1.15  1993/12/16  00:51:46  robertj
 * Made some container functions const.
 *
 * Revision 1.14  1993/12/15  21:10:10  robertj
 * Changes to fix inadequate reference system for containers.
 *
 * Revision 1.13  1993/12/14  18:44:56  robertj
 * Added RemoveAll() to collection classes.
 * Fixed incorrect destruction of objects in containers.
 *
 * Revision 1.12  1993/12/04  05:23:58  robertj
 * Added more string functions
 *
 * Revision 1.11  1993/09/27  16:35:25  robertj
 * Fixed bug in sorted lists.
 * Changed simple function for array of strings to a constructor.
 * Capitalised all macros.
 *
 * Revision 1.10  1993/08/27  18:17:47  robertj
 * Fixed bug with default number of elements in a collection.
 * Added missing Compare function to PAbstractSortedList
 * Added inline keywords for CFront compatibility.
 *
 * Revision 1.9  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.8  1993/08/19  18:00:32  robertj
 * Added two more standard base array classes
 *
 * Revision 1.7  1993/08/01  14:05:27  robertj
 * Added const to ToLower() and ToUpper() in the PString class.
 *
 * Revision 1.6  1993/07/16  14:40:55  robertj
 * Added PString constructor for individual characters.
 * Added string to C style literal format.
 *
 * Revision 1.5  1993/07/15  05:02:57  robertj
 * Removed redundant word in PString enum for string types.
 *
 * Revision 1.4  1993/07/15  04:23:39  robertj
 * Added constructor to PString to allow conversion from other string formats.
 * Fixed problem with variable parameter lists in sprintf() functions.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#ifndef _CONTAIN_H
#define _CONTAIN_H

#ifdef __GNUC__
#pragma interface
#endif

#include <object.h>


///////////////////////////////////////////////////////////////////////////////
// Abstract container class

PDECLARE_CLASS(PContainer, PObject)
  public:
    PContainer(PINDEX initialSize = 0);
    PContainer(const PContainer & cont);
    PContainer & operator=(const PContainer & cont);
    virtual ~PContainer();

    // New functions for class
    virtual PINDEX GetSize() const;
    virtual BOOL SetSize(PINDEX newSize) = 0;
    virtual BOOL IsEmpty() const;
    

  protected:
    PContainer(int dummy, const PContainer * cont);

    void Destruct();
      // Function called from container destructors. This will conditionally
      // call DestroyContents() to destroy the container content.

    virtual void DestroyContents() = 0;
      // Destroy the container content.

    virtual void CloneContents(const PContainer * c);
    virtual void CopyContents(const PContainer & c);
      // Copy the container content

    class Reference {
      public:
        inline Reference(PINDEX initialSize)
          : size(initialSize), count(1), deleteObjects(TRUE) { }
        PINDEX   size;      // Size of what the container contains
        unsigned count;     // reference count to the container content
        BOOL deleteObjects; // Used by PCollection but put here for efficieny
    } * reference;
    BOOL IsUnique() const;
};


#define PCONTAINERINFO(cls, par) \
    PCLASSINFO(cls, par) \
  public: \
    cls(const cls & c) : par(c) { CopyContents(c); } \
    cls & operator=(const cls & c) \
      { par::operator=(c); cls::CopyContents(c); return *this; } \
    virtual ~cls() { Destruct(); } \
  protected: \
    cls(int dummy, const cls * c) : par(dummy, c) { CloneContents(c); } \
    virtual void DestroyContents(); \
    virtual void CloneContents(const cls * c); \
    virtual void CopyContents(const cls & c); \

#define PDECLARE_CONTAINER(cls, par) \
                             PCLASS cls : public par { PCONTAINERINFO(cls, par)


///////////////////////////////////////////////////////////////////////////////
// The abstract array class

class PArrayObjects;

PDECLARE_CONTAINER(PAbstractArray, PContainer)
  friend class PArrayObjects;

  public:
    PAbstractArray(PINDEX elementSizeInBytes,
                   PINDEX initialSize = 0);
    PAbstractArray(PINDEX elementSizeInBytes,
                   const void *buffer,
                   PINDEX bufferSizeInElements);

    // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;

    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);

    // New functions for class
    BOOL SetMinSize(PINDEX minSize);
    void * GetPointer(PINDEX minSize = 0);

  protected:
    PINDEX elementSize;
    char * theArray;

    BOOL MakeUnique();
};



///////////////////////////////////////////////////////////////////////////////
// An array of some base type

#define PBASEARRAYCLASS(cls, T) \
  typedef T P_##cls##_Base_Type; \
  PDECLARE_CLASS(cls, PAbstractArray) \
  public: \
    inline cls(PINDEX initialSize = 0) \
      : PAbstractArray(sizeof(P_##cls##_Base_Type), initialSize) { } \
    inline cls(P_##cls##_Base_Type const * buffer, PINDEX length) \
      : PAbstractArray(sizeof(P_##cls##_Base_Type), buffer, length) { } \
    inline BOOL SetAt(PINDEX index, P_##cls##_Base_Type val) \
      { return SetMinSize(index+1) && val==(((P_##cls##_Base_Type *)theArray)[index] = val); } \
    inline P_##cls##_Base_Type GetAt(PINDEX index) const \
      { PASSERTINDEX(index); \
                  return index < GetSize() ? ((P_##cls##_Base_Type *)theArray)[index] : (P_##cls##_Base_Type)0; } \
    inline P_##cls##_Base_Type * GetPointer(PINDEX minSize = 0) \
      { return (P_##cls##_Base_Type *)PAbstractArray::GetPointer(minSize); } \
    inline P_##cls##_Base_Type operator[](PINDEX index) const \
      { PASSERTINDEX(index); return GetAt(index); } \
    inline P_##cls##_Base_Type & operator[](PINDEX index) \
      { PASSERTINDEX(index); PAssert(SetMinSize(index+1), POutOfMemory); \
        return ((P_##cls##_Base_Type *)theArray)[index]; } \
    inline operator P_##cls##_Base_Type const *() const \
      { return (P_##cls##_Base_Type const *)theArray; } \

#define PBASEARRAY(cls, T) PBASEARRAYCLASS(cls, T) }


PBASEARRAY(PCharArray, char);
PBASEARRAY(PShortArray, short);
PBASEARRAY(PIntArray, int);
PBASEARRAY(PLongArray, long);
PBASEARRAY(PBYTEArray, BYTE);
PBASEARRAY(PWORDArray, WORD);
PBASEARRAY(PUnsignedArray, unsigned);
PBASEARRAY(PDWORDArray, DWORD);




///////////////////////////////////////////////////////////////////////////////
// PString class (specialised version of PBASEARRAY(char))

class PStringArray;

PDECLARE_CLASS(PString, PCharArray)
  public:
    PINLINE PString();
    PString(const PString & str);
    PString(const char * cstr);
    PString(const char * cstr, PINDEX len);
    PString(char c);
    enum ConversionType {
      Pascal,   // Length byte followed by data
      Basic,    // Two Length bytes followed by data
      Literal,  // C style string with \ escape codes
      Signed,   // Signed integer
      Unsigned, // Unsigned integer
      Decimal,  // Real decimal number
      Exponent, // Real exponent number
      Printf,   // Formatted print
      NumConversionTypes
    };
    PString(ConversionType type, const char * str, ...);
    PString(ConversionType type, long value, unsigned base = 10);
    PString(ConversionType type, double value, unsigned places = 4);


    // Overrides from class PObject
    virtual PObject * Clone() const;
    virtual Comparison Compare(const PObject & obj) const;
    virtual ostream & PrintOn(ostream &strm) const;
    virtual istream & ReadFrom(istream &strm);
    virtual PINDEX HashFunction() const;


    // Overrides from class PContainer
    virtual BOOL IsEmpty() const;


    // New functions for class
    BOOL MakeMinimumSize();
    PINDEX GetLength() const;

    PString & operator=(const PString & str);
    PString & operator=(const char * cstr);
    PString operator+(const PString & str) const;
    PString operator+(const char * cstr) const;
    PString operator+(char c) const;
    friend PString operator+(const char * cstr, const PString & str);
    PString & operator+=(const PString & cstr);
    PString & operator+=(const char * cstr);

    BOOL operator==(const PObject & str) const;
    BOOL operator!=(const PObject & str) const;
    BOOL operator<(const PObject & str) const;
    BOOL operator>(const PObject & str) const;
    BOOL operator<=(const PObject & str) const;
    BOOL operator>=(const PObject & str) const;

    BOOL operator==(const char * cstr) const;
    BOOL operator!=(const char * cstr) const;
    BOOL operator<(const char * cstr) const;
    BOOL operator>(const char * cstr) const;
    BOOL operator<=(const char * cstr) const;
    BOOL operator>=(const char * cstr) const;

    virtual PINDEX Find(char ch, PINDEX offset = 0) const;
    virtual PINDEX Find(const PString & str, PINDEX offset = 0) const;
    virtual PINDEX FindLast(char ch, PINDEX offset = P_MAX_INDEX) const;
    virtual PINDEX FindLast(const PString & str,
                                          PINDEX offset = P_MAX_INDEX) const;

    virtual PINDEX FindOneOf(const PString & set, PINDEX offset = 0) const;

    void Delete(PINDEX start, PINDEX len);

    PString operator()(PINDEX start, PINDEX end) const; // Sub-string
    PString Left(PINDEX len) const;
    PString Right(PINDEX len) const;
    PString Mid(PINDEX start, PINDEX len = P_MAX_INDEX) const;

    PString LeftTrim() const;
    PString RightTrim() const;
    PString Trim() const;

    PString ToLower() const;
    PString ToUpper() const;

    PString & sprintf(const char * fmt, ...);
    PString & sprintf(PString fmt, ...);
    PString & vsprintf(const char * fmt, va_list args);
    PString & vsprintf(const PString & fmt, va_list args);

    long AsInteger(unsigned base = 10) const;
    double AsReal() const;

    PStringArray Tokenise(const PString & separators,
                                          BOOL onePerSeparator = TRUE) const;
    PStringArray Lines() const;


    // Some Mac support.
    PString ToPascal() const;
    operator const unsigned char *() const;

    PString ToLiteral() const; // Convert to C literal string format


  protected:
    PString(int dummy, const PString * str);
    virtual Comparison CompareString(const char * cstr) const;
};

PString psprintf(const char * fmt, ...);
PString psprintf(PString fmt, ...);
PString pvsprintf(const char * fmt, va_list args);
PString pvsprintf(const PString & fmt, va_list args);


PDECLARE_CLASS(PCaselessString, PString)
  public:
    PCaselessString();
    PCaselessString(const char * cstr);
    PCaselessString(const PString & str);
    PCaselessString & operator=(const PString & str);

    // Overrides from class PString
    virtual PINDEX Find(char ch, PINDEX offset = 0) const;
    virtual PINDEX Find(const char * cstr, PINDEX offset = 0) const;
    virtual PINDEX FindLast(char ch) const;
    virtual PINDEX FindLast(const char * cstr) const;

  protected:
    PCaselessString(int dummy, const PCaselessString * str);
    virtual Comparison CompareString(const char * cstr) const;
};


class PStringStream;

PCLASS PStringStreamBuffer : public PObject, public streambuf {
  PCLASSINFO(PStringStreamBuffer, PObject)

  public:
    PStringStreamBuffer(const PStringStreamBuffer & sbuf);
    PStringStreamBuffer & operator=(const PStringStreamBuffer & sbuf);

  protected:
    PStringStreamBuffer(PStringStream * str);
      // Construct the streambuf for standard streams

    virtual int overflow(int=EOF);
      // Function to flush the output buffer to the file

    virtual int underflow();
      // Function to refill the input buffer from the file

    virtual int sync();
      // Function to refill the input buffer from the file

    virtual streampos seekoff(streamoff, ios::seek_dir, int);
      // Function to seek a location in the file


  private:
    // Member variables
    PStringStream * string;


  friend class PStringStream;
};


PCLASS PStringStream : public PString, public iostream {
  PCLASSINFO(PStringStream, PString)

  public:
    PStringStream();
    PStringStream(const PString & str);
    PStringStream(const char * cstr);
    PStringStream & operator=(const PString & str);
      // Create a string that can do standard stream I/O.

    virtual ~PStringStream();
      // Destroy the string stream, deleting the stream buffer

  private:
    PStringStream(int, const PStringStream &) { }
    PStringStream & operator=(const PStringStream &) { return *this; }
};



///////////////////////////////////////////////////////////////////////////////
// Abstract collection of objects class

typedef BOOL (*PEnumerator)(const PObject & obj, PObject * info);

PDECLARE_CLASS(PCollection, PContainer)
  public:
    PCollection(PINDEX initialSize = 0);

    // Overrides from class PObject
    virtual ostream & PrintOn(ostream &strm) const;

    // New functions for class
    virtual PINDEX Append(PObject * obj) = 0;
    virtual PINDEX Insert(const PObject & before, PObject * obj) = 0;
    virtual PINDEX InsertAt(PINDEX index, PObject * obj) = 0;
    virtual BOOL Remove(const PObject * obj) = 0;
    virtual PObject * RemoveAt(PINDEX index) = 0;
    virtual void RemoveAll();
    virtual BOOL SetAt(PINDEX index, PObject * val) = 0;
    virtual PObject * GetAt(PINDEX index) const = 0;
    virtual PINDEX GetObjectsIndex(const PObject * obj) const = 0;
    virtual PINDEX GetValuesIndex(const PObject & obj) const = 0;

    virtual BOOL Enumerate(PEnumerator func, PObject * info = NULL) const = 0;

    void AllowDeleteObjects(BOOL yes = TRUE);
    void DisallowDeleteObjects();

  protected:
    PCollection(int dummy, const PCollection *);
};



///////////////////////////////////////////////////////////////////////////////
// Linear array of objects

PDECLARE_CONTAINER(PArrayObjects, PCollection)
  public:
    PINLINE PArrayObjects(PINDEX initialSize = 0);

    // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;

    // Overrides from class PContainer
    virtual PINDEX GetSize() const;
    virtual BOOL SetSize(PINDEX newSize);

    // Overrides from class PCollection
    virtual PINDEX Append(PObject * obj);
    virtual PINDEX Insert(const PObject & before, PObject * obj);
    virtual PINDEX InsertAt(PINDEX index, PObject * obj);
    virtual BOOL Remove(const PObject * obj);
    virtual PObject * RemoveAt(PINDEX index);
    virtual BOOL SetAt(PINDEX index, PObject * val);
    virtual PObject * GetAt(PINDEX index) const;
    virtual PINDEX GetObjectsIndex(const PObject * obj) const;
    virtual PINDEX GetValuesIndex(const PObject & obj) const;

    virtual BOOL Enumerate(PEnumerator func, PObject * info = NULL) const;

  protected:
    PBASEARRAY(ObjPtrArray, PObject *);
    ObjPtrArray * theArray;
};


#define PARRAYCLASS(cls, T) \
  PDECLARE_CLASS(cls, PArrayObjects) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PArrayObjects(dummy, c) { } \
  public: \
    inline cls(PINDEX initialSize = 0) \
      : PArrayObjects(initialSize) { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline T & operator[](PINDEX index) const\
      { PAssert((*theArray)[index] != NULL, PInvalidArrayElement); return *(T *)(*theArray)[index]; } \
    inline T & operator[](PINDEX index) \
      { return *(T *)((*theArray)[index] != NULL ?  (*theArray)[index] \
                                            : ((*theArray)[index] = PNEW T)); } \

#define PARRAY(cls, T) PARRAYCLASS(cls, T) }


PARRAYCLASS(PStringArray, PString)
  public:
    PStringArray(PINDEX count, char **strarr);
    PINDEX GetStringsIndex(const PString & str) const;
};



///////////////////////////////////////////////////////////////////////////////
// PList container class

class PAbstractList;

PCLASS PListElement {
  PListElement(PObject * theData);
  PListElement * prev, * next;
  PObject * data;
  friend class PAbstractList;
};

PDECLARE_CONTAINER(PAbstractList, PCollection)
  public:
    PINLINE PAbstractList();

    // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;

    // Overrides from class PCollection
    virtual PINDEX Append(PObject * obj);
    virtual PINDEX Insert(const PObject & before, PObject * obj);
    virtual PINDEX InsertAt(PINDEX index, PObject * obj);
    virtual BOOL Remove(const PObject * obj);
    virtual PObject * RemoveAt(PINDEX index);
    virtual BOOL SetAt(PINDEX index, PObject * val);
    virtual PObject * GetAt(PINDEX index) const;
    virtual PINDEX GetObjectsIndex(const PObject * obj) const;
    virtual PINDEX GetValuesIndex(const PObject & obj) const;

    virtual BOOL Enumerate(PEnumerator func, PObject * info = NULL) const;

  protected:
    class ListInfo {
      public:
        ListInfo() { head = tail = lastElement = NULL; }
        PListElement * head, * tail, * lastElement;
        PINDEX lastIndex;
    } * info;

    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);

    BOOL SetCurrent(PINDEX index) const;
};


#define PLISTCLASS(cls, T) \
  PDECLARE_CLASS(cls, PAbstractList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractList(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractList() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline T & operator[](PINDEX index) const \
      { return *(T *)GetAt(index); } \

#define PLIST(cls, T) PLISTCLASS(cls, T) }


#define PQUEUECLASS(cls, T) \
  PDECLARE_CLASS(cls, PAbstractList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractList(dummy, c) { DisallowDeleteObjects(); } \
  public: \
    inline cls() \
      : PAbstractList() { DisallowDeleteObjects(); } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    virtual inline void Enqueue(T * t) \
      { PAbstractList::Append(t); } \
    virtual inline T * Dequeue() \
      { return (T *)PAbstractList::RemoveAt(0);} \

#define PQUEUE(cls, T) PQUEUECLASS(cls, T) }


#define PSTACKCLASS(cls, T) \
  PDECLARE_CLASS(cls, PAbstractList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractList(dummy, c) { DisallowDeleteObjects(); } \
  public: \
    inline cls() \
      : PAbstractList() { DisallowDeleteObjects(); } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    virtual inline void Push(T * t) \
      { PAbstractList::Append(t); } \
    virtual inline T * Pop() \
      { return (T *)PAbstractList::RemoveAt(GetSize()-1); } \
    virtual inline T & Top() \
      { PAssert(GetSize() > 0, PStackEmpty); return *(T *)GetAt(GetSize()-1); } \

#define PSTACK(cls, T) PSTACKCLASS(cls, T) }


PLISTCLASS(PStringList, PString)
  public:
    PINDEX AppendString(const PString & str);
    PINDEX InsertString(const PString & before, const PString & str);
    PINDEX GetStringsIndex(const PString & str) const;
};



///////////////////////////////////////////////////////////////////////////////
// Sorted List of PObjects

class PAbstractSortedList;

PCLASS PSortedListElement {
  PSortedListElement(PObject * theData);
  PSortedListElement * parent, * left, * right;
  PObject * data;
  PINDEX subTreeSize;
  enum { Red, Black } colour;
  void MakeBlack();
  void MakeRed();
  BOOL IsBlack();
  BOOL IsLeftBlack();
  BOOL IsRightBlack();
  void DeleteSubTrees(BOOL deleteObject);
  BOOL LeftTreeSize();
  BOOL RightTreeSize();
  PSortedListElement * Successor() const;
  PSortedListElement * Predecessor() const;
  PSortedListElement * OrderSelect(PINDEX index);
  PINDEX ValueSelect(const PObject & obj);

  friend class PAbstractSortedList;
};



PDECLARE_CONTAINER(PAbstractSortedList, PCollection)
  public:
    PINLINE PAbstractSortedList();

    // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;

    // Overrides from class PCollection
    virtual PINDEX Append(PObject * obj);
    virtual PINDEX Insert(const PObject & before, PObject * obj);
    virtual PINDEX InsertAt(PINDEX index, PObject * obj);
    virtual BOOL Remove(const PObject * obj);
    virtual PObject * RemoveAt(PINDEX index);
    virtual void RemoveAll();
    virtual BOOL SetAt(PINDEX index, PObject * val);
    virtual PObject * GetAt(PINDEX index) const;
    virtual PINDEX GetObjectsIndex(const PObject * obj) const;
    virtual PINDEX GetValuesIndex(const PObject & obj) const;

    virtual BOOL Enumerate(PEnumerator func, PObject * info = NULL) const;

  protected:
    class SortedListInfo {
    public:
      SortedListInfo() { root = lastElement = NULL; }
        PSortedListElement * root, * lastElement;
        PINDEX lastIndex;
    } * info;

    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);

    // New functions for class
    void RemoveElement(PSortedListElement * node);
    void LeftRotate(PSortedListElement * node);
    void RightRotate(PSortedListElement * node);

    friend class PSortedListElement;
};


#define PSORTEDLISTCLASS(cls, T) \
  PDECLARE_CLASS(cls, PAbstractSortedList) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractSortedList(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractSortedList() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline T & operator[](PINDEX index) const \
      { return *(T *)GetAt(index); } \

#define PSORTEDLIST(cls, T) PSORTEDLISTCLASS(cls, T) }


PSORTEDLISTCLASS(PSortedStringList, PString)
  public:
    PINDEX AppendString(const PString & str);
    PINDEX InsertString(const PString & before, const PString & str);
    PINDEX GetStringsIndex(const PString & str) const;
};


///////////////////////////////////////////////////////////////////////////////
// PDictionary classes

PDECLARE_CLASS(PScalarKey, PObject)
  public:
    PScalarKey(PINDEX newKey);

    // Overrides from class PObject
    virtual PObject * Clone() const;
    virtual Comparison Compare(const PObject & obj) const;
    virtual PINDEX HashFunction() const;
    virtual ostream & PrintOn(ostream & strm) const;
    operator PINDEX() const;

  private:
    PINDEX theKey;
};


class PHashTable;
class PInternalHashTable;
class PAbstractDictionary;

PCLASS PHashTableElement {
  PObject * key;
  PObject * data;
  PHashTableElement * next;
  PHashTableElement * prev;
  friend class PHashTable;
  friend class PInternalHashTable;
  friend class PAbstractDictionary;
};

PBASEARRAYCLASS(PInternalHashTable, PHashTableElement *)
  private:
    virtual ~PInternalHashTable() { Destruct(); }
    virtual void DestroyContents();

    void AppendElement(PObject * key, PObject * data);
    PObject * RemoveElement(const PObject & key);
    BOOL SetLastElementAt(PINDEX index);
    PHashTableElement * GetElementAt(const PObject & key);
    PINDEX GetElementsIndex(const PObject * obj, BOOL byVal, BOOL keys) const;
    BOOL EnumerateElements(PEnumerator func, PObject * info, BOOL keys) const;

    PINDEX lastIndex, lastBucket;
    PHashTableElement * lastElement;

    BOOL deleteKeys;


  friend class PHashTable;
  friend class PAbstractSet;
  friend class PAbstractDictionary;
};


PDECLARE_CONTAINER(PHashTable, PCollection)
  public:
    PHashTable();

    // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;


  protected:
    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);

    // New functions for class
    virtual const PObject & AbstractGetKeyAt(PINDEX index) const;
    virtual PObject & AbstractGetDataAt(PINDEX index) const;

    // Member variables
    PInternalHashTable * hashTable;
};


PDECLARE_CONTAINER(PAbstractSet, PHashTable)
  public:
    PINLINE PAbstractSet();

    // Overrides from class PCollection
    virtual PINDEX Append(PObject * obj);
    virtual PINDEX Insert(const PObject & before, PObject * obj);
    virtual PINDEX InsertAt(PINDEX index, PObject * obj);
    virtual BOOL Remove(const PObject * obj);
    virtual PObject * RemoveAt(PINDEX index);
    virtual BOOL SetAt(PINDEX index, PObject * val);
    virtual PObject * GetAt(PINDEX index) const;
    virtual PINDEX GetObjectsIndex(const PObject * obj) const;
    virtual PINDEX GetValuesIndex(const PObject & obj) const;

    virtual BOOL Enumerate(PEnumerator func, PObject * info = NULL) const;

    // New functions for class
    PINLINE BOOL Contains(const PObject & key);
};


PDECLARE_CLASS(PStringSet, PAbstractSet)
  protected:
    PStringSet(int dummy, const PStringSet * c);
  public:
    PStringSet();
    virtual PObject * Clone() const;
    void Include(const PString & key);
    void Exclude(const PString & key);
    BOOL operator[](const PString & key);
    const PString & GetKeyAt(PINDEX index) const;
};


#define PSETCLASS(cls, K) \
  PDECLARE_CLASS(cls, PAbstractSet) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractSet(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractSet() { DisallowDeleteObjects(); } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline void Include(const PObject * key) \
      { Append((PObject *)key); } \
    inline void Exclude(const PObject * key) \
      { Remove(key); } \
    inline BOOL operator[](const K & key) \
        { return Contains(key); } \
    virtual const K & GetKeyAt(PINDEX index) const \
      { return (const K &)AbstractGetKeyAt(index); } \

#define PSET(cls, K) PSETCLASS(cls, K) }



PDECLARE_CLASS(PAbstractDictionary, PHashTable)
  public:
    PINLINE PAbstractDictionary();

    // Overrides from class PCollection
    virtual PINDEX Append(PObject * obj);
    virtual PINDEX Insert(const PObject & before, PObject * obj);
    virtual PINDEX InsertAt(PINDEX index, PObject * obj);
    virtual BOOL Remove(const PObject * obj);
    virtual PObject * RemoveAt(PINDEX index);
    virtual BOOL SetAt(PINDEX index, PObject * val);
    virtual PObject * GetAt(PINDEX index) const;
    virtual PINDEX GetObjectsIndex(const PObject * obj) const;
    virtual PINDEX GetValuesIndex(const PObject & obj) const;

    virtual BOOL Enumerate(PEnumerator func, PObject * info = NULL) const;

    // New functions for class
    virtual BOOL SetDataAt(PINDEX index, PObject * obj);
    virtual BOOL SetAt(const PObject & key, PObject * obj);
    virtual PObject * GetAt(const PObject & key) const;
    virtual PObject & GetRefAt(const PObject & key) const;
    virtual BOOL EnumerateKeys(PEnumerator func, PObject * info = NULL) const;

  protected:
    PAbstractDictionary(int dummy, const PAbstractDictionary * c);
};


PDECLARE_CLASS(PStringDictionary, PAbstractDictionary)
  public:
    PStringDictionary();
    virtual PObject * Clone() const;
    virtual BOOL SetAt(const PObject & key, PString str);
    PString & operator[](const PString & key) const;
    const PString & GetKeyAt(PINDEX index) const;
    PString & GetDataAt(PINDEX index) const;
    BOOL SetDataAt(PINDEX index, const PString & str);

  protected:
    PStringDictionary(int dummy, const PStringDictionary * c);
};


#define PDICTIONARYCLASS(cls, K, D) \
  PDECLARE_CLASS(cls, PAbstractDictionary) \
  protected: \
    inline cls(int dummy, const cls * c) \
      : PAbstractDictionary(dummy, c) { } \
  public: \
    inline cls() \
      : PAbstractDictionary() { } \
    inline virtual PObject * Clone() const \
      { return PNEW cls(0, this); } \
    inline D & operator[](const K & key) const \
      { return (D &)GetRefAt(key); } \
    inline const K & GetKeyAt(PINDEX index) const \
      { return (const K &)AbstractGetKeyAt(index); } \
    inline D & GetDataAt(PINDEX index) const \
      { return (D &)AbstractGetDataAt(index); } \

#define PDICTIONARY(cls, K, D) PDICTIONARYCLASS(cls, K, D) }



///////////////////////////////////////////////////////////////////////////////
// Fill in all the inline functions

#if defined(P_USE_INLINES)
#include "contain.inl"
#endif

#endif // _CONTAIN_H


// End Of File ///////////////////////////////////////////////////////////////
