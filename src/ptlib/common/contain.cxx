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
 * $Log: contain.cxx,v $
 * Revision 1.74  1998/09/23 06:21:54  robertj
 * Added open source copyright license.
 *
 * Revision 1.73  1998/09/22 02:42:39  robertj
 * Fixed problem treating unsigned integer as signed in PString contructor.
 *
 * Revision 1.72  1998/09/15 08:26:42  robertj
 * Fixed a number of warnings at maximum optimisation.
 *
 * Revision 1.71  1998/09/14 12:36:29  robertj
 * Fixed bug causing memory leak due to uninitialised member variable for dynamic allocation of arrays.
 *
 * Revision 1.70  1998/08/21 05:24:07  robertj
 * Added hex dump capability to base array types.
 * Added ability to have base arrays of static memory blocks.
 *
 * Revision 1.69  1998/03/17 10:13:23  robertj
 * Fixed bug in Trim() should do all white space not just the space character.
 *
 * Revision 1.68  1998/01/26 00:37:48  robertj
 * Fixed PString & operator putting space in if right hand side is empty string, it shouldn't..
 * Added Execute() functions to PRegularExpression that take PINDEX references instead of PIntArrays.
 * Added FindRegEx function to PString that returns position and length.
 *
 * Revision 1.67  1997/12/11 13:32:49  robertj
 * Added AsUnsigned() function to convert string to DWORD.
 *
 * Revision 1.66  1997/07/08 13:14:41  robertj
 * Fixed bug where freeing null pointer.
 *
 * Revision 1.65  1997/06/08 04:48:04  robertj
 * Added regular expressions.
 *
 * Revision 1.64  1997/03/02 03:41:42  robertj
 * Fixed bug in not being able to construct a zero length PStringArray.
 *
 * Revision 1.63  1996/10/08 13:13:25  robertj
 * Added operator += and &= for char so no implicit PString construction.
 *
 * Revision 1.62  1996/09/14 12:45:57  robertj
 * Fixed bug in PString::Splice() function, no end of string put in.
 *
 * Revision 1.61  1996/08/22 13:21:55  robertj
 * Fixed major bug in FindLast(), could scan all of memory in negative direction.
 *
 * Revision 1.60  1996/08/08 10:08:45  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.59  1996/05/26 03:46:27  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.58  1996/05/15 10:17:02  robertj
 * Fixed idiotic bug in string compare, caseless version always matched.
 *
 * Revision 1.57  1996/05/09 12:17:10  robertj
 * Fixed incorrect use of memcmp/strcmp return value.
 * Added assertion when finding empty string.
 *
 * Revision 1.56  1996/04/14 02:52:39  robertj
 * Fixed bug in PString::FindLast(), never found sub-strings.
 *
 * Revision 1.55  1996/03/31 08:58:49  robertj
 * Fixed hash function for strings to work for caseless strings.
 *
 * Revision 1.54  1996/03/16 04:56:59  robertj
 * Fixed bug in PStringStream assignment oeprator getting pointers wrong.
 *
 * Revision 1.53  1996/03/02 03:20:11  robertj
 * Fixed bug in PString::Find() not finding substring if exactly same as string.
 *
 * Revision 1.52  1996/02/22 10:23:54  robertj
 * Fixed buf in *= operator only comparing up to shortest string.
 * Fixed bug in & operator for if left string is empty.
 *
 * Revision 1.51  1996/02/19 13:34:53  robertj
 * Removed PCaselessString hash function to fix dictionary match failure.
 * Fixed *= operator yet again.
 *
 * Revision 1.50  1996/02/08 12:20:44  robertj
 * Added new operators to PString for case insensitive compare and spaced concatenate.
 * Fixed bug in Find() not finding case insensitive substrings.
 *
 * Revision 1.49  1996/02/03 11:08:51  robertj
 * Changed memcpy to memove to guarentee string operations will work correctly
 *    when moving overlapping strings around eg in PString::Splice().
 *
 * Revision 1.48  1996/01/28 14:12:22  robertj
 * Fixed bug in Tokenise() for first token empty and PINDEX unsigned.
 *
 * Revision 1.47  1996/01/28 02:53:40  robertj
 * Added assert into all Compare functions to assure comparison between compatible objects.
 * Fixed bug in Find() function, subset sum calculation added one to many bytes.
 *
 * Revision 1.46  1996/01/24 14:43:19  robertj
 * Added initialisers to string dictionaries.
 *
 * Revision 1.45  1996/01/23 13:17:38  robertj
 * Added Replace() function to strings.
 * String searching algorithm rewrite.
 *
 * Revision 1.44  1996/01/02 12:51:05  robertj
 * Mac OS compatibility changes.
 * Removed requirement that PArray elements have parameterless constructor..
 *
 * Revision 1.43  1995/10/14 15:07:42  robertj
 * Changed arrays to not break references, but strings still need to.
 *
 * Revision 1.42  1995/06/17 00:46:20  robertj
 * Added flag for PStringArray constructor to create caseless strings.
 * Fixed bug in arrays when size set to zero.
 *
 * Revision 1.41  1995/06/04 12:39:59  robertj
 * Made char * array all const in PStringArray constructor.
 *
 * Revision 1.40  1995/04/25 11:29:38  robertj
 * Fixed Borland compiler warnings.
 *
 * Revision 1.39  1995/04/02 09:27:27  robertj
 * Added "balloon" help.
 *
 * Revision 1.38  1995/03/12 04:46:02  robertj
 * Fixed use of PCaselessString as dictionary key.
 *
 * Revision 1.37  1995/01/15  04:56:28  robertj
 * Fixed PStringStream for correct pointer calculations in output.
 *
 * Revision 1.36  1995/01/10  11:44:13  robertj
 * Removed PString parameter in stdarg function for GNU C++ compatibility.
 *
 * Revision 1.35  1995/01/09  12:32:56  robertj
 * Removed unnecesary return value from I/O functions.
 * Changed function names due to Mac port.
 *
 * Revision 1.34  1995/01/04  10:57:08  robertj
 * Changed for HPUX and GNU2.6.x
 *
 * Revision 1.33  1995/01/03  09:39:08  robertj
 * Put standard malloc style memory allocation etc into memory check system.
 *
 * Revision 1.32  1994/12/13  11:50:56  robertj
 * Added MakeUnique() function to all container classes.
 *
 * Revision 1.31  1994/12/12  13:13:17  robertj
 * Fixed bugs in PString mods just made.
 *
 * Revision 1.30  1994/12/12  10:16:27  robertj
 * Restructuring and documentation of container classes.
 * Renaming of some macros for declaring container classes.
 * Added some extra functionality to PString.
 * Added start to 2 byte characters in PString.
 * Fixed incorrect overrides in PCaselessString.
 *
 * Revision 1.29  1994/12/05  11:19:36  robertj
 * Moved SetMinSize from PAbstractArray to PContainer.
 *
 * Revision 1.28  1994/11/28  12:37:29  robertj
 * Added dummy parameter to container classes.
 *
 * Revision 1.27  1994/10/30  11:50:44  robertj
 * Split into Object classes and Container classes.
 * Changed mechanism for doing notification callback functions.
 *
 * Revision 1.26  1994/10/23  03:43:07  robertj
 * Changed PBaseArray so can have zero elements in it.
 * Added Printf style constructor to PString.
 *
 * Revision 1.25  1994/09/25  10:49:44  robertj
 * Added empty functions for serialisation.
 *
 * Revision 1.24  1994/08/21  23:43:02  robertj
 * Added object serialisation classes.
 * Changed parameter before variable argument list to NOT be a reference.
 *
 * Revision 1.23  1994/08/04  12:57:10  robertj
 * Rewrite of memory check code.
 *
 * Revision 1.22  1994/08/01  03:40:28  robertj
 * Fixed PString() constructor from integer
 *
 * Revision 1.21  1994/07/27  05:58:07  robertj
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

