/*
 * $Id: ptime.h,v 1.2 1993/07/14 12:49:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ptime.h,v $
 * Revision 1.2  1993/07/14 12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#define _PTIME


///////////////////////////////////////////////////////////////////////////////
// System time and date class

class PTimeInterval;


DECLARE_CLASS(PTime,PObject)

  public:
    inline PTime(time_t t = time(NULL));
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

    static long Resolution();


// Class declaration continued in platform specific header file ///////////////
