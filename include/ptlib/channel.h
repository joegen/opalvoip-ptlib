/*
 * $Id: channel.h,v 1.1 1994/04/20 12:17:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.1  1994/04/20 12:17:44  robertj
 * Initial revision
 *
 */


#ifndef _PCHANNEL
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
      // Function to flush the output buffer to the file

    virtual int underflow();
      // Function to refill the input buffer from the file

    virtual int sync();
      // Function to refill the input buffer from the file

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
      
    virtual BOOL Read(void * buf, PINDEX len) = 0;
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual int ReadChar() = 0;
      // Read a single 8 bit byte from the channel. If one was not available
      // then the function returns immediately with a -1 return value.

    virtual BOOL Write(const void * buf, PINDEX len) = 0;
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    virtual BOOL WriteChar(char c) = 0;
      // Write a single character to the channel. This function does not block
      // and will return FALSE if it could not write the character.

    virtual PINDEX GetInputAvailable() = 0;
      // Return the number of characters that may be read from the channel
      // without causing the Read() function to block.

    virtual PINDEX GetOutputAvailable() = 0;
      // Return the number of characters that may be written to the channel
      // without causing the Write() function to block.

    virtual BOOL Close() = 0;
      // Close the channel.


    enum Errors {
      NoError,
      FileNotFound,
      FileExists,
      DiskFull,
      AccessDenied,
      DeviceInUse,
      Miscellaneous
    };
    Errors GetErrorCode() const;
      // Return the error result of the last file I/O operation in this object.
    PString GetErrorText() const;
      // Return a string indicating the error message that may be displayed to
      // the user. The error for the last I/O operation in this object is used.


  protected:
    // New member functions
    virtual BOOL FlushStreams();
      // Flush stream based descendents of PDiskFile


    // member variables
    int os_errno;
      // The operating system error number (eg as returned by errno).
};


#endif
