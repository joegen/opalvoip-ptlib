/*
 * $Id: file.h,v 1.16 1994/06/25 11:55:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
 * Revision 1.16  1994/06/25 11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.15  1994/04/20  12:17:44  robertj
 * Split name into PFilePath
 *
 * Revision 1.14  1994/04/01  14:11:03  robertj
 * Added const to functions.
 * Added SetName function.
 *
 * Revision 1.13  1994/03/07  07:38:19  robertj
 * Major enhancementsacross the board.
 *
 * Revision 1.12  1994/01/13  03:40:22  robertj
 * Added hidden flag to file info.
 *
 * Revision 1.12  1994/01/13  03:36:48  robertj
 * Created intermediate class PInteractorLayout for dialog-ish windows.
 *
 * Revision 1.11  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.10  1993/12/31  06:45:38  robertj
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

PDECLARE_CONTAINER(PFile, PChannel)

  public:
    PFile();
      // Create a file object but do not open it. It does not initially
      // have a valid file name.

    enum OpenMode {
      ReadOnly, 
      WriteOnly, 
      ReadWrite
    };
    enum OpenOptions {
      ModeDefault = -1, // File option depends on the OpenMode parameter
      MustExist = 0,    // File open fails if file does not exist
      Create = 1,       // File is created if it does not exist
      Truncate = 2,     // File is set to zero length if it already exists
      Exclusive = 4     // File open fails if file already exists
    };

    PFile(OpenMode mode, int opts = ModeDefault);
      // Create a unique temporary file name, and open the file for reading
      // and writing, creating the file. The file is initially empty.
      
    PFile(const PFilePath & name,
                           OpenMode mode = ReadWrite, int opts = ModeDefault);
      // Create a file object with name and open it in the specified mode.
      

    // Overrides from class PObject
    Comparison Compare(const PObject & obj) const;
      // Return TRUE if the two objects refer to the same file. This is
      // essentially if they have the same full path name.


    // Overrides from class PChannel
    virtual BOOL IsOpen() const;
      // Return TRUE if the channel is currently open.
      
    virtual PString GetName() const;
      // Return the name of the channel.

    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    virtual BOOL Close();
      // Close the channel.


    // New member functions
    const PFilePath & GetFilePath() const;
      // Return the file path.

    void SetFilePath(const PString & str);
      // Set the name of the file object. This has no effect if the file is
      // currently open.


    BOOL Exists() const;
    PINLINE static BOOL Exists(const PString & name);
      // Return TRUE if the file exists.
      
    BOOL Access(OpenMode mode) const;
    static BOOL Access(const PString & name, OpenMode mode);
      // Return TRUE if the file may be opened in the specified mode
      
    BOOL Remove();
    PINLINE static BOOL Remove(const PString & name);
      // Delete the specified file.
      
    BOOL Rename(const PString & newname);
    PINLINE static BOOL Rename(const PString & oldname,
                                                     const PString & newname);
      // Change the specified files name. Note that this object then refers to
      // the new filename.

    BOOL Copy(const PString & newname);
    static BOOL Copy(const PString & oldname, const PString & newname);
      // Make a copy of the specified file. Note that this object still refers
      // to the original file.


    BOOL Open(OpenMode  mode = ReadWrite, int opts = ModeDefault);
      // Open the file in the specified mode. If the file was already open then
      // it is closed and re-opened. If there has not been a filename attached
      // to the file object (via SetFilePath() or a previous open) then a new
      // unique temporary filename is generated.

    BOOL Open(const PFilePath & name,
                          OpenMode  mode = ReadWrite, int opts = ModeDefault);
      // Open the given filename in the specified mode. If the file object
      // already has an open file then it is closed beforehand. The opts
      // parameter, if ModeDefault will be MustExist for mode == ReadOnly,
      // Create|Truncate for mode == WriteOnly and Create|Exclusive for
      // mode == ReadWrite.
      
    int GetHandle() const;
      // Return the integer operating system handle for the file.

    off_t GetLength() const;
      // Get the current size of the file.
      
    BOOL SetLength(off_t len);
      // Set the size of the file, padding with 0 bytes if it would require
      // expanding the file.
      
    enum FilePositionOrigin {
      Start = SEEK_SET,
      Current = SEEK_CUR,
      End = SEEK_END
    };
    BOOL SetPosition(off_t pos, FilePositionOrigin origin = Start);
      // Set the current active position in the file for the next read/write
      // operation.

    off_t GetPosition() const;
      // Get the current active position in the file for the next read/write
      // operation.

    BOOL IsEndOfFile() const;
      // Return TRUE if at end of file.
      
    BOOL GetInfo(PFileInfo & info) const;
    static BOOL GetInfo(const PFilePath & name, PFileInfo & info);
      // Get information on the specified file


  protected:
    // Member variables
    PFilePath path;
      // The fully qualified path name for the file.
      
    int os_handle;
      // The operating system file handle return by standard open() function.


// Class declaration continued in platform specific header file ///////////////
