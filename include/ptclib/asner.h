/*
 * asner.h
 *
 * Abstract Syntax Notation Encoding Rules classes
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
 * $Log: asner.h,v $
 * Revision 1.6  1998/09/23 06:19:21  robertj
 * Added open source copyright license.
 *
 * Revision 1.5  1998/05/21 04:26:53  robertj
 * Fixed numerous PER problems.
 *
 * Revision 1.4  1998/05/07 05:19:28  robertj
 * Fixed problems with using copy constructor/assignment oeprator on PASN_Objects.
 *
 * Revision 1.3  1997/12/18 05:08:13  robertj
 * Added function to get choice discriminator name.
 *
 * Revision 1.2  1997/12/11 10:35:42  robertj
 * Support for new ASN file parser.
 *
 */

#ifndef _ASNER_H
#define _ASNER_H

#ifdef __GNUC__
#pragma interface
#endif


class PASN_Stream;
class PBER_Stream;
class PPER_Stream;


/////////////////////////////////////////////////////////////////////////////

class PASN_Object : public PObject
{
    PCLASSINFO(PASN_Object, PObject)
  public:
    virtual PString GetTypeAsString() const = 0;
    /* Return a string giving the type of the object */

    PINDEX GetObjectLength() const;
    virtual PINDEX GetDataLength() const = 0;
    virtual BOOL IsPrimitive() const { return TRUE; }

    virtual BOOL Decode(PASN_Stream &) = 0;
    virtual void Encode(PASN_Stream &) const = 0;

    BOOL IsExtendable() const { return extendable; }
    void SetExtendable(BOOL ext = TRUE) { extendable = ext; }

    enum TagClass {
      UniversalTagClass,
      ApplicationTagClass,
      ContextSpecificTagClass,
      PrivateTagClass,
      DefaultTagClass
    };
    TagClass GetTagClass() const { return tagClass; }

    enum UniversalTags {
      InvalidUniversalTag,
      UniversalBoolean,
      UniversalInteger,
      UniversalBitString,
      UniversalOctetString,
      UniversalNull,
      UniversalObjectId,
      UniversalObjectDescriptor,
      UniversalExternalType,
      UniversalReal,
      UniversalEnumeration,
      UniversalEmbeddedPDV,
      UniversalSequence = 16,
      UniversalSet,
      UniversalNumericString,
      UniversalPrintableString,
      UniversalTeletexString,
      UniversalVideotexString,
      UniversalIA5String,
      UniversalUTCTime,
      UniversalGeneralisedTime,
      UniversalGraphicString,
      UniversalVisibleString,
      UniversalGeneralString,
      UniversalUniversalString,
      UniversalBMPString = 30
    };

    unsigned GetTag() const  { return tag; }
    virtual void SetTag(unsigned newTag, TagClass tagClass = DefaultTagClass);

    enum ConstraintType {
      Unconstrained,
      PartiallyConstrained,
      FixedConstraint,
      ExtendableConstraint
    };

    enum MinimumValueTag { MinimumValue = INT_MIN };
    enum MaximumValueTag { MaximumValue = INT_MAX };
    void SetConstraints(ConstraintType type, int lower, MaximumValueTag upper);
    void SetConstraints(ConstraintType type, MinimumValueTag lower, unsigned upper);
    void SetConstraints(ConstraintType type, MinimumValueTag lower, MaximumValueTag upper);
    virtual void SetConstraints(ConstraintType type, int lower = 0, unsigned upper = UINT_MAX);
    virtual void SetCharacterSet(ConstraintType ctype, const char * charSet);
    virtual void SetCharacterSet(ConstraintType ctype, unsigned firstChar, unsigned lastChar);

  protected:
    PASN_Object(unsigned tag, TagClass tagClass, BOOL extend = FALSE);

    BOOL extendable;   // PER extension capability
    TagClass tagClass; // BER tag class
    unsigned tag;      // ASN object tag
};


class PASN_ConstrainedObject : public PASN_Object
{
    PCLASSINFO(PASN_ConstrainedObject, PASN_Object)
  public:
    virtual void SetConstraints(ConstraintType type, int lower = 0, unsigned upper = UINT_MAX);
    BOOL IsConstrained() const { return constraint != Unconstrained; }
    int GetLowerLimit() const { return lowerLimit; }
    unsigned GetUpperLimit() const { return upperLimit; }

