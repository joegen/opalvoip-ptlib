/*
 * channel.h
 *
 * I/O channel ancestor class.
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

#ifndef PTLIB_CHANNEL_H
#define PTLIB_CHANNEL_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/mutex.h>

///////////////////////////////////////////////////////////////////////////////
// I/O Channels

class PChannel;

/* Buffer class used in PChannel stream.
This class is necessary for implementing the standard C++ iostream interface
on <code>PChannel</code> classes and its descendents. It is an internal class and
should not ever be used by application writers.
*/
class PChannelStreamBuffer : public streambuf {

  protected:
    /* Construct the streambuf for standard streams on a channel. This is used
       internally by the <code>PChannel</code> class.
     */
    PChannelStreamBuffer(
      PChannel * chan   // Channel the buffer operates on.
    );

    virtual int_type overflow(int_type = EOF);
    virtual int_type underflow();
    virtual int sync();
    virtual pos_type seekoff(std::streamoff, ios_base::seekdir, ios_base::openmode = ios_base::in | ios_base::out);
    virtual pos_type seekpos(pos_type, ios_base::openmode = ios_base::in | ios_base::out);

    PBoolean SetBufferSize(
      PINDEX newSize
    );

  private:
    // Member variables
    PChannel * channel;
    PCharArray input, output;

  public:
    PChannelStreamBuffer(const PChannelStreamBuffer & sbuf);
    PChannelStreamBuffer & operator=(const PChannelStreamBuffer & sbuf);

  friend class PChannel;
};


/** Abstract class defining I/O channel semantics. An I/O channel can be a
   serial port, pipe, network socket or even just a simple file. Anything that
   requires opening and closing then reading and/or writing from.

   A descendent would typically have constructors and an open function which
   enables access to the I/O channel it represents. The <code>Read()</code> and
   <code>Write()</code> functions would then be overridden to the platform and I/O
   specific mechanisms required.

   The general model for a channel is that the channel accepts and/or supplies
   a stream of bytes. The access to the stream of bytes is via a set of
   functions that allow certain types of transfer. These include direct
   transfers, buffered transfers (via iostream) or asynchronous transfers.

   The model also has the fundamental state of the channel being <code>open</code>
   or <code>closed</code>. A channel instance that is closed contains sufficient
   information to describe the channel but does not allocate or lock any
   system resources. An open channel allocates or locks the particular system
   resource. The act of opening a channel is a key event that may fail. In this
   case the channel remains closed and error values are set.
 */
class PChannel : public PObject, public std::iostream
{
  PCLASSINFO(PChannel, PObject);

  /**@name Construction */
  //@{
  protected:
      /// Create the channel.
    PChannel();

  public:
      /// Close down the channel.
    ~PChannel();
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Get the relative rank of the two strings. The system standard function,
       eg strcmp(), is used.

       @return
       comparison of the two objects, <code>EqualTo</code> for same,
       <code>LessThan</code> for <code>obj</code> logically less than the
       object and <code>GreaterThan</code> for <code>obj</code> logically
       greater than the object.
     */
    virtual Comparison Compare(
      const PObject & obj   ///< Other PString to compare against.
    ) const;

    /**Calculate a hash value for use in sets and dictionaries.
    
       The hash function for strings will produce a value based on the sum of
       the first three characters of the string. This is a fairly basic
       function and make no assumptions about the string contents. A user may
       descend from PString and override the hash function if they can take
       advantage of the types of strings being used, eg if all strings start
       with the letter 'A' followed by 'B or 'C' then the current hash function
       will not perform very well.

       @return
       hash value for string.
     */
    virtual PINDEX HashFunction() const;
  //@}

  /**@name Information functions */
  //@{
    /** Determine if the channel is currently open.
       This indicates that read and write operations can be executed on the
       channel. For example, in the <code>PFile</code> class it returns if the file is
       currently open.

       @return true if the channel is open.
     */
    virtual PBoolean IsOpen() const;

    /** Get the platform and I/O channel type name of the channel. For example,
       it would return the filename in <code>PFile</code> type channels.

       @return the name of the channel.
     */
    virtual PString GetName() const = 0;

    /** Get the integer operating system handle for the channel.

       @return
       standard OS descriptor integer.
     */
    virtual P_INT_PTR GetHandle() const;

    /** Re-open the device using the stdio library. This closes the PChannel

       @return
       stdio FILE * handle for the opened device.
     */
    FILE * FDOpen(const char * mode);

