/*
 * $Id: pipechan.h,v 1.9 1995/07/31 12:15:45 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.h,v $
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
      ReadWrite   // Pipe is bidirectional between current and sub-processes.
    };
    // Channel mode for the pipe to the sub-process.

    PPipeChannel(
      const PString & subProgram,  // Sub program name or command line.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE       // Flag for system PATH to be searched.
    );
    PPipeChannel(
      const PString & subProgram,  // Sub program name or command line.
      const char * const * argumentPointers,
      /* This is an array of argument strings for the sub-program. The array
         is terminated with a NULL pointer. If the parameter itself is NULL
         then this is equivalent to first constructor, without the parameter
         at all.
       */
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE       // Flag for system PATH to be searched.
    );
    PPipeChannel(
      const PString & subProgram,  // Sub program name or command line.
      const PStringList & argumentList,  // List of arguments to sub-program.
      OpenMode mode = ReadWrite,   // Mode for the pipe channel.
      BOOL searchPath = TRUE       // Flag for system PATH to be searched.
    );
    /* Create a new pipe channel allowing the subProgram to be executed and
       data transferred from its stdin/stdout.
       
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
    void Construct(
      const PString & subProgram,  // Sub program name or command line.
      const char * const * argumentPointers,
      /* This is an array of argument strings for the sub-program. The array
         is terminated with a NULL pointer. If the parameter itself is NULL
         then this is equivalent to first constructor, without the parameter
         at all.
       */
      OpenMode mode,    // Mode for the pipe channel.
      BOOL searchPath   // Flag for system PATH to be searched.
    );
    /* Common, platform dependent, construction code for the pipe channel. This
       is called by all of the constructors.
     */


// Class declaration continued in platform specific header file ///////////////
