/*
 * $Id: pdirect.h,v 1.4 1993/08/21 01:50:33 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pdirect.h,v $
 * Revision 1.4  1993/08/21 01:50:33  robertj
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
  PSocket = 64,
  PUnknownFileType = 256
};

#define PAllFiles (PRegularFile|PSymbolicLink|PSubDirectory| \
                       PCharDevice|PBlockDevice|PFifo|PSocket|PUnknownFileType)


enum PPermissions {
  PUserRead = 1,
  PUserWrite = 2,
  PUserExecute = 4,
  PGroupRead = 8,
  PGroupWrite = 16,
  PGroupExecute = 32,
  PWorldRead = 64,
  PWorldWrite = 128,
  PWorldExecute = 256,
};

#define PAllPermissions (PUserRead|PUserWrite|PUserExecute|PGroupRead| \
                PGroupWrite|PGroupExecute|PWorldRead|PWorldWrite|PWorldExecute)

#define PDefaultPerms (PUserRead|PUserWrite|PUserExecute| \
                             PGroupRead|PGroupExecute|PWorldRead|PWorldExecute)



DECLARE_CLASS(PDirectory, PContainer)

  public:
    PDirectory();
      // Create a directory object of the current directory
      
    PDirectory(const PString & pathname);
      // Create a directory object of the specified directory
      
    virtual ~PDirectory();

    // Overrides from class PObject
    Comparison Compare(const PObject & obj) const;
    virtual ostream & PrintOn(ostream & strm) const;
    virtual istream & ReadFrom(istream & strm);

    // New member functions
    PString GetPath() const;
      // Return the full, unambigous, path name for the directory

    BOOL Change() const;
    inline static BOOL Change(const PString & p);
      // Change to the specified directory.
      
    BOOL Create(int perm = PDefaultPerms) const;
    inline static BOOL Create(const PString & p, int perm = PDefaultPerms);
      // Create a new directory with the specified permissions
      
    BOOL Remove() const;
    inline static BOOL Remove(const PString & p);
      // Delete the specified directory.
      

    BOOL Open(int scanMask = PAllFiles);
      // Open the directory for scanning its list of files.
      
    BOOL Restart(int scanMask = PAllFiles);
      // Restart file list scan from the beginning of directory.
      
    BOOL Next();
      // Move to the next file in the directory scan.
      
    void Close();
      // Close the directory after a file list scan.
      
    PString Entry() const;
      // Return the filename (without the volume or directory path) of the
      // current file in the directory scan.
      
    BOOL IsSubDir() const;
      // Return TRUE if the directory entry currently being scanned is itself
      // another directory entry.
      

  protected:
    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);
    virtual void DestroyContents();

    void Construct();
      // Common constructor code

    PString path;
    int     scanMask;


// Class declaration continued in platform specific header file ///////////////
