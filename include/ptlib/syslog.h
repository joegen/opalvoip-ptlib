/*
 * syslog.h
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
 */

#ifndef _PSYSTEMLOG
#define _PSYSTEMLOG

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include "ptlib_config.h"

#if P_SYSTEMLOG

#include "ptlib/udpsock.h"

class PSystemLogTarget;


/** This class abstracts the operating system dependent error logging facility.
    To send messages to the system error log, the PSYSTEMLOG macro should be used. 
  */

class PSystemLog : public PObject, public std::ostream
{
    PCLASSINFO(PSystemLog, PObject);
  public:
  /**@name Construction */
  //@{
    /// define the different error log levels
    P_DECLARE_ENUM_EX(Level,NumLogLevels,
      StdError,-1,  ///< Log from standard error stream
      Fatal,        ///< Log a fatal error
      Error,        ///< Log a non-fatal error
      Warning,      ///< Log a warning
      Info,         ///< Log general information
      Debug,        ///< Log debugging information
      Debug2,       ///< Log more debugging information
      Debug3,       ///< Log even more debugging information
      Debug4,       ///< Log a lot of debugging information
      Debug5,       ///< Log a real lot of debugging information
      Debug6        ///< Log a bucket load of debugging information
    );

    /// Create a system log stream
    PSystemLog(
     Level level = NumLogLevels  ///< only messages at this level or higher (smaller) will be logged
    );

    /// Destroy the string stream, deleting the stream buffer
    ~PSystemLog() { flush(); }
  //@}

  /**@name Miscellaneous functions */
  //@{
    /** Get the current target/destination for system logging.
      */
    static PSystemLogTarget & GetTarget();

    /** Set the current target/destination for system logging.
      */
    static void SetTarget(
      PSystemLogTarget * target,  ///< New target/destination for logging.
      bool autoDelete = true      ///< Indicate target is to be deleted when no longer in use.
    );

    /// Output string to active target.
    static void OutputToTarget(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    );
  //@}

  private:
    PSystemLog(const PSystemLog & other);
    PSystemLog & operator=(const PSystemLog &);

    class Buffer : public streambuf {
      public:
        Buffer();
        virtual int_type overflow(int_type=EOF);
        virtual int_type underflow();
        virtual int sync();
        PSystemLog * m_log;
        PCharArray   m_string;
    } m_buffer;
    friend class Buffer;

    Level m_logLevel;

  friend class PSystemLogTarget;
};


class PSystemLogTarget : public PObject
{
    PCLASSINFO(PSystemLogTarget, PObject);
  public:
  /**@name Construction */
  //@{
    PSystemLogTarget();
  //@}

  /**@name Miscellaneous functions */
  //@{
    /** Set the level at which errors are logged. Only messages higher than or
       equal to the specified level will be logged.
      */
    void SetThresholdLevel(
      PSystemLog::Level level  ///< New log level
    ) { m_thresholdLevel = level; }

    /** Get the current level for logging.

       @return
       Log level.
     */
    PSystemLog::Level GetThresholdLevel() const { return m_thresholdLevel; }

    /** Set flag to output the level name ("Eror", "Warning" etc).
      */
    void SetOutputLevelName(
      bool flag  ///< New flag
    ) { m_outputLevelName = flag; }

    /** Get flag to output the level name ("Eror", "Warning" etc).
     */
    bool GetOutputLevelName() const { return m_outputLevelName; }
  //@}

  protected:
  /**@name Output functions */
  //@{
    /** Log an error into the system log.
     */
    virtual void Output(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    ) = 0;

    /** Log an error into the specified stream.
     */
    void OutputToStream(
      ostream & strm,           ///< Stream to output
      PSystemLog::Level level,  ///< Level of this message
      const char * msg,         ///< Message to be logged
      int timeZone = PTime::Local ///< Zone for time output
    );
  //@}

  protected:
    PAtomicEnum<PSystemLog::Level> m_thresholdLevel;
    bool m_outputLevelName;

  private:
    PSystemLogTarget(const PSystemLogTarget & other);
    PSystemLogTarget & operator=(const PSystemLogTarget &);

  friend void PSystemLog::OutputToTarget(PSystemLog::Level level, const char * msg);
};


/** Log system output to nowhere.
  */
class PSystemLogToNowhere : public PSystemLogTarget
{
    PCLASSINFO(PSystemLogToNowhere, PSystemLogTarget);
  public:
    virtual void Output(PSystemLog::Level, const char *)
    {
    }
};


/** Log system output to stderr.
  */
class PSystemLogToStderr : public PSystemLogTarget
{
    PCLASSINFO(PSystemLogToStderr, PSystemLogTarget);
  public:
  /**@name Overrides of PSystemLogTarget */
  //@{
    /** Log an error into the system log.
     */
    virtual void Output(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    );
  //@}
};


#if PTRACING
/** Log system output to PTRACE.
  */
class PSystemLogToTrace : public PSystemLogTarget
{
    PCLASSINFO(PSystemLogToTrace, PSystemLogTarget);
  public:
  /**@name Construction */
  //@{
    PSystemLogToTrace();
  //@}

  /**@name Overrides of PSystemLogTarget */
  //@{
    /** Log an error into the system log.
     */
    virtual void Output(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    );
  //@}
};
#endif


