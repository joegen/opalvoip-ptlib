/*
 * asner.cxx
 *
 * Abstract Syntax Notation 1 Encoding Rules
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
 * $Log: asner.cxx,v $
 * Revision 1.49  2001/08/07 02:49:05  robertj
 * Fixed incorrect alignment if constrained string upper bound is exactly
 *     16 bits long. thanks Guntram Diehl & Thomas Arimont.
 *
 * Revision 1.48  2001/08/06 09:35:25  robertj
 * Fixed GNU compatibility.
 *
 * Revision 1.47  2001/08/06 09:31:48  robertj
 * Added conversion of BMPString to PString without losing special characters.
 *
 * Revision 1.46  2001/08/06 01:39:02  robertj
 * Added assignement operator with RHS of PASN_BMPString to classes
 *   descended from PASN_BMPString.
 *
 * Revision 1.45  2001/06/14 02:14:12  robertj
 * Added functions to encode and decode another ASN type that is inside
 *   an octet string, useful for ANY or EXTERNAL types etc.
 *
 * Revision 1.44  2001/05/29 00:59:16  robertj
 * Fixed excessive padding on constrained strings.
 *
 * Revision 1.43  2001/05/22 23:37:42  robertj
 * Fixed problem with assigning a constrained string value to itself, which
 *   can occur when changing constraints.
 *
 * Revision 1.42  2001/04/30 10:47:33  robertj
 * Fixed stupid error in last patch.
 *
 * Revision 1.41  2001/04/30 06:47:04  robertj
 * Fixed problem with en/decoding more than 16 extension fields in a sequence.
 *
 * Revision 1.40  2001/04/26 08:15:58  robertj
 * Fixed problem with ASN compile of single constraints on enumerations.
 *
 * Revision 1.39  2001/04/23 05:46:06  robertj
 * Fixed problem with unconstrained PASN_NumericString coding in 8 bits
 *   instead of 4, thanks Chew Kuan.
 *
 * Revision 1.38  2001/04/23 04:40:14  robertj
 * Added ASN standard types GeneralizedTime and UTCTime
 *
 * Revision 1.37  2001/04/12 03:26:59  robertj
 * Fixed PASN_Boolean cosntructor to be compatible with usage in ASN parser.
 * Changed all PASN_xxx types so constructor can take real type as only
 *   parameter. eg PASN_OctetString s = "fred";
 * Changed block encode/decode so does not do a ByteAlign() if zero
 *   length, required for interoperability even though spec implies otherwise..
 *
 * Revision 1.36  2001/01/24 04:37:07  robertj
 * Added more bulletproofing to ASN structures to obey constraints.
 *
 * Revision 1.35  2001/01/03 01:20:13  robertj
 * Fixed error in BlockEncode, should ByteAlign() even on zero length strings.
 *
 * Revision 1.34  2000/10/26 11:09:16  robertj
 * More bullet proofing of PER decoder, changed bit type to be unsigned.
 *
 * Revision 1.33  2000/10/26 01:29:32  robertj
 * Fixed MSVC warning.
 *
 * Revision 1.32  2000/10/25 04:05:38  robertj
 * More bullet proofing of PER decoder.
 *
 * Revision 1.31  2000/09/29 04:11:51  robertj
 * Fixed possible out of range memory access, thanks Petr Par�zek <paryzek@wo.cz>
 *
 * Revision 1.30  2000/02/29 06:32:12  robertj
 * Added ability to remove optional field in sequence, thanks Dave Harvey.
 *
 * Revision 1.29  2000/01/20 06:22:22  robertj
 * Fixed boundary condition error for constrained integer encoding (values 1, 256 etc)
 *
 * Revision 1.28  1999/11/22 23:15:43  robertj
 * Fixed bug in PASN_Choice::Compare(), should make sure choices are the same before comparing.
 *
 * Revision 1.27  1999/08/19 15:43:07  robertj
 * Fixed incorrect size of OID if zero length encoded.
 *
 * Revision 1.26  1999/08/09 13:02:45  robertj
 * dded ASN compiler #defines for backward support of pre GCC 2.9 compilers.
 * Added ASN compiler #defines to reduce its memory footprint.
 *
 * Revision 1.25  1999/08/08 15:45:59  robertj
 * Fixed incorrect encoding of unknown extensions.
 *
 * Revision 1.24  1999/08/05 00:44:28  robertj
 * Fixed PER encoding problems for large integer values.
 *
 * Revision 1.23  1999/07/22 06:48:54  robertj
 * Added comparison operation to base ASN classes and compiled ASN code.
 * Added support for ANY type in ASN parser.
 *
 * Revision 1.22  1999/07/08 08:39:12  robertj
 * Fixed bug when assigning negative number ot cosntrained PASN_Integer
 *
 * Revision 1.21  1999/06/30 08:58:12  robertj
 * Fixed bug in encoding/decoding OID greater than 2.39
 *
 * Revision 1.20  1999/06/17 13:27:09  robertj
 * Fixed bug causing crashes on pass through of unknown extensions.
 *
 * Revision 1.19  1999/06/07 00:31:25  robertj
 * Fixed signed/unsigned problem with number of unknown extensions check.
 *
 * Revision 1.18  1999/04/26 05:58:48  craigs
 * Fixed problems with encoding of extensions
 *
 * Revision 1.17  1999/03/09 08:12:38  robertj
 * Fixed problem with closing a steam encoding twice.
 *
 * Revision 1.16  1999/01/16 01:28:25  robertj
 * Fixed problems with reading stream multiple times.
 *
 * Revision 1.15  1998/11/30 04:50:44  robertj
 * New directory structure
 *
 * Revision 1.14  1998/10/22 04:33:11  robertj
 * Fixed bug in constrained strings and PER, incorrect order of character set.
 *
 * Revision 1.13  1998/09/23 06:21:49  robertj
 * Added open source copyright license.
 *
 * Revision 1.12  1998/05/26 05:29:23  robertj
 * Workaroung for g++ iostream bug.
 *
 * Revision 1.11  1998/05/21 04:58:54  robertj
 * GCC comptaibility.
 *
 * Revision 1.10  1998/05/21 04:26:54  robertj
 * Fixed numerous PER problems.
 *
 * Revision 1.9  1998/05/11 06:01:55  robertj
 * Why did this compile under MSC?
 *
 * Revision 1.8  1998/05/07 05:19:29  robertj
 * Fixed problems with using copy constructor/assignment oeprator on PASN_Objects.
 *
 * Revision 1.7  1998/03/05 12:49:50  robertj
 * MemCheck fixes.
 *
 * Revision 1.6  1998/02/03 06:28:27  robertj
 * Fixed length calculation of integers in BER.
 * Added new function to read a block with minimum number of bytes.
 *
 * Revision 1.5  1998/01/26 01:51:20  robertj
 * Removed uninitialised variable warnings.
 *
 * Revision 1.4  1997/12/18 05:07:56  robertj
 * Fixed bug in choice name display.
 * Added function to get choice discriminator name.
 * Fixed bug in encoding extensions.
 *
 * Revision 1.3  1997/12/11 10:36:22  robertj
 * Support for new ASN parser.
 *
 */

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "asner.h"
#endif

#include <ptclib/asner.h>

#define new PNEW


static PINDEX CountBits(unsigned range)
{
  if (range == 0)
    return sizeof(unsigned)*8;

  PINDEX nBits = 0;
  while (nBits < (sizeof(unsigned)*8) && range > (unsigned)(1 << nBits))
    nBits++;
  return nBits;
}


///////////////////////////////////////////////////////////////////////

PASN_Object::PASN_Object(unsigned theTag, TagClass theTagClass, BOOL extend)
{
  extendable = extend;

  tag = theTag;

  if (theTagClass != DefaultTagClass)
    tagClass = theTagClass;
  else
    tagClass = ContextSpecificTagClass;
}


void PASN_Object::SetTag(unsigned newTag, TagClass tagClass_)
{
  tag = newTag;
  if (tagClass_ != DefaultTagClass)
    tagClass = tagClass_;
}


PINDEX PASN_Object::GetObjectLength() const
{
  PINDEX len = 1;

  if (tag >= 31)
    len += (CountBits(tag)+6)/7;

  PINDEX dataLen = GetDataLength();
  if (dataLen < 128)
    len++;
  else
    len += (CountBits(dataLen)+7)/8 + 1;

  return len + dataLen;
}


void PASN_Object::SetConstraintBounds(ConstraintType, int, unsigned)
{
}


void PASN_Object::SetCharacterSet(ConstraintType, const char *)
{
}


void PASN_Object::SetCharacterSet(ConstraintType, unsigned, unsigned)
{
}


///////////////////////////////////////////////////////////////////////

PASN_ConstrainedObject::PASN_ConstrainedObject(unsigned tag, TagClass tagClass)
  : PASN_Object(tag, tagClass)
{
  constraint = Unconstrained;
  lowerLimit = 0;
  upperLimit =  UINT_MAX;
}


void PASN_ConstrainedObject::SetConstraintBounds(ConstraintType ctype,
                                                 int lower, unsigned upper)
{
  constraint = ctype;
  if (constraint == Unconstrained) {
    lower = 0;
    upper = UINT_MAX;
  }

  extendable = ctype == ExtendableConstraint;
  PAssert((lower >= 0 || upper < 0x7fffffff) &&
          (lower < 0 || (unsigned)lower <= upper), PInvalidParameter);
  lowerLimit = lower;
  upperLimit = upper;
}


int PASN_ConstrainedObject::ConstrainedLengthDecode(PPER_Stream & strm, unsigned & length)
{
  // The execution order is important in the following. The SingleBitDecode() function
  // must be called if extendable is TRUE, no matter what.
  if ((extendable && strm.SingleBitDecode()) || constraint == Unconstrained)
    return strm.LengthDecode(0, INT_MAX, length);
  else
    return strm.LengthDecode(lowerLimit, upperLimit, length);
}


void PASN_ConstrainedObject::ConstrainedLengthEncode(PPER_Stream & strm, unsigned length) const
{
  if (ConstraintEncode(strm, length)) // 26.4
    strm.LengthEncode(length, 0, INT_MAX);
  else
    strm.LengthEncode(length, lowerLimit, upperLimit);
}


BOOL PASN_ConstrainedObject::ConstraintEncode(PPER_Stream & strm, unsigned value) const
{
  if (!extendable)
    return constraint != FixedConstraint;

  BOOL needsExtending = value > upperLimit;

  if (!needsExtending) {
    if (lowerLimit < 0) {
      if ((int)value < lowerLimit)
        needsExtending = TRUE;
    }
    else {
      if (value < (unsigned)lowerLimit)
        needsExtending = TRUE;
    }
  }

  strm.SingleBitEncode(needsExtending);

  return needsExtending || constraint != FixedConstraint;
}


///////////////////////////////////////////////////////////////////////

PASN_Null::PASN_Null(unsigned tag, TagClass tagClass)
  : PASN_Object(tag, tagClass)
{
}


PObject::Comparison PASN_Null::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Null::Class()), PInvalidCast);
  return EqualTo;
}


PObject * PASN_Null::Clone() const
{
  PAssert(IsClass(PASN_Null::Class()), PInvalidCast);
  return new PASN_Null(*this);
}


void PASN_Null::PrintOn(ostream & strm) const
{
  strm << "<<null>>";
}


PString PASN_Null::GetTypeAsString() const
{
  return "Null";
}


PINDEX PASN_Null::GetDataLength() const
{
  return 0;
}


BOOL PASN_Null::Decode(PASN_Stream & strm)
{
  return strm.NullDecode(*this);
}


void PASN_Null::Encode(PASN_Stream & strm) const
{
  strm.NullEncode(*this);
}


BOOL PBER_Stream::NullDecode(PASN_Null & value)
{
  unsigned len;
  if (!HeaderDecode(value, len))
    return FALSE;

  byteOffset += len;
  return TRUE;
}


void PBER_Stream::NullEncode(const PASN_Null & value)
{
  HeaderEncode(value);
}


BOOL PPER_Stream::NullDecode(PASN_Null &)
{
  return TRUE;
}


void PPER_Stream::NullEncode(const PASN_Null &)
{
}


///////////////////////////////////////////////////////////////////////

PASN_Boolean::PASN_Boolean(BOOL val)
  : PASN_Object(UniversalBoolean, UniversalTagClass)
{
  value = val;
}


PASN_Boolean::PASN_Boolean(unsigned tag, TagClass tagClass, BOOL val)
  : PASN_Object(tag, tagClass)
{
  value = val;
}


PObject::Comparison PASN_Boolean::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Boolean::Class()), PInvalidCast);
  return value == ((const PASN_Boolean &)obj).value ? EqualTo : GreaterThan;
}


PObject * PASN_Boolean::Clone() const
{
  PAssert(IsClass(PASN_Boolean::Class()), PInvalidCast);
  return new PASN_Boolean(*this);
}


void PASN_Boolean::PrintOn(ostream & strm) const
{
  if (value)
    strm << "TRUE";
  else
    strm << "FALSE";
}


PString PASN_Boolean::GetTypeAsString() const
{
  return "Boolean";
}


PINDEX PASN_Boolean::GetDataLength() const
{
  return 1;
}


BOOL PASN_Boolean::Decode(PASN_Stream & strm)
{
  return strm.BooleanDecode(*this);
}


void PASN_Boolean::Encode(PASN_Stream & strm) const
{
  strm.BooleanEncode(*this);
}


BOOL PBER_Stream::BooleanDecode(PASN_Boolean & value)
{
  unsigned len;
  if (!HeaderDecode(value, len))
    return FALSE;

  while (len-- > 0) {
    if (IsAtEnd())
      return FALSE;
    value = (BOOL)ByteDecode();
  }

  return TRUE;
}


void PBER_Stream::BooleanEncode(const PASN_Boolean & value)
{
  HeaderEncode(value);
  ByteEncode((BOOL)value);
}


BOOL PPER_Stream::BooleanDecode(PASN_Boolean & value)
{
  if (IsAtEnd())
    return FALSE;

  // X.931 Section 11
  value = (BOOL)SingleBitDecode();
  return TRUE;
}


void PPER_Stream::BooleanEncode(const PASN_Boolean & value)
{
  // X.931 Section 11
  SingleBitEncode((BOOL)value);
}


///////////////////////////////////////////////////////////////////////

PASN_Integer::PASN_Integer(unsigned val)
  : PASN_ConstrainedObject(UniversalInteger, UniversalTagClass)
{
  value = val;
}


PASN_Integer::PASN_Integer(unsigned tag, TagClass tagClass, unsigned val)
  : PASN_ConstrainedObject(tag, tagClass)
{
  value = val;
}


PASN_Integer & PASN_Integer::operator=(unsigned val)
{
  if (constraint == Unconstrained)
    value = val;
  else if (lowerLimit >= 0) { // Is unsigned integer
    if (val < (unsigned)lowerLimit)
      value = lowerLimit;
    else if (val > upperLimit)
      value = upperLimit;
    else
      value = val;
  }
  else {
    int ival = (int)val;
    if (ival < lowerLimit)
      value = lowerLimit;
    else if (upperLimit < INT_MAX && ival > (int)upperLimit)
      value = upperLimit;
    else
      value = val;
  }

  return *this;
}


