/*
 * timeint.h
 *
 * Millisecond resolution time interval class (uses 64 bit integers).
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

#ifndef PTLIB_TIMEINTERVAL_H
#define PTLIB_TIMEINTERVAL_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// Difference between two system times

/** This class defines an arbitrary time interval to millisecond accuracy. The
   interval can be both positive and negative.
   
   A long int is used to store the time interval so it is limited to LONG_MAX
   (found in the standard C header file limits.h) milliseconds. This is
   approximately 596 hours for 32 bit integers.
   
   There is a constant, <code>PMaxTimeInterval</code> which defines the
   maximum number of milliseconds that a time interval may be.
 */
class PTimeInterval : public PObject
{
  PCLASSINFO(PTimeInterval, PObject);

  public:
  /**@name Construction */
  //@{
    /** Create a new time interval object. The time interval, in milliseconds,
       is the sum of all of the parameters. For example all of the following
       are equivalent:
<pre><code>
              PTimeInterval(120000)
              PTimeInterval(60000, 60)
              PTimeInterval(60000, 0, 1)
              PTimeInterval(0, 60, 1)
              PTimeInterval(0, 0, 2)
</code></pre>
     */
    PTimeInterval(
      PInt64 millisecs = 0   ///< Number of milliseconds for interval.
    );
    PTimeInterval(
      PInt64 millisecs,     ///< Number of milliseconds for interval.
      long seconds,         ///< Number of seconds for interval.
      long minutes = 0,     ///< Number of minutes for interval.
      long hours = 0,       ///< Number of hours for interval.
      int days = 0          ///< Number of days for interval.
    );
    PTimeInterval(
      const PString & str   ///< String representation of time interval.
    );

    PTimeInterval(const PTimeInterval & other);
    PTimeInterval & operator=(const PTimeInterval & other);

    /// Return a PTimeInterval in nanoseconds
    static PTimeInterval NanoSeconds(
      int64_t nsecs,
      int secs = 0
    );

    /// Return a PTimeInterval in microseconds
    static PTimeInterval MicroSeconds(
      int64_t usecs,
      int secs = 0
    );

    /// Return a PTimeInterval in seconds
    static PTimeInterval Seconds(
      double secs
    );

    /// Return a PTimeInterval from a frequency
    static PTimeInterval Frequency(
      double frequency
    );
  //@}

  /**@name Overrides from class PObject */
  //@{
    /** Create a new copy of the time interval. It is the responsibility of the
       called to delete the object.
       
       @return
       new time interval on heap.
     */
    PObject * Clone() const;

    /** Rank the two time intervals. This ranks the intervals as you would
       expect for two integers.
       
       @return
       <code>EqualTo</code>, <code>LessThan</code> or <code>GreaterThan</code>
       depending on their relative rank.
     */
    virtual Comparison Compare(
      const PObject & obj   ///< Time interval to compare against.
    ) const;

    /** Output the time interval to the I/O stream.
        The output is controlled by various stream flags:
          ios::scientific, ios::showbase    Output if < 1 then output SI notation, e.g. "234ms", >1 is as below
          ios::scientific, ios::noshowbase  Output as per a floating point number, e.g. -314.1592
          ios::fixed, ios::noshowbase       Output as hours/minutes/seconds, e.g. "12:34.567".
          ios::fixed, ios::showbase         Output as days/hours/minutes/seconds, e.g. "2d13:25:47.312141".
        The strm.precision() is honoured as expected, controlling number of decimals, clamped to 9.
        A width() greater than needed, with fillchar('0') will also be honoured, e.g. "01:23:56.678901"
     */
    virtual void PrintOn(
      ostream & strm    ///< I/O stream to output the time interval.
    ) const;

    /** Input the time interval from the I/O stream. This expects the input
       to be a signed decimal integer number.
     */
    virtual void ReadFrom(
      istream & strm    ///< I/O stream to input the time interval from.
    );
  //@}

  /**@name Conversion functions */
  //@{
    enum Formats {
      NormalFormat,     ///< Output as "hh:mm:ss.uuu"
      IncludeDays,      ///< Output as "d:hh:mm:ss.uuu"
      SecondsOnly,      ///< Output as "s.uuu"
      SecondsSI         ///< Ouptut using SI units, ns, us, ms, s, though not ks, Ms etc
    };

    /** Output the time interval as a string.
        If \p precision is <= 0 and \p format is NormalFormat then the
        format used will be IncludeDays, and the positive value
        for \p precision is then used as number of decimals.
      */
    PString AsString(
      int decimals = 3,                 ///< Decimals for milliseconds part
      Formats format = NormalFormat,    ///< Output format options
      int width = 1                     ///< Width of output
    ) const;
  //@}

