/*
 * $Id: modem.cxx,v 1.2 1996/04/15 10:57:59 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Inline Function Definitions
 *
 * Copyright 1993 Equivalence
 *
 * $Log: modem.cxx,v $
 * Revision 1.2  1996/04/15 10:57:59  robertj
 * Moved some functions from INL to serial.cxx so unix linker can make smaller executables.
 *
 * Revision 1.1  1996/04/14 02:54:14  robertj
 * Initial revision
 *
 */

#include <ptlib.h>

#include <ctype.h>


///////////////////////////////////////////////////////////////////////////////
// PSerialChannel

#ifdef _PSERIALCHANNEL

PSerialChannel::PSerialChannel()
{
  Construct();
}


PSerialChannel::PSerialChannel(const PString & port, DWORD speed, BYTE data,
       Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  Construct();
  Open(port, speed, data, parity, stop, inputFlow, outputFlow);
}


PSerialChannel::PSerialChannel(PConfig & cfg)
{
  Construct();
  Open(cfg);
}


PSerialChannel::~PSerialChannel()
{
  Close();
}


static const char PortName[] = "PortName";
static const char PortSpeed[] = "PortSpeed";
static const char PortDataBits[] = "PortDataBits";
static const char PortParity[] = "PortParity";
static const char PortStopBits[] = "PortStopBits";
static const char PortInputFlow[] = "PortInputFlow";
static const char PortOutputFlow[] = "PortOutputFlow";


BOOL PSerialChannel::Open(PConfig & cfg)
{
  PStringList ports = GetPortNames();
  return Open(cfg.GetString(PortName, ports[0]),
              cfg.GetInteger(PortSpeed, 9600),
              (BYTE)cfg.GetInteger(PortDataBits, 8),
              (PSerialChannel::Parity)cfg.GetInteger(PortParity, 1),
              (BYTE)cfg.GetInteger(PortStopBits, 1),
              (PSerialChannel::FlowControl)cfg.GetInteger(PortInputFlow, 1),
              (PSerialChannel::FlowControl)cfg.GetInteger(PortOutputFlow, 1));
}


