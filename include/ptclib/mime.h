/*
 * mime.h
 *
 * Multipurpose Internet Mail Extensions support classes.
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
 * $Log: mime.h,v $
 * Revision 1.15  2001/09/28 00:41:18  robertj
 * Added SetInteger() function to set numeric MIME fields.
 * Removed HasKey() as is confusing due to ancestor Contains().
 * Overrides of SetAt() and Contains() to assure PCaselessString used.
 *
 * Revision 1.14  2000/11/09 00:18:26  robertj
 * Cosmetic change: removed blank lines.
 *
 * Revision 1.13  1999/03/09 08:01:46  robertj
 * Changed comments for doc++ support (more to come).
 *
 * Revision 1.12  1999/02/16 08:07:10  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.11  1998/11/30 02:50:52  robertj
 * New directory structure
 *
 * Revision 1.10  1998/09/23 06:19:42  robertj
 * Added open source copyright license.
 *
 * Revision 1.9  1997/02/05 11:53:11  robertj
 * Changed construction of MIME dictionary to be delayed untill it is used.
 *
 * Revision 1.8  1996/09/14 13:09:15  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.7  1996/07/15 10:28:31  robertj
 * Changed memory block base64 conversion functions to be void *.
 *
 * Revision 1.6  1996/03/16 04:38:09  robertj
 * Fixed bug in MIME write function, should be const.
 *
 * Revision 1.5  1996/02/25 03:04:32  robertj
 * Added decoding of Base64 to a block of memory instead of PBYTEArray.
 *
 * Revision 1.4  1996/01/28 14:14:30  robertj
 * Further implementation of secure config.
 *
 * Revision 1.3  1996/01/28 02:46:07  robertj
 * Removal of MemoryPointer classes as usage didn't work for GNU.
 *
 * Revision 1.2  1996/01/26 02:24:27  robertj
 * Further implemetation.
 *
 * Revision 1.1  1996/01/23 13:06:18  robertj
 * Initial revision
 *
 */

#ifndef _PMIME
#define _PMIME

#ifdef __GNUC__
#pragma interface
#endif

#include <ptclib/inetprot.h>


//////////////////////////////////////////////////////////////////////////////
// PMIMEInfo

/** This class contains the Multipurpose Internet Mail Extensions parameters
   and variables.
 */

#ifdef DOC_PLUS_PLUS
class PMIMEInfo : public PStringToString {
#endif
PDECLARE_STRING_DICTIONARY(PMIMEInfo, PCaselessString);
  public:
    PMIMEInfo(
      istream &strm   // Stream to read the objects contents from.
    );
    PMIMEInfo(
      PInternetProtocol & socket   // Application socket to read MIME info.
    );
    // Construct a MIME infromation dictionary from the specified source.


  // Overrides from class PObject
    /** Output the contents of the MIME dictionary to the stream. This is
       primarily used by the standard ##operator<<## function.
     */
    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;

