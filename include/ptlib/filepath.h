/*
 * filepath.h
 *
 * File system path string abstraction class.
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
 * $Log: filepath.h,v $
 * Revision 1.12  1998/09/23 06:20:37  robertj
 * Added open source copyright license.
 *
 * Revision 1.11  1998/02/16 00:14:57  robertj
 * Added functions to validate characters in a filename.
 *
 * Revision 1.10  1995/07/31 12:03:37  robertj
 * Added copy constructor and assignment operator for right types.
 *
 * Revision 1.9  1995/04/22 00:43:43  robertj
 * Added Move() function and changed semantics of Rename().
 * Changed all file name strings to PFilePath objects.
 *
 * Revision 1.8  1995/03/14 12:41:25  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.7  1994/12/21  11:52:57  robertj
 * Documentation and variable normalisation.
 *
 * Revision 1.6  1994/10/24  00:06:58  robertj
 * Changed PFilePath and PDirectory so descends from either PString or
 *     PCaselessString depending on the platform.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/08/21  23:43:02  robertj
 * Changed parameter before variable argument list to NOT be a reference.
 *
 * Revision 1.2  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.1  1994/04/20  12:17:44  robertj
 * Initial revision
 *
 */


#define _PFILEPATH

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// File Specification

PDECLARE_CLASS(PFilePath, PFILE_PATH_STRING)
/* This class describes a full description for a file on the particular
   platform. This will always uniquely identify the file on currently mounted
   volumes.

   The ancestor class is dependent on the platform. For file systems that are
   case sensitive, eg Unix, the ancestor is <A>PString</A>. For other
   platforms, the ancestor class is <A>PCaselessString</A>.
 */

  public:
    PFilePath();
    PFilePath(
      const char * cstr   // Partial C string for file name.
    );
    PFilePath(
      const PString & str // Partial PString for file name.
    );
    PFilePath(
      const PFilePath & path // Previous path for file name.
    );
    /* Create a file specification object with the specified file name.
    
       The string passed in may be a full or partial specifiaction for a file
       as determined by the platform. It is unusual for this to be a literal
       string, unless only the file title is specified, as that would be
       platform specific.

       The partial file specification is translated into a canonical form
       which always absolutely references the file.
     */

    PFilePath(
      const char * prefix,  // Prefix string for file title.
      const char * dir      // Directory in which to place the file.
    );
    /* Create a file spec object with a generated temporary name. The first
       parameter is a prefix for the filename to which a unique number is
       appended. The second parameter is the directory in which the file is to
       be placed. If this is NULL a system standard directory is used.
     */

    PFilePath & operator=(
      const char * cstr // Partial "C" string for file name.
    );
    PFilePath & operator=(
      const PString & str // Partial PString for file name.
    );
    PFilePath & operator=(
      const PFilePath & path // Previous path for file name.
    );
    /* Change the file specification object to the specified file name.

       The string passed in may be a full or partial specifiaction for a file
       as determined by the platform. It is unusual for this to be a literal
       string, unless only the file title is specified, as that would be
       platform specific.

       The partial file specification is translated into a canonical form
       which always absolutely references the file.
     */


  // New member functions
    PFILE_PATH_STRING GetVolume() const;
    /* Get the drive/volume name component of the full file specification. This
       is very platform specific. For example in DOS & NT it is the drive
       letter followed by a colon ("C:"), for Macintosh it is the volume name
       ("Untitled") and for Unix it is empty ("").
       
       <H2>Returns:</H2>
       string for the volume name part of the file specification..
     */
      
    PFILE_PATH_STRING GetPath() const;
    /* Get the directory path component of the full file specification. This
       will include leading and trailing directory separators. For example
       on DOS this could be "\SRC\PWLIB\", for Macintosh ":Source:PwLib:" and
       for Unix "/users/equivalence/src/pwlib/".

       <H2>Returns:</H2>
       string for the path part of the file specification.
     */

    PFILE_PATH_STRING GetTitle() const;
    /* Get the title component of the full file specification, eg for the DOS
       file "C:\SRC\PWLIB\FRED.DAT" this would be "FRED".

       <H2>Returns:</H2>
       string for the title part of the file specification.
     */

    PFILE_PATH_STRING GetType() const;
    /* Get the file type of the file. Note that on some platforms this may
       actually be part of the full name string. eg for DOS file
       "C:\SRC\PWLIB\FRED.TXT" this would be ".TXT" but on the Macintosh this
       might be "TEXT".

       Note there are standard translations from file extensions, eg ".TXT"
       and some Macintosh file types, eg "TEXT".

       <H2>Returns:</H2>
       string for the type part of the file specification.
     */

    PFILE_PATH_STRING GetFileName() const;
    /* Get the actual directory entry name component of the full file
       specification. This may be identical to
       <CODE>GetTitle() + GetType()</CODE> or simply <CODE>GetTitle()</CODE>
       depending on the platform. eg for DOS file "C:\SRC\PWLIB\FRED.TXT" this
       would be "FRED.TXT".

       <H2>Returns:</H2>
       string for the file name part of the file specification.
     */

    PDirectory GetDirectory() const;
    /* Get the the directory that the file is contained in.  This may be 
       identical to <CODE>GetVolume() + GetPath()</CODE> depending on the 
       platform. eg for DOS file "C:\SRC\PWLIB\FRED.TXT" this would be 
       "C:\SRC\PWLIB\".

       Note that for Unix platforms, this returns the <EM>physical</EM> path
       of the directory. That is all symlinks are resolved. Thus the directory
       returned may not be the same as the value of <CODE>GetPath()</CODE>.

       <H2>Returns:</H2>
       Directory that the file is contained in.
     */

    void SetType(
      const PFILE_PATH_STRING & type  // New type of the file.
    );
    /* Set the type component of the full file specification, eg for the DOS
       file "C:\SRC\PWLIB\FRED.DAT" would become "C:\SRC\PWLIB\FRED.TXT".
     */


    static BOOL IsValid(
      char c
    );
    static BOOL IsValid(
      const PString & str
    );
    /* Test if the character is valid in a filename.

       <H2>Returns:</H2>
       TRUE if the character is valid for a filename.
     */


// Class declaration continued in platform specific header file ///////////////
