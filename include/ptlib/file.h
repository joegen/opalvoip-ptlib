/*
 * $Id: file.h,v 1.10 1993/12/31 06:45:38 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
 * Revision 1.10  1993/12/31 06:45:38  robertj
 * Made inlines optional for debugging purposes.
 *
 * Revision 1.9  1993/09/27  16:35:25  robertj
 * Changed GetName() to GetTitle(), better naming convention.
 * Moved internal functions to private section.
 *
 * Revision 1.8  1993/08/31  03:38:02  robertj
 * Changed PFile::Status to PFile::Info due to X-Windows compatibility.
 *
 * Revision 1.7  1993/08/27  18:17:47  robertj
 * Moved code from MS-DOS platform to common files.
 *
 * Revision 1.6  1993/08/21  04:40:19  robertj
 * Added Copy() function.
 *
 * Revision 1.5  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.4  1993/08/01  14:05:27  robertj
 * Added GetFileName() function required for proper portability.
 * Improved some comments.
 *
 * Revision 1.3  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
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
    Comparison Compare(const PObject & obj) const;
      // Return TRUE if the two objects refer to the same file. This is
      // essentially if they have the same full path name.


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

    PString GetFullName() const;
      // Return the full, unambiguous, path name for the file. This is
      // always GetVolume()+GetPath()+GetFileName().


    BOOL Exists() const;
    PINLINE static BOOL Exists(const PString & name);
      // Return TRUE if the file exists.
      
    BOOL Access(OpenMode mode) const;
    static BOOL Access(const PString & name, OpenMode mode);
      // Return TRUE if the file may be opened in the specified mode
      
    BOOL Remove() const;
    PINLINE static BOOL Remove(const PString & name);
      // Delete the specified file.
      
    BOOL Rename(const PString & newname);
    PINLINE static BOOL Rename(const PString & oldname, const PString & newname);
      // Change the specified files name. Note that this object then refers to
      // the new filename.

    BOOL Copy(const PString & newname);
    static BOOL Copy(const PString & oldname, const PString & newname);
      // Make a copy of the specified file. Note that this object still refers to
      // the original file.


    struct Info {
      PFileTypes type;
      PTime created;
      PTime modified;
      PTime accessed;
      DWORD filesize;
      int permissions;
    };
    BOOL GetInfo(Info & info) const;
    static BOOL GetInfo(const PString & name, Info & info);

    BOOL Open(OpenMode  mode, int opts = Normal);
      // Open the file in the spcvified mode.
      
    BOOL IsOpen();
      // Return TRUE if the file is currently open.
      
    BOOL Read(void * buffer, size_t amount);
      // Read a sequence of bytes into the specified buffer. Return TRUE if
      // the required number of bytes was successfully read.
      
    BOOL Write(const void * buffer, size_t amount);
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
    // New member functions for class
    int GetHandle() const;
      // Return the integer operating system handle for the file.


    // Member variables
    PString fullname;
      // The fully qualified path name for the file.
      
    int os_handle;
      // The operating system file handle return by standard open() function.

    int os_errno;
      // The operating system error number as returned by errno.


  private:
    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);
    virtual void DestroyContents();


    // New member functions
    void Construct();
      // Common construction code.


// Class declaration continued in platform specific header file ///////////////
