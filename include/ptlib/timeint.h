/*
 * $Id: timeint.h,v 1.10 1995/01/09 12:29:41 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: timeint.h,v $
 * Revision 1.10  1995/01/09 12:29:41  robertj
 * Removed unnecesary return value from I/O functions.
 *
 * Revision 1.9  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.8  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.7  1994/07/02  03:03:49  robertj
 * Timer redesign consequences and ability to compare a time interval against
 * ordinary integer milliseconds.
 *
 * Revision 1.6  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.5  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.4  1993/08/31  03:38:02  robertj
 * Added copy constructor and assignement oeprator due to G++ strangeness.
 *
 * Revision 1.3  1993/08/27  18:17:47  robertj
 * Added function to set the interval of a PTieInterval object.
 * Used a common type for number of milliseconds.
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#define _PTIMEINTERVAL

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// Difference between two system times

PDECLARE_CLASS(PTimeInterval, PObject)
  // Arbitrary time interval to millisecond accuracy. Can be both positive and
  // negative intervals.

  public:
    PTimeInterval(long milliseconds = 0,
                int seconds = 0, int minutes = 0, int hours = 0, int days = 0);
      // Make a new time interval object


    // Overrides from class PObject
    PObject * Clone() const;
    virtual Comparison Compare(const PObject & obj) const;
    virtual void PrintOn(ostream & strm) const;
    virtual void ReadFrom(istream & strm);


    // New member functions
    long GetMilliseconds() const;
    long GetSeconds() const;
    long GetMinutes() const;
    int GetHours() const;
    int GetDays() const;
      // Get the time interval in the specified magnitude.

    void SetInterval(long milliseconds,
                int seconds = 0, int minutes = 0, int hours = 0, int days = 0);
      //Set the value of the time interval.

    PTimeInterval operator+(const PTimeInterval & t) const;
    PTimeInterval & operator+=(const PTimeInterval & t);
    PTimeInterval operator-(const PTimeInterval & t) const;
    PTimeInterval & operator-=(const PTimeInterval & t);
      // Arithmetic with time intervals.

    BOOL operator==(const PTimeInterval & t) const;
    BOOL operator!=(const PTimeInterval & t) const;
    BOOL operator> (const PTimeInterval & t) const;
    BOOL operator>=(const PTimeInterval & t) const;
    BOOL operator< (const PTimeInterval & t) const;
    BOOL operator<=(const PTimeInterval & t) const;

    BOOL operator==(long msecs) const;
    BOOL operator!=(long msecs) const;
    BOOL operator> (long msecs) const;
    BOOL operator>=(long msecs) const;
    BOOL operator< (long msecs) const;
    BOOL operator<=(long msecs) const;
      // Relational operators so can compare against integers


  protected:
    // Member variables
    long milliseconds;


// Class declaration continued in platform specific header file ///////////////
