/*
 * $Id: asner.cxx,v 1.2 1997/10/30 10:18:34 robertj Exp $
 *
 * Portable Windows Library
 *
 * Packed Encoding Rules for Abstract Syntax Notation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: asner.cxx,v $
 * Revision 1.2  1997/10/30 10:18:34  robertj
 * Fixed GNU C warnings.
 *
 * Revision 1.1  1997/10/10 10:38:05  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include "perasn.h"


///////////////////////////////////////////////////////////////////////

PPER_BitStream::PPER_BitStream(BOOL alignment)
{
  Construct(alignment);
}


PPER_BitStream::PPER_BitStream(const PBYTEArray & bytes, BOOL alignment)
  : bitData(bytes)
{
  Construct(alignment);
}


PPER_BitStream::PPER_BitStream(const BYTE * buf, PINDEX size, BOOL alignment)
  : bitData(buf, size)
{
  Construct(alignment);
}


void PPER_BitStream::Construct(BOOL alignment)
{
  byteOffset = 0;
  bitOffset = 8;
  aligned = alignment;
}


void PPER_BitStream::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << " size=" << bitData.GetSize()
       << " pos=" << byteOffset << '.' << (8-bitOffset)
       << " {\n";
  PINDEX i = 0;
  while (i < bitData.GetSize()) {
    strm << setw(indent) << ' ';
    PINDEX j;
    for (j = 0; j < 16; j++)
      if (i+j < bitData.GetSize())
        strm << hex << setfill('0') << setw(2) << (unsigned)bitData[i+j] << ' ';
      else
        strm << "   ";
    strm << "  ";
    for (j = 0; j < 16; j++) {
      if (i+j < bitData.GetSize()) {
        if (isprint(bitData[i+j]))
          strm << bitData[i+j];
        else
          strm << ' ';
      }
    }
    strm << dec << setfill(' ') << '\n';
    i += 16;
  }
  strm << setw(indent-1) << '}';
}


PINDEX PPER_BitStream::GetBitsLeft() const
{
  return (GetSize() - byteOffset)*8 - (8 - bitOffset);
}


BOOL PPER_BitStream::Read(PChannel & chan)
{
  byteOffset = 0;
  bitOffset = 8;

  int len_len = chan.ReadChar();
  if (len_len < 0)
    return FALSE;

  int data_len = 0;
  for (int len_count = 0; len_count < len_len; len_count++) {
    int len_part = chan.ReadChar();
    if (len_part < 0)
      return FALSE;
    data_len = (data_len << 8) | len_part;
  }

  data_len -= len_len + 1;
  if (data_len < 0)
    return FALSE;

  BYTE * ptr = bitData.GetPointer(data_len);
  while (data_len > 0 && chan.Read(ptr, data_len)) {
    PINDEX last_read = chan.GetLastReadCount();
    data_len -= last_read;
    ptr += last_read;
  }

  return data_len == 0;
}


BOOL PPER_BitStream::Write(PChannel & chan)
{
  BYTE buf[5];
  PINDEX size = bitData.GetSize();
  PINDEX len_len = size > 0xfffffb ? 4 : 3;
  PINDEX len = size + len_len + 1;
  buf[0] = (BYTE)len_len;
  while (len_len > 0) {
    buf[len_len] = (BYTE)len;
    len >>= 8;
    len_len--;
  }
  return chan.Write(buf, buf[0]+1) && chan.Write((const BYTE *)bitData, size);
}


BOOL PPER_BitStream::SingleBitDecode()
{
  if ((bitData.GetSize() - byteOffset)*8 - (8 - bitOffset) == 0)
    return FALSE;

  bitOffset--;
  BOOL value = (bitData[byteOffset] & (1 << bitOffset)) != 0;

  if (bitOffset == 0) {
    bitOffset = 8;
    byteOffset++;
  }

  return value;
}


void PPER_BitStream::SingleBitEncode(BOOL value)
{
  bitOffset--;
  if (value)
    bitData[byteOffset] |= 1 << bitOffset;
  if (bitOffset == 0)
    ByteAlign(TRUE);
}


int PPER_BitStream::MultiBitDecode(PINDEX nBits)
{
  PAssert(nBits <= 32, PInvalidParameter);

  PINDEX bitsLeft = (bitData.GetSize() - byteOffset)*8 - (8 - bitOffset);
  if (nBits > bitsLeft)
    nBits = bitsLeft;

  if (nBits == 0)
    return 0;

  if (nBits < bitOffset) {
    bitOffset -= nBits;
    return ((BYTE)bitData[byteOffset] >> bitOffset) & ((1 << nBits) - 1);
  }

  int val = (BYTE)bitData[byteOffset] & ((1 << bitOffset) - 1);
  nBits -= bitOffset;
  bitOffset = 8;
  byteOffset++;

  while (nBits >= 8) {
    val = (val << 8) | (BYTE)bitData[byteOffset];
    byteOffset++;
    nBits -= 8;
  }

  if (nBits > 0) {
    bitOffset = 8 - nBits;
    val = (val << nBits) | ((BYTE)bitData[byteOffset] >> bitOffset);
  }

  return val;
}


void PPER_BitStream::MultiBitEncode(int value, PINDEX nBits)
{
  if (nBits == 0)
    return;

  // Make sure value is in bounds of bit available.
  value &= (1 << nBits) - 1;

  if (nBits < bitOffset) {
    bitOffset -= nBits;
    bitData[byteOffset] |= value << bitOffset;
    return;
  }

  nBits -= bitOffset;
  bitData[byteOffset] |= (BYTE)(value >> nBits);
  bitOffset = 0;
  ByteAlign(TRUE);

  while (nBits >= 8) {
    nBits -= 8;
    bitData[byteOffset] = (BYTE)(value >> nBits);
    bitOffset--; // Do this so ByteAlign expands the bitData array
    ByteAlign(TRUE);
  }

  if (nBits > 0) {
    bitOffset = 8 - nBits;
    bitData[byteOffset] |= (BYTE)((value & ((1 << nBits)-1)) << bitOffset);
  }
}


unsigned PPER_BitStream::SmallUnsignedDecode()
{
  // X.691 Section 10.6

  if (!SingleBitDecode())
    return MultiBitDecode(6);      // 10.6.1

  PINDEX len = LengthDecode(0, INT_MAX);  // 10.6.2
  ByteAlign(FALSE);
  return MultiBitDecode(len*8);
}


void PPER_BitStream::SmallUnsignedEncode(unsigned value)
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
  ByteAlign(TRUE);
  MultiBitEncode(value, len*8);
}


static PINDEX CountBits(unsigned range)
{
  if (range == 0)
    return sizeof(unsigned)*8;

  PINDEX nBits = 0;
  while (nBits < (sizeof(unsigned)*8) && range > (unsigned)(1 << nBits))
    nBits++;
  return nBits;
}


PINDEX PPER_BitStream::LengthDecode(int lower, int upper)
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
    ByteAlign(FALSE);
    return lower + MultiBitDecode(range != 256 ? 16 : 8);
  }

  ByteAlign(FALSE);
  if (SingleBitDecode() == 0)
    return lower + MultiBitDecode(7);   // 10.9.3.6

  PAssert(!SingleBitDecode(), PUnimplementedFunction);  // 10.9.3.8 unsupported
  return lower + MultiBitDecode(14);    // 10.9.3.7
}


void PPER_BitStream::LengthEncode(PINDEX len, int lower, int upper)
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
    ByteAlign(TRUE);
    MultiBitEncode(len, range != 256 ? 16 : 8);
    return;
  }

  ByteAlign(TRUE);
  if (len < 128) {
    MultiBitEncode(len, 8);   // 10.9.3.6
    return;
  }
  SingleBitEncode(TRUE);

  PAssert(len < 0x2000, PUnimplementedFunction);  // 10.9.3.8 unsupported
  MultiBitEncode(len, 15);    // 10.9.3.7
}


int PPER_BitStream::UnsignedDecode(unsigned range)
{
  PINDEX nBits = CountBits(range);

  if (nBits >= 8 && aligned) {      // 10.5.7.1
    if (nBits > 16)                 // not 10.5.7.4
      nBits = LengthDecode(1, (nBits+7)/8)*8;     // 12.2.6
    else if (nBits > 8)             // not 10.5.7.2
      nBits = 16;                   // 10.5.7.3
    ByteAlign(FALSE);               // 10.7.5.2 - 10.7.5.4
  }

  return MultiBitDecode(nBits);
}


void PPER_BitStream::UnsignedEncode(int value, unsigned range)
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
    ByteAlign(TRUE);                // 10.7.5.2 - 10.7.5.4
  }

  MultiBitEncode(value, nBits);
}


PINDEX PPER_BitStream::BlockDecode(BYTE * bufptr, PINDEX nBytes)
{
  ByteAlign(FALSE);

  if (byteOffset+nBytes > bitData.GetSize())
    nBytes = bitData.GetSize() - byteOffset;

  if (nBytes == 0)
    return 0;

  memcpy(bufptr, &bitData[byteOffset], nBytes);
  byteOffset += nBytes;
  return nBytes;
}


void PPER_BitStream::BlockEncode(const BYTE * bufptr, PINDEX nBytes)
{
  ByteAlign(TRUE);

  if (nBytes == 0)
    return;

  memcpy(bitData.GetPointer(byteOffset+nBytes)+byteOffset, bufptr, nBytes);
  byteOffset += nBytes;
}


void PPER_BitStream::SetPosition(PINDEX newPos)
{
  if (newPos > bitData.GetSize())
    byteOffset = bitData.GetSize();
  else
    byteOffset = newPos;
  bitOffset = 8;
}


void PPER_BitStream::ByteAlign(BOOL encoding)
{
  if (bitOffset != 8) {
    bitOffset = 8;
    byteOffset++;
    if (encoding && byteOffset >= bitData.GetSize())
      bitData.SetSize(byteOffset+20);
  }
}


void PPER_BitStream::BeginEncoding()
{
  bitOffset = 8;
  byteOffset = 0;
  bitData.SetSize(20);
  memset(bitData.GetPointer(), 0, 20);
}


void PPER_BitStream::CompleteEncoding()
{
  if (bitOffset != 8) {
    bitOffset = 8;
    byteOffset++;
  }
  bitData.SetSize(byteOffset);
  byteOffset = 0;
}



///////////////////////////////////////////////////////////////////////

PPERObject::PPERObject(BOOL extend)
{
  extendable = extend;
}


BOOL PPERObject::Read(PChannel & chan)
{
  PPER_BitStream strm;
  if (!strm.Read(chan))
    return FALSE;
  Decode(strm);
  return TRUE;
}


BOOL PPERObject::Write(PChannel & chan)
{
  PPER_BitStream strm;
  Encode(strm);
  strm.CompleteEncoding();
  return strm.Write(chan);
}


///////////////////////////////////////////////////////////////////////

PPERConstrainedObject::PPERConstrainedObject()
{
  SetUnconstrained();
}


PPERConstrainedObject::PPERConstrainedObject(int lower, unsigned upper, BOOL extend)
  : PPERObject(extend)
{
  SetLimits(lower, upper);
}


void PPERConstrainedObject::SetUnconstrained()
{
  lowerLimit = 0;
  upperLimit = 0;
  unconstrained = TRUE;
}


void PPERConstrainedObject::SetLimits(int lower, unsigned upper)
{
  PAssert((lower >= 0 || upper < 0x7fffffff) &&
          (lower < 0 || (unsigned)lower <= upper), PInvalidParameter);
  lowerLimit = lower;
  upperLimit = upper;
  unconstrained = FALSE;
}


BOOL PPERConstrainedObject::UnconstrainedDecode(PPER_BitStream & strm)
{
  if (extendable)
    if (strm.SingleBitDecode())
      return TRUE;

  return unconstrained;
}


BOOL PPERConstrainedObject::UnconstrainedEncode(PPER_BitStream & strm, unsigned value)
{
  if (!extendable || unconstrained)
    return unconstrained;

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
  return needsExtending;
}


///////////////////////////////////////////////////////////////////////

PPER_Boolean::PPER_Boolean(BOOL val)
{
  value = val;
}


void PPER_Boolean::Decode(PPER_BitStream & strm)
{
  // X.931 Section 11
  value = strm.SingleBitDecode();
}


void PPER_Boolean::Encode(PPER_BitStream & strm)
{
  // X.931 Section 11
  strm.SingleBitEncode(value);
}


void PPER_Boolean::PrintOn(ostream & strm) const
{
  if (value)
    strm << "TRUE";
  else
    strm << "FALSE";
}


PPER_Boolean & PPER_Boolean::operator=(BOOL val)
{
  value = val;
  return *this;
}


///////////////////////////////////////////////////////////////////////

PPER_Integer::PPER_Integer(unsigned val)
{
  value = val;
}


PPER_Integer::PPER_Integer(int lower, unsigned upper, BOOL extend, unsigned val)
  : PPERConstrainedObject(lower, upper, extend)
{
  operator=(val);
}


PPER_Integer & PPER_Integer::operator=(unsigned val)
{
  if (val > upperLimit)
    value = upperLimit;
  else if (lowerLimit < 0 ? ((int)val < lowerLimit) : (val < (unsigned)lowerLimit))
    value = lowerLimit;
  else
    value = val;
  return *this;
}


void PPER_Integer::Decode(PPER_BitStream & strm)
{
  // X.931 Sections 12

  if (UnconstrainedDecode(strm)) //  12.1
    value = lowerLimit + strm.MultiBitDecode(strm.LengthDecode(0, INT_MAX));
  else if (upperLimit == (unsigned)lowerLimit) // 12.2.1
    value = lowerLimit;
  else // 12.2.2 which devolves to 10.5
    value = lowerLimit + strm.UnsignedDecode((unsigned)(upperLimit - lowerLimit)+1);
}


void PPER_Integer::Encode(PPER_BitStream & strm)
{
  // X.931 Sections 12

  int adjusted_value = value - lowerLimit;

  if (UnconstrainedEncode(strm, value)) { //  12.1
    PINDEX nBits = adjusted_value == 0 ? 8 : CountBits(adjusted_value);
    strm.LengthEncode(nBits, 0, INT_MAX);
    strm.MultiBitEncode(adjusted_value, nBits);
    return;
  }

  if (upperLimit == (unsigned)lowerLimit) // 12.2.1
    return;

  // 12.2.2 which devolves to 10.5
  strm.UnsignedEncode(adjusted_value, (unsigned)(upperLimit - lowerLimit)+1);
}


void PPER_Integer::PrintOn(ostream & strm) const
{
  if (lowerLimit < 0)
    strm << (int)value;
  else
    strm << value;
}


///////////////////////////////////////////////////////////////////////

PPER_Enumeration::PPER_Enumeration(int nEnums,
                                   const PString & nameSpec,
                                   BOOL extend,
                                   int val)
  : PPERObject(extend),
    names(nameSpec.Tokenise(' '))
{
  PAssert(nEnums > 0, PInvalidParameter);
  PAssert(val >= 0 && val < nEnums, PInvalidParameter);
  value = val;
  numEnums = nEnums;
}


void PPER_Enumeration::Decode(PPER_BitStream & strm)
{
  // X.691 Section 13

  if (extendable) {  // 13.3
    if (strm.SingleBitDecode()) {
      value = strm.UnsignedDecode(strm.SmallUnsignedDecode());
      return;
    }
  }

  value = strm.UnsignedDecode(numEnums);  // 13.2
}


void PPER_Enumeration::Encode(PPER_BitStream & strm)
{
  // X.691 Section 13

  if (extendable) {  // 13.3
    BOOL extended = value >= numEnums;
    strm.SingleBitEncode(extended);
    if (extended) {
      strm.SmallUnsignedEncode(value+1);
      strm.UnsignedEncode(value, value);
      return;
    }
  }

  strm.UnsignedEncode(value, numEnums-1);  // 13.2
}


void PPER_Enumeration::PrintOn(ostream & strm) const
{
  if ((PINDEX)value < names.GetSize())
    strm << names[value];
  else
    strm << '<' << value << '>';
}


///////////////////////////////////////////////////////////////////////

PPER_Real::PPER_Real(double val)
{
  value = val;
}


void PPER_Real::Decode(PPER_BitStream & strm)
{
  // X.691 Section 14

  PINDEX len = strm.MultiBitDecode(8)+1;
  // ??? NOT IMPLEMENTED
  strm.SetPosition(strm.GetPosition()+len);
}


void PPER_Real::Encode(PPER_BitStream & strm)
{
  // X.691 Section 14

  strm.MultiBitEncode(0, 8);
  // ??? NOT IMPLEMENTED
  strm.MultiBitEncode(0, 8);
}


void PPER_Real::PrintOn(ostream & strm) const
{
  strm << value;
}


///////////////////////////////////////////////////////////////////////

PPER_BitString::PPER_BitString(int nBits)
  : totalBits(nBits),
    bitData((nBits+7)/8)
{
}


PPER_BitString::PPER_BitString(int lower, int upper, BOOL extend, int nBits)
  : PPERConstrainedObject(lower, upper, extend),
    totalBits(nBits < lower ? lower : (nBits > upper ? upper : nBits)),
    bitData((totalBits+7)/8)
{
  PAssert(lower >= 0, PInvalidParameter);
}


void PPER_BitString::Decode(PPER_BitStream & strm)
{
  // X.691 Section 15

  if (UnconstrainedDecode(strm)) // 15.5
    totalBits = strm.LengthDecode(0, INT_MAX);
  else if (upperLimit == (unsigned)lowerLimit)
    totalBits = upperLimit;
  else {  // 15.10
    totalBits = strm.LengthDecode(lowerLimit, upperLimit);
    strm.ByteAlign(FALSE);
  }

  if (totalBits == 0) {
    bitData.SetSize(0);   // 15.7
    return;
  }

  PINDEX bitsLeft = strm.GetBitsLeft();
  if (totalBits > bitsLeft)
    totalBits = bitsLeft;

  if (totalBits > 16) {
    PINDEX nBytes = (totalBits+7)/8;
    bitData.SetSize(strm.BlockDecode(bitData.GetPointer(nBytes), nBytes));   // 15.9
  }
  else if (totalBits > 8) {  // 15.8
    bitData.SetSize(2);
    bitData[0] = (BYTE)strm.MultiBitDecode(8);
    bitData[1] = (BYTE)(strm.MultiBitDecode(totalBits-8) << (16-totalBits));
  }
  else {
    bitData.SetSize(1);
    bitData[0] = (BYTE)(strm.MultiBitDecode(totalBits) << (8-totalBits));
  }
}


void PPER_BitString::Encode(PPER_BitStream & strm)
{
  // X.691 Section 15

  if (UnconstrainedEncode(strm, totalBits)) // 15.5
    strm.LengthEncode(totalBits, 0, INT_MAX);
  else if (upperLimit != (unsigned)lowerLimit) {  // 15.10
    strm.LengthEncode(totalBits, lowerLimit, upperLimit);
    strm.ByteAlign(TRUE);
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


void PPER_BitString::PrintOn(ostream & strm) const
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


void PPER_BitString::SetData(PINDEX nBits, const PBYTEArray & bytes)
{
  bitData = bytes;
  totalBits = nBits;
}


void PPER_BitString::SetData(PINDEX nBits, const BYTE * buf, PINDEX size)
{
  bitData = PBYTEArray(buf, size);
  totalBits = nBits;
}


BOOL PPER_BitString::SetSize(PINDEX nBits)
{
  totalBits = nBits;
  return bitData.SetSize((nBits+7)/8);
}


BOOL PPER_BitString::operator[](PINDEX bit) const
{
  if (bit < totalBits)
    return (bitData[bit>>3] & (1 << (7 - (bit&7)))) != 0;
  return FALSE;
}


void PPER_BitString::Set(PINDEX bit)
{
  if (bit < totalBits)
    bitData[bit>>3] |= 1 << (7 - (bit&7));
}


void PPER_BitString::Clear(PINDEX bit)
{
  if (bit < totalBits)
    bitData[bit>>3] &= ~(1 << (7 - (bit&7)));
}


void PPER_BitString::Invert(PINDEX bit)
{
  if (bit < totalBits)
    bitData[bit>>3] ^= 1 << (7 - (bit&7));
}


///////////////////////////////////////////////////////////////////////

PPER_OctetString::PPER_OctetString()
{
}


PPER_OctetString::PPER_OctetString(int lower, int upper, BOOL extend)
  : PPERConstrainedObject(lower, upper, extend), value(lower)
{
  PAssert(lower >= 0, PInvalidParameter);
}


void PPER_OctetString::Decode(PPER_BitStream & strm)
{
  // X.691 Section 16

  PINDEX nBytes;
  if (UnconstrainedDecode(strm)) // 16.3
    nBytes = strm.LengthDecode(0, INT_MAX);
  else if (upperLimit != (unsigned)lowerLimit)  // 16.8
    nBytes = strm.LengthDecode(lowerLimit, upperLimit);
  else {
    switch (upperLimit) {
      case 1 :  // 16.6
        value[0] = (BYTE)strm.MultiBitDecode(8);
        return;

      case 2 :  // 16.6
        value.SetSize(2);
        value[0] = (BYTE)strm.MultiBitDecode(8);
        value[1] = (BYTE)strm.MultiBitDecode(8);
        return;

      default: // 16.7
        nBytes = upperLimit;
    }
  }

  if (nBytes == 0)
    value.SetSize(0);   // 16.5
  else
    value.SetSize(strm.BlockDecode(value.GetPointer(nBytes), nBytes));
}


void PPER_OctetString::Encode(PPER_BitStream & strm)
{
  // X.691 Section 16

  PINDEX nBytes = value.GetSize();
  if (UnconstrainedEncode(strm, nBytes)) // 16.3
    strm.LengthEncode(nBytes, 0, INT_MAX);
  else if (upperLimit != (unsigned)lowerLimit)  // 16.8
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


void PPER_OctetString::PrintOn(ostream & strm) const
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


void PPER_OctetString::SetData(const BYTE * data, PINDEX len)
{
  if ((unsigned)len > upperLimit)
    len = upperLimit;
  value.SetSize((int)len < lowerLimit ? lowerLimit : len);
  memcpy(value.GetPointer(), data, len);
}


///////////////////////////////////////////////////////////////////////

PPER_ObjectId::PPER_ObjectId()
{
}


PPER_ObjectId::PPER_ObjectId(const PString & dotstr)
{
  operator=(dotstr);
}


PPER_ObjectId & PPER_ObjectId::operator=(const PString & dotstr)
{
  PStringArray parts = dotstr.Tokenise('.');
  value.SetSize(parts.GetSize());
  for (PINDEX i = 0; i < parts.GetSize(); i++)
    value[i] = parts[i].AsInteger();
  return *this;
}


void PPER_ObjectId::Decode(PPER_BitStream & strm)
{
  // X.691 Section 23

  PINDEX dataLen = strm.LengthDecode(0, 255);
  strm.ByteAlign(FALSE);

  value.SetSize(2);

  // handle zero length strings correctly
  if (dataLen != 0) {
    unsigned subId;

    // start at the second identifier in the buffer, because we will later
    // expand the first number into the first two IDs
    PINDEX i = 1;
    while (dataLen > 0) {
      unsigned byte;
      subId = 0;
      do {    /* shift and add in low order 7 bits */
        byte = strm.MultiBitDecode(8);
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
  }
}