    /** Get the base channel of channel indirection using PIndirectChannel.
       This function returns the eventual base channel for reading of a series
       of indirect channels provided by descendents of <code>PIndirectChannel</code>.

       The behaviour for this function is to return "this".
       
       @return
       Pointer to base I/O channel for the indirect channel.
     */
    virtual PChannel * GetBaseReadChannel() const;

    /** Get the base channel of channel indirection using PIndirectChannel.
       This function returns the eventual base channel for writing of a series
       of indirect channels provided by descendents of <code>PIndirectChannel</code>.

       The behaviour for this function is to return "this".
       
       @return
       Pointer to base I/O channel for the indirect channel.
     */
    virtual PChannel * GetBaseWriteChannel() const;
  //@}

  /**@name Error functions */
  //@{
    /** Normalised error codes.
        The error result of the last file I/O operation in this object.
     */
    enum Errors {
      NoError,        ///< No error
      NotFound,       ///< Open fail due to device or file not found
      FileExists,     ///< Open fail due to file already existing
      DiskFull,       ///< Write fail due to disk full
      AccessDenied,   ///< Operation fail due to insufficient privilege
      DeviceInUse,    ///< Open fail due to device already open for exclusive use
      BadParameter,   ///< Operation fail due to bad parameters
      NoMemory,       ///< Operation fail due to insufficient memory
      NotOpen,        ///< Operation fail due to channel not being open yet
      Timeout,        ///< Operation failed due to a timeout
      Interrupted,    ///< Operation was interrupted
      BufferTooSmall, ///< Operations buffer was too small for data.
      Miscellaneous,  ///< Miscellaneous error.
      ProtocolFailure,///< High level protocol failure
      Unavailable,    ///< Cannot use channel at this time
      NumNormalisedErrors
    };

    /**Error groups.
       To aid in multithreaded applications where reading and writing may be
       happening simultaneously, read and write errors are separated from
       other errors.
      */
    enum ErrorGroup {
      LastReadError,      ///< Error during Read() operation
      LastWriteError,     ///< Error during Write() operation
      LastGeneralError,   ///< Error during other operation, eg Open()
      NumErrorGroups
    };

    /** Get normalised error code.
      Return the error result of the last file I/O operation in this object.
      @return Normalised error code.
      */
    Errors GetErrorCode(
      ErrorGroup group = NumErrorGroups   ///< Error group to get
    ) const;

    /** Get OS errro code.
      Return the operating system error number of the last file I/O
      operation in this object.
      @return Operating System error code.
      */
    int GetErrorNumber(
      ErrorGroup group = NumErrorGroups   ///< Error group to get
    ) const;

    /** Get error message description.
        Return a string indicating the error message that may be displayed to
       the user. The error for the last I/O operation in this object is used.
       @return Operating System error description string.
     */
    virtual PString GetErrorText(
      ErrorGroup group = NumErrorGroups   ///< Error group to get
    ) const;

    /** Get error message description.
       Return a string indicating the error message that may be displayed to
       the user. The <code>osError</code> parameter is used unless zero, in which case
       the <code>lastError</code> parameter is used.
       @return Operating System error description string.
     */
    static PString GetErrorText(
      Errors lastError,   ///< Error code to translate.
      int osError = 0     ///< OS error number to translate.
    );
  //@}

  /**@name Reading functions */
  //@{
    /** Set the timeout for read operations. This may be zero for immediate
       return of data through to <code>PMaxTimeInterval</code> which will wait forever for
       the read request to be filled.
       
       Note that this function may not be available, or meaningfull, for all
       channels. In that case it is set but ignored.
     */
    void SetReadTimeout(
      const PTimeInterval & time  ///< The new time interval for read operations.
    );

    /** Get the timeout for read operations. Note that this function may not be
       available, or meaningfull, for all channels. In that case it returns the
       previously set value.

       @return time interval for read operations.
     */
    PTimeInterval GetReadTimeout() const;

    /** Low level read from the channel. This function may block until the
       requested number of characters were read or the read timeout was
       reached. The GetLastReadCount() function returns the actual number
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

    /**Get the number of bytes read by the last Read() call. This will be from
       0 to the maximum number of bytes as passed to the Read() call.
       
       Note that the number of bytes read may often be less than that asked
       for. Aside from the most common case of being at end of file, which the
       applications semantics may regard as an exception, there are some cases
       where this is normal. For example, if a PTextFile channel on the
       MSDOS platform is read from, then the translation of CR/LF pairs to \n
       characters will result in the number of bytes returned being less than
       the size of the buffer supplied.

       @return
       the number of bytes read.
     */
    virtual PINDEX GetLastReadCount() const;
    virtual PINDEX SetLastReadCount(PINDEX count);

