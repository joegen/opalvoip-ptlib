/*
 * $Id: timeint.h,v 1.2 1993/07/14 12:49:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: timeint.h,v $
 * Revision 1.2  1993/07/14 12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#define _PTIMEINTERVAL


///////////////////////////////////////////////////////////////////////////////
// Difference between two system times

DECLARE_CLASS(PTimeInterval,PObject)

  public:
    PTimeInterval(long milliseconds = 0,
                   int seconds = 0,int minutes = 0,int hours = 0,int days = 0);

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

    PTimeInterval operator+(const PTimeInterval & t) const;
    PTimeInterval & operator+=(const PTimeInterval & t);
    PTimeInterval operator-(const PTimeInterval & t) const;
    PTimeInterval & operator-=(const PTimeInterval & t);


// Class declaration continued in platform specific header file ///////////////
