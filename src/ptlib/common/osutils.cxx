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
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>

#include <ctype.h>
#include <ptlib/pfactory.h>
#include <ptlib/pprocess.h>
#include <ptlib/svcproc.h>
#include <ptlib/pluginmgr.h>
#include "../../../version.h"
#include "../../../revision.h"

#ifdef _WIN32
#include <ptlib/msos/ptlib/debstrm.h>
#endif


static const char * const VersionStatus[PProcess::NumCodeStatuses] = { "alpha", "beta", "." };
static const char DefaultRollOverPattern[] = "_yyyy_MM_dd_hh_mm";

class PExternalThread : public PThread
{
  PCLASSINFO(PExternalThread, PThread);
  public:
    PExternalThread()
      : PThread(false)
    {
      SetThreadName(PString::Empty());
      PTRACE(5, "PTLib\tCreated external thread " << this << ", id " << GetCurrentThreadId());
    }

    ~PExternalThread()
    {
#ifdef _WIN32
      CleanUp();
#endif
      PTRACE(5, "PTLib\tDestroyed external thread " << this << ", id " << GetThreadId());
    }

    virtual void Main()
    {
    }

    virtual void Terminate()
    {
      PTRACE(2, "PTLib\tCannot terminate external thread " << this << ", id " << GetThreadId());
    }
};


class PSimpleThread : public PThread
{
    PCLASSINFO(PSimpleThread, PThread);
  public:
    PSimpleThread(
      const PNotifier & notifier,
      INT parameter,
      AutoDeleteFlag deletion,
      Priority priorityLevel,
      const PString & threadName,
      PINDEX stackSize
    );
    void Main();
  protected:
    PNotifier callback;
    INT parameter;
};


#define new PNEW


#ifndef __NUCLEUS_PLUS__
static ostream * PErrorStream = &cerr;
#else
static ostream * PErrorStream = NULL;
#endif

ostream & PGetErrorStream()
{
  return *PErrorStream;
}


void PSetErrorStream(ostream * s)
{
#ifndef __NUCLEUS_PLUS__
  PErrorStream = s != NULL ? s : &cerr;
#else
  PErrorStream = s;
#endif
}

//////////////////////////////////////////////////////////////////////////////

#if PTRACING

class PTraceInfo
{
  /* NOTE you cannot have any complex types in this structure. Anything
     that might do an asert or PTRACE will crash due to recursion.
   */

public:
  unsigned        currentLevel;
  unsigned        options;
  unsigned        thresholdLevel;
  PCaselessString m_filename;
  ostream       * stream;
  PTimeInterval   startTick;
  PString         m_rolloverPattern;
  unsigned        lastRotate;
  ios::fmtflags   oldStreamFlags;
  std::streamsize oldPrecision;


#if defined(_WIN32)
  CRITICAL_SECTION mutex;
  void InitMutex() { InitializeCriticalSection(&mutex); }
  void Lock()      { EnterCriticalSection(&mutex); }
  void Unlock()    { LeaveCriticalSection(&mutex); }
#elif defined(P_PTHREADS) && P_HAS_RECURSIVE_MUTEX
  pthread_mutex_t mutex;
  void InitMutex() {
    // NOTE this should actually guard against various errors
    // returned.
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,
#if P_HAS_RECURSIVE_MUTEX == 2
PTHREAD_MUTEX_RECURSIVE
#else
PTHREAD_MUTEX_RECURSIVE_NP
#endif
    );
    pthread_mutex_init(&mutex, &attr);
    pthread_mutexattr_destroy(&attr);
  }
  void Lock()      { pthread_mutex_lock(&mutex); }
  void Unlock()    { pthread_mutex_unlock(&mutex); }
#else
  PMutex * mutex;
  void InitMutex() { mutex = new PMutex; }
  void Lock()      { mutex->Wait(); }
  void Unlock()    { mutex->Signal(); }
#endif
  
  struct ThreadLocalInfo {
    ThreadLocalInfo()
      : m_traceLevel(1)
      , m_traceBlockIndentLevel(0)
    { }

    PStack<PStringStream> m_traceStreams;
    unsigned              m_traceLevel;
    unsigned              m_traceBlockIndentLevel;
  };
  PThreadLocalStorage<ThreadLocalInfo> m_threadStorage;

  PTraceInfo()
    : currentLevel(0)
#ifdef __NUCLEUS_PLUS__
    , stream(NULL)
#else
    , stream(&cerr)
#endif
    , startTick(PTimer::Tick())
    , m_rolloverPattern(DefaultRollOverPattern)
    , lastRotate(0)
    , oldStreamFlags(ios::left)
    , oldPrecision(0)
  {
    InitMutex();

    const char * env = getenv("PWLIB_TRACE_STARTUP"); // Backward compatibility test
    if (env == NULL) 
      env = getenv("PTLIB_TRACE_STARTUP"); // Backward compatibility test
    if (env != NULL) {
      thresholdLevel = atoi(env);
      options = PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine;
    }
    else {
      env = getenv("PWLIB_TRACE_LEVEL");
      if (env == NULL)
        env = getenv("PTLIB_TRACE_LEVEL");
      thresholdLevel = env != NULL ? atoi(env) : 0;

      env = getenv("PWLIB_TRACE_OPTIONS");
      if (env == NULL)
        env = getenv("PTLIB_TRACE_OPTIONS");
      options = env != NULL ? atoi(env) : PTrace::FileAndLine;
    }
    env = getenv("PWLIB_TRACE_FILE");
    if (env == NULL)
      env = getenv("PTLIB_TRACE_FILE");

    OpenTraceFile(env);
  }

  ~PTraceInfo()
  {
    if (stream != &cerr && stream != &cout)
      delete stream;
  }

  static PTraceInfo & Instance()
  {
    static PTraceInfo info;
    return info;
  }

  void SetStream(ostream * newStream)
  {
#ifndef __NUCLEUS_PLUS__
    if (newStream == NULL)
      newStream = &cerr;
#endif

    Lock();

    if (stream != &cerr && stream != &cout)
      delete stream;
    stream = newStream;

    Unlock();
  }

  ostream * GetStream() const
  {
    return stream;
  }

  void OpenTraceFile(const char * newFilename)
  {
    if (newFilename == NULL || *newFilename == '\0') {
      m_filename.MakeEmpty();
      return;
    }

    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

    m_filename = newFilename;

    if (m_filename == "stderr")
      SetStream(&cerr);
    else if (m_filename == "stdout")
      SetStream(&cout);
#ifdef _WIN32
    else if (m_filename == "DEBUGSTREAM")
      SetStream(new PDebugStream);
#endif
    else {
      PFilePath fn(m_filename);
      fn.Replace("%P", PString(PProcess::GetCurrentProcessID()));
     
      if ((options & PTrace::RotateLogMask) != 0)
      {
          PTime now;
          fn = fn.GetDirectory() +
               fn.GetTitle() +
               now.AsString(m_rolloverPattern, ((options&PTrace::GMTTime) ? PTime::GMT : PTime::Local)) +
               fn.GetType();
      }

      ofstream * traceOutput;
      if (options & PTrace::AppendToFile) 
        traceOutput = new ofstream((const char *)fn, ios_base::out | ios_base::app);
      else 
        traceOutput = new ofstream((const char *)fn, ios_base::out | ios_base::trunc);

      if (traceOutput->is_open())
        SetStream(traceOutput);
      else {
        PStringStream msgstrm;
        msgstrm << PProcess::Current().GetName() << ": Could not open trace output file \"" << fn << '"';
#ifdef WIN32
        PVarString msg(msgstrm);
        MessageBox(NULL, msg, NULL, MB_OK|MB_ICONERROR);
#else
        fputs(msgstrm, stderr);
#endif
        delete traceOutput;
      }
    }
  }
};


void PTrace::SetStream(ostream * s)
{
  PTraceInfo::Instance().SetStream(s);
}

ostream * PTrace::GetStream()
{
  return PTraceInfo::Instance().GetStream();
}

