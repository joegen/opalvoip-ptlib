/*
 * $Id: config.h,v 1.4 1994/06/25 11:55:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: config.h,v $
 * Revision 1.4  1994/06/25 11:55:15  robertj
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


PDECLARE_CLASS(PConfig, PObject)
  // A class representing a configuration for the application. A configuration
  // file consists of a number of section each with a number of variables. Each
  // variable has a key to identify it and a value.

  public:
    enum Source {
      Environment,
      System,
      Application
    };
    PConfig(Source src, const PString & section = "Options");
    PConfig(const PFilePath & filename, const PString & section = "Options");
      // Create a new configuration object


    // New functions for class
    void SetDefaultSection(const char * section);
      // Set the default section for variable operations.

    PString GetDefaultSection() const;
      // Get the default section for variable operations.

    PStringList GetSections();
      // Return a list of all the section names in the file.

    PStringList GetKeys(const char * section = NULL) const;
      // Return a list of all the keys in the section. If the section name is
      // NULL then use the default section.


    void DeleteSection(const char * section = NULL);
      // Delete all variables in the specified section. If the section name is
      // NULL then use the default section.

    void DeleteKey(const char * key);
    void DeleteKey(const char * section, const char * key);
      // Delete the particular variable in the specified section.


    PString GetString(const char * key, const char * dflt = "");
    PString GetString(const char * section,
                                         const char * key, const char * dflt);
      // Get a string variable determined by the key in the section.

    void SetString(const char * key, const char * value);
    void SetString(const char * section, const char * key, const char * value);
      // Set a string variable determined by the key in the section.


    BOOL GetBoolean(const char * key, BOOL dflt = FALSE);
    BOOL GetBoolean(const char * section, const char * key, BOOL dflt = FALSE);
      // Get a boolean variable determined by the key in the section.

    void SetBoolean(const char * key, BOOL value);
    void SetBoolean(const char * section, const char * key, BOOL value);
      // Set a boolean variable determined by the key in the section.


    long GetInteger(const char * key, long dflt = 0);
    long GetInteger(const char * section, const char * key, long dflt = 0);
      // Get an integer variable determined by the key in the section.

    void SetInteger(const char * key, long value);
    void SetInteger(const char * section, const char * key, long value);
      // Set an integer variable determined by the key in the section.


    double GetReal(const char * key, double dflt = 0);
    double GetReal(const char * section, const char * key, double dflt = 0);
      // Get a floating point variable determined by the key in the section.

    void SetReal(const char * key, double value);
    void SetReal(const char * section, const char * key, double value);
      // Set a floating point variable determined by the key in the section.


  protected:
    // Member variables
    PString defaultSection;
      // The current section for variable values.


// Class declaration continued in platform specific header file ///////////////
