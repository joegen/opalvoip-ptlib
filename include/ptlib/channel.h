/*
 * $Id: channel.h,v 1.25 1997/07/08 13:15:03 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.25  1997/07/08 13:15:03  robertj
 * DLL support.
 *
 * Revision 1.24  1996/11/04 03:41:04  robertj
 * Added extra error message for UDP packet truncated.
 *
 * Revision 1.23  1996/09/14 13:09:17  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.22  1996/08/17 10:00:19  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.21  1996/07/27 04:15:07  robertj
 * Created static version of ConvertOSError().
 * Created static version of GetErrorText().
 *
 * Revision 1.20  1996/05/26 03:24:40  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.19  1996/04/15 12:33:03  robertj
 * Fixed SetReadTimeout/SetWriteTimeout to use const reference so works with GNU compiler.
 *
 * Revision 1.18  1996/04/14 02:53:30  robertj
 * Split serial and pipe channel into separate compilation units for Linux executable size reduction.
 *
 * Revision 1.17  1996/02/19 13:12:48  robertj
 * Added new error code for interrupted I/O.
 *
 * Revision 1.16  1996/01/23 13:09:14  robertj
 * Mac Metrowerks compiler support.
 *
 * Revision 1.15  1995/08/12 22:28:22  robertj
 * Work arounf for  GNU bug: can't have private copy constructor with multiple inheritance.
 *
 * Revision 1.14  1995/07/31 12:15:42  robertj
 * Removed PContainer from PChannel ancestor.
 *
 * Revision 1.13  1995/06/17 11:12:21  robertj
 * Documentation update.
 *
 * Revision 1.12  1995/06/04 08:42:00  robertj
 * Fixed comment.
 *
 * Revision 1.11  1995/03/14 12:41:03  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.10  1995/03/12  04:36:53  robertj
 * Moved GetHandle() function from PFile to PChannel.
 *
 * Revision 1.9  1994/12/21  11:52:48  robertj
 * Documentation and variable normalisation.
 *
 * Revision 1.8  1994/11/28  12:31:40  robertj
 * Documentation.
 *
 * Revision 1.7  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.6  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.5  1994/08/21  23:43:02  robertj
 * Moved meta-string transmitter from PModem to PChannel.
 * Added common entry point to convert OS error to PChannel error.
 *
 * Revision 1.4  1994/07/17  10:46:06  robertj
 * Unix support changes.
 *
 * Revision 1.3  1994/07/02  03:03:49  robertj
 * Changed to allow for platform dependent part.
 *
 * Revision 1.2  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.1  1994/04/20  12:17:44  robertj
 * Initial revision
 *
 */


#define _PCHANNEL

#ifdef __GNUC__
#pragma interface
#endif



///////////////////////////////////////////////////////////////////////////////
// I/O Channels

class PChannel;

class PChannelStreamBuffer : public streambuf {
/* This class is necessary for implementing the standard C++ iostream interface
   on <A>PChannel</A> classes and its descendents. It is an internal class and
   should not ever be used by application writers.
 */

  protected:
    PChannelStreamBuffer(
      PChannel * chan   // Channel the buffer operates on.
    );
    /* Construct the streambuf for standard streams on a channel. This is used
       internally by the <A>PChannel</A> class.
     */

    virtual int overflow(int=EOF);
    virtual int underflow();
    virtual int sync();
#ifdef __MWERKS__
    virtual streampos seekoff(streamoff, ios::seekdir, ios::openmode);
#else
    virtual streampos seekoff(streamoff, ios::seek_dir, int);
#endif

  private:
    // Member variables
    PChannel * channel;
    char buffer[1024];

  public:
    PChannelStreamBuffer(const PChannelStreamBuffer & sbuf);
    PChannelStreamBuffer & operator=(const PChannelStreamBuffer & sbuf);

  friend class PChannel;
};