void PTrace::Initialise(
    unsigned level,
    const char * filename,
    unsigned options
)
{
  Initialise(level, filename, NULL, options);
}

static unsigned GetRotateVal(unsigned options)
{
  PTime now;
  if (options & PTrace::RotateDaily)
    return now.GetDayOfYear();
  if (options & PTrace::RotateHourly) 
    return now.GetHour();
  if (options & PTrace::RotateMinutely)
    return now.GetMinute();
  return 0;
}

void PTrace::Initialise(unsigned level, const char * filename, const char * rolloverPattern, unsigned options)
{
  PTraceInfo & info = PTraceInfo::Instance();

  info.options = options;
  info.thresholdLevel = level;
  info.m_rolloverPattern = rolloverPattern;
  if (info.m_rolloverPattern.IsEmpty())
    info.m_rolloverPattern = DefaultRollOverPattern;
  // Does PTime::GetDayOfYear() etc. want to take zone param like PTime::AsString() to switch 
  // between os_gmtime and os_localtime?
  info.lastRotate = GetRotateVal(options);
  info.OpenTraceFile(filename);

  ostream & log = Begin(0, "", 0) << '\t';

  if (PProcess::IsInitialised ()) {
    PProcess & process = PProcess::Current();
    log << process.GetName()
        << " version " << process.GetVersion(true)
        << " by " << process.GetManufacturer()
        << " on ";
  }

  log << PProcess::GetOSClass() << ' ' << PProcess::GetOSName()
      << " (" << PProcess::GetOSVersion() << '-' << PProcess::GetOSHardware() << ")"
         " with PTLib (v" << PProcess::GetLibVersion() << ")"
         " at " << PTime().AsString("yyyy/M/d h:mm:ss.uuu") << ","
         " level=" << level
      << End;
}


void PTrace::SetOptions(unsigned options)
{
  PTraceInfo::Instance().options |= options;
}


void PTrace::ClearOptions(unsigned options)
{
  PTraceInfo::Instance().options &= ~options;
}


unsigned PTrace::GetOptions()
{
  return PTraceInfo::Instance().options;
}


void PTrace::SetLevel(unsigned level)
{
  PTraceInfo::Instance().thresholdLevel = level;
}


unsigned PTrace::GetLevel()
{
  return PTraceInfo::Instance().thresholdLevel;
}


PBoolean PTrace::CanTrace(unsigned level)
{
  return PProcess::IsInitialised() && level <= PTraceInfo::Instance().thresholdLevel;
}


ostream & PTrace::Begin(unsigned level, const char * fileName, int lineNum, const PObject * instance, const char * module)
{
  PTraceInfo & info = PTraceInfo::Instance();

  if (level == UINT_MAX || !PProcess::IsInitialised())
    return *info.stream;

  PTraceInfo::ThreadLocalInfo * threadInfo = info.m_threadStorage.Get();

  PStringStream * streamPtr = NULL;
  if (threadInfo != NULL) {
    streamPtr = new PStringStream;
    threadInfo->m_traceStreams.Push(streamPtr);
  }

  ostream & stream = streamPtr != NULL ? (ostream &)*streamPtr : *info.stream;

  info.oldStreamFlags = stream.flags();
  info.oldPrecision   = stream.precision();

  // Before we do new trace, make sure we clear any errors on the stream
  stream.clear();

  PThread * thread = PThread::Current();

  info.Lock();

  if (!info.m_filename.IsEmpty() && (info.options&RotateLogMask) != 0) {
    unsigned rotateVal = GetRotateVal(info.options);
    if (rotateVal != info.lastRotate) {
      info.OpenTraceFile(info.m_filename);
      info.lastRotate = rotateVal;
      if (info.stream == NULL)
        info.SetStream(&cerr);
    }
  }

  if ((info.options&SystemLogStream) == 0) {
    if ((info.options&DateAndTime) != 0) {
      PTime now;
      stream << now.AsString("yyyy/MM/dd hh:mm:ss.uuu\t", (info.options&GMTTime) ? PTime::GMT : PTime::Local);
    }

    if ((info.options&Timestamp) != 0)
      stream << setprecision(3) << setw(10) << (PTimer::Tick()-info.startTick) << '\t';

    if ((info.options&Thread) != 0) {
      PString name;
      if (thread == NULL)
        name.sprintf("ThreadID" PTHREAD_ID_FMT, PThread::GetCurrentThreadId());
      else
        name = thread->GetThreadName();
      if (name.GetLength() <= 23)
        stream << setw(23) << name;
      else
        stream << name.Left(10) << "..." << name.Right(10);
      stream << '\t';
    }

    if ((info.options&ThreadAddress) != 0)
      stream << hex << setfill('0')
             << setw(7) << (void *)PThread::Current()
             << dec << setfill(' ') << '\t';
  }

  if ((info.options&TraceLevel) != 0)
    stream << level << '\t';

  if ((info.options&FileAndLine) != 0) {
    const char * file;
    if (fileName == NULL)
      file = "-";
    else {
      file = strrchr(fileName, '/');
      if (file != NULL)
        file++;
      else {
        file = strrchr(fileName, '\\');
        if (file != NULL)
          file++;
        else
          file = fileName;
      }
    }

    stream << setw(16) << file << '(' << lineNum << ")\t";
  }

  if ((info.options&ObjectInstance) != 0) {
    if (instance != NULL)
      stream << instance->GetClass() << ':' << instance;
    stream << '\t';
  }

  if ((info.options&ContextIdentifier) != 0) {
    unsigned id = 0;
    if (instance != NULL)
      id = instance->GetTraceContextIdentifier();
    if (id == 0)
      id = thread->GetTraceContextIdentifier();
    if (id != 0)
      stream << setfill('0') << setw(13) << id << setfill(' ');
    else
      stream << "- - - - - - -";
    stream << '\t';
  }

  if (module != NULL)
    stream << left << setw(8) << module << '\t';

  // Save log level for this message so End() function can use. This is
  // protected by the PTraceMutex or is thread local
  if (threadInfo == NULL)
    info.currentLevel = level;
  else {
    threadInfo->m_traceLevel = level;
    info.Unlock();
  }

  return stream;
}


ostream & PTrace::End(ostream & paramStream)
{
  PTraceInfo & info = PTraceInfo::Instance();

  PTraceInfo::ThreadLocalInfo * threadInfo = info.m_threadStorage.Get();

  paramStream.flags(info.oldStreamFlags);
  paramStream.precision(info.oldPrecision);

  unsigned currentLevel;

  if (threadInfo != NULL && !threadInfo->m_traceStreams.IsEmpty()) {
    PStringStream * stackStream = threadInfo->m_traceStreams.Pop();
    if (!PAssert(&paramStream == stackStream, PLogicError))
      return paramStream;
    *stackStream << ends << flush;
    info.Lock();
    *info.stream << *stackStream;
    delete stackStream;

    currentLevel = threadInfo->m_traceLevel;
  }
  else {
    if (!PAssert(&paramStream == info.stream, PLogicError))
      return paramStream;
    info.Lock();

    currentLevel = info.currentLevel;
  }

  if ((info.options&SystemLogStream) != 0) {
    // Get the trace level for this message and set the stream width to that
    // level so that the PSystemLog can extract the log level back out of the
    // ios structure. There could be portability issues with this though it
    // should work pretty universally.
    info.stream->width(currentLevel + 1);
  }
  else
    *info.stream << '\n';
  info.stream->flush();

  info.Unlock();
  return paramStream;
}


PTrace::Block::Block(const char * fileName, int lineNum, const char * traceName)
{
  file = fileName;
  line = lineNum;
  name = traceName;

  if ((PTraceInfo::Instance().options&Blocks) != 0) {
    unsigned indent = 20;

    PTraceInfo::ThreadLocalInfo * threadInfo = PTraceInfo::Instance().m_threadStorage.Get();
    if (threadInfo != NULL)
      indent = (threadInfo->m_traceBlockIndentLevel += 2);

    ostream & s = PTrace::Begin(1, file, line);
    s << "B-Entry\t";
    for (unsigned i = 0; i < indent; i++)
      s << '=';
    s << "> " << name << PTrace::End;
  }
}


