/*
 * $Id: pprocess.h,v 1.10 1994/08/23 11:32:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.10  1994/08/23 11:32:52  robertj
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
  // Process object definition. This defines a running "programme" in the 
  // context of the operating system. Note that there can only be one instance
  // of a PProcess class in a programme.

  public:
    PProcess();
      // Create a new process instance.

    ~PProcess();
      // Destroy the process instance.


    // Overrides from class PObject
    Comparison Compare(const PObject & obj) const;
      // Return EqualTo if the two process object have the same name.


    // New functions for class
    static PProcess * Current();
      // Get the current processes object

    virtual void Terminate();
      // Terminate the process. Usually only used in abnormal abort
      // situation.

    void SetTerminationValue(int value);
      // Set the termination value for the process

    int GetTerminationValue() const;
      // Get the termination value for the process

    PArgList & GetArguments();
      // Return the argument list.

    PString GetName() const;
      // Return the root name of the processes executable image.

    const PFilePath & GetFile() const;
      // Return the processes executable image file path.

    PString GetUserName() const;
      // Return the effective user name of the process, eg "root" etc.

    PTimerList * GetTimerList();
      // Get the list of timers handled by the application.


#ifndef P_PLATFORM_HAS_THREADS

  protected:
    virtual void OperatingSystemYield();
      // Yield to the platforms operating system.

#endif


  private:
    friend int main(int argc, char ** argv);
      // The main() system entry point to programme. Calls PreInitialise then
      // the Main() function.

    void PreInitialise(int argc, char ** argv);
      // Internal initialisation function called directly from main().


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
