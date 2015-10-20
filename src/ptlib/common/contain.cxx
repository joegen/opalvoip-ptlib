/*
 * contain.cxx
 *
 * Container Classes
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ctype.h>

#include <ostream>
#include <limits>
#include <math.h>

#ifdef __NUCLEUS_PLUS__
extern "C" int vsprintf(char *, const char *, va_list);
#endif

#if P_REGEX
#include <regex.h>
#else
#include "regex/regex.h"
#endif


#if !P_USE_INLINES
#include "ptlib/contain.inl"
#endif


PDEFINE_POOL_ALLOCATOR(PContainerReference);


#define new PNEW
#undef  __CLASS__
#define __CLASS__ GetClass()


///////////////////////////////////////////////////////////////////////////////

PContainer::PContainer(PINDEX initialSize)
{
  reference = new PContainerReference(initialSize);
  PAssert(reference != NULL, POutOfMemory);
}


PContainer::PContainer(int, const PContainer * cont)
{
  if (cont == this)
    return;

  PAssert(cont != NULL, PInvalidParameter);
  PAssert2(cont->reference != NULL, cont->GetClass(), "Clone of deleted container");

  reference = new PContainerReference(*cont->reference);
  PAssert(reference != NULL, POutOfMemory);
}


PContainer::PContainer(const PContainer & cont)
{
  if (&cont == this)
    return;

  PAssert2(cont.reference != NULL, cont.GetClass(), "Copy of deleted container");

  ++cont.reference->count;
  reference = cont.reference;  // copy the reference pointer
}


PContainer::PContainer(PContainerReference & ref)
  : reference(&ref)
{
}


void PContainer::AssignContents(const PContainer & cont)
{
  if(cont.reference == NULL){
    PAssertAlways("container reference is null");
    return;
  }
  if(cont.GetClass() == NULL){
    PAssertAlways("container class is null");
    return;
  }

  if (reference == cont.reference)
    return;

  if (--reference->count == 0) {
    DestroyContents();
    DestroyReference();
  }

  PAssert(++cont.reference->count > 1, "Assignment of container that was deleted");
  reference = cont.reference;
}


void PContainer::Destruct()
{
  if (reference != NULL) {
    if (--reference->count <= 0) {
      DestroyContents();
      DestroyReference();
    }
    reference = NULL;
  }
}


void PContainer::DestroyReference()
{
  delete reference;
  reference = NULL;
}


PBoolean PContainer::SetMinSize(PINDEX minSize)
{
  PASSERTINDEX(minSize);
  if (minSize < 0)
    minSize = 0;
  if (minSize < GetSize())
    minSize = GetSize();
  return SetSize(minSize);
}


PBoolean PContainer::MakeUnique()
{
  if (IsUnique())
    return true;

  PContainerReference * oldReference = reference;
  reference = new PContainerReference(*reference);
  --oldReference->count;

  return false;
}


///////////////////////////////////////////////////////////////////////////////

#if !PMEMORY_CHECK

  #define PAbstractArrayAllocate(s)     (char *)PMemoryHeap::Allocate(s, __FILE__, __LINE__, "PAbstractArrayData")
  #define PAbstractArrayReallocate(p,s)         PMemoryHeap::Reallocate(p, s, __FILE__, __LINE__, "PAbstractArrayData")
  #define PAbstractArrayDeallocate(p)           PMemoryHeap::Deallocate(p, "PAbstractArrayData")

#else // PMEMORY_CHECK

  #define PAbstractArrayAllocate(s)     (char *)malloc(s)
  #define PAbstractArrayReallocate(p,s)         realloc(p, s)
  #define PAbstractArrayDeallocate(p)           free(p)

#endif // PMEMORY_CHECK


PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes, PINDEX initialSize)
  : PContainer(initialSize)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0, PInvalidParameter);

  if (GetSize() == 0)
    theArray = NULL;
  else {
    theArray = PAbstractArrayAllocate(GetSize() * elementSize);
    PAssert(theArray != NULL, POutOfMemory);
    memset(theArray, 0, GetSize() * elementSize);
  }

  allocatedDynamically = true;
}


PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes,
                               const void *buffer,
                               PINDEX bufferSizeInElements,
                               PBoolean dynamicAllocation)
  : PContainer(bufferSizeInElements)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0, PInvalidParameter);

  allocatedDynamically = dynamicAllocation;

  if (GetSize() == 0)
    theArray = NULL;
  else if (dynamicAllocation) {
    PINDEX sizebytes = elementSize*GetSize();
    theArray = PAbstractArrayAllocate(sizebytes);
    PAssert(theArray != NULL, POutOfMemory);
    memcpy(theArray, PAssertNULL(buffer), sizebytes);
  }
  else
    theArray = (char *)buffer;
}


PAbstractArray::PAbstractArray(PContainerReference & reference, PINDEX elementSizeInBytes)
  : PContainer(reference)
  , elementSize(elementSizeInBytes)
  , theArray(NULL)
  , allocatedDynamically(false)
{
}


void PAbstractArray::DestroyContents()
{
  if (theArray != NULL) {
    if (allocatedDynamically)
      PAbstractArrayDeallocate(theArray);
    theArray = NULL;
  }
}


void PAbstractArray::CopyContents(const PAbstractArray & array)
{
  elementSize = array.elementSize;
  theArray = array.theArray;
  allocatedDynamically = array.allocatedDynamically;

  if (reference->constObject)
    MakeUnique();
}


void PAbstractArray::CloneContents(const PAbstractArray * array)
{
  elementSize = array->elementSize;
  PINDEX sizebytes = elementSize*GetSize();
  char * newArray = PAbstractArrayAllocate(sizebytes);
  if (newArray == NULL)
    reference->size = 0;
  else
    memcpy(newArray, array->theArray, sizebytes);
  theArray = newArray;
  allocatedDynamically = true;
}


void PAbstractArray::PrintOn(ostream & strm) const
{
  char separator = strm.fill();
  int width = (int)strm.width();
  for (PINDEX  i = 0; i < GetSize(); i++) {
    if (i > 0 && separator != '\0')
      strm << separator;
    strm.width(width);
    PrintElementOn(strm, i);
  }
  if (separator == '\n')
    strm << '\n';
}


void PAbstractArray::ReadFrom(istream & strm)
{
  PINDEX i = 0;
  while (strm.good()) {
    ReadElementFrom(strm, i);
    if (!strm.fail())
      i++;
  }
  SetSize(i);
}


PObject::Comparison PAbstractArray::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PAbstractArray), PInvalidCast);
  const PAbstractArray & other = (const PAbstractArray &)obj;

  char * otherArray = other.theArray;
  if (theArray == otherArray)
    return EqualTo;

  if (elementSize < other.elementSize)
    return LessThan;

  if (elementSize > other.elementSize)
    return GreaterThan;

  PINDEX thisSize = GetSize();
  PINDEX otherSize = other.GetSize();

  if (thisSize < otherSize)
    return LessThan;

  if (thisSize > otherSize)
    return GreaterThan;

  if (thisSize == 0)
    return EqualTo;

  int retval = memcmp(theArray, otherArray, elementSize*thisSize);
  if (retval < 0)
    return LessThan;
  if (retval > 0)
    return GreaterThan;
  return EqualTo;
}


PBoolean PAbstractArray::SetSize(PINDEX newSize)
{
  return InternalSetSize(newSize, false);
}


PBoolean PAbstractArray::InternalSetSize(PINDEX newSize, PBoolean force)
{
  if (newSize < 0)
    newSize = 0;

  PINDEX newsizebytes = elementSize*newSize;
  PINDEX oldsizebytes = elementSize*GetSize();

  if (!force && (newsizebytes == oldsizebytes))
    return true;

  char * newArray;

  if (!IsUnique()) {

    if (newsizebytes == 0)
      newArray = NULL;
    else {
      if ((newArray = PAbstractArrayAllocate(newsizebytes)) == NULL)
        return false;
  
      allocatedDynamically = true;

      if (theArray != NULL)
        memcpy(newArray, theArray, PMIN(oldsizebytes, newsizebytes));
    }

    --reference->count;
    reference = new PContainerReference(newSize);

  } else {

    if (theArray != NULL) {
      if (newsizebytes == 0) {
        if (allocatedDynamically)
          PAbstractArrayDeallocate(theArray);
        newArray = NULL;
      }
      else {
        if ((newArray = PAbstractArrayAllocate(newsizebytes)) == NULL)
          return false;
        memcpy(newArray, theArray, PMIN(newsizebytes, oldsizebytes));
        if (allocatedDynamically)
          PAbstractArrayDeallocate(theArray);
        allocatedDynamically = true;
      }
    }
    else if (newsizebytes != 0) {
      if ((newArray = PAbstractArrayAllocate(newsizebytes)) == NULL)
        return false;
    }
    else
      newArray = NULL;

    reference->size = newSize;
  }

  if (newsizebytes > oldsizebytes)
    memset(newArray+oldsizebytes, 0, newsizebytes-oldsizebytes);

  theArray = newArray;
  return true;
}

void PAbstractArray::Attach(const void *buffer, PINDEX bufferSize)
{
  if (allocatedDynamically && theArray != NULL)
    PAbstractArrayDeallocate(theArray);

  theArray = (char *)buffer;
  reference->size = bufferSize;
  allocatedDynamically = false;
}


void * PAbstractArray::GetPointer(PINDEX minSize)
{
  PAssert(SetMinSize(minSize), POutOfMemory);
  return theArray;
}


PINDEX PAbstractArray::GetLength() const
{
  return elementSize*GetSize();
}


PBoolean PAbstractArray::Concatenate(const PAbstractArray & array)
{
  if (!allocatedDynamically || array.elementSize != elementSize)
    return false;

  PINDEX oldLen = GetSize();
  PINDEX addLen = array.GetSize();

  if (!SetSize(oldLen + addLen))
    return false;

  memcpy(theArray+oldLen*elementSize, array.theArray, addLen*elementSize);
  return true;
}


void PAbstractArray::PrintElementOn(ostream & /*stream*/, PINDEX /*index*/) const
{
}


void PAbstractArray::ReadElementFrom(istream & /*stream*/, PINDEX /*index*/)
{
}


///////////////////////////////////////////////////////////////////////////////

void PCharArray::PrintOn(ostream & strm) const
{
  PINDEX width = (int)strm.width();
  if (width > GetSize())
    width -= GetSize();
  else
    width = 0;

  PBoolean left = (strm.flags()&ios::adjustfield) == ios::left;
  if (left)
    strm.write(theArray, GetSize());

  while (width-- > 0)
    strm << (char)strm.fill();

  if (!left)
    strm.write(theArray, GetSize());
}


