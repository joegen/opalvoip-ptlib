/*
 * serial.cxx
 *
 * copyright 2005 Derek J Smithies
 *
 * Simple program to illustrate usage of the serial port.
 *
 * Get two computers. Connect com1 port of both computers by cross over serial cable.
 * run this program on both computers (without arguments).
 * type text in one, hit return.  See this text in the second computer.
 *
 *
 * $Log: serial.cxx,v $
 * Revision 1.1  2005/03/10 09:21:46  dereksmithies
 * Initial release of a program to illustrate the operation of the serial port.
 *
 *
 *
 *
 */

#include <ptlib.h>
#include <ptlib/serchan.h>
#include <ptlib/sockets.h>

class Serial : public PProcess
{
    PCLASSINFO(Serial, PProcess);
public:    
    Serial();

    void Main();
    
    BOOL Initialise(PConfigArgs & args);

    void HandleConsoleInput();

    void HandleSerialInput();

protected:
    PSerialChannel serial;

    PINDEX dataBits;

    PINDEX stopBits;

    BOOL hardwareFlow;

    PINDEX hardwarePort;

    PINDEX baud;
    
    PString parity;
    BOOL endNow;
};


class UserInterfaceThread : public PThread
{
    PCLASSINFO(UserInterfaceThread, PThread);
  public:
    UserInterfaceThread(Serial & _srl)
      : PThread(1000, NoAutoDeleteThread), srl(_srl) { Resume(); }

    void Main()
      { srl.HandleConsoleInput(); }

  protected:
    Serial & srl; 
};

class SerialInterfaceThread : public PThread
{
    PCLASSINFO(SerialInterfaceThread, PThread);
public:
    SerialInterfaceThread(Serial & _srl)
      : PThread(1000, NoAutoDeleteThread), srl(_srl) { Resume(); }

    void Main()
      { srl.HandleSerialInput(); }

  protected:
    Serial & srl; 
};
    

PCREATE_PROCESS(Serial)


Serial::Serial()
  : PProcess("PwLib Example Factory", "serial", 1, 0, ReleaseCode, 0)
{
    endNow = FALSE;
}


void Serial::Main()
{

 PConfigArgs args(GetArguments());

// Example command line is :
// serial --hardwareport 0 --baud 4800 --parity odd --stopbits 1 --databits 8 --hardwareflow off

  args.Parse(
#if PTRACING
             "t-trace."              "-no-trace."
             "o-output:"             "-no-output."
#endif
#ifdef PMEMORY_CHECK
             "-setallocationbreakpoint:"
#endif
             "-baud:"
             "-databits:"
             "-parity:"
             "-stopbits:"
             "-hardwareflow:"
             "-hardwareport:"
             "v-version"
          , FALSE);

#if PMEMORY_CHECK
  if (args.HasOption("setallocationbreakpoint"))
    PMemoryHeap::SetAllocationBreakpoint(args.GetOptionString("setallocationbreakpoint").AsInteger());
#endif

  PStringStream progName;
  progName << "Product Name: " << GetName() << endl
           << "Manufacturer: " << GetManufacturer() << endl
           << "Version     : " << GetVersion(TRUE) << endl
           << "System      : " << GetOSName() << '-'
           << GetOSHardware() << ' '
           << GetOSVersion();
  cout << progName << endl;

  if (args.HasOption('v'))
    return;


#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif



  if (!Initialise(args)) {
      cout << "Failed to initialise the program with args of " << args << endl;
      PThread::Sleep(100);
      return;
  }

  UserInterfaceThread *ui = new UserInterfaceThread(*this);
  SerialInterfaceThread *si = new SerialInterfaceThread(*this);

  ui->WaitForTermination();

  serial.Close();

  si->WaitForTermination();

  delete ui;
  delete si;

  cout << endl << "End of program" << endl << endl;
}

void Serial::HandleSerialInput()
{
#define MAXM 1000
    char buffer[MAXM];
    PString str;
    PINDEX  i;

    while(serial.IsOpen()) {
	memset(buffer, 0, MAXM);
        serial.Read(buffer, MAXM);

	if (endNow) {
	    cout << "End of thread to handle serial input" << endl;
	    return;
	}

        PINDEX len = serial.GetLastReadCount();
        if (len != 0) {
            str = buffer;
            for (i = str.GetLength(); i > 0; i--)
                if (str[i - 1] < 0x20)
                    str.Delete(i - 1, 1);
        }

        PINDEX err = serial.GetErrorCode();
        if ((err != PChannel::NoError) && (err != PChannel::Timeout)) {
            PTRACE(1, "get data from serial port, failed, error is " << serial.GetErrorText());
            cout << "get data from serial port, failed, error is " << serial.GetErrorText() << endl;
        }

        if (str.GetLength() > 0) {
            PTRACE(1, "Read the message \"" << str << "\"");
	    cout << "have read the message \"" << str << "\" from the serial port" << endl;
        }
    }
}

