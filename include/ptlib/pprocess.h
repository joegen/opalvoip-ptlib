/*
 * $Id: pprocess.h,v 1.17 1995/12/23 03:46:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.17  1995/12/23 03:46:02  robertj
 * Changed version numbers.
 *
 * Revision 1.16  1995/12/10 11:33:36  robertj
 * Added extra user information to processes and applications.
 * Changes to main() startup mechanism to support Mac.
 *
 * Revision 1.15  1995/06/17 11:13:05  robertj
 * Documentation update.
 *
 * Revision 1.14  1995/06/17 00:43:10  robertj
 * Made PreInitialise virtual for NT service support
 *
 * Revision 1.13  1995/03/14 12:42:14  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.12  1995/03/12  04:43:26  robertj
 * Remvoed redundent destructor.
 *
 * Revision 1.11  1995/01/11  09:45:09  robertj
 * Documentation and normalisation.
 *
 * Revision 1.10  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.9  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.8  1994/08/21  23:43:02  robertj
 * Added function to get the user name of the owner of a process.
 *
 * Revision 1.7  1994/08/04  11:51:04  robertj
 * Moved OperatingSystemYield() to protected for Unix.
 *
 * Revision 1.6  1994/08/01  03:42:23  robertj
 * Destructor needed for heap debugging.
 *
 * Revision 1.5  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.4  1994/07/21  12:33:49  robertj
 * Moved cooperative threads to common.
 *
 * Revision 1.3  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 */


#define _PPROCESS

#ifdef __GNUC__
#pragma interface
#endif

#include <semaphor.h>


/*$MACRO PCREATE_PROCESS(cls)
   This macro is used to create the components necessary for a user PWLib
   process. For a PWLib program to work correctly on all platforms the
   <CODE>main()</CODE> function must be defined in the same module as the
   instance of the application.
 */
#define PCREATE_PROCESS(cls) \
  int main(int argc, char ** argv, char ** envp) \
    { static cls instance; return PProcessInstance->_main(argc, argv, envp); }

/*$MACRO PDECLARE_PROCESS(cls,ancestor,manuf,name,major,minor,status,build)
   This macro is used to declare the components necessary for a user PWLib
   process. This will declare the PProcess descendent class, eg PApplication,
   and create an instance of the class. See the <A>PCREATE_PROCESS</A> macro
   for more details.
 */
#define PDECLARE_PROCESS(cls,ancestor,manuf,name,major,minor,status,build) \
  PDECLARE_CLASS(cls, ancestor) \
    public: \
      cls() : ancestor(manuf, name, major, minor, status, build) { } \
    private: \
      virtual void Main(); \
  }; \
  PCREATE_PROCESS(cls)


///////////////////////////////////////////////////////////////////////////////
// PProcess

