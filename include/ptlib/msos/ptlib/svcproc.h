/*
 * $Id: svcproc.h,v 1.1 1995/06/17 00:50:54 robertj Exp $
 *
 * Portable Windows Library
 *
 * Service Process for Windows NT platforms
 *
 * Copyright 1995 Equivalence
 *
 * $Log: svcproc.h,v $
 * Revision 1.1  1995/06/17 00:50:54  robertj
 * Initial revision
 *
 */

PDECLARE_CLASS(PServiceProcess, PProcess)
/* A process type that may be a service under the Windows NT operating system.
 */

  public:
    PServiceProcess();
    /* Create a new service process.
     */


  // New functions for class
    static PServiceProcess * Current();
    /* Get the current service process object.

       <H2>Returns:</H2>
       Pointer to service process.
     */


    virtual LPTSTR GetServiceName() const = 0;
    /* Get the name of the service. This must be supplied by the applications
       derived class.

       <H2>Returns:</H2>
       Pointer to service name.
     */

    enum SystemLogLevel {
      LogFatal,   // Log a fatal error
      LogError,   // Log a non-fatal error
      LogWarning, // Log a warning
      LogInfo     // Log general debug trace information
    };
    // Type of log message.

    void SystemLog(
      SystemLogLevel level, // Log level for this log message.
      const PString & msg   // Message to log.
    );
    /* Log an error into the system log.
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
    virtual void PreInitialise(
      int argc,     // Number of program arguments.
      char ** argv  // Array of strings for program arguments.
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

    SERVICE_STATUS        status;
    SERVICE_STATUS_HANDLE statusHandle;


  private:
    static void __stdcall MainEntry(DWORD argc, LPTSTR * argv);
    /* This function takes care of actually starting the service, informing the
       service controller at each step along the way. After launching the worker
       thread, it waits on the event that the worker thread will signal at its
       termination.
    */

    void BoundMainEntry();
    /* Internal function called directly from <CODE>MainEntry()</CODE>. The
       user should never call this function.
     */

    static void __stdcall ControlEntry(DWORD code);
    /* This function is called by the Service Controller whenever someone calls
       ControlService in reference to our service.
     */
};


#if defined(_WIN32) || !defined(_WINDLL)

inline PServiceProcess::PServiceProcess()
  { }

inline PServiceProcess * PServiceProcess::Current()
  { return (PServiceProcess *)PProcessInstance; }

#endif
