/*
 * $Id: msdos.cxx,v 1.7 1994/12/13 11:53:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: msdos.cxx,v $
 * Revision 1.7  1994/12/13 11:53:44  robertj
 * Added missing PConfig Construct() function for pure DOS.
 *
 * Revision 1.6  1994/10/30  11:25:36  robertj
 * Fixed DOS version of configuration files.
 *
 * Revision 1.5  1994/08/22  00:18:02  robertj
 * Added dummy socket function.
 *
 * Revision 1.4  1994/07/27  06:00:10  robertj
 * Backup
 *
 * Revision 1.3  1994/07/17  11:01:04  robertj
 * Ehancements, implementation, bug fixes etc.
 *
 * Revision 1.2  1994/07/02  03:18:09  robertj
 * Multi-threading implementation.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 */

#include "ptlib.h"

#include <bios.h>


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
  biosParm = 0xe3; // 9600 baud, no parity, 1 stop bit, 8 data bits
}


void PSerialChannel::CopyContents(const PSerialChannel & chan)
{
  biosParm = chan.biosParm;
}


PString PSerialChannel::GetName() const
{
  if (IsOpen())
    return psprintf("COM%i", os_handle+1);

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

  os_handle = -1;
  return TRUE;
}


BOOL PSerialChannel::SetCommsParam(DWORD speed, BYTE data, Parity parity,
                     BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  switch (speed) {
    case 0 :
      break;
    case 110 :
      biosParm &= 0x1f;
      break;
    case 150 :
      biosParm &= 0x1f;
      biosParm |= 0x20;
      break;
    case 300 :
      biosParm &= 0x1f;
      biosParm |= 0x40;
      break;
    case 600 :
      biosParm &= 0x1f;
      biosParm |= 0x60;
      break;
    case 1200 :
      biosParm &= 0x1f;
      biosParm |= 0x80;
      break;
    case 2400 :
      biosParm &= 0x1f;
      biosParm |= 0xa0;
      break;
    case 4800 :
      biosParm &= 0x1f;
      biosParm |= 0xc0;
      break;
    case 9600 :
      biosParm &= 0x1f;
      biosParm |= 0xe0;
      break;
    default :
      return FALSE;
  }

  switch (data) {
    case 0 :
      break;
    case 5 :
      biosParm &= 0xfc;
      break;
    case 6 :
      biosParm &= 0xfc;
      biosParm |= 1;
      break;
    case 7 :
      biosParm &= 0xfc;
      biosParm |= 2;
      break;
    case 8 :
      biosParm &= 0xfc;
      biosParm |= 3;
      break;
    default :
      return FALSE;
  }

  switch (parity) {
    case DefaultParity :
      break;
    case NoParity :
      biosParm &= 0xe7;
      break;
    case OddParity :
      biosParm &= 0xe7;
      biosParm |= 8;
      break;
    case EvenParity :
      biosParm &= 0xe7;
      biosParm |= 0x10;
      break;
    default :
      return FALSE;
  }

  switch (stop) {
    case 0 :
      break;
    case 1 :
      biosParm &= ~4;
      break;
    case 2 :
      biosParm |= 4;
      break;
    default :
      return FALSE;
  }

  if (outputFlow != DefaultFlowControl || inputFlow != DefaultFlowControl)
    return FALSE;

  _bios_serialcom(_COM_INIT, os_handle, biosParm);
  return TRUE;
}

BOOL PSerialChannel::Open(const PString & port, DWORD speed, BYTE data,
       Parity parity, BYTE stop, FlowControl inputFlow, FlowControl outputFlow)
{
  Close();

  os_handle = -1;
  if (PCaselessString("COM") != port.Left(3) &&
                                              port[3] >= '1' && port[3] <= '4')
    return FALSE;
  os_handle = port[3] - '1';
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

}


void PSerialChannel::SetRTS(BOOL state)
{
  if (!IsOpen())
    return;

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

  return (_bios_serialcom(_COM_STATUS, os_handle, 0)&0x8010) == 0x10;
}


BOOL PSerialChannel::GetDSR()
{
  if (!IsOpen())
    return FALSE;

  return (_bios_serialcom(_COM_STATUS, os_handle, 0)&0x8020) == 0x20;
}


BOOL PSerialChannel::GetDCD()
{
  if (!IsOpen())
    return FALSE;

  return (_bios_serialcom(_COM_STATUS, os_handle, 0)&0x8080) == 0x80;
}


BOOL PSerialChannel::GetRing()
{
  if (!IsOpen())
    return FALSE;

  return (_bios_serialcom(_COM_STATUS, os_handle, 0)&0x8040) == 0x40;
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

void PConfig::Construct(Source src)
{
  switch (src) {
    case Application :
      PFilePath appFile = PProcess::Current()->GetFile();
      configFile = appFile.GetVolume() +
                              appFile.GetPath() + appFile.GetTitle() + ".INI";
  }
}


void PConfig::Construct(const PFilePath & file)
{
  configFile = file;
}


PStringList PConfig::GetSections()
{
  PStringList sections;

  if (!configFile.IsEmpty()) {
    PAssertAlways(PUnimplementedFunction);
  }

  return sections;
}


PStringList PConfig::GetKeys(const PString &) const
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
    PAssertAlways(PUnimplementedFunction);
  }

  return keys;
}


void PConfig::DeleteSection(const PString &)
{
  if (configFile.IsEmpty())
    return;

  PAssertAlways(PUnimplementedFunction);
}


void PConfig::DeleteKey(const PString &, const PString & key)
{
  if (configFile.IsEmpty()) {
    PAssert(key.Find('=') == P_MAX_INDEX, PInvalidParameter);
    _putenv(key + "=");
  }
  else
    PAssertAlways(PUnimplementedFunction);
}


PString PConfig::GetString(const PString &,
                                          const PString & key, const PString & dflt)
{
  PString str;

  if (configFile.IsEmpty()) {
    PAssert(key.Find('=') == P_MAX_INDEX, PInvalidParameter);
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


void PConfig::SetString(const PString &, const PString & key, const PString & value)
{
  if (configFile.IsEmpty()) {
    PAssert(key.Find('=') == P_MAX_INDEX, PInvalidParameter);
    _putenv(key + "=" + value);
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
