/*
 * $Id: cypher.h,v 1.9 1997/10/10 10:44:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: cypher.h,v $
 * Revision 1.9  1997/10/10 10:44:01  robertj
 * Fixed bug in password encryption, missing string terminator.
 *
 * Revision 1.8  1996/11/16 10:50:24  robertj
 * Fixed bug in registration order form showing incorrect check code when have key.
 *
 * Revision 1.7  1996/07/15 10:29:38  robertj
 * Changed memory block cypher conversion functions to be void *.
 * Changed key types to be structures rather than arrays to avoid pinter/reference confusion by compilers.
 *
 * Revision 1.6  1996/03/17 05:47:00  robertj
 * Changed secured config to allow for expiry dates.
 *
 * Revision 1.5  1996/03/16 04:36:43  robertj
 * Redesign of secure config to accommodate expiry dates and option values passed in security key code.
 *
 * Revision 1.4  1996/02/25 02:52:46  robertj
 * Further secure config development.
 *
 * Revision 1.3  1996/01/28 14:16:11  robertj
 * Further implementation of secure config.
 *
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
      const void * dataBlock,  // Pointer to data to be part of the MD5
      PINDEX length            // Length of the data block.
    );
    // Incorporate the specified data into the message digest.

    class Code {
      private:
        PUInt32l value[4];
      friend class PMessageDigest5;
    };

    PString Complete();
    void Complete(
      Code & result   // The resultant 128 bit MD5 code
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
      Code & result            // The resultant 128 bit MD5 code
    );
    static PString Encode(
      const char * cstr        // C String to be encoded to MD5
    );
    static void Encode(
      const char * cstr,       // C String to be encoded to MD5
      Code & result            // The resultant 128 bit MD5 code
    );
    static PString Encode(
      const PBYTEArray & data  // Data block to be encoded to MD5
    );
    static void Encode(
      const PBYTEArray & data, // Data block to be encoded to MD5
      Code & result            // The resultant 128 bit MD5 code
    );
    static PString Encode(
      const void * dataBlock,  // Pointer to data to be encoded to MD5
      PINDEX length            // Length of the data block.
    );
    static void Encode(
      const void * dataBlock,  // Pointer to data to be encoded to MD5
      PINDEX length,           // Length of the data block.
      Code & result            // The resultant 128 bit MD5 code
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
    enum BlockChainMode {
      ElectronicCodebook,
        ECB = ElectronicCodebook,
      CypherBlockChaining,
        CBC = CypherBlockChaining,
      OutputFeedback,
        OFB = OutputFeedback,
      CypherFeedback,
        CFB = CypherFeedback,
      NumBlockChainModes
    };

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
      const PString & cypher   // Base64 Cypher text string to be decoded.
    );
    BOOL Decode(
      const PString & cypher,  // Base64 Cypher text string to be decoded.
      PString & clear          // Clear text string decoded.
    );
    BOOL Decode(
      const PString & cypher,  // Base64 Cypher text string to be decoded.
      PBYTEArray & clear       // Clear text binary data decoded.
    );
    PINDEX Decode(
      const PString & cypher,  // Base64 Cypher text string to be decoded.
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
      PINDEX blockSize,          // Size of encryption blocks (in bits)
      BlockChainMode chainMode   // Block chain mode
    );
    PCypher(
      const void * keyData,    // Key for the encryption/decryption algorithm.
      PINDEX keyLength,        // Length of the key.
      PINDEX blockSize,        // Size of encryption blocks (in bits)
      BlockChainMode chainMode // Block chain mode
    );
    /* Create a new encryption object instance.
     */


    virtual void Initialise(
      BOOL encoding   // Flag for encoding/decoding sequence about to start.
    ) = 0;
    // Initialise the encoding/decoding sequence.

    virtual void EncodeBlock(
      const void * in,    // Pointer to clear n bit block.
      void * out          // Pointer to coded n bit block.
    ) = 0;
    // Encode an n bit block of memory according to the encryption algorithm.


    virtual void DecodeBlock(
      const void * in,  // Pointer to coded n bit block.
      void * out        // Pointer to clear n bit block.
    ) = 0;
    // Dencode an n bit block of memory according to the encryption algorithm.


    PBYTEArray key;
    PINDEX blockSize;
    BlockChainMode chainMode;
};


