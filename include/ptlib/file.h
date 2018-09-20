/*
 * file.h
 *
 * Operating System file I/O channel class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */


#ifndef PTLIB_FILE_H
#define PTLIB_FILE_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#ifndef _WIN32
#include <sys/stat.h>
#endif

#include <ptlib/bitwise_enum.h>


///////////////////////////////////////////////////////////////////////////////
// Binary Files

/**This class represents a disk file. This is a particular type of I/O channel
   that has certain attributes. All platforms have a disk file, though exact
   details of naming convertions etc may be different.

   The basic model for files is that they are a named sequence of bytes that
   persists within a directory structure. The transfer of data to and from
   the file is made at a current position in the file. This may be set to
   random locations within the file.
 */
class PFile : public PChannel
{
  PCLASSINFO(PFile, PChannel);

  public:
  /**@name Construction */
  //@{
    /**Create a file object but do not open it. It does not initially have a
       valid file name. However, an attempt to open the file using the
       <code>Open()</code> function will generate a unique temporary file.
     */
    PFile();

    /**When a file is opened, it may restrict the access available to
       operations on the object instance. A value from this enum is passed to
       the <code>Open()</code> function to set the mode.
     */
    enum OpenMode {
      ReadOnly,   ///< File can be read but not written.
      WriteOnly,  ///< File can be written but not read.
      ReadWrite   ///< File can be both read and written.
    };

    /**When a file is opened, a number of options may be associated with the
       open file. These describe what action to take on opening the file and
       what to do on closure. A value from this enum is passed to the
       <code>Open()</code> function to set the options.

       The <code>ModeDefault</code> option will use the following values:
          \arg \c ReadOnly  <code>MustExist</code>
          \arg \c WriteOnly <code>Create | Truncate</code>
          \arg \c ReadWrite <code>Create</code>
     */
    P_DECLARE_BITWISE_ENUM_EX(OpenOptions, 7,
        (NoOptions,       ///< No options selected
         MustExist,       ///< File open fails if file does not exist.
         Create,          ///< File is created if it does not exist.
         Truncate,        ///< File is set to zero length if it already exists.
         Exclusive,       ///< File open fails if file already exists.
         Temporary,       ///< File is temporary and is to be deleted when closed.
         DenySharedRead,  ///< File may not be read by another process.
         DenySharedWrite  ///< File may not be written by another process.
        ),
        ModeDefault = -1  ///< File options depend on the OpenMode parameter.
    );

    /**Create a unique temporary file name, and open the file in the specified
       mode and using the specified options. Note that opening a new, unique,
       temporary file name in ReadOnly mode will always fail. This would only
       be usefull in a mode and options that will create the file.

       The <code>PChannel::IsOpen()</code> function may be used after object
       construction to determine if the file was successfully opened.
     */
    PFile(
      OpenMode mode,          ///< Mode in which to open the file.
      OpenOptions opts = ModeDefault  ///< <code>OpenOptions</code> enum# for open operation.
    );

    /**Create a file object with the specified name and open it in the
       specified mode and with the specified options.

       The <code>PChannel::IsOpen()</code> function may be used after object
       construction to determine if the file was successfully opened.
     */
    PFile(
      const PFilePath & name,    ///< Name of file to open.
      OpenMode mode = ReadWrite, ///< Mode in which to open the file.
      OpenOptions opts = ModeDefault     ///< <code>OpenOptions</code> enum# for open operation.
    );

    /// Close the file on destruction.
    ~PFile();
  //@}


  /**@name Overrides from class PObject */
  //@{
    /**Determine the relative rank of the two objects. This is essentially the
       string comparison of the <code>PFilePath</code> names of the files.

       @return
       relative rank of the file paths.
     */
    Comparison Compare(
      const PObject & obj   ///< Other file to compare against.
    ) const;
  //@}


  /**@name Overrides from class PChannel */
  //@{
    /**Get the platform and I/O channel type name of the channel. For example,
       it would return the filename in <code>PFile</code> type channels.

       @return
       the name of the channel.
     */
    virtual PString GetName() const;

