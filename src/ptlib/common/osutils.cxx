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
#elif defined(P_MACOSX)
  #include <mach-o/dyld.h>
#endif


#define PTraceModule() "PTLib"

static const char DefaultRollOverPattern[] = "_yyyy_MM_dd_hh_mm";

class PExternalThread : public PThread
{
  PCLASSINFO(PExternalThread, PThread);
  public:
    PExternalThread()
      : PThread(false)
    {
      SetThreadName("External thread");
      PTRACE(5, "Created external thread " << this << ", id=" << GetCurrentThreadId());
    }

    ~PExternalThread()
    {
      PTRACE(5, "Destroyed external thread " << this << ", id " << GetThreadId());
    }

    virtual void Main()
    {
    }

    virtual void Terminate()
    {
      PTRACE(2, "Cannot terminate external thread " << this << ", id " << GetThreadId());
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

unsigned PTrace::MaxStackWalk = 32;

class PTraceInfo : public PTrace
{
  /* NOTE you cannot have any complex types in this structure. Anything
     that might do an asert or PTRACE will crash due to recursion.
   */

public:
  unsigned        m_currentLevel;
  atomic<unsigned> m_thresholdLevel;
  unsigned        m_options;
  PCaselessString m_filename;
  ostream       * m_stream;
  PTimeInterval   m_startTick;
  PString         m_rolloverPattern;
  unsigned        m_lastRotate;


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
  {
    InitMutex();

    const char * levelEnv = getenv("PTLIB_TRACE_LEVEL");
    if (levelEnv == NULL) {
      levelEnv = getenv("PWLIB_TRACE_LEVEL");
      if (levelEnv == NULL) {
        levelEnv = getenv("PTLIB_TRACE_STARTUP"); // Backward compatibility test
        if (levelEnv == NULL)
          levelEnv = getenv("PWLIB_TRACE_STARTUP"); // Backward compatibility test
      }
    }

    const char * fileEnv = getenv("PTLIB_TRACE_FILE");
    if (fileEnv == NULL)
      fileEnv = getenv("PWLIB_TRACE_FILE");

    const char * optEnv = getenv("PWLIB_TRACE_OPTIONS");
    if (optEnv == NULL)
      optEnv = getenv("PTLIB_TRACE_OPTIONS");

    if (levelEnv != NULL || fileEnv != NULL || optEnv != NULL)
      InternalInitialise(levelEnv != NULL ? atoi(levelEnv) : m_thresholdLevel.load(),
                         fileEnv,
                         NULL,
                         optEnv != NULL ? atoi(optEnv) : m_options);
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

  bool AdjustOptions(unsigned addedOptions, unsigned removedOptions)
  {
    unsigned oldOptions = m_options;
    m_options &= ~removedOptions;
    m_options |= addedOptions;
    if (m_options == oldOptions)
      return false;

    if ((m_options & HasFilePermissions) == 0)
      m_options |= HasFilePermissions | (PFileInfo::DefaultPerms << FilePermissionShift);

#if P_SYSTEMLOG
    bool syslogBit = (m_options&SystemLogStream) != 0;
    bool syslogStrm = dynamic_cast<PSystemLog *>(m_stream) != NULL;
    if (syslogBit != syslogStrm) {
      SetStream(syslogBit ? new PSystemLog : &cerr);
      PSystemLog::GetTarget().SetThresholdLevel(PSystemLog::LevelFromInt(m_thresholdLevel));
    }
#endif

    return true;
  }


  bool HasOption(unsigned options) const { return (m_options & options) != 0; }

  void OpenTraceFile(const char * newFilename, bool outputFirstLog)
  {
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

    if ((m_options & RotateLogMask) == 0 && m_filename == newFilename)
      return;

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
#endif
#if P_SYSTEMLOG
#ifdef P_SYSTEMLOG_TO_SYSLOG
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
#endif
    else {
      PDirectory dir(m_filename);
      if (dir.Exists())
        m_filename = dir + "opal_%P.log";

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
      if (traceOutput->Open(fn, PFile::WriteOnly, options, permissions)) {
        traceOutput->SetPosition(0, PFile::End);
        SetStream(traceOutput);
        outputFirstLog = true;
      }
      else {
        ostringstream msgstrm;
        if (PProcess::IsInitialised())
          msgstrm << PProcess::Current().GetName() << ": ";
        msgstrm << "Could not open trace output file  \"" << fn << "\"\n"
                << traceOutput->GetErrorText();
#ifdef WIN32
        PVarString msg(msgstrm.str().c_str());
        MessageBox(NULL, msg, NULL, MB_OK|MB_ICONERROR);
#else
        fputs(msgstrm.str().c_str(), stderr);
#endif
        delete traceOutput;
      }
    }

    if (outputFirstLog) {
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
             " level=" << m_thresholdLevel << ", to ";
      if ((m_options & RotateLogMask) == 0)
        log << '"' << m_filename;
      else {
        log << " rollover every ";
        switch (m_options & RotateLogMask) {
          case RotateDaily :
            log << "day";
            break;
          case RotateHourly :
            log << "hour";
            break;
          case RotateMinutely :
            log << "minute";
            break;
        }
        PFilePath fn(m_filename);
        log << " to \"" << fn.GetDirectory() << fn.GetTitle() << m_rolloverPattern << fn.GetType();
      }
      log << '"' << endl;
    }
  }

  void InternalInitialise(unsigned level, const char * filename, const char * rolloverPattern, unsigned options);
  std::ostream & InternalBegin(bool topLevel, unsigned level, const char * fileName, int lineNum, const PObject * instance, const char * module);
  std::ostream & InternalEnd(std::ostream & stream);
};


void PTrace::SetStream(ostream * s)
{
  PTraceInfo & info = PTraceInfo::Instance();
  ostream * before = info.m_stream;
  info.SetStream(s);
  ostream * after = info.m_stream;
  PTRACE_IF(2, before != after, "Trace stream set to " << after << " (" << s << ')');
}


ostream * PTrace::GetStream()
{
  return PTraceInfo::Instance().m_stream;
}


ostream & PTrace::PrintInfo(ostream & strm, bool crlf)
{
  PTraceInfo & info = PTraceInfo::Instance();

  strm << "Level: " << info.m_thresholdLevel << ", Output: ";

  if (info.m_stream == NULL)
    strm << "null";
  else if (info.m_stream == &cout)
    strm << "stdout";
  else if (info.m_stream == &cerr)
    strm << "stderr";
  else if (dynamic_cast<PFile *>(info.m_stream) != NULL)
    strm << dynamic_cast<PFile *>(info.m_stream)->GetFilePath();
#ifdef _WIN32
  else if (dynamic_cast<PDebugStream *>(info.m_stream) != NULL)
    strm << "debugstream";
#endif
#if P_SYSTEMLOG
#ifdef P_SYSTEMLOG_TO_SYSLOG
  else if (dynamic_cast<PSystemLogToSyslog *>(info.m_stream) != NULL)
    strm << "syslog";
#endif
  else if (dynamic_cast<PSystemLogToNetwork *>(info.m_stream) != NULL)
    strm << "network: " << dynamic_cast<PSystemLogToNetwork *>(info.m_stream)->GetServer();
#endif
  else
    strm << typeid(*info.m_stream).name();

  strm << ", Options:";
  if (info.m_options&Blocks)
    strm << " blocks";
  if (info.m_options&TraceLevel)
    strm << " level";
  if (info.m_options&DateAndTime)
    strm << " date";
  if (info.m_options&GMTTime)
    strm << " GMT";
  if (info.m_options&Timestamp)
    strm << " timestamp";
  if (info.m_options&Thread)
    strm << " thread";
  if (info.m_options&ThreadAddress)
    strm << " thread-addr";
  if (info.m_options&FileAndLine)
    strm << " file/line";
  if (info.m_options&AppendToFile)
    strm << " append";
  if (info.m_options&ObjectInstance)
    strm << " object";
  if (info.m_options&ContextIdentifier)
    strm << " context";

  switch (info.m_options&RotateLogMask) {
    case RotateDaily :
      strm << " daily " << info.m_rolloverPattern;
      break;
    case RotateHourly :
      strm << " hourly " << info.m_rolloverPattern;
      break;
    case RotateMinutely :
      strm << " minute " << info.m_rolloverPattern;
      break;
  }

  if (crlf)
    strm << endl;

  return strm;
}


static void SetOptionBit(unsigned & options, unsigned option)
{
  options |= option;
}


static void ClearOptionBit(unsigned & options, unsigned option)
{
  options &= ~option;
}


static PString GetOptionOrParameter(const PArgList & args, const char * opt, const char * dflt = NULL)
{
  if (opt == NULL)
    return dflt;

  if (strspn(opt, "0123456789") < strlen(opt))
    return args.GetOptionString(opt, dflt);

  PINDEX optNum = atoi(opt);
  if (optNum < args.GetCount())
    return args[optNum];

  return dflt;
}


void PTrace::Initialise(const PArgList & args,
                        unsigned options,
                        const char * traceCount,
                        const char * outputFile,
                        const char * traceOpts,
                        const char * traceRollover,
                        const char * traceLevel)
{
  PTraceInfo & info = PTraceInfo::Instance();

  PCaselessString optStr = GetOptionOrParameter(args, traceOpts);
  if (optStr.IsEmpty())
    options = info.m_options;
  else {
    if ((options & HasFilePermissions) == 0)
      options |= HasFilePermissions | (PFileInfo::DefaultPerms << FilePermissionShift);

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
  }

  int level;
  if (traceCount != NULL && args.HasOption(traceCount))
    level = args.GetOptionCount(traceCount);
  else if ((level = GetOptionOrParameter(args, traceLevel, "-1").AsInteger()) < 0)
    level = info.m_thresholdLevel;

  info.InternalInitialise(level,
                          GetOptionOrParameter(args, outputFile, info.m_filename),
                          GetOptionOrParameter(args, traceRollover),
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
  m_rolloverPattern = rolloverPattern;
  if (m_rolloverPattern.IsEmpty())
    m_rolloverPattern = DefaultRollOverPattern;
  m_lastRotate = GetRotateVal(options);
  m_thresholdLevel = level;
  AdjustOptions(options, UINT_MAX);
  OpenTraceFile(filename, level > 0);
}


void PTrace::SetOptions(unsigned options)
{
  PTraceInfo & info = PTraceInfo::Instance();
  if (info.AdjustOptions(options, 0)) {
    PTRACE(2, "Trace options set to " << info.m_options);
  }
}


void PTrace::ClearOptions(unsigned options)
{
  PTraceInfo & info = PTraceInfo::Instance();
  if (info.AdjustOptions(0, options)) {
    PTRACE(2, "Trace options set to " << info.m_options);
  }
}


unsigned PTrace::GetOptions()
{
  return PTraceInfo::Instance().m_options;
}


void PTrace::SetLevel(unsigned level)
{
  if (PTraceInfo::Instance().m_thresholdLevel.exchange(level) != level) {
    PTRACE(1, "Trace threshold set to " << level);
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
        OpenTraceFile(m_filename, true);
        if (m_stream == NULL)
          SetStream(&cerr);
        if (threadInfo == NULL)
          streamPtr = m_stream;
      }
    }
  }

  ostream & stream = *streamPtr;

  // Before we do new trace, make sure we clear any errors on the stream
  stream.clear();

  if (!HasOption(SystemLogStream)) {
    if (HasOption(DateAndTime)) {
      PTime now;
      stream << now.AsString(PTime::LoggingFormat, HasOption(GMTTime) ? PTime::GMT : PTime::Local) << '\t';
    }

    if (HasOption(Timestamp))
      stream << setprecision(3) << setw(10) << (PTimer::Tick()-m_startTick) << '\t';
  }

  if (HasOption(TraceLevel))
    stream << level << '\t';

  if (HasOption(Thread)) {
    PString name = thread != NULL ? thread->GetThreadName() : PThread::GetCurrentThreadName();
#if P_64BIT && !defined(WIN32) && !defined(P_UNIQUE_THREAD_ID_FMT)
    static const PINDEX ThreadNameWidth = 31;
#else
    static const PINDEX ThreadNameWidth = 23;
#endif
    if (name.GetLength() <= ThreadNameWidth)
      stream << setw(ThreadNameWidth) << name;
    else
      stream << name.Left(10) << "..." << name.Right(ThreadNameWidth-13);
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
    static unsigned const FileWidth = 16;
    char buffer[FileWidth + 1];
    buffer[FileWidth] = '\0';
    stream << setw(FileWidth) << strncpy(buffer, file, FileWidth);

    if (lineNum > 0)
      stream << '(' << lineNum << ')';

    stream << '\t';
  }

  if (HasOption(ObjectInstance)) {
    if (instance != NULL)
      stream << instance->GetClass() << ':' << instance;
    stream << '\t';
  }

#if PTRACING==2
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
#endif

  if (module != NULL)
    stream << left << setw(8) << module << right << '\t';

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
  : file(fileName)
  , line(lineNum)
  , name(traceName)
{
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


PTrace::Block::Block(const Block & obj)
  : file(obj.file)
  , line(obj.line)
  , name(obj.name)
{
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


PTrace::ThrottleBase::ThrottleBase(unsigned lowLevel, unsigned interval, unsigned highLevel)
  : m_interval(interval)
  , m_lowLevel(lowLevel)
  , m_highLevel(highLevel)
  , m_lastLog(0)
  , m_count(0)
{
}


bool PTrace::ThrottleBase::CanTrace()
{
  PTimeInterval now = PTimer::Tick();

  if (m_lastLog == 0 || (now - m_lastLog) > m_interval) {
    m_currentLevel = m_lowLevel;
    m_lastLog = now.GetMilliSeconds();
  }
  else if (m_currentLevel == m_highLevel) {
    if (!PTrace::CanTrace(m_highLevel))
      ++m_count;
  }
  else {
    m_count = 1;
    m_currentLevel = m_highLevel;
  }

  return PTrace::CanTrace(m_currentLevel);
}


std::ostream & operator<<(ostream & strm, const PTrace::ThrottleBase & throttle)
{
  if (throttle.m_count > 1)
    strm << " (repeated " << throttle.m_count << " times)";
  return strm;
}


#if PTRACING==2

static atomic<uint32_t> g_lastContextIdentifer(0);

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

#endif // PTRACING==2

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
  if (mkdir(dir.Left(dir.GetLength()-1), perm) == 0)
#endif
    return true;

  return recurse && !dir.IsRoot() && dir.GetParent().Create(perm, true) && dir.Create(perm, false);
}


#if P_TIMERS

///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

int64_t PTimeInterval::InternalGet() const
{
  return m_nanoseconds.load();
}


void PTimeInterval::InternalSet(int64_t t)
{
  m_nanoseconds.store(t);
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


void PSimpleTimer::InternalSet(int64_t t)
{
  PTimeInterval::InternalSet(t);
  m_startTick = PTimer::Tick();
}


PTimeInterval PSimpleTimer::GetRemaining() const
{
  PTimeInterval remaining = *this - GetElapsed();
  return remaining > 0 ? remaining : PTimeInterval(0);
}


///////////////////////////////////////////////////////////////////////////////
// PTimer

static PIdGenerator s_handleGenerator;

PTimer::PTimer(long millisecs, int seconds, int minutes, int hours, int days)
  : PTimeInterval(millisecs, seconds, minutes, hours, days)
  , m_handle(s_handleGenerator.Create())
  , m_running(false)
{
  InternalStart(true, PTimeInterval::InternalGet());
}


PTimer::PTimer(const PTimeInterval & time)
  : PTimeInterval(time)
  , m_handle(s_handleGenerator.Create())
  , m_running(false)
{
  InternalStart(true, PTimeInterval::InternalGet());
}


PTimer::PTimer(const PTimer & timer)
  : PTimeInterval(timer.GetResetTime())
  , m_handle(s_handleGenerator.Create())
  , m_running(false)
{
  InternalStart(true, PTimeInterval::InternalGet());
}


PTimer::List * PTimer::TimerList()
{
  return PProcess::Current().m_timerList;
}


PTimer::~PTimer()
{
  Stop(); // And wait, if necessary
  s_handleGenerator.Release(m_handle);
}


PTimer & PTimer::operator=(const PTimer & timer)
{
  SetMilliSeconds(timer.GetMilliSeconds());
  return *this;
}

void PTimer::PrintOn(ostream & strm) const
{
  PTimeInterval::PrintOn(strm);
  strm << '/' << GetResetTime() << '[' << m_handle << ']';
}


int64_t PTimer::InternalGet() const
{
  if (!m_running)
      return 0;

  PTimeInterval diff = m_absoluteTime - Tick();
  if (diff < 0)
    diff = 0;
  return diff.GetNanoSeconds();
}


PBoolean PTimer::IsRunning() const
{
  if (m_running)
      return true;

  if (!m_callbackMutex.Try())
      return true;

  m_callbackMutex.Signal();
  return false;
}


void PTimer::InternalSet(int64_t nanoseconds)
{
  InternalStart(m_oneshot, nanoseconds);
}


void PTimer::RunContinuous(const PTimeInterval & time)
{
  InternalStart(false, time.GetNanoSeconds());
}


void PTimer::InternalStart(bool once, int64_t resetTime)
{
  List * list = TimerList();
  if (PAssertNULL(list) == NULL)
    return;

  Stop();

  /* We can use non-mutexed access here because the current implementation
     guarantees that system threads can't access the timer while it's stopped. */

  m_oneshot = once;
  PTimeInterval::InternalSet(resetTime);

  if (resetTime > 0) {
    m_absoluteTime = Tick() + GetResetTime();
    list->m_timersMutex.Wait();
    list->m_timers[m_handle] = this;
    m_running = true;
    list->m_timersMutex.Signal();

    PProcess::Current().SignalTimerChange();
  }
}


void PTimer::Stop(bool wait)
{
  List * list = TimerList();
  if (list == NULL)
    return;

  unsigned retry = 0;
  do {
    /* Take out of timer list first, so when callback is waited for it's
       completion it cannot then be called again. */
    list->m_timersMutex.Wait();
    PAssert(list->m_timers.erase(m_handle) == 1 || !m_running, PLogicError);
    m_running = false;
    list->m_timersMutex.Signal();

    if (wait) {
      m_callbackMutex.Wait();
      m_callbackMutex.Signal();
    }

    // We loop in case the callback function restarted the timer.
  } while (m_running && PAssert(++retry < 5, PLogicError));
}


void PTimer::Reset()
{
  InternalStart(m_oneshot, PTimeInterval::InternalGet());
}


// called only from the timer thread
void PTimer::OnTimeout()
{
  if (m_callback.IsNULL())
    return;

  PString oldName;
  PThread * thread = PThread::Current();
  if (thread != NULL && !m_threadName.IsEmpty()) {
    oldName = thread->GetThreadName();
    thread->SetThreadName(m_threadName);
  }

  m_callback(*this, IsRunning());

  if (!oldName.IsEmpty())
    thread->SetThreadName(oldName);
}


///////////////////////////////////////////////////////////////////////////////
// PTimer::List

PTimer::List::List()
  : m_threadPool(10, 0, "OnTimeout")
{
}


void PTimer::List::Timeout::Work()
{
  PTimer::List * list;
  while ((list = PTimer::TimerList()) != NULL) {
    PTRACE(6, NULL, PTraceModule(), "Timer: [" << m_handle << "] working");
    if (list->OnTimeout(m_handle))
      return;

    PTRACE(5, NULL, PTraceModule(), "Timer: [" << m_handle << "] already in OnTimeout(), waiting.");
    PThread::Sleep(10);
  }
}

bool PTimer::List::OnTimeout(PIdGenerator::Handle handle)
{
  PTimer * timer = NULL;

  {
    PWaitAndSignal mutex1(m_timersMutex);

    TimerMap::iterator it = m_timers.find(handle);
    if (it == m_timers.end())
      return true; // Don't try again

    timer = it->second;

    if (!timer->m_oneshot && !timer->m_running)
      return true; // Was recurring timer and was stopped

    if (!timer->m_callbackMutex.Try())
      return false; // Try again

    // Remove the expired one shot timers from map
    if (timer->m_oneshot && !timer->m_running)
      m_timers.erase(it);
  }

  // Must be outside of m_timersMutex and timer->m_timerMutex mutexes
  timer->OnTimeout();
  timer->m_callbackMutex.Signal();
  return true; // Done
}


PTimeInterval PTimer::List::Process()
{
  PTimeInterval now = PTimer::Tick();

  // Calculate interval before next Process() call
  PTimeInterval nextInterval(0, 1);

  m_timersMutex.Wait();

  for (TimerMap::iterator it = m_timers.begin(); it != m_timers.end(); ++it) {
    PTimer & timer = *it->second;
    if (timer.m_running) {
      PTimeInterval delta = timer.m_absoluteTime - now;
      if (delta > 0) {
        if (nextInterval > delta)
          nextInterval = delta;
      }
      else if (timer.m_callbackMutex.Try()) {
        /* PTimer is stopped and completely removed from the list before it's
           properties are changed from the external code, making this thread
           safe without a mutex. */
        if (timer.m_oneshot)
          timer.m_running = false;
        else {
          timer.m_absoluteTime = now + timer.GetResetTime();
          if (nextInterval > timer.GetResetTime())
            nextInterval = timer.GetResetTime();
        }
        timer.m_callbackMutex.Signal();

        m_threadPool.AddWork(new Timeout(it->first));
        PTRACE(6, &timer, "Timer: " << timer << " work added, lateness=" << -delta);
      }
    }
  }

  m_timersMutex.Signal();

  if (nextInterval < 10)
    nextInterval = 10;

  PTRACE(6, NULL, PTraceModule(), m_timers.size() << " timers processed, next=" << nextInterval);
  return nextInterval;
}


#endif //P_TIMERS


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


ostream & PArgList::Usage(ostream & strm, const char * usage, const char * prefix) const
{
  if (!m_parseError.IsEmpty())
    strm << m_parseError << "\n\n";

  PStringArray usages = PString(usage).Lines();
  switch (usages.GetSize()) {
    case 0 :
      break;

    case 1 :
      strm << prefix << GetCommandName() << ' ' << usage << '\n';
      break;

    default :
      strm << prefix << '\n';
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
  if (maxNameLength > 0)
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

    strm << (maxNameLength > 0 ? opt.m_letter == '\0' || opt.m_name.IsEmpty() ? "    ":  " or " : " ")
         << left;

    if (opt.m_name.IsEmpty())
      strm << setw(maxNameLength+5) << (opt.m_type == NoString ? "     " : "<arg>");
    else {
      strm << "--";
      if (opt.m_type == NoString)
        strm << setw(maxNameLength) << opt.m_name;
      else
        strm << opt.m_name << setw(maxNameLength - opt.m_name.GetLength()) << " <arg>";
    }

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


PString PArgList::Usage(const char * usage, const char * prefix) const
{
  PStringStream str;
  Usage(str, usage, prefix);
  return str;
}


void PArgList::SetArgs(const char * str)
{
  m_argumentArray.SetSize(0);

  for (;;) {
    while (isspace(*str)) // Skip leading whitespace
      str++;
    if (*str == '\0')
      break;

    PString & arg = m_argumentArray[m_argumentArray.GetSize()];
    while (*str != '\0' && !isspace(*str)) {
      if (*str != '"')
        arg += *str++;
      else {
        ++str;
        while (*str != '\0' && *str != '"') {
          if (str[0] != '\\' || str[1] != '"')
            arg += *str++;
          else {
            arg += '"';
            str += 2;
          }
        }
        if (*str != '\0')
          str++;
      }
    }
  }

  SetArgs(m_argumentArray);
}


void PArgList::SetArgs(const PStringArray & theArgs)
{
  if (!theArgs.IsEmpty())
    m_argumentArray = theArgs;

  m_parsed = false;
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
  return true;
}


bool PArgList::Parse(const char * spec, PBoolean optionsBeforeParams)
{
  m_parsed = false;
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
        if (InternalSpecificationError(opts.m_section.IsEmpty(), "Empty [] clause in specification."))
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
          opts.m_type = OptionalString;
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
      PINDEX equals = argStr.Find('=');
      if (equals != P_MAX_INDEX)
        ++equals;
      if (InternalParseOption(argStr(2, equals-2), equals, arg) < 0)
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

  m_parsed = true;
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
  if (idx >= m_options.size())
    m_parseError = "Unknown option ";
  else {
    OptionSpec & opt = m_options[idx];
    ++opt.m_count;
    if (opt.m_type == NoString)
      return 0;

    if (!opt.m_string)
      opt.m_string += '\n';

    if (opt.m_type == OptionalString && (offset == P_MAX_INDEX || m_argumentArray[arg][offset] == '\0'))
      return 0;

    if (offset != P_MAX_INDEX && m_argumentArray[arg][offset] != '\0') {
      opt.m_string += m_argumentArray[arg].Mid(offset);
      return 1;
    }

    if (++arg < m_argumentArray.GetSize()) {
      opt.m_string += m_argumentArray[arg];
      return 1;
    }

    m_parseError = "Argument required for option ";
  }

  m_parseError += offset == 0 ? "\"--" : "\"-";
  m_parseError += optStr;
  m_parseError += '"';
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
  if (idx < m_options.size() && m_options[idx].m_count > 0)
    return m_options[idx].m_string;

  if (dflt != NULL)
    return dflt;

  return PString::Empty();
}


PStringArray PArgList::GetParameters(PINDEX first, PINDEX last) const
{
  PStringArray params;

  if (last != P_MAX_INDEX)
    last += m_shift;

  if (last >= m_parameterIndex.GetSize())
    last = m_parameterIndex.GetSize()-1;

  if (m_shift < 0 && first < (PINDEX)-m_shift)
    return params;

  first += m_shift;
  if (first > last)
    return params;

  params.SetSize(last - first + 1);

  PINDEX idx = 0;
  while (first <= last)
    params[idx++] = m_argumentArray[m_parameterIndex[first++]];

  return params;
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
  : PArgList(args)
  , m_config(new PConfig)
  , m_sectionName(m_config->GetDefaultSection())
  , m_negationPrefix("no-")
{
}


PConfigArgs::~PConfigArgs()
{
  delete m_config;
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
  if (PArgList::GetOptionCount(m_negationPrefix + option) > 0)
    return 0;

  return m_config->HasKey(m_sectionName, option) ? 1 : 0;
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
  if (PArgList::HasOption(m_negationPrefix + option)) {
    if (dflt != NULL)
      return dflt;
    return PString();
  }

  return m_config->GetString(m_sectionName, option, dflt != NULL ? dflt : "");
}


void PConfigArgs::Save(const PString & saveOptionName)
{
  if (PArgList::GetOptionCount(saveOptionName) == 0)
    return;

  m_config->DeleteSection(m_sectionName);

  for (size_t i = 0; i < m_options.size(); i++) {
    PString optionName = m_options[i].m_name;
    if (m_options[i].m_count > 0 && optionName != saveOptionName) {
      if (!m_options[i].m_string.IsEmpty())
        m_config->SetString(m_sectionName, optionName, m_options[i].m_string);
      else
        m_config->SetBoolean(m_sectionName, optionName, true);
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
  , m_keepingHouse(false)
  , m_houseKeeper(NULL)
#ifndef P_VXWORKS
  , m_processID(GetCurrentProcessID())
#endif
{
  m_activeThreads[GetThreadId()] = this;

#if PTRACING
  // Do this before PProcessInstance is set to avoid a recursive loop with PTimedMutex
  PTraceInfo::Instance();
#endif

  PAssert(PProcessInstance == NULL, "Only one instance of PProcess allowed");
  PProcessInstance = this;

  /* Try to get the real image path for this process using platform dependent
     code, if this fails, then use the value urigivally set via argv[0] */
#if defined(_WIN32)
  TCHAR shortName[_MAX_PATH];
  if (GetModuleFileName(GetModuleHandle(NULL), shortName, sizeof(shortName)) > 0) {
    TCHAR longName[32768]; // Space for long image path
    if (GetLongPathName(shortName, longName, sizeof(longName)) > 0)
      executableFile = longName;
    else
      executableFile = shortName;
    executableFile.Replace("\\\\?\\", ""); // Name can contain \\?\ prefix, remove it
  }
#elif defined(P_MACOSX)
  char path[10000];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0)
    executableFile = path;
#elif defined(P_RTEMS)
  cout << "Enter program arguments:\n";
  arguments.ReadFrom(cin);
#else
  // Hope for a /proc, certainly works for Linux
  char path[10000];
  int len = readlink("/proc/self/exe", path, sizeof(path));
  if (len >= 0) {
    path[len] = '\0';
    executableFile = path;
  }
#endif // _WIN32

  if (productName.IsEmpty())
    productName = executableFile.GetTitle().ToLower();

  SetThreadName(GetName());

  Construct();

#if P_TIMERS
  m_timerList = new PTimer::List();
#endif

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
#ifdef PLUGIN_LOADER_STARTUP_NAME
  std::swap(list.front(), *std::find(list.begin(), list.end(), PLUGIN_LOADER_STARTUP_NAME));
#endif
  list.insert(list.begin(), "SetTraceLevel");
  for (PProcessStartupFactory::KeyList_T::const_iterator it = list.begin(); it != list.end(); ++it) {
    PProcessStartup * startup = PProcessStartupFactory::CreateInstance(*it);
    if (startup != NULL) {
      PTRACE(5, "Startup factory " << *it);
      startup->OnStartup();
    }
    else {
      PTRACE(1, "Could not create startup factory " << *it);
    }
  }
}


bool PProcess::SignalTimerChange()
{
  if (!PAssert(IsInitialised(), PLogicError) || m_shuttingDown) 
    return false;

  if (m_keepingHouse.exchange(true))
    m_signalHouseKeeper.Signal();
  else
    m_houseKeeper = new PThreadObj<PProcess>(*this, &PProcess::HouseKeeping, false, "PTLib Housekeeper");

  return true;
}


void PProcess::HouseKeeping()
{
  PSimpleTimer cleanExternalThreads;
  static PTimeInterval const CleanExternalThreadsTime(0, 10);

  while (m_keepingHouse) {
#if P_TIMERS
    PTimeInterval delay = m_timerList->Process();
    if (delay > CleanExternalThreadsTime)
      delay = CleanExternalThreadsTime;

    m_signalHouseKeeper.Wait(delay);
#else
    m_signalHouseKeeper.Wait(CleanExternalThreadsTime);
#endif

    if (cleanExternalThreads.HasExpired()) {
      cleanExternalThreads = CleanExternalThreadsTime;
      if (!m_externalThreads.IsEmpty()) {
        m_threadMutex.Wait();
        for (ThreadList::iterator it = m_externalThreads.begin(); it != m_externalThreads.end();) {
          if (it->IsTerminated())
            m_externalThreads.erase(it++);
          else
            ++it;
        }
        m_threadMutex.Signal();
      }
    }

    // m_autoDeleteThreads is inherently thread safe
    PThread * thread;
    while (m_autoDeleteThreads.Dequeue(thread, 0))
      delete thread;

#ifndef _WIN32
    PXCheckSignals();
#endif
  }
}


void PProcess::PreShutdown()
{
  PTRACE(4, "Starting process destruction.");

  m_shuttingDown = true;

  // Get rid of the house keeper (majordomocide)
  if (m_houseKeeper != NULL && m_houseKeeper->GetThreadId() != PThread::GetCurrentThreadId()) {
    PTRACE(4, "Terminating housekeeper thread.");
    m_keepingHouse = false;
    m_signalHouseKeeper.Signal();
    m_houseKeeper->WaitForTermination();
    delete m_houseKeeper;
    m_houseKeeper = NULL;
#if P_TIMERS
    delete m_timerList;
    m_timerList = NULL;
#endif
  }

  // Clean up factories
  PProcessStartupFactory::KeyList_T list = PProcessStartupFactory::GetKeyList();
  for (PProcessStartupFactory::KeyList_T::const_iterator it = list.begin(); it != list.end(); ++it)
    PProcessStartupFactory::CreateInstance(*it)->OnShutdown();

  Sleep(100);  // Give threads time to die a natural death

  m_threadMutex.Wait();

  // OK, if there are any other threads left, we get really insistent...
  PTRACE(4, "Cleaning up " << m_activeThreads.size()-1 << " remaining threads.");
  for (ThreadMap::iterator it = m_activeThreads.begin(); it != m_activeThreads.end(); ++it) {
    PThread & thread = *it->second;
    switch (thread.m_type) {
      case e_IsAutoDelete:
      case e_IsManualDelete:
        if (thread.IsTerminated()) {
          PTRACE(5, "Already terminated thread " << thread);
        }
        else {
          PTRACE(3, "Terminating thread " << thread);
          thread.Terminate();  // With extreme prejudice
        }
        break;
      case e_IsExternal :
        PTRACE(5, "Remaining external thread " << thread);
        break;
      default :
        break;
    }
  }
  m_activeThreads.clear();

  PTRACE(4, "Terminated all threads, destroying "
         << m_autoDeleteThreads.size() << " remaining auto-delete threads, and "
         << m_externalThreads.size() << " remaining external threads.");
  PThread * thread;
  while (m_autoDeleteThreads.Dequeue(thread, 0))
    delete thread;

  m_externalThreads.RemoveAll();

  m_threadMutex.Signal();

  OnThreadEnded(*this);
}


void PProcess::PostShutdown()
{
  PTRACE(4, PProcessInstance, "Completed process destruction.");

  PFactoryBase::GetFactories().DestroySingletons();
  PProcessInstance = NULL;

  // Can't do any more tracing after this ...
#if PTRACING
  PTrace::SetStream(NULL);
  PTrace::SetLevel(0);
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


void PProcess::OnThreadEnded(PThread &
#if PTRACING || P_PROFILING
                             thread
#endif
                             )
{
  if (!PProcess::IsInitialised())
    return;

#if P_PROFILING
  PThread::Times times;
  if (!thread.GetTimes(times))
    return;
  PProfiling::OnThreadEnded(thread, times.m_real, times.m_kernel, times.m_user);
#endif // P_PROFILING

#if PTRACING
  const int LogLevel = 3;
  if (PTrace::CanTrace(LogLevel)) {
#if !P_PROFILING
    PThread::Times times;
    if (thread.GetTimes(times))
#endif // P_PROFILING
    {
      ostream & trace = PTRACE_BEGIN(LogLevel, "PTLib");
      trace << "Thread ended: name=\"" << thread.GetThreadName() << "\", ";
      if (thread.GetThreadId() != (PThreadIdentifier)thread.GetUniqueIdentifier())
          trace << "id=" << thread.GetUniqueIdentifier() << ", ";
      trace << times << PTrace::End;
    }
  }
#endif //PTRACING
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


PTime PProcess::GetStartTime() const
{ 
  return programStartTime; 
}


bool PProcess::IsMultipleInstance() const
{
  static PSemaphore s_multiTest(GetName()+" UniqueProcessSeampahore", 1, 1);
  static bool s_multiple = !s_multiTest.Wait(0);
  return s_multiple;
}


PString PProcess::GetVersion(PBoolean full) const
{
  VersionInfo ver = { MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER };
  return ver.AsString(full);
}


PString PProcess::GetLibVersion()
{
  VersionInfo ver = { MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER, SVN_REVISION, GIT_COMMIT };
  return ver.AsString();
}


PString PProcess::VersionInfo::AsString(bool full)
{
  PStringStream str;
  str << m_major << '.' << m_minor;

  if (full) {
    switch (m_status) {
      case PProcess::AlphaCode :
        str << "alpha";
        break;

      case PProcess::BetaCode :
        str << "beta";
        break;

      default:
        if (m_build < 0x10000)
          str << '.';
    }

    if (m_build < 0x10000)
      str << m_build;

    if (m_git != NULL && *m_git != '\0')
      str << " (git:" << m_git << ')';
    else if (m_svn > 0)
      str << " (svn:" << m_svn << ')';
  }

  return str;
}


#if P_CONFIG_FILE
void PProcess::SetConfigurationPath(const PString & path)
{
  configurationPaths = path.Tokenise(PPATH_SEPARATOR, false);
  PTRACE(3, "Configuration path set to " << setfill(PPATH_SEPARATOR) << configurationPaths);
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

#if PTRACING
  size_t newHighWaterMark = 0;
  static size_t highWaterMark = 1; // Inside m_threadMutex so simple static is OK
  if (m_activeThreads.size() > highWaterMark+20)
    newHighWaterMark = highWaterMark = m_activeThreads.size();
#endif

  m_threadMutex.Signal();

  PTRACE_IF(2, newHighWaterMark  > 0, "Thread high water mark set: " << newHighWaterMark);

  SignalTimerChange();
}


void PProcess::InternalThreadEnded(PThread * thread)
{
  if (PAssertNULL(thread) == NULL)
    return;

  // Do the log before mutex and thread being removed from m_activeThreads
  PTRACE_IF(5, thread->IsAutoDelete(), thread, "Queuing auto-delete of thread " << *thread);

  PWaitAndSignal mutex(m_threadMutex);

  ThreadMap::iterator it = m_activeThreads.find(thread->GetThreadId());
  if (it != m_activeThreads.end() && it->second == thread)
    m_activeThreads.erase(it); // Not already gone, or re-used the thread ID for new thread.

  // Must be last thing to avoid race condition
  if (thread->IsAutoDelete()) {
    thread->SetNoAutoDelete();
    m_autoDeleteThreads.Enqueue(thread);
  }
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

PThreadIdentifier PThread::GetThreadId() const
{
  return m_threadId;
}


PThread * PThread::Current()
{
  if (!PProcess::IsInitialised())
    return NULL;

  PProcess & process = PProcess::Current();

  PWaitAndSignal mutex(process.m_threadMutex);
  PProcess::ThreadMap::iterator it = process.m_activeThreads.find(GetCurrentThreadId());
  if (it != process.m_activeThreads.end() && !it->second->IsTerminated())
    return it->second;

  if (process.m_shuttingDown)
    return NULL;

  PThread * thread = new PExternalThread;
  process.m_externalThreads.Append(thread);
  return thread;
}


PString PThread::GetThreadName(PThreadIdentifier id)
{
  if (id == PNullThreadIdentifier)
    return "(null)";

  if (PProcess::IsInitialised()) {
    PProcess & process = PProcess::Current();
    PWaitAndSignal mutex(process.m_threadMutex);
    PProcess::ThreadMap::iterator it = process.m_activeThreads.find(id);
    if (it != process.m_activeThreads.end())
      return it->second->GetThreadName();
  }

  return "Unknown thread, id=" + GetIdentifiersAsString(id, 0);
}


#ifdef P_UNIQUE_THREAD_ID_FMT
PString PThread::GetIdentifiersAsString(PThreadIdentifier tid, PUniqueThreadIdentifier uid)
{
  if (tid == PNullThreadIdentifier)
    return "(null)";
  if (uid == 0)
    return psprintf(P_THREAD_ID_FMT, tid);
  return PString(PString::Printf, P_THREAD_ID_FMT " (" P_UNIQUE_THREAD_ID_FMT ")", tid, uid);
}
#else
PString PThread::GetIdentifiersAsString(PThreadIdentifier tid, PUniqueThreadIdentifier)
{
  if (tid == PNullThreadIdentifier)
    return "(null)";

  return psprintf(P_THREAD_ID_FMT, tid);
}
#endif


bool PThread::GetTimes(PThreadIdentifier id, Times & times)
{
  if (!PProcess::IsInitialised())
    return false;

  PProcess & process = PProcess::Current();

  PWaitAndSignal mutex(process.m_threadMutex);
  PProcess::ThreadMap::iterator it = process.m_activeThreads.find(id);
  return it != process.m_activeThreads.end() && it->second->GetTimes(times);
}


void PThread::GetTimes(std::list<Times> & allThreadTimes)
{
  if (PProcess::IsInitialised()) {
    Times threadTimes;
    PProcess & process = PProcess::Current();
    PWaitAndSignal mutex(process.m_threadMutex);
    for (PProcess::ThreadMap::iterator it = process.m_activeThreads.begin(); it != process.m_activeThreads.end(); ++it) {
      if (it->second->GetTimes(threadTimes))
        allThreadTimes.push_back(threadTimes);
    }
  }
}


PThread::Times::Times()
  : m_threadId(PNullThreadIdentifier)
  , m_uniqueId(0)
{
}


float PThread::Times::AsPercentage() const
{
  PTimeInterval cpu = m_kernel + m_user;
  return (float)(cpu == 0 ? 0.0 : (100.0 * cpu.GetMilliSeconds() / m_real.GetMilliSeconds()));
}


PThread::Times PThread::Times::operator-(const Times & rhs) const
{
  Times diff;
  diff.m_real = m_real - rhs.m_real;
  diff.m_kernel = m_kernel - rhs.m_kernel;
  diff.m_user = m_user - rhs.m_user;
  return diff;
}


PThread::Times & PThread::Times::operator-=(const Times & rhs)
{
  m_real -= rhs.m_real;
  m_kernel -= rhs.m_kernel;
  m_user -= rhs.m_user;
  return *this;
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

  PUniqueThreadIdentifier threadId = GetUniqueIdentifier();
  if (name.Find('%') != P_MAX_INDEX)
    m_threadName = psprintf(name, threadId);
  else {
#ifdef P_UNIQUE_THREAD_ID_FMT
    PString idStr(PString::Printf, ":" P_UNIQUE_THREAD_ID_FMT, threadId);
#else
    PString idStr(PString::Printf, ":" P_THREAD_ID_FMT, threadId);
#endif

    m_threadName = name;
    if (m_threadName.Find(idStr) == P_MAX_INDEX)
      m_threadName += idStr;
  }

  SetWinDebugThreadName(m_threadName, threadId);
}
 

void PThread::SetAutoDelete(AutoDeleteFlag deletion)
{
  switch (m_type) {
    case e_IsAutoDelete :
      if (deletion == AutoDeleteThread)
        return;
      break;

    case e_IsManualDelete :
      if (deletion != AutoDeleteThread)
        return;
      break;

    case e_IsProcess :
      PAssert(deletion != AutoDeleteThread, PInvalidParameter);
      return;

    case e_IsExternal :
      PAssert(deletion == AutoDeleteThread, PInvalidParameter);
      return;
  }

  m_type = deletion == AutoDeleteThread ? e_IsAutoDelete : e_IsManualDelete;
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


bool PThread::WaitAndDelete(PThread * & threadToDelete, const PTimeInterval & maxWait, PMutex * mutex, bool lock)
{
  if (mutex != NULL && lock)
    mutex->Wait();

  PThread * thread = threadToDelete;
  threadToDelete = NULL;

  if (mutex != NULL)
    mutex->Signal();

  if (thread == NULL)
    return false;

  if (PThread::Current() == thread) {
    // Is me!
    thread->SetAutoDelete();
    return false;
  }

  if (thread->IsSuspended()) {
    // Ended before it started
    delete thread;
    return false;
  }

  PTRACE(4, thread, "Waiting for thread " << *thread << " to terminate in " << maxWait << " seconds");
  if (thread->WaitForTermination(maxWait)) {
    // Orderly exit
    delete thread;
    return false;
  }

  ostringstream strm;
  strm << "Thread \"" << *thread << "\""
#if PTRACING
          "\n";
  PTrace::WalkStack(strm, thread->GetThreadId());
  strm << "  "
#endif
          " failed to terminate in " << maxWait << " seconds";
  PAssertAlways(strm.str().c_str());

  delete thread;

  return true;
}


#define RELEASE_THREAD_LOCAL_STORAGE 1
#if RELEASE_THREAD_LOCAL_STORAGE
static std::set<PThread::LocalStorageBase*> s_ThreadLocalStorage;
static PCriticalSection s_ThreadLocalStorageMutex;
#endif

PThread::~PThread()
{
  if (m_type != e_IsProcess && m_type != e_IsExternal && !WaitForTermination(1000)) {
    Terminate();
    WaitForTermination(1000);
  }

  PTRACE(5, "Destroying thread " << this << ' ' << m_threadName << ", id=" << m_threadId);

#if RELEASE_THREAD_LOCAL_STORAGE
  s_ThreadLocalStorageMutex.Wait();
  for (std::set<PThread::LocalStorageBase*>::iterator it = s_ThreadLocalStorage.begin(); it != s_ThreadLocalStorage.end(); ++it)
    (*it)->ThreadDestroyed(*this);
  s_ThreadLocalStorageMutex.Signal();
#endif

  InternalDestroy();

  if (m_type != e_IsProcess)
    PProcess::Current().InternalThreadEnded(this);
}


PThread::LocalStorageBase::LocalStorageBase()
{
#if RELEASE_THREAD_LOCAL_STORAGE
  s_ThreadLocalStorageMutex.Wait();
  s_ThreadLocalStorage.insert(this);
  s_ThreadLocalStorageMutex.Signal();
#endif
}


void PThread::LocalStorageBase::StorageDestroyed()
{
#if RELEASE_THREAD_LOCAL_STORAGE
  s_ThreadLocalStorageMutex.Wait();
  s_ThreadLocalStorage.erase(this);
  s_ThreadLocalStorageMutex.Signal();
#endif

  m_mutex.Wait();
  for (DataMap::iterator it = m_data.begin(); it != m_data.end(); ++it)
    Deallocate(it->second);
  m_mutex.Signal();
}


void PThread::LocalStorageBase::ThreadDestroyed(PThread & thread)
{
  m_mutex.Wait();
  DataMap::iterator it = m_data.find(thread.GetUniqueIdentifier());
  if (it != m_data.end()) {
    Deallocate(it->second);
    m_data.erase(it);
  }
  m_mutex.Signal();
}


void * PThread::LocalStorageBase::GetStorage() const
{
  PThreadIdentifier threadId = PThread::GetCurrentUniqueIdentifier();
  PWaitAndSignal lock(m_mutex);
  DataMap::iterator it = m_data.find(threadId);
  if (it == m_data.end())
    it = m_data.insert(make_pair(threadId, Allocate())).first;
  return it->second;
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

bool PTimedMutex::EnableDeadlockStackWalk = getenv("PTLIB_DISABLE_DEADLOCK_STACK_WALK") == NULL;

#if PTRACING
static void OutputThreadInfo(ostream & strm, PThreadIdentifier tid, PUniqueThreadIdentifier uid, bool walkStack)
{
  strm << " id=" << PThread::GetIdentifiersAsString(tid, uid) << " name=\"" << PThread::GetThreadName(tid) << '"';
  if (walkStack)
    PTrace::WalkStack(strm, tid);
}
#endif


unsigned PTimedMutex::ExcessiveLockWaitTime;


PMutexExcessiveLockInfo::PMutexExcessiveLockInfo(const char * name, unsigned line, unsigned timeout)
  : m_fileOrName(name)
  , m_fileLine(line)
  , m_excessiveLockTimeout(timeout)
  , m_excessiveLockActive(false)
{
  if (m_excessiveLockTimeout == 0) {
    if (PTimedMutex::ExcessiveLockWaitTime == 0) {
      const char * env = getenv("PTLIB_DEADLOCK_TIME");
      int seconds = env != NULL ? atoi(env) : 0;
      PTimedMutex::ExcessiveLockWaitTime = seconds > 0 ? seconds*1000 : 15000;
    }
    m_excessiveLockTimeout = PTimedMutex::ExcessiveLockWaitTime;
  }
}


PMutexExcessiveLockInfo::PMutexExcessiveLockInfo(const PMutexExcessiveLockInfo & other)
  : m_fileOrName(other.m_fileOrName)
  , m_fileLine(other.m_fileLine)
  , m_excessiveLockTimeout(other.m_excessiveLockTimeout)
  , m_excessiveLockActive(false)
{
}


void PMutexExcessiveLockInfo::PrintOn(ostream &strm) const
{
  if (m_fileOrName != NULL) {
    strm << " (";
    if (m_fileLine != 0)
      strm << PFilePath(m_fileOrName).GetFileName() << ':' << m_fileLine;
    else
      strm << m_fileOrName;
    strm << ')';
  }
}


void PMutexExcessiveLockInfo::ExcessiveLockPhantom(const PObject & mutex) const
{
#if PTRACING
  PTRACE_BEGIN(0, "PTLib") << "Assertion fail: Phantom deadlock in " << mutex << PTrace::End;
#else
  PAssertAlways(PSTRSTRM("Phantom deadlock in mutex " << mutex));
#endif
}


void PMutexExcessiveLockInfo::LockReleased(const PObject & mutex)
{
  if (m_excessiveLockActive) {
#if PTRACING
    PTRACE_BEGIN(0, "PTLib") << "Assertion fail: Released phantom deadlock in " << mutex << PTrace::End;
#else
    PAssertAlways(PSTRSTRM("Released phantom deadlock in mutex " << mutex));
#endif
    m_excessiveLockActive = false;
  }
}


void PTimedMutex::ExcessiveLockWait()
{
#if PTRACING
  PThreadIdentifier lockerId = m_lockerId;
  PThreadIdentifier lastLockerId = m_lastLockerId;
  PUniqueThreadIdentifier lastUniqueId = m_lastUniqueId;

  ostream & trace = PTRACE_BEGIN(0, "PTLib");
  trace << "Assertion fail: Possible deadlock in " << *this << "\n  Blocked Thread";
  OutputThreadInfo(trace, PThread::GetCurrentThreadId(), PThread::GetCurrentUniqueIdentifier(), EnableDeadlockStackWalk);
  trace << "\n  Owner Thread ";
  if (lockerId != PNullThreadIdentifier)
    OutputThreadInfo(trace, lockerId, lastUniqueId, EnableDeadlockStackWalk);
  else {
    trace << "no longer has lock, last owner:";
    OutputThreadInfo(trace, lastLockerId, lastUniqueId, false);
  }
  trace << PTrace::End;
#else
  PAssertAlways(PSTRSTRM("Possible deadlock in " << *this));
#endif

  m_excessiveLockActive = true;
}


void PTimedMutex::CommonSignal()
{
  LockReleased(*this);
  m_lockerId = PNullThreadIdentifier;
}


void PTimedMutex::PrintOn(ostream &strm) const
{
  strm << "mutex " << this;
  PMutexExcessiveLockInfo::PrintOn(strm);
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

PReadWriteMutex::PReadWriteMutex(const char * name, unsigned line, unsigned timeout)
  : PMutexExcessiveLockInfo(name, line, timeout)
#if P_READ_WRITE_ALGO2
  , m_inSemaphore(1, 1)
  , m_inCount(0)
  , m_outSemaphore(1, 1)
  , m_outCount(0)
  , m_writeSemaphore(0, 1)
  , m_wait(false)
#else
  , m_readerSemaphore(1, 1)
  , m_readerMutex(name, line, timeout)
  , m_readerCount(0)
  , m_starvationPreventer(name, line, timeout)
  , m_writerSemaphore(1, 1)
  , m_writerMutex(name, line, timeout)
  , m_writerCount(0)
#endif
{
  PTRACE(5, "Created read/write mutex " << *this);
}


PReadWriteMutex::~PReadWriteMutex()
{
  PTRACE(5, "Destroying read/write mutex " << *this);

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

  if (sync.Wait(m_excessiveLockTimeout)) {
    nest.m_waiting = false;
    return;
  }

  m_excessiveLockActive = true;

  NestMap nestedThreadsToDump;
  {
    PWaitAndSignal mutex(m_nestingMutex);
    nestedThreadsToDump = m_nestedThreads;
  }

#if PTRACING
  {
    ostream & trace = PTRACE_BEGIN(0, "PTLib");
    trace << "Assertion fail: Possible deadlock in " << *this << " :\n";
    for (NestMap::const_iterator it = nestedThreadsToDump.begin(); it != nestedThreadsToDump.end(); ++it) {
      if (it != nestedThreadsToDump.begin())
        trace << '\n';
      trace << "  thread-id=" << it->first << " (0x" << std::hex << it->first << std::dec << "),"
        " unique-id=" << it->second.m_uniqueId << ","
        " readers=" << it->second.m_readerCount << ","
        " writers=" << it->second.m_writerCount;
      if (!it->second.m_waiting)
        trace << ", LOCKER";
      if (PTimedMutex::EnableDeadlockStackWalk)
        PTrace::WalkStack(trace, it->first);
    }
    trace << PTrace::End;
  }
#else
  PAssertAlways(PSTRSTRM("Possible deadlock in " << *this));
#endif

  sync.Wait();
  ExcessiveLockPhantom(*this);

  nest.m_waiting = false;
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

  LockReleased(*this);

  // Do text book read lock
  InternalEndRead(*nest);

  // At this point all read and write locks are gone for this thread so we can
  // reclaim the memory.
  EndNest();
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

  LockReleased(*this);

  // If have a read lock already in this thread then do the "real" unlock code
  // but do not change the lock count, calls to EndRead() will now just
  // decrement the count instead of doing the unlock (its already done!)
  if (nest.m_readerCount > 0)
    InternalEndRead(nest);

  // Note in this gap another thread could grab the write lock

  InternalStartWrite(nest);
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

  InternalEndWrite(*nest);

  // Now check to see if there was a read lock present for this thread, if so
  // then reacquire the read lock (not changing the count) otherwise clean up the
  // memory for the nested thread info structure
  if (nest->m_readerCount > 0)
    InternalStartRead(*nest);
  else
    EndNest();
}


void PReadWriteMutex::InternalStartRead(Nest & nest)
{
#if P_READ_WRITE_ALGO2
  InternalWait(nest, m_inSemaphore);
  ++m_inCount;
  m_inSemaphore.Signal();
#else
  InternalWait(nest, m_starvationPreventer);
   InternalWait(nest, m_readerSemaphore);
    InternalWait(nest, m_readerMutex);

     m_readerCount++;
     if (m_readerCount == 1)
       InternalWait(nest, m_writerSemaphore);

    m_readerMutex.Signal();
   m_readerSemaphore.Signal();
  m_starvationPreventer.Signal();
#endif
}


void PReadWriteMutex::InternalEndRead(Nest & nest)
{
#if P_READ_WRITE_ALGO2
  InternalWait(nest, m_outSemaphore);
  ++m_outCount;
  if (m_wait && m_inCount == m_outCount)
    m_writeSemaphore.Signal();
  m_outSemaphore.Signal();
#else
  InternalWait(nest, m_readerMutex);

  m_readerCount--;
  if (m_readerCount == 0)
    m_writerSemaphore.Signal();

  m_readerMutex.Signal();
#endif
}


void PReadWriteMutex::InternalStartWrite(Nest & nest)
{
#if P_READ_WRITE_ALGO2
  InternalWait(nest, m_inSemaphore);
  InternalWait(nest, m_outSemaphore);
  if (m_inCount == m_outCount)
    m_outSemaphore.Signal();
  else {
    m_wait = true;
    m_outSemaphore.Signal();
    InternalWait(nest, m_writeSemaphore);
    m_wait = false;
  }
#else
  InternalWait(nest, m_writerMutex);

  m_writerCount++;
  if (m_writerCount == 1)
    InternalWait(nest, m_readerSemaphore);

  m_writerMutex.Signal();

  InternalWait(nest, m_writerSemaphore);
#endif
}


void PReadWriteMutex::InternalEndWrite(Nest & nest)
{
#if P_READ_WRITE_ALGO2
  m_inSemaphore.Signal();
#else
  m_writerSemaphore.Signal();

  InternalWait(nest, m_writerMutex);

  m_writerCount--;
  if (m_writerCount == 0)
    m_readerSemaphore.Signal();

  m_writerMutex.Signal();
#endif
}

void PReadWriteMutex::PrintOn(ostream & strm) const
{
  strm << "read/write mutex " << this;
  PMutexExcessiveLockInfo::PrintOn(strm);
}


///////////////////////////////////////////////////////////////////////////////

PBoolean PCriticalSection::Wait(const PTimeInterval & timeout)
{
  if (timeout == 0)
    return Try();

  PTRACE(2, "PCriticalSection::Wait() called, this is very inefficient, consider using PTimedMutex!");

  PSimpleTimer timer(timeout);
  do {
    if (Try())
      return true;
    PThread::Sleep(100);
  } while (timer.IsRunning());
  return false;
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
  } while (id == Invalid || !m_inUse.insert(id).second);

  m_mutex->Signal();

  return id;
}


void PIdGenerator::Release(Handle id)
{
  if (id == Invalid || m_mutex == NULL) // Before construction or after destruction
    return;

  m_mutex->Wait();

  m_inUse.erase(id);

  m_mutex->Signal();
}


bool PIdGenerator::IsValid(Handle id) const
{
  if (id == Invalid || m_mutex == NULL) // Before construction or after destruction
    return false;

  PWaitAndSignal mutex(*m_mutex);
  return m_inUse.find(id) != m_inUse.end();
}

// End Of File ///////////////////////////////////////////////////////////////