    /** Read a single character from the channel. If one was not available
       within the read timeout period, or an I/O error occurred, then the
       function gives with a -1 return value.

       Note, normally the character read will be an 8 bit value, but for some
       derived classes, e.g. PConsoleChannel, this may be a code for special
       input.

       @return
       byte read or -1 if no character could be read.
     */
    virtual int ReadChar();

    /** Read len bytes into the buffer from the channel. This function uses
       Read(), so most remarks pertaining to that function also apply to this
       one. The only difference being that this function will not return until
       all of the bytes have been read, or an error occurs.

       @return
       true if the read of <code>len</code> bytes was sucessfull.
     */
    PBoolean ReadBlock(
      void * buf,   ///< Pointer to a block of memory to receive the read bytes.
      PINDEX len    ///< Maximum number of bytes to read into the buffer.
    );

    /** Read <code>len</code> character into a string from the channel. This
       function simply uses ReadBlock(), so all remarks pertaining to that
       function also apply to this one.

       @return
       String that was read.
     */
    PString ReadString(
      PINDEX len  ///< Length of string data to read.
    );
  //@}

  /**@name Writing functions */
  //@{
    /** Set the timeout for write operations to complete. This may be zero for
       immediate return through to PMaxTimeInterval which will wait forever for
       the write request to be completed.
       
       Note that this function may not be available, or meaningfull,  for all
       channels. In this case the parameter is et but ignored.
     */
    void SetWriteTimeout(
      const PTimeInterval & time ///< The new time interval for write operations.
    );

    /** Get the timeout for write operations to complete. Note that this
       function may not be available, or meaningfull, for all channels. In
       that case it returns the previously set value.

       @return
       time interval for writing.
     */
    PTimeInterval GetWriteTimeout() const;

    /** Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       false to determine what caused the failure.

       @return
       true if at least len bytes were written to the channel.
     */
    virtual PBoolean Write(
      const void * buf, ///< Pointer to a block of memory to write.
      PINDEX len        ///< Number of bytes to write.
    );

	/** Low level write to the channel with marker.	(H323plus Implementation)
       This function will block until the requested number of characters	 
       are written or the write timeout is reached. The GetLastWriteCount()	 
       function returns the actual number of bytes written. By default it	 
       calls the Write(void *,len) function	 
 	 
       The GetErrorCode() function should be consulted after Write() returns	 
       false to determine what caused the failure.	 
 	 
       @return	 
       true if at least len bytes were written to the channel.	 
      */	 
     virtual PBoolean Write(	 
       const void * buf,       ///< Pointer to a block of memory to write.	 
       PINDEX len,             ///< Number of bytes to write.	 
       const void * /*mark*/   ///< pointer for AVsync & render buffering.	 
     ) { return Write(buf,len); }	 
 

    /** Get the number of bytes written by the last Write() call.
       
       Note that the number of bytes written may often be less, or even more,
       than that asked for. A common case of it being less is where the disk
       is full. An example of where the bytes written is more is as follows.
       On a <code>PTextFile</code> channel on the MSDOS platform, there is
       translation of \n to CR/LF pairs. This will result in the number of
       bytes returned being more than that requested.

       @return
       the number of bytes written.
     */
    virtual PINDEX GetLastWriteCount() const;
    virtual PINDEX SetLastWriteCount(PINDEX count);

    /** Write a single character to the channel. This function simply uses the
       Write() function so all comments on that function also apply.
       
       Note that this asserts if the value is not in the range 0..255.

       @return
       true if the byte was successfully written.
     */
    PBoolean WriteChar(int c);

    /** Write a string to the channel. This function simply uses the Write()
       function so all comments on that function also apply.

       @return
       true if the character written.
     */
    PBoolean WriteString(
      const PString & str   ///< String to write
    );
  //@}

  /**@name Asynchronous I/O functions */
  //@{
    class AsyncContext;
    typedef PNotifierTemplate<PChannel::AsyncContext &> AsyncNotifier;
    #define PDECLARE_AsyncNotifier(cls, fn) PDECLARE_NOTIFIER2(PChannel, cls, fn, PChannel::AsyncContext &)
    #define PCREATE_AsyncNotifier(fn) PCREATE_NOTIFIER2(fn, PChannel::AsyncContext &)

    /** Parameters for asynchronous I/O operation.
      */
#ifdef _WIN32
    typedef OVERLAPPED AsyncContextBase;
#elif defined _AIO_H
    typedef struct aiocb AsyncContextBase;
#else
    class AsyncContextBase { off_t m_offset; };
#endif
    class AsyncContext : public AsyncContextBase
    {
      public:
        AsyncContext(
          void * buf = NULL,
          PINDEX len = 0,
          const AsyncNotifier & notifier = AsyncNotifier()
        );

