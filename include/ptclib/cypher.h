/*
 * cypher.h
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
 * $Log: cypher.h,v $
 * Revision 1.15  2002/11/06 22:47:23  robertj
 * Fixed header comment (copyright etc)
 *
 * Revision 1.14  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.13  2001/09/10 00:28:21  robertj
 * Fixed extra CR in comments.
 *
 * Revision 1.12  1999/03/09 08:01:46  robertj
 * Changed comments for doc++ support (more to come).
 *
 * Revision 1.11  1999/02/16 08:07:10  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.10  1998/09/23 06:19:24  robertj
 * Added open source copyright license.
 *
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

#ifdef P_USE_PRAGMA
#pragma interface
#endif


/** MD5 Message Digest.
 A class to produce a Message Digest for a block of text/data using the
 MD5 algorithm as defined in RFC1321 by Ronald Rivest of MIT Laboratory
 for Computer Science and RSA Data Security, Inc.
 */
class PMessageDigest5 : public PObject
{
  PCLASSINFO(PMessageDigest5, PObject)

  public:
    /// Create a new message digestor
    PMessageDigest5();

    /// Begin a Message Digest operation, initialising the object instance.
    void Start();

    /** Incorporate the specified data into the message digest. */
    void Process(
      const PString & str      /// String to be part of the MD5
    );
    /** Incorporate the specified data into the message digest. */
    void Process(
      const char * cstr        /// C String to be part of the MD5
    );
    /** Incorporate the specified data into the message digest. */
    void Process(
      const PBYTEArray & data  /// Data block to be part of the MD5
    );
    /** Incorporate the specified data into the message digest. */
    void Process(
      const void * dataBlock,  /// Pointer to data to be part of the MD5
      PINDEX length            /// Length of the data block.
    );

    /// Resultant 128 bit digest of input data.
    class Code {
      private:
        PUInt32l value[4];
      friend class PMessageDigest5;
    };

    /**
    Complete the message digest and return the magic number result.
    The parameterless form returns the MD5 code as a Base64 string.
    
    @return
       Base64 encoded MD5 code for the processed data.
    */
    PString Complete();
    void Complete(
      Code & result   /// The resultant 128 bit MD5 code
    );


    /** Encode the data in memory to and MD5 hash value. */
    static PString Encode(
      const PString & str      /// String to be encoded to MD5
    );
    /** Encode the data in memory to and MD5 hash value. */
    static void Encode(
      const PString & str,     /// String to be encoded to MD5
      Code & result            /// The resultant 128 bit MD5 code
    );
    /** Encode the data in memory to and MD5 hash value. */
    static PString Encode(
      const char * cstr        /// C String to be encoded to MD5
    );
    /** Encode the data in memory to and MD5 hash value. */
    static void Encode(
      const char * cstr,       /// C String to be encoded to MD5
      Code & result            /// The resultant 128 bit MD5 code
    );
    /** Encode the data in memory to and MD5 hash value. */
    static PString Encode(
      const PBYTEArray & data  /// Data block to be encoded to MD5
    );
    /** Encode the data in memory to and MD5 hash value. */
    static void Encode(
      const PBYTEArray & data, /// Data block to be encoded to MD5
      Code & result            /// The resultant 128 bit MD5 code
    );
    /** Encode the data in memory to and MD5 hash value. */
    static PString Encode(
      const void * dataBlock,  /// Pointer to data to be encoded to MD5
      PINDEX length            /// Length of the data block.
    );
    /** Encode the data in memory to and MD5 hash value.
    
    @return
       Base64 encoded MD5 code for the processed data.
    */
    static void Encode(
      const void * dataBlock,  /// Pointer to data to be encoded to MD5
      PINDEX length,           /// Length of the data block.
      Code & result            /// The resultant 128 bit MD5 code
    );

  private:
    void Transform(const BYTE * block);

