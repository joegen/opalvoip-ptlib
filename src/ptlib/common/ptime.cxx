/*
 * $Id: ptime.cxx,v 1.14 1996/10/26 01:40:12 robertj Exp $
 *
 * Portable Windows Library
 *
 * Time classes
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ptime.cxx,v $
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

#if defined(_PTIMEINTERVAL)

PTimeInterval::PTimeInterval(long millisecs,
                             long seconds,
                             long minutes,
                             long hours,
                             int days)
{
  milliseconds = millisecs;
  milliseconds += seconds*1000;
  milliseconds += minutes*60000L;
  milliseconds += hours*3600000L;
  milliseconds += PInt64(86400000L)*days;
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

  int decs = strm.precision();
  if (decs > 3)
    decs = 3;

  strm.fill('0');

  BOOL hadPrevious = FALSE;
  long tmp = (long)(ms/86400000);
  if (tmp > 0) {
    strm << tmp << ':';
    hadPrevious = TRUE;
  }

  tmp = (long)(ms%86400000)/3600000;
  if (hadPrevious || tmp > 0) {
    if (hadPrevious)
      strm.width(2);
    strm << tmp << ':';
    hadPrevious = TRUE;
  }

  tmp = (long)(ms%3600000)/60000;
  if (hadPrevious || tmp > 0) {
    if (hadPrevious)
      strm.width(2);
    strm << tmp << ':';
    hadPrevious = TRUE;
  }

  if (hadPrevious)
    strm.width(2);
  strm << (long)(ms%60000)/1000;

  if (decs > 0)
    strm << '.' << setw(decs) << (int)(ms%1000);
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
  millisecs += seconds*1000;
  millisecs += minutes*60000L;
  millisecs += hours*3600000L;
  millisecs += PInt64(86400000L)*days;
  milliseconds = millisecs;
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PTime

#if defined(_PTIME)

static time_t p_mktime(struct tm * t, int zone)
{
  // mktime returns GMT, calculated using input_time - timezone. However, this
  // assumes that the input time was a local time. If the input time wasn't a
  // local time, then we have have to add the local timezone (without daylight
  // savings) and subtract the specified zone offset to get GMT
  // and then subtract
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
  PStringStream s = str;
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

  if (zone == Local)
    t.tm_isdst = IsDaylightSavings() ? 1 : 0;
  else
    t.tm_isdst = 0;

  theTime = p_mktime(&t, zone);
}


PObject::Comparison PTime::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PTime::Class()), PInvalidCast);
  const PTime & other = (const PTime &)obj;
  return theTime < other.theTime ? LessThan :
         theTime > other.theTime ? GreaterThan : EqualTo;
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

  PString str;

  // the localtime call automatically adjusts for daylight savings time
  // so take this into account when converting non-local times
  if (zone == Local) 
    zone = GetTimeZone();  // includes daylight savings time
  time_t realTime = theTime + zone*60;     // to correct timezone
  struct tm * t = gmtime(&realTime);

  PINDEX repeatCount;

  while (*format != '\0') {
    repeatCount = 1;
    switch (*format) {
      case 'a' :
        while (*++format == 'a')
          ;
        if (t->tm_hour < 12)
          str += GetTimeAM();
        else
          str += GetTimePM();
        break;

      case 'h' :
        while (*++format == 'h')
          repeatCount++;
        str += psprintf("%0*u", repeatCount,
                                is12hour ? (t->tm_hour+11)%12+1 : t->tm_hour);
        break;

      case 'm' :
        while (*++format == 'm')
          repeatCount++;
        str += psprintf("%0*u", repeatCount, t->tm_min);
        break;

      case 's' :
        while (*++format == 's')
          repeatCount++;
        str += psprintf("%0*u", repeatCount, t->tm_sec);
        break;

      case 'w' :
        while (*++format == 'w')
          repeatCount++;
        if (repeatCount != 3 || *format != 'e')
          str += GetDayName((Weekdays)t->tm_wday,
                                    repeatCount <= 3 ? Abbreviated : FullName);
        else {
          static const char * const EnglishDayName[] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
          };
          str += EnglishDayName[t->tm_wday];
          format++;
        }
        break;

      case 'M' :
        while (*++format == 'M')
          repeatCount++;
        if (repeatCount < 3)
          str += psprintf("%0*u", repeatCount, t->tm_mon+1);
        else if (repeatCount > 3 || *format != 'E')
          str += GetMonthName((Months)(t->tm_mon+1),
                                    repeatCount == 3 ? Abbreviated : FullName);
        else {
          static const char * const EnglishMonthName[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
          };
          str += EnglishMonthName[t->tm_mon];
          format++;
        }
        break;

      case 'd' :
        while (*++format == 'd')
          repeatCount++;
        str += psprintf("%0*u", repeatCount, t->tm_mday);
        break;

      case 'y' :
        while (*++format == 'y')
          repeatCount++;
        if (repeatCount < 3)
          str += psprintf("%02u", t->tm_year%100);
        else
          str += psprintf("%04u", t->tm_year+1900);
        break;

      case 'z' :
        while (*++format == 'z')
          ;
        str += (zone < 0 ? '-' : '+');
        zone = PABS(zone);
        str += psprintf("%02u%02u", zone/60, zone%60);
        break;

      default :
        str += PString(*format++);
    }
  }

  return str;
}

///////////////////////////////////////////////////////////
//
//  Time parser
//

class yytype {
  public:
    PString sval;
    int     ival;
};

enum {
  IDENTIFIER   = 257,
  INTEGER,
  MONTH,
  DAY,
  AM,
  PM,
  ZONE
};

typedef struct {
  const char * string;
  int          token;
  int          value;
} stringKey;

#define NUM_LIST_ENTRIES  50

static const stringKey idList[NUM_LIST_ENTRIES] = {
  { "am",            AM },
  { "apr",           MONTH, PTime::April   },
  { "april",         MONTH, PTime::April   },
  { "aug",           MONTH, PTime::August  },
  { "august",        MONTH, PTime::August   },
  { "cdt",           ZONE,  -5*60+0 },
  { "cst",           ZONE,  -6*60+0 },
  { "dec",           MONTH, PTime::December },
  { "december",      MONTH, PTime::December   },
  { "edt",           ZONE,  -4*60+0 },
  { "est",           ZONE,  -5*60+0 },
  { "feb",           MONTH, PTime::February },
  { "february",      MONTH, PTime::February   },
  { "fri",           DAY,   PTime::Friday },
  { "friday",        DAY,   PTime::Friday },
  { "gmt",           ZONE,  PTime::GMT },
  { "jan",           MONTH, PTime::January  },
  { "january",       MONTH, PTime::January  },
  { "jul",           MONTH, PTime::July  },
  { "july",          MONTH, PTime::July  },
  { "jun",           MONTH, PTime::June  },
  { "june",          MONTH, PTime::June  },
  { "mar",           MONTH, PTime::March  },
  { "march",         MONTH, PTime::March   },
  { "may",           MONTH, PTime::May  },
  { "may",           MONTH, PTime::May  },
  { "mdt",           ZONE,  -6*60+0 },
  { "mon",           DAY,   PTime::Monday },
  { "monday",        DAY,   PTime::Monday },
  { "mst",           ZONE,  -7*60+0 },
  { "nov",           MONTH, PTime::November  },
  { "november",      MONTH, PTime::November },
  { "oct",           MONTH, PTime::October  },
  { "october",       MONTH, PTime::October  },
  { "pdt",           ZONE,  -7*60+0 },
  { "pm",            PM },
  { "pst",           ZONE,  -8*60+0 },
  { "sat",           DAY,   PTime::Saturday },
  { "saturday",      DAY,   PTime::Saturday },
  { "sep",           MONTH, PTime::September  },
  { "september",     MONTH, PTime::September   },
  { "sun",           DAY,   PTime::Sunday },
  { "sunday",        DAY,   PTime::Sunday },
  { "thu",           DAY,   PTime::Thursday },
  { "thursday",      DAY,   PTime::Thursday },
  { "tue",           DAY,   PTime::Tuesday },
  { "tuesday",       DAY,   PTime::Tuesday },
  { "utc",           ZONE,  PTime::UTC },
  { "wed",           DAY,   PTime::Wednesday },
  { "wednesday",     DAY,   PTime::Wednesday }
};

static int compare(const void * key, const void * str)
{
  PObject::Comparison result = ((PString*)key)->Compare(PString(((stringKey *)str)->string));
  return (result == PObject::EqualTo) ? 0 : (
          (result == PObject::LessThan) ? -1 : 1
         );
}

static int get_token(istream & strm, PString & yytext, yytype & yyval)
{
  for (;;) {
    int ch = strm.get();
    yytext = (char)tolower(ch);

    if (ch < 0)
      return -1;
    else if (isdigit(ch)) {
      yyval.ival = ch - '0';
      while (isdigit(ch = strm.get())) {
        yyval.ival *= 10;
        yyval.ival += ch - '0';
        yytext += (char)ch;
      }
      strm.putback((char)ch);
      return INTEGER;
  
    } else if (isalpha(ch)) {
      while (isalpha(ch = strm.get())) 
        yytext += (char)tolower(ch);
      strm.putback((char)ch);
  
      yyval.sval = yytext;

      if (yytext *= PTime::GetTimeZoneString(PTime::StandardTime)) {
        yyval.ival = PTime::Local;
        return ZONE;
      }

      if (yytext *= PTime::GetTimeZoneString(PTime::DaylightSavings)) {
        yyval.ival = PTime::GetTimeZone(PTime::DaylightSavings);
        return ZONE;
      }

      stringKey * found = (stringKey *)bsearch(&yytext,
                                               &idList,
                                               NUM_LIST_ENTRIES,
                                               sizeof(stringKey),
                                               compare);
      if (found != NULL) {
        yyval.ival = found->value;
        return found->token;
      }
        
      return IDENTIFIER;

    } else if (isspace(ch)) {
      while (isspace(ch = strm.get())) 
        ;
      strm.putback((char)ch);
    } else if (ch == '+' || ch == '-' || ch == ':') 
      return ch;
  }
}

void PTime::ReadFrom(istream &strm)
{
  int token;
  PString yytext;
  yytype  yyval;

  int month = -1;
  int day = -1;
  int year = -1;
  int hours = -1;
  int minutes = -1;
  int seconds = 0;
  int zone = 0;
  BOOL zoneSet = FALSE;
  int dow = -1;

  token = get_token(strm, yytext, yyval);

  int mul, tmp;
  BOOL putback      = TRUE;
  BOOL wantToFinish = FALSE;
  BOOL haveMinimum  = FALSE;
  BOOL finished     = FALSE;

  for (;;) {
    switch (token) {
      case -1:
        finished = TRUE;
        putback = FALSE;
        break;

      case IDENTIFIER :
        finished = TRUE;
        break;

      case DAY:
        if (dow < 0)
          dow = yyval.ival;
        else
          wantToFinish = TRUE;
        token = get_token(strm, yytext, yyval);
        break;

      case MONTH:
        if (month >= 0) {
          wantToFinish = TRUE;
          token = get_token(strm, yytext, yyval);
        } else {
          month = yyval.ival;
          token = get_token(strm, yytext, yyval);
          if (token == '-') {
            token = get_token(strm, yytext, yyval);
            if (token == INTEGER) {
              year = yyval.ival;
              if (year < 100)
                year += 1900;
              token = get_token(strm, yytext, yyval);
            }
          }
        }
        break;

      case INTEGER:
        if ((day > 0) && (hours > 0) && (year > 0)) {
          wantToFinish = TRUE;
          token = get_token(strm, yytext, yyval);
        } else {
          tmp = yyval.ival;
          token = get_token(strm, yytext, yyval);
          if (hours < 0 && token == ':') {
            hours = tmp;
            minutes = 0;
            seconds = 0;
            if ((token = get_token(strm, yytext, yyval)) == INTEGER) {
              minutes = yyval.ival;
              while ((token = get_token(strm, yytext, yyval)) == ':') 
                ;
              if (token == INTEGER) {
                seconds = yyval.ival;
                token = get_token(strm, yytext, yyval);
              }
            }
            if (token == AM || token == PM) {
              if (token == PM && hours <= 12)
                hours += 12;
              token = get_token(strm, yytext, yyval);
            }
          } else if (day <= 0 && token == '-') {
            day = tmp;
            token = get_token(strm, yytext, yyval);
          } else if (day <= 0 && tmp <= 31) 
            day = tmp;
          else if (year < 0) {
            year = tmp;
            if (year < 100)
              year += 1900;
          } else
            wantToFinish = TRUE;
        }
        break;

      case ZONE:
        if (zoneSet) 
          wantToFinish = TRUE;
        else {
          zone    = yyval.ival;
          zoneSet = TRUE;
        }
        token = get_token(strm, yytext, yyval);
        break;

      case '+':
      case '-':
        if (zoneSet) {
          wantToFinish = TRUE;
          token = get_token(strm, yytext, yyval);
        } else {
          mul = (token == '-') ? -1 : +1;
          token = get_token(strm, yytext, yyval);
          if (token == INTEGER) {
            if (yytext.GetLength() > 2) 
              zone = mul * ((60 * yyval.ival / 100) + (yyval.ival % 100));
            else {
              zone = mul * 60 * yyval.ival;
              token = get_token(strm, yytext, yyval);
              if (token == ':') {
                token = get_token(strm, yytext, yyval);
                if (token == INTEGER) 
                  zone += mul * yyval.ival;
              }
            }
            zoneSet = TRUE;
          }
        }
        break;

      default:
        wantToFinish = TRUE;
        break;
    }

    haveMinimum = (day > 0 && month > 0) || (hours >= 0 && minutes >= 0);

    if (finished)
      break;
    else if (haveMinimum && wantToFinish)
      break;
    else if (haveMinimum && day >= 0 && year >= 0 && zoneSet)
      break;
  }

  // put back the last lexeme
  if (putback) {
    PINDEX i = yytext.GetLength();
    do 
      strm.putback(yytext[--i]);
    while (i != 0);
  }

  // fill in the missing bits
  PTime now;
  if (year < 0) 
    year = now.GetYear();
  if (month < 0)
    month = now.GetMonth();
  if (day < 0)
    day = now.GetDay();

  
  if (hours < 0)
    hours = 0;
  if (minutes < 0)
    minutes = 0;
  if (seconds < 0)
    seconds = 0;

  // if no zone was set, then use the current local zone
  if (!zoneSet) 
    zone = Local;

  struct tm t;
  t.tm_sec   = seconds;
  t.tm_min   = minutes;
  t.tm_hour  = hours;
  t.tm_mday  = day;
  t.tm_mon   = month-1;
  t.tm_year  = year-1900;
  if (zone == Local)
    t.tm_isdst = IsDaylightSavings() ? 1 : 0;
  else
    t.tm_isdst = 0;

  theTime = p_mktime(&t, zone);
}


#endif


// End Of File ///////////////////////////////////////////////////////////////
