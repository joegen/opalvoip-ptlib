/*
 * $Id: pdirect.h,v 1.11 1994/04/20 12:17:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pdirect.h,v $
 * Revision 1.11  1994/04/20 12:17:44  robertj
 * Split name into PFilePath
 *
 * Revision 1.10  1994/04/11  14:16:27  robertj
 * Added function for determining if character is a valid directory separator.
 *
 * Revision 1.9  1994/04/01  14:14:57  robertj
 * Put platform independent file permissions and type codes back.
 *
 * Revision 1.7  1994/01/13  03:17:55  robertj
 * Added functions to get the name of the volume the directory is contained in
 *    and a function to determine if the directory is a root directory of the
 *    volume (ie is does not have a parent directory).
 *
 * Revision 1.6  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.5  1993/12/31  06:45:38  robertj
 * Made inlines optional for debugging purposes.
 *
 * Revision 1.4  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


#define _PDIRECTORY

///////////////////////////////////////////////////////////////////////////////
// File System

enum PFileTypes {
  PRegularFile = 1,
  PSymbolicLink = 2,
  PSubDirectory = 4,
  PCharDevice = 8,
  PBlockDevice = 16,
  PFifo = 32,
  PSocketDevice = 64,
  PUnknownFileType = 256,
  PAllFiles = PRegularFile|PSymbolicLink|PSubDirectory|
                 PCharDevice|PBlockDevice|PFifo|PSocketDevice|PUnknownFileType
};


enum PPermissions {
  PWorldExecute = 1,
  PWorldWrite = 2,
  PWorldRead = 4,
  PGroupExecute = 8,
  PGroupWrite = 16,
  PGroupRead = 32,
  PUserExecute = 64,
  PUserWrite = 128,
  PUserRead = 256,
  PAllPermissions = PUserRead|PUserWrite|PUserExecute|PGroupRead|
                PGroupWrite|PGroupExecute|PWorldRead|PWorldWrite|PWorldExecute,
  PDefaultPerms = PUserRead|PUserWrite|PGroupRead|PWorldRead
};


struct PFileInfo {
  PFileTypes type;
  PTime      created;
  PTime      modified;
  PTime      accessed;
  DWORD      size;
  int        permissions;
  BOOL       hidden;
};


PDECLARE_CONTAINER(PDirectory, PContainer)

  public:
    PDirectory();
      // Create a directory object of the current directory
      
    PDirectory(const PString & pathname);
      // Create a directory object of the specified directory
      

    // Overrides from class PObject
    Comparison Compare(const PObject & obj) const;
    virtual ostream & PrintOn(ostream & strm) const;
    virtual istream & ReadFrom(istream & strm);

    // New member functions
    PString GetPath() const;
      // Return the full, unambigous, path name for the directory

    PString GetVolume() const;
      // Return the volume name that the directory is in.

    BOOL IsRoot() const;
      // Return TRUE if the directory is the root directory of a volume.

    PINLINE static BOOL IsSeparator(char c);
      // Return TRUE if the character is a directory path separator.


    BOOL Change() const;
    PINLINE static BOOL Change(const PString & p);
      // Change to the specified directory.
      
    BOOL Create(int perm = PDefaultPerms) const;
    PINLINE static BOOL Create(const PString & p, int perm = PDefaultPerms);
      // Create a new directory with the specified permissions

    BOOL Remove();
    PINLINE static BOOL Remove(const PString & p);
      // Delete the specified directory.
      

    BOOL Open(int scanMask = PAllFiles);
      // Open the directory for scanning its list of files.
      
    BOOL Restart(int scanMask = PAllFiles);
      // Restart file list scan from the beginning of directory.
      
    BOOL Next();
      // Move to the next file in the directory scan.
      
    void Close();
      // Close the directory after a file list scan.
      
    PString GetEntryName() const;
      // Return the name (without the volume or directory path) of the
      // current entry in the directory scan. This may be the name of a file
      // or a subdirectory or even a link or device for operating systems that
      // support them.

    BOOL IsSubDir() const;
      // Return TRUE if the directory entry currently being scanned is itself
      // another directory entry.
      
    BOOL GetInfo(PFileInfo & info) const;
      // Get information on the current directory entry


  protected:
    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);


    // New functions for class
    void Construct();
      // Common constructor code


    // Member variables
    PString path;
    int     scanMask;


// Class declaration continued in platform specific header file ///////////////