#include <ptlib.h>

#include <ctype.h>

#ifndef __STDC__
#define __STDC__ 1
#endif
#include "regex.h"


#if !defined(P_USE_INLINES)
#include "ptlib/contain.inl"
#endif


///////////////////////////////////////////////////////////////////////////////

PContainer::PContainer(PINDEX initialSize)
{
  reference = new Reference(initialSize);
  PAssertNULL(reference);
}


PContainer::PContainer(int, const PContainer * cont)
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


BOOL PContainer::SetMinSize(PINDEX minSize)
{
  PASSERTINDEX(minSize);
  if (minSize < GetSize())
    minSize = GetSize();
  return SetSize(minSize);
}


BOOL PContainer::MakeUnique()
{
  if (IsUnique())
    return TRUE;

  reference->count--;
  reference = new Reference(GetSize());
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////

PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes, PINDEX initialSize)
  : PContainer(initialSize)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0, PInvalidParameter);

  if (GetSize() == 0)
    theArray = NULL;
  else
    theArray = (char *)PCALLOC(GetSize(), elementSize);

  allocatedDynamically = TRUE;
}


PAbstractArray::PAbstractArray(PINDEX elementSizeInBytes,
                               const void *buffer,
                               PINDEX bufferSizeInElements,
                               BOOL dynamicAllocation)
  : PContainer(bufferSizeInElements)
{
  elementSize = elementSizeInBytes;
  PAssert(elementSize != 0, PInvalidParameter);

  allocatedDynamically = dynamicAllocation;

  if (GetSize() == 0)
    theArray = NULL;
  else if (dynamicAllocation) {
    PINDEX sizebytes = elementSize*GetSize();
    theArray = (char *)PMALLOC(sizebytes);
    memcpy(theArray, PAssertNULL(buffer), sizebytes);
  }
  else
    theArray = (char *)buffer;
}


void PAbstractArray::DestroyContents()
{
  if (theArray != NULL) {
    if (allocatedDynamically)
      PFREE(theArray);
    theArray = NULL;
  }
}


void PAbstractArray::CopyContents(const PAbstractArray & array)
{
  elementSize = array.elementSize;
  theArray = array.theArray;
  allocatedDynamically = array.allocatedDynamically;
}


void PAbstractArray::CloneContents(const PAbstractArray * array)
{
  elementSize = array->elementSize;
  PINDEX sizebytes = elementSize*GetSize();
  char * newArray = (char *)PMALLOC(sizebytes);
  if (newArray == NULL)
    reference->size = 0;
  else
    memcpy(newArray, array->theArray, sizebytes);
  theArray = newArray;
  allocatedDynamically = TRUE;
}


