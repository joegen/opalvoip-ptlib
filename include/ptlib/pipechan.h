/*
 * pipechan.h
 *
 * Sub-process with communications using pipe I/O channel class.
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
 * $Log: pipechan.h,v $
 * Revision 1.16  1998/11/02 10:06:39  robertj
 * Added capability of pip output to go to stdout/stderr.
 *
 * Revision 1.15  1998/10/30 10:42:29  robertj
 * Better function arrangement for multi platforming.
 *
 * Revision 1.14  1998/10/29 11:29:17  robertj
 * Added ability to set environment in sub-process.
 *
 * Revision 1.13  1998/10/26 09:11:05  robertj
 * Added ability to separate out stdout from stderr on pipe channels.
 *
 * Revision 1.12  1998/09/23 06:21:08  robertj
 * Added open source copyright license.
 *
 * Revision 1.11  1997/01/03 05:25:05  robertj
 * Added Wait and Kill functions.
 *
 * Revision 1.10  1996/03/31 08:50:51  robertj
 * Changed string list to array.
 * Added function to idicate if sub-process is running.
 *
 * Revision 1.9  1995/07/31 12:15:45  robertj
 * Removed PContainer from PChannel ancestor.
 *
 * Revision 1.8  1995/06/17 11:12:53  robertj
 * Documentation update.
 *
 * Revision 1.7  1995/03/14 12:42:02  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.6  1995/01/09  12:39:01  robertj
 * Documentation.
 *
 * Revision 1.5  1994/10/23  04:50:55  robertj
 * Further refinement of semantics after implementation.
 *
 * Revision 1.4  1994/09/25  10:43:19  robertj
 * Added more implementation.
 *
 * Revision 1.3  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.2  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.1  1994/04/20  12:17:44  robertj
 * Initial revision
 *
 */