    /**Low level read from the file channel. The read timeout is ignored for
       file I/O. The GetLastReadCount() function returns the actual number
       of bytes read.

       The GetErrorCode() function should be consulted after Read() returns
       false to determine what caused the failure.

       @return
       true indicates that at least one character was read from the channel.
       false means no bytes were read due to timeout or some other I/O error.
     */
    virtual PBoolean Read(
      void * buf,   ///< Pointer to a block of memory to receive the read bytes.
      PINDEX len    ///< Maximum number of bytes to read into the buffer.
    );

    /**Low level write to the file channel. The write timeout is ignored for
       file I/O. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       false to determine what caused the failure.

       @return true if at least len bytes were written to the channel.
     */
    virtual PBoolean Write(
      const void * buf, ///< Pointer to a block of memory to write.
      PINDEX len        ///< Number of bytes to write.
    );

    /** Close the file channel.
        @return true if close was OK.
      */
    virtual PBoolean Close();
  //@}


  /**@name File manipulation functions */
  //@{
    /**Check for file existance. 
       Determine if the file specified actually exists within the platforms
       file system.

       @return
       true if the file exists.
     */
    static bool Exists(
      const PFilePath & name  ///< Name of file to see if exists.
    );

    /**Check for file existance.
       Determine if the file path specification associated with the instance
       of the object actually exists within the platforms file system.

       @return
       true if the file exists.
     */
    bool Exists() const;

    /**Check for file access modes.
       Determine if the file specified may be opened in the specified mode. This would
       check the current access rights to the file for the mode. For example,
       for a file that is read only, using mode == ReadWrite would return
       false but mode == ReadOnly would return true.

       @return
       true if a file open would succeed.
     */
    static bool Access(
      const PFilePath & name, ///< Name of file to have its access checked.
      OpenMode mode         ///< Mode in which the file open would be done.
    );

    /**Check for file access modes.
       Determine if the file path specification associated with the
       instance of the object may be opened in the specified mode. This would
       check the current access rights to the file for the mode. For example,
       for a file that is read only, using mode == ReadWrite would return
       false but mode == ReadOnly would return true.

       @return
       true if a file open would succeed.
     */
    bool Access(
      OpenMode mode         ///< Mode in which the file open would be done.
    );

    /**Set access & modification times for file.
       If accessTime or modTime is invalid then current time is used for that
       time. If the modTime is not present it is set to the same as the accessTime.

       @return
       true if file times changed.
     */
    static bool Touch(
      const PFilePath & name,       ///< Name of file to have its access time changed.
      const PTime & accessTime = 0  ///< Time to set access time to.
    );
    static bool Touch(
      const PFilePath & name,   ///< Name of file to have its access time changed.
      const PTime & accessTime, ///< Time to set access time to.
      const PTime & modTime     ///< Time to set modification time to.
    );

    /**Set access & modification times for file.

       @return
       true if file times changed.
     */
    bool Touch(
      const PTime & accessTime = 0 ///< Time to set access time to.
    );
    bool Touch(
      const PTime & accessTime,    ///< Time to set access time to.
      const PTime & modTime        ///< Time to set modification time to.
    );

    /**Delete the specified file. If <code>force</code> is false and the file
       is protected against being deleted then the function fails. If
       <code>force</code> is true then the protection is ignored. What
       constitutes file deletion protection is platform dependent, eg on DOS
       is the Read Only attribute and on a Novell network it is a Delete
       trustee right. Some protection may not be able to overridden with the
       <code>force</code> parameter at all, eg on a Unix system and you are
       not the owner of the file.

       @return
       true if the file was deleted.
     */
    static bool Remove(
      const PFilePath & name,   ///< Name of file to delete.
      bool force = false        ///< Force deletion even if file is protected.
    );
    static bool Remove(
      const PString & name,   ///< Name of file to delete.
      bool force = false      ///< Force deletion even if file is protected.
    );

