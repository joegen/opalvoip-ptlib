/*
 * $Id: sfile.h,v 1.8 1995/01/14 06:19:39 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: sfile.h,v $
 * Revision 1.8  1995/01/14 06:19:39  robertj
 * Documentation
 *
 * Revision 1.7  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.6  1994/08/22  00:46:48  robertj
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

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PStructuredFile, PFile)
/* A class representing a a structured file that is portable accross CPU
   architectures (as in the XDR protocol).
   
   This differs from object serialisation in that the access is always to a
   disk file and is random access. It would primarily be used for database
   type applications.
 */

  public:
    PStructuredFile();
    /* Create a structured file object but do not open it. It does not
       initially have a valid file name. However, an attempt to open the file
       using the $B$Open()$B$ function will generate a unique temporary file.
       
       The initial structure size is one byte.
     */

    PStructuredFile(
      OpenMode mode,          // Mode in which to open the file.
      int opts = ModeDefault  // $H$OpenOptions for open operation.
    );
    /* Create a unique temporary file name, and open the file in the specified
       mode and using the specified options. Note that opening a new, unique,
       temporary file name in ReadOnly mode will always fail. This would only
       be usefull in a mode and options that will create the file.

       The $B$IsOpen()$B$ function may be used after object construction to
       determine if the file was successfully opened.
     */
      
    PStructuredFile(
      const PFilePath & name,     // Name of file to open.
      OpenMode mode = ReadWrite,  // Mode in which to open the file.
      int opts = ModeDefault      // $H$OpenOptions for open operation.
    );
    /* Create a structured file object with the specified name and open it in
       the specified mode and with the specified options.

       The $B$IsOpen()$B$ function may be used after object construction to
       determine if the file was successfully opened.
     */


  // New member functions
    BOOL Read(void * buffer);
    /* Read a sequence of bytes into the specified buffer, translating the
       structure according to the specification made in the
       $B$SetStructure()$B$ function.

       Returns: TRUE if the structure was successfully read.
     */
      
    BOOL Write(void * buffer);
    /* Write a sequence of bytes into the specified buffer, translating the
       structure according to the specification made in the
       $B$SetStructure()$B$ function.

       Returns: TRUE if the structure was successfully written.
     */


  // New member functions
    size_t GetStructureSize();
    /* Get the size of each structure in the file.

       Returns: number of bytes in a structure.
     */

    enum ElementType {
      Character,    // Element is a single character.
      Integer16,    // Element is a 16 bit integer.
      Integer32,    // Element is a 32 bit integer.
      Integer64,    // Element is a 64 bit integer.
      Float32,      // Element is a 32 bit IEE floating point number.
      Float64,      // Element is a 64 bit IEE floating point number.
      Float80,      // Element is a 80 bit IEE floating point number.
      NumElementTypes
    };
    // All element types ina structure

    struct Element {
      ElementType type;   // Type of element in structure.
      PINDEX      count;  // Count of elements of this type.
    };
    // Elements in the structure definition.

    void SetStructure(
      Element * structure,  // Array of structure elements
      PINDEX numElements    // Number of structure elements in structure.
    );
    // Set the structure of each record in the file.


  protected:
  // Member variables
    size_t structureSize;
    // Number of bytes in structure.

    Element * structure;
    PINDEX numElements;


// Class declaration continued in platform specific header file ///////////////
