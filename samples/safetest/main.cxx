/*
 * main.cxx
 *
 * PWLib application source file for safetest
 *
 * Main program entry point.
 *
 * Copyright (c) 2006 Indranet Technologies Ltd
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
 * $Log: main.cxx,v $
 * Revision 1.2  2006/02/07 02:02:00  dereksmithies
 * use a more sane method to keep track of the number of running DelayThread instances.
 *
 * Revision 1.1  2006/02/07 01:02:56  dereksmithies
 * Initial release of code to test the PSafeDictionary structure in pwlib.
 * Thanks to Indranet Technologies for supporting this work.
 *
 *
 *
 */

#include "precompile.h"
#include "main.h"
#include "version.h"


PCREATE_PROCESS(SafeTest);

#include  <ptclib/dtmf.h>
#include  <ptclib/random.h>



SafeTest::SafeTest()
  : PProcess("Derek Smithies code factory", "safetest", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}

void SafeTest::Main()
{
  PArgList & args = GetArguments();

  args.Parse(
             "h-help."               "-no-help."
             "d-delay:"              "-no-delay."
	     "c-count:"              "-no-count."
#if PTRACING
             "o-output:"             "-no-output."
             "t-trace."              "-no-trace."
#endif
             "v-version."
  );

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  if (args.HasOption('v')) {
    cout << "Product Name: " << GetName() << endl
         << "Manufacturer: " << GetManufacturer() << endl
         << "Version     : " << GetVersion(TRUE) << endl
         << "System      : " << GetOSName() << '-'
         << GetOSHardware() << ' '
         << GetOSVersion() << endl;
    return;
  }

  if (args.HasOption('h')) {
    PError << "Available options are: " << endl         
           << "-h  or --help        :print this help" << endl
           << "-v  or --version      print version info" << endl
           << "-d  or --delay ##     where ## specifies how many milliseconds the created thread waits for" << endl
	   << "-c  or --count ##     the number of active threads allowed " << endl
#if PTRACING
           << "o-output              output file name for trace" << endl
           << "t-trace.              trace level to use." << endl
#endif
           << endl
           << endl << endl;
    return;
  }

  delay = 2000;
  if (args.HasOption('d'))
    delay = args.GetOptionString('d').AsInteger();

  delay = PMIN(1000000, PMAX(1, delay));
  cout << "Created thread will wait for " << delay << " milliseconds before ending" << endl;
 

  activeCount = 10;
  if (args.HasOption('c'))
    activeCount = args.GetOptionString('c').AsInteger();
  activeCount = PMIN(100, PMAX(1, activeCount));
  cout << "There will be " << activeCount << " threads in operation" << endl;

  garbageCollector = PThread::Create(PCREATE_NOTIFIER(GarbageMain), 30000,
                                     PThread::NoAutoDeleteThread,
                                     PThread::NormalPriority,
                                     "DelayThread Purger");

  UserInterfaceThread ui(*this);
  ui.Resume();
  ui.WaitForTermination();

  exitNow = TRUE;
  garbageCollector->WaitForTermination();
  delete garbageCollector;
  garbageCollector = NULL;
}

void SafeTest::OnReleased(DelayThread & delayThread)
{
  PString id = delayThread.GetId();
  PTRACE(3, "DelayThread " << id << " OnRelease");
  delayThreadsActive.RemoveAt(delayThread.GetId());
  PTRACE(3, "DelayThread " << id << " OnRelease all done");
  --currentSize;
}
    
void SafeTest::AppendRunning(PSafePtr<DelayThread> delayThread, PString id)
{
  ++currentSize;
  PTRACE(3, "Add a delay thread of " << id);
  delayThreadsActive.SetAt(id, delayThread);
}

void SafeTest::DelayThreadsDict::DeleteObject(PObject * object) const
{
  DelayThread * delayThread = PDownCast(DelayThread, object);
  if (delayThread != NULL) {
    PTRACE(3, "Delete DelayThread " << *delayThread);
    delete delayThread;
  }
}

void SafeTest::GarbageMain(PThread &, INT)
{
  while(!exitNow) {
    CollectGarbage();
  }

  while(delayThreadsActive.GetSize() > 0)
    CollectGarbage();
}

void SafeTest::CollectGarbage()
{
  delayThreadsActive.DeleteObjectsToBeRemoved();
}
////////////////////////////////////////////////////////////////////////////

DelayThread::DelayThread(SafeTest &_safeTest, PINDEX _delay)
  : safeTest(_safeTest),
    delay(_delay)
{
  PTRACE(5, "Constructor for a non auto deleted delay thread");
  PThread::Create(PCREATE_NOTIFIER(DelayThreadMain), 30000,
		  PThread::AutoDeleteThread,
		  PThread::NormalPriority);
}    


DelayThread::~DelayThread()
{
  PTRACE(5, "Destructor for a delay thread");
}

void DelayThread::DelayThreadMain(PThread &thisThread, INT)  
{
  id = thisThread.GetThreadName();
  PTRACE(3, "DelayThread starting " << *this);
  safeTest.AppendRunning(this, id);
  PThread::Sleep(delay);
  Release();
  PTRACE(3, "DelayThread finished " << *this);
}

void DelayThread::Release()
{
 // Add a reference for the thread we are about to start
  SafeReference();
  PThread::Create(PCREATE_NOTIFIER(OnReleaseThreadMain), 10000,
                  PThread::AutoDeleteThread,
                  PThread::NormalPriority,
		  "Release %X");
}

void DelayThread::OnReleaseThreadMain(PThread &, INT)
{
  safeTest.OnReleased(*this);
  SafeDereference();
}

void DelayThread::PrintOn(ostream & strm) const
{
  strm << id << " ";
}
///////////////////////////////////////////////////////////////////////////
  
LauncherThread::LauncherThread(SafeTest &_safeTest)
  : PThread(10000, NoAutoDeleteThread),
    safeTest(_safeTest)
{ 
  iteration = 0; 
  keepGoing = TRUE; 
}

void LauncherThread::Main()
{
  PINDEX delay = safeTest.Delay();
  PINDEX count = safeTest.ActiveCount();
  while(keepGoing) {
    while(safeTest.CurrentSize() < count) {
      iteration++;
      new DelayThread(safeTest, delay);
    }    
  }
}

////////////////////////////////////////////////////////////////////////////////

void UserInterfaceThread::Main()
{
  PConsoleChannel console(PConsoleChannel::StandardInput);
  cout << "This program will repeatedly create and destroy a thread until terminated from the console" << endl;

  PStringStream help;
  help << "Press : " << endl
       << "         D      average Delay time of each thread" << endl
       << "         H or ? help"                              << endl
       << "         R      report count of threads done"      << endl
       << "         T      time elapsed"                      << endl
       << "         X or Q exit "                             << endl;
 
  cout << endl << help;

  LauncherThread launch(safeTest);
  launch.Resume();

  console.SetReadTimeout(P_MAX_INDEX);
  for (;;) {
    char ch = console.ReadChar();

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