PTrace::Block::~Block()
{
  if ((PTraceInfo::Instance().options&Blocks) != 0) {
    unsigned indent = 20;

    PTraceInfo::ThreadLocalInfo * threadInfo = PTraceInfo::Instance().m_threadStorage.Get();
    if (threadInfo != NULL) {
      indent = threadInfo->m_traceBlockIndentLevel;
      threadInfo->m_traceBlockIndentLevel -= 2;
    }

    ostream & s = PTrace::Begin(1, file, line);
    s << "B-Exit\t<";
    for (unsigned i = 0; i < indent; i++)
      s << '=';
    s << ' ' << name << PTrace::End;
  }
}

void PTrace::Cleanup()
{
  PTraceInfo::Instance().m_threadStorage.Clean();
}


static PAtomicInteger g_lastContextIdentifer;

unsigned PTrace::GetNextContextIdentifier()
{
  return ++g_lastContextIdentifer;
}


PTraceSaveContextIdentifier::PTraceSaveContextIdentifier(const PObject & obj)
  : m_currentThread(PThread::Current())
  , m_savedContextIdentifier(m_currentThread->GetTraceContextIdentifier())
{
  m_currentThread->SetTraceContextIdentifier(obj.GetTraceContextIdentifier());
}


PTraceSaveContextIdentifier::PTraceSaveContextIdentifier(const PObject * obj)
  : m_currentThread(PThread::Current())
  , m_savedContextIdentifier(m_currentThread->GetTraceContextIdentifier())
{
  m_currentThread->SetTraceContextIdentifier(obj->GetTraceContextIdentifier());
}


PTraceSaveContextIdentifier::~PTraceSaveContextIdentifier()
{
  if (m_currentThread != NULL)
    m_currentThread->SetTraceContextIdentifier(m_savedContextIdentifier);
}


#endif // PTRACING


///////////////////////////////////////////////////////////////////////////////
// PDirectory

void PDirectory::CloneContents(const PDirectory * d)
{
  CopyContents(*d);
}


///////////////////////////////////////////////////////////////////////////////
// PSimpleTimer

PSimpleTimer::PSimpleTimer(long milliseconds,
                           int seconds,
                           int minutes,
                           int hours,
                           int days)
  : PTimeInterval(milliseconds, seconds, minutes, hours, days)
  , m_startTick(PTimer::Tick())
{
}


PSimpleTimer::PSimpleTimer(const PTimeInterval & time)
  : PTimeInterval(time)
  , m_startTick(PTimer::Tick())
{
}


PSimpleTimer::PSimpleTimer(const PSimpleTimer & timer)
  : PTimeInterval(timer)
  , m_startTick(PTimer::Tick())
{
}


PSimpleTimer & PSimpleTimer::operator=(DWORD milliseconds)
{
  PTimeInterval::operator=(milliseconds);
  m_startTick = PTimer::Tick();
  return *this;
}


PSimpleTimer & PSimpleTimer::operator=(const PTimeInterval & time)
{
  PTimeInterval::operator=(time);
  m_startTick = PTimer::Tick();
  return *this;
}


PSimpleTimer & PSimpleTimer::operator=(const PSimpleTimer & timer)
{
  PTimeInterval::operator=(timer);
  m_startTick = PTimer::Tick();
  return *this;
}


void PSimpleTimer::SetInterval(PInt64 milliseconds,
                               long seconds,
                               long minutes,
                               long hours,
                               int days)
{
  PTimeInterval::SetInterval(milliseconds, seconds, minutes, hours, days);
  m_startTick = PTimer::Tick();
}


PTimeInterval PSimpleTimer::GetRemaining() const
{
  PTimeInterval remaining = *this - GetElapsed();
  return remaining > 0 ? remaining : PTimeInterval(0);
}


///////////////////////////////////////////////////////////////////////////////
// PTimer

PTimer::PTimer(long millisecs, int seconds, int minutes, int hours, int days)
  : m_resetTime(millisecs, seconds, minutes, hours, days)
{
  Construct();
}


PTimer::PTimer(const PTimeInterval & time)
  : m_resetTime(time)
{
  Construct();
}


PTimer::PTimer(const PTimer & timer)
  : m_resetTime(timer.GetMilliSeconds())
{
  Construct();
}


void PTimer::Construct()
{
  m_timerList = PProcess::Current().GetTimerList();
  m_timerId = m_timerList->GetNewTimerId();
  m_state = Stopped;

  StartRunning(true);
}


PInt64 PTimer::GetMilliSeconds() const
{
  PInt64 diff = m_absoluteTime - Tick().GetMilliSeconds();
  if (diff < 0)
    diff = 0;
  return diff;
}


PTimer::~PTimer()
{
  // queue a request to remove this timer, and always do it synchronously
  Stop(true);
}


void PTimer::SetInterval(PInt64 milliseconds,
                         long seconds,
                         long minutes,
                         long hours,
                         int days)
{
  m_resetTime.SetInterval(milliseconds, seconds, minutes, hours, days);
  StartRunning(m_oneshot);
}


void PTimer::RunContinuous(const PTimeInterval & time)
{
  m_resetTime = time;
  StartRunning(false);
}


void PTimer::StartRunning(PBoolean once)
{
  Stop(true);

  PTimeInterval::operator=(m_resetTime);
  m_oneshot = once;

  if (m_resetTime > 0) {
    m_absoluteTime = Tick().GetMilliSeconds() + m_resetTime.GetMilliSeconds();
    m_timerList->QueueRequest(PTimerList::RequestType::Start, this, false);
  }
}


void PTimer::Stop(bool wait)
{
  if (m_state != Stopped)
    m_timerList->QueueRequest(PTimerList::RequestType::Stop, this, wait);
}


void PTimer::Pause()
{
  if (IsRunning())
    m_timerList->QueueRequest(PTimerList::RequestType::Pause, this);
}


void PTimer::Resume()
{
  if (m_state == Stopped || m_state == Paused)
    m_timerList->QueueRequest(PTimerList::RequestType::Start, this);
}


void PTimer::Reset()
{
  StartRunning(m_oneshot);
}

// called only from the timer thread
void PTimer::OnTimeout()
{
  if (!m_callback.IsNULL())
    m_callback(*this, IsRunning());
}


void PTimer::Process(PInt64 now)
{
  if (m_state == Running && m_absoluteTime <= now) {
    if (m_oneshot) 
      m_state = Stopped;
    OnTimeout();
  }
}


///////////////////////////////////////////////////////////////////////////////
// PTimerList

PTimerList::PTimerList()
{
  m_timerThread = NULL;
}

void PTimerList::QueueRequest(RequestType::Action action, PTimer * timer, bool isSync)
{
  bool inTimerThread = m_timerThread == PThread::Current();

  RequestType request(action, timer);

  // set synchronisation point
  PSyncPoint sync;
  if (!inTimerThread && isSync)
    request.m_sync = &sync;

  // queue the request
  m_queueMutex.Wait();
  m_requestQueue.push(request);
  m_queueMutex.Signal();

  // wait for synchronisation point
  if (!inTimerThread && PProcess::Current().SignalTimerChange() && isSync)
    sync.Wait();
}


void PTimerList::ProcessTimerQueue()
{
  m_queueMutex.Wait();

  // process the requests in the timer request queue
  while (!m_requestQueue.empty()) {

    RequestType request(m_requestQueue.front());
    m_requestQueue.pop();
    m_queueMutex.Signal();

    ActiveTimerInfoMap::iterator it = m_activeTimers.find(request.m_id);
    switch (request.m_action) {
      case PTimerList::RequestType::Start:
        if (it == m_activeTimers.end())
          m_activeTimers.insert(ActiveTimerInfoMap::value_type(request.m_id, ActiveTimerInfo(request.m_timer, request.m_serialNumber)));
        else
          it->second.m_serialNumber = request.m_serialNumber;
        m_expiryList.insert(TimerExpiryInfo(request.m_id, request.m_absoluteTime, request.m_serialNumber));
        request.m_timer->m_state = PTimer::Running;
        break;

      case PTimerList::RequestType::Stop:
        if (it != m_activeTimers.end())
          m_activeTimers.erase(it);
        request.m_timer->m_state = PTimer::Stopped;
        break;

      case PTimerList::RequestType::Pause:
        if (it != m_activeTimers.end())
          m_activeTimers.erase(it);
        request.m_timer->m_state = PTimer::Paused;
        break;

      default:
        PAssertAlways("unknown timer request code");
        break;
    }

    if (request.m_sync != NULL)
      request.m_sync->Signal();

    m_queueMutex.Wait();
  }

  m_queueMutex.Signal();
}

