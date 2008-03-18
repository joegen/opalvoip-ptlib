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

#include <ctype.h>
#include <ptlib/pfactory.h>
#include <ptlib/pprocess.h>

#ifdef _WIN32
#include <ptlib/msos/ptlib/debstrm.h>
#endif

#ifdef __MACOSX__
namespace PWLibStupidOSXHacks {
  extern int loadShmVideoStuff;
  extern int loadCoreAudioStuff;
  extern int loadFakeVideoStuff;
};
#endif

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
  const char    * filename;
  ostream       * stream;
  PTimeInterval   startTick;
  const char    * rolloverPattern;
  unsigned        lastDayOfYear;
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



  PTraceInfo()
    : currentLevel(0)
    , filename(NULL)
#ifdef __NUCLEUS_PLUS__
    , stream(NULL)
#else
    , stream(&cerr)
#endif
    , startTick(PTimer::Tick())
    , rolloverPattern("yyyy_MM_dd")
    , lastDayOfYear(0)
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

  void OpenTraceFile(const char * newFilename)
  {
    if (newFilename != NULL)
      filename = newFilename;

    if (filename == NULL)
      return;

    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

    if (strcasecmp(filename, "stderr") == 0)
      SetStream(&cerr);
    else if (strcasecmp(filename, "stdout") == 0)
      SetStream(&cout);
#ifdef _WIN32
    else if (strcasecmp(filename, "DEBUGSTREAM") == 0)
      SetStream(new PDebugStream);
#endif
    else {
      PFilePath fn(filename);
      fn.Replace("%P", PString((unsigned int) PProcess::Current().GetProcessID()));
     
      if ((options & PTrace::RotateDaily) != 0)
      {
          PTime now;
          fn = PFilePath(fn.GetDirectory() + fn.GetTitle() + now.AsString(rolloverPattern, ((options&PTrace::GMTTime) ? PTime::GMT : PTime::Local)) + fn.GetType());
      }

      PTextFile * traceOutput;
      if (options & PTrace::AppendToFile) {
        traceOutput = new PTextFile(fn, PFile::ReadWrite);
        traceOutput->SetPosition(0, PFile::End);
      }
      else 
        traceOutput = new PTextFile(fn, PFile::WriteOnly);

      if (traceOutput->IsOpen())
        SetStream(traceOutput);
      else {
        PTRACE(0, PProcess::Current().GetName() << "Could not open trace output file \"" << fn << '"');
        delete traceOutput;
      }
    }
  }
};


void PTrace::SetStream(ostream * s)
{
  PTraceInfo::Instance().SetStream(s);
}

void PTrace::Initialise(
    unsigned level,
    const char * filename,
    unsigned options
)
{
  Initialise(level, filename, NULL, options);
}

void PTrace::Initialise(unsigned level, const char * filename, const char * rolloverPattern, unsigned options)
{
  PTraceInfo & info = PTraceInfo::Instance();

  info.options = options;
  info.thresholdLevel = level;
  info.rolloverPattern = rolloverPattern != NULL ? rolloverPattern : "yyyy_MM_dd";
  // Does PTime::GetDayOfYear() etc. want to take zone param like PTime::AsString() to switch 
  // between os_gmtime and os_localtime?
  info.lastDayOfYear = (options & RotateDaily) != 0 ? PTime().GetDayOfYear() : 0;

  info.OpenTraceFile(filename);

#if PTRACING
  PProcess & process = PProcess::Current();
  Begin(0, "", 0) << "\tVersion " << process.GetVersion(PTrue)
                  << " by " << process.GetManufacturer()
                  << " on " << process.GetOSClass() << ' ' << process.GetOSName()
                  << " (" << process.GetOSVersion() << '-' << process.GetOSHardware()
                  << ") at " << PTime().AsString("yyyy/M/d h:mm:ss.uuu")
                  << End;
#endif

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
  return level <= PTraceInfo::Instance().thresholdLevel;
}


