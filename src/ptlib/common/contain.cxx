/*
 * $Id: contain.cxx,v 1.5 1993/12/04 05:22:38 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Class Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: contain.cxx,v $
 * Revision 1.5  1993/12/04 05:22:38  robertj
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

#include <contain.h>
#include <ctype.h>
#include <stdlib.h>


PObject * PObject::Clone() const
{
  PAssertAlways();
  return NULL;
}


void * PObject::operator new(size_t nSize)
{
  void * obj = malloc(nSize);
  return PAssertNULL(obj);
}


void PObject::operator delete(void * ptr)
{
  free(ptr);
}


ostream & PObject::PrintOn(ostream & strm) const
{
  return strm << ClassName();
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

PContainer::PContainer(const PContainer & cont)
  : size(cont.size), referenceCount(cont.referenceCount)
{                                                            
  ++*PAssertNULL(referenceCount);
}


PContainer & PContainer::operator=(const PContainer & cont)
{
  if (referenceCount != cont.referenceCount) {
    if (*PAssertNULL(referenceCount) > 1)
      --*referenceCount;
    else {
      DestroyContents();
      delete referenceCount;
    }
  
    referenceCount = cont.referenceCount;
    ++*PAssertNULL(referenceCount);
    size = cont.size;
  }
  return *this;
}


PContainer::~PContainer()
{
  if (--*PAssertNULL(referenceCount) == 0) {
    delete referenceCount;
    referenceCount = NULL;
  }
}


PINDEX PContainer::GetSize() const
{
  return size;
}


BOOL PContainer::IsEmpty() const
{
  return GetSize() == 0;
}



///////////////////////////////////////////////////////////////////////////////

PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes, PINDEX initialSize)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0);
  if ((size = initialSize) == 0)
    size = 1;
  theArray = new char[elementSize*size];
  memset(theArray, 0, elementSize*size);
}


PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes,
                               const void *buffer,
                               PINDEX bufferSizeInElements)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0);
  if ((size = bufferSizeInElements) == 0)
    size = 1;
  theArray = new char[elementSize*size];
  memcpy(theArray, PAssertNULL(buffer), elementSize*size);
}


PAbstractArray::PAbstractArray(const PAbstractArray & array)
  : PContainer(array),
    elementSize(array.elementSize),
    theArray(array.theArray)
{
}


PAbstractArray::PAbstractArray(const PAbstractArray * array)
  : PContainer(*array),
    elementSize(array->elementSize),
    theArray(array->theArray)
{
  MakeUnique();
}


PAbstractArray & PAbstractArray::operator=(const PAbstractArray & array)
{
  PContainer::operator=(array);
  elementSize = array.elementSize;
  theArray = array.theArray;
  return *this;
}


void PAbstractArray::DestroyContents()
{
  if (theArray != NULL && *PAssertNULL(referenceCount) == 1) {
    delete[] theArray;
    theArray = NULL;
  }
}


PObject::Comparison PAbstractArray::Compare(const PObject & obj) const
{
  const PAbstractArray & array = (const PAbstractArray &)obj;
  if (elementSize < array.elementSize)
    return LessThan;
  if (elementSize > array.elementSize)
    return GreaterThan;
  if (size < array.size)
    return LessThan;
  if (size > array.size)
    return GreaterThan;
  return (Comparison)memcmp(theArray, array.theArray, elementSize*size);
}


BOOL PAbstractArray::SetSize(PINDEX newSize)
{
  if (newSize == 0)
    newSize = 1;

  PINDEX newsizebytes = elementSize*newSize;
  char *newArray = new char[newsizebytes];
  if (newArray == NULL)
    return FALSE;

  PINDEX oldsizebytes = elementSize*size;
  if (newsizebytes <= oldsizebytes)
    memcpy(newArray, theArray, newsizebytes);
  else {
    memcpy(newArray, theArray, oldsizebytes);
    memset(newArray+oldsizebytes, 0, newsizebytes-oldsizebytes);
  }

  if (*PAssertNULL(referenceCount) <= 1)
    delete [] theArray;
  else {
    --*referenceCount;
    referenceCount = new unsigned(1);
  }

  theArray = newArray;
  size = newSize;
  return TRUE;
}


BOOL PAbstractArray::SetMinSize(PINDEX minSize)
{
  PASSERTINDEX(minSize);
  return minSize <= size ? MakeUnique() : SetSize(minSize);
}


void * PAbstractArray::GetPointer(PINDEX minSize)
{
  PAssert(SetMinSize(minSize));
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


PString::PString(StringType type, const char * str)
{
  switch (type) {
    case Pascal :
      if (*str != '\0') {
        PINDEX len = *str & 0xff;
        PAssert(SetSize(len+1));
        memcpy(theArray, str+1, len);
      }
      break;

    case Basic :
      if (str[0] != '\0' && str[1] != '\0') {
        PINDEX len = str[0] | (str[1] << 8);
        PAssert(SetSize(len+1));
        memcpy(theArray, str+1, len);
      }
      break;

    case Literal :
      PAssert(SetSize(strlen(str)+1));
      TranslateEscapes(str, theArray);
      PAssert(MakeMinimumSize());
      break;
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
  return new PString(*this);
}


ostream & PString::PrintOn(ostream &strm) const
{
  return strm << theArray;
}


istream & PString::ReadFrom(istream &strm)
{
  PAssert(MakeUnique());
  delete theArray;
  theArray = new char[1000];
  strm >> theArray;
  PAssert(MakeMinimumSize());
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
  PINDEX olen = Length();
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  PString str;
  str.SetSize(olen+alen);
  memcpy(str.theArray, theArray, olen);
  memcpy(str.theArray+olen, cstr, alen);
  return str;
}


PString PString::operator+(char c) const
{
  PINDEX olen = Length();
  PString str;
  str.SetSize(olen+2);
  memcpy(str.theArray, theArray, olen);
  str.theArray[olen] = c;
  return str;
}


PString & PString::operator+=(const char * cstr)
{
  PINDEX olen = Length();
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  SetSize(olen+alen);
  memcpy(theArray+olen, cstr, alen);
  return *this;
}


PString PString::operator()(PINDEX start, PINDEX end) const
{
  if (end < start)
    return PString();

  register PINDEX len = Length();
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

  if (len >= Length())
    return *this;

  return PString(theArray, len);
}


PString PString::Right(PINDEX len) const
{
  if (len == 0)
    return PString();

  PINDEX srclen = Length();
  if (len >= srclen)
    return *this;

  return PString(theArray+srclen-len, len);
}


PString PString::Mid(PINDEX start, PINDEX len) const
{
  if (start+len <= start) // Beware of wraparound
    return operator()(start, P_MAX_INDEX);
  else
    return operator()(start, start+len-1);
}


int PString::Find(char ch, PINDEX offset) const
{
  register PINDEX len = Length();
  if (offset > len)
    offset = len;
  char *cpos = strchr(theArray+offset, ch);
  return cpos != NULL ? (int)(cpos - theArray) : -1;
}


int PString::Find(const PString & str, PINDEX offset) const
{
  register PINDEX len = Length();
  if (offset > len)
    offset = len;
  char *cpos = strstr(theArray+offset, str.theArray);
  return cpos != NULL ? (int)(cpos - theArray) : -1;
}


int PString::FindLast(char ch, PINDEX offset) const
{
  PINDEX len = Length();
  if (offset > len)
    offset = len;
  do {
    if (theArray[offset] == ch)
      return offset;
    offset--;
  } while(offset > 0);
  return -1;
}


int PString::FindLast(const PString & str, PINDEX offset) const
{
  char *p1 = strstr(theArray, str.theArray);
  if (p1 == NULL)
    return -1;

  PINDEX len = Length();
  if (offset > len)
    offset = len;

  char *p2;
  while ((p2 = strstr(p1, str.theArray)) != NULL &&
                                                (int)(p2 - theArray) < offset)
    p1 = p2;
  return (int)(p1 - theArray);
}


int PString::FindOneOf(const PString & str, PINDEX offset) const
{
  register PINDEX len = Length();
  if (offset > len)
    offset = len;
  for (char * cpos = theArray+offset; *cpos != '\0'; cpos++) {
    if (strchr(str.theArray, *cpos) != NULL)
      return (int)(cpos - theArray);
  }
  return -1;
}


PStringArray
      PString::Tokenise(const PString & separators, BOOL onePerSeparator) const
{
  PStringArray tokens;
  
  if (IsEmpty())
    return tokens;
    
  int token = 0;
  int p1 = 0;
  int p2;
  while ((p2 = FindOneOf(separators, p1)) >= 0) {
    if (onePerSeparator)
      tokens[token++] = operator()(p1, p2-1);
    p1 = p2 + 1;
  }
  tokens[token] = operator()(p1, P_MAX_INDEX);
  return tokens;
}


PStringArray PString::Lines() const
{
  PStringArray lines;
  
  if (IsEmpty())
    return lines;
    
  int line = 0;
  int p1 = 0;
  int p2;
  while ((p2 = FindOneOf("\r\n", p1)) >= 0) {
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
  char * rpos = theArray+Length()-1;
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

  char * rpos = theArray+Length()-1;
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


long PString::AsInteger(int base) const
{
  PAssert(base >= 2 && base <= 36);
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
  PINDEX len = Length();
  PAssert(len < 256);
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


PString & PString::vsprintf(const char * fmt, va_list arg)
{
  ::vsprintf(GetPointer(1000), fmt, arg);
  PAssert(strlen(theArray) < 1000);
  PAssert(MakeMinimumSize());
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


int PCaselessString::Find(char ch, PINDEX offset) const
{
  register PINDEX len = Length();
  if (offset > len)
    offset = len;
  char *cpos = theArray+offset;
  int chl = tolower(ch);
  int chu = toupper(ch);
  while (isupper(*cpos) ? *cpos == chu : *cpos == chl) {
    if (*cpos++ == '\0')
      return -1;
  }
  return (int)(cpos - theArray);
}


int PCaselessString::Find(const char *cstr, PINDEX offset) const
{
  PString str1(*this);
  PString str2(cstr);
  str1.ToUpper();
  str2.ToUpper();
  return str1.Find(str2, offset);
}


int PCaselessString::FindLast(char ch) const
{
  char *cpos = theArray+Length()-1;
  int chl = tolower(ch);
  int chu = toupper(ch);
  while (isupper(*cpos) ? *cpos == chu : *cpos == chl) {
    if (--cpos == theArray)
      return -1;
  }
  return (int)(cpos - theArray);
}


int PCaselessString::FindLast(const char *cstr) const
{
  PString str1(*this);
  PString str2(cstr);
  str1.ToUpper();
  str2.ToUpper();
  return str1.FindLast(str2);
}


///////////////////////////////////////////////////////////////////////////////

void PArrayObjects::DestroyContents()
{
  if (deleteObjects && *PAssertNULL(referenceCount) <= 1) {
    PAssertNULL(theArray);
    for (PINDEX i = 0; i < size; i++) {
      if (theArray[i] != NULL)
        delete theArray[i];
    }
    deleteObjects = FALSE;
  }
}


PArrayObjects::PArrayObjects(const PArrayObjects * array)
{
  for (PINDEX i = 0; i < array->size; i++) {
    PObject * ptr = ((PArrayObjects *)array)->GetAt(i);
    if (ptr != NULL)
      SetAt(i, ptr->Clone());
  }
}


PObject::Comparison PArrayObjects::Compare(const PObject & obj) const
{
  for (PINDEX i = 0; i < size; i++) {
    if (*theArray[i] < *((const PArrayObjects &)obj).theArray[i])
      return LessThan;
    if (*theArray[i] > *((const PArrayObjects &)obj).theArray[i])
      return GreaterThan;
  }
  return EqualTo;
}


PINDEX PArrayObjects::GetSize() const
{
  return theArray.GetSize();
}


BOOL PArrayObjects::SetSize(PINDEX newSize)
{
  return theArray.SetSize(newSize);
}


PINDEX PArrayObjects::Append(PObject * obj)
{
  PINDEX where = GetSize();
  SetAt(where, obj);
  return where;
}


PINDEX PArrayObjects::Insert(const PObject & before, PObject * obj)
{
  PINDEX where = GetIndex(&before);
  InsertAt(where, obj);
  return where;
}


void PArrayObjects::Remove(const PObject * obj)
{
  RemoveAt(GetIndex(obj));
}


PObject * PArrayObjects::GetAt(PINDEX index)
{
  return theArray[index];
}


BOOL PArrayObjects::SetAt(PINDEX index, PObject * obj)
{
  if (!theArray.MakeUnique() ||
                   (index >= theArray.GetSize() && !theArray.SetSize(index+1)))
    return FALSE;
  PObject * oldObj = theArray.GetAt(index);
  if (oldObj != NULL && deleteObjects)
    delete oldObj;
  theArray[index] = obj;
  return TRUE;
}


PINDEX PArrayObjects::InsertAt(PINDEX index, PObject * obj)
{
  for (PINDEX i = index; i < size; i++)
    theArray.SetAt(i+1, theArray[i]);
  SetAt(index, obj);
  return index;
}


PObject * PArrayObjects::RemoveAt(PINDEX index)
{
  PObject * obj = theArray[index];
  for (PINDEX i = index; i < size; i++)
    theArray[i] = theArray[i+1];
  SetSize(size-1);
  if (obj != NULL && deleteObjects)
    delete obj;
  return obj;
}


PINDEX PArrayObjects::GetIndex(const PObject * obj)
{
  for (PINDEX i = 0; i < size; i++) {
    if (theArray[i] == obj)
      break;
  }
  return i;
}


BOOL PArrayObjects::Enumerate(PEnumerator func, PObject * info) const
{
  for (PINDEX i = 0; i < size; i++) {
    if (!func(*theArray[i], info))
      return FALSE;
  }
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////

PStringArray::PStringArray(PINDEX count, char **strarr)
{
  PAssertNULL(strarr);
  SetSize(count);
  for (PINDEX i = 0; i < count; i++)
    SetAt(i, new PString(strarr[i]));
}


///////////////////////////////////////////////////////////////////////////////

PAbstractList::PAbstractList(const PAbstractList & list)
  : PCollection(list),
    head(list.head),
    tail(list.tail),
    lastElement(NULL),
    lastIndex(0)
{
}


PAbstractList & PAbstractList::operator=(const PAbstractList & list)
{
  PCollection::operator=(list);
  head = list.head;
  tail = list.tail;
  lastElement = NULL;
  lastIndex = 0;
  return *this;
}


void PAbstractList::DestroyContents()
{
  if (*PAssertNULL(referenceCount) <= 1) {
    PListElement *elmt = head;
    while (elmt != NULL) {
      PListElement *next = elmt->next;
      if (deleteObjects && elmt->data != NULL)
        delete elmt->data;
      delete elmt;
      elmt = next;
    }
  }
}


PAbstractList::PAbstractList(const PAbstractList * list)
  : head(NULL),
    tail(NULL),
    lastElement(NULL),
    lastIndex(0)
{
  for (PListElement * element = list->head;
                                      element != NULL; element = element->next)
    Append(element->data->Clone());
}


PObject::Comparison PAbstractList::Compare(const PObject & obj) const
{
  PListElement * elmt1 = head;
  PListElement * elmt2 = ((const PAbstractList &)obj).head;
  while (elmt1 != NULL && elmt2 != NULL) {
    if (elmt1 == NULL)
      return LessThan;
    if (elmt2 == NULL)
      return GreaterThan;
    if (*elmt1->data < *elmt2->data)
      return LessThan;
    if (*elmt1->data > *elmt2->data)
      return GreaterThan;
    elmt1 = elmt1->next;
    elmt2 = elmt2->next;
  }
  return EqualTo;
}


BOOL PAbstractList::SetSize(PINDEX)
{
  return TRUE;
}


PINDEX PAbstractList::Append(PObject * obj)
{
  PListElement * element = new PListElement(PAssertNULL(obj));
  if (tail != NULL)
    tail->next = element;
  element->prev = tail;
  element->next = NULL;
  if (head == NULL)
    head = element;
  tail = element;
  lastElement = element;
  lastIndex = size;
  size++;
  return lastIndex;
}


PINDEX PAbstractList::Insert(const PObject & before, PObject * obj)
{
  PAssertNULL(obj);
  
  PINDEX where = GetIndex(&before);
  InsertAt(where, obj);
  return where;
}


PINDEX PAbstractList::InsertAt(PINDEX index, PObject * obj)
{
  PAssertNULL(obj);

  if (index >= size)
    return Append(obj);

  PAssert(SetCurrent(index));

  PListElement * newElement = new PListElement(obj);
  if (lastElement->prev != NULL)
    lastElement->prev->next = newElement;
  else
    head = newElement;
  newElement->prev = lastElement->prev;
  newElement->next = lastElement;
  lastElement->prev = newElement;
  lastElement = newElement;
  lastIndex = 0;
  size++;
  return index;
}


void PAbstractList::Remove(const PObject * obj)
{
  RemoveAt(GetIndex(obj));
}


PObject * PAbstractList::RemoveAt(PINDEX index)
{
  PAssert(SetCurrent(index));

  PObject * obj = lastElement->data;

  if (lastElement->prev != NULL)
    lastElement->prev->next = lastElement->next;
  else {
    head = lastElement->next;
    if (head != NULL)
      head->prev = NULL;
  }

  if (lastElement->next != NULL)
    lastElement->next->prev = lastElement->prev;
  else {
    tail = lastElement->prev;
    if (tail != NULL)
      tail->next = NULL;
  }

  lastElement = lastElement->next;
  size--;

  if (obj != NULL && deleteObjects)
    delete obj;
  return obj;
}


PObject * PAbstractList::GetAt(PINDEX index)
{
  return SetCurrent(index) ? lastElement->data : NULL;
}


BOOL PAbstractList::SetAt(PINDEX index, PObject * val)
{
  if (!SetCurrent(index))
    return FALSE;
  lastElement->data = val;
  return TRUE;
}


PINDEX PAbstractList::GetIndex(const PObject * obj)
{
  PListElement * element = lastElement;
  PINDEX index = lastIndex;
  while (element != NULL && element->data != obj) {
    element = element->next;
    index++;
  }
  if (element == NULL) {
    element = lastElement;
    index = lastIndex;
    while (element != NULL && element->data != obj) {
      element = element->prev;
      index--;
    }
  }

  lastElement = PAssertNULL(element);
  lastIndex = index;
  return index;
}


BOOL PAbstractList::Enumerate(PEnumerator func, PObject * info) const
{
  for (PListElement * element = head; element != NULL; element = element->next) {
    if (!func(*element->data, info))
      return FALSE;
  }
  return TRUE;
}


BOOL PAbstractList::SetCurrent(PINDEX index)
{
  if (index >= size)
    return FALSE;

  if (lastElement == NULL || lastIndex >= size || 
      index < lastIndex/2 || index > (lastIndex+size)/2) {
    if (index < size/2) {
      lastIndex = 0;
      lastElement = head;
    }
    else {
      lastIndex = size-1;
      lastElement = tail;
    }
  }

  while (lastIndex < index) {
    lastElement = lastElement->next;
    lastIndex++;
  }

  while (lastIndex > index) {
    lastElement = lastElement->prev;
    lastIndex--;
  }

  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////

PAbstractSortedList::PAbstractSortedList(const PAbstractSortedList * list)
  : root(NULL),
    lastElement(NULL),
    lastIndex(0)
{
  PSortedListElement * element = list->root;
  while (element->left != NULL)
    element = element->left;
  while (element != NULL) {
    Append(element->data->Clone());
    element = element->Successor();
  }
}


PAbstractSortedList::PAbstractSortedList(const PAbstractSortedList & list)
  : PCollection(list),
    root(list.root),
    lastElement(NULL),
    lastIndex(0)
{
}


PAbstractSortedList &
              PAbstractSortedList::operator=(const PAbstractSortedList & list)
{
  PCollection::operator=(list);
  root = list.root;
  lastElement = NULL;
  lastIndex = 0;
  return *this;
}


BOOL PAbstractSortedList::SetSize(PINDEX)
{
  return TRUE;
}


PObject::Comparison PAbstractSortedList::Compare(const PObject & obj) const
{
  PSortedListElement * elmt1 = root;
  while (elmt1->left != NULL)
    elmt1 = elmt1->left;

  PSortedListElement * elmt2 = ((const PAbstractSortedList &)obj).root;
  while (elmt2->left != NULL)
    elmt2 = elmt2->left;

  while (elmt1 != NULL && elmt2 != NULL) {
    if (elmt1 == NULL)
      return LessThan;
    if (elmt2 == NULL)
      return GreaterThan;
    if (*elmt1->data < *elmt2->data)
      return LessThan;
    if (*elmt1->data > *elmt2->data)
      return GreaterThan;
    elmt1 = elmt1->Successor();
    elmt2 = elmt2->Successor();
  }
  return EqualTo;
}


PINDEX PAbstractSortedList::Append(PObject * obj)
{
  PSortedListElement * element = new PSortedListElement(PAssertNULL(obj));
  PSortedListElement * child = root, * parent = NULL;
  while (child != NULL) {
    child->subTreeSize++;
    parent = child;
    child = *element->data < *child->data ? child->left : child->right;
  }
  element->parent = parent;
  if (parent == NULL)
    root = element;
  else if (*element->data < *parent->data)
    parent->left = element;
  else
    parent->right = element;

  element->MakeRed();
  while (element != root && !element->parent->IsBlack()) {
    if (element->parent == element->parent->parent->left) {
      child = element->parent->parent->right;
      if (child != NULL && !child->IsBlack()) {
        element->parent->MakeBlack();
        child->MakeBlack();
        element = element->parent->parent;
      }
      else {
        if (element == element->parent->right) {
          element = element->parent;
          LeftRotate(element);
        }
        element->parent->MakeBlack();
        element->parent->parent->MakeRed();
        RightRotate(element->parent->parent);
      }
    }
    else {
      child = element->parent->parent->left;
      if (child != NULL && !child->IsBlack()) {
        element->parent->MakeBlack();
        child->MakeBlack();
        element = element->parent->parent;
      }
      else {
        if (element == element->parent->left) {
          element = element->parent;
          RightRotate(element);
        }
        element->parent->MakeBlack();
        element->parent->parent->MakeRed();
        LeftRotate(element->parent->parent);
      }
    }
  }

  root->MakeBlack();

  size++;
  lastIndex = size+1;
  lastElement = NULL;
  return lastIndex;
}


void PAbstractSortedList::Remove(const PObject * obj)
{
  PSortedListElement * element = root;
  while (element != NULL && element->data != obj)
    element = *obj < *element->data ? element->left : element->right;
  RemoveElement(element);
}


PINDEX PAbstractSortedList::Insert(const PObject &, PObject * obj)
{
  return Append(obj);
}


PINDEX PAbstractSortedList::InsertAt(PINDEX, PObject * obj)
{
  return Append(obj);
}


PObject * PAbstractSortedList::RemoveAt(PINDEX index)
{
  PSortedListElement * node = root->OrderSelect(index+1);
  RemoveElement(node);
  return node->data;
}


BOOL PAbstractSortedList::SetAt(PINDEX, PObject *)
{
  return FALSE;
}


PINDEX PAbstractSortedList::GetIndex(const PObject *)
{
  return 0;
}


PObject * PAbstractSortedList::GetAt(PINDEX index)
{
  if (index >= size)
    return NULL;

  if (index == lastIndex-1) {
    lastIndex--;
    lastElement = lastElement->Predecessor();
  }
  else if (index == lastIndex+1) {
    lastIndex++;
    lastElement = lastElement->Successor();
  }
  else
    lastElement = root->OrderSelect(index+1);

  return lastElement->data;
}


BOOL PAbstractSortedList::Enumerate(PEnumerator func, PObject * info) const
{
  PSortedListElement * element = root;
  while (element->left != NULL)
    element = element->left;
  while (element != NULL) {
    if (!func(*element->data, info))
      return FALSE;
    element = element->Successor();
  }
  return TRUE;
}


void PAbstractSortedList::DestroyContents()
{
  if (root != NULL && *PAssertNULL(referenceCount) <= 1) {
    root->DeleteSubTrees(deleteObjects);
    delete root;
    root = NULL;
  }
}


void PAbstractSortedList::RemoveElement(PSortedListElement * node)
{
  PAssertNULL(node);

  PSortedListElement * y = 
          node->left == NULL || node->right == NULL ? node : node->Successor();
  PSortedListElement * x = y->left != NULL ? y->left : y->right;

  if (y->parent == NULL)
    root = x;
  else if (y == y->parent->left)
    y->parent->left = x;
  else
    y->parent->right = x;
  if (x != NULL)
    x->parent = y->parent;

  if (y != node)
    *y = *node;

  PSortedListElement * t = y->parent;
  while (t != NULL) {
    t->subTreeSize--;
    t = t->parent;
  }

  if (x != NULL && y->IsBlack()) {
    while (x != root && x->IsBlack()) {
      if (x == x->parent->left) {
        y = x->parent->right;
        if (!y->IsBlack()) {
          y->MakeBlack();
          x->parent->MakeRed();
          LeftRotate(x->parent);
          y = x->parent->right;
        }
        if (y->IsLeftBlack() && y->IsRightBlack()) {
          y->MakeRed();
          x = x->parent;
        }
        else {
          if (y->IsRightBlack()) {
            y->left->MakeBlack();
            y->MakeRed();
            RightRotate(y);
            y = x->parent->right;
          }
          y->colour = x->parent->colour;
          x->parent->MakeBlack();
          if (y->right != NULL)
            y->right->MakeBlack();
          LeftRotate(x->parent);
          x = root;
        }
      }
      else {
        y = x->parent->left;
        if (!y->IsBlack()) {
          y->MakeBlack();
          x->parent->MakeRed();
          RightRotate(x->parent);
          y = x->parent->left;
        }
        if (y->IsRightBlack() && y->IsLeftBlack()) {
          y->MakeRed();
          x = x->parent;
        }
        else {
          if (y->IsLeftBlack()) {
            y->right->MakeBlack();
            y->MakeRed();
            LeftRotate(y);
            y = x->parent->left;
          }
          y->colour = x->parent->colour;
          x->parent->MakeBlack();
          if (y->left != NULL)
            y->left->MakeBlack();
          RightRotate(x->parent);
          x = root;
        }
      }
    }
    x->MakeBlack();
  }

  size--;
  lastIndex = size+1;
  lastElement = NULL;
}


void PAbstractSortedList::LeftRotate(PSortedListElement * node)
{
  PSortedListElement * pivot = PAssertNULL(node)->right;
  node->right = pivot->left;
  if (pivot->left != NULL)
    pivot->left->parent = node;
  pivot->parent = node->parent;
  if (node->parent == NULL)
    root = pivot;
  else if (node == node->parent->left)
    node->parent->left = pivot;
  else
    node->parent->right = pivot;
  pivot->left = node;
  node->parent = pivot;
  pivot->subTreeSize = node->subTreeSize;
  node->subTreeSize = node->LeftTreeSize() + node->RightTreeSize() + 1;
}


void PAbstractSortedList::RightRotate(PSortedListElement * node)
{
  PSortedListElement * pivot = PAssertNULL(node)->left;
  node->left = pivot->right;
  if (pivot->right != NULL)
    pivot->right->parent = node;
  pivot->parent = node->parent;
  if (node->parent == NULL)
    root = pivot;
  else if (node == node->parent->right)
    node->parent->right = pivot;
  else
    node->parent->left = pivot;
  pivot->right = node;
  node->parent = pivot;
  pivot->subTreeSize = node->subTreeSize;
  node->subTreeSize = node->LeftTreeSize() + node->RightTreeSize() + 1;
}


PSortedListElement * PSortedListElement::Successor() const
{
  PSortedListElement * next;
  if (right != NULL) {
    next = right;
    while (next->left != NULL)
      next = next->left;
  }
  else {
    next = parent;
    const PSortedListElement * node = this;
    while (next != NULL && node == next->right) {
      node = next;
      next = node->parent;
    }
  }
  return next;
}


PSortedListElement * PSortedListElement::Predecessor() const
{
  PSortedListElement * pred;
  if (left != NULL) {
    pred = left;
    while (pred->right != NULL)
      pred = pred->right;
  }
  else {
    pred = parent;
    const PSortedListElement * node = this;
    while (pred != NULL && node == pred->right) {
      node = pred;
      pred = node->parent;
    }
  }
  return pred;
}


PSortedListElement * PSortedListElement::OrderSelect(PINDEX index)
{
  PINDEX r = LeftTreeSize()+1;
  if (index == r)
    return this;
  return index < r ? left != NULL ? left->OrderSelect(index) : NULL
                   : right != NULL ? right->OrderSelect(index - r) : NULL;
}


void PSortedListElement::DeleteSubTrees(BOOL deleteObject)
{
  if (left != NULL) {
    left->DeleteSubTrees(deleteObject);
    delete left;
    left = NULL;
  }
  if (right != NULL) {
    right->DeleteSubTrees(deleteObject);
    delete right;
    right = NULL;
  }
  if (deleteObject) {
    delete data;
    data = NULL;
  }
}


///////////////////////////////////////////////////////////////////////////////

PObject * PScalarKey::Clone() const
{
  return new PScalarKey(theKey);
}


PObject::Comparison PScalarKey::Compare(const PObject & obj) const
{
  const PScalarKey & other = (const PScalarKey &)obj;
  
  if (theKey < other.theKey)
    return LessThan;

  if (theKey > other.theKey)
    return GreaterThan;

  return EqualTo;
}


PINDEX PScalarKey::HashFunction() const
{
  return PABSINDEX(theKey)%23;
}


ostream & PScalarKey::PrintOn(ostream & strm) const
{
  return strm << theKey;
}


///////////////////////////////////////////////////////////////////////////////

PHashTable::PHashTable()
  : PCollection(0),
    lastIndex(0),
    lastBucket(0),
    lastElement(NULL)
{
}


PHashTable::PHashTable(const PHashTable & ht)
  : PCollection(ht),
    hashTable(ht.hashTable),
    lastIndex(0),
    lastBucket(0),
    lastElement(NULL)
{
}

  
PHashTable::PHashTable(const PHashTable * ht)
  : hashTable(*(HashArray *)ht->hashTable.Clone()),
    lastIndex(0),
    lastBucket(0),
    lastElement(NULL)
{
}


PHashTable & PHashTable::operator=(const PHashTable & ht)
{
  PCollection::operator=(ht);
  hashTable = ht.hashTable;
  lastIndex = 0;
  lastBucket = 0;
  lastElement = NULL;
  return *this;
}

  
PObject::Comparison PHashTable::Compare(const PObject & obj) const
{
  return referenceCount != ((const PHashTable &)obj).referenceCount
                                                      ? GreaterThan : EqualTo;
}


BOOL PHashTable::SetSize(PINDEX)
{
  return TRUE;
}


void PHashTable::DestroyContents()
{
  /* Do Nothing */
}