void PCharArray::ReadFrom(istream &strm)
{
  PINDEX size = 0;
  SetSize(size+100);

  while (strm.good()) {
    strm >> theArray[size++];
    if (size >= GetSize())
      SetSize(size+100);
  }

  SetSize(size);
}


void PBYTEArray::PrintOn(ostream & strm) const
{
  int line_width = (int)strm.width();
  if (line_width == 0)
    line_width = 16;
  strm.width(0);

  int indent = (int)strm.precision();

  int val_width = ((strm.flags()&ios::basefield) == ios::hex) ? 2 : 3;

  ios::fmtflags oldFlags = strm.flags();
  if (strm.fill() == '0')
    strm.setf(ios::right, ios::adjustfield);

  PINDEX i = 0;
  while (i < GetSize()) {
    if (i > 0)
      strm << '\n';
    int j;
    for (j = 0; j < indent; j++)
      strm << ' ';
    for (j = 0; j < line_width; j++) {
      if (indent >= 0 && j == line_width/2)
        strm << ' ';
      if (i+j < GetSize())
        strm << setw(val_width) << (theArray[i+j]&0xff);
      else {
        for (int k = 0; k < val_width; k++)
          strm << ' ';
      }
      if (indent >= 0)
        strm << ' ';
    }
    if ((strm.flags()&ios::floatfield) != ios::fixed) {
      strm << "  ";
      for (j = 0; j < line_width; j++) {
        if (i+j < GetSize()) {
          unsigned val = theArray[i+j]&0xff;
          if (isprint(val))
            strm << (char)val;
          else
            strm << '.';
        }
      }
    }
    i += line_width;
  }

  strm.flags(oldFlags);
}


void PBYTEArray::ReadFrom(istream &strm)
{
  PINDEX size = 0;
  SetSize(size+100);

  while (strm.good()) {
    unsigned v;
    strm >> v;
    theArray[size] = (BYTE)v;
    if (!strm.fail()) {
      size++;
      if (size >= GetSize())
        SetSize(size+100);
    }
  }

  SetSize(size);
}


///////////////////////////////////////////////////////////////////////////////

PBitArray::PBitArray(PINDEX initialSize)
  : PBYTEArray((initialSize+7)>>3)
{
}


PBitArray::PBitArray(const void * buffer,
                     PINDEX length,
                     PBoolean dynamic)
  : PBYTEArray((const BYTE *)buffer, (length+7)>>3, dynamic)
{
}


PObject * PBitArray::Clone() const
{
  return new PBitArray(*this);
}


PINDEX PBitArray::GetSize() const
{
  return PBYTEArray::GetSize()<<3;
}


PBoolean PBitArray::SetSize(PINDEX newSize)
{
  return PBYTEArray::SetSize((newSize+7)>>3);
}


PBoolean PBitArray::SetAt(PINDEX index, PBoolean val)
{
  if (!SetMinSize(index+1))
    return false;

  if (val)
    theArray[index>>3] |= (1 << (index&7));
  else
    theArray[index>>3] &= ~(1 << (index&7));
  return true;
}


PBoolean PBitArray::GetAt(PINDEX index) const
{
  PASSERTINDEX(index);
  if (index >= GetSize())
    return false;

  return (theArray[index>>3]&(1 << (index&7))) != 0;
}


void PBitArray::Attach(const void * buffer, PINDEX bufferSize)
{
  PBYTEArray::Attach((const BYTE *)buffer, (bufferSize+7)>>3);
}


BYTE * PBitArray::GetPointer(PINDEX minSize)
{
  return PBYTEArray::GetPointer((minSize+7)>>3);
}


PBoolean PBitArray::Concatenate(const PBitArray & array)
{
  return PAbstractArray::Concatenate(array);
}


///////////////////////////////////////////////////////////////////////////////

PString::PString()
  : PCharArray(1)
  , m_length(0)
{
}


PString::PString(const PString & str)
  : PCharArray(str)
  , m_length(str.GetLength())
{
}


PString::PString(const PCharArray & buf)
  : PCharArray(buf)
  , m_length(strlen(buf))
{
}


PString::PString(const PBYTEArray & buf)
  : m_length(0)
{
  PINDEX bufSize = buf.GetSize();
  if (bufSize > 0) {
    if (buf[bufSize-1] == '\0')
      --bufSize;
    memcpy(GetPointerAndSetLength(bufSize), buf, bufSize);
  }
}


PString::PString(int, const PString * str)
  : PCharArray(*str)
  , m_length(str->GetLength())
{
}


PString::PString(const std::string & str)
  : PCharArray(str.c_str(), str.length()+1)
  , m_length(str.length())
{
}


PString::PString(char c)
  : PCharArray(2)
  , m_length(1)
{
  *theArray = c;
}


const PString & PString::Empty()
{
  static int EmptyStringMemory[(sizeof(PConstString)+sizeof(int)-1)/sizeof(int)];
#undef new
  static PConstString const * EmptyString = new (EmptyStringMemory) PConstString("");
#define new PNEW
  return *EmptyString;
}


PString::PString(const char * cstr)
{
  if (cstr == NULL)
    MakeEmpty();
  else {
    m_length = (PINDEX)strlen(cstr);
    if (SetSize(m_length+1) && m_length > 0)
      memcpy(theArray, cstr, m_length);
  }
}

#ifdef P_HAS_WCHAR

PString::PString(const wchar_t * ustr)
{
  if (ustr == NULL)
    MakeEmpty();
  else {
    PINDEX len = 0;
    while (ustr[len] != 0)
      len++;
    InternalFromUCS2(ustr, len);
  }
}

PString::PString(const wchar_t * ustr, PINDEX len)
{
  InternalFromUCS2(ustr, len);
}


PString::PString(const PWCharArray & ustr)
{
  PINDEX size = ustr.GetSize();
  if (size > 0 && ustr[size-1] == 0) // Stip off trailing NULL if present
    size--;
  InternalFromUCS2(ustr, size);
}

#endif // P_HAS_WCHAR

PString::PString(const char * cstr, PINDEX len)
  : PCharArray(len+1)
  , m_length(len)
{
  if (len > 0)
    memcpy(theArray, PAssertNULL(cstr), len);
}


static int TranslateHex(char x)
{
  if (x >= 'a')
    return x - 'a' + 10;

  if (x >= 'A')
    return x - 'A' + '\x0a';

  return x - '0';
}


static const unsigned char PStringEscapeCode[]  = {  'a',  'b',  'f',  'n',  'r',  't',  'v' };
static const unsigned char PStringEscapeValue[] = { '\a', '\b', '\f', '\n', '\r', '\t', '\v' };

static void TranslateEscapes(const char * & src, char * dst)
{
  bool hadLeadingQuote = *src == '"';
  if (hadLeadingQuote)
    src++;

  while (*src != '\0') {
    int c = *src++ & 0xff;
    if (c == '"' && hadLeadingQuote) {
      *dst = '\0'; // Trailing '"' and remaining string is ignored
      break;
    }

    if (c == '\\') {
      c = *src++ & 0xff;
      for (PINDEX i = 0; i < PARRAYSIZE(PStringEscapeCode); i++) {
        if (c == PStringEscapeCode[i])
          c = PStringEscapeValue[i];
      }

      if (c == 'x' && isxdigit(*src & 0xff)) {
        c = TranslateHex(*src++);
        if (isxdigit(*src & 0xff))
          c = (c << 4) + TranslateHex(*src++);
      }
      else if (c >= '0' && c <= '7') {
        int count = c <= '3' ? 3 : 2;
        src--;
        c = 0;
        do {
          c = (c << 3) + *src++ - '0';
        } while (--count > 0 && *src >= '0' && *src <= '7');
      }
    }

    *dst++ = (char)c;
  }
}


PString::PString(ConversionType type, const char * str, ...)
  : PCharArray(1)
  , m_length(0)
{
  switch (type) {
    case Pascal :
      if (*str != '\0') {
        m_length = *str & 0xff;
        PAssert(SetSize(m_length+1), POutOfMemory);
        memcpy(theArray, str+1, m_length);
      }
      break;

    case Basic :
      if (str[0] != '\0' && str[1] != '\0') {
        m_length = (str[0] & 0xff) | ((str[1] & 0xff) << 8);
        PAssert(SetSize(m_length+1), POutOfMemory);
        memcpy(theArray, str+2, m_length);
      }
      break;

    case Literal :
      PAssert(SetSize(strlen(str)+1), POutOfMemory);
      TranslateEscapes(str, theArray);
      m_length = strlen(theArray);
      break;

    case Printf : {
      va_list args;
      va_start(args, str);
      vsprintf(str, args);
      va_end(args);
      break;
    }

    default :
      PAssertAlways(PInvalidParameter);
  }
}


template <typename T> PINDEX p_unsigned2string(T value, unsigned base, char * str)
{
  PINDEX len = value < base ? 0 : p_unsigned2string<T>((T)(value/base), base, str);
  value %= base;
  str[len] = (char)(value < 10 ? (value + '0') : (value + 'A'-10));
  return len+1;
}


template <typename S, typename U> PINDEX p_signed2string(S value, unsigned base, char * str)
{
  if (value >= 0)
    return p_unsigned2string<U>(value, base, str);

  *str++ = '-';
  return p_unsigned2string<U>(-value, base, str);
}


PString::PString(short n)
  : PCharArray(sizeof(short)*3+2)
  , m_length(p_signed2string<signed int, unsigned>(n, 10, theArray))
{
}


PString::PString(unsigned short n)
  : PCharArray(sizeof(unsigned short)*3+1)
  , m_length(p_unsigned2string<unsigned int>(n, 10, theArray))
{
}


PString::PString(int n)
  : PCharArray(sizeof(int)*3+2)
  , m_length(p_signed2string<signed int, unsigned>(n, 10, theArray))
{
}


PString::PString(unsigned int n)
  : PCharArray(sizeof(unsigned int)*3+1)
  , m_length(p_unsigned2string<unsigned int>(n, 10, theArray))
{
}


PString::PString(long n)
  : PCharArray(sizeof(long)*3+2)
  , m_length(p_signed2string<signed long, unsigned long>(n, 10, theArray))
{
}


PString::PString(unsigned long n)
  : PCharArray(sizeof(unsigned long)*3+1)
  , m_length(p_unsigned2string<unsigned long>(n, 10, theArray))
{
}


#ifdef HAVE_LONG_LONG_INT
PString::PString(long long n)
  : PCharArray(sizeof(long long)*3+2)
  , m_length(p_signed2string<signed long long, unsigned long long>(n, 10, theArray))
{
}
#endif


#ifdef HAVE_UNSIGNED_LONG_LONG_INT
PString::PString(unsigned long long n)
  : PCharArray(sizeof(unsigned long long)*3+1)
  , m_length(p_unsigned2string<unsigned long long>(n, 10, theArray))
{
}
#endif