        /** Set the offset to do the read/write operation.
          */
        void SetOffset(off_t offset);

        void        * m_buffer;       ///< Pointer to a block of memory to receive the read bytes.
        PINDEX        m_length;       ///< Maximum number of bytes to read into the buffer.
        AsyncNotifier m_notifier;     ///< Notification function for when asynchronous operation complete.

        Errors        m_errorCode;    ///< Error returned after operation completed.
        int           m_errorNumber;  ///< OS error returned after operation completed.

        // Internal stuff
        PChannel * m_channel;
        typedef void (PChannel::*CompletionFunction)(AsyncContext &);
        P_ALIGN_FIELD(CompletionFunction,m_onComplete,16);
        bool Initialise(PChannel * channel, CompletionFunction onComplete);
        void OnIOComplete(PINDEX length, int errorNumber);

        PNEW_AND_DELETE_FUNCTIONS(16)
    };

    /** Begin an asynchronous read from channel. The read timeout is used as in
       other read operations, in this case calling the OnReadComplete()
       function.

       If the channel is not capable of asynchronous read then returns false.

       Note: the life time of the context and the m_buffer within that context
       must exceed the duration of the asynchronous operation. After
       OnReadComplete has been called, these objects may be destroyed.

       @return
       true if the read was sucessfully queued.
     */
    virtual bool ReadAsync(
      AsyncContext & context ///< Context for asynchronous operation
    );

    /** User callback function for when a <code>ReadAsync()</code> call has completed or
       timed out.

       The default behaviour calls the m_callback field of context.
     */
    virtual void OnReadComplete(
      AsyncContext & context ///< Context for asynchronous operation
    );

    /** Begin an asynchronous write from channel. The write timeout is used as
       in other write operations, in this case calling the OnWriteComplete()
       function.

       If the channel is not capable of asynchronous write then returns false.

       Note: the life time of the context and the m_buffer within that context
       must exceed the duration of the asynchronous operation. After
       OnWriteComplete has been called, these objects may be destroyed.

       @return
       true of the write operation was succesfully queued.
     */
    virtual bool WriteAsync(
      AsyncContext & context ///< Context for asynchronous operation
    );

    /** User callback function for when a WriteAsync() call has completed or
       timed out.

       The default behaviour calls the m_callback field of context.
     */
    virtual void OnWriteComplete(
      AsyncContext & context ///< Context for asynchronous operation
    );
  //@}

  /**@name Miscellaneous functions */
  //@{
    /** Close the channel, shutting down the link to the data source.

       @return true if the channel successfully closed.
     */
    virtual PBoolean Close();

    enum ShutdownValue {
      ShutdownRead         = 0,
      ShutdownWrite        = 1,
      ShutdownReadAndWrite = 2
    };

    /** Close one or both of the data streams associated with a channel.

       The default behavour is to do nothing and return false.

       @return
       true if the shutdown was successfully performed.
     */
    virtual PBoolean Shutdown(
      ShutdownValue option
    );

    /**Set local echo mode.
       For some classes of channel, e.g. PConsoleChannel, data read by this
       channel is automatically echoed. This disables the function so things
       like password entry can work.

       Default behaviour does nothing and return true if the channel is open.
      */
    virtual bool SetLocalEcho(
      bool localEcho
    );

    /**Flow Control information 
       Pass data to the channel for flowControl determination.
      */
    virtual bool FlowControl(const void * flowData);

    /**Set the iostream buffer size for reads and writes.

       @return
       true if the new buffer size was set.
      */
    PBoolean SetBufferSize(
      PINDEX newSize    ///< New buffer size
    );

    /** Send a command meta-string. A meta-string is a string of characters
       that may contain escaped commands. The escape command is the \ as in
       the C language.

       The escape commands are:
          <table border=0>
          <tr><td>\\a    <td>alert (ascii value 7)
          <tr><td>\\b    <td>backspace (ascii value 8)
          <tr><td>\\f    <td>formfeed (ascii value 12)
          <tr><td>\\n    <td>newline (ascii value 10)
          <tr><td>\\r    <td>return (ascii value 13)
          <tr><td>\\t    <td>horizontal tab (ascii value 9)
          <tr><td>\\v    <td>vertical tab (ascii value 11)
          <tr><td>\\\\   <td> backslash
          <tr><td>\\ooo  <td>where ooo is octal number, ascii value ooo
          <tr><td>\\xhh  <td>where hh is hex number (ascii value 0xhh)
          <tr><td>\\0    <td>null character (ascii zero)
          <tr><td>\\dns  <td>delay for n seconds
          <tr><td>\\dnm  <td>delay for n milliseconds
          <tr><td>\\s    <td>characters following this, up to a \\w
                          command or the end of string, are to be
                          sent to modem
          <tr><td>\\wns  <td>characters following this, up to a \\s, \\d
                          or another \\w or the end of the string are
                          expected back from the modem. If the
                          string is not received within n seconds,
                          a failed command is registered. The
                          exception to this is if the command is at
                          the end of the string or the next
                          character in the string is the \\s, \\d or
                          \\w in which case all characters are
                          ignored from the modem until n seconds of
                          no data.
          <tr><td>\\wnm  <td>as for above but timeout is in milliseconds.
          </table>
       @return
       true if the command string was completely processed.
     */
    PBoolean SendCommandString(
      const PString & command  ///< Command to send to the channel
    );

