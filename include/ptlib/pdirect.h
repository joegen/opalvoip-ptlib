/*
 * $Id
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log
 */


#define _PDIRECTORY


///////////////////////////////////////////////////////////////////////////////
// File System

enum PFileTypes {
  Regular = 1,
  Link = 2,
  SubDirectory = 4,
  CharDevice = 8,
  BlockDevice = 16,
  Fifo = 32,
  Socket = 64,
  Unknown = 256
};

#define PAllFiles ((PFileTypes)(Regular|Link|SubDirectory|CharDevice|BlockDevice|Fifo|Socket))

#define PDefaultPerms ((PPermissions)(UserRead|UserWrite|UserExecute|GroupRead|GroupExecute|WorldRead|WorldExecute))



DECLARE_CLASS(PDirectory,PContainer)

  public:
    PDirectory();
      // Create a directory object of the current directory
      
    PDirectory(const PString & pathname);
      // Create a directory object of the specified directory
      
    virtual ~PDirectory();

    // Overrides from class PObject
    PObject * Clone() const;
    Comparison Compare(const PObject & obj) const;
    virtual ostream & PrintOn(ostream & strm) const;
    virtual istream & ReadFrom(istream & strm);

    // New member functions
    PString GetPath() const;
      // Return the full, unambigous, path name for the directory

    BOOL Change();
    inline static BOOL Change(const PString & p);
      // Change to the specified directory.
      
    BOOL Create(PPermissions perm = PDefaultPerms);
    inline static BOOL Create(const PString & p,PPermissions perm = PDefaultPerms);
      // Create a new directory with the specified permissions
      
    BOOL Remove();
    inline static BOOL Remove(const PString & p);
      // Delete the specified directory.
      

    BOOL Open(PFileTypes scanMask = PAllFiles);
      // Open the directory for scanning its list of files.
      
    BOOL Restart(PFileTypes scanMask = PAllFiles);
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

    PString     path;
    PFileTypes  theScanMask;


// Class declaration continued in platform specific header file ///////////////