PTimeInterval PTimerList::Process()
{
  m_timerThread = PThread::Current();

  PTRACE(6, "PTLib\tMONITOR: timers=" << m_activeTimers.size() << ", expiries=" << m_expiryList.size());

  // process the timer queue
  ProcessTimerQueue();

  // process timers that have expired
  PInt64 now = PTimer::Tick().GetMilliSeconds();
  while ((m_expiryList.size() > 0) && (m_expiryList.begin()->m_expireTime <= now)) {
    TimerExpiryInfo expiry = *m_expiryList.begin();
    m_expiryList.erase(m_expiryList.begin());

    ActiveTimerInfoMap::iterator t = m_activeTimers.find(expiry.m_timerId);
    if (t != m_activeTimers.end()) {
      ActiveTimerInfo & timer = t->second;
      if (expiry.m_serialNumber == timer.m_serialNumber) {
        timer.m_timer->Process(now);
        if (timer.m_timer->m_state != PTimer::Stopped)
          m_expiryList.insert(TimerExpiryInfo(expiry.m_timerId, now + timer.m_timer->m_resetTime.GetMilliSeconds(), timer.m_serialNumber));
        else
          m_activeTimers.erase(t);
      }
    }
  }

  // process the timer queue again
  ProcessTimerQueue();

  // use oldest timer to calculate minimum time left
  PTimeInterval minTimeLeft;
  if (m_expiryList.size() == 0) 
    minTimeLeft = 1000;
  else {
    minTimeLeft = m_expiryList.begin()->m_expireTime - now;
    if (minTimeLeft.GetMilliSeconds() < PTimer::Resolution())
      minTimeLeft = PTimer::Resolution();
    if (minTimeLeft < 25)
      minTimeLeft = 25;
  }

  return minTimeLeft;
}


///////////////////////////////////////////////////////////////////////////////
// PArgList

PArgList::PArgList(const char * theArgStr,
                   const char * theArgumentSpec,
                   PBoolean optionsBeforeParams)
{
  // get the program arguments
  if (theArgStr != NULL)
    SetArgs(theArgStr);
  else
    SetArgs(PStringArray());

  // if we got an argument spec - so process them
  if (theArgumentSpec != NULL)
    Parse(theArgumentSpec, optionsBeforeParams);
}


PArgList::PArgList(const PString & theArgStr,
                   const char * argumentSpecPtr,
                   PBoolean optionsBeforeParams)
{
  // get the program arguments
  SetArgs(theArgStr);

  // if we got an argument spec - so process them
  if (argumentSpecPtr != NULL)
    Parse(argumentSpecPtr, optionsBeforeParams);
}


PArgList::PArgList(const PString & theArgStr,
                   const PString & argumentSpecStr,
                   PBoolean optionsBeforeParams)
{
  // get the program arguments
  SetArgs(theArgStr);

  // if we got an argument spec - so process them
  Parse(argumentSpecStr, optionsBeforeParams);
}


PArgList::PArgList(int theArgc, char ** theArgv,
                   const char * theArgumentSpec,
                   PBoolean optionsBeforeParams)
{
  // get the program arguments
  SetArgs(theArgc, theArgv);

  // if we got an argument spec - so process them
  if (theArgumentSpec != NULL)
    Parse(theArgumentSpec, optionsBeforeParams);
}


PArgList::PArgList(int theArgc, char ** theArgv,
                   const PString & theArgumentSpec,
                   PBoolean optionsBeforeParams)
{
  // get the program name and path
  SetArgs(theArgc, theArgv);
  // we got an argument spec - so process them
  Parse(theArgumentSpec, optionsBeforeParams);
}


void PArgList::PrintOn(ostream & strm) const
{
  for (PINDEX i = 0; i < m_argumentArray.GetSize(); i++) {
    if (i > 0)
      strm << strm.fill();
    strm << m_argumentArray[i];
  }
}


void PArgList::ReadFrom(istream & strm)
{
  PString line;
  strm >> line;
  SetArgs(line);
}


void PArgList::Usage(ostream & strm) const
{
  size_t i;
  PINDEX maxNameLength = 0;
  for (i = 0; i < m_options.size(); ++i) {
    PINDEX len = m_options[i].m_name.GetLength();
    if (maxNameLength < len)
      maxNameLength = len;
  }
  maxNameLength += 6;

  for (i = 0; i < m_options.size(); ++i) {
    const OptionSpec & opt = m_options[i];
    if (!opt.m_section.IsEmpty())
      strm << '\n' << opt.m_section << '\n';

    strm << "  ";
    if (opt.m_letter != '\0')
      strm << '-' << opt.m_letter;
    else
      strm << "  ";

    if (opt.m_letter == '\0' || opt.m_name.IsEmpty())
      strm << "    ";
    else
      strm << " or ";
    strm << left << "--";
    if (opt.m_type == NoString)
      strm << setw(maxNameLength) << opt.m_name;
    else
      strm << opt.m_name << setw(maxNameLength-opt.m_name.GetLength()) << " <arg>";
    PStringArray lines = opt.m_usage.Lines();
    if (!lines.IsEmpty()) {
      strm << "  : " << lines[0] << '\n';
      for (PINDEX i = 1; i < lines.GetSize(); ++i)
        strm << setw(maxNameLength+14) << ' ' << lines[i] << '\n';
    }
  }
}


PString PArgList::Usage() const
{
  PStringStream str;
  Usage(str);
  return str;
}


