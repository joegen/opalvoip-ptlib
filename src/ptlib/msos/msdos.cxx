/*
 * $Id: msdos.cxx,v 1.3 1994/07/17 11:01:04 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: msdos.cxx,v $
 * Revision 1.3  1994/07/17 11:01:04  robertj
 * Ehancements, implementation, bug fixes etc.
 *
 * Revision 1.2  1994/07/02  03:18:09  robertj
 * Multi-threading implementation.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 *
// Revision 1.1  1994/04/01  14:39:35  robertj
// Initial revision
//
 */

#include "ptlib.h"

#include <dos.h>


///////////////////////////////////////////////////////////////////////////////
// PTime

PString PTime::GetTimeSeparator()
{
  return "";
}


BOOL PTime::GetTimeAMPM()
{
  return FALSE;
}


PString PTime::GetTimeAM()
{
  return "am";
}


PString PTime::GetTimePM()
{
  return "pm";
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
  return (abbreviated ? abbrev_weekdays : weekdays)[dayOfWeek];
}


PString PTime::GetDateSeparator()
{
  return "-";
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
  return (abbreviated ? abbrev_months : months)[month];
}


PTime::DateOrder PTime::GetDateOrder()
{
  return DayMonthYear;
}


///////////////////////////////////////////////////////////////////////////////
// PSerialChannel

void PSerialChannel::Construct()
{
  commsId = -1;
  biosParm = 0xe3; // 9600 baud, no parity, 1 stop bit, 8 data bits
}


void PSerialChannel::CopyContents(const PSerialChannel & chan)
{
  commsId = chan.commsId;
}


PString PSerialChannel::GetName() const
{
  if (IsOpen())
    return psprintf("COM%i", commsId+1);

  return PString();
}


BOOL PSerialChannel::Read(void * buf, PINDEX len)
{
  char * b = (char *)buf;
  while (len > 0) {
    int c = ReadChar();
    if (c >= 0) {
      *b++ = (char)c;
      len--;
    }
  }
  return len == 0;
}


BOOL PSerialChannel::Write(const void * buf, PINDEX len)
{
  const char * b = (const char *)buf;
  while (len-- > 0) {
    if (!WriteChar(*b++))
      return FALSE;
  }
  return TRUE;
}


BOOL PSerialChannel::Close()
{
  if (!IsOpen())
    return FALSE;

  commsId = -1;
  return TRUE;
}


BOOL PSerialChannel::SetCommsParam(DWORD speed, BYTE data, Parity parity,
                     BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  union _REGS reg;

  reg.h.ah = 0;
  reg.h.al = biosParm;

  switch (speed) {
    case 0 :
      break;
    case 110 :
      reg.h.al &= 0x1f;
      break;
    case 150 :
      reg.h.al &= 0x1f;
      reg.h.al |= 0x20;
      break;
    case 300 :
      reg.h.al &= 0x1f;
      reg.h.al |= 0x40;
      break;
    case 600 :
      reg.h.al &= 0x1f;
      reg.h.al |= 0x60;
      break;
    case 1200 :
      reg.h.al &= 0x1f;
      reg.h.al |= 0x80;
      break;
    case 2400 :
      reg.h.al &= 0x1f;
      reg.h.al |= 0xa0;
      break;
    case 4800 :
      reg.h.al &= 0x1f;
      reg.h.al |= 0xc0;
      break;
    case 9600 :
      reg.h.al &= 0x1f;
      reg.h.al |= 0xe0;
      break;
    default :
      return FALSE;
  }

  switch (data) {
    case 0 :
      break;
    case 5 :
      reg.h.al &= 0xfc;
      break;
    case 6 :
      reg.h.al &= 0xfc;
      reg.h.al |= 1;
      break;
    case 7 :
      reg.h.al &= 0xfc;
      reg.h.al |= 2;
      break;
    case 8 :
      reg.h.al &= 0xfc;
      reg.h.al |= 3;
      break;
    default :
      return FALSE;
  }

  switch (parity) {
    case DefaultParity :
      break;
    case NoParity :
      reg.h.al &= 0xe7;
      break;
    case OddParity :
      reg.h.al &= 0xe7;
      reg.h.al |= 8;
      break;
    case EvenParity :
      reg.h.al &= 0xe7;
      reg.h.al |= 0x10;
      break;
    default :
      return FALSE;
  }

  switch (stop) {
    case 0 :
      break;
    case 1 :
      reg.h.al &= ~4;
      break;
    case 2 :
      reg.h.al |= 4;
      break;
    default :
      return FALSE;
  }

  if (outputFlow != DefaultFlowControl || inputFlow != DefaultFlowControl)
    return FALSE;

  reg.x.dx = commsId;
  _int86(0x14, &reg, &reg);
  biosParm = reg.h.al;
  return TRUE;
}