void PSerialChannel::SaveSettings(PConfig & cfg)
{
  cfg.SetString(PortName, GetName());
  cfg.SetInteger(PortSpeed, GetSpeed());
  cfg.SetInteger(PortDataBits, GetDataBits());
  cfg.SetInteger(PortParity, GetParity());
  cfg.SetInteger(PortStopBits, GetStopBits());
  cfg.SetInteger(PortInputFlow, GetInputFlowControl());
  cfg.SetInteger(PortOutputFlow, GetOutputFlowControl());
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PModem

#ifdef _PMODEM

PModem::PModem()
{
  status = Unopened;
}


PModem::PModem(const PString & port, DWORD speed, BYTE data,
      Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
  : PSerialChannel(port, speed, data, parity, stop, inputFlow, outputFlow)
{
  status = IsOpen() ? Uninitialised : Unopened;
}


PModem::PModem(PConfig & cfg)
{
  status = Open(cfg) ? Uninitialised : Unopened;
}


void PModem::SetInitString(const PString & str)
{
  initCmd = str;
}

PString PModem::GetInitString() const
{
  return initCmd;
}

void PModem::SetDeinitString(const PString & str)
{
  deinitCmd = str;
}

PString PModem::GetDeinitString() const
{
  return deinitCmd;
}

void PModem::SetPreDialString(const PString & str)
{
  preDialCmd = str;
}

PString PModem::GetPreDialString() const
{
  return preDialCmd;
}

void PModem::SetPostDialString(const PString & str)
{
  postDialCmd = str;
}

PString PModem::GetPostDialString() const
{
  return postDialCmd;
}

void PModem::SetBusyString(const PString & str)
{
  busyReply = str;
}

PString PModem::GetBusyString() const
{
  return busyReply;
}

void PModem::SetNoCarrierString(const PString & str)
{
  noCarrierReply = str;
}

PString PModem::GetNoCarrierString() const
{ 
  return noCarrierReply;
}

void PModem::SetConnectString(const PString & str)
{
  connectReply = str;
}

PString PModem::GetConnectString() const
{
  return connectReply;
}

void PModem::SetHangUpString(const PString & str)
{
  hangUpCmd = str;
}

PString PModem::GetHangUpString() const
{
  return hangUpCmd;
}

PModem::Status PModem::GetStatus() const
{
  return status;
}


BOOL PModem::Close()
{
  status = Unopened;
  return PSerialChannel::Close();
}


BOOL PModem::Open(const PString & port, DWORD speed, BYTE data,
      Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  if (!PSerialChannel::Open(port,
                            speed, data, parity, stop, inputFlow, outputFlow))
    return FALSE;

  status = Uninitialised;
  return TRUE;
}


static const char ModemInit[] = "ModemInit";
static const char ModemDeinit[] = "ModemDeinit";
static const char ModemPreDial[] = "ModemPreDial";
static const char ModemPostDial[] = "ModemPostDial";
static const char ModemBusy[] = "ModemBusy";
static const char ModemNoCarrier[] = "ModemNoCarrier";
static const char ModemConnect[] = "ModemConnect";
static const char ModemHangUp[] = "ModemHangUp";

BOOL PModem::Open(PConfig & cfg)
{
  initCmd = cfg.GetString(ModemInit, "ATZ\\r\\w2sOK\\w100m");
  deinitCmd = cfg.GetString(ModemDeinit, "\\d2s+++\\d2sATH0\\r");
  preDialCmd = cfg.GetString(ModemPreDial, "ATDT");
  postDialCmd = cfg.GetString(ModemPostDial, "\\r");
  busyReply = cfg.GetString(ModemBusy, "BUSY");
  noCarrierReply = cfg.GetString(ModemNoCarrier, "NO CARRIER");
  connectReply = cfg.GetString(ModemConnect, "CONNECT");
  hangUpCmd = cfg.GetString(ModemHangUp, "\\d2s+++\\d2sATH0\\r");

  if (!PSerialChannel::Open(cfg))
    return FALSE;

  status = Uninitialised;
  return TRUE;
}


void PModem::SaveSettings(PConfig & cfg)
{
  PSerialChannel::SaveSettings(cfg);
  cfg.SetString(ModemInit, initCmd);
  cfg.SetString(ModemDeinit, deinitCmd);
  cfg.SetString(ModemPreDial, preDialCmd);
  cfg.SetString(ModemPostDial, postDialCmd);
  cfg.SetString(ModemBusy, busyReply);
  cfg.SetString(ModemNoCarrier, noCarrierReply);
  cfg.SetString(ModemConnect, connectReply);
  cfg.SetString(ModemHangUp, hangUpCmd);
}


BOOL PModem::CanInitialise() const
{
  switch (status) {
    case Unopened :
    case Initialising :
    case Dialling :
    case AwaitingResponse :
    case HangingUp :
    case Deinitialising :
    case SendingUserCommand :
      return FALSE;

    default :
      return TRUE;
  }
}


BOOL PModem::Initialise()
{
  if (CanInitialise()) {
    status = Initialising;
    if (SendCommandString(initCmd)) {
      status = Initialised;
      return TRUE;
    }
    status = InitialiseFailed;
  }
  return FALSE;
}


BOOL PModem::CanDeinitialise() const
{
  switch (status) {
    case Unopened :
    case Initialising :
    case Dialling :
    case AwaitingResponse :
    case Connected :
    case HangingUp :
    case Deinitialising :
    case SendingUserCommand :
      return FALSE;

    default :
      return TRUE;
  }
}


BOOL PModem::Deinitialise()
{
  if (CanDeinitialise()) {
    status = Deinitialising;
    if (SendCommandString(deinitCmd)) {
      status = Uninitialised;
      return TRUE;
    }
    status = DeinitialiseFailed;
  }
  return FALSE;
}


BOOL PModem::CanDial() const
{
  switch (status) {
    case Unopened :
    case Uninitialised :
    case Initialising :
    case InitialiseFailed :
    case Dialling :
    case AwaitingResponse :
    case Connected :
    case HangingUp :
    case Deinitialising :
    case DeinitialiseFailed :
    case SendingUserCommand :
      return FALSE;

    default :
      return TRUE;
  }
}


BOOL PModem::Dial(const PString & number)
{
  if (!CanDial())
    return FALSE;

  status = Dialling;
  if (!SendCommandString(preDialCmd + "\\s" + number + postDialCmd)) {
    status = DialFailed;
    return FALSE;
  }

  status = AwaitingResponse;

  PTimer timeout = 120000;
  PINDEX connectPosition = 0;
  PINDEX busyPosition = 0;
  PINDEX noCarrierPosition = 0;

  for (;;) {
    int nextChar;
    if ((nextChar = ReadCharWithTimeout(timeout)) < 0)
      return FALSE;

    if (ReceiveCommandString(nextChar, connectReply, connectPosition, 0))
      break;

    if (ReceiveCommandString(nextChar, busyReply, busyPosition, 0)) {
      status = LineBusy;
      return FALSE;
    }

    if (ReceiveCommandString(nextChar, noCarrierReply, noCarrierPosition, 0)) {
      status = NoCarrier;
      return FALSE;
    }
  }

  status = Connected;
  return TRUE;
}


BOOL PModem::CanHangUp() const
{
  switch (status) {
    case Unopened :
    case Uninitialised :
    case Initialising :
    case InitialiseFailed :
    case Dialling :
    case AwaitingResponse :
    case HangingUp :
    case Deinitialising :
    case SendingUserCommand :
      return FALSE;

    default :
      return TRUE;
  }
}


BOOL PModem::HangUp()
{
  if (CanHangUp()) {
    status = HangingUp;
    if (SendCommandString(hangUpCmd)) {
      status = Initialised;
      return TRUE;
    }
    status = HangUpFailed;
  }
  return FALSE;
}


BOOL PModem::CanSendUser() const
{
  switch (status) {
    case Unopened :
    case Uninitialised :
    case Initialising :
    case InitialiseFailed :
    case Dialling :
    case AwaitingResponse :
    case HangingUp :
    case Deinitialising :
    case SendingUserCommand :
      return FALSE;

    default :
      return TRUE;
  }
}


BOOL PModem::SendUser(const PString & str)
{
  if (CanSendUser()) {
    Status oldStatus = status;
    status = SendingUserCommand;
    if (SendCommandString(str)) {
      status = oldStatus;
      return TRUE;
    }
    status = oldStatus;
  }
  return FALSE;
}


void PModem::Abort()
{
  switch (status) {
    case Initialising :
      status = InitialiseFailed;
      break;
    case Dialling :
    case AwaitingResponse :
      status = DialFailed;
      break;
    case HangingUp :
      status = HangUpFailed;
      break;
    case Deinitialising :
      status = DeinitialiseFailed;
      break;
    default :
      break;
  }
}


BOOL PModem::CanRead() const
{
  switch (status) {
    case Unopened :
    case Initialising :
    case Dialling :
    case AwaitingResponse :
    case HangingUp :
    case Deinitialising :
    case SendingUserCommand :
      return FALSE;

    default :
      return TRUE;
  }
}


void PSerialChannel::ClearDTR()
{
  SetDTR(FALSE);
}


void PSerialChannel::ClearRTS()
{
  SetRTS(FALSE);
}


void PSerialChannel::ClearBreak()
{
  SetBreak(FALSE);
}


#endif


// End Of File ///////////////////////////////////////////////////////////////