void PArgList::SetArgs(const PString & argStr)
{
  m_argumentArray.SetSize(0);

  const char * str = argStr;

  for (;;) {
    while (isspace(*str)) // Skip leading whitespace
      str++;
    if (*str == '\0')
      break;

    PString & arg = m_argumentArray[m_argumentArray.GetSize()];
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

  SetArgs(m_argumentArray);
}


void PArgList::SetArgs(const PStringArray & theArgs)
{
  if (!theArgs.IsEmpty())
    m_argumentArray = theArgs;

  m_shift = 0;
  m_options.clear();
  m_parameterIndex.SetSize(m_argumentArray.GetSize());
  for (PINDEX i = 0; i < m_argumentArray.GetSize(); i++)
    m_parameterIndex[i] = i;
  m_argsParsed = 0;
}


PArgList::ParseResult PArgList::Parse(const char * spec, PBoolean optionsBeforeParams)
{
  // Find starting point, start at shift if first Parse() call.
  PINDEX arg = m_options.empty() ? m_shift : 0;

  // If not in parse all mode, have been parsed before, and had some parameters
  // from last time, then start argument parsing somewhere along instead of start.
  if (optionsBeforeParams && !m_options.empty() && m_argsParsed > 0)
    arg = m_argsParsed;

  if (spec == NULL) {
    if (m_options.empty() || !optionsBeforeParams)
      return ParseInvalidOptions;
    for (size_t opt = 0; opt < m_options.size(); ++opt) {
      m_options[opt].m_count = 0;
      m_options[opt].m_string.MakeEmpty();
    }
  }
  else {
    // Parse the option specification
    m_options.clear();

    while (*spec != '\0') {
      OptionSpec opts;

      if (*spec == '[') {
        const char * end = strchr(++spec, ']');
        if (end == NULL || spec == end)
          return ParseInvalidOptions;
        opts.m_section = PString(spec, end-spec);
        spec = end+1;
        if (spec == '\0')
          return ParseInvalidOptions;
      }

      if (*spec != '-')
        opts.m_letter = *spec++;

      if (*spec == '-') {
        size_t pos = strcspn(++spec, ".:; ");
        if (pos < 2)
          return ParseInvalidOptions;
        opts.m_name = PString(spec, pos);
        spec += pos;
      }

      switch (*spec++) {
        case '.' :
          opts.m_type = NoString;
          break;
        case ':' :
          opts.m_type = HasString;
          break;
        case ';' :
          opts.m_type = StringWithLetter;
          break;
        default :
          --spec;
      }

      if (*spec == ' ') {
        while (*spec == ' ')
          ++spec;
        const char * end = strchr(spec, '\n');
        if (end == NULL) {
          opts.m_usage = spec;
          while (*spec != '\0')
            ++spec;
        }
        else {
          opts.m_usage = PString(spec, end-spec);
          spec = end+1;
        }
      }

      // Check for duplicates
      for (size_t i = 0; i < m_options.size(); ++i) {
        if (opts.m_letter != '\0' && opts.m_letter == m_options[i].m_letter)
          return ParseInvalidOptions;
        if (!opts.m_name.IsEmpty() && opts.m_name == m_options[i].m_name)
          return ParseInvalidOptions;
      }

      m_options.push_back(opts);
    }
  }

  // Clear parameter indexes
  m_parameterIndex.SetSize(0);
  m_shift = 0;

  // Now work through the parameters and split out the options
  PINDEX param = 0;
  PBoolean hadMinusMinus = false;
  while (arg < m_argumentArray.GetSize()) {
    const PString & argStr = m_argumentArray[arg];
    if (hadMinusMinus || argStr[0] != '-' || argStr[1] == '\0') {
      // have a parameter string
      m_parameterIndex.SetSize(param+1);
      m_parameterIndex[param++] = arg;
    }
    else if (optionsBeforeParams && m_parameterIndex.GetSize() > 0)
      break;
    else if (argStr == "--") {
      if (optionsBeforeParams)
        hadMinusMinus = true; // ALL remaining args are parameters not options
      else {
        m_argsParsed = arg+1;
        break;
      }
    }
    else if (argStr[1] == '-') {
      ParseResult result = ParseOption(FindOption(argStr.Mid(2)), 0, arg);
      if (result < 0)
        return result;
    }
    else {
      for (PINDEX i = 1; i < argStr.GetLength(); i++) {
        ParseResult result = ParseOption(FindOption(argStr[i]), i+1, arg);
        if (result < 0)
          return result;
        if (result == ParseWithArguments)
          break;
      }
    }

    arg++;
  }

  if (optionsBeforeParams)
    m_argsParsed = arg;

  return param > 0 ? ParseWithArguments : ParseNoArguments;
}


size_t PArgList::FindOption(char letter) const
{
  size_t opt;
  for (opt = 0; opt < m_options.size(); ++opt) {
    if (m_options[opt].m_letter == letter)
      break;
  }
  return opt;
}


size_t PArgList::FindOption(const PString & name) const
{
  size_t opt;
  for (opt = 0; opt < m_options.size(); ++opt) {
    if (m_options[opt].m_name == name)
      break;
  }
  return opt;
}


PArgList::ParseResult PArgList::ParseOption(size_t idx, PINDEX offset, PINDEX & arg)
{
  if (idx >= m_options.size())
    return ParseUnknownOption;

  OptionSpec & opt = m_options[idx];
  ++opt.m_count;
  if (opt.m_type == NoString)
    return ParseNoArguments;

  if (!opt.m_string)
    opt.m_string += '\n';

  if (offset != 0 && (opt.m_type == StringWithLetter || m_argumentArray[arg][offset] != '\0')) {
    opt.m_string += m_argumentArray[arg].Mid(offset);
    return ParseWithArguments;
  }

  if (++arg >= m_argumentArray.GetSize())
    return ParseMissingOptionString;

  opt.m_string += m_argumentArray[arg];
  return ParseWithArguments;
}


PINDEX PArgList::GetOptionCount(char option) const
{
  return GetOptionCountByIndex(FindOption(option));
}


PINDEX PArgList::GetOptionCount(const char * option) const
{
  return GetOptionCountByIndex(FindOption(PString(option)));
}


PINDEX PArgList::GetOptionCount(const PString & option) const
{
  return GetOptionCountByIndex(FindOption(option));
}


PINDEX PArgList::GetOptionCountByIndex(size_t idx) const
{
  return idx < m_options.size() ? m_options[idx].m_count : 0;
}


PString PArgList::GetOptionString(char option, const char * dflt) const
{
  return GetOptionStringByIndex(FindOption(option), dflt);
}


PString PArgList::GetOptionString(const char * option, const char * dflt) const
{
  return GetOptionStringByIndex(FindOption(PString(option)), dflt);
}


PString PArgList::GetOptionString(const PString & option, const char * dflt) const
{
  return GetOptionStringByIndex(FindOption(option), dflt);
}


PString PArgList::GetOptionStringByIndex(size_t idx, const char * dflt) const
{
  if (idx < m_options.size() && !m_options[idx].m_string.IsEmpty())
    return m_options[idx].m_string;

  if (dflt != NULL)
    return dflt;

  return PString::Empty();
}


PStringArray PArgList::GetParameters(PINDEX first, PINDEX last) const
{
  PStringArray array;

  last += m_shift;
  if (last < 0)
    return array;

  if (last >= m_parameterIndex.GetSize())
    last = m_parameterIndex.GetSize()-1;

  first += m_shift;
  if (first < 0)
    first = 0;

  if (first > last)
    return array;

  array.SetSize(last-first+1);

  PINDEX idx = 0;
  while (first <= last)
    array[idx++] = m_argumentArray[m_parameterIndex[first++]];

  return array;
}


PString PArgList::GetParameter(PINDEX num) const
{
  int idx = m_shift + (int)num;
  if (idx >= 0 && idx < (int)m_parameterIndex.GetSize())
    return m_argumentArray[m_parameterIndex[idx]];

  return PString::Empty();
}


void PArgList::Shift(int sh) 
{
  m_shift += sh;
  if (m_shift < 0)
    m_shift = 0;
  else if (m_shift > (int)m_parameterIndex.GetSize())
    m_shift = m_parameterIndex.GetSize() - 1;
}


#ifdef P_CONFIG_FILE

///////////////////////////////////////////////////////////////////////////////
// PConfigArgs

PConfigArgs::PConfigArgs(const PArgList & args)
  : PArgList(args),
    sectionName(config.GetDefaultSection()),
    negationPrefix("no-")
{
}


PINDEX PConfigArgs::GetOptionCount(char option) const
{
  PINDEX count;
  if ((count = PArgList::GetOptionCount(option)) > 0)
    return count;

  PString stropt = CharToString(option);
  if (stropt.IsEmpty())
    return 0;

  return GetOptionCount(stropt);
}


PINDEX PConfigArgs::GetOptionCount(const char * option) const
{
  return GetOptionCount(PString(option));
}


PINDEX PConfigArgs::GetOptionCount(const PString & option) const
{
  // if specified on the command line, use that option
  PINDEX count = PArgList::GetOptionCount(option);
  if (count > 0)
    return count;

  // if user has specified "no-option", then ignore config file
  if (PArgList::GetOptionCount(negationPrefix + option) > 0)
    return 0;

  return config.HasKey(sectionName, option) ? 1 : 0;
}


PString PConfigArgs::GetOptionString(char option, const char * dflt) const
{
  if (PArgList::GetOptionCount(option) > 0)
    return PArgList::GetOptionString(option, dflt);

  PString stropt = CharToString(option);
  if (stropt.IsEmpty()) {
    if (dflt != NULL)
      return dflt;
    return PString();
  }

  return GetOptionString(stropt, dflt);
}


PString PConfigArgs::GetOptionString(const char * option, const char * dflt) const
{
  return GetOptionString(PString(option), dflt);
}


PString PConfigArgs::GetOptionString(const PString & option, const char * dflt) const
{
  // if specified on the command line, use that option
  if (PArgList::GetOptionCount(option) > 0)
    return PArgList::GetOptionString(option, dflt);

  // if user has specified "no-option", then ignore config file
  if (PArgList::HasOption(negationPrefix + option)) {
    if (dflt != NULL)
      return dflt;
    return PString();
  }

  return config.GetString(sectionName, option, dflt != NULL ? dflt : "");
}


void PConfigArgs::Save(const PString & saveOptionName)
{
  if (PArgList::GetOptionCount(saveOptionName) == 0)
    return;

  config.DeleteSection(sectionName);

  for (size_t i = 0; i < m_options.size(); i++) {
    PString optionName = m_options[i].m_name;
    if (m_options[i].m_count > 0 && optionName != saveOptionName) {
      if (!m_options[i].m_string.IsEmpty())
        config.SetString(sectionName, optionName, m_options[i].m_string);
      else
        config.SetBoolean(sectionName, optionName, true);
    }
  }
}


PString PConfigArgs::CharToString(char ch) const
{
  size_t index = FindOption(ch);
  return index < m_options.size() ? m_options[index].m_name : PString::Empty();
}

#endif // P_CONFIG_ARGS

///////////////////////////////////////////////////////////////////////////////
// PProcess

PProcess * PProcessInstance = NULL;

int PProcess::InternalMain(void *)
{
  Main();
  return terminationValue;
}


void PProcess::PreInitialise(int c, char ** v, char **)
{
  if (executableFile.IsEmpty()) {
    PString execFile = v[0];
    if (PFile::Exists(execFile))
      executableFile = execFile;
    else {
      execFile += ".exe";
      if (PFile::Exists(execFile))
        executableFile = execFile;
    }
  }

  if (productName.IsEmpty())
    productName = executableFile.GetTitle().ToLower();

  arguments.SetArgs(c-1, v+1);
}


PProcess::PProcess(const char * manuf, const char * name,
                   WORD major, WORD minor, CodeStatus stat, WORD build,
                   bool library, bool suppressStartup)
  : PThread(true)
  , m_library(library)
  , terminationValue(0)
  , manufacturer(manuf)
  , productName(name)
  , majorVersion(major)
  , minorVersion(minor)
  , status(stat)
  , buildNumber(build)
  , maxHandles(INT_MAX)
  , m_shuttingDown(false)
#ifndef P_VXWORKS
  , m_processID(GetCurrentProcessID())
#endif
{
  m_activeThreads[GetCurrentThreadId()] = this;

  PAssert(PProcessInstance == NULL, "Only one instance of PProcess allowed");
  PProcessInstance = this;

#ifdef P_RTEMS

  cout << "Enter program arguments:\n";
  arguments.ReadFrom(cin);

#endif // P_RTEMS

#ifdef _WIN32
  // Try to get the real image path for this process
  TCHAR moduleName[_MAX_PATH];
  if (GetModuleFileName(GetModuleHandle(NULL), moduleName, sizeof(moduleName)) > 0) {
    executableFile = moduleName;
    executableFile.Replace("\\??\\","");
  }

#ifndef __MINGW32__
  PPluginManager::AddPluginDirs(executableFile.GetDirectory());
#endif
#endif

  if (productName.IsEmpty())
    productName = executableFile.GetTitle().ToLower();

  Construct();

  if (!suppressStartup)
    Startup();

#if PMEMORY_HEAP
  // Now we start looking for memory leaks!
  PMemoryHeap::SetIgnoreAllocations(false);
#endif
}


void PProcess::Startup()
{
  PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

  // create one instance of each class registered in the PProcessStartup abstract factory
  // But make sure we have plugins first, to avoid bizarre behaviour where static objects
  // are initialised multiple times when libraries are loaded in Linux.
  PProcessStartupFactory::KeyList_T list = PProcessStartupFactory::GetKeyList();
  std::swap(list.front(), *std::find(list.begin(), list.end(), PLUGIN_LOADER_STARTUP_NAME));
  list.insert(list.begin(), "SetTraceLevel");
  for (PProcessStartupFactory::KeyList_T::const_iterator it = list.begin(); it != list.end(); ++it) {
    PProcessStartup * startup = PProcessStartupFactory::CreateInstance(*it);
    if (startup != NULL)
      startup->OnStartup();
  }
}


void PProcess::PreShutdown()
{
  PProcessInstance->m_shuttingDown = true;
  PProcessStartupFactory::KeyList_T list = PProcessStartupFactory::GetKeyList();
  for (PProcessStartupFactory::KeyList_T::const_iterator it = list.begin(); it != list.end(); ++it)
    PProcessStartupFactory::CreateInstance(*it)->OnShutdown();
}


void PProcess::PostShutdown()
{
  PFactoryBase::GetFactories().DestroySingletons();
  PProcessInstance = NULL;
}


PProcess & PProcess::Current()
{
  if (PProcessInstance == NULL) {
    fputs("Catastrophic failure, PProcess::Current() = NULL!!\n", stderr);
#if defined(_MSC_VER) && defined(_DEBUG) && !defined(_WIN32_WCE) && !defined(_WIN64)
    __asm int 3;
#endif
    _exit(1);
  }
  return *PProcessInstance;
}


void PProcess::OnThreadStart(PThread & /*thread*/)
{
}


static void OutputTime(ostream & strm, const char * name, const PTimeInterval & cpu, const PTimeInterval & real)
{
  strm << ", " << name << '=' << cpu << " (";

  if (real == 0)
    strm << '0';
  else {
    unsigned percent = (unsigned)((cpu.GetMilliSeconds()*1000)/real.GetMilliSeconds());
    if (percent == 0)
      strm << '0';
    else
      strm << (percent/10) << '.' << (percent%10);
  }

  strm << "%)";
}


ostream & operator<<(ostream & strm, const PThread::Times & times)
{
  strm << "real=" << scientific << times.m_real;
  OutputTime(strm, "kernel", times.m_kernel, times.m_real);
  OutputTime(strm, "user", times.m_user, times.m_real);
  OutputTime(strm, "both", times.m_kernel + times.m_user, times.m_real);
  return strm;
}


void PProcess::OnThreadEnded(PThread & PTRACE_PARAM(thread))
{
#if PTRACING
  const int LogLevel = 3;
  if (PTrace::CanTrace(LogLevel)) {
    PThread::Times times;
    if (thread.GetTimes(times)) {
      PTRACE(LogLevel, "PTLib\tThread ended: name=\"" << thread.GetThreadName() << "\", " << times);
    }
  }
#endif
}


bool PProcess::OnInterrupt(bool)
{
  return false;
}


PBoolean PProcess::IsInitialised()
{
  return PProcessInstance != NULL;
}


PObject::Comparison PProcess::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PProcess), PInvalidCast);
  return productName.Compare(((const PProcess &)obj).productName);
}


