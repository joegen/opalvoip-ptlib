/*
 * url.h
 *
 * Universal Resource Locator (for HTTP/HTML) class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PURL_H
#define PTLIB_PURL_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#if P_URL

#include <ptlib/pfactory.h>


class PURLScheme;
class PURLLegacyScheme;


//////////////////////////////////////////////////////////////////////////////
// PURL

/**
 This class describes a Universal Resource Locator.
 This is the desciption of a resource location as used by the World Wide
 Web and the <code>PHTTPSocket</code> class.

 Note, this has been extended to be for URI's (Universal Resource  Identifier)
 which is a superset of URL's. But for backward compatinility reasons the name
 is still PURL.
 */
class PURL : public PObject
{
  PCLASSINFO(PURL, PObject)
  public:
    /**Construct a new URL object from the URL string. */
    PURL();
    /**Construct a new URL object from the URL string. */
    PURL(
      const char * cstr,    ///< C string representation of the URL.
      const char * defaultScheme = "http" ///< Default scheme for URL
    );
    /**Construct a new URL object from the URL string. */
    PURL(
      const PString & str,  ///< String representation of the URL.
      const char * defaultScheme = "http" ///< Default scheme for URL
    );
    /**Construct a new URL object from the file path. */
    PURL(
      const PFilePath & path   ///< File path to turn into a "file:" URL.
    );

    PURL(const PURL & other);
    PURL & operator=(const PURL & other);

  /**@name Overrides from class PObject */
  //@{
    /**Compare the two URLs and return their relative rank.

     @return
       <code>LessThan</code>, <code>EqualTo</code> or <code>GreaterThan</code>
       according to the relative rank of the objects.
     */
    virtual Comparison Compare(
      const PObject & obj   ///< Object to compare against.
    ) const;

    /**This function yields a hash value required by the <code>PDictionary</code>
       class. A descendent class that is required to be the key of a dictionary
       should override this function. The precise values returned is dependent
       on the semantics of the class. For example, the <code>PString</code> class
       overrides it to provide a hash function for distinguishing text strings.

       The default behaviour is to return the value zero.

       @return
       hash function value for class instance.
     */
    virtual PINDEX HashFunction() const;

    /**Output the contents of the URL to the stream as a string.
     */
    virtual void PrintOn(
      ostream &strm   ///< Stream to print the object into.
    ) const;

    /**Input the contents of the URL from the stream. The input is a URL in
       string form.
     */
    virtual void ReadFrom(
      istream &strm   ///< Stream to read the objects contents from.
    );
  //@}
 
  /**@name New functions for class. */
  //@{
    /**Parse the URL string into the fields in the object instance. */
    inline PBoolean Parse(
      const char * cstr,   ///< URL as a string to parse.
      const char * defaultScheme = NULL ///< Default scheme for URL
    ) { return InternalParse(cstr, defaultScheme); }
    /**Parse the URL string into the fields in the object instance. */
    inline PBoolean Parse(
      const PString & str, ///< URL as a string to parse.
      const char * defaultScheme = NULL ///< Default scheme for URL
    ) { return InternalParse((const char *)str, defaultScheme); }

    /**Print/String output representation formats. */
    enum UrlFormat {
      FullURL,       ///< Output full URI
      PathOnly,      ///< Translate to a string as only path
      LocationOnly,  ///< Translate to a string with the location (scheme and user/pass/host/port)
      RelativeOnly,  ///< Translate to a string with no scheme or host, just the relative part

      // for backward compatibility
      HostPortOnly = LocationOnly,
      URIOnly = RelativeOnly
    };

    /**Convert the URL object into its string representation. The parameter
       indicates whether a full or partial representation os to be produced.

       @return
       String representation of the URL.
     */
    PString AsString(
      UrlFormat fmt = FullURL   ///< The type of string to be returned.
    ) const;
    operator PString() const { return AsString(); }

    /**Get the "file:" URL as a file path.
       If the URL is not a "file:" URL then returns an empty string.
      */
    PFilePath AsFilePath() const;

    /// Type for translation of strings to URL format,
    enum TranslationType {
      /// Translate a username/password field for a URL.
      LoginTranslation,
      /// Translate the path field for a URL.
      PathTranslation,
      /// Translate the query variable field for a URL.
      QueryTranslation,
      /// Translate the parameter variables field for a URL.
      ParameterTranslation,
      /// Translate the quoted parameter variables field for a URL.
      QuotedParameterTranslation
    };