    /** Input the contents of the MIME dictionary from the stream. This is
       primarily used by the standard ##operator>>## function.
     */
    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );


  // Overrides from class PStringToString
    /**Add a new value to the MIME info. If the value is already in the
       dictionary then this overrides the previous value.

       @return
       TRUE if the object was successfully added.
     */
    BOOL SetAt(
      const char * key,
      const PString value
    ) { return AbstractSetAt(PCaselessString(key), PNEW PString(value)); }

    /**Add a new value to the MIME info. If the value is already in the
       dictionary then this overrides the previous value.

       @return
       TRUE if the object was successfully added.
     */
    BOOL SetAt(
      const PString & key,
      const PString value
    ) { return AbstractSetAt(PCaselessString(key), PNEW PString(value)); }

    /**Add a new value to the MIME info. If the value is already in the
       dictionary then this overrides the previous value.

       @return
       TRUE if the object was successfully added.
     */
    BOOL SetAt(
      const PCaselessString & key,
      const PString value
    ) { return AbstractSetAt(PCaselessString(key), PNEW PString(value)); }

    /** Determine if the specified key is present in the MIME information
       set.

       @return
       TRUE if the MIME variable is present.
     */
    BOOL Contains(
      const char * key       // Key into MIME dictionary to get info.
    ) const { return GetAt(PCaselessString(key)) != NULL; }

    /** Determine if the specified key is present in the MIME information
       set.

       @return
       TRUE if the MIME variable is present.
     */
    BOOL Contains(
      const PString & key       // Key into MIME dictionary to get info.
    ) const { return GetAt(PCaselessString(key)) != NULL; }

    /** Determine if the specified key is present in the MIME information
       set.

       @return
       TRUE if the MIME variable is present.
     */
    BOOL Contains(
      const PCaselessString & key       // Key into MIME dictionary to get info.
    ) const { return GetAt(key) != NULL; }

  // New functions for class.
    /** Read MIME information from the socket.

       @return
       TRUE if the MIME information was successfully read.
     */
    BOOL Read(
      PInternetProtocol & socket   // Application socket to read MIME info.
    );

    /** Write MIME information to the socket.

       @return
       TRUE if the MIME information was successfully read.
     */
    BOOL Write(
      PInternetProtocol & socket   // Application socket to write MIME info.
    ) const;

    /** Get a string for the particular MIME info field with checking for
       existance. The #dflt# parameter is substituted if the field
       does not exist in the MIME information read in.

       @return
       String for the value of the MIME variable.
     */
    PString GetString(
      const PString & key,       // Key into MIME dictionary to get info.
      const PString & dflt       // Default value of field if not in MIME info.
    ) const;

    /** Get an integer value for the particular MIME info field with checking
       for existance. The #dflt# parameter is substituted if the
       field does not exist in the MIME information read in.

       @return
       Integer value for the MIME variable.
     */
    long GetInteger(
      const PString & key,    // Key into MIME dictionary to get info.
      long dflt = 0           // Default value of field if not in MIME info.
    ) const;

    /** Set an integer value for the particular MIME info field.
     */
    void SetInteger(
      const PCaselessString & key,  // Key into MIME dictionary to get info.
      long value                    // New value of field.
    );


    /** Set an association between a file type and a MIME content type. The
       content type is then sent for any file in the directory sub-tree that
       has the same extension.

       Note that if the #merge# parameter if TRUE then the
       dictionary is merged into the current association list and is not a
       simple replacement.

       The default values placed in this dictionary are:
\begin{verbatim}

          ".txt", "text/plain"
          ".text", "text/plain"
          ".html", "text/html"
          ".htm", "text/html"
          ".aif", "audio/aiff"
          ".aiff", "audio/aiff"
          ".au", "audio/basic"
          ".snd", "audio/basic"
          ".wav", "audio/wav"
          ".gif", "image/gif"
          ".xbm", "image/x-bitmap"
          ".tif", "image/tiff"
          ".tiff", "image/tiff"
          ".jpg", "image/jpeg"
          ".jpe", "image/jpeg"
          ".jpeg", "image/jpeg"
          ".avi", "video/avi"
          ".mpg", "video/mpeg"
          ".mpeg", "video/mpeg"
          ".qt", "video/quicktime"
          ".mov", "video/quicktime"
\end{verbatim}


       The default content type will be "application/octet-stream".
     */
    static void SetAssociation(
      const PStringToString & allTypes,  // MIME content type associations.
      BOOL merge = TRUE                  // Flag for merging associations.
    );
    static void SetAssociation(
      const PString & fileType,         // File type (extension) to match.
      const PString & contentType       // MIME content type string.
    ) { GetContentTypes().SetAt(fileType, contentType); }

    /** Look up the file type to MIME content type association dictionary and
       return the MIME content type string. If the file type is not found in
       the dictionary then the string "application/octet-stream" is returned.

       @return
       MIME content type for file type.
     */
    static PString GetContentType(
      const PString & fileType   // File type (extension) to look up.
    );

  private:
    static PStringToString & GetContentTypes();
};


