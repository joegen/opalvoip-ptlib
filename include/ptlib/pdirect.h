/*
 * $Id: pdirect.h,v 1.23 1995/11/09 12:17:23 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pdirect.h,v $
 * Revision 1.23  1995/11/09 12:17:23  robertj
 * Added platform independent base type access classes.
 *
 * Revision 1.22  1995/10/14 15:02:22  robertj
 * Added function to get parent directory.
 *
 * Revision 1.21  1995/06/17 11:12:52  robertj
 * Documentation update.
 *
 * Revision 1.20  1995/03/14 12:42:00  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.19  1995/03/12  04:42:48  robertj
 * Updated documentation.
 * Changed return type of functions to the correct case string.
 *
 * Revision 1.18  1995/02/22  10:50:33  robertj
 * Changes required for compiling release (optimised) version.
 *
 * Revision 1.17  1995/01/06  10:42:25  robertj
 * Moved identifiers into different scope.
 * Changed file size to 64 bit integer.
 * Documentation
 *
 * Revision 1.16  1994/10/24  00:07:03  robertj
 * Changed PFilePath and PDirectory so descends from either PString or
 *     PCaselessString depending on the platform.
 *
 * Revision 1.15  1994/10/23  04:49:25  robertj
 * Chnaged PDirectory to descend of PString.
 * Added PDirectory Exists() function.
 *
 * Revision 1.14  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.13  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.12  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.11  1994/04/20  12:17:44  robertj
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

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// File System

PDECLARE_CLASS(PFileInfo, PObject)
/* Class containing the system information on a file path. Information can be
   obtained on any directory entry event if it is not a "file" in the strictest
   sense. Sub-directories, devices etc may also have information retrieved.
 */

  public:
    enum FileTypes {
      RegularFile = 1,        // Ordinary disk file.
      SymbolicLink = 2,       // File path is a symbolic link.
      SubDirectory = 4,       // File path is a sub-directory
      CharDevice = 8,         // File path is a character device name.
      BlockDevice = 16,       // File path is a block device name.
      Fifo = 32,              // File path is a fifo (pipe) device.
      SocketDevice = 64,      // File path is a socket device.
      UnknownFileType = 256,  // File path is of an unknown type.
      AllFiles = 0x1ff        // Mask for all file types.
    };
    /* All types that a particular file path may be. Not all platforms support
       all of the file types. For example under DOS no file may be of the
       type <CODE>SymbolicLink</CODE>.
     */

    FileTypes type;
    // File type for this file. Only one bit is set at a time here.

    PTime created;
    /* Time of file creation of the file. Not all platforms support a separate
       creation time in which case the last modified time is returned.
     */

    PTime modified;
    // Time of last modifiaction of the file.

    PTime accessed;
    /* Time of last access to the file. Not all platforms support a separate
       access time in which case the last modified time is returned.
     */

    PUInt64 size;
    /* Size of the file in bytes. This is a quadword or 8 byte value to allow
       for files greater than 4 gigabytes.
     */

    enum Permissions {
      WorldExecute = 1,   // File has world execute permission
      WorldWrite = 2,     // File has world write permission
      WorldRead = 4,      // File has world read permission
      GroupExecute = 8,   // File has group execute permission
      GroupWrite = 16,    // File has group write permission
      GroupRead = 32,     // File has group read permission
      UserExecute = 64,   // File has owner execute permission
      UserWrite = 128,    // File has owner write permission
      UserRead = 256,     // File has owner read permission
      AllPermissions = 0x1ff,   // All possible permissions.
      DefaultPerms = 0x1A4
      // Owner read & write plus group and world read permissions.
    };
    // File access permissions for the file.

    int permissions;
    /* A bit mask of all the file acces permissions. See the
       <A>Permissions enum</A> for the possible bit values.
       
       Not all platforms support all permissions.
     */

    BOOL hidden;
    /* File is a hidden file. What constitutes a hidden file is platform
       dependent, for example under unix it is a file beginning with a '.'
       character while under MS-DOS there is a file system attribute for it.
     */
};


