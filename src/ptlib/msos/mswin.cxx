/*
 * $Id: mswin.cxx,v 1.1 1994/06/25 12:13:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: mswin.cxx,v $
 * Revision 1.1  1994/06/25 12:13:01  robertj
 * Initial revision
 *
// Revision 1.1  1994/04/01  14:39:35  robertj
// Initial revision
//
 */

#include "ptlib.h"
#include <errno.h>

#include <stdresid.h>


extern "C" HINSTANCE _hInstance;

///////////////////////////////////////////////////////////////////////////////
// PTime

PString PTime::GetTimeSeparator()
{
  PString str;
  GetProfileString("intl", "sTime", ":", str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


BOOL PTime::GetTimeAMPM()
{
  return GetProfileInt("intl", "iTime", 0) != 0;
}


PString PTime::GetTimeAM()
{
  PString str;
  GetProfileString("intl", "s1159", "am", str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetTimePM()
{
  PString str;
  GetProfileString("intl", "s2359", "pm", str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetDayName(Weekdays dayOfWeek, BOOL abbreviated)
{
  static const char * const weekdays[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };
  static const char * const abbrev_weekdays[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  PString str;
  if (LoadString(_hInstance, dayOfWeek+
        (abbreviated ? PSTD_ID_STR_ABBREV_WEEKDAYS : PSTD_ID_STR_WEEKDAYS),
        str.GetPointer(100), 99) == 0)
    return (abbreviated ? abbrev_weekdays : weekdays)[dayOfWeek];
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetDateSeparator()
{
  PString str;
  GetProfileString("intl", "sDate", "-", str.GetPointer(100), 99);
  str.MakeMinimumSize();
  return str;
}


PString PTime::GetMonthName(Months month, BOOL abbreviated)
{
  static const char * const months[] = { "",
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
  };
  static const char * const abbrev_months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  PString str;
  if (LoadString(_hInstance, month+
       (UINT)(abbreviated ? PSTD_ID_STR_ABBREV_MONTHS : PSTD_ID_STR_MONTHS),
       str.GetPointer(100), 99) == 0)
    return (abbreviated ? abbrev_months : months)[month];
  str.MakeMinimumSize();
  return str;
}


PTime::DateOrder PTime::GetDateOrder()
{
  return (DateOrder)GetProfileInt("intl", "iDate", 0);
}


///////////////////////////////////////////////////////////////////////////////
// PSerialChannel

void PSerialChannel::Construct()
{
  commsId = -1;
}


void PSerialChannel::CopyContents(const PSerialChannel & chan)
{
  commsId = chan.commsId;
}


BOOL PSerialChannel::IsOpen() const
{
  return commsId >= 0;
}


PString PSerialChannel::GetName() const
{
  if (IsOpen())
    return psprintf("COM%i", commsId+1);

  return PString();
}


BOOL PSerialChannel::IsReadBlocked(PObject * obj)
{
  COMSTAT stat;
  GetCommError(((PSerialChannel *)obj)->commsId, &stat);
  return stat.cbInQue <= 0 &&
                   PTimer::Tick() < ((PSerialChannel *)obj)->readTimeoutTarget;
}


BOOL PSerialChannel::Read(void * buf, PINDEX len)
{
  lastReadCount = 0;

  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  readTimeoutTarget = PTimer::Tick() + readTimeout;
  if (IsReadBlocked(this))
    PThread::Current()->Block(&PSerialChannel::IsReadBlocked, this);

  lastReadCount = ReadComm(commsId, buf, len);
  if (lastReadCount > 0)
    return TRUE;

  COMSTAT stat;
  GetCommError(commsId, &stat);
  osError = EFAULT;
  lastReadCount = -lastReadCount;
  return lastReadCount > 0;
}


BOOL PSerialChannel::IsWriteBlocked(PObject * obj)
{
  COMSTAT stat;
  GetCommError(((PSerialChannel *)obj)->commsId, &stat);
  return stat.cbOutQue >= OutputQueueSize &&
                 PTimer::Tick() < ((PSerialChannel *)obj)->writeTimeoutTarget;
}


BOOL PSerialChannel::Write(const void * buf, PINDEX len)
{
  lastWriteCount = 0;

  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  writeTimeoutTarget = PTimer::Tick() + writeTimeout;
  if (IsWriteBlocked(this))
    PThread::Current()->Block(&PSerialChannel::IsWriteBlocked, this);

  lastWriteCount = WriteComm(commsId, buf, len);
  if (lastWriteCount <= 0) {
    COMSTAT stat;
    GetCommError(commsId, &stat);
    osError = EFAULT;
    lastWriteCount = -lastWriteCount;
  }
  return lastWriteCount >= len;
}


BOOL PSerialChannel::Close()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  BOOL retVal = CloseComm(commsId) == 0;
  commsId = -1;
  return retVal;
}


BOOL PSerialChannel::SetCommsParam(DWORD speed, BYTE data, Parity parity,
                     BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  DCB dcb;
  PAssert(GetCommState(commsId, &dcb) == 0, POperatingSystemError);

  switch (speed) {
    case 0 :
      break;
    case 14400 :
      dcb.BaudRate = CBR_14400;
      break;
    case 19200 :
      dcb.BaudRate = CBR_19200;
      break;
    case 38400 :
      dcb.BaudRate = CBR_38400;
      break;
    case 56000 :
      dcb.BaudRate = CBR_56000;
      break;
    case 128000 :
      dcb.BaudRate = CBR_128000;
      break;
    case 256000 :
      dcb.BaudRate = CBR_256000;
      break;
    default :
      if (speed > 9600) {
        osError = EINVAL;
        return FALSE;
      }
      dcb.BaudRate = (UINT)speed;
  }

  if (data > 0)
    dcb.ByteSize = data;

  switch (parity) {
    case NoParity :
      dcb.Parity = NOPARITY;
      break;
    case OddParity :
      dcb.Parity = ODDPARITY;
      break;
    case EvenParity :
      dcb.Parity = EVENPARITY;
      break;
    case MarkParity :
      dcb.Parity = MARKPARITY;
      break;
    case SpaceParity :
      dcb.Parity = SPACEPARITY;
      break;
  }
  switch (stop) {
    case 1 :
      dcb.StopBits = ONESTOPBIT;
      break;
    case 2 :
      dcb.StopBits = TWOSTOPBITS;
      break;
  }
  switch (inputFlow) {
    case NoFlowControl :
      dcb.fRtsflow = FALSE;
      dcb.fInX = FALSE;
      break;
    case XonXoff :
      dcb.fRtsflow = FALSE;
      dcb.fInX = TRUE;
      break;
    case RtsCts :
      dcb.fRtsflow = TRUE;
      dcb.fInX = FALSE;
      break;
  }

  switch (outputFlow) {
    case NoFlowControl :
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fOutX = FALSE;
      break;
    case XonXoff :
      dcb.fOutxCtsFlow = FALSE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fOutX = TRUE;
      break;
    case RtsCts :
      dcb.fOutxCtsFlow = TRUE;
      dcb.fOutxDsrFlow = FALSE;
      dcb.fOutX = FALSE;
      break;
  }

  if (SetCommState(&dcb) >= 0)
    return TRUE;

  osError = EINVAL;
  return FALSE;
}


BOOL PSerialChannel::Open(const PString & port, DWORD speed, BYTE data,
       Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  Close();

  commsId = OpenComm(port, InputQueueSize, OutputQueueSize);
  if (commsId < 0) {
    switch (commsId) {
      case IE_BADID :
      case IE_HARDWARE :
        osError = ENOENT;
        lastError = NotFound;
        break;
      case IE_OPEN :
        osError = EBUSY;
        lastError = DeviceInUse;
        break;
      case IE_MEMORY :
        osError = ENOMEM;
        lastError = NoMemory;
        break;
      case IE_BAUDRATE :
      case IE_BYTESIZE :
        osError = EINVAL;
        lastError = BadParameter;
        break;
      default :
        osError = EFAULT;
        lastError = Miscellaneous;
    }
    commsId = -1;
    return FALSE;
  }

  char str[30];
  GetProfileString("ports", port, "9600,n,8,1,x", str, sizeof(str));
  DCB dcb;
  if (BuildCommDCB(port + ":" + str, &dcb))
    SetCommState(&dcb);

  if (!SetCommsParam(speed, data, parity, stop, inputFlow, outputFlow)) {
    CloseComm(commsId);
    return FALSE;
  }

  SetCommEventMask(commsId, EV_CTSS|EV_DSR|EV_RING|EV_RLSDS);
  return TRUE;
}


BOOL PSerialChannel::SetSpeed(DWORD speed)
{
  return SetCommsParam(speed,
                 0, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


DWORD PSerialChannel::GetSpeed() const
{
  if (!IsOpen())
    return 9600;

  DCB dcb;
  PAssert(GetCommState(commsId, &dcb) == 0, POperatingSystemError);
  switch (dcb.BaudRate) {
    case CBR_110 :
      return 110;
    case CBR_300 :
      return 300;
    case CBR_600 :
      return 600;
    case CBR_1200 :
      return 1200;
    case CBR_2400 :
      return 2400;
    case CBR_4800 :
      return 4800;
    case CBR_9600 :
      return 9600;
    case CBR_14400 :
      return 14400;
    case CBR_19200 :
      return 19200;
    case CBR_38400 :
      return 38400;
    case CBR_56000 :
      return 56000;
    case CBR_128000 :
      return 128000;
    case CBR_256000 :
      return 256000;
  }
  return dcb.BaudRate;
}


BOOL PSerialChannel::SetDataBits(BYTE data)
{
  return SetCommsParam(0,
              data, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


BYTE PSerialChannel::GetDataBits() const
{
  if (!IsOpen())
    return 8;

  DCB dcb;
  PAssert(GetCommState(commsId, &dcb) == 0, POperatingSystemError);
  return dcb.ByteSize;
}


BOOL PSerialChannel::SetParity(Parity parity)
{
  return SetCommsParam(0,0, parity, 0, DefaultFlowControl, DefaultFlowControl);
}


PSerialChannel::Parity PSerialChannel::GetParity() const
{
  if (!IsOpen())
    return NoParity;

  DCB dcb;
  PAssert(GetCommState(commsId, &dcb) == 0, POperatingSystemError);
  switch (dcb.Parity) {
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
  if (!IsOpen())
    return 1;

  DCB dcb;
  PAssert(GetCommState(commsId, &dcb) == 0, POperatingSystemError);
  return (BYTE)(dcb.StopBits == ONESTOPBIT ? 1 : 2);
}


BOOL PSerialChannel::SetInputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0, DefaultParity, 0, flowControl, DefaultFlowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetInputFlowControl() const
{
  if (!IsOpen())
    return NoFlowControl;

  DCB dcb;
  PAssert(GetCommState(commsId, &dcb) == 0, POperatingSystemError);
  if (dcb.fRtsflow)
    return RtsCts;
  if (dcb.fInX != 0)
    return XonXoff;
  return NoFlowControl;
}


BOOL PSerialChannel::SetOutputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0, DefaultParity, 0, DefaultFlowControl, flowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetOutputFlowControl() const
{
  if (!IsOpen())
    return NoFlowControl;

  DCB dcb;
  PAssert(GetCommState(commsId, &dcb) == 0, POperatingSystemError);
  if (dcb.fOutxCtsFlow != 0)
    return RtsCts;
  if (dcb.fOutX != 0)
    return XonXoff;
  return NoFlowControl;
}


void PSerialChannel::SetDTR(BOOL state)
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return;
  }

  PAssert(EscapeCommFunction(commsId, state ? SETDTR : CLRDTR),
                                                      POperatingSystemError);
}


void PSerialChannel::SetRTS(BOOL state)
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return;
  }

  PAssert(EscapeCommFunction(commsId, state ? SETRTS : CLRRTS),
                                                      POperatingSystemError);
}


void PSerialChannel::SetBreak(BOOL state)
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return;
  }

  if (state)
    PAssert(SetCommBreak(commsId), POperatingSystemError);
  else
    PAssert(ClearCommBreak(commsId), POperatingSystemError);
}


BOOL PSerialChannel::GetCTS()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  return (GetCommEventMask(commsId, 0)&EV_CTSS) != 0;
}


BOOL PSerialChannel::GetDSR()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  return (GetCommEventMask(commsId, 0)&EV_DSR) != 0;
}


BOOL PSerialChannel::GetDCD()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  return (GetCommEventMask(commsId, 0)&EV_RLSDS) != 0;
}


BOOL PSerialChannel::GetRing()
{
  if (!IsOpen()) {
    osError = EBADF;
    lastError = NotOpen;
    return FALSE;
  }

  return (GetCommEventMask(commsId, 0)&EV_RING) != 0;
}


PStringList PSerialChannel::GetPortNames()
{
  static char buf[] = "COM ";
  PStringList ports;
  for (char p = '1'; p <= '4'; p++) {
    buf[3] = p;
    ports.Append(new PString(buf));
  }
  return ports;
}


///////////////////////////////////////////////////////////////////////////////
// Configuration files

PConfig::PConfig(Source src, const PString & section)
  : defaultSection(section)
{
  switch (src) {
    case System :
      configFile = PFilePath("WIN.INI");
      break;

    case Application :
      PFilePath appFile = PProcess::Current()->GetFile();
      configFile = appFile.GetVolume() +
                              appFile.GetPath() + appFile.GetTitle() + ".INI";
  }
}


PConfig::PConfig(const PFilePath & filename, const PString & section)
  : configFile(filename), defaultSection(section)
{
}


PStringList PConfig::GetSections()
{
  PStringList sections;
  PString buf;
  char * ptr = buf.GetPointer(10000);
  GetPrivateProfileString(NULL, NULL, "", ptr, 9999, configFile);
  while (*ptr != '\0') {
    sections.AppendString(ptr);
    ptr += strlen(ptr)+1;
  }
  return sections;
}


PStringList PConfig::GetKeys(const char * section) const
{
  if (section == NULL)
    section = defaultSection;

  PStringList keys;
  PString buf;
  char * ptr = buf.GetPointer(10000);
  GetPrivateProfileString(section, NULL, "", ptr, 9999, configFile);
  while (*ptr != '\0') {
    keys.AppendString(ptr);
    ptr += strlen(ptr)+1;
  }
  return keys;
}


void PConfig::DeleteSection(const char * section)
{
  if (section == NULL)
    section = defaultSection;

  PAssert(WritePrivateProfileString(section,
                              NULL, NULL, configFile), POperatingSystemError);
}


void PConfig::DeleteKey(const char * section, const char * key)
{
  PAssert(WritePrivateProfileString(PAssertNULL(section),
                  PAssertNULL(key), NULL, configFile), POperatingSystemError);
}


PString PConfig::GetString(const char * section,
                                          const char * key, const char * dflt)
{
  PString str;
  GetPrivateProfileString(PAssertNULL(section), PAssertNULL(key),
                    PAssertNULL(dflt), str.GetPointer(1000), 999, configFile);
  str.MakeMinimumSize();
  return str;
}


void PConfig::SetString(const char * section,
                                         const char * key, const char * value)
{
  PAssert(WritePrivateProfileString(PAssertNULL(section), PAssertNULL(key),
                      PAssertNULL(value), configFile), POperatingSystemError);
}



///////////////////////////////////////////////////////////////////////////////
// Threads

static char NEAR * NEAR * const StackBase = (char NEAR * NEAR *)0xa;
static char NEAR * NEAR * const StackUsed = (char NEAR * NEAR *)0xc;
static char NEAR * NEAR * const StackTop  = (char NEAR * NEAR *)0xe;

void PThread::SwitchContext(PThread * from)
{
  if (setjmp(from->context) != 0) // Are being reactivated from previous yield
    return;

  // Save some magic global variables in MS-Windows DGROUP segment
  from->stackBase = *StackBase;
  from->stackTop  = *StackTop;
  from->stackUsed = *StackTop - *StackUsed;

  if (status == Starting) {
    if (setjmp(context) != 0) {
      status = Running;
      Main();
      status = Terminating;
      Yield();
      PAssertAlways("Thread not terminated"); // Should never get here
    }
    context[7] = (int)stackTop;  // Change the stack pointer in jmp_buf
  }

  // Restore those MS-Windows magic global for the next context
  *StackBase = stackBase;
  *StackTop = stackTop;
  *StackUsed = stackTop - stackUsed;
  
  longjmp(context, TRUE);
  PAssertAlways("longjmp failed"); // Should never get here
}


// End Of File ///////////////////////////////////////////////////////////////
