/*
 * timing.cxx
 *
 * Sample program to test PWLib PAdaptiveDelay.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Roger Hardiman
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
 * The Initial Developer of the Original Code is Roger Hardiman
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/delaychan.h>

/*
 * The main program class
 */
class TimingTest : public PProcess
{
  PCLASSINFO(TimingTest, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(TimingTest);

#define TEST_TIME(t) cout << t << " => " << PTime(t) << '\n'

// The main program
void TimingTest::Main()
{
  cout << "Timing Test Program\n" << endl;

  PTimeInterval nano = PTimeInterval::NanoSeconds(-543210123456789LL);
  int w, p;
  for (w = 1; w <= 18; w++)
    for (p = 0; p <= 9; p++)
      cout << "TimeInterval scientific, width " << w << ", precision " << p << ": \""
             << setiosflags(ios::scientific)
             << setw(w) << setprecision(p) << nano
             << resetiosflags(ios::scientific) << '"' << endl;
  nano.SetNanoSeconds(PTimeInterval::DaysToNano*10+
                      PTimeInterval::HoursToNano*12+
                      PTimeInterval::MinsToNano*34+
                      PTimeInterval::SecsToNano*56+
                      123456789);
  for (w = 1; w <= 23; w++)
    for (p = -9; p <= 9; p++)
      cout << "TimeInterval output, width " << w << ", precision " << p << ": \""
           << setw(w) << setprecision(p) << nano << '"' << endl;

  cout << "\n\n";

  PTime now;
  cout << "Time is now " << now.AsString("h:m:s.u d/M/y") << "\n"
          "Time is now " << now.AsString("yyyy/MM/dd h:m:s.uuuu") << "\n"
          "Time is now " << now.AsString("MMM/d/yyyyy w h:m:sa") << "\n"
          "Time is now " << now.AsString("wwww d/M/yyy h:m:s.uu") << "\n"
          "Time is now " << now.AsString("www d/MMMM/yy h:m:s.uuu") << "\n"
          "Time is now " << now.AsString(PTime::RFC3339) << "\n"
          "Time is now " << now.AsString(PTime::ShortISO8601) << "\n"
          "Time is now " << now.AsString(PTime::LongISO8601) << "\n"
          "Time is now " << now.AsString(PTime::RFC1123) << endl;

  cout << "\nTesting time string conversion" << endl;
  TEST_TIME("2001-06-03 T 12:34:56");
  TEST_TIME("2001-02-03 T 12:34:56+1000");
  TEST_TIME("2001-02-03T12:34:56+09:30");
  TEST_TIME("20010203T123456Z");
  TEST_TIME("20010203T000056Z");
  TEST_TIME("20010203T123456+1100");
  TEST_TIME("5/03/1999 12:34:56");
  TEST_TIME("5/03/1999 12:34");
  TEST_TIME("12:34 5/03/1999");
  TEST_TIME("15/06/1999 12:34:56");
  TEST_TIME("15/06/01 12:34:56 PST");
  TEST_TIME("5/06/02 12:34:56");
  TEST_TIME("5/23/1999 12:34am");
  TEST_TIME("23/5/1999 12:34am");
  TEST_TIME("1999/04/23 12:34:56");
  TEST_TIME("5/23/00 12:34am");
  TEST_TIME("Mar 3, 1999 12:34pm");
  TEST_TIME("3 Jul 2004 12:34pm");
  TEST_TIME("12:34:56 5 December 1999");
  TEST_TIME("12:34:56");
  TEST_TIME("10 minutes ago");
  TEST_TIME("2 weeks");

  cout << "\nTesting time interval arithmetic" << endl;
  PTime then("1 month ago");
  PTimeInterval elapsed = now - then;
  cout << "Now=" << now << "\n"
          "Then=" << then << "\n"
          "Elapsed=" << elapsed << "\n"
          "Milliseconds=" << elapsed.GetMilliSeconds() << "\n"
          "Seconds=" << elapsed.GetSeconds() << "\n"
          "Minutes=" << elapsed.GetMinutes() << "\n"
          "Hours=" << elapsed.GetHours() << "\n"
          "Days=" << elapsed.GetDays() << endl;

  then += PTimeInterval(0,0,0,0,30);
  cout << "Then plus 30 days=" << then << endl;

  cout << "\nTesting timer resolution, reported as " << PTimer::Resolution() << "ms" << endl;
  time_t oldSec = time(NULL);   // Wait for second boundary
  while (oldSec == time(NULL))
    ;

  oldSec++;
  PTimeInterval newTick = PTimer::Tick();
  PTimeInterval oldTick = newTick;
  unsigned count = 0;

  while (oldSec == time(NULL)) {  // For one full second
    while (newTick == oldTick)
      newTick = PTimer::Tick();
    oldTick = newTick;
    count++;                      // Count the changes in tick
  } ;

  cout << "Actual resolution is " << 1000000/count << "us\n"
          "Current tick: \"" << newTick << '"' << endl;

  cout << "\nTesting sleep function" << endl;
  PTime start_time1;
  PINDEX loop;
  for (loop = 1; loop < 5; loop++) {
    cout << "Sleeping " << loop << " seconds" << endl;
    PSimpleTimer beforeSleep;
    PThread::Sleep(PTimeInterval(0, loop));
    cout << beforeSleep.GetElapsed() << ' ' << (PTime()-start_time1) << endl;
  }
  cout << "The first loop took " << (PTime()-start_time1) << " seconds." << endl;


  cout << "\nTesting adaptive delay function" << endl;
  PAdaptiveDelay delay;

  PTime start_time2;
  cout << "Start at " << start_time2.AsString("hh.mm:ss.uuu") << endl;
  for(loop = 0; loop < 10; loop++) {
    delay.Delay(150);
    PTime now2;
    cout << "#" << setw(2) << (loop + 1) <<" ";
    cout << "After " << setw(4) << ((loop + 1)* 150) << "ms, time is " << now2.AsString("hh.mm:ss.uuu");
    PTimeInterval gap = now2-start_time2;
    cout << "  Elapsed time since start is " << setfill('0') << setw(2) << gap.GetSeconds() << ":" << setw(3) << (gap.GetMilliSeconds() % 1000) << endl;
  }
  PTime end_time2;

  cout << "The second loop took "<< end_time2-start_time2 << " milliseconds." << endl;
}

