/*
 * $Id: channel.h,v 1.3 1994/07/02 03:03:49 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.3  1994/07/02 03:03:49  robertj
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


///////////////////////////////////////////////////////////////////////////////
// I/O Channels

class PChannel;

PCLASS PChannelStreamBuffer : public streambuf {
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
    virtual BOOL IsOpen() const = 0;
      // Return TRUE if the channel is currently open.

    virtual PString GetName() const = 0;
      // Return the name of the channel.


    void SetReadTimeout(PTimeInterval time);
      // Set the timeout for read operations. This may be zero for immediate
      // return of data through to PMaxMilliseconds which will wait forever
      // for the read request to be filled. Note that this function may not
      // be available for all channels.

    PTimeInterval GetReadTimeout() const;
      // Get the current read timeout.

    virtual BOOL Read(void * buf, PINDEX len) = 0;
      // Low level read from the channel. This function may block until the
      // requested number of characters were read or the read timeout was
      // reached. The return value indicates that at least one character was
      // read from the channel.

    PINDEX GetLastReadCount() const;
      // Return the number of bytes read by the last Read() call.

    PString ReadString(PINDEX len);
      // Read up to len bytes into a string from the channel. This function may
      // block as for Read().

    virtual int ReadChar();
      // Read a single 8 bit byte from the channel. If one was not available
      // within the readtimeout period then the function returns with a -1
      // return value.

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

    virtual BOOL Write(const void * buf, PINDEX len) = 0;
      // Low level write to the channel. This function will block until the
      // requested number of characters are written or the write timeout is
      // reached. The return value is TRUE if at least len bytes were written
      // to the channel.

    PINDEX GetLastWriteCount() const;
      // Return the number of bytes written by the last Write() call.

    BOOL WriteString(const PString & str);
      // Write a string to the channel. This function will block until the
      // requested number of characters are written or the write timeout is
      // reached.

    BOOL WriteChar(int c);
      // Write a single character to the channel. This function will block
      // until the requested number of characters are written or the write
      // timeout is reached. Note that this asserts if the value is not in the
      // range 0..255.

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


    virtual BOOL Close() = 0;
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
    // member variables
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


// Class declaration continued in platform specific header file ///////////////
