/*
 * $Id: ptime.h,v 1.6 1994/01/13 03:16:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ptime.h,v $
 * Revision 1.6  1994/01/13 03:16:09  robertj
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


PDECLARE_CLASS(PTime,PObject)

  public:
    PINLINE PTime(time_t t = time(NULL));
    PTime(int second,int minute,int hour,int day,int month,int year);

    // Overrides from class PObject
    PObject * Clone() const;
    virtual Comparison Compare(const PObject & obj) const;
    virtual ostream & PrintOn(ostream & strm) const;
    virtual istream & ReadFrom(istream & strm);

    // New member functions
    int GetSecond() const;
    int GetMinute() const;
    int GetHour() const;
    int GetDay() const;
    int GetMonth() const;
    int GetYear() const;
    int GetDayOfWeek() const;
    int GetDayOfYear() const;
    BOOL IsDaylightSavings() const;

    PTime operator+(const PTimeInterval & t) const;
    PTime & operator+=(const PTimeInterval & t);
    PTimeInterval operator-(const PTime & t) const;
    PTime operator-(const PTimeInterval & t) const;
    PTime & operator-=(const PTimeInterval & t);

    PString AsString() const;


  protected:
    // Member variables
    time_t theTime;


// Class declaration continued in platform specific header file ///////////////
