/*
 * $Id: config.h,v 1.14 1996/02/25 02:50:33 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: config.h,v $
 * Revision 1.14  1996/02/25 02:50:33  robertj
 * Added consts to all GetXxxx functions.
 *
 * Revision 1.13  1996/01/28 14:10:10  robertj
 * Added time functions to PConfig.
 *
 * Revision 1.12  1995/12/10 11:54:30  robertj
 * Added WIN32 registry support for PConfig objects.
 *
 * Revision 1.11  1995/03/14 12:41:12  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.10  1995/01/27  11:06:20  robertj
 * Changed single string default constructor to be section name not file name.
 *
 * Revision 1.9  1994/12/12  10:11:59  robertj
 * Documentation.
 *
 * Revision 1.8  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.7  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.6  1994/08/21  23:43:02  robertj
 * Removed default argument when of PString type (MSC crashes).
 *
 * Revision 1.5  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.4  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.3  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


#define _PCONFIG

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PConfig, PObject)
/* A class representing a configuration for the application. There are four
   sources of configuration information. The system environment, a system wide
   configuration file, an application specific configuration file or an
   explicit configuration file.

   Configuration information follows a three level hierarchy: <I>file</I>,
   <I>section</I> and <I>variable</I>. Thus, a configuration file consists of
   a number of sections each with a number of variables selected by a
   <I>key</I>. Each variable has an associated value.

   Note that the evironment source for configuration information does not have
   sections. The section is ignored and the same set of keys are available.
   
   The configuration file is a standard text file for the platform with its
   internals appearing in the form:
        <PRE><CODE>
        [Section String]
        Key Name=Value String
        </CODE></PRE>
 */

  public:
    enum Source {
      Environment,
      /* The platform specific environment. For Unix, MSDOS, NT etc this is
         <EM>the</EM> environment current when the program was run. For the
         MacOS this is a subset of the Gestalt and SysEnviron information.
       */
      System,
      /* The platform specific system wide configuration file. For MS-Windows
         this is the WIN.INI file. For Unix, plain MS-DOS, etc this is a
         configuration file similar to that for applications except there is
         only a single file that applies to all PWLib applications.
       */
      Application,
      /* The application specific configuration file. This is the most common
         source of configuration for an application. The location of this file
         is platform dependent, but its contents are always the same. For
         MS-Windows the file should be either in the same directory as the
         executable or in the Windows directory. For the MacOS this would be
         either in the System Folder or the Preferences folder within it. For
         Unix this would be the users home directory.
       */
      NumSources
    };
    /* This enum describes the standard source for configuration information.
     */

    PConfig(
      Source src = Application  // Standard source for the configuration.
    );
    PConfig(
      const PString & section,  // Default section to search for variables.
      Source src = Application  // Standard source for the configuration.
    );
    PConfig(
      const PFilePath & filename, // Explicit name of the configuration file.
      const PString & section     // Default section to search for variables.
    );
    /* Create a new configuration object. Once a source is selected for the
       configuration it cannot be changed. Only at the next level of the
       hierarchy (sections) are selection able to be made dynamically with an
       active PConfig object.
     */


    // New functions for class
    void SetDefaultSection(
      const PString & section  // New default section name.
    );
    /* Set the default section for variable operations. All functions that deal
       with keys and get or set configuration values will use this section
       unless an explicit section name is specified.

       Note when the <CODE>Environment</CODE> source is being used the default
       section may be set but it is ignored.
     */

    PString GetDefaultSection() const;
    /* Get the default section for variable operations. All functions that deal
       with keys and get or set configuration values will use this section
       unless an explicit section name is specified.

       Note when the <CODE>Environment</CODE> source is being used the default
       section may be retrieved but it is ignored.

       <H2>Returns:</H2>
       default section name string.
     */

    PStringList GetSections() const;
    /* Get all of the section names currently specified in the file. A section
       is the part specified by the [ and ] characters.

       Note when the <CODE>Environment</CODE> source is being used this will
       return an empty list as there are no section present.

       <H2>Returns:</H2>
       list of all section names.
     */

    PStringList GetKeys() const;
    PStringList GetKeys(
      const PString & section   // Section to use instead of the default.
    ) const;
    /* Get a list of all the keys in the section. If the section name is not
       specified then use the default section.

       <H2>Returns:</H2>
       list of all key names.
     */


    void DeleteSection();
    void DeleteSection(
      const PString & section   // Name of section to delete.
    );
    /* Delete all variables in the specified section. If the section name is
       not specified then the default section is deleted.

       Note that the section header is also removed so the section will not
       appear in the GetSections() function.
     */

    void DeleteKey(
      const PString & key       // Key of the variable to delete.
    );
    void DeleteKey(
      const PString & section,  // Section to use instead of the default.
      const PString & key       // Key of the variable to delete.
    );
    /* Delete the particular variable in the specified section. If the section
       name is not specified then the default section is used.

       Note that the variable and key are removed from the file. The key will
       no longer appear in the GetKeys() function. If you wish to delete the
       value without deleting the key, use SetString() to set it to the empty
       string.
     */


    PString GetString(
      const PString & key       // The key name for the variable.
    ) const;
    PString GetString(
      const PString & key,      // The key name for the variable.
      const PString & dflt      // Default value for the variable.
    ) const;
    PString GetString(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      const PString & dflt      // Default value for the variable.
    ) const;
    /* Get a string variable determined by the key in the section. If the
       section name is not specified then the default section is used.
       
       If the key is not present the value returned is the that provided by
       the <CODE>dlft</CODE> parameter. Note that this is different from the
       key being present but having no value, in which case an empty string is
       returned.

       <H2>Returns:</H2>
       string value of the variable.
     */

    void SetString(
      const PString & key,      // The key name for the variable.
      const PString & value     // New value to set for the variable.
    );
    void SetString(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      const PString & value     // New value to set for the variable.
    );
    /* Set a string variable determined by the key in the section. If the
       section name is not specified then the default section is used.
     */


    BOOL GetBoolean(
      const PString & key,      // The key name for the variable.
      BOOL dflt = FALSE         // Default value for the variable.
    ) const;
    BOOL GetBoolean(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      BOOL dflt = FALSE         // Default value for the variable.
    ) const;
    /* Get a boolean variable determined by the key in the section. If the
       section name is not specified then the default section is used.

       The boolean value can be specified in a number of ways. The TRUE value
       is returned if the string value for the variable begins with either the
       'T' character or the 'Y' character. Alternatively if the string can
       be converted to a numeric value, a non-zero value will also return TRUE.
       Thus the values can be Key=True, Key=Yes or Key=1 for TRUE and
       Key=False, Key=No, or Key=0 for FALSE.

       If the key is not present the value returned is the that provided by
       the <CODE>dlft</CODE> parameter. Note that this is different from the
       key being present but having no value, in which case FALSE is returned.

       <H2>Returns:</H2>
       boolean value of the variable.
     */

    void SetBoolean(
      const PString & key,      // The key name for the variable.
      BOOL value                // New value to set for the variable.
    );
    void SetBoolean(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      BOOL value                // New value to set for the variable.
    );
    /* Set a boolean variable determined by the key in the section. If the
       section name is not specified then the default section is used.

       If value is TRUE then the string "True" is written to the variable
       otherwise the string "False" is set.
     */


    long GetInteger(
      const PString & key,      // The key name for the variable.
      long dflt = 0             // Default value for the variable.
    ) const;
    long GetInteger(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      long dflt = 0             // Default value for the variable.
    ) const;
    /* Get an integer variable determined by the key in the section. If the
       section name is not specified then the default section is used.

       If the key is not present the value returned is the that provided by
       the <CODE>dlft</CODE> parameter. Note that this is different from the
       key being present but having no value, in which case zero is returned.

       <H2>Returns:</H2>
       integer value of the variable.
     */

    void SetInteger(
      const PString & key,      // The key name for the variable.
      long value                // New value to set for the variable.
    );
    void SetInteger(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      long value                // New value to set for the variable.
    );
    /* Set an integer variable determined by the key in the section. If the
       section name is not specified then the default section is used.

       The value is always formatted as a signed number with no leading or
       trailing blanks.
     */


    double GetReal(
      const PString & key,      // The key name for the variable.
      double dflt = 0           // Default value for the variable.
    ) const;
    double GetReal(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      double dflt = 0           // Default value for the variable.
    ) const;
    /* Get a floating point variable determined by the key in the section. If
       the section name is not specified then the default section is used.

       If the key is not present the value returned is the that provided by
       the <CODE>dlft</CODE> parameter. Note that this is different from the
       key being present but having no value, in which case zero is returned.

       <H2>Returns:</H2>
       floating point value of the variable.
     */

    void SetReal(
      const PString & key,      // The key name for the variable.
      double value              // New value to set for the variable.
    );
    void SetReal(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      double value              // New value to set for the variable.
    );
    /* Set a floating point variable determined by the key in the section. If
       the section name is not specified then the default section is used.

       The value is always formatted as a signed decimal or exponential form
       number with no leading or trailing blanks, ie it uses the %g formatter
       from the printf() function.
     */

    PTime GetTime(
      const PString & key       // The key name for the variable.
    ) const;
    PTime GetTime(
      const PString & key,      // The key name for the variable.
      const PTime & dflt        // Default value for the variable.
    ) const;
    PTime GetTime(
      const PString & section,  // Section to use instead of the default.
      const PString & key       // The key name for the variable.
    ) const;
    PTime GetTime(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      const PTime & dflt        // Default value for the variable.
    ) const;
    /* Get a <A>PTime</A> variable determined by the key in the section. If
       the section name is not specified then the default section is used.

       If the key is not present the value returned is the that provided by
       the <CODE>dlft</CODE> parameter. Note that this is different from the
       key being present but having no value, in which case zero is returned.

       <H2>Returns:</H2>
       time/date value of the variable.
     */

    void SetTime(
      const PString & key,      // The key name for the variable.
      const PTime & value       // New value to set for the variable.
    );
    void SetTime(
      const PString & section,  // Section to use instead of the default.
      const PString & key,      // The key name for the variable.
      const PTime & value       // New value to set for the variable.
    );
    /* Set a <A>PTime</A> variable determined by the key in the section. If
       the section name is not specified then the default section is used.
     */


  protected:
    // Member variables
    PString defaultSection;
      // The current section for variable values.


  private:
    void Construct(
      Source src                  // Standard source for the configuration.
    );
    void Construct(
      const PFilePath & filename  // Explicit name of the configuration file.
    );
    // Do common construction code.


// Class declaration continued in platform specific header file ///////////////