ostream & PTrace::Begin(unsigned level, const char * fileName, int lineNum)
{
  PTraceInfo & info = PTraceInfo::Instance();

  if (level == UINT_MAX)
    return *info.stream;

  info.Lock();

  if ((info.filename != NULL) && (info.options&RotateDaily) != 0) {
    unsigned day = PTime().GetDayOfYear();
    if (day != info.lastDayOfYear) {
      info.OpenTraceFile(NULL);
      info.lastDayOfYear = day;
      if (info.stream == NULL)
        info.SetStream(&cerr);
    }
  }

  PThread * thread = PThread::Current();

  if (thread != NULL)
    thread->traceStreams.Push(new PStringStream);

  ostream & stream = thread != NULL ? (ostream &)thread->traceStreams.Top() : *info.stream;

  info.oldStreamFlags = stream.flags();
  info.oldPrecision = stream.precision();

  // Before we do new trace, make sure we clear any errors on the stream
  stream.clear();

  if ((info.options&SystemLogStream) == 0) {
    if ((info.options&DateAndTime) != 0) {
      PTime now;
      stream << now.AsString("yyyy/MM/dd hh:mm:ss.uuu\t", (info.options&GMTTime) ? PTime::GMT : PTime::Local);
    }

    if ((info.options&Timestamp) != 0)
      stream << setprecision(3) << setw(10) << (PTimer::Tick()-info.startTick) << '\t';

    if ((info.options&Thread) != 0) {
      if (thread == NULL)
        stream << "ThreadID=0x"
               << setfill('0') << hex << setw(8)
               << PThread::GetCurrentThreadId()
               << setfill(' ') << dec;
      else {
        PString name = thread->GetThreadName();
        if (name.GetLength() <= 23)
          stream << setw(23) << name;
        else
          stream << name.Left(10) << "..." << name.Right(10);
      }
      stream << '\t';
    }

    if ((info.options&ThreadAddress) != 0)
      stream << hex << setfill('0')
             << setw(7) << (void *)PThread::Current()
             << dec << setfill(' ') << '\t';
  }

  if ((info.options&TraceLevel) != 0)
    stream << level << '\t';

  if ((info.options&FileAndLine) != 0 && fileName != NULL) {
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

    stream << setw(16) << file << '(' << lineNum << ")\t";
  }

  // Save log level for this message so End() function can use. This is
  // protected by the PTraceMutex or is thread local
  if (thread == NULL)
    info.currentLevel = level;
  else {
    thread->traceLevel = level;
    info.Unlock();
  }

  return stream;
}


ostream & PTrace::End(ostream & paramStream)
{
  PTraceInfo & info = PTraceInfo::Instance();

  paramStream.flags(info.oldStreamFlags);
  paramStream.precision(info.oldPrecision);

  PThread * thread = PThread::Current();

  if (thread != NULL) {
    PStringStream * stackStream = thread->traceStreams.Pop();
    PAssert(&paramStream == stackStream, PLogicError);
    info.Lock();
    *info.stream << *stackStream;
    delete stackStream;
  }
  else {
    PAssert(&paramStream == info.stream, PLogicError);
  }

  if ((info.options&SystemLogStream) != 0) {
    // Get the trace level for this message and set the stream width to that
    // level so that the PSystemLog can extract the log level back out of the
    // ios structure. There could be portability issues with this though it
    // should work pretty universally.
    info.stream->width((thread != NULL ? thread->traceLevel : info.currentLevel) + 1);
    info.stream->flush();
  }
  else
    *info.stream << endl;

  info.Unlock();
  return paramStream;
}


PTrace::Block::Block(const char * fileName, int lineNum, const char * traceName)
{
  file = fileName;
  line = lineNum;
  name = traceName;

  if ((PTraceInfo::Instance().options&Blocks) != 0) {
    PThread * thread = PThread::Current();
    thread->traceBlockIndentLevel += 2;

    ostream & s = PTrace::Begin(1, file, line);
    s << "B-Entry\t";
    for (unsigned i = 0; i < thread->traceBlockIndentLevel; i++)
      s << '=';
    s << "> " << name << PTrace::End;
  }
}


PTrace::Block::~Block()
{
  if ((PTraceInfo::Instance().options&Blocks) != 0) {
    PThread * thread = PThread::Current();

    ostream & s = PTrace::Begin(1, file, line);
    s << "B-Exit\t<";
    for (unsigned i = 0; i < thread->traceBlockIndentLevel; i++)
      s << '=';
    s << ' ' << name << PTrace::End;

    thread->traceBlockIndentLevel -= 2;
  }
}

#endif // PTRACING