PObject::Comparison PAbstractArray::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PAbstractArray::Class()), PInvalidCast);
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

  if (thisSize == 0)
    return EqualTo;

  char * p2 = array.theArray;
  PINDEX len = elementSize*thisSize;

  int retval = memcmp(theArray, p2, len);
  if (retval < 0)
    return LessThan;
  if (retval > 0)
    return GreaterThan;
  return EqualTo;
}


BOOL PAbstractArray::SetSize(PINDEX newSize)
{
  PINDEX newsizebytes = elementSize*newSize;
  PINDEX oldsizebytes = elementSize*GetSize();

  if (newsizebytes == oldsizebytes)
    return TRUE;

  char * newArray;

  if (theArray != NULL) {
    if (newsizebytes == 0) {
      if (allocatedDynamically)
        PFREE(theArray);
      newArray = NULL;
    }
    else if (allocatedDynamically) {
      if ((newArray = (char *)PREALLOC(theArray, newsizebytes)) == NULL)
        return FALSE;
    }
    else {
      if ((newArray = (char *)PMALLOC(newsizebytes)) == NULL)
        return FALSE;
      memcpy(newArray, theArray, PMIN(newsizebytes, oldsizebytes));
      allocatedDynamically = TRUE;
    }
  }
  else if (newsizebytes != 0) {
    if ((newArray = (char *)PMALLOC(newsizebytes)) == NULL)
      return FALSE;
  }
  else
    newArray = NULL;

  reference->size = newSize;

  if (newsizebytes > oldsizebytes)
    memset(newArray+oldsizebytes, 0, newsizebytes-oldsizebytes);

  theArray = newArray;
  return TRUE;
}


void PAbstractArray::Attach(const void *buffer, PINDEX bufferSize)
{
  if (allocatedDynamically && theArray != NULL)
    PFREE(theArray);

  theArray = (char *)buffer;
  reference->size = bufferSize;
  allocatedDynamically = FALSE;
}


void * PAbstractArray::GetPointer(PINDEX minSize)
{
  PAssert(SetMinSize(minSize), POutOfMemory);
  return theArray;
}


void PAbstractArray::PrintNumbersOn(ostream & strm, PINDEX size, BOOL is_signed) const
{
  PINDEX line_width = strm.width();
  if (line_width == 0)
    line_width = 16/size;

  PINDEX indent = strm.precision();

  PINDEX val_width;
  switch (strm.flags()&ios::basefield) {
    case ios::hex :
      val_width = size*2;
      is_signed = FALSE;
      break;
    case ios::oct :
      val_width = ((size*8)+2)/3;
      is_signed = FALSE;
      break;
    default :
      switch (size) {
        case 1 :
          val_width = 3;
          break;
        case 2 :
          val_width = 5;
          break;
        default :
          val_width = 10;
          break;
      }
      if (is_signed)
        val_width++;
  }

  long mask = -1;
  if (size < sizeof(mask))
    mask = (1L << (size*8)) - 1;

  PINDEX i = 0;
  while (i < GetSize()) {
    if (i > 0)
      strm << '\n';
    PINDEX j;
    for (j = 0; j < indent; j++)
      strm.put(' ');
    for (j = 0; j < line_width; j++) {
      if (j == line_width/2)
        strm.put(' ');
      if (i+j < GetSize()) {
        strm << setw(val_width);
        if (is_signed)
          strm << GetNumberValueAt(i+j);
        else
          strm << (DWORD)(GetNumberValueAt(i+j)&mask);
      }
      else {
        PINDEX k;
        for (k = 0; k < val_width; k++)
          strm.put(' ');
      }
      strm << ' ';
    }
    strm << "  ";
    for (j = 0; j < line_width; j++) {
      if (i+j < GetSize()) {
        long val = GetNumberValueAt(i+j);
        if (val >= 0 && val < 256 && isprint(val))
          strm << (char)val;
        else
          strm << '.';
      }
    }
    i += line_width;
  }
}


long PAbstractArray::GetNumberValueAt(PINDEX) const
{
  PAssertAlways(PUnimplementedFunction);
  return 0;
}


///////////////////////////////////////////////////////////////////////////////

void PCharArray::PrintOn(ostream & strm) const
{
  PINDEX width = strm.width();
  if (width > GetSize())
    width -= GetSize();
  else
    width = 0;

  BOOL left = (strm.flags()&ios::adjustfield) == ios::left;
  if (left)
    strm.write(theArray, GetSize());

  while (width-- > 0)
    strm.put(strm.fill());

  if (!left)
    strm.write(theArray, GetSize());
}


void PShortArray::PrintOn(ostream & strm) const
{
  PrintNumbersOn(strm, sizeof(short), TRUE);
}


long PShortArray::GetNumberValueAt(PINDEX idx) const
{
  return ((short *)theArray)[idx];
}


void PIntArray::PrintOn(ostream & strm) const
{
  PrintNumbersOn(strm, sizeof(int), TRUE);
}


long PIntArray::GetNumberValueAt(PINDEX idx) const
{
  return ((int *)theArray)[idx];
}


void PLongArray::PrintOn(ostream & strm) const
{
  PrintNumbersOn(strm, sizeof(long), TRUE);
}


long PLongArray::GetNumberValueAt(PINDEX idx) const
{
  return ((long *)theArray)[idx];
}


