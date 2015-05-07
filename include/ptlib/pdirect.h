/*
 * pdirect.h
 *
 * File system directory class.
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
 *
 * $Revision$
 * $Author$
 * $Date$
 */


#ifndef PTLIB_DIRECTORY_H
#define PTLIB_DIRECTORY_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/bitwise_enum.h>


#ifdef Fifo
#undef Fifo
#endif

#ifdef _WIN32
#define PDIR_SEPARATOR '\\'
#define PPATH_SEPARATOR ';'  // As used in PATH environment variable
const PINDEX P_MAX_PATH = _MAX_PATH;
typedef PCaselessString PFilePathString;
#else
#define PDIR_SEPARATOR '/'
#define PPATH_SEPARATOR ':'  // As used in PATH environment variable
#define P_MAX_PATH    (_POSIX_PATH_MAX)
typedef PString PFilePathString;
#endif


///////////////////////////////////////////////////////////////////////////////
// File System

/**Class containing the system information on a file path. Information can be
   obtained on any directory entry event if it is not a "file" in the strictest
   sense. Sub-directories, devices etc may also have information retrieved.
 */
class PFileInfo : public PObject
{
  PCLASSINFO(PFileInfo, PObject);

  public:
    /**All types that a particular file path may be. Not all platforms support
       all of the file types. For example under DOS no file may be of the
       type <code>SymbolicLink</code>.
     */
    P_DECLARE_BITWISE_ENUM_EX(FileTypes, 10,
      (
        NoFileType, // Should not happen!

        RegularFile,      ///< Ordinary disk file.
        SymbolicLink,     ///< File path is a symbolic link.
        SubDirectory,     ///< File path is a sub-directory
        ParentDirectory,  ///< File path is ".."
        CurrentDirectory, ///< File path is "."
        CharDevice,       ///< File path is a character device name.
        BlockDevice,      ///< File path is a block device name.
        Fifo,             ///< File path is a fifo (pipe) device.
        SocketDevice,     ///< File path is a socket device.
        UnknownFileType   ///< File path is of an unknown type.
      ),
      AllFiles = 0x3ff,    ///< Mask for all file types.
      DefaultSearch = RegularFile | SymbolicLink | SubDirectory  ///< Mask for common file types in directory search.
    );

    /// File type for this file. Only one bit is set at a time here.
    FileTypes type;

    /**Time of file creation of the file. Not all platforms support a separate
       creation time in which case the last modified time is returned.
     */
    PTime created;

    /// Time of last modifiaction of the file.
    PTime modified;

    /**Time of last access to the file. Not all platforms support a separate
       access time in which case the last modified time is returned.
     */
    PTime accessed;

    /**Size of the file in bytes. This is a quadword or 8 byte value to allow
       for files greater than 4 gigabytes.
     */
    PUInt64 size;

    /// File access permissions for the file.
    P_DECLARE_BITWISE_ENUM_EX(Permissions,9,
      (
        // Note the following bits are as per unixish / C standard lib values, do not change!
        NoPermissions, ///< No permissions selected
        WorldExecute,  ///< File has world execute permission
        WorldWrite,    ///< File has world write permission
        WorldRead,     ///< File has world read permission
        GroupExecute,  ///< File has group execute permission
        GroupWrite,    ///< File has group write permission
        GroupRead,     ///< File has group read permission
        UserExecute,   ///< File has owner execute permission
        UserWrite,     ///< File has owner write permission
        UserRead       ///< File has owner read permission
      ),
      AllPermissions = 0x1ff,                                               ///< All possible permissions.
      DefaultPerms = UserRead|UserWrite|GroupRead|WorldRead,                ///< Owner read & write plus group and world read permissions.
      DefaultDirPerms = DefaultPerms|UserExecute|GroupExecute|WorldExecute  /// Owner read & write & execute plus group and world read & exectute permissions.
    );

    /**A bit mask of all the file acces permissions. See the
       <code>Permissions</code> enum# for the possible bit values.
       
       Not all platforms support all permissions.
     */
    Permissions permissions;

