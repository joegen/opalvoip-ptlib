/*
 * $Id: pipechan.h,v 1.4 1994/09/25 10:43:19 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.h,v $
 * Revision 1.4  1994/09/25 10:43:19  robertj
 * Added more implementation.
 *
 * Revision 1.3  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.2  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.1  1994/04/20  12:17:44  robertj
 * Initial revision
 *
 */


#define _PPIPECHANNEL

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CONTAINER(PPipeChannel, PChannel)
  // A channel that uses a operating system pipe between the current process
  // and a sub-process. On platforms that support multi-processing, the sub-
  // program is executed concurrently with the calling process. Where full
  // multi-prcessing is not supported then the sub-program is run with its
  // input supplied from, or output captured to, a disk file. The current
  // process is then suspended during the execution of the sub-program. In the
  // latter case the semantics of the Close() function change from the usual
  // for channels.

  public:
    enum OpenMode {
      ReadOnly,
      WriteOnly,
      ReadWrite
    };

    PPipeChannel(const PString & subProgram,
                 const char * const * arguments = NULL,
                 OpenMode mode = ReadOnly,
                 BOOL searchPath = TRUE);
    PPipeChannel(const PString & subProgram,
                 const PStringList & arguments,
                 OpenMode mode = ReadOnly,
                 BOOL searchPath = TRUE);
      // Create a new pipe channel allowing the subProgram to be executed and
      // data transferred from its stdin/stdout. If the mode is ReadOnly then
      // the stdout of the sub-program is supplied via the Read() calls of the
      // PPipeChannel. The sub-programs input is set to the platforms NULL
      // device. If mode is WriteOnly then Write() calls of the PPipeChannel
      // are suppied to the sub-programs stdin and its stdout is sent to the
      // NULL device. If mode is ReadWrite then both actions occur. The
      // searchPath parameter indicates that the system PATH for executables
      // should be searched for the sub-program.
      //
      // Note that for platforms that do not support multi-processing, the
      // current process is suspended until the sub-program terminates. The
      // input and output of the sub-program is transferred via a file. The
      // exact moment of execution of the sub-program depnds on the mode. If
      // mode is ReadOnly then it is executed immediately. In the other modes
      // the sub-program is run when the Execute() function is called
      // indicating that the output from the current process to the sub-program
      // has completed.


    // Overrides from class PObject
    Comparison Compare(const PObject & obj) const;
      // Return TRUE if the two objects refer to the same file. This is
      // essentially if they have the same full path name.


    // Overrides from class PChannel
    virtual PString GetName() const;
      // Return the name of the channel.

    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    virtual BOOL Close();
      // Close the channel. This will kill the sub-program (on platforms where
      // that is relevent).


    // New member functions
    const PFilePath & GetSubProgram() const;
      // Return the file path for the subprogram.

    BOOL Execute();
      // Start execution of sub-program for platforms that do not support
      // multi-processing, this will actually run the sub-program passing all
      // data written to the PPipeChannel. For other platforms this will do
      // nothing as the sub-program is run immediately and concurrently.

    static BOOL CanReadAndWrite();
      // Return TRUE if the platform can support simultaneous read and writes
      // from the PPipeChannel (eg MSDOS returns FALSE, Unix returns TRUE).


  protected:
    // Member variables
    PFilePath subProgName;
      // The fully qualified path name for the sub-program executable.


  private:
    void Construct(const PString & subProgram,
               const char * const * arguments, OpenMode mode, BOOL searchPath);


// Class declaration continued in platform specific header file ///////////////
