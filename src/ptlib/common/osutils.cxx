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
#include <ptlib/id_generator.h>
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
      PTRACE(5, "PTLib\tCreated external thread " << this << ", id=" << GetCurrentThreadId());
    }

    ~PExternalThread()
    {
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

class PTraceInfo : public PTrace
{
  /* NOTE you cannot have any complex types in this structure. Anything
     that might do an asert or PTRACE will crash due to recursion.
   */

public:
  unsigned        m_currentLevel;
  unsigned        m_thresholdLevel;
  unsigned        m_options;
  PCaselessString m_filename;
  ostream       * m_stream;
  PTimeInterval   m_startTick;
  PString         m_rolloverPattern;
  unsigned        m_lastRotate;
  ios::fmtflags   m_oldStreamFlags;
  std::streamsize m_oldPrecision;


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
      , m_prefixLength(0)
    { }

    PStack<PStringStream> m_traceStreams;
    unsigned              m_traceLevel;
    unsigned              m_traceBlockIndentLevel;
    PINDEX                m_prefixLength;
  };
  PThreadLocalStorage<ThreadLocalInfo> m_threadStorage;

  PTraceInfo()
    : m_currentLevel(0)
    , m_thresholdLevel(0)
    , m_options(Blocks | Timestamp | Thread | FileAndLine | HasFilePermissions | (PFileInfo::DefaultPerms << FilePermissionShift))
#ifdef __NUCLEUS_PLUS__
    , m_stream(NULL)
#else
    , m_stream(&cerr)
#endif
    , m_startTick(PTimer::Tick())
    , m_rolloverPattern(DefaultRollOverPattern)
    , m_lastRotate(0)
    , m_oldStreamFlags(ios::left)
    , m_oldPrecision(0)
  {
    InitMutex();

    const char * env = getenv("PWLIB_TRACE_STARTUP"); // Backward compatibility test
    if (env == NULL)
      env = getenv("PTLIB_TRACE_STARTUP"); // Backward compatibility test
    if (env != NULL)
      m_thresholdLevel = atoi(env);

    env = getenv("PWLIB_TRACE_LEVEL");
    if (env == NULL)
      env = getenv("PTLIB_TRACE_LEVEL");
    if (env != NULL)
      m_thresholdLevel = atoi(env);

    env = getenv("PWLIB_TRACE_OPTIONS");
    if (env == NULL)
      env = getenv("PTLIB_TRACE_OPTIONS");
    if (env != NULL)
      AdjustOptions(atoi(env), UINT_MAX);

    env = getenv("PWLIB_TRACE_FILE");
    if (env == NULL)
      env = getenv("PTLIB_TRACE_FILE");
    OpenTraceFile(env);
  }

  ~PTraceInfo()
  {
    if (m_stream != &cerr && m_stream != &cout)
      delete m_stream;
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

    if (m_stream != &cerr && m_stream != &cout)
      delete m_stream;
    m_stream = newStream;

    Unlock();
  }

  ostream * GetStream() const
  {
    return m_stream;
  }

  bool AdjustOptions(unsigned addedOptions, unsigned removedOptions)
  {
    unsigned oldOptions = m_options;
    m_options &= ~removedOptions;
    m_options |= addedOptions;
    if (m_options == oldOptions)
      return false;

    if ((m_options & HasFilePermissions) == 0)
      m_options |= HasFilePermissions | (PFileInfo::DefaultPerms << FilePermissionShift);

    bool syslogBit = (m_options&SystemLogStream) != 0;
    bool syslogStrm = dynamic_cast<PSystemLog *>(m_stream) != NULL;
    if (syslogBit != syslogStrm) {
      SetStream(syslogBit ? new PSystemLog : &cerr);
      PSystemLog::GetTarget().SetThresholdLevel(PSystemLog::LevelFromInt(m_thresholdLevel));
    }

    return true;
  }


  bool HasOption(unsigned options) const { return (m_options & options) != 0; }

  void OpenTraceFile(const char * newFilename)
  {
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

    m_filename = newFilename == NULL || *newFilename == '\0' ? "stderr" : newFilename;
    PStringArray tokens = m_filename.Tokenise(',');

    AdjustOptions(0, SystemLogStream);

    if (m_filename == "stderr")
      SetStream(&cerr);
    else if (m_filename == "stdout")
      SetStream(&cout);
#ifdef _WIN32
    else if (m_filename == "DEBUGSTREAM")
      SetStream(new PDebugStream);
#elif !defined(P_VXWORKS)
    else if (tokens[0] *= "syslog") {
      PSystemLog::SetTarget(new PSystemLogToSyslog(tokens[1],
                                                   tokens.GetSize() > 2 ? tokens[2].AsInteger() : -1,
                                                   tokens.GetSize() > 3 ? tokens[3].AsInteger() : -1,
                                                   tokens.GetSize() > 4 ? tokens[4].AsInteger() : -1));
      AdjustOptions(SystemLogStream, 0);
    }
#endif
    else if (tokens[0] *= "network") {
      switch (tokens.GetSize()) {
        case 1 :
          PSystemLog::SetTarget(new PSystemLogToNetwork("localhost"));
          break;

        case 2 :
          PSystemLog::SetTarget(new PSystemLogToNetwork(tokens[1]));
          break;

        default :
          PSystemLog::SetTarget(new PSystemLogToNetwork(tokens[1], PSystemLogToNetwork::RFC3164_Port, tokens[2].AsInteger()));
      }
      AdjustOptions(SystemLogStream, 0);
    }
    else {
      PFilePath fn(m_filename);
      fn.Replace("%P", PString(PProcess::GetCurrentProcessID()));
     
      if ((m_options & RotateLogMask) != 0)
        fn = fn.GetDirectory() +
             fn.GetTitle() +
             PTime().AsString(m_rolloverPattern, ((m_options&GMTTime) ? PTime::GMT : PTime::Local)) +
             fn.GetType();

      PFile::OpenOptions options = PFile::Create;
      if ((m_options & AppendToFile) == 0)
        options |= PFile::Truncate;
      PFileInfo::Permissions permissions = PFileInfo::DefaultPerms;
      if ((m_options & HasFilePermissions) != 0)
        permissions.FromBits((m_options&FilePermissionMask)>>FilePermissionShift);

      PFile * traceOutput = new PTextFile();
      if (traceOutput->Open(fn, PFile::WriteOnly, options, permissions))
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

    if (m_thresholdLevel > 0) {
      ostream & log = InternalBegin(false, 0, NULL, 0, NULL, NULL) << '\t';

      if (PProcess::IsInitialised()) {
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
             " level=" << m_thresholdLevel << ", to \"" << m_filename << '"'
          << endl;
    }
  }

  void InternalInitialise(unsigned level, const char * filename, const char * rolloverPattern, unsigned options);
  std::ostream & InternalBegin(bool topLevel, unsigned level, const char * fileName, int lineNum, const PObject * instance, const char * module);
  std::ostream & InternalEnd(std::ostream & stream);
};


void PTrace::SetStream(ostream * s)
{
  PTraceInfo & info = PTraceInfo::Instance();
  ostream * before = info.GetStream();
  info.SetStream(s);
  ostream * after = info.GetStream();
  PTRACE_IF(2, before != after, "PTLib\tTrace stream set to " << after << " (" << s << ')');
}


ostream * PTrace::GetStream()
{
  return PTraceInfo::Instance().GetStream();
}


static void SetOptionBit(unsigned & options, unsigned option)
{
  options |= option;
}


static void ClearOptionBit(unsigned & options, unsigned option)
{
  options &= ~option;
}


void PTrace::Initialise(const PArgList & args,
                        unsigned options,
                        const char * traceCount,
                        const char * outputFile,
                        const char * traceOpts,
                        const char * traceRollover,
                        const char * traceLevel)
{
  if ((options & HasFilePermissions) == 0)
    options = HasFilePermissions | (PFileInfo::DefaultPerms << FilePermissionShift);

  PCaselessString optStr = args.GetOptionString(traceOpts);
  PINDEX pos = 0;
  while ((pos = optStr.FindOneOf("+-", pos)) != P_MAX_INDEX) {
    void (*operation)(unsigned & options, unsigned option) = optStr[pos++] == '+' ? SetOptionBit : ClearOptionBit;
    if (optStr.NumCompare("block", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, Blocks);
    else if (optStr.NumCompare("date", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, DateAndTime);
    else if (optStr.NumCompare("time", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, Timestamp);
    else if (optStr.NumCompare("thread", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, Thread);
    else if (optStr.NumCompare("level", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, TraceLevel);
    else if (optStr.NumCompare("file", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, FileAndLine);
    else if (optStr.NumCompare("object", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, ObjectInstance);
    else if (optStr.NumCompare("context", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, ContextIdentifier);
    else if (optStr.NumCompare("gmt", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, GMTTime);
    else if (optStr.NumCompare("daily", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, RotateDaily);
    else if (optStr.NumCompare("hour", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, RotateHourly);
    else if (optStr.NumCompare("minute", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, RotateMinutely);
    else if (optStr.NumCompare("append", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, AppendToFile);
    else if (optStr.NumCompare("ax", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, (PFileInfo::WorldExecute|PFileInfo::GroupExecute|PFileInfo::UserExecute) << FilePermissionShift);
    else if (optStr.NumCompare("aw", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, (PFileInfo::WorldWrite|PFileInfo::GroupWrite|PFileInfo::UserWrite) << FilePermissionShift);
    else if (optStr.NumCompare("ar", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, (PFileInfo::WorldRead|PFileInfo::GroupRead|PFileInfo::UserRead) << FilePermissionShift);
    else if (optStr.NumCompare("ox", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::WorldExecute << FilePermissionShift);
    else if (optStr.NumCompare("ow", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::WorldWrite << FilePermissionShift);
    else if (optStr.NumCompare("or", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::WorldRead << FilePermissionShift);
    else if (optStr.NumCompare("gx", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::GroupExecute << FilePermissionShift);
    else if (optStr.NumCompare("gw", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::GroupWrite << FilePermissionShift);
    else if (optStr.NumCompare("gr", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::GroupRead << FilePermissionShift);
    else if (optStr.NumCompare("ux", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::UserExecute << FilePermissionShift);
    else if (optStr.NumCompare("uw", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::UserWrite << FilePermissionShift);
    else if (optStr.NumCompare("ur", P_MAX_INDEX, pos) == PObject::EqualTo)
      operation(options, PFileInfo::UserRead << FilePermissionShift);
  }

  PTraceInfo::Instance().InternalInitialise(std::max((unsigned)args.GetOptionCount(traceCount),
                                                     (unsigned)args.GetOptionString(traceLevel).AsUnsigned()),
                                            args.GetOptionString(outputFile),
                                            args.GetOptionString(traceRollover),
                                            options);
}


void PTrace::Initialise(unsigned level, const char * filename, unsigned options, const char * rolloverPattern)
{
  PTraceInfo::Instance().InternalInitialise(level, filename, rolloverPattern, options);
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


void PTraceInfo::InternalInitialise(unsigned level, const char * filename, const char * rolloverPattern, unsigned options)
{
  AdjustOptions(options, UINT_MAX);
  m_thresholdLevel = level;
  m_rolloverPattern = rolloverPattern;
  if (m_rolloverPattern.IsEmpty())
    m_rolloverPattern = DefaultRollOverPattern;
  // Does PTime::GetDayOfYear() etc. want to take zone param like PTime::AsString() to switch 
  // between os_gmtime and os_localtime?
  m_lastRotate = GetRotateVal(options);
  OpenTraceFile(filename);
}


void PTrace::SetOptions(unsigned options)
{
  PTraceInfo & info = PTraceInfo::Instance();
  if (info.AdjustOptions(options, 0)) {
    PTRACE(2, NULL, "PTLib", "Trace options set to " << info.m_options);
  }
}


void PTrace::ClearOptions(unsigned options)
{
  PTraceInfo & info = PTraceInfo::Instance();
  if (info.AdjustOptions(0, options)) {
    PTRACE(2, NULL, "PTLib", "Trace options set to " << info.m_options);
  }
}


unsigned PTrace::GetOptions()
{
  return PTraceInfo::Instance().m_options;
}


void PTrace::SetLevel(unsigned level)
{
  PTraceInfo & info = PTraceInfo::Instance();
  if (info.m_thresholdLevel != level) {
    info.m_thresholdLevel = level;
    PTRACE(2, "PTLib\tTrace threshold set to " << level);
  }
}


unsigned PTrace::GetLevel()
{
  return PTraceInfo::Instance().m_thresholdLevel;
}


PBoolean PTrace::CanTrace(unsigned level)
{
  return PProcess::IsInitialised() && level <= GetLevel();
}


ostream & PTrace::Begin(unsigned level, const char * fileName, int lineNum, const PObject * instance, const char * module)
{
  return PTraceInfo::Instance().InternalBegin(true, level, fileName, lineNum, instance, module);
}


ostream & PTraceInfo::InternalBegin(bool topLevel, unsigned level, const char * fileName, int lineNum, const PObject * instance, const char * module)
{
  PThread * thread = NULL;
  PTraceInfo::ThreadLocalInfo * threadInfo = NULL;
  ostream * streamPtr = m_stream;

  if (topLevel) {
    if (PProcess::IsInitialised()) {
      thread = PThread::Current();

      threadInfo = m_threadStorage.Get();
      if (threadInfo != NULL) {
        PStringStream * stringStreamPtr = new PStringStream;
        threadInfo->m_traceStreams.Push(stringStreamPtr);
        streamPtr = stringStreamPtr;
      }
    }

    Lock();

    if (!m_filename.IsEmpty() && HasOption(RotateLogMask)) {
      unsigned rotateVal = GetRotateVal(m_options);
      if (rotateVal != m_lastRotate) {
        m_lastRotate = rotateVal;
        OpenTraceFile(m_filename);
        if (m_stream == NULL)
          SetStream(&cerr);
        streamPtr = m_stream;
      }
    }
  }

  ostream & stream = *streamPtr;

  // Before we do new trace, make sure we clear any errors on the stream
  stream.clear();

  m_oldStreamFlags = stream.flags();
  m_oldPrecision   = stream.precision();

  if (!HasOption(SystemLogStream)) {
    if (HasOption(DateAndTime)) {
      PTime now;
      stream << now.AsString("yyyy/MM/dd hh:mm:ss.uuu\t", HasOption(GMTTime) ? PTime::GMT : PTime::Local);
    }

    if (HasOption(Timestamp))
      stream << setprecision(3) << setw(10) << (PTimer::Tick()-m_startTick) << '\t';
  }

  if (HasOption(TraceLevel))
    stream << level << '\t';

  if (HasOption(Thread)) {
    PString name;
    if (thread == NULL)
      name.sprintf("Thread:" PTHREAD_ID_FMT, PThread::GetCurrentThreadId());
    else
      name = thread->GetThreadName();
    if (name.GetLength() <= 23)
      stream << setw(23) << name;
    else
      stream << name.Left(10) << "..." << name.Right(10);
    stream << '\t';
  }

  if (HasOption(ThreadAddress))
    stream << hex << setfill('0') << setw(7) << (void *)thread << dec << setfill(' ') << '\t';

  if (HasOption(FileAndLine)) {
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

  if (HasOption(ObjectInstance)) {
    if (instance != NULL)
      stream << instance->GetClass() << ':' << instance;
    stream << '\t';
  }

  if (HasOption(ContextIdentifier)) {
    unsigned id = instance != NULL ? instance->GetTraceContextIdentifier() : 0;
    if (id == 0 && thread != NULL)
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
    m_currentLevel = level;
  else {
    threadInfo->m_traceLevel = level;
    threadInfo->m_prefixLength = threadInfo->m_traceStreams.Top().GetLength();
    Unlock();
  }

  return stream;
}


ostream & PTrace::End(ostream & paramStream)
{
  return PTraceInfo::Instance().InternalEnd(paramStream);
}


ostream & PTraceInfo::InternalEnd(ostream & paramStream)
{
  PTraceInfo::ThreadLocalInfo * threadInfo = PProcess::IsInitialised() ? m_threadStorage.Get() : NULL;

  paramStream.flags(m_oldStreamFlags);
  paramStream.precision(m_oldPrecision);

  unsigned currentLevel;

  if (threadInfo != NULL && !threadInfo->m_traceStreams.IsEmpty()) {
    PStringStream * stackStream = threadInfo->m_traceStreams.Pop();
    if (!PAssert(&paramStream == stackStream, PLogicError))
      return paramStream;
    *stackStream << ends << flush;
    PINDEX tab = stackStream->Find('\t', threadInfo->m_prefixLength);
    if (tab != P_MAX_INDEX) {
      PINDEX len = tab - threadInfo->m_prefixLength;
      if (len < 8)
        stackStream->Splice("      ", tab, 0);
    }
    Lock();
    *m_stream << *stackStream;
    delete stackStream;

    currentLevel = threadInfo->m_traceLevel;
  }
  else {
    if (!PAssert(&paramStream == m_stream, PLogicError)) {
      Unlock();
      return paramStream;
    }

    currentLevel = m_currentLevel;
    // Inherit lock from PTrace::Begin()
  }

  if (HasOption(SystemLogStream)) {
    // Get the trace level for this message and set the stream width to that
    // level so that the PSystemLog can extract the log level back out of the
    // ios structure. There could be portability issues with this though it
    // should work pretty universally.
    m_stream->width(currentLevel + 1);
  }
  else
    *m_stream << '\n';
  m_stream->flush();

  Unlock();
  return paramStream;
}


PTrace::Block::Block(const char * fileName, int lineNum, const char * traceName)
{
  file = fileName;
  line = lineNum;
  name = traceName;

  if (PTraceInfo::Instance().HasOption(Blocks)) {
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
  if (PTraceInfo::Instance().HasOption(Blocks)) {
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


bool PDirectory::Create(const PString & p, int perm, bool recurse)
{
  PAssert(!p.IsEmpty(), "attempt to create dir with empty name");

  PDirectory dir = p;
  
#if defined(WIN32)
  if (_mkdir(dir) == 0)
#elif defined(P_VXWORKS)
  if (mkdir(dir) == 0)
#else    
  if (mkdir(dir.Left(p.GetLength()-1), perm) == 0)
#endif
    return true;

  return recurse && !dir.IsRoot() && dir.GetParent().Create(perm, true) && dir.Create(perm, false);
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

PIdGenerator PTimer::Guard::Data::s_handleGenerator;

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


PTimer::List& PTimer::TimerList() const
{
  return PProcess::Current().m_timers;
}


void PTimer::Construct()
{
  StartRunning(true);
}


PInt64 PTimer::GetMilliSeconds() const
{
  PInt64 diff = GetAbsoluteTime() - Tick().GetMilliSeconds();
  if (diff < 0)
    diff = 0;
  return diff;
}


PTimer::~PTimer()
{
  Stop();
  PAssert(!m_guard.InTimeout(), "Timer is destroying from OnTimeout!");
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
  Stop();

  PTimeInterval::operator=(m_resetTime);
  m_oneshot = once;

  if (m_resetTime > 0) {
    m_absoluteTime = Tick().GetMilliSeconds() + m_resetTime.GetMilliSeconds();
    TimerList().RegisterTimer(this);
  }
}


void PTimer::Stop(bool /* wait */)
{
  // New implementation doesn't need sync mode any more, because implementation guarantees that
  // OnTimeout will never be called after Stop() is finished.
  m_guard.Stop();
  if (!TimerList().IsTimerThread())
  {
    Guard tempGuard = m_guard; // Prevent guard destruction
    m_guard.Reset(); // Free guard for current PTimer instance
    if (tempGuard)
      tempGuard.WaitAndReset();
  }
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


///////////////////////////////////////////////////////////////////////////////
// PTimer::List

PTimer::List::List()
  : m_ticks(0)
  , m_mininalInterval(25)
  , m_timerThread(NULL)
{
}


void PTimer::List::RegisterTimer(PTimer * aTimer)
{
  PAssert(aTimer != NULL, "Timer is NULL!");

  m_insertMutex.Wait();
  m_eventsForInsertion.push_back(PreparedEventInfo(aTimer));
  m_insertMutex.Signal();

  if (!IsTimerThread())
    PProcess::Current().SignalTimerChange();
}


void PTimer::List::ProcessInsertion()
{
  PWaitAndSignal locker(m_insertMutex);
  while (m_eventsForInsertion.size() > 0)
  {
    EventsForInsertion::iterator it = m_eventsForInsertion.begin();
    PIdGenerator::Handle handle = it->emitter->GetHandle();
    m_events[handle] = it->emitter;
    m_timeToEventRelations.insert(TimerEventRelations::value_type(m_ticks + it->msecs, handle));
    m_eventsForInsertion.erase(it);
  }
}


bool PTimer::List::IsTimerThread() const
{
  return (m_timerThread == PThread::Current());
}


PTimeInterval PTimer::List::Process()
{
  if (!m_timerThread)
    m_timerThread = PThread::Current();

  m_ticks = PTimer::Tick().GetMilliSeconds();

  ProcessInsertion();

  TimerEventRelations::iterator it = m_timeToEventRelations.begin();
  while ((it != m_timeToEventRelations.end()) && (it->first <= m_ticks)) {
    Events::iterator eventIt = m_events.find(it->second);
    if (eventIt != m_events.end()) {
      PIdGenerator::Handle currentHandle = eventIt->second->Timeout();
      PInt64 interval = eventIt->second->GetRepeat();
      // If timer is repeatable and valid add it again
      if (interval > 0 && PIdGenerator::Invalid != currentHandle)
        m_timeToEventRelations.insert(TimerEventRelations::value_type(m_ticks + interval, it->second));
      else
        m_events.erase(eventIt); // One shot or stopped timer - remove it
    }
    m_timeToEventRelations.erase(it++);
  }

  // Calculate interval before next Process() call
  PTimeInterval nextInterval = 1000;
  it = m_timeToEventRelations.begin();
  if (it != m_timeToEventRelations.end())
  {
    nextInterval = it->first - m_ticks;
    if (nextInterval.GetMilliSeconds() < PTimer::Resolution())
      nextInterval = PTimer::Resolution();
    if (nextInterval.GetMilliSeconds() < m_mininalInterval)
      nextInterval = m_mininalInterval;
  }

  return nextInterval;
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


ostream & PArgList::Usage(ostream & strm, const char * usage) const
{
  if (!m_parseError.IsEmpty())
    strm << m_parseError << "\n\n";

  PStringArray usages = PString(usage).Lines();
  switch (usages.GetSize()) {
    case 0 :
      break;

    case 1 :
      strm << "usage: " << GetCommandName() << ' ' << usage << '\n';
      break;

    default :
      strm << "Usage:\n";
      PINDEX i;
      for (i = 0; i < usages.GetSize(); ++i) {
        if (usages[i].IsEmpty())
          break;
        strm << "   " << GetCommandName() << ' ' << usages[i] << '\n';
      }

      for (; i < usages.GetSize(); ++i)
        strm << usages[i] << '\n';
  }

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
    if (lines.IsEmpty())
      strm << '\n';
    else {
      strm << "  : " << lines[0] << '\n';
      for (PINDEX i = 1; i < lines.GetSize(); ++i)
        strm << setw(maxNameLength+14) << ' ' << lines[i] << '\n';
    }
  }

  return strm;
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


bool PArgList::InternalSpecificationError(bool isError, const PString & msg)
{
  if (!isError)
    return false;

  m_parseError = msg;
  PAssertAlways(msg);
  m_options.clear();
  return true;
}


bool PArgList::Parse(const char * spec, PBoolean optionsBeforeParams)
{
  m_parseError.MakeEmpty();

  // Find starting point, start at shift if first Parse() call.
  PINDEX arg = m_options.empty() ? m_shift : 0;

  // If not in parse all mode, have been parsed before, and had some parameters
  // from last time, then start argument parsing somewhere along instead of start.
  if (optionsBeforeParams && !m_options.empty() && m_argsParsed > 0)
    arg = m_argsParsed;

  if (spec == NULL) {
    if (InternalSpecificationError(m_options.empty() || !optionsBeforeParams, "Cannot reparse without any options."))
      return false;
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
        if (InternalSpecificationError(end == NULL || spec == end, "Unbalanced [] clause in specification."))
          return false;
        opts.m_section = PString(spec, end-spec);
        spec = end+1;
        if (InternalSpecificationError(spec == '\0', "Empty [] clause in specification."))
          return false;
      }

      if (*spec != '-')
        opts.m_letter = *spec++;

      if (*spec == '-') {
        size_t pos = strcspn(++spec, ".:; ");
        if (InternalSpecificationError(pos < 2, "Empty long form for option."))
          return false;
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
        if (InternalSpecificationError(opts.m_letter != '\0' && opts.m_letter == m_options[i].m_letter,
                        "Duplicate option character '" + PString(opts.m_letter) + "' in specification") ||
            InternalSpecificationError(!opts.m_name.IsEmpty() && opts.m_name == m_options[i].m_name,
                           "Duplicate option string \"" + PString(opts.m_name) + "\" in specification"))
          return false;
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
    else if (argStr == "--") {
      if (optionsBeforeParams) {
        ++arg;
        break;
      }
      hadMinusMinus = true; // ALL remaining args are parameters not options
    }
    else if (optionsBeforeParams && m_parameterIndex.GetSize() > 0)
      break;
    else if (argStr[1] == '-') {
      if (InternalParseOption(argStr.Mid(2), 0, arg) < 0)
        return false;
    }
    else {
      for (PINDEX i = 1; i < argStr.GetLength(); i++) {
        int result = InternalParseOption(argStr[i], i+1, arg);
        if (result < 0)
          return false;
        if (result > 0)
          break;
      }
    }

    arg++;
  }

  if (optionsBeforeParams)
    m_argsParsed = arg;

  return param > 0;
}


size_t PArgList::InternalFindOption(const PString & name) const
{
  size_t opt;
  for (opt = 0; opt < m_options.size(); ++opt) {
    if (name.GetLength() == 1 ? (m_options[opt].m_letter == name[0]) : (m_options[opt].m_name == name))
      break;
  }
  return opt;
}


int PArgList::InternalParseOption(const PString & optStr, PINDEX offset, PINDEX & arg)
{
  size_t idx = InternalFindOption(optStr);
  if (idx >= m_options.size()) {
    m_parseError = "Unknown option \"" + optStr + "\".";
    m_options.clear();
    return -1;
  }

  OptionSpec & opt = m_options[idx];
  ++opt.m_count;
  if (opt.m_type == NoString)
    return 0;

  if (!opt.m_string)
    opt.m_string += '\n';

  if (offset != 0 && (opt.m_type == StringWithLetter || m_argumentArray[arg][offset] != '\0')) {
    opt.m_string += m_argumentArray[arg].Mid(offset);
    return 1;
  }

  if (++arg < m_argumentArray.GetSize()) {
    opt.m_string += m_argumentArray[arg];
    return 1;
  }

  m_parseError = "Option \"" + optStr + "\" requires an argument.";
  m_options.clear();
  return -1;
}


PINDEX PArgList::GetOptionCount(char option) const
{
  return InternalGetOptionCountByIndex(InternalFindOption(option));
}


PINDEX PArgList::GetOptionCount(const char * option) const
{
  return InternalGetOptionCountByIndex(InternalFindOption(PString(option)));
}


PINDEX PArgList::GetOptionCount(const PString & option) const
{
  return InternalGetOptionCountByIndex(InternalFindOption(option));
}


PINDEX PArgList::InternalGetOptionCountByIndex(size_t idx) const
{
  return idx < m_options.size() ? m_options[idx].m_count : 0;
}


PString PArgList::GetOptionString(char option, const char * dflt) const
{
  return InternalGetOptionStringByIndex(InternalFindOption(option), dflt);
}


PString PArgList::GetOptionString(const char * option, const char * dflt) const
{
  return InternalGetOptionStringByIndex(InternalFindOption(option), dflt);
}


PString PArgList::GetOptionString(const PString & option, const char * dflt) const
{
  return InternalGetOptionStringByIndex(InternalFindOption(option), dflt);
}


PString PArgList::InternalGetOptionStringByIndex(size_t idx, const char * dflt) const
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


PString PConfigArgs::CharToString(char letter) const
{
  for (size_t opt = 0; opt < m_options.size(); ++opt) {
    if (m_options[opt].m_letter == letter)
      return m_options[opt].m_name;
  }
  return PString::Empty();
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


void PProcess::PreInitialise(int c, char ** v)
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
  arguments.SetCommandName(executableFile.GetTitle());
}


PProcess::PProcess(const char * manuf, const char * name,
                   unsigned major, unsigned minor, CodeStatus stat, unsigned build,
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
  , m_houseKeeper(NULL)
#ifndef P_VXWORKS
  , m_processID(GetCurrentProcessID())
#endif
{
  m_activeThreads[GetThreadId()] = this;
  m_autoDeleteThreads.DisallowDeleteObjects();

#if PTRACING
  // Do this before PProcessInstance is set to avoid a recursive loop with PTimedMutex
  PTraceInfo::Instance();
#endif

  PAssert(PProcessInstance == NULL, "Only one instance of PProcess allowed");
  PProcessInstance = this;

#ifdef P_RTEMS

  cout << "Enter program arguments:\n";
  arguments.ReadFrom(cin);

#endif // P_RTEMS

#ifdef _WIN32
  // Try to get the real image path for this process
  TCHAR shortName[_MAX_PATH];
  if (GetModuleFileName(GetModuleHandle(NULL), shortName, sizeof(shortName)) > 0) {
    TCHAR longName[32768]; // Space for long image path
    if (GetLongPathName(shortName, longName, sizeof(longName)) > 0)
      executableFile = longName;
    else
      executableFile = shortName;
    executableFile.Replace("\\\\?\\", ""); // Name can contain \\?\ prefix, remove it
  }

#endif // _WIN32

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

  if (!executableFile.IsEmpty())
    PPluginManager::GetPluginManager().AddDirectory(executableFile.GetDirectory());

  // create one instance of each class registered in the PProcessStartup abstract factory
  // But make sure we have plugins first, to avoid bizarre behaviour where static objects
  // are initialised multiple times when libraries are loaded in Linux.
  PProcessStartupFactory::KeyList_T list = PProcessStartupFactory::GetKeyList();
  std::swap(list.front(), *std::find(list.begin(), list.end(), PLUGIN_LOADER_STARTUP_NAME));
  list.insert(list.begin(), "SetTraceLevel");
  for (PProcessStartupFactory::KeyList_T::const_iterator it = list.begin(); it != list.end(); ++it) {
    PProcessStartup * startup = PProcessStartupFactory::CreateInstance(*it);
    if (startup != NULL) {
      PTRACE(5, "PTLib", "Startup factory " << *it);
      startup->OnStartup();
    }
    else {
      PTRACE(1, "PTLib", "Could not create startup factory " << *it);
    }
  }
}


bool PProcess::SignalTimerChange()
{
  if (!PAssert(IsInitialised(), PLogicError) || m_shuttingDown) 
    return false;

  if (m_keepingHouse.TestAndSet(true))
    m_signalHouseKeeper.Signal();
  else
    m_houseKeeper = new PThreadObj<PProcess>(*this, &PProcess::HouseKeeping, false, "PTLib Housekeeper");

  return true;
}


void PProcess::PreShutdown()
{
  PTRACE(4, "PTLib\tStarting process destruction.");

  m_shuttingDown = true;

  // Get rid of the house keeper (majordomocide)
  if (m_houseKeeper != NULL && m_houseKeeper->GetThreadId() != PThread::GetCurrentThreadId()) {
    PTRACE(4, "PTLib\tTerminating housekeeper thread.");
    m_keepingHouse = false;
    m_signalHouseKeeper.Signal();
    m_houseKeeper->WaitForTermination();
    delete m_houseKeeper;
    m_houseKeeper = NULL;
  }

  // Clean up factories
  PProcessStartupFactory::KeyList_T list = PProcessStartupFactory::GetKeyList();
  for (PProcessStartupFactory::KeyList_T::const_iterator it = list.begin(); it != list.end(); ++it)
    PProcessStartupFactory::CreateInstance(*it)->OnShutdown();

  Sleep(100);  // Give threads time to die a natural death

  m_threadMutex.Wait();

  // OK, if there are any other threads left, we get really insistent...
  PTRACE(4, "PTLib\tTerminating " << m_activeThreads.size()-1 << " remaining threads.");
  for (ThreadMap::iterator it = m_activeThreads.begin(); it != m_activeThreads.end(); ++it) {
    PThread & thread = *it->second;
    if (this != &thread && !thread.IsTerminated()) {
      PTRACE(3, "PTLib\tTerminating thread " << thread);
      thread.Terminate();  // With extreme prejudice
    }
  }
  PTRACE(4, "PTLib\tTerminated all threads, destroying " << m_autoDeleteThreads.GetSize() << " remaining auto-delete threads.");
  m_autoDeleteThreads.AllowDeleteObjects();
  m_autoDeleteThreads.RemoveAll();
  m_activeThreads.clear();

  m_threadMutex.Signal();
}


void PProcess::PostShutdown()
{
  PTRACE(4, PProcessInstance, "PTLib", "Completed process destruction.");

  PFactoryBase::GetFactories().DestroySingletons();
  PProcessInstance = NULL;

  // Can't do any more tracing after this ...
#if PTRACING
  PTrace::SetStream(NULL);
#endif
}


PProcess & PProcess::Current()
{
  if (PProcessInstance == NULL) {
    PAssertFunc("Catastrophic failure, PProcess::Current() = NULL!!");
    PBreakToDebugger();
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


#if P_CONFIG_FILE
void PProcess::SetConfigurationPath(const PString & path)
{
  configurationPaths = path.Tokenise(";:", false);
  PTRACE(3, "PTlib", "Configuration path set to " << setfill(';') << configurationPaths);
}
#endif


PThread * PProcess::GetThread(PThreadIdentifier threadId) const
{
  PWaitAndSignal mutex(m_threadMutex);
  ThreadMap::const_iterator it = m_activeThreads.find(threadId);
  return it != m_activeThreads.end() ? it->second : NULL;

}


void PProcess::InternalThreadStarted(PThread * thread)
{
  if (PAssertNULL(thread) == NULL)
    return;

  m_threadMutex.Wait();

  m_activeThreads[thread->GetThreadId()] = thread;

  if (thread->IsAutoDelete())
    InternalSetAutoDeleteThread(thread);

#if PTRACING
  size_t newHighWaterMark = 0;
  static size_t highWaterMark = 1; // Inside m_threadMutex so simple static is OK
  if (m_activeThreads.size() > highWaterMark+20)
    newHighWaterMark = highWaterMark = m_activeThreads.size();
#endif

  m_threadMutex.Signal();

  PTRACE_IF(3, newHighWaterMark  > 0, "PTLib\tThread high water mark set: " << newHighWaterMark);

  SignalTimerChange();
}


void PProcess::InternalThreadEnded(PThread * thread)
{
  if (PAssertNULL(thread) == NULL)
    return;

  PWaitAndSignal mutex(m_threadMutex);

  ThreadMap::iterator it = m_activeThreads.find(thread->GetThreadId());
  if (it == m_activeThreads.end())
    return; // Already gone

  if (it->second != thread)
    return; // Already re-used the thread ID for new thread.

  m_activeThreads.erase(it);
}


void PProcess::InternalSetAutoDeleteThread(PThread * thread)
{
  m_threadMutex.Wait();

  if (thread->IsAutoDelete()) {
    if (m_autoDeleteThreads.GetObjectsIndex(thread) == P_MAX_INDEX)
      m_autoDeleteThreads.Append(thread);
  }
  else
    m_autoDeleteThreads.Remove(thread);

  m_threadMutex.Signal();
}


void PProcess::InternalCleanAutoDeleteThreads()
{
  ThreadList threadsToDelete;

  m_threadMutex.Wait();

  ThreadList::iterator thread = m_autoDeleteThreads.begin();
  while (thread != m_autoDeleteThreads.end()) {
    if (thread->IsAutoDelete() && thread->IsTerminated()) {
      PThread * threadPtr = &*thread;
      PTRACE(5, "PTLib\tAuto-deleting terminated thread: " << threadPtr << ", id=" << threadPtr->GetThreadId());
      InternalThreadEnded(threadPtr);
      threadsToDelete.Append(threadPtr);
      m_autoDeleteThreads.erase(thread++);
    }
    else
      ++thread;
  }

  m_threadMutex.Signal();

  /* Do actual deletions outside of mutex as who knows what in users derived
     class destructor and this is a critical mutex, used by all sorts of stuff. */
  threadsToDelete.RemoveAll();
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


///////////////////////////////////////////////////////////////////////////////
// PThread

PThread * PThread::Current()
{
  if (!PProcess::IsInitialised())
    return NULL;

  PProcess & process = PProcess::Current();

  PWaitAndSignal mutex(process.m_threadMutex);
  PProcess::ThreadMap::iterator it = process.m_activeThreads.find(GetCurrentThreadId());
  if (it != process.m_activeThreads.end() && !it->second->IsTerminated())
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
    m_threadName.sprintf(":" PTHREAD_ID_FMT, threadId);
  }
  else {
    PString idStr;
    idStr.sprintf(":" PTHREAD_ID_FMT, threadId);

    m_threadName = name;
    if (m_threadName.Find(idStr) == P_MAX_INDEX)
      m_threadName += idStr;
  }

  SetWinDebugThreadName(m_threadName, threadId);
}
 

void PThread::SetAutoDelete(AutoDeleteFlag deletion)
{
  PAssert(!m_isProcess, PLogicError);

  bool newAutoDelete = (deletion == AutoDeleteThread);
  if (m_autoDelete == newAutoDelete)
    return;

  m_autoDelete = newAutoDelete;

  PProcess::Current().InternalSetAutoDeleteThread(this);
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


PThread::~PThread()
{
  if (!m_isProcess && !IsTerminated())
    Terminate();

  PTRACE(5, "PTLib\tDestroying thread " << this << ' ' << m_threadName << ", id=" << m_threadId);

  InternalDestroy();

  // Clean up any thread local storage
  for (LocalStorageList::iterator it = m_localStorage.begin(); it != m_localStorage.end(); ++it)
    (*it)->ThreadDestroyed(this);

  if (!m_isProcess && !m_autoDelete)
    PProcess::Current().InternalThreadEnded(this);
}


void PThread::LocalStorageBase::StorageDestroyed()
{
  m_mutex.Wait();
  for (StorageMap::iterator it = m_storage.begin(); it != m_storage.end(); ++it) {
    Deallocate(it->second);
    it->first->m_localStorage.remove(this);
  }
  m_storage.clear();
  m_mutex.Signal();
}


void PThread::LocalStorageBase::ThreadDestroyed(PThread * thread) const
{
  PWaitAndSignal mutex(m_mutex);

  StorageMap::iterator it = m_storage.find(thread);
  if (!PAssert(it != m_storage.end(), PLogicError))
    return;

  Deallocate(it->second);
  m_storage.erase(it);
}


void * PThread::LocalStorageBase::GetStorage() const
{
  PThread * thread = PThread::Current();
  if (thread == NULL)
    return NULL;

  PWaitAndSignal mutex(m_mutex);

  StorageMap::const_iterator it = m_storage.find(thread);
  if (it != m_storage.end())
    return it->second;

  void * threadLocal = Allocate();
  if (threadLocal == NULL)
    return NULL;

  m_storage[thread] = threadLocal;
  thread->m_localStorage.push_back(this);
  return threadLocal;
}


/////////////////////////////////////////////////////////////////////////////

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
  : m_readerSemaphore(1, 1)
  , m_readerCount(0)
  , m_writerSemaphore(1, 1)
  , m_writerCount(0)
{
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
  nest.m_readerCount++;

  // If this is the first call to StartRead() and there has not been a
  // previous call to StartWrite() then actually do the text book read only
  // lock, otherwise we leave it as just having incremented the reader count.
  if (nest.m_readerCount == 1 && nest.m_writerCount == 0)
    InternalStartRead(nest);
}


void PReadWriteMutex::InternalWait(Nest & nest, PSync & sync) const
{
  nest.m_waiting = true;

#if PTRACING
  if (sync.Wait(15000)) {
    nest.m_waiting = false;
    return;
  }

  if (PTrace::CanTrace(1)) {
    ostream & trace = PTrace::Begin(1, __FILE__, __LINE__);
    trace << "PTLib\tPossible deadlock in read/write mutex " << this << " :\n";
    for (std::map<PThreadIdentifier, Nest>::const_iterator it = m_nestedThreads.begin(); it != m_nestedThreads.end(); ++it) {
      if (it != m_nestedThreads.begin())
        trace << '\n';
      trace << "  thread-id=" << it->first << " (0x" << std::hex << it->first << std::dec << "),"
                " readers=" << it->second.m_readerCount << ","
                " writers=" << it->second.m_writerCount;
      if (!it->second.m_waiting)
        trace << ", LOCKED";
    }
    trace << PTrace::End;
  }

  sync.Wait();

  PTRACE(1, "PTLib\tPhantom deadlock in read/write mutex " << this);
#else
  sync.Wait();
#endif


  nest.m_waiting = false;
}


void PReadWriteMutex::InternalStartRead(Nest & nest)
{
  // Text book read only lock

  InternalWait(nest, m_starvationPreventer);
   InternalWait(nest, m_readerSemaphore);
    InternalWait(nest, m_readerMutex);

     m_readerCount++;
     if (m_readerCount == 1)
       InternalWait(nest, m_writerSemaphore);

    m_readerMutex.Signal();
   m_readerSemaphore.Signal();
  m_starvationPreventer.Signal();
}


void PReadWriteMutex::EndRead()
{
  // Get the nested thread info structure for the curent thread
  Nest * nest = GetNest();

  // If don't have an active read or write lock or there is a write lock but
  // the StartRead() was never called, then assert and ignore call.
  if (nest == NULL || nest->m_readerCount == 0) {
    PAssertAlways("Unbalanced PReadWriteMutex::EndRead()");
    return;
  }

  // One less nested lock by this thread, note this does not
  // need to be mutexed as it is always in the context of a single thread.
  nest->m_readerCount--;

  // If this is a nested read or a write lock is present then we don't do the
  // real unlock, the decrement is enough.
  if (nest->m_readerCount > 0 || nest->m_writerCount > 0)
    return;

  // Do text book read lock
  InternalEndRead(*nest);

  // At this point all read and write locks are gone for this thread so we can
  // reclaim the memory.
  EndNest();
}


void PReadWriteMutex::InternalEndRead(Nest & nest)
{
  // Text book read only unlock

  InternalWait(nest, m_readerMutex);

  m_readerCount--;
  if (m_readerCount == 0)
    m_writerSemaphore.Signal();

  m_readerMutex.Signal();
}


void PReadWriteMutex::StartWrite()
{
  // Get the nested thread info structure, create one it it doesn't exist
  Nest & nest = StartNest();

  // One more nested call to StartWrite() by this thread, note this does not
  // need to be mutexed as it is always in the context of a single thread.
  nest.m_writerCount++;

  // If is a nested call to StartWrite() then simply return, the writer count
  // increment is all we haev to do.
  if (nest.m_writerCount > 1)
    return;

  // If have a read lock already in this thread then do the "real" unlock code
  // but do not change the lock count, calls to EndRead() will now just
  // decrement the count instead of doing the unlock (its already done!)
  if (nest.m_readerCount > 0)
    InternalEndRead(nest);

  // Note in this gap another thread could grab the write lock, thus

  // Now do the text book write lock
  InternalWait(nest, m_writerMutex);

  m_writerCount++;
  if (m_writerCount == 1)
    InternalWait(nest, m_readerSemaphore);

  m_writerMutex.Signal();

  InternalWait(nest, m_writerSemaphore);
}


void PReadWriteMutex::EndWrite()
{
  // Get the nested thread info structure for the curent thread
  Nest * nest = GetNest();

  // If don't have an active read or write lock or there is a read lock but
  // the StartWrite() was never called, then assert and ignore call.
  if (nest == NULL || nest->m_writerCount == 0) {
    PAssertAlways("Unbalanced PReadWriteMutex::EndWrite()");
    return;
  }

  // One less nested lock by this thread, note this does not
  // need to be mutexed as it is always in the context of a single thread.
  nest->m_writerCount--;

  // If this is a nested write lock then the decrement is enough and we
  // don't do the actual write unlock.
  if (nest->m_writerCount > 0)
    return;

  // Begin text book write unlock
  m_writerSemaphore.Signal();

  InternalWait(*nest, m_writerMutex);

  m_writerCount--;
  if (m_writerCount == 0)
    m_readerSemaphore.Signal();

  m_writerMutex.Signal();
  // End of text book write unlock

  // Now check to see if there was a read lock present for this thread, if so
  // then reacquire the read lock (not changing the count) otherwise clean up the
  // memory for the nested thread info structure
  if (nest->m_readerCount > 0)
    InternalStartRead(*nest);
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


///////////////////////////////////////////////////////////////////////////////
// PIdGenerator

const PIdGenerator::Handle PIdGenerator::Invalid = 0;

PIdGenerator::PIdGenerator()
  : m_nextId(1)
  , m_mutex(new PCriticalSection)
{
}


PIdGenerator::~PIdGenerator()
{
  delete m_mutex;
  m_mutex = NULL;
}


PIdGenerator::Handle PIdGenerator::Create()
{
  if (m_mutex == NULL) // Before construction or after destruction
    return Invalid;

  Handle id;

  m_mutex->Wait();

  do {
    id = m_nextId++;
  } while (!m_inUse.insert(id).second);

  m_mutex->Signal();

  return id;
}


void PIdGenerator::Release(Handle id)
{
  if (m_mutex == NULL) // Before construction or after destruction
    return;

  m_mutex->Wait();

  m_inUse.erase(id);

  m_mutex->Signal();
}


bool PIdGenerator::IsValid(Handle id) const
{
  if (m_mutex == NULL) // Before construction or after destruction
    return false;

  PWaitAndSignal mutex(*m_mutex);
  return m_inUse.find(id) != m_inUse.end();
}

// End Of File ///////////////////////////////////////////////////////////////