BOOL PSerialChannel::Open(const PString & port, DWORD speed, BYTE data,
       Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  Close();

  commsId = -1;
  if (PCaselessString("COM") != port.Left(3) &&
                                              port[3] >= '1' && port[3] <= '4')
    return FALSE;
  commsId = port[3] - '1';
  return SetCommsParam(speed, data, parity, stop, inputFlow, outputFlow);
}


BOOL PSerialChannel::SetSpeed(DWORD speed)
{
  return SetCommsParam(speed,
                 0, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


DWORD PSerialChannel::GetSpeed() const
{
  static int speed[8] = { 110, 150, 300, 600, 1200, 2400, 4800, 9600 };
  return speed[biosParm>>5];
}


BOOL PSerialChannel::SetDataBits(BYTE data)
{
  return SetCommsParam(0,
              data, DefaultParity, 0, DefaultFlowControl, DefaultFlowControl);
}


BYTE PSerialChannel::GetDataBits() const
{
  return (BYTE)((biosParm&3)+5);
}


BOOL PSerialChannel::SetParity(Parity parity)
{
  return SetCommsParam(0,0, parity, 0, DefaultFlowControl, DefaultFlowControl);
}


PSerialChannel::Parity PSerialChannel::GetParity() const
{
  return (biosParm&8) == 0 ? NoParity :
                                (biosParm&0x10) == 0 ? OddParity : EvenParity;
}


BOOL PSerialChannel::SetStopBits(BYTE stop)
{
  return SetCommsParam(0,
               0, DefaultParity, stop, DefaultFlowControl, DefaultFlowControl);
}


BYTE PSerialChannel::GetStopBits() const
{
  return (BYTE)(((biosParm&4)>>3)+1);
}


BOOL PSerialChannel::SetInputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0, DefaultParity, 0, flowControl, DefaultFlowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetInputFlowControl() const
{
  return RtsCts;
}


BOOL PSerialChannel::SetOutputFlowControl(FlowControl flowControl)
{
  return SetCommsParam(0,0, DefaultParity, 0, DefaultFlowControl, flowControl);
}


PSerialChannel::FlowControl PSerialChannel::GetOutputFlowControl() const
{
  return RtsCts;
}


void PSerialChannel::SetDTR(BOOL state)
{
  if (!IsOpen())
    return;

  union _REGS reg;
  reg.h.ah = 5;
  reg.h.al = 0;
  reg.x.dx = commsId;
  _int86(0x14, &reg, &reg);
  if (state)
    reg.h.bl |= 1;
  else
    reg.h.bl &= ~1;
  reg.h.ah = 5;
  reg.h.al = 1;
  reg.x.dx = commsId;
  _int86(0x14, &reg, &reg);
}


void PSerialChannel::SetBreak(BOOL state)
{
  if (!IsOpen())
    return;

  int s = state;
}


BOOL PSerialChannel::GetCTS()
{
  if (!IsOpen())
    return FALSE;

  union _REGS reg;
  reg.h.ah = 3;
  reg.x.dx = commsId;
  return (_int86(0x14, &reg, &reg)&0x8010) == 0x10;
}


BOOL PSerialChannel::GetDSR()
{
  if (!IsOpen())
    return FALSE;

  union _REGS reg;
  reg.h.ah = 3;
  reg.x.dx = commsId;
  return (_int86(0x14, &reg, &reg)&0x8020) == 0x20;
}


BOOL PSerialChannel::GetDCD()
{
  if (!IsOpen())
    return FALSE;

  union _REGS reg;
  reg.h.ah = 3;
  reg.x.dx = commsId;
  return (_int86(0x14, &reg, &reg)&0x8080) == 0x80;
}


BOOL PSerialChannel::GetRing()
{
  if (!IsOpen())
    return FALSE;

  union _REGS reg;
  reg.h.ah = 3;
  reg.x.dx = commsId;
  return (_int86(0x14, &reg, &reg)&0x8040) == 0x40;
}


PStringList PSerialChannel::GetPortNames()
{
  static char buf[] = "COM ";
  PStringList ports;
  for (char p = '1'; p <= '4'; p++) {
    if (*(WORD *)(0x00400000+p-'1') != 0) {
      buf[3] = p;
      ports.Append(new PString(buf));
    }
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

  if (!configFile.IsEmpty()) {
    PAssertAlways(PUnimplementedFunction);
  }

  return sections;
}


PStringList PConfig::GetKeys(const char * section) const
{
  PStringList keys;

  if (configFile.IsEmpty()) {
    char ** ptr = _environ;
    while (*ptr != NULL) {
      PString buf = *ptr++;
      keys.AppendString(buf.Left(buf.Find('=')));
    }
  }
  else {
    if (section == NULL)
      section = defaultSection;
    PAssertAlways(PUnimplementedFunction);
  }

  return keys;
}


void PConfig::DeleteSection(const char * section)
{
  if (configFile.IsEmpty())
    return;

  if (section == NULL)
    section = defaultSection;

  PAssertAlways(PUnimplementedFunction);
}


void PConfig::DeleteKey(const char * section, const char * key)
{
  PAssert(section != NULL && *section != '\0', PInvalidParameter);
  PAssert(key != NULL && *key != '\0', PInvalidParameter);

  if (configFile.IsEmpty()) {
    PString str = key;
    PAssert(str.Find('=') == P_MAX_INDEX, PInvalidParameter);
    _putenv(str + "=");
  }
  else
    PAssertAlways(PUnimplementedFunction);
}


PString PConfig::GetString(const char * section,
                                          const char * key, const char * dflt)
{
  PString str;

  PAssert(section != NULL && *section != '\0', PInvalidParameter);
  PAssert(key != NULL && *key != '\0', PInvalidParameter);

  if (configFile.IsEmpty()) {
    PAssert(strchr(key, '=') == NULL, PInvalidParameter);
    char * env = getenv(key);
    if (env != NULL)
      str = env;
    else
      str = dflt;
  }
  else {
    PAssertAlways(PUnimplementedFunction);
  }
  return str;
}


void PConfig::SetString(const char * section,
                                         const char * key, const char * value)
{
  PAssert(section != NULL && *section != '\0', PInvalidParameter);
  PAssert(key != NULL && *key != '\0', PInvalidParameter);
  PAssert(value != NULL, PInvalidParameter);

  if (configFile.IsEmpty()) {
    PString str = key;
    PAssert(str.Find('=') == P_MAX_INDEX, PInvalidParameter);
    _putenv(str + "=" + value);
  }
  else
    PAssertAlways(PUnimplementedFunction);
}


///////////////////////////////////////////////////////////////////////////////
// Threads

void PThread::SwitchContext(PThread * from)
{
  if (setjmp(from->context) != 0) // Are being reactivated from previous yield
    return;

  if (status == Starting) {
    if (setjmp(context) != 0) {
      status = Running;
      Main();
      Terminate(); // Never returns from here
    }
    context[7] = (int)stackTop-16;  // Change the stack pointer in jmp_buf
  }

  longjmp(context, TRUE);
  PAssertAlways("longjmp failed"); // Should never get here
}


// End Of File ///////////////////////////////////////////////////////////////