    /**File is a hidden file. What constitutes a hidden file is platform
       dependent, for example under unix it is a file beginning with a '.'
       character while under MS-DOS there is a file system attribute for it.
     */
    bool hidden;
};


/**Class to represent a directory in the operating system file system. A
   directory is a special file that contains a list of file paths.
   
   The directory paths are highly platform dependent and a minimum number of
   assumptions should be made.
   
   The PDirectory object is a string consisting of a possible volume name, and
   a series directory names in the path from the volumes root to the directory
   that the object represents. Each directory is separated by the platform
   dependent separator character which is defined by the PDIR_SEPARATOR macro.
   The path always has a trailing separator.

   Some platforms allow more than one character to act as a directory separator
   so when doing any processing the <code>IsSeparator()</code> function should be
   used to determine if a character is a possible separator.

   The directory may be opened to gain access to the list of files that it
   contains. Note that the directory does {\b not} contain the "." and ".."
   entries that some platforms support.

   The ancestor class is dependent on the platform. For file systems that are
   case sensitive, eg Unix, the ancestor is <code>PString</code>. For other
   platforms, the ancestor class is <code>PCaselessString</code>.
 */
class PDirectory : public PFilePathString
{
  PCONTAINERINFO(PDirectory, PFilePathString);

  public:
  /**@name Construction */
  //@{
    /// Create a directory object of the current working directory
    PDirectory();
      
    /**Create a directory object of the specified directory. The
       <code>cpathname</code> parameter may be a relative directory which is
       made absolute by the creation of the <code>PDirectory</code> object.
     */
    PDirectory(
      const char * cpathname      ///< Directory path name for new object.
    );

    /**Create a directory object of the specified directory. The
       <code>pathname</code> parameter may be a relative directory which is
       made absolute by the creation of the <code>PDirectory</code> object.
     */
    PDirectory(
      const PString & pathname    ///< Directory path name for new object.
    );

    /**Set the directory to the specified path.
     */
    PDirectory & operator=(
      const PString & pathname    ///< Directory path name for new object.
    );

    /**Set the directory to the specified path.
     */
    PDirectory & operator=(
      const char * cpathname      ///< Directory path name for new object.
    );
  //@}

  /**@name Access functions */
  //@{
    /**Get the directory for the parent to the current directory. If the
       directory is already the root directory it returns the root directory
       again.

       @return
       parent directory.
     */
    PDirectory GetParent() const;

    /**Get the volume name that the directory is in.
    
       This is platform dependent, for example for MS-DOS it is the 11
       character volume name for the drive, eg "DOS_DISK", and for Macintosh it
       is the disks volume name eg "Untitled". For a unix platform it is the
       device name for the file system eg "/dev/sda1".

       @return
       string for the directory volume.
     */
    PFilePathString GetVolume() const;

    /**Determine if the directory is the root directory of a volume.
    
       @return
       true if the object is a root directory.
     */
    bool IsRoot() const;

    /**Get the root directory of a volume.
    
       @return
       root directory.
     */
    PDirectory GetRoot() const;

    /**Get the directory path as an array of strings.
       The first element in the array is the volume string, eg under Win32 it
       is "c:" or "\\machine", while under unix it is an empty string.
      */
    PStringArray GetPath() const;

    /**Determine if the character <code>ch</code> is a directory path
       separator.

       @return
       true if may be used to separate directories in a path.
     */
    PINLINE static bool IsSeparator(
      char ch    ///< Character to check as being a separator.
    );

    /**Determine the total number of bytes and number of bytes free on the
       volume that this directory is contained on.

       Note that the free space will be the physical limit and if user quotas
       are in force by the operating system, the use may not actually be able
       to use all of these bytes.

       @return
       true if the information could be determined.
     */
    bool GetVolumeSpace(
      PInt64 & total,     ///< Total number of bytes available on volume
      PInt64 & free,      ///< Number of bytes unused on the volume
      DWORD & clusterSize ///< "Quantisation factor" in bytes for files on volume
    ) const;
  //@}

  /**@name File system functions */
  //@{
    /**Test for if the directory exists.

       @return
       true if directory exists.
     */
    bool Exists() const;

