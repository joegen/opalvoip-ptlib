/*
 * textfile.h
 *
 * A text file I/O channel class.
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
 * $Log: textfile.h,v $
 * Revision 1.13  1998/09/23 06:21:39  robertj
 * Added open source copyright license.
 *
 * Revision 1.12  1995/07/31 12:15:49  robertj
 * Removed PContainer from PChannel ancestor.
 *
 * Revision 1.11  1995/06/17 11:13:34  robertj
 * Documentation update.
 *
 * Revision 1.10  1995/03/14 12:42:48  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.9  1995/01/14  06:19:42  robertj
 * Documentation
 *
 * Revision 1.8  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.7  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.6  1994/04/20  12:17:44  robertj
 * PFilePath addition
 *
 * Revision 1.5  1994/04/01  14:17:26  robertj
 * Fixed container for text file.
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

#define _PTEXTFILE

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// Text Files

PDECLARE_CLASS(PTextFile, PFile)
/* A class representing a a structured file that is portable accross CPU
   architectures. Essentially this will normalise the end of line character
   which differs fromplatform to platform.
 */

  public:
    PTextFile();
    /* Create a text file object but do not open it. It does not initially
       have a valid file name. However, an attempt to open the file using the
       <A>PFile::Open()</A> function will generate a unique temporary file.
     */

    PTextFile(
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
      
    PTextFile(
      const PFilePath & name,    // Name of file to open.
      OpenMode mode = ReadWrite, // Mode in which to open the file.
      int opts = ModeDefault     // <A>OpenOptions enum</A> for open operation.
    );
    /* Create a text file object with the specified name and open it in the
       specified mode and with the specified options.

       The <A>PChannel::IsOpen()</A> function may be used after object
       construction to determine if the file was successfully opened.
     */


  // New member functions
    BOOL ReadLine(PString & str);
    /* Read a line from the text file. What constitutes an end of line in the
       file is platform dependent.
       
       Use the <A>PChannel::GetLastError()</A> function to determine if there
       was some error other than end of file.
       
       <H2>Returns:</H2>
       TRUE if successful, FALSE if at end of file or a read error.
     */

    BOOL WriteLine(const PString & str);
    /* Read a line from the text file. What constitutes an end of line in the
       file is platform dependent.
       
       Use the <A>PChannel::GetLastError()</A> function to determine the
       failure mode.

       <H2>Returns:</H2>
       TRUE if successful, FALSE if an error occurred.
     */


// Class declaration continued in platform specific header file ///////////////
