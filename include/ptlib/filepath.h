/*
 * $Id: filepath.h,v 1.6 1994/10/24 00:06:58 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: filepath.h,v $
 * Revision 1.6  1994/10/24 00:06:58  robertj
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

  public:
    PFilePath();
    PFilePath(const PString & str);
    PFilePath(const char * cstr);
    PFilePath & operator=(const PString & str);
      // Create a file spec object with the specified name.

    PFilePath(const char * prefix, const char * dir);
      // Create a file spec object with a generated temporary name. The
      // first parameter is a prefix for the filename to which a unique
      // number is appended. The second parameter is the directory in which
      // the file is to be placed. If this is NULL a system standard
      // directory is used.


    // New member functions
    PString GetVolume() const;
      // Get the drive/volume name component of the full file specification.
      // Note this may not be relevent on some platforms and returns "" eg for
      // unix filesystems.
      
    PString GetPath() const;
      // Get the directory path component of the full file specification. This
      // will include leading and trailing directory separators.

    PString GetTitle() const;
      // Get the title component of the full file specification, eg for the DOS
      // file "c:\fred.dat" this would be "fred"

    PString GetType() const;
      // Get the file type of the file. Note that on some platforms this may
      // actually be part of the full name string. eg for DOS file
      // "c:\fred.txt" this would be ".txt" but on the Macintosh this might be
      // "TEXT".

    PString GetFileName() const;
      // Get the actual directory entry name component of the full file
      // specification. This may be identical to GetName()+GetType() or
      // simply GetName() depending on the platform. eg for DOS file
      // "c:\fred.txt" this would be "fred.txt".

    void SetType(const PString & type);
      // Set the type component of the full file specification, eg for the DOS
      // file "c:\fred.dat" would become "c:\fred.txt"


// Class declaration continued in platform specific header file ///////////////
