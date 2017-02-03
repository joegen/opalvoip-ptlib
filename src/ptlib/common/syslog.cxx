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

#if P_SYSTEMLOG

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
  : std::ostream(&m_buffer)
  , m_logLevel(level)
{ 
  m_buffer.m_log = this;
}


PSystemLog::PSystemLog(const PSystemLog & other)
  : PObject(other)
  , std::ostream(&m_buffer)
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
  // Make sure there is a trailing NULL at end of string
  overflow('\0');

#if PTRACING
  PSystemLog::Level logLevel = m_log->m_logLevel;

  if ((PTrace::GetOptions()&PTrace::SystemLogStream) != 0) {
    /* If we get called via PTrace::End, then the "width" is set to the log level
       used. If it is set to special value, then this is a PSYSTEMLOG output and
       we go through the tracing system so get consistent output format. */
    if (m_log->width() == -12345678) {
      PTrace::Begin(logLevel, NULL, 0, NULL, "SystemLog") << m_string << PTrace::End;
      logLevel = NumLogLevels;
    }
    else if (m_log->width() > 0) {
      unsigned traceLevel = (unsigned)m_log->width() - 1;
      m_log->width(0);
      logLevel = traceLevel < NumLogLevels ? LevelFromInt(traceLevel) : EndLevel;
    }
  }
  if (logLevel < NumLogLevels)
    OutputToTarget(logLevel, m_string);
#else
  OutputToTarget(m_log->m_logLevel, m_string);
#endif

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


void PSystemLog::OutputToTarget(PSystemLog::Level level, const char * msg)
{
  g_SystemLogTarget.m_targetMutex.Wait();
  if (g_SystemLogTarget.m_targetPointer != NULL)
    g_SystemLogTarget.m_targetPointer->Output(level, msg);
  g_SystemLogTarget.m_targetMutex.Signal();
}


///////////////////////////////////////////////////////////////

PSystemLogTarget::PSystemLogTarget()
  : m_thresholdLevel(PSystemLog::Warning)
  , m_outputLevelName(true)
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


