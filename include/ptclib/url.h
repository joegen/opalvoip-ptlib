/*
 * $Id: url.h,v 1.12 1998/02/16 00:12:53 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: url.h,v $
 * Revision 1.12  1998/02/16 00:12:53  robertj
 * Added function to open a URL in a browser.
 *
 * Revision 1.11  1998/02/03 10:02:35  robertj
 * Added ability to get scheme, host and port from URL as a string.
 *
 * Revision 1.10  1998/02/03 06:18:49  robertj
 * Fixed URL encoding to be closer to RFC
 *
 * Revision 1.9  1997/01/12 04:22:54  robertj
 * Added has function so URL can be dictionary key.
 *
 * Revision 1.8  1996/08/19 13:37:28  robertj
 * Fixed URL parsing and definition (cannot have relative paths).
 *
 * Revision 1.7  1996/06/10 09:55:44  robertj
 * Added global function for query parameters parsing.
 *
 * Revision 1.6  1996/03/31 08:53:13  robertj
 * Added string representation for URI part only.
 *
 * Revision 1.5  1996/03/16 04:46:02  robertj
 * Added translation type to TranslateString() to accommodate query variables.
 *
 * Revision 1.4  1996/03/02 03:12:13  robertj
 * Added function to translate a string to a form suitable for inclusion in a URL.
 *
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

    virtual PINDEX HashFunction() const;
    /* This function yields a hash value required by the <A>PDictionary</A>
       class. A descendent class that is required to be the key of a dictionary
       should override this function. The precise values returned is dependent
       on the semantics of the class. For example, the <A>PString</A> class
       overrides it to provide a hash function for distinguishing text strings.

       The default behaviour is to return the value zero.

       <H2>Returns:</H2>
       hash function value for class instance.
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
      FullURL,      // Translate to a string as a full URL
      PathOnly,     // Translate to a string as only path
      URIOnly,      // Translate to a string with no scheme or host
      HostPortOnly  // Translate to a string with scheme and host/port
    };

    PString AsString(
      UrlFormat fmt = FullURL   // The type of string to be returned.
    ) const;
    /* Convert the URL object into its string representation. The parameter
       indicates whether a full or partial representation os to be produced.

       <H2>Returns:</H2>
       String representation of the URL.
     */

    enum TranslationType {
       LoginTranslation,
       PathTranslation,
       QueryTranslation
    };
    static PString TranslateString(
      const PString & str,    // String to be translated
      TranslationType type
    );
    /* Translate a string from general form to one that can be included into
       a URL. All reserved characters are escaped.

       <H2>Returns:</H2>
       String for the URL ready translation.
     */

    static void SplitQueryVars(
      const PString & queryStr,   // String to split into variables.
      PStringToString & queryVars // Dictionary of variable names and values.
    );
    // Split a string in &= form to a dictionary of names and values.


    const PCaselessString & GetScheme() const   { return scheme; }
    const PString & GetUserName() const         { return username; }
    const PString & GetPassword() const         { return password; }
    const PCaselessString & GetHostName() const { return hostname; }
    WORD GetPort() const                        { return port; }
    const PString & GetPathStr() const          { return pathStr; }
    const PStringArray & GetPath() const        { return path; }
    const PString & GetParameters() const       { return parameters; }
    const PString & GetFragment() const         { return fragment; }
    const PString & GetQuery() const            { return queryStr; }
    PStringToString GetQueryVars() const        { return queryVars; }

    void SetPort(WORD newPort)                  { port = newPort; }

    static BOOL OpenBrowser(
      const PString & url   // URL to open
    );
    /* Open the URL in a browser.

       <H2>Returns:</H2>
       The browser was successfully opened. This does not mean the URL exists and was
       displayed.
     */


  protected:
    PCaselessString scheme;
    PString username;
    PString password;
    PCaselessString hostname;
    WORD port;
    PString pathStr;
    PStringArray path;
    PString parameters;
    PString fragment;
    PString queryStr;
    PStringToString queryVars;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
