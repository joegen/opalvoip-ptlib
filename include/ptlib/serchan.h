/*
 * $Id: serchan.h,v 1.2 1994/06/25 11:55:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
 * Revision 1.2  1994/06/25 11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.1  1994/04/20  12:17:44  robertj
 * Initial revision
 *
 */


#define _PSERIALCHANNEL

#ifndef _PCHANNEL
#include <channel.h>
#endif

class PConfig;


///////////////////////////////////////////////////////////////////////////////
// Serial Channel

PDECLARE_CONTAINER(PSerialChannel, PChannel)
  public:
    PSerialChannel();
      // Create a new serial channel, but do not open it.

    enum Parity {
      DefaultParity, NoParity, EvenParity, OddParity, MarkParity, SpaceParity
    };
    enum FlowControl {
      DefaultFlowControl, NoFlowControl, XonXoff, RtsCts
    };
    PSerialChannel(const PString & port,
                   DWORD speed = 0,
                   BYTE data = 0,
                   Parity parity = DefaultParity,
                   BYTE stop = 0,
                   FlowControl inputFlow = DefaultFlowControl,
                   FlowControl outputFlow = DefaultFlowControl);
      // Open the serial channel as specified.

    PSerialChannel(PConfig & cfg);
      // Open the serial channel obtaining the parameters from standard
      // variables in the configuration file. Note that it assumed that the
      // correct configuration file section is already set.


    // Overrides from class PChannel
    virtual BOOL IsOpen() const;
      // Return TRUE if the channel is currently open.

    virtual PString GetName() const;
      // Return the name of the channel.

      
    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    virtual BOOL Close();
      // Close the channel.


    // New functions for class
    virtual BOOL Open(const PString & port,
                      DWORD speed = 0,
                      BYTE data = 0,
                      Parity parity = DefaultParity,
                      BYTE stop = 0,
                      FlowControl inputFlow = DefaultFlowControl,
                      FlowControl outputFlow = DefaultFlowControl);
      // Open the serial channel on the specified port.

    virtual BOOL Open(PConfig & cfg);
      // Open the serial channel obtaining the parameters from standard
      // variables in the configuration file. Note that it assumed that the
      // correct configuration file section is already set.


    BOOL SetSpeed(DWORD speed);
      // Set the speed (baud rate) of the serial channel.

    DWORD GetSpeed() const;
      // Get the speed (baud rate) of the serial channel.

    BOOL SetDataBits(BYTE data);
      // Set the data bits (5, 6, 7 or 8) of the serial port.

    BYTE GetDataBits() const;
      // Get the data bits (5, 6, 7 or 8) of the serial port.

    BOOL SetParity(Parity parity);
      // Set the parity of the serial port.

    Parity GetParity() const;
      // Get the parity of the serial port.

    BOOL SetStopBits(BYTE stop);
      // Set the stop bits (1 or 2) of the serial port.

    BYTE GetStopBits() const;
      // Get the stop bits (1 or 2) of the serial port.

    BOOL SetInputFlowControl(FlowControl flowControl);
      // Set the flow control (handshaking) protocol of the serial port.

    FlowControl GetInputFlowControl() const;
      // Get the flow control (handshaking) protocol of the serial port.

    BOOL SetOutputFlowControl(FlowControl flowControl);
      // Set the flow control (handshaking) protocol of the serial port.

    FlowControl GetOutputFlowControl() const;
      // Get the flow control (handshaking) protocol of the serial port.

    virtual void SaveSettings(PConfig & cfg);
      // Save the current port settings into the configuration file


    void SetDTR(BOOL state = TRUE);
      // Set the Data Terminal Ready signal of the serial port.

    void ClearDTR();
      // Clear the Data Terminal Ready signal of the serial port.

    void SetRTS(BOOL state = TRUE);
      // Set the Request To Send signal of the serial port.

    void ClearRTS();
      // Clear the Request To Send signal of the serial port.

    void SetBreak(BOOL state = TRUE);
      // Set the break condition of the serial port.

    void ClearBreak();
      // Clear the break condition of the serial port.

    BOOL GetCTS();
      // Get the Clear To Send signal of the serial port.

    BOOL GetDSR();
      // Get the Data Set Ready signal of the serial port.

    BOOL GetDCD();
      // Get the Data Carrier Detect signal of the serial port.

    BOOL GetRing();
      // Get the Ring Indicator signal of the serial port.

    static PStringList GetPortNames();
      // Return a list of the available serial ports.


  private:
    void Construct();
      // Platform dependent construct of the serial channel.


// Class declaration continued in platform specific header file ///////////////
