/*
 * $Id: pipechan.h,v 1.3 1994/08/23 11:32:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.h,v $
 * Revision 1.3  1994/08/23 11:32:52  robertj
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
  // A channel that uses a operating system pipe between two processes.

  public:
    PPipeChannel();
      // Create a new pipe channel.


    // Overrides from class PChannel
    BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    int Read();
      // Read a single 8 bit byte from the channel. If one was not available
      // then the function returns immediately with a -1 return value.

    BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    BOOL Write(char c);
      // Write a single character to the channel. This function does not block
      // and will return FALSE if it could not write the character.

    PINDEX InputAvailable();
      // Return the number of characters that may be read from the channel
      // without causing the Read() function to block.

    PINDEX OutputAvailable();
      // Return the number of characters that may be written to the channel
      // without causing the Write() function to block.


// Class declaration continued in platform specific header file ///////////////
