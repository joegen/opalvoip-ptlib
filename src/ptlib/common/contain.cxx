/*
 * $Id: contain.cxx,v 1.8 1993/12/16 00:51:46 robertj Exp $
 *
 * Portable Windows Library
 *
 * PContainer Class Implementation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: contain.cxx,v $
 * Revision 1.8  1993/12/16 00:51:46  robertj
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
  : reference(PAssertNULL(cont.reference))
{                                                            
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


PContainer::~PContainer()
{
  if (!IsUnique())
    reference->count--;
  else {
    delete reference;
    reference = NULL;
  }
}


PINDEX PContainer::GetSize() const
{
  return reference->size;
}


BOOL PContainer::IsEmpty() const
{
  return GetSize() == 0;
}



///////////////////////////////////////////////////////////////////////////////

PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes, PINDEX initialSize)
  : PContainer(initialSize)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0);
  if (GetSize() == 0)
    reference->size = 1;
  PINDEX sizebytes = elementSize*GetSize();
  theArray = new char[sizebytes];
  memset(theArray, 0, sizebytes);
}


PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes,
                               const void *buffer,
                               PINDEX bufferSizeInElements)
  : PContainer(bufferSizeInElements)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0);
  if (GetSize() == 0)
    reference->size = 1;
  PINDEX sizebytes = elementSize*GetSize();
  theArray = new char[sizebytes];
  memcpy(theArray, PAssertNULL(buffer), sizebytes);
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
  if (theArray != NULL && IsUnique()) {
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
  if (GetSize() < array.GetSize())
    return LessThan;
  if (GetSize() > array.GetSize())
    return GreaterThan;
  return (Comparison)memcmp(theArray, array.theArray, elementSize*GetSize());
}


BOOL PAbstractArray::SetSize(PINDEX newSize)
{
  if (newSize == 0)
    newSize = 1;

  PINDEX newsizebytes = elementSize*newSize;
  char *newArray = new char[newsizebytes];
  if (newArray == NULL)
    return FALSE;

  PINDEX oldsizebytes = elementSize*GetSize();
  if (newsizebytes <= oldsizebytes)
    memcpy(newArray, theArray, newsizebytes);
  else {
    memcpy(newArray, theArray, oldsizebytes);
    memset(newArray+oldsizebytes, 0, newsizebytes-oldsizebytes);
  }

  if (IsUnique()) {
    delete [] theArray;
    reference->size = newSize;
  }
  else {
    reference->count--;
    reference = new Reference(newSize);
  }

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

void PCollection::RemoveAll()
{
  while (GetSize() > 0)
    RemoveAt(0);
}


///////////////////////////////////////////////////////////////////////////////

void PArrayObjects::DestroyContents()
{
  if (deleteObjects && IsUnique()) {
    PAssertNULL(theArray);
    for (PINDEX i = 0; i < theArray.GetSize(); i++) {
      if (theArray[i] != NULL)
        delete theArray[i];
    }
    deleteObjects = FALSE;
  }
}


PArrayObjects::PArrayObjects(const PArrayObjects * array)
{
  for (PINDEX i = 0; i < array->GetSize(); i++) {
    PObject * ptr = array->GetAt(i);
    if (ptr != NULL)
      SetAt(i, ptr->Clone());
  }
}


PObject::Comparison PArrayObjects::Compare(const PObject & obj) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
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


PObject * PArrayObjects::GetAt(PINDEX index) const
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
  for (PINDEX i = index; i < GetSize(); i++)
    theArray.SetAt(i+1, theArray[i]);
  SetAt(index, obj);
  return index;
}


PObject * PArrayObjects::RemoveAt(PINDEX index)
{
  PObject * obj = theArray[index];
  for (PINDEX i = index; i < GetSize(); i++)
    theArray[i] = theArray[i+1];
  SetSize(GetSize()-1);
  if (obj != NULL && deleteObjects)
    delete obj;
  return obj;
}


PINDEX PArrayObjects::GetIndex(const PObject * obj) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    if (theArray[i] == obj)
      return i;
  }
  return P_MAX_INDEX;
}


BOOL PArrayObjects::Enumerate(PEnumerator func, PObject * info) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
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

PAbstractList::Info::Info()
{
  head = tail = lastElement = NULL;
  lastIndex = 0;
}


PAbstractList::PAbstractList(const PAbstractList & list)
  : PCollection(list),
    info(list.info)
{
}


PAbstractList::PAbstractList(const PAbstractList * list)
  : info(new Info)
{
  for (PListElement * element = list->info->head;
                                      element != NULL; element = element->next)
    Append(element->data->Clone());
}


PAbstractList & PAbstractList::operator=(const PAbstractList & list)
{
  PCollection::operator=(list);
  info = list.info;
  return *this;
}


void PAbstractList::DestroyContents()
{
  if (info != NULL && IsUnique()) {
    RemoveAll();
    delete info;
    info = NULL;
  }
}


PObject::Comparison PAbstractList::Compare(const PObject & obj) const
{
  PListElement * elmt1 = info->head;
  PListElement * elmt2 = ((const PAbstractList &)obj).info->head;
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
  if (info->tail != NULL)
    info->tail->next = element;
  element->prev = info->tail;
  element->next = NULL;
  if (info->head == NULL)
    info->head = element;
  info->tail = element;
  info->lastElement = element;
  info->lastIndex = GetSize();
  reference->size++;
  return info->lastIndex;
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

  if (index >= GetSize())
    return Append(obj);

  PAssert(SetCurrent(index));

  PListElement * newElement = new PListElement(obj);
  if (info->lastElement->prev != NULL)
    info->lastElement->prev->next = newElement;
  else
    info->head = newElement;
  newElement->prev = info->lastElement->prev;
  newElement->next = info->lastElement;
  info->lastElement->prev = newElement;
  info->lastElement = newElement;
  info->lastIndex = 0;
  reference->size++;
  return index;
}


void PAbstractList::Remove(const PObject * obj)
{
  RemoveAt(GetIndex(obj));
}


PObject * PAbstractList::RemoveAt(PINDEX index)
{
  PAssert(SetCurrent(index));

  PListElement * elmt = info->lastElement;

  if (elmt->prev != NULL)
    elmt->prev->next = elmt->next;
  else {
    info->head = elmt->next;
    if (info->head != NULL)
      info->head->prev = NULL;
  }

  if (elmt->next != NULL)
    elmt->next->prev = elmt->prev;
  else {
    info->tail = elmt->prev;
    if (info->tail != NULL)
      info->tail->next = NULL;
  }

  if (elmt->next != NULL)
    info->lastElement = elmt->next;
  else {
    info->lastElement = elmt->prev;
    info->lastIndex--;
  }
  reference->size--;

  PObject * obj = elmt->data;
  if (obj != NULL && deleteObjects) {
    delete obj;
    obj = NULL;
  }
  delete elmt;
  return obj;
}


PObject * PAbstractList::GetAt(PINDEX index) const
{
  return SetCurrent(index) ? info->lastElement->data : NULL;
}


BOOL PAbstractList::SetAt(PINDEX index, PObject * val)
{
  if (!SetCurrent(index))
    return FALSE;
  info->lastElement->data = val;
  return TRUE;
}


PINDEX PAbstractList::GetIndex(const PObject * obj) const
{
  PListElement * element = info->lastElement;
  PINDEX index = info->lastIndex;
  while (element != NULL && element->data != obj) {
    element = element->next;
    index++;
  }
  if (element == NULL) {
    element = info->lastElement;
    index = info->lastIndex;
    while (element != NULL && element->data != obj) {
      element = element->prev;
      index--;
    }
  }

  if (element == NULL)
    return P_MAX_INDEX;

  info->lastElement = element;
  info->lastIndex = index;
  return index;
}


BOOL PAbstractList::Enumerate(PEnumerator func, PObject * inf) const
{
  for (PListElement * element = info->head;
                                    element != NULL; element = element->next) {
    if (!func(*element->data, inf))
      return FALSE;
  }
  return TRUE;
}


BOOL PAbstractList::SetCurrent(PINDEX index) const
{
  if (index >= GetSize())
    return FALSE;

  if (info->lastElement == NULL || info->lastIndex >= GetSize() || 
      index < info->lastIndex/2 || index > (info->lastIndex+GetSize())/2) {
    if (index < GetSize()/2) {
      info->lastIndex = 0;
      info->lastElement = info->head;
    }
    else {
      info->lastIndex = GetSize()-1;
      info->lastElement = info->tail;
    }
  }

  while (info->lastIndex < index) {
    info->lastElement = info->lastElement->next;
    info->lastIndex++;
  }

  while (info->lastIndex > index) {
    info->lastElement = info->lastElement->prev;
    info->lastIndex--;
  }

  return TRUE;
}


PListElement::PListElement(PObject * theData)
{
  next = prev = NULL;
  data = theData;
}


///////////////////////////////////////////////////////////////////////////////

PAbstractSortedList::Info::Info()
{
  root = lastElement = NULL;
  lastIndex = 0;
}


PAbstractSortedList::PAbstractSortedList(const PAbstractSortedList * list)
  : info(new Info)
{
  PSortedListElement * element = list->info->root;
  while (element->left != NULL)
    element = element->left;
  while (element != NULL) {
    Append(element->data->Clone());
    element = element->Successor();
  }
}


PAbstractSortedList::PAbstractSortedList(const PAbstractSortedList & list)
  : PCollection(list),
    info(list.info)
{
}


PAbstractSortedList &
              PAbstractSortedList::operator=(const PAbstractSortedList & list)
{
  PCollection::operator=(list);
  info = list.info;
  return *this;
}


BOOL PAbstractSortedList::SetSize(PINDEX)
{
  return TRUE;
}


PObject::Comparison PAbstractSortedList::Compare(const PObject & obj) const
{
  PSortedListElement * elmt1 = info->root;
  while (elmt1->left != NULL)
    elmt1 = elmt1->left;

  PSortedListElement * elmt2 = ((const PAbstractSortedList &)obj).info->root;
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
  PSortedListElement * child = info->root, * parent = NULL;
  while (child != NULL) {
    child->subTreeSize++;
    parent = child;
    child = *element->data < *child->data ? child->left : child->right;
  }
  element->parent = parent;
  if (parent == NULL)
    info->root = element;
  else if (*element->data < *parent->data)
    parent->left = element;
  else
    parent->right = element;

  element->MakeRed();
  while (element != info->root && !element->parent->IsBlack()) {
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

  info->root->MakeBlack();

  reference->size++;
  info->lastIndex = P_MAX_INDEX;
  info->lastElement = NULL;
  return GetSize();
}


void PAbstractSortedList::Remove(const PObject * obj)
{
  PSortedListElement * element = info->root;
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
  PSortedListElement * node = info->root->OrderSelect(index+1);
  RemoveElement(node);
  return node->data;
}


BOOL PAbstractSortedList::SetAt(PINDEX, PObject *)
{
  return FALSE;
}


PINDEX PAbstractSortedList::GetIndex(const PObject *) const
{
  return 0;
}


PObject * PAbstractSortedList::GetAt(PINDEX index) const
{
  if (index >= GetSize())
    return NULL;

  if (index == info->lastIndex-1) {
    info->lastIndex--;
    info->lastElement = info->lastElement->Predecessor();
  }
  else if (index == info->lastIndex+1) {
    info->lastIndex++;
    info->lastElement = info->lastElement->Successor();
  }
  else
    info->lastElement = info->root->OrderSelect(index+1);

  return info->lastElement->data;
}


BOOL PAbstractSortedList::Enumerate(PEnumerator func, PObject * inf) const
{
  PSortedListElement * element = info->root;
  while (element->left != NULL)
    element = element->left;
  while (element != NULL) {
    if (!func(*element->data, inf))
      return FALSE;
    element = element->Successor();
  }
  return TRUE;
}


void PAbstractSortedList::DestroyContents()
{
  if (info != NULL && IsUnique()) {
    if (info->root != NULL) {
      info->root->DeleteSubTrees(deleteObjects);
      delete info->root;
    }
    delete info;
    info = NULL;
  }
}


void PAbstractSortedList::RemoveElement(PSortedListElement * node)
{
  PAssertNULL(node);

  PSortedListElement * y = 
          node->left == NULL || node->right == NULL ? node : node->Successor();
  PSortedListElement * x = y->left != NULL ? y->left : y->right;

  if (y->parent == NULL)
    info->root = x;
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
    while (x != info->root && x->IsBlack()) {
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
          x = info->root;
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
          x = info->root;
        }
      }
    }
    x->MakeBlack();
  }

  reference->size--;
  info->lastIndex = P_MAX_INDEX;
  info->lastElement = NULL;
  delete node;
}


void PAbstractSortedList::LeftRotate(PSortedListElement * node)
{
  PSortedListElement * pivot = PAssertNULL(node)->right;
  node->right = pivot->left;
  if (pivot->left != NULL)
    pivot->left->parent = node;
  pivot->parent = node->parent;
  if (node->parent == NULL)
    info->root = pivot;
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
    info->root = pivot;
  else if (node == node->parent->right)
    node->parent->right = pivot;
  else
    node->parent->left = pivot;
  pivot->right = node;
  node->parent = pivot;
  pivot->subTreeSize = node->subTreeSize;
  node->subTreeSize = node->LeftTreeSize() + node->RightTreeSize() + 1;
}


PSortedListElement::PSortedListElement(PObject * theData)
{
  parent = left = right = NULL;
  colour = Black;
  subTreeSize = 1;
  data = theData;
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
    hashTable(new InternalHashTable)
{
}


PHashTable::PHashTable(const PHashTable & ht)
  : PCollection(ht),
    hashTable(ht.hashTable)
{
}

  
PHashTable::PHashTable(const PHashTable * ht)
  : hashTable((InternalHashTable *)ht->hashTable->Clone())
{
}


PHashTable & PHashTable::operator=(const PHashTable & ht)
{
  PCollection::operator=(ht);
  hashTable = ht.hashTable;
  return *this;
}

  
PObject::Comparison PHashTable::Compare(const PObject & obj) const
{
  return reference != ((const PHashTable &)obj).reference
                                                      ? GreaterThan : EqualTo;
}


BOOL PHashTable::SetSize(PINDEX)
{
  return TRUE;
}


void PHashTable::DestroyContents()
{
  if (hashTable != NULL && IsUnique()) {
    delete hashTable;
    hashTable = NULL;
  }
}


void PHashTable::InternalHashTable::AppendElement(const PObject & key, PObject * data)
{
  lastElement = NULL;

  PINDEX bucket = key.HashFunction();
  HashTableElement * list = GetAt(bucket);
  HashTableElement * element = new HashTableElement;
  element->key = key.Clone();
  element->data = data;
  if (list == NULL) {
    element->next = element->prev = element;
    SetAt(bucket, element);
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
}


PObject * PHashTable::InternalHashTable::RemoveElement(const PObject & key)
{
  PObject * obj = NULL;
  if (GetElementAt(key) != NULL) {
    if (lastElement == lastElement->prev)
      SetAt(key.HashFunction(), NULL);
    else {
      lastElement->prev->next = lastElement->next;
      lastElement->next->prev = lastElement->prev;
      SetAt(key.HashFunction(), lastElement->next);
    }
    obj = lastElement->data;
    delete lastElement->key;
    delete lastElement;
    lastElement = NULL;
  }
  return obj;
}


BOOL PHashTable::InternalHashTable::SetLastElementAt(PINDEX index)
{
  if (lastElement == NULL || lastIndex == P_MAX_INDEX) {
    lastIndex = 0;
    lastBucket = 0;
    while ((lastElement = operator[](lastBucket)) == NULL) {
      if (lastBucket >= GetSize())
        return FALSE;
      lastBucket++;
    }
  }

  if (lastIndex == index)
    return TRUE;

  if (lastIndex < index) {
    while (lastIndex != index) {
      if (lastElement->next != operator[](lastBucket))
        lastElement = lastElement->next;
      else {
        do {
          if (++lastBucket >= GetSize())
            return FALSE;
        } while ((lastElement = operator[](lastBucket)) == NULL);
      }
      lastIndex++;
    }
  }
  else {
    while (lastIndex != index) {
      if (lastElement != operator[](lastBucket))
        lastElement = lastElement->prev;
      else {
        do {
          if (lastBucket-- == 0)
            return FALSE;
        } while ((lastElement = operator[](lastBucket)) == NULL);
      }
      lastIndex--;
    }
  }

  return TRUE;
}


PHashTable::HashTableElement *
               PHashTable::InternalHashTable::GetElementAt(const PObject & key)
{
  if (lastElement != NULL && *lastElement->key == key)
    return lastElement;

  HashTableElement * list = GetAt(key.HashFunction());
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


BOOL PHashTable::InternalHashTable::EnumerateElements(
                            PEnumerator func, PObject * info, BOOL keys) const
{
  for (PINDEX i = 0; i < GetSize(); i++) {
    HashTableElement * list = operator[](i);
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
  if (!Contains(*obj)) {
    hashTable->AppendElement(*obj, NULL);
    reference->size++;
  }
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
  hashTable->RemoveElement(*obj);
  reference->size--;
}


PObject * PAbstractSet::RemoveAt(PINDEX)
{
  PAssertAlways();
  return NULL;
}


PINDEX PAbstractSet::GetIndex(const PObject *) const
{
  PAssertAlways();
  return 0;
}


BOOL PAbstractSet::SetAt(PINDEX, PObject *)
{
  PAssertAlways();
  return FALSE;
}


PObject * PAbstractSet::GetAt(PINDEX) const
{
  PAssertAlways();
  return NULL;
}


BOOL PAbstractSet::Enumerate(PEnumerator func, PObject * info) const
{
  return hashTable->EnumerateElements(func, info, TRUE);
}



///////////////////////////////////////////////////////////////////////////////

/*
PObject * PAbstractDictionary::Clone() const
{
  PAbstractDictionary * newDict = new PAbstractDictionary();
  InternalHashTable * ht = (InternalHashTable *)hashTable.Clone();
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


PINDEX PAbstractDictionary::GetIndex(const PObject *) const
{
  PAssertAlways();
  return 0;
}


BOOL PAbstractDictionary::SetAt(PINDEX index,PObject * val)
{
  return SetAt(PScalarKey(index), val);
}


PObject * PAbstractDictionary::GetAt(PINDEX index) const
{
  return GetAt(PScalarKey(index));
}
 
 
BOOL PAbstractDictionary::Enumerate(PEnumerator func, PObject * info) const
{
  return hashTable->EnumerateElements(func, info, FALSE);
}


BOOL PAbstractDictionary::EnumerateKeys(PEnumerator func, PObject * info) const
{
  return hashTable->EnumerateElements(func, info, TRUE);
}


BOOL PAbstractDictionary::SetAt(const PObject & key, PObject * obj)
{
  if (obj == NULL) {
    obj = hashTable->RemoveElement(key);
    if (deleteObjects)
      delete obj;
    reference->size--;
  }
  else {
    HashTableElement * element = hashTable->GetElementAt(key);
    if (element == NULL) {
      hashTable->AppendElement(key, obj);
      reference->size++;
    }
    else {
      if (deleteObjects)
        delete hashTable->lastElement->data;
      hashTable->lastElement->data = obj;
    }
  }
  return TRUE;
}


PObject * PAbstractDictionary::GetAt(const PObject & key) const
{
  HashTableElement * element = hashTable->GetElementAt(key);
  return element != NULL ? element->data : NULL;
}



// End Of File ///////////////////////////////////////////////////////////////