void PBYTEArray::PrintOn(ostream & strm) const
{
  PrintNumbersOn(strm, sizeof(BYTE), FALSE);
}


long PBYTEArray::GetNumberValueAt(PINDEX idx) const
{
  return ((BYTE *)theArray)[idx];
}


void PWORDArray::PrintOn(ostream & strm) const
{
  PrintNumbersOn(strm, sizeof(WORD), FALSE);
}


long PWORDArray::GetNumberValueAt(PINDEX idx) const
{
  return ((WORD *)theArray)[idx];
}


void PUnsignedArray::PrintOn(ostream & strm) const
{
  PrintNumbersOn(strm, sizeof(unsigned), FALSE);
}


long PUnsignedArray::GetNumberValueAt(PINDEX idx) const
{
  return ((unsigned *)theArray)[idx];
}


void PDWORDArray::PrintOn(ostream & strm) const
{
  PrintNumbersOn(strm, sizeof(DWORD), FALSE);
}


long PDWORDArray::GetNumberValueAt(PINDEX idx) const
{
  return ((DWORD *)theArray)[idx];
}


///////////////////////////////////////////////////////////////////////////////

#ifdef PHAS_UNICODE
#define PSTRING_COPY(d, s, l) UnicodeCopy((WORD *)(d), (s), (l))
#define PSTRING_MOVE(d, doff, s, soff, l) \
            memmove(((WORD*)(d))+(doff), ((WORD*)(s))+(soff), (l)*sizeof(WORD))
static void UnicodeCopy(WORD * theArray, char * src, size_t len)
{
  while (len-- > 0)
    *theArray++ = *src++;
}
#else
#define PSTRING_COPY(d, s, l) memcpy((d), (s), (l))
#define PSTRING_MOVE(d, doff, s, soff, l) memmove((d)+(doff), (s)+(soff), (l))
#endif

PString::PString(const char * cstr)
  : PSTRING_ANCESTOR_CLASS(strlen(PAssertNULL(cstr))+1)
{
  PSTRING_COPY(theArray, cstr, GetSize());
}


PString::PString(const WORD * ustr)
{
  const WORD * ptr = PAssertNULL(ustr);
  PINDEX len = 0;
  while (*ptr++ != 0)
    len++;
  SetSize(len+1);
#ifdef PHAS_UNICODE
  memcpy(theArray, ustr, len*sizeof(WORD))
#else
  char * cstr = theArray;
  while (len-- > 0)
    *cstr++ = (char)*ustr++;
#endif
}


PString::PString(const char * cstr, PINDEX len)
  : PSTRING_ANCESTOR_CLASS(len+1)
{
  PSTRING_COPY(theArray, PAssertNULL(cstr), len);
}


PString::PString(const WORD * ustr, PINDEX len)
  : PSTRING_ANCESTOR_CLASS(len+1)
{
  PAssertNULL(ustr);
#ifdef PHAS_UNICODE
  memcpy(theArray, ustr, len*sizeof(WORD))
#else
  char * cstr = theArray;
  while (len-- > 0)
    *cstr++ = (char)*ustr++;
#endif
}


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


PString::PString(ConversionType type, const char * str, ...)
{
  switch (type) {
    case Pascal :
      if (*str != '\0') {
        PINDEX len = *str & 0xff;
        PAssert(SetSize(len+1), POutOfMemory);
        PSTRING_COPY(theArray, str+1, len);
      }
      break;

    case Basic :
      if (str[0] != '\0' && str[1] != '\0') {
        PINDEX len = str[0] | (str[1] << 8);
        PAssert(SetSize(len+1), POutOfMemory);
        PSTRING_COPY(theArray, str+2, len);
      }
      break;

    case Literal :
      PAssert(SetSize(strlen(str)+1), POutOfMemory);
      TranslateEscapes(str, theArray);
      PAssert(MakeMinimumSize(), POutOfMemory);
      break;

    case Printf :
      va_list args;
      va_start(args, str);
      vsprintf(str, args);
      break;

    default :
      PAssertAlways(PInvalidParameter);
  }
}


static char * ltostr(DWORD value, unsigned base, char * str)
{
  if (value >= base)
    str = ltostr(value/base, base, str);
  value %= base;
  if (value < 10)
    *str = (char)(value + '0');
  else
    *str = (char)(value + 'A'-10);
  return str+1;
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


PString & PString::operator=(char ch)
{
  PString pstr(ch);
  PCharArray::operator=(pstr);
  return *this;
}


PObject * PString::Clone() const
{
  return PNEW PString(*this);
}


void PString::PrintOn(ostream &strm) const
{
  strm << theArray;
}


void PString::ReadFrom(istream &strm)
{
  SetMinSize(100);
  char * ptr = theArray;
  PINDEX len = 0;
  int c;
  while ((c = strm.get()) != EOF && c != '\n') {
    *ptr++ = (char)c;
    len++;
    if (len >= GetSize()) {
      SetSize(len + 100);
      ptr = theArray + len;
    }
  }
  *ptr = '\0';
  PAssert(MakeMinimumSize(), POutOfMemory);
}


PObject::Comparison PString::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PString::Class()), PInvalidCast);
  return InternalCompare(0, P_MAX_INDEX, ((const PString &)obj).theArray);
}