    int ConstrainedLengthDecode(PPER_Stream & strm, unsigned & length);
    void ConstrainedLengthEncode(PPER_Stream & strm, unsigned length) const;

    BOOL ConstraintEncode(PPER_Stream & strm, unsigned value) const;

  protected:
    PASN_ConstrainedObject(unsigned tag, TagClass tagClass);

    ConstraintType constraint;
    int lowerLimit;
    unsigned upperLimit;
};


class PASN_Null : public PASN_Object
{
    PCLASSINFO(PASN_Null, PASN_Object)
  public:
    PASN_Null(unsigned tag = UniversalNull,
              TagClass tagClass = UniversalTagClass);

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;
};


class PASN_Boolean : public PASN_Object
{
    PCLASSINFO(PASN_Boolean, PASN_Object)
  public:
    PASN_Boolean(BOOL val = FALSE,
                 unsigned tag = UniversalBoolean,
                 TagClass tagClass = UniversalTagClass);

    PASN_Boolean & operator=(BOOL v) { value = v; return *this; }
    operator BOOL() const { return value; }
    BOOL GetValue() const { return value; }
    void SetValue(BOOL v) { value = v; }

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

  protected:
    BOOL value;
};


class PASN_Integer : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_Integer, PASN_ConstrainedObject)
  public:
    PASN_Integer(unsigned tag = UniversalInteger,
                 TagClass tagClass = UniversalTagClass,
                 unsigned val = 0);

    PASN_Integer & operator=(unsigned value);
    operator unsigned() const { return value; }
    unsigned GetValue() const { return value; }
    void SetValue(unsigned v) { operator=(v); }

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    unsigned value;
};


class PASN_Enumeration : public PASN_Object
{
    PCLASSINFO(PASN_Enumeration, PASN_Object)
  public:
    PASN_Enumeration(unsigned tag = UniversalEnumeration,
                     TagClass tagClass = UniversalTagClass,
                     unsigned nEnums = P_MAX_INDEX,
                     BOOL extendable = FALSE,
                     unsigned val = 0);
    PASN_Enumeration(unsigned tag,
                     TagClass tagClass,
                     unsigned nEnums,
                     BOOL extendable,
                     const PString & nameSpec,
                     unsigned val = 0);

    PASN_Enumeration & operator=(unsigned v) { value = v; return *this; }
    operator unsigned() const { return value; }
    unsigned GetValue() const { return value; }
    void SetValue(unsigned v) { value = v; }

    unsigned GetMaximum() const { return maxEnumValue; }

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    unsigned maxEnumValue;
    unsigned value;
    POrdinalToString names;
};


class PASN_Real : public PASN_Object
{
    PCLASSINFO(PASN_Real, PASN_Object)
  public:
    PASN_Real(unsigned tag = UniversalEnumeration,
              TagClass tagClass = UniversalTagClass,
              double val = 0);

    PASN_Real & operator=(double val) { value = val; return *this; }
    operator double() const { return value; }
    double GetValue() const { return value; }
    void SetValue(double v) { value = v; }

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

  protected:
    double value;
};


class PASN_ObjectId : public PASN_Object
{
    PCLASSINFO(PASN_ObjectId, PASN_Object)
  public:
    PASN_ObjectId(unsigned tag = UniversalObjectId,
                  TagClass tagClass = UniversalTagClass);

    PASN_ObjectId(const PASN_ObjectId & other);
    PASN_ObjectId & operator=(const PASN_ObjectId & other);

    PASN_ObjectId & operator=(const char * dotstr);
    PASN_ObjectId & operator=(const PString & dotstr);
    void SetValue(const PString & dotstr);

    BOOL operator==(const char * dotstr) const;
    BOOL operator!=(const char * dotstr) const { return !operator==(dotstr); }
    BOOL operator==(const PASN_ObjectId & id) const { return value == id.value; }

    PINDEX GetSize() const { return value.GetSize(); }
    const PUnsignedArray & GetValue() const { return value; }
    unsigned operator[](PINDEX idx) const { return value[idx]; }

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL CommonDecode(PASN_Stream & strm, unsigned dataLen);
    void CommonEncode(PBYTEArray & eObjId) const;

  protected:
    PUnsignedArray value;
};


