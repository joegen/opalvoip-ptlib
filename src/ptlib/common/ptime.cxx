/*
 * ptime.cxx
 *
 * Time and date classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: ptime.cxx,v $
 * Revision 1.34  2000/04/29 04:50:16  robertj
 * Added microseconds to string output.
 * Fixed uninitialised microseconds on some constructors.
 *
 * Revision 1.33  2000/04/28 13:23:25  robertj
 * Fixed printing of negative time intervals.
 *
 * Revision 1.32  2000/04/18 06:01:01  robertj
 * Fixed integer overflow bug in PTime addition functions, thanks Ian MacDonald
 *
 * Revision 1.31  2000/04/05 02:50:17  robertj
 * Added microseconds to PTime class.
 *
 * Revision 1.30  2000/03/08 17:56:33  rogerh
 * Fix error in creation of a PStringStream from a PString
 *
 * Revision 1.29  2000/03/06 04:09:23  robertj
 * Added constructor to do PString conversion to PTimeInterval
 *
 * Revision 1.28  1999/10/14 08:23:20  robertj
 * Fixed display of decimals in milliseconds when precision other than 3.
 *
 * Revision 1.27  1999/08/08 12:39:24  robertj
 * Fixed bug in display of PTimeIntervals larger than 24 hours.
 *
 * Revision 1.26  1999/06/14 07:58:39  robertj
 * Fixed bug in PTimeInteerval output, left stream fill char as '0'.
 * Added ability to set the width of the PTimeInterval stream output.
 *
 * Revision 1.25  1999/03/29 03:38:58  robertj
 * Improved time & date parser.
 *
 * Revision 1.24  1998/11/30 12:31:46  robertj
 * Removed redundent conditionals.
 *
 * Revision 1.23  1998/11/14 01:11:45  robertj
 * PPC linux GNU compatibility.
 *
 * Revision 1.22  1998/09/23 06:22:36  robertj
 * Added open source copyright license.
 *
 * Revision 1.21  1998/03/05 12:49:53  robertj
 * MemCheck fixes.
 *
 * Revision 1.20  1998/01/26 00:48:30  robertj
 * Removed days from PTimeInterval PrintOn(), can get it back by using negative precision.
 * Fixed Y2K problem in parsing dates.
 *
 * Revision 1.19  1998/01/04 07:24:32  robertj
 * Added operating system specific versions of gmtime and localtime.
 *
 * Revision 1.18  1997/07/08 13:05:21  robertj
 * Fixed bug in parsing time zone incorrectly.
 *
 * Revision 1.17  1997/05/16 12:05:57  robertj
 * Changed PTimeInterval to guarentee no overflow in millisecond calculations.
 *
 * Revision 1.16  1997/03/18 21:24:19  robertj
 * Fixed parsing of time putting back token after time zone.
 *
 * Revision 1.15  1997/01/03 04:40:03  robertj
 * Changed default time so if no year goes to last 12 months rather than current year.
 *
 * Revision 1.14  1996/10/26 01:40:12  robertj
 * Fixed bug in time parser that caused endless looping.
 *
 * Revision 1.13  1996/08/20 12:07:29  robertj
 * Fixed volatile milliseconds member of PTimeInterval for printing.
 *
 * Revision 1.12  1996/07/27 04:11:28  robertj
 * Added bullet proofing for invlid times - especially in Western time zones.
 *
 * Revision 1.11  1996/07/15 12:43:01  robertj
 * Fixed MSVC 4.1 compiler bug.
 *
 * Revision 1.10  1996/06/17 11:34:48  robertj
 * Fixed bug in NOT localising RFC1123 time.
 *
 * Revision 1.9  1996/05/09 12:18:16  robertj
 * Fixed syntax error found by Mac platform.
 * Resolved C++ problems with 64 bit PTimeInterval for Mac platform.
 *
 * Revision 1.8  1996/03/17 05:43:42  robertj
 * Changed PTimeInterval to 64 bit integer.
 *
 * Revision 1.7  1996/03/05 14:09:20  robertj
 * Fixed bugs in PTimerInterval stream output.
 *
 * Revision 1.6  1996/03/04 12:21:42  robertj
 * Fixed output of leading zeros in PTimeInterval stream output.
 *
 * Revision 1.5  1996/02/25 11:22:13  robertj
 * Added check for precision field in stream when outputting time interval..
 *
 * Revision 1.4  1996/02/25 03:07:47  robertj
 * Changed PrintOn and ReadFrom on PTimeInterval to use dd:hh:mm:ss.mmm format.
 *
 * Revision 1.3  1996/02/15 14:47:35  robertj
 * Fixed bugs in time zone compensation (some in the C library).
 *
 * Revision 1.2  1996/02/13 12:59:32  robertj
 * Changed GetTimeZone() so can specify standard/daylight time.
 * Split PTime into separate module after major change to ReadFrom().
 *
 * Revision 1.1  1996/02/13 10:11:52  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <ctype.h>

///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

PTimeInterval::PTimeInterval(long millisecs,
                             long seconds,
                             long minutes,
                             long hours,
                             int days)
{
  SetInterval(millisecs, seconds, minutes, hours, days);
}


PTimeInterval::PTimeInterval(const PString & str)
{
  PStringStream strm(str);
  ReadFrom(strm);
}


PObject::Comparison PTimeInterval::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PTimeInterval::Class()), PInvalidCast);
  const PTimeInterval & other = (const PTimeInterval &)obj;
  return milliseconds < other.milliseconds ? LessThan :
         milliseconds > other.milliseconds ? GreaterThan : EqualTo;
}


void PTimeInterval::PrintOn(ostream & strm) const
{
  PInt64 ms = milliseconds;

  if (ms < 0) {
    strm << '-';
    ms = -ms;
  }

  char prevFill = strm.fill();
  strm.fill('0');

  int precision = strm.precision();
  if (precision > 3)
    precision = 3;

  int width = strm.width();
  strm.width(1);

  BOOL hadPrevious = FALSE;
  long tmp;

  if (precision < 0) {
    precision = -precision;
    if (precision > 3)
      precision = 3;

    tmp = (long)(ms/86400000);
    if (tmp > 0 || width > (precision+9)) {
      strm << tmp << ':';
      hadPrevious = TRUE;
    }

    tmp = (long)(ms%86400000)/3600000;
  }
  else
    tmp = (long)(ms/3600000);

  if (hadPrevious || tmp > 0 || width > (precision+6)) {
    if (hadPrevious)
      strm.width(2);
    strm << tmp << ':';
    hadPrevious = TRUE;
  }

  tmp = (long)(ms%3600000)/60000;
  if (hadPrevious || tmp > 0 || width > (precision+3)) {
    if (hadPrevious)
      strm.width(2);
    strm << tmp << ':';
    hadPrevious = TRUE;
  }

  if (hadPrevious)
    strm.width(2);
  strm << (long)(ms%60000)/1000;

  switch (precision) {
    case 1 :
      strm << '.' << (int)(ms%1000)/100;
      break;

    case 2 :
      strm << '.' << setw(2) << (int)(ms%1000)/10;
      break;

    case 3 :
      strm << '.' << setw(3) << (int)(ms%1000);

    default :
      break;
  }

  strm.fill(prevFill);
}


void PTimeInterval::ReadFrom(istream &strm)
{
  long day = 0;
  long hour = 0;
  long min = 0;
  float sec;
  strm >> sec;
  while (strm.peek() == ':') {
    day = hour;
    hour = min;
    min = (long)sec;
    strm.get();
    strm >> sec;
  }

  SetInterval(((long)(sec*1000))%1000, (int)sec, min, hour, day);
}


void PTimeInterval::SetInterval(PInt64 millisecs,
                                long seconds,
                                long minutes,
                                long hours,
                                int days)
{
  milliseconds = days;
  milliseconds *= 24;
  milliseconds += hours;
  milliseconds *= 60;
  milliseconds += minutes;
  milliseconds *= 60;
  milliseconds += seconds;
  milliseconds *= 1000;
  milliseconds += millisecs;
}


///////////////////////////////////////////////////////////////////////////////
// PTime

static time_t p_mktime(struct tm * t, int zone)
{
  // mktime returns GMT, calculated using input_time - timezone. However, this
  // assumes that the input time was a local time. If the input time wasn't a
  // local time, then we have have to add the local timezone (without daylight
  // savings) and subtract the specified zone offset to get GMT
  // and then subtract
  t->tm_isdst = PTime::IsDaylightSavings() ? 1 : 0;
  time_t theTime = mktime(t);
  if (theTime == (time_t)-1)
    theTime = 0;
  else if (zone != PTime::Local) {
    theTime += PTime::GetTimeZone()*60;  // convert to local time
    if (theTime > zone*60)
      theTime -= zone*60;           // and then to GMT
  }
  return theTime;
}


PTime::PTime(const PString & str)
{
  PStringStream s(str);
  ReadFrom(s);
}


PTime::PTime(int second, int minute, int hour,
             int day,    int month,  int year,
             int zone)
{
  microseconds = 0;

  struct tm t;
  PAssert(second >= 0 && second <= 59, PInvalidParameter);
  t.tm_sec = second;
  PAssert(minute >= 0 && minute <= 59, PInvalidParameter);
  t.tm_min = minute;
  PAssert(hour >= 0 && hour <= 23, PInvalidParameter);
  t.tm_hour = hour;
  PAssert(day >= 1 && day <= 31, PInvalidParameter);
  t.tm_mday = day;
  PAssert(month >= 1 && month <= 12, PInvalidParameter);
  t.tm_mon = month-1;
  PAssert(year >= 1970 && year <= 2038, PInvalidParameter);
  t.tm_year   = year-1900;

  theTime = p_mktime(&t, zone);
}


PObject::Comparison PTime::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PTime::Class()), PInvalidCast);
  const PTime & other = (const PTime &)obj;
  if (theTime < other.theTime)
    return LessThan;
  if (theTime > other.theTime)
    return GreaterThan;
  if (microseconds < other.microseconds)
    return LessThan;
  if (microseconds > other.microseconds)
    return GreaterThan;
  return EqualTo;
}




PString PTime::AsString(TimeFormat format, int zone) const
{
  if (format == RFC1123)
    return AsString("wwwe, dd MMME yyyy hh:mm:ss z", zone);

  PString fmt, dsep;

  PString tsep = GetTimeSeparator();
  BOOL is12hour = GetTimeAMPM();

  switch (format & 7) {
    case LongDateTime :
    case LongTime :
    case MediumDateTime :
    case ShortDateTime :
    case ShortTime :
      if (!is12hour)
        fmt = "h";

      fmt += "h" + tsep + "mm";

      switch (format) {
        case LongDateTime :
        case LongTime :
          fmt += tsep + "ss";

        default :
          break;
      }

      if (is12hour)
        fmt += "a";
      break;

    default :
      break;
  }

  switch (format & 7) {
    case LongDateTime :
    case MediumDateTime :
    case ShortDateTime :
      fmt += ' ';
      break;

    default :
      break;
  }

  switch (format & 7) {
    case LongDateTime :
    case LongDate :
      fmt += "wwww ";
      switch (GetDateOrder()) {
        case MonthDayYear :
          fmt += "MMMM d, yyyy";
          break;
        case DayMonthYear :
          fmt += "d MMMM yyyy";
          break;
        case YearMonthDay :
          fmt += "yyyy MMMM d";
      }
      break;

    case MediumDateTime :
    case MediumDate :
      fmt += "www ";
      switch (GetDateOrder()) {
        case MonthDayYear :
          fmt += "MMM d, yy";
          break;
        case DayMonthYear :
          fmt += "d MMM yy";
          break;
        case YearMonthDay :
          fmt += "yy MMM d";
      }
      break;

    case ShortDateTime :
    case ShortDate :
      dsep = GetDateSeparator();
      switch (GetDateOrder()) {
        case MonthDayYear :
          fmt += "MM" + dsep + "dd" + dsep + "yy";
          break;
        case DayMonthYear :
          fmt += "dd" + dsep + "MM" + dsep + "yy";
          break;
        case YearMonthDay :
          fmt += "yy" + dsep + "MM" + dsep + "dd";
      }
      break;

    default :
      break;
  }

  if (zone != Local)
    fmt += " z";

  return AsString(fmt, zone);
}


PString PTime::AsString(const char * format, int zone) const
{
  PAssert(format != NULL, PInvalidParameter);

  BOOL is12hour = strchr(format, 'a') != NULL;

  PStringStream str;
  str.fill('0');

  // the localtime call automatically adjusts for daylight savings time
  // so take this into account when converting non-local times
  if (zone == Local)
    zone = GetTimeZone();  // includes daylight savings time
  time_t realTime = theTime + zone*60;     // to correct timezone
  struct tm ts;
  struct tm * t = os_gmtime(&realTime, &ts);

  PINDEX repeatCount;

  while (*format != '\0') {
    repeatCount = 1;
    switch (*format) {
      case 'a' :
        while (*++format == 'a')
          ;
        if (t->tm_hour < 12)
          str << GetTimeAM();
        else
          str << GetTimePM();
        break;

      case 'h' :
        while (*++format == 'h')
          repeatCount++;
        str << setw(repeatCount) << (is12hour ? (t->tm_hour+11)%12+1 : t->tm_hour);
        break;

      case 'm' :
        while (*++format == 'm')
          repeatCount++;
        str << setw(repeatCount) << t->tm_min;
        break;

      case 's' :
        while (*++format == 's')
          repeatCount++;
        str << setw(repeatCount) << t->tm_sec;
        break;

      case 'w' :
        while (*++format == 'w')
          repeatCount++;
        if (repeatCount != 3 || *format != 'e')
          str << GetDayName((Weekdays)t->tm_wday, repeatCount <= 3 ? Abbreviated : FullName);
        else {
          static const char * const EnglishDayName[] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
          };
          str << EnglishDayName[t->tm_wday];
          format++;
        }
        break;

      case 'M' :
        while (*++format == 'M')
          repeatCount++;
        if (repeatCount < 3)
          str << setw(repeatCount) << (t->tm_mon+1);
        else if (repeatCount > 3 || *format != 'E')
          str << GetMonthName((Months)(t->tm_mon+1),
                                    repeatCount == 3 ? Abbreviated : FullName);
        else {
          static const char * const EnglishMonthName[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
          };
          str << EnglishMonthName[t->tm_mon];
          format++;
        }
        break;

      case 'd' :
        while (*++format == 'd')
          repeatCount++;
        str << setw(repeatCount) << t->tm_mday;
        break;

      case 'y' :
        while (*++format == 'y')
          repeatCount++;
        if (repeatCount < 3)
          str << setw(2) << (t->tm_year%100);
        else
          str << setw(4) << (t->tm_year+1900);
        break;

      case 'z' :
        while (*++format == 'z')
          ;
        if (zone == 0)
          str << "GMT";
        else {
          str << (zone < 0 ? '-' : '+');
          zone = PABS(zone);
          str << setw(2) << (zone/60) << setw(2) << (zone%60);
        }
        break;

      case 'u' :
        while (*++format == 'u')
          repeatCount++;
        switch (repeatCount) {
          case 1 :
            str << ((microseconds+50000)/100000);
            break;
          case 2 :
            str << setw(2) << ((microseconds+5000)/10000);
            break;
          case 3 :
            str << setw(3) << ((microseconds+500)/1000);
            break;
          default :
            str << setw(6) << microseconds;
            break;
        }
        break;

      default :
        str << *format++;
    }
  }

  return str;
}

///////////////////////////////////////////////////////////
//
//  Time parser
//

extern "C" {

#ifndef STDAPICALLTYPE
#define STDAPICALLTYPE
#endif

time_t STDAPICALLTYPE PTimeParse(void *, struct tm *, int);

int STDAPICALLTYPE PTimeGetChar(void * stream)
{
  return ((istream *)stream)->get();
}


void STDAPICALLTYPE PTimeUngetChar(void * stream, int c)
{
  ((istream *)stream)->putback((char)c);
}


int STDAPICALLTYPE PTimeGetDateOrder()
{
  return PTime::GetDateOrder();
}


int STDAPICALLTYPE PTimeIsMonthName(const char * str, int month, int abbrev)
{
  return PTime::GetMonthName((PTime::Months)month,
                             abbrev ? PTime::Abbreviated : PTime::FullName) *= str;
}


int STDAPICALLTYPE PTimeIsDayName(const char * str, int day, int abbrev)
{
  return PTime::GetDayName((PTime::Weekdays)day,
                             abbrev ? PTime::Abbreviated : PTime::FullName) *= str;
}


};



void PTime::ReadFrom(istream & strm)
{
  time_t now;
  struct tm timeBuf;
  time(&now);
  microseconds = 0;
  theTime = PTimeParse(&strm, os_localtime(&now, &timeBuf), GetTimeZone());
}



PTime PTime::operator+(const PTimeInterval & t) const
{
  PInt64 msecs = (PInt64)theTime*1000 + t.GetMilliSeconds();
  return PTime((time_t)(msecs/1000), (long)(msecs%1000)*1000);
}


PTime & PTime::operator+=(const PTimeInterval & t)
{
  PInt64 msecs = (PInt64)theTime*1000 + t.GetMilliSeconds();
  theTime = (time_t)(msecs/1000);
  microseconds = (long)(msecs%1000)*1000;
  return *this;
}


PTimeInterval PTime::operator-(const PTime & t) const
{
  time_t secs = theTime - t.theTime;
  long usecs = microseconds - t.microseconds;
  if (usecs < 0) {
    usecs += 1000000;
    secs--;
  }
  else if (usecs >= 1000000) {
    usecs -= 1000000;
    secs++;
  }
  return PTimeInterval(usecs/1000, secs);
}


PTime PTime::operator-(const PTimeInterval & t) const
{
  time_t secs = theTime - t.GetSeconds();
  long msecs = (long)(microseconds/1000 - t.GetMilliSeconds()%1000);
  if (msecs < 0) {
    msecs += 1000;
    secs--;
  }
  else if (msecs >= 1000) {
    msecs -= 1000;
    secs++;
  }
  return PTime(secs, msecs*1000);
}


PTime & PTime::operator-=(const PTimeInterval & t)
{
  theTime -= t.GetSeconds();
  microseconds -= (long)(t.GetMilliSeconds()%1000)*1000;
  if (microseconds < 0) {
    microseconds += 1000000;
    theTime--;
  }
  else if (microseconds >= 1000000) {
    microseconds -= 1000000;
    theTime++;
  }
  return *this;
}


// End Of File ///////////////////////////////////////////////////////////////