#define _PPIPECHANNEL

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PPipeChannel, PChannel)
/* A channel that uses a operating system pipe between the current process and
   a sub-process. On platforms that support <I>multi-processing</I>, the
   sub-program is executed concurrently with the calling process.
   
   Where full multi-processing is not supported then the sub-program is run
   with its input supplied from, or output captured to, a disk file. The
   current process is then suspended during the execution of the sub-program.
   In the latter case the semantics of the <A>Execute()</A> and <A>Close()</A>
   functions change from the usual for channels.

   Note that for platforms that do not support multi-processing, the current
   process is suspended until the sub-program terminates. The input and output
   of the sub-program is transferred via a temporary file. The exact moment of
   execution of the sub-program depends on the mode. If mode is
   <CODE>ReadOnly</CODE> then it is executed immediately and its output
   captured. In <CODE>WriteOnly</CODE> mode the sub-program is run when the
   <A>Close()</A> function is called, or when the pipe channel is destroyed.
   In <CODE>ReadWrite</CODE> mode the sub-program is run when the
   <A>Execute()</A> function is called indicating that the output from the
   current process to the sub-program has completed and input is now desired.
   
   The <A>CanReadAndWrite()</A> function effectively determines whether full
   multi-processing is supported by the platform. Note that this is different
   to whether <I>multi-threading</I> is supported.
 */

  public:
    enum OpenMode {
      ReadOnly,   // Pipe is only from the sub-process to the current process.
      WriteOnly,  // Pipe is only from the current process to the sub-process.
      ReadWrite,  // Pipe is bidirectional between current and sub-processes.
      ReadWriteStd
      /* Pipe is bidirectional between current and sub-processes but the write
         side goes to stdout and stderr */
    };
    // Channel mode for the pipe to the sub-process.

    PPipeChannel();
    PPipeChannel(
      const PString & subProgram,  // Sub program name or command line.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    PPipeChannel(
      const PString & subProgram,  // Sub program name or command line.
      const PStringArray & argumentList, // Array of arguments to sub-program.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    PPipeChannel(
      const PString & subProgram,  // Sub program name or command line.
      const PStringToString & environment, // Array of arguments to sub-program.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    PPipeChannel(
      const PString & subProgram,  // Sub program name or command line.
      const PStringArray & argumentList, // Array of arguments to sub-program.
      const PStringToString & environment, // Array of arguments to sub-program.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    /* Create a new pipe channel allowing the subProgram to be executed and
       data transferred from its stdin/stdout/stderr.
       
       See the <A>Open()</A> function for details of various parameters.
     */

  ~PPipeChannel();
  // Close the pipe channel, killing the sub-process.


  // Overrides from class PObject
    Comparison Compare(
      const PObject & obj   // Another pipe channel to compare against.
    ) const;
    /* Determine if the two objects refer to the same pipe channel. This
       actually compares the sub-program names that are passed into the
       constructor.

       <H2>Returns:</H2>
       Comparison value of the sub-program strings.
     */


  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the name of the channel.
    
       <H2>Returns:</H2>
       string for the sub-program that is run.
     */

    virtual BOOL Read(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Low level read from the channel. This function may block until the
       requested number of characters were read or the read timeout was
       reached. The GetLastReadCount() function returns the actual number
       of bytes read.

       If there are no more characters available as the sub-program has
       stopped then the number of characters available is returned. This is
       similar to end of file for the PFile channel.

       The GetErrorCode() function should be consulted after Read() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE indicates that at least one character was read from the channel.
       FALSE means no bytes were read due to timeout or some other I/O error.
     */

    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       If the sub-program has completed its run then this function will fail
       returning FALSE.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
     */

    virtual BOOL Close();
    /* Close the channel. This will kill the sub-program's process (on
       platforms where that is relevent).
       
       For <CODE>WriteOnly</CODE> or <CODE>ReadWrite</CODE> mode pipe channels
       on platforms that do no support concurrent multi-processing and have
       not yet called the <A>Execute()</A> function this will run the
       sub-program.
     */

  // New member functions
    BOOL Open(
      const PString & subProgram,  // Sub program name or command line.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    BOOL Open(
      const PString & subProgram,  // Sub program name or command line.
      const PStringArray & argumentList, // Array of arguments to sub-program.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    BOOL Open(
      const PString & subProgram,  // Sub program name or command line.
      const PStringToString & environment, // Array of arguments to sub-program.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    BOOL Open(
      const PString & subProgram,  // Sub program name or command line.
      const PStringArray & argumentList, // Array of arguments to sub-program.
      const PStringToString & environment, // Array of arguments to sub-program.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE,      // Flag for system PATH to be searched.
      BOOL stderrSeparate = FALSE  // Standard error is on separate pipe
    );
    /* Open a new pipe channel allowing the subProgram to be executed and
       data transferred from its stdin/stdout/stderr.
       
       If the mode is <CODE>ReadOnly</CODE> then the <CODE>stdout</CODE> of the
       sub-program is supplied via the <A>Read()</A> calls of the PPipeChannel.
       The sub-programs input is set to the platforms null device (eg
       /dev/nul).

       If mode is <CODE>WriteOnly</CODE> then <A>Write()</A> calls of the
       PPipeChannel are suppied to the sub-programs <CODE>stdin</CODE> and its
       <CODE>stdout</CODE> is sent to the null device.
       
       If mode is <CODE>ReadWrite</CODE> then both read and write actions can
       occur.

       The <CODE>subProgram</CODE> parameter may contain just the path of the
       program to be run or a program name and space separated arguments,
       similar to that provided to the platforms command processing shell.
       Which use of this parameter is determiend by whether arguments are
       passed via the <CODE>argumentPointers</CODE> or
       <CODE>argumentList</CODE> parameters.

       The <CODE>searchPath</CODE> parameter indicates that the system PATH
       for executables should be searched for the sub-program. If FALSE then
       only the explicit or implicit path contained in the
       <CODE>subProgram</CODE> parameter is searched for the executable.

       The <CODE>stderrSeparate</CODE> parameter indicates that the standard
       error stream is not included in line with the standard output stream.
       In this case, data in this stream must be read using the
       <A>ReadStandardError()</A> function.

       The <CODE>environment</CODE> parameter is a null terminated sequence
       of null terminated strings of the form name=value. If NULL is passed
       then the same invironment as calling process uses is passed to the
       child process.
     */

    const PFilePath & GetSubProgram() const;
    /* Get the full file path for the sub-programs executable file.

       <H2>Returns:</H2>
       file path name for sub-program.
     */

    BOOL Execute();
    /* Start execution of sub-program for platforms that do not support
       multi-processing, this will actually run the sub-program passing all
       data written to the PPipeChannel.
       
       For platforms that do support concurrent multi-processing this will
       close the pipe from the current process to the sub-process.
      
       As the sub-program is run immediately and concurrently, this will just
       give an end of file to the stdin of the remote process. This is often
       necessary
     */

    BOOL IsRunning() const;
    /* Determine if the program associated with the PPipeChannel is still
       executing. This is useful for determining the status of PPipeChannels
       which take a long time to execute on operating systems which support
       concurrent multi-processing.
       
       <H2>Returns:</H2>
       TRUE if the program associated with the PPipeChannel is still running
     */

    int GetReturnCode() const;
    /* Get the return code from the most recent Close;

       <H2>Returns:</H2>
       Return code from the closing process
     */

    int WaitForTermination();
    int WaitForTermination(
      const PTimeInterval & timeout  // Amount of time to wait for process.
    );
    /* This function will block and wait for the sub-program to terminate.
    
       <H2>Returns:</H2>
       Return code from the closing process
     */
    
    BOOL Kill(
      int signal = 9  // Signal code to be sent to process.
    );
    /* This function will terminate the sub-program using the signal code
       specified.
     
       <H2>Returns:</H2>
       TRUE if the process received the signal. Note that this does not mean
       that the process has actually terminated.
     */

    BOOL ReadStandardError(
      PString & errors,   // String to receive standard error text.
      BOOL wait = FALSE   // Flag to indicate if function should block
    );
    /* Read all available data on the standard error stream of the
       sub-process. If the <CODE>wait</CODE> parameter is FALSE then only
       the text currently available is returned. If TRUE then the function
       blocks as long as necessary to get some number of bytes.

       <H2>Returns:</H2>
       TRUE indicates that at least one character was read from stderr.
       FALSE means no bytes were read due to timeout or some other I/O error.
     */

    static BOOL CanReadAndWrite();
    /* Determine if the platform can support simultaneous read and writes from
       the PPipeChannel (eg MSDOS returns FALSE, Unix returns TRUE).
       
       <H2>Returns:</H2>
       TRUE if platform supports concurrent multi-processing.
     */


  protected:
    // Member variables
    PFilePath subProgName;
    // The fully qualified path name for the sub-program executable.


  private:
    BOOL PlatformOpen(const PString & subProgram,
                      const PStringArray & arguments,
                      OpenMode mode,
                      BOOL searchPath,
                      BOOL stderrSeparate,
                      const PStringToString * environment);


// Class declaration continued in platform specific header file ///////////////
