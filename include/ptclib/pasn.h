/*
 * $Id: pasn.h,v 1.2 1996/11/04 03:56:00 robertj Exp $
 *
 * ASN Classes
 *
 * Copyright 1996 Equivalence
 *
 * $Log: pasn.h,v $
 * Revision 1.2  1996/11/04 03:56:00  robertj
 * Added ASN types to class.
 *
 * Revision 1.1  1996/09/14 12:58:57  robertj
 * Initial revision
 *
 */

#ifndef _PASN_H
#define _PASN_H

#ifdef __GNUC__
#pragma interface
#endif

#include <sockets.h>

//
// define some types used by the ASN classes
//
typedef	PInt32		PASNInt;
typedef DWORD		PASNUnsigned;
typedef	DWORD           PASNOid;

class PASNObject;
class PASNSequence;

//////////////////////////////////////////////////////////////////////////
//
//  PASNObject
//

PLIST(PASNObjectList, PASNObject);

PDECLARE_CLASS(PASNObject, PObject)
/* This class defines the common behviour of all ASN objects. It also contains
   several functions which are used for encoding common ASN primitives.

   This class will never be instantiated directly. See the <A>PASNInteger</A>,
   <A>PASNSequence</A>, <A>PASNString</A> and <A>PASNObjectID</A> classes for examples
   of ASN objects that can be created.

   Only descendants of this class can be put into the <A>ASNSequence</A> class.
*/

  public:
    enum ASNType {
      Integer,		// ASN Integer object
      String,           // ASN Octet String object
      ObjectID,		// ASN Object ID object
      Sequence,		// ASN Sequence object
      Choice,		// ASN Sequence with discriminator
      IPAddress,        // ASN IPAddress object
      Counter,          // ASN Counter object
      Gauge,            // ASN Gauge object
      TimeTicks,        // ASN TimeTicks object
      Opaque,           // ASN Opaque object
      NsapAddress,      // ASN NsapAddress
      Counter64,        // ASN Counter64
      UInteger32,       // ASN Unsigned integer 32
      Unknown,		// unknown ASN object type
      ASNTypeMax	// maximum of number of ASN object types
    };
    /* Value returned by the <A>GetType()</A> function to indicate the type of
       an ASN object
     */

    virtual ASNType GetType() const;
    /* Return a value of type <A>enum ASNType</A> which indicates the type
       of the object
     */


    int GetChoice() const;
    /* Return the descriminator for Choice sequences
    */

    virtual PString GetTypeAsString() const;
    /* Return a string giving the type of the object */

    virtual PASNInt GetInteger () const;
    /* Return the value of the ASN object as a PASNInt.

       This function will assert if the object is not a descendant of
       <A>PASNInteger</A>.
     */

    virtual PASNUnsigned GetUnsigned () const;
    /* Return the value of the object as a PASNUnsigned

       This function will assert if the object is not a descendant of
       <A>PASNTimeTicks</A> or <P>PASNCounter</A>.
     */

    virtual PString GetString () const;
    /* Return the value of the object as a PString. This function can
       be use for all ASN object types
     */

    virtual const PASNSequence & GetSequence() const;
    /* Return the value of the object as a PString

       This function will assert if the object is not a descendant of
       <A>PASNSequence</A>.
     */

    virtual PIPSocket::Address GetIPAddress () const;
    /* Return the value of the object as an IPAddress

       This function will assert if the object is not a descendant of
       <A>PASNIPAddress</A>.
     */

    virtual void PrintOn(
      ostream & strm		// stream to print on
    ) const;
    /* Virtual functions used by the <A>PObject::operator<<</A> function to
       print the value of the object.
    */

    virtual void Encode(
      PBYTEArray & buffer	// buffer to encode into
    );
    /* Virtual function used to encode the object into ASN format */

    virtual WORD GetEncodedLength();
    /* Virtual function used to get the length of object when encoded into
       ASN format 
    */

    virtual PObject * Clone() const;
    /* Virtual function used to duplicate objects */

    static void EncodeASNLength (
      PBYTEArray & buffer,		// buffer to encode into
      WORD length			// ASN length to encode
    );
    /* Encode an ASN length value */

    static WORD GetASNLengthLength (
      WORD length			// length to find length of
    );
    /* Return the length of an encoded ASN length value */

    static BOOL DecodeASNLength (
      const PBYTEArray & buffer,		// buffer to decode data from
      PINDEX & ptr,			// ptr to decode from
      WORD & len			// returned length
    );
    /* Decode an ASN length in the buffer at the given ptr. The ptr is moved
       to the byte after the end of the encoded length.
     */

    static void EncodeASNSequenceStart (
      PBYTEArray & buffer,		// buffer to encode data into
      BYTE type,			// sequence type
      WORD length			// length of sequence data
    );
    /* Encode a sequence header into the buffer at the specified offset. */

    static WORD GetASNSequenceStartLength (
      WORD length			// length of sequence data
    );
    /* Return the encoded length of a sequence if it has the specified length */

    static void EncodeASNHeader(
      PBYTEArray & buffer,		// buffer to encode into
      PASNObject::ASNType type,		// ASN type of the object
      WORD length			// length of the object
    );
    /* Encode an ASN object header into the buffer */

    static WORD GetASNHeaderLength (
      WORD length			// length of object
    );
    /* Return the length of an ASN object header if the object is the specified length */

    static void EncodeASNInteger    (
      PBYTEArray & buffer,		// buffer to encode into
      PASNInt data,			// value to encode
      PASNObject::ASNType type		// actual integer type
    );
    // Encode an ASN integer value into the specified buffer */

    static void EncodeASNUnsigned (
      PBYTEArray & buffer,		// buffer to encode into
      PASNUnsigned data,		// value to encode
      PASNObject::ASNType type		// actual integer type
    );
    // Encode an ASN integer value into the specified buffer */

    static WORD GetASNIntegerLength (
      PASNInt data			// value to get length of
    );
    // Return the length of an encoded ASN integer with the specified value 

    static WORD GetASNUnsignedLength (
      PASNUnsigned data			// value to get length of
    );
    // Return the length of an encoded ASN integer with the specified value 

    static BOOL DecodeASNInteger (
      const PBYTEArray & buffer,		// buffer to decode from
      PINDEX & ptr,			// ptr to data in buffer
      PASNInt & value,			// returned value
      ASNType type = Integer	        // actual integer type
    );
    // Decode an ASN integer value in the specified buffer 

    static BOOL DecodeASNUnsigned (
      const PBYTEArray & buffer,		// buffer to decode from
      PINDEX & ptr,			// ptr to data in buffer
      PASNUnsigned & value,		// returned value
      ASNType type = TimeTicks	        // actual integer type
    );
    // Decode an ASN integer value in the specified buffer 

  protected:
    PASNObject();
    /* Create an empty ASN object. Used only by descendant constructors */

    static BYTE ASNTypeToType[ASNTypeMax];
    /* Table to map <A>enum ASNType</A> values to ASN identifiers */

};