    /**Translate a string from general form to one that can be included into
       a URL. All reserved characters for the particular field type are
       escaped.

       @return
       String for the URL ready translation.
     */
    static PString TranslateString(
      const PString & str,    ///< String to be translated.
      TranslationType type    ///< Type of translation.
    );

    /**Untranslate a string from a form that was included into a URL into a
       normal string. All reserved characters for the particular field type
       are unescaped.

       @return
       String from the URL untranslated.
     */
    static PString UntranslateString(
      const PString & str,    ///< String to be translated.
      TranslationType type    ///< Type of translation.
    );

    /** Split a string to a dictionary of names and values. */
    static void SplitVars(
      const PString & str,    ///< String to split into variables.
      PStringToString & vars, ///< Dictionary of variable names and values.
      char sep1 = ';',        ///< Separater between pairs
      char sep2 = '=',        ///< Separater between key and value
      TranslationType type = ParameterTranslation ///< Type of translation.
    );

    /** Split a string in &= form to a dictionary of names and values. */
    static void SplitQueryVars(
      const PString & queryStr,   ///< String to split into variables.
      PStringToString & queryVars ///< Dictionary of variable names and values.
    ) { SplitVars(queryStr, queryVars, '&', '=', QueryTranslation); }

    /** Construct string from a dictionary using separators.
      */
    static void OutputVars(
      ostream & strm,               ///< Stream to output dictionary to
      const PStringToString & vars, ///< Dictionary of variable names and values.
      char sep0 = ';',              ///< First separater before all ('\0' means none)
      char sep1 = ';',              ///< Separater between pairs
      char sep2 = '=',              ///< Separater between key and value
      TranslationType type = ParameterTranslation ///< Type of translation.
    );


    /// Extract scheme as per RFC2396
    static PCaselessString ExtractScheme(const char * str);

    /// Get the scheme field of the URL.
    const PCaselessString & GetScheme() const { return m_scheme; }

    /// Set the scheme field of the URL
    bool SetScheme(const PString & scheme);

    /// Get the username field of the URL.
    const PString & GetUserName() const { return m_username; }

    /// Set the username field of the URL.
    void SetUserName(const PString & username);

    /// Get the password field of the URL.
    const PString & GetPassword() const { return m_password; }

    /// Set the password field of the URL.
    void SetPassword(const PString & password);

    /// Get the hostname field of the URL.
    const PCaselessString & GetHostName() const { return m_hostname; }

    /// Set the hostname field of the URL.
    void SetHostName(const PString & hostname);

    /// Get the port field of the URL.
    WORD GetPort() const { return m_port; }

    /// Set the port field in the URL. Zero resets to default.
    void SetPort(WORD newPort);
    
    /// Get if explicit port is specified.
    PBoolean GetPortSupplied() const { return m_portSupplied; }

    /// Get the hostname and optional port fields of the URL.
    PString GetHostPort() const;

    /// Get if path is relative or absolute
    PBoolean GetRelativePath() const { return m_relativePath; }

    /// Get the path field of the URL as a string.
    PString GetPathStr() const;

    /// Set the path field of the URL as a string.
    void SetPathStr(const PString & pathStr);

    /// Get the path field of the URL as a string array.
    const PStringArray & GetPath() const { return m_path; }

    /// Set the path field of the URL as a string array.
    void SetPath(const PStringArray & path);

    /// Append segment to the path field of the URL.
    void AppendPath(const PString & segment);

    /// Change segment in the path field of the URL.
    void ChangePath(
      const PString & segment, ///< New value for segment, empty means remove
      PINDEX idx = P_MAX_INDEX ///< Segment index, P_MAX_INDEX means last segment
    );

    /// Get the parameter (;) field of the URL.
    PString GetParameters() const;

    /// Set the parameter (;) field of the URL.
    void SetParameters(const PString & parameters);

    /// Get the parameter (;) field(s) of the URL as a string dictionary.
    /// Note the values have already been translated using UntranslateString
    const PStringOptions & GetParamVars() const { return m_paramVars; }

    /// Set the parameter (;) field(s) of the URL as a string dictionary.
    /// Note the values will be translated using TranslateString
    void SetParamVars(
      const PStringToString & paramVars,
      bool merge = false
    );

