/*
 * $Id: http.h,v 1.16 1996/05/23 10:00:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: http.h,v $
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

#include <appsock.h>
#include <mime.h>
#include <url.h>
#include <html.h>


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
// PHTTPConnectionInfo

PDECLARE_CLASS(PHTTPConnectionInfo, PObject)
/* This object describes the connectiono associated with a HyperText Transport
   Protocol request. This information is required by handler functions on
   <A>PHTTPResource</A> descendant classes to manage the connection correctly.
*/
  public:
    PHTTPConnectionInfo();
    void Construct(const PMIMEInfo & inMIME, int majorVersion, int MinorVersion);

    void SetPersistance(BOOL newPersist);
    BOOL IsCompatible(int major, int minor) const;

    BOOL IsPersistant() const         { return isPersistant; }
    BOOL IsProxyConnection() const    { return isProxyConnection; }
    int  GetMajorVersion() const      { return majorVersion; }
    int  GetMinorVersion() const      { return minorVersion; }

  protected:
    BOOL isPersistant;
    BOOL isProxyConnection;
    int  majorVersion;
    int  minorVersion;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPSocket

PDECLARE_CLASS(PHTTPSocket, PApplicationSocket)
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
    PHTTPSocket(
      WORD port = 80              // Port number to connect to.
    );
    PHTTPSocket(
      const PString & address,    // Address of remote machine to connect to.
      WORD port = 80              // Port number to connect to.
    );
    PHTTPSocket(
      const PString & address,    // Address of remote machine to connect to.
      const PString & service     // Service name to connect to.
    );
    PHTTPSocket(
      PSocket & socket            // Listening socket making the connection.
    );
    PHTTPSocket(
      PSocket & socket,           // Listening socket making the connection.
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
      const PURL & url,          // Universal Resource Locator for document.
      const PMIMEInfo & outMIME, // MIME info in request
      PMIMEInfo & replyMIME      // MIME info in response
    );
    /* Get the document specified by the URL.

       <H2>Returns:</H2>
       TRUE if document is being transferred.
     */

    BOOL GetHeader(
      const PURL & url,          // Universal Resource Locator for document.
      const PMIMEInfo & outMIME, // MIME info in request
      PMIMEInfo & replyMIME      // MIME info in response
    );
    /* Get the header for the document specified by the URL.

       <H2>Returns:</H2>
       TRUE if document header is being transferred.
     */


    BOOL PostData(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & outMIME,    // MIME info in request
      const PStringToString & data, // Information posted to the HTTP server.
      PMIMEInfo & replyMIME         // MIME info in response
    );
    /* Post the data specified to the URL.

       <H2>Returns:</H2>
       TRUE if document is being transferred.
     */


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
      Commands cmd,                 // Command to be proxied.
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PString & body,         // Body of request.
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

    virtual BOOL OnUnknown(
      const PCaselessString & command, // Complete command line received.
      const PHTTPConnectionInfo & connectInfo
    );
    /* Handle an unknown command.

       <H2>Returns:</H2>
       TRUE if the connection may persist, FALSE if the connection must close
     */

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
      const PString & extra,                   // Extra information included in the response.
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


    void ConstructServer();

    PINDEX majorVersion;
    PINDEX minorVersion;
    PINDEX transactionCount;
    PString userAgent;
    PTimeInterval nextTimeout;

    PHTTPSpace urlSpace;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPAuthority

