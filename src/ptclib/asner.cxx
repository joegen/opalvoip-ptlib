/*
 * $Id: asner.cxx,v 1.6 1998/02/03 06:28:27 robertj Exp $
 *
 * Portable Windows Library
 *
 * Abstract Syntax Notation Encoding Rules
 *
 * Copyright 1993 Equivalence
 *
 * $Log: asner.cxx,v $
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

PASN_Object::PASN_Object(unsigned tag_, TagClass tagClass_, BOOL extend)
{
  extendable = extend;
  tag = tag_;
  if (tagClass_ != DefaultTagClass)
    tagClass = tagClass_;
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


///////////////////////////////////////////////////////////////////////

PASN_ConstrainedObject::PASN_ConstrainedObject(unsigned tag, TagClass tagClass,
                                               int lower, unsigned upper,
                                               ConstraintType ctype)
  : PASN_Object(tag, tagClass, ctype == ExtendableConstraint)
{
  SetConstraints(ctype, lower, upper);
}


void PASN_ConstrainedObject::SetConstraints(ConstraintType ctype,
                                            int lower, unsigned upper)
{
  unconstrained = ctype == Unconstrained;
  extendable = ctype == ExtendableConstraint;
  PAssert((lower >= 0 || upper < 0x7fffffff) &&
          (lower < 0 || (unsigned)lower <= upper), PInvalidParameter);
  lowerLimit = lower;
  upperLimit = upper;
}


PASN_ConstrainedObject::ConstraintType
              PASN_ConstrainedObject::ConstraintDecode(PPER_Stream & strm,
                                                       unsigned * range)
{
  if (extendable)
    if (strm.SingleBitDecode())
      return Unconstrained;

  if (unconstrained)
    return Unconstrained;

  if (range != NULL)
    *range = (unsigned)(upperLimit - lowerLimit) + 1;
  return FixedConstraint;
}


PASN_ConstrainedObject::ConstraintType
              PASN_ConstrainedObject::ConstraintEncode(PPER_Stream & strm,
                                                       unsigned value,
                                                       unsigned * range) const
{
  if (extendable) {
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
    if (needsExtending)
      return Unconstrained;
  }

  if (unconstrained)
    return Unconstrained;

  if (range != NULL)
    *range = (unsigned)(upperLimit - lowerLimit) + 1;
  return FixedConstraint;
}


///////////////////////////////////////////////////////////////////////

PASN_Null::PASN_Null(unsigned tag, TagClass tagClass)
  : PASN_Object(tag, tagClass, FALSE)
{
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
  PINDEX len;
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
  : PASN_Object(tag, tagClass, FALSE)
{
  value = val;
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
  PINDEX len;
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

PASN_Integer::PASN_Integer(unsigned tag, TagClass tagClass,
                           int lower, unsigned upper,
                           ConstraintType ctype,
                           unsigned val)
  : PASN_ConstrainedObject(tag, tagClass, lower, upper, ctype)
{
  operator=(val);
}


PASN_Integer & PASN_Integer::operator=(unsigned val)
{
  if (unconstrained)
    value = val;
  else if (val > upperLimit)
    value = upperLimit;
  else if (lowerLimit < 0 ? ((int)val < lowerLimit) : (val < (unsigned)lowerLimit))
    value = lowerLimit;
  else
    value = val;
  return *this;
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
  PINDEX len;
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
  // X.931 Sections 12

  unsigned val;
  unsigned range = 0;
  if (value.ConstraintDecode(*this, &range) == PASN_ConstrainedObject::Unconstrained) { //  12.1
    if (IsAtEnd())
      return FALSE;
    val = MultiBitDecode(LengthDecode(0, INT_MAX));
  }
  else if (range == 1) // 12.2.1
    val = 0;
  else { // 12.2.2 which devolves to 10.5
    if (IsAtEnd())
      return FALSE;
    val = UnsignedDecode(range);
  }
  value = val + value.GetLowerLimit();
  return TRUE;
}


void PPER_Stream::IntegerEncode(const PASN_Integer & value)
{
  // X.931 Sections 12

  int adjusted_value = value - value.GetLowerLimit();

  unsigned range;
  if (value.ConstraintEncode(*this, (int)value, &range) == PASN_ConstrainedObject::Unconstrained) { //  12.1
    PINDEX nBits = adjusted_value == 0 ? 8 : CountBits(adjusted_value);
    LengthEncode(nBits, 0, INT_MAX);
    MultiBitEncode(adjusted_value, nBits);
    return;
  }

  if (range == 1) // 12.2.1
    return;

  // 12.2.2 which devolves to 10.5
  UnsignedEncode(adjusted_value, range);
}


///////////////////////////////////////////////////////////////////////

PASN_Enumeration::PASN_Enumeration(unsigned tag, TagClass tagClass,
                                   unsigned nEnums, BOOL extend,
                                   unsigned val)
  : PASN_Object(tag, tagClass, extend)
{
  value = val;
  numEnums = nEnums;
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
                                   unsigned nEnums, BOOL extend,
                                   const PString & nameSpec,
                                   unsigned val)
  : PASN_Object(tag, tagClass, extend),
    names(BuildNamesDict(nameSpec))
{
  PAssert(nEnums > 0, PInvalidParameter);
  PAssert(val < nEnums, PInvalidParameter);
  value = val;
  numEnums = nEnums;
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
  PINDEX len;
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
  // X.691 Section 13

  if (value.IsExtendable()) {  // 13.3
    if (SingleBitDecode()) {
      if (IsAtEnd())
        return FALSE;
      value = UnsignedDecode(SmallUnsignedDecode());
      return TRUE;
    }
  }

  if (IsAtEnd())
    return FALSE;
  value = UnsignedDecode(value.GetMaximum());  // 13.2
  return TRUE;
}


void PPER_Stream::EnumerationEncode(const PASN_Enumeration & value)
{
  // X.691 Section 13

  if (value.IsExtendable()) {  // 13.3
    BOOL extended = value >= value.GetMaximum();
    SingleBitEncode(extended);
    if (extended) {
      SmallUnsignedEncode(1+value);
      UnsignedEncode(value, value);
      return;
    }
  }

  UnsignedEncode(value, value.GetMaximum()-1);  // 13.2
}


///////////////////////////////////////////////////////////////////////

PASN_Real::PASN_Real(unsigned tag, TagClass tagClass, double val)
  : PASN_Object(tag, tagClass, FALSE)
{
  value = val;
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
  PINDEX len;
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
  : PASN_Object(tag, tagClass, FALSE)
{
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


BOOL PASN_ObjectId::CommonDecode(PASN_Stream & strm, PINDEX dataLen)
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
  PINDEX len;
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

  PINDEX dataLen = LengthDecode(0, 255);
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

PASN_BitString::PASN_BitString(unsigned tag, TagClass tagClass,
                               int lower, unsigned upper, ConstraintType ctype,
                               PINDEX nBits)
  : PASN_ConstrainedObject(tag, tagClass, lower, upper, ctype),
    totalBits((int)nBits < lower ? lower : ((unsigned)nBits > upper ? upper : nBits)),
    bitData((totalBits+7)/8)
{
  PAssert(lower >= 0, PInvalidParameter);
}


void PASN_BitString::SetData(PINDEX nBits, const PBYTEArray & bytes)
{
  bitData = bytes;
  totalBits = nBits;
}


void PASN_BitString::SetData(PINDEX nBits, const BYTE * buf, PINDEX size)
{
  bitData = PBYTEArray(buf, size);
  totalBits = nBits;
}


BOOL PASN_BitString::SetSize(PINDEX nBits)
{
  totalBits = nBits;
  return bitData.SetSize((nBits+7)/8);
}


BOOL PASN_BitString::operator[](PINDEX bit) const
{
  if (bit < totalBits)
    return (bitData[bit>>3] & (1 << (7 - (bit&7)))) != 0;
  return FALSE;
}


void PASN_BitString::Set(PINDEX bit)
{
  if (bit < totalBits)
    bitData[bit>>3] |= 1 << (7 - (bit&7));
}


void PASN_BitString::Clear(PINDEX bit)
{
  if (bit < totalBits)
    bitData[bit>>3] &= ~(1 << (7 - (bit&7)));
}


void PASN_BitString::Invert(PINDEX bit)
{
  if (bit < totalBits)
    bitData[bit>>3] ^= 1 << (7 - (bit&7));
}


void PASN_BitString::PrintOn(ostream & strm) const
{
  BYTE mask = 0x80;
  PINDEX offset = 0;
  for (PINDEX i = 0; i < totalBits; i++) {
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


BOOL PASN_BitString::DecodeBER(PBER_Stream & strm, PINDEX len)
{
  totalBits = len*8 - strm.ByteDecode();
  PINDEX nBytes = (totalBits+7)/8;
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

  unsigned range = 0;
  if (ConstraintDecode(strm, &range) == PASN_ConstrainedObject::Unconstrained) // 15.5
    totalBits = strm.LengthDecode(0, INT_MAX);
  else if (range == 1)
    totalBits = upperLimit;
  else { // 15.10
    totalBits = strm.LengthDecode(lowerLimit, upperLimit);
    strm.ByteAlign();
  }
  SetSize(totalBits);

  if (totalBits == 0)
    return TRUE;   // 15.7

  if (totalBits > strm.GetBitsLeft())
    return FALSE;

  if (totalBits > 16) {
    PINDEX nBytes = (totalBits+7)/8;
    return strm.BlockDecode(bitData.GetPointer(), nBytes) == nBytes;   // 15.9
  }
  else if (totalBits <= 8)
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

  unsigned range;
  if (ConstraintEncode(strm, totalBits, &range) == PASN_ConstrainedObject::Unconstrained) // 15.5
    strm.LengthEncode(totalBits, 0, INT_MAX);
  else if (range != 1) { // 15.10
    strm.LengthEncode(totalBits, lowerLimit, upperLimit);
    strm.ByteAlign();
  }

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
  PINDEX len;
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

PASN_OctetString::PASN_OctetString(unsigned tag, TagClass tagClass,
                                   int lower, unsigned upper, ConstraintType ctype)
  : PASN_ConstrainedObject(tag, tagClass, lower, upper, ctype),
    value(lower)
{
  PAssert(lower >= 0, PInvalidParameter);
}


PString PASN_OctetString::AsString() const
{
  if (value.IsEmpty())
    return PString();
  return PString((const char *)(const BYTE *)value, value.GetSize());
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

  PINDEX nBytes;
  unsigned range = 0;
  if (ConstraintDecode(strm, &range) == Unconstrained) // 16.3
    nBytes = strm.LengthDecode(0, INT_MAX);
  else if (range != 1)  // 16.8
    nBytes = strm.LengthDecode(lowerLimit, upperLimit);
  else {
    switch (upperLimit) {
      case 1 :  // 16.6
        value[0] = (BYTE)strm.MultiBitDecode(8);
        return TRUE;

      case 2 :  // 16.6
        value.SetSize(2);
        value[0] = (BYTE)strm.MultiBitDecode(8);
        value[1] = (BYTE)strm.MultiBitDecode(8);
        return TRUE;

      default: // 16.7
        nBytes = upperLimit;
    }
  }
  value.SetSize(nBytes);   // 16.5

  if (nBytes == 0)
    return TRUE;

  return strm.BlockDecode(value.GetPointer(), nBytes) == nBytes;
}


void PASN_OctetString::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 16

  PINDEX nBytes = value.GetSize();
  unsigned range;
  if (ConstraintEncode(strm, nBytes, &range) == PASN_ConstrainedObject::Unconstrained) // 16.3
    strm.LengthEncode(nBytes, 0, INT_MAX);
  else if (range != 1) // 16.8
    strm.LengthEncode(nBytes, lowerLimit, upperLimit);
  else {
    switch (upperLimit) {
      case 1 :  // 16.6
        strm.MultiBitEncode(value[0], 8);
        return;

      case 2 :  // 16.6
        strm.MultiBitEncode(value[0], 8);
        strm.MultiBitEncode(value[1], 8);
        return;

      default: // 16.7
        nBytes = upperLimit;
    }
  }

  if (nBytes != 0)    // 16.5
    strm.BlockEncode(value, nBytes);
}


BOOL PBER_Stream::OctetStringDecode(PASN_OctetString & value)
{
  PINDEX len;
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

PASN_ConstrainedString::PASN_ConstrainedString(const char * canonicalSet, PINDEX size,
                                               unsigned tag, TagClass tagClass,
                                               int lower, unsigned upper,
                                               ConstraintType ctype,
                                               const char * set, BOOL)
  : PASN_ConstrainedObject(tag, tagClass, lower, upper, ctype)
{
  PAssert(lower >= 0, PInvalidParameter);
  canonicalSetBits = CountBits(size);

  if (set == NULL)
    charSet = canonicalSet;
  else {
    for (PINDEX i = 0; i < size; i++) {
      if (strchr(set, canonicalSet[i]) != NULL)
        charSet += canonicalSet[i];
    }
  }

  charSetUnalignedBits = CountBits(charSet.GetLength());
  charSetAlignedBits = 1;
  while (charSetUnalignedBits > charSetAlignedBits)
    charSetAlignedBits <<= 1;
}


PASN_ConstrainedString & PASN_ConstrainedString::operator=(const char * str)
{
  value = PString();
  PINDEX len = strlen(str);
  for (PINDEX i = 0; i < len; i++) {
    if (charSet.Find(str[i]) != P_MAX_INDEX)
      value += str[i];
  }
  return *this;
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


BOOL PASN_ConstrainedString::DecodeBER(PBER_Stream & strm, PINDEX len)
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

  PINDEX len;
  if (ConstraintDecode(strm) == Unconstrained) // 26.4
    len = strm.LengthDecode(0, INT_MAX);
  else
    len = strm.LengthDecode(lowerLimit, upperLimit);

  PINDEX nBits = strm.IsAligned() ? charSetAlignedBits : charSetUnalignedBits;

  if (len*nBits > 16) {
    if (nBits == 8)
      return strm.BlockDecode((BYTE *)value.GetPointer(len+1), len) == len;
    if (strm.IsAligned())
      strm.ByteAlign();
  }

  value.SetSize(len+1);

  PINDEX i;
  for (i = 0; i < len; i++) {
    if (nBits >= canonicalSetBits && canonicalSetBits > 4)
      value[i] = (char)strm.MultiBitDecode(nBits);
    else
      value[i] = charSet[strm.MultiBitDecode(nBits)];
  }
  value[i] = '\0';

  return TRUE;
}


void PASN_ConstrainedString::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 26

  PINDEX len = value.GetSize()-1;
  if (ConstraintEncode(strm, len) == PASN_ConstrainedObject::Unconstrained) // 26.4
    strm.LengthEncode(len, 0, INT_MAX);
  else
    strm.LengthEncode(len, lowerLimit, upperLimit);

  PINDEX nBits = strm.IsAligned() ? charSetAlignedBits : charSetUnalignedBits;

  if (len*nBits > 16) {
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
    else
      strm.MultiBitEncode(charSet.Find(value[i]), nBits);
  }
}


BOOL PBER_Stream::ConstrainedStringDecode(PASN_ConstrainedString & value)
{
  PINDEX len;
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



static const char NumericStringSet[] = " 0123456789";

PASN_NumericString::PASN_NumericString(unsigned tag, TagClass tagClass,
                                       int lower, unsigned upper, ConstraintType ctype,
                                       const char * charSet, BOOL extendChars)
  : PASN_ConstrainedString(NumericStringSet, sizeof(NumericStringSet)-1,
                           tag, tagClass, lower, upper, ctype, charSet, extendChars)
{
}

PString PASN_NumericString::GetTypeAsString() const
{
  return "Numeric String";
}



static const char PrintableStringSet[] =
            " '()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

PASN_PrintableString::PASN_PrintableString(unsigned tag, TagClass tagClass,
                                           int lower, unsigned upper, ConstraintType ctype,
                                           const char * charSet, BOOL extendChars)
  : PASN_ConstrainedString(PrintableStringSet, sizeof(PrintableStringSet)-1,
                           tag, tagClass, lower, upper, ctype, charSet, extendChars)
{
}

PString PASN_PrintableString::GetTypeAsString() const
{
  return "Printable String";
}



static const char VisibleStringSet[] =
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~";

PASN_VisibleString::PASN_VisibleString(unsigned tag, TagClass tagClass,
                                       int lower, unsigned upper, ConstraintType ctype,
                                       const char * charSet, BOOL extendChars)
  : PASN_ConstrainedString(VisibleStringSet, sizeof(VisibleStringSet)-1,
                           tag, tagClass, lower, upper, ctype, charSet, extendChars)
{
}

PString PASN_VisibleString::GetTypeAsString() const
{
  return "Visible String";
}



static const char IA5StringSet[] =
    "\200\001\002\003\004\005\006\007"
    "\010\011\012\013\014\015\016\017"
    "\020\021\022\023\024\025\026\027"
    "\030\031\032\033\034\035\036\037"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~\177";

PASN_IA5String::PASN_IA5String(unsigned tag, TagClass tagClass,
                               int lower, unsigned upper, ConstraintType ctype,
                               const char * charSet, BOOL extendChars)
  : PASN_ConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                           tag, tagClass, lower, upper, ctype, charSet, extendChars)
{
}

PString PASN_IA5String::GetTypeAsString() const
{
  return "IA5 String";
}



PASN_GeneralString::PASN_GeneralString(unsigned tag, TagClass tagClass,
                                       int lower, unsigned upper, ConstraintType ctype,
                                       const char * charSet, BOOL extendChars)
  : PASN_ConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                           tag, tagClass, lower, upper, ctype, charSet, extendChars)
{
}

PString PASN_GeneralString::GetTypeAsString() const
{
  return "General String";
}


///////////////////////////////////////////////////////////////////////

PASN_BMPString::PASN_BMPString(unsigned tag, TagClass tagClass,
                               int lower, unsigned upper, ConstraintType ctype)
  : PASN_ConstrainedObject(tag, tagClass, lower, upper, ctype)
{
  PAssert(lower >= 0, PInvalidParameter);
}


PASN_BMPString & PASN_BMPString::operator=(const PString & str)
{
  value.SetSize(str.GetLength());
  for (PINDEX i = 0; i < value.GetSize(); i++)
    value[i] = (BYTE)str[i];
  return *this;
}


PString PASN_BMPString::GetValue() const
{
  PString str;
  for (PINDEX i = 0; i < value.GetSize(); i++) {
    if (value[i] < 256)
      str += PString((char)value[i]);
  }
  return str;
}


void PASN_BMPString::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << ' ' << value.GetSize() << " characters {\n";
  PINDEX i = 0;
  while (i < value.GetSize()) {
    strm << setw(indent) << ' ';
    PINDEX j;
    for (j = 0; j < 8; j++)
      if (i+j < value.GetSize())
        strm << hex << setfill('0') << setw(4) << value[i+j] << ' ';
      else
        strm << "     ";
    strm << "  ";
    for (j = 0; j < 8; j++) {
      if (i+j < value.GetSize()) {
        if (isprint(value[i+j]))
          strm << (char)value[i+j];
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


BOOL PASN_BMPString::DecodeBER(PBER_Stream & strm, PINDEX len)
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

  PINDEX len;
  if (ConstraintDecode(strm) == Unconstrained) // 26.4
    len = strm.LengthDecode(0, INT_MAX);
  else
    len = strm.LengthDecode(lowerLimit, upperLimit);
  value.SetSize(len);

  if (len == 1)
    value[0] = (WORD)strm.MultiBitDecode(16);
  else {
    strm.ByteAlign();
    for (PINDEX i = 0; i < len; i++)
      value[i] = (WORD)strm.MultiBitDecode(16);
  }
  return TRUE;
}


void PASN_BMPString::EncodePER(PPER_Stream & strm) const
{
  // X.691 Section 26

  PINDEX len = value.GetSize();
  if (ConstraintEncode(strm, len) == PASN_ConstrainedObject::Unconstrained) // 26.4
    strm.LengthEncode(len, 0, INT_MAX);
  else
    strm.LengthEncode(len, lowerLimit, upperLimit);

  if (len == 1)
    strm.MultiBitEncode(value[0], 16);
  else {
    strm.ByteAlign();
    for (PINDEX i = 0; i < len; i++)
      strm.MultiBitEncode(value[i], 16);
  }
}


BOOL PBER_Stream::BMPStringDecode(PASN_BMPString & value)
{
  PINDEX len;
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

PASN_Choice::PASN_Choice(PINDEX nChoices, BOOL extend)
  : PASN_Object(0, ApplicationTagClass, extend)
{
  maxChoices = nChoices;
  choice = NULL;
}


PASN_Choice::PASN_Choice(unsigned tag, TagClass tagClass,
                         PINDEX upper, BOOL extend)
  : PASN_Object(tag, tagClass, extend)
{
  maxChoices = upper;
  choice = NULL;
}


PASN_Choice::PASN_Choice(unsigned tag, TagClass tagClass,
                         PINDEX upper, BOOL extend, const PString & nameSpec)
  : PASN_Object(tag, tagClass, extend),
    names(BuildNamesDict(nameSpec))
{
  maxChoices = upper;
  choice = NULL;
}


PASN_Choice & PASN_Choice::operator=(const PASN_Choice & other)
{
  delete choice;

  maxChoices = other.maxChoices;
  if (CreateObject() && other.choice != NULL)
    *choice = *other.choice;
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

  return PString(PString::Unsigned, tag);
}


PASN_Object & PASN_Choice::GetObject() const
{
  PAssert(choice != NULL, "NULL Choice");
  return *choice;
}


void PASN_Choice::PrintOn(ostream & strm) const
{
  if (tag == UINT_MAX)
    strm << "<uninitialised>";
  else if (names.Contains(tag))
    strm << names[tag];
  else
    strm << '<' << tag << '>';

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

  if (extendable) {
    if (strm.SingleBitDecode()) {
      tag = strm.SmallUnsignedDecode() + maxChoices;
      PINDEX len = strm.LengthDecode(0, INT_MAX);
      if (CreateObject()) {
        PINDEX nextPos = strm.GetPosition() + len;
        choice->Decode(strm);
        strm.SetPosition(nextPos);
      }
      else {
        PASN_OctetString * open_type = new PASN_OctetString(
                              UniversalOctetString, UniversalTagClass,
                              len, len, PASN_ConstrainedObject::FixedConstraint);
        open_type->Decode(strm);
        if (open_type->GetSize() > 0)
          choice = open_type;
        else
          delete open_type;
      }
      return TRUE;
    }
  }

  if (maxChoices > 1)
    tag = strm.UnsignedDecode(maxChoices);
  else
    tag = 0;

  if (CreateObject())
    choice->Decode(strm);
  return TRUE;
}


void PASN_Choice::EncodePER(PPER_Stream & strm) const
{
  PAssert(tag != UINT_MAX, PLogicError);

  if (extendable) {
    BOOL extended = tag >= maxChoices;
    strm.SingleBitEncode(extended);
    if (extended) {
      strm.SmallUnsignedEncode(tag - maxChoices);
      strm.AnyTypeEncode(choice);
      return;
    }
  }

  if (maxChoices > 1)
    strm.UnsignedEncode(tag, maxChoices);

  if (choice != NULL)
    choice->Encode(strm);
}


BOOL PBER_Stream::ChoiceDecode(PASN_Choice & value)
{
  PINDEX savedPosition = GetPosition();

  unsigned tag;
  PASN_Object::TagClass tagClass;
  BOOL primitive;
  PINDEX entryLen;
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
                             PINDEX nOpts, BOOL extend, PINDEX nExtend)
  : PASN_Object(tag, tagClass, extend),
    optionMap(UniversalBitString, UniversalTagClass,
              nOpts, nOpts, PASN_ConstrainedObject::FixedConstraint)
{
  knownExtensions = nExtend;
  totalExtensions = 0;
}


PASN_Sequence & PASN_Sequence::operator=(const PASN_Sequence & other)
{
  optionMap = other.optionMap;
  knownExtensions = other.knownExtensions;
  totalExtensions = other.totalExtensions;
  extensionMap = other.extensionMap;
  for (PINDEX i = 0; i < other.fields.GetSize(); i++)
    fields[i] = other.fields[i];
  return *this;
}


BOOL PASN_Sequence::HasOptionalField(PINDEX opt) const
{
  if (opt < optionMap.GetSize())
    return optionMap[opt];
  else
    return extensionMap[opt - optionMap.GetSize()];
}


void PASN_Sequence::IncludeOptionalField(PINDEX opt)
{
  PAssert(extendable, "Must be extendable type");

  if (opt < optionMap.GetSize())
    optionMap.Set(opt);
  else {
    opt -= optionMap.GetSize();
    if (opt < extensionMap.GetSize())
      extensionMap.Set(opt);
  }
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

  PINDEX len;
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
    PINDEX entryLen;
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

  if (extendable)
    totalExtensions = strm.SingleBitDecode() ? -1 : 0;  // 18.1
  else
    totalExtensions = 0;
  return optionMap.Decode(strm);  // 18.2
}


void PASN_Sequence::PreambleEncodePER(PPER_Stream & strm) const
{
  // X.691 Section 18

  if (extendable) {
    BOOL hasExtensions = FALSE;
    for (PINDEX i = 0; i < extensionMap.GetSize(); i++) {
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

  PINDEX len = strm.LengthDecode(0, INT_MAX);
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

  for (i = knownExtensions; i < extensionMap.GetSize(); i++) {
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
                   PINDEX nOpts, BOOL extend, PINDEX nExtend)
  : PASN_Sequence(tag, tagClass, nOpts, extend, nExtend)
{
}


PString PASN_Set::GetTypeAsString() const
{
  return "Set";
}


///////////////////////////////////////////////////////////////////////

PASN_Array::PASN_Array(unsigned tag, TagClass tagClass,
                       int lower, unsigned upper, ConstraintType ctype)
  : PASN_ConstrainedObject(tag, tagClass, lower, upper, ctype)
{
  PAssert(lower >= 0, PInvalidParameter);
}


PASN_Array & PASN_Array::operator=(const PASN_Array & other)
{
  for (PINDEX i = 0; i < other.array.GetSize(); i++)
    array[i] = other.array[i];
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
  strm << "{\n";
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

  PINDEX len;
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

  PINDEX size;
  if (array.ConstraintDecode(*this) == PASN_ConstrainedObject::Unconstrained)
    size = LengthDecode(0, INT_MAX);
  else
    size = LengthDecode(array.GetLowerLimit(), array.GetUpperLimit());

  array.SetSize(size);

  for (PINDEX i = 0; i < size; i++)
    if (!array[i].Decode(*this))
      return FALSE;
  return TRUE;
}


void PPER_Stream::ArrayEncode(const PASN_Array & array)
{
  PINDEX size = array.GetSize();
  if (array.ConstraintEncode(*this, size) == PASN_ConstrainedObject::Unconstrained)
    LengthEncode(size, 0, INT_MAX);
  else
    LengthEncode(size, array.GetLowerLimit(), array.GetUpperLimit());

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
  SetSize(20);
  memset(theArray, 0, 20);
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


PINDEX PASN_Stream::BlockDecode(BYTE * bufptr, PINDEX nBytes)
{
  ByteAlign();

  if (byteOffset+nBytes > GetSize())
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
                               PINDEX & len)
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


BOOL PBER_Stream::HeaderDecode(PASN_Object & obj, PINDEX & len)
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


PINDEX PPER_Stream::GetBitsLeft() const
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


int PPER_Stream::MultiBitDecode(PINDEX nBits)
{
  PAssert(nBits <= 32, PInvalidParameter);

  PINDEX bitsLeft = (GetSize() - byteOffset)*8 - (8 - bitOffset);
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


void PPER_Stream::MultiBitEncode(int value, PINDEX nBits)
{
  if (nBits == 0)
    return;

  if (byteOffset+nBits/8+1 >= GetSize())
    SetSize(byteOffset+10);

  // Make sure value is in bounds of bit available.
  value &= (1 << nBits) - 1;

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

  PINDEX len = LengthDecode(0, INT_MAX);  // 10.6.2
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


PINDEX PPER_Stream::LengthDecode(int lower, int upper)
{
  // X.691 section 10.9

  if (upper == lower)
    return upper;

  if (upper != INT_MAX && !aligned) {
    PAssert(upper - lower < 0x10000, PUnimplementedFunction);  // 10.9.4.2 unsupperted
    return lower + MultiBitDecode(CountBits(upper - lower + 1));   // 10.9.4.1
  }

  // 10.9.3.3
  unsigned range = upper - lower + 1;
  if (range < 256)
    return lower + MultiBitDecode(CountBits(range));
  if (range <= 65536) {
    ByteAlign();
    return lower + MultiBitDecode(range != 256 ? 16 : 8);
  }

  ByteAlign();
  if (SingleBitDecode() == 0)
    return lower + MultiBitDecode(7);   // 10.9.3.6

  PAssert(!SingleBitDecode(), PUnimplementedFunction);  // 10.9.3.8 unsupported
  return lower + MultiBitDecode(14);    // 10.9.3.7
}


void PPER_Stream::LengthEncode(PINDEX len, int lower, int upper)
{
  if (upper == lower)
    return;

  len -= lower;

  if (upper != INT_MAX && !aligned) {
    PAssert(upper - lower < 0x10000, PUnimplementedFunction);  // 10.9.4.2 unsupperted
    MultiBitEncode(len, CountBits(upper - lower + 1));   // 10.9.4.1
    return;
  }

  // 10.9.3.3
  unsigned range = upper - lower + 1;
  if (range < 256) {
    MultiBitEncode(len, CountBits(range));
    return;
  }
  if (range <= 65536) {
    ByteAlign();
    MultiBitEncode(len, range != 256 ? 16 : 8);
    return;
  }

  ByteAlign();
  if (len < 128) {
    MultiBitEncode(len, 8);   // 10.9.3.6
    return;
  }
  SingleBitEncode(TRUE);

  PAssert(len < 0x2000, PUnimplementedFunction);  // 10.9.3.8 unsupported
  MultiBitEncode(len, 15);    // 10.9.3.7
}


int PPER_Stream::UnsignedDecode(unsigned range)
{
  PINDEX nBits = CountBits(range);

  if (nBits >= 8 && aligned) {      // 10.5.7.1
    if (nBits > 16)                 // not 10.5.7.4
      nBits = LengthDecode(1, (nBits+7)/8)*8;     // 12.2.6
    else if (nBits > 8)             // not 10.5.7.2
      nBits = 16;                   // 10.5.7.3
    ByteAlign();               // 10.7.5.2 - 10.7.5.4
  }

  return MultiBitDecode(nBits);
}


void PPER_Stream::UnsignedEncode(int value, unsigned range)
{
  PINDEX nBits = CountBits(range);

  if (nBits >= 8 && aligned) {      // 10.5.7.1
    if (nBits > 16) {               // not 10.5.7.4
      int numBytes = value == 0 ? 1 : (((CountBits(value))+7)/8);
      LengthEncode(numBytes, 1, (nBits+7)/8);    // 12.2.6
      nBits = numBytes*8;
    }
    else if (nBits > 8)             // not 10.5.7.2
      nBits = 16;                   // 10.5.7.3
    ByteAlign();                // 10.7.5.2 - 10.7.5.4
  }

  MultiBitEncode(value, nBits);
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
