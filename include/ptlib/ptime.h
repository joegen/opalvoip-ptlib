/*
 * $Id: ptime.h,v 1.8 1994/07/27 05:58:07 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ptime.h,v $
 * Revision 1.8  1994/07/27 05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.7  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.6  1994/01/13  03:16:09  robertj
 * Added function to return time as a string.
 *
 * Revision 1.5  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.4  1993/12/31  06:45:38  robertj
 * Made inlines optional for debugging purposes.
 *
 * Revision 1.3  1993/08/27  18:17:47  robertj
 * Made time functions common to all platforms.
 * Moved timer resolution function to PTimeInterval wher it belongs.
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#define _PTIME

#include <time.h>


///////////////////////////////////////////////////////////////////////////////
// System time and date class

class PTimeInterval;


PDECLARE_CLASS(PTime, PObject)

  public:
    PINLINE PTime(time_t t = time(NULL));
    PTime(int second, int minute, int hour, int day, int month, int year);
    PTime(const PTime & t);
    PTime & operator=(const PTime & t);
      // Create time objects from various sources.


    // Overrides from class PObject
    PObject * Clone() const;
    virtual Comparison Compare(const PObject & obj) const;
    virtual ostream & PrintOn(ostream & strm) const;
    virtual istream & ReadFrom(istream & strm);
      // I/O of time objects


    // New member functions
    int GetSecond() const;
      // Get the seconds: 0..59

    int GetMinute() const;
      // Get the minutes: 0..59

    int GetHour() const;
      // Get the hour: 0..23

    int GetDay() const;
      // Get the day of the month: 1..31

    enum Months {
      January = 1, February, March, April, May, June,
      July, August, September, October, November, December
    };
    Months GetMonth() const;
      // Get the month: 1..12

    int GetYear() const;
      // Get the year: 1970..2100

    enum Weekdays {
      Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday
    };
    Weekdays GetDayOfWeek() const;
      // Get the day of the week: 0=Sun, 1=Mon, ..., 6=Sat

    int GetDayOfYear() const;
      // Get the day in the year: 0..365

    BOOL IsDaylightSavings() const;
      // Get flag indicating daytlight savings is current


    PTime operator+(const PTimeInterval & t) const;
    PTime & operator+=(const PTimeInterval & t);
    PTimeInterval operator-(const PTime & t) const;
    PTime operator-(const PTimeInterval & t) const;
    PTime & operator-=(const PTimeInterval & t);
      // Arithmetic on times

    enum TimeFormat {
      LongDateTime,
      LongDate,
      LongTime,
      MediumDateTime,
      MediumDate,
      ShortDateTime,
      ShortDate,
      ShortTime,
      NumTimeStrings
    };
    PString AsString(TimeFormat format = LongDateTime) const;
      // Return the time as a string in one of the standard formats

    PString AsString(const char * format) const;
    PString AsString(const PString & format) const;
      // Return the time as a string using the string as a format template.
      //    h         hour without leading zero
      //    hh        hour with leading zero
      //    m         minute without leading zero
      //    mm        minute with leading zero
      //    s         second without leading zero
      //    ss        second with leading zero
      //    a         the am/pm string
      //    w/ww/www  abbreviated day of week name
      //    wwww      full day of week name
      //    d         day of month without leading zero
      //    dd        day of month with leading zero
      //    M         month of year without leading zero
      //    MM        month of year with leading zero
      //    MMM       month of year as abbreviated text
      //    MMMM      month of year as full text
      //    y/yy      year without century
      //    yyy/yyyy  year with century
      // All other characters are copied to the output string unchanged.
      // Note if there is an a character in the string, the hour will be in
      // 12 hour format, otherwise in 24 hour format.

    static PString GetTimeSeparator();
      // Return the internationlised time separator.

    static BOOL GetTimeAMPM();
      // Return the internationlised time format: AM/PM or 24 hour.

    static PString GetTimeAM();
      // Return the internationlised time AM string.

    static PString GetTimePM();
      // Return the internationlised time PM string.

    static PString GetDayName(Weekdays dayOfWeek, BOOL abbreviated = FALSE);
      // Return the internationlised day of week name (0=Sun etc).

    static PString GetDateSeparator();
      // Return the internationlised date separator.

    static PString GetMonthName(Months month, BOOL abbreviated = FALSE);
      // Return the internationlised month name string (1=Jan etc).

    enum DateOrder { MonthDayYear, DayMonthYear, YearMonthDay };
    static DateOrder GetDateOrder();
      // Return the internationlised date order.


  protected:
    // Member variables
    time_t theTime;


// Class declaration continued in platform specific header file ///////////////
