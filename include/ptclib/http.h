/*
 * $Id: http.h,v 1.4 1996/02/03 11:03:32 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: http.h,v $
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
    BOOL AddResource(
      PHTTPResource * resource  // Resource to add to the name space.
    );
    /* Add a new resource to the URL space. If there is already a resource at
       the location in the tree, or that location in the tree is already in
       the path to another resource then the function will fail.

       <H2>Returns:</H2>
       TRUE if resource added, FALSE if failed.
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
      const PString & name   // Name of URL hierarchy section
    );

    PString name;
    PSORTED_LIST(ChildList, PHTTPSpace);
    ChildList children;
    PHTTPResource * resource;
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
    BOOL GetDocument(
      const PString & url    // Universal Resource Locator for document.
    );
    /* Get the document specified by the URL.

       <H2>Returns:</H2>
       TRUE if document is being transferred.
     */

    BOOL GetHeader(
      const PString & url    // Universal Resource Locator for document.
    );
    /* Get the header for the document specified by the URL.

       <H2>Returns:</H2>
       TRUE if document header is being transferred.
     */


    BOOL PostData(
      const PString & url,          // Universal Resource Locator for document.
      const PStringToString & data  // Information posted to the HTTP server.
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


    enum Commands {
      GET,
      HEAD,
      POST,
      NumCommands
    };

    BOOL ProcessCommand();
    /* Process commands, dispatching to the appropriate virtual function. This
       is used when the socket is acting as a server.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the socket closed or the
       <A>OnUnknown()</A> function returns FALSE.
     */

    virtual void OnGET(
      const PURL & url,       // Universal Resource Locator for document.
      const PMIMEInfo & info  // Extra MIME information in command.
    );
    /* Handle a GET command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.
     */

    virtual void OnHEAD(
      const PURL & url,       // Universal Resource Locator for document.
      const PMIMEInfo & info  // Extra MIME information in command.
    );
    /* Handle a HEAD command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.
     */

    virtual void OnPOST(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data  // Variables provided in the POST data.
    );
    /* Handle a POST command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.
     */

    virtual BOOL OnUnknown(
      const PCaselessString & command  // Complete command line received.
    );
    /* Handle an unknown command.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
     */


    enum StatusCode {
      Information,
      OK,                    // request has succeeded
      Created,               // new resource created: entity body contains URL
      Accepted,              // request accepted, but not yet completed
      NoContent,             // no new information
      MovedPermanently,      // resource moved permanently: location field has new URL
      MovedTemporarily,      // resource moved temporarily: location field has new URL
      NotModified,           // document has not been modified
      BadRequest,            // request malformed or not understood
      UnAuthorised,          // request requires authentication
      Forbidden,             // request is refused due to unsufficient authorisation
      NotFound,              // resource cannot be found
      InternalServerError,   // server has encountered an unexpected error
      NotImplemented,        // server does not implement request
      BadGateway,            // error whilst acting as gateway
      ServiceUnavailable,    // server temporarily unable to service request
      NumStatusCodes
    };

    void StartResponse(
      StatusCode code,      // Status code for the response.
      PMIMEInfo & headers,  // MIME variables included in response.
      PINDEX bodySize       // Size of the rest of the response.
    );
    /* Write a reply back to the client. This will send the properly formatted
       reply field followed by the MIME headers which will automatically
       include mandatory fields. Additional fields may be included into the
       <CODE>headers</CODE> parameter beforehand.

       If the major version of the request was 0.9 then this function does
       nothing.
     */

    virtual void OnError(
      StatusCode code,      // Status code for the error response.
      const PString & extra // Extra information included in the response.
    );
    /* Write an error response for the specified code.

       Depending on the <CODE>code</CODE> parameter this function will also
       send a HTML version of the status code for display on the remote client
       viewer.
     */

    void SetDefaultMIMEInfo(
      PMIMEInfo & info       // Extra MIME information in command.
    );
    /* Set the default mime info
     */


  protected:
    int majorVersion;
    int minorVersion;

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

  protected:
    PString realm;
    PString username;
    PString password;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPResource

PDECLARE_CLASS(PHTTPResource, PObject)
/* This object describes a HyperText Transport Protocol resource. A tree of
   these resources are available to the <A>PHTTPSocket</A> class.
 */

  protected:
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
    /* Get the current content type for the file.

       <H2>Returns:</H2>
       string for the current MIME content type.
     */

    void SetContentType(
      const PString & newType
    ) { contentType = newType; }
    /* Set the current content type for the file.
     */

    virtual void OnGET(
      PHTTPSocket & socket,       // HTTP socket that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info      // Extra MIME information in command.
    );
    /* Handle the GET command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <A>OnLoadData()</A> to get a memory block to be sent
       to the socket.
     */

    virtual void OnHEAD(
      PHTTPSocket & socket,       // HTTP socket that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info      // Extra MIME information in command.
    );
    /* Handle the HEAD command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <A>OnLoadHead()</A> to get a memory block to be sent
       to the socket.
     */

    virtual void OnPOST(
      PHTTPSocket & socket,         // HTTP socket that received the request
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data  // Variables in the POST data.
    );
    /* Handle the POST command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <A>Post()</A> function to handle the data being
       received.
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

    virtual PHTTPSocket::StatusCode OnLoadData(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PString & data,             // Data used in reply.
      PMIMEInfo & outMIME         // MIME information used in reply.
    ) = 0;
    /* Get a block of data (eg HTML) that the resource contains.

       <H2>Returns:</H2>
       Status of load operation.
     */

    virtual PHTTPSocket::StatusCode OnLoadHead(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PString & data,             // Data used in reply.
      PMIMEInfo & outMIME         // MIME information used in reply.
    );
    /* Get the head of block of data (eg HTML) that the resource contains.

       The default action of this function is to return whatever the
       <A>OnLoadData()</A> function does.

       <H2>Returns:</H2>
       Status of load operation.
     */

    virtual PHTTPSocket::StatusCode Post(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data  // Variables in the POST data.
    );
    /* Get a block of data (eg HTML) that the resource contains.

       The default action for this function is to do nothing and return
       success.

       <H2>Returns:</H2>
       Status of post operation.
     */


  protected:
    BOOL CheckAuthority(
      PHTTPSocket & socket,   // Socket to send response to.
      const PMIMEInfo & info  // Extra MIME information in command.
    );
    /* See if the resource is authorised given the mime info
     */

    PURL baseURL;
    PString contentType;
    PHTTPAuthority * authority;
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
    virtual PHTTPSocket::StatusCode OnLoadData(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PString & data,             // Data used in reply.
      PMIMEInfo & outMIME         // MIME information used in reply.
    );
    /* Get a block of data (eg HTML) that the resource contains.

       <H2>Returns:</H2>
       Status of load operation.
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
    virtual PHTTPSocket::StatusCode OnLoadData(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PString & data,             // Data used in reply.
      PMIMEInfo & outMIME         // MIME information used in reply.
    );
    /* Get a block of data (eg HTML) that the resource contains.

       <H2>Returns:</H2>
       Status of load operation.
     */


  protected:
    PFilePath filePath;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

PDECLARE_CLASS(PHTTPDirectory, PHTTPResource)
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
    virtual PHTTPSocket::StatusCode OnLoadData(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PString & data,             // Data used in reply.
      PMIMEInfo & outMIME         // MIME information used in reply.
    );
    /* Get a block of data (eg HTML) that the resource contains.

       <H2>Returns:</H2>
       Status of load operation.
     */


  protected:
    PDirectory basePath;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPPassThrough

PDECLARE_CLASS(PHTTPPassThrough, PHTTPResource)
/* This object describes a HyperText Transport Protocol resource which is a
   passes through to another HTTP server.
 */

  public:
    PHTTPPassThrough(
      const PURL & localURL,       // Name of the resource in URL space.
      const PURL & remoteURL       // Name of the resource on other server.
    );
    /* Contruct a new pass through resource for the HTTP space.
     */


  // Overrides from class PHTTPResource
    virtual void OnGET(
      PHTTPSocket & socket,       // HTTP socket that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info      // Extra MIME information in command.
    );
    /* Handle the GET command passed from the HTTP socket.

       This will pass the request on to another server and send the reply
       back on to the client.
     */


  protected:
    PURL remoteURL;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