PINDEX PString::HashFunction() const
{
#ifdef PHAS_UNICODE
  return (((WORD*)theArray)[0]+((WORD*)theArray)[1]+((WORD*)theArray)[2])%23;
#else
  return ((BYTE)toupper(theArray[0]) +
          (BYTE)toupper(theArray[1]) +
          (BYTE)toupper(theArray[2]))%23;
#endif
}


BOOL PString::IsEmpty() const
{
#ifdef PHAS_UNICODE
  return *(WORD*)theArray == '\0';
#else
  return *theArray == '\0';
#endif
}


BOOL PString::SetSize(PINDEX newSize)
{
  if (IsUnique())
    return PAbstractArray::SetSize(newSize);

  PINDEX newsizebytes = elementSize*newSize;
  PINDEX oldsizebytes = elementSize*GetSize();
  char * newArray;

  if (newsizebytes == 0)
    newArray = NULL;
  else {
    if ((newArray = (char *)PMALLOC(newsizebytes)) == NULL)
      return FALSE;

    if (theArray != NULL)
      memcpy(newArray, theArray, PMIN(oldsizebytes, newsizebytes));
  }

  reference->count--;
  reference = new Reference(newSize);

  if (newsizebytes > oldsizebytes)
    memset(newArray+oldsizebytes, 0, newsizebytes-oldsizebytes);

  theArray = newArray;
  return TRUE;
}


BOOL PString::MakeUnique()
{
  if (IsUnique())
    return TRUE;

  SetSize(GetSize());
  return FALSE;
}


#ifdef PHAS_UNICODE
PINDEX PString::GetLength() const
{
  for (len = 0; len < GetSize(); len++)
    if (((WORD *)theArray)[len] == 0)
      break;
  return len;
}
#endif


PString PString::operator+(const char * cstr) const
{
  PINDEX olen = GetLength();
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  PString str;
  str.SetSize(olen+alen);
  PSTRING_MOVE(str.theArray, 0, theArray, 0, olen);
  PSTRING_COPY(str.theArray+olen, cstr, alen);
  return str;
}


PString PString::operator+(char c) const
{
  PINDEX olen = GetLength();
  PString str;
  str.SetSize(olen+2);
  PSTRING_MOVE(str.theArray, 0, theArray, 0, olen);
  str.theArray[olen] = c;
  return str;
}


PString & PString::operator+=(const char * cstr)
{
  PINDEX olen = GetLength();
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  SetSize(olen+alen);
  PSTRING_COPY(theArray+olen, cstr, alen);
  return *this;
}


PString & PString::operator+=(char ch)
{
  PINDEX olen = GetLength();
  SetSize(olen+2);
  theArray[olen] = ch;
  return *this;
}


PString PString::operator&(const char * cstr) const
{
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  if (alen == 1)
    return *this;
  PINDEX olen = GetLength();
  PString str;
  PINDEX space = olen > 0 && theArray[olen-1]!=' ' && *cstr!=' ' ? 1 : 0;
  str.SetSize(olen+alen+space);
  PSTRING_MOVE(str.theArray, 0, theArray, 0, olen);
  if (space != 0)
    str.theArray[olen] = ' ';
  PSTRING_COPY(str.theArray+olen+space, cstr, alen);
  return str;
}


PString PString::operator&(char c) const
{
  PINDEX olen = GetLength();
  PString str;
  PINDEX space = olen > 0 && theArray[olen-1] != ' ' && c != ' ' ? 1 : 0;
  str.SetSize(olen+2+space);
  PSTRING_MOVE(str.theArray, 0, theArray, 0, olen);
  if (space != 0)
    str.theArray[olen] = ' ';
  str.theArray[olen+space] = c;
  return str;
}


PString & PString::operator&=(const char * cstr)
{
  PINDEX alen = strlen(PAssertNULL(cstr))+1;
  if (alen == 1)
    return *this;
  PINDEX olen = GetLength();
  PINDEX space = olen > 0 && theArray[olen-1]!=' ' && *cstr!=' ' ? 1 : 0;
  SetSize(olen+alen+space);
  if (space != 0)
    theArray[olen] = ' ';
  PSTRING_COPY(theArray+olen+space, cstr, alen);
  return *this;
}


PString & PString::operator&=(char ch)
{
  PINDEX olen = GetLength();
  PINDEX space = olen > 0 && theArray[olen-1] != ' ' && ch != ' ' ? 1 : 0;
  SetSize(olen+2+space);
  if (space != 0)
    theArray[olen] = ' ';
  theArray[olen+space] = ch;
  return *this;
}


void PString::Delete(PINDEX start, PINDEX len)
{
  MakeUnique();

  register PINDEX slen = GetLength();
  if (start > slen)
    return;

  if (len > slen - start)
    SetAt(start, '\0');
  else
    PSTRING_MOVE(theArray, start, theArray, start+len, slen-start-len+1);
  MakeMinimumSize();
}


