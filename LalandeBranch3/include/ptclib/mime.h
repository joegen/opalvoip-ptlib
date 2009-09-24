/*
 * mime.h
 *
 * Multipurpose Internet Mail Extensions support classes.
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

#ifndef PTLIB_PMIME_H
#define PTLIB_PMIME_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/inetprot.h>
#include <ptclib/cypher.h>

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
      istream &strm   ///< Stream to read the objects contents from.
    );
    PMIMEInfo(
      PInternetProtocol & socket   ///< Application socket to read MIME info.
    );
    // Construct a MIME information dictionary from the specified source.


  // Overrides from class PObject
    /** Output the contents of the MIME dictionary to the stream. This is
       primarily used by the standard ##operator<<## function.
     */
    virtual void PrintOn(
      ostream &strm   ///< Stream to print the object into.
    ) const;

    /** Input the contents of the MIME dictionary from the stream. This is
       primarily used by the standard ##operator>>## function.
     */
    virtual void ReadFrom(
      istream &strm   ///< Stream to read the objects contents from.
    );


  // Overrides from class PStringToString
    /**Add a new value to the MIME info. If the value is already in the
       dictionary then this overrides the previous value.

       @return
       PTrue if the object was successfully added.
     */
    PBoolean SetAt(
      const char * key,
      const PString value
    ) { return AbstractSetAt(PCaselessString(key), PNEW PString(value)); }

    /**Add a new value to the MIME info. If the value is already in the
       dictionary then this overrides the previous value.

       @return
       PTrue if the object was successfully added.
     */
    PBoolean SetAt(
      const PString & key,
      const PString value
    ) { return AbstractSetAt(PCaselessString(key), PNEW PString(value)); }

    /**Add a new value to the MIME info. If the value is already in the
       dictionary then this overrides the previous value.

       @return
       PTrue if the object was successfully added.
     */
    PBoolean SetAt(
      const PCaselessString & key,
      const PString value
    ) { return AbstractSetAt(PCaselessString(key), PNEW PString(value)); }

    /** Determine if the specified key is present in the MIME information
       set.

       @return
       PTrue if the MIME variable is present.
     */
    PBoolean Contains(
      const char * key       ///< Key into MIME dictionary to get info.
    ) const { return GetAt(PCaselessString(key)) != NULL; }

    /** Determine if the specified key is present in the MIME information
       set.

       @return
       PTrue if the MIME variable is present.
     */
    PBoolean Contains(
      const PString & key       ///< Key into MIME dictionary to get info.
    ) const { return GetAt(PCaselessString(key)) != NULL; }

    /** Determine if the specified key is present in the MIME information
       set.

       @return
       PTrue if the MIME variable is present.
     */
    PBoolean Contains(
      const PCaselessString & key       ///< Key into MIME dictionary to get info.
    ) const { return GetAt(key) != NULL; }

  // New functions for class.
    /** Read MIME information from the socket.

       @return
       PTrue if the MIME information was successfully read.
     */
    PBoolean Read(
      PInternetProtocol & socket   ///< Application socket to read MIME info.
    );

    /** Write MIME information to the socket.

       @return
       PTrue if the MIME information was successfully read.
     */
    PBoolean Write(
      PInternetProtocol & socket   ///< Application socket to write MIME info.
    ) const;

    /**Add a MIME field given a "name: value" format string.
       Note that if the field name was already in the MIME dictionary then
       this will append the new value after a '\n' character to the previous
       value.

       @return
       PTrue is a field was added.
      */
    PBoolean AddMIME(
      const PString & line
    );
    PBoolean AddMIME(
      const PString & fieldName, 
      const PString & _fieldValue
    );

    /** Get a string for the particular MIME info field with checking for
       existance. The #dflt# parameter is substituted if the field
       does not exist in the MIME information read in.

       @return
       String for the value of the MIME variable.
     */
    PString GetString(
      const PString & key,       ///< Key into MIME dictionary to get info.
      const PString & dflt = PString::Empty() ///< Default value of field if not in MIME info.
    ) const;

    /** Get an integer value for the particular MIME info field with checking
       for existance. The #dflt# parameter is substituted if the
       field does not exist in the MIME information read in.

       @return
       Integer value for the MIME variable.
     */
    long GetInteger(
      const PString & key,    ///< Key into MIME dictionary to get info.
      long dflt = 0           ///< Default value of field if not in MIME info.
    ) const;

    /** Set an integer value for the particular MIME info field.
     */
    void SetInteger(
      const PCaselessString & key,  ///< Key into MIME dictionary to get info.
      long value                    ///< New value of field.
    );


    /** Set an association between a file type and a MIME content type. The
       content type is then sent for any file in the directory sub-tree that
       has the same extension.

       Note that if the #merge# parameter if PTrue then the
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
      const PStringToString & allTypes,  ///< MIME content type associations.
      PBoolean merge = PTrue                  ///< Flag for merging associations.
    );
    static void SetAssociation(
      const PString & fileType,         ///< File type (extension) to match.
      const PString & contentType       ///< MIME content type string.
    ) { GetContentTypes().SetAt(fileType, contentType); }

    /** Look up the file type to MIME content type association dictionary and
       return the MIME content type string. If the file type is not found in
       the dictionary then the string "application/octet-stream" is returned.

       @return
       MIME content type for file type.
     */
    static PString GetContentType(
      const PString & fileType   ///< File type (extension) to look up.
    );

  private:
    static PStringToString & GetContentTypes();
};


#endif // PTLIB_PMIME_H


// End Of File ///////////////////////////////////////////////////////////////