static const char siTable[] = { 'f', 'p', 'n', 'u', 'm', '\0', 'k', 'M', 'G', 'T', 'E' };
static const size_t siZero = sizeof(siTable)/2;

static PINDEX InternalConvertScaleSI(PInt64 value, unsigned param, char * theArray)
{
  // Scale it according to SI multipliers
  if (value > -1000 && value < 1000)
    return p_signed2string<PInt64, PUInt64>(value, 10, theArray);

  if (param > 4)
    param = 4;

  PInt64 absValue = value;
  if (absValue < 0) {
    absValue = -absValue;
    ++param;
  }

  PINDEX length = 0;
  PInt64 multiplier = 1;
  for (size_t i = siZero+1; i < sizeof(siTable); ++i) {
    multiplier *= 1000;
    if (absValue < multiplier*1000) {
      length = p_signed2string<PInt64, PUInt64>(value/multiplier, 10, theArray);
      param -= length;
      if (param > 0 && absValue%multiplier != 0) {
        theArray[length++] = '.';
        do {
          multiplier /= 10;
          theArray[length++] = (absValue/multiplier)%10 + '0';
        } while (--param > 0 && absValue%multiplier != 0);
      }
      theArray[length++] = siTable[i];
      break;
    }
  }

  return length;
}

template <typename S, typename U>
  PINDEX p_convert(PString::ConversionType type, S value, unsigned param, char * theArray)
{
#define GetClass() NULL
  PAssert(param >= 2 && param <= 36, PInvalidParameter);
  switch (type) {
    case PString::Signed :
      return p_signed2string<S, U>(value, param, theArray);

    case PString::Unsigned :
      return p_unsigned2string<U>(value, param, theArray);

    case PString::ScaleSI :
      return InternalConvertScaleSI(value, param, theArray);

    default :
      break;
  }

  PAssertAlways(PInvalidParameter);
  return 0;
#undef GetClass
}

#define PSTRING_CONV_CTOR(paramType, signedType, unsignedType) \
PString::PString(ConversionType type, paramType value, unsigned param) \
  : PCharArray(sizeof(paramType)*3+1) \
{ \
  m_length = p_convert<signedType, unsignedType>(type, value, param, theArray); \
}

PSTRING_CONV_CTOR(unsigned char,  char,   unsigned char);
PSTRING_CONV_CTOR(         short, short,  unsigned short);
PSTRING_CONV_CTOR(unsigned short, short,  unsigned short);
PSTRING_CONV_CTOR(         int,   int,    unsigned int);
PSTRING_CONV_CTOR(unsigned int,   int,    unsigned int);
PSTRING_CONV_CTOR(         long,  long,   unsigned long);
PSTRING_CONV_CTOR(unsigned long,  long,   unsigned long);
#ifdef HAVE_LONG_LONG_INT
PSTRING_CONV_CTOR(         long long, long long, unsigned long long);
#endif
#ifdef HAVE_UNSIGNED_LONG_LONG_INT
PSTRING_CONV_CTOR(unsigned long long, long long, unsigned long long);
#endif


PString::PString(ConversionType type, double value, unsigned places)
  : m_length(0)
{
  switch (type) {
    case Decimal :
      sprintf("%0.*f", (int)places, value);
      break;

    case Exponent :
      sprintf("%0.*e", (int)places, value);
      break;

    case ScaleSI :
      if (value == 0)
        sprintf("%0.*f", (int)places, value);
      else {
        // Scale it according to SI multipliers
        double multiplier = 1e-15;
        double absValue = fabs(value);
        size_t i;
        for (i = 0; i < sizeof(siTable)-1; ++i) {
          double nextMultiplier = multiplier * 1000;
          if (absValue < nextMultiplier)
            break;
          multiplier = nextMultiplier;
        }
        value /= multiplier;
        // Want places to be significant figures
        if (places >= 3 && value >= 100)
          places -= 3;
        else if (places >= 2 && value >= 10)
          places -= 2;
        else if (places >= 1)
          --places;
        sprintf("%0.*f%c", (int)places, value, siTable[i]);
      }
      break;

    default :
      PAssertAlways(PInvalidParameter);
      MakeEmpty();
  }
}


PString & PString::operator=(short n)
{
  SetMinSize(sizeof(short)*3+1);
  m_length = p_signed2string<signed int, unsigned int>(n, 10, theArray);
  return *this;
}


PString & PString::operator=(unsigned short n)
{
  SetMinSize(sizeof(unsigned short)*3+1);
  m_length = p_unsigned2string<unsigned int>(n, 10, theArray);
  return *this;
}


PString & PString::operator=(int n)
{
  SetMinSize(sizeof(int)*3+1);
  m_length = p_signed2string<signed int, unsigned int>(n, 10, theArray);
  return *this;
}


PString & PString::operator=(unsigned int n)
{
  SetMinSize(sizeof(unsigned int)*3+1);
  m_length = p_unsigned2string<unsigned int>(n, 10, theArray);
  return *this;
}


PString & PString::operator=(long n)
{
  SetMinSize(sizeof(long)*3+1);
  m_length = p_signed2string<signed long,  unsigned long>(n, 10, theArray);
  return *this;
}


PString & PString::operator=(unsigned long n)
{
  SetMinSize(sizeof(unsigned long)*3+1);
  m_length = p_unsigned2string<unsigned long>(n, 10, theArray);
  return *this;
}


#ifdef HAVE_LONG_LONG_INT
PString & PString::operator=(long long n)
{
  SetMinSize(sizeof(long long)*3+1);
  m_length = p_signed2string<signed long long, unsigned long long>(n, 10, theArray);
  return *this;
}
#endif


#ifdef HAVE_UNSIGNED_LONG_LONG_INT
PString & PString::operator=(unsigned long long n)
{
  SetMinSize(sizeof(unsigned long long)*3+1);
  m_length = p_unsigned2string<unsigned long long>(n, 10, theArray);
  return *this;
}
#endif


void PString::AssignContents(const PContainer & cont)
{
  PCharArray::AssignContents(cont);
  m_length = ((const PString &)cont).GetLength();
}


PString & PString::MakeEmpty()
{
  AssignContents(Empty());
  return *this;
}


PObject * PString::Clone() const
{
  return new PString(*this);
}


void PString::PrintOn(ostream &strm) const
{
  strm << theArray;
}


void PString::ReadFrom(istream &strm)
{
  PINDEX bump = 16;
  m_length = 0;
  do {
    if (!SetMinSize(m_length + (bump *= 2))) {
      strm.setstate(ios::badbit);
      return;
    }

    strm.clear();
    strm.getline(theArray + m_length, GetSize() - m_length);
    m_length += (PINDEX)strm.gcount();
  } while (strm.fail() && !strm.eof());

  if (m_length > 0 && !strm.eof())
    --m_length; // Allow for extracted '\n'

  if (m_length > 0 && theArray[m_length-1] == '\r')
    theArray[--m_length] = '\0';

  if (GetSize() > m_length*2)
    PAssert(MakeMinimumSize(m_length), POutOfMemory);
}


PObject::Comparison PString::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PString), PInvalidCast);
  return InternalCompare(0, P_MAX_INDEX, ((const PString &)obj).theArray);
}


PINDEX PString::HashFunction() const
{
  /* Hash function from "Data Structures and Algorithm Analysis in C++" by
     Mark Allen Weiss, with limit of only executing over at most the first and
     last characters to increase speed when dealing with large strings. */

  switch (GetLength()) { // Use virtual function so PStringStream recalculates length
    case 0:
      return 0;
    case 1:
      return tolower(theArray[0] & 0xff) % 127;
  }

  static const PINDEX MaxCount = 18; // Make sure big enough to cover whole PGloballyUniqueID::AsString()
  PINDEX count = std::min(m_length / 2, MaxCount);
  PINDEX hash = 0;
  PINDEX i;
  for (i = 0; i < count; i++)
    hash = (hash << 5) ^ tolower(theArray[i] & 0xff) ^ hash;
  for (i = m_length - count - 1; i < m_length; i++)
    hash = (hash << 5) ^ tolower(theArray[i] & 0xff) ^ hash;
  return PABSINDEX(hash) % 127;
}


PBoolean PString::IsEmpty() const
{
  return (theArray == NULL) || (*theArray == '\0');
}


PBoolean PString::SetSize(PINDEX newSize)
{
  if (newSize < 1)
    newSize = 1;

  if (!InternalSetSize(newSize, !IsUnique()))
    return false;

  if (GetLength() >= newSize) {
    m_length = newSize-1;
    theArray[m_length] = '\0';
  }

  return true;
}


PBoolean PString::MakeUnique()
{
  if (IsUnique())
    return true;

  InternalSetSize(GetSize(), true);
  return false;
}


PString PString::operator+(const char * cstr) const
{
  if (cstr == NULL)
    return *this;

  PINDEX olen = GetLength();
  PINDEX alen = strlen(cstr);
  PString str;
  str.m_length = olen + alen;
  str.SetSize(str.m_length+1);
  memmove(str.theArray, theArray, olen);
  memcpy(str.theArray+olen, cstr, alen+1);
  return str;
}


PString PString::operator+(char c) const
{
  PINDEX olen = GetLength();
  PString str;
  str.m_length = olen + 1;
  str.SetSize(str.m_length+1);
  memmove(str.theArray, theArray, olen);
  str.theArray[olen++] = c;
  str.theArray[olen] = '\0';
  return str;
}


PString & PString::operator+=(const char * cstr)
{
  if (cstr == NULL)
    return *this;

  PINDEX olen = GetLength();
  PINDEX alen = strlen(cstr);
  m_length = olen + alen;
  SetMinSize(m_length+1);
  memcpy(theArray+olen, cstr, alen+1);
  return *this;
}


PString & PString::operator+=(char ch)
{
  PINDEX olen = GetLength();
  m_length = olen + 1;
  SetMinSize(m_length+1);
  theArray[olen] = ch;
  theArray[m_length] = '\0';
  return *this;
}


PString PString::operator&(const char * cstr) const
{
  if (cstr == NULL)
    return *this;

  PINDEX alen = strlen(cstr);
  if (alen == 0)
    return *this;

  PINDEX olen = GetLength();
  PString str;
  PINDEX space = olen > 0 && theArray[olen-1]!=' ' && *cstr!=' ' ? 1 : 0;
  str.m_length = olen + space + alen;
  str.SetSize(str.m_length+1);
  memmove(str.theArray, theArray, olen);
  if (space != 0)
    str.theArray[olen] = ' ';
  memcpy(str.theArray+olen+space, cstr, alen+1);
  return str;
}


