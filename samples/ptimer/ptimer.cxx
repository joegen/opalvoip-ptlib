/*
 * main.cxx
 *
 * PWLib application source file for ptimer test program
 *
 * Main program entry point.
 *
 * Copyright (c) 2006 Indranet Technologies Ltd.
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
 * The Initial Developer of the Original Code is Indranet Technologies Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/random.h>


/**A class that does a PTimer functionality. This class runs once. It
is started, and on completion of the delay it toggles a flag. At
that point, this timer has finished. */
class MyTimer : public PTimer
{
  PCLASSINFO(MyTimer, PTimer);
public:
  /**constructor */
  MyTimer();

  /**method used to start everything */
  void StartRunning(PSyncPoint * _exitFlag, PINDEX delayMs);

  /**Method called when this timer finishes */
  virtual void OnTimeout();

protected:
  /**The flag to mark the end of this timer */
  PSyncPoint *exitFlag;

  /**The duration we delay for */
  PTimeInterval delayPeriod;

  /**The time at which we started */
  PTime startTime;


};

/////////////////////////////////////////////////////////////////////////////

/**This class is a simple simple thread that just creates, waits a
period of time, and exits.It is designed to test the PwLib methods
for reporting the status of a thread. This class will be created
over and over- millions of times is possible if left long
enough. If the pwlib thread status functions are broken, a segfault
will result. Past enxperience has found a fault in pwlib with the
BusyWait option on, with SMP machines and a delay period of 20ms */
class DelayThread : public PThread
{
  PCLASSINFO(DelayThread, PThread);

public:
  DelayThread(PINDEX _delay, PBoolean _checkTimer);

  ~DelayThread();

  void Main();

protected:
  PINDEX delay;

  PBoolean checkTimer;

  MyTimer localPTimer;

  PSyncPoint endMe;
};

/////////////////////////////////////////////////////////////////////////////
/**This class turns the ptimer on from a separate thread to the delay thread.
Then, DelayThread checks this ptimer, and waits for it to finish.
*/
class TimerOnThread : public PThread
{
  PCLASSINFO(TimerOnThread, PThread);

public:
  TimerOnThread(PTimer & _timer);

  void Main();

protected:

  PTimer timer;
};

////////////////////////////////////////////////////////////////////////////////
/**This thread handles the Users console requests to query the status of
the launcher thread. It provides a means for the user to close down this
program - without having to use Ctrl-C*/
class UserInterfaceThread : public PThread
{
  PCLASSINFO(UserInterfaceThread, PThread);

public:
  UserInterfaceThread()
    : PThread(10000, NoAutoDeleteThread)
  {
  }

  void Main();

protected:
};

///////////////////////////////////////////////////////////////////////////////
/**This thread launches multiple instances of the BusyWaitThread. Each
thread launched is busy monitored for termination. When the thread
terminates, the thread is deleted, and a new one is created. This
process repeats until segfault or termination by the user */
class LauncherThread : public PThread
{
  PCLASSINFO(LauncherThread, PThread);

public:
  LauncherThread()
    : PThread(10000, NoAutoDeleteThread)
  {
    iteration = 0; keepGoing = true;
  }

  void Main();

  PINDEX GetIteration()
  {
    return iteration;
  }

  virtual void Terminate()
  {
    keepGoing = false;
  }

  PTimeInterval GetElapsedTime()
  {
    return PTime() - startTime;
  }

protected:
  PINDEX iteration;
  PTime startTime;
  PBoolean  keepGoing;
};


////////////////////////////////////////////////////////////////////////////////

class PTimerTest : public PProcess
{
  PCLASSINFO(PTimerTest, PProcess)

public:
  PTimerTest();
  virtual void Main();

  PINDEX Delay()
  {
    return delay;
  }

  PINDEX Interval()
  {
    return interval;
  }

  PBoolean   CheckTimer()
  {
    return checkTimer;
  }