PObject::Comparison PASN_Integer::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Integer::Class()), PInvalidCast);
  const PASN_Integer & other = (const PASN_Integer &)obj;

  if (value < other.value)
    return LessThan;

  if (value > other.value)
    return GreaterThan;

  return EqualTo;
}


PObject * PASN_Integer::Clone() const
{
  PAssert(IsClass(PASN_Integer::Class()), PInvalidCast);
  return new PASN_Integer(*this);
}


void PASN_Integer::PrintOn(ostream & strm) const
{
  if (lowerLimit < 0)
    strm << (int)value;
  else
    strm << value;
}


void PASN_Integer::SetConstraintBounds(ConstraintType type, int lower, unsigned upper)
{
  PASN_ConstrainedObject::SetConstraintBounds(type, lower, upper);
  if (constraint != Unconstrained) {
    if ((int)value < lowerLimit)
      value = lowerLimit;
    else if (value > upperLimit)
      value = upperLimit;
  }
}


PString PASN_Integer::GetTypeAsString() const
{
  return "Integer";
}


static PINDEX GetIntegerDataLength(int value)
{
  // create a mask which is the top nine bits of a DWORD, or 0xFF800000
  // on a big endian machine
  int shift = (sizeof(value)-1)*8-1;

  // remove all sequences of nine 0's or 1's at the start of the value
  while (shift > 0 && ((value >> shift)&0x1ff) == (value < 0 ? 0x1ff : 0))
    shift -= 8;

  return (shift+9)/8;
}


PINDEX PASN_Integer::GetDataLength() const
{
  return GetIntegerDataLength(value);
}


BOOL PASN_Integer::Decode(PASN_Stream & strm)
{
  return strm.IntegerDecode(*this);
}


void PASN_Integer::Encode(PASN_Stream & strm) const
{
  strm.IntegerEncode(*this);
}


BOOL PBER_Stream::IntegerDecode(PASN_Integer & value)
{
  unsigned len;
  if (!HeaderDecode(value, len) || len == 0 || IsAtEnd())
    return FALSE;

  int accumulator = (char)ByteDecode(); // sign extended first byte
  while (--len > 0) {
    if (IsAtEnd())
      return FALSE;
    accumulator = (accumulator << 8) | ByteDecode();
  }

  value = accumulator;
  return TRUE;
}


void PBER_Stream::IntegerEncode(const PASN_Integer & value)
{
  HeaderEncode(value);
  // output the integer bits
  for (int count = GetIntegerDataLength(value)-1; count >= 0; count--)
    ByteEncode(value >> (count*8));
}


BOOL PPER_Stream::IntegerDecode(PASN_Integer & value)
{
  return value.DecodePER(*this);
}


void PPER_Stream::IntegerEncode(const PASN_Integer & value)
{
  value.EncodePER(*this);
}


BOOL PASN_Integer::DecodePER(PPER_Stream & strm)
{
  // X.931 Sections 12

  if ((extendable && strm.SingleBitDecode()) || constraint != FixedConstraint) { //  12.1
    unsigned len;
    if (strm.LengthDecode(0, INT_MAX, len) != 0)
      return FALSE;
    return strm.MultiBitDecode(len*8, value);
  }

  if ((unsigned)lowerLimit != upperLimit)  // 12.2.1
    return strm.UnsignedDecode(lowerLimit, upperLimit, value) == 0; // 12.2.2 which devolves to 10.5

  value = lowerLimit;
  return TRUE;
}


void PASN_Integer::EncodePER(PPER_Stream & strm) const
{
  // X.931 Sections 12

  if (ConstraintEncode(strm, (int)value)) { //  12.1
    PINDEX nBytes;
    unsigned adjusted_value = value - lowerLimit;
    if (adjusted_value == 0)
      nBytes = 1;
    else {
      PINDEX nBits = CountBits(adjusted_value+1);
      nBytes = (nBits+7)/8;
    }
    strm.LengthEncode(nBytes, 0, INT_MAX);
    strm.MultiBitEncode(adjusted_value, nBytes*8);
    return;
  }

  if ((unsigned)lowerLimit == upperLimit) // 12.2.1
    return;

  // 12.2.2 which devolves to 10.5
  strm.UnsignedEncode(value, lowerLimit, upperLimit);
}


///////////////////////////////////////////////////////////////////////

PASN_Enumeration::PASN_Enumeration(unsigned val)
  : PASN_Object(UniversalEnumeration, UniversalTagClass, FALSE)
{
  value = val;
  maxEnumValue = P_MAX_INDEX;
}


PASN_Enumeration::PASN_Enumeration(unsigned tag, TagClass tagClass,
                                   unsigned maxEnum, BOOL extend,
                                   unsigned val)
  : PASN_Object(tag, tagClass, extend)
{
  value = val;
  maxEnumValue = maxEnum;
}


static POrdinalToString BuildNamesDict(const PString & nameSpec)
{
  POrdinalToString names;

  PStringArray nameList = nameSpec.Tokenise(' ', FALSE);

  int num = 0;
  for (PINDEX i = 0; i < nameList.GetSize(); i++) {
    const PString & thisName = nameList[i];
    if (!thisName) {
      PINDEX equalPos = thisName.Find('=');
      if (equalPos != P_MAX_INDEX)
        num = (int)thisName.Mid(equalPos+1).AsInteger();
      names.SetAt(POrdinalKey(num), thisName.Left(equalPos));
      num++;
    }
  }

  return names;
}


PASN_Enumeration::PASN_Enumeration(unsigned tag, TagClass tagClass,
                                   unsigned maxEnum, BOOL extend,
                                   const PString & nameSpec,
                                   unsigned val)
  : PASN_Object(tag, tagClass, extend),
    names(BuildNamesDict(nameSpec))
{
  PAssert(maxEnum > 0, PInvalidParameter);
  maxEnumValue = maxEnum;

  PAssert(val < maxEnum, PInvalidParameter);
  value = val;
}


PObject::Comparison PASN_Enumeration::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Enumeration::Class()), PInvalidCast);
  const PASN_Enumeration & other = (const PASN_Enumeration &)obj;

  if (value < other.value)
    return LessThan;

  if (value > other.value)
    return GreaterThan;

  return EqualTo;
}


PObject * PASN_Enumeration::Clone() const
{
  PAssert(IsClass(PASN_Enumeration::Class()), PInvalidCast);
  return new PASN_Enumeration(*this);
}


void PASN_Enumeration::PrintOn(ostream & strm) const
{
  if (names.Contains(value))
    strm << names[value];
  else
    strm << '<' << value << '>';
}


PString PASN_Enumeration::GetTypeAsString() const
{
  return "Enumeration";
}


PINDEX PASN_Enumeration::GetDataLength() const
{
  return GetIntegerDataLength(value);
}


BOOL PASN_Enumeration::Decode(PASN_Stream & strm)
{
  return strm.EnumerationDecode(*this);
}


void PASN_Enumeration::Encode(PASN_Stream & strm) const
{
  strm.EnumerationEncode(*this);
}


BOOL PBER_Stream::EnumerationDecode(PASN_Enumeration & value)
{
  unsigned len;
  if (!HeaderDecode(value, len) || len == 0 || IsAtEnd())
    return FALSE;

  unsigned val = 0;
  while (len-- > 0) {
    if (IsAtEnd())
      return FALSE;
    val = (val << 8) | ByteDecode();
  }

  value = val;
  return TRUE;
}


void PBER_Stream::EnumerationEncode(const PASN_Enumeration & value)
{
  HeaderEncode(value);
  // output the integer bits
  for (int count = GetIntegerDataLength(value)-1; count >= 0; count--)
    ByteEncode(value >> (count*8));
}


BOOL PPER_Stream::EnumerationDecode(PASN_Enumeration & value)
{
  return value.DecodePER(*this);
}


void PPER_Stream::EnumerationEncode(const PASN_Enumeration & value)
{
  value.EncodePER(*this);
}


BOOL PASN_Enumeration::DecodePER(PPER_Stream & strm)
{
  // X.691 Section 13

  if (extendable) {  // 13.3
    if (strm.SingleBitDecode()) {
      unsigned len = 0;
      return strm.SmallUnsignedDecode(len) &&
             len > 0 &&
             strm.UnsignedDecode(0, len-1, value) == 0;
    }
  }

  return strm.UnsignedDecode(0, maxEnumValue, value) == 0;  // 13.2
}


void PASN_Enumeration::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 13

  if (extendable) {  // 13.3
    BOOL extended = value > maxEnumValue;
    strm.SingleBitEncode(extended);
    if (extended) {
      strm.SmallUnsignedEncode(1+value);
      strm.UnsignedEncode(value, 0, value);
      return;
    }
  }

  strm.UnsignedEncode(value, 0, maxEnumValue);  // 13.2
}


///////////////////////////////////////////////////////////////////////

PASN_Real::PASN_Real(double val)
  : PASN_Object(UniversalReal, UniversalTagClass)
{
  value = val;
}


PASN_Real::PASN_Real(unsigned tag, TagClass tagClass, double val)
  : PASN_Object(tag, tagClass)
{
  value = val;
}


PObject::Comparison PASN_Real::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Real::Class()), PInvalidCast);
  const PASN_Real & other = (const PASN_Real &)obj;

  if (value < other.value)
    return LessThan;

  if (value > other.value)
    return GreaterThan;

  return EqualTo;
}


PObject * PASN_Real::Clone() const
{
  PAssert(IsClass(PASN_Real::Class()), PInvalidCast);
  return new PASN_Real(*this);
}


void PASN_Real::PrintOn(ostream & strm) const
{
  strm << value;
}


PString PASN_Real::GetTypeAsString() const
{
  return "Real";
}


PINDEX PASN_Real::GetDataLength() const
{
  PAssertAlways(PUnimplementedFunction);
  return 0;
}


BOOL PASN_Real::Decode(PASN_Stream & strm)
{
  return strm.RealDecode(*this);
}


void PASN_Real::Encode(PASN_Stream & strm) const
{
  strm.RealEncode(*this);
}


BOOL PBER_Stream::RealDecode(PASN_Real & value)
{
  unsigned len;
  if (!HeaderDecode(value, len) || len == 0 || IsAtEnd())
    return FALSE;

  PAssertAlways(PUnimplementedFunction);
  byteOffset += len;

  return TRUE;
}


void PBER_Stream::RealEncode(const PASN_Real &)
{
  PAssertAlways(PUnimplementedFunction);
}


BOOL PPER_Stream::RealDecode(PASN_Real &)
{
  // X.691 Section 14

  if (IsAtEnd())
    return FALSE;

  unsigned len;
  if (!MultiBitDecode(8, len))
    return FALSE;

  PAssertAlways(PUnimplementedFunction);
  byteOffset += len+1;
  return TRUE;
}


void PPER_Stream::RealEncode(const PASN_Real &)
{
  // X.691 Section 14

  MultiBitEncode(0, 8);
  PAssertAlways(PUnimplementedFunction);
  MultiBitEncode(0, 8);
}


///////////////////////////////////////////////////////////////////////

PASN_ObjectId::PASN_ObjectId(const char * dotstr)
  : PASN_Object(UniversalObjectId, UniversalTagClass)
{
  if (dotstr != NULL)
    SetValue(dotstr);
}


PASN_ObjectId::PASN_ObjectId(unsigned tag, TagClass tagClass)
  : PASN_Object(tag, tagClass)
{
}


PASN_ObjectId::PASN_ObjectId(const PASN_ObjectId & other)
  : PASN_Object(other),
    value(other.value, other.GetSize())
{
}


PASN_ObjectId & PASN_ObjectId::operator=(const PASN_ObjectId & other)
{
  PASN_Object::operator=(other);
  value = PUnsignedArray(other.value, other.GetSize());
  return *this;
}


PASN_ObjectId & PASN_ObjectId::operator=(const char * dotstr)
{
  if (dotstr != NULL)
    SetValue(dotstr);
  else
    value.SetSize(0);
  return *this;
}


PASN_ObjectId & PASN_ObjectId::operator=(const PString & dotstr)
{
  SetValue(dotstr);
  return *this;
}


void PASN_ObjectId::SetValue(const PString & dotstr)
{
  PStringArray parts = dotstr.Tokenise('.');
  value.SetSize(parts.GetSize());
  for (PINDEX i = 0; i < parts.GetSize(); i++)
    value[i] = parts[i].AsUnsigned();
}


BOOL PASN_ObjectId::operator==(const char * dotstr) const
{
  PASN_ObjectId id;
  id.SetValue(dotstr);
  return *this == id;
}


PObject::Comparison PASN_ObjectId::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_ObjectId::Class()), PInvalidCast);
  const PASN_ObjectId & other = (const PASN_ObjectId &)obj;
  return value.Compare(other.value);
}


PObject * PASN_ObjectId::Clone() const
{
  PAssert(IsClass(PASN_ObjectId::Class()), PInvalidCast);
  return new PASN_ObjectId(*this);
}


void PASN_ObjectId::PrintOn(ostream & strm) const
{
  for (PINDEX i = 0; i < value.GetSize(); i++) {
    strm << (unsigned)value[i];
    if (i < value.GetSize()-1)
      strm << '.';
  }
}


PString PASN_ObjectId::GetTypeAsString() const
{
  return "Object ID";
}


BOOL PASN_ObjectId::CommonDecode(PASN_Stream & strm, unsigned dataLen)
{
  value.SetSize(0);

  // handle zero length strings correctly
  if (dataLen == 0)
    return TRUE;

  unsigned subId;

  // start at the second identifier in the buffer, because we will later
  // expand the first number into the first two IDs
  PINDEX i = 1;
  while (dataLen > 0) {
    unsigned byte;
    subId = 0;
    do {    /* shift and add in low order 7 bits */
      if (strm.IsAtEnd())
        return FALSE;
      byte = strm.ByteDecode();
      subId = (subId << 7) + (byte & 0x7f);
      dataLen--;
    } while ((byte & 0x80) != 0);
    value.SetAt(i++, subId);
  }

  /*
   * The first two subidentifiers are encoded into the first component
   * with the value (X * 40) + Y, where:
   *  X is the value of the first subidentifier.
   *  Y is the value of the second subidentifier.
   */
  subId = value[1];
  if (subId < 40) {
    value[0] = 0;
    value[1] = subId;
  }
  else if (subId < 80) {
    value[0] = 1;
    value[1] = subId-40;
  }
  else {
    value[0] = 2;
    value[1] = subId-80;
  }

  return TRUE;
}