    /**Delete the current file. If <code>force</code> is false and the file
       is protected against being deleted then the function fails. If
       <code>force</code> is true then the protection is ignored. What
       constitutes file deletion protection is platform dependent, eg on DOS
       is the Read Only attribute and on a Novell network it is a Delete
       trustee right. Some protection may not be able to overridden with the
       <code>force</code> parameter at all, eg on a Unix system and you are
       not the owner of the file.

       @return
       true if the file was deleted.
     */
    bool Remove(
      bool force = false      ///< Force deletion even if file is protected.
    );

    /**Change the specified files name. This does not move the file in the
       directory hierarchy, it only changes the name of the directory entry.

       The <code>newname</code> parameter must consist only of the file name
       part, as returned by the <code>PFilePath::GetFileName()</code> function. Any
       other file path parts will cause an error.

       The first form uses the file path specification associated with the
       instance of the object. The name within the instance is changed to the
       new name if the function succeeds. The second static function uses an
       arbitrary file specified by name.

       @return
       true if the file was renamed.
     */
    static bool Rename(
      const PFilePath & oldname,  ///< Old name of the file.
      const PString & newname,    ///< New name for the file.
      bool force = false          ///< Delete file if a destination exists with the same name.
    );

    /**Change the current files name.
       This does not move the file in the
       directory hierarchy, it only changes the name of the directory entry.

       The <code>newname</code> parameter must consist only of the file name
       part, as returned by the <code>PFilePath::GetFileName()</code> function. Any
       other file path parts will cause an error.

       The first form uses the file path specification associated with the
       instance of the object. The name within the instance is changed to the
       new name if the function succeeds. The second static function uses an
       arbitrary file specified by name.

       @return
       true if the file was renamed.
     */
    bool Rename(
      const PString & newname,  ///< New name for the file.
      bool force = false        ///< Delete file if a destination exists with the same name.
    );

    /**Make a copy of the specified file.

       @return
       true if the file was renamed.
     */
    static bool Copy(
      const PFilePath & oldname,  ///< Old name of the file.
      const PFilePath & newname,  ///< New name for the file.
      bool force = false,         ///< Delete file if a destination exists with the same name.
      bool recurse = false        ///< Recursively create all intermediate sub-directories
    );

    /**Make a copy of the current file.

       @return
       true if the file was renamed.
     */
    bool Copy(
      const PFilePath & newname,  ///< New name for the file.
      bool force = false,         ///< Delete file if a destination exists with the same name.
      bool recurse = false        ///< Recursively create all intermediate sub-directories
    );

    /**Move the specified file. This will move the file from one position in
       the directory hierarchy to another position. The actual operation is
       platform dependent but  the reslt is the same. For instance, for Unix,
       if the move is within a file system then a simple rename is done, if
       it is across file systems then a copy and a delete is performed.

       @return
       true if the file was moved.
     */
    static bool Move(
      const PFilePath & oldname,  ///< Old path and name of the file.
      const PFilePath & newname,  ///< New path and name for the file.
      bool force = false,         ///< Delete file if a destination exists with the same name.
      bool recurse = false        ///< Recursively create all intermediate sub-directories
    );

    /**Move the current file. This will move the file from one position in
       the directory hierarchy to another position. The actual operation is
       platform dependent but  the reslt is the same. For instance, for Unix,
       if the move is within a file system then a simple rename is done, if
       it is across file systems then a copy and a delete is performed.

       @return
       true if the file was moved.
     */
    bool Move(
      const PFilePath & newname,  ///< New path and name for the file.
      bool force = false,         ///< Delete file if a destination exists with the same name.
      bool recurse = false        ///< Recursively create all intermediate sub-directories
    );
  //@}

  /**@name File channel functions */
  //@{
    /**Get the full path name of the file. The <code>PFilePath</code> object
       describes the full file name specification for the particular platform.

       @return
       the name of the file.
     */
    const PFilePath & GetFilePath() const;

    /**Set the full path name of the file. The <code>PFilePath</code> object
       describes the full file name specification for the particular platform.
     */
    void SetFilePath(
      const PString & path    ///< New file path.
    );