///////////////////////////////////////////////////////////////////////////////
// PDirectory

void PDirectory::CloneContents(const PDirectory * d)
{
  CopyContents(*d);
}


///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

DWORD PTimeInterval::GetInterval() const
{
  if (milliseconds <= 0)
    return 0;

  if (milliseconds >= UINT_MAX)
    return UINT_MAX;

  return (DWORD)milliseconds;
}


///////////////////////////////////////////////////////////////////////////////
// PTimer

PTimer::PTimer(long millisecs, int seconds, int minutes, int hours, int days)
  : resetTime(millisecs, seconds, minutes, hours, days)
{
  Construct();
}


PTimer::PTimer(const PTimeInterval & time)
  : resetTime(time)
{
  Construct();
}


void PTimer::Construct()
{
  timerList = PProcess::Current().GetTimerList();
  timerId = timerList->GetNewTimerId();
  state = Stopped;

  StartRunning(PTrue);
}


PTimer & PTimer::operator=(DWORD milliseconds)
{
  resetTime.SetInterval(milliseconds);
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
  // queue a request to remove this timer, and always do it synchronously
  timerList->QueueRequest(PTimerList::RequestType::Stop, this, true);
}


void PTimer::SetInterval(PInt64 milliseconds,
                         long seconds,
                         long minutes,
                         long hours,
                         int days)
{
  resetTime.SetInterval(milliseconds, seconds, minutes, hours, days);
  StartRunning(oneshot);
}


void PTimer::RunContinuous(const PTimeInterval & time)
{
  resetTime = time;
  StartRunning(PFalse);
}


void PTimer::StartRunning(PBoolean once)
{
  PTimeInterval::operator=(resetTime);
  oneshot = once;
  state = (*this) != 0 ? Starting : Stopped;

  if (IsRunning())
    timerList->QueueRequest(PTimerList::RequestType::Start, this, false);
  else if (state != Stopped)
    timerList->QueueRequest(PTimerList::RequestType::Stop, this);
}


void PTimer::Stop(bool wait)
{
  state = Stopped;
  milliseconds = 0;

  timerList->QueueRequest(PTimerList::RequestType::Stop, this, wait);
}


void PTimer::Pause()
{
  if (IsRunning()) {
    state = Paused;
    timerList->QueueRequest(PTimerList::RequestType::Stop, this);
  }
}


void PTimer::Resume()
{
  if (state == Paused) {
    state = Starting;
    timerList->QueueRequest(PTimerList::RequestType::Start, this);
  }
}


void PTimer::Reset()
{
  StartRunning(oneshot);
}

// called only from the timer thread
void PTimer::OnTimeout()
{
  if (!callback.IsNULL())
    callback(*this, IsRunning());
}


void PTimer::Process(const PTimeInterval & delta, PTimeInterval & minTimeLeft)
{
  switch (state) {
    case Starting :
      state = Running;
      if (resetTime < minTimeLeft)
        minTimeLeft = resetTime;
      break;

    case Running :
      operator-=(delta);

      if (milliseconds > 0) {
        if (milliseconds < minTimeLeft.GetMilliSeconds())
          minTimeLeft = *this;
      }
      else {
        if (oneshot) {
          milliseconds = 0;
          state = Stopped;
        }
        else {
          PTimeInterval::operator=(resetTime);
          if (resetTime < minTimeLeft)
            minTimeLeft = resetTime;
        }

        /* This must be outside the mutex or if OnTimeout() changes the
           timer value (quite plausible) it deadlocks.
         */
        OnTimeout();
        return;
      }
      break;

    default : // Stopped or Paused, do nothing.
      break;
  }
}


///////////////////////////////////////////////////////////////////////////////
// PTimerList

PTimerList::PTimerList()
{
  timerThread = NULL;
}