void PPER_ObjectId::Encode(PPER_BitStream & strm)
{
  // X.691 Section 23

  PBYTEArray  eObjId;
  PINDEX      offs = 0;
  unsigned    subId, mask, testmask;
  int         bits, testbits;
  PINDEX      objIdLen = value.GetSize();
  unsigned    *objId = value.GetPointer();

  if (objIdLen < 2) {
    eObjId [offs++] = 0;
    objIdLen = 0;
  } else {
    eObjId [offs++] = (BYTE)(objId[1] + (objId[0] * 40));
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
      for(;mask != 0x7F; mask >>= 7, bits -= 7) {
        /* fix a mask that got truncated above */
	      if (mask == 0x1E00000)
	        mask = 0xFE00000;
        eObjId [offs++] = (u_char)(((subId & mask) >> bits) | 0x80);
      }
      eObjId [offs++] = (u_char)(subId & mask);
    }
  }

  strm.LengthEncode(eObjId.GetSize(), 0, 255);
  strm.BlockEncode(eObjId, eObjId.GetSize());
}


void PPER_ObjectId::PrintOn(ostream & strm) const
{
  for (PINDEX i = 0; i < value.GetSize(); i++) {
    strm << (unsigned)value[i];
    if (i < value.GetSize()-1)
      strm << '.';
  }
}


