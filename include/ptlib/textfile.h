/*
 * $Id: textfile.h,v 1.8 1994/08/23 11:32:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: textfile.h,v $
 * Revision 1.8  1994/08/23 11:32:52  robertj
 * Oops
 *
 * Revision 1.7  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.6  1994/04/20  12:17:44  robertj
 * PFilePath addition
 *
 * Revision 1.5  1994/04/01  14:17:26  robertj
 * Fixed container for text file.
 *
 * Revision 1.4  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.3  1993/08/21  01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#define _PTEXTFILE

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// Text Files

PDECLARE_CONTAINER(PTextFile, PFile)

  public:
    PTextFile();
      // Create a text file object but do not open it. It does not initially
      // have a valid file name.

    PTextFile(OpenMode mode, int opts = ModeDefault);
      // Create a unique temporary file name for the text file object, then
      // open it for reading and writing. It is initially empty.
      
    PTextFile(const PFilePath & name,
                           OpenMode mode = ReadWrite, int opts = ModeDefault);
      // Create a new text file object with the specified name and open
      // or create it according to the specified options.


    // New member functions
    BOOL ReadLine(PString & str);
      // Read a line from the text file. What constitutes an end of line in
      // the file is platform dependent. Return TRUE if successful, FALSE if
      // at end of file or other read error. Use the LastError function to
      // determine if there was some error other than end of file.

    BOOL WriteLine(const PString & str);
      // Write a line to the text file. What constitutes an end of line in
      // the file is platform dependent. Return TRUE if successful.


// Class declaration continued in platform specific header file ///////////////
