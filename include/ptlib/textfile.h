/*
 * $Id: textfile.h,v 1.3 1993/08/21 01:50:33 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: textfile.h,v $
 * Revision 1.3  1993/08/21 01:50:33  robertj
 * Made Clone() function optional, default will assert if called.
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */

#define _PTEXTFILE


///////////////////////////////////////////////////////////////////////////////
// Text Files

DECLARE_CLASS(PTextFile, PFile)

  public:
    PTextFile();
      // Create a unique temporary file name for the text file object. Do not
      // open or create it.
      
    PTextFile(const PString & name);
      // Create a new text file object with the specified name but do not open
      // or create it.

    PTextFile(const PString & name, OpenMode mode, int opts = Normal);
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