    /** Abort a command string that is in progress. Note that as the
       SendCommandString() function blocks the calling thread when it runs,
       this can only be called from within another thread.
     */
    void AbortCommandString();
  //@}

  protected:
    PChannel(const PChannel &);
    PChannel & operator=(const PChannel &);
    // Prevent usage by external classes


    /** Convert an operating system error into platform independent error.
      The internal error codes are set by this function. They may be obtained
      via the <code>GetErrorCode()</code> and <code>GetErrorNumber()</code> functions.
       
       @return true if there was no error.
     */
    virtual PBoolean ConvertOSError(
      P_INT_PTR libcReturnValue,                ///< Return value from standard library
      ErrorGroup group = LastGeneralError ///< Error group to set
    );

  public:
    /**Set error values to those specified.
       Return true if errorCode is NoError, false otherwise
      */
    PBoolean SetErrorValues(
      Errors errorCode,   ///< Error code to translate.
      int osError,        ///< OS error number to translate.
      ErrorGroup group = LastGeneralError ///< Error group to set
    );

  protected:
    /** Read a character with specified timeout.
      This reads a single character from the channel waiting at most the
      amount of time specified for it to arrive. The <code>timeout</code> parameter
      is adjusted for amount of time it actually took, so it can be used
      for a multiple character timeout.

       @return true if there was no error.
     */
    int ReadCharWithTimeout(
      PTimeInterval & timeout  // Timeout for read.
    );

    // Receive a (partial) command string, determine if completed yet.
    PBoolean ReceiveCommandString(
      int nextChar,
      const PString & reply,
      PINDEX & pos,
      PINDEX start
    );

    bool CheckNotOpen();
    virtual int os_errno() const;

    // Member variables
    PTimeInterval readTimeout;  ///< Timeout for read operations.
    PTimeInterval writeTimeout; ///< Timeout for write operations.
 
    P_INT_PTR os_handle; ///< The operating system file handle return by standard open() function.

    struct Status
    {
      PINDEX m_lastCount;        ///< Last read/write count
      Errors m_lastErrorCode;    ///< The platform independant error code. 
      int    m_lastErrorNumber;  ///< The operating system error number (eg as returned by errno).

      Status()
        : m_lastCount(0)
        , m_lastErrorCode(NoError)
        , m_lastErrorNumber(0)
      { }
    };
    PThreadLocalStorage<Status> m_status[NumErrorGroups+1];

  private:
    // New functions for class
    void Construct();
      // Complete platform dependent construction.

    // Member variables
    bool m_abortCommandString;
      // Flag to abort the transmission of a command in SendCommandString().


// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/channel.h"
#else
#include "unix/ptlib/channel.h"
#endif

};


/** A channel that does nothing.
  */
class PNullChannel : public PChannel
{
    PCLASSINFO(PNullChannel, PChannel)
  public:
    PNullChannel();

    virtual PString GetName() const { return "null"; }
    virtual PBoolean Read(void *, PINDEX);
    virtual PBoolean Write(const void *, PINDEX);
};


class P_fd_set
{
public:
  P_fd_set();
  P_fd_set(intptr_t fd);
  ~P_fd_set()
  {
    free(set);
  }

  P_fd_set & operator=(intptr_t fd);
  P_fd_set & operator+=(intptr_t fd);
  P_fd_set & operator-=(intptr_t fd);

  void Zero();

  PBoolean IsPresent(intptr_t fd) const;

  operator fd_set*() const
  {
    return set;
  }

protected:
  void Construct();

  intptr_t max_fd;
  fd_set * set;

private:
  P_fd_set(const P_fd_set &) { }
  void operator=(const P_fd_set &) { }
};


#endif // PTLIB_CHANNEL_H


// End Of File ///////////////////////////////////////////////////////////////
