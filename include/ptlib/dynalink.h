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
 * Revision 1.6  1999/03/09 02:59:49  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.5  1999/02/16 08:07:11  robertj
 * MSVC 6.0 compatibility changes.
 *
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


/**A dynamic link library. This allows the loading at run time of code
   modules for use by an application.
 */
class PDynaLink : public PObject
{
  PCLASSINFO(PDynaLink, PObject);

  public:
  /**@name Construction */
  //@{
    /**Create a new dyna-link, loading the specified module. The first,
       parameterless, form does load a library.
     */
    PDynaLink();
    /**Create a new dyna-link, loading the specified module. The first,
       parameterless, form does load a library.
     */
    PDynaLink(
      const PString & name    /// Name of the dynamically loadable module.
    );

    /**Destroy the dyna-link, freeing the module.
     */
    ~PDynaLink();
  //@}

  /**@name Load/Unload function */
  //@{
    /* Open a new dyna-link, loading the specified module.

       @return
       TRUE if the library was loaded.
     */
    BOOL Open(
      const PString & name    /// Name of the dynamically loadable module.
    );

    /**Close the dyna-link library.
     */
    void Close();

    /**Dyna-link module is loaded and may be accessed.
     */
    BOOL IsLoaded() const;

    /**Get the name of the loaded library. If the library is not loaded
       this may return an empty string.

       If #full# is TRUE then the full pathname of the library
       is returned otherwise only the name part is returned.

       @return
       String for the library name.
     */
    PString GetName(
      BOOL full = FALSE  /// Flag for full or short path name
    ) const;

    /**Get the extension used by this platform for dynamic link libraries.

       @return
       String for file extension.
     */
    static PString GetExtension();
  //@}

  /**@name DLL entry point functions */
  //@{
    /// Primitive pointer to a function for a dynamic link module.
    typedef void (*Function)();


    /**Get a pointer to the function in the dynamically loadable module.

       @return
       TRUE if function was found.
     */
    BOOL GetFunction(
      PINDEX index,    /// Ordinal number of the function to get.
      Function & func  /// Refrence to point to function to get.
    );

    /**Get a pointer to the function in the dynamically loadable module.

       @return
       TRUE if function was found.
     */
    BOOL GetFunction(
      const PString & name,  /// Name of the function to get.
      Function & func        /// Refrence to point to function to get.
    );
  //@}

#ifdef DOC_PLUS_PLUS
};
#endif

// Class declaration continued in platform specific header file ///////////////
