/*
 * $Id: asner.h,v 1.1 1997/10/10 10:37:35 robertj Exp $
 *
 * Portable Windows Library
 *
 * Packed Encoding Rules for Abstract Syntax Notation
 *
 * Copyright 1993 Equivalence
 *
 * $Log: asner.h,v $
 * Revision 1.1  1997/10/10 10:37:35  robertj
 * Initial revision
 *
 */

#ifndef _PERASN_H
#define _PERASN_H

#ifdef __GNUC__
#pragma interface
#endif


class PPER_BitStream : public PObject
{
PCLASSINFO(PPER_BitStream, PObject)
  public:
    PPER_BitStream(BOOL aligned = TRUE);
    PPER_BitStream(const PBYTEArray & bytes, BOOL aligned = TRUE);
    PPER_BitStream(const BYTE * buf, PINDEX size, BOOL aligned = TRUE);

    void PrintOn(ostream & strm) const;

    PINDEX GetSize() const { return bitData.GetSize(); }
    BYTE * GetPointer() { return bitData.GetPointer(); }
    PINDEX GetPosition() const { return byteOffset; }
    void SetPosition(PINDEX newPos);
    PINDEX GetBitsLeft() const;
    BOOL IsAtEnd() { return byteOffset >= bitData.GetSize(); }
    void ResetDecoder();
    void BeginEncoding();
    void CompleteEncoding();

    BOOL Read(PChannel & chan);
    BOOL Write(PChannel & chan);

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

    PINDEX BlockDecode(BYTE * bufptr, PINDEX nBytes);
    void BlockEncode(const BYTE * bufptr, PINDEX nBytes);

    void ByteAlign(BOOL encoding);

  protected:
    PBYTEArray bitData;
    PINDEX byteOffset, bitOffset;
    BOOL aligned;

  private:
    void Construct(BOOL alignment);
};


PDECLARE_CLASS(PPERObject, PObject)
  public:
    PPERObject(BOOL extendable = FALSE);

    virtual void Decode(PPER_BitStream &) = 0;
    virtual void Encode(PPER_BitStream &) = 0;

    BOOL Read(PChannel & chan);
    BOOL Write(PChannel & chan);

  protected:
    BOOL extendable;

  private:
    PPERObject(const PPERObject &) { }
};


PDECLARE_CLASS(PPERConstrainedObject, PPERObject)
  public:
    PPERConstrainedObject();
    PPERConstrainedObject(int lower, unsigned upper, BOOL extendable);

    void SetUnconstrained();
    void SetLimits(int lower, unsigned upper);

  protected:
    BOOL UnconstrainedDecode(PPER_BitStream & strm);
    BOOL UnconstrainedEncode(PPER_BitStream & strm, unsigned value);

    BOOL unconstrained;
    int lowerLimit;
    unsigned upperLimit;
};


PDECLARE_CLASS(PPER_Boolean, PPERObject)
  public:
    PPER_Boolean(BOOL val = FALSE);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    PPER_Boolean & operator=(BOOL value);
    operator BOOL() const { return value; }

  protected:
    BOOL value;
};


PDECLARE_CLASS(PPER_Integer, PPERConstrainedObject)
  public:
    PPER_Integer(unsigned val = 0);
    PPER_Integer(int lower, unsigned upper, BOOL extendable = FALSE, unsigned val = 0);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    PPER_Integer & operator=(unsigned value);
    operator unsigned() const { return value; }

  protected:
    unsigned value;
};


PDECLARE_CLASS(PPER_Enumeration, PPERObject)
  public:
    PPER_Enumeration(int nEnums,
                     const PString & nameSpec,
                     BOOL extendable = FALSE,
                     int val = 0);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    operator int() const { return value; }

  protected:
    int numEnums;
    int value;
    PStringArray names;
};


PDECLARE_CLASS(PPER_Real, PPERObject)
  public:
    PPER_Real(double val = 0);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    operator double() const { return value; }

  protected:
    double value;
};


PDECLARE_CLASS(PPER_BitString, PPERConstrainedObject)
  public:
    PPER_BitString(int nBits = 0);
    PPER_BitString(int lower, int upper, BOOL extendable = FALSE, int nBits = 0);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    void SetData(PINDEX nBits, const PBYTEArray & bytes);
    void SetData(PINDEX nBits, const BYTE * buf, PINDEX size);

    PINDEX GetSize() const { return totalBits; }
    BOOL SetSize(PINDEX nBits);

    BOOL operator[](PINDEX bit) const;
    void Set(PINDEX bit);
    void Clear(PINDEX bit);
    void Invert(PINDEX bit);

  protected:
    PINDEX totalBits;
    PBYTEArray bitData;
};


PDECLARE_CLASS(PPER_OctetString, PPERConstrainedObject)
  public:
    PPER_OctetString();
    PPER_OctetString(int lower, int upper, BOOL extendable = FALSE);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    PINDEX GetSize() const { return value.GetSize(); }
    BYTE * GetPointer() { return value.GetPointer(); }
    BYTE operator[](PINDEX i) const { return value[i]; }
    BYTE & operator[](PINDEX i) { return value[i]; }
    void SetData(const PString & str) { SetData((const BYTE *)(const char *)str, str.GetLength()); }
    void SetData(const BYTE * data, PINDEX len);

  protected:
    PBYTEArray value;
};