void PTimerList::QueueRequest(RequestType::Action action, PTimer * timer, bool _isSync)
{
  // if this operation is occurring in the timer thread, then handle synchronoously
  if (timerThread == PThread::Current()) {
    switch (action) {
      case PTimerList::RequestType::Start:
        {
          TimerInfoMapType::iterator r = timerInfoMap.find(timer->GetTimerId());
          if (r == timerInfoMap.end())
            timerInfoMap.insert(TimerInfoMapType::value_type(timer->GetTimerId(), TimerInfoType(timer)));
        }
        break;
      case PTimerList::RequestType::Stop:
        {
          TimerInfoMapType::iterator r = timerInfoMap.find(timer->GetTimerId());
          if (r != timerInfoMap.end())
            r->second.removed = true;
        }
        break;
    }
    return;
  }

  // handle asynchronoously
  RequestType request(action, timer);
  PSyncPoint sync;
  bool isSync = false;
  if (_isSync) {
    request.sync = &sync;
    isSync = true;
  }

  {
    PWaitAndSignal m(queueMutex);
    requestQueue.push(request);
  }

  PProcess::Current().SignalTimerChange();

  if (isSync)
    sync.Wait();
}

PTimeInterval PTimerList::Process()
{
  timerThread = PThread::Current();

  PWaitAndSignal l(timerListMutex);

  // process the requests in the timer request queue
  {
    PWaitAndSignal q(queueMutex);
    while (requestQueue.size() > 0) {
      RequestType request = requestQueue.front();
      requestQueue.pop();
      TimerInfoMapType::iterator r = timerInfoMap.find(request.id);
      switch (request.action) {
        case PTimerList::RequestType::Start:
          if (r == timerInfoMap.end()) 
            timerInfoMap.insert(TimerInfoMapType::value_type(request.id, TimerInfoType(request.timer)));
          break;
        case PTimerList::RequestType::Stop:
          if (r != timerInfoMap.end()) 
            timerInfoMap.erase(r);
          break;
        default:
          PAssertAlways("unknown timer request code");
          break;
      }
      if (request.sync != NULL)
        request.sync->Signal();
    }
  }

  // calculate interval since last processing, and update time of last processing to now
  PTimeInterval now = PTimer::Tick();
  PTimeInterval sampleTime;
  if (lastSample == 0 || lastSample > now)
    sampleTime = 0;
  else {
    sampleTime = now - lastSample;
    if (now < lastSample)
      sampleTime += PMaxTimeInterval;
  }
  lastSample = now;

  // process the timers and find the minimum amount of time remaining
  // also remove any timer labelled as removed
  PTimeInterval minTimeLeft = PMaxTimeInterval;
  {
    TimerInfoMapType::iterator r = timerInfoMap.begin();
    while (r != timerInfoMap.end()) {
      PTimeInterval oldMinTimeLeft(minTimeLeft);
      if (!r->second.removed) 
        r->second.timer->Process(sampleTime, minTimeLeft);
      if (!r->second.removed) 
        ++r;
      else {
        if (r == timerInfoMap.begin()) {
          timerInfoMap.erase(r);
          r = timerInfoMap.begin();
        }
        else
        {
          TimerInfoMapType::iterator s = r;
          --s;
          timerInfoMap.erase(r);
          r = s;
        }
        minTimeLeft = oldMinTimeLeft;
      }
    }
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
  for (PINDEX i = 0; i < argumentArray.GetSize(); i++) {
    if (i > 0)
      strm << strm.fill();
    strm << argumentArray[i];
  }
}


void PArgList::ReadFrom(istream & strm)
{
  PString line;
  strm >> line;
  SetArgs(line);
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


PBoolean PArgList::Parse(const char * spec, PBoolean optionsBeforeParams)
{
  if (PAssertNULL(spec) == NULL)
    return PFalse;

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
  PBoolean hadMinusMinus = PFalse;
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
      hadMinusMinus = PTrue;
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


PBoolean PArgList::ParseOption(PINDEX idx, PINDEX offset, PINDEX & arg,
                           const PIntArray & canHaveOptionString)
{
  if (idx == P_MAX_INDEX) {
    UnknownOption(argumentArray[arg]);
    return PFalse;
  }

  optionCount[idx]++;
  if (canHaveOptionString[idx] == 0)
    return PFalse;

  if (!optionString[idx])
    optionString[idx] += '\n';

  if (offset != 0 &&
        (canHaveOptionString[idx] == 1 || argumentArray[arg][offset] != '\0')) {
    optionString[idx] += argumentArray[arg].Mid(offset);
    return PTrue;
  }

  if (++arg >= argumentArray.GetSize())
    return PFalse;

  optionString[idx] += argumentArray[arg];
  return PTrue;
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


PStringArray PArgList::GetParameters(PINDEX first, PINDEX last) const
{
  PStringArray array;

  last += shift;
  if (last < 0)
    return array;

  if (last >= parameterIndex.GetSize())
    last = parameterIndex.GetSize()-1;

  first += shift;
  if (first < 0)
    first = 0;

  if (first > last)
    return array;

  array.SetSize(last-first+1);

  PINDEX idx = 0;
  while (first <= last)
    array[idx++] = argumentArray[parameterIndex[first++]];

  return array;
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
  else if (shift > (int)parameterIndex.GetSize())
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

  for (PINDEX i = 0; i < optionCount.GetSize(); i++) {
    PString optionName = optionNames[i];
    if (optionCount[i] > 0 && optionName != saveOptionName) {
      if (optionString.GetAt(i) != NULL)
        config.SetString(sectionName, optionName, optionString[i]);
      else
        config.SetBoolean(sectionName, optionName, PTrue);
    }
  }
}


PString PConfigArgs::CharToString(char ch) const
{
  PINDEX index = optionLetters.Find(ch);
  if (index == P_MAX_INDEX)
    return PString();

  if (optionNames.GetAt(index) == NULL)
    return PString();

  return optionNames[index];
}

#endif // P_CONFIG_ARGS

///////////////////////////////////////////////////////////////////////////////
// PProcess

PProcess * PProcessInstance;
int PProcess::p_argc;
char ** PProcess::p_argv;
char ** PProcess::p_envp;

typedef std::map<PString, PProcessStartup *> PProcessStartupList;

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


static PProcessStartupList & GetPProcessStartupList()
{
  static PProcessStartupList list;
  return list;
}


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

#ifdef P_RTEMS

  cout << "Enter program arguments:\n";
  arguments.ReadFrom(cin);

#else // P_RTEMS

  if (p_argv != 0 && p_argc > 0) {
    executableFile = p_argv[0];
    arguments.SetArgs(p_argc-1, p_argv+1);
  }

#ifdef _WIN32
  if (executableFile.IsEmpty()) {
    // Try to get the real image path for this process
    PVarString moduleName;
    if (GetModuleFileName(GetModuleHandle(NULL), moduleName.GetPointer(1024), 1024) > 0) {
      executableFile = moduleName;
      executableFile.Replace("\\??\\","");
    }
  }
  else {
    if (!PFile::Exists(executableFile)) {
      PString execFile = executableFile + ".exe";
      if (PFile::Exists(execFile))
        executableFile = execFile;
    }
  }
#endif // _WIN32

  if (productName.IsEmpty())
    productName = executableFile.GetTitle().ToLower();

#endif // P_RTEMS

  InitialiseProcessThread();

  Construct();

#ifdef __MACOSX__
  
#ifdef HAS_VIDEO
  PWLibStupidOSXHacks::loadFakeVideoStuff = 1;
#ifdef USE_SHM_VIDEO_DEVICES
  PWLibStupidOSXHacks::loadShmVideoStuff = 1;
#endif // USE_SHM_VIDEO_DEVICES
#endif // HAS_VIDEO
  
#ifdef HAS_AUDIO
  PWLibStupidOSXHacks::loadCoreAudioStuff = 1;
#endif // HAS_AUDIO
  
#endif // __MACOSX__

  // create one instance of each class registered in the 
  // PProcessStartup abstract factory
  PProcessStartupList & startups = GetPProcessStartupList();
  {
    PProcessStartup * levelSet = PFactory<PProcessStartup>::CreateInstance("SetTraceLevel");
    if (levelSet != NULL) 
      levelSet->OnStartup();

    PProcessStartupFactory::KeyList_T list = PProcessStartupFactory::GetKeyList();
    PProcessStartupFactory::KeyList_T::const_iterator r;
    for (r = list.begin(); r != list.end(); ++r) {
      if (*r != "SetTraceLevel") {
        PProcessStartup * instance = PProcessStartupFactory::CreateInstance(*r);
        instance->OnStartup();
        startups.insert(std::pair<PString, PProcessStartup *>(*r, instance));
      }
    }
  }

#if PMEMORY_HEAP
  // Now we start looking for memory leaks!
  PMemoryHeap::SetIgnoreAllocations(PFalse);
#endif
}


void PProcess::PreShutdown()
{
  PProcessStartupList & startups = GetPProcessStartupList();
  for (PProcessStartupList::iterator startup = startups.begin(); startup != startups.end(); ++startup)
    startup->second->OnShutdown();
}


void PProcess::PostShutdown()
{
  PProcessStartupList & startups = GetPProcessStartupList();
  for (PProcessStartupList::iterator startup = startups.begin(); startup != startups.end(); ++startup)
    delete startup->second;

  startups.clear();
}


PProcess & PProcess::Current()
{
  if (PProcessInstance == NULL) {
    cerr << "Catastrophic failure, PProcess::Current() = NULL!!\n";
#if defined(_MSC_VER) && defined(_DEBUG) && !defined(_WIN32_WCE)
    __asm int 3;
#endif
    _exit(1);
  }
  return *PProcessInstance;
}


void PProcess::OnThreadStart(PThread & /*thread*/)
{
}


void PProcess::OnThreadEnded(PThread & /*thread*/)
{
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
  const char * const statusLetter[NumCodeStatuses] =
    { "alpha", "beta", "." };
  return psprintf(full ? "%u.%u%s%u" : "%u.%u",
                  majorVersion, minorVersion, statusLetter[status], buildNumber);
}


void PProcess::SetConfigurationPath(const PString & path)
{
  configurationPaths = path.Tokenise(";:", PFalse);
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
    RegistryKey key3(keyRoot,            RegistryKey::Create);
    key3.SetValue("", type & "protocol");
    key3.SetValue("URL Protocol", "");

    RegistryKey key2(keyRoot + "\\shell",  RegistryKey::Create);

    for (PINDEX i = 0; i < cmds.GetSize(); ++i) {
      RegistryKey key1(keyRoot + "\\shell\\" + cmds.GetKeyAt(i),              RegistryKey::Create);
      RegistryKey key(keyRoot + "\\shell\\" + cmds.GetKeyAt(i) + "\\command", RegistryKey::Create);
      key.SetValue("", cmds.GetDataAt(i));
    }
  }
#endif

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// PThread

void PThread::PrintOn(ostream & strm) const
{
  strm << GetThreadName();
}


PString PThread::GetThreadName() const
{
  return threadName; 
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
    RaiseException(0x406D1388, 0, sizeof(threadInfo)/sizeof(DWORD), (DWORD *)&threadInfo) ;
    // if not running under debugger exception comes back
  }
  __except(EXCEPTION_CONTINUE_EXECUTION)
  {
    // just keep on truckin'
  }
}

#else
#define SetWinDebugThreadName(p1,p2)
#endif // defined(_DEBUG) && defined(_MSC_VER)


void PThread::SetThreadName(const PString & name)
{
  if (name.IsEmpty())
    threadName = psprintf("%s:%u", GetClass(), GetThreadId());
  else
    threadName = psprintf(name, GetThreadId());

  SetWinDebugThreadName(threadName, threadId);
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
}


PReadWriteMutex::~PReadWriteMutex()
{
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
  while (!nestedThreads.IsEmpty())
    PThread::Sleep(10);
}


PReadWriteMutex::Nest * PReadWriteMutex::GetNest() const
{
  PWaitAndSignal mutex(nestingMutex);
  return nestedThreads.GetAt(POrdinalKey((PINDEX)PThread::GetCurrentThreadId()));
}


void PReadWriteMutex::EndNest()
{
  nestingMutex.Wait();
  nestedThreads.RemoveAt(POrdinalKey((PINDEX)PThread::GetCurrentThreadId()));
  nestingMutex.Signal();
}


PReadWriteMutex::Nest & PReadWriteMutex::StartNest()
{
  POrdinalKey threadId = (PINDEX)PThread::GetCurrentThreadId();

  nestingMutex.Wait();

  Nest * nest = nestedThreads.GetAt(threadId);

  if (nest == NULL) {
    nest = new Nest;
    nestedThreads.SetAt(threadId, nest);
  }

  nestingMutex.Signal();

  return *nest;
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


void PReadWriteMutex::InternalStartRead()
{
  // Text book read only lock

  starvationPreventer.Wait();
   readerSemaphore.Wait();
    readerMutex.Wait();

     readerCount++;
     if (readerCount == 1)
       writerSemaphore.Wait();

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
    readerSemaphore.Wait();

  writerMutex.Signal();

  writerSemaphore.Wait();
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
