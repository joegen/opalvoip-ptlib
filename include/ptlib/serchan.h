/*
 * $Id: serchan.h,v 1.1 1994/04/20 12:17:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
 * Revision 1.1  1994/04/20 12:17:44  robertj
 * Initial revision
 *
 */


#define _PSERIALCHANNEL

#ifndef _PCHANNEL
#include <channel.h>
#endif


///////////////////////////////////////////////////////////////////////////////
// Serial Channel

PDECLARE_CLASS(PSerialChannel, PChannel)
  public:
    PSerialChannel();
      // Create a new serial channel.

    enum Parity { DefaultParity, NoParity, EvenParity, OddParity };
    enum FlowControl { DefaultFlowControl, NoFlowControl, XonXoff, RtsCts };
    PSerialChannel(const PString & port,
                   DWORD speed = 0,
                   Parity parity = DefaultParity,
                   FlowControl = DefaultFlowControl);
      // Open the serial channel as specified.


    // Overrides from class PChannel
    BOOL IsOpen() const;
      // Return TRUE if the channel is currently open.

    BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    int ReadChar();
      // Read a single 8 bit byte from the channel. If one was not available
      // then the function returns immediately with a -1 return value.

    BOOL WriteChar(char c);
      // Write a single character to the channel. This function does not block
      // and will return FALSE if it could not write the character.

    PINDEX GetInputAvailable();
      // Return the number of characters that may be read from the channel
      // without causing the Read() function to block.

    PINDEX GetOutputAvailable();
      // Return the number of characters that may be written to the channel
      // without causing the Write() function to block.

    BOOL Close();
      // Close the open serial port.


    // New functions for class
    BOOL Open(const PString & port,
              DWORD speed = 0,
              Parity parity = DefaultParity,
              FlowControl = DefaultFlowControl);
      // Open the serial channel on the specified port.

    BOOL SetSpeed(DWORD speed);
      // Set the speed (baud rate) of the serial channel.

    DWORD GetSpeed() const;
      // Get the speed (baud rate) of the serial channel.

    BOOL SetParity(Parity parity);
      // Set the parity of the serial port.

    Parity GetParity() const;
      // Get the parity of the serial port.

    BOOL SetFlowControl(FlowControl flowControl);
      // Set the flow control (handshaking) protocol of the serial port.

    FlowControl GetFlowControl() const;
      // Get the flow control (handshaking) protocol of the serial port.


// Class declaration continued in platform specific header file ///////////////
