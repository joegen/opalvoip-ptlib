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
 * $Log: pprocess.h,v $
 * Revision 1.35  1998/09/23 06:21:10  robertj
 * Added open source copyright license.
 *
 * Revision 1.34  1998/09/14 12:30:38  robertj
 * Fixed memory leak dump under windows to not include static globals.
 *
 * Revision 1.33  1998/04/07 13:33:53  robertj
 * Changed startup code to support PApplication class.
 *
 * Revision 1.32  1998/04/01 01:56:21  robertj
 * Fixed standard console mode app main() function generation.
 *
 * Revision 1.31  1998/03/29 06:16:44  robertj
 * Rearranged initialisation sequence so PProcess descendent constructors can do "things".
 *
 * Revision 1.30  1998/03/20 03:16:10  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.29  1997/07/08 13:13:46  robertj
 * DLL support.
 *
 * Revision 1.28  1997/04/27 05:50:13  robertj
 * DLL support.
 *
 * Revision 1.27  1997/02/05 11:51:56  robertj
 * Changed current process function to return reference and validate objects descendancy.
 *
 * Revision 1.26  1996/06/28 13:17:08  robertj
 * Fixed incorrect declaration of internal timer list.
 *
 * Revision 1.25  1996/06/13 13:30:49  robertj
 * Rewrite of auto-delete threads, fixes Windows95 total crash.
 *
 * Revision 1.24  1996/05/23 09:58:47  robertj
 * Changed process.h to pprocess.h to avoid name conflict.
 * Added mutex to timer list.
 *
 * Revision 1.23  1996/05/18 09:18:30  robertj
 * Added mutex to timer list.
 *
 * Revision 1.22  1996/04/29 12:18:48  robertj
 * Added function to return process ID.
 *
 * Revision 1.21  1996/03/12 11:30:21  robertj
 * Moved destructor to platform dependent code.
 *
 * Revision 1.20  1996/02/25 11:15:26  robertj
 * Added platform dependent Construct function to PProcess.
 *
 * Revision 1.19  1996/02/03 11:54:09  robertj
 * Added operating system identification functions.
 *
 * Revision 1.18  1996/01/02 11:57:17  robertj
 * Added thread for timers.
 *
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

#include <mutex.h>


/*$MACRO PCREATE_PROCESS(cls)
   This macro is used to create the components necessary for a user PWLib
   process. For a PWLib program to work correctly on all platforms the
   <CODE>main()</CODE> function must be defined in the same module as the
   instance of the application.
 */
#define PCREATE_PROCESS(cls) \
  int main(int argc, char ** argv, char ** envp) \
    { PProcess::PreInitialise(argc, argv, envp); \
      static cls instance; \
      return instance._main(); \
    }

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
  };


PLIST(PInternalTimerList, PTimer);

class PTimerList : PInternalTimerList // Want this to be private
/* This class defines a list of <A>PTimer</A> objects. It is primarily used
   internally by the library and the user should never create an instance of
   it. The <A>PProcess</A> instance for the application maintains an instance
   of all of the timers created so that it may decrements them at regular
   intervals.
 */
{
  PCLASSINFO(PTimerList, PInternalTimerList)

  public:
    PTimerList();
    // Create a new timer list

    PTimeInterval Process();
    /* Decrement all the created timers and dispatch to their callback
       functions if they have expired. The <A>PTimer::Tick()</A> function
       value is used to determine the time elapsed since the last call to
       Process().

       The return value is the number of milliseconds until the next timer
       needs to be despatched. The function need not be called again for this
       amount of time, though it can (and usually is).
       
       <H2>Returns:</H2>
       maximum time interval before function should be called again.
     */

    void AppendTimer(PTimer * timer);
    void RemoveTimer(PTimer * timer);
    // Overrides for mutex

  private:
    PMutex mutex;
    // Mutual exclusion for multi tasking

    PTimeInterval lastSample;
    // The last system timer tick value that was used to process timers.
};


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
    static PProcess & Current();
    /* Get the current processes object instance. The <I>current process</I>
       is the one the application is running in.
       
       <H2>Returns:</H2>
       pointer to current process instance.
     */

    static PString GetOSClass();
    /* Get the class of the operating system the process is running on, eg
       "unix".
       
       <H2>Returns:</H2>
       String for OS class.
     */

    static PString GetOSName();
    /* Get the class of the operating system the process is running on, eg
       "linux".
       
       <H2>Returns:</H2>
       String for OS name.
     */

    static PString GetOSVersion();
    /* Get the version of the operating system the process is running on, eg
       "v1.3pl1".
       
       <H2>Returns:</H2>
       String for OS version.
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

    DWORD GetProcessID() const;
    /* Get the platform dependent process identifier for the process. This is
       an arbitrary (and unique) integer attached to a process by the operating
       system.

       <H2>Returns:</H2>
       Process ID for process.
     */


    PTimerList * GetTimerList();
    /* Get the list of timers handled by the application. This is an internal
       function and should not need to be called by the user.
       
       <H2>Returns:</H2>
       list of timers.
     */

    static void PreInitialise(
      int argc,     // Number of program arguments.
      char ** argv, // Array of strings for program arguments.
      char ** envp  // Array of string for the system environment
    );
    /* Internal initialisation function called directly from
       <CODE>_main()</CODE>. The user should never call this function.
     */

    virtual int _main(void * arg = NULL);
    // Main function for process, called from real main after initialisation

  protected:
#if !defined(P_PLATFORM_HAS_THREADS)

    virtual void OperatingSystemYield();
    /* Yield to the platforms operating system. This is an internal function
       and should never be called by the user. It is provided on platforms
       that do not provide multiple threads in a single process.
     */

#endif

  private:
    void Construct();

  // Member variables
    static int argc;
    static char ** argv;
    static char ** envp;
    // main arguments

#if defined(_DEBUG) && defined(_MSC_VER)
    PMemoryState memState;
#endif

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

#if !defined(P_PLATFORM_HAS_THREADS)

    PThread * currentThread;
    // Currently running thread in the process

#endif


  friend class PThread;


// Class declaration continued in platform specific header file ///////////////