///////////////////////////////////////////////////////////////////////

PPERConstrainedString::PPERConstrainedString(const char * canonicalSet, PINDEX size,
                                             const PString & set,
                                             int lower, int upper, BOOL extend)
  : PPERConstrainedObject(lower, upper, extend)
{
  PAssert(lower >= 0, PInvalidParameter);
  canonicalSetBits = CountBits(size);

  for (PINDEX i = 0; i < size; i++) {
    if (set.Find(canonicalSet[i]) != P_MAX_INDEX)
      charSet += canonicalSet[i];
  }

  charSetUnalignedBits = CountBits(charSet.GetLength());
  charSetAlignedBits = 1;
  while (charSetUnalignedBits > charSetAlignedBits)
    charSetAlignedBits <<= 1;
}


PPERConstrainedString & PPERConstrainedString::operator=(const PString & str)
{
  value = PString();
  PINDEX len = str.GetLength();
  for (PINDEX i = 0; i < len; i++) {
    if (charSet.Find(str[i]) != P_MAX_INDEX)
      value += str[i];
  }
  return *this;
}


void PPERConstrainedString::Decode(PPER_BitStream & strm)
{
  // X.691 Section 26

  PINDEX len;
  if (UnconstrainedDecode(strm)) // 26.4
    len = strm.LengthDecode(0, INT_MAX);
  else
    len = strm.LengthDecode(lowerLimit, upperLimit);
  value.SetSize(len+1);

  PINDEX nBits = strm.IsAligned() ? charSetAlignedBits : charSetUnalignedBits;

  if (len*nBits > 16) {
    if (nBits == 8) {
      value.SetSize(strm.BlockDecode((BYTE *)value.GetPointer(), len)+1);
      return;
    }
    if (strm.IsAligned())
      strm.ByteAlign(FALSE);
  }

  for (PINDEX i = 0; i < len; i++) {
    if (nBits >= canonicalSetBits && canonicalSetBits > 4)
      value[i] = (char)strm.MultiBitDecode(nBits);
    else
      value[i] = charSet[strm.MultiBitDecode(nBits)];
  }
}


