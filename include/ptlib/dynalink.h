/*
 * dynalink.h
 *
 * Dynamic Link Library abstraction class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: dynalink.h,v $
 * Revision 1.4  1998/09/23 06:20:29  robertj
 * Added open source copyright license.
 *
 * Revision 1.3  1997/06/16 13:15:52  robertj
 * Added function to get a dyna-link libraries name.
 *
 * Revision 1.2  1997/06/08 04:49:20  robertj
 * Added DLL file extension string function.
 *
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
    static PString GetExtension();
    /* Get the extension used by this platform for dynamic link libraries.

       <H2>Returns:</H2>
       String for file extension.
     */

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

    PString GetName(
      BOOL full = FALSE  // Flag for full or short path name
    ) const;
    /* Get the name of the loaded library. If the library is not loaded
       this may return an empty string.

       If <CODE>full</CODE> is TRUE then the full pathname of the library
       is returned otherwise only the name part is returned.

       <H2>Returns:</H2>
       String for the library name.
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
