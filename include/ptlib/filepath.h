/*
 * $Id: filepath.h,v 1.1 1994/04/20 12:17:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: filepath.h,v $
 * Revision 1.1  1994/04/20 12:17:44  robertj
 * Initial revision
 *
 */


#define _PFILEPATH

///////////////////////////////////////////////////////////////////////////////
// File Specification

PDECLARE_CLASS(PFilePath, PString)

  public:
    PFilePath();
    PFilePath(const PString & str);
    PFilePath(const char * cstr);
    PFilePath & operator=(const PString & str);
      // Create a file spec object with the specified name.


    // New member functions
    PString GetVolume() const;
      // Get the drive/volume name component of the full file specification.
      // Note this may not be relevent on some platforms and returns "".
      
    PString GetPath() const;
      // Get the directory path component of the full file specification.

    PString GetTitle() const;
      // Get the title component of the full file specification.

    PString GetType() const;
      // Get the file type component of the full file specification. Note that
      // on some platforms this may not actually be part of the GetFullName()
      // string.

    PString GetFileName() const;
      // Get the actual directory entry name component of the full file
      // specification. This may be identical to GetName()+GetType() or
      // simply GetName() depending on the platform.


// Class declaration continued in platform specific header file ///////////////
