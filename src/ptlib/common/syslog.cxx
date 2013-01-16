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
  if (level > m_thresholdLevel)
    return;

  PTime now;
  stream << now.AsString("yyyy/MM/dd hh:mm:ss.uuu\t");

  if (level < 0)
    stream << "Message";
  else {
    static const char * const levelName[4] = {
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
  PTrace::Begin(level, NULL, 0) << msg << PTrace::End;
}
#endif


///////////////////////////////////////////////////////////////

PSystemLogToFile::PSystemLogToFile(const PString & filename)
  : m_file(filename, PFile::WriteOnly)
{
}


void PSystemLogToFile::Output(PSystemLog::Level level, const char * msg)
{
  OutputToStream(m_file, level, msg);
}


///////////////////////////////////////////////////////////////

PSystemLogToNetwork::PSystemLogToNetwork(const PIPSocket::Address & address, WORD port, unsigned facility)
  : m_host(address)
  , m_port(port)
  , m_facility(facility)
{
}


PSystemLogToNetwork::PSystemLogToNetwork(const PString & hostname, WORD port, unsigned facility)
  : m_port(port)
  , m_facility(facility)
{
  PIPSocket::GetHostAddress(hostname, m_host);
}


void PSystemLogToNetwork::Output(PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel || m_port == 0 || !m_host.IsValid())
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
  m_socket.WriteTo((const char *)str, str.GetLength(), m_host, m_port);
}


///////////////////////////////////////////////////////////////

#ifdef WIN32
void PSystemLogToDebug::Output(PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel)
    return;

  PStringStream strm;
  OutputToStream(strm, level, msg);
  PVarString str = strm;
  OutputDebugString(str);
}
#else

#include <syslog.h>

PSystemLogToSyslog::PSystemLogToSyslog(const char * ident)
{
  if (ident == NULL)
    ident = (const char *)PProcess::Current().GetName();
  openlog((char *)ident, LOG_PID, LOG_DAEMON);
}

PSystemLogToSyslog::~PSystemLogToSyslog()
{
  closelog();
}


void PSystemLogToSyslog::Output(PSystemLog::Level level, const char * msg)
{
  if (level > m_thresholdLevel)
    return;

  int syslog_level;
  switch (level) {
    case PSystemLog::Fatal :
      syslog_level = LOG_CRIT;
      break;
    case PSystemLog::Error :
      syslog_level = LOG_ERR;
      break;
    case PSystemLog::StdError :
    case PSystemLog::Warning :
      syslog_level = LOG_WARNING;
      break;
    case PSystemLog::Info :
      syslog_level = LOG_INFO;
      break;
    default :
      syslog_level = LOG_DEBUG;
  }

  syslog(syslog_level, "%s", msg);
}
#endif


// End Of File ///////////////////////////////////////////////////////////////