  static PTimerTest & Current()
  {
    return (PTimerTest &)PProcess::Current();
  }

  void TooSoon(const PTimeInterval & elapsed, const PTimeInterval& expected);

protected:

  PINDEX delay;

  PINDEX interval;

  PBoolean   checkTimer;


  /**Code to run the second test supported by this application. */
  void RunSecondTest();
  void StressTest();
  void MultiTimerlTest();
  void LongOnTimeoutTest();
  void MassStopTest();
  void StartStopTest();
  void PullCheck();
  void CallbackCheck();
  void DestroyCheck();
  void TestStopInTimeout();
  void DoubleStopTest();
  void DestroyWhenTimeoutCheck();
  void OneShotStopOnTimeoutTest();
  void ContinuousStopOnTimeoutTest();
  void OneShotToContinuousSwitchTest();
  void ContinuousRestartInTimeout();

  /**First internal timer that we manage */
  PTimer firstTimer;

  /**Second internal timer that we manage */
  PTimer secondTimer;

  PTimer* thirdTimer;

  PDECLARE_NOTIFIER(PTimer, PTimerTest, OnFirstTimerExpired);
  PDECLARE_NOTIFIER(PTimer, PTimerTest, OnSecondTimerExpired);

  PDECLARE_NOTIFIER(PThread, PTimerTest, RestartFirstTimerMain);
  PDECLARE_NOTIFIER(PThread, PTimerTest, RestartSecondTimerMain);
  PDECLARE_NOTIFIER(PThread, PTimerTest, RestartThirdTimerMain);

  /**The integer that is set, to indicate activity of the RestartTimer thread */
  PAtomicInteger restartActivity;
};

class MyTimerTester : public PTimer
{
public:
  MyTimerTester(PSyncPoint & _sync)
    : sync(_sync)
  {
  }
  void OnTimeout()
  {
    sync.Signal();
  }
  PSyncPoint & sync;
};


////////////////////////////////////////////////////////////////////////////////

PCREATE_PROCESS(PTimerTest);


void PTimerTest::Main()
{
  srand( (unsigned)time( NULL ) );

  PArgList & args = GetArguments();

  args.Parse("h-help.      print this help\n"
             "c-check.     check the timer is running when it should be running\n"
             "d-delay:     duration (ms) the timer waits for\n"
             "i-interval:  interval (ms) between timer tests\n"
             "s-reset.     A second test, which repeatedly resets two internal timers.\n"
             "z-slow.      A test that try to delete PTimer instance while timers OnTimeout works\n"
             "x-stress.    A test create 10 timers and change it repeatedly from 1000 threads\n"
             "m-multi.     A test create 25 threads. Each thread create 5000 random timers\r"
                           "and calculate average delay between expected and actual timeouts\n"
             "g-stoptest.  Measure Stop() time for many timers.\n"
             PTRACE_ARGLIST
  );
  PTRACE_INITIALISE(args);

  if (!args.IsParsed() || args.HasOption('h')) {
    args.Usage(cerr);
    return;
  }

  PullCheck();
  CallbackCheck();
  StartStopTest();
  TestStopInTimeout();
  DoubleStopTest();
  DestroyCheck();
  DestroyWhenTimeoutCheck();
  OneShotStopOnTimeoutTest();
  ContinuousStopOnTimeoutTest();
  OneShotToContinuousSwitchTest();
  ContinuousRestartInTimeout();

  if (args.HasOption("z"))
    LongOnTimeoutTest();

  if (args.HasOption("m"))
    MultiTimerlTest();

  if (args.HasOption("x"))
    StressTest();

  if (args.HasOption("g"))
    MassStopTest();

  if (args.HasOption('s')) {
    RunSecondTest();
    return;
  }

  checkTimer = args.HasOption('c');

  delay = 200;
  if (args.HasOption('d'))
    delay = args.GetOptionString('d').AsInteger();

  delay = PMIN(1000000, PMAX(0, delay));
  cout << "Created ptimer will wait for " << delay 
       << " milliseconds before ending" << endl;
 
  interval = 50;
  if (args.HasOption('i'))
    interval = args.GetOptionString('i').AsInteger();

  interval = PMIN(1000000, PMAX(0, interval));
  cout << "Separate each instance of PTimer by " << interval 
       << " milliseconds " << endl;

  UserInterfaceThread ui;
  ui.Resume();
  ui.WaitForTermination();
}

