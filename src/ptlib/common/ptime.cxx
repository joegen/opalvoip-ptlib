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
 */

#include <ptlib.h>
#include <ctype.h>

///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

PTimeInterval::PTimeInterval(PInt64 millisecs,
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
  int64_t otherNano = dynamic_cast<const PTimeInterval &>(obj).GetNanoSeconds();
  int64_t thisNano = GetNanoSeconds();
  return thisNano < otherNano ? LessThan : thisNano > otherNano ? GreaterThan : EqualTo;
}


void PTimeInterval::PrintOn(ostream & strm) const
{
  PInt64 ns = GetNanoSeconds();

  std::streamsize decimals = strm.precision();

  if ((strm.flags()&ios::floatfield) == ios::scientific) {
    if ((strm.flags()&ios::showbase) != 0 && ns < SecsToNano)
      strm << PString(PString::ScaleSI, ns/(double)SecsToNano, decimals);
    else {
      unsigned oldMode = strm.flags()&ios::floatfield;
      strm << fixed << ns/(double)SecsToNano;
      strm.setf(oldMode, ios::floatfield);
    }
    return;
  }

  bool includeDays;
  if (decimals > 0)
    includeDays = (strm.flags()&ios::showbase) != 0;
  else {
    includeDays = true;
    decimals = -decimals;
  }

  if (decimals > 9)
    decimals = 9; // Can't be more than nanoseconds
  std::streamsize decimalsWidth = decimals > 0 ? decimals+1 : 0;

  std::streamsize width = strm.width();
  std::streamsize nextFieldWidth = 0;

  char oldFill = strm.fill();
  unsigned adjustMode = strm.flags()&ios_base::adjustfield;
  strm << right;

  if (includeDays && (ns > DaysToNano || width > (decimalsWidth + 9))) {
    strm << setw(width-decimalsWidth-9) << ns/DaysToNano << 'd' << setfill('0');
    if (ns < 0)
      ns = -ns;
    ns = ns % DaysToNano;
    nextFieldWidth = 2;
  }

  if (nextFieldWidth > 0 || ns >= HoursToNano || width > (decimalsWidth + 6)) {
    strm << setw(nextFieldWidth > 0 ? nextFieldWidth : (width-decimalsWidth-6)) << ns/HoursToNano << ':' << setfill('0');
    nextFieldWidth = 2;
  }

  if (ns < 0)
    ns = -ns;
  ns = ns % HoursToNano;
  if (nextFieldWidth > 0 || ns >= MinsToNano || width > (decimalsWidth + 3)) {
    strm << setw(nextFieldWidth > 0 ? nextFieldWidth : (width-decimalsWidth-3)) << ns/MinsToNano << ':' << setfill('0');
    nextFieldWidth = 2;
  }

  if (ns < 0)
    ns = -ns;
  strm << setw(nextFieldWidth > 0 ? nextFieldWidth : (width-decimalsWidth)) << (ns % MinsToNano)/SecsToNano;

  ns = ns%SecsToNano;

  if (decimals > 0) {
    int64_t powerOfTen = SecsToNano;
    for (int i = 0; i < decimals; ++i)
      powerOfTen /= 10;
    strm << '.' << setw(decimals) << std::min((ns+powerOfTen/2)/powerOfTen, SecsToNano/powerOfTen-1);
  }

  strm.setf(adjustMode, ios::adjustfield);
  strm.fill(oldFill);
}


void PTimeInterval::ReadFrom(istream &strm)
{
  long day = 0;
  long hour = 0;
  long min = 0;
  double sec;
  strm >> sec;
  while (strm.good() && strm.peek() == ':') {
    day = hour;
    hour = min;
    min = (long)sec;
    strm.get();
    strm >> sec;
  }

  InternalSet((int64_t)((((day*24 + hour)*60 + min)*60 + sec)*1000000000));
}


PString PTimeInterval::AsString(int precision, Formats format, int width) const
{
  PStringStream str;
  switch (format) {
    default :
      str << fixed;
      break;
    case SecondsSI :
      str << scientific << showbase;
      break;
    case SecondsOnly :
      str << scientific << noshowbase;
      break;
  }
  str << setprecision(format == IncludeDays ? -precision : precision) << setw(width) << *this;
  return str;
}


#ifdef _WIN32
static const DWORD TimeIntervalLimit = UINT_MAX;
static const DWORD TimeIntervalInfinite = INFINITE;
#else
static const int TimeIntervalLimit = INT_MAX;
static const int TimeIntervalInfinite = -1;
#endif

PTimeInterval::IntervalType PTimeInterval::GetInterval() const
{
  if (*this == PMaxTimeInterval)
    return TimeIntervalInfinite;

  PInt64 msecs = GetMilliSeconds();
  if (msecs <= 0)
    return 0;

  if (msecs >= TimeIntervalLimit)
    return TimeIntervalLimit;

  return (IntervalType)msecs;
}