void PPERConstrainedString::Encode(PPER_BitStream & strm)
{
  // X.691 Section 26

  PINDEX len = value.GetSize()-1;
  if (UnconstrainedEncode(strm, len)) // 26.4
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
      strm.ByteAlign(TRUE);
  }

  for (PINDEX i = 0; i < len; i++) {
    if (nBits >= canonicalSetBits && canonicalSetBits > 4)
      strm.MultiBitEncode(value[i], nBits);
    else
      strm.MultiBitEncode(charSet.Find(value[i]), nBits);
  }
}


void PPERConstrainedString::PrintOn(ostream & strm) const
{
  strm << value.ToLiteral();
}



static const char NumericStringSet[] = " 0123456789";

PPER_NumericString::PPER_NumericString()
  : PPERConstrainedString(NumericStringSet, sizeof(NumericStringSet)-1,
                          NumericStringSet, 0, INT_MAX, FALSE)
{
}


PPER_NumericString::PPER_NumericString(int lower, int upper, BOOL extend)
  : PPERConstrainedString(NumericStringSet, sizeof(NumericStringSet)-1,
                          NumericStringSet, lower, upper, extend)
{
}

PPER_NumericString::PPER_NumericString(const PString & set, int lower, int upper, BOOL extend)
  : PPERConstrainedString(NumericStringSet, sizeof(NumericStringSet)-1,
                          set, lower, upper, extend)
{
}



