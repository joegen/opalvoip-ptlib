/*
 * $Id: svcproc.h,v 1.1 1995/12/23 03:47:25 robertj Exp $
 *
 * Portable Windows Library
 *
 * Service Process for Windows NT platforms
 *
 * Copyright 1995 Equivalence
 *
 * $Log: svcproc.h,v $
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


PDECLARE_CLASS(PServiceProcess, PProcess)
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


  // New functions for class
    static PServiceProcess * Current();
    /* Get the current service process object.

       <H2>Returns:</H2>
       Pointer to service process.
     */


    enum SystemLogLevel {
      LogFatal,   // Log a fatal error
      LogError,   // Log a non-fatal error
      LogWarning, // Log a warning
      LogInfo,    // Log general debug trace information
      NumLogLevels
    };
    // Type of log message.

    void SystemLog(
      SystemLogLevel level, // Log level for this log message.
      const char * cmsg,    // Message to log.
      ...                   // Optional printf style parameters.
    );
    void SystemLog(
      SystemLogLevel level, // Log level for this log message.
      const PString & msg   // Message to log.
    );
    /* Log an error into the system log.
     */

    void SetLogLevel(
      SystemLogLevel level  // New log level
    ) { currentLogLevel = level; }
    /* Set the level at which errors are logged. Only messages higher than or
       equal to the specified level will be logged.
    
       The default is <CODE>LogError</CODE> allowing fatal errors and ordinary\
       errors to be logged and warning and information to be ignored.

       If in debug mode then the default is <CODE>LogInfo</CODE> allowing all
       messages to be displayed.
     */

    SystemLogLevel GetLogLevel() const { return currentLogLevel; }
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


  protected:
  // Overrides from class PProcess
    virtual int _main(
      int argc,     // Number of program arguments.
      char ** argv, // Array of strings for program arguments.
      char ** envp  // Array of strings for program environment.
    );
    /* Internal initialisation function called directly from
       <CODE>main()</CODE>. The user should never call this function.
     */


  // Member variables
    BOOL           debugMode;
    SystemLogLevel currentLogLevel;


// Class declaration continued in platform specific header file ///////////////
