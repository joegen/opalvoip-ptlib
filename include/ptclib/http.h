/*
 * $Id: http.h,v 1.6 1996/02/13 13:09:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: http.h,v $
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
    BOOL AddResource(
      PHTTPResource * resource  // Resource to add to the name space.
    );
    /* Add a new resource to the URL space. If there is already a resource at
       the location in the tree, or that location in the tree is already in
       the path to another resource then the function will fail.

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

    virtual BOOL OnGET(
      const PURL & url,       // Universal Resource Locator for document.
      const PMIMEInfo & info  // Extra MIME information in command.
    );
    /* Handle a GET command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
     */

    virtual BOOL OnHEAD(
      const PURL & url,       // Universal Resource Locator for document.
      const PMIMEInfo & info  // Extra MIME information in command.
    );
    /* Handle a HEAD command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
     */

    virtual BOOL OnPOST(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data  // Variables provided in the POST data.
    );
    /* Handle a POST command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <A>PHTTPSpace</A> class tree and despatches to the
       <A>PHTTPResource</A> object contained therein.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
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

    virtual BOOL OnError(
      StatusCode code,      // Status code for the error response.
      const PString & extra // Extra information included in the response.
    );
    /* Write an error response for the specified code.

       Depending on the <CODE>code</CODE> parameter this function will also
       send a HTML version of the status code for display on the remote client
       viewer.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
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


    virtual void OnGET(
      PHTTPSocket & socket,       // HTTP socket that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info      // Extra MIME information in command.
    );
    /* Handle the GET command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtuals <A>LoadHeaders()</A> and <A>LoadData()</A> to get a
       resource to be sent to the socket.
     */

    virtual void OnHEAD(
      PHTTPSocket & socket,       // HTTP socket that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info      // Extra MIME information in command.
    );
    /* Handle the HEAD command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <A>LoadHeaders()</A> to get the header information to
       be sent to the socket.
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

    virtual BOOL LoadHeaders(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PHTTPSocket::StatusCode & code,   // Status code for OnError() reply.
      PMIMEInfo & outMIME,        // MIME information used in reply.
      PINDEX & contentSize        // Size of the body of the resource data
    ) = 0;
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual BOOL LoadData(
           PCharArray & data,           // Data used in reply.
            PMIMEInfo & outMIME,
           const PURL & url,
      const PMIMEInfo & inMIME
    );
    /* Get a block of data that the resource contains.

       The default behaviour is to call the <A>LoadText()</A> function and
       if successful, call the <A>OnLoadedText()</A> function.

       <H2>Returns:</H2>
       TRUE if there is still more to load.
     */

    virtual PString LoadText();
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
     */

    virtual void OnLoadedText(
              PString & text,       // Data used in reply.
            PMIMEInfo & outMIME,
           const PURL & url,
      const PMIMEInfo & inMIME
    );
    /* This is called after the text has been loaded and may be used to
       customise or otherwise mangle a loaded piece of text. Typically this is
       used with HTML responses.

       The default action for this function is to do nothing.
     */

    virtual PHTTPSocket::StatusCode Post(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data, // Variables in the POST data.
      PStringStream & replyMessage  // Reply message for post.
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
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PHTTPSocket::StatusCode & code,   // Status code for OnError() reply.
      PMIMEInfo & outMIME,        // MIME information used in reply.
      PINDEX & contentSize        // Size of the body of the resource data
    );
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual PString LoadText();
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
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
    virtual BOOL LoadHeaders(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PHTTPSocket::StatusCode & code,   // Status code for OnError() reply.
      PMIMEInfo & outMIME,        // MIME information used in reply.
      PINDEX & contentSize        // Size of the body of the resource data
    );
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual BOOL LoadData(
           PCharArray & data,       // Data used in reply.
            PMIMEInfo & outMIME,
           const PURL & url,
      const PMIMEInfo & inMIME
    );
    /* Get a block of data that the resource contains.

       <H2>Returns:</H2>
       TRUE if more to load.
     */

    virtual PString LoadText();
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
     */


  protected:
    PHTTPFile(
      const PURL & url       // Name of the resource in URL space.
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
    virtual BOOL LoadHeaders(
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,   // Extra MIME information in command.
      PHTTPSocket::StatusCode & code,   // Status code for OnError() reply.
      PMIMEInfo & outMIME,        // MIME information used in reply.
      PINDEX & contentSize        // Size of the body of the resource data
    );
    /* Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       <H2>Returns:</H2>
       TRUE if all OK, FALSE if an error occurred.
     */

    virtual PString LoadText();
    /* Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <A>LoadText()</A> or
       <A>LoadData()</A> functions must be overridden for correct operation.

       <H2>Returns:</H2>
       String for loaded text.
     */


  protected:
    PDirectory basePath;
    PString fakeIndex;
};


