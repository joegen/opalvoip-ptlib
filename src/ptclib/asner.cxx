/*
 * $Id: asner.cxx,v 1.11 1998/05/21 04:58:54 robertj Exp $
 *
 * Portable Windows Library
 *
 * Abstract Syntax Notation Encoding Rules
 *
 * Copyright 1993 Equivalence
 *
 * $Log: asner.cxx,v $
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

#include <asner.h>

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


void PASN_Object::SetConstraints(ConstraintType, MinimumValueTag lower, unsigned upper)
{
  SetConstraints(PartiallyConstrained, (int)lower, upper);
}


void PASN_Object::SetConstraints(ConstraintType, int lower, MaximumValueTag upper)
{
  SetConstraints(PartiallyConstrained, lower, (unsigned)upper);
}


void PASN_Object::SetConstraints(ConstraintType, MinimumValueTag lower, MaximumValueTag upper)
{
  SetConstraints(PartiallyConstrained, (int)lower, (unsigned)upper);
}


void PASN_Object::SetConstraints(ConstraintType, int, unsigned)
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


void PASN_ConstrainedObject::SetConstraints(ConstraintType ctype,
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

PASN_Boolean::PASN_Boolean(BOOL val, unsigned tag, TagClass tagClass)
  : PASN_Object(tag, tagClass)
{
  value = val;
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

PASN_Integer::PASN_Integer(unsigned tag, TagClass tagClass, unsigned val)
  : PASN_ConstrainedObject(tag, tagClass)
{
  value = val;
}


PASN_Integer & PASN_Integer::operator=(unsigned val)
{
  if (constraint == Unconstrained)
    value = val;
  else if (val > upperLimit)
    value = upperLimit;
  else if (lowerLimit < 0 ? ((int)val < lowerLimit) : (val < (unsigned)lowerLimit))
    value = lowerLimit;
  else
    value = val;
  return *this;
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
    value = strm.MultiBitDecode(len*8);
    return TRUE;
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
    if (strm.SingleBitDecode())
      return strm.UnsignedDecode(0, strm.SmallUnsignedDecode()-1, value) == 0;
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

PASN_Real::PASN_Real(unsigned tag, TagClass tagClass, double val)
  : PASN_Object(tag, tagClass)
{
  value = val;
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

  PINDEX len = MultiBitDecode(8)+1;
  PAssertAlways(PUnimplementedFunction);
  byteOffset += len;
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
  SetValue(dotstr);
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
  value.SetSize(2);

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
  if (subId == 0x2B) {
    value[0] = 1;
    value[1] = 3;
  }
  else {
    value[1] = subId % 40;
    value[0] = (subId - value[1]) / 40;
  }

  return TRUE;
}


void PASN_ObjectId::CommonEncode(PBYTEArray & eObjId) const
{
  PINDEX      offs = 0;
  unsigned    subId, mask, testmask;
  int         bits, testbits;
  PINDEX      objIdLen = value.GetSize();
  const unsigned * objId = value;

  if (objIdLen < 2) {
    eObjId[offs++] = 0;
    objIdLen = 0;
  } else {
    eObjId[offs++] = (BYTE)(objId[1] + (objId[0] * 40));
    objIdLen -= 2;
    objId += 2;
  }

  while (objIdLen-- > 0) {
    subId = *objId++;
    if (subId < 128) 
      eObjId [offs++] = (BYTE)subId;
    else {
      mask = 0x7F; /* handle subid == 0 case */
      bits = 0;

      /* testmask *MUST* !!!! be of an unsigned type */
      for (testmask = 0x7F, testbits = 0;
           testmask != 0;
           testmask <<= 7, testbits += 7) {
        if (subId & testmask) {  /* if any bits set */
          mask = testmask;
	        bits = testbits;
	      }
      }

      /* mask can't be zero here */
      for (; mask != 0x7F; mask >>= 7, bits -= 7) {
        /* fix a mask that got truncated above */
	if (mask == 0x1E00000)
	  mask = 0xFE00000;
        eObjId[offs++] = (u_char)(((subId & mask) >> bits) | 0x80);
      }
      eObjId[offs++] = (u_char)(subId & mask);
    }
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
  totalBits = nBits;
}


void PASN_BitString::SetData(unsigned nBits, const BYTE * buf, PINDEX size)
{
  bitData = PBYTEArray(buf, size);
  totalBits = nBits;
}