PString PString::operator&(char c) const
{
  PINDEX olen = GetLength();
  PString str;
  PINDEX space = olen > 0 && theArray[olen-1] != ' ' && c != ' ' ? 1 : 0;
  str.m_length = olen + space + 1;
  str.SetSize(str.m_length+1);
  memmove(str.theArray, theArray, olen);
  if (space != 0)
    str.theArray[olen] = ' ';
  str.theArray[olen+space] = c;
  str.theArray[str.m_length] = '\0';
  return str;
}


PString & PString::operator&=(const char * cstr)
{
  if (cstr == NULL)
    return *this;

  PINDEX alen = strlen(cstr);
  if (alen == 0)
    return *this;

  PINDEX olen = GetLength();
  PINDEX space = olen > 0 && theArray[olen-1]!=' ' && *cstr!=' ' ? 1 : 0;
  m_length = olen + space + alen;
  SetMinSize(m_length+1);
  if (space != 0)
    theArray[olen] = ' ';
  memcpy(theArray+olen+space, cstr, alen+1);
  return *this;
}


PString & PString::operator&=(char ch)
{
  PINDEX olen = GetLength();
  PINDEX space = olen > 0 && theArray[olen-1] != ' ' && ch != ' ' ? 1 : 0;
  m_length = olen + space + 1;
  SetMinSize(m_length+1);
  if (space != 0)
    theArray[olen] = ' ';
  theArray[olen+space] = ch;
  theArray[m_length] = '\0';
  return *this;
}


PString & PString::Delete(PINDEX start, PINDEX len)
{
  if (start < 0 || len < 0)
    return *this;

  MakeUnique();

  PINDEX slen = GetLength();
  if (start > slen)
    return *this;

  if (len >= slen - start) {
    theArray[start] = '\0';
    m_length = start;
  }
  else {
    memmove(theArray+start, theArray+start+len, m_length-start-len+1);
    m_length -= len;
  }

  if (GetSize() > m_length*2)
    PAssert(MakeMinimumSize(m_length), POutOfMemory);
  return *this;
}


PString PString::operator()(PINDEX start, PINDEX end) const
{
  if (end < 0 || start < 0 || end < start)
    return Empty();

  PINDEX len = GetLength();
  if (start > len)
    return Empty();

  if (end >= len) {
    if (start == 0)
      return *this;
    end = len-1;
  }

  return PString(theArray+start, end - start + 1);
}


PString PString::Left(PINDEX len) const
{
  if (len <= 0)
    return Empty();

  if (len >= GetLength())
    return *this;

  return PString(theArray, len);
}


PString PString::Right(PINDEX len) const
{
  if (len <= 0)
    return Empty();

  PINDEX srclen = GetLength();
  if (len >= srclen)
    return *this;

  return PString(theArray+srclen-len, len);
}


PString PString::Mid(PINDEX start, PINDEX len) const
{
  if (len <= 0 || start < 0)
    return Empty();

  if (len == P_MAX_INDEX || start+len < start) // If open ended or check for wraparound
    return operator()(start, P_MAX_INDEX);
  else
    return operator()(start, start+len-1);
}


bool PString::operator*=(const char * cstr) const
{
  if (cstr == NULL)
    return IsEmpty() != false;

  const char * pstr = theArray;
  while (*pstr != '\0' && *cstr != '\0') {
    if (toupper(*pstr & 0xff) != toupper(*cstr & 0xff))
      return false;
    pstr++;
    cstr++;
  }
  return *pstr == *cstr;
}


PObject::Comparison PString::NumCompare(const PString & str, PINDEX count, PINDEX offset) const
{
  if (offset < 0 || count < 0)
    return LessThan;
  PINDEX len = str.GetLength();
  if (count > len)
    count = len;
  return InternalCompare(offset, count, str);
}


PObject::Comparison PString::NumCompare(const char * cstr, PINDEX count, PINDEX offset) const
{
  if (offset < 0 || count < 0)
    return LessThan;
  PINDEX len = ::strlen(cstr);
  if (count > len)
    count = len;
  return InternalCompare(offset, count, cstr);
}


PObject::Comparison PString::InternalCompare(PINDEX offset, char c) const
{
  if (offset < 0)
    return LessThan;
  const int ch = theArray[offset] & 0xff;
  if (ch < (c & 0xff))
    return LessThan;
  if (ch > (c & 0xff))
    return GreaterThan;
  return EqualTo;
}


PObject::Comparison PString::InternalCompare(PINDEX offset, PINDEX length, const char * cstr) const
{
  if (offset < 0 || length < 0)
    return LessThan;

  if (offset == 0 && theArray == cstr)
    return EqualTo;

  if (offset < 0 || cstr == NULL)
    return IsEmpty() ? EqualTo : LessThan;

  int retval;
  if (length == P_MAX_INDEX)
    retval = strcmp(theArray+offset, cstr);
  else
    retval = strncmp(theArray+offset, cstr, length);

  if (retval < 0)
    return LessThan;

  if (retval > 0)
    return GreaterThan;

  return EqualTo;
}


PINDEX PString::Find(char ch, PINDEX offset) const
{
  if (offset < 0)
    return P_MAX_INDEX;

  PINDEX len = GetLength();
  while (offset < len) {
    if (InternalCompare(offset, ch) == EqualTo)
      return offset;
    offset++;
  }
  return P_MAX_INDEX;
}


PINDEX PString::Find(const char * cstr, PINDEX offset) const
{
  if (cstr == NULL || *cstr == '\0' || offset < 0)
    return P_MAX_INDEX;

  PINDEX len = GetLength();
  PINDEX clen = strlen(cstr);
  if (clen > len)
    return P_MAX_INDEX;

  if (offset > len - clen)
    return P_MAX_INDEX;

  if (len - clen < 10) {
    while (offset+clen <= len) {
      if (InternalCompare(offset, clen, cstr) == EqualTo)
        return offset;
      offset++;
    }
    return P_MAX_INDEX;
  }

  int strSum = 0;
  int cstrSum = 0;
  for (PINDEX i = 0; i < clen; i++) {
    strSum += toupper(theArray[offset+i] & 0xff);
    cstrSum += toupper(cstr[i] & 0xff);
  }

  // search for a matching substring
  while (offset+clen <= len) {
    if (strSum == cstrSum && InternalCompare(offset, clen, cstr) == EqualTo)
      return offset;
    strSum += toupper(theArray[offset+clen] & 0xff);
    strSum -= toupper(theArray[offset] & 0xff);
    offset++;
  }

  return P_MAX_INDEX;
}


PINDEX PString::FindLast(char ch, PINDEX offset) const
{
  PINDEX len = GetLength();
  if (len == 0 || offset < 0)
    return P_MAX_INDEX;
  if (offset >= len)
    offset = len-1;

  while (InternalCompare(offset, ch) != EqualTo) {
    if (offset == 0)
      return P_MAX_INDEX;
    offset--;
  }

  return offset;
}


PINDEX PString::FindLast(const char * cstr, PINDEX offset) const
{
  if (cstr == NULL || *cstr == '\0' || offset < 0)
    return P_MAX_INDEX;

  PINDEX len = GetLength();
  PINDEX clen = strlen(cstr);
  if (clen > len)
    return P_MAX_INDEX;

  if (offset > len - clen)
    offset = len - clen;

  int strSum = 0;
  int cstrSum = 0;
  for (PINDEX i = 0; i < clen; i++) {
    strSum += toupper(theArray[offset+i] & 0xff);
    cstrSum += toupper(cstr[i] & 0xff);
  }

  // search for a matching substring
  while (strSum != cstrSum || InternalCompare(offset, clen, cstr) != EqualTo) {
    if (offset == 0)
      return P_MAX_INDEX;
    --offset;
    strSum += toupper(theArray[offset] & 0xff);
    strSum -= toupper(theArray[offset+clen] & 0xff);
  }

  return offset;
}


PINDEX PString::FindOneOf(const char * cset, PINDEX offset) const
{
  if (cset == NULL || *cset == '\0' || offset < 0)
    return P_MAX_INDEX;

  PINDEX len = GetLength();
  while (offset < len) {
    const char * p = cset;
    while (*p != '\0') {
      if (InternalCompare(offset, *p) == EqualTo)
        return offset;
      p++;
    }
    offset++;
  }
  return P_MAX_INDEX;
}


PINDEX PString::FindSpan(const char * cset, PINDEX offset) const
{
  if (cset == NULL || *cset == '\0' || offset < 0)
    return P_MAX_INDEX;

  PINDEX len = GetLength();
  while (offset < len) {
    const char * p = cset;
    while (InternalCompare(offset, *p) != EqualTo) {
      if (*++p == '\0')
        return offset;
    }
    offset++;
  }
  return P_MAX_INDEX;
}


PINDEX PString::FindRegEx(const PRegularExpression & regex, PINDEX offset) const
{
  if (offset < 0)
    return P_MAX_INDEX;

  PINDEX pos = 0;
  PINDEX len = 0;
  if (FindRegEx(regex, pos, len, offset))
    return pos;

  return P_MAX_INDEX;
}


PBoolean PString::FindRegEx(const PRegularExpression & regex,
                        PINDEX & pos,
                        PINDEX & len,
                        PINDEX offset,
                        PINDEX maxPos) const
{
  PINDEX olen = GetLength();
  if (offset < 0 || maxPos < 0 || offset > olen)
    return false;

  if (offset == olen) {
    if (!regex.Execute("", pos, len))
      return false;
  }
  else {
    if (!regex.Execute(&theArray[offset], pos, len))
      return false;
  }

  pos += offset;
  if (pos+len > maxPos)
    return false;

  return true;
}

PBoolean PString::MatchesRegEx(const PRegularExpression & regex) const
{
  PINDEX pos = 0;
  PINDEX len = 0;

  if (!regex.Execute(theArray, pos, len))
    return false;

  return (pos == 0) && (len == GetLength());
}

PString & PString::Replace(const PString & target, const PString & subs, PBoolean all, PINDEX offset)
{
  if (offset < 0)
    return *this;
    
  MakeUnique();

  PINDEX tlen = target.GetLength();
  PINDEX slen = subs.GetLength();
  do {
    PINDEX pos = Find(target, offset);
    if (pos == P_MAX_INDEX)
      return *this;
    Splice(subs, pos, tlen);
    offset = pos + slen;
  } while (all);

  return *this;
}


PString & PString::Splice(const char * cstr, PINDEX pos, PINDEX len)
{
  if (len < 0 || pos < 0)
    return *this;

  PINDEX slen = GetLength();
  if (pos >= slen)
    return operator+=(cstr);

  MakeUnique();

  if (len > slen-pos)
    len = slen-pos;
  PINDEX clen = cstr != NULL ? strlen(cstr) : 0;
  PINDEX newlen = slen-len+clen;
  if (clen > len)
    SetMinSize(newlen+1);
  if (pos+len < slen)
    memmove(theArray+pos+clen, theArray+pos+len, slen-pos-len+1);
  if (clen > 0)
    memcpy(theArray+pos, cstr, clen);
  theArray[newlen] = '\0';
  m_length = newlen;

  return *this;
}


