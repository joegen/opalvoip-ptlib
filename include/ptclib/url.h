/*
 * $Id: url.h,v 1.3 1996/02/03 11:06:27 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: url.h,v $
 * Revision 1.3  1996/02/03 11:06:27  robertj
 * Added splitting of query field into variables dictionary.
 *
 * Revision 1.2  1996/01/26 02:24:32  robertj
 * Further implemetation.
 *
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
    const PString & GetPathStr() const          { return pathStr; }
    const PStringArray & GetPath() const        { return path; }
    BOOL IsAbsolutePath() const                 { return absolutePath; }
    const PString & GetParameters() const       { return parameters; }
    const PString & GetFragment() const         { return fragment; }
    const PString & GetQuery() const            { return queryStr; }
    PStringToString GetQueryVars() const        { return queryVars; }

  protected:
    PCaselessString scheme;
    PString username;
    PString password;
    PCaselessString hostname;
    WORD port;
    PString pathStr;
    PStringArray path;
    BOOL absolutePath;
    PString parameters;
    PString fragment;
    PString queryStr;
    PStringToString queryVars;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