    /// Set the parameter (;) field of the URL as a string dictionary.
    /// Note the values will be translated using TranslateString
    void SetParamVar(
      const PString & key,          ///< Key to add/delete
      const PString & data,         ///< Vlaue to add at key, if empty string may be removed
      bool emptyDataDeletes = true  ///< If true, and data empty string, key is removed
    );

    /// Get the fragment (\#) field of the URL.
    const PString & GetFragment() const { return m_fragment; }

    /// Get the Query (?) field of the URL as a string.
    PString GetQuery() const;

    /// Set the Query (?) field of the URL as a string.
    /// Note the values will be translated using UntranslateString
    void SetQuery(const PString & query);

    /// Get the Query (?) field of the URL as a string dictionary.
    /// Note the values have already been translated using UntranslateString
    const PStringOptions & GetQueryVars() const { return m_queryVars; }

    /// Set the Query (?) field(s) of the URL as a string dictionary.
    /// Note the values will be translated using TranslateString
    void SetQueryVars(const PStringToString & queryVars);

    /// Set the Query (?) field of the URL as a string dictionary.
    /// Note the values will be translated using TranslateString
    void SetQueryVar(const PString & key, const PString & data);

    /// Get the contents of URL, data left after all elemetns are parsed out
    const PString & GetContents() const { return m_contents; }

    /// Set the contents of URL, data left after all elemetns are parsed out
    void SetContents(const PString & str);

    /// Return true if the URL is an empty string.
    PBoolean IsEmpty() const { return m_urlString.IsEmpty(); }


    struct LoadParams {
      LoadParams(
        const PString & requiredContentType = PString::Empty(),
        const PTimeInterval & timeout = PMaxTimeInterval
      ) : m_requiredContentType(requiredContentType)
        , m_timeout(timeout)
      {
      }

      PString       m_requiredContentType;
      PTimeInterval m_timeout;

      PString       m_username;     // Basic authentication
      PString       m_password;
#if P_SSL
      PString       m_authority;    // Directory, file or data
      PString       m_certificate;  // File or data
      PString       m_privateKey;   // File or data
#endif
      PStringOptions m_customOptions; // E.g. headers for http
    };
    /**Get the resource the URL is pointing at.
       The data returned is obtained according to the scheme and the factory
       PURLLoaderFactory.
      */
    bool LoadResource(
      PString & data,  ///< Resource data as a string
      const LoadParams & params = LoadParams()  ///< Parameters for load
    ) const;
    bool LoadResource(
      PBYTEArray & data,  ///< Resource data as a binary blob
      const LoadParams & params = LoadParams()  ///< Parameters for load
    ) const;

    // For backward compatibility
    template <class T>
    bool LoadResource(
      T & data,  ///< Resource data as a string
      const PString & requiredContentType = PString::Empty(), ///< Expected content type where applicable
      const PTimeInterval & timeout = PMaxTimeInterval        ///< Timeout to wait for resource
    ) const { return LoadResource(data, LoadParams(requiredContentType, timeout)); }

    /**Open the URL in a browser.

       @return
       The browser was successfully opened. This does not mean the URL exists and was
       displayed.
     */
    bool OpenBrowser() const { return OpenBrowser(AsString()); }
    static bool OpenBrowser(
      const PString & url   ///< URL to open
    );
  //@}

    bool LegacyParse(const char * str, const PURLLegacyScheme * schemeInfo);
    PString LegacyAsString(PURL::UrlFormat fmt, const PURLLegacyScheme * schemeInfo) const;

  protected:
    void CopyContents(const PURL & other);
    virtual PBoolean InternalParse(
      const char * cstr,         ///< URL as a string to parse.
      const char * defaultScheme ///< Default scheme for URL
    );
    void Recalculate();

    const PURLScheme * m_schemeInfo;
    PString            m_urlString;

    PCaselessString m_scheme;
    PString         m_username;
    PString         m_password;
    PCaselessString m_hostname;
    WORD            m_port;
    bool            m_portSupplied;          /// port was supplied in string input
    bool            m_relativePath;
    PStringArray    m_path;
    PStringOptions  m_paramVars;
    PString         m_fragment;
    PStringOptions  m_queryVars;
    PString         m_contents;  // Anything left after parsing other elements
};


//////////////////////////////////////////////////////////////////////////////
// PURLScheme