void PTimerTest::TooSoon(const PTimeInterval & elapsed, const PTimeInterval& expected)
{
  cerr << "Timer returned too soon, interval: " << elapsed.GetMilliSeconds() << " < " << expected.GetMilliSeconds() << endl;
}

PTimerTest::PTimerTest()
: PProcess("Derek Smithies code factory", "ptimer test", 1, 0, AlphaCode, 0)
{
}


////////////////////////////////////////////////////////////////////////////////

class TestTimer
  : public PTimer
{
  static bool destroyed;
public:
  TestTimer()
    : PTimer(1000)
  {
  }
  virtual ~TestTimer()
  {
    Stop();
    destroyed = true;
  }
protected:
  virtual void OnTimeout()
  {
    PThread::Sleep(5000);
    if (TestTimer::destroyed)
    {
      cerr << "Destroyed timer while timeout is running" << endl;
    }
  }
};

bool TestTimer::destroyed = false;

void PTimerTest::DestroyWhenTimeoutCheck()
{
  TestTimer* t = new TestTimer();
  PThread::Sleep(1500);
  delete t;
  PThread::Sleep(5000);
}


////////////////////////////////////////////////////////////////////////////////

class TestTimer1
  : public PTimer
{
public:
  TestTimer1()
    : PTimer(1000)
  {
  }
protected:
  virtual void OnTimeout()
  {
    if (IsRunning())
      cerr << "Error!!! One shot timer must be stopped in OnTimeout" << endl;
  }
};

void PTimerTest::OneShotStopOnTimeoutTest()
{
  TestTimer1 t;
  if (!t.IsRunning())
    cerr << "Error!!! IsRunning() must be true" << endl;
  PThread::Sleep(2000);
  if (t.IsRunning())
    cerr << "Error!!! IsRunning() must be false" << endl;
}


////////////////////////////////////////////////////////////////////////////////

class TestTimer2
  : public PTimer
{
public:
  TestTimer2()
    : PTimer(0)
  {
    RunContinuous(1000);
  }
protected:
  virtual void OnTimeout()
  {
    if (!IsRunning())
      cerr << "Error!!! Continuous timer must be running in OnTimeout" << endl;
  }
};

void PTimerTest::ContinuousStopOnTimeoutTest()
{
  TestTimer2 t;
  if (!t.IsRunning())
    cerr << "Error!!! IsRunning() must be true" << endl;
  PThread::Sleep(4000);
  if (!t.IsRunning())
    cerr << "Error!!! IsRunning() must be true" << endl;
}


////////////////////////////////////////////////////////////////////////////////

class TestTimer3
  : public PTimer
{
  PSyncPoint& m_sync;
  int m_shots;
public:
  TestTimer3(PSyncPoint& sync)
    : PTimer(500)
    , m_sync(sync)
    , m_shots(4)
  {
  }
protected:
  virtual void OnTimeout()
  {
    if (m_shots > 0)
    {
      if (m_shots == 4)
      {
        if (IsRunning())
          cerr << "Error!!! OneShot timer must be stopped in OnTimeout" << endl;
        RunContinuous(500);
        if (!IsRunning())
          cerr << "Error!!! Continuous timer must be running in OnTimeout" << endl;
      }
      else if (m_shots == 1)
      {
        m_sync.Signal();
        return;
      }
      --m_shots;
    }
  }
};

void PTimerTest::OneShotToContinuousSwitchTest()
{
  PSyncPoint sync;
  TestTimer3 t(sync);
  sync.Wait();
  if (!t.IsRunning())
    cerr << "Error!!! IsRunning() must be true" << endl;
}


