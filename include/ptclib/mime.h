/*
 * $Id: mime.h,v 1.1 1996/01/23 13:06:18 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: mime.h,v $
 * Revision 1.1  1996/01/23 13:06:18  robertj
 * Initial revision
 *
 */

#ifndef _PMIME
#define _PMIME

#ifdef __GNUC__
#pragma interface
#endif

#include <appsock.h>


//////////////////////////////////////////////////////////////////////////////
// PMIMEInfo

PDECLARE_STRING_DICTIONARY(PMIMEInfo, PCaselessString)
/* This class contains the Multipurpose Internet Mail Extensions parameters
   and variables.
 */

  public:
  // Overrides from class PObject
    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;
    /* Output the contents of the MIME dictionary to the stream. This is
       primarily used by the standard <CODE><A>operator<<</A></CODE> function.
     */

    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );
    /* Input the contents of the MIME dictionary from the stream. This is
       primarily used by the standard <CODE><A>operator>></A></CODE> function.
     */


  // New functions for class.
    BOOL Read(
      PApplicationSocket & socket   // Application socket to read MIME info.
    );
    /* Read MIME information from the socket.

       <H2>Returns:</H2>
       TRUE if the MIME information was successfully read.
     */

    BOOL Write(
      PApplicationSocket & socket   // Application socket to write MIME info.
    );
    /* Write MIME information to the socket.

       <H2>Returns:</H2>
       TRUE if the MIME information was successfully read.
     */

    PString GetString(
      const PString & key,    // Key into MIME dictionary to get information.
      const PString & dflt    // Default value of field if not in MIME info.
    ) const;
    /* Get a string for the particular MIME info field with checking for
       existance. The <CODE>dflt</CODE> parameter is substituted if the field
       does not exist in the MIME information read in.

       <H2>Returns:</H2>
       String for the value of the MIME variable.
     */

    long GetInteger(
      const PString & key,    // Key into MIME dictionary to get information.
      long dflt               // Default value of field if not in MIME info.
    ) const;
    /* Get an integer value for the particular MIME info field with checking
       for existance. The <CODE>dflt</CODE> parameter is substituted if the
       field does not exist in the MIME information read in.

       <H2>Returns:</H2>
       Integer value for the MIME variable.
     */
};


PDECLARE_CLASS(PBase64, PObject)
/* This class is used to encode/decode data using the MIME standard base64
   encoding mechanism as defined in RFC1521.

   To encode a large block of data use the following seqeunce:
      <PRE><CODE>
      PBase64 base;
      base.StartEncoding();
      while (Read(dataChunk)) {
        base.ProcessEncoding(dataChunk);
        out << base.GetEncodedString();
      }
      out << base.CompleteEncoding();
      </CODE></PRE>
    if smaller blocks that fit easily in memory are to be encoded the
    <A>Encode()</A> functions can be used to everything in one go.

    To decode a large block of data use the following sequence:
      <PRE><CODE>
      PBase64 base;
      base.StartDecoding();
      while (Read(str) && ProcessDecoding(str))
        Write(base.GetDecodedData());
      Write(base.GetDecodedData());
      </CODE></PRE>
    if smaller blocks that fit easily in memory are to be decoded the
    <A>Decode()</A> functions can be used to everything in one go.
 */

  public:
    PBase64();
    /* Construct a base 64 encoder/decoder and initialise both encode and
       decode members as in <A>StartEncoding()</A> and <A>StartDecoding()</A>.
     */

    void StartEncoding(
      BOOL useCRLFs = TRUE  // Use CR, LF pairs in end of line characters.
    );
    // Begin a base 64 encoding operation, initialising the object instance.

    void ProcessEncoding(
      const PString & str      // String to be encoded
    );
    void ProcessEncoding(
      const char * cstr        // C String to be encoded
    );
    void ProcessEncoding(
      const PBYTEArray & data  // Data block to be encoded
    );
    void ProcessEncoding(
      PConstMemoryPointer dataBlock,  // Pointer to data to be encoded
      PINDEX length                   // Length of the data block.
    );
    // Incorporate the specified data into the base 64 encoding.

    PString GetEncodedString();
    /* Get the partial Base64 string for the data encoded so far.
    
       <H2>Returns:</H2>
       Base64 encoded string for the processed data.
     */

    PString CompleteEncoding();
    /* Complete the base 64 encoding and return the remainder of the encoded
       Base64 string. Previous data may have been already removed by the
       <A>GetInterim()</A> function.
    
       <H2>Returns:</H2>
       Base64 encoded string for the processed data.
     */


    static PString Encode(
      const PString & str     // String to be encoded to MD5
    );
    static PString Encode(
      const char * cstr       // C String to be encoded to MD5
    );
    static PString Encode(
      const PBYTEArray & data // Data block to be encoded to MD5
    );
    static PString Encode(
      PConstMemoryPointer dataBlock,  // Pointer to data to be encoded to MD5
      PINDEX length                   // Length of the data block.
    );
    // Encode the data in memory to Base 64 data returnin the string.


    void StartDecoding();
    // Begin a base 64 decoding operation, initialising the object instance.

    BOOL ProcessDecoding(
      const PString & str      // String to be encoded
    );
    BOOL ProcessDecoding(
      const char * cstr        // C String to be encoded
    );
    /* Incorporate the specified data into the base 64 decoding.
    
       <H2>Returns:</H2>
       TRUE if block was last in the Base64 encoded string.
     */

    PBYTEArray GetDecodedData();
    /* Get the data decoded so far from the Base64 strings processed.
    
       <H2>Returns:</H2>
       Decoded data for the processed Base64 string.
     */

    BOOL IsDecodeOK() { return perfectDecode; }
    /* Return a flag to indicate that the input was decoded without any
       extraneous or illegal characters in it that were ignored. This does not
       mean that the data is not valid, only that it is suspect.
    
       <H2>Returns:</H2>
       Decoded data for the processed Base64 string.
     */


    static BOOL Decode(
      const PString & str, // Encoded base64 string to be decoded.
      PBYTEArray & data    // Converted binary data from base64.
    );
    static PString Decode(
      const PString & str // Encoded base64 string to be decoded.
    );
    /* Convert a printable text string to binary data using the Internet MIME
       standard base 64 content transfer encoding.

       The base64 string is checked and TRUE returned if all perfectly correct.
       If FALSE is returned then the string had extraneous or illegal
       characters in it that were ignored. This does not mean that the data is
       not valid, only that it is suspect.
    
       <H2>Returns:</H2>
       Base 64 string decoded from input string.
     */



  private:
    void OutputBase64(const BYTE * data);

    PString encodedString;
    PINDEX  encodeLength;
    BYTE    saveTriple[3];
    PINDEX  saveCount;
    PINDEX  nextLine;
    BOOL    useCRLFs;

    BOOL       perfectDecode;
    PINDEX     quadPosition;
    PBYTEArray decodedData;
    PINDEX     decodeSize;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
