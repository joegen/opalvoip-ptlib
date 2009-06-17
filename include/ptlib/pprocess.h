/*
 * pprocess.h
 *
 * Operating System Process (running program executable) class.
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

#ifndef _PPROCESS
#define _PPROCESS

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/mutex.h>
#include <ptlib/syncpoint.h>
#include <ptlib/thread.h>
#include <ptlib/pfactory.h>

#include <queue>

/**Create a process.
   This macro is used to create the components necessary for a user PWLib
   process. For a PWLib program to work correctly on all platforms the
   #main()# function must be defined in the same module as the
   instance of the application.
 */
#ifdef P_VXWORKS
#define PCREATE_PROCESS(cls) \
  PProcess::PreInitialise(0, NULL, NULL); \
  cls instance; \
  instance._main();
#elif defined(P_RTEMS)
#define PCREATE_PROCESS(cls) \
extern "C" {\
   void* POSIX_Init( void* argument) \
     { PProcess::PreInitialise(0, 0, 0); \
       static cls instance; \
       exit( instance._main() ); \
     } \
}
#elif defined(_WIN32_WCE)
#define PCREATE_PROCESS(cls) \
  int WinMain(HINSTANCE, HINSTANCE, LPWSTR, int) \
    { cls *pInstance = new cls(); \
      int terminationValue = pInstance->_main(); \
      delete pInstance; \
      return terminationValue; \
    }
#else
#define PCREATE_PROCESS(cls) \
  int main(int argc, char ** argv, char ** envp) \
    { PProcess::PreInitialise(argc, argv, envp); \
      cls *pInstance = new cls(); \
      int terminationValue = pInstance->_main(); \
      delete pInstance; \
      return terminationValue; \
    }
#endif // P_VXWORKS

/*$MACRO PDECLARE_PROCESS(cls,ancestor,manuf,name,major,minor,status,build)
   This macro is used to declare the components necessary for a user PWLib
   process. This will declare the PProcess descendent class, eg PApplication,
   and create an instance of the class. See the #PCREATE_PROCESS# macro
   for more details.
 */
#define PDECLARE_PROCESS(cls,ancestor,manuf,name,major,minor,status,build) \
  class cls : public ancestor { \
    PCLASSINFO(cls, ancestor); \
    public: \
      cls() : ancestor(manuf, name, major, minor, status, build) { } \
    private: \
      virtual void Main(); \
  };


class PTimerList : public PObject
/* This class defines a list of #PTimer# objects. It is primarily used
   internally by the library and the user should never create an instance of
   it. The #PProcess# instance for the application maintains an instance
   of all of the timers created so that it may decrements them at regular
   intervals.
 */
{
  PCLASSINFO(PTimerList, PObject);

  public:
    // Create a new timer list
    PTimerList();

    /* Decrement all the created timers and dispatch to their callback
       functions if they have expired. The #PTimer::Tick()# function
       value is used to determine the time elapsed since the last call to
       Process().

       The return value is the number of milliseconds until the next timer
       needs to be despatched. The function need not be called again for this
       amount of time, though it can (and usually is).
       
       @return
       maximum time interval before function should be called again.
     */
    PTimeInterval Process();

    PTimer::IDType GetNewTimerId() const { return ++timerId; }

    class RequestType {
      public:
        enum Action {
          Stop,
          Start
        } action;

        RequestType(Action _action, PTimer * _timer) : action(_action), timer(_timer), id(timer->GetTimerId()), sync(NULL) { }

        PTimer * timer;
        PTimer::IDType id;
        PSyncPoint * sync;
    };

    typedef std::queue<RequestType> RequestQueueType;
    typedef std::vector<RequestType> RequestListType;

    void QueueRequest(RequestType::Action action, PTimer * timer, bool _isSync = true);

  private:
    mutable PAtomicInteger timerId; 

    // map used to store timer information
    PMutex timerListMutex;
    struct TimerInfoType {
      TimerInfoType(PTimer * _timer) : timer(_timer)  { removed = false; }
      PTimer * timer;
      bool removed;
    };
    typedef std::map<PTimer::IDType, TimerInfoType> TimerInfoMapType;
    TimerInfoMapType activeTimers;
    PThread * timerThread;

    // queue of timer action requests
    PMutex queueMutex;
    RequestQueueType requestQueue;
    RequestListType addQueue;

    // The last system timer tick value that was used to process timers.
    PTimeInterval lastSample;
};


///////////////////////////////////////////////////////////////////////////////
// PProcess

