/*
 * $Id: timeint.h,v 1.4 1993/08/31 03:38:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: timeint.h,v $
 * Revision 1.4  1993/08/31 03:38:02  robertj
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


///////////////////////////////////////////////////////////////////////////////
// Difference between two system times

DECLARE_CLASS(PTimeInterval,PObject)

  public:
    PTimeInterval(long milliseconds = 0,
                int seconds = 0, int minutes = 0, int hours = 0, int days = 0);
      // Make a new time interval object

    PTimeInterval(const PTimeInterval & ti);
    PTimeInterval & operator=(const PTimeInterval & ti);
      // Make a copy of the time interval object

    // Overrides from class PObject
    PObject * Clone() const;
    virtual Comparison Compare(const PObject & obj) const;
    virtual ostream & PrintOn(ostream & strm) const;
    virtual istream & ReadFrom(istream & strm);

    // New member functions
    long Milliseconds() const;
    long Seconds() const;
    long Minutes() const;
    int Hours() const;
    int Days() const;

    void SetInterval(long milliseconds = 0,
                int seconds = 0, int minutes = 0, int hours = 0, int days = 0);

    PTimeInterval operator+(const PTimeInterval & t) const;
    PTimeInterval & operator+=(const PTimeInterval & t);
    PTimeInterval operator-(const PTimeInterval & t) const;
    PTimeInterval & operator-=(const PTimeInterval & t);


  protected:
    // Member variables
    PMilliseconds milliseconds;


// Class declaration continued in platform specific header file ///////////////