static const char PrintableStringSet[] =
            " '()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

PPER_PrintableString::PPER_PrintableString()
  : PPERConstrainedString(PrintableStringSet, sizeof(PrintableStringSet)-1,
                          PrintableStringSet, 0, INT_MAX, FALSE)
{
}

PPER_PrintableString::PPER_PrintableString(int lower, int upper, BOOL extend)
  : PPERConstrainedString(PrintableStringSet, sizeof(PrintableStringSet)-1,
                          PrintableStringSet, lower, upper, extend)
{
}

PPER_PrintableString::PPER_PrintableString(const PString & set, int lower, int upper, BOOL extend)
  : PPERConstrainedString(PrintableStringSet, sizeof(PrintableStringSet)-1,
                          set, lower, upper, extend)
{
}



static const char VisibleStringSet[] =
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~";

PPER_VisibleString::PPER_VisibleString()
  : PPERConstrainedString(VisibleStringSet, sizeof(VisibleStringSet)-1,
                          VisibleStringSet, 0, INT_MAX, FALSE)
{
}

PPER_VisibleString::PPER_VisibleString(int lower, int upper, BOOL extend)
  : PPERConstrainedString(VisibleStringSet, sizeof(VisibleStringSet)-1,
                          VisibleStringSet, lower, upper, extend)
{
}