bool PString::InternalSplit(const PString & delimiter, PString & before, PString & after, SplitOptions_Bits options) const
{
  PINDEX pos = Find(delimiter);

  if (pos != P_MAX_INDEX) {
    before = (options&SplitTrimBefore) ? Left(pos).Trim() : Left(pos);
    pos += delimiter.GetLength();
    after = (options&SplitTrimAfter) ? Mid(pos).Trim() : Mid(pos);
  }
  else {
    if (!(options&(SplitDefaultToBefore|SplitDefaultToAfter)))
      return false;

    if (options&SplitDefaultToBefore)
      before = (options&SplitTrimBefore) ? Trim() : *this;

    if (options&SplitDefaultToAfter)
      after = (options&SplitTrimBefore) ? Trim() : *this;
  }

  if (before.IsEmpty() && (options&SplitBeforeNonEmpty))
    return false;

  if (after.IsEmpty() && (options&SplitAfterNonEmpty))
    return false;

  return true;
}


PStringArray PString::Tokenise(const char * separators, PBoolean onePerSeparator) const
{
  PStringArray tokens;
  
  if (separators == NULL || IsEmpty())  // No tokens
    return tokens;
    
  PINDEX token = 0;
  PINDEX p1 = 0;
  PINDEX p2 = FindOneOf(separators);

  if (p2 == 0) {
    if (onePerSeparator) { // first character is a token separator
      tokens[token] = Empty();
      token++;                        // make first string in array empty
      p1 = 1;
      p2 = FindOneOf(separators, 1);
    }
    else {
      do {
        p1 = p2 + 1;
      } while ((p2 = FindOneOf(separators, p1)) == p1);
    }
  }

  while (p2 != P_MAX_INDEX) {
    if (p2 > p1)
      tokens[token] = operator()(p1, p2-1);
    else
      tokens[token] = Empty();
    token++;

    // Get next separator. If not one token per separator then continue
    // around loop to skip over all the consecutive separators.
    do {
      p1 = p2 + 1;
    } while ((p2 = FindOneOf(separators, p1)) == p1 && !onePerSeparator);
  }

  tokens[token] = operator()(p1, P_MAX_INDEX);

  return tokens;
}


PStringArray PString::Lines() const
{
  PStringArray lines;
  
  if (IsEmpty())
    return lines;
    
  PINDEX line = 0;
  PINDEX p1 = 0;
  PINDEX p2;
  while ((p2 = FindOneOf("\r\n", p1)) != P_MAX_INDEX) {
    lines[line++] = operator()(p1, p2-1);
    p1 = p2 + 1;
    if (theArray[p2] == '\r' && theArray[p1] == '\n') // CR LF pair
      p1++;
  }
  if (p1 < GetLength())
    lines[line] = operator()(p1, P_MAX_INDEX);
  return lines;
}


PString PString::LeftTrim() const
{
  if (IsEmpty())
    return PString::Empty();

  const char * lpos = theArray;
  while (isspace(*lpos & 0xff))
    lpos++;
  return PString(lpos);
}


PString PString::RightTrim() const
{
  if (IsEmpty())
    return PString::Empty();

  char * rpos = theArray+GetLength()-1;
  if (!isspace(*rpos & 0xff))
    return *this;

  while (isspace(*rpos & 0xff)) {
    if (rpos == theArray)
      return Empty();
    rpos--;
  }

  // make Apple & Tornado gnu compiler happy
  PString retval(theArray, rpos - theArray + 1);
  return retval;
}


PString PString::Trim() const
{
  if (IsEmpty())
    return PString::Empty();

  const char * lpos = theArray;
  while (isspace(*lpos & 0xff))
    lpos++;
  if (*lpos == '\0')
    return Empty();

  const char * rpos = theArray+GetLength()-1;
  if (!isspace(*rpos & 0xff)) {
    if (lpos == theArray)
      return *this;
    else
      return PString(lpos);
  }

  while (isspace(*rpos & 0xff))
    rpos--;
  return PString(lpos, rpos - lpos + 1);
}


PString PString::ToLower() const
{
  PString newStr(theArray);
  for (char *cpos = newStr.theArray; *cpos != '\0'; cpos++) {
    if (isupper(*cpos & 0xff))
      *cpos = (char)tolower(*cpos & 0xff);
  }
  return newStr;
}


PString PString::ToUpper() const
{
  PString newStr(theArray);
  for (char *cpos = newStr.theArray; *cpos != '\0'; cpos++) {
    if (islower(*cpos & 0xff))
      *cpos = (char)toupper(*cpos & 0xff);
  }
  return newStr;
}


long PString::AsInteger(unsigned base) const
{
  PAssert(base >= 2 && base <= 36, PInvalidParameter);
  char * dummy;
  return strtol(theArray, &dummy, base);
}


DWORD PString::AsUnsigned(unsigned base) const
{
  PAssert(base >= 2 && base <= 36, PInvalidParameter);
  char * dummy;
  return strtoul(theArray, &dummy, base);
}


double PString::AsReal() const
{
#ifndef __HAS_NO_FLOAT
  char * dummy;
  return strtod(theArray, &dummy);
#else
  return 0.0;
#endif
}


#ifdef P_HAS_WCHAR

PWCharArray PString::AsUCS2() const
{
  PWCharArray ucs2(1); // Null terminated empty string

  if (IsEmpty())
    return ucs2;

#ifdef P_HAS_G_CONVERT

  gsize g_len = 0;
  gchar * g_ucs2 = g_convert(theArray, GetSize()-1, "UCS-2", "UTF-8", 0, &g_len, 0);
  if (g_ucs2 != NULL) {
    if (ucs2.SetSize(g_len+1))
      memcpy(ucs2.GetPointer(), g_ucs2, g_len*2);
    g_free(g_ucs2);
    return ucs2;
  }

  PTRACE(1, "PTLib\tg_convert failed with error " << errno);

#elif defined(_WIN32)

  // Note that MB_ERR_INVALID_CHARS is the only dwFlags value supported by Code page 65001 (UTF-8). Windows XP and later.
  PINDEX count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, theArray, GetLength(), NULL, 0);
  if (count > 0 && ucs2.SetSize(count+1)) { // Allow for trailing NULL
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, theArray, GetLength(), ucs2.GetPointer(), ucs2.GetSize());
    return ucs2;
  }

#if PTRACING
  if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
    PTRACE(1, "PTLib\tMultiByteToWideChar failed on non legal UTF-8 \"" << theArray << '"');
  else
    PTRACE(1, "PTLib\tMultiByteToWideChar failed with error " << ::GetLastError());
#endif

#endif

  if (ucs2.SetSize(GetSize())) { // Will be at least this big
    PINDEX count = 0;
    PINDEX i = 0;
    PINDEX length = GetLength()+1; // Include the trailing '\0'
    while (i < length) {
      int c = theArray[i];
      if ((c&0x80) == 0)
        ucs2[count++] = (BYTE)theArray[i++];
      else if ((c&0xe0) == 0xc0) {
        if (i < length-1)
          ucs2[count++] = (WORD)(((theArray[i  ]&0x1f)<<6)|
                                  (theArray[i+1]&0x3f));
        i += 2;
      }
      else if ((c&0xf0) == 0xe0) {
        if (i < length-2)
          ucs2[count++] = (WORD)(((theArray[i  ]&0x0f)<<12)|
                                 ((theArray[i+1]&0x3f)<< 6)|
                                  (theArray[i+2]&0x3f));
        i += 3;
      }
      else {
        if ((c&0xf8) == 0xf0)
          i += 4;
        else if ((c&0xfc) == 0xf8)
          i += 5;
        else
          i += 6;
        if (i <= length)
          ucs2[count++] = 0xffff;
      }
    }

    ucs2.SetSize(count);  // Final size
  }

  return ucs2;
}


void PString::InternalFromUCS2(const wchar_t * ptr, PINDEX len)
{
  if (ptr == NULL || len <= 0) {
    MakeEmpty();
    return;
  }

#ifdef P_HAS_G_CONVERT

  gsize g_len = 0;
  gchar * g_utf8 = g_convert(ptr, len, "UTF-8", "UCS-2", 0, &g_len, 0);
  if (g_utf8 == NULL) {
    MakeEmpty();
    return;
  }

  m_length = g_len;
  if (SetSize(m_length+1))
    memcpy(theArray, g_char, g_len);
  g_free(g_utf8);

#elif defined(_WIN32)

  m_length = WideCharToMultiByte(CP_UTF8, 0, ptr, len, NULL, 0, NULL, NULL);
  if (SetSize(m_length+1))
    WideCharToMultiByte(CP_UTF8, 0, ptr, len, theArray, GetSize(), NULL, NULL);

#else

  PINDEX i;
  PINDEX count = 0;
  for (i = 0; i < len; i++) {
    if (ptr[i] < 0x80)
      count++;
    else if (ptr[i] < 0x800)
      count += 2;
    else
      count += 3;
  }

  m_length = count;
  if (SetSize(m_length+1)) {
    count = 0;
    for (i = 0; i < len; i++) {
      unsigned v = *ptr++;
      if (v < 0x80)
        theArray[count++] = (char)v;
      else if (v < 0x800) {
        theArray[count++] = (char)(0xc0+(v>>6));
        theArray[count++] = (char)(0x80+(v&0x3f));
      }
      else {
        theArray[count++] = (char)(0xd0+(v>>12));
        theArray[count++] = (char)(0x80+((v>>6)&0x3f));
        theArray[count++] = (char)(0x80+(v&0x3f));
      }
    }
  }

#endif
}

#endif // P_HAS_WCHAR


PBYTEArray PString::ToPascal() const
{
  PAssert(m_length < 256, "Cannot convert to PASCAL string");
  BYTE buf[256];
  buf[0] = (BYTE)m_length;
  memcpy(&buf[1], theArray, m_length);
  return PBYTEArray(buf, m_length);
}


PString PString::ToLiteral() const
{
  PString str('"');
  for (char * p = theArray; *p != '\0'; p++) {
    if (*p == '"')
      str += "\\\"";
    else if (*p == '\\')
      str += "\\\\";
    else if (isprint(*p & 0xff))
      str += *p;
    else {
      PINDEX i;
      for (i = 0; i < PARRAYSIZE(PStringEscapeValue); i++) {
        if (*p == PStringEscapeValue[i]) {
          str += PString('\\') + (char)PStringEscapeCode[i];
          break;
        }
      }
      if (i >= PARRAYSIZE(PStringEscapeValue))
        str.sprintf("\\%03o", *p & 0xff);
    }
  }
  return str + '"';
}