void PTimeInterval::SetInterval(PInt64 millisecs,
                                long seconds,
                                long minutes,
                                long hours,
                                int days)
{
  PInt64 milliseconds;

  milliseconds = days;
  milliseconds *= 24;
  milliseconds += hours;
  milliseconds *= 60;
  milliseconds += minutes;
  milliseconds *= 60;
  milliseconds += seconds;
  milliseconds *= 1000;
  milliseconds += millisecs;

  SetMilliSeconds(milliseconds);
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
    if (theTime > (time_t) zone*60)
      theTime -= zone*60;           // and then to GMT
  }
  return theTime;
}


PTime::PTime(time_t tsecs, int64_t usecs)
  : m_microSecondsSinceEpoch(tsecs*Micro+usecs)
{
}


PTime::PTime(const PString & str)
  : m_microSecondsSinceEpoch(0)
{
  PStringStream s(str);
  ReadFrom(s);
}


PTime::PTime(int second, int minute, int hour,
             int day,    int month,  int year,
             int zone)
{
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

  m_microSecondsSinceEpoch.store(p_mktime(&t, zone)*Micro);
}


PObject::Comparison PTime::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PTime), PInvalidCast);
  const PTime & other = (const PTime &)obj;

  int64_t myMicroSeconds = m_microSecondsSinceEpoch.load();
  int64_t otherMicroSeconds = other.m_microSecondsSinceEpoch.load();
  if (myMicroSeconds < otherMicroSeconds)
    return LessThan;
  if (myMicroSeconds > otherMicroSeconds)
    return GreaterThan;

  return EqualTo;
}


static PUInt64 const SecondsFrom1900to1970 = (70*365+17)*24*60*60U;
static PUInt64 const MicrosecondsToFraction = 4294;

void PTime::SetNTP(PUInt64 ntp)
{
  m_microSecondsSinceEpoch.store(((ntp>>32) - SecondsFrom1900to1970)*Micro + (ntp&0xffffffff)/MicrosecondsToFraction);
}


PUInt64 PTime::GetNTP() const
{
  if (!IsValid())
    return 0;
  int64_t usecs = m_microSecondsSinceEpoch.load();
  return ((usecs/Micro+SecondsFrom1900to1970)<<32) | ((usecs%Micro)*MicrosecondsToFraction);
}


bool PTime::InternalLocalTime(struct tm & ts) const
{
  if (!IsValid())
    return false;

  time_t t = m_microSecondsSinceEpoch.load()/Micro;
  return os_localtime(&t, &ts) != NULL;
}


int PTime::GetSecond() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? ts.tm_sec : -1;
}


int PTime::GetMinute() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? ts.tm_min : -1;
}


int PTime::GetHour() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? ts.tm_hour : -1;
}


int PTime::GetDay() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? ts.tm_mday : -1;
}


PTime::Months PTime::GetMonth() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? (Months)(ts.tm_mon+January) : InvalidMonth;
}


int PTime::GetYear() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? ts.tm_year+1900 : -1;
}


PTime::Weekdays PTime::GetDayOfWeek() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? (Weekdays)ts.tm_wday : InvalidWeekday;
}

int PTime::GetDayOfYear() const
{
  struct tm ts;
  return InternalLocalTime(ts) ? ts.tm_yday : -1;
}


void PTime::PrintOn(ostream & strm) const
{
  int fmt = (int)strm.precision();
  if (fmt >= 0 || fmt <= -NumTimeStrings)
    strm << AsString();
  else
    strm << AsString((NumTimeStrings)-fmt);
}


