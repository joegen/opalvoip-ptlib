/*
 * $Id: contain.cxx,v 1.21 1994/07/27 05:58:07 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Class Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: contain.cxx,v $
 * Revision 1.21  1994/07/27 05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.20  1994/07/25  03:38:38  robertj
 * Added more memory tests.
 *
 * Revision 1.19  1994/07/17  10:46:06  robertj
 * Added number conversions to PString.
 *
 * Revision 1.18  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.17  1994/04/20  12:17:44  robertj
 * assert changes
 *
 * Revision 1.16  1994/04/11  12:08:37  robertj
 * Fixed bug in memory leak hash table hash function, cant have negative numbers.
 *
 * Revision 1.15  1994/04/03  08:34:18  robertj
 * Added help and focus functionality.
 *
 * Revision 1.14  1994/04/01  14:01:11  robertj
 * Streams and stuff.
 *
 * Revision 1.13  1994/03/07  07:47:00  robertj
 * Major upgrade
 *
 * Revision 1.12  1994/01/15  03:14:22  robertj
 * Mac portability problems.
 *
 * Revision 1.11  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.10  1993/12/31  06:53:02  robertj
 * Made inlines optional for debugging purposes.
 *
 * Revision 1.9  1993/12/24  04:20:52  robertj
 * Mac CFront port.
 *
 * Revision 1.8  1993/12/16  00:51:46  robertj
 * Made some container functions const.
 *
 * Revision 1.7  1993/12/15  21:10:10  robertj
 * Fixed reference system used by container classes.
 * Plugged memory leaks in PList and PSortedList.
 *
 * Revision 1.6  1993/12/14  18:44:56  robertj
 * Added RemoveAll() function to collections.
 * Fixed bug in list processing when being destroyed (removes the item being
 *     deleted from the list before deleting it).
 * Changed GetIndex() so does not assert if entry not in collection.
 *
 * Revision 1.5  1993/12/04  05:22:38  robertj
 * Added more string functions.
 *
 * Revision 1.4  1993/09/27  16:35:25  robertj
 * Fixed bugs in sorted list.
 * Fixed compatibility problem with sprintf return value (SVR4).
 * Change function for making string array to a constructor.
 *
 * Revision 1.3  1993/08/27  18:17:47  robertj
 * Fixed bugs in PAbstractSortedList (including some formatting).
 *
 * Revision 1.2  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.8  1993/08/01  14:05:27  robertj
 * Added const to ToLower() and ToUpper() in the PString class.
 *
 * Revision 1.7  1993/07/16  14:40:55  robertj
 * Added PString constructor for individual characters.
 * Added string to C style literal format.
 *
 * Revision 1.6  1993/07/15  05:02:57  robertj
 * Removed redundant word in PString enum for string types.
 *
 * Revision 1.5  1993/07/15  04:29:39  robertj
 * Added new constructor to convert from other string formats.
 * Fixed sprintf variable parameter list bug.
 *
 * Revision 1.4  1993/07/14  12:41:52  robertj
 * Fixed comment leader.
 *
 * Revision 1.3  1993/07/14  02:06:34  robertj
 * Fixed header comment for RCS.
 */

#define _CONTAIN_CXX
#include <contain.h>

#include <ctype.h>
#include <stdlib.h>
#include <iomanip.h>

#if !defined(P_USE_INLINES)
#include "contain.inl"
#endif


void PAssertFunc(const char * file, int line, PStandardAssertMessage msg)
{
  static const char * const textmsg[PMaxStandardAssertMessage] = {
    NULL,
    "Out of memory",
    "Null pointer reference",
    "Invalid array index",
    "Invalid array element",
    "Stack empty",
    "Unimplemented function",
    "Invalid parameter",
    "Operating System error",
    "File not open",
    "Unsupported feature",
    "Invalid or closed operating system window"
  };

  const char * theMsg;
  char msgbuf[20];
  if (msg < PMaxStandardAssertMessage)
    theMsg = textmsg[msg];
  else {
    sprintf(msgbuf, "Error code: %i", msg);
    theMsg = msgbuf;
  }
  PAssertFunc(file, line, theMsg);
}