/**This class represents an operating system process. This is a running
   "programme" in the  context of the operating system. Note that there can
   only be one instance of a PProcess class in a given programme.
   
   The instance of a PProcess or its GUI descendent #PApplication# is
   usually a static variable created by the application writer. This is the
   initial "anchor" point for all data structures in an application. As the
   application writer never needs to access the standard system
   #main()# function, it is in the library, the programmes
   execution begins with the virtual function #PThread::Main()# on a
   process.
 */
class PProcess : public PThread
{
  PCLASSINFO(PProcess, PThread);

  public:
  /**@name Construction */
  //@{
    /// Release status for the program.
    enum CodeStatus {
      /// Code is still very much under construction.
      AlphaCode,    
      /// Code is largely complete and is under test.
      BetaCode,     
      /// Code has all known bugs removed and is shipping.
      ReleaseCode,  
      NumCodeStatuses
    };

    /** Create a new process instance.
     */
    PProcess(
      const char * manuf = "",         ///< Name of manufacturer
      const char * name = "",          ///< Name of product
      WORD majorVersion = 1,           ///< Major version number of the product
      WORD minorVersion = 0,           ///< Minor version number of the product
      CodeStatus status = ReleaseCode, ///< Development status of the product
      WORD buildNumber = 1             ///< Build number of the product
    );
  //@}

  /**@name Overrides from class PObject */
  //@{
    /**Compare two process instances. This should almost never be called as
       a programme only has access to a single process, its own.

       @return
       #EqualTo# if the two process object have the same name.
     */
    Comparison Compare(
      const PObject & obj   ///< Other process to compare against.
    ) const;
  //@}

  /**@name Overrides from class PThread */
  //@{
    /**Terminate the process. Usually only used in abnormal abort situation.
     */
    virtual void Terminate();

    /** Get the name of the thread. Thread names are a optional debugging aid.

       @return
       current thread name.
     */
    virtual PString GetThreadName() const;

    /** Change the name of the thread. Thread names are a optional debugging aid.

       @return
       current thread name.
     */
    virtual void SetThreadName(
      const PString & name        ///< New name for the thread.
    );
  //@}

  /**@name Process information functions */
  //@{
    /**Get the current processes object instance. The {\it current process}
       is the one the application is running in.
       
       @return
       pointer to current process instance.
     */
    static PProcess & Current();

    /**Callback for when a thread is started by the PTLib system. Note this is
       called in the context of the new thread.
      */
    virtual void OnThreadStart(
      PThread & thread
    );

    /**Callback for when a thread is ended if wqas started in the PTLib system.
       Note this is called in the context of the old thread.
      */
    virtual void OnThreadEnded(
      PThread & thread
    );

    /**Determine if the current processes object instance has been initialised.
       If this returns PTrue it is safe to use the PProcess::Current() function.
       
       @return
       PTrue if process class has been initialised.
     */
    static PBoolean IsInitialised();

    /**Set the termination value for the process.
    
       The termination value is an operating system dependent integer which
       indicates the processes termiantion value. It can be considered a
       "return value" for an entire programme.
     */
    void SetTerminationValue(
      int value  ///< Value to return a process termination status.
    );

    /**Get the termination value for the process.
    
       The termination value is an operating system dependent integer which
       indicates the processes termiantion value. It can be considered a
       "return value" for an entire programme.
       
       @return
       integer termination value.
     */
    int GetTerminationValue() const;

    /**Get the programme arguments. Programme arguments are a set of strings
       provided to the programme in a platform dependent manner.
    
       @return
       argument handling class instance.
     */
    PArgList & GetArguments();

    /**Get the name of the manufacturer of the software. This is used in the
       default "About" dialog box and for determining the location of the
       configuration information as used by the #PConfig# class.

       The default for this information is the empty string.
    
       @return
       string for the manufacturer name eg "Equivalence".
     */
    virtual const PString & GetManufacturer() const;

    /**Get the name of the process. This is used in the
       default "About" dialog box and for determining the location of the
       configuration information as used by the #PConfig# class.

       The default is the title part of the executable image file.

       @return
       string for the process name eg "MyApp".
     */
    virtual const PString & GetName() const;

    /**Get the version of the software. This is used in the default "About"
       dialog box and for determining the location of the configuration
       information as used by the #PConfig# class.

       If the #full# parameter is PTrue then a version string
       built from the major, minor, status and build veriosn codes is
       returned. If PFalse then only the major and minor versions are
       returned.

       The default for this information is "1.0".
    
       @return
       string for the version eg "1.0b3".
     */
    virtual PString GetVersion(
      PBoolean full = PTrue ///< PTrue for full version, PFalse for short version.
    ) const;