PString PString::operator()(PINDEX start, PINDEX end) const
{
  if (end < start)
    return PString();

  register PINDEX len = GetLength();
  if (start > len)
    return PString();

  if (end >= len) {
    if (start == 0)
      return *this;
    end = len-1;
  }
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


BOOL PString::operator*=(const char * cstr) const
{
  PAssertNULL(cstr);
  const char * pstr = theArray;
  while (*pstr != '\0' && *cstr != '\0') {
    if (toupper(*pstr) != toupper(*cstr))
      return FALSE;
    pstr++;
    cstr++;
  }
  return *pstr == *cstr;
}


PObject::Comparison PString::InternalCompare(PINDEX offset, char c) const
{
  char ch = theArray[offset];
  if (ch < c)
    return LessThan;
  if (ch > c)
    return GreaterThan;
  return EqualTo;
}


PObject::Comparison PString::InternalCompare(
                         PINDEX offset, PINDEX length, const char * cstr) const
{
  if (offset == 0 && theArray == cstr)
    return EqualTo;

  int retval;
  if (length == P_MAX_INDEX)
    retval = strcmp(theArray+offset, PAssertNULL(cstr));
  else
    retval = strncmp(theArray+offset, PAssertNULL(cstr), length);

  if (retval < 0)
    return LessThan;

  if (retval > 0)
    return GreaterThan;

  return EqualTo;
}


PINDEX PString::Find(char ch, PINDEX offset) const
{
  register PINDEX len = GetLength();
  while (offset < len) {
    if (InternalCompare(offset, ch) == EqualTo)
      return offset;
    offset++;
  }
  return P_MAX_INDEX;
}


PINDEX PString::Find(const char * cstr, PINDEX offset) const
{
  PAssertNULL(cstr);
  PAssert(*cstr != '\0', PInvalidParameter);

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
    strSum += toupper(theArray[offset+i]);
    cstrSum += toupper(cstr[i]);
  }

  // search for a matching substring
  while (offset+clen <= len) {
    if (strSum == cstrSum && InternalCompare(offset, clen, cstr) == EqualTo)
      return offset;
    strSum += toupper(theArray[offset+clen]);
    strSum -= toupper(theArray[offset++]);
  }

  return P_MAX_INDEX;
}


PINDEX PString::FindLast(char ch, PINDEX offset) const
{
  PINDEX len = GetLength();
  if (len == 0)
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
  PAssertNULL(cstr);
  PAssert(*cstr != '\0', PInvalidParameter);

  PINDEX len = GetLength();
  PINDEX clen = strlen(cstr);
  if (clen > len)
    return P_MAX_INDEX;

  if (offset == 0)
    return P_MAX_INDEX;

  if (offset > len - clen)
    offset = len - clen;

  int strSum = 0;
  int cstrSum = 0;
  for (PINDEX i = 0; i < clen; i++) {
    strSum += toupper(theArray[offset+i]);
    cstrSum += toupper(cstr[i]);
  }

  // search for a matching substring
  while (offset > 0) {
    if (strSum == cstrSum && InternalCompare(offset, clen, cstr) == EqualTo)
      return offset;
    strSum += toupper(theArray[--offset]);
    strSum -= toupper(theArray[offset+clen]);
  }

  return P_MAX_INDEX;
}


PINDEX PString::FindOneOf(const char * cset, PINDEX offset) const
{
  PAssertNULL(cset);
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


PINDEX PString::FindRegEx(const PRegularExpression & regex, PINDEX offset) const
{
  PINDEX pos = 0;
  PINDEX len = 0;
  if (FindRegEx(regex, pos, len, offset))
    return pos;

  return P_MAX_INDEX;
}


BOOL PString::FindRegEx(const PRegularExpression & regex,
                        PINDEX & pos,
                        PINDEX & len,
                        PINDEX offset,
                        PINDEX maxPos) const
{
  if (offset >= GetLength())
    return FALSE;

  if (!regex.Execute(&theArray[offset], pos, len, 0))
    return FALSE;

  pos += offset;
  if (pos+len > maxPos)
    return FALSE;

  return TRUE;
}


void PString::Replace(const PString & target,
                      const PString & subs,
                      BOOL all, PINDEX offset)
{
  MakeUnique();

  PINDEX tlen = target.GetLength();
  PINDEX slen = subs.GetLength();
  do {
    PINDEX pos = Find(target, offset);
    if (pos == P_MAX_INDEX)
      return;
    Splice(subs, pos, tlen);
    offset = pos + slen;
  } while (all);
}


void PString::Splice(const char * cstr, PINDEX pos, PINDEX len)
{
  register PINDEX slen = GetLength();
  if (pos >= slen)
    operator+=(cstr);
  else {
    MakeUnique();
    PINDEX clen = strlen(PAssertNULL(cstr));
    PINDEX newlen = slen-len+clen;
    if (clen > len)
      SetSize(newlen+1);
    if (pos+len < slen)
      PSTRING_MOVE(theArray, pos+clen, theArray, pos+len, slen-pos-len+1);
    PSTRING_COPY(theArray+pos, cstr, clen);
    theArray[newlen] = '\0';
  }
}


PStringArray
        PString::Tokenise(const char * separators, BOOL onePerSeparator) const
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
    if (p2 > p1)
      tokens[token] = operator()(p1, p2-1);
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
  const char * lpos = theArray;
  while (isspace(*lpos))
    lpos++;
  return PString(lpos);
}


PString PString::RightTrim() const
{
  char * rpos = theArray+GetLength()-1;
  if (isspace(*rpos))
    return *this;

  while (isspace(*rpos)) {
    if (rpos == theArray)
      return PString();
    rpos--;
  }
  return PString(theArray, rpos - theArray);
}