PDECLARE_CONTAINER(PDirectory, PFILE_PATH_STRING)
/* Class to represent a directory in the operating system file system. A
   directory is a special file that contains a list of file paths.
   
   The directory paths are highly platform dependent and a minimum number of
   assumptions should be made.
   
   The PDirectory object is a string consisting of a possible volume name, and
   a series directory names in the path from the volumes root to the directory
   that the object represents. Each directory is separated by the platform
   dependent separator character which is defined by the PDIR_SEPARATOR macro.
   The path always has a trailing separator.

   Some platforms allow more than one character to act as a directory separator
   so when doing any processing the <A>IsSeparator()</A> function should be
   used to determine if a character is a possible separator.

   The directory may be opened to gain access to the list of files that it
   contains. Note that the directory does <EM>not</EM> contain the "." and ".."
   entries that some platforms support.

   The ancestor class is dependent on the platform. For file systems that are
   case sensitive, eg Unix, the ancestor is <A>PString</A>. For other
   platforms, the ancestor class is <A>PCaselessString</A>.
 */

  public:
    PDirectory();
    // Create a directory object of the current working directory
      
    PDirectory(
      const PString & pathname    // Directory path name for new object.
    );
    /* Create a directory object of the specified directory. The
       <CODE>pathname</CODE> parameter may be a relative directory which is
       made absolute by the creation of the <A>PDirectory</A> object.
     */


  // New member functions
    PDirectory GetParent() const;
    /* Get the directory for the parent to the current directory. If the
       directory is already the root directory it returns the root directory
       again.

       <H2>Returns:</H2>
       parent directory.
     */

    PFILE_PATH_STRING GetVolume() const;
    /* Get the volume name that the directory is in.
    
       This is platform dependent, for example for MS-DOS it is the 11
       character volume name for the drive, eg "DOS_DISK", and for Macintosh it
       is the disks volume name eg "Untitled". For a unix platform it is the
       device name for the file system eg "/dev/sda1".

       <H2>Returns:</H2>
       string for the directory volume.
     */

    BOOL IsRoot() const;
    /* Determine if the directory is the root directory of a volume.
    
       <H2>Returns:</H2>
       TRUE if the object is a root directory.
     */

    PINLINE static BOOL IsSeparator(
      char ch    // Character to check as being a separator.
    );
    /* Determine if the character <CODE>ch</CODE> is a directory path
       separator.

       <H2>Returns:</H2>
       TRUE if may be used to separate directories in a path.
     */


    BOOL Exists() const;
    static BOOL Exists(
      const PString & p   // Directory file path.
    );
    /* Test for if the directory exists. The parameterless version tests
       against the name contained in the object instance. The static version
       may be simply passed a path name.

       <H2>Returns:</H2>
       TRUE if directory exists.
     */
      
    BOOL Change() const;
    static BOOL Change(
      const PString & p   // Directory file path.
    );
    /* Change the current working directory. The parameterless version changes
       to the name contained in the object instance. The static version may be
       simply passed a path name.

       <H2>Returns:</H2>
       TRUE if current working directory was changed.
     */
      
    BOOL Create(
      int perm = PFileInfo::DefaultPerms    // Permission on new directory.
    ) const;
    PINLINE static BOOL Create(
      const PString & p,   // Directory file path.
      int perm = PFileInfo::DefaultPerms    // Permission on new directory.
    );
    /* Create a new directory with the specified permissions. The parameterless
       version changes to the name contained in the object instance. The static
       version may be simply passed a path name.

       <H2>Returns:</H2>
       TRUE if directory created.
     */

    BOOL Remove();
    PINLINE static BOOL Remove(
      const PString & p   // Directory file path.
    );
    /* Delete the specified directory. The parameterless version changes to the
       name contained in the object instance. The static version may be simply
       passed a path name.

       <H2>Returns:</H2>
       TRUE if directory was deleted.
     */
      

    BOOL Open(
      int scanMask = PFileInfo::AllFiles    // Mask of files to provide.
    );
    /* Open the directory for scanning its list of files. Once opened the
       <A>GetEntryName()</A> function may be used to get the current directory
       entry and the <A>Next()</A> function used to move to the next directory
       entry.
       
       Only files that are of a type that is specified in the mask will be
       returned.
       
       Note that the directory scan will <EM>not</EM> return the "." and ".."
       entries that some platforms support.

       <H2>Returns:</H2>
       TRUE if directory was successfully opened, and there was at least one
       file in it of the specified types.
     */
      
    BOOL Restart(
      int scanMask = PFileInfo::AllFiles    // Mask of files to provide.
    );
    /* Restart file list scan from the beginning of directory. This is similar
       to the <A>Open()</A> command but does not require that the directory be
       closed (using <A>Close()</A>) first.

       Only files that are of a type that is specified in the mask will be
       returned.

       Note that the directory scan will <EM>not</EM> return the "." and ".."
       entries that some platforms support.

       <H2>Returns:</H2>
       TRUE if directory was successfully opened, and there was at least one
       file in it of the specified types.
     */
      
    BOOL Next();
    /* Move to the next file in the directory scan.
    
       Only files that are of a type that is specified in the mask passed to
       the <A>Open()</A> or <A>Restart()</A> functions will be returned.

       Note that the directory scan will <EM>not</EM> return the "." and ".."
       entries that some platforms support.

       <H2>Returns:</H2>
       TRUS if there is another valid file in the directory.
     */
      
    void Close();
    // Close the directory during or after a file list scan.

    PFILE_PATH_STRING GetEntryName() const;
    /* Get the name (without the volume or directory path) of the current
       entry in the directory scan. This may be the name of a file or a
       subdirectory or even a link or device for operating systems that support
       them.
       
       To get a full path name concatenate the PDirectory object itself with
       the entry name.
       
       Note that the directory scan will <EM>not</EM> return the "." and ".."
       entries that some platforms support.

       <H2>Returns:</H2>
       string for directory entry.
     */

    BOOL IsSubDir() const;
    /* Determine if the directory entry currently being scanned is itself
       another directory entry.
       
       Note that the directory scan will <EM>not</EM> return the "." and ".."
       entries that some platforms support.

       <H2>Returns:</H2>
       TRUE if entry is a subdirectory.
     */

    BOOL GetInfo(
      PFileInfo & info    // Object to receive the file information.
    ) const;
    /* Get file information on the current directory entry.
    
       <H2>Returns:</H2>
       TRUE if file information was successfully retrieved.
     */


  protected:
    // New functions for class
    void Construct();
      // Common constructor code


    // Member variables
    int scanMask;
    // Mask of file types that the directory scan will return.


// Class declaration continued in platform specific header file ///////////////