PPER_VisibleString::PPER_VisibleString(const PString & set, int lower, int upper, BOOL extend)
  : PPERConstrainedString(VisibleStringSet, sizeof(VisibleStringSet)-1,
                          set, lower, upper, extend)
{
}



static const char IA5StringSet[] =
    "\200\001\002\003\004\005\006\007"
    "\010\011\012\013\014\015\016\017"
    "\020\021\022\023\024\025\026\027"
    "\030\031\032\033\034\035\036\037"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~\177";

PPER_IA5String::PPER_IA5String()
  : PPERConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                          IA5StringSet, 0, INT_MAX, FALSE)
{
}

PPER_IA5String::PPER_IA5String(int lower, int upper, BOOL extend)
  : PPERConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                          IA5StringSet, lower, upper, extend)
{
}

PPER_IA5String::PPER_IA5String(const PString & set, int lower, int upper, BOOL extend)
  : PPERConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                          set, lower, upper, extend)
{
}



PPER_GeneralString::PPER_GeneralString()
  : PPERConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                          IA5StringSet, 0, INT_MAX, FALSE)
{
}

PPER_GeneralString::PPER_GeneralString(int lower, int upper, BOOL extend)
  : PPERConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                          IA5StringSet, lower, upper, extend)
{
}

PPER_GeneralString::PPER_GeneralString(const PString & set, int lower, int upper, BOOL extend)
  : PPERConstrainedString(IA5StringSet, sizeof(IA5StringSet)-1,
                          set, lower, upper, extend)
{
}


///////////////////////////////////////////////////////////////////////

PPER_BMPString::PPER_BMPString()
{
}


PPER_BMPString::PPER_BMPString(int lower, int upper, BOOL extend)
  : PPERConstrainedObject(lower, upper, extend)
{
  PAssert(lower >= 0, PInvalidParameter);
}


void PPER_BMPString::Decode(PPER_BitStream & strm)
{
  // X.691 Section 26

  PINDEX len;
  if (UnconstrainedDecode(strm)) // 26.4
    len = strm.LengthDecode(0, INT_MAX);
  else
    len = strm.LengthDecode(lowerLimit, upperLimit);
  value.SetSize(len);

  if (len == 1)
    value[0] = (WORD)strm.MultiBitDecode(16);
  else {
    strm.ByteAlign(FALSE);
    for (PINDEX i = 0; i < len; i++)
      value[i] = (WORD)strm.MultiBitDecode(16);
  }
}


void PPER_BMPString::Encode(PPER_BitStream & strm)
{
  // X.691 Section 26

  PINDEX len = value.GetSize();
  if (UnconstrainedEncode(strm, len)) // 26.4
    strm.LengthEncode(len, 0, INT_MAX);
  else
    strm.LengthEncode(len, lowerLimit, upperLimit);

  if (len == 1)
    strm.MultiBitEncode(value[0], 16);
  else {
    strm.ByteAlign(TRUE);
    for (PINDEX i = 0; i < len; i++)
      strm.MultiBitEncode(value[i], 16);
  }
}


