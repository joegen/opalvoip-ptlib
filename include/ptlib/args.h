/*
 * $Id: args.h,v 1.4 1994/08/22 00:46:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * Argument Parsing Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: args.h,v $
 * Revision 1.4  1994/08/22 00:46:48  robertj
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

#ifdef __GNU__
#pragma interface
#endif


PCLASS PArgList {
  public:
    PArgList();
    PArgList(int theArgc, char ** theArgv, const char * argumentSpec = NULL);
    PArgList(int theArgc, char ** theArgv, const PString & argumentSpec);
      // Create an argument list object

    void SetArgs(int argc, char ** argv);
      // Set the internal arguments list.

    void Parse(const char * theArgumentSpec);
    void Parse(const PString & theArgumentSpec);
      // Parse the standard C program arguments into an argument list

    PINDEX GetOptionCount(char option) const;
    PINDEX GetOptionCount(const char * option) const;
      // Return count of the number of times the option was specified on the
      // command line.

    BOOL HasOption(char option) const;
    BOOL HasOption(const char * option) const;
      // Return TRUE if the option was specified on the command line.

    PString GetOptionString(char option, const char * dflt = NULL) const;
    PString GetOptionString(const char * option, const char * dflt = NULL) const;
      // Return the string associated with an option e.g. -ofile or -o file
      // would return "file".

    PINDEX GetCount() const;
      // Return number of arguments (not including options and option values)

    PString GetParameter(PINDEX num) const;
    PString operator[](PINDEX num) const;
      // Return the argument at the specified index

    void Shift(int sh);
    void operator<<(int sh);
    void operator>>(int sh);
      // Shift arguments by specified amount


    virtual void IllegalArgumentIndex(PINDEX idx) const;
      // Called when access to illegal argument index made.

    virtual void UnknownOption (char option) const;
      // Called when unknown option encountered.

    virtual void MissingArgument (char option) const;
      // Called when option argument missing.


  protected:
    void ParseProgName(const PString & argv0);
      // Parse the program name out of the argument list


    // Member variables
    int     arg_count;
    char ** arg_values;
           
    PString      argumentSpec;
    PStringArray argumentList;
    PIntArray    optionCount;
    PINDEX       shift;
};


#endif

// End Of File ///////////////////////////////////////////////////////////////
