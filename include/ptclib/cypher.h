/*
 * $Id: cypher.h,v 1.2 1996/01/28 02:41:00 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: cypher.h,v $
 * Revision 1.2  1996/01/28 02:41:00  robertj
 * Removal of MemoryPointer classes as usage didn't work for GNU.
 * Added the secure configuration mechanism for protecting applications.
 *
 * Revision 1.1  1996/01/23 13:04:20  robertj
 * Initial revision
 *
 */


#ifndef _PCYPHER
#define _PCYPHER

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PMessageDigest5, PObject)
/* A class to produce a Message Digest for a block of text/data using the
   MD5 algorithm as defined in RFC1321 by Ronald Rivest of MIT Laboratory
   for Computer Science and RSA Data Security, Inc.
 */

  public:
    PMessageDigest5();

    void Start();
    // Begin a Message Digest operation, initialising the object instance.

    void Process(
      const PString & str      // String to be part of the MD5
    );
    void Process(
      const char * cstr        // C String to be part of the MD5
    );
    void Process(
      const PBYTEArray & data  // Data block to be part of the MD5
    );
    void Process(
      const BYTE * dataBlock,  // Pointer to data to be part of the MD5
      PINDEX length            // Length of the data block.
    );
    // Incorporate the specified data into the message digest.

    typedef BYTE Code[16];

    PString Complete();
    void Complete(
      Code result   // The resultant 128 bit MD5 code
    );
    /* Complete the message digest and return the magic number result.
       The parameterless form returns the MD5 code as a Base64 string.
    
       <H2>Returns:</H2>
       Base64 encoded MD5 code for the processed data.
     */


    static PString Encode(
      const PString & str      // String to be encoded to MD5
    );
    static void Encode(
      const PString & str,     // String to be encoded to MD5
      Code result              // The resultant 128 bit MD5 code
    );
    static PString Encode(
      const char * cstr        // C String to be encoded to MD5
    );
    static void Encode(
      const char * cstr,       // C String to be encoded to MD5
      Code result              // The resultant 128 bit MD5 code
    );
    static PString Encode(
      const PBYTEArray & data  // Data block to be encoded to MD5
    );
    static void Encode(
      const PBYTEArray & data, // Data block to be encoded to MD5
      Code result              // The resultant 128 bit MD5 code
    );
    static PString Encode(
      const BYTE * dataBlock,  // Pointer to data to be encoded to MD5
      PINDEX length            // Length of the data block.
    );
    static void Encode(
      const BYTE * dataBlock,  // Pointer to data to be encoded to MD5
      PINDEX length,           // Length of the data block.
      Code result              // The resultant 128 bit MD5 code
    );
    /* Encode the data in memory to and MD5 hash value.
    
       <H2>Returns:</H2>
       Base64 encoded MD5 code for the processed data.
     */

  private:
    void Transform(const BYTE * block);

    BYTE buffer[64];  // input buffer
    DWORD state[4];   // state (ABCD)
    PUInt64 count;    // number of bits, modulo 2^64 (lsb first)
};