PDECLARE_CLASS(PHTTPAuthority, PObject)
/* This abstract class describes the authorisation mechanism for a Universal
   Resource Locator.
 */

  public:
  // New functions for class.
    virtual PString GetRealm() const = 0;
    /* Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       <H2>Returns:</H2>
       String for the authorisation realm name.
     */

    virtual BOOL Validate(
      const PString & authInfo  // Authority information string.
    ) const = 0;
    /* Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       <H2>Returns:</H2>
       TRUE if the user and password are authorised in the realm.
     */
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
    virtual PString GetRealm() const;
    /* Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       <H2>Returns:</H2>
       String for the authorisation realm name.
     */

    virtual BOOL Validate(
      const PString & authInfo  // Authority information string.
    ) const;
    /* Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       <H2>Returns:</H2>
       TRUE if the user and password are authorised in the realm.
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
// PHTTPRequest

PDECLARE_CLASS(PHTTPRequest, PObject)
/* This object describes a HyperText Transport Protocol request. An individual
   request is passed to handler functions on <A>PHTTPResource</A> descendant
   classes.
 */

  public:
    PHTTPRequest(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & inMIME      // Extra MIME information in command.
    );

    const PURL & url;               // Universal Resource Locator for document.
    const PMIMEInfo & inMIME;       // Extra MIME information in command.
    PHTTPSocket::StatusCode code;   // Status code for OnError() reply.
    PMIMEInfo outMIME;              // MIME information used in reply.
    PINDEX contentSize;             // Size of the body of the resource data.
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
      PHTTPSocket & socket,       // HTTP socket that received the request
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
      PHTTPSocket & socket,
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
      PHTTPSocket & socket,       // HTTP socket that received the request
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
      PHTTPSocket & socket,         // HTTP socket that received the request
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
      const PMIMEInfo & inMIME            // Extra MIME information in command.
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
      PHTTPSocket & socket,   // Socket to send response to.
      const PMIMEInfo & info, // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo
    );
    /* See if the resource is authorised given the mime info
     */

    virtual BOOL OnGETOrHEAD(
      PHTTPSocket & socket,       // HTTP socket that received the request
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
      const PMIMEInfo & inMIME           // Extra MIME information in command.
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
    const PMIMEInfo & inMIME      // Extra MIME information in command.
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
      const PMIMEInfo & inMIME           // Extra MIME information in command.
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
    const PMIMEInfo & inMIME      // Extra MIME information in command.
  );

  PString fakeIndex;
};


///////////////////////////////////////////////////////////////////////////////
// PHTTPForm

PDECLARE_CLASS(PHTTPField, PObject)
/* This class is the abstract base class for fields in a <A>PHTTPForm</A>
   resource type.
 */
  public:
    PHTTPField(
      const char * name,   // Name (identifier) for the field.
      const char * title,  // Title text for field (defaults to name).
      const char * help    // Help text for the field.
    );
    // Create a new field in a HTTP form.

    virtual Comparison Compare(
      const PObject & obj
    ) const;
    /* Compare the fields using the field names.

       <H2>Returns:</H2>
       Comparison of the name fields of the two fields.
     */

    const PCaselessString & GetName() const { return name; }
    /* Get the identifier name of the field.

       <H2>Returns:</H2>
       String for field name.
     */

    const PString & GetTitle() const { return title; }
    /* Get the title of the field.

       <H2>Returns:</H2>
       String for title placed next to the field.
     */

    const PString & GetHelp() const { return help; }
    /* Get the title of the field.

       <H2>Returns:</H2>
       String for title placed next to the field.
     */

    void SetHelp(
      const PString & text        // Help text.
    ) { help = text; }
    void SetHelp(
      const PString & hotLinkURL, // URL for link to help page.
      const PString & linkText    // Help text in the link.
    );
    void SetHelp(
      const PString & hotLinkURL, // URL for link to help page.
      const PString & imageURL,   // URL for image to be displayed in link.
      const PString & imageText   // Text in the link when image unavailable.
    );
    // Set the help text for the field.

    virtual void GetHTML(
      PHTML & html    // HTML to receive the field info.
    ) = 0;
    /* Convert the field to HTML for inclusion into the HTTP page.
     */

    virtual PString GetValue() const = 0;
    /* Get the value of the field.

       <H2>Returns:</H2>
       String for field value.
     */

    virtual void SetValue(
      const PString & val   // New value for the field.
    ) = 0;
    /* Set the value of the field.
     */

    virtual BOOL Validated(
      const PString & newVal, // Proposed new value for the field.
      PStringStream & msg     // Stream to take error HTML if value not valid.
    ) const;
    /* Validate the new field value before <A>SetValue()</A> if called.

       <H2>Returns:</H2>
       BOOL if the new field value is OK.
     */

    BOOL NotYetInHTML() const { return notInHTML; }

  protected:
    PCaselessString name;
    PString title;
    PString help;
    BOOL notInHTML;
};


PDECLARE_CLASS(PHTTPStringField, PHTTPField)
  public:
    PHTTPStringField(
      const char * name,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );
    PHTTPStringField(
      const char * name,
      const char * title,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    PString value;
    PINDEX size;
};


PDECLARE_CLASS(PHTTPPasswordField, PHTTPStringField)
  public:
    PHTTPPasswordField(
      const char * name,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );
    PHTTPPasswordField(
      const char * name,
      const char * title,
      PINDEX size,
      const char * initVal = NULL,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );
};


