#include <ptlib.h>
#include <ptlib/serchan.h>

///////////////////////////////////////////////////////////////////////////////
// PSerialChannel

void PSerialChannel::Construct()
{
  commsResource = INVALID_HANDLE_VALUE;

  char str[50];
  strcpy(str, "com1");
  GetProfileString("ports", str, "9600,n,8,1,x", &str[5], sizeof(str)-6);
  str[4] = ':';
  memset(&deviceControlBlock, 0, sizeof(deviceControlBlock));
  deviceControlBlock.DCBlength = sizeof(deviceControlBlock);
  BuildCommDCB(str, &deviceControlBlock);
}


PString PSerialChannel::GetName() const
{
  return portName;
}


BOOL PSerialChannel::Read(void * buf, PINDEX len)
{
  lastReadCount = 0;

  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  COMMTIMEOUTS cto;
  PAssertOS(GetCommTimeouts(commsResource, &cto));
  cto.ReadIntervalTimeout = 0;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 0;
  cto.ReadIntervalTimeout = MAXDWORD; // Immediate timeout
  PAssertOS(SetCommTimeouts(commsResource, &cto));

  DWORD eventMask;
  PAssertOS(GetCommMask(commsResource, &eventMask));
  if (eventMask != (EV_RXCHAR|EV_TXEMPTY))
    PAssertOS(SetCommMask(commsResource, EV_RXCHAR|EV_TXEMPTY));

  PWin32Overlapped overlap;
  DWORD timeToGo = readTimeout.GetInterval();
  DWORD bytesToGo = len;
  char * bufferPtr = (char *)buf;

  for (;;) {
    DWORD readCount = 0;
    if (!ReadFile(commsResource, bufferPtr, bytesToGo, &readCount, &overlap)) {
      if (GetLastError() != ERROR_IO_PENDING)
        return ConvertOSError(-2);
      if (!GetOverlappedResult(commsResource, &overlap, &readCount, FALSE))
        return ConvertOSError(-2);
    }

    bytesToGo -= readCount;
    bufferPtr += readCount;
    lastReadCount += readCount;
    if (lastReadCount >= len || timeToGo == 0)
      return lastReadCount > 0;

    if (!WaitCommEvent(commsResource, &eventMask, &overlap)) {
      if (GetLastError() != ERROR_IO_PENDING)
        return ConvertOSError(-2);
      if (WaitForSingleObject(overlap.hEvent, timeToGo) == WAIT_FAILED)
        return ConvertOSError(-2);
    }
  }
}


BOOL PSerialChannel::Write(const void * buf, PINDEX len)
{
  lastWriteCount = 0;

  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  COMMTIMEOUTS cto;
  PAssertOS(GetCommTimeouts(commsResource, &cto));
  cto.WriteTotalTimeoutMultiplier = 0;
  if (writeTimeout == PMaxTimeInterval)
    cto.WriteTotalTimeoutConstant = 0;
  else if (writeTimeout <= PTimeInterval(0))
    cto.WriteTotalTimeoutConstant = 1;
  else
    cto.WriteTotalTimeoutConstant = writeTimeout.GetInterval();
  PAssertOS(SetCommTimeouts(commsResource, &cto));

  PWin32Overlapped overlap;
  memset(&overlap, 0, sizeof(overlap));
  overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (WriteFile(commsResource, buf, len, (LPDWORD)&lastWriteCount, &overlap)) {
    CloseHandle(overlap.hEvent);
    return lastWriteCount == len;
  }

  if (GetLastError() == ERROR_IO_PENDING)
    if (GetOverlappedResult(commsResource,
                                   &overlap, (LPDWORD)&lastWriteCount, TRUE)) {
      CloseHandle(overlap.hEvent);
      return lastWriteCount == len;
    }

  ConvertOSError(-2);
  CloseHandle(overlap.hEvent);

  return FALSE;
}


BOOL PSerialChannel::Close()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  CloseHandle(commsResource);
  commsResource = INVALID_HANDLE_VALUE;
  os_handle = -1;
  return ConvertOSError(-2);
}


BOOL PSerialChannel::SetCommsParam(DWORD speed, BYTE data, Parity parity,
                     BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  if (speed > 0)
    deviceControlBlock.BaudRate = speed;

  if (data > 0)
    deviceControlBlock.ByteSize = data;

  switch (parity) {
    case NoParity :
      deviceControlBlock.Parity = NOPARITY;
      break;
    case OddParity :
      deviceControlBlock.Parity = ODDPARITY;
      break;
    case EvenParity :
      deviceControlBlock.Parity = EVENPARITY;
      break;
    case MarkParity :
      deviceControlBlock.Parity = MARKPARITY;
      break;
    case SpaceParity :
      deviceControlBlock.Parity = SPACEPARITY;
      break;
  }

  switch (stop) {
    case 1 :
      deviceControlBlock.StopBits = ONESTOPBIT;
      break;
    case 2 :
      deviceControlBlock.StopBits = TWOSTOPBITS;
      break;
  }

  switch (inputFlow) {
    case NoFlowControl :
      deviceControlBlock.fRtsControl = RTS_CONTROL_DISABLE;
      deviceControlBlock.fInX = FALSE;
      break;
    case XonXoff :
      deviceControlBlock.fRtsControl = RTS_CONTROL_DISABLE;
      deviceControlBlock.fInX = TRUE;
      break;
    case RtsCts :
      deviceControlBlock.fRtsControl = RTS_CONTROL_HANDSHAKE;
      deviceControlBlock.fInX = FALSE;
      break;
  }

  switch (outputFlow) {
    case NoFlowControl :
      deviceControlBlock.fOutxCtsFlow = FALSE;
      deviceControlBlock.fOutxDsrFlow = FALSE;
      deviceControlBlock.fOutX = FALSE;
      break;
    case XonXoff :
      deviceControlBlock.fOutxCtsFlow = FALSE;
      deviceControlBlock.fOutxDsrFlow = FALSE;
      deviceControlBlock.fOutX = TRUE;
      break;
    case RtsCts :
      deviceControlBlock.fOutxCtsFlow = TRUE;
      deviceControlBlock.fOutxDsrFlow = FALSE;
      deviceControlBlock.fOutX = FALSE;
      break;
  }

  if (IsOpen())
   return ConvertOSError(SetCommState(commsResource,
                                                &deviceControlBlock) ? 0 : -2);

  osError = EBADF;
  lastError = NotOpen;
  return FALSE;
}


