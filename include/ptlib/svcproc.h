/*
 * svcproc.h
 *
 * Service Process (daemon) class.
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
 * $Log: svcproc.h,v $
 * Revision 1.15  1999/02/16 08:11:17  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.14  1998/10/13 14:06:15  robertj
 * Complete rewrite of memory leak detection code.
 *
 * Revision 1.13  1998/09/23 06:21:31  robertj
 * Added open source copyright license.
 *
 * Revision 1.12  1998/04/07 13:33:21  robertj
 * Changed startup code to support PApplication class.
 *
 * Revision 1.11  1998/03/29 06:16:50  robertj
 * Rearranged initialisation sequence so PProcess descendent constructors can do "things".
 *
 * Revision 1.10  1998/02/16 00:13:16  robertj
 * Added tray icon support.
 *
 * Revision 1.9  1998/02/03 06:19:14  robertj
 * Added extra log levels.
 *
 * Revision 1.8  1997/07/08 13:02:32  robertj
 * DLL support.
 *
 * Revision 1.7  1997/02/05 11:51:15  robertj
 * Changed current process function to return reference and validate objects descendancy.
 *
 * Revision 1.6  1996/08/19 13:39:20  robertj
 * Added "Debug" level to system log.
 * Moved PSYSTEMLOG macro to common code.
 * Changed PSYSTEMLOG macro so does not execute << expression if below debug level.
 * Fixed memory leak in PSystemLog stream buffer.
 *
 * Revision 1.5  1996/08/17 10:00:27  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.4  1996/08/09 11:16:53  robertj
 * Moved log macro to platform dependent header.
 *
 * Revision 1.3  1996/07/30 12:24:13  robertj
 * Added SYSTEMLOG macro for GNU compiler compatibility.
 *
 * Revision 1.2  1996/07/27 04:10:06  robertj
 * Changed SystemLog to be stream based rather than printf based.
 *
 * Revision 1.1  1995/12/23 03:47:25  robertj
 * Initial revision
 *
 * Revision 1.3  1995/12/10 11:50:05  robertj
 * Numerous fixes for WIN32 service processes.
 *
 * Revision 1.2  1995/07/02 01:23:27  robertj
 * Set up service process to be in subthread not main thread.
 *
 * Revision 1.1  1995/06/17 00:50:54  robertj
 * Initial revision
 *
 */

#define _PSERVICEPROCESS

#ifdef __GNUC__
#pragma interface
#endif

class PSystemLog : public PObject, public iostream {
  PCLASSINFO(PSystemLog, PObject)

  public:
    enum Level {
      StdError = -1, // Log from standard error stream
      Fatal,    // Log a fatal error
      Error,    // Log a non-fatal error
      Warning,  // Log a warning
      Info,     // Log general information
      Debug,    // Log debugging information
      Debug2,   // Log more debugging information
      Debug3,   // Log even more debugging information
      NumLogLevels
    };
    // Type of log message.

    PSystemLog(Level level) { logLevel = level; buffer.log = this; init(&buffer); }
    // Create a system log stream

    ~PSystemLog() { flush(); }
    // Destroy the string stream, deleting the stream buffer

    static void Output(
      Level level,      // Log level for this log message.
      const char * msg  // Message to be logged
    );
    /* Log an error into the system log.
     */


  private:
    PSystemLog(const PSystemLog &) { }
    PSystemLog & operator=(const PSystemLog &) { return *this; }

    class Buffer : public streambuf {
      public:
        virtual int overflow(int=EOF);
        virtual int underflow();
        virtual int sync();
        PSystemLog * log;
        PString string;
    } buffer;
    friend class Buffer;

    Level logLevel;
};


#define PSYSTEMLOG(l, v) \
  if (PServiceProcess::Current().GetLogLevel() >= PSystemLog::l) { \
    PSystemLog s(PSystemLog::l); \
    s << v; \
  } else (void)0



class PServiceProcess : public PProcess
{
  PCLASSINFO(PServiceProcess, PProcess)
/* A process type that may be a service under the Windows NT operating system.
 */

  public:
    PServiceProcess(
      const char * manuf,   // Name of manufacturer
      const char * name,    // Name of product
      WORD majorVersion,    // Major version number of the product
      WORD minorVersion,    // Minor version number of the product
      CodeStatus status,    // Development status of the product
      WORD buildNumber      // Build number of the product
    );
    /* Create a new service process.
     */


  // Overrides from class PProcess
    virtual int _main(void * arg = NULL);
    /* Internal initialisation function called directly from
       <CODE>main()</CODE>. The user should never call this function.
     */


  // New functions for class
    static PServiceProcess & Current();
    /* Get the current service process object.

       <H2>Returns:</H2>
       Pointer to service process.
     */


    void SetLogLevel(
      PSystemLog::Level level  // New log level
    ) { currentLogLevel = level; }
    /* Set the level at which errors are logged. Only messages higher than or
       equal to the specified level will be logged.
    
       The default is <CODE>LogError</CODE> allowing fatal errors and ordinary\
       errors to be logged and warning and information to be ignored.

       If in debug mode then the default is <CODE>LogInfo</CODE> allowing all
       messages to be displayed.
     */

    PSystemLog::Level GetLogLevel() const { return currentLogLevel; }
    /* Get the current level for logging.

       <H2>Returns:</H2>
       Log level.
     */

    virtual BOOL OnStart() = 0;
    /* Called when the service is started. This typically initialises the
       service and returns TRUE if the service is ready to run. The
       <CODE>Main()</CODE> function is then executed.

       <H2>Returns:</H2>
       TRUE if service may start, FALSE if an initialisation failure occurred.
     */

    virtual void OnStop();
    /* Called by the system when the service is stopped. One return from this
       function there is no guarentee that any more user code will be executed.
       Any cleaning up or closing of resource must be done in here.
     */

    virtual BOOL OnPause();
    /* Called by the system when the service is to be paused. This will
       suspend any actions that the service may be executing. Usually this is
       less expensive in resource allocation etc than stopping and starting
       the service.

       <H2>Returns:</H2>
       TRUE if the service was successfully paused.
     */

    virtual void OnContinue();
    /* Resume after the service was paused.
     */

    virtual void OnControl() = 0;
    /* The Control menu option was used in the SysTray menu.
     */


  protected:
  // Member variables
    BOOL debugMode;
    PSystemLog::Level currentLogLevel;

    friend void PSystemLog::Output(PSystemLog::Level, const char *);


// Class declaration continued in platform specific header file ///////////////