void PASN_ObjectId::CommonEncode(PBYTEArray & encodecObjectId) const
{
  PINDEX length = value.GetSize();
  const unsigned * objId = value;

  if (length < 2) {
    // Thise case is really illegal, but we have to do SOMETHING
    encodecObjectId.SetSize(0);
    return;
  }

  unsigned subId = (objId[0] * 40) + objId[1];
  objId += 2;

  PINDEX outputPosition = 0;

  while (--length > 0) {
    if (subId < 128)
      encodecObjectId[outputPosition++] = (BYTE)subId;
    else {
      unsigned mask = 0x7F; /* handle subid == 0 case */
      int bits = 0;

      /* testmask *MUST* !!!! be of an unsigned type */
      unsigned testmask = 0x7F;
      int      testbits = 0;
      while (testmask != 0) {
        if (subId & testmask) {  /* if any bits set */
          mask = testmask;
	        bits = testbits;
	      }
        testmask <<= 7;
        testbits += 7;
      }

      /* mask can't be zero here */
      while (mask != 0x7F) {
        /* fix a mask that got truncated above */
      	if (mask == 0x1E00000)
	        mask = 0xFE00000;

        encodecObjectId[outputPosition++] = (BYTE)(((subId & mask) >> bits) | 0x80);

        mask >>= 7;
        bits -= 7;
      }

      encodecObjectId[outputPosition++] = (BYTE)(subId & mask);
    }

    if (length > 1)
      subId = *objId++;
  }
}


PINDEX PASN_ObjectId::GetDataLength() const
{
  PBYTEArray dummy;
  CommonEncode(dummy);
  return dummy.GetSize();
}


BOOL PASN_ObjectId::Decode(PASN_Stream & strm)
{
  return strm.ObjectIdDecode(*this);
}


void PASN_ObjectId::Encode(PASN_Stream & strm) const
{
  strm.ObjectIdEncode(*this);
}


BOOL PBER_Stream::ObjectIdDecode(PASN_ObjectId & value)
{
  unsigned len;
  if (!HeaderDecode(value, len))
    return FALSE;

  return value.CommonDecode(*this, len);
}


void PBER_Stream::ObjectIdEncode(const PASN_ObjectId & value)
{
  HeaderEncode(value);
  PBYTEArray data;
  value.CommonEncode(data);
  BlockEncode(data, data.GetSize());
}


BOOL PPER_Stream::ObjectIdDecode(PASN_ObjectId & value)
{
  // X.691 Section 23

  unsigned dataLen;
  if (LengthDecode(0, 255, dataLen) < 0)
    return FALSE;

  ByteAlign();
  return value.CommonDecode(*this, dataLen);
}


void PPER_Stream::ObjectIdEncode(const PASN_ObjectId & value)
{
  // X.691 Section 23

  PBYTEArray eObjId;
  value.CommonEncode(eObjId);
  LengthEncode(eObjId.GetSize(), 0, 255);
  BlockEncode(eObjId, eObjId.GetSize());
}


///////////////////////////////////////////////////////////////////////

PASN_BitString::PASN_BitString(unsigned nBits, const BYTE * buf)
  : PASN_ConstrainedObject(UniversalBitString, UniversalTagClass),
    totalBits(nBits),
    bitData((totalBits+7)/8)
{
  if (buf != NULL)
    memcpy(bitData.GetPointer(), buf, bitData.GetSize());
}


PASN_BitString::PASN_BitString(unsigned tag, TagClass tagClass, unsigned nBits)
  : PASN_ConstrainedObject(tag, tagClass),
    totalBits(nBits),
    bitData((totalBits+7)/8)
{
}


PASN_BitString::PASN_BitString(const PASN_BitString & other)
  : PASN_ConstrainedObject(other),
    bitData(other.bitData, other.bitData.GetSize())
{
  totalBits = other.totalBits;
}


PASN_BitString & PASN_BitString::operator=(const PASN_BitString & other)
{
  PASN_ConstrainedObject::operator=(other);
  totalBits = other.totalBits;
  bitData = PBYTEArray(other.bitData, other.bitData.GetSize());
  return *this;
}


void PASN_BitString::SetData(unsigned nBits, const PBYTEArray & bytes)
{
  bitData = bytes;
  SetSize(nBits);
}


void PASN_BitString::SetData(unsigned nBits, const BYTE * buf, PINDEX size)
{
  if (size == 0)
    size = (nBits+7)/8;
  memcpy(bitData.GetPointer(size), buf, size);
  SetSize(nBits);
}


BOOL PASN_BitString::SetSize(unsigned nBits)
{
  if (constraint == Unconstrained)
    totalBits = nBits;
  else if (totalBits < (unsigned)lowerLimit)
    totalBits = lowerLimit;
  else if ((unsigned)totalBits > upperLimit)
    totalBits = upperLimit;
  else
    totalBits = nBits;
  return bitData.SetSize((nBits+7)/8);
}


BOOL PASN_BitString::operator[](PINDEX bit) const
{
  if ((unsigned)bit < totalBits)
    return (bitData[bit>>3] & (1 << (7 - (bit&7)))) != 0;
  return FALSE;
}


void PASN_BitString::Set(unsigned bit)
{
  if (bit < totalBits)
    bitData[(PINDEX)(bit>>3)] |= 1 << (7 - (bit&7));
}


void PASN_BitString::Clear(unsigned bit)
{
  if (bit < totalBits)
    bitData[(PINDEX)(bit>>3)] &= ~(1 << (7 - (bit&7)));
}


void PASN_BitString::Invert(unsigned bit)
{
  if (bit < totalBits)
    bitData[(PINDEX)(bit>>3)] ^= 1 << (7 - (bit&7));
}


PObject::Comparison PASN_BitString::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_BitString::Class()), PInvalidCast);
  const PASN_BitString & other = (const PASN_BitString &)obj;
  if (totalBits < other.totalBits)
    return LessThan;
  if (totalBits > other.totalBits)
    return GreaterThan;
  return bitData.Compare(other.bitData);
}


PObject * PASN_BitString::Clone() const
{
  PAssert(IsClass(PASN_BitString::Class()), PInvalidCast);
  return new PASN_BitString(*this);
}


void PASN_BitString::PrintOn(ostream & strm) const
{
  BYTE mask = 0x80;
  PINDEX offset = 0;
  for (unsigned i = 0; i < totalBits; i++) {
    strm << ((bitData[offset]&mask) != 0 ? '1' : '0');
    mask >>= 1;
    if (mask == 0) {
      mask = 0x80;
      offset++;
    }
  }
}


void PASN_BitString::SetConstraintBounds(ConstraintType type, int lower, unsigned upper)
{
  PAssert(lower >= 0, PInvalidParameter);
  PASN_ConstrainedObject::SetConstraintBounds(type, lower, upper);
  SetSize(GetSize());
}


PString PASN_BitString::GetTypeAsString() const
{
  return "Bit String";
}


PINDEX PASN_BitString::GetDataLength() const
{
  return (totalBits+7)/8 + 1;
}


BOOL PASN_BitString::Decode(PASN_Stream & strm)
{
  return strm.BitStringDecode(*this);
}


void PASN_BitString::Encode(PASN_Stream & strm) const
{
  strm.BitStringEncode(*this);
}


BOOL PASN_BitString::DecodeBER(PBER_Stream & strm, unsigned len)
{
  totalBits = len*8 - strm.ByteDecode();
  unsigned nBytes = (totalBits+7)/8;
  return strm.BlockDecode(bitData.GetPointer(nBytes), nBytes) == nBytes;
}


void PASN_BitString::EncodeBER(PBER_Stream & strm) const
{
  if (totalBits == 0)
    strm.ByteEncode(0);
  else {
    strm.ByteEncode(8-totalBits%8);
    strm.BlockEncode(bitData, (totalBits+7)/8);
  }
}


BOOL PASN_BitString::DecodePER(PPER_Stream & strm)
{
  // X.691 Section 15

  if (ConstrainedLengthDecode(strm, totalBits) < 0)
    return FALSE;

  SetSize(totalBits);

  if (totalBits == 0)
    return TRUE;   // 15.7

  if (totalBits > strm.GetBitsLeft())
    return FALSE;

  if (totalBits > 16) {
    unsigned nBytes = (totalBits+7)/8;
    return strm.BlockDecode(bitData.GetPointer(), nBytes) == nBytes;   // 15.9
  }

  unsigned theBits;
  if (totalBits <= 8) {
    if (!strm.MultiBitDecode(totalBits, theBits))
      return FALSE;

    bitData[0] = (BYTE)(theBits << (8-totalBits));
  }
  else {  // 15.8
    if (!strm.MultiBitDecode(8, theBits))
      return FALSE;

    bitData[0] = (BYTE)theBits;

    if (!strm.MultiBitDecode(totalBits-8, theBits))
      return FALSE;

    bitData[1] = (BYTE)(theBits << (16-totalBits));
  }

  return TRUE;
}


void PASN_BitString::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 15

  ConstrainedLengthEncode(strm, totalBits);

  if (totalBits == 0)
    return;

  if (totalBits > 16)
    strm.BlockEncode(bitData, (totalBits+7)/8);   // 15.9
  else if (totalBits <= 8)  // 15.8
    strm.MultiBitEncode(bitData[0] >> (8 - totalBits), totalBits);
  else {
    strm.MultiBitEncode(bitData[0], 8);
    strm.MultiBitEncode(bitData[1] >> (16 - totalBits), totalBits-8);
  }
}


BOOL PASN_BitString::DecodeSequenceExtensionBitmap(PPER_Stream & strm)
{
  if (!strm.SmallUnsignedDecode(totalBits))
    return FALSE;

  totalBits++;

  SetSize(totalBits);

  if (totalBits > strm.GetBitsLeft())
    return FALSE;

  unsigned theBits;

  PINDEX idx = 0;
  unsigned bitsLeft = totalBits;
  while (bitsLeft >= 8) {
    if (!strm.MultiBitDecode(8, theBits))
      return FALSE;
    bitData[idx++] = (BYTE)theBits;
    bitsLeft -= 8;
  }

  if (bitsLeft > 0) {
    if (!strm.MultiBitDecode(bitsLeft, theBits))
      return FALSE;
    bitData[idx] = (BYTE)(theBits << (8-bitsLeft));
  }

  return TRUE;
}


void PASN_BitString::EncodeSequenceExtensionBitmap(PPER_Stream & strm) const
{
  PAssert(totalBits > 0, PLogicError);

  strm.SmallUnsignedEncode(totalBits-1);

  PINDEX idx = 0;
  unsigned bitsLeft = totalBits;
  while (bitsLeft >= 8) {
    strm.MultiBitEncode(bitData[idx++], 8);
    bitsLeft -= 8;
  }

  if (bitsLeft > 0)
    strm.MultiBitEncode(bitData[idx] >> (8 - bitsLeft), bitsLeft);
}


BOOL PBER_Stream::BitStringDecode(PASN_BitString & value)
{
  unsigned len;
  if (!HeaderDecode(value, len) || len == 0 || IsAtEnd())
    return FALSE;

  return value.DecodeBER(*this, len);
}


void PBER_Stream::BitStringEncode(const PASN_BitString & value)
{
  HeaderEncode(value);
  value.EncodeBER(*this);
}


BOOL PPER_Stream::BitStringDecode(PASN_BitString & value)
{
  return value.DecodePER(*this);
}


void PPER_Stream::BitStringEncode(const PASN_BitString & value)
{
  value.EncodePER(*this);
}


///////////////////////////////////////////////////////////////////////

PASN_OctetString::PASN_OctetString(const char * str, PINDEX size)
  : PASN_ConstrainedObject(UniversalOctetString, UniversalTagClass)
{
  if (str != NULL) {
    if (size == 0)
      size = ::strlen(str);
    SetValue((const BYTE *)str, size);
  }
}


PASN_OctetString::PASN_OctetString(unsigned tag, TagClass tagClass)
  : PASN_ConstrainedObject(tag, tagClass)
{
}


PASN_OctetString::PASN_OctetString(const PASN_OctetString & other)
  : PASN_ConstrainedObject(other),
    value(other.value, other.GetSize())
{
}


PASN_OctetString & PASN_OctetString::operator=(const PASN_OctetString & other)
{
  PASN_ConstrainedObject::operator=(other);
  value = PBYTEArray(other.value, other.GetSize());
  return *this;
}


PASN_OctetString & PASN_OctetString::operator=(const char * str)
{
  if (str == NULL)
    value.SetSize(lowerLimit);
  else
    SetValue((const BYTE *)str, strlen(str));
  return *this;
}


PASN_OctetString & PASN_OctetString::operator=(const PString & str)
{
  SetValue((const BYTE *)(const char *)str, str.GetSize()-1);
  return *this;
}


PASN_OctetString & PASN_OctetString::operator=(const PBYTEArray & arr)
{
  PINDEX len = arr.GetSize();
  if ((unsigned)len > upperLimit || (int)len < lowerLimit)
    SetValue(arr, len);
  else
    value = arr;
  return *this;
}


void PASN_OctetString::SetValue(const BYTE * data, PINDEX len)
{
  if ((unsigned)len > upperLimit)
    len = upperLimit;
  value.SetSize((int)len < lowerLimit ? lowerLimit : len);
  memcpy(value.GetPointer(), data, len);
}


PString PASN_OctetString::AsString() const
{
  if (value.IsEmpty())
    return PString();
  return PString((const char *)(const BYTE *)value, value.GetSize());
}


PObject::Comparison PASN_OctetString::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_OctetString::Class()), PInvalidCast);
  const PASN_OctetString & other = (const PASN_OctetString &)obj;
  return value.Compare(other.value);
}


PObject * PASN_OctetString::Clone() const
{
  PAssert(IsClass(PASN_OctetString::Class()), PInvalidCast);
  return new PASN_OctetString(*this);
}


void PASN_OctetString::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << ' ' << value.GetSize() << " octets {\n";
  PINDEX i = 0;
  while (i < value.GetSize()) {
    strm << setw(indent) << " " << hex << setfill('0');
    PINDEX j;
    for (j = 0; j < 16; j++) {
      if (i+j < value.GetSize())
        strm << setw(2) << (unsigned)value[i+j] << ' ';
      else
        strm << "   ";
    }
    strm << "  ";
    for (j = 0; j < 16; j++) {
      if (i+j < value.GetSize()) {
        if (isprint(value[i+j]))
          strm << value[i+j];
        else
          strm << ' ';
      }
    }
    strm << dec << setfill(' ') << '\n';
    i += 16;
  }
  strm << setw(indent-1) << "}";
}


void PASN_OctetString::SetConstraintBounds(ConstraintType type, int lower, unsigned upper)
{
  PAssert(lower >= 0, PInvalidParameter);
  PASN_ConstrainedObject::SetConstraintBounds(type, lower, upper);
  if (constraint != Unconstrained) {
    if (value.GetSize() < (PINDEX)lowerLimit)
      value.SetSize(lowerLimit);
    else if ((unsigned)value.GetSize() > upperLimit)
      value.SetSize(upperLimit);
  }
}


PString PASN_OctetString::GetTypeAsString() const
{
  return "Octet String";
}


PINDEX PASN_OctetString::GetDataLength() const
{
  return value.GetSize();
}