////////////////////////////////////////////////////////////////////////////////

class TestTimer4
  : public PTimer
{
  PSyncPoint& m_sync;
  int m_shots;
public:
  TestTimer4(PSyncPoint& sync, int shots)
    : PTimer()
    , m_sync(sync)
    , m_shots(shots)
  {
  }

  int GetShots() const
  {
    return m_shots;
  }

protected:
  virtual void OnTimeout()
  {
    m_sync.Signal();
    if (m_shots == 0)
    {
      cerr << "Error! Unexpected call to OnTimeout()!" << endl;
    }
    else if (m_shots > 0)
    {
      if (!IsRunning())
      {
        cerr << "Error! Timer should be running here!" << endl;
      }
      RunContinuous(50);
      --m_shots;
    }
  }
};

void PTimerTest::ContinuousRestartInTimeout()
{
  static const int shotsCount = 20;

  PSyncPoint sync;
  TestTimer4 t(sync, shotsCount);
  for (int i = 0; i < shotsCount; ++i)
  {
    t.RunContinuous(50);
    sync.Wait();
  }
  t.Stop();
  if (t.IsRunning())
  {
    cerr << "Error! Timer should be stopped here!" << endl;
  }
  PThread::Sleep(1000);
  if (t.GetShots() != 0)
    cerr << "Error! Shots count must be 0!" << endl;
}

/////////////////////////////////////////////////////////////////////////////

void PTimerTest::RunSecondTest()
{
  cerr << "Will run the second test, which goes forever (if pwlib works correctly)" << endl
       << "or stops, on detecting an error" << endl
       << " " << endl
       << "This test runs two threads, which continually restart two timer instances " << endl
       << " " << endl
       <<"---There is no output, until an error is detected. All going well, you will have" << endl
       << "to stop this program with Ctrl-C" << endl;

  firstTimer.SetNotifier(PCREATE_NOTIFIER(OnFirstTimerExpired));
  secondTimer.SetNotifier(PCREATE_NOTIFIER(OnSecondTimerExpired));
  thirdTimer = 0;

  PThread::Create(PCREATE_NOTIFIER(RestartFirstTimerMain), 30000,
                  PThread::NoAutoDeleteThread,
                  PThread::NormalPriority);

  PThread::Create(PCREATE_NOTIFIER(RestartSecondTimerMain), 30000,
                  PThread::NoAutoDeleteThread,
                  PThread::NormalPriority);

  PThread::Create(PCREATE_NOTIFIER(RestartThirdTimerMain), 30000,
                  PThread::NoAutoDeleteThread,
                  PThread::NormalPriority);

  PTime restartActive;
  PTimeInterval quietPeriod(4000);

  for (;;) {
    if (restartActivity > 0) {
        restartActive = PTime();
        restartActivity.SetValue(0);
    }
    if ((restartActive + quietPeriod) < PTime()) {
        cerr << "No activity for four seconds. Timers Locked up. PWlib Error" << endl;
        exit(0);
    }
    PThread::Sleep(100);
  }
}

void PTimerTest::OnFirstTimerExpired(PTimer &, INT)
{
  cerr << "The first timer has expired " << endl;
}

void PTimerTest::OnSecondTimerExpired(PTimer &, INT)
{
  cerr << "The second timer has expired " << endl;
}

void PTimerTest::RestartFirstTimerMain(PThread &, INT)
{
  for (;;) {
    firstTimer = PTimeInterval(1900);
    restartActivity.SetValue(1);
    PThread::Sleep(200);
  }
}

void PTimerTest::RestartSecondTimerMain(PThread &, INT)
{
  for (;;) {
    secondTimer = PTimeInterval(2000);
    restartActivity.SetValue(1);
    PThread::Sleep(100);
  }
}