class PASN_BitString : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_BitString, PASN_ConstrainedObject)
  public:
    PASN_BitString(unsigned tag = UniversalBitString,
                   TagClass tagClass = UniversalTagClass,
                   unsigned nBits = 0);

    PASN_BitString(const PASN_BitString & other);
    PASN_BitString & operator=(const PASN_BitString & other);

    void SetData(unsigned nBits, const PBYTEArray & bytes);
    void SetData(unsigned nBits, const BYTE * buf, PINDEX size);

    unsigned GetSize() const { return totalBits; }
    BOOL SetSize(unsigned nBits);

    BOOL operator[](PINDEX bit) const;
    void Set(unsigned bit);
    void Clear(unsigned bit);
    void Invert(unsigned bit);

    virtual void SetConstraints(ConstraintType type, int lower = 0, unsigned upper = UINT_MAX);

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodeBER(PBER_Stream & strm, unsigned len);
    void EncodeBER(PBER_Stream & strm) const;
    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    unsigned totalBits;
    PBYTEArray bitData;
};


class PASN_OctetString : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_OctetString, PASN_ConstrainedObject)
  public:
    PASN_OctetString(unsigned tag = UniversalOctetString,
                     TagClass tagClass = UniversalTagClass);

    PASN_OctetString(const PASN_OctetString & other);
    PASN_OctetString & operator=(const PASN_OctetString & other);

    PASN_OctetString & operator=(const char * str);
    PASN_OctetString & operator=(const PString & str);
    PASN_OctetString & operator=(const PBYTEArray & arr);
    void SetValue(const char * str) { operator=(str); }
    void SetValue(const PString & str) { operator=(str); }
    void SetValue(const PBYTEArray & arr) { operator=(arr); }
    void SetValue(const BYTE * data, PINDEX len);
    const PBYTEArray & GetValue() const { return value; }
    operator const PBYTEArray &() const { return value; }
    operator const BYTE *() const { return value; }
    PString AsString() const;
    BYTE operator[](PINDEX i) const { return value[i]; }
    BYTE & operator[](PINDEX i) { return value[i]; }
    BYTE * GetPointer(PINDEX sz = 0) { return value.GetPointer(sz); }
    PINDEX GetSize() const { return value.GetSize(); }
    BOOL SetSize(PINDEX newSize) { return value.SetSize(newSize); }

    virtual void SetConstraints(ConstraintType type, int lower = 0, unsigned upper = UINT_MAX);

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    PBYTEArray value;
};


class PASN_ConstrainedString : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_ConstrainedString, PASN_ConstrainedObject)
  public:
    PASN_ConstrainedString & operator=(const char * str);
    PASN_ConstrainedString & operator=(const PString & str) { return operator=((const char *)str); }
    operator const PString &() const { return value; }
    const PString & GetValue() const { return value; }
    void SetValue(const PString & v) { operator=(v); }

    void SetCharacterSet(ConstraintType ctype, const char * charSet);
    void SetCharacterSet(ConstraintType ctype, unsigned firstChar = 0, unsigned lastChar = 255);
    void SetCharacterSet(const char * charSet, PINDEX size, ConstraintType ctype);

    virtual void PrintOn(ostream & strm) const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodeBER(PBER_Stream & strm, unsigned len);
    void EncodeBER(PBER_Stream & strm) const;
    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    PASN_ConstrainedString(const char * canonicalSet, PINDEX setSize,
                           unsigned tag, TagClass tagClass);

    PString value;
    PCharArray characterSet;
    const char * canonicalSet;
    PINDEX canonicalSetSize;
    unsigned canonicalSetBits;
    unsigned charSetUnalignedBits;
    unsigned charSetAlignedBits;
};


#define DECLARE_STRING_CLASS(name) \
  PDECLARE_CLASS(PASN_##name##String, PASN_ConstrainedString) \
    public: \
      PASN_##name##String(unsigned tag = UniversalNumericString, \
                          TagClass tagClass = UniversalTagClass); \
      PASN_##name##String & operator=(const char * str); \
      PASN_##name##String & operator=(const PString & str); \
      virtual PObject * Clone() const; \
      virtual PString GetTypeAsString() const; \
  }