class PChannel : public PObject, public iostream {
  PCLASSINFO(PChannel, PObject)
/* Abstract class defining I/O channel semantics. An I/O channel can be a
   serial port, pipe, network socket or even just a simple file. Anything that
   requires opening and closing then reading and/or writing from.

   A descendent would typically have constructors and an Open() function which
   enables access to the I/O channel it represents. The <A>Read()</A> and
   <A>Write()</A> functions would then be overridden to the platform and I/O
   specific mechanisms required.

   The general model for a channel is that the channel accepts and/or supplies
   a stream of bytes. The access to the stream of bytes is via a set of
   functions that allow certain types of transfer. These include direct
   transfers, buffered transfers (via iostream) or asynchronous transfers.

   The model also has the fundamental state of the channel being <I>open</I>
   or <I>closed</I>. A channel instance that is closed contains sufficient
   information to describe the channel but does not allocate or lock any
   system resources. An open channel allocates or locks the particular system
   resource. The act of opening a channel is a key event that may fail. In this
   case the channel remains closed and error values are set.
 */

  public:
    PChannel();
      // Create the channel.

    ~PChannel();
      // Close down the channel.


    // New functions for class
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For example,
       it would return the filename in <A>PFile</A> type channels.

       <H2>Returns:</H2>
       the name of the channel.
     */

    virtual BOOL Close();
    /* Close the channel, shutting down the link to the data source.

       <H2>Returns:</H2>
       TRUE if the channel successfully closed.
     */

    virtual BOOL IsOpen() const;
    /* Determine if the channel is currently open and read and write operations
       can be executed on it. For example, in the <A>PFile</A> class it returns
       if the file is currently open.

       <H2>Returns:</H2>
       TRUE if the channel is open.
     */


    void SetReadTimeout(
      const PTimeInterval & time  // The new time interval for read operations.
    );
    /* Set the timeout for read operations. This may be zero for immediate
       return of data through to PMaxMilliseconds which will wait forever for
       the read request to be filled.
       
       Note that this function may not be available, or meaningfull, for all
       channels. In that case it is set but ignored.
     */

    PTimeInterval GetReadTimeout() const;
    /* Get the timeout for read operations. Note that this function may not be
       available, or meaningfull, for all channels. In that case it returns the
       previously set value.

       <H2>Returns:</H2>
       time interval for read operations.
     */