PString PString::FromLiteral(PINDEX & offset) const
{
  if (offset >= GetLength())
    return PString::Empty();

  PString str;
  str.SetSize(GetLength()-offset);
  const char * cstr = theArray+offset;
  TranslateEscapes(cstr, str.theArray);
  str.MakeMinimumSize();
  offset = cstr - theArray;

  return str;
}


PString & PString::sprintf(const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  return vsprintf(fmt, args);
}

#if defined(__GNUC__) || defined(__SUNPRO_CC)
#define _vsnprintf vsnprintf
#endif

PString & PString::vsprintf(const char * fmt, va_list arg)
{
  PINDEX len = theArray != NULL ? GetLength() : 0;
#ifdef P_VXWORKS
  // The library provided with tornado 2.0 does not have the implementation
  // for vsnprintf
  // as workaround, just use a array size of 2000
  PAssert(SetMinSize(2000), POutOfMemory);
  m_length = ::vsprintf(theArray+len, fmt, arg);
#else
  int providedSpace = 0;
  int requiredSpace;
  do {
    providedSpace += 1000;
    PAssert(SetSize(providedSpace+len), POutOfMemory);
    requiredSpace = _vsnprintf(theArray+len, providedSpace, fmt, arg);
  } while (requiredSpace == -1 || requiredSpace >= providedSpace);
  m_length += requiredSpace;
#endif // P_VXWORKS

  if (GetSize() > m_length*2)
    PAssert(MakeMinimumSize(m_length), POutOfMemory);
  return *this;
}


PString psprintf(const char * fmt, ...)
{
  PString str;
  va_list args;
  va_start(args, fmt);
  return str.vsprintf(fmt, args);
}


PString pvsprintf(const char * fmt, va_list arg)
{
  PString str;
  return str.vsprintf(fmt, arg);
}


PBoolean PString::MakeMinimumSize(PINDEX newLength)
{
  if (newLength == 0 && theArray != NULL)
    newLength = strlen(theArray);
  GetPointerAndSetLength(newLength);
  return SetSize(m_length+1);
}


char * PString::GetPointerAndSetLength(PINDEX len)
{
  if (!SetMinSize(len+1))
    return NULL;

  m_length = len;
  theArray[len] = '\0';
  return theArray;
}


///////////////////////////////////////////////////////////////////////////////

PObject * PCaselessString::Clone() const
{
  return new PCaselessString(*this);
}


PObject::Comparison PCaselessString::InternalCompare(PINDEX offset, char c) const
{
  if (offset < 0)
    return LessThan;

  int c1 = toupper(theArray[offset] & 0xff);
  int c2 = toupper(c & 0xff);
  if (c1 < c2)
    return LessThan;
  if (c1 > c2)
    return GreaterThan;
  return EqualTo;
}


PObject::Comparison PCaselessString::InternalCompare(
                         PINDEX offset, PINDEX length, const char * cstr) const
{
  if (offset < 0 || length < 0)
    return LessThan;

  if (cstr == NULL)
    return IsEmpty() ? EqualTo : LessThan;

  while (length-- > 0 && (theArray[offset] != '\0' || *cstr != '\0')) {
    Comparison c = PCaselessString::InternalCompare(offset++, *cstr++);
    if (c != EqualTo)
      return c;
  }
  return EqualTo;
}



///////////////////////////////////////////////////////////////////////////////

PStringStream::Buffer::Buffer(PStringStream & str, PINDEX size)
  : string(str),
    fixedBufferSize(size != 0)
{
  string.SetMinSize(size > 0 ? size : 256);
  sync();
}


streambuf::int_type PStringStream::Buffer::overflow(int_type c)
{
  if (pptr() >= epptr()) {
    if (fixedBufferSize)
      return EOF;

    size_t gpos = gptr() - eback();
    size_t ppos = pptr() - pbase();
    char * newptr = string.GetPointer(string.GetSize() + 32);
    setp(newptr, newptr + string.GetSize() - 1);
    pbump(ppos);
    setg(newptr, newptr + gpos, newptr + ppos);
  }

  if (c != EOF) {
    *pptr() = (char)c;
    pbump(1);
  }

  return 0;
}


streambuf::int_type PStringStream::Buffer::underflow()
{
  return gptr() >= egptr() ? EOF : *gptr();
}


int PStringStream::Buffer::sync()
{
  size_t len = strlen(string);
  char * base = string.GetPointer(len);
  setg(base, base, base + len);
  setp(base, base + string.GetSize() - 1);
  pbump(len);
  return 0;
}

streambuf::pos_type PStringStream::Buffer::seekoff(streamoff off, ios_base::seekdir dir, ios_base::openmode mode)
{
  streamoff len = (streamoff)strlen(string);
  streamoff gpos = (streamoff)(gptr() - eback());
  streamoff ppos = (streamoff)(pptr() - pbase());
  char * newgptr;
  char * newpptr;
  switch (dir) {
    case ios::beg :
      if (off < 0)
        newpptr = newgptr = eback();
      else if (off >= len)
        newpptr = newgptr = egptr();
      else
        newpptr = newgptr = eback()+off;
      break;

    case ios::cur :
      if (off < -ppos)
        newpptr = eback();
      else if (off >= len-ppos)
        newpptr = epptr();
      else
        newpptr = pptr()+off;
      if (off < -gpos)
        newgptr = eback();
      else if (off >= len-gpos)
        newgptr = egptr();
      else
        newgptr = gptr()+off;
      break;

    case ios::end :
      if (off < -len)
        newpptr = newgptr = eback();
      else if (off >= 0)
        newpptr = newgptr = egptr();
      else
        newpptr = newgptr = egptr()+off;
      break;

    default:
      PAssertAlways2(string.GetClass(), PInvalidParameter);
      return (pos_type)-1;
  }

  if ((mode&ios::in) != 0)
    setg(eback(), newgptr, egptr());

  if ((mode&ios::out) != 0) {
    setp(newpptr, epptr());
    return pptr() - pbase();
  }

  return gptr() - eback();
}


PStringStream::Buffer::pos_type PStringStream::Buffer::seekpos(pos_type pos, ios_base::openmode mode)
{
  return seekoff(pos, ios_base::beg, mode);
}


PStringStream::PStringStream()
  : P_DISABLE_MSVC_WARNINGS(4355, std::iostream(new PStringStream::Buffer(*this, 0)))
{
}


PStringStream::PStringStream(PINDEX fixedBufferSize)
  : P_DISABLE_MSVC_WARNINGS(4355, std::iostream(new PStringStream::Buffer(*this, fixedBufferSize)))
{
}


PStringStream::PStringStream(const PString & str)
  : PString(str)
  , P_DISABLE_MSVC_WARNINGS(4355, std::iostream(new PStringStream::Buffer(*this, 0)))
{
}


PStringStream::PStringStream(const char * cstr)
  : PString(cstr)
  , P_DISABLE_MSVC_WARNINGS(4355, std::iostream(new PStringStream::Buffer(*this, 0)))
{
}


PStringStream::~PStringStream()
{
  PStringStream::Buffer * buf = dynamic_cast<PStringStream::Buffer *>(rdbuf());
#if P_HAS_SET_RDBUF
  set_rdbuf(NULL);
#elif !defined(_MSC_VER)
  init(NULL);
#endif
  delete buf;
}


PString & PStringStream::MakeEmpty()
{
  memset(theArray, 0, GetSize());
  m_length = 0;
  clear();
  flush();
  return *this;
}


PINDEX PStringStream::GetLength() const
{
  if (m_length == 0 || theArray[m_length] != '\0')
    m_length = strlen(theArray);
  return m_length;
}


void PStringStream::AssignContents(const PContainer & cont)
{
  PString::AssignContents(cont);
  clear();
  flush();
}


///////////////////////////////////////////////////////////////////////////////

PStringArray::PStringArray(PINDEX count, char const * const * strarr, PBoolean caseless)
{
  if (count == 0)
    return;

  if (PAssertNULL(strarr) == NULL)
    return;

  if (count == P_MAX_INDEX) {
    count = 0;
    while (strarr[count] != NULL)
      count++;
  }

  SetSize(count);
  for (PINDEX i = 0; i < count; i++) {
    PString * newString;
    if (caseless)
      newString = new PCaselessString(strarr[i]);
    else
      newString = new PString(strarr[i]);
    SetAt(i, newString);
  }
}


PStringArray::PStringArray(const PString & str)
{
  SetSize(1);
  (*theArray)[0] = str.CloneAs<PString>();
}


PStringArray::PStringArray(const char * cstr)
{
  SetSize(1);
  (*theArray)[0] = new PString(cstr);
}


PStringArray::PStringArray(const PStringList & list)
{
  SetSize(list.GetSize());
  PINDEX count = 0;
  for (PStringList::const_iterator it = list.begin(); it != list.end(); ++it)
    (*theArray)[count++] = it->CloneAs<PString>();
}


PStringArray::PStringArray(const PSortedStringList & list)
{
  SetSize(list.GetSize());
  for (PINDEX i = 0; i < list.GetSize(); i++)
    (*theArray)[i] = list[i].CloneAs<PString>();
}


PStringArray::PStringArray(const PStringSet & set)
{
  SetSize(set.GetSize());
  PINDEX count = 0;
  for (PStringSet::const_iterator it = set.begin(); it != set.end(); ++it)
    (*theArray)[count++] = it->CloneAs<PString>();
}


PStringArray & PStringArray::operator+=(const PStringArray & v)
{
  PINDEX i;
  for (i = 0; i < v.GetSize(); i++)
    AppendString(v[i]);

  return *this;
}


void PStringArray::ReadFrom(istream & strm)
{
  while (strm.good()) {
    PString str;
    strm >> str;
    AppendString(str);
  }
}


PString PStringArray::operator[](PINDEX index) const
{
  PASSERTINDEX(index);
  if (index < GetSize() && (*theArray)[index] != NULL)
    return *(PString *)(*theArray)[index];
  return PString::Empty();
}


PString & PStringArray::operator[](PINDEX index)
{
  PASSERTINDEX(index);
  PAssert(SetMinSize(index+1), POutOfMemory);
  if ((*theArray)[index] == NULL)
    (*theArray)[index] = new PString;
  return *(PString *)(*theArray)[index];
}


static void strcpy_with_increment(char * & strPtr, const PString & str)
{
  PINDEX len = str.GetLength()+1;
  memcpy(strPtr, (const char *)str, len);
  strPtr += len;
}

char ** PStringArray::ToCharArray(PCharArray * storage) const
{
  PINDEX i;

  PINDEX mySize = GetSize();
  PINDEX storageSize = (mySize+1)*sizeof(char *);
  for (i = 0; i < mySize; i++)
    storageSize += (*this)[i].GetLength()+1;

  char ** storagePtr;
  if (storage != NULL)
    storagePtr = (char **)storage->GetPointer(storageSize);
  else
    storagePtr = (char **)malloc(storageSize);

  if (storagePtr == NULL)
    return NULL;

  char * strPtr = (char *)&storagePtr[mySize+1];

  for (i = 0; i < mySize; i++) {
    storagePtr[i] = strPtr;
    strcpy_with_increment(strPtr, (*this)[i]);
  }

  storagePtr[i] = NULL;

  return storagePtr;
}