PDECLARE_CLASS(PPERConstrainedString, PPERConstrainedObject)
  public:
    PPERConstrainedString(const char * canonicalSet, PINDEX setSize,
                          const PString & charSet,
                          int lower, int upper, BOOL extend);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    PINDEX GetSize() const { return value.GetSize(); }
    char * GetPointer() { return value.GetPointer(); }

    PPERConstrainedString & operator=(const PString & str);
    operator PString() const { return value; }

  protected:
    PString value;
    PString charSet;
    PINDEX canonicalSetBits;
    PINDEX charSetUnalignedBits;
    PINDEX charSetAlignedBits;
};


PDECLARE_CLASS(PPER_NumericString, PPERConstrainedString)
  public:
    PPER_NumericString();
    PPER_NumericString(int lower, int upper, BOOL extend = FALSE);
    PPER_NumericString(const PString & charset, int lower, int upper, BOOL extend = FALSE);
};


PDECLARE_CLASS(PPER_PrintableString, PPERConstrainedString)
  public:
    PPER_PrintableString();
    PPER_PrintableString(int lower, int upper, BOOL extend = FALSE);
    PPER_PrintableString(const PString & charset, int lower, int upper, BOOL extend = FALSE);
};


PDECLARE_CLASS(PPER_VisibleString, PPERConstrainedString)
  public:
    PPER_VisibleString();
    PPER_VisibleString(int lower, int upper, BOOL extend = FALSE);
    PPER_VisibleString(const PString & charset, int lower, int upper, BOOL extend = FALSE);
};


PDECLARE_CLASS(PPER_IA5String, PPERConstrainedString)
  public:
    PPER_IA5String();
    PPER_IA5String(int lower, int upper, BOOL extend = FALSE);
    PPER_IA5String(const PString & charset, int lower, int upper, BOOL extend = FALSE);
};


PDECLARE_CLASS(PPER_GeneralString, PPERConstrainedString)
  public:
    PPER_GeneralString();
    PPER_GeneralString(int lower, int upper, BOOL extend = FALSE);
    PPER_GeneralString(const PString & charset, int lower, int upper, BOOL extend = FALSE);
};


PDECLARE_CLASS(PPER_BMPString, PPERConstrainedObject)
  public:
    PPER_BMPString();
    PPER_BMPString(int lower, int upper, BOOL extend = FALSE);

    virtual void Decode(PPER_BitStream & strm);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    PPER_BMPString & operator=(const PString &);
    operator PString() const;

  protected:
    PWORDArray value;
};


PDECLARE_CLASS(PPER_ObjectId, PPERObject)
  public:
    PPER_ObjectId();
    PPER_ObjectId(const PString & dotstr);

    PPER_ObjectId & operator=(const PString & dotstr);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

  protected:
    PUnsignedArray value;
};


PDECLARE_CLASS(PPER_Sequence, PPERObject)
  public:
    PPER_Sequence(PINDEX nOpts = 0, BOOL extend = FALSE, PINDEX nExtend = 0);

    PPER_Sequence & operator=(const PPER_Sequence & other);

    void PreambleDecode(PPER_BitStream & strm);
    void PreambleEncode(PPER_BitStream & strm);

    BOOL HasOptionalField(int opt) const;
    void IncludeOptionalField(int opt);

    BOOL ExtensionsDecode(PPER_BitStream & strm);
    BOOL ExtensionsEncode(PPER_BitStream & strm);
    void KnownExtensionDecode(PPER_BitStream & strm, PINDEX fld, PPERObject & field);
    void KnownExtensionEncode(PPER_BitStream & strm, PINDEX fld, PPERObject & field);
    void UnknownExtensionsDecode(PPER_BitStream & strm);
    void UnknownExtensionsEncode(PPER_BitStream & strm);

  protected:
    PPER_BitString optionMap;
    int knownExtensions;
    int totalExtensions;
    PPER_BitString extensionMap;
    PARRAY(InternalExtensionsArray, PPER_OctetString);
    InternalExtensionsArray unknownExtensions;
};


PDECLARE_CLASS(PPER_Choice, PPERObject)
  public:
    PPER_Choice(int nChoices, BOOL extend = FALSE);
    PPER_Choice(int nChoices, BOOL extend, const PString & nameSpec);
    ~PPER_Choice();

    PPER_Choice & operator=(const PPER_Choice & other);

    virtual void Decode(PPER_BitStream &);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    virtual void CreateObject() = 0;

    void SetChoice(int newChoice);
    int GetChoice() const { return discriminator; }
    PPERObject & GetObject() const;

  protected:
    int maxChoices;
    int discriminator;
    PPERObject * choice;
    PStringArray names;
};


PDECLARE_CLASS(PPER_Array, PPERConstrainedObject)
  public:
    PPER_Array();
    PPER_Array(int lower, int upper, BOOL extend = FALSE);

    PPER_Array & operator=(const PPER_Array & other);

    virtual void Decode(PPER_BitStream & strm);
    virtual void Encode(PPER_BitStream &);
    virtual void PrintOn(ostream & strm) const;

    virtual PPERObject * CreateObject() const = 0;

    PINDEX GetSize() const { return array.GetSize(); }
    void SetSize(PINDEX newSize);
    PPERObject & operator[](PINDEX i) const { return array[i]; }
    void RemoveAt(PINDEX i) { array.RemoveAt(i); }

  protected:
    PARRAY(InternalObjectArray, PPERObject);
    InternalObjectArray array;
};


#endif // _PERASN_H
