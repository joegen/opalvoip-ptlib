/*
 * $Id: http.h,v 1.29 1998/04/14 03:42:59 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: http.h,v $
 * Revision 1.29  1998/04/14 03:42:59  robertj
 * Fixed error code propagation in HTTP client.
 *
 * Revision 1.28  1998/02/03 06:29:38  robertj
 * Added local address and port to PHTTPRequest.
 *
 * Revision 1.27  1998/01/26 00:24:24  robertj
 * Added more information to PHTTPConnectionInfo.
 * Added function to allow HTTPClient to automatically connect if URL has hostname.
 *
 * Revision 1.26  1997/10/30 10:22:52  robertj
 * Added multiple user basic authorisation scheme.
 *
 * Revision 1.25  1997/10/03 13:30:15  craigs
 * Added ability to access client socket from within HTTP resources
 *
 * Revision 1.24  1997/03/28 04:40:22  robertj
 * Added tags for cookies.
 *
 * Revision 1.23  1997/01/12 04:15:19  robertj
 * Globalised MIME tag strings.
 *
 * Revision 1.22  1996/10/26 03:31:05  robertj
 * Changed OnError so can pass in full HTML page as parameter.
 *
 * Revision 1.21  1996/09/14 13:09:10  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.20  1996/08/22 13:20:55  robertj
 * Fixed bug in authorisation, missing virtual prevented polymorphism.
 *
 * Revision 1.19  1996/08/19 13:44:06  robertj
 * Fixed authorisation so if have no user/password on basic authentication, does not require it.
 *
 * Revision 1.18  1996/06/28 13:15:23  robertj
 * Modified HTTPAuthority so gets PHTTPReqest (mainly for URL) passed in.
 * Moved HTTP form resource to another compilation module.
 *
 * Revision 1.17  1996/06/07 13:52:20  robertj
 * Added PUT to HTTP proxy FTP. Necessitating redisign of entity body processing.
 *
 * Revision 1.16  1996/05/23 10:00:52  robertj
 * Added common function for GET and HEAD commands.
 * Fixed status codes to be the actual status code instead of sequential enum.
 * This fixed some problems with proxy pass through of status codes.
 *
 * Revision 1.14  1996/03/31 08:46:51  robertj
 * HTTP 1.1 upgrade.
 *
 * Revision 1.13  1996/03/17 05:41:57  robertj
 * Added hit count to PHTTPResource.
 *
 * Revision 1.12  1996/03/16 04:39:55  robertj
 * Added ParseReponse() for splitting reponse line into code and info.
 * Added client side support for HTTP socket.
 * Added hooks for proxy support in HTTP socket.
 *
 * Revision 1.11  1996/03/10 13:15:23  robertj
 * Redesign to make resources thread safe.
 *
 * Revision 1.10  1996/03/02 03:12:55  robertj
 * Added radio button and selection boxes to HTTP form resource.
 *
 * Revision 1.9  1996/02/25 11:14:21  robertj
 * Radio button support for forms.
 *
 * Revision 1.8  1996/02/25 02:57:48  robertj
 * Removed pass through HTTP resource.
 *
 * Revision 1.7  1996/02/19 13:25:43  robertj
 * Added overwrite option to AddResource().
 * Added get/set string to PHTTPString resource.
 * Moved nested classes from PHTTPForm.
 *
 * Revision 1.6  1996/02/13 13:09:16  robertj
 * Added extra parameters to callback function in PHTTPResources, required
 *   by descendants to make informed decisions on data being loaded.
 *
 * Revision 1.5  1996/02/08 12:04:19  robertj
 * Redesign of resource object callback virtuals.
 * Added HTML form resource type.
 *
 * Revision 1.4  1996/02/03 11:03:32  robertj
 * Added ismodified since and expires time checking.
 * Added PHTTPString that defaults to empty string.
 *
 * Revision 1.3  1996/01/28 14:15:38  robertj
 * Changed PCharArray in OnLoadData to PString for convenience in mangling data.
 * Beginning of pass through resource type.
 *
 * Revision 1.2  1996/01/26 02:24:26  robertj
 * Further implemetation.
 *
 * Revision 1.1  1996/01/23 13:04:20  robertj
 * Initial revision
 *
 */

#ifndef _PHTTP
#define _PHTTP

#ifdef __GNUC__
#pragma interface
#endif

#include <inetprot.h>
#include <mime.h>
#include <url.h>
#include <html.h>
#include <ipsock.h>


//////////////////////////////////////////////////////////////////////////////
// PHTTPSpace

class PHTTPResource;

