/*
 * $Id: channel.h,v 1.7 1994/08/23 11:32:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.7  1994/08/23 11:32:52  robertj
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

PCLASS PChannelStreamBuffer : public PObject, public streambuf {
  PCLASSINFO(PChannelStreamBuffer, PObject)

  public:
    PChannelStreamBuffer(const PChannelStreamBuffer & sbuf);
    PChannelStreamBuffer & operator=(const PChannelStreamBuffer & sbuf);

  protected:
    PChannelStreamBuffer(PChannel * chan);
      // Construct the streambuf for standard streams on the channel

    virtual int overflow(int=EOF);
      // Function to flush the output buffer to the stream.

    virtual int underflow();
      // Function to refill the input buffer from the stream.

    virtual int sync();
      // Function to flush input and output buffer of the stream.

    virtual streampos seekoff(streamoff, ios::seek_dir, int);
      // Function to seek a location in the file


  private:
    // Member variables
    PChannel * channel;
    char buffer[1024];


  friend class PChannel;
};


PCLASS PChannel : public PContainer, public iostream {
  PCONTAINERINFO(PChannel, PContainer)
  // Abstract class defining I/O channel semantics. An I/O channel can be a
  // serial port, pipe, network socket or even just a simple file.

  public:
    PChannel();
      // Construct the channel


    // New functions for class
    virtual BOOL IsOpen() const;
      // Return TRUE if the channel is currently open.

    virtual PString GetName() const;
      // Return the name of the channel.


    void SetReadTimeout(PTimeInterval time);
      // Set the timeout for read operations. This may be zero for immediate
      // return of data through to PMaxMilliseconds which will wait forever
      // for the read request to be filled. Note that this function may not
      // be available for all channels.

    PTimeInterval GetReadTimeout() const;
      // Get the current read timeout.

    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function may block until the
      // requested number of characters were read or the read timeout was
      // reached. The return value indicates that at least one character was
      // read from the channel.

    PINDEX GetLastReadCount() const;
      // Return the number of bytes read by the last Read() call.

    virtual int ReadChar();
      // Read a single 8 bit byte from the channel. If one was not available
      // within the readtimeout period then the function returns with a -1
      // return value.

    PString ReadString(PINDEX len);
      // Read up to len bytes into a string from the channel. This function may
      // block as for Read().

    virtual BOOL ReadAsync(void * buf, PINDEX len);
      // Begin an asynchronous read from channel. The read timeout is used as
      // in other read operations, in this case calling the OnReadComplete()
      // function. Note that if the channel is not capable of asynchronous
      // read then this will do a sychronous read is in the Read() function
      // with the addition of calling the OnReadComplete() before returning.
      // The return value is TRUE if the read was sucessfully queued.

    virtual void OnReadComplete(void * buf, PINDEX len);
      // User callback function for when a ReadAsync() call has completed or
      // timed out. The original pointer to the buffer passed in ReadAsync()
      // is passed in here and the len parameter as the actual number of
      // characters read.


    void SetWriteTimeout(PTimeInterval time);
      // Set the timeout for write operations. This may be zero for immediate
      // return of data through to PMaxMilliseconds which will wait forever
      // for the write request to be completed. Note that this function may not
      // be available for all channels.

    PTimeInterval GetWriteTimeout() const;
      // Get the current write timeout.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters are written or the write timeout is
      // reached. The return value is TRUE if at least len bytes were written
      // to the channel.

    PINDEX GetLastWriteCount() const;
      // Return the number of bytes written by the last Write() call.

    BOOL WriteChar(int c);
      // Write a single character to the channel. This function will block
      // until the requested number of characters are written or the write
      // timeout is reached. Note that this asserts if the value is not in the
      // range 0..255.

    BOOL WriteString(const PString & str);
      // Write a string to the channel. This function will block until the
      // requested number of characters are written or the write timeout is
      // reached.

    virtual BOOL WriteAsync(void * buf, PINDEX len);
      // Begin an asynchronous write from channel. The write timeout is used as
      // in other write operations, in this case calling the OnWriteComplete()
      // function. Note that if the channel is not capable of asynchronous
      // write then this will do a sychronous write as in the Write() function
      // with the addition of calling the OnWriteComplete() before returning.

    virtual void OnWriteComplete(void * buf, PINDEX len);
      // User callback function for when a WriteAsync() call has completed or
      // timed out. The original pointer to the buffer passed in WriteAsync()
      // is passed in here and the len parameter is the actual number of
      // characters written.


    BOOL SendCommandString(const PString & command);
      // Send a command meta-string. A meta-string is a string of characters
      // that may contain escaped commands. The escape command is the \ as in
      // the C language. The escape commands are:
      //    \a    alert (ascii value 7)
      //    \b    backspace (ascii value 8)
      //    \f    formfeed (ascii value 12)
      //    \n    newline (ascii value 10)
      //    \r    return (ascii value 13)
      //    \t    horizontal tab (ascii value 9)
      //    \v    vertical tab (ascii value 11)
      //    \\    backslash
      //    \ooo  where ooo is octal number (ascii value ooo)
      //    \xhh  where hh is hex number (ascii value 0xhh)
      //    \0    null character (ascii zero)
      //    \dns  delay for n seconds
      //    \dnm  delay for n milliseconds
      //    \s    characters following this, up to a \w command or the end of
      //          string, are to be sent to modem
      //    \wns  characters following this, up to a \s, \d or another \w or
      //          the end of the string are expected back from the modem. If
      //          the string is not received within n seconds, a failed command
      //          is registered. The exception to this is if the command is at
      //          the end of the string or the next character in the string is
      //          the \s, \d or \w in which case all characters are ignored
      //          from the modem until n seconds of no data.
      //    \wnm  as for above but timeout is in milliseconds

    void AbortCommandString();


    virtual BOOL Close();
      // Close the channel.


    enum Errors {
      NoError,
      NotFound,     // Open fail due to device or file not found
      FileExists,   // Open fail due to file already existing
      DiskFull,     // Write fail due to disk full
      AccessDenied, // Operation fail due to insufficient privilege
      DeviceInUse,  // Open fail due to device already open for exclusive use
      BadParameter, // Operation fail due to bad parameters
      NoMemory,     // Operation fail due to insufficient memory
      NotOpen,      // Operation fail due to channel not being open yet
      Timeout,      // Operation failed due to a timeout
      Miscellaneous
    };
    Errors GetErrorCode() const;
      // Return the error result of the last file I/O operation in this object.

    int GetErrorNumber() const;
      // Return the operating system error number of the last file I/O
      // operation in this object.

    PString GetErrorText() const;
      // Return a string indicating the error message that may be displayed to
      // the user. The error for the last I/O operation in this object is used.


  protected:
    BOOL ConvertOSError(int error);
      // Convert an operating system error into platform independent error.
      // This will set the lastError and osError member variables for access
      // by GetErrorCode() and GetErrorNumber(). Returns TRUE if there was
      // no error.


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
    // Overrides from class PContainer
    virtual BOOL SetSize(PINDEX newSize);


    // New functions for class
    void Construct();
      // Complete platform dependent construction.

    // Member variables
    BOOL abortCommandString;
      // Flag to abort the transmission of a command in SendCommandString().


// Class declaration continued in platform specific header file ///////////////