  /**@name Access functions */
  //@{
    /** Get the number of nanoseconds for the time interval.
    
       @return
       very long integer number of nanoseconds.
     */
    PInt64 GetNanoSeconds() const;

    /** Set the number of nanoseconds for the time interval.
     */
    void SetNanoSeconds(
      int64_t nsecs,
      int secs = 0
    );

    /** Get the number of microseconds for the time interval.
    
       @return
       very long integer number of microseconds.
     */
    PInt64 GetMicroSeconds() const;

    /** Set the number of microseconds for the time interval.
     */
    void SetMicroSeconds(
      int64_t usecs,
      int secs = 0
    );

    /** Get time interval as a frequency.
      */
    double GetFrequency() const;

    /** Set time interval from a frequency.
      */
    void SetFrequency(double frequency);

    /** Get the number of milliseconds for the time interval.
    
       @return
       very long integer number of milliseconds.
     */
    PInt64 GetMilliSeconds() const;

    /** Set the number of milliseconds for the time interval.
     */
    void SetMilliSeconds(PInt64 msecs);

    /** Get the number of whole seconds for the time interval.
    
       @return
       long integer number of seconds.
     */
    long GetSeconds() const;

    /** Get the number of seconds, and partial seconds, as floating point, for the time interval.
    
       @return
       number of seconds as double.
     */
    double GetSecondsAsDouble() const;

    /** Set the number of seconds for the time interval.
     */
    void SetSeconds(
      double secs
    );

    /** Get the number of whole minutes for the time interval.
    
       @return
       integer number of minutes.
     */
    long GetMinutes() const;

    /** Get the number of whole hours for the time interval.
    
       @return
       integer number of hours.
     */
    int GetHours() const;

    /** Get the number of whole days for the time interval.
    
       @return
       integer number of days.
     */
    int GetDays() const;

#ifdef _WIN32
    typedef DWORD IntervalType;
#else
    typedef int IntervalType;
#endif

    /** Get the "clamped" number of milliseconds for the time interval.
        This returns an operating system dependent value for use in system
        calls. For Windows, a value of PMaxTimeInterval returns INFINITE.
        For unix systems, PMaxTimeInterval returns -1. In both cases a
        negative value returns zero.
    
       @return
       number of milliseconds.
     */
    IntervalType GetInterval() const;

    /** Set the value of the time interval. The time interval, in milliseconds,
       is the sum of all of the parameters. For example all of the following
       are equivalent:
<pre><code>
              SetInterval(120000)
              SetInterval(60000, 60)
              SetInterval(60000, 0, 1)
              SetInterval(0, 60, 1)
              SetInterval(0, 0, 2)
</code></pre>
     */
    void SetInterval(
      PInt64 milliseconds = 0,  ///< Number of milliseconds for interval.
      long seconds = 0,         ///< Number of seconds for interval.
      long minutes = 0,         ///< Number of minutes for interval.
      long hours = 0,           ///< Number of hours for interval.
      int days = 0              ///< Number of days for interval.
    );
  //@}

  /**@name Operations */
  //@{
    /** Unary minus, get negative of time interval.
    
       @return
       difference of the time intervals.
     */
    PTimeInterval operator-() const;

    /** Add the two time intervals yielding a third time interval.
    
       @return
       sum of the time intervals.
     */
    PTimeInterval operator+(
      const PTimeInterval & interval   ///< Time interval to add.
    ) const;
    friend PTimeInterval operator+(int64_t left, const PTimeInterval & right);

    /** Add the second time interval to the first time interval.
    
       @return
       reference to first time interval.
     */
    PTimeInterval & operator+=(
      const PTimeInterval & interval   ///< Time interval to add.
    );

    /** Subtract the two time intervals yielding a third time interval.
    
       @return
       difference of the time intervals.
     */
    PTimeInterval operator-(
      const PTimeInterval & interval   ///< Time interval to subtract.
    ) const;
    friend PTimeInterval operator-(int64_t left, const PTimeInterval & right);

    /** Subtract the second time interval from the first time interval.
    
       @return
       reference to first time interval.
     */
    PTimeInterval & operator-=(
      const PTimeInterval & interval   ///< Time interval to subtract.
    );

    /** Multiply the time interval by a factor yielding a third time interval.
    
       @return
       the time intervals times the factor.
     */
    PTimeInterval operator*(
      int factor   ///< factor to multiply.
    ) const;
    friend PTimeInterval operator*(int left, const PTimeInterval & right);

    /** Multiply the time interval by a factor.
    
       @return
       reference to time interval.
     */
    PTimeInterval & operator*=(
      int factor   ///< factor to multiply.
    );

    /** Divide the time interval by another interval yielding a count.
    
       @return
       the number of times the second interval occurs in the larger.
     */
    int operator/(
      const PTimeInterval & smaller   ///< factor to divide.
    ) const;