#ifdef PMEMORY_CHECK

struct PointerHashTableStruct {
  void       * ptr;
  size_t       size;
  const char * fileName;
  int          line;
  const char * className;
};


static const char GuardBytes[] = { '\x55', '\xaa', '\xff', '\xaa', '\x55' };
static const unsigned PointerHashTableSize =
                                        0xffff/sizeof(PointerHashTableStruct);
static PointerHashTableStruct PointerHashTable[PointerHashTableSize];


static PointerHashTableStruct *
                          FindObjectMemoryPointer(void * object, void * search)
{
  int hash = (int)(((((long)object)&0x7fffffff)>>3)%PointerHashTableSize);
  PointerHashTableStruct * forward = &PointerHashTable[hash];
  PointerHashTableStruct * backward = forward;
  static PointerHashTableStruct * const EndPointerHashTable =
                                     &PointerHashTable[PointerHashTableSize-1];

  do {
    if (forward->ptr == search)
      return forward;

    if (forward == EndPointerHashTable)
      forward = PointerHashTable;
    else
      forward++;

    if (backward->ptr == search)
      return backward;

    if (backward == PointerHashTable)
      backward = EndPointerHashTable;
    else
      backward--;
  } while (forward != backward);

  return NULL;
}


void * PObject::AllocateObjectMemory(size_t nSize,
                           const char * file, int line, const char * className)
{
  void * obj = malloc(nSize + sizeof(GuardBytes));
  if (obj == NULL) {
    PAssertAlways(POutOfMemory);
    return NULL;
  }

  memcpy((char *)obj+nSize, GuardBytes, sizeof(GuardBytes));

  PointerHashTableStruct * entry = FindObjectMemoryPointer(obj, NULL);
  if (entry == NULL)
    PAssertAlways("Pointer hash table full.");
  else {
    entry->ptr       = obj;
    entry->size      = nSize;
    entry->fileName  = file;
    entry->line      = line;
    entry->className = className;
  }

  return obj;
}


void PObject::DeallocateObjectMemory(void * ptr, const char * className)
{
  PointerHashTableStruct * entry = FindObjectMemoryPointer(ptr, ptr);
  if (entry == NULL)
    PAssertAlways("Deallocating invalid pointer.");
  else {
    entry->ptr = NULL;

    if (strcmp(entry->className, className) != 0) {
      char buf[200];
      sprintf(buf,
              "Pointer allocated as \"%1.70s\", deallocated as \"%1.70s\".",
              entry->className, className);
      PAssertAlways(buf);
    }
  
    PAssert(memcmp((char *)ptr+entry->size,
               GuardBytes, sizeof(GuardBytes)) == 0, "Heap pointer overrun.");
  
    memset(ptr, 0x55, entry->size);  // Make use of freed pointer noticable
  }

  free(ptr);
}


void PDumpMemoryLeaks()
{
  PError.setf(ios::uppercase);
  PointerHashTableStruct * entry = PointerHashTable;
  for (int i = 0; i < PointerHashTableSize; i++, entry++) {
    if (entry->ptr != NULL) {
      PError << entry->fileName << '(' << entry->line << ") : memory leak : '"
             << entry->className << "' @"
             << resetiosflags(ios::basefield) << setiosflags(ios::hex)
             << setw(8) << setfill('0') << (long)(entry->ptr)
             << resetiosflags(ios::basefield) << setiosflags(ios::dec)
             << setw(1) << " (" << entry->size << ')' << endl;
    }
  }
}


#endif


PObject * PObject::Clone() const
{
  PAssertAlways(PUnimplementedFunction);
  return NULL;
}


PObject::Comparison PObject::Compare(const PObject & obj) const
{
  return (Comparison)CompareObjectMemoryDirect(obj);
}


ostream & PObject::PrintOn(ostream & strm) const
{
  return strm << GetClass();
}


istream & PObject::ReadFrom(istream & strm)
{
  return strm;
}