PString PTime::AsString(TimeFormat format, int zone) const
{ 
  if (format >= NumTimeStrings)
    return "Invalid format : " + AsString("yyyy-MM-dd T hh:mm:ss Z");

  if (!IsValid())
    return "N/A";

  switch (format) {
    case RFC1123 :
      return AsString("wwwe, dd MMME yyyy hh:mm:ss z", zone);

    case RFC3339 :
      return AsString("yyyy-MM-ddThh:mm:ssZZ", zone);

    case ShortISO8601 :
      return AsString("yyyyMMddThhmmssZ", zone);

    case LongISO8601 :
      return AsString("yyyy-MM-ddThh:mm:ss.uuuZ", zone);

    case EpochTime:
    {
      int64_t usecs = m_microSecondsSinceEpoch.load();
      return psprintf("%u.%06lu", usecs / Micro, usecs % Micro);
    }

    case TodayFormat:
    {
      PTime now;
      static const PTimeInterval halfDay(0, 0, 0, 12);
      if (*this > (now - halfDay) && *this < (now + halfDay))
        return AsString("hh:mm:ss.uuu", zone);
      // Do next case
    }

    case LoggingFormat :
      return AsString("yyyy/MM/dd hh:mm:ss.uuu", zone);

    default:
      break;
  }

  PString fmt, dsep;

  PString tsep = GetTimeSeparator();
  PBoolean is12hour = GetTimeAMPM();

  switch (format ) {
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

  switch (format ) {
    case LongDateTime :
    case MediumDateTime :
    case ShortDateTime :
      fmt += ' ';
      break;

    default :
      break;
  }

  switch (format ) {
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
  PAssert(zone == Local || std::abs(zone) <= 13, PInvalidParameter);

  if (!IsValid())
    return "<invalid>";

  // the localtime call automatically adjusts for daylight savings time
  // so take this into account when converting non-local times
  if (zone == Local)
    zone = GetTimeZone();  // includes daylight savings time
  time_t realTime = m_microSecondsSinceEpoch.load()/Micro + zone*60;     // to correct timezone
  struct tm ts;
  struct tm * t = os_gmtime(&realTime, &ts);
  if (t == NULL)
    return "<error>";

  PStringStream str;
  str.fill('0');

  bool is12hour = strchr(format, 'a') != NULL;

  while (*format != '\0') {
    char formatLetter = *format;
    PINDEX repeatCount = 1;
    while (*++format == formatLetter)
      repeatCount++;

    switch (formatLetter) {
      case 'a' :
        if (t->tm_hour < 12)
          str << GetTimeAM();
        else
          str << GetTimePM();
        break;

      case 'h' :
        str << setw(repeatCount) << (is12hour ? (t->tm_hour+11)%12+1 : t->tm_hour);
        break;

      case 'm' :
        str << setw(repeatCount) << t->tm_min;
        break;

      case 's' :
        str << setw(repeatCount) << t->tm_sec;
        break;

      case 'w' :
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
        str << setw(repeatCount) << t->tm_mday;
        break;

      case 'y' :
        if (repeatCount < 3)
          str << setw(2) << (t->tm_year%100);
        else
          str << setw(4) << (t->tm_year+1900);
        break;

      case 'z' :
      case 'Z' :
        if (zone == 0 && formatLetter == 'Z')
          str << 'Z';
        else if (zone == 0 && repeatCount == 1)
          str << "GMT";
        else {
          str << (zone < 0 ? '-' : '+');
          zone = PABS(zone);
          str << setw(2) << (zone/60);
          if (repeatCount > 0)
            str << ':';
          str << setw(2) << (zone%60);
        }
        break;

      case 'u':
      {
        unsigned usecs = m_microSecondsSinceEpoch.load()%Micro;
        switch (repeatCount) {
          case 1:
            str << (usecs / 100000);
            break;
          case 2:
            str << setw(2) << (usecs / 10000);
            break;
          case 3:
            str << setw(3) << (usecs / 1000);
            break;
          default:
            str << setw(6) << usecs;
            break;
        }
        break;
      }

      case '\\' :
        // Escaped character, put straight through to output string

      default :
        str << formatLetter;
    }
  }

  return str;
}

///////////////////////////////////////////////////////////
//
//  Time parser
//

extern "C" {

#ifdef _WIN32
#  ifndef STDAPICALLTYPE
#  define STDAPICALLTYPE __stdcall
#  endif
#else
#  define STDAPICALLTYPE
#endif

// This funcctions implementation is in getdate_tab.c
// Which in turn is generated from getdate.y by yacc/bison
extern time_t STDAPICALLTYPE PTimeParse(void *, struct tm *, int);


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
  strm >> ws;
  m_microSecondsSinceEpoch.store(PTimeParse(&strm, os_localtime(&now, &timeBuf), GetTimeZone(StandardTime))*Micro);
}


bool PTime::Parse(const PString & str)
{
  PStringStream strm(str);
  ReadFrom(strm);
  return IsValid();
}


//////////////////////////////////////////////////////////////////////////////
// P_timeval

P_timeval::P_timeval(long secs, long usecs)
  : m_infinite(false)
{
  m_timeval.tv_sec = secs;
  m_timeval.tv_usec = usecs;
}


P_timeval & P_timeval::operator=(const PTimeInterval & time)
{
  m_infinite = time == PMaxTimeInterval;
  if (m_infinite)
    m_timeval.tv_sec = m_timeval.tv_usec = -1;
  else {
    m_timeval.tv_sec = time.GetSeconds();
    m_timeval.tv_usec = (long)(time.GetMicroSeconds() % 1000000);
  }
  return *this;
}


P_timeval & P_timeval::operator=(const PTime & time)
{
  m_infinite = !time.IsValid();
  if (m_infinite)
    m_timeval.tv_sec = m_timeval.tv_usec = -1;
  else {
    m_timeval.tv_sec = (long)time.GetTimeInSeconds();
    m_timeval.tv_usec = time.GetMicrosecond();
  }
  return *this;
}


// End Of File ///////////////////////////////////////////////////////////////
