/*
 * $Id: sfile.h,v 1.6 1994/08/22 00:46:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: sfile.h,v $
 * Revision 1.6  1994/08/22 00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.5  1994/04/20  12:17:44  robertj
 * PFilePath split
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


#define _PSTRUCTUREDFILE

#ifdef __GNU__
#pragma interface
#endif


PDECLARE_CLASS(PStructuredFile, PFile)
  // A class representing a a structured file that is portable accross CPU
  // architectures (as in the XDR protocol).

  public:
    PStructuredFile();
      // Create a structured file object but do not open it. It does not
      // initially have a valid file name.

    PStructuredFile(OpenMode mode, int opts = ModeDefault);
      // Create a unique temporary file name for the structured file object,
      // then open it for reading and writing. It is initially empty.
      
    PStructuredFile(const PFilePath & name,
                           OpenMode mode = ReadWrite, int opts = ModeDefault);
      // Create a new structured file object with the specified name and open
      // or create it according to the specified options.


    // New member functions
    BOOL Read(void * buffer);
      // Read a sequence of bytes into the specified buffer. Return TRUE if
      // the required number of bytes was successfully read.
      
    BOOL Write(void * buffer);
      // Write a sequence of bytes into the specified buffer. Return TRUE if
      // the required number of bytes was successfully written.


    // New member functions
    size_t GetStructureSize();
      // Get the size of each structure in the file.

    void SetStructureSize(size_t newSize);
      // Set the size of each structure in the file.


  protected:
    // Member variables
    size_t structureSize;


// Class declaration continued in platform specific header file ///////////////