void PHashTable::AppendElement(const PObject & key, PObject * data)
{
  lastElement = NULL;

  PINDEX bucket = key.HashFunction();
  HashTableElement * list = hashTable.GetAt(bucket);
  HashTableElement * element = new HashTableElement;
  element->key = key.Clone();
  element->data = data;
  if (list == NULL) {
    element->next = element->prev = element;
    hashTable.SetAt(bucket, element);
  }
  else if (list == list->prev) {
    list->next = list->prev = element;
    element->next = element->prev = list;
  }
  else {
    element->next = list;
    element->prev = list->prev;
    list->prev->next = element;
    list->prev = element;
  }
  lastElement = element;
  lastIndex = P_MAX_INDEX;
  size++;
}


void PHashTable::RemoveElement(const PObject & key)
{
  if (GetElementAt(key) != NULL) {
    if (lastElement == lastElement->prev)
      hashTable.SetAt(key.HashFunction(), NULL);
    else {
      lastElement->prev->next = lastElement->next;
      lastElement->next->prev = lastElement->prev;
      hashTable.SetAt(key.HashFunction(), lastElement->next);
    }
    if (deleteObjects)
      delete lastElement->data;
    delete lastElement->key;
    delete lastElement;
    lastElement = NULL;
    size--;
  }
}


