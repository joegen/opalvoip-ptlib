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


#define _PFILE


///////////////////////////////////////////////////////////////////////////////
// Binary Files

DECLARE_CLASS(PFile, PContainer)

  public:
    PFile();
      // Create a unique temporary file name, without opening the file.
      
    PFile(const PString & name);
      // Create a file object with the spcified name without opening the file.
      
    enum OpenMode {
      ReadOnly, 
      WriteOnly, 
      ReadWrite
    };
    enum OpenOptions {
      Normal = 0,
      Create = 1,
      Truncate = 2,
      Exclusive = 4
    };
    PFile(const PString & name, OpenMode mode, int opts = Normal);
      // Create a file object with name and open it in the specified mode.
      
    virtual ~PFile();
      // Destroy the object and close the file. Note this does not delete the
      // file!!


    // Overrides from class PObject
    PObject * Clone() const;
      // Make a copy of the file object. Note that this does not actually make
      // a copy of the file on the disk.

    Comparison Compare(const PObject & obj) const;
      // Return TRUE if the two objects refer to the same file. This is
      // essentially if they have the same full path name.


    // New member functions
    PString GetFullName() const;
      // Return the full, unambiguous, path name for the file.

    PString GetVolume() const;
      // Get the drive/volume name component of the full file specification.
      // Note this may not be relevent on some platforms and returns "".
      
    PString GetPath() const;
      // Get the directory path component of the full file specification.

    PString GetName() const;
      // Get the file name component of the full file specification.

    PString GetType() const;
      // Get the file type component of the full file specification. Note that
      // on some platforms this may not actually be part of the GetFullName()
      // string.


    BOOL Exists() const;
    inline static BOOL Exists(const PString & name);
      // Return TRUE if the file exists.
      
    BOOL Access(OpenMode mode) const;
    static BOOL Access(const PString & name, OpenMode mode);
      // Return TRUE if the file may be opened in the specified mode
      
    BOOL Remove() const;
    inline static BOOL Remove(const PString & name);
      // Delete the specified file.
      
    BOOL Rename(const PString & newname);
    inline static BOOL Rename(const PString & oldname, const PString & newname);
      // Change the specified files name. Note that this object then refers to
      // the new filename.


    struct Status {
      PFileTypes type;
      PTime created;
      PTime modified;
      PTime accessed;
      DWORD filesize;
      int permissions;
    };
    BOOL GetStatus(Status & status) const;
    static BOOL GetStatus(const PString & name, Status & status);

    BOOL Open(OpenMode  mode, int opts = Normal);
      // Open the file in the spcvified mode.
      
    BOOL IsOpen();
      // Return TRUE if the file is currently open.
      
    BOOL Read(void * buffer, size_t amount);
      // Read a sequence of bytes into the specified buffer. Return TRUE if
      // the required number of bytes was successfully read.
      
    BOOL Write(void * buffer, size_t amount);
      // Write a sequence of bytes into the specified buffer. Return TRUE if
      // the required number of bytes was successfully written.
      
    off_t GetLength();
      // Get the current size of the file.
      
    BOOL SetLength(off_t len);
      // Set the size of the file, padding with 0 bytes if it would require
      // expanding the file.
      
    off_t GetPosition();
      // Get the current active position in the file for the next read/write
      // operation.
      
    enum FilePositionOrigin {
      Start = SEEK_SET,
      Current = SEEK_CUR,
      End = SEEK_END
    };
    BOOL SetPosition(off_t pos, FilePositionOrigin origin = Start);
      // Set the current active position in the file for the next read/write
      // operation.
      
    BOOL Close();
      // Close the open file.

    enum Errors {
      NoError,
      FileNotFound,
      FileExists,
      DiskFull,
      AccessDenied,
      Miscellaneous
    };
    Errors LastError(PString * errtext = NULL);
      // Return the error result of the last file I/O operation in this object.
      // The string if not NULL will have a message indicating the error type
      // that may be displayed to the user.


  protected:
    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);
    virtual void DestroyContents();


    // New member functions
    void Construct();
      // Common construction code.


    // Member variables
    PString fullname;
      // The fully qualified path name for the file.
      

// Class declaration continued in platform specific header file ///////////////