    /**Open the current file in the specified mode and with
       the specified options. If the file object already has an open file then
       it is closed.
       
       If there has not been a filename attached to the file object (via
       <code>SetFilePath()</code>, the <code>name</code> parameter or a previous
       open) then a new unique temporary filename is generated.

       @return
       true if the file was successfully opened.
     */
    bool Open(
      OpenMode mode = ReadWrite,        ///< Mode in which to open the file.
      OpenOptions opts = ModeDefault    ///< Options for open operation.
    );

    /**Open the specified file name in the specified mode and with
       the specified options and permissions. If the file object already has an open file then
       it is closed.
       
       @return
       true if the file was successfully opened.
     */
    bool Open(
      OpenMode mode,                      ///< Mode in which to open the file.
      OpenOptions opts,                   ///< <code>OpenOptions</code> enum# for open operation.
      PFileInfo::Permissions permissions  ///< Permission for file if created
    );

    /**Open the specified file name in the specified mode and with
       the specified options. If the file object already has an open file then
       it is closed.
       
       @return
       true if the file was successfully opened.
     */
    bool Open(
      const PFilePath & name,         ///< Name of file to open.
      OpenMode mode = ReadWrite,      ///< Mode in which to open the file.
      OpenOptions opts = ModeDefault  ///< <code>OpenOptions</code> enum# for open operation.
    );

    /**Open the specified file name in the specified mode and with
       the specified options and permissions. If the file object already has an open file then
       it is closed.
       
       @return
       true if the file was successfully opened.
     */
    bool Open(
      const PFilePath & name,             ///< Name of file to open.
      OpenMode mode,                      ///< Mode in which to open the file.
      OpenOptions opts,                   ///< <code>OpenOptions</code> enum# for open operation.
      PFileInfo::Permissions permissions  ///< Permission for file if created
    );

    /**Get the current size of the file.

       @return
       length of file in bytes.
     */
    virtual off_t GetLength() const;
      
    /**Set the size of the file, padding with 0 bytes if it would require
       expanding the file, or truncating it if being made shorter.

       @return
       true if the file size was changed to the length specified.
     */
    virtual PBoolean SetLength(
      off_t len   // New length of file.
    );

    /// Options for the origin in setting the file position.
    enum FilePositionOrigin {
      Start = SEEK_SET,    ///< Set position relative to start of file.
      Current = SEEK_CUR,  ///< Set position relative to current file position.
      End = SEEK_END       ///< Set position relative to end of file.
    };

    /**Set the current active position in the file for the next read or write
       operation. The <code>pos</code> variable is a signed number which is
       added to the specified origin. For <code>origin == PFile::Start</code>
       only positive values for <code>pos</code> are meaningful. For
       <code>origin == PFile::End</code> only negative values for
       <code>pos</code> are meaningful.

       @return
       true if the new file position was set.
     */
    virtual PBoolean SetPosition(
      off_t pos,                         ///< New position to set.
      FilePositionOrigin origin = Start  ///< Origin for position change.
    );

    /**Get the current active position in the file for the next read or write
       operation.

       @return
       current file position relative to start of file.
     */
    virtual off_t GetPosition() const;

    /**Determine if the current file position is at the end of the file. If
       this is true then any read operation will fail.

       @return
       true if at end of file.
     */
    bool IsEndOfFile() const;
      
    /**Get information (eg protection, timestamps) on the specified file.

       @return
       true if the file info was retrieved.
     */
    static bool GetInfo(
      const PFilePath & name,  ///< Name of file to get the information on.
      PFileInfo & info         ///< <code>PFileInfo</code> structure to receive the information.
    );

    /**Get information (eg protection, timestamps) on the current file.

       @return
       true if the file info was retrieved.
     */
    bool GetInfo(
      PFileInfo & info ///< <code>PFileInfo</code> structure to receive the information.
    );