BOOL Serial::Initialise(PConfigArgs & args)
{
    if (!args.HasOption("baud")) {
	baud = 4800;
	cout << "Baud not specifed.          Using 4800" << endl;
    } else
	baud = args.GetOptionString("baud").AsInteger();
    
    if (!args.HasOption("databits")) {
	cout << "databits not specified.     Using 1" << endl;
	dataBits = 1;
    } else
	dataBits = args.GetOptionString("databits").AsInteger();
    
    if (!args.HasOption("parity")) {
	cout << "parity not specified.       Using \"odd\"" << endl;
	parity = "odd";
    } else
	parity = args.GetOptionString("parity");
    
    if (!args.HasOption("stopbits")) {
	cout << "stopbits not specified.     Using 1" << endl;
	stopBits = 1;
    }
    stopBits = args.GetOptionString("stopbits").AsInteger();
    
    PString flow;
    if (!args.HasOption("hardwareflow")) {
	cout << "hardwareflow not specified. Hardware flow is set to off" << endl;
	flow = "off";
    } else
	flow = args.GetOptionString("hardwareflow");
    if ((flow *= "on") || (flow *= "off"))
	hardwareFlow = (flow *= "on");
    else {
	cout << "Valid args to hardware flow are on or off" << endl;
	return FALSE;
    }
    
    if (!args.HasOption("hardwareport")) {
	cout << "Hardware port is not set.   Using 0 - the first hardware port" << endl;
	hardwarePort = 0;
    } else
	hardwarePort = args.GetOptionString("hardwareport").AsInteger();
    
    
    PStringList names = serial.GetPortNames();
    PStringStream allNames;
    for(PINDEX i = 0; i < names.GetSize(); i++)
	allNames << names[i] << " ";
    PTRACE(1, "available serial ports are " << allNames);
    
    PString portName;
    if (hardwarePort >= names.GetSize()) {
	cout << "hardware port is too large, list is only " << names.GetSize() << " long" << endl;
	return FALSE;
    }
    portName = names[hardwarePort];
    
    PSerialChannel::Parity pValue = PSerialChannel::DefaultParity;
    if (parity *= "none")
	pValue = PSerialChannel::NoParity;
    if (parity *= "even")
	pValue = PSerialChannel::EvenParity;
    if (parity *= "odd")
	pValue = PSerialChannel::OddParity;
    if (pValue == PSerialChannel::DefaultParity) {
	cout << "Parity value of " << parity << " could not be interpreted" << endl;
	return FALSE;
    }
    
    PSerialChannel::FlowControl flowControl;
    if (hardwareFlow)
	flowControl = PSerialChannel::RtsCts;
    else
	flowControl = PSerialChannel::NoFlowControl;
    
    if (!serial.Open(portName, baud, dataBits, pValue, stopBits, flowControl, flowControl)) {
	cout << "Failed to open serial port " << endl;
	cout << "Error code is " << serial.GetErrorText() << endl;
	cout << "Failed in attempt to open port  /dev/" << portName << endl;
	return FALSE;
    }
    
    
    return TRUE;
}

void Serial::HandleConsoleInput()
{

  PTRACE(2, "Serial\tUser interface thread started.");

  PStringStream help;
  help << "Select:\n";
  help << "  ?   : display this help\n";
  help << "  H   : display this help\n";
  help << "  X   : Exit program\n";
  help << "  Q   : Exit program\n";
  help << "      : anything else to send a message\n";


  for (;;) {

    // display the prompt
    PError << "Command ? " << flush;
    char oneLine[200];
    fgets(oneLine, 200, stdin);
    
    PString str(oneLine);
    for (PINDEX i = str.GetLength(); i > 0; i--)
        if (str[i - 1] < 0x20)
            str.Delete(i - 1, 1);
    
    if (str.GetLength() < 1)
	continue;

    char ch = str.ToLower()[0];
    switch(ch) {
	case '?' :
	case 'h' : PError << help << endl;
	    break;
	    
	case 'x' :
	case 'q' :
	    PError << "\nEnd of thread to read from keyboard " << endl << endl;
	    endNow = TRUE;
	    return;
	    break;

	default:
	    PTRACE(1, "Serial\t Write the message\"" << str << "\" to the serial port");
	    serial.Write(str.GetPointer(), str.GetLength());
    }
  }
}
 
// End of serial.cxx
