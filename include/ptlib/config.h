/*
 * $Id: config.h,v 1.6 1994/08/21 23:43:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: config.h,v $
 * Revision 1.6  1994/08/21 23:43:02  robertj
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
    PConfig(Source src = Application);
    PConfig(Source src, const PString & section);
    PConfig(const PFilePath & filename);
    PConfig(const PFilePath & filename, const PString & section);
      // Create a new configuration object


    // New functions for class
    void SetDefaultSection(const PString & section);
      // Set the default section for variable operations.

    PString GetDefaultSection() const;
      // Get the default section for variable operations.

    PStringList GetSections();
      // Return a list of all the section names in the file.

    PStringList GetKeys() const;
    PStringList GetKeys(const PString & section) const;
      // Return a list of all the keys in the section. If the section name is
      // not specified then use the default section.


    void DeleteSection();
    void DeleteSection(const PString & section);
      // Delete all variables in the specified section. If the section name is
      // not specified then use the default section.

    void DeleteKey(const PString & key);
    void DeleteKey(const PString & section, const PString & key);
      // Delete the particular variable in the specified section.


    PString GetString(const PString & key);
    PString GetString(const PString & key, const PString & dflt);
    PString GetString(const PString & section,
                                    const PString & key, const PString & dflt);
      // Get a string variable determined by the key in the section.

    void SetString(const PString & key, const PString & value);
    void SetString(const PString & section,
                                   const PString & key, const PString & value);
      // Set a string variable determined by the key in the section.


    BOOL GetBoolean(const PString & key, BOOL dflt = FALSE);
    BOOL GetBoolean(const PString & section,
                                       const PString & key, BOOL dflt = FALSE);
      // Get a boolean variable determined by the key in the section.

    void SetBoolean(const PString & key, BOOL value);
    void SetBoolean(const PString & section, const PString & key, BOOL value);
      // Set a boolean variable determined by the key in the section.


    long GetInteger(const PString & key, long dflt = 0);
    long GetInteger(const PString & section, const PString & key, long dflt=0);
      // Get an integer variable determined by the key in the section.

    void SetInteger(const PString & key, long value);
    void SetInteger(const PString & section, const PString & key, long value);
      // Set an integer variable determined by the key in the section.


    double GetReal(const PString & key, double dflt = 0);
    double GetReal(const PString & section,const PString & key,double dflt=0);
      // Get a floating point variable determined by the key in the section.

    void SetReal(const PString & key, double value);
    void SetReal(const PString & section, const PString & key, double value);
      // Set a floating point variable determined by the key in the section.


  protected:
    // Member variables
    PString defaultSection;
      // The current section for variable values.


  private:
    void Construct(Source src);
    void Construct(const PFilePath & filename);
      // Do common construction code


// Class declaration continued in platform specific header file ///////////////