void PTimerTest::RestartThirdTimerMain(PThread &, INT)
{
  static int tmp = 0;
  for (;;) {
    if (thirdTimer == 0)
    {
      thirdTimer = new PTimer(PTimeInterval((rand() % 10) * 500));
      if (rand() % 3)
      {
        delete thirdTimer;
        thirdTimer = 0;
      }
    }
    else
    {
      thirdTimer->SetInterval((rand() % 10) * 500);
    }
    restartActivity.SetValue(1);
    PThread::Sleep(50);
  }
}

class TimerTestThread : public PThread
{
  PCLASSINFO(TimerTestThread, PThread);
  PInt64 m_iteration;
public:
  static PAtomicInteger m_counter;
  typedef std::pair<PTimer, PMutex> TimerPair;
  static std::vector<TimerPair> s_timers;
public:
  TimerTestThread(PInt64 aIterations)
    : PThread(10000, AutoDeleteThread)
    , m_iteration(aIterations)
  {
  }
  static int randomTime(int maxSec)
  {
    return (rand() % maxSec) * 100;
  }
  void Main()
  {
    ++m_counter;
    while (m_iteration--)
    {
      size_t index = rand() % s_timers.size();
      s_timers[index].second.Wait();
      int xxx = rand() % 5;
      if (xxx == 0)
      {
        s_timers[index].first.SetInterval(randomTime(5));
      }
      else if (xxx == 1)
      {
        s_timers[index].first = PTimer(randomTime(5));
      }
      else if (xxx == 2)
      {
        s_timers[index].first.Stop();
      }
      else if (xxx == 3)
      {
        s_timers[index].first.Reset();
      }
      else
      {
        s_timers[index].first.RunContinuous(randomTime(5));
      }
      s_timers[index].second.Signal();
      PThread::Sleep(randomTime(3));
    }
    --m_counter;
  }
};

std::vector<TimerTestThread::TimerPair> TimerTestThread::s_timers;
PAtomicInteger TimerTestThread::m_counter;

void PTimerTest::StressTest()
{
  cout << "First test run 10 timers and randomly change it from different threads" << endl;
  cout << "Be patient it will take some time..." << endl;
  TimerTestThread::s_timers.resize(10);
  std::list<TimerTestThread*> threads;
  for (int i = 0; i < 1000; ++i)
  {
    TimerTestThread* t = new TimerTestThread(500);
    t->Resume();
    threads.push_back(t);
  }

  while (TimerTestThread::m_counter > 0)
    PThread::Sleep(1000);

  threads.clear();
  TimerTestThread::s_timers.clear();
}


////////////////////////////////////////////////////////////////////////////////

class MultiTimerThread
  : public PThread
{
  PCLASSINFO(MultiTimerThread, PThread);

  class Timer : public PTimer
  {
    PTime m_start;
    PInt64 m_expected;
  public:
    Timer(MultiTimerThread* ownThread)
      : m_thread(ownThread)
    { }
    void startRun(PInt64 period)
    {
      m_start = PTime();
      if (period == 0)
        period = 1500;
      m_expected = period;
      SetInterval(period);
    }
    void OnTimeout()
    {
      PInt64 period = (PTime() - m_start).GetMilliSeconds();
      m_thread->m_diff += abs(period - m_expected);
      m_thread->timers--;
    }
    MultiTimerThread* m_thread;
  };
  std::list<Timer*> _timers;
public:
  PInt64 m_diff;
  PAtomicInteger timers;
  static PAtomicInteger threads;
  MultiTimerThread()
    : PThread(10000, AutoDeleteThread)
    , m_diff(0)
  {
  }
  ~MultiTimerThread()
  {
    cout << "Average diff: " << (float(m_diff) / _timers.size()) << endl;
    while (_timers.size() > 0)
    {
      delete _timers.back();
      _timers.pop_back();
    }
    --threads;
  }
  void Main()
  {
    ++threads;
    for (timers = 0; timers < 500; ++timers)
    {
      Timer* t = new Timer(this);
      _timers.push_back(t);
      t->startRun((rand() % 5) * 1000);
    }
    
    while (timers != 0)
    {
      PTRACE(5, "Timers = " << timers <<  ", thread =" << this << endl);
      PThread::Sleep(100);
    }
  }
};