DECLARE_STRING_CLASS(Numeric);
DECLARE_STRING_CLASS(Printable);
DECLARE_STRING_CLASS(Visible);
DECLARE_STRING_CLASS(IA5);
DECLARE_STRING_CLASS(General);


class PASN_BMPString : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_BMPString, PASN_ConstrainedObject)
  public:
    PASN_BMPString(unsigned tag = UniversalBMPString,
                   TagClass tagClass = UniversalTagClass);

    PASN_BMPString(const PASN_BMPString & other);
    PASN_BMPString & operator=(const PASN_BMPString & other);

    PASN_BMPString & operator=(const PString & v);
    PASN_BMPString & operator=(const PWORDArray & v);
    operator PString() const { return GetValue(); }
    PString GetValue() const;
    void SetValue(const PString & v) { operator=(v); }
    void SetValue(const PWORDArray & v) { operator=(v); }

    void SetCharacterSet(ConstraintType ctype, const char * charSet);
    void SetCharacterSet(ConstraintType ctype, const PWORDArray & charSet);
    void SetCharacterSet(ConstraintType ctype, unsigned firstChar, unsigned lastChar);

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodeBER(PBER_Stream & strm, unsigned len);
    void EncodeBER(PBER_Stream & strm) const;
    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    BOOL IsLegalCharacter(WORD ch);

    PWORDArray value;
    PWORDArray characterSet;
    WORD firstChar, lastChar;
    unsigned charSetUnalignedBits;
    unsigned charSetAlignedBits;
};


class PASN_Sequence;

class PASN_Choice : public PASN_Object
{
    PCLASSINFO(PASN_Choice, PASN_Object)
  public:
    ~PASN_Choice();

    virtual void SetTag(unsigned newTag, TagClass tagClass = DefaultTagClass);
    PString GetTagName() const;
    PASN_Object & GetObject() const;
    BOOL IsValid() const { return choice != NULL; }

    operator PASN_Null &() const;
    operator PASN_Boolean &() const;
    operator PASN_Integer &() const;
    operator PASN_Enumeration &() const;
    operator PASN_Real &() const;
    operator PASN_ObjectId &() const;
    operator PASN_BitString &() const;
    operator PASN_OctetString &() const;
    operator PASN_NumericString &() const;
    operator PASN_PrintableString &() const;
    operator PASN_VisibleString &() const;
    operator PASN_IA5String &() const;
    operator PASN_GeneralString &() const;
    operator PASN_BMPString &() const;
    operator PASN_Sequence &() const;

    virtual BOOL CreateObject() = 0;

    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL IsPrimitive() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    virtual BOOL DecodePER(PPER_Stream &);
    virtual void EncodePER(PPER_Stream &) const;

  protected:
    PASN_Choice(unsigned nChoices = UINT_MAX, BOOL extend = FALSE);
    PASN_Choice(unsigned tag, TagClass tagClass, unsigned nChoices, BOOL extend);
    PASN_Choice(unsigned tag, TagClass tagClass, unsigned nChoices, BOOL extend, const PString & nameSpec);

    PASN_Choice(const PASN_Choice & other);
    PASN_Choice & operator=(const PASN_Choice & other);

    unsigned numChoices;
    PASN_Object * choice;
    POrdinalToString names;
};


PARRAY(PASN_ObjectArray, PASN_Object);


class PASN_Sequence : public PASN_Object
{
    PCLASSINFO(PASN_Sequence, PASN_Object)
  public:
    PASN_Sequence(unsigned tag = UniversalSequence,
                  TagClass tagClass = UniversalTagClass,
                  unsigned nOpts = 0, BOOL extend = FALSE, unsigned nExtend = 0);

    PASN_Sequence(const PASN_Sequence & other);
    PASN_Sequence & operator=(const PASN_Sequence & other);

    PINDEX GetSize() const { return fields.GetSize(); }
    void SetSize(PINDEX newSize);
    PASN_Object & operator[](PINDEX i) const { return fields[i]; }

    BOOL HasOptionalField(PINDEX opt) const;
    void IncludeOptionalField(PINDEX opt);

    virtual PObject * Clone() const;
    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL IsPrimitive() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL PreambleDecode(PASN_Stream & strm);
    void PreambleEncode(PASN_Stream & strm) const;
    BOOL KnownExtensionDecode(PASN_Stream & strm, PINDEX fld, PASN_Object & field);
    void KnownExtensionEncode(PASN_Stream & strm, PINDEX fld, const PASN_Object & field) const;
    BOOL UnknownExtensionsDecode(PASN_Stream & strm);
    void UnknownExtensionsEncode(PASN_Stream & strm) const;

