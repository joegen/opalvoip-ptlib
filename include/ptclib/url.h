/*
 * $Id: url.h,v 1.1 1996/01/23 13:04:20 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: url.h,v $
 * Revision 1.1  1996/01/23 13:04:20  robertj
 * Initial revision
 *
 */

#ifndef _PURL
#define _PURL

#ifdef __GNUC__
#pragma interface
#endif


//////////////////////////////////////////////////////////////////////////////
// PURL

PDECLARE_CLASS(PURL, PObject)
/* This class describes a Universal Resource Locator as used by the World Wide
   Web and the <A>PHTTPSocket</A> class.
 */

  public:
    PURL();
    PURL(
      const char * cstr     // C string representation of the URL.
    );
    PURL(
      const PString & str   // String representation of the URL.
    );
    /* Construct a new URL object from the URL string.
     */

  // Overrides from class PObject
    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;
    /* Compare the two URLs and return their relative rank.

       <H2>Returns:</H2>
       <CODE>LessThan</CODE>, <CODE>EqualTo</CODE> or <CODE>GreaterThan</CODE>
       according to the relative rank of the objects.
     */

    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;
    /* Output the contents of the URL to the stream as a string.
     */

    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );
    /* Input the contents of the URL from the stream. The input is a URL in
       string form.
     */

 
  // New functions for class.
    void Parse(
      const char * cstr
    );
    void Parse(
      const PString & str
    ) { Parse((const char *)str); }
    /* Parse the URL string into the fields in the object instance.
     */

    enum UrlFormat {
      FullURL,  // Translate to a string as a full URL
      PathOnly  // Translate to a string as a partial URL.
    };

    PString AsString(
      UrlFormat fmt = FullURL   // The type of string to be returned.
    ) const;
    /* Convert the URL object into its string representation. The parameter
       indicates whether a full or partial representation os to be produced.

       <H2>Returns:</H2>
       String representation of the URL.
     */

    const PCaselessString & GetScheme() const   { return scheme; }
    const PString & GetUserName() const         { return username; }
    const PString & GetPassword() const         { return password; }
    const PCaselessString & GetHostName() const { return hostname; }
    WORD GetPort() const                        { return port; }
    const PStringArray & GetHierarchy() const   { return hierarchy; }
    const PString & GetParameters() const       { return parameters; }
    const PString & GetQuery() const            { return query; }

  protected:
    PCaselessString scheme;
    PString username;
    PString password;
    PCaselessString hostname;
    WORD port;
    PStringArray hierarchy;
    PString parameters;
    PString fragment;
    PString query;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