///////////////////////////////////////////////////////////////////////////////
// PHTTPForm

PDECLARE_CLASS(PHTTPForm, PHTTPString)
  public:
    PHTTPForm(
      const PURL & url
    );
    PHTTPForm(
      const PURL & url,
      PHTTPAuthority & auth
    );
    PHTTPForm(
      const PURL & url,
      const PString & html
    );
    PHTTPForm(
      const PURL & url,
      const PString & html,
      PHTTPAuthority & auth
    );


    virtual PHTTPSocket::StatusCode Post(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data, // Variables in the POST data.
      PStringStream & replyMessage  // Reply message for post.
    );


    PDECLARE_CLASS(Field, PObject)
      public:
        Field(
          const char * name,
          const char * title = NULL
        );

        virtual Comparison Compare(
          const PObject & obj
        ) const;

        const PCaselessString & GetName() const { return name; }

        const PString & GetTitle() const { return title; }

        virtual PHTML::Element GetHTML() const = 0;

        virtual PString GetValue() const = 0;

        virtual void SetValue(
          const PString & val
        ) = 0;

        virtual BOOL Validated(
          const PString & newVal,
          PStringStream & msg
        ) const;


      protected:
        PCaselessString name;
        PString title;
    };

    PDECLARE_CLASS(StringField, Field)
      public:
        StringField(
          const char * name,
          PINDEX size,
          const char * initVal = ""
        );
        StringField(
          const char * name,
          const char * title,
          PINDEX size,
          const char * initVal
        );

        virtual PHTML::Element GetHTML() const;

        virtual PString GetValue() const;

        virtual void SetValue(
          const PString & val
        );


      protected:
        PString value;
        PINDEX size;
    };

    PDECLARE_CLASS(PasswordField, StringField)
      public:
        PasswordField(
          const char * name,
          PINDEX size,
          const char * initVal = ""
        );
        PasswordField(
          const char * name,
          const char * title,
          PINDEX size,
          const char * initVal
        );

        virtual PHTML::Element GetHTML() const;
    };

    PDECLARE_CLASS(IntegerField, Field)
      public:
        IntegerField(
          const char * name,
          int low, int high,
          int initVal = 0,
          const char * units = ""
        );
        IntegerField(
          const char * name,
          const char * title,
          int low, int high,
          int initVal = 0,
          const char * units = ""
        );

        virtual PHTML::Element GetHTML() const;

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

    PDECLARE_CLASS(BooleanField, Field)
      public:
        BooleanField(
          const char * name,
          BOOL initVal
        );
        BooleanField(
          const char * name,
          const char * title,
          BOOL initVal
        );

        virtual PHTML::Element GetHTML() const;
        virtual PString GetValue() const;
        virtual void SetValue(
          const PString & val
        );


      protected:
        BOOL value;
    };


    void Add(
      Field * fld
    );

    void BuildHTML(
      const char * heading,
      BOOL complete = TRUE
    );
    void BuildHTML(
      const PString & heading,
      BOOL complete = TRUE
    );
    void BuildHTML(
      PHTML & html,
      BOOL complete = TRUE
    );


  protected:
    PLIST(FieldList, Field);
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
      PHTTPAuthority & auth
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
      PHTTPAuthority & auth
    );

    virtual PHTTPSocket::StatusCode Post(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & info,       // Extra MIME information in command.
      const PStringToString & data, // Variables in the POST data.
      PStringStream & replyMessage  // Reply message for post.
    );


    void SetConfigValues();


  protected:
    PString section;
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
