/*
 * args.h
 *
 * Program Argument Parsing class
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
 * $Log: args.h,v $
 * Revision 1.15  1998/10/30 05:24:29  robertj
 * Added return value to << and >> operators for shifting arguments.
 *
 * Revision 1.14  1998/10/29 05:35:14  robertj
 * Fixed porblem with GetCount() == 0 if do not call Parse() function.
 *
 * Revision 1.13  1998/10/28 03:26:41  robertj
 * Added multi character arguments (-abc style) and options precede parameters mode.
 *
 * Revision 1.12  1998/10/28 00:59:46  robertj
 * New improved argument parsing.
 *
 * Revision 1.11  1998/09/23 06:20:14  robertj
 * Added open source copyright license.
 *
 * Revision 1.10  1995/12/10 11:26:38  robertj
 * Fixed signed/unsigned bug in shift count.
 *
 * Revision 1.9  1995/06/17 11:12:17  robertj
 * Documentation update.
 *
 * Revision 1.8  1995/03/14 12:40:58  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.7  1994/12/05  11:15:13  robertj
 * Documentation.
 *
 * Revision 1.6  1994/11/26  03:44:19  robertj
 * Documentation.
 *
 * Revision 1.6  1994/11/24  11:48:26  robertj
 * Documentation.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/07/27  05:58:07  robertj
 * Synchronisation.
 *
 * Revision 1.2  1994/07/17  10:46:06  robertj
 * Changed to use container classes to plug memory leak.
 *
 * Revision 1.1  1994/04/01  14:08:52  robertj
 * Initial revision
 *
 */