PString PStringArray::ToString(char separator) const
{
  PStringStream str;

  for (PINDEX i = 0; i < GetSize(); ++i) {
    if (i > 0)
      str << separator;
    str << (*this)[i];
  }

  return str;
}


///////////////////////////////////////////////////////////////////////////////

PStringList::PStringList(PINDEX count, char const * const * strarr, PBoolean caseless)
{
  if (count == 0)
    return;

  if (PAssertNULL(strarr) == NULL)
    return;

  for (PINDEX i = 0; i < count; i++) {
    PString * newString;
    if (caseless)
      newString = new PCaselessString(strarr[i]);
    else
      newString = new PString(strarr[i]);
    Append(newString);
  }
}


PStringList::PStringList(const PString & str)
{
  Append(str.Clone());
}


PStringList::PStringList(const char * cstr)
{
  AppendString(cstr);
}


PStringList::PStringList(const PStringArray & array)
{
  for (PINDEX i = 0; i < array.GetSize(); i++)
    Append(array[i].Clone());
}


PStringList::PStringList(const PSortedStringList & list)
{
  for (PSortedStringList::const_iterator it = list.begin(); it != list.end(); ++it)
    Append(it->Clone());
}


PStringList::PStringList(const PStringSet & set)
{
  for (PStringSet::const_iterator it = set.begin(); it != set.end(); ++it)
    Append(it->Clone());
}


PStringList & PStringList::operator += (const PStringList & v)
{
  for (PStringList::const_iterator i = v.begin(); i != v.end(); i++)
    AppendString(*i);

  return *this;
}


void PStringList::ReadFrom(istream & strm)
{
  while (strm.good()) {
    PString str;
    strm >> str;
    AppendString(str);
  }
}


///////////////////////////////////////////////////////////////////////////////

PSortedStringList::PSortedStringList(PINDEX count,
                                     char const * const * strarr,
                                     PBoolean caseless)
{
  if (count == 0)
    return;

  if (PAssertNULL(strarr) == NULL)
    return;

  for (PINDEX i = 0; i < count; i++) {
    PString * newString;
    if (caseless)
      newString = new PCaselessString(strarr[i]);
    else
      newString = new PString(strarr[i]);
    Append(newString);
  }
}


PSortedStringList::PSortedStringList(const PString & str)
{
  Append(str.Clone());
}


PSortedStringList::PSortedStringList(const char * cstr)
{
  AppendString(cstr);
}


PSortedStringList::PSortedStringList(const PStringArray & array)
{
  for (PINDEX i = 0; i < array.GetSize(); i++)
    Append(array[i].Clone());
}


PSortedStringList::PSortedStringList(const PStringList & list)
{
  for (PStringList::const_iterator it = list.begin(); it != list.end(); ++it)
    Append(it->Clone());
}


PSortedStringList::PSortedStringList(const PStringSet & set)
{
  for (PStringSet::const_iterator it = set.begin(); it != set.end(); ++it)
    Append(it->Clone());
}


void PSortedStringList::ReadFrom(istream & strm)
{
  while (strm.good()) {
    PString str;
    strm >> str;
    AppendString(str);
  }
}


PINDEX PSortedStringList::GetNextStringsIndex(const PString & str) const
{
  PINDEX len = str.GetLength();
  PSortedListElement * element;
  PINDEX index = InternalStringSelect(str, len, m_info->m_root, element);

  if (index != 0) {
    PSortedListElement * prev;
    while ((prev = m_info->Predecessor(element)) != &m_info->nil && ((PString *)prev->m_data)->NumCompare(str, len) >= EqualTo) {
      element = prev;
      index--;
    }
  }

  return index;
}


PINDEX PSortedStringList::InternalStringSelect(const char * str,
                                               PINDEX len,
                                               PSortedListElement * thisElement,
                                               PSortedListElement * & selectedElement) const
{
  if (thisElement == &m_info->nil)
    return 0;

  switch (((PString *)thisElement->m_data)->NumCompare(str, len)) {
    case PObject::LessThan :
    {
      PINDEX index = InternalStringSelect(str, len, thisElement->m_right, selectedElement);
      return thisElement->m_left->m_subTreeSize + index + 1;
    }

    case PObject::GreaterThan :
      return InternalStringSelect(str, len, thisElement->m_left, selectedElement);

    default :
      selectedElement = thisElement;
      return thisElement->m_left->m_subTreeSize;
  }
}


///////////////////////////////////////////////////////////////////////////////

PStringSet::PStringSet(PINDEX count, char const * const * strarr, PBoolean caseless)
  : BaseClass(true)
{
  if (count == 0)
    return;

  if (PAssertNULL(strarr) == NULL)
    return;

  for (PINDEX i = 0; i < count; i++) {
    if (caseless)
      Include(PCaselessString(strarr[i]));
    else
      Include(PString(strarr[i]));
  }
}


PStringSet::PStringSet(const PString & str)
  : BaseClass(true)
{
  Include(str);
}


PStringSet::PStringSet(const char * cstr)
  : BaseClass(true)
{
  Include(cstr);
}


PStringSet::PStringSet(const PStringArray & strArray)
  : BaseClass(true)
{
  for (PINDEX i = 0; i < strArray.GetSize(); ++i)
    Include(strArray[i]);
}


PStringSet::PStringSet(const PStringList & strList)
  : BaseClass(true)
{
  for (PStringList::const_iterator it = strList.begin(); it != strList.end(); ++it)
    Include(*it);
}


void PStringSet::ReadFrom(istream & strm)
{
  while (strm.good()) {
    PString str;
    strm >> str;
    Include(str);
  }
}


///////////////////////////////////////////////////////////////////////////////

POrdinalToString::POrdinalToString(PINDEX count, const Initialiser * init)
{
  while (count-- > 0) {
    SetAt(init->key, init->value);
    init++;
  }
}


void POrdinalToString::ReadFrom(istream & strm)
{
  while (strm.good()) {
    POrdinalKey key;
    char equal;
    PString str;
    strm >> key >> ws >> equal >> str;
    if (equal != '=')
      SetAt(key, PString::Empty());
    else
      SetAt(key, str.Mid(equal+1));
  }
}


///////////////////////////////////////////////////////////////////////////////

PStringToOrdinal::PStringToOrdinal(PINDEX count,
                                   const Initialiser * init,
                                   PBoolean caseless)
{
  while (count-- > 0) {
    if (caseless)
      SetAt(PCaselessString(init->key), init->value);
    else
      SetAt(init->key, init->value);
    init++;
  }
}


PStringToOrdinal::PStringToOrdinal(PINDEX count,
                                   const POrdinalToString::Initialiser * init,
                                   PBoolean caseless)
{
  while (count-- > 0) {
    if (caseless)
      SetAt(PCaselessString(init->value), init->key);
    else
      SetAt(init->value, init->key);
    init++;
  }
}


void PStringToOrdinal::ReadFrom(istream & strm)
{
  while (strm.good()) {
    PString str;
    strm >> str;
    PINDEX equal = str.FindLast('=');
    if (equal == P_MAX_INDEX)
      SetAt(str, 0);
    else
      SetAt(str.Left(equal), str.Mid(equal+1).AsInteger());
  }
}


///////////////////////////////////////////////////////////////////////////////

PStringToString::PStringToString(PINDEX count,
                                 const Initialiser * init,
                                 PBoolean caselessKeys,
                                 PBoolean caselessValues)
{
  while (count-- > 0) {
    if (caselessValues)
      if (caselessKeys)
        SetAt(PCaselessString(init->key), PCaselessString(init->value));
      else
        SetAt(init->key, PCaselessString(init->value));
    else
      if (caselessKeys)
        SetAt(PCaselessString(init->key), init->value);
      else
        SetAt(init->key, init->value);
    init++;
  }
}


void PStringToString::ReadFrom(istream & strm)
{
  while (strm.good()) {
    PString str;
    strm >> str;
    if (str.IsEmpty())
      continue;

    PString key, value;
    str.Split('=', key, value, PString::SplitDefaultToBefore);

    PString * ptr = GetAt(key);
    if (ptr != NULL)
      *ptr += '\n' + value;
    else
      SetAt(key, value);
  }
}


void PStringToString::FromString(const PString & str)
{
  RemoveAll();

  PStringStream strm(str);
  strm >> *this;
}


void PStringToString::Merge(const PStringToString & other, MergeAction action)
{
  for (const_iterator it = other.begin(); it != other.end(); ++it) {
    PString * str = GetAt(it->first);
    if (str == NULL || action == e_MergeOverwrite)
      SetAt(it->first, it->second);
    else if (action == e_MergeAppend)
      *str += '\n' + it->second;
  }
}


char ** PStringToString::ToCharArray(bool withEqualSign, PCharArray * storage) const
{
  const_iterator it;

  PINDEX mySize = GetSize();
  PINDEX numPointers = mySize*(withEqualSign ? 1 : 2) + 1;
  PINDEX storageSize = numPointers*sizeof(char *);
  for (it = begin(); it != end(); ++it)
    storageSize += it->first.GetLength()+1 + it->second.GetLength()+1;

  char ** storagePtr;
  if (storage != NULL)
    storagePtr = (char **)storage->GetPointer(storageSize);
  else
    storagePtr = (char **)malloc(storageSize);

  if (storagePtr == NULL)
    return NULL;

  char * strPtr = (char *)&storagePtr[numPointers];
  PINDEX strIndex = 0;

  for (it = begin(); it != end(); ++it) {
    storagePtr[strIndex++] = strPtr;
    if (withEqualSign)
      strcpy_with_increment(strPtr, it->first + '=' + it->second);
    else {
      strcpy_with_increment(strPtr, it->first);
      storagePtr[strIndex++] = strPtr;
      strcpy_with_increment(strPtr, it->second);
    }
  }

  storagePtr[strIndex] = NULL;

  return storagePtr;
}


///////////////////////////////////////////////////////////////////////////////


PString PStringOptions::GetString(const PCaselessString & key, const char * dflt) const
{
  PString * str = PStringToString::GetAt(key);
  return str != NULL ? *str : PString(dflt);
}


bool PStringOptions::GetBoolean(const PCaselessString & key, bool dflt) const
{
  PString * str = PStringToString::GetAt(key);
  if (str == NULL)
    return dflt;

  if (str->IsEmpty() || str->AsUnsigned() != 0)
    return true;

  static char const * const synonymsForTrue[] = { "true", "yes", "enabled" };
  for (PINDEX i = 0; i < PARRAYSIZE(synonymsForTrue); ++i) {
    if (PConstCaselessString(synonymsForTrue[i]).NumCompare(*str) == EqualTo)
      return true;
  }

  return false;
}