void PProcess::Terminate()
{
#ifdef _WINDLL
  FatalExit(terminationValue);
#else
  exit(terminationValue);
#endif
}


PString PProcess::GetThreadName() const
{
  return GetName(); 
}
 
 
void PProcess::SetThreadName(const PString & /*name*/)
{
}

PTime PProcess::GetStartTime() const
{ 
  return programStartTime; 
}

PString PProcess::GetVersion(PBoolean full) const
{
  return psprintf(full ? "%u.%u%s%u" : "%u.%u",
                  majorVersion, minorVersion, VersionStatus[status], buildNumber);
}


PString PProcess::GetLibVersion()
{
  return psprintf("%u.%u%s%u (svn:%u)",
                  MAJOR_VERSION,
                  MINOR_VERSION,
                  VersionStatus[BUILD_TYPE],
                  BUILD_NUMBER,
                  SVN_REVISION);
}


void PProcess::SetConfigurationPath(const PString & path)
{
  configurationPaths = path.Tokenise(";:", false);
}

///////////////////////////////////////////////////////////////////////////////

bool PProcess::HostSystemURLHandlerInfo::RegisterTypes(const PString & _types, bool force)
{
  PStringArray types(_types.Lines());

  for (PINDEX i = 0; i < types.GetSize(); ++i) {
    PString type = types[i];
    HostSystemURLHandlerInfo handler(type);
    handler.SetIcon("%base");
    handler.SetCommand("open", "%exe %1");
    if (!handler.CheckIfRegistered()) {
      if (!force)
        return false;
      handler.Register();
    }
  }
  return true;
}