BOOL PHashTable::SetLastElementAt(PINDEX index)
{
  if (lastElement == NULL || lastIndex == P_MAX_INDEX) {
    lastIndex = 0;
    lastBucket = 0;
    while ((lastElement = hashTable[lastBucket]) == NULL) {
      if (lastBucket >= hashTable.GetSize())
        return FALSE;
      lastBucket++;
    }
  }

  if (lastIndex == index)
    return TRUE;

  if (lastIndex < index) {
    while (lastIndex != index) {
      if (lastElement->next != hashTable[lastBucket])
        lastElement = lastElement->next;
      else {
        do {
          if (++lastBucket >= hashTable.GetSize())
            return FALSE;
        } while ((lastElement = hashTable[lastBucket]) == NULL);
      }
      lastIndex++;
    }
  }
  else {
    while (lastIndex != index) {
      if (lastElement != hashTable[lastBucket])
        lastElement = lastElement->prev;
      else {
        do {
          if (lastBucket-- == 0)
            return FALSE;
        } while ((lastElement = hashTable[lastBucket]) == NULL);
      }
      lastIndex--;
    }
  }

  return TRUE;
}


PHashTable::HashTableElement * PHashTable::GetElementAt(const PObject & key)
{
  if (lastElement != NULL && *lastElement->key == key)
    return lastElement;

  HashTableElement * list = hashTable.GetAt(key.HashFunction());
  if (list != NULL) {
    HashTableElement * element = list;
    do {
      if (*element->key == key) {
        lastElement = element;
        lastIndex = P_MAX_INDEX;
        return lastElement;
      }
      element = element->next;
    } while (element != list);
  }
  return NULL;
}