    /** Divide the time interval by a factor yielding a third time interval.
    
       @return
       the time intervals divided by the factor.
     */
    PTimeInterval operator/(
      int factor   ///< factor to divide.
    ) const;
    friend PTimeInterval operator/(int64_t left, const PTimeInterval & right);

    /** Divide the time interval by a factor.
    
       @return
       reference to time interval.
     */
    PTimeInterval & operator/=(
      int factor   ///< factor to divide.
    );
  //@}

  /**@name Comparison functions */
  //@{
    /** Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       @return
       true if intervals are equal.
     */
    bool operator==(
      const PTimeInterval & interval   ///< Time interval to compare.
    ) const;
    bool operator==(
      long msecs    ///< Time interval as integer milliseconds to compare.
    ) const;

    /** Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       @return
       true if intervals are not equal.
     */
    bool operator!=(
      const PTimeInterval & interval   ///< Time interval to compare.
    ) const;
    bool operator!=(
      long msecs    ///< Time interval as integer milliseconds to compare.
    ) const;

    /** Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       @return
       true if intervals are greater than.
     */
    bool operator> (
      const PTimeInterval & interval   ///< Time interval to compare.
    ) const;
    bool operator> (
      long msecs    ///< Time interval as integer milliseconds to compare.
    ) const;

    /** Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       @return
       true if intervals are greater than or equal.
     */
    bool operator>=(
      const PTimeInterval & interval   ///< Time interval to compare.
    ) const;
    bool operator>=(
      long msecs    ///< Time interval as integer milliseconds to compare.
    ) const;

    /** Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       @return
       true if intervals are less than.
     */
    bool operator< (
      const PTimeInterval & interval   ///< Time interval to compare.
    ) const;
    bool operator< (
      long msecs    ///< Time interval as integer milliseconds to compare.
    ) const;

    /** Compare to the two time intervals. This is provided as an override to
       the default in PObject so that comparisons can be made to integer
       literals that represent milliseconds.

       @return
       true if intervals are less than or equal.
     */
    bool operator<=(
      const PTimeInterval & interval   ///< Time interval to compare.
    ) const;
    bool operator<=(
      long msecs    ///< Time interval as integer milliseconds to compare.
    ) const;
  //@}

    static int64_t const MicroToNano = 1000;
    static int64_t const MilliToNano = MicroToNano*1000;
    static int64_t const SecsToNano  = MilliToNano*1000;
    static int64_t const MinsToNano  = SecsToNano*60;
    static int64_t const HoursToNano = MinsToNano*60;
    static int64_t const DaysToNano  = HoursToNano*24;

  protected:
    virtual int64_t InternalGet() const;
    virtual void InternalSet(int64_t t);

  private:
    atomic<int64_t> m_nanoseconds;


// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/timeint.h"
#else
#include "unix/ptlib/timeint.h"
#endif
};


/// Class wrapper of PTimeInterval to initialise to nanoseconds.
struct PNanoSeconds : PTimeInterval
{
  PNanoSeconds(int64_t nanoseconds) { SetNanoSeconds(nanoseconds); }
};

/// Class wrapper of PTimeInterval to initialise to microseconds.
struct PMicroSeconds : PTimeInterval
{
  PMicroSeconds(int64_t microseconds) { SetMicroSeconds(microseconds); }
};

/// As we have PNanoSeconds & PMicroSeconds, this is for completeness.
typedef PTimeInterval PMilliSeconds;

/// Class wrapper of PTimeInterval to initialise to a frequency.
struct PFrequencyTime : PTimeInterval
{
  PFrequencyTime(double frequency) { SetFrequency(frequency); }
};


namespace std
{
    /// Specialisation of numeric_limits for PTimeInterval
  template<> class numeric_limits<PTimeInterval>
  {
  public:
    static PTimeInterval min() { return -PMaxTimeInterval; }
    static PTimeInterval max() { return  PMaxTimeInterval; }
    static PTimeInterval epsilon() { return PTimeInterval::NanoSeconds(1); }
    static PTimeInterval round_error() { return PTimeInterval::NanoSeconds(1); }
    static PTimeInterval denorm_min() { return PTimeInterval::NanoSeconds(numeric_limits<int64_t>::denorm_min()); }
    static PTimeInterval infinity() { return PTimeInterval::NanoSeconds(numeric_limits<int64_t>::infinity()); }
    static PTimeInterval quiet_NaN() { return PTimeInterval::NanoSeconds(numeric_limits<int64_t>::quiet_NaN()); }
    static PTimeInterval signaling_NaN() { return PTimeInterval::NanoSeconds(numeric_limits<int64_t>::signaling_NaN()); }
  };
};


#endif // PTLIB_TIMEINTERVAL_H


// End Of File ///////////////////////////////////////////////////////////////