void PProcess::HostSystemURLHandlerInfo::SetIcon(const PString & _icon)
{
#if _WIN32
  PString icon(_icon);
  PFilePath exe(PProcess::Current().GetFile());
  icon.Replace("%exe",  exe, true);
  icon.Replace("%base", exe.GetFileName(), true);
  iconFileName = icon;
#endif
}

PString PProcess::HostSystemURLHandlerInfo::GetIcon() const 
{
#if _WIN32
  return iconFileName;
#else
  return PString();
#endif
}

void PProcess::HostSystemURLHandlerInfo::SetCommand(const PString & key, const PString & _cmd)
{
#if _WIN32
  PString cmd(_cmd);

  // do substitutions
  PFilePath exe(PProcess::Current().GetFile());
  cmd.Replace("%exe", "\"" + exe + "\"", true);
  cmd.Replace("%1",   "\"%1\"", true);

  // save command
  cmds.SetAt(key, cmd);
#endif
}

PString PProcess::HostSystemURLHandlerInfo::GetCommand(const PString & key) const
{
#if _WIN32
  return cmds(key);
#else
  return PString();
#endif
}

bool PProcess::HostSystemURLHandlerInfo::GetFromSystem()
{
#if _WIN32
  if (type.IsEmpty())
    return false;

  // get icon file
  {
    RegistryKey key("HKEY_CLASSES_ROOT\\" + type + "\\DefaultIcon", RegistryKey::ReadOnly);
    key.QueryValue("", iconFileName);
  }

  // enumerate the commands
  {
    PString keyRoot("HKEY_CLASSES_ROOT\\" + type + "\\");
    RegistryKey key(keyRoot + "shell", RegistryKey::ReadOnly);
    PString str;
    for (PINDEX idx = 0; key.EnumKey(idx, str); ++idx) {
      RegistryKey cmd(keyRoot + "shell\\" + str + "\\command", RegistryKey::ReadOnly);
      PString value;
      if (cmd.QueryValue("", value)) 
        cmds.SetAt(str, value);
    }
  }
#endif

  return true;
}

bool PProcess::HostSystemURLHandlerInfo::CheckIfRegistered()
{
#if _WIN32
  // if no type information in system, definitely not registered
  HostSystemURLHandlerInfo currentInfo(type);
  if (!currentInfo.GetFromSystem()) 
    return false;

  // check icon file
  if (!iconFileName.IsEmpty() && !(iconFileName *= currentInfo.GetIcon()))
    return false;

  // check all of the commands
  return (currentInfo.cmds.GetSize() != 0) && (currentInfo.cmds == cmds);
#else
  return true;
#endif
}