    BOOL PreambleDecodeBER(PBER_Stream & strm);
    void PreambleEncodeBER(PBER_Stream & strm) const;
    BOOL KnownExtensionDecodeBER(PBER_Stream & strm, PINDEX fld, PASN_Object & field);
    void KnownExtensionEncodeBER(PBER_Stream & strm, PINDEX fld, const PASN_Object & field) const;
    BOOL UnknownExtensionsDecodeBER(PBER_Stream & strm);
    void UnknownExtensionsEncodeBER(PBER_Stream & strm) const;

    BOOL PreambleDecodePER(PPER_Stream & strm);
    void PreambleEncodePER(PPER_Stream & strm) const;
    BOOL KnownExtensionDecodePER(PPER_Stream & strm, PINDEX fld, PASN_Object & field);
    void KnownExtensionEncodePER(PPER_Stream & strm, PINDEX fld, const PASN_Object & field) const;
    BOOL UnknownExtensionsDecodePER(PPER_Stream & strm);
    void UnknownExtensionsEncodePER(PPER_Stream & strm) const;

  protected:
    BOOL NoExtensionsToDecode(PPER_Stream & strm);
    BOOL NoExtensionsToEncode(PPER_Stream & strm);

    PASN_ObjectArray fields;
    PASN_BitString optionMap;
    int knownExtensions;
    int totalExtensions;
    PASN_BitString extensionMap;
    PINDEX endBasicEncoding;
};


class PASN_Set : public PASN_Sequence
{
    PCLASSINFO(PASN_Set, PASN_Sequence)
  public:
    PASN_Set(unsigned tag = UniversalSet,
             TagClass tagClass = UniversalTagClass,
             unsigned nOpts = 0, BOOL extend = FALSE, unsigned nExtend = 0);

    virtual PObject * Clone() const;
    virtual PString GetTypeAsString() const;
};


class PASN_Array : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_Array, PASN_ConstrainedObject)
  public:
    PINDEX GetSize() const { return array.GetSize(); }
    void SetSize(PINDEX newSize);
    PASN_Object & operator[](PINDEX i) const { return array[i]; }
    void RemoveAt(PINDEX i) { array.RemoveAt(i); }
    void RemoveAll() { array.RemoveAll(); }

    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL IsPrimitive() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    virtual PASN_Object * CreateObject() const = 0;

  protected:
    PASN_Array(unsigned tag = UniversalSequence,
               TagClass tagClass = UniversalTagClass);

    PASN_Array(const PASN_Array & other);
    PASN_Array & operator=(const PASN_Array & other);

    PASN_ObjectArray array;
};


/////////////////////////////////////////////////////////////////////////////

class PASN_Stream : public PBYTEArray
{
    PCLASSINFO(PASN_Stream, PBYTEArray)
  public:
    PASN_Stream();
    PASN_Stream(const PBYTEArray & bytes);
    PASN_Stream(const BYTE * buf, PINDEX size);

    void PrintOn(ostream & strm) const;

    PINDEX GetPosition() const { return byteOffset; }
    void SetPosition(PINDEX newPos);
    BOOL IsAtEnd() { return byteOffset >= GetSize(); }
    void ResetDecoder();
    void BeginEncoding();
    void CompleteEncoding();

    virtual BOOL Read(PChannel & chan) = 0;
    virtual BOOL Write(PChannel & chan) = 0;

