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
 * $Log: timing.cxx,v $
 * Revision 1.1  2003/02/19 14:10:11  rogerh
 * Add a program to test PAdaptiveDelay
 *
 *
 *
 */

#include <ptlib.h>
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

// The main program
void TimingTest::Main()
{
  cout << "Timing Test Program" << endl;

  PTime start_time1;
  for(PINDEX loop = 0; loop<10; loop++) {
    usleep(10*1000);
    PTime now1;
    cout << now1-start_time1 << endl;
  }
  PTime end_time1;

  cout << "the first loop took "<< end_time1-start_time1 << " milliseconds." << endl;


  PAdaptiveDelay delay;

  PTime start_time2;
  for(PINDEX loop = 0; loop<10; loop++) {
    delay.Delay(10);
    PTime now2;
    cout << now2-start_time2 << endl;
  }
  PTime end_time2;

  cout << "the first loop took "<< end_time2-start_time2 << " milliseconds." << endl;
}