BOOL PASN_OctetString::Decode(PASN_Stream & strm)
{
  return strm.OctetStringDecode(*this);
}


void PASN_OctetString::Encode(PASN_Stream & strm) const
{
  strm.OctetStringEncode(*this);
}


BOOL PASN_OctetString::DecodePER(PPER_Stream & strm)
{
  // X.691 Section 16

  unsigned nBytes;
  if (ConstrainedLengthDecode(strm, nBytes) < 0)
    return FALSE;

  value.SetSize(nBytes);   // 16.5

  unsigned theBits;
  switch (nBytes) {
    case 0 :
      break;

    case 1 :  // 16.6
      if (!strm.MultiBitDecode(8, theBits))
        return FALSE;
      value[0] = (BYTE)theBits;
      break;

    case 2 :  // 16.6
      if (!strm.MultiBitDecode(8, theBits))
        return FALSE;
      value[0] = (BYTE)theBits;
      if (!strm.MultiBitDecode(8, theBits))
        return FALSE;
      value[1] = (BYTE)theBits;
      break;

    default: // 16.7
      return strm.BlockDecode(value.GetPointer(), nBytes) == nBytes;
  }

  return TRUE;
}


void PASN_OctetString::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 16

  PINDEX nBytes = value.GetSize();
  ConstrainedLengthEncode(strm, nBytes);

  switch (nBytes) {
    case 0 :  // 16.5
      break;

    case 1 :  // 16.6
      strm.MultiBitEncode(value[0], 8);
      break;

    case 2 :  // 16.6
      strm.MultiBitEncode(value[0], 8);
      strm.MultiBitEncode(value[1], 8);
      break;

    default: // 16.7
      strm.BlockEncode(value, nBytes);
  }
}


BOOL PBER_Stream::OctetStringDecode(PASN_OctetString & value)
{
  unsigned len;
  if (!HeaderDecode(value, len))
    return FALSE;

  return BlockDecode(value.GetPointer(len), len) == len;
}


void PBER_Stream::OctetStringEncode(const PASN_OctetString & value)
{
  HeaderEncode(value);
  BlockEncode(value, value.GetSize());
}


BOOL PPER_Stream::OctetStringDecode(PASN_OctetString & value)
{
  return value.DecodePER(*this);
}


void PPER_Stream::OctetStringEncode(const PASN_OctetString & value)
{
  value.EncodePER(*this);
}


BOOL PASN_OctetString::DecodeSubType(PASN_Object & obj)
{
  PPER_Stream stream = GetValue();
  return obj.Decode(stream);
}


void PASN_OctetString::EncodeSubType(const PASN_Object & obj)
{
  PPER_Stream stream;
  obj.Encode(stream);
  stream.CompleteEncoding();
  SetValue(stream);
}


///////////////////////////////////////////////////////////////////////

PASN_ConstrainedString::PASN_ConstrainedString(const char * canonical, PINDEX size,
                                               unsigned tag, TagClass tagClass)
  : PASN_ConstrainedObject(tag, tagClass)
{
  canonicalSet = canonical;
  canonicalSetSize = size;
  canonicalSetBits = CountBits(size);
  SetCharacterSet(canonicalSet, canonicalSetSize, Unconstrained);
}


PASN_ConstrainedString & PASN_ConstrainedString::operator=(const char * str)
{
  if (str == NULL)
    str = "";

  PStringStream newValue;

  PINDEX len = strlen(str);

  // Can't copy any more characters than the upper constraint
  if ((unsigned)len > upperLimit)
    len = upperLimit;

  // Now copy individual characters, if they are in character set constraint
  for (PINDEX i = 0; i < len; i++) {
    PINDEX sz = characterSet.GetSize();
    if (sz == 0 || memchr(characterSet, str[i], sz) != NULL)
      newValue << str[i];
  }

  // Make sure string meets minimum length constraint
  while ((int)len < lowerLimit) {
    newValue << characterSet[0];
    len++;
  }

  value = newValue;
  value.MakeMinimumSize();
  return *this;
}


void PASN_ConstrainedString::SetCharacterSet(ConstraintType ctype, const char * set)
{
  SetCharacterSet(set, strlen(set), ctype);
}


void PASN_ConstrainedString::SetCharacterSet(ConstraintType ctype, unsigned firstChar, unsigned lastChar)
{
  char buffer[256];
  for (unsigned i = firstChar; i < lastChar; i++)
    buffer[i] = (char)i;
  SetCharacterSet(buffer, lastChar - firstChar + 1, ctype);
}


void PASN_ConstrainedString::SetCharacterSet(const char * set, PINDEX setSize, ConstraintType ctype)
{
  if (ctype == Unconstrained) {
    characterSet.SetSize(canonicalSetSize);
    memcpy(characterSet.GetPointer(), canonicalSet, canonicalSetSize);
  }
  else {
    characterSet.SetSize(setSize);
    PINDEX count = 0;
    for (PINDEX i = 0; i < canonicalSetSize; i++) {
      if (memchr(set, canonicalSet[i], setSize) != NULL)
        characterSet[count++] = canonicalSet[i];
    }
    PAssert(count > 0, PInvalidParameter);
    characterSet.SetSize(count);
  }

  charSetUnalignedBits = CountBits(characterSet.GetSize());

  charSetAlignedBits = 1;
  while (charSetUnalignedBits > charSetAlignedBits)
    charSetAlignedBits <<= 1;

  operator=((const char *)value);
}


PObject::Comparison PASN_ConstrainedString::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_ConstrainedString::Class()), PInvalidCast);
  const PASN_ConstrainedString & other = (const PASN_ConstrainedString &)obj;
  return value.Compare(other.value);
}


void PASN_ConstrainedString::PrintOn(ostream & strm) const
{
  strm << value.ToLiteral();
}


void PASN_ConstrainedString::SetConstraintBounds(ConstraintType type,
                                                 int lower, unsigned upper)
{
  PAssert(lower >= 0, PInvalidParameter);
  PASN_ConstrainedObject::SetConstraintBounds(type, lower, upper);
  if (constraint != Unconstrained) {
    if (value.GetSize() < (PINDEX)lowerLimit)
      value.SetSize(lowerLimit);
    else if ((unsigned)value.GetSize() > upperLimit)
      value.SetSize(upperLimit);
  }
}


PINDEX PASN_ConstrainedString::GetDataLength() const
{
  return value.GetSize()-1;
}


BOOL PASN_ConstrainedString::Decode(PASN_Stream & strm)
{
  return strm.ConstrainedStringDecode(*this);
}


void PASN_ConstrainedString::Encode(PASN_Stream & strm) const
{
  strm.ConstrainedStringEncode(*this);
}


BOOL PASN_ConstrainedString::DecodeBER(PBER_Stream & strm, unsigned len)
{
  return strm.BlockDecode((BYTE *)value.GetPointer(len+1), len) == len;
}


void PASN_ConstrainedString::EncodeBER(PBER_Stream & strm) const
{
  strm.BlockEncode(value, value.GetSize()-1);
}


BOOL PASN_ConstrainedString::DecodePER(PPER_Stream & strm)
{
  // X.691 Section 26

  unsigned len;
  if (ConstrainedLengthDecode(strm, len) < 0)
    return FALSE;

  unsigned nBits = strm.IsAligned() ? charSetAlignedBits : charSetUnalignedBits;
  unsigned totalBits = upperLimit*nBits;

  if (constraint == Unconstrained ||
            (lowerLimit == (int)upperLimit ? (totalBits > 16) : (totalBits >= 16))) {
    if (nBits == 8)
      return strm.BlockDecode((BYTE *)value.GetPointer(len+1), len) == len;
    if (strm.IsAligned())
      strm.ByteAlign();
  }

  value.SetSize(len+1);

  PINDEX i;
  for (i = 0; i < (PINDEX)len; i++) {
    unsigned theBits;
    if (!strm.MultiBitDecode(nBits, theBits))
      return FALSE;
    if (nBits >= canonicalSetBits && canonicalSetBits > 4)
      value[i] = (char)theBits;
    else
      value[i] = characterSet[(PINDEX)theBits];
  }
  value[i] = '\0';

  return TRUE;
}


void PASN_ConstrainedString::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 26

  PINDEX len = value.GetSize()-1;
  ConstrainedLengthEncode(strm, len);

  unsigned nBits = strm.IsAligned() ? charSetAlignedBits : charSetUnalignedBits;
  unsigned totalBits = upperLimit*nBits;

  if (constraint == Unconstrained ||
            (lowerLimit == (int)upperLimit ? (totalBits > 16) : (totalBits >= 16))) {
    if (nBits == 8) {
      strm.BlockEncode((const BYTE *)(const char *)value, len);
      return;
    }
    if (strm.IsAligned())
      strm.ByteAlign();
  }

  for (PINDEX i = 0; i < len; i++) {
    if (nBits >= canonicalSetBits && canonicalSetBits > 4)
      strm.MultiBitEncode(value[i], nBits);
    else {
      const void * ptr = memchr(characterSet, value[i], characterSet.GetSize());
      PINDEX pos = 0;
      if (ptr != NULL)
        pos = ((const char *)ptr - (const char *)characterSet);
      strm.MultiBitEncode(pos, nBits);
    }
  }
}


BOOL PBER_Stream::ConstrainedStringDecode(PASN_ConstrainedString & value)
{
  unsigned len;
  if (!HeaderDecode(value, len))
    return FALSE;

  return value.DecodeBER(*this, len);
}


void PBER_Stream::ConstrainedStringEncode(const PASN_ConstrainedString & value)
{
  HeaderEncode(value);
  value.Encode(*this);
}


BOOL PPER_Stream::ConstrainedStringDecode(PASN_ConstrainedString & value)
{
  return value.DecodePER(*this);
}


void PPER_Stream::ConstrainedStringEncode(const PASN_ConstrainedString & value)
{
  value.EncodePER(*this);
}