    /**Set permissions on the specified file.

       @return
       true if the file was renamed.
     */
    static bool SetPermissions(
      const PFilePath & name,             ///< Name of file to change the permission of.
      PFileInfo::Permissions permissions  ///< New permissions mask for the file.
    );
    /**Set permissions on the current file.

       @return
       true if the file was renamed.
     */
    bool SetPermissions(
      PFileInfo::Permissions permissions  ///< New permissions mask for the file.
    );

    /** Information on how to rotate files.
        The system will generate a new file path via
        <code>
            m_directory + m_prefix + PTime::AsString(m_timeTemplate) + m_suffix
        </code>
        which is used with renaming/moving the file.

        The wildcard:
        <code>
            m_directory + m_prefix + '*' + m_suffix
        </code>
        is used to determine the files to remove.
    */
    struct RotateInfo
    {
      static const PString & DefaultTimestamp();
      explicit RotateInfo(
        const PDirectory & dir = PDirectory(),
        const PString & prefix = PString::Empty(), // If empty, uses PProcess::Current().GetName()
        const PString & suffix = PString::Empty(),
        const PString & timestamp = DefaultTimestamp(),
        PINDEX maxSize = 1000000000 // A gigabyte
      );
      RotateInfo(const RotateInfo & other);
      RotateInfo & operator=(const RotateInfo & other);
      virtual ~RotateInfo() { }

      /** Inidcate that the RotateInfo is configured so that rotations can be made
          via the Rotate() function.
        */
      bool CanRotate() const;

      /** Execute a rotation.
          The \b file is closed, renamed to new name, and re-opened. The old rotated
          files are are aos checked and deleted according to the criteria here.

          If \b force is false then the file must be larger than m_maxSize.
          If \b force is true the the file is moved and re-opened regardless of it's
          current size.

          Note, m_maxSize must be non zero and m_timestamp must be non-empty string
          for this to operate, even in the \b force == true case.
        */
      bool Rotate(
        PFile & activeFile,
        bool force = false, ///< Force rotate regardless of time/size conditions
        const PTime & now = PTime()
      );

      /** Callback when a rotation of an open file is about to be performed.
          This is called just before the PFile is closed, and new one is opened.
        */
      virtual void OnCloseFile(
        PFile & file,               ///< File to be rotated.
        const PFilePath & rotatedTo ///< Name to which file will be rotated
      );

      /** Callback to open the file.
          The default behaviour calls file.Open(PFile::WriteOnly).
        */
      virtual bool OnOpenFile(
        PFile & file                ///< File to be reopened.
      );

      /** Callback when have a message on thte rotation
        */
      virtual void OnMessage(
        bool error,           ///< Message is about an error, otherwise just informative.
        const PString & msg   ///< Message to output
      );

      enum Period {
        SizeOnly,
        Hourly,
        Daily,
        Weekly,
        Monthly
      };

      PDirectory      m_directory;    ///< Destination directory for rotated file, default to same s log file
      PFilePathString m_prefix;       ///< File name prefix, default PProcess::GetName()
      PString         m_timestamp;    ///< Time template for rotated file, default "_yyyy_MM_dd_hh_mm"
      int             m_timeZone;     ///< TIme zone for output and rotated file names
      PFilePathString m_suffix;       ///< File name suffix, default ".log"
      off_t           m_maxSize;      ///< Size in bytes which triggers a rotation, default zero disables
      Period          m_period;       ///< Rotate on the peroid regardless of size.
      unsigned        m_maxFileCount; ///< When this many files have been rotated, oldest is deleted
      PTimeInterval   m_maxFileAge;   ///< Rotated files older than this are deleted

      PTime           m_lastTime;
    };
  //@}

  protected:
    virtual bool InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions);

    // Member variables

    PFilePath m_path;         ///< The fully qualified path name for the file.
    bool      m_removeOnClose; ///< File is to be removed when closed.


// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/file.h"
#else
#include "unix/ptlib/file.h"
#endif
};


#endif // PTLIB_FILE_H


// End Of File ///////////////////////////////////////////////////////////////