/** Log system output to a file.
  */
class PSystemLogToFile : public PSystemLogTarget
{
    PCLASSINFO(PSystemLogToFile, PSystemLogTarget);
  public:
  /**@name Construction */
  //@{
    PSystemLogToFile(
      const PFilePath & filename
    );
  //@}

  /**@name Overrides of PSystemLogTarget */
  //@{
    /** Log an error into the system log.
     */
    virtual void Output(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    );
  //@}

  /**@name Miscellaneous functions */
  //@{
    /**Get the path to the file being logged to.
      */
    const PFilePath & GetFilePath() const { return m_file.GetFilePath(); }

    /**Clear the log file
      */
    bool Clear();

    struct RotateInfo : PFile::RotateInfo
    {
      RotateInfo(const PDirectory & dir) : PFile::RotateInfo(dir) { m_suffix = ".log"; }
      virtual void OnCloseFile(PFile & file, const PFilePath & rotatedTo);
      virtual bool OnOpenFile(PFile & file);
      virtual void OnMessage(bool error, const PString & msg);
    };

    /** Set file rotation template information.
      */
    virtual void SetRotateInfo(
      const RotateInfo & info,  ///< New info for log rotation
      bool force = false        ///< Force rotation immediately
    );

    /** Get file rotation template information.
    */
    const RotateInfo & GetRotateInfo() const { return m_rotateInfo; }

    /** Execute a rotation.
        If \b force is false then the file must be larger than RotateInfo::maxSize.
        If \b force is true the the file is moved regardless of it's current size.

        Note, RotateInfo::maxSize must be non zero and RotateInfo::m_timestamp must
        be non-empty string for this to operate.
      */
    virtual bool Rotate(
      bool force
    );
    //@}

  protected:
    PTextFile  m_file;
    RotateInfo m_rotateInfo;
    PMutex     m_mutex;
    bool       m_outputting;
};


/** Log system output to the network using RFC 3164 BSD syslog protocol.
  */
class PSystemLogToNetwork : public PSystemLogTarget
{
    PCLASSINFO(PSystemLogToNetwork, PSystemLogTarget);
  public:
    enum { RFC3164_Port = 514 };

  /**@name Construction */
  //@{
    PSystemLogToNetwork(
      const PIPSocket::Address & address, ///< Host to which data is sent.
      WORD port = RFC3164_Port,           ///< Port to which data is sent.
      unsigned facility = 16              ///< Facility code
    );
    PSystemLogToNetwork(
      const PString & server,             ///< Host/port to which data is sent.
      WORD port = RFC3164_Port,           ///< Default port to which data is sent.
      unsigned facility = 16              ///< Facility code
    );
  //@}

  /**@name Overrides of PSystemLogTarget */
  //@{
    /** Log an error into the system log.
     */
    virtual void Output(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    );
  //@}

    const PIPSocket::AddressAndPort & GetServer() const { return m_server; }

  protected:
    PIPSocket::AddressAndPort m_server;
    unsigned                  m_facility;
    PUDPSocket                m_socket;
};


#ifdef WIN32
/** Log system output to the Windows OutputDebugString() function.
  */
class PSystemLogToDebug : public PSystemLogTarget
{
    PCLASSINFO(PSystemLogToDebug, PSystemLogTarget);
  public:
  /**@name Overrides of PSystemLogTarget */
  //@{
    /** Log an error into the system log.
     */
    virtual void Output(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    );
  //@}
};
#elif !defined(P_VXWORKS)
#define P_SYSTEMLOG_TO_SYSLOG 1
/** Log system output to the Posix syslog() function.
  */
class PSystemLogToSyslog : public PSystemLogTarget
{
    PCLASSINFO(PSystemLogToSyslog, PSystemLogTarget);
  public:
  /**@name Construction */
  //@{
    PSystemLogToSyslog(
      const char * ident = NULL,  ///< Identification for openlog(), default to PProcess::GetName()
      int priority = -1,          /**< Priority for syslog() call, if < 0, one derived from
                                       PSystemLog::GetThresholdLevel() is used. */
      int options = -1,           ///< Option flags for openlog(), -1 is use default (LOG_PID)
      int facility = -1           ///< Facility codes for openlog(), -1 is use default (LOG_DAEMON)
    );

    ~PSystemLogToSyslog();
  //@}

  /**@name Overrides of PSystemLogTarget */
  //@{
    /** Log an error into the system log.
     */
    virtual void Output(
      PSystemLog::Level level,  ///< Level of this message
      const char * msg          ///< Message to be logged
    );
  //@}

  protected:
    PString m_ident;
    int     m_priority;
};
#endif


/** Log a message to the system log.
The current log level is checked and if allowed, the second argument is evaluated
as a stream output sequence which is them output to the system log.
*/
#define PSYSTEMLOG(level, variables) \
  if (PSystemLog::GetTarget().GetThresholdLevel() >= PSystemLog::level) { \
    PSystemLog P_systemlog(PSystemLog::level); \
    P_systemlog << variables; \
    P_systemlog.width(-12345678); \
  } else (void)0


#endif


#endif //P_SYSTEMLOG

// End Of File ///////////////////////////////////////////////////////////////