void PPER_BMPString::PrintOn(ostream & strm) const
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


PPER_BMPString & PPER_BMPString::operator=(const PString & str)
{
  value.SetSize(str.GetLength());
  for (PINDEX i = 0; i < value.GetSize(); i++)
    value[i] = (BYTE)str[i];
  return *this;
}


PPER_BMPString::operator PString() const
{
  PString str;
  for (PINDEX i = 0; i < value.GetSize(); i++) {
    if (value[i] < 256)
      str += PString((char)value[i]);
  }
  return str;
}


///////////////////////////////////////////////////////////////////////

PPER_Sequence::PPER_Sequence(PINDEX nOpts, BOOL extend, PINDEX nExtend)
  : PPERObject(extend), optionMap(nOpts, nOpts)
{
  knownExtensions = nExtend;
  totalExtensions = 0;
}


PPER_Sequence & PPER_Sequence::operator=(const PPER_Sequence & other)
{
  optionMap = other.optionMap;
  knownExtensions = other.knownExtensions;
  totalExtensions = other.totalExtensions;
  extensionMap = other.extensionMap;
  for (PINDEX i = 0; i < other.unknownExtensions.GetSize(); i++)
    unknownExtensions[i] = other.unknownExtensions[i];
  return *this;
}


BOOL PPER_Sequence::HasOptionalField(int opt) const
{
  if (opt < optionMap.GetSize())
    return optionMap[opt];
  else
    return extensionMap[opt - optionMap.GetSize()];
}


void PPER_Sequence::IncludeOptionalField(int opt)
{
  if (opt < optionMap.GetSize())
    optionMap.Set(opt);
  else {
    opt -= optionMap.GetSize();
    if (opt < extensionMap.GetSize())
      extensionMap.Set(opt);
  }
}


void PPER_Sequence::PreambleDecode(PPER_BitStream & strm)
{
  // X.691 Section 18

  if (extendable)
    totalExtensions = strm.SingleBitDecode() ? -1 : 0;  // 18.1
  else
    totalExtensions = 0;
  optionMap.Decode(strm);  // 18.2
}


void PPER_Sequence::PreambleEncode(PPER_BitStream & strm)
{
  // X.691 Section 18

  if (extendable)
    strm.SingleBitEncode(totalExtensions > 0);  // 18.1
  optionMap.Encode(strm);  // 18.2
}


BOOL PPER_Sequence::ExtensionsDecode(PPER_BitStream & strm)
{
  if (totalExtensions == 0)
    return FALSE;

  if (totalExtensions < 0) {
    totalExtensions = strm.SmallUnsignedDecode() + 1;
    extensionMap.SetLimits(totalExtensions, totalExtensions);
    extensionMap.Decode(strm);
  }

  if (knownExtensions > 0)
    return TRUE;

  UnknownExtensionsDecode(strm);
  return FALSE;
}


BOOL PPER_Sequence::ExtensionsEncode(PPER_BitStream & strm)
{
  if (totalExtensions == 0)
    return FALSE;
  PAssert(totalExtensions > 0, PLogicError);

  strm.SmallUnsignedEncode(totalExtensions-1);
  extensionMap.Encode(strm);

  if (knownExtensions > 0)
    return TRUE;

  UnknownExtensionsEncode(strm);
  return FALSE;
}


void PPER_Sequence::KnownExtensionDecode(PPER_BitStream & strm, PINDEX fld, PPERObject & field)
{
  if (totalExtensions == 0)
    return;

  if (!extensionMap[fld-optionMap.GetSize()])
    return;

  PINDEX len = strm.LengthDecode(0, INT_MAX);
  PINDEX nextExtensionPosition = strm.GetPosition() + len;
  field.Decode(strm);
  strm.SetPosition(nextExtensionPosition);
}


void PPER_Sequence::KnownExtensionEncode(PPER_BitStream & strm, PINDEX fld, PPERObject & field)
{
  if (totalExtensions == 0)
    return;

  if (!extensionMap[fld-optionMap.GetSize()])
    return;

  PPER_BitStream substream;
  field.Encode(substream);
  substream.CompleteEncoding();
  strm.LengthEncode(substream.GetSize(), 0, INT_MAX);
  strm.BlockEncode(substream.GetPointer(), substream.GetSize());
}


void PPER_Sequence::UnknownExtensionsDecode(PPER_BitStream & strm)
{
  if (totalExtensions == 0)
    return;

  if (totalExtensions < 0)
    if (!ExtensionsDecode(strm))
      return;

  PINDEX unknownCount = totalExtensions - knownExtensions;
  if (unknownExtensions.GetSize() >= unknownCount)
    return;  // Already read them

  unknownExtensions.SetSize(unknownCount);

  PINDEX i;
  for (i = 0; i < unknownExtensions.GetSize(); i++)
    unknownExtensions.SetAt(i, PNEW PPER_OctetString);

  for (i = knownExtensions; i < extensionMap.GetSize(); i++) {
    if (extensionMap[i])
      unknownExtensions[i-knownExtensions].Decode(strm);
  }
}


