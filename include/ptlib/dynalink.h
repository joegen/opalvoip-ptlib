/*
 * $Id: dynalink.h,v 1.1 1995/03/14 12:44:08 robertj Exp $
 *
 * Portable Windows Library
 *
 * Dynamic Link Library Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: dynalink.h,v $
 * Revision 1.1  1995/03/14 12:44:08  robertj
 * Initial revision
 *
 */

#define _PDYNALINK

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PDynaLink, PObject)
/* A dynamic link library. This allows the loading at run time of code
   modules for use by an application.
 */

  public:
    PDynaLink();
    PDynaLink(
      const PString & name    // Name of the dynamically loadable module.
    );
    /* Create a new dyna-link, loading the specified module. The first,
       parameterless, form does load a library.
     */

    ~PDynaLink();
    /* Destroy the dyna-link, freeing the module.
     */


  // New functions for class
    BOOL Open(
      const PString & name    // Name of the dynamically loadable module.
    );
    /* Open a new dyna-link, loading the specified module.

       <H2>Returns:</H2>
       TRUE if the library was loaded.
     */

    void Close();
    /* Close the dyna-link library.
     */

    BOOL IsLoaded() const;
    /* Dyna-link module is loaded and may be accessed.
     */


    typedef void (*Function)();
    // Primitive pointer to a function for a dynamic link module.


    BOOL GetFunction(
      PINDEX index,    // Ordinal number of the function to get.
      Function & func  // Refrence to point to function to get.
    );
    BOOL GetFunction(
      const PString & name,  // Name of the function to get.
      Function & func        // Refrence to point to function to get.
    );
    /* Get a pointer to the function in the dynamically loadable module.

       <H2>Returns:</H2>
       TRUE if function was found.
     */


// Class declaration continued in platform specific header file ///////////////