    virtual BOOL NullDecode(PASN_Null &) = 0;
    virtual void NullEncode(const PASN_Null &) = 0;
    virtual BOOL BooleanDecode(PASN_Boolean &) = 0;
    virtual void BooleanEncode(const PASN_Boolean &) = 0;
    virtual BOOL IntegerDecode(PASN_Integer &) = 0;
    virtual void IntegerEncode(const PASN_Integer &) = 0;
    virtual BOOL EnumerationDecode(PASN_Enumeration &) = 0;
    virtual void EnumerationEncode(const PASN_Enumeration &) = 0;
    virtual BOOL RealDecode(PASN_Real &) = 0;
    virtual void RealEncode(const PASN_Real &) = 0;
    virtual BOOL ObjectIdDecode(PASN_ObjectId &) = 0;
    virtual void ObjectIdEncode(const PASN_ObjectId &) = 0;
    virtual BOOL BitStringDecode(PASN_BitString &) = 0;
    virtual void BitStringEncode(const PASN_BitString &) = 0;
    virtual BOOL OctetStringDecode(PASN_OctetString &) = 0;
    virtual void OctetStringEncode(const PASN_OctetString &) = 0;
    virtual BOOL ConstrainedStringDecode(PASN_ConstrainedString &) = 0;
    virtual void ConstrainedStringEncode(const PASN_ConstrainedString &) = 0;
    virtual BOOL BMPStringDecode(PASN_BMPString &) = 0;
    virtual void BMPStringEncode(const PASN_BMPString &) = 0;
    virtual BOOL ChoiceDecode(PASN_Choice &) = 0;
    virtual void ChoiceEncode(const PASN_Choice &) = 0;
    virtual BOOL ArrayDecode(PASN_Array &) = 0;
    virtual void ArrayEncode(const PASN_Array &) = 0;
    virtual BOOL SequencePreambleDecode(PASN_Sequence &) = 0;
    virtual void SequencePreambleEncode(const PASN_Sequence &) = 0;
    virtual BOOL SequenceKnownDecode(PASN_Sequence &, PINDEX, PASN_Object &) = 0;
    virtual void SequenceKnownEncode(const PASN_Sequence &, PINDEX, const PASN_Object &) = 0;
    virtual BOOL SequenceUnknownDecode(PASN_Sequence &) = 0;
    virtual void SequenceUnknownEncode(const PASN_Sequence &) = 0;

    BYTE ByteDecode();
    void ByteEncode(unsigned value);

    unsigned BlockDecode(BYTE * bufptr, unsigned nBytes);
    void BlockEncode(const BYTE * bufptr, PINDEX nBytes);

    void ByteAlign();

  protected:
    PINDEX byteOffset;
    unsigned bitOffset;

  private:
    void Construct();
};


class PBER_Stream : public PASN_Stream
{
    PCLASSINFO(PBER_Stream, PASN_Stream)
  public:
    PBER_Stream();
    PBER_Stream(const PBYTEArray & bytes);
    PBER_Stream(const BYTE * buf, PINDEX size);

    PBER_Stream & operator=(const PBYTEArray & bytes);

    virtual BOOL Read(PChannel & chan);
    virtual BOOL Write(PChannel & chan);

    virtual BOOL NullDecode(PASN_Null &);
    virtual void NullEncode(const PASN_Null &);
    virtual BOOL BooleanDecode(PASN_Boolean &);
    virtual void BooleanEncode(const PASN_Boolean &);
    virtual BOOL IntegerDecode(PASN_Integer &);
    virtual void IntegerEncode(const PASN_Integer &);
    virtual BOOL EnumerationDecode(PASN_Enumeration &);
    virtual void EnumerationEncode(const PASN_Enumeration &);
    virtual BOOL RealDecode(PASN_Real &);
    virtual void RealEncode(const PASN_Real &);
    virtual BOOL ObjectIdDecode(PASN_ObjectId &);
    virtual void ObjectIdEncode(const PASN_ObjectId &);
    virtual BOOL BitStringDecode(PASN_BitString &);
    virtual void BitStringEncode(const PASN_BitString &);
    virtual BOOL OctetStringDecode(PASN_OctetString &);
    virtual void OctetStringEncode(const PASN_OctetString &);
    virtual BOOL ConstrainedStringDecode(PASN_ConstrainedString &);
    virtual void ConstrainedStringEncode(const PASN_ConstrainedString &);
    virtual BOOL BMPStringDecode(PASN_BMPString &);
    virtual void BMPStringEncode(const PASN_BMPString &);
    virtual BOOL ChoiceDecode(PASN_Choice &);
    virtual void ChoiceEncode(const PASN_Choice &);
    virtual BOOL ArrayDecode(PASN_Array &);
    virtual void ArrayEncode(const PASN_Array &);
    virtual BOOL SequencePreambleDecode(PASN_Sequence &);
    virtual void SequencePreambleEncode(const PASN_Sequence &);
    virtual BOOL SequenceKnownDecode(PASN_Sequence &, PINDEX, PASN_Object &);
    virtual void SequenceKnownEncode(const PASN_Sequence &, PINDEX, const PASN_Object &);
    virtual BOOL SequenceUnknownDecode(PASN_Sequence &);
    virtual void SequenceUnknownEncode(const PASN_Sequence &);

