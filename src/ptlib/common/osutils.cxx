/*
 * osutils.cxx
 *
 * Operating System utilities.
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
 *
 * $Log: osutils.cxx,v $
 * Revision 1.129  2000/02/17 11:34:28  robertj
 * Changed PTRACE output to help line up text after filename output.
 *
 * Revision 1.128  2000/01/06 14:09:42  robertj
 * Fixed problems with starting up timers,losing up to 10 seconds
 *
 * Revision 1.127  1999/10/19 09:21:30  robertj
 * Added functions to get current trace options and level.
 *
 * Revision 1.126  1999/10/14 08:08:27  robertj
 * Fixed problem, assuring millisecond accuracy in timestamp of trace output.
 *
 * Revision 1.125  1999/09/14 13:02:52  robertj
 * Fixed PTRACE to PSYSTEMLOG conversion problem under Unix.
 *
 * Revision 1.124  1999/09/13 13:15:07  robertj
 * Changed PTRACE so will output to system log in PServiceProcess applications.
 *
 * Revision 1.123  1999/08/22 12:54:35  robertj
 * Fixed warnings about inlines on older GNU compiler
 *
 * Revision 1.122  1999/06/23 14:19:46  robertj
 * Fixed core dump problem with SIGINT/SIGTERM terminating process.
 *
 * Revision 1.121  1999/06/14 07:59:38  robertj
 * Enhanced tracing again to add options to trace output (timestamps etc).
 *
 * Revision 1.120  1999/04/26 08:06:51  robertj
 * Added missing function in cooperative threading.
 *
 * Revision 1.119  1999/03/01 13:51:30  craigs
 * Fixed ugly little bug in the cooperative multithreading that meant that threads blocked
 * on timers didn't always get rescheduled.
 *
 * Revision 1.118  1999/02/23 10:13:31  robertj
 * Changed trace to only diplay filename and not whole path.
 *
 * Revision 1.117  1999/02/23 07:11:27  robertj
 * Improved trace facility adding trace levels and #define to remove all trace code.
 *
 * Revision 1.116  1998/11/30 12:45:54  robertj
 * Fissioned into pchannel.cxx and pconfig.cxx
 *
 * Revision 1.115  1998/11/24 01:17:33  robertj
 * Type discrepency between declaration and definition for PFile::SetPosition
 *
 * Revision 1.114  1998/11/06 02:37:53  robertj
 * Fixed the fix for semaphore timeout race condition.
 *
 * Revision 1.113  1998/11/03 10:52:19  robertj
 * Fixed bug in semaphores with timeout saying timed out when really signalled.
 *
 * Revision 1.112  1998/11/03 03:44:05  robertj
 * Fixed missng strings on multiple parameters of same letter.
 *
 * Revision 1.111  1998/11/02 10:13:01  robertj
 * Removed GNU warning.
 *
 * Revision 1.110  1998/11/01 04:56:53  robertj
 * Added BOOl return value to Parse() to indicate there are parameters available.
 *
 * Revision 1.109  1998/10/31 14:02:20  robertj
 * Removed StartImmediate capability as causes race condition in preemptive version.
 *
 * Revision 1.108  1998/10/31 12:47:10  robertj
 * Added conditional mutex and read/write mutex thread synchronisation objects.
 *
 * Revision 1.107  1998/10/30 12:24:15  robertj
 * Added ability to get all key values as a dictionary.
 * Fixed warnings in GNU C.
 *
 * Revision 1.106  1998/10/30 11:22:15  robertj
 * Added constructors that take strings as well as const char *'s.
 *
 * Revision 1.105  1998/10/30 05:25:09  robertj
 * Allow user to shift past some arguments before parsing for the first time.
 *
 * Revision 1.104  1998/10/29 05:35:17  robertj
 * Fixed porblem with GetCount() == 0 if do not call Parse() function.
 *
 * Revision 1.103  1998/10/28 03:26:43  robertj
 * Added multi character arguments (-abc style) and options precede parameters mode.
 *
 * Revision 1.102  1998/10/28 00:59:49  robertj
 * New improved argument parsing.
 *
 * Revision 1.101  1998/10/19 00:19:59  robertj
 * Moved error and trace stream functions to common code.
 *
 * Revision 1.100  1998/10/18 14:28:45  robertj
 * Renamed argv/argc to eliminate accidental usage.
 *
 * Revision 1.99  1998/10/13 14:06:28  robertj
 * Complete rewrite of memory leak detection code.
 *
 * Revision 1.98  1998/09/24 07:23:54  robertj
 * Moved structured fiel into separate module so don't need silly implementation file for GNU C.
 *
 * Revision 1.97  1998/09/23 06:22:24  robertj
 * Added open source copyright license.
 *
 * Revision 1.96  1998/06/13 15:11:56  robertj
 * Added stack check in Yield().
 * Added immediate schedule of semaphore timeout thread.
 *
 * Revision 1.95  1998/05/30 13:28:18  robertj
 * Changed memory check code so global statics are not included in leak check.
 * Fixed deadlock in cooperative threading.
 * Added PSyncPointAck class.
 *
 * Revision 1.94  1998/05/25 09:05:56  robertj
 * Fixed close of channels on destruction.
 *
 * Revision 1.93  1998/04/07 13:33:33  robertj
 * Changed startup code to support PApplication class.
 *
 * Revision 1.92  1998/03/29 06:16:45  robertj
 * Rearranged initialisation sequence so PProcess descendent constructors can do "things".
 *
 * Revision 1.91  1998/03/20 03:18:17  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.90  1998/02/05 13:33:12  robertj
 * Fixed close of non-autodelete PIndirectChannels
 *
 * Revision 1.89  1998/02/03 06:19:55  robertj
 * Added new function to read a block with minimum number of bytes.
 *
 * Revision 1.88  1998/01/26 00:47:13  robertj
 * Added functions to get/set 64bit integers from a PConfig.
 *
 * Revision 1.87  1998/01/04 07:22:16  robertj
 * Fixed bug in thread deletion not removing it from active thread list.
 *
 * Revision 1.86  1997/10/10 10:41:22  robertj
 * Fixed problem with cooperative threading and Sleep() function returning immediately.
 *
 * Revision 1.85  1997/08/28 12:49:00  robertj
 * Fixed possible assert on exit of application.
 *
 * Revision 1.84  1997/07/08 13:08:12  robertj
 * DLL support.
 *
 * Revision 1.83  1997/04/27 05:50:15  robertj
 * DLL support.
 *
 * Revision 1.82  1997/02/09 04:05:56  robertj
 * Changed PProcess::Current() from pointer to reference.
 *
 * Revision 1.81  1997/02/05 11:51:42  robertj
 * Changed current process function to return reference and validate objects descendancy.
 *
 * Revision 1.80  1996/12/21 05:54:38  robertj
 * Fixed possible deadlock in timers.
 *
 * Revision 1.79  1996/12/05 11:44:22  craigs
 * Made indirect close from different thread less likely to have
 * race condition
 *
 * Revision 1.78  1996/11/30 12:08:42  robertj
 * Removed extraneous compiler warning.
 *
 * Revision 1.77  1996/11/10 21:05:43  robertj
 * Fixed bug of missing flush in close of indirect channel.
 *
 * Revision 1.76  1996/10/08 13:07:07  robertj
 * Fixed bug in indirect channel being reopened double deleting subchannel.
 *
 * Revision 1.75  1996/09/14 13:09:37  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.74  1996/08/11 06:53:04  robertj
 * Fixed bug in Sleep() function (nonpreemptive version).
 *
 * Revision 1.73  1996/07/27 04:12:09  robertj
 * Fixed bug in timer thread going into busy loop instead of blocking.
 *
 * Revision 1.72  1996/07/15 10:36:12  robertj
 * Fixed bug in timer on startup, getting LARGE times timing out prematurely.
 *
 * Revision 1.71  1996/06/28 13:22:43  robertj
 * Rewrite of timers to make OnTimeout more thread safe.
 *
 * Revision 1.70  1996/06/13 13:31:05  robertj
 * Rewrite of auto-delete threads, fixes Windows95 total crash.
 *
 * Revision 1.69  1996/06/03 10:01:31  robertj
 * Fixed GNU support bug fix for the fix.
 *
 * Revision 1.68  1996/06/01 05:03:37  robertj
 * Fixed GNU compiler having difficulty with PTimeInterval *this.
 *
 * Revision 1.67  1996/05/26 03:46:56  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.66  1996/05/23 23:05:07  robertj
 * Fixed process filename on MSOS platforms.
 *
 * Revision 1.65  1996/05/23 09:56:57  robertj
 * Added mutex to timer list.
 *
 * Revision 1.64  1996/05/18 09:18:33  robertj
 * Added mutex to timer list.
 *
 * Revision 1.63  1996/05/09 12:19:00  robertj
 * Resolved C++ problems with 64 bit PTimeInterval for Mac platform.
 *
 * Revision 1.62  1996/04/14 02:53:34  robertj
 * Split serial and pipe channel into separate compilation units for Linux executable size reduction.
 *
 * Revision 1.61  1996/04/10 12:51:29  robertj
 * Fixed startup race condtion in timer thread.
 *
 * Revision 1.60  1996/04/09 03:32:58  robertj
 * Fixed bug in config GetTime() cannot use PTime(0) in western hemisphere.
 *
 * Revision 1.59  1996/04/02 11:29:19  robertj
 * Eliminated printing of patch level in version when there isn't one.
 *
 * Revision 1.58  1996/03/31 09:06:14  robertj
 * Fixed WriteString() so works with sockets.
 * Changed PPipeSokcet argument string list to array.
 *
 * Revision 1.57  1996/03/16 04:51:50  robertj
 * Fixed yet another bug in the scheduler.
 *
 * Revision 1.56  1996/03/12 11:30:50  robertj
 * Moved PProcess destructor to platform dependent code.
 *
 * Revision 1.55  1996/03/05 14:05:51  robertj
 * Fixed some more bugs in scheduling.
 *
 * Revision 1.54  1996/03/04 12:22:46  robertj
 * Fixed threading for unix stack check and loop list start point.
 *
 * Revision 1.53  1996/03/03 07:39:51  robertj
 * Fixed bug in thread scheduler for correct termination of "current" thread.
 *
 * Revision 1.52  1996/03/02 03:24:48  robertj
 * Changed timer thread to update timers periodically, this allows timers to be
 *    views dynamically by other threads.
 * Added automatic deletion of thread object instances on thread completion.
 *
 * Revision 1.51  1996/02/25 11:15:27  robertj
 * Added platform dependent Construct function to PProcess.
 *
 * Revision 1.50  1996/02/25 03:09:46  robertj
 * Added consts to all GetXxxx functions in PConfig.
 *
 * Revision 1.49  1996/02/15 14:44:09  robertj
 * Used string constructor for PTime, more "efficient".
 *
 * Revision 1.48  1996/02/13 12:59:30  robertj
 * Changed GetTimeZone() so can specify standard/daylight time.
 * Split PTime into separate module after major change to ReadFrom().
 *
 * Revision 1.47  1996/02/08 12:26:55  robertj
 * Changed time for full support of time zones.
 *
 * Revision 1.46  1996/02/03 11:06:49  robertj
 * Added string constructor for times, parses date/time from string.
 *
 * Revision 1.45  1996/01/28 14:09:39  robertj
 * Fixed bug in time reading function for dates before 1980.
 * Fixed bug in time reading, was out by one month.
 * Added time functions to PConfig.
 *
 * Revision 1.44  1996/01/28 02:52:04  robertj
 * Added assert into all Compare functions to assure comparison between compatible objects.
 *
 * Revision 1.43  1996/01/23 13:16:30  robertj
 * Mac Metrowerks compiler support.
 * Fixed timers so background thread not created if a windows app.
 *
 * Revision 1.42  1996/01/03 23:15:39  robertj
 * Fixed some PTime bugs.
 *
 * Revision 1.41  1996/01/03 11:09:35  robertj
 * Added Universal Time and Time Zones to PTime class.
 *
 * Revision 1.39  1995/12/23 03:40:40  robertj
 * Changed version number system
 *
 * Revision 1.38  1995/12/10 11:41:12  robertj
 * Added extra user information to processes and applications.
 * Implemented timer support in text only applications with platform threads.
 * Fixed bug in non-platform threads and semaphore timeouts.
 *
 * Revision 1.37  1995/11/21 11:50:57  robertj
 * Added timeout on semaphore wait.
 *
 * Revision 1.36  1995/11/09 12:22:58  robertj
 * Fixed bug in stream when reading an FF (get EOF).
 *
 * Revision 1.35  1995/07/31 12:09:25  robertj
 * Added semaphore class.
 * Removed PContainer from PChannel ancestor.
 *
 * Revision 1.34  1995/06/04 12:41:08  robertj
 * Fixed bug in accessing argument strings with no argument.
 *
 * Revision 1.33  1995/04/25 11:30:06  robertj
 * Fixed Borland compiler warnings.
 *
 * Revision 1.32  1995/04/22 00:51:00  robertj
 * Changed file path strings to use PFilePath object.
 * Changed semantics of Rename().
 *
 * Revision 1.31  1995/04/02 09:27:31  robertj
 * Added "balloon" help.
 *
 * Revision 1.30  1995/04/01 08:30:58  robertj
 * Fixed bug in timeout code of timers.
 *
 * Revision 1.29  1995/01/27 11:15:17  robertj
 * Removed enum to int warning from GCC.
 *
 * Revision 1.28  1995/01/18  09:02:43  robertj
 * Added notifier to timer.
 *
 * Revision 1.27  1995/01/15  04:57:15  robertj
 * Implemented PTime::ReadFrom.
 * Fixed flush of iostream at end of file.
 *
 * Revision 1.26  1995/01/11  09:45:14  robertj
 * Documentation and normalisation.
 *
 * Revision 1.25  1995/01/10  11:44:15  robertj
 * Removed PString parameter in stdarg function for GNU C++ compatibility.
 *
 * Revision 1.24  1995/01/09  12:31:51  robertj
 * Removed unnecesary return value from I/O functions.
 *
 * Revision 1.23  1994/12/12  10:09:24  robertj
 * Fixed flotain point configuration variable format.
 *
 * Revision 1.22  1994/11/28  12:38:23  robertj
 * Async write functions should have const pointer.
 *
 * Revision 1.21  1994/10/30  11:36:58  robertj
 * Fixed missing space in tine format string.
 *
 * Revision 1.20  1994/10/23  03:46:41  robertj
 * Shortened OS error assert.
 *
 * Revision 1.19  1994/09/25  10:51:04  robertj
 * Fixed error conversion code to use common function.
 * Added pipe channel.
 *
 * Revision 1.18  1994/08/21  23:43:02  robertj
 * Moved meta-string transmitter from PModem to PChannel.
 * Added SuspendBlock state to cooperative multi-threading to fix logic fault.
 * Added "force" option to Remove/Rename etc to override write protection.
 * Added common entry point to convert OS error to PChannel error.
 *
 * Revision 1.17  1994/08/04  12:57:10  robertj
 * Changed CheckBlock() to better name.
 * Moved timer porcessing so is done at every Yield().
 *
 * Revision 1.16  1994/08/01  03:39:42  robertj
 * Fixed temporary variable problem with GNU C++
 *
 * Revision 1.15  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.14  1994/07/25  03:39:22  robertj
 * Fixed problems with C++ temporary variables.
 *
 * Revision 1.13  1994/07/21  12:33:49  robertj
 * Moved cooperative threads to common.
 *
 * Revision 1.12  1994/07/17  10:46:06  robertj
 * Fixed timer bug.
 * Moved handle from file to channel.
 * Changed args to use container classes.
 *
 * Revision 1.11  1994/07/02  03:03:49  robertj
 * Time interval and timer redesign.
 *
 * Revision 1.10  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.9  1994/04/20  12:17:44  robertj
 * assert changes
 *
 * Revision 1.8  1994/04/01  14:05:06  robertj
 * Text file streams
 *
 * Revision 1.7  1994/03/07  07:47:00  robertj
 * Major upgrade
 *
 * Revision 1.6  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.5  1993/12/31  06:53:02  robertj
 * Made inlines optional for debugging purposes.
 *
 * Revision 1.4  1993/12/29  04:41:26  robertj
 * Mac port.
 *
 * Revision 1.3  1993/11/20  17:26:28  robertj
 * Removed separate osutil.h
 *
 * Revision 1.2  1993/08/31  03:38:02  robertj
 * G++ needs explicit casts for char * / void * interchange.
 *
 * Revision 1.1  1993/08/27  18:17:47  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <ptlib/svcproc.h>

#include <ctype.h>


static ostream * PErrorStream = &cerr;

ostream & PGetErrorStream()
{
  return *PErrorStream;
}


void PSetErrorStream(ostream * s)
{
  PErrorStream = s != NULL ? s : &cerr;
}


///////////////////////////////////////////////////////////////////////////////

ostream * PTrace::Stream = &cerr;
unsigned PTrace::Options = PTrace::FileAndLine;
unsigned PTrace::LevelThreshold = 0;
unsigned PTrace::Block::IndentLevel = 0;
static PTimeInterval ApplicationStartTick = PTimer::Tick();

void PTrace::SetStream(ostream * s)
{
  Stream = s != NULL ? s : &cerr;
}


void PTrace::SetOptions(unsigned options)
{
  Options |= options;
}


void PTrace::ClearOptions(unsigned options)
{
  Options &= ~options;
}


unsigned PTrace::GetOptions()
{
  return Options;
}


void PTrace::SetLevel(unsigned level)
{
  LevelThreshold = level;
}


unsigned PTrace::GetLevel()
{
  return LevelThreshold;
}


BOOL PTrace::CanTrace(unsigned level)
{
  return level <= LevelThreshold;
}


static PMutex PTraceMutex;

ostream & PTrace::Begin(unsigned level, const char * fileName, int lineNum)
{
  PTraceMutex.Wait();

  if ((Options&SystemLogStream) != 0) {
    unsigned lvl = level+PSystemLog::Warning;
    if (lvl >= PSystemLog::NumLogLevels)
      lvl = PSystemLog::NumLogLevels-1;
    ((PSystemLog*)Stream)->SetLevel((PSystemLog::Level)lvl);
  }
  else {
    if ((Options&DateAndTime) != 0) {
      PTime now;
      *Stream << now.AsString("yyyy/MM/dd hh:mm:ss\t");
    }

    if ((Options&Timestamp) != 0)
      *Stream << setprecision(3) << setw(10) << (PTimer::Tick()-ApplicationStartTick) << '\t';

    if ((Options&Thread) != 0)
      *Stream << hex << setfill('0')
              << setw(7) << (unsigned)PThread::Current()
              << dec << setfill(' ') << '\t';
  }

  if ((Options&TraceLevel) != 0)
    *Stream << level << '\t';

  if ((Options&FileAndLine) != 0 && fileName != NULL) {
    const char * file = strrchr(fileName, '/');
    if (file != NULL)
      file++;
    else {
      file = strrchr(fileName, '\\');
      if (file != NULL)
        file++;
      else
        file = fileName;
    }

    *Stream << setw(16) << file << '(' << lineNum << ")\t";
  }

  return *Stream;
}


ostream & PTrace::End(ostream & s)
{
  if ((Options&SystemLogStream) != 0)
    s.flush();
  else
    s << endl;

  PTraceMutex.Signal();

  return s;
}


PTrace::Block::Block(const char * fileName, int lineNum, const char * traceName)
{
  file = fileName;
  line = lineNum;
  name = traceName;

  IndentLevel += 2;

  if ((Options&Blocks) != 0) {
    ostream & s = PTrace::Begin(1, file, line);
    for (unsigned i = 0; i < IndentLevel; i++)
      s << '=';
    s << "> " << name << PTrace::End;
  }
}


PTrace::Block::~Block()
{
  if ((Options&Blocks) != 0) {
    ostream & s = PTrace::Begin(1, file, line);
    s << '<';
    for (unsigned i = 0; i < IndentLevel; i++)
      s << '=';
    s << ' ' << name << PTrace::End;
  }

  IndentLevel -= 2;
}


///////////////////////////////////////////////////////////////////////////////
// PDirectory

void PDirectory::CloneContents(const PDirectory * d)
{
  CopyContents(*d);
}


///////////////////////////////////////////////////////////////////////////////
// PTimer

PTimer::PTimer(long millisecs, int seconds, int minutes, int hours, int days)
  : resetTime(millisecs, seconds, minutes, hours, days)
{
  state = Stopped;
  timeoutThread = NULL;
  StartRunning(TRUE);
}


PTimer::PTimer(const PTimeInterval & time)
  : resetTime(time)
{
  state = Stopped;
  timeoutThread = NULL;
  StartRunning(TRUE);
}


PTimer & PTimer::operator=(DWORD milliseconds)
{
  resetTime = (PInt64)milliseconds;
  StartRunning(oneshot);
  return *this;
}


PTimer & PTimer::operator=(const PTimeInterval & time)
{
  resetTime = time;
  StartRunning(oneshot);
  return *this;
}


PTimer::~PTimer()
{
  PAssert(timeoutThread == NULL || PThread::Current() != timeoutThread, "Timer destroyed in OnTimeout()");
  if (IsRunning())
    PProcess::Current().GetTimerList()->RemoveTimer(this);
}


void PTimer::RunContinuous(const PTimeInterval & time)
{
  resetTime = time;
  StartRunning(FALSE);
}


void PTimer::StartRunning(BOOL once)
{
  if (IsRunning() && timeoutThread == NULL)
    PProcess::Current().GetTimerList()->RemoveTimer(this);

  PTimeInterval::operator=(resetTime);
  oneshot = once;
  state = (*this) != 0 ? Starting : Stopped;

  if (IsRunning() && timeoutThread == NULL)
    PProcess::Current().GetTimerList()->AppendTimer(this);
}


void PTimer::Stop()
{
  if (IsRunning() && timeoutThread == NULL)
    PProcess::Current().GetTimerList()->RemoveTimer(this);
  state = Stopped;
  SetInterval(0);
}


void PTimer::Pause()
{
  if (IsRunning()) {
    if (timeoutThread == NULL)
      PProcess::Current().GetTimerList()->RemoveTimer(this);
    state = Paused;
  }
}


void PTimer::Resume()
{
  if (state == Paused) {
    if (timeoutThread == NULL)
      PProcess::Current().GetTimerList()->AppendTimer(this);
    state = Starting;
  }
}


void PTimer::OnTimeout()
{
  if (!callback.IsNULL())
    callback(*this, IsRunning());
}


BOOL PTimer::Process(const PTimeInterval & delta, PTimeInterval & minTimeLeft)
{
  if (state == Starting) {
    state = Running;
    if (resetTime < minTimeLeft)
      minTimeLeft = resetTime;
    return FALSE;
  }

  operator-=(delta);

  if (milliseconds > 0) {
    if (milliseconds < minTimeLeft.GetMilliSeconds())
      minTimeLeft = milliseconds;
    return FALSE;
  }

  timeoutThread = PThread::Current();
  if (oneshot) {
    operator=(PTimeInterval(0));
    state = Stopped;
  }
  else {
    operator=(resetTime);
    if (resetTime < minTimeLeft)
      minTimeLeft = resetTime;
  }

  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// PTimerList

PTimerList::PTimerList()
{
  DisallowDeleteObjects();
}


void PTimerList::AppendTimer(PTimer * timer)
{
  mutex.Wait();
  PInternalTimerList::InsertAt(0, timer);
  mutex.Signal();
#if defined(P_PLATFORM_HAS_THREADS)
  PProcess::Current().SignalTimerChange();
#endif
}


void PTimerList::RemoveTimer(PTimer * timer)
{
  mutex.Wait();
  PInternalTimerList::Remove(timer);
  mutex.Signal();
#if defined(P_PLATFORM_HAS_THREADS)
  PProcess::Current().SignalTimerChange();
#endif
}


PTimeInterval PTimerList::Process()
{
  PINDEX i;
  PTimeInterval minTimeLeft = PMaxTimeInterval;
  PInternalTimerList timeouts;
  timeouts.DisallowDeleteObjects();

  mutex.Wait();
  PTimeInterval now = PTimer::Tick();
  PTimeInterval sampleTime;
  if (lastSample == 0)
    sampleTime = 0;
  else {
    sampleTime = now - lastSample;
    if (now < lastSample)
      sampleTime += PMaxTimeInterval;
  }
  lastSample = now;

  for (i = 0; i < GetSize(); i++)
    if ((*this)[i].Process(sampleTime, minTimeLeft))
      timeouts.Append(RemoveAt(i--));
  mutex.Signal();

  for (i = 0; i < timeouts.GetSize(); i++)
    timeouts[i].OnTimeout();

  mutex.Wait();
  for (i = 0; i < timeouts.GetSize(); i++) {
    timeouts[i].timeoutThread = NULL;
    if (timeouts[i].IsRunning())
      Append(timeouts.GetAt(i));
  }
  mutex.Signal();

  return minTimeLeft;
}


///////////////////////////////////////////////////////////////////////////////
// PArgList

PArgList::PArgList(const char * theArgStr,
                   const char * theArgumentSpec,
                   BOOL optionsBeforeParams)
{
  // get the program arguments
  if (theArgStr != NULL)
    SetArgs(theArgStr);

  // if we got an argument spec - so process them
  if (theArgumentSpec != NULL)
    Parse(theArgumentSpec, optionsBeforeParams);
}


PArgList::PArgList(const PString & theArgStr,
                   const char * argumentSpecPtr,
                   BOOL optionsBeforeParams)
{
  // get the program arguments
  SetArgs(theArgStr);

  // if we got an argument spec - so process them
  if (argumentSpecPtr != NULL)
    Parse(argumentSpecPtr, optionsBeforeParams);
}


PArgList::PArgList(const PString & theArgStr,
                   const PString & argumentSpecStr,
                   BOOL optionsBeforeParams)
{
  // get the program arguments
  SetArgs(theArgStr);

  // if we got an argument spec - so process them
  Parse(argumentSpecStr, optionsBeforeParams);
}


PArgList::PArgList(int theArgc, char ** theArgv,
                   const char * theArgumentSpec,
                   BOOL optionsBeforeParams)
{
  // get the program arguments
  SetArgs(theArgc, theArgv);

  // if we got an argument spec - so process them
  if (theArgumentSpec != NULL)
    Parse(theArgumentSpec, optionsBeforeParams);
}


PArgList::PArgList(int theArgc, char ** theArgv,
                   const PString & theArgumentSpec,
                   BOOL optionsBeforeParams)
{
  // get the program name and path
  SetArgs(theArgc, theArgv);
  // we got an argument spec - so process them
  Parse(theArgumentSpec, optionsBeforeParams);
}


void PArgList::SetArgs(const PString & argStr)
{
  argumentArray.SetSize(0);

  const char * str = argStr;

  for (;;) {
    while (isspace(*str)) // Skip leading whitespace
      str++;
    if (*str == '\0')
      break;

    PString & arg = argumentArray[argumentArray.GetSize()];
    while (*str != '\0' && !isspace(*str)) {
      switch (*str) {
        case '"' :
          str++;
          while (*str != '\0' && *str != '"')
            arg += *str++;
          if (*str != '\0')
            str++;
          break;

        case '\'' :
          str++;
          while (*str != '\0' && *str != '\'')
            arg += *str++;
          if (*str != '\0')
            str++;
          break;

        default :
          if (str[0] == '\\' && str[1] != '\0')
            str++;
          arg += *str++;
      }
    }
  }

  SetArgs(argumentArray);
}


void PArgList::SetArgs(const PStringArray & theArgs)
{
  argumentArray = theArgs;
  shift = 0;
  optionLetters = "";
  optionNames.SetSize(0);
  parameterIndex.SetSize(argumentArray.GetSize());
  for (PINDEX i = 0; i < argumentArray.GetSize(); i++)
    parameterIndex[i] = i;
}


BOOL PArgList::Parse(const char * spec, BOOL optionsBeforeParams)
{
  PAssertNULL(spec);

  // Find starting point, start at shift if first Parse() call.
  PINDEX arg = optionLetters.IsEmpty() ? shift : 0;

  // If not in parse all mode, have been parsed before, and had some parameters
  // from last time, then start argument parsing somewhere along instead of start.
  if (optionsBeforeParams && !optionLetters && parameterIndex.GetSize() > 0)
    arg = parameterIndex[parameterIndex.GetSize()-1] + 1;

  // Parse the option specification
  optionLetters = "";
  optionNames.SetSize(0);
  PIntArray canHaveOptionString;

  PINDEX codeCount = 0;
  while (*spec != '\0') {
    if (*spec == '-')
      optionLetters += ' ';
    else
      optionLetters += *spec++;
    if (*spec == '-') {
      const char * base = ++spec;
      while (*spec != '\0' && *spec != '.' && *spec != ':' && *spec != ';')
        spec++;
      optionNames[codeCount] = PString(base, spec-base);
      if (*spec == '.')
        spec++;
    }
    if (*spec == ':' || *spec == ';') {
      canHaveOptionString.SetSize(codeCount+1);
      canHaveOptionString[codeCount] = *spec == ':' ? 2 : 1;
      spec++;
    }
    codeCount++;
  }

  // Clear and reset size of option information
  optionCount.SetSize(0);
  optionCount.SetSize(codeCount);
  optionString.SetSize(0);
  optionString.SetSize(codeCount);

  // Clear parameter indexes
  parameterIndex.SetSize(0);
  shift = 0;

  // Now work through the arguments and split out the options
  PINDEX param = 0;
  BOOL hadMinusMinus = FALSE;
  while (arg < argumentArray.GetSize()) {
    const PString & argStr = argumentArray[arg];
    if (hadMinusMinus || argStr[0] != '-' || argStr[1] == '\0') {
      // have a parameter string
      parameterIndex.SetSize(param+1);
      parameterIndex[param++] = arg;
    }
    else if (optionsBeforeParams && parameterIndex.GetSize() > 0)
      break;
    else if (argStr == "--") // ALL remaining args are parameters not options
      hadMinusMinus = TRUE;
    else if (argStr[1] == '-')
      ParseOption(optionNames.GetValuesIndex(argStr.Mid(2)), 0, arg, canHaveOptionString);
    else {
      for (PINDEX i = 1; i < argStr.GetLength(); i++)
        if (ParseOption(optionLetters.Find(argStr[i]), i+1, arg, canHaveOptionString))
          break;
    }

    arg++;
  }

  return param > 0;
}


BOOL PArgList::ParseOption(PINDEX idx, PINDEX offset, PINDEX & arg,
                           const PIntArray & canHaveOptionString)
{
  if (idx == P_MAX_INDEX) {
    UnknownOption(argumentArray[arg]);
    return FALSE;
  }

  optionCount[idx]++;
  if (canHaveOptionString[idx] == 0)
    return FALSE;

  if (!optionString[idx])
    optionString[idx] += '\n';

  if (offset != 0 &&
        (canHaveOptionString[idx] == 1 || argumentArray[arg][offset] != '\0')) {
    optionString[idx] += argumentArray[arg].Mid(offset);
    return TRUE;
  }

  if (++arg >= argumentArray.GetSize())
    return FALSE;

  optionString[idx] += argumentArray[arg];
  return TRUE;
}


PINDEX PArgList::GetOptionCount(char option) const
{
  return GetOptionCountByIndex(optionLetters.Find(option));
}


PINDEX PArgList::GetOptionCount(const char * option) const
{
  return GetOptionCountByIndex(optionNames.GetValuesIndex(PString(option)));
}


PINDEX PArgList::GetOptionCount(const PString & option) const
{
  return GetOptionCountByIndex(optionNames.GetValuesIndex(option));
}


PINDEX PArgList::GetOptionCountByIndex(PINDEX idx) const
{
  if (idx < optionCount.GetSize())
    return optionCount[idx];

  return 0;
}


PString PArgList::GetOptionString(char option, const char * dflt) const
{
  return GetOptionStringByIndex(optionLetters.Find(option), dflt);
}


PString PArgList::GetOptionString(const char * option, const char * dflt) const
{
  return GetOptionStringByIndex(optionNames.GetValuesIndex(PString(option)), dflt);
}


PString PArgList::GetOptionString(const PString & option, const char * dflt) const
{
  return GetOptionStringByIndex(optionNames.GetValuesIndex(option), dflt);
}


PString PArgList::GetOptionStringByIndex(PINDEX idx, const char * dflt) const
{
  if (idx < optionString.GetSize() && optionString.GetAt(idx) != NULL)
    return optionString[idx];

  if (dflt != NULL)
    return dflt;

  return PString();
}


PString PArgList::GetParameter(PINDEX num) const
{
  int idx = shift+(int)num;
  if (idx >= 0 && idx < (int)parameterIndex.GetSize())
    return argumentArray[parameterIndex[idx]];

  IllegalArgumentIndex(idx);
  return PString();
}


void PArgList::Shift(int sh) 
{
  shift += sh;
  if (shift < 0)
    shift = 0;
  else if (shift >= (int)parameterIndex.GetSize())
    shift = parameterIndex.GetSize() - 1;
}


void PArgList::IllegalArgumentIndex(PINDEX idx) const
{
  PError << "attempt to access undefined argument at index "
         << idx << endl;
}
 

void PArgList::UnknownOption(const PString & option) const
{
  PError << "unknown option \"" << option << "\"\n";
}


void PArgList::MissingArgument(const PString & option) const
{
  PError << "option \"" << option << "\" requires argument\n";
}


///////////////////////////////////////////////////////////////////////////////
// PProcess

static PProcess * PProcessInstance;
int PProcess::p_argc;
char ** PProcess::p_argv;
char ** PProcess::p_envp;

#ifndef P_PLATFORM_HAS_THREADS
static BOOL PProcessTerminating = FALSE;
#endif


PProcess::PProcess(const char * manuf, const char * name,
                           WORD major, WORD minor, CodeStatus stat, WORD build)
  : manufacturer(manuf), productName(name)
{
  PProcessInstance = this;
  terminationValue = 0;

  majorVersion = major;
  minorVersion = minor;
  status = stat;
  buildNumber = build;

  if (p_argv != 0 && p_argc > 0) {
    arguments.SetArgs(p_argc-1, p_argv+1);

    executableFile = PString(p_argv[0]);
    if (!PFile::Exists(executableFile))
      executableFile += ".exe";

    if (productName.IsEmpty())
      productName = executableFile.GetTitle().ToLower();
  }

  InitialiseProcessThread();

  Construct();
}


int PProcess::_main(void *)
{
  Main();
  return terminationValue;
}


void PProcess::PreInitialise(int c, char ** v, char ** e)
{
  p_argc = c;
  p_argv = v;
  p_envp = e;
}


PProcess & PProcess::Current()
{
  PAssertNULL(PProcessInstance);
  return *PProcessInstance;
}


PObject::Comparison PProcess::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PProcess::Class()), PInvalidCast);
  return productName.Compare(((const PProcess &)obj).productName);
}


void PProcess::Terminate()
{
#ifdef _WINDLL
  FatalExit(terminationValue);
#else
#ifndef P_PLATFORM_HAS_THREADS
  // Can only terminate process from the processes thread
  if (currentThread == this)
    exit(terminationValue);
  else
    PProcessTerminating = TRUE;
#else
  exit(terminationValue);
#endif
#endif
}


PString PProcess::GetVersion(BOOL full) const
{
  const char * const statusLetter[NumCodeStatuses] =
    { "alpha", "beta", "pl" };
  return psprintf(full && buildNumber != 0 ? "%u.%u%s%u" : "%u.%u",
                majorVersion, minorVersion, statusLetter[status], buildNumber);
}


///////////////////////////////////////////////////////////////////////////////
// PThread

#ifndef P_PLATFORM_HAS_THREADS

void PThread::InitialiseProcessThread()
{
  basePriority = NormalPriority;  // User settable priority
  dynamicPriority = 0;            // Only thing running
  suspendCount = 0;               // Not suspended (would not be a good idea)
  ClearBlock();                   // No I/O blocking function
  status = Running;               // Thread is already running
  stackBase = NULL;
  link = this;
  ((PProcess*)this)->currentThread = this;
}


PThread::PThread(PINDEX stackSize, AutoDeleteFlag deletion, Priority priorityLevel)
{
  autoDelete = deletion == AutoDeleteThread;
  basePriority = priorityLevel;   // Threads user settable priority level
  dynamicPriority = 0;            // Run immediately
  suspendCount = 1;

  AllocateStack(stackSize);
  PAssert(stackBase != NULL, "Insufficient near heap for thread");

  status = Terminated; // Set to this so Restart() works
  Restart();
}


void PThread::Restart()
{
  if (status != Terminated) // Is already running
    return;

  ClearBlock();             // No I/O blocking function

  PThread * current = Current();
  link = current->link;
  current->link = this;

  status = Starting;
}


void PThread::Terminate()
{
  if (link == this || status == Terminated)
    return;   // Is only thread or already terminated

  if (status == Running) {
    status = Terminating;
    Yield(); // Never returns from here
  }

  PThread * prev = PThread::Current();
  while (prev->link != this)
    prev = prev->link;
  prev->link = link;   // Unlink it from the list
  status = Terminated;
}


void PThread::WaitForTermination() const
{
  while (!IsTerminated())
    Yield();
}


BOOL PThread::WaitForTermination(const PTimeInterval & maxWait) const
{
  PTimer timeout = maxWait;
  while (!IsTerminated()) {
    if (timeout == 0)
      return FALSE;
    Yield();
  }

  return TRUE;
}


void PThread::Suspend(BOOL susp)
{
  // Suspend/Resume the thread
  if (susp)
    suspendCount++;
  else
    suspendCount--;

  switch (status) {
    case Running : // Suspending itself, yield to next thread
      if (IsSuspended()) {
        status = Suspended;
        Yield();
      }
      break;

    case Waiting :
      if (IsSuspended())
        status = Suspended;
      break;

    case BlockedIO :
      if (IsSuspended())
        status = SuspendedBlockIO;
      break;

    case BlockedSem :
      if (IsSuspended())
        status = SuspendedBlockSem;
      break;

    case Suspended :
      if (!IsSuspended())
        status = Waiting;
      break;

    case SuspendedBlockIO :
      if (!IsSuspended())
        status = BlockedIO;
      break;

    case SuspendedBlockSem :
      if (!IsSuspended())
        status = BlockedSem;
      break;

    default :
      break;
  }
}


void PThread::Sleep(const PTimeInterval & time)
{
  sleepTimer = time;
  if (time == PMaxTimeInterval)
    sleepTimer.Pause();

  switch (status) {
    case Running : // Suspending itself, yield to next thread
      status = Sleeping;
      Yield();
      break;

    case Waiting :
    case Suspended :
      status = Sleeping;
      break;

    default :
      break;
  }
}


void PThread::BeginThread()
{
  if (IsSuspended()) { // Begins suspended
    status = Suspended;
    Yield();
  }
  else
    status = Running;

  Main();

  status = Terminating;
  Yield(); // Never returns from here
}


void PThread::Yield()
{
  PThread * current = PProcessInstance->currentThread;
  if (current == PProcessInstance) {
    PProcessInstance->GetTimerList()->Process();
    if (current->link == current)
      return;
  }
  else {
    char stackUsed;
    if (&stackUsed < current->stackBase) {
      char * buf = (char *)malloc(1000);
      sprintf(buf, "Stack overflow!\n"
                   "\n"
                   "Thread: 0x%08x - %s\n"
                   "Stack top  : 0x%08x\n"
                   "Stack base : 0x%08x\n"
                   "Stack frame: 0x%08x\n"
                   "\n",
              (int)current, current->GetClass(),
              (int)current->stackTop,
              (int)current->stackBase,
              (int)&stackUsed);
      PAssertAlways(buf);
      PError << "Aborting." << endl;
      _exit(1);
    }
  }

  if (current->status == Running && current->basePriority == HighestPriority)
    return;

  do {
    if (current->status == Running)
      current->status = Waiting;

    static const int dynamicLevel[NumPriorities] = { -1, 3, 1, 0, 0 };
    current->dynamicPriority = dynamicLevel[current->basePriority];

    PThread * next = NULL; // Next thread to be scheduled
    PThread * prev = current; // Need the thread in the previous link
    PThread * start = current;  // Need thread in list that is the "start"
    PThread * thread = current->link;
    BOOL pass = 0;
    BOOL canUseLowest = TRUE;

    for (;;) {
      if (PProcessTerminating) {
        next = PProcessInstance;
        next->status = Running;
        break;
      }

      switch (thread->status) {
        case Waiting :
          if (thread->dynamicPriority == 0) {
            next = thread;
            next->status = Running;
          }
          else if (thread->dynamicPriority > 0) {
            thread->dynamicPriority--;
            canUseLowest = FALSE;
          }
          else if (pass > 1 && canUseLowest)
            thread->dynamicPriority++;
          break;

        case Sleeping :
          if (thread->sleepTimer == 0) {
            if (thread->IsSuspended())
              thread->status = Suspended;
            else {
              thread->status = Running;
              next = thread;
            }
          }
          break;

        case BlockedIO :
          if (thread->IsNoLongerBlocked()) {
            if (PProcessTerminating)
              next = PProcessInstance;
            else {
              thread->ClearBlock();
              next = thread;
            }
            next->status = Running;
          }
          break;

        case BlockedSem :
        case SuspendedBlockSem :
          if (thread->blockingSemaphore->timeout == 0) {
            thread->blockingSemaphore->PSemaphore::Signal();
            thread->blockingSemaphore->timeout = 0;
            if (thread->status == Waiting) {
              next = thread;
              next->status = Running;
            }
          }
          break;

        case Starting :
          if (!thread->IsSuspended())
            next = thread;
          break;

        case Terminating :
          if (thread == current)         // Cannot self terminate
            next = PProcessInstance;     // So switch to process thread first
          else {
            prev->link = thread->link;   // Unlink it from the list
            if (thread == start)         // If unlinking the "start" thread
              start = prev;              //    then we better make it still in list
            thread->status = Terminated; // Flag thread as terminated
            if (thread->autoDelete)
              delete thread;             // Destroy if auto-delete
            thread = prev;
          }
          break;

        default :
          break;
      }

      if (next != NULL)  // Have a thread to run
        break;

      // Need to have previous thread so can unlink a terminating thread
      prev = thread;
      thread = thread->link;
      if (thread == start) {
        pass++;
        if (pass > 3) // Everything is blocked
          PProcessInstance->OperatingSystemYield();
      }
    }

    PProcessInstance->currentThread = next;

    next->SwitchContext(current);

    // Could get here with a self terminating thread, so go around again if all
    // we did was switch stacks, and do not actually have a running thread.
  } while (PProcessInstance->currentThread->status != Running);

  if (PProcessTerminating) {
    PProcessTerminating = FALSE;
    if (PProcessInstance->IsDescendant(PServiceProcess::Class()))
      ((PServiceProcess *)PProcessInstance)->OnStop();
    exit(PProcessInstance->terminationValue);
  }
}


#endif


///////////////////////////////////////////////////////////////////////////////
// PSemaphore

#ifndef P_PLATFORM_HAS_THREADS

PSemaphore::PSemaphore(unsigned initial, unsigned maxCount)
{
  PAssert(maxCount > 0, "Invalid semaphore maximum.");
  if (initial > maxCount)
    initial = maxCount;
  currentCount = initial;
  maximumCount = maxCount;
}


PSemaphore::~PSemaphore()
{
  PAssert(blockedThreads.IsEmpty(),
                        "Semaphore destroyed while still has blocked threads");
}


void PSemaphore::Wait()
{
  Wait(PMaxTimeInterval);
}


BOOL PSemaphore::Wait(const PTimeInterval & time)
{
  if (currentCount > 0)
    currentCount--;
  else {
    PThread * thread = PThread::Current();
    blockedThreads.Enqueue(thread);
    thread->blockingSemaphore = this;
    thread->status = PThread::BlockedSem;
    timeout = time;
    if (time == PMaxTimeInterval)
      timeout.Pause();
    PThread::Yield();
    if (timeout == 0)
      return FALSE;
  }
  return TRUE;
}


void PSemaphore::Signal()
{
  if (blockedThreads.GetSize() > 0) {
    PThread * thread = blockedThreads.Dequeue();
    switch (thread->status) {
      case PThread::BlockedSem :
        thread->status = PThread::Waiting;
        break;
      case PThread::SuspendedBlockSem :
        thread->status = PThread::Suspended;
        break;
      default:
        PAssertAlways("Semaphore unblock of thread that is not blocked");
    }
    thread->sleepTimer = 0;
    timeout = PMaxTimeInterval;
    timeout.Pause();
  }
  else if (currentCount < maximumCount)
    currentCount++;
}


BOOL PSemaphore::WillBlock() const
{
  return currentCount == 0;
}


PMutex::PMutex()
  : PSemaphore(1, 1)
{
}


PSyncPoint::PSyncPoint()
  : PSemaphore(0, 1)
{
}


#endif

void PSyncPointAck::Signal()
{
  PSyncPoint::Signal();
  ack.Wait();
}


void PSyncPointAck::Signal(const PTimeInterval & wait)
{
  PSyncPoint::Signal();
  ack.Wait(wait);
}


void PSyncPointAck::Acknowledge()
{
  ack.Signal();
}


void PCondMutex::WaitCondition()
{
  for (;;) {
    Wait();
    if (Condition())
      return;
    PMutex::Signal();
    OnWait();
    syncPoint.Wait();
  }
}


void PCondMutex::Signal()
{
  if (Condition())
    syncPoint.Signal();
  PMutex::Signal();
}


void PCondMutex::OnWait()
{
  // Do nothing
}


PIntCondMutex::PIntCondMutex(int val, int targ, Operation op)
{
  value = val;
  target = targ;
  operation = op;
}


void PIntCondMutex::PrintOn(ostream & strm) const
{
  strm << '(' << value;
  switch (operation) {
    case LT :
      strm << " < ";
    case LE :
      strm << " <= ";
    case GE :
      strm << " >= ";
    case GT :
      strm << " > ";
    default:
      strm << " == ";
  }
  strm << target << ')';
}


BOOL PIntCondMutex::Condition()
{
  switch (operation) {
    case LT :
      return value < target;
    case LE :
      return value <= target;
    case GE :
      return value >= target;
    case GT :
      return value > target;
    default :
      break;
  }
  return value == target;
}


PIntCondMutex & PIntCondMutex::operator=(int newval)
{
  Wait();
  value = newval;
  Signal();
  return *this;
}


PIntCondMutex & PIntCondMutex::operator++()
{
  Wait();
  value++;
  Signal();
  return *this;
}


PIntCondMutex & PIntCondMutex::operator+=(int inc)
{
  Wait();
  value += inc;
  Signal();
  return *this;
}


PIntCondMutex & PIntCondMutex::operator--()
{
  Wait();
  value--;
  Signal();
  return *this;
}


PIntCondMutex & PIntCondMutex::operator-=(int dec)
{
  Wait();
  value -= dec;
  Signal();
  return *this;
}


void PReadWriteMutex::StartRead()
{
  starvationPreventer.Wait();
  starvationPreventer.Signal();
  ++readers;
}


void PReadWriteMutex::EndRead()
{
  --readers;
}


void PReadWriteMutex::StartWrite()
{
  starvationPreventer.Wait();
  readers.WaitCondition();
}


void PReadWriteMutex::EndWrite()
{
  starvationPreventer.Signal();
  readers.Signal();
}


// End Of File ///////////////////////////////////////////////////////////////