BOOL PHashTable::EnumerateElements(PEnumerator func, PObject * info, BOOL keys) const
{
  for (PINDEX i = 0; i < hashTable.GetSize(); i++) {
    HashTableElement * list = hashTable[i];
    if (list != NULL) {
      HashTableElement * element = list;
      do {
        if (!func(keys ? *element->key : *element->data, info))
          return FALSE;
      } while (element != list);
    }
  }
  return TRUE;
}



///////////////////////////////////////////////////////////////////////////////

PINDEX PAbstractSet::Append(PObject * obj)
{
  if (!Contains(*obj))
    AppendElement(*obj, NULL);
  return 0;
}


PINDEX PAbstractSet::Insert(const PObject &, PObject * obj)
{
  return Append(obj);
}


PINDEX PAbstractSet::InsertAt(PINDEX, PObject * obj)
{
  return Append(obj);
}


void PAbstractSet::Remove(const PObject * obj)
{
  RemoveElement(*obj);
}


PObject * PAbstractSet::RemoveAt(PINDEX)
{
  PAssertAlways();
  return NULL;
}


PINDEX PAbstractSet::GetIndex(const PObject *)
{
  PAssertAlways();
  return 0;
}


BOOL PAbstractSet::SetAt(PINDEX, PObject *)
{
  PAssertAlways();
  return FALSE;
}


