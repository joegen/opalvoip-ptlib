/*
 * $Id: file.h,v 1.23 1995/03/12 04:37:13 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
 * Revision 1.23  1995/03/12 04:37:13  robertj
 * Moved GetHandle() function from PFile to PChannel.
 *
 * Revision 1.22  1995/01/14  06:22:11  robertj
 * Documentation
 *
 * Revision 1.21  1994/12/21  11:52:54  robertj
 * Documentation and variable normalisation.
 *
 * Revision 1.20  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.19  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.18  1994/08/21  23:43:02  robertj
 * Added "remove on close" feature for temporary files.
 * Added "force" option to Remove/Rename etc to override write protection.
 * Added function to set file permissions.
 *
 * Revision 1.17  1994/07/17  10:46:06  robertj
 * Moved data to platform dependent files.
 *
 * Revision 1.16  1994/06/25  11:55:15  robertj
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

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// Binary Files

PDECLARE_CONTAINER(PFile, PChannel)
/* This class represents a disk file. This is a particular type of I/O channel
   that has certain attributes. All platforms have a disk file, though exact
   details of naming convertions etc may be different.

   The basic model for files is that they are a named sequence of bytes that
   persists within a directory structure. The transfer of data to and from
   the file is made at a current position in the file. This may be set to
   random locations within the file.
 */

  public:
    PFile();
    /* Create a file object but do not open it. It does not initially have a
       valid file name. However, an attempt to open the file using the
       $B$Open()$B$ function will generate a unique temporary file.
     */

    enum OpenMode {
      ReadOnly,     // File can be read but not written.
      WriteOnly,    // File can be written but not read.
      ReadWrite     // File can be both read and written.
    };
    /* When a file is opened, it may restrict the access available to
       operations on the object instance. A value from this enum is passed to
       the $B$Open()$B$ function to set the mode.
     */

    enum OpenOptions {
      ModeDefault = -1, // File options depend on the OpenMode parameter.
      MustExist = 0,    // File open fails if file does not exist.
      Create = 1,       // File is created if it does not exist.
      Truncate = 2,     // File is set to zero length if it already exists.
      Exclusive = 4,    // File open fails if file already exists.
      Temporary = 8     // File is temporary and is to be deleted when closed.
    };
    /* When a file is opened, a number of options may be associated with the
       open file. These describe what action to take on opening the file and
       what to do on closure. A value from this enum is passed to the
       $B$Open()$B$ function to set the options.

       The $B$ModeDefault$B$ option will use the following values:
          $U$Mode$U$          $U$Options$U$
          ReadOnly      MustExist
          WriteOnly     Create | Truncate
          ReadWrite     Create
     */

    PFile(
      OpenMode mode,          // Mode in which to open the file.
      int opts = ModeDefault  // $H$OpenOptions for open operation.
    );
    /* Create a unique temporary file name, and open the file in the specified
       mode and using the specified options. Note that opening a new, unique,
       temporary file name in ReadOnly mode will always fail. This would only
       be usefull in a mode and options that will create the file.

       The $B$IsOpen()$B$ function may be used after object construction to
       determine if the file was successfully opened.
     */

    PFile(
      const PFilePath & name,     // Name of file to open.
      OpenMode mode = ReadWrite,  // Mode in which to open the file.
      int opts = ModeDefault      // $H$OpenOptions for open operation.
    );
    /* Create a file object with the specified name and open it in the
       specified mode and with the specified options.

       The $B$IsOpen()$B$ function may be used after object construction to
       determine if the file was successfully opened.
     */


    // Overrides from class PObject
    Comparison Compare(
      const PObject & obj   // Other file to compare against.
    ) const;
    /* Determine the relative rank of the two objects. This is essentially the
       string comparison of the $H$PFilePath names of the files.

       Returns: relative rank of the file paths.
     */


  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For example,
       it would return the filename in $H$PFile type channels.

       Returns: the name of the channel.
     */

    virtual BOOL Read(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Low level read from the file channel. The read timeout is ignored for
       file I/O. The GetLastReadCount() function returns the actual number
       of bytes read.

       The GetErrorCode() function should be consulted after Read() returns
       FALSE to determine what caused the failure.

       Returns: TRUE indicates that at least one character was read from the
                channel. FALSE means no bytes were read due to timeout or
                some other I/O error.
     */

    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the file channel. The write timeout is ignored for
       file I/O. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       Returns TRUE if at least len bytes were written to the channel.
     */

    virtual BOOL Close();
    // Close the file channel.


  // New member functions
    const PFilePath & GetFilePath() const;
    /* Get the full path name of the file. The $H$PFilePath object describes
       the full file name specification for the particular platform.

       Returns: the name of the file.
     */

    void SetFilePath(const PString & str);
    /* Set the full path name of the file. The $H$PFilePath object describes
       the full file name specification for the particular platform.
     */


    BOOL Exists() const;
    static BOOL Exists(
      const PString & name  // Name of file to see if exists.
    );
    /* Determine if the file actually exists within the platforms file system.

       The first form uses the file path specification associated with the
       instance of the object. The second static function uses an arbitrary
       file specified by name.

       Returns: TRUE if the file exists.
     */

    BOOL Access(
      OpenMode mode         // Mode in which the file open would be done.
    );
    static BOOL Access(
      const PString & name, // Name of file to have its access checked.
      OpenMode mode         // Mode in which the file open would be done.
    );
    /* Determine if the file may be opened in the specified mode. This would
       check the current access rights to the file for the mode. For example,
       for a file that is read only, using mode == ReadWrite would return
       FALSE but mode == ReadOnly would return TRUE.

       The first form uses the file path specification associated with the
       instance of the object. The second static function uses an arbitrary
       file specified by name.

       Returns: TRUE if a file open would succeed.
     */
      
    BOOL Remove(
      BOOL force = FALSE      // Force deletion even if file is protected.
    );
    static BOOL Remove(
      const PString & name,   // Name of file to delete.
      BOOL force = FALSE      // Force deletion even if file is protected.
    );
    /* Delete the specified file. If $B$force$B$ is FALSE and the file is
       protected against being deleted then the function fails. If $B$force$B$
       is TRUE then the protection is ignored. What constitutes file deletion
       protection is platform dependent, eg on DOS is the Read Only attribute
       and on a Novell network it is a Delete trustee right. Some protection
       may not be able to overridden with the $B$force$B$ parameter at all, eg
       on a Unix system and you are not the owner of the file.

       The first form uses the file path specification associated with the
       instance of the object. The second static function uses an arbitrary
       file specified by name.

       Returns: TRUE if the file was deleted.
     */
      
    BOOL Rename(
      const PString & newname,  // New name for the file.
      BOOL force = FALSE
        // Delete file if a destination exists with the same name.
    );
    static BOOL Rename(
      const PString & oldname,  // Old name of the file.
      const PString & newname,  // New name for the file.
      BOOL force = FALSE
        // Delete file if a destination exists with the same name.
    );
    /* Change the specified files name. Depending on the platform and the
       relationship between the new name and the old, the function may fail to
       rename the file, eg for DOS you cannot rename where the disk drives are
       different.

       The first form uses the file path specification associated with the
       instance of the object. The name within the instance is changed to the
       new name if the function succeeds. The second static function uses an
       arbitrary file specified by name.

       Returns: TRUE if the file was renamed.
     */

    BOOL Copy(
      const PString & newname,  // New name for the file.
      BOOL force = FALSE
        // Delete file if a destination exists with the same name.
    );
    static BOOL Copy(
      const PString & oldname,  // Old name of the file.
      const PString & newname,  // New name for the file.
      BOOL force = FALSE
        // Delete file if a destination exists with the same name.
    );
    /* Make a copy of the specified file.

       The first form uses the file path specification associated with the
       instance of the object. The name within the instance is changed to the
       new name if the function succeeds. The second static function uses an
       arbitrary file specified by name.

       Returns: TRUE if the file was renamed.
     */


    BOOL Open(
      OpenMode mode = ReadWrite,  // Mode in which to open the file.
      int opts = ModeDefault      // Options for open operation.
    );
    BOOL Open(
      const PFilePath & name,     // Name of file to open.
      OpenMode mode = ReadWrite,  // Mode in which to open the file.
      int opts = ModeDefault      // $H$OpenOptions for open operation.
    );
    /* Open the given file name (if specified) in the specified mode and with
       the specified options. If the file object already has an open file then
       it is closed.
       
       If there has not been a filename attached to the file object (via
       $B$SetFilePath()$B$, the $B$name$B$ parameter or a previous open) then
       a new unique temporary filename is generated.

       Returns: TRUE if the file was successfully opened.
     */
      
    off_t GetLength() const;
    /* Get the current size of the file.

       Returns: length of file in bytes.
     */
      
    BOOL SetLength(off_t len);
    /* Set the size of the file, padding with 0 bytes if it would require
       expanding the file, or truncating it if being made shorter.

       Returns: TRUE if the file size was changed to the length specified.
     */

    enum FilePositionOrigin {
      Start = SEEK_SET,   // Set position relative to start of file.
      Current = SEEK_CUR, // Set position relative to current file position.
      End = SEEK_END      // Set position relative to end of file.
    };
    // Options for the origin in setting the file position.

    BOOL SetPosition(
      off_t pos,                         // New position to set.
      FilePositionOrigin origin = Start  // Origin for position change.
    );
    /* Set the current active position in the file for the next read or write
       operation. The $B$pos$B$ variable is a signed number which is added to
       the specified origin. For $B$origin$B$ == PFile::Start only positive
       values for $B$pos$B$ are meaningful. For $B$origin$B$ == PFile::End only
       negative values for $B$pos$B$ are meaningful.

       Returns: TRUE if the new file position was set.
     */

    off_t GetPosition() const;
    /* Get the current active position in the file for the next read or write
       operation.

       Returns: current file position relative to start of file.
     */

    BOOL IsEndOfFile() const;
    /* Determine if the current file position is at the end of the file. If
       this is TRUE then any read operation will fail.

       Returns: TRUE if at end of file.
     */
      
    BOOL GetInfo(
      PFileInfo & info    // $H$PFileInfo structure to receive the information.
    );
    static BOOL GetInfo(
      const PFilePath & name,  // Name of file to get the information on.
      PFileInfo & info    // $H$PFileInfo structure to receive the information.
    );
    /* Get information (eg protection, timestamps) on the specified file.

       The first form uses the file path specification associated with the
       instance of the object.The second static function uses an arbitrary
       file specified by name.

       Returns: TRUE if the file was renamed.
     */

    BOOL SetPermissions(
      int permissions           // New permissions mask for the file.
    );
    static BOOL SetPermissions(
      const PFilePath & name,   // Name of file to change the permission of.
      int permissions           // New permissions mask for the file.
    );
    /* Set permissions on the specified file.

       The first form uses the file path specification associated with the
       instance of the object.The second static function uses an arbitrary
       file specified by name.

       Returns: TRUE if the file was renamed.
     */


  protected:
    // Member variables
    PFilePath path;
      // The fully qualified path name for the file.

    BOOL removeOnClose;
      // File is to be removed when closed.
      

// Class declaration continued in platform specific header file ///////////////