PDECLARE_CLASS(PProcess, PThread)
/* This class represents an operating system process. This is a running
   "programme" in the  context of the operating system. Note that there can
   only be one instance of a PProcess class in a given programme.
   
   The instance of a PProcess or its GUI descendent <A>PApplication</A> is
   usually a static variable created by the application writer. This is the
   initial "anchor" point for all data structures in an application. As the
   application writer never needs to access the standard system
   <CODE>main()</CODE> function, it is in the library, the programmes
   execution begins with the virtual function <A>PThread::Main()</A> on a
   process.
 */

  public:
    enum CodeStatus {
      AlphaCode,    // Code is still very much under construction.
      BetaCode,     // Code is largely complete and is under test.
      ReleaseCode,  // Code has all known bugs removed and is shipping.
      NumCodeStatuses
    };
    PProcess(
      const char * manuf = "",         // Name of manufacturer
      const char * name = "",          // Name of product
      WORD majorVersion = 1,           // Major version number of the product
      WORD minorVersion = 0,           // Minor version number of the product
      CodeStatus status = ReleaseCode, // Development status of the product
      WORD buildNumber = 1             // Build number of the product
    );
    // Create a new process instance.

    ~PProcess();
    // Destroy the process


  // Overrides from class PObject
    Comparison Compare(
      const PObject & obj   // Other process to compare against.
    ) const;
    /* Compare two process instances. This should almost never be called as
       a programme only has access to a single process, its own.

       <H2>Returns:</H2>
       <CODE>EqualTo</CODE> if the two process object have the same name.
     */


  // New functions for class
    static PProcess * Current();
    /* Get the current processes object instance. The <I>current process</I>
       is the one the application is running in.
       
       <H2>Returns:</H2>
       pointer to current process instance.
     */

    virtual void Terminate();
    /* Terminate the process. Usually only used in abnormal abort situation.
     */

    void SetTerminationValue(
      int value  // Value to return a process termination status.
    );
    /* Set the termination value for the process.
    
       The termination value is an operating system dependent integer which
       indicates the processes termiantion value. It can be considered a
       "return value" for an entire programme.
     */

    int GetTerminationValue() const;
    /* Get the termination value for the process.
    
       The termination value is an operating system dependent integer which
       indicates the processes termiantion value. It can be considered a
       "return value" for an entire programme.
       
       <H2>Returns:</H2>
       integer termination value.
     */

    PArgList & GetArguments();
    /* Get the programme arguments. Programme arguments are a set of strings
       provided to the programme in a platform dependent manner.
    
       <H2>Returns:</H2>
       argument handling class instance.
     */

    const PString & GetManufacturer() const;
    /* Get the name of the manufacturer of the software. This is used in the
       default "About" dialog box and for determining the location of the
       configuration information as used by the <A>PConfig<A/> class.

       The default for this information is the empty string.
    
       <H2>Returns:</H2>
       string for the manufacturer name eg "Equivalence".
     */

    const PString & GetName() const;
    /* Get the name of the process. This is used in the
       default "About" dialog box and for determining the location of the
       configuration information as used by the <A>PConfig<A/> class.

       The default is the title part of the executable image file.
    
       <H2>Returns:</H2>
       string for the process name eg ".
     */

    PString GetVersion(BOOL full) const;
    /* Get the version of the software. This is used in the default "About"
       dialog box and for determining the location of the configuration
       information as used by the <A>PConfig<A/> class.

       If the <CODE>full</CODE> parameter is TRUE then a version string
       built from the major, minor, status and build veriosn codes is
       returned. If FALSE then only the major and minor versions are
       returned.

       The default for this information is "1.0".
    
       <H2>Returns:</H2>
       string for the version eg "1.0b3".
     */

    const PFilePath & GetFile() const;
    /* Get the processes executable image file path.

       <H2>Returns:</H2>
       file path for program.
     */

    PString GetUserName() const;
    /* Get the effective user name of the owner of the process, eg "root" etc.
       This is a platform dependent string only provided by platforms that are
       multi-user. Note that some value may be returned as a "simulated" user.
       For example, in MS-DOS an environment variable

       <H2>Returns:</H2>
       user name of processes owner.
     */

    PTimerList * GetTimerList();
    /* Get the list of timers handled by the application. This is an internal
       function and should not need to be called by the user.
       
       <H2>Returns:</H2>
       list of timers.
     */

#if defined(P_PLATFORM_HAS_THREADS)

    void SignalTimerChange();
    // Signal to the timer thread that a change was made.

#endif


  protected:
    virtual int _main(
      int argc,     // Number of program arguments.
      char ** argv, // Array of strings for program arguments.
      char ** envp  // Array of strings for program environment.
    );
    /* Internal initialisation function called directly from
       <CODE>main()</CODE>. The user should never call this function.
     */

    void PreInitialise(
      int argc,     // Number of program arguments.
      char ** argv  // Array of strings for program arguments.
    );
    /* Internal initialisation function called directly from
       <CODE>_main()</CODE>. The user should never call this function.
     */

#ifndef P_PLATFORM_HAS_THREADS

    virtual void OperatingSystemYield();
    /* Yield to the platforms operating system. This is an internal function
       and should never be called by the user. It is provided on platforms
       that do not provide multiple threads in a single process.
     */

#endif


  private:
  // Member variables
    int terminationValue;
    // Application return value

    PString manufacturer;
    // Application manufacturer name.

    PString productName;
    // Application executable base name from argv[0]

    WORD majorVersion;
    // Major version number of the product
    
    WORD minorVersion;
    // Minor version number of the product
    
    CodeStatus status;
    // Development status of the product
    
    WORD buildNumber;
    // Build number of the product

    PFilePath executableFile;
    // Application executable file from argv[0] (not open)

    PArgList arguments;
    // The list of arguments

    PTimerList timers;
    // List of active timers in system

#if defined(P_PLATFORM_HAS_THREADS)

    PDECLARE_CLASS(TimerThread, PThread)
      public:
        TimerThread();
        void Main();
        PSemaphore semaphore;
    };
    TimerThread * timerThread;
    // Thread for doing timers.

#else

    PThread * currentThread;
    // Currently running thread in the process

#endif


  friend class PThread;

  friend int main(
      int argc,     // Number of program arguments.
      char ** argv, // Array of strings for program arguments.
      char ** envp  // Array of strings for program environment.
    );
    /* The main() system entry point to programme. This does platform dependent
       initialisation and then calls the <A>PreInitialise()</A> function, then
       the <A>PThread::Main()</A> function.
     */


// Class declaration continued in platform specific header file ///////////////