    virtual PASN_Object * CreateObject(unsigned tag,
                                       PASN_Object::TagClass tagClass,
                                       BOOL primitive) const;

    BOOL HeaderDecode(unsigned & tagVal,
                      PASN_Object::TagClass & tagClass,
                      BOOL & primitive,
                      unsigned & len);
    BOOL HeaderDecode(PASN_Object & obj, unsigned & len);
    void HeaderEncode(const PASN_Object & obj);
};


class PPER_Stream : public PASN_Stream
{
    PCLASSINFO(PPER_Stream, PASN_Stream)
  public:
    PPER_Stream(BOOL aligned = TRUE);
    PPER_Stream(const PBYTEArray & bytes, BOOL aligned = TRUE);
    PPER_Stream(const BYTE * buf, PINDEX size, BOOL aligned = TRUE);

    PPER_Stream & operator=(const PBYTEArray & bytes);

    unsigned GetBitsLeft() const;

    virtual BOOL Read(PChannel & chan);
    virtual BOOL Write(PChannel & chan);

    virtual BOOL NullDecode(PASN_Null &);
    virtual void NullEncode(const PASN_Null &);
    virtual BOOL BooleanDecode(PASN_Boolean &);
    virtual void BooleanEncode(const PASN_Boolean &);
    virtual BOOL IntegerDecode(PASN_Integer &);
    virtual void IntegerEncode(const PASN_Integer &);
    virtual BOOL EnumerationDecode(PASN_Enumeration &);
    virtual void EnumerationEncode(const PASN_Enumeration &);
    virtual BOOL RealDecode(PASN_Real &);
    virtual void RealEncode(const PASN_Real &);
    virtual BOOL ObjectIdDecode(PASN_ObjectId &);
    virtual void ObjectIdEncode(const PASN_ObjectId &);
    virtual BOOL BitStringDecode(PASN_BitString &);
    virtual void BitStringEncode(const PASN_BitString &);
    virtual BOOL OctetStringDecode(PASN_OctetString &);
    virtual void OctetStringEncode(const PASN_OctetString &);
    virtual BOOL ConstrainedStringDecode(PASN_ConstrainedString &);
    virtual void ConstrainedStringEncode(const PASN_ConstrainedString &);
    virtual BOOL BMPStringDecode(PASN_BMPString &);
    virtual void BMPStringEncode(const PASN_BMPString &);
    virtual BOOL ChoiceDecode(PASN_Choice &);
    virtual void ChoiceEncode(const PASN_Choice &);
    virtual BOOL ArrayDecode(PASN_Array &);
    virtual void ArrayEncode(const PASN_Array &);
    virtual BOOL SequencePreambleDecode(PASN_Sequence &);
    virtual void SequencePreambleEncode(const PASN_Sequence &);
    virtual BOOL SequenceKnownDecode(PASN_Sequence &, PINDEX, PASN_Object &);
    virtual void SequenceKnownEncode(const PASN_Sequence &, PINDEX, const PASN_Object &);
    virtual BOOL SequenceUnknownDecode(PASN_Sequence &);
    virtual void SequenceUnknownEncode(const PASN_Sequence &);

    BOOL IsAligned() const { return aligned; }

    BOOL SingleBitDecode();
    void SingleBitEncode(BOOL value);

    int MultiBitDecode(unsigned nBits);
    void MultiBitEncode(int value, unsigned nBits);

    unsigned SmallUnsignedDecode();
    void SmallUnsignedEncode(unsigned val);

    int LengthDecode(unsigned lower, unsigned upper, unsigned & len);
    void LengthEncode(unsigned len, unsigned lower, unsigned upper);

    int UnsignedDecode(unsigned lower, unsigned upper, unsigned & value);
    void UnsignedEncode(int value, unsigned lower, unsigned upper);

    void AnyTypeEncode(const PASN_Object * value);

  protected:
    BOOL aligned;
};


#endif // _ASNER_H