PObject * PAbstractSet::GetAt(PINDEX)
{
  PAssertAlways();
  return NULL;
}


BOOL PAbstractSet::Enumerate(PEnumerator func, PObject * info) const
{
  return EnumerateElements(func, info, TRUE);
}



///////////////////////////////////////////////////////////////////////////////

/*
PObject * PAbstractDictionary::Clone() const
{
  PAbstractDictionary * newDict = new PAbstractDictionary();
  HashArray * ht = (HashArray *)hashTable.Clone();
  newDict->hashTable = *ht;
  delete ht;
  return newDict;
}
*/


PINDEX PAbstractDictionary::Append(PObject *)
{
  PAssertAlways();
  return 0;
}


PINDEX PAbstractDictionary::Insert(const PObject &, PObject *)
{
  PAssertAlways();
  return 0;
}


PINDEX PAbstractDictionary::InsertAt(PINDEX index,PObject * obj)
{
  SetAt(PScalarKey(index), obj);
  return index;
}
 
 
void PAbstractDictionary::Remove(const PObject *)
{
  PAssertAlways();
}


PObject * PAbstractDictionary::RemoveAt(PINDEX index)
{
  PObject * obj = GetAt(index);
  SetAt(PScalarKey(index), NULL);
  return obj;
}


PINDEX PAbstractDictionary::GetIndex(const PObject *)
{
  PAssertAlways();
  return 0;
}


BOOL PAbstractDictionary::SetAt(PINDEX index,PObject * val)
{
  return SetAt(PScalarKey(index), val);
}


PObject * PAbstractDictionary::GetAt(PINDEX index)
{
  return GetAt(PScalarKey(index));
}
 
 
BOOL PAbstractDictionary::Enumerate(PEnumerator func, PObject * info) const
{
  return EnumerateElements(func, info, FALSE);
}


BOOL PAbstractDictionary::EnumerateKeys(PEnumerator func, PObject * info) const
{
  return EnumerateElements(func, info, TRUE);
}


BOOL PAbstractDictionary::SetAt(const PObject & key, PObject * obj)
{
  if (obj == NULL)
    RemoveElement(key);
  else {
    HashTableElement * element = GetElementAt(key);
    if (element == NULL)
      AppendElement(key, obj);
    else {
      if (deleteObjects)
        delete lastElement->data;
      lastElement->data = obj;
    }
  }
  return TRUE;
}


PObject * PAbstractDictionary::GetAt(const PObject & key)
{
  HashTableElement * element = GetElementAt(key);
  return element != NULL ? element->data : NULL;
}



// End Of File ///////////////////////////////////////////////////////////////