    /**Test for if the specified directory exists.

       @return
       true if directory exists.
     */
    static bool Exists(
      const PString & path   ///< Directory file path.
    );
      
    /**Change the current working directory to the objects location.

       @return
       true if current working directory was changed.
     */
    bool Change() const;

    /**Change the current working directory to that specified..

       @return
       true if current working directory was changed.
     */
    static bool Change(
      const PString & path   ///< Directory file path.
    );
      
    /**Create a new directory with the specified permissions.

       @return
       true if directory created.
     */
    bool Create(
      int perm = PFileInfo::DefaultDirPerms,    // Permission on new directory.
      bool recurse = false
    ) const;
    /**Create a new directory as specified with the specified permissions.

       @return
       true if directory created.
     */
    static bool Create(
      const PString & p,   ///< Directory file path.
      int perm = PFileInfo::DefaultDirPerms,    // Permission on new directory.
      bool recurse = false
    );

    /**Delete the directory.

       @return
       true if directory was deleted.
     */
    bool Remove();

    /**Delete the specified directory.

       @return
       true if directory was deleted.
     */
    static bool Remove(
      const PString & path   ///< Directory file path.
    );
  //@}

  /**@name Directory listing functions */
  //@{
    /**Open the directory for scanning its list of files. Once opened the
       <code>GetEntryName()</code> function may be used to get the current directory
       entry and the <code>Next()</code> function used to move to the next directory
       entry.
       
       Only files that are of a type that is specified in the mask will be
       returned.
       
       Note that the directory scan will {\b not} return the "." and ".."
       entries that some platforms support.

       @return
       true if directory was successfully opened, and there was at least one
       file in it of the specified types.
     */
    virtual bool Open(
      PFileInfo::FileTypes scanMask = PFileInfo::DefaultSearch    ///< Mask of files to provide.
    );
      
    /**Restart file list scan from the beginning of directory. This is similar
       to the <code>Open()</code> command but does not require that the directory be
       closed (using <code>Close()</code>) first.

       Only files that are of a type that is specified in the mask will be
       returned.

       Note that the directory scan will {\b not} return the "." and ".."
       entries that some platforms support.

       @return
       true if directory was successfully opened, and there was at least one
       file in it of the specified types.
     */
    virtual bool Restart(
      PFileInfo::FileTypes scanMask = PFileInfo::DefaultSearch    ///< Mask of files to provide.
    );
      
    /**Move to the next file in the directory scan.
    
       Only files that are of a type that is specified in the mask passed to
       the <code>Open()</code> or <code>Restart()</code> functions will be returned.

       Note that the directory scan will {\b not} return the "." and ".."
       entries that some platforms support.

       @return
       true if there is another valid file in the directory.
     */
    virtual bool Next();
      
    /// Close the directory during or after a file list scan.
    virtual void Close();

    /**Get the name (without the volume or directory path) of the current
       entry in the directory scan. This may be the name of a file or a
       subdirectory or even a link or device for operating systems that support
       them.
       
       To get a full path name concatenate the PDirectory object itself with
       the entry name.
       
       Note that the directory scan will {\b not} return the "." and ".."
       entries that some platforms support.

       @return
       string for directory entry.
     */
    virtual PFilePathString GetEntryName() const;

    /**Determine if the directory entry currently being scanned is itself
       another directory entry.
       
       Note that the directory scan will {\b not} return the "." and ".."
       entries that some platforms support.

       @return
       true if entry is a subdirectory.
     */
    virtual bool IsSubDir() const;

    /**Get file information on the current directory entry.
    
       @return
       true if file information was successfully retrieved.
     */
    virtual bool GetInfo(
      PFileInfo & info    ///< Object to receive the file information.
    ) const;
  //@}


  protected:
    // New functions for class
    void Construct();
    void Destruct()
    { Close(); PFilePathString::Destruct(); }

    // Member variables
    /// Mask of file types that the directory scan will return.
    PFileInfo::FileTypes m_scanMask;

// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/pdirect.h"
#else
#include "unix/ptlib/pdirect.h"
#endif

};


#endif // PTLIB_DIRECTORY_H


// End Of File ///////////////////////////////////////////////////////////////