void PPER_Sequence::UnknownExtensionsEncode(PPER_BitStream & strm)
{
  if (totalExtensions == 0)
    return;

  PINDEX i;
  for (i = 0; i < unknownExtensions.GetSize(); i++) {
    if (extensionMap[i + knownExtensions])
      unknownExtensions[i].Encode(strm);
  }
}


///////////////////////////////////////////////////////////////////////

PPER_Choice::PPER_Choice(int upper, BOOL extend)
  : PPERObject(extend)
{
  maxChoices = upper;
  discriminator = -1;
  choice = NULL;
}


PPER_Choice::PPER_Choice(int upper, BOOL extend, const PString & nameSpec)
  : PPERObject(extend),
    names(nameSpec.Tokenise(' '))
{
  maxChoices = upper;
  discriminator = -1;
  choice = NULL;
}


PPER_Choice & PPER_Choice::operator=(const PPER_Choice & other)
{
  delete choice;

  maxChoices = other.maxChoices;
  discriminator = other.discriminator;

  CreateObject();

  if (choice != NULL && other.choice != NULL)
    *choice = *other.choice;
  return *this;
}


PPER_Choice::~PPER_Choice()
{
  delete choice;
}


void PPER_Choice::Decode(PPER_BitStream & strm)
{
  // X.691 Section 22
  delete choice;

  if (extendable) {
    if (strm.SingleBitDecode()) {
      discriminator = strm.SmallUnsignedDecode() + maxChoices;
      PINDEX len = strm.LengthDecode(0, INT_MAX);
      CreateObject();
      if (choice == NULL) {
        PPER_OctetString * open_type = PNEW PPER_OctetString(len, len, FALSE);
        open_type->Decode(strm);
        if (open_type->GetSize() > 0)
          choice = open_type;
        else
          delete open_type;
      }
      else {
        PINDEX nextPos = strm.GetPosition() + len;
        choice->Decode(strm);
        strm.SetPosition(nextPos);
      }
      return;
    }
  }

  if (maxChoices > 1)
    discriminator = strm.UnsignedDecode(maxChoices);
  else
    discriminator = 0;

  CreateObject();

  if (choice != NULL)
    choice->Decode(strm);
}


void PPER_Choice::Encode(PPER_BitStream & strm)
{
  if (extendable) {
    BOOL extended = discriminator >= maxChoices;
    strm.SingleBitEncode(extended);
    if (extended) {
      strm.SmallUnsignedEncode(discriminator - maxChoices);
      if (choice == NULL)
        strm.LengthEncode(0, 0, INT_MAX);
      else {
        PPER_BitStream substream;
        choice->Encode(substream);
        substream.CompleteEncoding();
        strm.LengthEncode(substream.GetSize(), 0, INT_MAX);
        strm.BlockEncode(substream.GetPointer(), substream.GetSize());
      }
      return;
    }
  }

  if (maxChoices > 1)
    strm.UnsignedEncode(discriminator, maxChoices);

  if (choice != NULL)
    choice->Encode(strm);
}


void PPER_Choice::PrintOn(ostream & strm) const
{
  if (discriminator < 0)
    strm << "<uninitialised>";
  else if (discriminator < names.GetSize())
    strm << names[discriminator];
  else
    strm << '<' << discriminator << '>';
  if (choice != NULL)
    strm << ' ' << *choice;
  else
    strm << " (NULL)";
}


void PPER_Choice::SetChoice(int newChoice)
{
  discriminator = newChoice;
  delete choice;
  CreateObject();
}


PPERObject & PPER_Choice::GetObject() const
{
  PAssert(choice != NULL, "NULL Choice");
  return *choice;
}


///////////////////////////////////////////////////////////////////////

PPER_Array::PPER_Array()
{
}


PPER_Array::PPER_Array(int lower, int upper, BOOL extend)
  : PPERConstrainedObject(lower, upper, extend)
{
  PAssert(lower >= 0, PInvalidParameter);
}


PPER_Array & PPER_Array::operator=(const PPER_Array & other)
{
  for (PINDEX i = 0; i < other.array.GetSize(); i++)
    array[i] = other.array[i];
  return *this;
}


void PPER_Array::Decode(PPER_BitStream & strm)
{
  PINDEX size;
  if (UnconstrainedDecode(strm))
    size = strm.LengthDecode(0, INT_MAX);
  else
    size = strm.LengthDecode(lowerLimit, upperLimit);
  array.SetSize(size);

  for (PINDEX i = 0; i < size; i++) {
    PPERObject * element = CreateObject();
	  element->Decode(strm);
    array.SetAt(i, element);
  }
}


void PPER_Array::Encode(PPER_BitStream & strm)
{
  PINDEX size = array.GetSize();
  if (UnconstrainedEncode(strm, size))
    strm.LengthEncode(size, 0, INT_MAX);
  else
    strm.LengthEncode(size, lowerLimit, upperLimit);

  for (PINDEX i = 0; i < size; i++)
	  array[i].Encode(strm);
}


void PPER_Array::PrintOn(ostream & strm) const
{
  int indent = strm.precision()+2;
  strm << setprecision(indent) << "{\n";
  for (PINDEX i = 0; i < array.GetSize(); i++)
    strm << setw(indent+1) << '[' << i << "]=" << array[i] << '\n';
  strm << setw(indent-1) << '}'
       << setprecision(indent-2);
}


void PPER_Array::SetSize(PINDEX newSize)
{
  PINDEX originalSize = array.GetSize();
  array.SetSize(newSize);
  for (PINDEX i = originalSize; i < newSize; i++)
    array.SetAt(i, CreateObject());
}


// PERASN.CXX
