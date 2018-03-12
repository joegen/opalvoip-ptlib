/*
 * guid.cxx
 *
 * Globally Unique Identifier
 *
 * Open H323 Library
 *
 * Copyright (c) 1998-2001 Equivalence Pty. Ltd.
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
 * The Original Code is Open H323 Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 */

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "guid.h"
#endif

#include <ptclib/guid.h>

#if P_GUID

#include <ptlib/sockets.h>
#include <ptclib/random.h>

#if P_ASN
#include <ptclib/asner.h>
#endif


#define new PNEW


static atomic<bool> s_haveMacAddress;
static PEthSocket::Address s_macAddress;


///////////////////////////////////////////////////////////////////////////////

PGloballyUniqueID::PGloballyUniqueID()
  : PBYTEArray(Size)
{
  static PMutex mutex(PDebugLocation(__FILE__, __LINE__, "GUID"));
  PWaitAndSignal wait(mutex);

  // Want time of UTC in 0.1 microseconds since 15 Oct 1582.
  static PTimeInterval const delta(0, 0, 0, 0,
                                   16              // Days from 15th October
                                 + 31              // Days in December 1583
                                 + 30              // Days in November 1583
                                 + (1970-1583)*365 // Days in years
                                 + (1970-1583)/4   // Leap days
                                 - 3);             // Allow for 1700, 1800, 1900 not leap years
  int64_t timestamp = (PTime() + delta).GetTimestamp()*10;

  theArray[0] = (BYTE)(timestamp&0xff);
  theArray[1] = (BYTE)((timestamp>>8)&0xff);
  theArray[2] = (BYTE)((timestamp>>16)&0xff);
  theArray[3] = (BYTE)((timestamp>>24)&0xff);
  theArray[4] = (BYTE)((timestamp>>32)&0xff);
  theArray[5] = (BYTE)((timestamp>>40)&0xff);
  theArray[6] = (BYTE)((timestamp>>48)&0xff);
  theArray[7] = (BYTE)(((timestamp>>56)&0x0f) + 0x10);  // Version number is 1

  static WORD clockSequence = (WORD)PRandom::Number();
  static PInt64 lastTimestamp = 0;
  if (lastTimestamp < timestamp)
    lastTimestamp = timestamp;
  else
    clockSequence++;

  theArray[8] = (BYTE)(((clockSequence>>8)&0x1f) | 0x80); // DCE compatible GUID
  theArray[9] = (BYTE)clockSequence;

  if (!s_haveMacAddress.exchange(true)) {
    PString str = PIPSocket::GetInterfaceMACAddress();
    if (str.IsEmpty()) {
      PRandom::Octets(s_macAddress.b, sizeof(s_macAddress.b));
      s_macAddress.b[0] |= '\x80';
    }
    else
      s_macAddress = str;
  }

  memcpy(theArray+10, s_macAddress.b, 6);
}


PGloballyUniqueID::PGloballyUniqueID(const char * cstr)
  : PBYTEArray(Size)
{
  if (cstr != NULL && *cstr != '\0') {
    PStringStream strm(cstr);
    ReadFrom(strm);
  }
}


PGloballyUniqueID::PGloballyUniqueID(const PString & str)
  : PBYTEArray(Size)
{
  PStringStream strm(str);
  ReadFrom(strm);
}


PGloballyUniqueID::PGloballyUniqueID(const void * data, PINDEX size)
  : PBYTEArray(Size)
{
  memcpy(theArray, PAssertNULL(data), PMIN(size, GetSize()));
}


#if P_ASN
PGloballyUniqueID::PGloballyUniqueID(const PASN_OctetString & newId)
  : PBYTEArray(newId)
{
  PAssert(GetSize() == Size, PInvalidParameter);
  PBYTEArray::SetSize(Size);
}
#endif


#ifdef GUID_DEFINED
PGloballyUniqueID::PGloballyUniqueID(const GUID & guid)
  : PBYTEArray(reinterpret_cast<const BYTE *>(&guid), sizeof(GUID))
{
}
#endif

PObject * PGloballyUniqueID::Clone() const
{
  PAssert(GetSize() == Size, "PGloballyUniqueID is invalid size");

  return new PGloballyUniqueID(*this);
}


PINDEX PGloballyUniqueID::HashFunction() const
{
  PAssert(GetSize() == Size, "PGloballyUniqueID is invalid size");

  static PINDEX NumBuckets = 53; // Should be prime number

#if P_64BIT
  uint64_t * qwords = (uint64_t *)theArray;
  return (qwords[0] ^ qwords[1]) % NumBuckets;
#else
  uint32_t * dwords = (uint32_t *)theArray;
  return (dwords[0] ^ dwords[1] ^ dwords[2] ^ dwords[3]) % NumBuckets;
#endif
}


void PGloballyUniqueID::PrintOn(ostream & strm) const
{
  PAssert(GetSize() == Size, "PGloballyUniqueID is invalid size");

  char fillchar = strm.fill();
  strm << hex << setfill('0')
       << setw(2) << (unsigned)(BYTE)theArray[0]
       << setw(2) << (unsigned)(BYTE)theArray[1]
       << setw(2) << (unsigned)(BYTE)theArray[2]
       << setw(2) << (unsigned)(BYTE)theArray[3] << '-'
       << setw(2) << (unsigned)(BYTE)theArray[4]
       << setw(2) << (unsigned)(BYTE)theArray[5] << '-'
       << setw(2) << (unsigned)(BYTE)theArray[6]
       << setw(2) << (unsigned)(BYTE)theArray[7] << '-'
       << setw(2) << (unsigned)(BYTE)theArray[8]
       << setw(2) << (unsigned)(BYTE)theArray[9] << '-'
       << setw(2) << (unsigned)(BYTE)theArray[10]
       << setw(2) << (unsigned)(BYTE)theArray[11]
       << setw(2) << (unsigned)(BYTE)theArray[12]
       << setw(2) << (unsigned)(BYTE)theArray[13]
       << setw(2) << (unsigned)(BYTE)theArray[14]
       << setw(2) << (unsigned)(BYTE)theArray[15]
       << dec << setfill(fillchar);
}


void PGloballyUniqueID::ReadFrom(istream & strm)
{
  PAssert(GetSize() == Size, "PGloballyUniqueID is invalid size");
  PBYTEArray::SetSize(Size);

  strm >> ws;

  PINDEX count = 0;

  while (count < 2*Size) {
    if (isxdigit(strm.peek())) {
      char digit = (char)(strm.get() - '0');
      if (digit >= 10) {
        digit -= 'A'-('9'+1);
        if (digit >= 16)
          digit -= 'a'-'A';
      }
      theArray[count/2] = (BYTE)((theArray[count/2] << 4) | digit);
      count++;
    }
    else if (strm.peek() == '-') {
      if (count != 8 && count != 12 && count != 16 && count != 20)
        break;
      strm.get(); // Ignore the dash if it was in the right place
    }
    else if (strm.peek() == ' ') 
      strm.get(); // Ignore spaces
    else
      break;
  }

  if (count < 2*Size) {
    memset(theArray, 0, Size);
    strm.clear(ios::failbit);
  }
}


PString PGloballyUniqueID::AsString() const
{
  PStringStream strm;
  PrintOn(strm);
  return strm;
}


PBoolean PGloballyUniqueID::IsNULL() const
{
  PAssert(GetSize() == Size, "PGloballyUniqueID is invalid size");

  return memcmp(theArray, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", Size) == 0;
}


#endif // P_GUID


/////////////////////////////////////////////////////////////////////////////
