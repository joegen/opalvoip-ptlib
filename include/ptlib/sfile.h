/*
 * sfile.h
 *
 * Structured file I/O channel class.
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
 * $Log: sfile.h,v $
 * Revision 1.13  1998/09/24 07:24:01  robertj
 * Moved structured fiel into separate module so don't need silly implementation file for GNU C.
 *
 * Revision 1.12  1998/09/23 06:21:23  robertj
 * Added open source copyright license.
 *
 * Revision 1.11  1996/01/23 13:15:38  robertj
 * Mac Metrowerks compiler support.
 *
 * Revision 1.10  1995/06/17 11:13:19  robertj
 * Documentation update.
 *
 * Revision 1.9  1995/03/14 12:42:34  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.8  1995/01/14  06:19:39  robertj
 * Documentation
 *
 * Revision 1.7  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.6  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.5  1994/04/20  12:17:44  robertj
 * PFilePath split
 *
 * Revision 1.4  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.3  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


#define _PSTRUCTUREDFILE

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PStructuredFile, PFile)
/* A class representing a a structured file that is portable accross CPU
   architectures (as in the XDR protocol).
   
   This differs from object serialisation in that the access is always to a
   disk file and is random access. It would primarily be used for database
   type applications.
 */

  private:
    BOOL Read(void * buf, PINDEX len) { return PFile::Read(buf, len); }
    BOOL Write(const void * buf, PINDEX len) { return PFile::Write(buf, len); }

  public:
    PStructuredFile();
    /* Create a structured file object but do not open it. It does not
       initially have a valid file name. However, an attempt to open the file
       using the <A>PFile::Open()</A> function will generate a unique
       temporary file.
       
       The initial structure size is one byte.
     */

    PStructuredFile(
      OpenMode mode,          // Mode in which to open the file.
      int opts = ModeDefault  // <A>OpenOptions enum</A> for open operation.
    );
    /* Create a unique temporary file name, and open the file in the specified
       mode and using the specified options. Note that opening a new, unique,
       temporary file name in ReadOnly mode will always fail. This would only
       be usefull in a mode and options that will create the file.

       The <A>PChannel::IsOpen()</A> function may be used after object
       construction to determine if the file was successfully opened.
     */
      
    PStructuredFile(
      const PFilePath & name,    // Name of file to open.
      OpenMode mode = ReadWrite, // Mode in which to open the file.
      int opts = ModeDefault     // <A>OpenOptions enum</A> for open operation.
    );
    /* Create a structured file object with the specified name and open it in
       the specified mode and with the specified options.

       The <A>PChannel::IsOpen()</A> function may be used after object
       construction to determine if the file was successfully opened.
     */


  // New member functions
    BOOL Read(void * buffer);
    /* Read a sequence of bytes into the specified buffer, translating the
       structure according to the specification made in the
       <A>SetStructure()</A> function.

       <H2>Returns:</H2>
       TRUE if the structure was successfully read.
     */
      
    BOOL Write(void * buffer);
    /* Write a sequence of bytes into the specified buffer, translating the
       structure according to the specification made in the
       <A>SetStructure()</A> function.

       <H2>Returns:</H2>
       TRUE if the structure was successfully written.
     */


  // New member functions
    PINDEX GetStructureSize() { return structureSize; }
    /* Get the size of each structure in the file.

       <H2>Returns:</H2>
       number of bytes in a structure.
     */

    enum ElementType {
      Character,    // Element is a single character.
      Integer16,    // Element is a 16 bit integer.
      Integer32,    // Element is a 32 bit integer.
      Integer64,    // Element is a 64 bit integer.
      Float32,      // Element is a 32 bit IEE floating point number.
      Float64,      // Element is a 64 bit IEE floating point number.
      Float80,      // Element is a 80 bit IEE floating point number.
      NumElementTypes
    };
    // All element types ina structure

    struct Element {
      ElementType type;   // Type of element in structure.
      PINDEX      count;  // Count of elements of this type.
    };
    // Elements in the structure definition.

    void SetStructure(
      Element * structure,  // Array of structure elements
      PINDEX numElements    // Number of structure elements in structure.
    );
    // Set the structure of each record in the file.


  protected:
  // Member variables
    PINDEX structureSize;
    // Number of bytes in structure.

    Element * structure;
    PINDEX numElements;


// Class declaration continued in platform specific header file ///////////////