    /// input buffer
    BYTE buffer[64];
    /// state (ABCD)
    DWORD state[4];
    /// number of bits, modulo 2^64 (lsb first)
    PUInt64 count;
};



/**This abstract class defines an encryption/decryption algortihm.
A specific algorithm is implemented in a descendent class.
*/
class PCypher : public PObject
{
  PCLASSINFO(PCypher, PObject)

  public:
    /// Mechanism by which sequential blocks are linked.
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
    /**Encode the data. */
    PString Encode(
      const PString & str       /// Clear text string to be encoded.
    );
    /**Encode the data. */
    PString Encode(
      const PBYTEArray & clear  /// Clear text binary data to be encoded.
    );
    /**Encode the data. */
    PString Encode(
      const void * data,        /// Clear text binary data to be encoded.
      PINDEX length             /// Number of bytes of data to be encoded.
    );
    /**Encode the data. */
    void Encode(
      const PBYTEArray & clear, /// Clear text binary data to be encoded.
      PBYTEArray & coded        /// Encoded data.
    );
    /**Encode the data.
    The data is encoded using the algorithm embodied by the descendent class
    and the key specifed in the construction of the objects instance.

    The first form takes a string and returns an encoded string. The second
    form takes arbitrary binary data bytes and returns an encoded string. In
    both cases the encoded string is always 7 bit printable ASCII suitable
    for use in mail systems etc.

    The final form takes and arbitrary block of bytes and encodes them into
    another block of binary data.
    
    @return
      encoded string.
    */
    void Encode(
      const void * data,        // Clear text binary data to be encoded.
      PINDEX length,            // Number of bytes of data to be encoded.
      PBYTEArray & coded        // Encoded data.
    );

    /**Decode the data. */
    PString Decode(
      const PString & cypher   /// Base64 Cypher text string to be decoded.
    );
    /**Decode the data. */
    BOOL Decode(
      const PString & cypher,  /// Base64 Cypher text string to be decoded.
      PString & clear          /// Clear text string decoded.
    );
    /**Decode the data. */
    BOOL Decode(
      const PString & cypher,  /// Base64 Cypher text string to be decoded.
      PBYTEArray & clear       /// Clear text binary data decoded.
    );
    /**Decode the data. */
    PINDEX Decode(
      const PString & cypher,  /// Base64 Cypher text string to be decoded.
      void * data,             /// Clear text binary data decoded.
      PINDEX length            /// Maximum number of bytes of data decoded.
    );
    /**Decode the data. */
    PINDEX Decode(
      const PBYTEArray & coded, /// Encoded data (cyphertext).
      void * data,              /// Clear text binary data decoded.
      PINDEX length             /// Maximum number of bytes of data decoded.
    );
    /**Decode the data.
    Decode the data using the algorithm embodied by the descendent class
    and the key specifed in the construction of the objects instance.

    The first form takes a string and returns a decoded string. The second
    form takes an encoded string and returns arbitrary binary data bytes. In
    both cases the encoded string is always 7 bit printable ASCII suitable
    for use in mail systems etc.

    The final form takes and arbitrary block of bytes and decodes them into
    another block of binary data.
    
    @return
      decoded string.
    */
    BOOL Decode(
      const PBYTEArray & coded, /// Encoded data (cyphertext).
      PBYTEArray & clear       /// Clear text binary data decoded.
    );


  protected:
    /**
    Create a new encryption object instance.
    */
    PCypher(
      PINDEX blockSize,          /// Size of encryption blocks (in bits)
      BlockChainMode chainMode   /// Block chain mode
    );
    PCypher(
      const void * keyData,    /// Key for the encryption/decryption algorithm.
      PINDEX keyLength,        /// Length of the key.
      PINDEX blockSize,        /// Size of encryption blocks (in bits)
      BlockChainMode chainMode /// Block chain mode
    );


    /** Initialise the encoding/decoding sequence. */
    virtual void Initialise(
      BOOL encoding   /// Flag for encoding/decoding sequence about to start.
    ) = 0;