PINDEX PObject::HashFunction() const
{
  return 0;
}


///////////////////////////////////////////////////////////////////////////////

PContainer::PContainer(PINDEX initialSize)
{
  reference = new Reference(initialSize);
  PAssertNULL(reference);
}


PContainer::PContainer(const PContainer * cont)
{
  reference = new Reference(0);
  *PAssertNULL(reference) = *cont->reference;
}


PContainer::PContainer(const PContainer & cont)
{                                                            
  reference = PAssertNULL(cont.reference);
  reference->count++;
}


PContainer & PContainer::operator=(const PContainer & cont)
{
  if (reference != cont.reference) {
    if (!IsUnique())
      reference->count--;
    else {
      DestroyContents();
      delete reference;
    }
  
    reference = PAssertNULL(cont.reference);
    reference->count++;
  }
  return *this;
}


void PContainer::Destruct()
{
  if (reference != NULL) {
    if (reference->count > 1)
      reference->count--;
    else {
      DestroyContents();
      delete reference;
    }
    reference = NULL;
  }
}


///////////////////////////////////////////////////////////////////////////////

PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes, PINDEX initialSize)
  : PContainer(initialSize)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0, PInvalidParameter);
  if (GetSize() == 0)
    reference->size = 1;
  theArray = (char *)calloc(GetSize(), elementSize);
}


PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes,
                               const void *buffer,
                               PINDEX bufferSizeInElements)
  : PContainer(bufferSizeInElements)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0, PInvalidParameter);
  if (GetSize() == 0)
    reference->size = 1;
  PINDEX sizebytes = elementSize*GetSize();
  theArray = (char *)malloc(sizebytes);
  memcpy(theArray, PAssertNULL(buffer), sizebytes);
}


void PAbstractArray::DestroyContents()
{
#ifdef PMEMORY_CHECK
  memset(theArray, 0x55, elementSize*GetSize());
#endif
  free(theArray);
  theArray = NULL;
}


void PAbstractArray::CopyContents(const PAbstractArray & array)
{
  elementSize = array.elementSize;
  theArray = array.theArray;
}


void PAbstractArray::CloneContents(const PAbstractArray * array)
{
  elementSize = array->elementSize;
  theArray = array->theArray;
  MakeUnique();
}


PObject::Comparison PAbstractArray::Compare(const PObject & obj) const
{
  const PAbstractArray & array = (const PAbstractArray &)obj;

  if (elementSize < array.elementSize)
    return LessThan;

  if (elementSize > array.elementSize)
    return GreaterThan;

  PINDEX thisSize = GetSize();
  PINDEX arraySize = array.GetSize();

  if (thisSize < arraySize)
    return LessThan;

  if (thisSize > arraySize)
    return GreaterThan;

  char * p2 = array.theArray;
  PINDEX len = elementSize*thisSize;
  return (Comparison)memcmp(theArray, p2, len);
}


BOOL PAbstractArray::SetSize(PINDEX newSize)
{
  if (newSize == 0)
    newSize = 1;

  PINDEX newsizebytes = elementSize*newSize;
  PINDEX oldsizebytes = elementSize*GetSize();
  char * newArray;

  if (IsUnique()) {
    if ((newArray = (char *)realloc(theArray, newsizebytes)) == NULL)
      return FALSE;
    reference->size = newSize;
  }
  else {
    if ((newArray = (char *)malloc(newsizebytes)) == NULL)
      return FALSE;

    memcpy(newArray, theArray, PMIN(oldsizebytes, newsizebytes));

    reference->count--;
    reference = new Reference(newSize);
  }

  if (newsizebytes > oldsizebytes)
    memset(newArray+oldsizebytes, 0, newsizebytes-oldsizebytes);

  theArray = newArray;
  return TRUE;
}


BOOL PAbstractArray::SetMinSize(PINDEX minSize)
{
  PASSERTINDEX(minSize);
  return minSize <= GetSize() ? MakeUnique() : SetSize(minSize);
}


void * PAbstractArray::GetPointer(PINDEX minSize)
{
  PAssert(SetMinSize(minSize), POutOfMemory);
  return theArray;
}


