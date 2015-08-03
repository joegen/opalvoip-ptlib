/*
 * cypher.cxx
 *
 * Encryption support classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "cypher.h"
#endif

#define P_DISABLE_FACTORY_INSTANCES

#include <ptlib.h>
#include <ptclib/cypher.h>

#if P_CYPHER

#include <ptclib/mime.h>
#include <ptclib/random.h>



///////////////////////////////////////////////////////////////////////////////
// PSASLString

void PSASLString::AppendValidated(const char * str)
{
  PWCharArray wide(PConstString(str).AsUCS2());
  for (PINDEX i = 0; i < wide.GetSize(); ++i)
    AppendValidated(wide[i]);
}


void PSASLString::AppendValidated(wchar_t c)
{
  // Strictly speaking, control characters are simply "illegal" in RFC4013

  // RFC3454/C.1.2 ASCII control characters
  if (c < ' ')
    return;

  // RFC3454/C.2.2 Non-ASCII control characters
  if (c >= 0x80 && c <= 0x9f)
    return;

  // RFC3454/C.3 Private use
  if (c >= 0xE000 && c <= 0xF8FF)
    return;

  // RFC3454/C.4 Non-character code points
  if (c >= 0xFDD0 && c <= 0xFDEF)
    return;

  // RFC3454/C.5 Surrogate codes
  if (c >= 0xD800 && c <= 0xDFFF)
    return;

  // RFC3454/C.7 Inappropriate for canonical representation
  if (c >= 0x2FF0 && c <= 0x2FFB)
    return;

  switch (c) {
    // RFC3454/C.1.2 Non-ASCII space characters
    case 0x00A0: // NO - BREAK SPACE
    case 0x1680: // OGHAM SPACE MARK
    case 0x2000: // EN QUAD
    case 0x2001: // EM QUAD
    case 0x2002: // EN SPACE
    case 0x2003: // EM SPACE
    case 0x2004: // THREE - PER - EM SPACE
    case 0x2005: // FOUR - PER - EM SPACE
    case 0x2006: // SIX - PER - EM SPACE
    case 0x2007: // FIGURE SPACE
    case 0x2008: // PUNCTUATION SPACE
    case 0x2009: // THIN SPACE
    case 0x200A: // HAIR SPACE
    case 0x200B: // ZERO WIDTH SPACE
    case 0x202F: // NARROW NO - BREAK SPACE
    case 0x205F: // MEDIUM MATHEMATICAL SPACE
    case 0x3000: // IDEOGRAPHIC SPACE
      PString::operator+=((char)c);
      break;

    // RFC3454/C.1.2 ASCII control characters
    case 0x007F:
    // RFC3454/C.2.2 Non-ASCII control characters
    case 0x06DD: // ARABIC END OF AYAH
    case 0x070F: // SYRIAC ABBREVIATION MARK
    case 0x180E: // MONGOLIAN VOWEL SEPARATOR
    case 0x200C: // ZERO WIDTH NON - JOINER
    case 0x200D: // ZERO WIDTH JOINER
    case 0x2028: // LINE SEPARATOR
    case 0x2029: // PARAGRAPH SEPARATOR
    case 0x2060: // WORD JOINER
    case 0x2061: // FUNCTION APPLICATION
    case 0x2062: // INVISIBLE TIMES
    case 0x2063: // INVISIBLE SEPARATOR
    case 0xFEFF: // ZERO WIDTH NO - BREAK SPACE
    // RFC3454/C.4 Non-character code points
    case 0xFFFE:
    case 0xFFFF:
    // RFC3454/C.6 Inappropriate for plain text
    case 0xFFF9: // INTERLINEAR ANNOTATION ANCHOR
    case 0xFFFA: // INTERLINEAR ANNOTATION SEPARATOR
    case 0xFFFB: // INTERLINEAR ANNOTATION TERMINATOR
    case 0xFFFC: // OBJECT REPLACEMENT CHARACTER
    case 0xFFFD: // REPLACEMENT CHARACTER
    // RFC3454/C.8 Change display properties or are deprecated
    case 0x0340: // COMBINING GRAVE TONE MARK
    case 0x0341: // COMBINING ACUTE TONE MARK
    case 0x200E: // LEFT - TO - RIGHT MARK
    case 0x200F: // RIGHT - TO - LEFT MARK
    case 0x202A: // LEFT - TO - RIGHT EMBEDDING
    case 0x202B: // RIGHT - TO - LEFT EMBEDDING
    case 0x202C: // POP DIRECTIONAL FORMATTING
    case 0x202D: // LEFT - TO - RIGHT OVERRIDE
    case 0x202E: // RIGHT - TO - LEFT OVERRIDE
    case 0x206A: // INHIBIT SYMMETRIC SWAPPING
    case 0x206B: // ACTIVATE SYMMETRIC SWAPPING
    case 0x206C: // INHIBIT ARABIC FORM SHAPING
    case 0x206D: // ACTIVATE ARABIC FORM SHAPING
    case 0x206E: // NATIONAL DIGIT SHAPES
    case 0x206F: // NOMINAL DIGIT SHAPES
      break;

    default:
      PString::operator+=((char)c);
  }
}


///////////////////////////////////////////////////////////////////////////////
// PBase64

PBase64::PBase64()
{
  StartEncoding();
  StartDecoding();
}


void PBase64::StartEncoding(bool useCRLF, PINDEX width)
{
  encodedString.MakeEmpty();
  currentLineLength = saveCount = 0;
  endOfLine = useCRLF ? "\r\n" : "\n";
  maxLineLength = width - (useCRLF ? 1 : 0);
}


void PBase64::StartEncoding(const char * eol, PINDEX width)
{
  encodedString.MakeEmpty();
  currentLineLength = saveCount = 0;
  endOfLine = eol;
  maxLineLength = width - endOfLine.GetLength();
}


void PBase64::ProcessEncoding(const PString & str)
{
  ProcessEncoding((const char *)str);
}


void PBase64::ProcessEncoding(const char * cstr)
{
  ProcessEncoding((const BYTE *)cstr, (int)strlen(cstr));
}


void PBase64::ProcessEncoding(const PBYTEArray & data)
{
  ProcessEncoding(data, data.GetSize());
}


static const char Binary2Base64[65] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void PBase64::OutputBase64(const BYTE * data)
{
  encodedString.SetMinSize(((encodedString.GetLength()+7)&~255) + 256);

  encodedString += Binary2Base64[data[0] >> 2];
  encodedString += Binary2Base64[((data[0]&3)<<4) | (data[1]>>4)];
  encodedString += Binary2Base64[((data[1]&15)<<2) | (data[2]>>6)];
  encodedString += Binary2Base64[data[2]&0x3f];

  currentLineLength += 4;
  if (currentLineLength >= maxLineLength) {
    encodedString += endOfLine;
    currentLineLength = 0;
  }
}


void PBase64::ProcessEncoding(const void * dataPtr, PINDEX length)
{
  if (length == 0)
    return;

  const BYTE * data = (const BYTE *)dataPtr;
  while (saveCount < 3) {
    saveTriple[saveCount++] = *data++;
    if (--length == 0) {
      if (saveCount == 3) {
        OutputBase64(saveTriple);
        saveCount = 0;
      }
      return;
    }
  }

  OutputBase64(saveTriple);

  PINDEX i;
  for (i = 0; i+2 < length; i += 3)
    OutputBase64(data+i);

  saveCount = length - i;
  switch (saveCount) {
    case 2 :
      saveTriple[0] = data[i++];
      saveTriple[1] = data[i];
      break;
    case 1 :
      saveTriple[0] = data[i];
  }
}


PString PBase64::GetEncodedString()
{
  PString retval = encodedString;
  encodedString.MakeEmpty();
  return retval;
}


PString PBase64::CompleteEncoding()
{
  encodedString.SetMinSize(encodedString.GetLength() + 5);

  switch (saveCount) {
    case 1 :
      encodedString += Binary2Base64[saveTriple[0] >> 2];
      encodedString += Binary2Base64[(saveTriple[0]&3)<<4];
      encodedString += '=';
      encodedString += '=';
      break;

    case 2 :
      encodedString += Binary2Base64[saveTriple[0] >> 2];
      encodedString += Binary2Base64[((saveTriple[0]&3)<<4) | (saveTriple[1]>>4)];
      encodedString += Binary2Base64[((saveTriple[1]&15)<<2)];
      encodedString += '=';
  }

  return encodedString;
}


PString PBase64::Encode(const PString & str, const char * endOfLine, PINDEX width)
{
  if (str.IsEmpty())
    return PString::Empty();

  return Encode((const char *)str, str.GetLength(), endOfLine, width);
}


PString PBase64::Encode(const char * cstr, const char * endOfLine, PINDEX width)
{
  if (cstr == NULL || *cstr == '\0')
    return PString::Empty();

  return Encode((const BYTE *)cstr, (PINDEX)strlen(cstr), endOfLine, width);
}


PString PBase64::Encode(const PBYTEArray & data, const char * endOfLine, PINDEX width)
{
  if (data.IsEmpty())
    return PString::Empty();

  return Encode(data, data.GetSize(), endOfLine, width);
}


PString PBase64::Encode(const void * data, PINDEX length, const char * endOfLine, PINDEX width)
{
  if (length == 0)
    return PString::Empty();

  PBase64 encoder;
  encoder.StartEncoding(endOfLine, width);
  encoder.ProcessEncoding(data, length);
  return encoder.CompleteEncoding();
}


void PBase64::StartDecoding()
{
  perfectDecode = true;
  quadPosition = 0;
  decodedData.SetSize(0);
  decodeSize = 0;
}


PBoolean PBase64::ProcessDecoding(const PString & str)
{
  return ProcessDecoding((const char *)str);
}


PBoolean PBase64::ProcessDecoding(const char * cstr)
{
  static const BYTE Base642Binary[256] = {
    96, 99, 99, 99, 99, 99, 99, 99, 99, 99, 98, 99, 99, 98, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 99, 99, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 97, 99, 99,
    99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99,
    99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
  };

  for (;;) {
    BYTE value = Base642Binary[(BYTE)*cstr++];
    switch (value) {
      case 96 : // end of string
        return false;

      case 97 : // '=' sign
        if (quadPosition == 3 || (quadPosition == 2 && *cstr == '=')) {
          quadPosition = 0;  // Reset this to zero, as have a perfect decode
          return true; // Stop decoding now as must be at end of data
        }
        perfectDecode = false;  // Ignore '=' sign but flag decode as suspect
        break;

      case 98 : // CRLFs
        break;  // Ignore totally

      case 99 :  // Illegal characters
        perfectDecode = false;  // Ignore rubbish but flag decode as suspect
        break;

      default : // legal value from 0 to 63
        BYTE * out = decodedData.GetPointer(((decodeSize+1)&~255) + 256);
        switch (quadPosition) {
          case 0 :
            out[decodeSize] = (BYTE)(value << 2);
            break;
          case 1 :
            out[decodeSize++] |= (BYTE)(value >> 4);
            out[decodeSize] = (BYTE)((value&15) << 4);
            break;
          case 2 :
            out[decodeSize++] |= (BYTE)(value >> 2);
            out[decodeSize] = (BYTE)((value&3) << 6);
            break;
          case 3 :
            out[decodeSize++] |= (BYTE)value;
            break;
        }
        quadPosition = (quadPosition+1)&3;
    }
  }
}


PBYTEArray PBase64::GetDecodedData()
{
  perfectDecode = quadPosition == 0;
  decodedData.SetSize(decodeSize);
  PBYTEArray retval = decodedData;
  retval.MakeUnique();
  decodedData.SetSize(0);
  decodeSize = 0;
  return retval;
}


PBoolean PBase64::GetDecodedData(void * dataBlock, PINDEX length)
{
  perfectDecode = quadPosition == 0;
  PBoolean bigEnough = length >= decodeSize;
  memcpy(dataBlock, decodedData, bigEnough ? decodeSize : length);
  decodedData.SetSize(0);
  decodeSize = 0;
  return bigEnough;
}


PString PBase64::Decode(const PString & str)
{
  if (str.IsEmpty())
    return str;

  PBYTEArray data;
  Decode(str, data);
  return PString(data);
}


PBoolean PBase64::Decode(const PString & str, PBYTEArray & data)
{
  if (str.IsEmpty())
    return false;

  PBase64 decoder;
  decoder.ProcessDecoding(str);
  data = decoder.GetDecodedData();
  return decoder.IsDecodeOK();
}


PBoolean PBase64::Decode(const PString & str, void * dataBlock, PINDEX length)
{
  if (str.IsEmpty())
    return false;

  PBase64 decoder;
  decoder.ProcessDecoding(str);
  return decoder.GetDecodedData(dataBlock, length);
}


///////////////////////////////////////////////////////////////////////////////
// PMessageDigest

PMessageDigest::PMessageDigest()
{
}

void PMessageDigest::Process(const PString & str)
{
  Process((const char *)str);
}


void PMessageDigest::Process(const char * cstr)
{
  Process(cstr, (int)strlen(cstr));
}


void PMessageDigest::Process(const PBYTEArray & data)
{
  Process(data, data.GetSize());
}

void PMessageDigest::Process(const void * dataBlock, PINDEX length)
{
  InternalProcess(dataBlock, length);
}

PString PMessageDigest::CompleteDigest()
{
  Result result;
  CompleteDigest(result);
  return result.AsBase64();
}

void PMessageDigest::CompleteDigest(Result & result)
{
  InternalCompleteDigest(result);
}


void PMessageDigest::Result::PrintOn(ostream & strm) const
{
  if ((strm.flags()&ios::basefield) == ios::dec)
    strm << AsBase64();
  else {
    char oldFill = strm.fill();
    strm.fill('0');
    for (PINDEX i = 0; i < GetSize(); ++i)
      strm << setw(2) << (unsigned)GetAt(i);
    strm.fill(oldFill);
  }
}


PString PMessageDigest::Result::AsHex() const
{
  PStringStream strm;
  strm << hex << *this;
  return strm;
}


///////////////////////////////////////////////////////////////////////////////
// PMessageDigest5

PMessageDigest5::PMessageDigest5()
{
  Start();
}


// Constants for MD5Transform routine.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

// F, G, H and I are basic MD5 functions.
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
#define FF(a, b, c, d, x, s, ac) \
 (a) += F ((b), (c), (d)) + (x) + (DWORD)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \

#define GG(a, b, c, d, x, s, ac) \
 (a) += G ((b), (c), (d)) + (x) + (DWORD)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \

#define HH(a, b, c, d, x, s, ac) \
 (a) += H ((b), (c), (d)) + (x) + (DWORD)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \

#define II(a, b, c, d, x, s, ac) \
 (a) += I ((b), (c), (d)) + (x) + (DWORD)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \


void PMessageDigest5::Transform(const BYTE * block)
{
  DWORD a = state[0];
  DWORD b = state[1];
  DWORD c = state[2];
  DWORD d = state[3];

  DWORD x[16];
  for (PINDEX i = 0; i < 16; i++)
    x[i] = ((PUInt32l*)block)[i];

  /* Round 1 */
  FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG(d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH(b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  // Zeroize sensitive information.
  memset(x, 0, sizeof(x));
}


void PMessageDigest5::Start()
{
  // Load magic initialization constants.
  state[0] = 0x67452301;
  state[1] = 0xefcdab89;
  state[2] = 0x98badcfe;
  state[3] = 0x10325476;

  count = 0;
}

void PMessageDigest5::InternalProcess(const void * dataPtr, PINDEX length)
{
  const BYTE * data = (const BYTE *)dataPtr;

  // Compute number of bytes mod 64
  PINDEX index = (PINDEX)((count >> 3) & 0x3F);
  PINDEX partLen = 64 - index;

  // Update number of bits
  count += (PUInt64)length << 3;

  // See if have a buffer full
  PINDEX i;
  if (length < partLen)
    i = 0;
  else {
    // Transform as many times as possible.
    memcpy(&buffer[index], data, partLen);
    Transform(buffer);
    for (i = partLen; i + 63 < length; i += 64)
      Transform(&data[i]);
    index = 0;
  }

  // Buffer remaining input
  memcpy(&buffer[index], &data[i], length-i);
}


void PMessageDigest5::InternalCompleteDigest(Result & result)
{
  // Put the count into bytes platform independently
  PUInt64l countBytes = count;

  // Pad out to 56 mod 64.
  PINDEX index = (PINDEX)((count >> 3) & 0x3f);
  PINDEX padLen = (index < 56) ? (56 - index) : (120 - index);
  static BYTE const padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  Process(padding, padLen);

  // Append length
  Process(&countBytes, sizeof(countBytes));

  // Store state in digest
  PUInt32l * valuep = (PUInt32l *)result.GetPointer(4 * sizeof(PUInt32l));
  for (PINDEX i = 0; i < PARRAYSIZE(state); i++)
    valuep[i] = state[i];

  // Zeroize sensitive information.
  memset(buffer, 0, sizeof(buffer));
  memset(state, 0, sizeof(state));
}


PString PMessageDigest5::Encode(const PString & str)
{
  return Encode((const char *)str);
}


void PMessageDigest5::Encode(const PString & str, Result & result)
{
  Encode((const char *)str, result);
}


PString PMessageDigest5::Encode(const char * cstr)
{
  return Encode((const BYTE *)cstr, (int)strlen(cstr));
}


void PMessageDigest5::Encode(const char * cstr, Result & result)
{
  Encode((const BYTE *)cstr, (int)strlen(cstr), result);
}


PString PMessageDigest5::Encode(const PBYTEArray & data)
{
  return Encode(data, data.GetSize());
}


void PMessageDigest5::Encode(const PBYTEArray & data, Result & result)
{
  Encode(data, data.GetSize(), result);
}


PString PMessageDigest5::Encode(const void * data, PINDEX length)
{
  Result result;
  Encode(data, length, result);
  return result.AsBase64();
}


void PMessageDigest5::Encode(const void * data, PINDEX len, Result & result)
{
  PMessageDigest5 stomach;
  stomach.Process(data, len);
  stomach.CompleteDigest(result);
}

////  backwards compatability functions

void PMessageDigest5::Encode(const PString & str, Code & result)
{
  Encode((const char *)str, result);
}

void PMessageDigest5::Encode(const char * cstr, Code & result)
{
  Encode((const BYTE *)cstr, (int)strlen(cstr), result);
}

void PMessageDigest5::Encode(const PBYTEArray & data, Code & result)
{
  Encode(data, data.GetSize(), result);
}

void PMessageDigest5::Encode(const void * data, PINDEX len, Code & codeResult)
{
  PMessageDigest5 stomach;
  stomach.Process(data, len);
  stomach.Complete(codeResult);
}

PString PMessageDigest5::Complete()
{
  Code result;
  Complete(result);
  return PBase64::Encode(&result, sizeof(result));
}

void PMessageDigest5::Complete(Code & codeResult)
{
  Result result;
  InternalCompleteDigest(result);
  memcpy(codeResult.value, result.GetPointer(), sizeof(codeResult.value));
}

///////////////////////////////////////////////////////////////////////////////
// PMessageDigestSHA1

#if P_SSL

#include <openssl/sha.h>

#ifdef _MSC_VER

#pragma comment(lib, P_SSL_LIB1)
#pragma comment(lib, P_SSL_LIB2)

#endif


PMessageDigestSHA1::PMessageDigestSHA1()
{
  shaContext = NULL;
  Start();
}

PMessageDigestSHA1::~PMessageDigestSHA1()
{
  delete (SHA_CTX *)shaContext;
}

void PMessageDigestSHA1::Start()
{
  delete (SHA_CTX *)shaContext;
  shaContext = new SHA_CTX;

  SHA1_Init((SHA_CTX *)shaContext);
}

void PMessageDigestSHA1::InternalProcess(const void * data, PINDEX len)
{
  if (shaContext == NULL)
    return;

  SHA1_Update((SHA_CTX *)shaContext, data, (unsigned long)len);
}

void PMessageDigestSHA1::InternalCompleteDigest(Result & result)
{
  if (shaContext == NULL)
    return;

  SHA1_Final(result.GetPointer(DigestLength), (SHA_CTX *)shaContext);
  delete ((SHA_CTX *)shaContext);
  shaContext = NULL;
}


PString PMessageDigestSHA1::Encode(const PString & str)
{
  return Encode((const char *)str);
}


void PMessageDigestSHA1::Encode(const PString & str, Result & result)
{
  Encode((const char *)str, result);
}


PString PMessageDigestSHA1::Encode(const char * cstr)
{
  return Encode((const BYTE *)cstr, strlen(cstr));
}


void PMessageDigestSHA1::Encode(const char * cstr, Result & result)
{
  Encode((const BYTE *)cstr, strlen(cstr), result);
}


PString PMessageDigestSHA1::Encode(const PBYTEArray & data)
{
  return Encode(data, data.GetSize());
}


void PMessageDigestSHA1::Encode(const PBYTEArray & data, Result & result)
{
  Encode(data, data.GetSize(), result);
}


PString PMessageDigestSHA1::Encode(const void * data, PINDEX length)
{
  Result result;
  Encode(data, length, result);
  return result.AsBase64();
}


void PMessageDigestSHA1::Encode(const void * data, PINDEX len, Result & result)
{
  PMessageDigestSHA1 stomach;
  stomach.Process(data, len);
  stomach.CompleteDigest(result);
}

#endif

///////////////////////////////////////////////////////////////////////////////
// PCypher

PCypher::PCypher(PINDEX blkSize, BlockChainMode mode)
  : blockSize(blkSize),
    chainMode(mode)
{
}


PCypher::PCypher(const void * keyData, PINDEX keyLength,
                 PINDEX blkSize, BlockChainMode mode)
  : key((const BYTE *)keyData, keyLength),
    blockSize(blkSize),
    chainMode(mode)
{
}


PString PCypher::Encode(const PString & str)
{
  return Encode((const char *)str, str.GetLength());
}


PString PCypher::Encode(const PBYTEArray & clear)
{
  return Encode((const BYTE *)clear, clear.GetSize());
}


PString PCypher::Encode(const void * data, PINDEX length)
{
  PBYTEArray coded;
  Encode(data, length, coded);
  return PBase64::Encode(coded);
}


void PCypher::Encode(const PBYTEArray & clear, PBYTEArray & coded)
{
  Encode((const BYTE *)clear, clear.GetSize(), coded);
}


void PCypher::Encode(const void * data, PINDEX length, PBYTEArray & coded)
{
  PAssert((blockSize%8) == 0, PUnsupportedFeature);

  Initialise(true);

  const BYTE * in = (const BYTE *)data;
  BYTE * out = coded.GetPointer(
                      blockSize > 1 ? (length/blockSize+1)*blockSize : length);

  while (length >= blockSize) {
    EncodeBlock(in, out);
    in += blockSize;
    out += blockSize;
    length -= blockSize;
  }

  if (blockSize > 1) {
    PBYTEArray extra(blockSize);
    PINDEX i;
    for (i = 0; i < length; i++)
      extra[i] = *in++;
    PTime now;
    PRandom rand((DWORD)now.GetTimestamp());
    for (; i < blockSize-1; i++)
      extra[i] = (BYTE)rand.Generate();
    extra[blockSize-1] = (BYTE)length;
    EncodeBlock(extra, out);
  }
}


PString PCypher::Decode(const PString & cypher)
{
  PString clear;
  if (Decode(cypher, clear))
    return clear;
  return PString();
}


PBoolean PCypher::Decode(const PString & cypher, PString & clear)
{
  clear = PString();

  PBYTEArray clearText;
  if (!Decode(cypher, clearText))
    return false;

  if (clearText.IsEmpty())
    return true;

  PINDEX sz = clearText.GetSize();
  memcpy(clear.GetPointerAndSetLength(sz), (const BYTE *)clearText, sz);
  return true;
}


PBoolean PCypher::Decode(const PString & cypher, PBYTEArray & clear)
{
  PBYTEArray coded;
  if (!PBase64::Decode(cypher, coded))
    return false;
  return Decode(coded, clear);
}


PINDEX PCypher::Decode(const PString & cypher, void * data, PINDEX length)
{
  PBYTEArray coded;
  PBase64::Decode(cypher, coded);
  PBYTEArray clear;
  if (!Decode(coded, clear))
    return 0;
  memcpy(data, clear, PMIN(length, clear.GetSize()));
  return clear.GetSize();
}


PINDEX PCypher::Decode(const PBYTEArray & coded, void * data, PINDEX length)
{
  PBYTEArray clear;
  if (!Decode(coded, clear))
    return 0;
  memcpy(data, clear, PMIN(length, clear.GetSize()));
  return clear.GetSize();
}


PBoolean PCypher::Decode(const PBYTEArray & coded, PBYTEArray & clear)
{
  PAssert((blockSize%8) == 0, PUnsupportedFeature);
  if (coded.IsEmpty() || (coded.GetSize()%blockSize) != 0)
    return false;

  Initialise(false);

  const BYTE * in = coded;
  PINDEX length = coded.GetSize();
  BYTE * out = clear.GetPointer(length);

  for (PINDEX count = 0; count < length; count += blockSize) {
    DecodeBlock(in, out);
    in += blockSize;
    out += blockSize;
  }

  if (blockSize != 1) {
    if (*--out >= blockSize)
      return false;
    clear.SetSize(length - blockSize + *out);
  }

  return true;
}



///////////////////////////////////////////////////////////////////////////////
// PTEACypher

PTEACypher::PTEACypher(BlockChainMode chainMode)
  : PCypher(8, chainMode)
{
  GenerateKey(*(Key*)key.GetPointer(sizeof(Key)));
}


PTEACypher::PTEACypher(const Key & keyData, BlockChainMode chainMode)
  : PCypher(&keyData, sizeof(Key), 8, chainMode)
{
}


void PTEACypher::SetKey(const Key & newKey)
{
  memcpy(key.GetPointer(sizeof(Key)), &newKey, sizeof(Key));
}


void PTEACypher::GetKey(Key & newKey) const
{
  memcpy(&newKey, key, sizeof(Key));
}


void PTEACypher::GenerateKey(Key & newKey)
{
  static PRandom rand; //=1 // Explicitly set seed if need known random sequence
  for (size_t i = 0; i < sizeof(Key); i++)
    newKey.value[i] = (BYTE)rand;
}


static const DWORD TEADelta = 0x9e3779b9;    // Magic number for key schedule

void PTEACypher::Initialise(PBoolean)
{
  k0 = ((const PUInt32l *)(const BYTE *)key)[0];
  k1 = ((const PUInt32l *)(const BYTE *)key)[1];
  k2 = ((const PUInt32l *)(const BYTE *)key)[2];
  k3 = ((const PUInt32l *)(const BYTE *)key)[3];
}


void PTEACypher::EncodeBlock(const void * in, void * out)
{
  DWORD y = ((PUInt32b*)in)[0];
  DWORD z = ((PUInt32b*)in)[1];
  DWORD sum = 0;
  for (PINDEX count = 32; count > 0; count--) {
    sum += TEADelta;    // Magic number for key schedule
    y += ((z<<4)+k0) ^ (z+sum) ^ ((z>>5)+k1);
    z += ((y<<4)+k2) ^ (y+sum) ^ ((y>>5)+k3);   /* end cycle */
  }
  ((PUInt32b*)out)[0] = y;
  ((PUInt32b*)out)[1] = z;
}


void PTEACypher::DecodeBlock(const void * in, void * out)
{
  DWORD y = ((PUInt32b*)in)[0];
  DWORD z = ((PUInt32b*)in)[1];
  DWORD sum = TEADelta<<5;
  for (PINDEX count = 32; count > 0; count--) {
    z -= ((y<<4)+k2) ^ (y+sum) ^ ((y>>5)+k3); 
    y -= ((z<<4)+k0) ^ (z+sum) ^ ((z>>5)+k1);
    sum -= TEADelta;    // Magic number for key schedule
  }
  ((PUInt32b*)out)[0] = y;
  ((PUInt32b*)out)[1] = z;
}


///////////////////////////////////////////////////////////////////////////////
//
// PHMAC
//

void PHMAC::Initialise(const BYTE * key, PINDEX len)
{
  // ensure the key is at least one block long and pad out if necessary
  if (len < GetB())
    memcpy(m_key.GetPointer(len), key,  len);
  else if (len > GetB()) {
    Result result;
    Hash((const BYTE *)m_key, m_key.GetSize(), result);
    m_key = result;
  }
}

void PHMAC::InternalProcess(const BYTE * data, PINDEX len, PHMAC::Result & result)
{
  PINDEX i, l;
  BYTE * s;
  BYTE * d;

  // construct key XOR ipad. This will always
  PBYTEArray buffer(GetB() + len);
  s = m_key.GetPointer();
  l = m_key.GetSize();
  d = buffer.GetPointer();
  for (i = 0; i < l; ++i)
    *d++ = 0x36 ^ *s++;
  for (;i < (PINDEX)GetB(); ++i)
    *d++ = 0x36;

  // append text
  memcpy(d, data, len);

  // hash 
  Result hash1;
  Hash(buffer, buffer.GetSize(), hash1);

  // create key XOR opad
  buffer.SetSize(GetB() + hash1.GetSize());
  s = m_key.GetPointer();
  l = m_key.GetSize();
  d = buffer.GetPointer();
  for (i = 0; i < l; ++i)
    *d++ = 0x5c ^ *s++;
  for (;i < (PINDEX)GetB(); ++i)
    *d++ = 0x5c;

  // append hash
  memcpy(d, hash1.GetPointer(), hash1.GetSize());

  // hash 
  Hash(buffer.GetPointer(), buffer.GetSize(), result);
}


PString PHMAC::Encode(const BYTE * data, PINDEX len) { Result result; InternalProcess(data, len, result);            return result.AsBase64(); }
PString PHMAC::Encode(const PBYTEArray & data)       { Result result; InternalProcess(data, data.GetSize(), result); return result.AsBase64(); }
PString PHMAC::Encode(const PString & str)           { Result result; InternalProcess(str, str.GetLength(), result); return result.AsBase64(); }

void PHMAC::Process(const BYTE * data, PINDEX len, PHMAC::Result & result)   { InternalProcess(data, len, result); }
void PHMAC::Process(const PBYTEArray & data, PHMAC::Result & result)         { InternalProcess(data, data.GetSize(), result); }
void PHMAC::Process(const PString & str, PHMAC::Result & result)             { InternalProcess(str, str.GetLength(), result); }

///////////////////////////////////////////////////////////////////////////////
// PSecureConfig

#ifdef P_CONFIG_FILE

static const char DefaultSecuredOptions[] = "Secured Options";
static const char DefaultSecurityKey[] = "Validation";
static const char DefaultExpiryDateKey[] = "Expiry Date";
static const char DefaultOptionBitsKey[] = "Option Bits";
static const char DefaultPendingPrefix[] = "Pending:";

PSecureConfig::PSecureConfig(const PTEACypher::Key & prodKey,
                             const PStringArray & secKeys,
                             Source src)
  : PConfig(PString(DefaultSecuredOptions), src),
    securedKeys(secKeys),
    securityKey(DefaultSecurityKey),
    expiryDateKey(DefaultExpiryDateKey),
    optionBitsKey(DefaultOptionBitsKey),
    pendingPrefix(DefaultPendingPrefix)
{
  productKey = prodKey;
}


PSecureConfig::PSecureConfig(const PTEACypher::Key & prodKey,
                             const char * const * secKeys,
                             PINDEX count,
                             Source src)
  : PConfig(PString(DefaultSecuredOptions), src),
    securedKeys(count, secKeys),
    securityKey(DefaultSecurityKey),
    expiryDateKey(DefaultExpiryDateKey),
    optionBitsKey(DefaultOptionBitsKey),
    pendingPrefix(DefaultPendingPrefix)
{
  productKey = prodKey;
}


void PSecureConfig::GetProductKey(PTEACypher::Key & prodKey) const
{
  prodKey = productKey;
}


PSecureConfig::ValidationState PSecureConfig::GetValidation() const
{
  PString str;
  PBoolean allEmpty = true;
  PMessageDigest5 digestor;
  for (PINDEX i = 0; i < securedKeys.GetSize(); i++) {
    str = GetString(securedKeys[i]);
    if (!str.IsEmpty()) {
      digestor.Process(str.Trim());
      allEmpty = false;
    }
  }
  str = GetString(expiryDateKey);
  if (!str.IsEmpty()) {
    digestor.Process(str);
    allEmpty = false;
  }
  str = GetString(optionBitsKey);
  if (!str.IsEmpty()) {
    digestor.Process(str);
    allEmpty = false;
  }

  PString vkey = GetString(securityKey);
  if (allEmpty)
    return (!vkey || GetBoolean(pendingPrefix + securityKey)) ? Pending : Defaults;

  PMessageDigest5::Code code;
  digestor.Complete(code);

  if (vkey.IsEmpty())
    return Invalid;

  BYTE info[sizeof(code)+1+sizeof(DWORD)];
  PTEACypher crypt(productKey);
  if (crypt.Decode(vkey, info, sizeof(info)) != sizeof(info))
    return Invalid;

  if (memcmp(info, &code, sizeof(code)) != 0)
    return Invalid;

  PTime now;
  if (now > GetTime(expiryDateKey))
    return Expired;

  return IsValid;
}


PBoolean PSecureConfig::ValidatePending()
{
  if (GetValidation() != Pending)
    return false;

  PString vkey = GetString(securityKey);
  if (vkey.IsEmpty())
    return true;

  PMessageDigest5::Code code;
  BYTE info[sizeof(code)+1+sizeof(DWORD)];
  PTEACypher crypt(productKey);
  if (crypt.Decode(vkey, info, sizeof(info)) != sizeof(info))
    return false;

  PTime expiryDate(0, 0, 0,
            1, info[sizeof(code)]&15, (info[sizeof(code)]>>4)+1996, PTime::GMT);
  PString expiry = expiryDate.AsString("d MMME yyyy", PTime::GMT);

  // This is for alignment problems on processors that care about such things
  PUInt32b opt;
  void * dst = &opt;
  void * src = &info[sizeof(code)+1];
  memcpy(dst, src, sizeof(opt));
  PString options(PString::Unsigned, (DWORD)opt);

  PMessageDigest5 digestor;
  PINDEX i;
  for (i = 0; i < securedKeys.GetSize(); i++)
    digestor.Process(GetString(pendingPrefix + securedKeys[i]).Trim());
  digestor.Process(expiry);
  digestor.Process(options);
  digestor.Complete(code);

  if (memcmp(info, &code, sizeof(code)) != 0)
    return false;

  SetString(expiryDateKey, expiry);
  SetString(optionBitsKey, options);

  for (i = 0; i < securedKeys.GetSize(); i++) {
    PString str = GetString(pendingPrefix + securedKeys[i]);
    if (!str.IsEmpty())
      SetString(securedKeys[i], str);
    DeleteKey(pendingPrefix + securedKeys[i]);
  }
  DeleteKey(pendingPrefix + securityKey);

  return true;
}


void PSecureConfig::ResetPending()
{
  if (GetBoolean(pendingPrefix + securityKey)) {
    for (PINDEX i = 0; i < securedKeys.GetSize(); i++)
      DeleteKey(securedKeys[i]);
  }
  else {
    SetBoolean(pendingPrefix + securityKey, true);

    for (PINDEX i = 0; i < securedKeys.GetSize(); i++) {
      PString str = GetString(securedKeys[i]);
      if (!str.IsEmpty())
        SetString(pendingPrefix + securedKeys[i], str);
      DeleteKey(securedKeys[i]);
    }
  }
  DeleteKey(expiryDateKey);
  DeleteKey(optionBitsKey);
}

#endif // P_CONFIG_FILE

#endif // P_CYPHER

///////////////////////////////////////////////////////////////////////////////