    /** Encode an n bit block of memory according to the encryption algorithm. */
    virtual void EncodeBlock(
      const void * in,    /// Pointer to clear n bit block.
      void * out          /// Pointer to coded n bit block.
    ) = 0;


    /** Dencode an n bit block of memory according to the encryption algorithm. */
    virtual void DecodeBlock(
      const void * in,  /// Pointer to coded n bit block.
      void * out        /// Pointer to clear n bit block.
    ) = 0;


    /// Key for the encryption/decryption.
    PBYTEArray key;
    /// Size of each encryption block in bytes
    PINDEX blockSize;
    /// Mode for sequential encryption each block
    BlockChainMode chainMode;
};


/** Tiny Encryption Algorithm.
This class implements the Tiny Encryption Algorithm by David Wheeler and
Roger Needham at Cambridge University.

This is a simple algorithm using a 128 bit binary key and encrypts data in
64 bit blocks.
*/
class PTEACypher : public PCypher
{
  PCLASSINFO(PTEACypher, PCypher)

  public:
    struct Key {
      BYTE value[16];
    };

    /**
    Create a new TEA encryption object instance. The parameterless version
    automatically generates a new, random, key.
    */
    PTEACypher(
      BlockChainMode chainMode = ElectronicCodebook   /// Block chain mode
    );
    PTEACypher(
      const Key & keyData,     /// Key for the encryption/decryption algorithm.
      BlockChainMode chainMode = ElectronicCodebook   /// Block chain mode
    );


    /** Set the key used by this encryption method. */
    void SetKey(
      const Key & newKey    /// Variable to take the key used by cypher.
    );

    /** Get the key used by this encryption method. */
    void GetKey(
      Key & newKey    /// Variable to take the key used by cypher.
    ) const;


    /** Generate a new key suitable for use for encryption using random data. */
    static void GenerateKey(
      Key & newKey    /// Variable to take the newly generated key.
    );


  protected:
    /** Initialise the encoding/decoding sequence. */
    virtual void Initialise(
      BOOL encoding   /// Flag for encoding/decoding sequence about to start.
    );

    /** Encode an n bit block of memory according to the encryption algorithm. */
    virtual void EncodeBlock(
      const void * in,  /// Pointer to clear n bit block.
      void * out        /// Pointer to coded n bit block.
    );

    /** Decode an n bit block of memory according to the encryption algorithm. */
    virtual void DecodeBlock(
      const void * in,  /// Pointer to coded n bit block.
      void * out        /// Pointer to clear n bit block.
    );

  private:
    DWORD k0, k1, k2, k3;
};



class PSecureConfig : public PConfig
{
  PCLASSINFO(PSecureConfig, PConfig)
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

       @return
       Array of  strings for the secured keys.
     */

    const PString & GetSecurityKey() const { return securityKey; }
    /* Get the security keys name in the configuration file section.

       @return
       String for the security values key.
     */

    const PString & GetExpiryDateKey() const { return expiryDateKey; }
    /* Get the expiry date keys name in the configuration file section.

       @return
       String for the expiry date values key.
     */

    const PString & GetOptionBitsKey() const { return optionBitsKey; }
    /* Get the Option Bits keys name in the configuration file section.

       @return
       String for the Option Bits values key.
     */

    const PString & GetPendingPrefix() const { return pendingPrefix; }
    /* Get the pending prefix name in the configuration file section.

       @return
       String for the pending prefix.
     */

    void GetProductKey(
      PTEACypher::Key & productKey  // Variable to receive the product key.
    ) const;
    /* Get the pending prefix name in the configuration file section.

       @return
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

       @return
       State of the validation keys.
     */

    BOOL ValidatePending();
    /* Validate a pending secured option list for the product. All secured
       keys with the <CODE>pendingPrefix</CODE> name will be checked against
       the value of the field <CODE>securityKey</CODE>. If they match then
       they are copied to the secured variables.

       @return
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