BOOL PSerialChannel::Open(const PString & port, DWORD speed, BYTE data,
               Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  Close();

  portName = port;
  if (portName.Find(PDIR_SEPARATOR) == P_MAX_INDEX)
    portName = "\\\\.\\" + port;
  commsResource = CreateFile(portName,
                             GENERIC_READ|GENERIC_WRITE,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL);
  if (commsResource == INVALID_HANDLE_VALUE)
    return ConvertOSError(-2);

  os_handle = 0;

  SetupComm(commsResource, 2048, 2048);

  if (SetCommsParam(speed, data, parity, stop, inputFlow, outputFlow))
    return TRUE;

  ConvertOSError(-2);
  CloseHandle(commsResource);
  os_handle = -1;
  return FALSE;
}


BOOL PSerialChannel::SetSpeed(DWORD speed)
{
  return SetCommsParam(speed,
                  0, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


DWORD PSerialChannel::GetSpeed() const
{
  return deviceControlBlock.BaudRate;
}


BOOL PSerialChannel::SetDataBits(BYTE data)
{
  return SetCommsParam(0,
               data, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


BYTE PSerialChannel::GetDataBits() const
{
  return deviceControlBlock.ByteSize;
}


BOOL PSerialChannel::SetParity(Parity parity)
{
  return SetCommsParam(0,0,parity,0,DefaultFlowControl,DefaultFlowControl);
}


PSerialChannel::Parity PSerialChannel::GetParity() const
{
  switch (deviceControlBlock.Parity) {
    case ODDPARITY :
      return OddParity;
    case EVENPARITY :
      return EvenParity;
    case MARKPARITY :
      return MarkParity;
    case SPACEPARITY :
      return SpaceParity;
  }
  return NoParity;
}


BOOL PSerialChannel::SetStopBits(BYTE stop)
{
  return SetCommsParam(0,
               0, DefaultParity, stop, DefaultFlowControl, DefaultFlowControl);
}


BYTE PSerialChannel::GetStopBits() const
{
  return (BYTE)(deviceControlBlock.StopBits == ONESTOPBIT ? 1 : 2);
}


BOOL PSerialChannel::SetInputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0,DefaultParity,0,flowControl,DefaultFlowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetInputFlowControl() const
{
  if (deviceControlBlock.fRtsControl == RTS_CONTROL_HANDSHAKE)
    return RtsCts;
  if (deviceControlBlock.fInX != 0)
    return XonXoff;
  return NoFlowControl;
}


BOOL PSerialChannel::SetOutputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0,DefaultParity,0,DefaultFlowControl,flowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetOutputFlowControl() const
{
  if (deviceControlBlock.fOutxCtsFlow != 0)
    return RtsCts;
  if (deviceControlBlock.fOutX != 0)
    return XonXoff;
  return NoFlowControl;
}


void PSerialChannel::SetDTR(BOOL state)
{
  if (IsOpen())
    PAssertOS(EscapeCommFunction(commsResource, state ? SETDTR : CLRDTR));
  else {
    osError = EBADF;
    lastError = NotOpen;
  }
}


void PSerialChannel::SetRTS(BOOL state)
{
  if (IsOpen())
    PAssertOS(EscapeCommFunction(commsResource, state ? SETRTS : CLRRTS));
  else {
    osError = EBADF;
    lastError = NotOpen;
  }
}


void PSerialChannel::SetBreak(BOOL state)
{
  if (IsOpen())
    if (state)
      PAssertOS(SetCommBreak(commsResource));
    else
      PAssertOS(ClearCommBreak(commsResource));
  else {
    osError = EBADF;
    lastError = NotOpen;
  }
}


BOOL PSerialChannel::GetCTS()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_CTS_ON) != 0;
}


BOOL PSerialChannel::GetDSR()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_DSR_ON) != 0;
}


BOOL PSerialChannel::GetDCD()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_RLSD_ON) != 0;
}


BOOL PSerialChannel::GetRing()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DWORD stat;
  PAssertOS(GetCommModemStatus(commsResource, &stat));
  return (stat&MS_RING_ON) != 0;
}


PStringList PSerialChannel::GetPortNames()
{
  PStringList ports;
  for (char p = 1; p <= 9; p++)
    ports.AppendString(psprintf("\\\\.\\COM%u", p));
  return ports;
}