PDECLARE_CLASS(PHTTPSpace, PObject)
/* This class describes a name space that a Universal Resource Locator operates
   in. Each section of the hierarchy field of the URL points to a leg in the
   tree specified by this class.
 */

  public:
    PHTTPSpace();
    // Construct the root of the URL name space tree.

    ~PHTTPSpace();
    // Destroy the sub-tree of the URL name space.


  // Overrides from class PObject
    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;
    /* Compare the two URLs and return their relative rank.

       <H2>Returns:</H2>
       <CODE>LessThan</CODE>, <CODE>EqualTo</CODE> or <CODE>GreaterThan</CODE>
       according to the relative rank of the objects.
     */


  // New functions for class.
    enum AddOptions {
      ErrorOnExist,
      Overwrite
    };
    BOOL AddResource(
      PHTTPResource * resource, // Resource to add to the name space.
      AddOptions overwrite = ErrorOnExist
        // Flag to overwrite an existing resource if it already exists.
    );
    /* Add a new resource to the URL space. If there is already a resource at
       the location in the tree, or that location in the tree is already in
       the path to another resource then the function will fail.

       The <CODE>overwrite</CODE> flag can be used to replace an existing
       resource. TH function will still fail if the resource is on a partial
       path to antoher resource but not if it is a leaf node.

       <H2>Returns:</H2>
       TRUE if resource added, FALSE if failed.
     */

    BOOL DelResource(
      const PURL & url          // URL to search for in the name space.
    );
    /* Delete an existing resource to the URL space. If there is not a resource
       at the location in the tree, or that location in the tree is in the
       path to another resource then the function will fail.

       <H2>Returns:</H2>
       TRUE if resource deleted, FALSE if failed.
     */

    PHTTPResource * FindResource(
      const PURL & url          // URL to search for in the name space.
    );
    /* Locate the resource specified by the URL in the URL name space.

       <H2>Returns:</H2>
       The resource found or NULL if no resource at that position in hiearchy.
     */

  protected:
    PHTTPSpace(
      const PString & name,           // Name of URL hierarchy section.
      PHTTPSpace * parentNode = NULL  // Pointer to parent node in tree.
    );

    PString name;
    PSORTED_LIST(ChildList, PHTTPSpace);
    ChildList children;
    PHTTPSpace * parent;
    PHTTPResource * resource;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTP

PDECLARE_CLASS(PHTTP, PInternetProtocol)
/* A TCP/IP socket for the HyperText Transfer Protocol version 1.0.

   When acting as a client, the procedure is to make the connection to a
   remote server, then to retrieve a document using the following procedure:
      <PRE><CODE>
      PHTTPSocket web("webserver");
      if (web.IsOpen()) {
        if (web.GetDocument("http://www.someone.com/somewhere/url")) {
          while (web.Read(block, sizeof(block)))
            ProcessHTML(block);
        }
        else
           PError << "Could not get page." << endl;
      }
      else
         PError << "HTTP conection failed." << endl;
      </PRE></CODE>

    When acting as a server, a descendant class would be created to override
    at least the <A>HandleOpenMailbox()</A>, <A>HandleSendMessage()</A> and
    <A>HandleDeleteMessage()</A> functions. Other functions may be overridden
    for further enhancement to the sockets capabilities, but these will give a
    basic POP3 server functionality.

    The server socket thread would continuously call the
    <A>ProcessMessage()</A> function until it returns FALSE. This will then
    call the appropriate virtual function on parsing the POP3 protocol.
 */

  public:
  // New functions for class.
    enum Commands {
      GET,
      HEAD,
      POST,
      PUT,
      PATCH,
      COPY,
      MOVE,
      DELETE,
      LINK,
      UNLINK,
      TRACE,
      WRAPPED,
      OPTIONS,
      CONNECT,
      NumCommands
    };

    enum StatusCode {
      Continue = 100,              // 100 - Continue
      SwitchingProtocols,          // 101 - upgrade allowed
      OK = 200,                    // 200 - request has succeeded
      Created,                     // 201 - new resource created: entity body contains URL
      Accepted,                    // 202 - request accepted, but not yet completed
      NonAuthoritativeInformation, // 203 - not definitive entity header
      NoContent,                   // 204 - no new information
      ResetContent,                // 205 - contents have been reset
      PartialContent,              // 206 - partial GET succeeded
      MultipleChoices = 300,       // 300 - requested resource available elsewehere 
      MovedPermanently,            // 301 - resource moved permanently: location field has new URL
      MovedTemporarily,            // 302 - resource moved temporarily: location field has new URL
      SeeOther,                    // 303 - see other URL
      NotModified,                 // 304 - document has not been modified
      UseProxy,                    // 305 - proxy redirect
      BadRequest = 400,            // 400 - request malformed or not understood
      UnAuthorised,                // 401 - request requires authentication
      PaymentRequired,             // 402 - reserved 
      Forbidden,                   // 403 - request is refused due to unsufficient authorisation
      NotFound,                    // 404 - resource cannot be found
      MethodNotAllowed,            // 405 - not allowed on this resource
      NoneAcceptable,              // 406 - encoding not acceptable
      ProxyAuthenticationRequired, // 407 - must authenticate with proxy first
      RequestTimeout,              // 408 - server timeout on request
      Conflict,                    // 409 - resource conflict on action
      Gone,                        // 410 - resource gone away
      LengthRequired,              // 411 - no Content-Length
      UnlessTrue,                  // 412 - no Range header for TRUE Unless
      InternalServerError = 500,   // 500 - server has encountered an unexpected error
      NotImplemented,              // 501 - server does not implement request
      BadGateway,                  // 502 - error whilst acting as gateway
      ServiceUnavailable,          // 503 - server temporarily unable to service request
      GatewayTimeout               // 504 - timeout whilst talking to gateway
    };

    // Common MIME header tags
    static const PCaselessString AllowTag;
    static const PCaselessString AuthorizationTag;
    static const PCaselessString ContentEncodingTag;
    static const PCaselessString ContentLengthTag;
    static const PCaselessString ContentTypeTag;
    static const PCaselessString DateTag;
    static const PCaselessString ExpiresTag;
    static const PCaselessString FromTag;
    static const PCaselessString IfModifiedSinceTag;
    static const PCaselessString LastModifiedTag;
    static const PCaselessString LocationTag;
    static const PCaselessString PragmaTag;
    static const PCaselessString PragmaNoCacheTag;
    static const PCaselessString RefererTag;
    static const PCaselessString ServerTag;
    static const PCaselessString UserAgentTag;
    static const PCaselessString WWWAuthenticateTag;
    static const PCaselessString MIMEVersionTag;
    static const PCaselessString ConnectionTag;
    static const PCaselessString KeepAliveTag;
    static const PCaselessString ProxyConnectionTag;
    static const PCaselessString ProxyAuthorizationTag;
    static const PCaselessString ProxyAuthenticateTag;
    static const PCaselessString ForwardedTag;
    static const PCaselessString SetCookieTag;
    static const PCaselessString CookieTag;


  protected:
    PHTTP();
    /* Create a TCP/IP HTTP protocol channel.
     */

    virtual PINDEX ParseResponse(
      const PString & line // Input response line to be parsed
    );
    /* Parse a response line string into a response code and any extra info
       on the line. Results are placed into the member variables
       <CODE>lastResponseCode</CODE> and <CODE>lastResponseInfo</CODE>.

       The default bahaviour looks for a space or a '-' and splits the code
       and info either side of that character, then returns FALSE.

       <H2>Returns:</H2>
       Position of continuation character in response, 0 if no continuation
       lines are possible.
     */
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPClient

PDECLARE_CLASS(PHTTPClient, PHTTP)
/* A TCP/IP socket for the HyperText Transfer Protocol version 1.0.

   When acting as a client, the procedure is to make the connection to a
   remote server, then to retrieve a document using the following procedure:
      <PRE><CODE>
      PHTTPSocket web("webserver");
      if (web.IsOpen()) {
        if (web.GetDocument("http://www.someone.com/somewhere/url")) {
          while (web.Read(block, sizeof(block)))
            ProcessHTML(block);
        }
        else
           PError << "Could not get page." << endl;
      }
      else
         PError << "HTTP conection failed." << endl;
      </PRE></CODE>
 */

  public:
    PHTTPClient();
    // Create a new HTTP client channel.


  // New functions for class.
    int ExecuteCommand(Commands cmd,
                       const PString & url,
                       const PMIMEInfo & outMIME,
                       const PString & dataBody,
                       PMIMEInfo & replyMime);

    BOOL WriteCommand(Commands cmd,
                      const PString & url,
                      const PMIMEInfo & outMIME,
                      const PString & dataBody);

    BOOL ReadResponse(PMIMEInfo & replyMIME);


    /* Send a command and wait for the response header (including MIME fields).
       Note that a body may still be on its way even if lasResponseCode is not
       200!

       <H2>Returns:</H2>
       TRUE if all of header returned and ready to receive body.
     */

    BOOL GetDocument(
      const PURL & url,         // Universal Resource Locator for document.
      PMIMEInfo & outMIME,      // MIME info in request
      PMIMEInfo & replyMIME     // MIME info in response
    );
    /* Get the document specified by the URL.

       <H2>Returns:</H2>
       TRUE if document is being transferred.
     */

    BOOL GetHeader(
      const PURL & url,         // Universal Resource Locator for document.
      PMIMEInfo & outMIME,      // MIME info in request
      PMIMEInfo & replyMIME     // MIME info in response
    );
    /* Get the header for the document specified by the URL.

       <H2>Returns:</H2>
       TRUE if document header is being transferred.
     */


    BOOL PostData(
      const PURL & url,             // Universal Resource Locator for document.
      PMIMEInfo & outMIME,          // MIME info in request
      const PStringToString & data, // Information posted to the HTTP server.
      PMIMEInfo & replyMIME         // MIME info in response
    );
    /* Post the data specified to the URL.

       <H2>Returns:</H2>
       TRUE if document is being transferred.
     */

  protected:
    BOOL AssureConnect(const PURL & url, PMIMEInfo & outMIME);
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPServer

class PHTTPConnectionInfo;

PDECLARE_CLASS(PHTTPServer, PHTTP)
/* A TCP/IP socket for the HyperText Transfer Protocol version 1.0.

    When acting as a server, a descendant class would be created to override
    at least the <A>HandleOpenMailbox()</A>, <A>HandleSendMessage()</A> and
    <A>HandleDeleteMessage()</A> functions. Other functions may be overridden
    for further enhancement to the sockets capabilities, but these will give a
    basic POP3 server functionality.

    The server socket thread would continuously call the
    <A>ProcessMessage()</A> function until it returns FALSE. This will then
    call the appropriate virtual function on parsing the POP3 protocol.
 */

  public:
    PHTTPServer();
    PHTTPServer(
      const PHTTPSpace & urlSpace  // Name space to use for URLs received.
    );
    /* Create a TCP/IP HTTP protocol socket channel. The form with the single
       <CODE>port</CODE> parameter creates an unopened socket, the form with
       the <CODE>address</CODE> parameter makes a connection to a remote
       system, opening the socket. The form with the <CODE>socket</CODE>
       parameter opens the socket to an incoming call from a "listening"
       socket.
     */


  // New functions for class.
    virtual PString GetServerName() const;
    /* Get the name of the server.

       <H2>Returns:</H2>
       String name of the server.
     */

    PHTTPSpace & GetURLSpace() { return urlSpace; }
    /* Get the name space being used by the HTTP server socket.

       <H2>Returns:</H2>
       URL name space tree.
     */

    void SetURLSpace(
      PHTTPSpace & space   // New URL name space to use.
    ) { urlSpace = space; }
    // Use a new URL name space for this HTTP socket.


    BOOL ProcessCommand();
    /* Process commands, dispatching to the appropriate virtual function. This
       is used when the socket is acting as a server.

       <H2>Returns:</H2>
       TRUE if the request specified persistant mode and the request version
       allows it, FALSE if the socket closed, timed out, the protocol does not
       allow persistant mode, or the client did not request it
       timed out
     */

    virtual BOOL OnGET(
      const PURL & url,                    // Universal Resource Locator for document.
      const PMIMEInfo & info,              // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo
    );
    /* Handle a GET command from a client.

       The default implementation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
     */



    virtual BOOL OnHEAD(
      const PURL & url,                   // Universal Resource Locator for document.
      const PMIMEInfo & info,             // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo
    );
    /* Handle a HEAD command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
     */

    virtual BOOL OnPOST(
      const PURL & url,                   // Universal Resource Locator for document.
      const PMIMEInfo & info,             // Extra MIME information in command.
      const PStringToString & data,       // Variables provided in the POST data.
      const PHTTPConnectionInfo & conInfo
    );
    /* Handle a POST command from a client.

       The default implementation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
     */

    virtual BOOL OnProxy(
      const PHTTPConnectionInfo & conInfo
    );
    /* Handle a proxy command request from a client. This will only get called
       if the request was not for this particular server. If it was a proxy
       request for this server (host and port number) then the appropriate
       <A>OnGET()</A>, <A>OnHEAD()</A> or <A>OnPOST()</A> command is called.

       The default implementation returns OnError(BadGateway).

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
     */


    virtual PString ReadEntityBody(
      const PHTTPConnectionInfo & connectInfo
    );
    /* Read the entity body associated with a HTTP request, and close the
       socket if not a persistant connection.

       <H2>Returns:</H2>
       The entity body of the command
     */

    virtual BOOL OnUnknown(
      const PCaselessString & command, // Complete command line received.
      const PHTTPConnectionInfo & connectInfo
    );
    /* Handle an unknown command.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
     */

    void StartResponse(
      StatusCode code,      // Status code for the response.
      PMIMEInfo & headers,  // MIME variables included in response.
      long bodySize         // Size of the rest of the response.
    );
    /* Write a command reply back to the client, and construct some of the
       outgoing MIME fields. The MIME fields are not sent.

       The <CODE>bodySize</CODE> parameter determines the size of the 
       entity body associated with the response. If <CODE>bodySize</CODE> is
       >= 0, then a ContentLength field will be added to the outgoing MIME
       headers if one does not already exist.

       If <CODE>bodySize</CODE> is < 0, then it is assumed that the size of
       the entity body is unknown, or has already been added, and no
       ContentLength field will be constructed. 

       If the version of the request is less than 1.0, then this function does
       nothing.
     */

    virtual BOOL OnError(
      StatusCode code,                         // Status code for the error response.
      const PCaselessString & extra,           // Extra information included in the response.
      const PHTTPConnectionInfo & connectInfo
    );
    /* Write an error response for the specified code.

       Depending on the <CODE>code</CODE> parameter this function will also
       send a HTML version of the status code for display on the remote client
       viewer.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
     */

    void SetDefaultMIMEInfo(
      PMIMEInfo & info,      // Extra MIME information in command.
      const PHTTPConnectionInfo & connectInfo
    );
    /* Set the default mime info
     */


  protected:
    void Construct();

    PINDEX majorVersion;
    PINDEX minorVersion;
    PINDEX transactionCount;
    PString userAgent;
    PTimeInterval nextTimeout;

    PHTTPSpace urlSpace;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPConnectionInfo

PDECLARE_CLASS(PHTTPConnectionInfo, PObject)
/* This object describes the connectiono associated with a HyperText Transport
   Protocol request. This information is required by handler functions on
   <A>PHTTPResource</A> descendant classes to manage the connection correctly.
*/
  public:
    PHTTPConnectionInfo(PHTTP::Commands cmd);
    PHTTPConnectionInfo(PHTTP::Commands cmd, const PURL & url, const PMIMEInfo & mime);
    void Construct(PHTTPServer & server, int majorVersion, int MinorVersion);

    PHTTP::Commands GetCommand() const { return command; }

    void SetURL(const PURL & u, WORD defPort);
    const PURL & GetURL() const       { return url; }

    const PMIMEInfo & GetMIME() const { return mimeInfo; }


    void SetPersistance(BOOL newPersist);
    BOOL IsCompatible(int major, int minor) const;

    BOOL IsPersistant() const         { return isPersistant; }
    BOOL IsProxyConnection() const    { return isProxyConnection; }
    int  GetMajorVersion() const      { return majorVersion; }
    int  GetMinorVersion() const      { return minorVersion; }

    long GetEntityBodyLength() const  { return entityBodyLength; }

  protected:
    PHTTP::Commands command;
    PURL      url;
    PMIMEInfo mimeInfo;
    BOOL      isPersistant;
    BOOL      isProxyConnection;
    int       majorVersion;
    int       minorVersion;
    long      entityBodyLength;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPRequest

PDECLARE_CLASS(PHTTPRequest, PObject)
/* This object describes a HyperText Transport Protocol request. An individual
   request is passed to handler functions on <A>PHTTPResource</A> descendant
   classes.
 */

  public:
    PHTTPRequest(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,     // Extra MIME information in command.
      PHTTPServer & socket          // socket that request initiated on
    );

    const PURL & url;               // Universal Resource Locator for document.
    const PMIMEInfo & inMIME;       // Extra MIME information in command.
    PHTTP::StatusCode code;         // Status code for OnError() reply.
    PMIMEInfo outMIME;              // MIME information used in reply.
    PINDEX contentSize;             // Size of the body of the resource data.
    PIPSocket::Address origin;      // IP address of origin host for request
    PIPSocket::Address localAddr;   // IP address of local interface for request
    WORD               localPort;   // Port number of local server for request
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPAuthority

PDECLARE_CLASS(PHTTPAuthority, PObject)
/* This abstract class describes the authorisation mechanism for a Universal
   Resource Locator.
 */

  public:
  // New functions for class.
    virtual PString GetRealm(
      const PHTTPRequest & request   // Request information.
    ) const = 0;
    /* Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       <H2>Returns:</H2>
       String for the authorisation realm name.
     */

    virtual BOOL Validate(
      const PHTTPRequest & request,  // Request information.
      const PString & authInfo       // Authority information string.
    ) const = 0;
    /* Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       <H2>Returns:</H2>
       TRUE if the user and password are authorised in the realm.
     */

    virtual BOOL IsActive() const;
    /* Determine if the authirisation is to be applied. This could be used to
       distinguish between net requiring authorisation and requiring autorisation
       but having no password.

       The default behaviour is to return TRUE.

       <H2>Returns:</H2>
       TRUE if the authorisation in the realm is to be applied.
     */

  protected:
    static void DecodeBasicAuthority(
      const PString & authInfo,   // Authority information string.
      PString & username,         // User name decoded from authInfo
      PString & password          // Password decoded from authInfo
    );
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPSimpleAuth

PDECLARE_CLASS(PHTTPSimpleAuth, PHTTPAuthority)
/* This class describes the simplest authorisation mechanism for a Universal
   Resource Locator, a fixed realm, username and password.
 */

  public:
    PHTTPSimpleAuth(
      const PString & realm,      // Name space for the username and password.
      const PString & username,   // Username that this object wiull authorise.
      const PString & password    // Password for the above username.
    );
    // Construct the simple authorisation structure.


  // Overrides from class PObject.
    virtual PObject * Clone() const;
    /* Create a copy of the class on the heap. This is used by the
       <A>PHTTPResource</A> classes for maintaining authorisation to
       resources.

       <H2>Returns:</H2>
       pointer to new copy of the class instance.
     */


  // Overrides from class PHTTPAuthority.
    virtual PString GetRealm(
      const PHTTPRequest & request   // Request information.
    ) const;
    /* Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       <H2>Returns:</H2>
       String for the authorisation realm name.
     */

    virtual BOOL Validate(
      const PHTTPRequest & request,  // Request information.
      const PString & authInfo       // Authority information string.
    ) const;
    /* Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       <H2>Returns:</H2>
       TRUE if the user and password are authorised in the realm.
     */

    virtual BOOL IsActive() const;
    /* Determine if the authirisation is to be applied. This could be used to
       distinguish between net requiring authorisation and requiring autorisation
       but having no password.

       The default behaviour is to return TRUE.

       <H2>Returns:</H2>
       TRUE if the authorisation in the realm is to be applied.
     */

    const PString & GetUserName() const { return username; }
    /* Get the user name allocated to this simple authorisation.

       <H2>Returns:</H2>
       String for the authorisation user name.
     */

    const PString & GetPassword() const { return password; }
    /* Get the password allocated to this simple authorisation.

       <H2>Returns:</H2>
       String for the authorisation password.
     */


  protected:
    PString realm;
    PString username;
    PString password;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPMultiSimpAuth

PDECLARE_CLASS(PHTTPMultiSimpAuth, PHTTPAuthority)
/* This class describes the simple authorisation mechanism for a Universal
   Resource Locator, a fixed realm, multiple usernames and passwords.
 */

  public:
    PHTTPMultiSimpAuth(
      const PString & realm      // Name space for the username and password.
    );
    PHTTPMultiSimpAuth(
      const PString & realm,             // Name space for the usernames.
      const PStringToString & userList // List of usernames and passwords.
    );
    // Construct the simple authorisation structure.


  // Overrides from class PObject.
    virtual PObject * Clone() const;
    /* Create a copy of the class on the heap. This is used by the
       <A>PHTTPResource</A> classes for maintaining authorisation to
       resources.

       <H2>Returns:</H2>
       pointer to new copy of the class instance.
     */


  // Overrides from class PHTTPAuthority.
    virtual PString GetRealm(
      const PHTTPRequest & request   // Request information.
    ) const;
    /* Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       <H2>Returns:</H2>
       String for the authorisation realm name.
     */

    virtual BOOL Validate(
      const PHTTPRequest & request,  // Request information.
      const PString & authInfo       // Authority information string.
    ) const;
    /* Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       <H2>Returns:</H2>
       TRUE if the user and password are authorised in the realm.
     */

    virtual BOOL IsActive() const;
    /* Determine if the authirisation is to be applied. This could be used to
       distinguish between net requiring authorisation and requiring autorisation
       but having no password.

       The default behaviour is to return TRUE.

       <H2>Returns:</H2>
       TRUE if the authorisation in the realm is to be applied.
     */

    void AddUser(
      const PString & username,   // Username that this object wiull authorise.
      const PString & password    // Password for the above username.
    );
    /* Get the user name allocated to this simple authorisation.

       <H2>Returns:</H2>
       String for the authorisation user name.
     */


  protected:
    PString realm;
    PStringToString users;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPResource

