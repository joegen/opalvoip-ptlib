/*
 * $Id: ptime.h,v 1.13 1995/03/14 12:42:18 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ptime.h,v $
 * Revision 1.13  1995/03/14 12:42:18  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.12  1995/01/11  09:45:12  robertj
 * Documentation and normalisation.
 *
 * Revision 1.11  1995/01/09  12:34:05  robertj
 * Removed unnecesary return value from I/O functions.
 *
 * Revision 1.10  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.9  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.8  1994/07/27  05:58:07  robertj
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

#ifdef __GNUC__
#pragma interface
#endif


#include <time.h>


///////////////////////////////////////////////////////////////////////////////
// System time and date class

class PTimeInterval;


PDECLARE_CLASS(PTime, PObject)
/* This class defines an absolute time and date. It has a number of time and
   date rendering and manipulation functions. It is based on the standard C
   library functions for time. Thus it is based on a number of seconds since
   1 January 1970.
 */

  public:
    PTime(
      time_t t = time(NULL)   // Time in seconds since 1 January 1970.
    );
    PTime(
      int second,   // Second from 0 to 59.
      int minute,   // Minute from 0 to 59.
      int hour,     // Hour from 0 to 23.
      int day,      // Day of month from 1 to 31.
      int month,    // Month from 1 to 12.
      int year      // Year from 1970 to 2038
    );
    /* Create a time object instance. The first form creates a time from a
       number of seconds since 1 January 1970. The second will build the
       structure from the specified date and time components. If no parameter
       is provided then the time is the actual real time of construction.
     */


  // Overrides from class PObject
    PObject * Clone() const;
    /* Create a copy of the time on the heap. It is the responsibility of the
       caller to delete the created object.
    
       <H2>Returns:</H2>
       pointer to new time.
     */

    virtual Comparison Compare(
      const PObject & obj   // Other time to compare against.
    ) const;
    /* Determine the relative rank of the specified times. This ranks the
       times as you would expect.
       
       <H2>Returns:</H2>
       rank of the two times.
     */

    virtual void PrintOn(
      ostream & strm    // Stream to output the time to.
    ) const;
    /* Output the time to the stream. This uses the
       <A><CODE>AsString()</CODE></A> function with the
       <CODE>ShortDateTime</CODE> parameter.
     */

    virtual void ReadFrom(
      istream & strm    // Stream to input the time from.
    );
    /* Input the time from the specified stream. If a parse error occurs the
       time is set to the current time. The input is expected in the same
       format as produced by the <A><CODE>AsString()</CODE></A> function with
       the <CODE>ShortDateTime</CODE> parameter.
     */


  // New member functions
    int GetSecond() const;
    /* Get the second of the time.

       <H2>Returns:</H2>
       integer in range 0..59.
     */

    int GetMinute() const;
    /* Get the minute of the time.

       <H2>Returns:</H2>
       integer in range 0..59.
     */

    int GetHour() const;
    /* Get the hour of the time.

       <H2>Returns:</H2>
       integer in range 0..23.
     */

    int GetDay() const;
    /* Get the day of the month of the date.

       <H2>Returns:</H2>
       integer in range 1..31.
     */

    enum Months {
      January = 1,
      February,
      March,
      April,
      May,
      June,
      July,
      August,
      September,
      October,
      November,
      December
    };
    // Months in a year.

    Months GetMonth() const;
    /* Get the month of the date.

       <H2>Returns:</H2>
       enum for month.
     */

    int GetYear() const;
    /* Get the year of the date.

       <H2>Returns:</H2>
       integer in range 1970..2038.
     */

    enum Weekdays {
      Sunday,
      Monday,
      Tuesday,
      Wednesday,
      Thursday,
      Friday,
      Saturday
    };
    // DAys of the week.

    Weekdays GetDayOfWeek() const;
    /* Get the day of the week of the date.
    
       <H2>Returns:</H2>
       enum for week days with 0=Sun, 1=Mon, ..., 6=Sat.
     */

    int GetDayOfYear() const;
    /* Get the day in the year of the date.
    
       <H2>Returns:</H2>
       integer from 1..366.
     */

    BOOL IsDaylightSavings() const;
    /* Get flag indicating daylight savings is current.
    
       <H2>Returns:</H2>
       TRUE if daylight savings time is active.
     */


    PTime operator+(
      const PTimeInterval & t   // Time interval to add to the time.
    ) const;
    /* Add the interval to the time to yield a new time.
    
       <H2>Returns:</H2>
       Time altered by the interval.
     */

    PTime & operator+=(
      const PTimeInterval & t   // Time interval to add to the time.
    );
    /* Add the interval to the time changing the instance.
    
       <H2>Returns:</H2>
       reference to the current time instance.
     */

    PTimeInterval operator-(
      const PTime & t   // Time to subtract from the time.
    ) const;
    /* Calculate the difference between two times to get a time interval.
    
       <H2>Returns:</H2>
       Time intervale difference between the times.
     */

    PTime operator-(
      const PTimeInterval & t   // Time interval to subtract from the time.
    ) const;
    /* Subtract the interval from the time to yield a new time.
    
       <H2>Returns:</H2>
       Time altered by the interval.
     */

    PTime & operator-=(
      const PTimeInterval & t   // Time interval to subtract from the time.
    );
    /* Subtract the interval from the time changing the instance.

       <H2>Returns:</H2>
       reference to the current time instance.
     */

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
    // Standard time formats for string representations of a time and date.

    PString AsString(
      TimeFormat formatCode = LongDateTime    // Standard format for time.
    ) const;
    PString AsString(
      const char * formatPtr     // Arbitrary format C string pointer for time.
    ) const;
    PString AsString(
      const PString & formatStr  // Arbitrary format string for time.
    ) const;
    /* Convert the time to a string using the format code or string as a
       formatting template. The special characters in the formatting string
       are:

       <DL>
       <DT>h         <DD>hour without leading zero
       <DT>hh        <DD>hour with leading zero
       <DT>m         <DD>minute without leading zero
       <DT>mm        <DD>minute with leading zero
       <DT>s         <DD>second without leading zero
       <DT>ss        <DD>second with leading zero
       <DT>a         <DD>the am/pm string
       <DT>w/ww/www  <DD>abbreviated day of week name
       <DT>wwww      <DD>full day of week name
       <DT>d         <DD>day of month without leading zero
       <DT>dd        <DD>day of month with leading zero
       <DT>M         <DD>month of year without leading zero
       <DT>MM        <DD>month of year with leading zero
       <DT>MMM       <DD>month of year as abbreviated text
       <DT>MMMM      <DD>month of year as full text
       <DT>y/yy      <DD>year without century
       <DT>yyy/yyyy  <DD>year with century
       </DL>

       All other characters are copied to the output string unchanged.
       
       Note if there is an 'a' character in the string, the hour will be in 12
       hour format, otherwise in 24 hour format.
     */

    static PString GetTimeSeparator();
    /* Get the internationalised time separator.
    
       <H2>Returns:</H2>
       string for time separator.
     */

    static BOOL GetTimeAMPM();
    /* Get the internationalised time format: AM/PM or 24 hour.
    
       <H2>Returns:</H2>
       TRUE is 12 hour, FALSE if 24 hour.
     */

    static PString GetTimeAM();
    /* Get the internationalised time AM string.
    
       <H2>Returns:</H2>
       string for AM.
     */

    static PString GetTimePM();
    /* Get the internationalised time PM string.
    
       <H2>Returns:</H2>
       string for PM.
     */

    static PString GetDayName(
      Weekdays dayOfWeek,       // Code for day of week.
      BOOL abbreviated = FALSE  // Flag for abbreviated or full name.
    );
    /* Get the internationalised day of week day name (0=Sun etc).
    
       <H2>Returns:</H2>
       string for week day.
     */

    static PString GetDateSeparator();
    /* Get the internationalised date separator.
    
       <H2>Returns:</H2>
       string for date separator.
     */

    static PString GetMonthName(
      Months month,             // Code for month in year.
      BOOL abbreviated = FALSE  // Flag for abbreviated or full name.
    );
    /* Get the internationalised month name string (1=Jan etc).
    
       <H2>Returns:</H2>
       string for month.
     */

    enum DateOrder {
      MonthDayYear,   // Date is ordered month then day then year.
      DayMonthYear,   // Date is ordered day then month then year.
      YearMonthDay    // Date is ordered year then day month then day.
    };
    // Possible orders for date components.

    static DateOrder GetDateOrder();
    /* Return the internationalised date order.
    
       <H2>Returns:</H2>
       code for date ordering.
     */


  protected:
    // Member variables
    time_t theTime;
    // Number of seconds since 1 January 1970.


// Class declaration continued in platform specific header file ///////////////