    /**Get the processes executable image file path.

       @return
       file path for program.
     */
    const PFilePath & GetFile() const;

    /**Get the platform dependent process identifier for the process. This is
       an arbitrary (and unique) integer attached to a process by the operating
       system.

       @return
       Process ID for process.
     */
    DWORD GetProcessID() const;

    /**Return the time at which the program was started 
    */
    PTime GetStartTime() const;

    /**Get the effective user name of the owner of the process, eg "root" etc.
       This is a platform dependent string only provided by platforms that are
       multi-user. Note that some value may be returned as a "simulated" user.
       For example, in MS-DOS an environment variable

       @return
       user name of processes owner.
     */
    PString GetUserName() const;

    /**Set the effective owner of the process.
       This is a platform dependent string only provided by platforms that are
       multi-user.

       For unix systems if the username may consist exclusively of digits and
       there is no actual username consisting of that string then the numeric
       uid value is used. For example "0" is the superuser. For the rare
       occassions where the users name is the same as their uid, if the
       username field starts with a '#' then the numeric form is forced.

       If an empty string is provided then original user that executed the
       process in the first place (the real user) is set as the effective user.

       The permanent flag indicates that the user will not be able to simple
       change back to the original user as indicated above, ie for unix
       systems setuid() is used instead of seteuid(). This is not necessarily
       meaningful for all platforms.

       @return
       PTrue if processes owner changed. The most common reason for failure is
       that the process does not have the privilege to change the effective user.
      */
    PBoolean SetUserName(
      const PString & username, ///< New user name or uid
      PBoolean permanent = PFalse    ///< Flag for if effective or real user
    );

    /**Get the effective group name of the owner of the process, eg "root" etc.
       This is a platform dependent string only provided by platforms that are
       multi-user. Note that some value may be returned as a "simulated" user.
       For example, in MS-DOS an environment variable

       @return
       group name of processes owner.
     */
    PString GetGroupName() const;

    /**Set the effective group of the process.
       This is a platform dependent string only provided by platforms that are
       multi-user.

       For unix systems if the groupname may consist exclusively of digits and
       there is no actual groupname consisting of that string then the numeric
       uid value is used. For example "0" is the superuser. For the rare
       occassions where the groups name is the same as their uid, if the
       groupname field starts with a '#' then the numeric form is forced.

       If an empty string is provided then original group that executed the
       process in the first place (the real group) is set as the effective
       group.

       The permanent flag indicates that the group will not be able to simply
       change back to the original group as indicated above, ie for unix
       systems setgid() is used instead of setegid(). This is not necessarily
       meaningful for all platforms.

       @return
       PTrue if processes group changed. The most common reason for failure is
       that the process does not have the privilege to change the effective
       group.
      */
    PBoolean SetGroupName(
      const PString & groupname, ///< New group name or gid
      PBoolean permanent = PFalse     ///< Flag for if effective or real group
    );

    /**Get the maximum file handle value for the process.
       For some platforms this is meaningless.

       @return
       user name of processes owner.
     */
    int GetMaxHandles() const;

    /**Set the maximum number of file handles for the process.
       For unix systems the user must be run with the approriate privileges
       before this function can set the value above the system limit.

       For some platforms this is meaningless.

       @return
       PTrue if successfully set the maximum file hadles.
      */
    PBoolean SetMaxHandles(
      int newLimit  ///< New limit on file handles
    );

#ifdef P_CONFIG_FILE
    /**Get the default file to use in PConfig instances.
      */
    virtual PString GetConfigurationFile();
#endif

    /**Set the default file or set of directories to search for use in PConfig.
       To find the .ini file for use in the default PConfig() instance, this
       explicit filename is used, or if it is a set of directories separated
       by either ':' or ';' characters, then the application base name postfixed
       with ".ini" is searched for through those directories.

       The search is actually done when the GetConfigurationFile() is called,
       this function only sets the internal variable.

       Note for Windows, a path beginning with "HKEY_LOCAL_MACHINE\\" or
       "HKEY_CURRENT_USER\\" will actually search teh system registry for the
       application base name only (no ".ini") in that folder of the registry.
      */
    void SetConfigurationPath(
      const PString & path   ///< Explicit file or set of directories
    );
  //@}

  /**@name Operating System information functions */
  //@{
    /**Get the class of the operating system the process is running on, eg
       "unix".
       
       @return
       String for OS class.
     */
    static PString GetOSClass();

    /**Get the name of the operating system the process is running on, eg
       "Linux".
       
       @return
       String for OS name.
     */
    static PString GetOSName();

