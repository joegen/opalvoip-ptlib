/*
 * $Id
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log
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


    // Overrides from class PObject
    PObject * Clone() const;
      // Make a copy of the file object. Note that this does not actually make
      // a copy of the file on the disk.


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
