/*
 * $Id: serchan.h,v 1.8 1995/06/17 11:13:18 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
 * Revision 1.8  1995/06/17 11:13:18  robertj
 * Documentation update.
 *
 * Revision 1.7  1995/03/14 12:42:33  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.6  1995/01/14  06:19:37  robertj
 * Documentation
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/07/17  10:46:06  robertj
 * Moved data to PChannel class.
 *
 * Revision 1.2  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.1  1994/04/20  12:17:44  robertj
 * Initial revision
 *
 */


#define _PSERIALCHANNEL

#ifdef __GNUC__
#pragma interface
#endif


class PConfig;


///////////////////////////////////////////////////////////////////////////////
// Serial Channel

PDECLARE_CONTAINER(PSerialChannel, PChannel)
/* This class defines an I/O channel that communicates via a serial port. This
   is usually an RS-232 port.
 */

  public:
    PSerialChannel();
      // Create a new serial channel object, but do not open it.

    enum Parity {
      DefaultParity,    // Use the default parity, ie do not change it.
      NoParity,         // Set the port for no parity bit.
      EvenParity,       // Set the port to generate parity and make it even.
      OddParity,        // Set the port to generate parity and make it odd.
      MarkParity,       // Set the port parity bit to mark only.
      SpaceParity       // Set the port parity bit to space only.
    };
    // Configuration of serial port parity options.

    enum FlowControl {
      DefaultFlowControl, // Use the default flow control, ie do not change it.
      NoFlowControl,     // Set the port for no flow control.
      XonXoff,          // Set the port for software or XON/XOFF flow control.
      RtsCts           // Set the port for hardware or RTS/CTS flow control.
    };
    // Configuration of serial port flow control options.

    PSerialChannel(
      const PString & port,
      /* The name of the serial port to connect to. This is a platform
         dependent string and woiuld rarely be a literal. The static function
         <A>GetPortNames()</A> can be used to find the platforms serial ports.
       */
      DWORD speed = 0,
      /* Serial port speed or baud rate. The actual values possible here are
         platform dependent, but the standard value of 300, 1200, 2400, 4800,
         9600, 19200, 38400 always be legal.
       */
      BYTE data = 0,
      /* Number of data bits for serial port. The actual values possible here
         are platform dependent, but 7 and 8 should always be legal.
       */
      Parity parity = DefaultParity,
      /* Parity for serial port. The actual values possible here are platform
         dependent, but <CODE>NoParity</CODE>, <CODE>OddParity</CODE> and
         <CODE>EvenParity</CODE> should always be legal.
       */
      BYTE stop = 0,
      /* Number of stop bits for serial port. The actual values possible here
         are platform dependent, but 1 and 2 should always be legal.
       */
      FlowControl inputFlow = DefaultFlowControl,
      // Flow control for data from the remote system into this conputer.
      FlowControl outputFlow = DefaultFlowControl
      // Flow control for data from this conputer out to remote system.
    );
    PSerialChannel(
      PConfig & cfg  // Configuration file to read serial port attributes from.
    );
    /* Create a serial chennal and open it on the specified port and with the
       specified attributes. The second form obtains the attributes from
       standard variables in the configuration file. Note that it assumed that
       the correct configuration file section is already set.
     */


    // New functions for class
    virtual BOOL Open(
      const PString & port,
      /* The name of the serial port to connect to. This is a platform
         dependent string and woiuld rarely be a literal. The static function
         <A>GetPortNames()</A> can be used to find the platforms serial ports.
       */
      DWORD speed = 0,
      /* Serial port speed or baud rate. The actual values possible here are
         platform dependent, but the standard value of 300, 1200, 2400, 4800,
         9600, 19200, 38400 always be legal.
       */
      BYTE data = 0,
      /* Number of data bits for serial port. The actual values possible here
         are platform dependent, but 7 and 8 should always be legal.
       */
      Parity parity = DefaultParity,
      /* Parity for serial port. The actual values possible here are platform
         dependent, but <CODE>NoParity</CODE>, <CODE>OddParity</CODE> and
         <CODE>EvenParity</CODE> should always be legal.
       */
      BYTE stop = 0,
      /* Number of stop bits for serial port. The actual values possible here
         are platform dependent, but 1 and 2 should always be legal.
       */
      FlowControl inputFlow = DefaultFlowControl,
      // Flow control for data from the remote system into this conputer.
      FlowControl outputFlow = DefaultFlowControl
      // Flow control for data from this conputer out to remote system.
    );
    virtual BOOL Open(
      PConfig & cfg  // Configuration file to read serial port attributes from.
    );
    /* Open the serial channel on the specified port and with the specified
       attributes. The second form obtains the attributes from standard
       variables in the configuration file. Note that it assumed that the
       correct configuration file section is already set.
     */


    BOOL SetSpeed(
      DWORD speed   // New speed for serial channel.
    );
    /* Set the speed (baud rate) of the serial channel.

       <H2>Returns:</H2>
       TRUE if the change was successfully made.
     */

    DWORD GetSpeed() const;
    /* Get the speed (baud rate) of the serial channel.

       <H2>Returns:</H2>
       current setting.
     */

    BOOL SetDataBits(
      BYTE data   // New number of data bits for serial channel.
    );
    /* Set the data bits (5, 6, 7 or 8) of the serial port.

       <H2>Returns:</H2>
       TRUE if the change was successfully made.
     */

    BYTE GetDataBits() const;
    /* Get the data bits (5, 6, 7 or 8) of the serial port.

       <H2>Returns:</H2>
       current setting.
     */

    BOOL SetParity(
      Parity parity   // New parity option for serial channel.
    );
    /* Set the parity of the serial port.

       <H2>Returns:</H2>
       TRUE if the change was successfully made.
     */

    Parity GetParity() const;
    /* Get the parity of the serial port.

       <H2>Returns:</H2>
       current setting.
     */

    BOOL SetStopBits(
      BYTE stop   // New number of stop bits for serial channel.
    );
    /* Set the stop bits (1 or 2) of the serial port.

       <H2>Returns:</H2>
       TRUE if the change was successfully made.
     */

    BYTE GetStopBits() const;
    /* Get the stop bits (1 or 2) of the serial port.

       <H2>Returns:</H2>
       current setting.
     */

    BOOL SetInputFlowControl(
      FlowControl flowControl   // New flow control for serial channel input.
    );
    /* Set the flow control (handshaking) protocol of the input to the serial
       port.

       <H2>Returns:</H2>
       TRUE if the change was successfully made.
     */

    FlowControl GetInputFlowControl() const;
    /* Get the flow control (handshaking) protocol of the input to the serial
       port.

       <H2>Returns:</H2>
       current setting.
     */

    BOOL SetOutputFlowControl(
      FlowControl flowControl   // New flow control for serial channel output.
    );
    /* Set the flow control (handshaking) protocol of the output to the serial
       port.

       <H2>Returns:</H2>
       TRUE if the change was successfully made.
     */

    FlowControl GetOutputFlowControl() const;
    /* Get the flow control (handshaking) protocol of the output from the
       serial port.

       <H2>Returns:</H2>
       current setting.
     */

    virtual void SaveSettings(
      PConfig & cfg   // Configuration file to save setting into.
    );
    /* Save the current port settings into the configuration file. Note that
       it assumed that the correct configuration file section is already set.
     */

    void SetDTR(
      BOOL state = TRUE   // New state of the DTR signal.
    );
    // Set the Data Terminal Ready signal of the serial port.

    void ClearDTR();
    /* Clear the Data Terminal Ready signal of the serial port. This is
       equivalent to <CODE>SetDTR(FALSE)</CODE>.
     */

    void SetRTS(
      BOOL state = TRUE   // New state of the RTS signal.
    );
    // Set the Request To Send signal of the serial port.

    void ClearRTS();
    /* Clear the Request To Send signal of the serial port. This is equivalent
       to <CODE>SetRTS(FALSE)</CODE>.
     */

    void SetBreak(
      BOOL state = TRUE   // New state of the serial port break condition.
    );
    // Set the break condition of the serial port.

    void ClearBreak();
    /* Clear the break condition of the serial port. This is equivalent to
       <CODE>SetBreak(FALSE)</CODE>.
     */

    BOOL GetCTS();
    /* Get the Clear To Send signal of the serial port.
    
       <H2>Returns:</H2>
       TRUE if the CTS signal is asserted.
     */

    BOOL GetDSR();
    /* Get the Data Set Ready signal of the serial port.
    
       <H2>Returns:</H2>
       TRUE if the DSR signal is asserted.
     */

    BOOL GetDCD();
    /* Get the Data Carrier Detect signal of the serial port.
    
       <H2>Returns:</H2>
       TRUE if the DCD signal is asserted.
     */

    BOOL GetRing();
    /* Get the Ring Indicator signal of the serial port.
    
       <H2>Returns:</H2>
       TRUE if the RI signal is asserted.
     */

    static PStringList GetPortNames();
    /* Get a list of the available serial ports. This returns a set of
       platform dependent strings which describe the serial ports of the
       computer. For example under unix it may be "ttyS0", under MS-DOS or
       NT it would be "COM1" and for the Macintosh it could be "Modem".

       <H2>Returns:</H2>
       list of strings for possible serial ports.
     */


  private:
    void Construct();
    // Platform dependent construct of the serial channel.


// Class declaration continued in platform specific header file ///////////////