    virtual BOOL Read(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Low level read from the channel. This function may block until the
       requested number of characters were read or the read timeout was
       reached. The GetLastReadCount() function returns the actual number
       of bytes read.

       The GetErrorCode() function should be consulted after Read() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE indicates that at least one character was read from the channel.
       FALSE means no bytes were read due to timeout or some other I/O error.
     */

    PINDEX GetLastReadCount() const;
    /* Get the number of bytes read by the last Read() call. This will be from
       0 to the maximum number of bytes as passed to the Read() call.
       
       Note that the number of bytes read may often be less than that asked
       for. Aside from the most common case of being at end of file, which the
       applications semantics may regard as an exception, there are some cases
       where this is normal. For example, if a <A>PTextFile</A> channel on the
       MSDOS platform is read from, then the translation of CR/LF pairs to \n
       characters will result in the number of bytes returned being less than
       the size of the buffer supplied.

       <H2>Returns:</H2>
       the number of bytes read.
     */

    virtual int ReadChar();
    /* Read a single 8 bit byte from the channel. If one was not available
       within the read timeout period, or an I/O error occurred, then the
       function gives with a -1 return value.

       <H2>Returns:</H2>
       byte read or -1 if no character could be read.
     */

    PString ReadString(PINDEX len);
    /* Read up to len bytes into a string from the channel. This function
       simply uses Read(), so all remarks pertaining to that function also
       apply to this one.
     */

    virtual BOOL ReadAsync(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Begin an asynchronous read from channel. The read timeout is used as in
       other read operations, in this case calling the OnReadComplete()
       function.

       Note that if the channel is not capable of asynchronous read then this
       will do a sychronous read is in the Read() function with the addition
       of calling the OnReadComplete() before returning.

       <H2>Returns:</H2>
       TRUE if the read was sucessfully queued.
     */

    virtual void OnReadComplete(
      void * buf, // Pointer to a block of memory that received the read bytes.
      PINDEX len  // Actual number of bytes to read into the buffer.
    );
    /* User callback function for when a ReadAsync() call has completed or
       timed out. The original pointer to the buffer passed in ReadAsync() is
       passed to the function.
     */


    void SetWriteTimeout(
      const PTimeInterval & time // The new time interval for write operations.
    );
    /* Set the timeout for write operations to complete. This may be zero for
       immediate return through to PMaxMilliseconds which will wait forever for
       the write request to be completed.
       
       Note that this function may not be available, or meaningfull,  for all
       channels. In this case the parameter is et but ignored.
     */

    PTimeInterval GetWriteTimeout() const;
    /* Get the timeout for write operations to complete. Note that this
       function may not be available, or meaningfull, for all channels. In
       that case it returns the previously set value.

       <H2>Returns:</H2>
       time interval for writing.
     */

    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
     */

    PINDEX GetLastWriteCount() const;
    /* Get the number of bytes written by the last Write() call.
       
       Note that the number of bytes written may often be less, or even more,
       than that asked for. A common case of it being less is where the disk
       is full. An example of where the bytes written is more is as follows.
       On a <A>PTextFile</A> channel on the MSDOS platform, there is
       translation of \n to CR/LF pairs. This will result in the number of
       bytes returned being more than that requested.

       <H2>Returns:</H2>
       the number of bytes written.
     */

    BOOL WriteChar(int c);
    /* Write a single character to the channel. This function simply uses the
       Write() function so all comments on that function also apply.
       
       Note that this asserts if the value is not in the range 0..255.

       <H2>Returns:</H2>
       TRUE if the byte was successfully written.
     */

    BOOL WriteString(const PString & str);
    /* Write a string to the channel. This function simply uses the Write()
       function so all comments on that function also apply.

       <H2>Returns:</H2>
       TRUE if the string was completely written.
     */

    virtual BOOL WriteAsync(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Begin an asynchronous write from channel. The write timeout is used as
       in other write operations, in this case calling the OnWriteComplete()
       function. Note that if the channel is not capable of asynchronous write
       then this will do a sychronous write as in the Write() function with
       the addition of calling the OnWriteComplete() before returning.

       <H2>Returns:</H2>
       TRUE of the write operation was succesfully queued.
     */

    virtual void OnWriteComplete(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* User callback function for when a WriteAsync() call has completed or
       timed out. The original pointer to the buffer passed in WriteAsync() is
       passed in here and the len parameter is the actual number of characters
       written.
     */


    BOOL SendCommandString(
      const PString & command  // Command to send to the channel
    );
    /* Send a command meta-string. A meta-string is a string of characters
       that may contain escaped commands. The escape command is the \ as in
       the C language.

       The escape commands are:

          <DL>
          <DT><CODE>\a</CODE><DD>    alert (ascii value 7)
          <DT><CODE>\b</CODE><DD>    backspace (ascii value 8)
          <DT><CODE>\f</CODE><DD>    formfeed (ascii value 12)
          <DT><CODE>\n</CODE><DD>    newline (ascii value 10)
          <DT><CODE>\r</CODE><DD>    return (ascii value 13)
          <DT><CODE>\t</CODE><DD>    horizontal tab (ascii value 9)
          <DT><CODE>\v</CODE><DD>    vertical tab (ascii value 11)
          <DT><CODE>\\</CODE><DD>    backslash
          <DT><CODE>\ooo</CODE><DD>  where ooo is octal number, ascii value ooo
          <DT><CODE>\xhh</CODE><DD>  where hh is hex number (ascii value 0xhh)
          <DT><CODE>\0</CODE><DD>    null character (ascii zero)
          <DT><CODE>\dns</CODE><DD>  delay for n seconds
          <DT><CODE>\dnm</CODE><DD>  delay for n milliseconds
          <DT><CODE>\s</CODE><DD>    characters following this, up to a \w
                                     command or the end of string, are to be
                                     sent to modem
          <DT><CODE>\wns</CODE><DD>  characters following this, up to a \s, \d
                                     or another \w or the end of the string are
                                     expected back from the modem. If the
                                     string is not received within n seconds,
                                     a failed command is registered. The
                                     exception to this is if the command is at
                                     the end of the string or the next
                                     character in the string is the \s, \d or
                                     \w in which case all characters are
                                     ignored from the modem until n seconds of
                                     no data.
          <DT><CODE>\wnm</CODE><DD>  as for above but timeout is in
                                     milliseconds.

       <H2>Returns:</H2>
       TRUE if the command string was completely processed.
     */

    void AbortCommandString();
    /* Abort a command string that is in progress. Note that as the
       SendCommandString() function blocks the calling thread when it runs,
       this can only be called from within another thread.
     */


    int GetHandle() const;
    /* Get the integer operating system handle for the channel.

       <H2>Returns:</H2>
       standard OS descriptor integer.
     */


    enum ShutdownValue {
      ShutdownRead         = 0,
      ShutdownWrite        = 1,
      ShutdownReadAndWrite = 2
    };

    virtual BOOL Shutdown(
      ShutdownValue option
    );
    /* Close one or both of the data streams associated with a channel.

       The default behavour is to do nothing and return FALSE.

       <H2>Returns:</H2>
       TRUE if the shutdown was successfully performed.
     */


    virtual PChannel * GetBaseReadChannel() const;
    /* This function returns the eventual base channel for reading of a series
       of indirect channels provided by descendents of <A>PIndirectChannel</A>.

       The behaviour for this function is to return "this".
       
       <H2>Returns:</H2>
       Pointer to base I/O channel for the indirect channel.
     */

    virtual PChannel * GetBaseWriteChannel() const;
    /* This function returns the eventual base channel for writing of a series
       of indirect channels provided by descendents of <A>PIndirectChannel</A>.

       The behaviour for this function is to return "this".
       
       <H2>Returns:</H2>
       Pointer to base I/O channel for the indirect channel.
     */


    enum Errors {
      NoError,
      NotFound,       // Open fail due to device or file not found
      FileExists,     // Open fail due to file already existing
      DiskFull,       // Write fail due to disk full
      AccessDenied,   // Operation fail due to insufficient privilege
      DeviceInUse,    // Open fail due to device already open for exclusive use
      BadParameter,   // Operation fail due to bad parameters
      NoMemory,       // Operation fail due to insufficient memory
      NotOpen,        // Operation fail due to channel not being open yet
      Timeout,        // Operation failed due to a timeout
      Interrupted,    // Operation was interrupted
      BufferTooSmall, // Operations buffer was too small for data.
      Miscellaneous
    };
    Errors GetErrorCode() const;
      // Return the error result of the last file I/O operation in this object.

    int GetErrorNumber() const;
      // Return the operating system error number of the last file I/O
      // operation in this object.

    PString GetErrorText() const;
    static PString GetErrorText(Errors lastError, int osError = 0);
      // Return a string indicating the error message that may be displayed to
      // the user. The error for the last I/O operation in this object is used.


  protected:
    PChannel(const PChannel &);
    PChannel & operator=(const PChannel &);
    // Prevent usage by external classes


    BOOL ConvertOSError(int error);
    static BOOL ConvertOSError(int error, Errors & lastError, int & osError);
    /* Convert an operating system error into platform independent error. This
       will set the lastError and osError member variables for access by
       GetErrorCode() and GetErrorNumber().
       
       <H2>Returns:</H2>
       TRUE if there was no error.
     */

    int ReadCharWithTimeout(PTimeInterval & timeout);
      // Read a character with specified timeout, adjust for amount of time it took.

    BOOL ReceiveCommandString(int nextChar,
                            const PString & reply, PINDEX & pos, PINDEX start);
      // Receive a (partial) command string, determine if completed yet.


    // Member variables
    int os_handle;
      // The operating system file handle return by standard open() function.

    Errors lastError;
      // The platform independant error code.

    int osError;
      // The operating system error number (eg as returned by errno).

    PINDEX lastReadCount;
      // Number of byte last read by the Read() function.

    PINDEX lastWriteCount;
      // Number of byte last written by the Write() function.

    PTimeInterval readTimeout;
      // Timeout for read operations.

    PTimeInterval writeTimeout;
      // Timeout for write operations.


  private:
    // New functions for class
    void Construct();
      // Complete platform dependent construction.

    // Member variables
    BOOL abortCommandString;
      // Flag to abort the transmission of a command in SendCommandString().


// Class declaration continued in platform specific header file ///////////////
