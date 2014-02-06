/*
 * syslog.cxx
 *
 * System Logging class.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2009 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "syslog.h"
#endif

#include <ptlib/syslog.h>
#include <ptlib/pprocess.h>


/////////////////////////////////////////////////////////////////////////////

static struct PSystemLogTargetGlobal
{
  PSystemLogTargetGlobal()
  {
    m_targetPointer = new PSystemLogToNowhere;
    m_targetAutoDelete = true;
  }
  ~PSystemLogTargetGlobal()
  {
    if (m_targetAutoDelete)
      delete m_targetPointer;
    m_targetPointer = NULL;
  }
  void Set(PSystemLogTarget * target, bool autoDelete)
  {
    m_targetMutex.Wait();

    PSystemLog::Level level = m_targetPointer->GetThresholdLevel();

    if (m_targetAutoDelete)
      delete m_targetPointer;

    if (target != NULL) {
      m_targetPointer = target;
      m_targetAutoDelete = autoDelete;
    }
    else {
      m_targetPointer = new PSystemLogToNowhere;
      m_targetAutoDelete = true;
    }

    m_targetPointer->SetThresholdLevel(level);

    m_targetMutex.Signal();
  }

  PMutex             m_targetMutex;
  PSystemLogTarget * m_targetPointer;
  bool               m_targetAutoDelete;
} g_SystemLogTarget;


/////////////////////////////////////////////////////////////////////////////

PSystemLog::PSystemLog(Level level)   ///< only messages at this level or higher will be logged
  : P_IOSTREAM(cout.rdbuf())
  , m_logLevel(level)
{ 
  m_buffer.m_log = this;
  init(&m_buffer);
}


PSystemLog::PSystemLog(const PSystemLog & other)
  : PObject(other)
  , P_IOSTREAM(cout.rdbuf()) 
{
}


PSystemLog & PSystemLog::operator=(const PSystemLog &)
{ 
  return *this; 
}


///////////////////////////////////////////////////////////////

PSystemLog::Buffer::Buffer()
{
  PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
  char * newptr = m_string.GetPointer(32);
  setp(newptr, newptr + m_string.GetSize() - 1);
}


streambuf::int_type PSystemLog::Buffer::overflow(int_type c)
{
  if (pptr() >= epptr()) {
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

    size_t ppos = pptr() - pbase();
    char * newptr = m_string.GetPointer(m_string.GetSize() + 32);
    setp(newptr, newptr + m_string.GetSize() - 1);
    pbump(ppos);
  }

  if (c != EOF) {
    *pptr() = (char)c;
    pbump(1);
  }

  return 0;
}


streambuf::int_type PSystemLog::Buffer::underflow()
{
  return EOF;
}


int PSystemLog::Buffer::sync()
{
  PSystemLog::Level logLevel = m_log->m_logLevel;

#if PTRACING
  if (m_log->width() > 0 && (PTrace::GetOptions()&PTrace::SystemLogStream) != 0) {
    // Trace system sets the ios stream width as the last thing it does before
    // doing a flush, which gets us here. SO now we can get a PTRACE looking
    // exactly like a PSYSTEMLOG of appropriate level.
    unsigned traceLevel = (unsigned)m_log->width() - 1;
    m_log->width(0);
    if (traceLevel >= PSystemLog::NumLogLevels)
      traceLevel = PSystemLog::NumLogLevels-1;
    logLevel = (Level)traceLevel;
  }
#endif

  // Make sure there is a trailing NULL at end of string
  overflow('\0');

  g_SystemLogTarget.m_targetMutex.Wait();
  if (g_SystemLogTarget.m_targetPointer != NULL)
    g_SystemLogTarget.m_targetPointer->Output(logLevel, m_string);
  g_SystemLogTarget.m_targetMutex.Signal();

  PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;

  m_string.SetSize(10);
  char * base = m_string.GetPointer();
  *base = '\0';
  setp(base, base + m_string.GetSize() - 1);
 
  return 0;
}


PSystemLogTarget & PSystemLog::GetTarget()
{
  return *PAssertNULL(g_SystemLogTarget.m_targetPointer);
}


void PSystemLog::SetTarget(PSystemLogTarget * target, bool autoDelete)
{
  g_SystemLogTarget.Set(target, autoDelete);
}


///////////////////////////////////////////////////////////////

PSystemLogTarget::PSystemLogTarget()
  : m_thresholdLevel(PSystemLog::Warning)
{
}


PSystemLogTarget::PSystemLogTarget(const PSystemLogTarget & other)
  : PObject(other)
{
}


PSystemLogTarget & PSystemLogTarget::operator=(const PSystemLogTarget &)
{ 
  return *this; 
}


void PSystemLogTarget::OutputToStream(ostream & stream, PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel || !PProcess::IsInitialised())
    return;

  PTime now;
  stream << now.AsString(PTime::LoggingFormat) << '\t';

  if (level < 0)
    stream << "Message";
  else {
    static const char * const levelName[] = {
      "Fatal error",
      "Error",
      "Warning",
      "Info"
    };
    if (level < PARRAYSIZE(levelName))
      stream << levelName[level];
    else
      stream << "Debug" << (level - PSystemLog::Info);
  }

  stream << '\t' << msg;
  if (msg[0] == '\0' || msg[strlen(msg)-1] != '\n')
    stream << endl;
}


///////////////////////////////////////////////////////////////

void PSystemLogToStderr::Output(PSystemLog::Level level, const char * msg)
{
  OutputToStream(cerr, level, msg);
}


///////////////////////////////////////////////////////////////

#if PTRACING
PSystemLogToTrace::PSystemLogToTrace()
{
  // Must be off or recurses!
  PTrace::ClearOptions(PTrace::SystemLogStream);
  if (PIsDescendant(PTrace::GetStream(), PSystemLog))
    PTrace::SetStream(NULL);
}

void PSystemLogToTrace::Output(PSystemLog::Level level, const char * msg)
{
  if (PTrace::CanTrace(level))
    PTrace::Begin(level, NULL, 0) << msg << PTrace::End;
}
#endif


///////////////////////////////////////////////////////////////

PSystemLogToFile::PSystemLogToFile(const PFilePath & filename)
  : m_rotateInfo(filename.GetDirectory())
{
  m_file.SetFilePath(filename);
}


void PSystemLogToFile::Output(PSystemLog::Level level, const char * msg)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalOpen())
    return;

  OutputToStream(m_file, level, msg);
  Rotate(false);
}


bool PSystemLogToFile::Clear()
{
  PWaitAndSignal mutex(m_mutex);
  if (!m_file.Remove(true))
    return false;

  if (!m_file.Open())
    return false;

  OutputToStream(m_file, PSystemLog::Warning, "Cleared log file.");
  return true;
}


void PSystemLogToFile::SetRotateInfo(const RotateInfo & info, bool force)
{
  m_mutex.Wait();
  m_rotateInfo = info;
  Rotate(force);
  m_mutex.Signal();
}


bool PSystemLogToFile::Rotate(bool force)
{
  // Lock expected to be already in place
  if (!m_rotateInfo.CanRotate())
    return false;

  if (!force && m_file.GetLength() < m_rotateInfo.m_maxSize)
    return false;

  PFilePath rotatedFile = m_rotateInfo.m_directory +
                          m_rotateInfo.m_prefix +
                          PTime().AsString(m_rotateInfo.m_timestamp) +
                          m_rotateInfo.m_suffix;

  if (m_file.IsOpen()) {
    OutputToStream(m_file, PSystemLog::StdError, "Log rotated to " + rotatedFile);
    m_file.Close();
  }

  bool ok = PFile::Move(m_file.GetFilePath(), rotatedFile, false, true);
  InternalOpen();

  if (m_rotateInfo.m_maxFileCount > 0 || m_rotateInfo.m_maxFileAge > 0) {
    std::multimap<PTime, PFilePath> rotatedFiles;
    PDirectory dir(m_rotateInfo.m_directory);
    if (dir.Open(PFileInfo::RegularFile)) {
      do {
        PString name = dir.GetEntryName();
        PFileInfo info;
        if (m_rotateInfo.m_prefix == name.Left(m_rotateInfo.m_prefix.GetLength()) &&
            m_rotateInfo.m_suffix == name.Right(m_rotateInfo.m_suffix.GetLength()) &&
            dir.GetInfo(info))
          rotatedFiles.insert(std::multimap<PTime, PFilePath>::value_type(info.modified, dir + name));
      } while (dir.Next());
    }

    if (m_rotateInfo.m_maxFileCount > 0) {
      while (rotatedFiles.size() > m_rotateInfo.m_maxFileCount) {
        if (PFile::Remove(rotatedFiles.begin()->second))
          PTRACE(3, "SystemLog", "Removed excess rotated log " << rotatedFiles.begin()->second);
        rotatedFiles.erase(rotatedFiles.begin());
      }
    }

    if (m_rotateInfo.m_maxFileAge > 0) {
      PTime then = PTime() - m_rotateInfo.m_maxFileAge;
      while (rotatedFiles.begin()->first < then) {
        if (PFile::Remove(rotatedFiles.begin()->second))
          PTRACE(3, "SystemLog", "Removed aged rotated log " << rotatedFiles.begin()->second);
        rotatedFiles.erase(rotatedFiles.begin());
      }
    }
  }

  return ok;
}


bool PSystemLogToFile::InternalOpen()
{
  if (m_file.IsOpen())
    return true;

  if (!m_file.Open(PFile::WriteOnly))
    return false;

  PProcess & process = PProcess::Current();
  PStringStream log;
  log << process.GetName()
      << " version " << process.GetVersion(true)
      << " by " << process.GetManufacturer()
      << " on "
      << PProcess::GetOSClass() << ' ' << PProcess::GetOSName()
      << " (" << PProcess::GetOSVersion() << '-' << PProcess::GetOSHardware() << ")"
         " with PTLib (v" << PProcess::GetLibVersion() << ")"
         " to \"" << m_file.GetFilePath() << '"';
  OutputToStream(m_file, PSystemLog::StdError, log);
  return true;
}


PSystemLogToFile::RotateInfo::RotateInfo(const PDirectory & dir)
  : m_directory(dir)
  , m_prefix(PProcess::Current().GetName())
  , m_timestamp("_yyyy_MM_dd_hh_mm")
  , m_suffix(".log")
  , m_maxSize(0)
  , m_maxFileCount(0)
{
}


///////////////////////////////////////////////////////////////

PSystemLogToNetwork::PSystemLogToNetwork(const PIPSocket::Address & address, WORD port, unsigned facility)
  : m_server(address, port)
  , m_facility(facility)
{
}


PSystemLogToNetwork::PSystemLogToNetwork(const PString & server, WORD port, unsigned facility)
  : m_facility(facility)
{
  m_server.Parse(server, port, ':', "udp");
}


void PSystemLogToNetwork::Output(PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel || !m_server.IsValid() || !PProcess::IsInitialised())
    return;

  static int PwlibLogToSeverity[PSystemLog::NumLogLevels] = {
    2, 3, 4, 5, 6, 7, 7, 7, 7, 7
  };

  PStringStream str;
  str << '<' << (((m_facility*8)+PwlibLogToSeverity[level])%1000) << '>'
    << PTime().AsString("MMM dd hh:mm:ss ")
    << PIPSocket::GetHostName() << ' '
    << PProcess::Current().GetName() << ' '
    << msg;
  m_socket.WriteTo((const char *)str, str.GetLength(), m_server);
}


///////////////////////////////////////////////////////////////

#if defined(_WIN32)

void PSystemLogToDebug::Output(PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel || !PProcess::IsInitialised())
    return;

  PStringStream strm;
  OutputToStream(strm, level, msg);
  PVarString str = strm;
  OutputDebugString(str);
}

#elif defined(P_ANDROID)

#include <android/log.h>

PSystemLogToSyslog::PSystemLogToSyslog(const char * ident, int priority, int options, int facility)
  : m_ident(ident)
  , m_priority(priority)
{
  if (m_ident.IsEmpty() && PProcess::IsInitialised())
    m_ident = PProcess::Current().GetName();
}


PSystemLogToSyslog::~PSystemLogToSyslog()
{
}


void PSystemLogToSyslog::Output(PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel || !PProcess::IsInitialised())
    return;

  android_LogPriority priority;
  switch (level) {
    case PSystemLog::Fatal :
      priority = ANDROID_LOG_FATAL;
      break;
    case PSystemLog::Error :
      priority = ANDROID_LOG_ERROR;
      break;
    case PSystemLog::StdError :
    case PSystemLog::Warning :
      priority = ANDROID_LOG_WARN;
      break;
    case PSystemLog::Info :
      priority = ANDROID_LOG_INFO;
      break;
    case PSystemLog::Debug :
      priority = ANDROID_LOG_DEBUG;
      break;
    default :
      priority = ANDROID_LOG_VERBOSE;
  }

  const char * tag = m_ident;
  if (m_ident.IsEmpty() && PProcess::IsInitialised())
    tag = PProcess::Current().GetName();
  else
    tag = m_ident;

  __android_log_write(priority, tag, msg);
}

#else // _WIN32/P_ANDROID

#include <syslog.h>

PSystemLogToSyslog::PSystemLogToSyslog(const char * ident, int priority, int options, int facility)
  : m_ident(ident)
  , m_priority(priority)
{
  if (m_ident.IsEmpty())
    m_ident = PProcess::Current().GetName();

  if (options < 0)
    options = LOG_PID;

  if (facility < 0)
    facility = LOG_DAEMON;

  openlog(m_ident, options, facility);
}


PSystemLogToSyslog::~PSystemLogToSyslog()
{
  closelog();
}


void PSystemLogToSyslog::Output(PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel || !PProcess::IsInitialised())
    return;

  int priority = m_priority;
  if (priority < 0) {
    switch (level) {
      case PSystemLog::Fatal :
        priority = LOG_CRIT;
        break;
      case PSystemLog::Error :
        priority = LOG_ERR;
        break;
      case PSystemLog::StdError :
      case PSystemLog::Warning :
        priority = LOG_WARNING;
        break;
      case PSystemLog::Info :
        priority = LOG_INFO;
        break;
      default :
        priority = LOG_DEBUG;
    }
    syslog(priority, "%s", msg);
  }
  else {
    static const char * const levelName[] = {
        "FATAL",    // LogFatal,
        "ERROR",    // LogError,
        "WARNING",  // LogWarning,
        "INFO",     // LogInfo,
    };
    if (level < PARRAYSIZE(levelName))
      syslog(priority, "%-8s%s", levelName[level], msg);
    else
      syslog(priority, "DEBUG%-3u%s", level - PSystemLog::Info, msg);
  }
}

#endif // _WIN32/P_ANDROID


// End Of File ///////////////////////////////////////////////////////////////