PDECLARE_CLASS(PTEACypher, PCypher)
/* This class implements the Tiny Encryption Algorithm by David Wheeler and
   Roger Needham at Cambridge University.

   This is a simple algorithm using a 128 bit binary key and encrypts data in
   64 bit blocks.
 */

  public:
    struct Key {
      BYTE value[16];
    };

    PTEACypher(
      BlockChainMode chainMode = ElectronicCodebook
    );
    PTEACypher(
      const Key & keyData,     // Key for the encryption/decryption algorithm.
      BlockChainMode chainMode = ElectronicCodebook   // Block chain mode
    );
    /* Create a new TEA encryption object instance. The parameterless version
       automatically generates a new, random, key.
     */


    void SetKey(
      const Key & newKey    // Variable to take the key used by cypher.
    );
    // Set the key used by this encryption method.

    void GetKey(
      Key & newKey    // Variable to take the key used by cypher.
    ) const;
    // Get the key used by this encryption method.


    static void GenerateKey(
      Key & newKey    // Variable to take the newly generated key.
    );
    // Generate a new key suitable for use for encryption using random data.


  protected:
    virtual void Initialise(
      BOOL encoding   // Flag for encoding/decoding sequence about to start.
    );
    // Initialise the encoding/decoding sequence.

    virtual void EncodeBlock(
      const void * in,  // Pointer to clear n bit block.
      void * out        // Pointer to coded n bit block.
    );
    // Encode an n bit block of memory according to the encryption algorithm.

    virtual void DecodeBlock(
      const void * in,  // Pointer to coded n bit block.
      void * out        // Pointer to clear n bit block.
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
      const PTEACypher::Key & productKey,    // Key to decrypt validation code.
      const PStringArray    & securedKeys,   // List of secured keys.
      Source src = Application        // Standard source for the configuration.
    );
    PSecureConfig(
      const PTEACypher::Key & productKey,   // Key to decrypt validation code.
      const char * const * securedKeyArray, // List of secured keys.
      PINDEX count,                         // Number of secured keys in list.
      Source src = Application        // Standard source for the configuration.
    );
    /* Create a secured configuration. The default section for the
       configuration keys is "Secured Options", the default security key is
       "Validation" and the defualt prefix string is "Pending:".

       The user can descend from this class and change any of the member
       variable for the names of keys or the configuration file section.
     */


  // New functions for class
    const PStringArray & GetSecuredKeys() const { return securedKeys; }
    /* Get the list of secured keys in the configuration file section.

       <H2>Returns:</H2>
       Array of  strings for the secured keys.
     */

    const PString & GetSecurityKey() const { return securityKey; }
    /* Get the security keys name in the configuration file section.

       <H2>Returns:</H2>
       String for the security values key.
     */

    const PString & GetExpiryDateKey() const { return expiryDateKey; }
    /* Get the expiry date keys name in the configuration file section.

       <H2>Returns:</H2>
       String for the expiry date values key.
     */

    const PString & GetOptionBitsKey() const { return optionBitsKey; }
    /* Get the Option Bits keys name in the configuration file section.

       <H2>Returns:</H2>
       String for the Option Bits values key.
     */

    const PString & GetPendingPrefix() const { return pendingPrefix; }
    /* Get the pending prefix name in the configuration file section.

       <H2>Returns:</H2>
       String for the pending prefix.
     */

    void GetProductKey(
      PTEACypher::Key & productKey  // Variable to receive the product key.
    ) const;
    /* Get the pending prefix name in the configuration file section.

       <H2>Returns:</H2>
       String for the pending prefix.
     */


    enum ValidationState {
      Defaults,
      Pending,
      IsValid,
      Expired,
      Invalid
    };
    ValidationState GetValidation() const;
    /* Check the current values attached to the keys specified in the
       constructor against an encoded validation key.

       <H2>Returns:</H2>
       State of the validation keys.
     */

    BOOL ValidatePending();
    /* Validate a pending secured option list for the product. All secured
       keys with the <CODE>pendingPrefix</CODE> name will be checked against
       the value of the field <CODE>securityKey</CODE>. If they match then
       they are copied to the secured variables.

       <H2>Returns:</H2>
       TRUE if secure key values are valid.
     */

    void ResetPending();
    /* "Unvalidate" a security configuration going back to a pending state,
       usually used after an <CODE>Invalid</CODE> response was recieved from
       the <A>GetValidation()</A> function.
     */


  protected:
    PTEACypher::Key productKey;
    PStringArray    securedKeys;
    PString         securityKey;
    PString         expiryDateKey;
    PString         optionBitsKey;
    PString         pendingPrefix;
};


#endif // _PCYPHER


// End Of File ///////////////////////////////////////////////////////////////