//////////////////////////////////////////////////////////////////////////
//
//  PASNInteger
//     A descendant of PASNObject which is a simple ASN integer type
//

PDECLARE_CLASS(PASNInteger, PASNObject)
/* This class defines an ASN integer object.
*/

  public:
    PASNInteger(PASNInt val);
    PASNInteger(const PBYTEArray & buffer, PINDEX & ptr);

    void PrintOn(ostream & strm) const;
    void Encode(PBYTEArray & buffer);
    WORD GetEncodedLength();
    PObject * Clone() const;

    PASNInt GetInteger() const;
    PString GetString () const;
    ASNType GetType() const;
    PString GetTypeAsString() const;

  private:
    PASNInt value;
};

//////////////////////////////////////////////////////////////////////////
//
//  PASNString
//     A descendant of PASNObject which is a simple ASN OctetStr type
//

PDECLARE_CLASS(PASNString, PASNObject)
  public:
    PASNString(const PString & str);
    PASNString(const PBYTEArray & buffer,               PASNObject::ASNType = String);
    PASNString(const PBYTEArray & buffer, PINDEX & ptr, PASNObject::ASNType = String);

    void PrintOn(ostream & strm) const;

    void Encode(PBYTEArray & buffer)
      { Encode(buffer, String); }

    WORD GetEncodedLength();
    PObject * Clone() const;

    PString GetString() const;
    ASNType GetType() const;
    PString GetTypeAsString() const;

  protected:
    BOOL Decode(const PBYTEArray & buffer, PINDEX & i, PASNObject::ASNType type);
    void Encode(PBYTEArray & buffer,             PASNObject::ASNType type);

    PString value;
};

PDECLARE_CLASS(PASNIPAddress, PASNString)
  public:
    PASNIPAddress(const PIPSocket::Address & addr)
      : PASNString((PString)addr) { }

    PASNIPAddress(const PString & str);

    PASNIPAddress(const PBYTEArray & buffer)
      : PASNString(buffer, IPAddress) { }

    PASNIPAddress(const PBYTEArray & buffer, PINDEX & ptr)
      : PASNString(buffer, ptr, IPAddress) { }

    PASNObject::ASNType GetType() const
      { return IPAddress; }

    void Encode(PBYTEArray & buffer)
      { PASNString::Encode(buffer, IPAddress); }

    PString GetString() const;

    PString GetTypeAsString() const;

    PObject * Clone() const
      { return PNEW PASNIPAddress(*this); }

    PIPSocket::Address GetIPAddress () const;
};

//////////////////////////////////////////////////////////////////////////
//
//