///////////////////////////////////////////////////////////////////////////////

static int TranslateHex(char x)
{
  if (x >= 'a')
    return x - 'a' + 10;

  if (x >= 'A')
    return x - 'A' + '\x0a';

  return x - '0';
}


static const char PStringEscapeCode[]  = {  'a',  'b',  'f',  'n',  'r',  't',  'v' };
static const char PStringEscapeValue[] = { '\a', '\b', '\f', '\n', '\r', '\t', '\v' };

static void TranslateEscapes(const char * src, char * dst)
{
  if (*src == '"')
    src++;

  while (*src != '\0') {
    int c = *src++;
    if (c == '"' && *src == '\0')
      c  = '\0'; // Trailing '"' is ignored
    else if (c == '\\') {
      c = *src++;
      for (PINDEX i = 0; i < PARRAYSIZE(PStringEscapeCode); i++) {
        if (c == PStringEscapeCode[i])
          c = PStringEscapeValue[i];
      }

      if (c == 'x' && isxdigit(*src)) {
        c = TranslateHex(*src++);
        if (isxdigit(*src))
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


PString::PString(ConversionType type, const char * str)
{
  switch (type) {
    case Pascal :
      if (*str != '\0') {
        PINDEX len = *str & 0xff;
        PAssert(SetSize(len+1), POutOfMemory);
        memcpy(theArray, str+1, len);
      }
      break;

    case Basic :
      if (str[0] != '\0' && str[1] != '\0') {
        PINDEX len = str[0] | (str[1] << 8);
        PAssert(SetSize(len+1), POutOfMemory);
        memcpy(theArray, str+1, len);
      }
      break;

    case Literal :
      PAssert(SetSize(strlen(str)+1), POutOfMemory);
      TranslateEscapes(str, theArray);
      PAssert(MakeMinimumSize(), POutOfMemory);
      break;

    default :
      PAssertAlways(PInvalidParameter);
  }
}


static void ltostr(long value, unsigned base, char * str)
{
  if (value >= (long)base)
    ltostr(value/base, base, str+1);
  else if (value < 10)
    *str = (char)(value + '0');
  else
    *str = (char)(value + 'A'-10);
}


PString::PString(ConversionType type, long value, unsigned base)
  : PCharArray(100)
{
  PAssert(base >= 2 && base <= 36, PInvalidParameter);
  switch (type) {
    case Signed :
      if (value < 0) {
        *theArray = '-';
        ltostr(-value, base, theArray+1);
        break;
      }
      // Otherwise do Unsigned case

    case Unsigned :
      ltostr(value, base, theArray);
      break;

    default :
      PAssertAlways(PInvalidParameter);
  }
  MakeMinimumSize();
}


PString::PString(ConversionType type, double value, unsigned places)
{
  switch (type) {
    case Decimal :
      sprintf("%0.*f", places, value);
      break;

    case Exponent :
      sprintf("%0.*e", places, value);
      break;

    default :
      PAssertAlways(PInvalidParameter);
  }
}


PString & PString::operator=(const char * cstr)
{
  PString pstr(cstr);
  PCharArray::operator=(pstr);
  return *this;
}


PObject * PString::Clone() const
{
  return PNEW PString(*this);
}


ostream & PString::PrintOn(ostream &strm) const
{
  return strm << theArray;
}


istream & PString::ReadFrom(istream &strm)
{
  SetMinSize(100);
  char * ptr = theArray;
  PINDEX len = 0;
  int c;
  while ((c = strm.get()) != EOF && c != '\n') {
    *ptr++ = (char)c;
    len++;
    if (len >= GetSize())
      SetSize(len + 100);
  }
  *ptr = '\0';
  PAssert(MakeMinimumSize(), POutOfMemory);
  return strm;
}


PObject::Comparison PString::Compare(const PObject & obj) const
{
  return CompareString(((const PString &)obj).theArray);
}


PINDEX PString::HashFunction() const
{
  return ((BYTE)theArray[0]+(BYTE)theArray[1]+(BYTE)theArray[2])%23;
}


BOOL PString::IsEmpty() const
{
  return *theArray == '\0';
}


PString PString::operator+(const char * cstr) const
{
  PINDEX olen = GetLength();
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  PString str;
  str.SetSize(olen+alen);
  memcpy(str.theArray, theArray, olen);
  memcpy(str.theArray+olen, cstr, alen);
  return str;
}


PString PString::operator+(char c) const
{
  PINDEX olen = GetLength();
  PString str;
  str.SetSize(olen+2);
  memcpy(str.theArray, theArray, olen);
  str.theArray[olen] = c;
  return str;
}


PString & PString::operator+=(const char * cstr)
{
  PINDEX olen = GetLength();
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  SetSize(olen+alen);
  memcpy(theArray+olen, cstr, alen);
  return *this;
}


void PString::Delete(PINDEX start, PINDEX len)
{
  register PINDEX slen = GetLength();
  if (start > slen)
    return;

  MakeUnique();
  if (start + len > slen)
    theArray[start] = '\0';
  else
    strcpy(theArray+start, theArray+start+len);
  MakeMinimumSize();
}


PString PString::operator()(PINDEX start, PINDEX end) const
{
  if (end < start)
    return PString();

  register PINDEX len = GetLength();
  if (start > len)
    return PString();

  if (end >= len)
    end = len-1;
  len = end - start + 1;

  return PString(theArray+start, len);
}


PString PString::Left(PINDEX len) const
{
  if (len == 0)
    return PString();

  if (len >= GetLength())
    return *this;

  return PString(theArray, len);
}


PString PString::Right(PINDEX len) const
{
  if (len == 0)
    return PString();

  PINDEX srclen = GetLength();
  if (len >= srclen)
    return *this;

  return PString(theArray+srclen-len, len);
}


PString PString::Mid(PINDEX start, PINDEX len) const
{
  if (len == 0)
    return PString();

  if (start+len < start) // Beware of wraparound
    return operator()(start, P_MAX_INDEX);
  else
    return operator()(start, start+len-1);
}


PINDEX PString::Find(char ch, PINDEX offset) const
{
  register PINDEX len = GetLength();
  if (offset > len)
    offset = len;
  char *cpos = strchr(theArray+offset, ch);
  return cpos != NULL ? (int)(cpos - theArray) : P_MAX_INDEX;
}


PINDEX PString::Find(const PString & str, PINDEX offset) const
{
  register PINDEX len = GetLength();
  if (offset > len)
    offset = len;
  char *cpos = strstr(theArray+offset, str.theArray);
  return cpos != NULL ? (int)(cpos - theArray) : P_MAX_INDEX;
}


PINDEX PString::FindLast(char ch, PINDEX offset) const
{
  PINDEX len = GetLength();
  if (offset > len)
    offset = len;

  while (theArray[offset] != ch) {
    if (offset == 0)
      return P_MAX_INDEX;
    offset--;
  }

  return offset;
}


PINDEX PString::FindLast(const PString & str, PINDEX offset) const
{
  char *p1 = strstr(theArray, str.theArray);
  if (p1 == NULL)
    return P_MAX_INDEX;

  PINDEX len = GetLength();
  if (offset > len)
    offset = len;

  char *p2;
  while ((p2 = strstr(p1, str.theArray)) != NULL &&
                                                (int)(p2 - theArray) < offset)
    p1 = p2;
  return (int)(p1 - theArray);
}


PINDEX PString::FindOneOf(const PString & str, PINDEX offset) const
{
  register PINDEX len = GetLength();
  if (offset > len)
    offset = len;
  for (char * cpos = theArray+offset; *cpos != '\0'; cpos++) {
    if (strchr(str.theArray, *cpos) != NULL)
      return (int)(cpos - theArray);
  }
  return P_MAX_INDEX;
}


PStringArray
      PString::Tokenise(const PString & separators, BOOL onePerSeparator) const
{
  PStringArray tokens;
  
  if (IsEmpty())  // No tokens
    return tokens;
    
  PINDEX token = 0;
  PINDEX p1 = 0;
  PINDEX p2 = FindOneOf(separators);

  if (p2 == 0 && onePerSeparator) { // first character is a token separator
    token++;                        // make first string in array empty
    p1 = 1;
    p2 = FindOneOf(separators, 1);
  }

  while (p2 != P_MAX_INDEX) {
    tokens[token++] = operator()(p1, p2-1);

    // Get next separator. If not one token per separator then continue
    // around loop to skip over all the consecutive separators.
    do {
      p1 = p2 + 1;
    } while ((p2 = FindOneOf(separators, p1)) == p1 && !onePerSeparator);
  }

  if (p1 < GetLength() || onePerSeparator)         // Last token (if has one)
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
  lines[line] = operator()(p1, P_MAX_INDEX);
  return lines;
}


PString PString::LeftTrim() const
{
  const char * lpos = theArray;
  while (*lpos == ' ')
    lpos++;
  return PString(lpos);
}


PString PString::RightTrim() const
{
  char * rpos = theArray+GetLength()-1;
  if (*rpos != ' ')
    return *this;

  while (*rpos == ' ') {
    if (rpos == theArray)
      return PString();
    rpos--;
  }
  return PString(theArray, rpos - theArray);
}


PString PString::Trim() const
{
  const char * lpos = theArray;
  while (*lpos == ' ')
    lpos++;
  if (*lpos == '\0')
    return PString();

  const char * rpos = theArray+GetLength()-1;
  if (*rpos != ' ')
    return PString(lpos);

  while (*rpos == ' ')
    rpos--;
  return PString(lpos, rpos - lpos);
}


PString PString::ToLower() const
{
  PString newStr(theArray);
  for (char *cpos = newStr.theArray; *cpos != '\0'; cpos++) {
    if (isupper(*cpos))
      *cpos = (char)tolower(*cpos);
  }
  return newStr;
}


PString PString::ToUpper() const
{
  PString newStr(theArray);
  for (char *cpos = newStr.theArray; *cpos != '\0'; cpos++) {
    if (islower(*cpos))
      *cpos = (char)toupper(*cpos);
  }
  return newStr;
}


long PString::AsInteger(unsigned base) const
{
  PAssert(base >= 2 && base <= 36, PInvalidParameter);
  char * dummy;
  return strtol(theArray, &dummy, base);
}


double PString::AsReal() const
{
  char * dummy;
  return strtod(theArray, &dummy);
}


PString PString::ToPascal() const
{
  PINDEX len = GetLength();
  PAssert(len < 256, "Cannot convert to PASCAL string");
  char buf[256];
  buf[0] = (char)len;
  memcpy(&buf[1], theArray, len);
  return PString(buf, len+1);
}


PString PString::ToLiteral() const
{
  PString str('"');
  for (char * p = theArray; *p != '\0'; p++) {
    if (*p == '"')
      str += "\\\"";
    else if (isprint(*p))
      str += *p;
    else {
      for (PINDEX i = 0; i < PARRAYSIZE(PStringEscapeValue); i++) {
        if (*p == PStringEscapeValue[i]) {
          str += PString('\\') + PStringEscapeCode[i];
          break;
        }
      }
      if (i >= PARRAYSIZE(PStringEscapeValue))
        str += psprintf("\\%03o", *p & 0xff);
    }
  }
  return str + '"';
}


PString & PString::sprintf(const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  return vsprintf(fmt, args);
}


PString & PString::sprintf(const PString & fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  return vsprintf((const char *)fmt, args);
}


PString & PString::vsprintf(const char * fmt, va_list arg)
{
  ::vsprintf(GetPointer(1000), fmt, arg);
  PAssert(strlen(theArray) < 1000, "Single sprintf() too large");
  PAssert(MakeMinimumSize(), POutOfMemory);
  return *this;
}


PString psprintf(const char * fmt, ...)
{
  PString str;
  va_list args;
  va_start(args, fmt);
  return str.vsprintf(fmt, args);
}


PString psprintf(const PString & fmt, ...)
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


///////////////////////////////////////////////////////////////////////////////

PObject::Comparison PCaselessString::CompareString(const char * cstr) const
{
  PAssertNULL(cstr);
  const char * pstr = PAssertNULL(theArray);
  for (;;) {
    int c1 = toupper(*pstr);
    int c2 = toupper(*cstr);
    if (c1 < c2)
      return LessThan;
    if (c1 > c2)
      return GreaterThan;
    if (*pstr == '\0')
      return EqualTo;
    pstr++;
    cstr++;
  }
}


PINDEX PCaselessString::Find(char ch, PINDEX offset) const
{
  register PINDEX len = GetLength();
  if (offset > len)
    offset = len;
  char *cpos = theArray+offset;
  int chu = toupper(ch);
  while (toupper(*cpos) != chu) {
    if (*cpos++ == '\0')
      return P_MAX_INDEX;
  }
  return (int)(cpos - theArray);
}


PINDEX PCaselessString::Find(const char *cstr, PINDEX offset) const
{
  PString str(cstr);
  return ToUpper().Find(str.ToUpper(), offset);
}


PINDEX PCaselessString::FindLast(char ch) const
{
  char *cpos = theArray+GetLength()-1;
  int chu = toupper(ch);
  while (toupper(*cpos) != chu) {
    if (--cpos == theArray)
      return P_MAX_INDEX;
  }
  return (int)(cpos - theArray);
}


PINDEX PCaselessString::FindLast(const char *cstr) const
{
  PString str(cstr);
  return ToUpper().FindLast(str.ToUpper());
}


///////////////////////////////////////////////////////////////////////////////

int PStringStreamBuffer::overflow(int c)
{
  if (pptr() >= epptr()) {
    int gpos = gptr() - eback();
    int ppos = pptr() - pbase();
    char * newptr = string->GetPointer(string->GetSize() + 10);
    setp(newptr + ppos, newptr + string->GetSize() - 1);
    setg(newptr, newptr + gpos, newptr + ppos);
  }
  if (c != EOF) {
    *pptr() = (char)c;
    pbump(1);
  }
  return 0;
}


int PStringStreamBuffer::underflow()
{
  return gptr() >= egptr() ? EOF : *gptr();
}


int PStringStreamBuffer::sync()
{
  char * base = string->GetPointer();
  char * end = base + string->GetLength();
  setg(base, base, end);
  setp(end, base + string->GetSize() - 1);
  return 0;
}


streampos PStringStreamBuffer::seekoff(streamoff off, ios::seek_dir dir, int mode)
{
  int len = string->GetLength();
  int gpos = gptr() - eback();
  int ppos = pptr() - pbase();
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
        newpptr = newpptr = newgptr = eback();
      else if (off >= 0)
        newpptr = newgptr = egptr();
      else
        newpptr = newgptr = egptr()+off;
      break;

    default:
      PAssertAlways(PInvalidParameter);
      newgptr = gptr();
      newpptr = pptr();
  }

  if ((mode&ios::in) != 0)
    setg(eback(), newgptr, egptr());

  if ((mode&ios::out) != 0)
    setp(newpptr, epptr());

  return 0;
}


PStringStream::PStringStream()
{
  init(PNEW PStringStreamBuffer(this));
}


PStringStream::PStringStream(const PString & str)
  : PString(str)
{
  init(PNEW PStringStreamBuffer(this));
}


PStringStream::PStringStream(const char * cstr)
  : PString(cstr)
{
  init(PNEW PStringStreamBuffer(this));
}


PStringStream & PStringStream::operator=(const PString & str)
{
  PString::operator=(str);
  rdbuf()->sync();
  return *this;
}


///////////////////////////////////////////////////////////////////////////////

PStringArray::PStringArray(PINDEX count, char **strarr)
{
  PAssertNULL(strarr);
  SetSize(count);
  for (PINDEX i = 0; i < count; i++)
    SetAt(i, PNEW PString(strarr[i]));
}


// End Of File ///////////////////////////////////////////////////////////////
