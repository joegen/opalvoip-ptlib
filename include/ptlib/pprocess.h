/*
 * $Id: pprocess.h,v 1.1 1994/04/01 14:25:36 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.1  1994/04/01 14:25:36  robertj
 * Initial revision
 *
 */


#define _PTEXTAPPLICATION


///////////////////////////////////////////////////////////////////////////////
// PTextApplication

PDECLARE_CLASS(PTextApplication, PObject)
  public:
    // PApplication initialisation
    PTextApplication();
      // Create a new application instance.

    ~PTextApplication();
      // Destroy the application

    // Overrides from class PObject
    Comparison Compare(const PObject & obj) const;
      // Return EqualTo if the two application object have the same name.


    // New functions for class
    virtual BOOL Initialise() = 0;
      // User override function to initialise an application.

    virtual void MainBody() = 0;
      // User override function to initialise an application.

    virtual int Termination();
      // User override function for cleaning up on application termination.
      // Most cleaning up would be accommodated by the destruction of all
      // objects in use, but this may sometime be overridden to do additional
      // clean up or returning a programme return code, that may be used by the
      // operating system.

    virtual void Terminate();
      // Terminate the application. Usually only used in abnormal abort
      // situation.

    const PArgList & GetArguments() const;
      // Return the argument list.

    PString GetAppName() const;
      // Return the root name of the applications executable image.

    const PFile & GetAppFile() const;
      // Return the applications executable image file.

    PString GetEnvironment(const PString & varName) const;
      // Return the environment variable.

    void AddTimer(PTimer * timer);
      // Add a timer object to the list of timers handled by the application.

    void RemoveTimer(PTimer * timer);
      // Remvoe a timer from the list of timers handled by the application.


  protected:
    friend int main(int argc, char ** argv);
    int Main(int argc, char ** argv);
      // Internal function called directly from main().

    void PreInitialise(int argc, char ** argv);
      // Internal function called directly from main().


    // Member variables
    int terminationValue;
      // Application return value

    PString applicationName;
      // Application executable base name from argv[0]

    PFile applicationFile;
      // Application executable file from argv[0] (not open)

    PArgList arguments;
      // The list of arguments

    PTimerList timers;
      // List of active timers in system


// Class declaration continued in platform specific header file ///////////////