long PStringOptions::GetInteger(const PCaselessString & key, long dflt) const
{
  PString * str = PStringToString::GetAt(key);
  return str != NULL ? str->AsInteger() : dflt;
}


void PStringOptions::SetInteger(const PCaselessString & key, long value)
{
  PStringToString::SetAt(key, PString(PString::Signed, value));
}


double PStringOptions::GetReal(const PCaselessString & key, double dflt) const
{
  PString * str = PStringToString::GetAt(key);
  return str != NULL ? str->AsReal() : dflt;
}


void PStringOptions::SetReal(const PCaselessString & key, double value, int decimals)
{
  PStringToString::SetAt(key, PString(decimals < 0 ? PString::Exponent : PString::Decimal, value, decimals));
}


///////////////////////////////////////////////////////////////////////////////

PRegularExpression::PRegularExpression()
  : m_compileOptions(IgnoreCase)
  , m_compiledRegex(NULL)
  , m_lastError(NotCompiled)
{
}


PRegularExpression::PRegularExpression(const PString & pattern, CompileOptions options)
  : m_pattern(pattern)
  , m_compileOptions(options)
  , m_compiledRegex(NULL)
{
  PAssert(InternalCompile(), "Regular expression compile failed: " + GetErrorText());
}


PRegularExpression::PRegularExpression(const char * pattern, CompileOptions options)
  : m_pattern(pattern)
  , m_compileOptions(options)
  , m_compiledRegex(NULL)
{
  PAssert(InternalCompile(), "Regular expression compile failed: " + GetErrorText());
}


PRegularExpression::PRegularExpression(const PRegularExpression & from)
  : m_pattern(from.m_pattern)
  , m_compileOptions(from.m_compileOptions)
  , m_compiledRegex(NULL)
{
  if (m_pattern.IsEmpty())
    m_lastError = NotCompiled;
  else
    PAssert(InternalCompile(), "Regular expression compile failed: " + GetErrorText());
}


PRegularExpression & PRegularExpression::operator=(const PRegularExpression & from)
{
  if (&from != this) {
    m_pattern = from.m_pattern;
    m_compileOptions = from.m_compileOptions;
    PAssert(InternalCompile(), "Regular expression compile failed: " + GetErrorText());
  }

  return *this;
}


PRegularExpression::~PRegularExpression()
{
  InternalClean();
}


void PRegularExpression::InternalClean()
{
  if (m_compiledRegex != NULL) {
    regfree((regex_t*)m_compiledRegex);
    free(m_compiledRegex);
    m_compiledRegex = NULL;
  }
}


void PRegularExpression::PrintOn(ostream &strm) const
{
  strm << m_pattern;
}


PString PRegularExpression::GetErrorText() const
{
  char str[256];
  regerror(m_lastError, (regex_t*)m_compiledRegex, str, sizeof(str));
  return str;
}


bool PRegularExpression::Compile(const PString & pattern, CompileOptions options)
{
  m_pattern = pattern;
  m_compileOptions = options;
  return InternalCompile();
}


bool PRegularExpression::Compile(const char * pattern, CompileOptions options)
{
  m_pattern = pattern;
  m_compileOptions = options;
  return InternalCompile();
}


bool PRegularExpression::InternalCompile()
{
  InternalClean();

  if (m_pattern.IsEmpty()) {
    m_lastError = BadPattern;
    return false;
  }

  m_compiledRegex = malloc(sizeof(regex_t));
  m_lastError = (ErrorCodes)regcomp((regex_t*)m_compiledRegex, m_pattern, m_compileOptions);
  if (m_lastError == NoError)
    return true;

  InternalClean();
  return false;
}


bool PRegularExpression::Execute(const PString & str, PINDEX & start, ExecOptions options) const
{
  PINDEX dummy;
  return Execute((const char *)str, start, dummy, options);
}


bool PRegularExpression::Execute(const PString & str, PINDEX & start, PINDEX & len, ExecOptions options) const
{
  return Execute((const char *)str, start, len, options);
}


bool PRegularExpression::Execute(const char * cstr, PINDEX & start, ExecOptions options) const
{
  PINDEX dummy;
  return Execute(cstr, start, dummy, options);
}


bool PRegularExpression::Execute(const char * cstr, PINDEX & start, PINDEX & len, ExecOptions options) const
{
  if (m_compiledRegex == NULL)
    m_lastError = NotCompiled;

  if (m_lastError != NoError && m_lastError != NoMatch)
    return false;

  regmatch_t match;

  m_lastError = (ErrorCodes)regexec((regex_t*)m_compiledRegex, cstr, 1, &match, options);
  if (m_lastError != NoError)
    return false;

  start = match.rm_so;
  len = match.rm_eo - start;
  return true;
}


bool PRegularExpression::Execute(const PString & str, PIntArray & starts, ExecOptions options) const
{
  PIntArray dummy;
  return Execute((const char *)str, starts, dummy, options);
}


bool PRegularExpression::Execute(const PString & str,
                                 PIntArray & starts,
                                 PIntArray & ends,
                                 ExecOptions options) const
{
  return Execute((const char *)str, starts, ends, options);
}


bool PRegularExpression::Execute(const char * cstr, PIntArray & starts, ExecOptions options) const
{
  PIntArray dummy;
  return Execute(cstr, starts, dummy, options);
}


bool PRegularExpression::Execute(const char * cstr,
                                 PIntArray & starts,
                                 PIntArray & ends,
                                 ExecOptions options) const
{
  if (m_compiledRegex == NULL) {
    m_lastError = NotCompiled;
    return false;
  }

  PINDEX count = starts.GetSize();
  if (count == 0) {
    starts.SetSize(1);
    count = 1;
  }
  ends.SetSize(count);

  regmatch_t * matches = new regmatch_t[count];

  m_lastError = (ErrorCodes)::regexec((regex_t*)m_compiledRegex, cstr, count, matches, options);
  if (m_lastError == NoError) {
    for (PINDEX i = 0; i < count; i++) {
      starts[i] = matches[i].rm_so;
      ends[i] = matches[i].rm_eo;
    }
  }

  delete [] matches;

  return m_lastError == NoError;
}


bool PRegularExpression::Execute(const char * cstr, PStringArray & substring, ExecOptions options) const
{
  if (m_compiledRegex == NULL) {
    m_lastError = NotCompiled;
    return false;
  }

  PINDEX count = substring.GetSize();
  if (count == 0) {
    substring.SetSize(1);
    count = 1;
  }

  regmatch_t * matches = new regmatch_t[count];

  m_lastError = (ErrorCodes)::regexec((regex_t*)m_compiledRegex, cstr, count, matches, options);
  if (m_lastError == NoError) {
    for (PINDEX i = 0; i < count; i++)
      substring[i] = PString(cstr+matches[i].rm_so, matches[i].rm_eo-matches[i].rm_so);
  }

  delete [] matches;

  return m_lastError == NoError;
}


PString PRegularExpression::EscapeString(const PString & str)
{
  PString translated = str;

  PINDEX lastPos = 0;
  PINDEX nextPos;
  while ((nextPos = translated.FindOneOf("\\^$+?*.[]()|{}", lastPos)) != P_MAX_INDEX) {
    translated.Splice("\\", nextPos);
    lastPos = nextPos+2;
  }

  return translated;
}


///////////////////////////////////////////////////////////////////////////////

void PPrintEnum(std::ostream & strm, int value, int begin, int end, char const * const * names)
{
  if (value < begin || value >= end)
    strm << '<' << value << '>';
  else
    strm << names[value-begin];
}


int PReadEnum(std::istream & strm, int begin, int end, char const * const * names, bool matchCase)
{
  char name[100]; // If someone has an enumeration longer than this, it deserves to fail!
  strm >> ws;
  strm.get(name, sizeof(name), ' ');
  if (strm.fail() || strm.bad())
    return end;

  size_t len = strlen(name);
  int match = end;
  for (int value = begin; value < end; ++value) {
    const char * cmp = names[value-begin];
    if ((matchCase ? strncmp(name, cmp, len) : strncasecmp(name, cmp, len)) == 0) {
      if (match < end) {
        match = end; // Not unique for the length
        break;
      }
      match = value;
    }
  }

  if (match < end)
    return match;

  do {
    strm.putback(name[--len]);
  } while (len > 0);

  strm.clear();
  strm.setstate(ios::failbit);

  return end;
}


int PParseEnum(const char * str, int begin, int end, char const * const * names, bool matchCase)
{
  std::stringstream strm;
  strm.str(str);
  return PReadEnum(strm, begin, end, names, matchCase);
}


void PPrintBitwiseEnum(std::ostream & strm, unsigned bits, char const * const * names)
{
  if (bits == 0) {
    strm << *names;
    return;
  }

  ++names;

  std::streamsize width = strm.width();
  if (width > 0) {
    std::streamsize len = 0;
    unsigned bit = 1;
    const char * const * name = names;
    while (*name != NULL) {
      if (bits & bit) {
        if (len > 0)
          ++len;
        len += strlen(*name);
      }
      bit <<= 1;
      ++name;
    }
    if (width > len)
      width -= len;
    else
      width = 0;
    strm.width(0);
  }

  if (width > 0 && (strm.flags()&ios::left) == 0)
    strm << setw(width) << " ";

  bool needSpace = false;
  for (unsigned bit = 1; *names != NULL; bit <<= 1, ++names) {
    if (bits & bit) {
      if (needSpace)
        strm << ' ';
      else
        needSpace = true;
      strm << *names;
    }
  }

  if (width > 0 && (strm.flags()&ios::right) == 0)
    strm << setw(width) << " ";
}


unsigned PReadBitwiseEnum(std::istream & strm, char const * const * names, bool continueOnError)
{
  unsigned bits = 0;
  while ((continueOnError || strm.good()) && !strm.eof()) {
    char name[100]; // If someone has an enumeration longer than this, it deserves to fail!
    strm >> ws;
    strm.get(name, sizeof(name), ' ');
    if (strm.fail() || strm.bad())
      break;

    if (strcmp(name, *names) == 0)
      return 0;

    bool unknown = true;
    for (unsigned i = 1; names[i] != NULL; ++i) {
      if (strcmp(name, names[i]) == 0) {
        bits |= (1 << (i-1));
        unknown = false;
        break;
      }
    }

    if (continueOnError)
      continue;

    if (unknown) {
      size_t i = strlen(name);
      do {
        strm.putback(name[--i]);
      } while (i > 0);

      break;
    }
  }
  return bits;
}


// End Of File ///////////////////////////////////////////////////////////////