bool PProcess::HostSystemURLHandlerInfo::Register()
{
#if _WIN32
  if (type.IsEmpty())
    return false;

  // delete any existing icon name
  {
    RegistryKey key("HKEY_CLASSES_ROOT\\" + type, RegistryKey::ReadOnly);
    key.DeleteKey("DefaultIcon");
  }

  // set icon file
  if (!iconFileName.IsEmpty()) {
    RegistryKey key("HKEY_CLASSES_ROOT\\" + type + "\\DefaultIcon", RegistryKey::Create);
    key.SetValue("", iconFileName);
  }

  // delete existing commands
  PString keyRoot("HKEY_CLASSES_ROOT\\" + type);
  {
    RegistryKey key(keyRoot + "\\shell", RegistryKey::ReadOnly);
    PString str;
    for (PINDEX idx = 0; key.EnumKey(idx, str); ++idx) {
      {
        RegistryKey key(keyRoot + "\\shell\\" + str, RegistryKey::ReadOnly);
        key.DeleteKey("command");
      }
      {
        RegistryKey key(keyRoot + "\\shell", RegistryKey::ReadOnly);
        key.DeleteKey(str);
      }
    }
  }

  // create new commands
  {
    RegistryKey key3(keyRoot, RegistryKey::Create);
    key3.SetValue("", type & "protocol");
    key3.SetValue("URL Protocol", "");

    RegistryKey key2(keyRoot + "\\shell",  RegistryKey::Create);

    for (PStringToString::iterator it = cmds.begin(); it != cmds.end(); ++it) {
      RegistryKey key1(keyRoot + "\\shell\\" + it->first,              RegistryKey::Create);
      RegistryKey key(keyRoot + "\\shell\\" + it->first + "\\command", RegistryKey::Create);
      key.SetValue("", it->second);
    }
  }
#endif

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// PThread

PThread * PThread::Current()
{
  if (!PProcess::IsInitialised())
    return NULL;

  PProcess & process = PProcess::Current();

  PWaitAndSignal mutex(process.m_activeThreadMutex);
  PProcess::ThreadMap::iterator it = process.m_activeThreads.find(GetCurrentThreadId());
  if (it != process.m_activeThreads.end())
    return it->second;

  return process.m_shuttingDown ? NULL : new PExternalThread;
}


void PThread::PrintOn(ostream & strm) const
{
  strm << GetThreadName();
}


PString PThread::GetThreadName() const
{
  PWaitAndSignal mutex(m_threadNameMutex);
  PString reply = m_threadName;
  reply.MakeUnique();
  return reply; 
}

#if defined(_DEBUG) && defined(_MSC_VER) && !defined(_WIN32_WCE)

static void SetWinDebugThreadName(const char * threadName, DWORD threadId)
{
  struct THREADNAME_INFO
  {
    DWORD dwType;      // must be 0x1000
    LPCSTR szName;     // pointer to name (in user addr space)
    DWORD dwThreadID;  // thread ID (-1=caller thread, but seems to set more than one thread's name)
    DWORD dwFlags;     // reserved for future use, must be zero
  } threadInfo = { 0x1000, threadName, threadId, 0 };

  __try
  {
    RaiseException(0x406D1388, 0, sizeof(threadInfo)/sizeof(DWORD), (const ULONG_PTR *)&threadInfo) ;
    // if not running under debugger exception comes back
  }
  __except(EXCEPTION_CONTINUE_EXECUTION)
  {
    // just keep on truckin'
  }
}

#else

#define SetWinDebugThreadName(p1,p2)

#endif // defined(_DEBUG) && defined(_MSC_VER) && !defined(_WIN32_WCE)


void PThread::SetThreadName(const PString & name)
{
  PWaitAndSignal mutex(m_threadNameMutex);

  PThreadIdentifier threadId = GetThreadId();
  if (name.Find('%') != P_MAX_INDEX)
    m_threadName = psprintf(name, threadId);
  else if (name.IsEmpty()) {
    m_threadName = GetClass();
    m_threadName.sprintf(PTHREAD_ID_FMT, threadId);
  }
  else {
    PString idStr;
    idStr.sprintf(PTHREAD_ID_FMT, threadId);

    m_threadName = name;
    if (m_threadName.Find(idStr) == P_MAX_INDEX)
      m_threadName += idStr;
  }

  SetWinDebugThreadName(m_threadName, threadId);
}
 
PThread * PThread::Create(const PNotifier & notifier,
                          INT parameter,
                          AutoDeleteFlag deletion,
                          Priority priorityLevel,
                          const PString & threadName,
                          PINDEX stackSize)
{
  PThread * thread = new PSimpleThread(notifier,
                                       parameter,
                                       deletion,
                                       priorityLevel,
                                       threadName,
                                       stackSize);
  if (deletion != AutoDeleteThread)
    return thread;

  // Do not return a pointer to the thread if it is auto-delete as this
  // pointer is extremely dangerous to use, it could be deleted at any moment
  // from now on so using the pointer could crash the program.
  return NULL;
}


PSimpleThread::PSimpleThread(const PNotifier & notifier,
                             INT param,
                             AutoDeleteFlag deletion,
                             Priority priorityLevel,
                             const PString & threadName,
                             PINDEX stackSize)
  : PThread(stackSize, deletion, priorityLevel, threadName),
    callback(notifier),
    parameter(param)
{
  Resume();
}


void PSimpleThread::Main()
{
  callback(*this, parameter);
}

/////////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////////

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


PBoolean PIntCondMutex::Condition()
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


/////////////////////////////////////////////////////////////////////////////

PReadWriteMutex::PReadWriteMutex()
  : readerSemaphore(1, 1),
    writerSemaphore(1, 1)
{
  readerCount = 0;
  writerCount = 0;
  PTRACE(5, "PTLib\tCreated read/write mutex " << this);
}


PReadWriteMutex::~PReadWriteMutex()
{
  PTRACE(5, "PTLib\tDestroying read/write mutex " << this);

  EndNest(); // Destruction while current thread has a lock is OK

  /* There is a small window during destruction where another thread is on the
     way out of EndRead() or EndWrite() where it checks for nested locks.
     While the check is protected by mutex, there is a moment between one
     check and the next where the object is unlocked. This is normally fine,
     except for if a thread then goes and deletes the object out from under
     the threads about to do the second check.

     Note if this goes into an endless loop then there is a big problem with
     the user of the PReadWriteMutex, as it must be CONTINUALLY trying to use
     the object when someone wants it gone. Technically this fix should be
     done by the user of the class too, but it is easier to fix here than
     there so practicality wins out!
   */
  while (!m_nestedThreads.empty())
    PThread::Sleep(10);
}


PReadWriteMutex::Nest * PReadWriteMutex::GetNest()
{
  PWaitAndSignal mutex(m_nestingMutex);
  NestMap::iterator it = m_nestedThreads.find(PThread::GetCurrentThreadId());
  return it != m_nestedThreads.end() ? &it->second : NULL;
}


void PReadWriteMutex::EndNest()
{
  m_nestingMutex.Wait();
  m_nestedThreads.erase(PThread::GetCurrentThreadId());
  m_nestingMutex.Signal();
}


PReadWriteMutex::Nest & PReadWriteMutex::StartNest()
{
  PWaitAndSignal mutex(m_nestingMutex);
  // The std::map will create the entry if it doesn't exist
  return m_nestedThreads[PThread::GetCurrentThreadId()];
}


void PReadWriteMutex::StartRead()
{
  // Get the nested thread info structure, create one it it doesn't exist
  Nest & nest = StartNest();

  // One more nested call to StartRead() by this thread, note this does not
  // need to be mutexed as it is always in the context of a single thread.
  nest.readerCount++;

  // If this is the first call to StartRead() and there has not been a
  // previous call to StartWrite() then actually do the text book read only
  // lock, otherwise we leave it as just having incremented the reader count.
  if (nest.readerCount == 1 && nest.writerCount == 0)
    InternalStartRead();
}


void PReadWriteMutex::InternalWait(PSemaphore & semaphore) const
{
#if PTRACING
  if (semaphore.Wait(15000))
    return;

  ostream & trace = PTrace::Begin(1, __FILE__, __LINE__, this);
  trace << "PTLib\tPossible deadlock in read/write mutex " << this << " :\n";
  for (std::map<PThreadIdentifier, Nest>::const_iterator it = m_nestedThreads.begin(); it != m_nestedThreads.end(); ++it)
    trace << "  thread-id=" << it->first << " (0x" << std::hex << it->first << std::dec << "),"
              " readers=" << it->second.readerCount << ","
              " writers=" << it->second.writerCount << '\n';
  trace << PTrace::End;
#endif

  semaphore.Wait();
}


void PReadWriteMutex::InternalStartRead()
{
  // Text book read only lock

  starvationPreventer.Wait();
   InternalWait(readerSemaphore);
    readerMutex.Wait();

     readerCount++;
     if (readerCount == 1)
       InternalWait(writerSemaphore);

    readerMutex.Signal();
   readerSemaphore.Signal();
  starvationPreventer.Signal();
}


void PReadWriteMutex::EndRead()
{
  // Get the nested thread info structure for the curent thread
  Nest * nest = GetNest();

  // If don't have an active read or write lock or there is a write lock but
  // the StartRead() was never called, then assert and ignore call.
  if (nest == NULL || nest->readerCount == 0) {
    PAssertAlways("Unbalanced PReadWriteMutex::EndRead()");
    return;
  }

  // One less nested lock by this thread, note this does not
  // need to be mutexed as it is always in the context of a single thread.
  nest->readerCount--;

  // If this is a nested read or a write lock is present then we don't do the
  // real unlock, the decrement is enough.
  if (nest->readerCount > 0 || nest->writerCount > 0)
    return;

  // Do text book read lock
  InternalEndRead();

  // At this point all read and write locks are gone for this thread so we can
  // reclaim the memory.
  EndNest();
}


void PReadWriteMutex::InternalEndRead()
{
  // Text book read only unlock

  readerMutex.Wait();

  readerCount--;
  if (readerCount == 0)
    writerSemaphore.Signal();

  readerMutex.Signal();
}


void PReadWriteMutex::StartWrite()
{
  // Get the nested thread info structure, create one it it doesn't exist
  Nest & nest = StartNest();

  // One more nested call to StartWrite() by this thread, note this does not
  // need to be mutexed as it is always in the context of a single thread.
  nest.writerCount++;

  // If is a nested call to StartWrite() then simply return, the writer count
  // increment is all we haev to do.
  if (nest.writerCount > 1)
    return;

  // If have a read lock already in this thread then do the "real" unlock code
  // but do not change the lock count, calls to EndRead() will now just
  // decrement the count instead of doing the unlock (its already done!)
  if (nest.readerCount > 0)
    InternalEndRead();

  // Note in this gap another thread could grab the write lock, thus

  // Now do the text book write lock
  writerMutex.Wait();

  writerCount++;
  if (writerCount == 1)
    InternalWait(readerSemaphore);

  writerMutex.Signal();

  InternalWait(writerSemaphore);
}


void PReadWriteMutex::EndWrite()
{
  // Get the nested thread info structure for the curent thread
  Nest * nest = GetNest();

  // If don't have an active read or write lock or there is a read lock but
  // the StartWrite() was never called, then assert and ignore call.
  if (nest == NULL || nest->writerCount == 0) {
    PAssertAlways("Unbalanced PReadWriteMutex::EndWrite()");
    return;
  }

  // One less nested lock by this thread, note this does not
  // need to be mutexed as it is always in the context of a single thread.
  nest->writerCount--;

  // If this is a nested write lock then the decrement is enough and we
  // don't do the actual write unlock.
  if (nest->writerCount > 0)
    return;

  // Begin text book write unlock
  writerSemaphore.Signal();

  writerMutex.Wait();

  writerCount--;
  if (writerCount == 0)
    readerSemaphore.Signal();

  writerMutex.Signal();
  // End of text book write unlock

  // Now check to see if there was a read lock present for this thread, if so
  // then reacquire the read lock (not changing the count) otherwise clean up the
  // memory for the nested thread info structure
  if (nest->readerCount > 0)
    InternalStartRead();
  else
    EndNest();
}


/////////////////////////////////////////////////////////////////////////////

PReadWaitAndSignal::PReadWaitAndSignal(const PReadWriteMutex & rw, PBoolean start)
  : mutex((PReadWriteMutex &)rw)
{
  if (start)
    mutex.StartRead();
}


PReadWaitAndSignal::~PReadWaitAndSignal()
{
  mutex.EndRead();
}


/////////////////////////////////////////////////////////////////////////////

PWriteWaitAndSignal::PWriteWaitAndSignal(const PReadWriteMutex & rw, PBoolean start)
  : mutex((PReadWriteMutex &)rw)
{
  if (start)
    mutex.StartWrite();
}


PWriteWaitAndSignal::~PWriteWaitAndSignal()
{
  mutex.EndWrite();
}

// End Of File ///////////////////////////////////////////////////////////////