PDECLARE_CLASS(PHTTPResource, PObject)
/* This object describes a HyperText Transport Protocol resource. A tree of
   these resources are available to the <A>PHTTPSocket</A> class.
 */

  protected:
    PHTTPResource(
      const PURL & url               // Name of the resource in URL space.
    );
    PHTTPResource(
      const PURL & url,              // Name of the resource in URL space.
      const PHTTPAuthority & auth    // Authorisation for the resource.
    );
    PHTTPResource(
      const PURL & url,              // Name of the resource in URL space.
      const PString & contentType    // MIME content type for the resource.
    );
    PHTTPResource(
      const PURL & url,              // Name of the resource in URL space.
      const PString & contentType,   // MIME content type for the resource.
      const PHTTPAuthority & auth    // Authorisation for the resource.
    );
    // Create a new HTTP Resource.


  public:
    virtual ~PHTTPResource();
    // Destroy the HTTP Resource.


  // New functions for class.
    const PURL & GetURL() const { return baseURL; }
    /* Get the URL for this resource.

       <H2>Returns:</H2>
       The URL for this resource.
     */

    const PString & GetContentType() const { return contentType; }
    /* Get the current content type for the resource.

       <H2>Returns:</H2>
       string for the current MIME content type.
     */

    void SetContentType(
      const PString & newType
    ) { contentType = newType; }
    /* Set the current content type for the resource.
     */

    PHTTPAuthority * GetAuthority() const { return authority; }
    /* Get the current authority for the resource.

       <H2>Returns:</H2>
       Pointer to authority or NULL if unrestricted.
     */

    void SetAuthority(
      const PHTTPAuthority & auth
    );
    /* Set the current authority for the resource.
     */

    void ClearAuthority();
    /* Set the current authority for the resource to unrestricted.
     */

    DWORD GetHitCount() const { return hitCount; }
    /* Get the current hit count for the resource. This is the total number of
       times the resource was asked for by a remote client.

       <H2>Returns:</H2>
       Hit count for the resource.
     */

    void ClearHitCount() { hitCount = 0; }
    // Clear the hit count for the resource.


    virtual BOOL OnGET(
      PHTTPServer & server,       // HTTP server that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info,     // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo
    );
    /* Handle the GET command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtuals <A>LoadHeaders()</A> and <A>OnGETData()</A> to get
       a resource to be sent to the socket.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close.
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
     */

    virtual BOOL OnGETData(
      PHTTPServer & server,
      const PURL & url,
      const PHTTPConnectionInfo & connectInfo,
      PHTTPRequest & request
    );
    /* Load the data associated with a GET command.

       The default action is to call the virtual <A>LoadData()</A> to get a
       resource to be sent to the socket.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close.
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
    */

    virtual BOOL OnHEAD(
      PHTTPServer & server,       // HTTP server that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info,     // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo
    );
    /* Handle the HEAD command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <A>LoadHeaders()</A> to get the header information to
       be sent to the socket.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
     */

    virtual BOOL OnPOST(
      PHTTPServer & server,         // HTTP server that received the request
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data, // Variables in the POST data.
      const PHTTPConnectionInfo & conInfo
    );
    /* Handle the POST command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <A>Post()</A> function to handle the data being
       received.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
       If there is no ContentLength field in the response, this value must
       be FALSE for correct operation.
     */

    virtual BOOL IsModifiedSince(
      const PTime & when    // Time to see if modified later than
    );
    /* Check to see if the resource has been modified since the date
       specified.

       <H2>Returns:</H2>
       TRUE if has been modified since.
     */

    virtual BOOL GetExpirationDate(
      PTime & when          // Time that the resource expires
    );
    /* Get a block of data (eg HTML) that the resource contains.

       <H2>Returns:</H2>
       Status of load operation.
     */

    virtual PHTTPRequest * CreateRequest(
      const PURL & url,                   // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,           // Extra MIME information in command.
	  PHTTPServer & socket
    );
    /* Create a new request block for this type of resource.

       The default behaviour is to create a new PHTTPRequest instance.

       <H2>Returns:</H2>
       Pointer to instance of PHTTPRequest descendant class.
     */

    virtual BOOL LoadHeaders(
      PHTTPRequest & request    // Information on this request.
    ) = 0;
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual BOOL LoadData(
      PHTTPRequest & request,    // Information on this request.
      PCharArray & data          // Data used in reply.
    );
    /* Get a block of data that the resource contains.

       The default behaviour is to call the <A>LoadText()</A> function and
       if successful, call the <A>OnLoadedText()</A> function.

       <H2>Returns:</H2>
       TRUE if there is still more to load.
     */

    virtual PString LoadText(
      PHTTPRequest & request    // Information on this request.
    );
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
     */

    virtual void OnLoadedText(
      PHTTPRequest & request,    // Information on this request.
      PString & text             // Data used in reply.
    );
    /* This is called after the text has been loaded and may be used to
       customise or otherwise mangle a loaded piece of text. Typically this is
       used with HTML responses.

       The default action for this function is to do nothing.
     */

    virtual BOOL Post(
      PHTTPRequest & request,       // Information on this request.
      const PStringToString & data, // Variables in the POST data.
      PHTML & replyMessage          // Reply message for post.
    );
    /* Get a block of data (eg HTML) that the resource contains.

       The default action for this function is to do nothing and return
       success.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
     */


  protected:
    BOOL CheckAuthority(
      PHTTPServer & server,               // Server to send response to.
      const PHTTPRequest & request,       // Information on this request.
      const PHTTPConnectionInfo & conInfo // Information on the connection
    );
    /* See if the resource is authorised given the mime info
     */

    virtual BOOL OnGETOrHEAD(
      PHTTPServer & server,       // HTTP server that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info,     // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo,
      BOOL  IsGet
    );
    /* common code for GET and HEAD commands */

    PURL             baseURL;
    PString          contentType;
    PHTTPAuthority * authority;
    volatile DWORD   hitCount;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPString

PDECLARE_CLASS(PHTTPString, PHTTPResource)
/* This object describes a HyperText Transport Protocol resource which is a
   string kept in memory. For instance a pre-calculated HTML string could be
   set in this type of resource.
 */

  public:
    PHTTPString(
      const PURL & url             // Name of the resource in URL space.
    );
    PHTTPString(
      const PURL & url,            // Name of the resource in URL space.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    PHTTPString(
      const PURL & url,            // Name of the resource in URL space.
      const PString & str          // String to return in this resource.
    );
    PHTTPString(
      const PURL & url,            // Name of the resource in URL space.
      const PString & str,         // String to return in this resource.
      const PString & contentType  // MIME content type for the file.
    );
    PHTTPString(
      const PURL & url,            // Name of the resource in URL space.
      const PString & str,         // String to return in this resource.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    PHTTPString(
      const PURL & url,            // Name of the resource in URL space.
      const PString & str,         // String to return in this resource.
      const PString & contentType, // MIME content type for the file.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    /* Contruct a new simple string resource for the HTTP space. If no MIME
       content type is specified then a default type is "text/html".
     */


  // Overrides from class PHTTPResource
    virtual BOOL LoadHeaders(
      PHTTPRequest & request    // Information on this request.
    );
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual PString LoadText(
      PHTTPRequest & request    // Information on this request.
    );
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
     */

  // New functions for class.
    const PString & GetString() { return string; }
    /* Get the string for this resource.

       <H2>Returns:</H2>
       String for resource.
     */

    void SetString(
      const PString & str   // New string for the resource.
    ) { string = str; }
    /* Set the string to be returned by this resource.
     */


  protected:
    PString string;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

PDECLARE_CLASS(PHTTPFile, PHTTPResource)
/* This object describes a HyperText Transport Protocol resource which is a
   single file. The file can be anywhere in the file system and is mapped to
   the specified URL location in the HTTP name space defined by the
   <A>PHTTPSpace</A> class.
 */

  public:
    PHTTPFile(
      const PString & filename     // file in file system and URL name.
    );
    PHTTPFile(
      const PString & filename,    // file in file system and URL name.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    PHTTPFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file       // Location of file in file system.
    );
    PHTTPFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file,      // Location of file in file system.
      const PString & contentType  // MIME content type for the file.
    );
    PHTTPFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file,      // Location of file in file system.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    PHTTPFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file,      // Location of file in file system.
      const PString & contentType, // MIME content type for the file.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    /* Contruct a new simple file resource for the HTTP space. If no MIME
       content type is specified then a default type is used depending on the
       file type. For example, "text/html" is used of the file type is
       ".html" or ".htm". The default for an unknown type is
       "application/octet-stream".
     */


  // Overrides from class PHTTPResource
    virtual PHTTPRequest * CreateRequest(
      const PURL & url,                  // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,          // Extra MIME information in command.
  	  PHTTPServer & socket
    );
    /* Create a new request block for this type of resource.

       <H2>Returns:</H2>
       Pointer to instance of PHTTPRequest descendant class.
     */

    virtual BOOL LoadHeaders(
      PHTTPRequest & request    // Information on this request.
    );
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual BOOL LoadData(
      PHTTPRequest & request,    // Information on this request.
      PCharArray & data          // Data used in reply.
    );
    /* Get a block of data that the resource contains.

       <H2>Returns:</H2>
       TRUE if more to load.
     */

    virtual PString LoadText(
      PHTTPRequest & request    // Information on this request.
    );
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
     */


  protected:
    PHTTPFile(
      const PURL & url,       // Name of the resource in URL space.
      int dummy
    );
    // Constructor used by PHTTPDirectory


    PFilePath filePath;
};


PDECLARE_CLASS(PHTTPFileRequest, PHTTPRequest)
  PHTTPFileRequest(
    const PURL & url,             // Universal Resource Locator for document.
    const PMIMEInfo & inMIME,     // Extra MIME information in command.
	PHTTPServer & server
  );

  PFile file;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

PDECLARE_CLASS(PHTTPDirectory, PHTTPFile)
/* This object describes a HyperText Transport Protocol resource which is a
   set of files in a directory. The directory can be anywhere in the file
   system and is mapped to the specified URL location in the HTTP name space
   defined by the <A>PHTTPSpace</A> class.

   All subdirectories and files are available as URL names in the HTTP name
   space. This effectively grafts a file system directory tree onto the URL
   name space tree.

   See the <A>PMIMEInfo</A> class for more information on the mappings between
   file types and MIME types.
 */

  public:
    PHTTPDirectory(
      const PURL & url,            // Name of the resource in URL space.
      const PDirectory & dir       // Location of file in file system.
    );
    PHTTPDirectory(
      const PURL & url,            // Name of the resource in URL space.
      const PDirectory & dir,      // Location of file in file system.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    // Construct a new directory resource for HTTP.


  // Overrides from class PHTTPResource
    virtual PHTTPRequest * CreateRequest(
      const PURL & url,                  // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,          // Extra MIME information in command.
  	  PHTTPServer & socket
    );
    /* Create a new request block for this type of resource.

       <H2>Returns:</H2>
       Pointer to instance of PHTTPRequest descendant class.
     */

    virtual BOOL LoadHeaders(
      PHTTPRequest & request    // Information on this request.
    );
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual PString LoadText(
      PHTTPRequest & request    // Information on this request.
    );
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
     */


  protected:
    PDirectory basePath;
};


PDECLARE_CLASS(PHTTPDirRequest, PHTTPFileRequest)
  PHTTPDirRequest(
    const PURL & url,             // Universal Resource Locator for document.
    const PMIMEInfo & inMIME,     // Extra MIME information in command.
	PHTTPServer & server
  );

  PString fakeIndex;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
