/*
 * $Id: pprocess.h,v 1.3 1994/06/25 11:55:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.3  1994/06/25 11:55:15  robertj
 * Unix version synchronisation.
 *
 */


#define _PPROCESS


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
      // Destroy the process.

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

    PTimerList * GetTimerList();
      // Get the list of timers handled by the application.


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


// Class declaration continued in platform specific header file ///////////////