PString PString::Trim() const
{
  const char * lpos = theArray;
  while (isspace(*lpos))
    lpos++;
  if (*lpos == '\0')
    return PString();

  const char * rpos = theArray+GetLength()-1;
  if (!isspace(*rpos))
    return PString(lpos);

  while (isspace(*rpos))
    rpos--;
  return PString(lpos, rpos - lpos + 1);
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


DWORD PString::AsUnsigned(unsigned base) const
{
  PAssert(base >= 2 && base <= 36, PInvalidParameter);
  char * dummy;
  return strtoul(theArray, &dummy, base);
}


double PString::AsReal() const
{
  char * dummy;
  return strtod(theArray, &dummy);
}


PBYTEArray PString::ToPascal() const
{
  PINDEX len = GetLength();
  PAssert(len < 256, "Cannot convert to PASCAL string");
  BYTE buf[256];
  buf[0] = (BYTE)len;
#ifdef PHAS_UNICODE
  WORD * ptr = (WORD *)theArray;
  while (len > 0) {
    buf[len] = (BYTE)(*ptr < 256 ? *ptr : 255);
    len--;
  }
#else
  memcpy(&buf[1], theArray, len);
#endif
  return PBYTEArray(buf, len+1);
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
      PINDEX i;
      for (i = 0; i < PARRAYSIZE(PStringEscapeValue); i++) {
        if (*p == PStringEscapeValue[i]) {
          str += PString('\\') + PStringEscapeCode[i];
          break;
        }
      }
      if (i >= PARRAYSIZE(PStringEscapeValue))
        str.sprintf("\\%03o", *p & 0xff);
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
  char * p = GetPointer(1000);
  ::vsprintf(p+strlen(p), fmt, arg);
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


PString pvsprintf(const char * fmt, va_list arg)
{
  PString str;
  return str.vsprintf(fmt, arg);
}


///////////////////////////////////////////////////////////////////////////////

PObject * PCaselessString::Clone() const
{
  return PNEW PCaselessString(*this);
}


PObject::Comparison PCaselessString::InternalCompare(PINDEX offset, char c) const
{
  int c1 = toupper(theArray[offset]);
  int c2 = toupper(c);
  if (c1 < c2)
    return LessThan;
  if (c1 > c2)
    return GreaterThan;
  return EqualTo;
}


PObject::Comparison PCaselessString::InternalCompare(
                         PINDEX offset, PINDEX length, const char * cstr) const
{
  PAssertNULL(cstr);
  while (length-- > 0 && (theArray[offset] != '\0' || *cstr != '\0')) {
    Comparison c = PCaselessString::InternalCompare(offset++, *cstr++);
    if (c != EqualTo)
      return c;
  }
  return EqualTo;
}



///////////////////////////////////////////////////////////////////////////////

int PStringStream::Buffer::overflow(int c)
{
  if (pptr() >= epptr()) {
    int gpos = gptr() - eback();
    int ppos = pptr() - pbase();
    char * newptr = string->GetPointer(string->GetSize() + 10);
    setp(newptr, newptr + string->GetSize() - 1);
    pbump(ppos);
    setg(newptr, newptr + gpos, newptr + ppos);
  }
  if (c != EOF) {
    *pptr() = (char)c;
    pbump(1);
  }
  return 0;
}


int PStringStream::Buffer::underflow()
{
  return gptr() >= egptr() ? EOF : *gptr();
}


int PStringStream::Buffer::sync()
{
  char * base = string->GetPointer();
  char * end = base + string->GetLength();
  setg(base, base, end);
  setp(end, base + string->GetSize() - 1);
  return 0;
}


streampos PStringStream::Buffer::seekoff(streamoff off,
#ifdef __MWERKS__
                                 ios::seekdir dir, ios::openmode mode)
#else
                                 ios::seek_dir dir, int mode)
#endif
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
  init(new PStringStream::Buffer(this));
}


PStringStream::PStringStream(const PString & str)
  : PString(str)
{
  init(new PStringStream::Buffer(this));
}


PStringStream::PStringStream(const char * cstr)
  : PString(cstr)
{
  init(new PStringStream::Buffer(this));
}


PStringStream & PStringStream::operator=(const char * cstr)
{
  PString::operator=(cstr);
  flush();
  return *this;
}


PStringStream & PStringStream::operator=(const PString & str)
{
  PString::operator=(str);
  flush();
  return *this;
}


PStringStream::~PStringStream()
{
  delete (PStringStream::Buffer *)rdbuf();
  init(NULL);
}


///////////////////////////////////////////////////////////////////////////////

PStringArray::PStringArray(PINDEX count,
                                    char const * const * strarr, BOOL caseless)
{
  if (count == 0)
    return;

  PAssertNULL(strarr);
  SetSize(count);
  for (PINDEX i = 0; i < count; i++) {
    PString * newString;
    if (caseless)
      newString = PNEW PCaselessString(strarr[i]);
    else
      newString = PNEW PString(strarr[i]);
    SetAt(i, newString);
  }
}


PString & PStringArray::operator[](PINDEX index)
{
  PASSERTINDEX(index);
  PAssert(SetMinSize(index+1), POutOfMemory);
  if ((*theArray)[index] == NULL)
    (*theArray)[index] = PNEW PString;
  return *(PString *)(*theArray)[index];
}


