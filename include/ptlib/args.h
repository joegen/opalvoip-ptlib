/*
 * $Id: args.h,v 1.6 1994/11/26 03:44:19 robertj Exp $
 *
 * Portable Windows Library
 *
 * Argument Parsing Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: args.h,v $
 * Revision 1.6  1994/11/26 03:44:19  robertj
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
    PArgList();
    PArgList(
      int theArgc,     // Count of argument strings in theArgv
      char ** theArgv, // An array of strings constituting the arguments
      const char * argumentSpecPtr = NULL
      /* The specification C string for argument options. See description for
         details.
       */
    );
    PArgList(
      int theArgc,     // Count of argument strings in theArgv
      char ** theArgv, // An array of strings constituting the arguments
      const PString & argumentSpecStr
      /* The specification string for argument options. See description for
         details.
       */
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
      int theArgc,     // Count of argument strings in theArgv
      char ** theArgv  // An array of strings constituting the arguments
    );
      // Set the internal copy of the program arguments.

    void Parse(
      const char * theArgumentSpec
      /* The specification string for argument options. See description for
         details.
       */
    );
    void Parse(
      const PString & theArgumentSpec
      /* The specification string for argument options. See description for
         details.
       */
    );
    /* Parse the standard C program arguments into an argument of options and
       parameters.
       
       The specification string consists of case significant letters for each
       option. If the letter is followed by the ':' character then the option
       has an associated string. This string must be in the argument or in the
       next argument.
     */

    PINDEX GetOptionCount(
      char option  // Character letter code for the option
    ) const;
    PINDEX GetOptionCount(
      const char * option // String letter code for the option
    ) const;
    /* Get the count of the number of times the option was specified on the
       command line.
       Returns: option repeat count.
     */

    BOOL HasOption(
      char option  // Character letter code for the option
    ) const;
    BOOL HasOption(
      const char * option // String letter code for the option
    ) const;
    /* Get the whether the option was specified on the command line.
       Returns: TRUE if the option was present.
     */

    PString GetOptionString(
      char option,              // Character letter code for the option
      const char * dflt = NULL  // Default value of the option string
    ) const;
    PString GetOptionString(
      const char * option,      // String letter code for the option
      const char * dflt = NULL  // Default value of the option string
    ) const;
    /* Get the string associated with an option e.g. -ofile or -o file
       would return the string "file". An option may have an associated string
       if it had a ':' character folowing it in the specification string passed
       to the Parse() function.
       Returns: the options associated string.
     */

    PINDEX GetCount() const;
    /* Get the number of parameters that may be obtained via the GetParameter()
       function. Note that this does not include options and option strings.
       Returns: count of parameters.
     */

    PString GetParameter(
      PINDEX num   // Number of the parameter to retrieve.
    ) const;
    /* Get the parameter that was parsed in the argument list.
       Returns: parameter string at the specified index.
     */

    PString operator[](
      PINDEX num   // Number of the parameter to retrieve.
    ) const;
    /* Get the parameter that was parsed in the argument list. The argument
       list object can thus be treated as an "array" of parameters.
       Returns: parameter string at the specified index.
     */

    void Shift(
      int sh // Number of parameters to shift forward through list
    );
    /* Shift the parameters by the specified amount. This allows the parameters
       to be parsed at the same position in the argument list "array".
     */

    void operator<<(
      int sh // Number of parameters to shift forward through list
    );
    /* Shift the parameters by the specified amount. This allows the parameters
       to be parsed at the same position in the argument list "array".
     */

    void operator>>(
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
       message to the standard $H$PError stream.
     */

    virtual void UnknownOption(
      char option   // Option that was illegally placed on command line.
    ) const;
    /* This function is called when an unknown option was specified on the
       command line. The default behaviour is to output a message to the
       standard $H$PError stream.
     */

    virtual void MissingArgument(
      char option  // Option for which the associated string was missing.
    ) const;
    /* This function is called when an option that requires an associated
       string was specified on the command line but no associated string was
       provided. The default behaviour is to output a message to the standard
       $H$PError stream.
     */


  protected:
    // Member variables
    int     arg_count;
    // The original program argument count as provided by main().

    char ** arg_values;
    // The original program argument list as provided by main().

    PString      argumentSpec;
    // The specification for options

    PIntArray    optionCount;
    // The count of the number of times an option appeared in the command line.

    PStringArray argumentList;
    // The array of associated strings to options.

    PINDEX       shift;
    // Shift count for the parameters in the argument list.
};


#endif

// End Of File ///////////////////////////////////////////////////////////////