PDECLARE_CLASS(PHTTPIntegerField, PHTTPField)
  public:
    PHTTPIntegerField(
      const char * name,
      int low, int high,
      int initVal = 0,
      const char * units = NULL,
      const char * help = NULL
    );
    PHTTPIntegerField(
      const char * name,
      const char * title,
      int low, int high,
      int initVal = 0,
      const char * units = NULL,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );

    virtual BOOL Validated(
      const PString & newVal,
      PStringStream & msg
    ) const;


  protected:
    int low, high, value;
    PString units;
};


PDECLARE_CLASS(PHTTPBooleanField, PHTTPField)
  public:
    PHTTPBooleanField(
      const char * name,
      BOOL initVal = FALSE,
      const char * help = NULL
    );
    PHTTPBooleanField(
      const char * name,
      const char * title,
      BOOL initVal = FALSE,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    BOOL value;
};


PDECLARE_CLASS(PHTTPRadioField, PHTTPField)
  public:
    PHTTPRadioField(
      const char * name,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const PStringArray & valueArray,
      const PStringArray & titleArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      PINDEX count,
      const char * const * valueStrings,
      const char * const * titleStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      const PStringArray & valueArray,
      const PStringArray & titleArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPRadioField(
      const char * name,
      const char * groupTitle,
      PINDEX count,
      const char * const * valueStrings,
      const char * const * titleStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    PStringArray values;
    PStringArray titles;
    PString value;
};


PDECLARE_CLASS(PHTTPSelectField, PHTTPField)
  public:
    PHTTPSelectField(
      const char * name,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPSelectField(
      const char * name,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPSelectField(
      const char * name,
      const char * title,
      const PStringArray & valueArray,
      PINDEX initVal = 0,
      const char * help = NULL
    );
    PHTTPSelectField(
      const char * name,
      const char * title,
      PINDEX count,
      const char * const * valueStrings,
      PINDEX initVal = 0,
      const char * help = NULL
    );

    virtual void GetHTML(
      PHTML & html
    );

    virtual PString GetValue() const;

    virtual void SetValue(
      const PString & val
    );


  protected:
    PStringArray values;
    PString value;
};


PDECLARE_CLASS(PHTTPForm, PHTTPString)
  public:
    PHTTPForm(
      const PURL & url
    );
    PHTTPForm(
      const PURL & url,
      const PHTTPAuthority & auth
    );
    PHTTPForm(
      const PURL & url,
      const PString & html
    );
    PHTTPForm(
      const PURL & url,
      const PString & html,
      const PHTTPAuthority & auth
    );


    virtual BOOL Post(
      PHTTPRequest & request,       // Information on this request.
      const PStringToString & data, // Variables in the POST data.
      PHTML & replyMessage          // Reply message for post.
    );


    PHTTPField * Add(
      PHTTPField * fld
    );

    enum BuildOptions {
      CompleteHTML,
      InsertIntoForm,
      InsertIntoHTML
    };

    void BuildHTML(
      const char * heading
    );
    void BuildHTML(
      const PString & heading
    );
    void BuildHTML(
      PHTML & html,
      BuildOptions option = CompleteHTML
    );


  protected:
    PLIST(FieldList, PHTTPField);
    FieldList fields;
    PStringSet fieldNames;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPConfig

PDECLARE_CLASS(PHTTPConfig, PHTTPForm)
  public:
    PHTTPConfig(
      const PURL & url,
      const PString & section
    );
    PHTTPConfig(
      const PURL & url,
      const PString & section,
      const PHTTPAuthority & auth
    );
    PHTTPConfig(
      const PURL & url,
      const PString & section,
      const PString & html
    );
    PHTTPConfig(
      const PURL & url,
      const PString & section,
      const PString & html,
      const PHTTPAuthority & auth
    );

    virtual BOOL Post(
      PHTTPRequest & request,       // Information on this request.
      const PStringToString & data, // Variables in the POST data.
      PHTML & replyMessage          // Reply message for post.
    );


    const PString & GetConfigSection() const { return section; }
    /* Get the configuration file section that the page will alter.

       <H2>Returns:</H2>
       String for config file section.
     */

    void SetConfigSection(
      const PString & sect   // New section for the config page.
    ) { section = sect; }
    // Set the configuration file section.

    void SetConfigValues();
    /* Set all of the field values to the config files current values. Their
       current values are used as the defaults if no entry is present in the
       config file.
     */

    void AddNewKeyFields(
      PHTTPField * keyFld,  // Field for the key to be added.
      PHTTPField * valFld   // Field for the value of the key yto be added.
    );
    /* Add fields to the HTTP form for adding a new key to the config file
       section.
     */


  protected:
    PString section;
    PHTTPField * keyField;
    PHTTPField * valField;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
