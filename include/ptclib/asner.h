/*
 * $Id: asner.h,v 1.3 1997/12/18 05:08:13 robertj Exp $
 *
 * Portable Windows Library
 *
 * Abstract Syntax Notation Encoding Rules
 *
 * Copyright 1993 Equivalence
 *
 * $Log: asner.h,v $
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

  protected:
    PASN_Object(unsigned tag, TagClass tagClass, BOOL extendable);

    BOOL extendable;   // PER extension capability
    TagClass tagClass; // BER tag class
    unsigned tag;      // ASN object tag

  private:
    PASN_Object(const PASN_Object &) { }
};


class PASN_ConstrainedObject : public PASN_Object
{
    PCLASSINFO(PASN_ConstrainedObject, PASN_Object)
  public:
    enum ConstraintType {
      Unconstrained,
      FixedConstraint,
      ExtendableConstraint
    };

    void SetConstraints(ConstraintType type, int lower = 0, unsigned upper = UINT_MAX);
    BOOL IsConstrained() const { return !unconstrained; }
    int GetLowerLimit() const { return lowerLimit; }
    unsigned GetUpperLimit() const { return upperLimit; }

    ConstraintType ConstraintDecode(PPER_Stream & strm, unsigned * range = NULL);
    ConstraintType ConstraintEncode(PPER_Stream & strm, unsigned value, unsigned * range = NULL) const;

  protected:
    PASN_ConstrainedObject(unsigned tag, TagClass tagClass,
                           int lower, unsigned upper,
                           ConstraintType ctype = Unconstrained);

    BOOL unconstrained;
    int lowerLimit;
    unsigned upperLimit;
};


class PASN_Null : public PASN_Object
{
    PCLASSINFO(PASN_Null, PASN_Object)
  public:
    PASN_Null(unsigned tag = UniversalNull,
              TagClass tagClass = UniversalTagClass);

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
                 int lower = 0, unsigned upper = UINT_MAX,
                 ConstraintType ctype = Unconstrained,
                 unsigned val = 0);

    PASN_Integer & operator=(unsigned value);
    operator unsigned() const { return value; }
    unsigned GetValue() const { return value; }
    void SetValue(unsigned v) { operator=(v); }

    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

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

    unsigned GetMaximum() const { return numEnums; }

    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

  protected:
    unsigned numEnums;
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

    PASN_ObjectId & operator=(const char * dotstr);
    PASN_ObjectId & operator=(const PString & dotstr);
    void SetValue(const PString & dotstr);

    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL CommonDecode(PASN_Stream & strm, PINDEX dataLen);
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
                   int lower = 0, unsigned upper = UINT_MAX,
                   ConstraintType ctype = Unconstrained,
                   PINDEX nBits = 0);

    void SetData(PINDEX nBits, const PBYTEArray & bytes);
    void SetData(PINDEX nBits, const BYTE * buf, PINDEX size);

    PINDEX GetSize() const { return totalBits; }
    BOOL SetSize(PINDEX nBits);

    BOOL operator[](PINDEX bit) const;
    void Set(PINDEX bit);
    void Clear(PINDEX bit);
    void Invert(PINDEX bit);

    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodeBER(PBER_Stream & strm, PINDEX len);
    void EncodeBER(PBER_Stream & strm) const;
    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    PINDEX totalBits;
    PBYTEArray bitData;
};


class PASN_OctetString : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_OctetString, PASN_ConstrainedObject)
  public:
    PASN_OctetString(unsigned tag = UniversalOctetString,
                     TagClass tagClass = UniversalTagClass,
                     int lower = 0, unsigned upper = UINT_MAX,
                     ConstraintType ctype = Unconstrained);

    PINDEX GetSize() const { return value.GetSize(); }
    BOOL SetSize(PINDEX newSize) { return value.SetSize(newSize); }
    BYTE * GetPointer(PINDEX sz = 0) { return value.GetPointer(sz); }
    operator const BYTE *() const { return value; }
    BYTE operator[](PINDEX i) const { return value[i]; }
    BYTE & operator[](PINDEX i) { return value[i]; }
    operator const PBYTEArray &() const { return value; }
    const PBYTEArray & GetValue() const { return value; }
    PString AsString() const;
    PASN_OctetString & operator=(const char * str);
    PASN_OctetString & operator=(const PString & str);
    PASN_OctetString & operator=(const PBYTEArray & arr);
    void SetValue(const char * str) { operator=(str); }
    void SetValue(const PString & str) { operator=(str); }
    void SetValue(const PBYTEArray & arr) { operator=(arr); }
    void SetValue(const BYTE * data, PINDEX len);

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
    PASN_ConstrainedString(const char * canonicalSet, PINDEX setSize,
                           unsigned tag, TagClass tagClass,
                           int lower, unsigned upper,
                           ConstraintType ctype,
                           const char * charSet, BOOL extendChars);

    PASN_ConstrainedString & operator=(const char * str);
    PASN_ConstrainedString & operator=(const PString & str) { return operator=((const char *)str); }
    operator const PString &() const { return value; }
    const PString & GetValue() const { return value; }
    void SetValue(const PString & v) { operator=(v); }

    virtual void PrintOn(ostream & strm) const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodeBER(PBER_Stream & strm, PINDEX len);
    void EncodeBER(PBER_Stream & strm) const;
    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    PString value;
    PString charSet;
    PINDEX canonicalSetBits;
    PINDEX charSetUnalignedBits;
    PINDEX charSetAlignedBits;
};


class PASN_NumericString : public PASN_ConstrainedString
{
    PCLASSINFO(PASN_NumericString, PASN_ConstrainedString)
  public:
    PASN_NumericString(unsigned tag = UniversalNumericString,
                       TagClass tagClass = UniversalTagClass,
                       int lower = 0, unsigned upper = UINT_MAX,
                       ConstraintType ctype = Unconstrained,
                       const char * charSet = NULL, BOOL extendChars = FALSE);

    PASN_NumericString & operator=(const char * str) { PASN_ConstrainedString::SetValue(str); return *this; }
    PASN_NumericString & operator=(const PString & str) { PASN_ConstrainedString::SetValue(str); return *this; }

    virtual PString GetTypeAsString() const;
};


class PASN_PrintableString : public PASN_ConstrainedString
{
    PCLASSINFO(PASN_PrintableString, PASN_ConstrainedString)
  public:
    PASN_PrintableString(unsigned tag = UniversalPrintableString,
                         TagClass tagClass = UniversalTagClass,
                         int lower = 0, unsigned upper = UINT_MAX,
                         ConstraintType ctype = Unconstrained,
                         const char * charSet = NULL, BOOL extendChars = FALSE);

    PASN_PrintableString & operator=(const char * str) { PASN_ConstrainedString::SetValue(str); return *this; }
    PASN_PrintableString & operator=(const PString & str) { PASN_ConstrainedString::SetValue(str); return *this; }

    virtual PString GetTypeAsString() const;
};


class PASN_VisibleString : public PASN_ConstrainedString
{
    PCLASSINFO(PASN_VisibleString, PASN_ConstrainedString)
  public:
    PASN_VisibleString(unsigned tag = UniversalVisibleString,
                       TagClass tagClass = UniversalTagClass,
                       int lower = 0, unsigned upper = UINT_MAX,
                       ConstraintType ctype = Unconstrained,
                       const char * charSet = NULL, BOOL extendChars = FALSE);

    PASN_VisibleString & operator=(const char * str) { PASN_ConstrainedString::SetValue(str); return *this; }
    PASN_VisibleString & operator=(const PString & str) { PASN_ConstrainedString::SetValue(str); return *this; }

    virtual PString GetTypeAsString() const;
};


class PASN_IA5String : public PASN_ConstrainedString
{
    PCLASSINFO(PASN_IA5String, PASN_ConstrainedString)
  public:
    PASN_IA5String(unsigned tag = UniversalIA5String,
                   TagClass tagClass = UniversalTagClass,
                   int lower = 0, unsigned upper = UINT_MAX,
                   ConstraintType ctype = Unconstrained,
                   const char * charSet = NULL, BOOL extendChars = FALSE);

    PASN_IA5String & operator=(const char * str) { PASN_ConstrainedString::SetValue(str); return *this; }
    PASN_IA5String & operator=(const PString & str) { PASN_ConstrainedString::SetValue(str); return *this; }

    virtual PString GetTypeAsString() const;
};


class PASN_GeneralString : public PASN_ConstrainedString
{
    PCLASSINFO(PASN_GeneralString, PASN_ConstrainedString)
  public:
    PASN_GeneralString(unsigned tag = UniversalGeneralString,
                       TagClass tagClass = UniversalTagClass,
                       int lower = 0, unsigned upper = UINT_MAX,
                       ConstraintType ctype = Unconstrained,
                       const char * charSet = NULL, BOOL extendChars = FALSE);

    PASN_GeneralString & operator=(const char * str) { PASN_ConstrainedString::SetValue(str); return *this; }
    PASN_GeneralString & operator=(const PString & str) { PASN_ConstrainedString::SetValue(str); return *this; }

    virtual PString GetTypeAsString() const;
};


class PASN_BMPString : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_BMPString, PASN_ConstrainedObject)
  public:
    PASN_BMPString(unsigned tag = UniversalBMPString,
                   TagClass tagClass = UniversalTagClass,
                   int lower = 0, unsigned upper = UINT_MAX,
                   ConstraintType ctype = Unconstrained);

    PASN_BMPString & operator=(const PString & v);
    operator PString() const { return GetValue(); }
    PString GetValue() const;
    void SetValue(const PString & v) { operator=(v); }

    virtual void PrintOn(ostream & strm) const;
    virtual PString GetTypeAsString() const;
    virtual PINDEX GetDataLength() const;
    virtual BOOL Decode(PASN_Stream &);
    virtual void Encode(PASN_Stream &) const;

    BOOL DecodeBER(PBER_Stream & strm, PINDEX len);
    void EncodeBER(PBER_Stream & strm) const;
    BOOL DecodePER(PPER_Stream & strm);
    void EncodePER(PPER_Stream & strm) const;

  protected:
    PWORDArray value;
};


class PASN_Choice : public PASN_Object
{
    PCLASSINFO(PASN_Choice, PASN_Object)
  public:
    PASN_Choice(PINDEX nChoices = UINT_MAX, BOOL extend = FALSE);
    PASN_Choice(unsigned tag, TagClass tagClass, PINDEX nChoices, BOOL extend);
    PASN_Choice(unsigned tag, TagClass tagClass, PINDEX nChoices, BOOL extend, const PString & nameSpec);
    ~PASN_Choice();

    PASN_Choice & operator=(const PASN_Choice & other);

    virtual void SetTag(unsigned newTag, TagClass tagClass = DefaultTagClass);
    PString GetTagName() const;
    PASN_Object & GetObject() const;
    BOOL IsValid() const { return choice != NULL; }

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
    unsigned maxChoices;
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
                  PINDEX nOpts = 0, BOOL extend = FALSE, PINDEX nExtend = 0);

    PASN_Sequence & operator=(const PASN_Sequence & other);

    PINDEX GetSize() const { return fields.GetSize(); }
    void SetSize(PINDEX newSize);
    PASN_Object & operator[](PINDEX i) const { return fields[i]; }

    BOOL HasOptionalField(PINDEX opt) const;
    void IncludeOptionalField(PINDEX opt);

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
             PINDEX nOpts = 0, BOOL extend = FALSE, PINDEX nExtend = 0);

    virtual PString GetTypeAsString() const;
};


class PASN_Array : public PASN_ConstrainedObject
{
    PCLASSINFO(PASN_Array, PASN_ConstrainedObject)
  public:
    PASN_Array(unsigned tag = UniversalSequence,
               TagClass tagClass = UniversalTagClass,
               int lower = 0, unsigned upper = UINT_MAX,
               ConstraintType ctype = Unconstrained);

    PASN_Array & operator=(const PASN_Array & other);

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

    PINDEX BlockDecode(BYTE * bufptr, PINDEX nBytes);
    void BlockEncode(const BYTE * bufptr, PINDEX nBytes);

    void ByteAlign();

  protected:
    PINDEX byteOffset, bitOffset;

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
                      PINDEX & len);
    BOOL HeaderDecode(PASN_Object & obj, PINDEX & len);
    void HeaderEncode(const PASN_Object & obj);
};


class PPER_Stream : public PASN_Stream
{
    PCLASSINFO(PPER_Stream, PASN_Stream)
  public:
    PPER_Stream(BOOL aligned = TRUE);
    PPER_Stream(const PBYTEArray & bytes, BOOL aligned = TRUE);
    PPER_Stream(const BYTE * buf, PINDEX size, BOOL aligned = TRUE);

    PINDEX GetBitsLeft() const;

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

    int MultiBitDecode(PINDEX nBits);
    void MultiBitEncode(int value, PINDEX nBits);

    unsigned SmallUnsignedDecode();
    void SmallUnsignedEncode(unsigned val);

    PINDEX LengthDecode(int lower, int upper);
    void LengthEncode(PINDEX len, int lower, int upper);

    int UnsignedDecode(unsigned range);
    void UnsignedEncode(int value, unsigned range);

    void AnyTypeEncode(const PASN_Object * value);

  protected:
    BOOL aligned;
};


#endif // _ASNER_H