void PSystemLogTarget::OutputToStream(ostream & stream, PSystemLog::Level level, const char * msg, int timeZone)
{
  if (level > m_thresholdLevel || !PProcess::IsInitialised())
    return;

  PTime now;
  stream << now.AsString(PTime::LoggingFormat, timeZone) << '\t';

  if (m_outputLevelName) {
    if (level < 0)
      stream << "Message";
    else {
      static const char * const levelName[] = {
        "Fatal error",
        "Error",
        "Warning",
        "Info"
      };
      if ((PINDEX)level < PARRAYSIZE(levelName))
        stream << levelName[level];
      else
        stream << "Debug" << (level - PSystemLog::Info);
    }
    stream << '\t';
  }

  stream << msg;
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
  , m_outputting(false)
{
  m_file.SetFilePath(filename);
}


void PSystemLogToFile::Output(PSystemLog::Level level, const char * msg)
{
  PWaitAndSignal mutex(m_mutex);

  if (m_outputting)
      return;

  m_outputting = true;

  if (InternalOpen()) {
      OutputToStream(m_file, level, msg, m_rotateInfo.m_timeZone);
      Rotate(false);
  }

  m_outputting = false;
}


bool PSystemLogToFile::Clear()
{
  PWaitAndSignal mutex(m_mutex);
  if (!m_file.Remove(true))
    return false;

  if (!m_file.Open())
    return false;

  OutputToStream(m_file, PSystemLog::Warning, "Cleared log file.", m_rotateInfo.m_timeZone);
  return true;
}


void PSystemLogToFile::SetRotateInfo(const RotateInfo & info, bool force)
{
#if PTRACING
  if (info.m_timeZone == PTime::GMT)
    PTrace::SetOptions(PTrace::GMTTime);
  else
    PTrace::ClearOptions(PTrace::GMTTime);
#endif
  m_mutex.Wait();
  m_rotateInfo = info;
  Rotate(force);
  m_mutex.Signal();
}


bool PSystemLogToFile::Rotate(bool force)
{
  PWaitAndSignal mutex(m_mutex);

  if (!m_rotateInfo.CanRotate())
    return false;

  if (!force && m_file.GetLength() < m_rotateInfo.m_maxSize)
    return false;

  PFilePath rotatedFile;
  PString timestamp = PTime().AsString(m_rotateInfo.m_timestamp, m_rotateInfo.m_timeZone);
  PString tiebreak;
  do {
      rotatedFile = PSTRSTRM(m_rotateInfo.m_directory <<
                             m_rotateInfo.m_prefix <<
                             timestamp << tiebreak <<
                             m_rotateInfo.m_suffix);
      tiebreak = tiebreak.AsInteger()-1;
  } while (PFile::Exists(rotatedFile));

  if (m_file.IsOpen()) {
    OutputToStream(m_file, PSystemLog::StdError, "Log rotated to " + rotatedFile, m_rotateInfo.m_timeZone);
    m_file.Close();
  }

  bool ok = PFile::Move(m_file.GetFilePath(), rotatedFile, false, true);
  InternalOpen();

  if (m_rotateInfo.m_maxFileCount > 0 || m_rotateInfo.m_maxFileAge > 0) {
    std::multimap<PTime, PFilePath> rotatedFiles;
    PDirectory dir(m_rotateInfo.m_directory);
    if (dir.Open(PFileInfo::RegularFile)) {
      int failsafe = 10000;
      do {
        PString name = dir.GetEntryName();
        PFileInfo info;
        if (m_rotateInfo.m_prefix == name.Left(m_rotateInfo.m_prefix.GetLength()) &&
            m_rotateInfo.m_suffix == name.Right(m_rotateInfo.m_suffix.GetLength()) &&
            dir.GetInfo(info))
          rotatedFiles.insert(std::multimap<PTime, PFilePath>::value_type(info.modified, dir + name));
      } while (dir.Next() && --failsafe > 0);
    }

    if (m_rotateInfo.m_maxFileCount > 0) {
      while (rotatedFiles.size() > m_rotateInfo.m_maxFileCount) {
        PFilePath filePath = rotatedFiles.begin()->second;
        if (PFile::Remove(filePath))
          OutputToStream(m_file, PSystemLog::Info, "Removed excess rotated log " + filePath, m_rotateInfo.m_timeZone);
        else
          OutputToStream(m_file, PSystemLog::Warning, "Could not remove excess rotated log " + filePath, m_rotateInfo.m_timeZone);
        rotatedFiles.erase(rotatedFiles.begin());
      }
    }

    if (m_rotateInfo.m_maxFileAge > 0) {
      PTime then = PTime() - m_rotateInfo.m_maxFileAge;
      while (!rotatedFiles.empty() && rotatedFiles.begin()->first < then) {
        PFilePath filePath = rotatedFiles.begin()->second;
        if (PFile::Remove(filePath))
          OutputToStream(m_file, PSystemLog::Info, "Removed aged rotated log " + filePath, m_rotateInfo.m_timeZone);
        else
          OutputToStream(m_file, PSystemLog::Warning, "Could not remove aged rotated log " + filePath, m_rotateInfo.m_timeZone);
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

  // Make sure directory exists, don't care if Create() fails, which is nearly all the time
  m_file.GetFilePath().GetDirectory().Create(PFileInfo::DefaultDirPerms, true);

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
         " to \"" << m_file.GetFilePath() << "\", ";
  switch (m_rotateInfo.m_timeZone) {
    case PTime::GMT :
      log << "GMT";
      break;
    case PTime::Local :
      log << "Local Time";
      break;
    default :
      log << "Time Zone: " << setw(4) << setfill('0') << showpos << m_rotateInfo.m_timeZone;
  }
  OutputToStream(m_file, PSystemLog::StdError, log, m_rotateInfo.m_timeZone);
  return true;
}


PSystemLogToFile::RotateInfo::RotateInfo(const PDirectory & dir)
  : m_directory(dir)
  , m_prefix(PProcess::Current().GetName())
  , m_timestamp("_yyyy_MM_dd_hh_mm")
#if PTRACING
  , m_timeZone((PTrace::GetOptions()&PTrace::GMTTime) ? PTime::GMT : PTime::Local)
#else
  , m_timeZone(PTime::Local)
#endif
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
    if ((PINDEX)level < PARRAYSIZE(levelName))
      syslog(priority, "%-8s%s", levelName[level], msg);
    else
      syslog(priority, "DEBUG%-3u%s", level - PSystemLog::Info, msg);
  }
}

#endif // _WIN32/P_ANDROID


#endif // P_SYSTEMLOG


// End Of File ///////////////////////////////////////////////////////////////
