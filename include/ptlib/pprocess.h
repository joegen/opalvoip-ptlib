/*
 * $Id: pprocess.h,v 1.11 1995/01/11 09:45:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.11  1995/01/11 09:45:09  robertj
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


///////////////////////////////////////////////////////////////////////////////
// PProcess

PDECLARE_CLASS(PProcess, PThread)
/* This class represents an operating system process. This is a running
   "programme" in the  context of the operating system. Note that there can
   only be one instance of a PProcess class in a given programme.
   
   The instance of a PProcess or it GUI descendent $H$PApplication is usually
   a static variable created by the application writer. This is the initial
   "anchor" point for all data structures in an application. As the application
   writer never needs to access the standard system $B$main()$B$ function, it
   is in the library, the programmes exexcution begins with the virtual
   function $B$Main()$B$ on a process.
 */

  public:
    PProcess();
    // Create a new process instance.

    ~PProcess();
    // Destroy the process instance.


  // Overrides from class PObject
    Comparison Compare(
      const PObject & obj   // Other process to compare against.
    ) const;
    /* Compare two process instances. This should almost never be called as
       a programme only has access to a single process, its own.

       Returns: EqualTo if the two process object have the same name.
     */


  // New functions for class
    static PProcess * Current();
    /* Get the current processes object instance. The $I$current process$I$ is
       the one the application is running in.
       
       Returns: pointer to current process instance.
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
       
       Returns: integer termination value.
     */

    PArgList & GetArguments();
    /* Get the programme arguments. Programme arguments are a set of strings
       provided to the programme in a platform dependent manner.
    
       Returns: argument handling class instance.
     */

    PString GetName() const;
    /* Get the name of the process. This is usually the title part of the
       executable image file.
    
       Returns: string for the process name.
     */

    const PFilePath & GetFile() const;
    /* Get the processes executable image file path.

       Returns: file path for program.
     */

    PString GetUserName() const;
    /* Get the effective user name of the owner of the process, eg "root" etc.
       This is a platform dependent string only provided by platforms that are
       multi-user. Note that some value may be returned as a "simulated" user.
       For example, in MS-DOS an environment variable

       Returns: user name of processes owner.
     */

    PTimerList * GetTimerList();
    /* Get the list of timers handled by the application. This is an internal
       function and should not need to be called by the user.
       
       Returns: list of timers.
     */


#ifndef P_PLATFORM_HAS_THREADS

  protected:
    virtual void OperatingSystemYield();
    /* Yield to the platforms operating system. This is an internal function
       and should never be called by the user. It is provided on platforms
       that do not provide multiple threads in a single process.
     */

#endif


  private:
    friend int main(
      int argc,     // Number of program arguments.
      char ** argv  // Array of strings for program arguments.
    );
    /* The main() system entry point to programme. This does platform dependent
       initialisation and then calls the $B$PreInitialise()$B$ function, then
       the $B$Main()$B$ function.
     */

    void PreInitialise(
      int argc,     // Number of program arguments.
      char ** argv  // Array of strings for program arguments.
    );
    /* Internal initialisation function called directly from $B$main()$B$. The
       user should never call this function.
     */


  // Member variables
    int terminationValue;
    // Application return value

    PString executableName;
    // Application executable base name from argv[0]

    PFilePath executableFile;
    // Application executable file from argv[0] (not open)

    PArgList arguments;
    // The list of arguments

    PTimerList timers;
    // List of active timers in system

#ifndef P_PLATFORM_HAS_THREADS

    PThread * currentThread;
    // Currently running thread in the process

  friend class PThread;

#endif


// Class declaration continued in platform specific header file ///////////////