PDECLARE_CLASS(PCypher, PObject)
/* This abstract class defines an encryption/decryption algortihm. A
   specific algorithm is implemented in a descendent class.
 */

  public:
  // New functions for class
    PString Encode(
      const PString & str       // Clear text string to be encoded.
    );
    PString Encode(
      const PBYTEArray & clear  // Clear text binary data to be encoded.
    );
    PString Encode(
      const void * data,        // Clear text binary data to be encoded.
      PINDEX length             // Number of bytes of data to be encoded.
    );
    void Encode(
      const PBYTEArray & clear, // Clear text binary data to be encoded.
      PBYTEArray & coded        // Encoded data.
    );
    void Encode(
      const void * data,        // Clear text binary data to be encoded.
      PINDEX length,            // Number of bytes of data to be encoded.
      PBYTEArray & coded        // Encoded data.
    );
    /* Encode the data using the algorithm embodied by the descendent class
       and the key specifed in the construction of the objects instance.

       The first form takes a string and returns an encoded string. The second
       form takes arbitrary binary data bytes and returns an encoded string. In
       both cases the encoded string is always 7 bit printable ASCII suitable
       for use in mail systems etc.

       The final form takes and arbitrary block of bytes and encodes them into
       another block of binary data.
    
       <H2>Returns:</H2>
       encoded string.
     */

    PString Decode(
      const PString & cypher   // Cypher text string to be decoded.
    );
    BOOL Decode(
      const PString & cypher,  // Cypher text string to be decoded.
      PBYTEArray & clear       // Clear text binary data decoded.
    );
    PINDEX Decode(
      const PString & cypher,  // Cypher text string to be decoded.
      void * data,             // Clear text binary data decoded.
      PINDEX length            // Maximum number of bytes of data decoded.
    );
    PINDEX Decode(
      const PBYTEArray & coded, // Encoded data (cyphertext).
      void * data,              // Clear text binary data decoded.
      PINDEX length             // Maximum number of bytes of data decoded.
    );
    BOOL Decode(
      const PBYTEArray & coded, // Encoded data (cyphertext).
      PBYTEArray & clear       // Clear text binary data decoded.
    );
    /* Decode the data using the algorithm embodied by the descendent class
       and the key specifed in the construction of the objects instance.

       The first form takes a string and returns a decoded string. The second
       form takes an encoded string and returns arbitrary binary data bytes. In
       both cases the encoded string is always 7 bit printable ASCII suitable
       for use in mail systems etc.

       The final form takes and arbitrary block of bytes and decodes them into
       another block of binary data.
    
       <H2>Returns:</H2>
       decoded string.
     */


  protected:
    PCypher(
      PINDEX blockSize      // Size of encryption blocks (in bits)
    );
    PCypher(
      PINDEX blockSize,     // Size of encryption blocks (in bits)
      const BYTE * keyData, // Key for the encryption/decryption algorithm.
      PINDEX keyLength      // Length of the key.
    );
    /* Create a new encryption object instance.
     */


    virtual void Initialise(
      BOOL encoding   // Flag for encoding/decoding sequence about to start.
    ) = 0;
    // Initialise the encoding/decoding sequence.

    virtual void EncodeBlock(
      const BYTE * in,    // Pointer to clear n bit block.
      BYTE * out          // Pointer to coded n bit block.
    ) = 0;
    // Encode an n bit block of memory according to the encryption algorithm.


    virtual void DecodeBlock(
      const BYTE * in,  // Pointer to coded n bit block.
      BYTE * out        // Pointer to clear n bit block.
    ) = 0;
    // Dencode an n bit block of memory according to the encryption algorithm.


    PBYTEArray key;
    PINDEX blockSize;
};


PDECLARE_CLASS(PTEACypher, PCypher)
/* This class implements the Tiny Encryption Algorithm by David Wheeler and
   Roger Needham at Cambridge University.

   This is a simple algorithm using a 128 bit binary key and encrypts data in
   64 bit blocks.
 */

  public:
    typedef BYTE Key[16];

    PTEACypher();
    PTEACypher(
      const Key keyData  // Key for the encryption/decryption algorithm.
    );
    /* Create a new TEA encryption object instance. The parameterless version
       automatically generates a new, random, key.
     */


    void SetKey(
      const Key newKey    // Variable to take the key used by cypher.
    );
    // Set the key used by this encryption method.

    void GetKey(
      Key newKey    // Variable to take the key used by cypher.
    ) const;
    // Get the key used by this encryption method.


    static void GenerateKey(
      Key newKey    // Variable to take the newly generated key.
    );
    // Generate a new key suitable for use for encryption using random data.


  protected:
    virtual void Initialise(
      BOOL encoding   // Flag for encoding/decoding sequence about to start.
    );
    // Initialise the encoding/decoding sequence.

    virtual void EncodeBlock(
      const BYTE * in,  // Pointer to clear n bit block.
      BYTE * out        // Pointer to coded n bit block.
    );
    // Encode an n bit block of memory according to the encryption algorithm.

    virtual void DecodeBlock(
      const BYTE * in,  // Pointer to coded n bit block.
      BYTE * out        // Pointer to clear n bit block.
    );
    // Dencode an n bit block of memory according to the encryption algorithm.

  private:
    DWORD k0, k1, k2, k3;
};



PDECLARE_CLASS(PSecureConfig, PConfig)
/* This class defines a set of configuration keys which may be secured by an
   encrypted hash function. Thus values contained in keys specified by this
   class cannot be changed without invalidating the hash function.
 */

  public:
    PSecureConfig(
      const char * encryptPhrase,         // Phrase to be used for encryption.
      const char * const * securedKeys,   // List of secured keys.
      PINDEX count,                       // Number of secured keys in list.
      const char * securedSection = "Secured Options",
        // Section for secured key values
      Source src = Application      // Standard source for the configuration.
    );
    /* Create a secured configuration. 
     */


  // New functions for class
    void SetValidation(
      const char * validationKey = "Validation" // Key to store validation.
    );
    /* Set the configuration file key with an encoded validation of all of the
       values attached to the keys specified in the constructor.
     */

    BOOL IsValid(
      const char * validationKey = "Validation" // Key validation stored in.
    );
    /* Check the current values attached to the keys specified in the
       constructor against an encoded validation key.

       <H2>Returns:</H2>
       TRUE if secure key values are valid.
     */


  protected:
    PString CalculateValidation();

    PTEACypher::Key cryptKey;
    PStringArray    securedKey;
};


#endif // _PCYPHER


// End Of File ///////////////////////////////////////////////////////////////