BOOL PASN_BitString::SetSize(unsigned nBits)
{
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


void PASN_BitString::SetConstraints(ConstraintType type, int lower, unsigned upper)
{
  PAssert(lower >= 0, PInvalidParameter);
  PASN_ConstrainedObject::SetConstraints(type, lower, upper);
  if (constraint != Unconstrained) {
    if (totalBits < (unsigned)lowerLimit)
      SetSize(lowerLimit);
    else if ((unsigned)totalBits > upperLimit)
      SetSize(upperLimit);
  }
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

  if (totalBits <= 8)
    bitData[0] = (BYTE)(strm.MultiBitDecode(totalBits) << (8-totalBits));
  else {  // 15.8
    bitData[0] = (BYTE)strm.MultiBitDecode(8);
    bitData[1] = (BYTE)(strm.MultiBitDecode(totalBits-8) << (16-totalBits));
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


void PASN_OctetString::SetConstraints(ConstraintType type, int lower, unsigned upper)
{
  PAssert(lower >= 0, PInvalidParameter);
  PASN_ConstrainedObject::SetConstraints(type, lower, upper);
  if (constraint != Unconstrained) {
    if (value.GetSize() < (PINDEX)lowerLimit)
      value.SetSize(lowerLimit);
    else if ((unsigned)value.GetSize() > upperLimit)
      value.SetSize(upperLimit);
  }
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
    strm << setw(indent) << ' ';
    PINDEX j;
    for (j = 0; j < 16; j++)
      if (i+j < value.GetSize())
        strm << hex << setfill('0') << setw(2) << (unsigned)value[i+j] << ' ';
      else
        strm << "   ";
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
  strm << setw(indent-1) << '}';
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

  switch (nBytes) {
    case 0 :
      break;

    case 1 :  // 16.6
      if (strm.IsAtEnd())
        return FALSE;
      value[0] = (BYTE)strm.MultiBitDecode(8);
      break;

    case 2 :  // 16.6
      if (strm.IsAtEnd())
        return FALSE;
      value[0] = (BYTE)strm.MultiBitDecode(8);
      value[1] = (BYTE)strm.MultiBitDecode(8);
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


///////////////////////////////////////////////////////////////////////

PASN_ConstrainedString::PASN_ConstrainedString(const char * canonical, PINDEX size,
                                               unsigned tag, TagClass tagClass)
  : PASN_ConstrainedObject(tag, tagClass)
{
  canonicalSet = canonical;
  canonicalSetSize = size;
  canonicalSetBits = CountBits(size);
  charSetUnalignedBits = 8;
  charSetAlignedBits = 8;
}


PASN_ConstrainedString & PASN_ConstrainedString::operator=(const char * str)
{
  value = PString();
  PINDEX len = strlen(str);
  for (PINDEX i = 0; i < len; i++) {
    PINDEX sz = characterSet.GetSize();
    if (sz == 0 || memchr(characterSet, str[i], sz) != NULL)
      value += str[i];
  }
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
  if (ctype == Unconstrained)
    characterSet.SetSize(0);
  else {
    characterSet.SetSize(setSize);
    PINDEX count = 0;
    for (PINDEX i = 0; i < setSize; i++) {
      if (memchr(canonicalSet, *set, canonicalSetSize) != NULL)
        characterSet[count++] = *set;
      set++;
    }
    PAssert(count > 0, PInvalidParameter);
    characterSet.SetSize(count);
  }

  if (characterSet.IsEmpty())
    charSetUnalignedBits = 8;
  else
    charSetUnalignedBits = CountBits(characterSet.GetSize());

  charSetAlignedBits = 1;
  while (charSetUnalignedBits > charSetAlignedBits)
    charSetAlignedBits <<= 1;
}


void PASN_ConstrainedString::PrintOn(ostream & strm) const
{
  strm << value.ToLiteral();
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

  if (constraint == Unconstrained || upperLimit*nBits > 16) {
    if (nBits == 8)
      return strm.BlockDecode((BYTE *)value.GetPointer(len+1), len) == len;
    if (strm.IsAligned())
      strm.ByteAlign();
  }

  value.SetSize(len+1);

  PINDEX i;
  for (i = 0; i < (PINDEX)len; i++) {
    if (strm.IsAtEnd())
      return FALSE;
    if (nBits >= canonicalSetBits && canonicalSetBits > 4)
      value[i] = (char)strm.MultiBitDecode(nBits);
    else
      value[i] = characterSet[strm.MultiBitDecode(nBits)];
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

  if (constraint == Unconstrained || upperLimit*nBits > 16) {
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

PASN_BMPString::PASN_BMPString(unsigned tag, TagClass tagClass)
  : PASN_ConstrainedObject(tag, tagClass)
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


PASN_BMPString & PASN_BMPString::operator=(const PString & str)
{
  PINDEX sz = str.GetLength();
  value.SetSize(sz);

  PINDEX count = 0;
  for (PINDEX i = 0; i < sz; i++) {
    WORD c = (BYTE)str[i];
    if (IsLegalCharacter(c))
      value[count++] = c;
  }

  value.SetSize(count);
  return *this;
}


PASN_BMPString & PASN_BMPString::operator=(const PWORDArray & array)
{
  PINDEX sz = array.GetSize();
  value.SetSize(sz);

  PINDEX count = 0;
  for (PINDEX i = 0; i < sz; i++) {
    WORD c = array[i];
    if (IsLegalCharacter(c))
      value[count++] = c;
  }

  value.SetSize(count);
  return *this;
}


PString PASN_BMPString::GetValue() const
{
  PString str;
  for (PINDEX i = 0; i < value.GetSize(); i++) {
    if (value[i] < 256)
      str += (char)value[i];
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


void PASN_BMPString::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  PINDEX sz = value.GetSize();
  strm << ' ' << sz << " characters {\n";
  PINDEX i = 0;
  while (i < sz) {
    strm << setw(indent) << ' ';
    PINDEX j;
    for (j = 0; j < 8; j++)
      if (i+j < sz)
        strm << hex << setfill('0') << setw(4) << value[i+j] << ' ';
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
  strm << setw(indent-1) << '}';
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
    if (strm.IsAtEnd())
      return FALSE;
    if (characterSet.IsEmpty())
      value[i] = (WORD)(strm.MultiBitDecode(nBits) + firstChar);
    else
      value[i] = characterSet[strm.MultiBitDecode(nBits)];
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
      tag = strm.SmallUnsignedDecode() + numChoices;
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
        open_type->SetConstraints(PASN_ConstrainedObject::FixedConstraint, len, len);
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
  optionMap.SetConstraints(PASN_ConstrainedObject::FixedConstraint, nOpts, nOpts);
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
  PAssert(extendable, "Must be extendable type");

  if (opt < (PINDEX)optionMap.GetSize())
    optionMap.Set(opt);
  else {
    opt -= optionMap.GetSize();
    if (opt < (PINDEX)extensionMap.GetSize())
      extensionMap.Set(opt);
  }
}


PObject * PASN_Sequence::Clone() const
{
  PAssert(IsClass(PASN_Sequence::Class()), PInvalidCast);
  return new PASN_Sequence(*this);
}


void PASN_Sequence::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << setprecision(indent) << "{\n";
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    strm << setw(indent) << ' ' << "field[" << i << "] <";
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
  strm << setw(indent-1) << '}';
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
    totalExtensions = strm.SmallUnsignedDecode() + 1;
    extensionMap.SetConstraints(PASN_ConstrainedObject::FixedConstraint,
                                totalExtensions, totalExtensions);
    extensionMap.Decode(strm);
  }

  return FALSE;
}


BOOL PASN_Sequence::NoExtensionsToEncode(PPER_Stream & strm)
{
  if (totalExtensions == 0)
    return TRUE;

  if (totalExtensions < 0) {
    totalExtensions = extensionMap.GetSize();
    strm.SmallUnsignedEncode(totalExtensions-1);
    extensionMap.Encode(strm);
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
      fields[i].Encode(strm);
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


void PASN_Array::PrintOn(ostream & strm) const
{
  int indent = strm.precision()+2;
  strm << array.GetSize() << " entries {\n";
  for (PINDEX i = 0; i < array.GetSize(); i++)
    strm << setw(indent+1) << '[' << i << "]=" << setprecision(indent) << array[i] << '\n';
  strm << setw(indent-1) << '}';
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
    strm << setw(indent) << ' ';
    PINDEX j;
    for (j = 0; j < 16; j++)
      if (i+j < GetSize())
        strm << hex << setfill('0') << setw(2) << (unsigned)(BYTE)theArray[i+j] << ' ';
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
  strm << setw(indent-1) << '}';
}


void PASN_Stream::SetPosition(PINDEX newPos)
{
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
  if (bitOffset != 8) {
    bitOffset = 8;
    byteOffset++;
  }
  SetSize(byteOffset);
  byteOffset = 0;
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
  ByteAlign();

  if (byteOffset+nBytes > (unsigned)GetSize())
    nBytes = GetSize() - byteOffset;

  if (nBytes == 0)
    return 0;

  memcpy(bufptr, &theArray[byteOffset], nBytes);
  byteOffset += nBytes;
  return nBytes;
}


void PASN_Stream::BlockEncode(const BYTE * bufptr, PINDEX nBytes)
{
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
    PINDEX count = (CountBits(len)+7)/8;
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
  // Get RFC1006 TPKT length
  BYTE tpkt[4];
  if (!chan.ReadBlock(tpkt, sizeof(tpkt)))
    return FALSE;

  if (tpkt[0] != 3) { // Only support version 3
    SetSize(0);
    return TRUE;
  }

  PINDEX data_len = ((tpkt[2] << 8)|tpkt[3]) - 4;

  return chan.ReadBlock(GetPointer(data_len), data_len);
}


BOOL PPER_Stream::Write(PChannel & chan)
{
  BYTE buf[5];
  PINDEX size = GetSize();
  PINDEX len_len = size > 0xfffffb ? 4 : 3;
  PINDEX len = size + len_len + 1;
  buf[0] = (BYTE)len_len;
  while (len_len > 0) {
    buf[len_len] = (BYTE)len;
    len >>= 8;
    len_len--;
  }
  return chan.Write(buf, buf[0]+1) && chan.Write(theArray, size);
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
  if (byteOffset >= GetSize())
    SetSize(byteOffset+10);
  bitOffset--;
  if (value)
    theArray[byteOffset] |= 1 << bitOffset;
  if (bitOffset == 0)
    ByteAlign();
}


int PPER_Stream::MultiBitDecode(unsigned nBits)
{
  PAssert(nBits <= 32, PInvalidParameter);

  unsigned bitsLeft = (GetSize() - byteOffset)*8 - (8 - bitOffset);
  if (nBits > bitsLeft)
    nBits = bitsLeft;

  if (nBits == 0)
    return 0;

  if (nBits < bitOffset) {
    bitOffset -= nBits;
    return (theArray[byteOffset] >> bitOffset) & ((1 << nBits) - 1);
  }

  int val = theArray[byteOffset] & ((1 << bitOffset) - 1);
  nBits -= bitOffset;
  bitOffset = 8;
  byteOffset++;

  while (nBits >= 8) {
    val = (val << 8) | (BYTE)theArray[byteOffset];
    byteOffset++;
    nBits -= 8;
  }

  if (nBits > 0) {
    bitOffset = 8 - nBits;
    val = (val << nBits) | ((BYTE)theArray[byteOffset] >> bitOffset);
  }

  return val;
}


void PPER_Stream::MultiBitEncode(int value, unsigned nBits)
{
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


unsigned PPER_Stream::SmallUnsignedDecode()
{
  // X.691 Section 10.6

  if (!SingleBitDecode())
    return MultiBitDecode(6);      // 10.6.1

  unsigned len;
  if (LengthDecode(0, INT_MAX, len) != 0)  // 10.6.2
    return FALSE;

  ByteAlign();
  return MultiBitDecode(len*8);
}


void PPER_Stream::SmallUnsignedEncode(unsigned value)
{
  if (value < 64) {
    MultiBitEncode(value, 7);
    return;
  }

  PINDEX len = 4;
  if (value < 256)
    len = 1;
  else if (value < 65536)
    len = 2;
  else if (value < 0x1000000)
    len = 3;
  LengthEncode(len, 0, INT_MAX);  // 10.6.2
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

  value = MultiBitDecode(nBits) + lower;
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
      int numBytes = value == 0 ? 1 : (((CountBits(value))+7)/8);
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
    PAssert(upper - lower < 0x10000, PUnimplementedFunction);  // 10.9.4.2 unsupported
    return lower + MultiBitDecode(CountBits(upper - lower + 1));   // 10.9.4.1
  }

  if (upper < 65536)  // 10.9.3.3
    return UnsignedDecode(lower, upper, len);

  // 10.9.3.5
  ByteAlign();
  if (IsAtEnd())
    return -1;

  if (SingleBitDecode() == 0) {
    len = MultiBitDecode(7);   // 10.9.3.6
    return 0;
  }

  if (SingleBitDecode() == 0) {
    len = MultiBitDecode(14);    // 10.9.3.7
    return 0;
  }

  PAssertAlways(PUnimplementedFunction);  // 10.9.3.8 unsupported
  return 1;
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
