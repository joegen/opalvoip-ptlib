/*
 * $Id: filepath.h,v 1.7 1994/12/21 11:52:57 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: filepath.h,v $
 * Revision 1.7  1994/12/21 11:52:57  robertj
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
   case sensitive, eg Unix, the ancestor is $H$PString. For other platforms,
   the ancestor class is $H$PCaselessString.
 */

  public:
    PFilePath();
    PFilePath(
      const char * cstr   // Partial C string for file name.
    );
    PFilePath(
      const PString & str // Partial PString for file name.
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
      const PString & str // Partial PString for file name.
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
       
       Returns: string for the volume name part of the file specification..
     */
      
    PFILE_PATH_STRING GetPath() const;
    /* Get the directory path component of the full file specification. This
       will include leading and trailing directory separators. For example
       on DOS this could be "\SRC\PWLIB\", for Macintosh ":Source:PwLib:" and
       for Unix "/users/equivalence/src/pwlib/".

       Returns: string for the path part of the file specification.
     */

    PFILE_PATH_STRING GetTitle() const;
    /* Get the title component of the full file specification, eg for the DOS
       file "C:\SRC\PWLIB\FRED.DAT" this would be "FRED".

       Returns: string for the title part of the file specification.
     */

    PFILE_PATH_STRING GetType() const;
    /* Get the file type of the file. Note that on some platforms this may
       actually be part of the full name string. eg for DOS file
       "C:\SRC\PWLIB\FRED.TXT" this would be ".TXT" but on the Macintosh this
       might be "TEXT".

       Note there are standard translations from file extensions, eg ".TXT"
       and some Macintosh file types, eg "TEXT".

       Returns: string for the type part of the file specification.
     */

    PFILE_PATH_STRING GetFileName() const;
    /* Get the actual directory entry name component of the full file
       specification. This may be identical to $B$GetTitle()+GetType()$B$ or
       simply $B$GetTitle()$B$ depending on the platform. eg for DOS file
       "C:\SRC\PWLIB\FRED.TXT" this would be "FRED.TXT".

       Returns: string for the file name part of the file specification.
     */

    void SetType(
      const PFILE_PATH_STRING & type  // New type of the file.
    );
    /* Set the type component of the full file specification, eg for the DOS
       file "C:\SRC\PWLIB\FRED.DAT" would become "C:\SRC\PWLIB\FRED.TXT".
     */


// Class declaration continued in platform specific header file ///////////////
