/*
 * $Id: timeint.h,v 1.12 1995/03/14 12:42:50 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: timeint.h,v $
 * Revision 1.12  1995/03/14 12:42:50  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.11  1995/01/18  09:01:32  robertj
 * Documentation.
 *
 * Revision 1.10  1995/01/09  12:29:41  robertj
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
/* This class defines an arbitrary time interval to millisecond accuracy. The
   interval can be both positive and negative.
   
   A long int is used to store the time interval so it is limited to LONG_MAX
   (found in the standard C header file limits.h) milliseconds. This is
   approximately 596 hours for 32 bit integers.
   
   There is a constant, <CODE>PMaxTimeInterval</CODE> which defines the
   maximum number of milliseconds that a time interval may be.
 */

  public:
    PTimeInterval(
      long milliseconds = 0,  // Number of milliseconds for interval.
      int seconds = 0,        // Number of seconds for interval.
      int minutes = 0,        // Number of minutes for interval.
      int hours = 0,          // Number of hours for interval.
      int days = 0            // Number of days for interval.
    );
    /* Create a new time interval object. The time interval, in milliseconds,
       is the sum of all of the parameters. For example all of the following
       are equivalent:

       <PRE><CODE>
              PTimeInterval(120000)
              PTimeInterval(60000, 60)
              PTimeInterval(60000, 0, 1)
              PTimeInterval(0, 60, 1)
              PTimeInterval(0, 0, 2)
       </CODE></PRE>
     */


  // Overrides from class PObject
    PObject * Clone() const;
    /* Create a new copy of the time interval. It is the responsibility of the
       called to delete the object.
       
       <H2>Returns:</H2>
       new time interval on heap.
     */

    virtual Comparison Compare(
      const PObject & obj   // Time interval to compare against.
    ) const;
    /* Rank the two time intervals. This ranks the intervals as you would
       expect for two integers.
       
       <H2>Returns:</H2>
       <CODE>EqualTo</CODE>, <CODE>LessThan</CODE> or <CODE>GreaterThan</CODE>
       depending on their relative rank.
     */

    virtual void PrintOn(
      ostream & strm    // I/O stream to output the time interval.
    ) const;
    /* Output the time interval to the I/O stream. This outputs the number of
       milliseconds as a signed decimal integer number.
     */

    virtual void ReadFrom(
      istream & strm    // I/O stream to input the time interval from.
    );
    /* Input the time interval from the I/O stream. This expects the input
       to be a signed decimal integer number.
     */


  // New member functions
    long GetMilliseconds() const;
    /* Get the number of milliseconds for the time interval.
    
       <H2>Returns:</H2>
       long integer number of milliseconds.
     */

    long GetSeconds() const;
    /* Get the number of whole seconds for the time interval.
    
       <H2>Returns:</H2>
       long integer number of seconds.
     */

    long GetMinutes() const;
    /* Get the number of whole minutes for the time interval.
    
       <H2>Returns:</H2>
       integer number of minutes.
     */

    int GetHours() const;
    /* Get the number of whole hours for the time interval.
    
       <H2>Returns:</H2>
       integer number of hours.
     */

    int GetDays() const;
    /* Get the number of whole days for the time interval.
    
       <H2>Returns:</H2>
       integer number of days.
     */


    void SetInterval(
      long milliseconds = 0,  // Number of milliseconds for interval.
      int seconds = 0,        // Number of seconds for interval.
      int minutes = 0,        // Number of minutes for interval.
      int hours = 0,          // Number of hours for interval.
      int days = 0            // Number of days for interval.
    );
    /* Set the value of the time interval. The time interval, in milliseconds,
       is the sum of all of the parameters. For example all of the following
       are equivalent:
       <PRE><CODE>
              SetInterval(120000)
              SetInterval(60000, 60)
              SetInterval(60000, 0, 1)
              SetInterval(0, 60, 1)
              SetInterval(0, 0, 2)
       </CODE></PRE>
     */

    PTimeInterval operator+(
      const PTimeInterval & interval   // Time interval to add.
    ) const;
    /* Add the two time intervals yielding a third time interval.
    
       <H2>Returns:</H2>
       sum of the time intervals.
     */

    PTimeInterval & operator+=(
      const PTimeInterval & interval   // Time interval to add.
    );
    /* Add the second time interval to the first time interval.
    
       <H2>Returns:</H2>
       reference to first time interval.
     */

    PTimeInterval operator-(
      const PTimeInterval & interval   // Time interval to subtract.
    ) const;
    /* Subtract the two time intervals yielding a third time interval.
    
       <H2>Returns:</H2>
       difference of the time intervals.
     */

    PTimeInterval & operator-=(
      const PTimeInterval & interval   // Time interval to subtract.
    );
    /* Subtract the second time interval from the first time interval.
    
       <H2>Returns:</H2>
       reference to first time interval.
     */

    BOOL operator==(
      const PTimeInterval & interval   // Time interval to compare.
    ) const;
    BOOL operator==(
      long msecs    // Time interval as integer milliseconds to compare.
    ) const;
    /* Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       <H2>Returns:</H2>
       TRUE if intervals are equal.
     */

    BOOL operator!=(
      const PTimeInterval & interval   // Time interval to compare.
    ) const;
    BOOL operator!=(
      long msecs    // Time interval as integer milliseconds to compare.
    ) const;
    /* Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       <H2>Returns:</H2>
       TRUE if intervals are not equal.
     */

    BOOL operator> (
      const PTimeInterval & interval   // Time interval to compare.
    ) const;
    BOOL operator> (
      long msecs    // Time interval as integer milliseconds to compare.
    ) const;
    /* Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       <H2>Returns:</H2>
       TRUE if intervals are greater than.
     */

    BOOL operator>=(
      const PTimeInterval & interval   // Time interval to compare.
    ) const;
    BOOL operator>=(
      long msecs    // Time interval as integer milliseconds to compare.
    ) const;
    /* Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       <H2>Returns:</H2>
       TRUE if intervals are greater than or equal.
     */

    BOOL operator< (
      const PTimeInterval & interval   // Time interval to compare.
    ) const;
    BOOL operator< (
      long msecs    // Time interval as integer milliseconds to compare.
    ) const;
    /* Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       <H2>Returns:</H2>
       TRUE if intervals are less than.
     */

    BOOL operator<=(
      const PTimeInterval & interval   // Time interval to compare.
    ) const;
    BOOL operator<=(
      long msecs    // Time interval as integer milliseconds to compare.
    ) const;
    /* Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       <H2>Returns:</H2>
       TRUE if intervals are less than or equal.
     */


  protected:
  // Member variables
    long milliseconds;
    // Number of milliseconds in time interval.


// Class declaration continued in platform specific header file ///////////////
