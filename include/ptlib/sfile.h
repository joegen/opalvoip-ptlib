/*
 * $Id
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log
 */


#define _PSTRUCTUREDFILE


DECLARE_CLASS(PStructuredFile, PFile)
  // A class representing a a structured file that is portable accross CPU
  // architectures (as in the XDR protocol).

  public:
    PStructuredFile();
      // Create a unique temporary file name for the structured file object. Do
      // not open or create it.
      
    PStructuredFile(const PString & name);
      // Create a new structured file object with the specified name but do not
      // open or create it.

    PStructuredFile(const PString & name, OpenMode mode, int opts = Normal);
      // Create a new structured file object with the specified name and open
      // or create it according to the specified options.


    // Overrides from class PObject
    PObject * Clone() const;
      // Make a copy of the file object. Note that this does not actually make
      // a copy of the file on the disk.


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