PAtomicInteger MultiTimerThread::threads(0);

void PTimerTest::MultiTimerlTest()
{
  for (int i = 0; i < 25; ++i)
  {
    MultiTimerThread* t = new MultiTimerThread();
    t->Resume();
    PThread::Sleep(200);
  }

  while (MultiTimerThread::threads > 0)
    PThread::Sleep(1000);
}

////////////////////////////////////////////////////////////////////////////////

class SlowTimer
  : public PTimer
{
  bool& m_flag;
  static bool destructed;
  void privateFunc()
  {
    if (destructed)
      PAssert(false, "Destructor already called!!!");
    else
      cout << "Long work finished..." << endl;
  }
public:
  PInt64 interval;

  SlowTimer(bool& _flag)
    : m_flag(_flag)
    , interval((rand() % 2) * 1000)
  {
    destructed = false;
  }
  ~SlowTimer()
  {
    Stop();
    destructed = true;
  }
  virtual void OnTimeout()
  {
    cout << "Simulate long work: " << interval << " ms" << endl;
    PThread::Sleep(interval);
    privateFunc();
    m_flag = true;
  }
};

bool SlowTimer::destructed;

class SlowThread
  : public PThread
{
  PCLASSINFO(SlowThread, PThread);
  PSyncPoint& m_sync;
public:
  SlowThread(PSyncPoint& _sync)
    : PThread(10000, AutoDeleteThread)
    , m_sync(_sync)
  {
  }
  void Main()
  {
    bool flag = false;
    SlowTimer* st = new SlowTimer(flag);
    PTime start;
    st->SetInterval(100);
    PInt64 interval = st->interval;
    // Wait when PTimer starts its OnTimeout...
    PThread::Sleep(100 + (interval / 2));
    delete st;
    if (!flag)
      cout << "OnTimeout didn't called!!!" << endl;
    PInt64 actual = (PTime() - start).GetMilliSeconds();
    if ( actual < interval + 100)
      cout << "Deletion don't wait for finish: " << actual << " < " << interval + 100 << endl;
  }
  ~SlowThread()
  {
    m_sync.Signal();
  }
};

void PTimerTest::LongOnTimeoutTest()
{
  cout << "Create timer and delete it when OnTimeout in progress..." << endl;
  PSyncPoint sync;
  SlowThread* t = new SlowThread(sync);
  t->Resume();
  sync.Wait();
}