class PURLScheme : public PObject
{
  PCLASSINFO(PURLScheme, PObject);
  public:
    virtual bool Parse(const char * cstr, PURL & url) const = 0;
    virtual PString AsString(PURL::UrlFormat fmt, const PURL & purl) const = 0;
    virtual WORD GetDefaultPort() const { return 0; }
};

typedef PFactory<PURLScheme> PURLSchemeFactory;


//////////////////////////////////////////////////////////////////////////////
// PURLLegacyScheme

class PURLLegacyScheme : public PURLScheme
{
  public:
    PURLLegacyScheme(
      bool user    = false,
      bool pass    = false,
      bool host    = false,
      bool def     = false,
      bool defhost = false,
      bool query   = false,
      bool params  = false,
      bool frags   = false,
      bool path    = false,
      bool rel     = false,
      WORD port    = 0
    )
      : hasUsername           (user)
      , hasPassword           (pass)
      , hasHostPort           (host)
      , defaultToUserIfNoAt   (def)
      , defaultHostToLocal    (defhost)
      , hasQuery              (query)
      , hasParameters         (params)
      , hasFragments          (frags)
      , hasPath               (path)
      , relativeImpliesScheme (rel)
      , defaultPort           (port)
    { }

    bool Parse(const char * cstr, PURL & url) const
    {
      return url.LegacyParse(cstr, this);
    }

    PString AsString(PURL::UrlFormat fmt, const PURL & url) const
    {
      return url.LegacyAsString(fmt, this);
    }

    virtual WORD GetDefaultPort() const { return defaultPort; }

    bool hasUsername;
    bool hasPassword;
    bool hasHostPort;
    bool defaultToUserIfNoAt;
    bool defaultHostToLocal;
    bool hasQuery;
    bool hasParameters;
    bool hasFragments;
    bool hasPath;
    bool relativeImpliesScheme;
    WORD defaultPort;
};

/** Define a scheme based on basic legacy syntax.
    The full HTTP style URL is:
      scheme://user:pass@host:port/path#fragment;params?query
    the various flags indicate if the subsection is present in the scheme.

  */
#define PURL_LEGACY_SCHEME(schemeName, \
                           hasUsername,           /* URL scheme has a username */ \
                           hasPassword,           /* URL scheme has a password */ \
                           hasHostPort,           /* URL scheme has a host:port */ \
                           defaultToUserIfNoAt,   /* URL scheme is username if no @, otherwise host:port */ \
                           defaultHostToLocal,    /* URL scheme defaults to PIPSocket::GetHostName() if not present */ \
                           hasQuery,              /* URL scheme has a query section */ \
                           hasParameters,         /* URL scheme has a parameter section */ \
                           hasFragments,          /* URL scheme has a fragment section */ \
                           hasPath,               /* URL scheme has a path */ \
                           relativeImpliesScheme, /* URL scheme has relative path (no //) then scheme: is not output */ \
                           defaultPort)           /* URL scheme default port if not specified in host:port */ \
  class PURLLegacyScheme_##schemeName : public PURLLegacyScheme \
  { \
    public: \
      PURLLegacyScheme_##schemeName() \
        : PURLLegacyScheme(hasUsername, \
                           hasPassword, \
                           hasHostPort, \
                           defaultToUserIfNoAt, \
                           defaultHostToLocal, \
                           hasQuery, \
                           hasParameters, \
                           hasFragments, \
                           hasPath, \
                           relativeImpliesScheme, \
                           defaultPort) \
        { } \
  }; \
  PFACTORY_CREATE(PURLSchemeFactory, PURLLegacyScheme_##schemeName, #schemeName, true)



//////////////////////////////////////////////////////////////////////////////
// PURLLoader

class PURLLoader : public PObject
{
  PCLASSINFO(PURLLoader, PObject);
  public:
    virtual bool Load(PString & str, const PURL & url, const PURL::LoadParams & params) const = 0;
    virtual bool Load(PBYTEArray & data, const PURL & url, const PURL::LoadParams & params) const = 0;
};

typedef PFactory<PURLLoader> PURLLoaderFactory;

#if P_HTTP
  PFACTORY_LOAD(PURL_HttpLoader);
#endif // P_HTTP
#if P_FTP
  PFACTORY_LOAD(PURL_FtpLoader);
#endif // P_HTTP


#endif // P_URL

#endif // PTLIB_PURL_H


// End Of File ///////////////////////////////////////////////////////////////