#define DEFINE_STRING_CLASS(name, set) \
  static const char name##StringSet[] = set; \
  PASN_##name##String::PASN_##name##String(const char * str) \
    : PASN_ConstrainedString(name##StringSet, sizeof(name##StringSet)-1, \
                             Universal##name##String, UniversalTagClass) \
    { PASN_ConstrainedString::SetValue(str); } \
  PASN_##name##String::PASN_##name##String(unsigned tag, TagClass tagClass) \
    : PASN_ConstrainedString(name##StringSet, sizeof(name##StringSet)-1, tag, tagClass) \
    { } \
  PASN_##name##String & PASN_##name##String::operator=(const char * str) \
    { PASN_ConstrainedString::SetValue(str); return *this; } \
  PASN_##name##String & PASN_##name##String::operator=(const PString & str) \
    { PASN_ConstrainedString::SetValue(str); return *this; } \
  PObject * PASN_##name##String::Clone() const \
    { PAssert(IsClass(PASN_##name##String::Class()), PInvalidCast); \
      return new PASN_##name##String(*this); } \
  PString PASN_##name##String::GetTypeAsString() const \
    { return #name " String"; }

DEFINE_STRING_CLASS(Numeric,   " 0123456789")
DEFINE_STRING_CLASS(Printable, " '()+,-./0123456789:=?"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz")
DEFINE_STRING_CLASS(Visible,   " !\"#$%&'()*+,-./0123456789:;<=>?"
                               "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                               "`abcdefghijklmnopqrstuvwxyz{|}~")
DEFINE_STRING_CLASS(IA5,       "\000\001\002\003\004\005\006\007"
                               "\010\011\012\013\014\015\016\017"
                               "\020\021\022\023\024\025\026\027"
                               "\030\031\032\033\034\035\036\037"
                               " !\"#$%&'()*+,-./0123456789:;<=>?"
                               "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                               "`abcdefghijklmnopqrstuvwxyz{|}~\177")
DEFINE_STRING_CLASS(General,   "\000\001\002\003\004\005\006\007"
                               "\010\011\012\013\014\015\016\017"
                               "\020\021\022\023\024\025\026\027"
                               "\030\031\032\033\034\035\036\037"
                               " !\"#$%&'()*+,-./0123456789:;<=>?"
                               "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                               "`abcdefghijklmnopqrstuvwxyz{|}~\177"
                               "\200\201\202\203\204\205\206\207"
                               "\210\211\212\213\214\215\216\217"
                               "\220\221\222\223\224\225\226\227"
                               "\230\231\232\233\234\235\236\237"
                               "\240\241\242\243\244\245\246\247"
                               "\250\251\252\253\254\255\256\257"
                               "\260\261\262\263\264\265\266\267"
                               "\270\271\272\273\274\275\276\277"
                               "\300\301\302\303\304\305\306\307"
                               "\310\311\312\313\314\315\316\317"
                               "\320\321\322\323\324\325\326\327"
                               "\330\331\332\333\334\335\336\337"
                               "\340\341\342\343\344\345\346\347"
                               "\350\351\352\353\354\355\356\357"
                               "\360\361\362\363\364\365\366\367"
                               "\370\371\372\373\374\375\376\377")


///////////////////////////////////////////////////////////////////////

PASN_BMPString::PASN_BMPString(const char * str)
  : PASN_ConstrainedObject(UniversalBMPString, UniversalTagClass)
{
  Construct();
  if (str != NULL)
    SetValue(str);
}


PASN_BMPString::PASN_BMPString(const PWORDArray & wstr)
  : PASN_ConstrainedObject(UniversalBMPString, UniversalTagClass)
{
  Construct();
  SetValue(wstr);
}


PASN_BMPString::PASN_BMPString(unsigned tag, TagClass tagClass)
  : PASN_ConstrainedObject(tag, tagClass)
{
  Construct();
}


void PASN_BMPString::Construct()
{
  firstChar = 0;
  lastChar = 0xffff;
  charSetAlignedBits = 16;
  charSetUnalignedBits = 16;
}


PASN_BMPString::PASN_BMPString(const PASN_BMPString & other)
  : PASN_ConstrainedObject(other),
    value(other.value, other.value.GetSize()),
    characterSet(other.characterSet)
{
  firstChar = other.firstChar;
  lastChar = other.lastChar;
  charSetAlignedBits = other.charSetAlignedBits;
  charSetUnalignedBits = other.charSetUnalignedBits;
}


PASN_BMPString & PASN_BMPString::operator=(const PASN_BMPString & other)
{
  PASN_ConstrainedObject::operator=(other);

  value = PWORDArray(other.value, other.value.GetSize());
  characterSet = other.characterSet;
  firstChar = other.firstChar;
  lastChar = other.lastChar;
  charSetAlignedBits = other.charSetAlignedBits;
  charSetUnalignedBits = other.charSetUnalignedBits;

  return *this;
}


BOOL PASN_BMPString::IsLegalCharacter(WORD ch)
{
  if (ch < firstChar)
    return FALSE;

  if (ch > lastChar)
    return FALSE;

  if (characterSet.IsEmpty())
    return TRUE;

  const WORD * wptr = characterSet;
  PINDEX count = characterSet.GetSize();
  while (count-- > 0) {
    if (*wptr == ch)
      return TRUE;
    wptr++;
  }

  return FALSE;
}


PASN_BMPString & PASN_BMPString::operator=(const char * str)
{
  // Must be at least this big for string conversion
  value.SetSize(::strlen(str));

  // Convert string looking for "&#1234;" style characters for 16 bit stuff
  PINDEX count = 0;
  while (*str != '\0') {
    WORD c = (BYTE)*str++;

    if (c == '&' && *str == '#') {
      const char * semicolon = strchr(str, ';');
      if (semicolon != NULL) {
        unsigned bigChar = 0;
        const char * p = str+1;
        while (isdigit(*p))
          bigChar = bigChar*10 + *p++ - '0';
        if (p == semicolon && bigChar < 65536) {
          c = (WORD)bigChar;
          str = semicolon + 1;
        }
      }
    }

    if (IsLegalCharacter(c))
      value[count++] = c;
  }

  // Can't have any more than the upper constraint
  if ((unsigned)count > upperLimit)
    count = upperLimit;

  // Number of bytes must be at least lhe lower constraint
  PINDEX newSize = (int)count < lowerLimit ? lowerLimit : count;
  value.SetSize(newSize);

  // Pad out with the first character till required size
  while (count < newSize)
    value[count++] = firstChar;

  return *this;
}


PASN_BMPString & PASN_BMPString::operator=(const PWORDArray & array)
{
  PINDEX paramSize = array.GetSize();

  // Can't copy any more than the upper constraint
  if ((unsigned)paramSize > upperLimit)
    paramSize = upperLimit;

  // Number of bytes must be at least lhe lower constraint
  PINDEX newSize = (int)paramSize < lowerLimit ? lowerLimit : paramSize;
  value.SetSize(newSize);

  PINDEX count = 0;
  for (PINDEX i = 0; i < paramSize; i++) {
    WORD c = array[i];
    if (IsLegalCharacter(c))
      value[count++] = c;
  }

  // Pad out with the first character till required size
  while (count < newSize)
    value[count++] = firstChar;

  return *this;
}


PString PASN_BMPString::GetValue() const
{
  PString str;
  for (PINDEX i = 0; i < value.GetSize(); i++) {
    if (value[i] < 256)
      str += (char)value[i];
    else
      str.sprintf("&#%u;", value[i]);
  }
  return str;
}


void PASN_BMPString::SetCharacterSet(ConstraintType ctype, const char * charSet)
{
  PWORDArray array(strlen(charSet));

  PINDEX count = 0;
  while (*charSet != '\0')
    array[count++] = (BYTE)*charSet++;

  SetCharacterSet(ctype, array);
}


void PASN_BMPString::SetCharacterSet(ConstraintType ctype, const PWORDArray & charSet)
{
  if (ctype == Unconstrained) {
    firstChar = 0;
    lastChar = 0xffff;
    characterSet.SetSize(0);
  }
  else {
    characterSet = charSet;

    charSetUnalignedBits = CountBits(lastChar - firstChar + 1);
    if (!charSet.IsEmpty()) {
      unsigned count = 0;
      for (PINDEX i = 0; i < charSet.GetSize(); i++) {
        if (characterSet[i] >= firstChar && characterSet[i] <= lastChar)
          count++;
      }
      count = CountBits(count);
      if (charSetUnalignedBits > count)
        charSetUnalignedBits = count;
    }

    charSetAlignedBits = 1;
    while (charSetUnalignedBits > charSetAlignedBits)
      charSetAlignedBits <<= 1;

    SetValue(value);
  }
}


void PASN_BMPString::SetCharacterSet(ConstraintType ctype, unsigned first, unsigned last)
{
  if (ctype != Unconstrained) {
    PAssert(first < 0x10000 && last < 0x10000 && last > first, PInvalidParameter);
    firstChar = (WORD)first;
    lastChar = (WORD)last;
  }
  SetCharacterSet(ctype, characterSet);
}


PObject * PASN_BMPString::Clone() const
{
  PAssert(IsClass(PASN_BMPString::Class()), PInvalidCast);
  return new PASN_BMPString(*this);
}


PObject::Comparison PASN_BMPString::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_BMPString::Class()), PInvalidCast);
  const PASN_BMPString & other = (const PASN_BMPString &)obj;
  return value.Compare(other.value);
}


void PASN_BMPString::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  PINDEX sz = value.GetSize();
  strm << ' ' << sz << " characters {\n";
  PINDEX i = 0;
  while (i < sz) {
    strm << setw(indent) << " " << hex << setfill('0');
    PINDEX j;
    for (j = 0; j < 8; j++)
      if (i+j < sz)
        strm << setw(4) << value[i+j] << ' ';
      else
        strm << "     ";
    strm << "  ";
    for (j = 0; j < 8; j++) {
      if (i+j < sz) {
        WORD c = value[i+j];
        if (c < 128 && isprint(c))
          strm << (char)c;
        else
          strm << ' ';
      }
    }
    strm << dec << setfill(' ') << '\n';
    i += 8;
  }
  strm << setw(indent-1) << "}";
}


PString PASN_BMPString::GetTypeAsString() const
{
  return "BMP String";
}


PINDEX PASN_BMPString::GetDataLength() const
{
  return value.GetSize()*2;
}


BOOL PASN_BMPString::Decode(PASN_Stream & strm)
{
  return strm.BMPStringDecode(*this);
}


void PASN_BMPString::Encode(PASN_Stream & strm) const
{
  strm.BMPStringEncode(*this);
}


BOOL PASN_BMPString::DecodeBER(PBER_Stream & strm, unsigned len)
{
  value.SetSize(len/2);
  return strm.BlockDecode((BYTE *)value.GetPointer(len), len) == len;
}


void PASN_BMPString::EncodeBER(PBER_Stream & strm) const
{
  strm.BlockEncode((const BYTE *)(const WORD *)value, value.GetSize()*2);
}


BOOL PASN_BMPString::DecodePER(PPER_Stream & strm)
{
  // X.691 Section 26

  unsigned len;
  if (ConstrainedLengthDecode(strm, len) < 0)
    return FALSE;

  value.SetSize(len);

  PINDEX nBits = strm.IsAligned() ? charSetAlignedBits : charSetUnalignedBits;

  if ((constraint == Unconstrained || upperLimit*nBits > 16) && strm.IsAligned())
    strm.ByteAlign();

  for (PINDEX i = 0; i < (PINDEX)len; i++) {
    unsigned theBits;
    if (!strm.MultiBitDecode(nBits, theBits))
      return FALSE;
    if (characterSet.IsEmpty())
      value[i] = (WORD)(theBits + firstChar);
    else
      value[i] = characterSet[(PINDEX)theBits];
  }

  return TRUE;
}


void PASN_BMPString::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 26

  PINDEX len = value.GetSize();
  ConstrainedLengthEncode(strm, len);

  PINDEX nBits = strm.IsAligned() ? charSetAlignedBits : charSetUnalignedBits;

  if ((constraint == Unconstrained || upperLimit*nBits > 16) && strm.IsAligned())
    strm.ByteAlign();

  for (PINDEX i = 0; i < len; i++) {
    if (characterSet.IsEmpty())
      strm.MultiBitEncode(value[i] - firstChar, nBits);
    else {
      for (PINDEX pos = 0; pos < characterSet.GetSize(); pos++) {
        if (characterSet[pos] == value[i]) {
          strm.MultiBitEncode(pos, nBits);
          break;
        }
      }
    }
  }
}


BOOL PBER_Stream::BMPStringDecode(PASN_BMPString & value)
{
  unsigned len;
  if (!HeaderDecode(value, len))
    return FALSE;

  return value.DecodeBER(*this, len);
}


void PBER_Stream::BMPStringEncode(const PASN_BMPString & value)
{
  HeaderEncode(value);
  value.EncodeBER(*this);
}


BOOL PPER_Stream::BMPStringDecode(PASN_BMPString & value)
{
  return value.DecodePER(*this);
}


void PPER_Stream::BMPStringEncode(const PASN_BMPString & value)
{
  value.EncodePER(*this);
}


///////////////////////////////////////////////////////////////////////

PASN_GeneralisedTime & PASN_GeneralisedTime::operator=(const PTime & time)
{
  value = time.AsString("yyyyMMddhhmmss.uz");
  value.Replace("GMT", "Z");
  return *this;
}


PTime PASN_GeneralisedTime::GetValue() const
{
  int year = value(0,3).AsInteger();
  int month = value(4,5).AsInteger();
  int day = value(6,7).AsInteger();
  int hour = value(8,9).AsInteger();
  int minute = value(10,11).AsInteger();
  int seconds = 0;
  int zonePos = 12;

  if (isdigit(value[12])) {
    seconds = value(12,13).AsInteger();
    if (value[14] != '.')
      zonePos = 14;
    else {
      zonePos = 15;
      while (isdigit(value[zonePos]))
        zonePos++;
    }
  }

  int zone = PTime::Local;
  switch (value[zonePos]) {
    case 'Z' :
      zone = PTime::UTC;
      break;
    case '+' :
    case '-' :
      zone = value(zonePos+1,zonePos+2).AsInteger()*60 +
             value(zonePos+3,zonePos+4).AsInteger();
  }

  return PTime(seconds, minute, hour, day, month, year, zone);
}


///////////////////////////////////////////////////////////////////////

PASN_UniversalTime & PASN_UniversalTime::operator=(const PTime & time)
{
  value = time.AsString("yyMMddhhmmssz");
  value.Replace("GMT", "Z");
  return *this;
}


PTime PASN_UniversalTime::GetValue() const
{
  int year = value(0,1).AsInteger();
  if (year < 36)
    year += 2000;
  else
    year += 1900;

  int month = value(2,3).AsInteger();
  int day = value(4,5).AsInteger();
  int hour = value(6,7).AsInteger();
  int minute = value(8,9).AsInteger();
  int seconds = 0;
  int zonePos = 10;

  if (isdigit(value[10])) {
    seconds = value(10,11).AsInteger();
    zonePos = 12;
  }

  int zone = PTime::UTC;
  if (value[zonePos] != 'Z')
    zone = value(zonePos+1,zonePos+2).AsInteger()*60 +
           value(zonePos+3,zonePos+4).AsInteger();

  return PTime(seconds, minute, hour, day, month, year, zone);
}


///////////////////////////////////////////////////////////////////////

PASN_Choice::PASN_Choice(unsigned nChoices, BOOL extend)
  : PASN_Object(0, ApplicationTagClass, extend)
{
  numChoices = nChoices;
  choice = NULL;
}


PASN_Choice::PASN_Choice(unsigned tag, TagClass tagClass,
                         unsigned upper, BOOL extend)
  : PASN_Object(tag, tagClass, extend)
{
  numChoices = upper;
  choice = NULL;
}


PASN_Choice::PASN_Choice(unsigned tag, TagClass tagClass,
                         unsigned upper, BOOL extend, const PString & nameSpec)
  : PASN_Object(tag, tagClass, extend),
    names(BuildNamesDict(nameSpec))
{
  numChoices = upper;
  choice = NULL;
}


PASN_Choice::PASN_Choice(const PASN_Choice & other)
  : PASN_Object(other),
    names(other.names)
{
  numChoices = other.numChoices;

  if (other.choice != NULL)
    choice = (PASN_Object *)other.choice->Clone();
  else
    choice = NULL;
}


PASN_Choice & PASN_Choice::operator=(const PASN_Choice & other)
{
  delete choice;

  PASN_Object::operator=(other);

  numChoices = other.numChoices;
  names = other.names;

  if (other.choice != NULL)
    choice = (PASN_Object *)other.choice->Clone();
  else
    choice = NULL;

  return *this;
}


PASN_Choice::~PASN_Choice()
{
  delete choice;
}


void PASN_Choice::SetTag(unsigned newTag, TagClass tagClass)
{
  PASN_Object::SetTag(newTag, tagClass);

  delete choice;

  if (CreateObject())
    choice->SetTag(newTag, tagClass);
}


PString PASN_Choice::GetTagName() const
{
  if (tag == UINT_MAX)
    return "<uninitialised>";

  if (names.Contains(tag))
    return names[tag];

  if (choice != NULL && choice->IsDescendant(PASN_Choice::Class()) &&
      choice->GetTag() == tag && choice->GetTagClass() == tagClass)
    return PString(choice->GetClass()) + "->" + ((PASN_Choice *)choice)->GetTagName();

  return psprintf("<%u>", tag);
}


PASN_Object & PASN_Choice::GetObject() const
{
  PAssert(choice != NULL, "NULL Choice");
  return *choice;
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9

PASN_Choice::operator PASN_Null &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Null::Class()), PInvalidCast);
  return *(PASN_Null *)choice;
}


PASN_Choice::operator PASN_Boolean &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Boolean::Class()), PInvalidCast);
  return *(PASN_Boolean *)choice;
}


PASN_Choice::operator PASN_Integer &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Integer::Class()), PInvalidCast);
  return *(PASN_Integer *)choice;
}


PASN_Choice::operator PASN_Enumeration &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Enumeration::Class()), PInvalidCast);
  return *(PASN_Enumeration *)choice;
}


PASN_Choice::operator PASN_Real &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Real::Class()), PInvalidCast);
  return *(PASN_Real *)choice;
}


PASN_Choice::operator PASN_ObjectId &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_ObjectId::Class()), PInvalidCast);
  return *(PASN_ObjectId *)choice;
}


PASN_Choice::operator PASN_BitString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_BitString::Class()), PInvalidCast);
  return *(PASN_BitString *)choice;
}


PASN_Choice::operator PASN_OctetString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_OctetString::Class()), PInvalidCast);
  return *(PASN_OctetString *)choice;
}


PASN_Choice::operator PASN_NumericString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_NumericString::Class()), PInvalidCast);
  return *(PASN_NumericString *)choice;
}


PASN_Choice::operator PASN_PrintableString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_PrintableString::Class()), PInvalidCast);
  return *(PASN_PrintableString *)choice;
}


PASN_Choice::operator PASN_VisibleString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_VisibleString::Class()), PInvalidCast);
  return *(PASN_VisibleString *)choice;
}


PASN_Choice::operator PASN_IA5String &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_IA5String::Class()), PInvalidCast);
  return *(PASN_IA5String *)choice;
}


PASN_Choice::operator PASN_GeneralString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_GeneralString::Class()), PInvalidCast);
  return *(PASN_GeneralString *)choice;
}


PASN_Choice::operator PASN_BMPString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_BMPString::Class()), PInvalidCast);
  return *(PASN_BMPString *)choice;
}


PASN_Choice::operator PASN_Sequence &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Sequence::Class()), PInvalidCast);
  return *(PASN_Sequence *)choice;
}


#else


PASN_Choice::operator PASN_Null &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Null::Class()), PInvalidCast);
  return *(PASN_Null *)choice;
}


PASN_Choice::operator PASN_Boolean &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Boolean::Class()), PInvalidCast);
  return *(PASN_Boolean *)choice;
}


PASN_Choice::operator PASN_Integer &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Integer::Class()), PInvalidCast);
  return *(PASN_Integer *)choice;
}


PASN_Choice::operator PASN_Enumeration &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Enumeration::Class()), PInvalidCast);
  return *(PASN_Enumeration *)choice;
}


PASN_Choice::operator PASN_Real &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Real::Class()), PInvalidCast);
  return *(PASN_Real *)choice;
}


PASN_Choice::operator PASN_ObjectId &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_ObjectId::Class()), PInvalidCast);
  return *(PASN_ObjectId *)choice;
}


PASN_Choice::operator PASN_BitString &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_BitString::Class()), PInvalidCast);
  return *(PASN_BitString *)choice;
}


PASN_Choice::operator PASN_OctetString &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_OctetString::Class()), PInvalidCast);
  return *(PASN_OctetString *)choice;
}


PASN_Choice::operator PASN_NumericString &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_NumericString::Class()), PInvalidCast);
  return *(PASN_NumericString *)choice;
}


PASN_Choice::operator PASN_PrintableString &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_PrintableString::Class()), PInvalidCast);
  return *(PASN_PrintableString *)choice;
}


PASN_Choice::operator PASN_VisibleString &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_VisibleString::Class()), PInvalidCast);
  return *(PASN_VisibleString *)choice;
}


PASN_Choice::operator PASN_IA5String &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_IA5String::Class()), PInvalidCast);
  return *(PASN_IA5String *)choice;
}


PASN_Choice::operator PASN_GeneralString &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_GeneralString::Class()), PInvalidCast);
  return *(PASN_GeneralString *)choice;
}


PASN_Choice::operator PASN_BMPString &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_BMPString::Class()), PInvalidCast);
  return *(PASN_BMPString *)choice;
}


PASN_Choice::operator PASN_Sequence &()
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Sequence::Class()), PInvalidCast);
  return *(PASN_Sequence *)choice;
}


PASN_Choice::operator const PASN_Null &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Null::Class()), PInvalidCast);
  return *(const PASN_Null *)choice;
}


PASN_Choice::operator const PASN_Boolean &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Boolean::Class()), PInvalidCast);
  return *(const PASN_Boolean *)choice;
}


PASN_Choice::operator const PASN_Integer &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Integer::Class()), PInvalidCast);
  return *(const PASN_Integer *)choice;
}


PASN_Choice::operator const PASN_Enumeration &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Enumeration::Class()), PInvalidCast);
  return *(const PASN_Enumeration *)choice;
}


PASN_Choice::operator const PASN_Real &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Real::Class()), PInvalidCast);
  return *(const PASN_Real *)choice;
}


PASN_Choice::operator const PASN_ObjectId &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_ObjectId::Class()), PInvalidCast);
  return *(const PASN_ObjectId *)choice;
}


PASN_Choice::operator const PASN_BitString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_BitString::Class()), PInvalidCast);
  return *(const PASN_BitString *)choice;
}


PASN_Choice::operator const PASN_OctetString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_OctetString::Class()), PInvalidCast);
  return *(const PASN_OctetString *)choice;
}


PASN_Choice::operator const PASN_NumericString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_NumericString::Class()), PInvalidCast);
  return *(const PASN_NumericString *)choice;
}


PASN_Choice::operator const PASN_PrintableString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_PrintableString::Class()), PInvalidCast);
  return *(const PASN_PrintableString *)choice;
}


PASN_Choice::operator const PASN_VisibleString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_VisibleString::Class()), PInvalidCast);
  return *(const PASN_VisibleString *)choice;
}


PASN_Choice::operator const PASN_IA5String &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_IA5String::Class()), PInvalidCast);
  return *(const PASN_IA5String *)choice;
}


PASN_Choice::operator const PASN_GeneralString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_GeneralString::Class()), PInvalidCast);
  return *(const PASN_GeneralString *)choice;
}


PASN_Choice::operator const PASN_BMPString &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_BMPString::Class()), PInvalidCast);
  return *(const PASN_BMPString *)choice;
}


PASN_Choice::operator const PASN_Sequence &() const
{
  PAssert(PAssertNULL(choice)->IsDescendant(PASN_Sequence::Class()), PInvalidCast);
  return *(const PASN_Sequence *)choice;
}


#endif


PObject::Comparison PASN_Choice::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Choice::Class()), PInvalidCast);
  const PASN_Choice & other = (const PASN_Choice &)obj;

  if (choice == other.choice)
    return EqualTo;

  if (choice == NULL)
    return LessThan;

  if (other.choice == NULL)
    return GreaterThan;

  if (tag < other.tag)
    return LessThan;

  if (tag > other.tag)
    return GreaterThan;

  return choice->Compare(*other.choice);
}


void PASN_Choice::PrintOn(ostream & strm) const
{
  strm << GetTagName();

  if (choice != NULL)
    strm << ' ' << *choice;
  else
    strm << " (NULL)";
}


PString PASN_Choice::GetTypeAsString() const
{
  return "Choice";
}


PINDEX PASN_Choice::GetDataLength() const
{
  if (choice != NULL)
    return choice->GetDataLength();
  return 0;
}


BOOL PASN_Choice::IsPrimitive() const
{
  if (choice != NULL)
    return choice->IsPrimitive();
  return FALSE;
}


BOOL PASN_Choice::Decode(PASN_Stream & strm)
{
  return strm.ChoiceDecode(*this);
}


void PASN_Choice::Encode(PASN_Stream & strm) const
{
  strm.ChoiceEncode(*this);
}


BOOL PASN_Choice::DecodePER(PPER_Stream & strm)
{
  // X.691 Section 22
  delete choice;

  if (strm.IsAtEnd()) {
    choice = NULL;
    return FALSE;
  }

  BOOL ok = TRUE;

  if (extendable) {
    if (strm.SingleBitDecode()) {
      if (!strm.SmallUnsignedDecode(tag))
        return FALSE;
      tag += numChoices;
      unsigned len;
      if (strm.LengthDecode(0, INT_MAX, len) != 0)
        return FALSE;
      if (CreateObject()) {
        PINDEX nextPos = strm.GetPosition() + len;
        ok = choice->Decode(strm);
        strm.SetPosition(nextPos);
      }
      else {
        PASN_OctetString * open_type = new PASN_OctetString;
        open_type->SetConstraints(PASN_ConstrainedObject::FixedConstraint, len);
        open_type->Decode(strm);
        if (open_type->GetSize() > 0)
          choice = open_type;
        else {
          delete open_type;
          ok = FALSE;
        }
      }
      return ok;
    }
  }

  if (numChoices < 2)
    tag = 0;
  else {
    if (strm.UnsignedDecode(0, numChoices-1, tag) < 0)
      return FALSE;
  }

  if (CreateObject())
    ok = choice->Decode(strm);

  return ok;
}


void PASN_Choice::EncodePER(PPER_Stream & strm) const
{
  PAssert(tag != UINT_MAX, PLogicError);

  if (extendable) {
    BOOL extended = tag >= numChoices;
    strm.SingleBitEncode(extended);
    if (extended) {
      strm.SmallUnsignedEncode(tag - numChoices);
      strm.AnyTypeEncode(choice);
      return;
    }
  }

  if (numChoices > 1)
    strm.UnsignedEncode(tag, 0, numChoices-1);

  if (choice != NULL)
    choice->Encode(strm);
}


BOOL PBER_Stream::ChoiceDecode(PASN_Choice & value)
{
  PINDEX savedPosition = GetPosition();

  unsigned tag;
  PASN_Object::TagClass tagClass;
  BOOL primitive;
  unsigned entryLen;
  if (!HeaderDecode(tag, tagClass, primitive, entryLen))
    return FALSE;

  SetPosition(savedPosition);

  value.SetTag(tag, tagClass);
  if (value.IsValid())
    return value.GetObject().Decode(*this);

  return TRUE;
}


void PBER_Stream::ChoiceEncode(const PASN_Choice & value)
{
  if (value.IsValid())
    value.GetObject().Encode(*this);
}


BOOL PPER_Stream::ChoiceDecode(PASN_Choice & value)
{
  return value.DecodePER(*this);
}


void PPER_Stream::ChoiceEncode(const PASN_Choice & value)
{
  value.EncodePER(*this);
}


///////////////////////////////////////////////////////////////////////

PASN_Sequence::PASN_Sequence(unsigned tag, TagClass tagClass,
                             unsigned nOpts, BOOL extend, unsigned nExtend)
  : PASN_Object(tag, tagClass, extend)
{
  optionMap.SetConstraints(PASN_ConstrainedObject::FixedConstraint, nOpts);
  knownExtensions = nExtend;
  totalExtensions = 0;
  endBasicEncoding = 0;
}


PASN_Sequence::PASN_Sequence(const PASN_Sequence & other)
  : PASN_Object(other),
    fields(other.fields.GetSize()),
    optionMap(other.optionMap),
    extensionMap(other.extensionMap)
{
  for (PINDEX i = 0; i < other.fields.GetSize(); i++)
    fields.SetAt(i, other.fields[i].Clone());

  knownExtensions = other.knownExtensions;
  totalExtensions = other.totalExtensions;
  endBasicEncoding = 0;
}


PASN_Sequence & PASN_Sequence::operator=(const PASN_Sequence & other)
{
  PASN_Object::operator=(other);

  fields.SetSize(other.fields.GetSize());
  for (PINDEX i = 0; i < other.fields.GetSize(); i++)
    fields.SetAt(i, other.fields[i].Clone());

  optionMap = other.optionMap;
  knownExtensions = other.knownExtensions;
  totalExtensions = other.totalExtensions;
  extensionMap = other.extensionMap;

  return *this;
}


BOOL PASN_Sequence::HasOptionalField(PINDEX opt) const
{
  if (opt < (PINDEX)optionMap.GetSize())
    return optionMap[opt];
  else
    return extensionMap[opt - optionMap.GetSize()];
}


void PASN_Sequence::IncludeOptionalField(PINDEX opt)
{
  if (opt < (PINDEX)optionMap.GetSize())
    optionMap.Set(opt);
  else {
    PAssert(extendable, "Must be extendable type");
    opt -= optionMap.GetSize();
    if (opt >= (PINDEX)extensionMap.GetSize())
      extensionMap.SetSize(opt+1);
    extensionMap.Set(opt);
  }
}


void PASN_Sequence::RemoveOptionalField(PINDEX opt)
{
  if (opt < (PINDEX)optionMap.GetSize())
    optionMap.Clear(opt);
  else {
    PAssert(extendable, "Must be extendable type");
    opt -= optionMap.GetSize();
    extensionMap.Clear(opt);
  }
}


PObject::Comparison PASN_Sequence::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Sequence::Class()), PInvalidCast);
  const PASN_Sequence & other = (const PASN_Sequence &)obj;
  return fields.Compare(other.fields);
}


PObject * PASN_Sequence::Clone() const
{
  PAssert(IsClass(PASN_Sequence::Class()), PInvalidCast);
  return new PASN_Sequence(*this);
}


void PASN_Sequence::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    strm << setw(indent+6) << "field[" << i << "] <";
    switch (fields[i].GetTagClass()) {
      case UniversalTagClass :
        strm << "Universal";
        break;
      case ApplicationTagClass :
        strm << "Application";
        break;
      case ContextSpecificTagClass :
        strm << "ContextSpecific";
        break;
      case PrivateTagClass :
        strm << "Private";
      default :
        break;
    }
    strm << '-' << fields[i].GetTag() << '-'
         << fields[i].GetTypeAsString() << "> = "
         << fields[i] << '\n';
  }
  strm << setw(indent-1) << "}";
}


PString PASN_Sequence::GetTypeAsString() const
{
  return "Sequence";
}


PINDEX PASN_Sequence::GetDataLength() const
{
  PINDEX len = 0;
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    len += fields[i].GetObjectLength();
  return len;
}


BOOL PASN_Sequence::IsPrimitive() const
{
  return FALSE;
}


BOOL PASN_Sequence::Decode(PASN_Stream & strm)
{
  return PreambleDecode(strm) && UnknownExtensionsDecode(strm);
}


void PASN_Sequence::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);
  UnknownExtensionsEncode(strm);
}


BOOL PASN_Sequence::PreambleDecode(PASN_Stream & strm)
{
  return strm.SequencePreambleDecode(*this);
}


void PASN_Sequence::PreambleEncode(PASN_Stream & strm) const
{
  strm.SequencePreambleEncode(*this);
}


BOOL PASN_Sequence::KnownExtensionDecode(PASN_Stream & strm, PINDEX fld, PASN_Object & field)
{
  return strm.SequenceKnownDecode(*this, fld, field);
}


void PASN_Sequence::KnownExtensionEncode(PASN_Stream & strm, PINDEX fld, const PASN_Object & field) const
{
  strm.SequenceKnownEncode(*this, fld, field);
}


BOOL PASN_Sequence::UnknownExtensionsDecode(PASN_Stream & strm)
{
  return strm.SequenceUnknownDecode(*this);
}


void PASN_Sequence::UnknownExtensionsEncode(PASN_Stream & strm) const
{
  strm.SequenceUnknownEncode(*this);
}


BOOL PASN_Sequence::PreambleDecodeBER(PBER_Stream & strm)
{
  fields.RemoveAll();

  unsigned len;
  if (!strm.HeaderDecode(*this, len))
    return FALSE;

  endBasicEncoding = strm.GetPosition() + len;
  return !strm.IsAtEnd();
}


void PASN_Sequence::PreambleEncodeBER(PBER_Stream & strm) const
{
  strm.HeaderEncode(*this);
}


BOOL PASN_Sequence::KnownExtensionDecodeBER(PBER_Stream & strm, PINDEX, PASN_Object & field)
{
  if (strm.GetPosition() >= endBasicEncoding)
    return FALSE;

  return field.Decode(strm);
}


void PASN_Sequence::KnownExtensionEncodeBER(PBER_Stream & strm, PINDEX, const PASN_Object & field) const
{
  field.Encode(strm);
}


BOOL PASN_Sequence::UnknownExtensionsDecodeBER(PBER_Stream & strm)
{
  while (strm.GetPosition() < endBasicEncoding) {
    PINDEX savedPosition = strm.GetPosition();

    unsigned tag;
    PASN_Object::TagClass tagClass;
    BOOL primitive;
    unsigned entryLen;
    if (!strm.HeaderDecode(tag, tagClass, primitive, entryLen))
      return FALSE;

    PINDEX nextEntryPosition = strm.GetPosition() + entryLen;
    strm.SetPosition(savedPosition);

    PASN_Object * obj = strm.CreateObject(tag, tagClass, primitive);
    if (obj == NULL)
      strm.SetPosition(nextEntryPosition);
    else {
      if (!obj->Decode(strm))
        return FALSE;

      fields.Append(obj);
    }
  }

  return TRUE;
}


void PASN_Sequence::UnknownExtensionsEncodeBER(PBER_Stream & strm) const
{
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    fields[i].Encode(strm);
}


BOOL PASN_Sequence::PreambleDecodePER(PPER_Stream & strm)
{
  // X.691 Section 18

  if (extendable) {
    if (strm.IsAtEnd())
      return FALSE;
    totalExtensions = strm.SingleBitDecode() ? -1 : 0;  // 18.1
  }
  else
    totalExtensions = 0;
  return optionMap.Decode(strm);  // 18.2
}


void PASN_Sequence::PreambleEncodePER(PPER_Stream & strm) const
{
  // X.691 Section 18

  if (extendable) {
    BOOL hasExtensions = FALSE;
    for (unsigned i = 0; i < extensionMap.GetSize(); i++) {
      if (extensionMap[i]) {
        hasExtensions = TRUE;
        break;
      }
    }
    strm.SingleBitEncode(hasExtensions);  // 18.1
    ((PASN_Sequence*)this)->totalExtensions = hasExtensions ? -1 : 0;
  }
  optionMap.Encode(strm);  // 18.2
}


BOOL PASN_Sequence::NoExtensionsToDecode(PPER_Stream & strm)
{
  if (totalExtensions == 0)
    return TRUE;

  if (totalExtensions < 0) {
    if (!extensionMap.DecodeSequenceExtensionBitmap(strm))
      return FALSE;
    totalExtensions = extensionMap.GetSize();
  }

  return FALSE;
}


BOOL PASN_Sequence::NoExtensionsToEncode(PPER_Stream & strm)
{
  if (totalExtensions == 0)
    return TRUE;

  if (totalExtensions < 0) {
    totalExtensions = extensionMap.GetSize();
    extensionMap.EncodeSequenceExtensionBitmap(strm);
  }

  return FALSE;
}


BOOL PASN_Sequence::KnownExtensionDecodePER(PPER_Stream & strm, PINDEX fld, PASN_Object & field)
{
  if (NoExtensionsToDecode(strm))
    return TRUE;

  if (!extensionMap[fld-optionMap.GetSize()])
    return TRUE;

  unsigned len;
  if (strm.LengthDecode(0, INT_MAX, len) != 0)
    return FALSE;

  PINDEX nextExtensionPosition = strm.GetPosition() + len;
  BOOL ok = field.Decode(strm);
  strm.SetPosition(nextExtensionPosition);
  return ok;
}


void PASN_Sequence::KnownExtensionEncodePER(PPER_Stream & strm, PINDEX fld, const PASN_Object & field) const
{
  if (((PASN_Sequence*)this)->NoExtensionsToEncode(strm))
    return;

  if (!extensionMap[fld-optionMap.GetSize()])
    return;

  strm.AnyTypeEncode(&field);
}


BOOL PASN_Sequence::UnknownExtensionsDecodePER(PPER_Stream & strm)
{
  if (NoExtensionsToDecode(strm))
    return TRUE;

  if (totalExtensions <= knownExtensions)
    return TRUE;  // Already read them

  PINDEX unknownCount = totalExtensions - knownExtensions;
  if (fields.GetSize() >= unknownCount)
    return TRUE;  // Already read them

  fields.SetSize(unknownCount);

  PINDEX i;
  for (i = 0; i < fields.GetSize(); i++)
    fields.SetAt(i, new PASN_OctetString);

  for (i = knownExtensions; i < (PINDEX)extensionMap.GetSize(); i++) {
    if (extensionMap[i])
      if (!fields[i-knownExtensions].Decode(strm))
        return FALSE;
  }

  return TRUE;
}


void PASN_Sequence::UnknownExtensionsEncodePER(PPER_Stream & strm) const
{
  if (((PASN_Sequence*)this)->NoExtensionsToEncode(strm))
    return;

  int i;
  for (i = knownExtensions; i < totalExtensions; i++) {
    if (extensionMap[i])
      fields[i-knownExtensions].Encode(strm);
  }
}


BOOL PBER_Stream::SequencePreambleDecode(PASN_Sequence & seq)
{
  return seq.PreambleDecodeBER(*this);
}


void PBER_Stream::SequencePreambleEncode(const PASN_Sequence & seq)
{
  seq.PreambleEncodeBER(*this);
}


BOOL PBER_Stream::SequenceKnownDecode(PASN_Sequence & seq, PINDEX fld, PASN_Object & field)
{
  return seq.KnownExtensionDecodeBER(*this, fld, field);
}


void PBER_Stream::SequenceKnownEncode(const PASN_Sequence & seq, PINDEX fld, const PASN_Object & field)
{
  seq.KnownExtensionEncodeBER(*this, fld, field);
}


BOOL PBER_Stream::SequenceUnknownDecode(PASN_Sequence & seq)
{
  return seq.UnknownExtensionsDecodeBER(*this);
}


void PBER_Stream::SequenceUnknownEncode(const PASN_Sequence & seq)
{
  seq.UnknownExtensionsEncodeBER(*this);
}


BOOL PPER_Stream::SequencePreambleDecode(PASN_Sequence & seq)
{
  return seq.PreambleDecodePER(*this);
}


void PPER_Stream::SequencePreambleEncode(const PASN_Sequence & seq)
{
  seq.PreambleEncodePER(*this);
}


BOOL PPER_Stream::SequenceKnownDecode(PASN_Sequence & seq, PINDEX fld, PASN_Object & field)
{
  return seq.KnownExtensionDecodePER(*this, fld, field);
}


void PPER_Stream::SequenceKnownEncode(const PASN_Sequence & seq, PINDEX fld, const PASN_Object & field)
{
  seq.KnownExtensionEncodePER(*this, fld, field);
}


BOOL PPER_Stream::SequenceUnknownDecode(PASN_Sequence & seq)
{
  return seq.UnknownExtensionsDecodePER(*this);
}


void PPER_Stream::SequenceUnknownEncode(const PASN_Sequence & seq)
{
  seq.UnknownExtensionsEncodePER(*this);
}


///////////////////////////////////////////////////////////////////////

PASN_Set::PASN_Set(unsigned tag, TagClass tagClass,
                   unsigned nOpts, BOOL extend, unsigned nExtend)
  : PASN_Sequence(tag, tagClass, nOpts, extend, nExtend)
{
}


PObject * PASN_Set::Clone() const
{
  PAssert(IsClass(PASN_Set::Class()), PInvalidCast);
  return new PASN_Set(*this);
}


PString PASN_Set::GetTypeAsString() const
{
  return "Set";
}


///////////////////////////////////////////////////////////////////////

PASN_Array::PASN_Array(unsigned tag, TagClass tagClass)
  : PASN_ConstrainedObject(tag, tagClass)
{
}


PASN_Array::PASN_Array(const PASN_Array & other)
  : PASN_ConstrainedObject(other),
    array(other.array.GetSize())
{
  for (PINDEX i = 0; i < other.array.GetSize(); i++)
    array.SetAt(i, other.array[i].Clone());
}


PASN_Array & PASN_Array::operator=(const PASN_Array & other)
{
  PASN_ConstrainedObject::operator=(other);

  array.SetSize(other.array.GetSize());
  for (PINDEX i = 0; i < other.array.GetSize(); i++)
    array.SetAt(i, other.array[i].Clone());

  return *this;
}


void PASN_Array::SetSize(PINDEX newSize)
{
  PINDEX originalSize = array.GetSize();
  array.SetSize(newSize);
  for (PINDEX i = originalSize; i < newSize; i++)
    array.SetAt(i, CreateObject());
}


PObject::Comparison PASN_Array::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PASN_Array::Class()), PInvalidCast);
  const PASN_Array & other = (const PASN_Array &)obj;
  return array.Compare(other.array);
}


void PASN_Array::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << array.GetSize() << " entries {\n";
  for (PINDEX i = 0; i < array.GetSize(); i++)
    strm << setw(indent+1) << "[" << i << "]=" << setprecision(indent) << array[i] << '\n';
  strm << setw(indent-1) << "}";
}


void PASN_Array::SetConstraintBounds(ConstraintType type, int lower, unsigned upper)
{
  PAssert(lower >= 0, PInvalidParameter);
  PASN_ConstrainedObject::SetConstraintBounds(type, lower, upper);
  if (constraint != Unconstrained) {
    if (GetSize() < (PINDEX)lowerLimit)
      SetSize(lowerLimit);
    else if (GetSize() > (PINDEX)upperLimit)
      SetSize(upperLimit);
  }
}


PString PASN_Array::GetTypeAsString() const
{
  return "Array";
}


PINDEX PASN_Array::GetDataLength() const
{
  PINDEX len = 0;
  for (PINDEX i = 0; i < array.GetSize(); i++)
    len += array[i].GetObjectLength();
  return len;
}


BOOL PASN_Array::IsPrimitive() const
{
  return FALSE;
}


BOOL PASN_Array::Decode(PASN_Stream & strm)
{
  return strm.ArrayDecode(*this);
}


void PASN_Array::Encode(PASN_Stream & strm) const
{
  strm.ArrayEncode(*this);
}


BOOL PBER_Stream::ArrayDecode(PASN_Array & array)
{
  array.RemoveAll();

  unsigned len;
  if (!HeaderDecode(array, len))
    return FALSE;

  PINDEX endOffset = byteOffset + len;
  PINDEX count = 0;
  while (byteOffset < endOffset) {
    array.SetSize(count+1);
    if (!array[count].Decode(*this))
      return FALSE;
    count++;
  }

  byteOffset = endOffset;

  return TRUE;
}


void PBER_Stream::ArrayEncode(const PASN_Array & array)
{
  HeaderEncode(array);
  for (PINDEX i = 0; i < array.GetSize(); i++)
    array[i].Encode(*this);
}


BOOL PPER_Stream::ArrayDecode(PASN_Array & array)
{
  array.RemoveAll();

  unsigned size;
  if (array.ConstrainedLengthDecode(*this, size) < 0)
    return FALSE;

  array.SetSize(size);

  for (PINDEX i = 0; i < (PINDEX)size; i++) {
    if (!array[i].Decode(*this))
      return FALSE;
  }

  return TRUE;
}


void PPER_Stream::ArrayEncode(const PASN_Array & array)
{
  PINDEX size = array.GetSize();
  array.ConstrainedLengthEncode(*this, size);
  for (PINDEX i = 0; i < size; i++)
    array[i].Encode(*this);
}


///////////////////////////////////////////////////////////////////////

PASN_Stream::PASN_Stream()
{
  Construct();
}


PASN_Stream::PASN_Stream(const PBYTEArray & bytes)
  : PBYTEArray(bytes)
{
  Construct();
}


PASN_Stream::PASN_Stream(const BYTE * buf, PINDEX size)
  : PBYTEArray(buf, size)
{
  Construct();
}


void PASN_Stream::Construct()
{
  byteOffset = 0;
  bitOffset = 8;
}


void PASN_Stream::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << " size=" << GetSize()
       << " pos=" << byteOffset << '.' << (8-bitOffset)
       << " {\n";
  PINDEX i = 0;
  while (i < GetSize()) {
    strm << setw(indent) << " " << hex << setfill('0');
    PINDEX j;
    for (j = 0; j < 16; j++)
      if (i+j < GetSize())
        strm << setw(2) << (unsigned)(BYTE)theArray[i+j] << ' ';
      else
        strm << "   ";
    strm << "  ";
    for (j = 0; j < 16; j++) {
      if (i+j < GetSize()) {
        if (isprint(theArray[i+j]))
          strm << theArray[i+j];
        else
          strm << ' ';
      }
    }
    strm << dec << setfill(' ') << '\n';
    i += 16;
  }
  strm << setw(indent-1) << "}";
}


void PASN_Stream::SetPosition(PINDEX newPos)
{
  PAssert(byteOffset != P_MAX_INDEX, PLogicError);

  if (newPos > GetSize())
    byteOffset = GetSize();
  else
    byteOffset = newPos;
  bitOffset = 8;
}


void PASN_Stream::ResetDecoder()
{
  byteOffset = 0;
  bitOffset = 8;
}


void PASN_Stream::BeginEncoding()
{
  bitOffset = 8;
  byteOffset = 0;
  PBYTEArray::operator=(PBYTEArray(20));
}


void PASN_Stream::CompleteEncoding()
{
  if (byteOffset != P_MAX_INDEX) {
    if (bitOffset != 8) {
      bitOffset = 8;
      byteOffset++;
    }
    SetSize(byteOffset);
    byteOffset = P_MAX_INDEX;
  }
}


BYTE PASN_Stream::ByteDecode()
{
  if (byteOffset >= GetSize())
    return 0;

  bitOffset = 8;
  return theArray[byteOffset++];
}


void PASN_Stream::ByteEncode(unsigned value)
{
  PAssert(byteOffset != P_MAX_INDEX, PLogicError);

  if (bitOffset != 8) {
    bitOffset = 8;
    byteOffset++;
  }
  if (byteOffset >= GetSize())
    SetSize(byteOffset+10);
  theArray[byteOffset++] = (BYTE)value;
}


unsigned PASN_Stream::BlockDecode(BYTE * bufptr, unsigned nBytes)
{
  if (nBytes == 0)
    return 0;

  ByteAlign();

  if (byteOffset+nBytes > (unsigned)GetSize()) {
    nBytes = GetSize() - byteOffset;
    if (nBytes == 0)
      return 0;
  }

  memcpy(bufptr, &theArray[byteOffset], nBytes);
  byteOffset += nBytes;
  return nBytes;
}


void PASN_Stream::BlockEncode(const BYTE * bufptr, PINDEX nBytes)
{
  PAssert(byteOffset != P_MAX_INDEX, PLogicError);

  if (nBytes == 0)
    return;

  ByteAlign();

  if (byteOffset+nBytes >= GetSize())
    SetSize(byteOffset+nBytes+10);

  memcpy(theArray+byteOffset, bufptr, nBytes);
  byteOffset += nBytes;
}


void PASN_Stream::ByteAlign()
{
  PAssert(byteOffset != P_MAX_INDEX, PLogicError);

  if (bitOffset != 8) {
    bitOffset = 8;
    byteOffset++;
  }
}


///////////////////////////////////////////////////////////////////////

PBER_Stream::PBER_Stream()
{
}


PBER_Stream::PBER_Stream(const PBYTEArray & bytes)
  : PASN_Stream(bytes)
{
}


PBER_Stream::PBER_Stream(const BYTE * buf, PINDEX size)
  : PASN_Stream(buf, size)
{
}


PBER_Stream & PBER_Stream::operator=(const PBYTEArray & bytes)
{
  PBYTEArray::operator=(bytes);
  ResetDecoder();
  return *this;
}


BOOL PBER_Stream::Read(PChannel & chan)
{
  SetSize(0);
  PINDEX offset = 0;

  // read the sequence header
  int b;
  if ((b = chan.ReadChar()) < 0)
    return FALSE;

  SetAt(offset++, (char)b);

  // only support direct read of simple sequences
  if ((b&0x1f) == 0x1f) {
    do {
      if ((b = chan.ReadChar()) < 0)
        return FALSE;
      SetAt(offset++, (char)b);
    } while ((b & 0x80) != 0);
  }

  // read the first byte of the ASN length
  if ((b = chan.ReadChar()) < 0)
    return FALSE;

  SetAt(offset++, (char)b);

  // determine how many bytes in the length
  PINDEX dataLen = 0;
  if ((b & 0x80) == 0)
    dataLen = b;
  else {
    PINDEX lenLen = b&0x7f;
    SetSize(lenLen+2);
    while (lenLen-- > 0) {
      // read the length
      if ((b = chan.ReadChar()) < 0)
        return FALSE;
      dataLen = (dataLen << 8) | b;
      SetAt(offset++, (char)b);
    }
  }

  // read the data, all of it
  BYTE * bufptr = GetPointer(dataLen+offset) + offset;
  while (dataLen > 0) {
    if (!chan.Read(bufptr, dataLen))
      return FALSE;
    PINDEX readbytes = chan.GetLastReadCount();
    bufptr += readbytes;
    dataLen -= readbytes;
  }
  return TRUE;
}


BOOL PBER_Stream::Write(PChannel & chan)
{
  CompleteEncoding();
  return chan.Write(theArray, GetSize());
}


PASN_Object * PBER_Stream::CreateObject(unsigned tag,
                                        PASN_Object::TagClass tagClass,
                                        BOOL primitive) const
{
  if (tagClass == PASN_Object::UniversalTagClass) {
    switch (tag) {
      case PASN_Object::UniversalBoolean :
        return new PASN_Boolean();

      case PASN_Object::UniversalInteger :
        return new PASN_Integer();

      case PASN_Object::UniversalBitString :
        return new PASN_BitString();

      case PASN_Object::UniversalOctetString :
        return new PASN_OctetString();

      case PASN_Object::UniversalNull :
        return new PASN_Null();

      case PASN_Object::UniversalObjectId :
        return new PASN_ObjectId();

      case PASN_Object::UniversalReal :
        return new PASN_Real();

      case PASN_Object::UniversalEnumeration :
        return new PASN_Enumeration();

      case PASN_Object::UniversalSequence :
        return new PASN_Sequence();

      case PASN_Object::UniversalSet :
        return new PASN_Set();

      case PASN_Object::UniversalNumericString :
        return new PASN_NumericString();

      case PASN_Object::UniversalPrintableString :
        return new PASN_PrintableString();

      case PASN_Object::UniversalIA5String :
        return new PASN_IA5String();

      case PASN_Object::UniversalVisibleString :
        return new PASN_VisibleString();

      case PASN_Object::UniversalGeneralString :
        return new PASN_GeneralString();

      case PASN_Object::UniversalBMPString :
        return new PASN_BMPString();
    }
  }

  if (primitive)
    return new PASN_OctetString(tag, tagClass);
  else
    return new PASN_Sequence(tag, tagClass, 0, FALSE, 0);
}


BOOL PBER_Stream::HeaderDecode(unsigned & tagVal,
                               PASN_Object::TagClass & tagClass,
                               BOOL & primitive,
                               unsigned & len)
{
  BYTE ident = ByteDecode();
  tagClass = (PASN_Object::TagClass)(ident>>6);
  primitive = (ident&0x20) == 0;
  tagVal = ident&31;
  if (tagVal == 31) {
    BYTE b;
    tagVal = 0;
    do {
      if (IsAtEnd())
        return FALSE;

      b = ByteDecode();
      tagVal = (tagVal << 7) | (b&0x7f);
    } while ((b&0x80) != 0);
  }

  if (IsAtEnd())
    return FALSE;

  BYTE len_len = ByteDecode();
  if ((len_len & 0x80) == 0) {
    len = len_len;
    return TRUE;
  }

  len_len &= 0x7f;

  len = 0;
  while (len_len-- > 0) {
    if (IsAtEnd())
      return FALSE;

    len = (len << 8) | ByteDecode();
  }

  return TRUE;
}


BOOL PBER_Stream::HeaderDecode(PASN_Object & obj, unsigned & len)
{
  PINDEX pos = byteOffset;

  unsigned tagVal;
  PASN_Object::TagClass tagClass;
  BOOL primitive;
  if (HeaderDecode(tagVal, tagClass, primitive, len) &&
              tagVal == obj.GetTag() && tagClass == obj.GetTagClass())
    return TRUE;

  byteOffset = pos;
  return FALSE;
}


void PBER_Stream::HeaderEncode(const PASN_Object & obj)
{
  BYTE ident = (BYTE)(obj.GetTagClass() << 6);
  if (!obj.IsPrimitive())
    ident |= 0x20;
  unsigned tag = obj.GetTag();
  if (tag < 31)
    ByteEncode(ident|tag);
  else {
    ByteEncode(ident|31);
    unsigned count = (CountBits(tag)+6)/7;
    while (count-- > 1)
      ByteEncode((tag >> (count*7))&0x7f);
    ByteEncode(tag&0x7f);
  }

  PINDEX len = obj.GetDataLength();
  if (len < 128)
    ByteEncode(len);
  else {
    PINDEX count = (CountBits(len+1)+7)/8;
    ByteEncode(count|0x80);
    while (count-- > 0)
      ByteEncode(len >> (count*8));
  }
}



///////////////////////////////////////////////////////////////////////

PPER_Stream::PPER_Stream(BOOL alignment)
{
  aligned = alignment;
}


PPER_Stream::PPER_Stream(const PBYTEArray & bytes, BOOL alignment)
  : PASN_Stream(bytes)
{
  aligned = alignment;
}


PPER_Stream::PPER_Stream(const BYTE * buf, PINDEX size, BOOL alignment)
  : PASN_Stream(buf, size)
{
  aligned = alignment;
}


PPER_Stream & PPER_Stream::operator=(const PBYTEArray & bytes)
{
  PBYTEArray::operator=(bytes);
  ResetDecoder();
  aligned = TRUE;
  return *this;
}


unsigned PPER_Stream::GetBitsLeft() const
{
  return (GetSize() - byteOffset)*8 - (8 - bitOffset);
}


BOOL PPER_Stream::Read(PChannel & chan)
{
  ResetDecoder();
  SetSize(0);

  // Get RFC1006 TPKT length
  BYTE tpkt[4];
  if (!chan.ReadBlock(tpkt, sizeof(tpkt)))
    return FALSE;

  if (tpkt[0] != 3) // Only support version 3
    return TRUE;

  PINDEX data_len = ((tpkt[2] << 8)|tpkt[3]) - 4;

  return chan.ReadBlock(GetPointer(data_len), data_len);
}


BOOL PPER_Stream::Write(PChannel & chan)
{
  CompleteEncoding();

  PINDEX size = GetSize();

  // Put RFC1006 TPKT length
  BYTE tpkt[4];
  tpkt[0] = 3;  // Version 3
  tpkt[1] = 0;

  PINDEX len = size + sizeof(tpkt);
  tpkt[2] = (BYTE)(len >> 8);
  tpkt[3] = (BYTE)len;

  return chan.Write(tpkt, sizeof(tpkt)) && chan.Write(theArray, size);
}


BOOL PPER_Stream::SingleBitDecode()
{
  if ((GetSize() - byteOffset)*8 - (8 - bitOffset) == 0)
    return FALSE;

  bitOffset--;
  BOOL value = (theArray[byteOffset] & (1 << bitOffset)) != 0;

  if (bitOffset == 0) {
    bitOffset = 8;
    byteOffset++;
  }

  return value;
}


void PPER_Stream::SingleBitEncode(BOOL value)
{
  PAssert(byteOffset != P_MAX_INDEX, PLogicError);

  if (byteOffset >= GetSize())
    SetSize(byteOffset+10);

  bitOffset--;

  if (value)
    theArray[byteOffset] |= 1 << bitOffset;

  if (bitOffset == 0)
    ByteAlign();
}


BOOL PPER_Stream::MultiBitDecode(unsigned nBits, unsigned & value)
{
  if (nBits > sizeof(value)*8)
    return FALSE;

  unsigned bitsLeft = (GetSize() - byteOffset)*8 - (8 - bitOffset);
  if (nBits > bitsLeft)
    return FALSE;

  if (nBits == 0) {
    value = 0;
    return TRUE;
  }

  if (nBits < bitOffset) {
    bitOffset -= nBits;
    value = (theArray[byteOffset] >> bitOffset) & ((1 << nBits) - 1);
    return TRUE;
  }

  value = theArray[byteOffset] & ((1 << bitOffset) - 1);
  nBits -= bitOffset;
  bitOffset = 8;
  byteOffset++;

  while (nBits >= 8) {
    value = (value << 8) | (BYTE)theArray[byteOffset];
    byteOffset++;
    nBits -= 8;
  }

  if (nBits > 0) {
    bitOffset = 8 - nBits;
    value = (value << nBits) | ((BYTE)theArray[byteOffset] >> bitOffset);
  }

  return TRUE;
}


void PPER_Stream::MultiBitEncode(unsigned value, unsigned nBits)
{
  PAssert(byteOffset != P_MAX_INDEX, PLogicError);

  if (nBits == 0)
    return;

  if (byteOffset+nBits/8+1 >= (unsigned)GetSize())
    SetSize(byteOffset+10);

  // Make sure value is in bounds of bit available.
  if (nBits < sizeof(int)*8)
    value &= ((1 << nBits) - 1);

  if (nBits < bitOffset) {
    bitOffset -= nBits;
    theArray[byteOffset] |= value << bitOffset;
    return;
  }

  nBits -= bitOffset;
  theArray[byteOffset] |= (BYTE)(value >> nBits);
  bitOffset = 8;
  byteOffset++;

  while (nBits >= 8) {
    nBits -= 8;
    theArray[byteOffset] = (BYTE)(value >> nBits);
    byteOffset++;
  }

  if (nBits > 0) {
    bitOffset = 8 - nBits;
    theArray[byteOffset] |= (BYTE)((value & ((1 << nBits)-1)) << bitOffset);
  }
}


BOOL PPER_Stream::SmallUnsignedDecode(unsigned & value)
{
  // X.691 Section 10.6

  if (!SingleBitDecode())
    return MultiBitDecode(6, value);      // 10.6.1

  unsigned len;
  if (LengthDecode(0, INT_MAX, len) != 0)  // 10.6.2
    return FALSE;

  ByteAlign();
  return MultiBitDecode(len*8, value);
}


void PPER_Stream::SmallUnsignedEncode(unsigned value)
{
  if (value < 64) {
    MultiBitEncode(value, 7);
    return;
  }

  SingleBitEncode(1);  // 10.6.2

  PINDEX len = 4;
  if (value < 256)
    len = 1;
  else if (value < 65536)
    len = 2;
  else if (value < 0x1000000)
    len = 3;
  LengthEncode(len, 0, INT_MAX);  // 10.9
  ByteAlign();
  MultiBitEncode(value, len*8);
}


int PPER_Stream::UnsignedDecode(unsigned lower, unsigned upper, unsigned & value)
{
  // X.691 section 10.5

  if (lower == upper) { // 10.5.4
    value = lower;
    return 0;
  }

  if (IsAtEnd())
    return -1;

  unsigned range = (upper - lower) + 1;
  unsigned nBits = CountBits(range);

  if (aligned && (range == 0 || range > 255)) { // not 10.5.6 and not 10.5.7.1
    if (nBits > 16) {                           // not 10.5.7.4
      LengthDecode(1, (nBits+7)/8, nBits);      // 12.2.6
      nBits *= 8;
    }
    else if (nBits > 8)    // not 10.5.7.2
      nBits = 16;          // 10.5.7.3
    ByteAlign();           // 10.7.5.2 - 10.7.5.4
  }

  if (!MultiBitDecode(nBits, value))
    return -1;

  value += lower;
  return 0;
}


void PPER_Stream::UnsignedEncode(int value, unsigned lower, unsigned upper)
{
  // X.691 section 10.5

  if (lower == upper) // 10.5.4
    return;

  unsigned range = (upper - lower) + 1;
  PINDEX nBits = CountBits(range);

  if (aligned && (range == 0 || range > 255)) { // not 10.5.6 and not 10.5.7.1
    if (nBits > 16) {                           // not 10.5.7.4
      int numBytes = value == 0 ? 1 : (((CountBits(value - lower + 1))+7)/8);
      LengthEncode(numBytes, 1, (nBits+7)/8);    // 12.2.6
      nBits = numBytes*8;
    }
    else if (nBits > 8)      // not 10.5.7.2
      nBits = 16;            // 10.5.7.3
    ByteAlign();             // 10.7.5.2 - 10.7.5.4
  }

  MultiBitEncode(value - lower, nBits);
}


int PPER_Stream::LengthDecode(unsigned lower, unsigned upper, unsigned & len)
{
  // X.691 section 10.9

  if (upper != INT_MAX && !aligned) {
    if (upper - lower > 0xffff)
      return -1; // 10.9.4.2 unsupported
    unsigned base;
    if (!MultiBitDecode(CountBits(upper - lower + 1), base))
      return -1;
    return lower + base;   // 10.9.4.1
  }

  if (upper < 65536)  // 10.9.3.3
    return UnsignedDecode(lower, upper, len);

  // 10.9.3.5
  ByteAlign();
  if (IsAtEnd())
    return -1;

  if (SingleBitDecode() == 0)
    return MultiBitDecode(7, len) ? 0 : -1;   // 10.9.3.6

  if (SingleBitDecode() == 0)
    return MultiBitDecode(14, len) ? 0 : -1;    // 10.9.3.7

  return -1;  // 10.9.3.8 unsupported
}


void PPER_Stream::LengthEncode(unsigned len, unsigned lower, unsigned upper)
{
  // X.691 section 10.9

  if (upper != INT_MAX && !aligned) {
    PAssert(upper - lower < 0x10000, PUnimplementedFunction);  // 10.9.4.2 unsupperted
    MultiBitEncode(len - lower, CountBits(upper - lower + 1));   // 10.9.4.1
    return;
  }

  if (upper < 65536) { // 10.9.3.3
    UnsignedEncode(len, lower, upper);
    return;
  }

  ByteAlign();

  if (len < 128) {
    MultiBitEncode(len, 8);   // 10.9.3.6
    return;
  }

  SingleBitEncode(TRUE);

  if (len < 0x2000) {
    MultiBitEncode(len, 15);    // 10.9.3.7
    return;
  }

  SingleBitEncode(TRUE);
  PAssert(len < 0x2000, PUnimplementedFunction);  // 10.9.3.8 unsupported
}


void PPER_Stream::AnyTypeEncode(const PASN_Object * value)
{
  PPER_Stream substream;

  if (value != NULL)
    value->Encode(substream);

  if (substream.GetPosition() == 0)   // Make sure extension has at least one
    substream.SingleBitEncode(FALSE); // byte in its ANY type encoding.

  substream.CompleteEncoding();

  PINDEX nBytes = substream.GetSize();
  LengthEncode(nBytes, 0, INT_MAX);
  BlockEncode(substream.GetPointer(), nBytes);
}


// PERASN.CXX
