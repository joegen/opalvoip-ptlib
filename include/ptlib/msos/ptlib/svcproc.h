/*
 * $Id: svcproc.h,v 1.3 1995/12/10 11:50:05 robertj Exp $
 *
 * Portable Windows Library
 *
 * Service Process for Windows NT platforms
 *
 * Copyright 1995 Equivalence
 *
 * $Log: svcproc.h,v $
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

PDECLARE_CLASS(PServiceProcess, PProcess)
/* A process type that may be a service under the Windows NT operating system.
 */

  public:
    PServiceProcess(
      const char * manuf,   // Name of manufacturer
      const char * name,    // Name of product
      const char * ver      // Version of the product
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


    BOOL ReportStatus(
      DWORD dwCurrentState,
      DWORD dwWin32ExitCode = NO_ERROR,
      DWORD dwCheckPoint = 0,
      DWORD dwWaitHint = 0
    );
    /* This function is called by the Main() and Control() functions to update the
       service's status to the service control manager.
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


  // New functions for class
    virtual void OnPause();
    virtual void OnContinue();
    virtual void OnStop();
    virtual void OnInterrogate();


  // Member variables
    BOOL                  debugMode;
    SystemLogLevel        currentLogLevel;

    SERVICE_STATUS        status;
    SERVICE_STATUS_HANDLE statusHandle;
    HANDLE                terminationEvent;


  private:
    void BeginService();
    /* Internal function function that takes care of actually starting the
       service, informing the service controller at each step along the way.
       After launching the worker thread, it waits on the event that the worker
       thread will signal at its termination.

       The user should never call this function.
    */

    static void __stdcall MainEntry(DWORD argc, LPTSTR * argv);
    /* Internal function called from the Service Manager. This simply calls the
       <A>MainEntry()</A> function on the PServiceProcess instance.

       The user should never call this function.
    */

    static DWORD EXPORTED ThreadEntry(LPVOID);
    /* Internal function called to begin the work of the service process. This
       essentially just calls the <A>Main()</A> function on the
       PServiceProcess instance.

       The user should never call this function.
    */

    static void __stdcall ControlEntry(DWORD code);
    /* This function is called by the Service Controller whenever someone calls
       ControlService in reference to our service.
     */

    enum ProcessCommandResult {
      DebugCommandMode, ProcessCommandError, CommandProcessed
    };
    ProcessCommandResult ProcessCommand(const char * cmd);
    // Process command line argument for controlling the service.


  friend void PAssertFunc(const char * file, int line, const char * msg);
};


#if defined(_WIN32) || !defined(_WINDLL)

inline PServiceProcess * PServiceProcess::Current()
  { return (PServiceProcess *)PProcessInstance; }

#endif