/** This class is used to encode/decode data using the MIME standard base64
   encoding mechanism as defined in RFC1521.

   To encode a large block of data use the following seqeunce:
\begin{verbatim}
      PBase64 base;
      base.StartEncoding();
      while (Read(dataChunk)) {
        base.ProcessEncoding(dataChunk);
        out << base.GetEncodedString();
      }
      out << base.CompleteEncoding();
\end{verbatim}

    if smaller blocks that fit easily in memory are to be encoded the
    #Encode()# functions can be used to everything in one go.

    To decode a large block of data use the following sequence:
\begin{verbatim}

      PBase64 base;
      base.StartDecoding();
      while (Read(str) && ProcessDecoding(str))
        Write(base.GetDecodedData());
      Write(base.GetDecodedData());
\end{verbatim}

    if smaller blocks that fit easily in memory are to be decoded the
    #Decode()# functions can be used to everything in one go.
 */
class PBase64 : public PObject
{
  PCLASSINFO(PBase64, PObject);

  public:
    /** Construct a base 64 encoder/decoder and initialise both encode and
       decode members as in #StartEncoding()# and #StartDecoding()#.
     */
    PBase64();

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
      const void * dataBlock,  // Pointer to data to be encoded
      PINDEX length            // Length of the data block.
    );
    // Incorporate the specified data into the base 64 encoding.

    /** Get the partial Base64 string for the data encoded so far.
    
       @return
       Base64 encoded string for the processed data.
     */
    PString GetEncodedString();

    /** Complete the base 64 encoding and return the remainder of the encoded
       Base64 string. Previous data may have been already removed by the
       #GetInterim()# function.
    
       @return
       Base64 encoded string for the processed data.
     */
    PString CompleteEncoding();


    static PString Encode(
      const PString & str     // String to be encoded to Base64
    );
    static PString Encode(
      const char * cstr       // C String to be encoded to Base64
    );
    static PString Encode(
      const PBYTEArray & data // Data block to be encoded to Base64
    );
    static PString Encode(
      const void * dataBlock, // Pointer to data to be encoded to Base64
      PINDEX length           // Length of the data block.
    );
    // Encode the data in memory to Base 64 data returnin the string.


    void StartDecoding();
    // Begin a base 64 decoding operation, initialising the object instance.

    /** Incorporate the specified data into the base 64 decoding.
    
       @return
       TRUE if block was last in the Base64 encoded string.
     */
    BOOL ProcessDecoding(
      const PString & str      // String to be encoded
    );
    BOOL ProcessDecoding(
      const char * cstr        // C String to be encoded
    );

    /** Get the data decoded so far from the Base64 strings processed.
    
       @return
       Decoded data for the processed Base64 string.
     */
    BOOL GetDecodedData(
      void * dataBlock,    // Pointer to data to be decoded from base64
      PINDEX length        // Length of the data block.
    );
    PBYTEArray GetDecodedData();

    /** Return a flag to indicate that the input was decoded without any
       extraneous or illegal characters in it that were ignored. This does not
       mean that the data is not valid, only that it is suspect.
    
       @return
       Decoded data for the processed Base64 string.
     */
    BOOL IsDecodeOK() { return perfectDecode; }


    /** Convert a printable text string to binary data using the Internet MIME
       standard base 64 content transfer encoding.

       The base64 string is checked and TRUE returned if all perfectly correct.
       If FALSE is returned then the string had extraneous or illegal
       characters in it that were ignored. This does not mean that the data is
       not valid, only that it is suspect.
    
       @return
       Base 64 string decoded from input string.
     */
    static PString Decode(
      const PString & str // Encoded base64 string to be decoded.
    );
    static BOOL Decode(
      const PString & str, // Encoded base64 string to be decoded.
      PBYTEArray & data    // Converted binary data from base64.
    );
    static BOOL Decode(
      const PString & str, // Encoded base64 string to be decoded.
      void * dataBlock,    // Pointer to data to be decoded from base64
      PINDEX length        // Length of the data block.
    );



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