///////////////////////////////////////////////////////////////////////////////

POrdinalToString::POrdinalToString(PINDEX count, const Initialiser * init)
{
  while (count-- > 0) {
    SetAt(init->key, init->value);
    init++;
  }
}


///////////////////////////////////////////////////////////////////////////////

PStringToOrdinal::PStringToOrdinal(PINDEX count,
                                   const Initialiser * init,
                                   BOOL caseless)
{
  while (count-- > 0) {
    if (caseless)
      SetAt(PCaselessString(init->key), init->value);
    else
      SetAt(init->key, init->value);
    init++;
  }
}


///////////////////////////////////////////////////////////////////////////////

PStringToString::PStringToString(PINDEX count,
                                 const Initialiser * init,
                                 BOOL caselessKeys,
                                 BOOL caselessValues)
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


///////////////////////////////////////////////////////////////////////////////

PRegularExpression::PRegularExpression()
{
  lastError = NotCompiled;
  expression = NULL;
}


PRegularExpression::PRegularExpression(const PString & pattern, int flags)
{
  expression = NULL;
  Compile(pattern, flags);
}


PRegularExpression::PRegularExpression(const char * pattern, int flags)
{
  expression = NULL;
  Compile(pattern, flags);
}


PRegularExpression::~PRegularExpression()
{
  if (expression != NULL) {
    regfree(expression);
    delete expression;
  }
}


PRegularExpression::ErrorCodes PRegularExpression::GetErrorCode() const
{
  return (ErrorCodes)lastError;
}


PString PRegularExpression::GetErrorText() const
{
  PString str;
  regerror(lastError, expression, str.GetPointer(256), 256);
  return str;
}


BOOL PRegularExpression::Compile(const PString & pattern, int flags)
{
  return Compile((const char *)pattern, flags);
}


BOOL PRegularExpression::Compile(const char * pattern, int flags)
{
  if (expression != NULL) {
    regfree(expression);
    delete expression;
  }
  if (pattern == NULL || *pattern == '\0')
    return BadPattern;

  expression = new regex_t;
  lastError = regcomp(expression, pattern, flags);
  return lastError == NoError;
}


BOOL PRegularExpression::Execute(const PString & str, PINDEX & start, int flags) const
{
  PINDEX dummy;
  return Execute((const char *)str, start, dummy, flags);
}


BOOL PRegularExpression::Execute(const PString & str, PINDEX & start, PINDEX & len, int flags) const
{
  return Execute((const char *)str, start, len, flags);
}


BOOL PRegularExpression::Execute(const char * cstr, PINDEX & start, int flags) const
{
  PINDEX dummy;
  return Execute(cstr, start, dummy, flags);
}


BOOL PRegularExpression::Execute(const char * cstr, PINDEX & start, PINDEX & len, int flags) const
{
  if (expression == NULL) {
    ((PRegularExpression*)this)->lastError = NotCompiled;
    return FALSE;
  }

  regmatch_t match;

  ((PRegularExpression*)this)->lastError = regexec(expression, cstr, 1, &match, flags);
  if (lastError != NoError)
    return FALSE;

  start = match.rm_so;
  len = match.rm_eo - start;
  return TRUE;
}


BOOL PRegularExpression::Execute(const PString & str, PIntArray & starts, int flags) const
{
  PIntArray dummy;
  return Execute((const char *)str, starts, dummy, flags);
}


BOOL PRegularExpression::Execute(const PString & str,
                                 PIntArray & starts,
                                 PIntArray & ends,
                                 int flags) const
{
  return Execute((const char *)str, starts, ends, flags);
}


BOOL PRegularExpression::Execute(const char * cstr, PIntArray & starts, int flags) const
{
  PIntArray dummy;
  return Execute(cstr, starts, dummy, flags);
}


BOOL PRegularExpression::Execute(const char * cstr,
                                 PIntArray & starts,
                                 PIntArray & ends,
                                 int flags) const
{
  if (expression == NULL) {
    ((PRegularExpression*)this)->lastError = NotCompiled;
    return FALSE;
  }

  regmatch_t single_match;
  regmatch_t * matches = &single_match;

  PINDEX count = starts.GetSize();
  if (count > 1)
    matches = new regmatch_t[count];
  else
    count = 1;

  ((PRegularExpression*)this)->lastError = regexec(expression, cstr, count, matches, flags);

  if (lastError == NoError) {
    starts.SetMinSize(count);
    ends.SetMinSize(count);
    for (PINDEX i = 0; i < count; i++) {
      starts[i] = matches[i].rm_so;
      ends[i] = matches[i].rm_eo;
    }
  }

  if (matches != &single_match)
    delete [] matches;

  return lastError == NoError;
}


PString PRegularExpression::EscapeString(const PString & str)
{
  PString translated;

  PINDEX lastPos = 0;
  PINDEX nextPos;
  while ((nextPos = str.FindOneOf("\\^$+?*.[]()|{}", lastPos+1)) != P_MAX_INDEX) {
    translated += str(lastPos, nextPos-1) + "\\";
    lastPos = nextPos;
  }

  if (lastPos == 0)
    return str;

  return translated + str.Mid(lastPos);
}


// End Of File ///////////////////////////////////////////////////////////////