    /**Get the hardware the process is running on, eg "sparc".
       
       @return
       String for OS name.
     */
    static PString GetOSHardware();

    /**Get the version of the operating system the process is running on, eg
       "2.0.33".
       
       @return
       String for OS version.
     */
    static PString GetOSVersion();

    /**Get the configuration directory of the operating system the process is
       running on, eg "/etc" for Unix, "c:\windows" for Win95 or
       "c:\winnt\system32\drivers\etc" for NT.

       @return
       Directory for OS configuration files.
     */
    static PDirectory GetOSConfigDir();

    /**Get the version of the PTLib the process is running on, eg
       "2.0.33".
       
       @return
       String for PTLib version.
     */
    static PString GetLibraryVersion();
  //@}

    /**Get the list of timers handled by the application. This is an internal
       function and should not need to be called by the user.
       
       @return
       list of timers.
     */
    PTimerList * GetTimerList();

    /**Internal initialisation function called directly from
       #_main()#. The user should never call this function.
     */
    static void PreInitialise(
      int argc,     // Number of program arguments.
      char ** argv, // Array of strings for program arguments.
      char ** envp  // Array of string for the system environment
    );

    /**Internal shutdown function called directly from the ~PProcess
       #_main()#. The user should never call this function.
     */
    static void PreShutdown();
    static void PostShutdown();

    /// Main function for process, called from real main after initialisation
    virtual int _main(void * arg = NULL);

    /**@name Operating System URL manager functions */
    /**
        This class can be used to register various URL types with the host operating system
        so that URLs will automatically launch the correct application.

        The simplest way to use these functions is to add the code similar to the following
        to the Main function of the PProcess descendant

            PString urlTypes("sip\nh323\nsips\nh323s");
            if (!PProcess::HostSystemURLHandlerInfo::RegisterTypes(urlTypes, false))
              PProcess::HostSystemURLHandlerInfo::RegisterTypes(urlTypes, true);

        This will check to see if the URL types sip, h323, sips and h323s are registered
        with the operating system to launch the current application. If they are not, it
        will rewrite the system configuraton so that they will.

        For more information on the Windows implementation, see the following link:

              http://msdn2.microsoft.com/en-us/library/aa767914.aspx
      */
    //@{
    class HostSystemURLHandlerInfo 
    {
      public:
        HostSystemURLHandlerInfo()
        { }

        HostSystemURLHandlerInfo(const PString & _type)
          : type(_type)
        { }

        static bool RegisterTypes(const PString & _types, bool force = true);

        void SetIcon(const PString & icon);
        PString GetIcon() const;

        void SetCommand(const PString & key, const PString & command);
        PString GetCommand(const PString & key) const;

        bool GetFromSystem();
        bool CheckIfRegistered();

        bool Register();

        PString type;

    #if _WIN32
        PString iconFileName;
        PStringToString cmds;
    #endif
    };
  //@}

  private:
    void Construct();

  // Member variables
    static int p_argc;
    static char ** p_argv;
    static char ** p_envp;
    // main arguments

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

    PStringArray configurationPaths;
    // Explicit file or set of directories to find default PConfig

    PArgList arguments;
    // The list of arguments

    PTimerList timers;
    // List of active timers in system

    PTime programStartTime;
    // time at which process was intantiated, i.e. started

    int maxHandles;
    // Maximum number of file handles process can open.


  friend class PThread;


// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/pprocess.h"
#else
#include "unix/ptlib/pprocess.h"
#endif
};

/*
 *  one instance of this class (or any descendants) will be instantiated
 *  via PGenericFactory<PProessStartup> one "main" has been started, and then
 *  the OnStartup() function will be called. The OnShutdown function will
 *  be called after main exits, and the instances will be destroyed if they
 *  are not singletons
 */
class PProcessStartup : public PObject
{
  PCLASSINFO(PProcessStartup, PObject)
  public:
    virtual void OnStartup()  { }
    virtual void OnShutdown() { }
};

typedef PFactory<PProcessStartup> PProcessStartupFactory;

#if PTRACING

// using an inline definition rather than a #define crashes gcc 2.95. Go figure
#define P_DEFAULT_TRACE_OPTIONS ( PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine )

template <unsigned _level, unsigned _options = P_DEFAULT_TRACE_OPTIONS >
class PTraceLevelSetStartup : public PProcessStartup
{
  public:
    void OnStartup()
    { PTrace::Initialise(_level, NULL, _options); }
};

#endif // PTRACING

#endif

// End Of File ///////////////////////////////////////////////////////////////