PDECLARE_CLASS(PASNUnsignedInteger, PASNObject)
  public:
    PASNUnsignedInteger(PASNUnsigned val)
      { value = val; }

    PASNUnsignedInteger(const PBYTEArray & buffer, PINDEX & ptr);

    void PrintOn(ostream & strm) const;
    WORD GetEncodedLength();
    PString GetString () const;
    PASNUnsigned GetUnsigned() const;

  protected:
    inline PASNUnsignedInteger() { }

    BOOL Decode(const PBYTEArray & buffer, PINDEX & i, PASNObject::ASNType theType);
    void Encode(PBYTEArray & buffer, PASNObject::ASNType theType);

  private:
    PASNUnsigned value;
};


PDECLARE_CLASS(PASNTimeTicks, PASNUnsignedInteger)
  public:
    PASNTimeTicks(PASNUnsigned val) 
      : PASNUnsignedInteger(val) { }

    PASNTimeTicks(const PBYTEArray & buffer, PINDEX & ptr)
      { PASNUnsignedInteger::Decode(buffer, ptr, TimeTicks); }

    void Encode(PBYTEArray & buffer)
      { PASNUnsignedInteger::Encode(buffer, TimeTicks); }

    PObject * Clone() const
      { return PNEW PASNTimeTicks(*this); }

    PASNObject::ASNType GetType() const
      { return TimeTicks; }

    PString GetTypeAsString() const;
};


PDECLARE_CLASS(PASNCounter, PASNUnsignedInteger)
  public:
    PASNCounter(PASNUnsigned val) 
      : PASNUnsignedInteger(val) { }

    PASNCounter(const PBYTEArray & buffer, PINDEX & ptr)
      {  PASNUnsignedInteger::Decode(buffer, ptr, Counter); }

    void Encode(PBYTEArray & buffer)
      { PASNUnsignedInteger::Encode(buffer, Counter); }

    PObject * Clone() const
      { return PNEW PASNCounter(*this); }

    PASNObject::ASNType GetType() const
      { return Counter; }

    PString GetTypeAsString() const;
};


PDECLARE_CLASS(PASNGauge, PASNUnsignedInteger)
  public:
    PASNGauge(PASNUnsigned val) 
      : PASNUnsignedInteger(val) { }

    PASNGauge(const PBYTEArray & buffer, PINDEX & ptr)
      { Decode(buffer, ptr); }

    BOOL Decode(const PBYTEArray & buffer, PINDEX & i)
      { return PASNUnsignedInteger::Decode(buffer, i, Gauge); }

    void Encode(PBYTEArray & buffer)
      { PASNUnsignedInteger::Encode(buffer, Gauge); }

    PObject * Clone() const
      { return PNEW PASNGauge(*this); }

    PASNObject::ASNType GetType() const
      { return Gauge; }

    PString GetTypeAsString() const;
};



//////////////////////////////////////////////////////////////////////////
//
//  PASNObjectID
//     A descendant of PASNObject which is a simple ASN ObjID type
//

PDECLARE_CLASS(PASNObjectID, PASNObject)
  public:
    PASNObjectID(const PString & str);
    PASNObjectID(PASNOid * val, BYTE theLen);
    PASNObjectID(const PBYTEArray & buffer);
    PASNObjectID(const PBYTEArray & buffer, PINDEX & ptr);

    void PrintOn(ostream & strm) const;
    void Encode(PBYTEArray & buffer);
    WORD GetEncodedLength();
    PObject * Clone() const;

    ASNType GetType() const;
    PString GetString () const;
    PString GetTypeAsString() const;

  protected:
    BOOL Decode(const PBYTEArray & buffer, PINDEX & i);

  private:
    PDWORDArray value;
};

//////////////////////////////////////////////////////////////////////////
//
//  PASNSequence
//     A descendant of PASNObject which is the complex sequence type
//

PDECLARE_CLASS(PASNSequence, PASNObject)
  
  public:
    PASNSequence();
    PASNSequence(BYTE selector);
    PASNSequence(const PBYTEArray & buffer);
    PASNSequence(const PBYTEArray & buffer, PINDEX & i);

    void Append(PASNObject * obj);
    PINDEX GetSize() const;
    PASNObject & operator [] (PINDEX idx) const;
    const PASNSequence & GetSequence() const;

    void AppendInteger (PASNInt value);
    void AppendString  (const PString & str);
    void AppendObjectID(const PString & str);
    void AppendObjectID(PASNOid * val, BYTE len);

    int GetChoice() const;

//    PASNInt GetInteger (PINDEX idx) const;
//    PString GetString  (PINDEX idx) const;

    void PrintOn(ostream & strm) const;
    void Encode(PBYTEArray & buffer);
    BOOL Decode(const PBYTEArray & buffer, PINDEX & i);
    WORD GetEncodedLength();
    ASNType GetType() const;
    PString GetTypeAsString() const;

    BOOL Encode(PBYTEArray & buffer, PINDEX maxLen) ;

  private:
    PASNObjectList sequence;
    BYTE     type;
    ASNType  asnType;
    WORD     encodedLen;
    WORD     seqLen;
};

#endif