#ifndef _PARGLIST
#define _PARGLIST

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PArgList, PObject)
/* This class allows the parsing of a set of program arguments. This translates
   the standard argc/argv style variables passed into the main() function into
   a set of options (preceded by a '-' character) and parameters.
*/

  public:
    PArgList(
      const char * theArgStr = NULL,   // A string constituting the arguments
      const char * argumentSpecPtr = NULL,
      /* The specification C string for argument options. See description for
         details.
       */
      BOOL optionsBeforeParams = TRUE  // Parse options only before parameters
    );
    PArgList(
      int theArgc,     // Count of argument strings in theArgv
      char ** theArgv, // An array of strings constituting the arguments
      const char * argumentSpecPtr = NULL,
      /* The specification C string for argument options. See description for
         details.
       */
      BOOL optionsBeforeParams = TRUE  // Parse options only before parameters
    );
    PArgList(
      int theArgc,     // Count of argument strings in theArgv
      char ** theArgv, // An array of strings constituting the arguments
      const PString & argumentSpecStr,
      /* The specification string for argument options. See description for
         details.
       */
      BOOL optionsBeforeParams = TRUE  // Parse options only before parameters
    );
    /* Create an argument list object given the standard arguments and a
       specification for options. The program arguments are parsed from this
       into options and parameters.
       
       The specification string consists of case significant letters for each
       option. If the letter is followed by the ':' character then the option
       has an associated string. This string must be in the argument or in the
       next argument.
     */

    void SetArgs(
      const PString & theArgStr // A string constituting the arguments
    );
    void SetArgs(
      int theArgc,     // Count of argument strings in theArgv
      char ** theArgv  // An array of strings constituting the arguments
    );
    void SetArgs(
      const PStringArray & theArgs // A string array constituting the arguments
    );
      // Set the internal copy of the program arguments.

    void Parse(
      const char * theArgumentSpec,
      /* The specification string for argument options. See description for
         details.
       */
      BOOL optionsBeforeParams = TRUE  // Parse options only before parameters
    );
    void Parse(
      const PString & theArgumentStr,
      /* The specification string for argument options. See description for
         details.
       */
      BOOL optionsBeforeParams = TRUE  // Parse options only before parameters
    );
    /* Parse the standard C program arguments into an argument of options and
       parameters. Consecutive calls with <CODE>optionsBeforeParams</CODE> set
       to TRUE will parse out different options and parameters. If SetArgs()
       function is called then the Parse() function will restart from the
       beginning of the argument list.
       
       The specification string consists of case significant letters for each
       option. If the letter is followed by a '-' character then a long name
       version of the option is present. This is terminated either by a '.' or
       a ':' character. If the single letter or long name is followed by the
       ':' character then the option has may have an associated string. This
       string must be within the argument or in the next argument. If a single
       letter option is followed by a ';' character, then the option may have
       an associated string but this MUST follow the letter immediately, if
       it is present at all.

       For example, "ab:c" allows for "-a -b arg -barg -c" and
       "a-an-arg.b-option:c;" allows for "-a --an-arg --option arg -c -copt".
     */

    PINDEX GetOptionCount(
      char optionChar  // Character letter code for the option
    ) const;
    PINDEX GetOptionCount(
      const char * optionStr // String code for the option
    ) const;
    PINDEX GetOptionCount(
      const PString & optionName // String code for the option
    ) const;
    /* Get the count of the number of times the option was specified on the
       command line.

       <H2>Returns:</H2>
       option repeat count.
     */

    BOOL HasOption(
      char optionChar  // Character letter code for the option
    ) const;
    BOOL HasOption(
      const char * optionStr // String letter code for the option
    ) const;
    BOOL HasOption(
      const PString & optionName // String code for the option
    ) const;
    /* Get the whether the option was specified on the command line.

       <H2>Returns:</H2>
       TRUE if the option was present.
     */

    PString GetOptionString(
      char optionChar,          // Character letter code for the option
      const char * dflt = NULL  // Default value of the option string
    ) const;
    PString GetOptionString(
      const char * optionStr,   // String letter code for the option
      const char * dflt = NULL  // Default value of the option string
    ) const;
    PString GetOptionString(
      const PString & optionName, // String code for the option
      const char * dflt = NULL    // Default value of the option string
    ) const;
    /* Get the string associated with an option e.g. -ofile or -o file
       would return the string "file". An option may have an associated string
       if it had a ':' character folowing it in the specification string passed
       to the Parse() function.

       <H2>Returns:</H2>
       the options associated string.
     */

    PINDEX GetCount() const;
    /* Get the number of parameters that may be obtained via the
       <A>GetParameter()<A> function. Note that this does not include options
       and option strings.

       <H2>Returns:</H2>
       count of parameters.
     */

    PString GetParameter(
      PINDEX num   // Number of the parameter to retrieve.
    ) const;
    /* Get the parameter that was parsed in the argument list.

       <H2>Returns:</H2>
       parameter string at the specified index.
     */

    PString operator[](
      PINDEX num   // Number of the parameter to retrieve.
    ) const;
    /* Get the parameter that was parsed in the argument list. The argument
       list object can thus be treated as an "array" of parameters.

       <H2>Returns:</H2>
       parameter string at the specified index.
     */

    void Shift(
      int sh // Number of parameters to shift forward through list
    );
    /* Shift the parameters by the specified amount. This allows the parameters
       to be parsed at the same position in the argument list "array".
     */

    PArgList & operator<<(
      int sh // Number of parameters to shift forward through list
    );
    /* Shift the parameters by the specified amount. This allows the parameters
       to be parsed at the same position in the argument list "array".
     */

    PArgList & operator>>(
      int sh // Number of parameters to shift backward through list
    );
    /* Shift the parameters by the specified amount. This allows the parameters
       to be parsed at the same position in the argument list "array".
     */


    virtual void IllegalArgumentIndex(
      PINDEX idx // Number of the parameter that was accessed.
    ) const;
    /* This function is called when access to illegal parameter index is made
       in the GetParameter function. The default behaviour is to output a
       message to the standard <A>PError</A> stream.
     */

    virtual void UnknownOption(
      const PString & option   // Option that was illegally placed on command line.
    ) const;
    /* This function is called when an unknown option was specified on the
       command line. The default behaviour is to output a message to the
       standard <A>PError</A> stream.
     */

    virtual void MissingArgument(
      const PString & option  // Option for which the associated string was missing.
    ) const;
    /* This function is called when an option that requires an associated
       string was specified on the command line but no associated string was
       provided. The default behaviour is to output a message to the standard
       <A>PError</A> stream.
     */


  protected:
    // Member variables
    PStringArray argumentArray;
    // The original program arguments.

    PString      optionLetters;
    // The specification letters for options

    PStringArray optionNames;
    // The specification strings for options

    PIntArray    optionCount;
    // The count of the number of times an option appeared in the command line.

    PStringArray optionString;
    // The array of associated strings to options.

    PIntArray    parameterIndex;
    // The index of each .

    int          shift;
    // Shift count for the parameters in the argument list.

  private:
    BOOL ParseOption(PINDEX idx, PINDEX offset, PINDEX & arg, const PIntArray & canHaveOptionString);
    PINDEX GetOptionCountByIndex(PINDEX idx) const;
    PString GetOptionStringByIndex(PINDEX idx, const char * dflt) const;
};


#endif

// End Of File ///////////////////////////////////////////////////////////////