void PTimerTest::MassStopTest()
{
  std::list<PTimer*> timers;
  cout << "Creating 10000 timers, be patient..." << endl;
  for (int i = 0; i < 10000; ++i)
  {
    PTimer* t = new PTimer(500 + ((rand() % 10) * 1000));
    timers.push_back(t);
    PThread::Sleep(5);
  }
  while (timers.size() > 0)
  {
    cout << "Delete 5000 timers..." << endl;
    std::list<PTimer*>::iterator it = timers.begin();
    std::list<PTimer*>::iterator endIt = timers.end();
    PTime start;
    for (int i = 0; i < 5000 && it != endIt; ++i)
      (*(it++))->Stop();
    cout << "Average time to stop timers: " << float((PTime() - start).GetMilliSeconds()) << endl;
    for (int i = 0; i < 5000 && timers.begin() != timers.end(); ++i)
    {
      delete *(timers.begin());
      timers.erase(timers.begin());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

class EarlyStopTimerTester
  : public MyTimerTester
{
public:
  EarlyStopTimerTester(PSyncPoint& sync)
    : MyTimerTester(sync)
  {
  }
  void callStop()
  {
    Stop();
    cout << "Stop timer here!!!!" << endl;
  }
  virtual void OnTimeout()
  {
    cout << "Why we are here???" << endl;
    MyTimerTester::OnTimeout();
  }
};

void PTimerTest::StartStopTest()
{
  PSyncPoint sync;
  EarlyStopTimerTester timer(sync);
  cout << "Starting 1 second timer..." << endl;
  timer.SetInterval(1000);
  timer.callStop();
  sync.Wait(2000);
}

void PTimerTest::PullCheck()
{
  PTime then;
  cout << "Starting 5 second timer for poll check..." << endl;
  PTimer timer(5000);
  while (timer.IsRunning())
    PThread::Sleep(25);
  PInt64 duration = (PTime() - then).GetMilliSeconds();
  if (duration < 5000)
    cout << "Too fast - duration = " << duration << " ms" << endl;
  else
    cout << "Finished - duration = " << duration << " ms" << endl;
}

void PTimerTest::CallbackCheck()
{
  PTime then;
  cout << "Starting 5 second timer for callback check..." << endl;
  PSyncPoint sync;
  MyTimerTester timer(sync);
  timer.SetInterval(5000);
  sync.Wait();
  cout << "Finished - duration = " << (PTime() - then).GetMilliSeconds() << " ms" << endl;
}

void PTimerTest::DestroyCheck()
{
  PTimer* t = new PTimer;
  cout << "Create 1 sec timer and destroy it." << endl;
  t->SetInterval(1000);
  delete t;
  PThread::Sleep(3000);
  cout << "No crash detected" << endl;
}

////////////////////////////////////////////////////////////////////////////////

class TimerWithStopInTimeout
  : public MyTimerTester
{
public:
  TimerWithStopInTimeout(PSyncPoint & _sync, PInt64 aTimeout)
    : MyTimerTester(_sync)
  {
    RunContinuous(aTimeout);
  }
  virtual void OnTimeout()
  {
    Stop();
    MyTimerTester::OnTimeout();
  }
};

void PTimerTest::TestStopInTimeout()
{
  PSyncPoint sp;
  TimerWithStopInTimeout timer(sp, 1000);
  sp.Wait();
  if (timer.IsRunning())
    cerr << "Error. Timer must be stopped.";
  // Timer restarts in OnTimeout must be true
  //cout << "timer.IsRunning() ==" << (timer.IsRunning() ? "false" : "true") << endl;
}

class TimerWithStopAndSleepInTimeout
  : public MyTimerTester
{
public:
  TimerWithStopAndSleepInTimeout(PSyncPoint & _sync, PInt64 aTimeout)
    : MyTimerTester(_sync)
  {
    RunContinuous(aTimeout);
  }
  virtual void OnTimeout()
  {
    SetInterval(10000);
    MyTimerTester::OnTimeout();
    PThread::Sleep(5000);
  }
};

void PTimerTest::DoubleStopTest()
{
  /*
  PSyncPoint sp;
  TimerWithStopAndSleepInTimeout timer(sp, 1000);
  sp.Wait(); // Wait for OnTimeout starts
  timer.Stop(); // Stops it. It must wait OnTimeout has finished
  */
}

/////////////////////////////////////////////////////////////////////////////

MyTimer::MyTimer()
{
  exitFlag = NULL;
}

void MyTimer::StartRunning(PSyncPoint * _exitFlag, PINDEX delayMs)
{
  exitFlag = _exitFlag;

  Stop();
  
  SetInterval(delayMs);
  delayPeriod = PTimeInterval(delayMs);
  startTime = PTime();

  PThread * startIt = new TimerOnThread(*this);
  startIt->Resume();
}

void MyTimer::OnTimeout()
{
  PTimeInterval error(3); //provide a 3ms error in return time, to avoid
  //any ounding issue in timers.

  PTimeInterval elapsed = (PTime() - startTime);
  
  if (error + elapsed < delayPeriod)
    PTimerTest::Current().TooSoon(elapsed, delayPeriod);

  exitFlag->Signal();
};

/////////////////////////////////////////////////////////////////////////////

DelayThread::DelayThread(PINDEX _delay, PBoolean _checkTimer)
  : PThread(10000, AutoDeleteThread), delay(_delay), checkTimer(_checkTimer)
{
  PTRACE(5, "Constructor for a auto deleted PTimer test thread");
}    

DelayThread::~DelayThread()
{
  PTRACE(5, "Destructor for a delay thread");
  //This thread must not have a PTRACE statement in the debugger, if it is an autodeleted thread.
  //If a PTRACE statement is here, the PTRACE will fail as the PThread::Current() returns empty.
}

void DelayThread::Main()  
{
  PTRACE(5, "DelayThread\t start now");
  localPTimer.StartRunning(&endMe, delay);

  if (checkTimer) {
    if (!localPTimer.IsRunning())
      cerr << "PTimer has been detected as finishing too soon" << endl;
  }

  endMe.Wait();
  PTRACE(5, "DelayThread\t all finished");
}

/////////////////////////////////////////////////////////////////////////////
TimerOnThread::TimerOnThread(PTimer & _timer)
  : PThread(10000, AutoDeleteThread), timer(_timer)
{
  PTRACE(5, "Constructor for a auto deleted Ptimer On thread.");
}    

void TimerOnThread::Main()  
{
  PTRACE(5, "DelayThread\t start now");
  //timer.Resume();
}

///////////////////////////////////////////////////////////////////////////

void LauncherThread::Main()
{
  PINDEX delay      = PTimerTest::Current().Delay();
  PINDEX interval   = PTimerTest::Current().Interval();
  PBoolean   checkTimer = PTimerTest::Current().CheckTimer();

  while (keepGoing) {
    PThread * thread = new DelayThread(delay, checkTimer);
    thread->Resume();
    PThread::Sleep(interval);
    iteration++;
  }

  return;
}

/////////////////////////////////////////////////////////////////////////////

void UserInterfaceThread::Main()
{
  PConsoleChannel console(PConsoleChannel::StandardInput);
  cout << endl;
  cout << "This program repeatedly create and destroys a PTimer" << endl;
  cout << "Warnings are printed if the PTimer runs too quick" << endl;
  cout<< "This program will end when the user enters \"q\"  " << endl;

  PStringStream help;
  help << "Press : " << endl
       << "         D      average interval between instances " << endl
       << "         H or ? help"                                << endl
       << "         R      report count of threads done"        << endl
       << "         T      time elapsed"                        << endl
       << "         X or Q exit "                               << endl;
 
  cout << endl << help;

  LauncherThread launch;
  launch.Resume();

  console.SetReadTimeout(P_MAX_INDEX);
  for (;;) {
    int ch = console.ReadChar();

    switch (tolower(ch)) {
    case 'd' :
      {
        int i = launch.GetIteration();
        if (i == 0) {
          cout << "Have not completed an iteration yet, so time per iteration is unavailable" << endl;
        } else {
          cout << "Average time per iteration is " << (launch.GetElapsedTime().GetMilliSeconds()/((double) i)) 
               << " milliseconds" << endl;
        }
        cout << "Command ? " << flush;
        break;
      }
    case 'r' :
      cout << "\nHave completed " << launch.GetIteration() << " iterations" << endl;
      cout << "Command ? " << flush;
      break;
    case 't' :
      cout << "\nElapsed time is " << launch.GetElapsedTime() << " (Hours:mins:seconds.millseconds)" << endl;
      cout << "Command ? " << flush;
      break;

    case 'x' :
    case 'q' :
      cout << "Exiting." << endl;
      launch.Terminate();
      launch.WaitForTermination();
      return;
      break;
    case '?' :
    case 'h' :
      cout << help << endl;
      cout << "Command ? " << flush;
    default:
      break;

    } // end switch
  } // end for
}


// End of File ///////////////////////////////////////////////////////////////
