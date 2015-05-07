/*
 * http.h
 *
 * HyperText Transport Protocol classes.
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

#ifndef PTLIB_HTTP_H
#define PTLIB_HTTP_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#if P_HTTP

#include <ptclib/inetprot.h>
#include <ptclib/mime.h>
#include <ptclib/url.h>
#include <ptlib/ipsock.h>
#include <ptlib/pfactory.h>


#include <ptclib/html.h>

//////////////////////////////////////////////////////////////////////////////
// PHTTPSpace

class PHTTPResource;

/** This class describes a name space that a Universal Resource Locator operates
   in. Each section of the hierarchy field of the URL points to a leg in the
   tree specified by this class.
 */
class PHTTPSpace : public PContainer
{
  PCONTAINERINFO(PHTTPSpace, PContainer)
  public:
    /// Constructor for HTTP URL Name Space
    PHTTPSpace();


  // New functions for class.
    enum AddOptions {
      ErrorOnExist, ///< Generate error if resource already exists
      Overwrite     ///< Overwrite the existing resource at URL location
    };


    /** Add a new resource to the URL space. If there is already a resource at
       the location in the tree, or that location in the tree is already in
       the path to another resource then the function will fail.

       The <CODE>overwrite</CODE> flag can be used to replace an existing
       resource. The function will still fail if the resource is on a partial
       path to another resource but not if it is a leaf node.

       @return
       true if resource added, false if failed.
     */
    PBoolean AddResource(
      PHTTPResource * resource, ///< Resource to add to the name space.
      AddOptions overwrite = ErrorOnExist
        ///< Flag to overwrite an existing resource if it already exists.
    );

    /** Delete an existing resource to the URL space. If there is not a resource
       at the location in the tree, or that location in the tree is in the
       path to another resource then the function will fail.

       @return
       true if resource deleted, false if failed.
     */
    PBoolean DelResource(
      const PURL & url          ///< URL to search for in the name space.
    );

    /** Locate the resource specified by the URL in the URL name space.

       @return
       The resource found or NULL if no resource at that position in hiearchy.
     */
    PHTTPResource * FindResource(
      const PURL & url   ///< URL to search for in the name space.
    );

    /** This function attempts to acquire the mutex for reading.
     */
    void StartRead() const
      { mutex->StartRead(); }

    /** This function attempts to release the mutex for reading.
     */
    void EndRead() const
      { mutex->EndRead(); }

    /** This function attempts to acquire the mutex for writing.
     */
    void StartWrite() const
      { mutex->StartWrite(); }

    /** This function attempts to release the mutex for writing.
     */
    void EndWrite() const
      { mutex->EndWrite(); }


  protected:
    PReadWriteMutex * mutex;

    class Node;
    PSORTED_LIST(ChildList, Node);
    class Node : public PString
    {
      PCLASSINFO(Node, PString)
      public:
        Node(const PString & name, Node * parentNode);
        ~Node();

        Node          * parent;
        ChildList       children;
        PHTTPResource * resource;
    } * root;

  private:
    PBoolean SetSize(PINDEX) { return false; }
};

#ifdef TRACE
#undef TRACE
#endif

//////////////////////////////////////////////////////////////////////////////
// PHTTP

/** A common base class for TCP/IP socket for the HyperText Transfer Protocol
version 1.0 client and server.
 */
class PHTTP : public PInternetProtocol
{
  PCLASSINFO(PHTTP, PInternetProtocol)

  public:
  // New functions for class.
    enum Commands {
      // HTTP/1.0 commands
      GET, HEAD, POST,
      // HTTP/1.1 commands
      PUT, DELETE, TRACE, OPTIONS,
      // HTTPS command
      CONNECT,
      NumCommands
    };

    enum StatusCode {
      BadResponse = 1,             ///<   1 = response is malformed
      ContentProcessorError,       ///< Content Processer returned error
      UnknownTransferEncoding,     ///< Unknown "Transfer-Encoding" extension
      TransportConnectError,       ///< Could not connect transport
      TransportWriteError,         ///< Could not write transport
      TransportReadError,          ///< Could not read transport
      Continue = 100,              ///< 100 - Continue
      SwitchingProtocols,          ///< 101 - upgrade allowed
      RequestOK = 200,             ///< 200 - request has succeeded
      Created,                     ///< 201 - new resource created: entity body contains URL
      Accepted,                    ///< 202 - request accepted, but not yet completed
      NonAuthoritativeInformation, ///< 203 - not definitive entity header
      NoContent,                   ///< 204 - no new information
      ResetContent,                ///< 205 - contents have been reset
      PartialContent,              ///< 206 - partial GET succeeded
      MultipleChoices = 300,       ///< 300 - requested resource available elsewehere 
      MovedPermanently,            ///< 301 - resource moved permanently: location field has new URL
      MovedTemporarily,            ///< 302 - resource moved temporarily: location field has new URL
      SeeOther,                    ///< 303 - see other URL
      NotModified,                 ///< 304 - document has not been modified
      UseProxy,                    ///< 305 - proxy redirect
      BadRequest = 400,            ///< 400 - request malformed or not understood
      UnAuthorised,                ///< 401 - request requires authentication
      PaymentRequired,             ///< 402 - reserved 
      Forbidden,                   ///< 403 - request is refused due to unsufficient authorisation
      NotFound,                    ///< 404 - resource cannot be found
      MethodNotAllowed,            ///< 405 - not allowed on this resource
      NoneAcceptable,              ///< 406 - encoding not acceptable
      ProxyAuthenticationRequired, ///< 407 - must authenticate with proxy first
      RequestTimeout,              ///< 408 - server timeout on request
      Conflict,                    ///< 409 - resource conflict on action
      Gone,                        ///< 410 - resource gone away
      LengthRequired,              ///< 411 - no Content-Length
      UnlessTrue,                  ///< 412 - no Range header for true Unless
      InternalServerError = 500,   ///< 500 - server has encountered an unexpected error
      NotImplemented,              ///< 501 - server does not implement request
      BadGateway,                  ///< 502 - error whilst acting as gateway
      ServiceUnavailable,          ///< 503 - server temporarily unable to service request
      GatewayTimeout               ///< 504 - timeout whilst talking to gateway
    };

    // Common MIME header tags
    static const PCaselessString & HostTag();
    static const PCaselessString & AllowTag();
    static const PCaselessString & AuthorizationTag();
    static const PCaselessString & ContentEncodingTag();
    static const PCaselessString & ContentLengthTag();
    static const PCaselessString & ContentTypeTag() { return PMIMEInfo::ContentTypeTag(); }
    static const PCaselessString & DateTag();
    static const PCaselessString & ExpiresTag();
    static const PCaselessString & FromTag();
    static const PCaselessString & IfModifiedSinceTag();
    static const PCaselessString & LastModifiedTag();
    static const PCaselessString & LocationTag();
    static const PCaselessString & PragmaTag();
    static const PCaselessString & PragmaNoCacheTag();
    static const PCaselessString & RefererTag();
    static const PCaselessString & ServerTag();
    static const PCaselessString & UserAgentTag();
    static const PCaselessString & WWWAuthenticateTag();
    static const PCaselessString & MIMEVersionTag();
    static const PCaselessString & ConnectionTag();
    static const PCaselessString & KeepAliveTag();
    static const PCaselessString & UpgradeTag();
    static const PCaselessString & WebSocketTag();
    static const PCaselessString & WebSocketKeyTag();
    static const PCaselessString & WebSocketAcceptTag();
    static const PCaselessString & WebSocketProtocolTag();
    static const PCaselessString & WebSocketVersionTag();
    static const PCaselessString & TransferEncodingTag();
    static const PCaselessString & ChunkedTag();
    static const PCaselessString & ProxyConnectionTag();
    static const PCaselessString & ProxyAuthorizationTag();
    static const PCaselessString & ProxyAuthenticateTag();
    static const PCaselessString & ForwardedTag();
    static const PCaselessString & SetCookieTag();
    static const PCaselessString & CookieTag();

  protected:
    /** Create a TCP/IP HTTP protocol channel.
     */
    PHTTP();
    PHTTP(
      const char * defaultServiceName  ///< Service name for the protocol.
    );

    /** Parse a response line string into a response code and any extra info
       on the line. Results are placed into the member variables
       <CODE>lastResponseCode</CODE> and <CODE>lastResponseInfo</CODE>.

       The default bahaviour looks for a space or a '-' and splits the code
       and info either side of that character, then returns false.

       @return
       Position of continuation character in response, 0 if no continuation
       lines are possible.
     */
    virtual PINDEX ParseResponse(
      const PString & line    ///< Input response line to be parsed
    );
};


class PHTTPContentProcessor
{
public:
  PHTTPContentProcessor(bool reader) : m_reader(reader) { }
  virtual ~PHTTPContentProcessor() { }
  virtual void * GetBuffer(PINDEX & size) = 0;
  virtual bool Process(const void * /*data*/, PINDEX /*length*/) { return true; }
  virtual void Reset() { }
protected:
  bool m_reader;
};


class PHTTPClientAuthentication : public PObject
{
  PCLASSINFO(PHTTPClientAuthentication, PObject);
  public:
    class AuthObject {
      public:
        virtual ~AuthObject() { }
        virtual PMIMEInfo & GetMIME() = 0;
        virtual PString GetURI() = 0;
        virtual PString GetEntityBody() { return PString::Empty(); }
        virtual PHTTPContentProcessor * GetContentProcessor() { return NULL; }
        virtual PString GetMethod() = 0;
    };

    PHTTPClientAuthentication();

    virtual Comparison Compare(
      const PObject & other
    ) const;

    virtual PBoolean Parse(
      const PString & auth,
      PBoolean proxy
    ) = 0;

    virtual PBoolean Authorise(
      AuthObject & pdu
    ) const =  0;

    virtual PBoolean IsProxy() const               { return isProxy; }

    virtual PString GetUsername() const   { return username; }
    virtual PString GetPassword() const   { return password; }
    virtual PString GetAuthRealm() const  { return PString::Empty(); }

    virtual void SetUsername(const PString & user) { username = user; }
    virtual void SetPassword(const PString & pass) { password = pass; }
    virtual void SetAuthRealm(const PString &)     { }

    PString GetAuthParam(const PString & auth, const char * name) const;
    PString AsHex(PMessageDigest5::Code & digest) const;
    PString AsHex(const PBYTEArray & data) const;

    static PHTTPClientAuthentication * ParseAuthenticationRequired(bool isProxy, const PMIMEInfo & line, PString & errorMsg);


  protected:
    PBoolean  isProxy;
    PString   username;
    PString   password;
};

typedef PFactory<PHTTPClientAuthentication> PHTTPClientAuthenticationFactory;

class PHTTPClientAuthenticator : public PHTTPClientAuthentication::AuthObject
{
  public:
    PHTTPClientAuthenticator(
      const PString & cmdName, 
      const PString & uri, 
      PMIMEInfo & mime, 
      PHTTPContentProcessor & processor
    );
    virtual PMIMEInfo & GetMIME();
    virtual PString GetURI();
    virtual PHTTPContentProcessor * GetContentProcessor();
    virtual PString GetMethod();
  protected:
    PString m_method;
    PString m_uri;
    PMIMEInfo & m_mime;
    PHTTPContentProcessor & m_contentProcessor;
};

///////////////////////////////////////////////////////////////////////

class PHTTPClientBasicAuthentication : public PHTTPClientAuthentication
{
  PCLASSINFO(PHTTPClientBasicAuthentication, PHTTPClientAuthentication);
  public:
    PHTTPClientBasicAuthentication();

    virtual Comparison Compare(
      const PObject & other
    ) const;

    virtual PBoolean Parse(
      const PString & auth,
      PBoolean proxy
    );

    virtual PBoolean Authorise(
      AuthObject & pdu
    ) const;
};

///////////////////////////////////////////////////////////////////////

class PHTTPClientDigestAuthentication : public PHTTPClientAuthentication
{
  PCLASSINFO(PHTTPClientDigestAuthentication, PHTTPClientAuthentication);
  public:
    PHTTPClientDigestAuthentication();

    PHTTPClientDigestAuthentication & operator =(
      const PHTTPClientDigestAuthentication & auth
    );

    virtual Comparison Compare(
      const PObject & other
    ) const;

    virtual PBoolean Parse(
      const PString & auth,
      PBoolean proxy
    );

    virtual PBoolean Authorise(
      AuthObject & pdu
    ) const;

    virtual PString GetAuthRealm() const         { return authRealm; }
    virtual void SetAuthRealm(const PString & r) { authRealm = r; }

    enum Algorithm {
      Algorithm_MD5,
      NumAlgorithms
    };
    const PString & GetNonce() const       { return nonce; }
    Algorithm GetAlgorithm() const         { return algorithm; }
    const PString & GetOpaque() const      { return opaque; }
    bool GetStale() const                  { return stale; }

  protected:
    PString   authRealm;
    PString   nonce;
    Algorithm algorithm;
    PString   opaque;

    bool    qopAuth;
    bool    qopAuthInt;
    bool    stale;
    PString cnonce;
    mutable atomic<uint32_t> nonceCount;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPClient

/** A TCP/IP socket for the HyperText Transfer Protocol version 1.0.

   When acting as a client, the procedure is to make the connection to a
   remote server, then to retrieve a document using the following procedure:
      <PRE><CODE>
      PHTTPSocket web("webserver");
      if (web.IsOpen()) {
        PINDEX len;
        if (web.GetDocument("http://www.someone.com/somewhere/url", len)) {
          PString html = web.ReadString(len);
          if (!html.IsEmpty())
            ProcessHTML(html);
        }
        else
           PError << "Could not get page." << endl;
      }
      else
         PError << "HTTP conection failed." << endl;
      </CODE></PRE>
 */
class PHTTPClient : public PHTTP
{
  PCLASSINFO(PHTTPClient, PHTTP)

  public:
    /// Create a new HTTP client channel.
    PHTTPClient(
      const PString & userAgentName = PString::Empty()
    );


  // New functions for class.
    /// Connect at transport level to remote, based on URL
    bool ConnectURL(
      const PURL & url
    );

    /// Call back to process the body of the HTTP command
    typedef PHTTPContentProcessor ContentProcessor;

    /** Send a command and wait for the response header (including MIME fields).
       Note that a body may still be on its way even if lasResponseCode is not
       200!

       @return
       true if all of header returned and ready to receive body.
     */
    int ExecuteCommand(
      Commands cmd,
      const PURL & url,
      PMIMEInfo & outMIME,
      const PString & dataBody,
      PMIMEInfo & replyMime
    );
    int ExecuteCommand(
      Commands cmd,
      const PURL & url,
      PMIMEInfo & outMIME,
      const PBYTEArray & dataBody,
      PMIMEInfo & replyMime
    );
    int ExecuteCommand(
      Commands cmd,
      const PURL & url,
      PMIMEInfo & outMIME,
      ContentProcessor & processor,
      PMIMEInfo & replyMime
    );

    /// Write a HTTP command to server
    bool WriteCommand(
      Commands cmd,
      const PString & url,
      PMIMEInfo & outMIME,
      ContentProcessor & processor
    );

    /// Read a response from the server
    bool ReadResponse(
      PMIMEInfo & replyMIME
    );

    /// Read the body of the HTTP command
    bool ReadContentBody(
      PMIMEInfo & replyMIME,        ///< Reply MIME from server
      ContentProcessor & processor  ///< Processor for received body
    );

    /// Read the body of the HTTP command, throwing it away
    bool ReadContentBody(
      PMIMEInfo & replyMIME   ///< Reply MIME from server
    );

    /// Read the body of the HTTP command as a string
    bool ReadContentBody(
      PMIMEInfo & replyMIME,        ///< Reply MIME from server
      PString & body                ///< Received body as a string
    );

    /// Read the body of the HTTP command as a binary block
    bool ReadContentBody(
      PMIMEInfo & replyMIME,        ///< Reply MIME from server
      PBYTEArray & body             ///< Received body as binary data
    );


    /** Start getting the document specified by the URL.
        This does not return until completed, and continuously calls the
        ContentProcessor with chunks of data.

       @return true if document has successfully been transferred.
     */
    bool GetDocument(
      const PURL & url,             ///< Universal Resource Locator for document.
      ContentProcessor & processor  ///< Processor for received body
    );

    /** Start getting the document specified by the URL.
        It is expected that ReadContentBody() is called after this returns to
        get, or throw away, the data body received.

       @return true if document is being transferred.
     */
    bool GetDocument(
      const PURL & url,         ///< Universal Resource Locator for document.
      PMIMEInfo & outMIME,      ///< MIME info in request
      PMIMEInfo & replyMIME     ///< MIME info in response
    );
    bool GetDocument(
      const PURL & url,         ///< Universal Resource Locator for document.
      PMIMEInfo & replyMIME     ///< MIME info in response
    );


    /** Get the document specified by the URL.

        An empty string for the contentType parameter means that any content
        type is acceptable.

        @return true if document is being transferred.
     */
    bool GetTextDocument(
      const PURL & url,         ///< Universal Resource Locator for document.
      PString & document,       ///< Body read
      const PString & contentType = PString::Empty() ///< Content-Type header to expect
    );

    /** Get the document specified by the URL.

        An empty string for the contentType parameter means that any content
        type is acceptable.

        @return true if document is being transferred.
     */
    bool GetBinaryDocument(
      const PURL & url,         ///< Universal Resource Locator for document.
      PBYTEArray & document,    ///< Body read
      const PString & contentType = PString::Empty() ///< Content-Type header to expect
    );


    /** Get the header for the document specified by the URL.

       @return
       true if document header is being transferred.
     */
    bool GetHeader(
      const PURL & url,         ///< Universal Resource Locator for document.
      PMIMEInfo & outMIME,      ///< MIME info in request
      PMIMEInfo & replyMIME     ///< MIME info in response
    );
    bool GetHeader(
      const PURL & url,         ///< Universal Resource Locator for document.
      PMIMEInfo & replyMIME     ///< MIME info in response
    );


    /** Post the "application/x-www-form-urlencoded" data specified to the URL.
        The data returned is thrown away.

       @return true if document is being transferred.
     */
    bool PostData(
      const PURL & url,         ///< Universal Resource Locator for document.
      const PStringToString & data  ///< Information posted to the HTTP server.
    );

    /** Post the data specified to the URL.
        The data returned is thrown away.

       @return true if document is being transferred.
     */
    bool PostData(
      const PURL & url,         ///< Universal Resource Locator for document.
      PMIMEInfo & outMIME,      ///< MIME info in request
      const PString & data    ///< Information posted to the HTTP server.
    );

    /** Start a post of the data specified to the URL.
        It is expected that ReadContentBody() is called after this returns to
        get, or throw away, the data body received.

       @return true if document is being transferred.
     */
    bool PostData(
      const PURL & url,       ///< Universal Resource Locator for document.
      PMIMEInfo & outMIME,    ///< MIME info in request
      const PString & data,   ///< Information posted to the HTTP server.
      PMIMEInfo & replyMIME   ///< MIME info in response
    );

    /** Post the data specified to the URL.
        The body of the reply is returned

       @return true if document is has been transferred.
     */
    bool PostData(
      const PURL & url,       ///< Universal Resource Locator for document.
      PMIMEInfo & outMIME,    ///< MIME info in request
      const PString & data,   ///< Information posted to the HTTP server.
      PMIMEInfo & replyMIME,  ///< MIME info in response
      PString & replyBody     ///< Body of response
    );

    /** Put the document specified by the URL.

       @return
       true if document is being transferred.
     */
    bool PutDocument(
      const PURL & url,           ///< Universal Resource Locator for document.
      const PString & document,  ///< Body to write
      const PString & contentType = PMIMEInfo::TextPlain() ///< Content-Type header to use
    );

    bool PutTextDocument(
      const PURL & url,           ///< Universal Resource Locator for document.
      const PString & document,  ///< Body to write
      const PString & contentType = PMIMEInfo::TextPlain() ///< Content-Type header to use
    ) { return PutDocument(url, document, contentType); }

    /** Put the document specified by the URL.

       @return
       true if document is being transferred.
     */
    bool PutDocument(
      const PURL & url,             ///< Universal Resource Locator for document.
      const PBYTEArray & document,  ///< Body to write
      const PString & contentType   ///< Content-Type header to use
    );

    /** Put the document specified by the URL.

       @return
       true if document is being transferred.
     */
    bool PutDocument(
      const PURL & url,             ///< Universal Resource Locator for document.
      const PFilePath & document,   ///< Body to write
      const PString & contentType = PString::Empty() ///< Content-Type header to use
    );

    /** Put the document specified by the URL.

       @return
       true if document is being transferred.
     */
    bool PutDocument(
      const PURL & url,         ///< Universal Resource Locator for document.
      PMIMEInfo & outMIME,      ///< MIME info in request
      PMIMEInfo & replyMIME     ///< MIME info in response
    );

    /** Delete the document specified by the URL.

       @return
       true if document is deleted.
     */
    bool DeleteDocument(
      const PURL & url        ///< Universal Resource Locator for document.
    );

    /** Set authentication paramaters to be use for retreiving documents
    */
    void SetAuthenticationInfo(
      const PString & userName,
      const PString & password
    );

#if P_SSL
    void SetSSLCredentials(
      const PString & authority,
      const PString & certificate,
      const PString & privateKey
    );
#endif

    /// Set persistent connection mode
    void SetPersistent(
      bool persist = true
    ) { m_persist = persist; }

    /// Get persistent connection mode
    bool GetPersistent() const { return m_persist; }

#if PTRACING
    static PINDEX MaxTraceContentSize;
#endif

  protected:
    PString m_userAgentName;
    bool    m_persist;
    PString m_userName;
    PString m_password;
#if P_SSL
    PString m_authority;    // Directory, file or data
    PString m_certificate;  // File or data
    PString m_privateKey;   // File or data
#endif
    PHTTPClientAuthentication * m_authentication;
};


//////////////////////////////////////////////////////////////////////////////
// PWebSocket

#if P_SSL

/** This channel reads from a channel (usually socket) that is under the RFC6455
    framing rules.
    
    Note the WebSocket handshake is assumed to have already occurred.
*/

class PWebSocket : public PIndirectChannel
{
    PCLASSINFO(PWebSocket, PIndirectChannel)
  public:
    /// Create a new WebSocket channel.
    PWebSocket();

  // Overrides from PChannel
    /** Low level read from the channel.

        @return
        true indicates that at least one character was read from the channel.
        false means no bytes were read due to timeout or some other I/O error.
    */
    virtual PBoolean Read(
      void * buf,   ///< Pointer to a block of memory to receive the read bytes.
      PINDEX len    ///< Maximum number of bytes to read into the buffer.
    );

    /** Low level write to the channel.
        This function will always write a single WebSocket "frame".

        If m_fragmenting is false, a single write will constitute a single
        WebSocket "message" as well. If true, then this will be a fragment of
        the message. The user should call SetFragmenting(false) before sending
        the last part of the message.

        @return
        true if at least len bytes were written to the channel.
    */
    virtual PBoolean Write(
      const void * buf, ///< Pointer to a block of memory to write.
      PINDEX len        ///< Number of bytes to write.
    );


    /** Connect to the WebSocket.
        This performs the HTTP handshake for the WebSocket establishment.
      */
    bool Connect(
      const PStringArray & protocols,    ///< WebSocket sub-protocol to use.
      PString * selectedProtocol = NULL  ///< Selected protocol by server
    );


    /// Read a complete WebSocket message
    virtual bool ReadMessage(
      PBYTEArray & msg
    );

    /// Indicate the last Read() completed the WebSocket message.
    bool IsMessageComplete() const { return m_fragmentedRead && m_remainingPayload == 0; }

    /** Indicate Write() calls are fragments of a large or indeterminate
        message. The user should call SetFragmenting(false) before sending
        the last part of the message.

        Default is true, so every Write() call is a single message.
      */
    void SetWriteFragmentation(
      bool frag
    ) { m_fragmentingWrite = frag; }

    /// Set writing binary data.
    void SetBinaryMode(
      bool bin = true
    ) { m_binaryWrite = bin; }

    /// Set writing text data.
    void SetTextMode(
      bool txt = true
    ) { m_binaryWrite = !txt; }


  protected:
    enum OpCodes
    {
      Continuation = 0x0,
      TextFrame = 0x1,
      BinaryFrame = 0x2,
      ConnectionClose = 0x8,
      Ping = 0x9,
      Pong = 0xA
    };

    virtual bool ReadHeader(
      OpCodes  & opCode,
      bool     & fragment,
      uint64_t & payloadLength,
      int64_t  & masking
    );

    virtual bool WriteHeader(
      OpCodes  opCode,
      bool     fragment,
      uint64_t payloadLength,
      int64_t  masking
    );

    bool WriteMasked(
      const uint32_t * data,
      PINDEX len,
      uint32_t mask
    );

    bool     m_client;
    bool     m_fragmentingWrite;
    bool     m_binaryWrite;

    uint64_t m_remainingPayload;
    int64_t  m_currentMask;
    bool     m_fragmentedRead;

    bool     m_recursiveRead;
};

#endif // P_SSL


//////////////////////////////////////////////////////////////////////////////
// PHTTPConnectionInfo

class PHTTPServer;

/** This object describes the connectiono associated with a HyperText Transport
   Protocol request. This information is required by handler functions on
   <code>PHTTPResource</code> descendant classes to manage the connection correctly.
*/
class PHTTPConnectionInfo : public PObject
{
  PCLASSINFO(PHTTPConnectionInfo, PObject)
  public:
    PHTTPConnectionInfo();

    PHTTP::Commands GetCommandCode() const { return commandCode; }
    const PString & GetCommandName() const { return commandName; }

    const PURL & GetURL() const       { return url; }

    const PMIMEInfo & GetMIME() const { return mimeInfo; }
    void SetMIME(const PString & tag, const PString & value);

    PBoolean IsCompatible(int major, int minor) const;

    bool IsPersistent() const         { return isPersistent; }
    bool WasPersistent() const        { return wasPersistent; }
    bool IsProxyConnection() const    { return isProxyConnection; }
    int  GetMajorVersion() const      { return majorVersion; }
    int  GetMinorVersion() const      { return minorVersion; }

    long GetEntityBodyLength() const  { return entityBodyLength; }

    /**Get the maximum time a persistent connection may persist.
      */
    PTimeInterval GetPersistenceTimeout() const { return persistenceTimeout; }

    /**Set the maximum time a persistent connection may persist.
      */
    void SetPersistenceTimeout(const PTimeInterval & t) { persistenceTimeout = t; }

    /**Get the maximum number of transations (GET/POST etc) for persistent connection.
       If this is zero then there is no maximum.
      */
    unsigned GetPersistenceMaximumTransations() const { return persistenceMaximum; }

    /**Set the maximum number of transations (GET/POST etc) for persistent connection.
       If this is zero then there is no maximum.
      */
    void SetPersistenceMaximumTransations(unsigned m) { persistenceMaximum = m; }

    const PMultiPartList & GetMultipartFormInfo() const
      { return m_multipartFormInfo; }

    void ResetMultipartFormInfo()
      { m_multipartFormInfo.RemoveAll(); }

    PString GetEntityBody() const   { return entityBody; }

    bool IsWebSocket() const { return m_isWebSocket; }
    void ClearWebSocket() { m_isWebSocket = false; }

  protected:
    PBoolean Initialise(PHTTPServer & server, PString & args);
    bool DecodeMultipartFormInfo() { return mimeInfo.DecodeMultiPartList(m_multipartFormInfo, entityBody); }

    PHTTP::Commands commandCode;
    PString         commandName;
    PURL            url;
    PMIMEInfo       mimeInfo;
    bool            isPersistent;
    bool            wasPersistent;
    bool            isProxyConnection;
    bool            m_isWebSocket;
    int             majorVersion;
    int             minorVersion;
    PString         entityBody;        // original entity body (POST only)
    long            entityBodyLength;
    PTimeInterval   persistenceTimeout;
    unsigned        persistenceMaximum;
    PMultiPartList  m_multipartFormInfo;

  friend class PHTTPServer;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPServer

/** A TCP/IP socket for the HyperText Transfer Protocol version 1.0.

    The server socket thread would continuously call the
    ProcessCommand() function until it returns false. This will then
    call the appropriate virtual function on parsing the HTTP protocol.
    <PRE><CODE>
    PTCPSocket socket(80);
    if (!socket.Listen())
      return;

    PHTTPSpace httpNameSpace;
    httpNameSpace.AddResource(new PHTTPDirectory("data", "data"))

    PHTTServer httpServer(httpNameSpace);
    if (!httpServer.Open(socket))
      return;

    while (httpServer.ProcessCommand())
      ;
    </CODE></PRE>
 */
class PHTTPServer : public PHTTP
{
  PCLASSINFO(PHTTPServer, PHTTP)

  public:
    /** Create a TCP/IP HTTP protocol socket channel. The form with the single
       <CODE>port</CODE> parameter creates an unopened socket, the form with
       the <CODE>address</CODE> parameter makes a connection to a remote
       system, opening the socket. The form with the <CODE>socket</CODE>
       parameter opens the socket to an incoming call from a "listening"
       socket.
     */
    PHTTPServer();
    PHTTPServer(
     const PHTTPSpace & urlSpace  ///< Name space to use for URLs received.
    );


  // New functions for class.
    /** Get the name of the server.

       @return
       String name of the server.
     */
    virtual PString GetServerName() const;

    /** Get the name space being used by the HTTP server socket.

       @return
       URL name space tree.
     */
    PHTTPSpace & GetURLSpace() { return urlSpace; }

    /// Use a new URL name space for this HTTP socket.
    void SetURLSpace(
      const PHTTPSpace & space   ///< New URL name space to use.
    );


    /** Process commands, dispatching to the appropriate virtual function. This
       is used when the socket is acting as a server.

       @return
       true if the request specified persistent mode and the request version
       allows it, false if the socket closed, timed out, the protocol does not
       allow persistent mode, or the client did not request it
       timed out
     */
    virtual PBoolean ProcessCommand();

    /** Handle a GET command from a client.

       The default implementation looks up the URL in the name space declared by
       the <code>PHTTPSpace</code> class tree and despatches to the
       <code>PHTTPResource</code> object contained therein.

       @return
       true if the connection may persist, false if the connection must close
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
     */
    virtual bool OnGET(
      const PHTTPConnectionInfo & conInfo  ///< HTTP connection information
    );

    /** Handle a HEAD command from a client.

       The default implemetation looks up the URL in the name space declared by
       the <code>PHTTPSpace</code> class tree and despatches to the
       <code>PHTTPResource</code> object contained therein.

       @return
       true if the connection may persist, false if the connection must close
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
     */
    virtual bool OnHEAD(
      const PHTTPConnectionInfo & conInfo ///< HTTP connection information
    );

    /** Handle a POST command from a client.

       The default implementation looks up the URL in the name space declared by
       the <code>PHTTPSpace</code> class tree and despatches to the
       <code>PHTTPResource</code> object contained therein.

       @return
       true if the connection may persist, false if the connection must close
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
     */
    virtual bool OnPOST(
      const PHTTPConnectionInfo & conInfo ///< HTTP connection information
    );

    /** Handle a proxy command request from a client. This will only get called
       if the request was not for this particular server. If it was a proxy
       request for this server (host and port number) then the appropriate
       <code>OnGET()</code>, <code>OnHEAD()</code> or <code>OnPOST()</code> command is called.

       The default implementation returns OnError(BadGateway).

       @return
       true if the connection may persist, false if the connection must close
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
     */
    virtual PBoolean OnProxy(
      const PHTTPConnectionInfo & conInfo   ///<  HTTP connection information
    );


    /** Read the entity body associated with a HTTP request, and close the
       socket if not a persistent connection.

       @return
       The entity body of the command
     */
    virtual PString ReadEntityBody();

    /** Handle an unknown command.

       @return
       true if the connection may persist, false if the connection must close
     */
    virtual PBoolean OnUnknown(
      const PCaselessString & command,         ///< Complete command line received.
      const PHTTPConnectionInfo & connectInfo  ///< HTTP connection information
    );

    /** Write a command reply back to the client, and construct some of the
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

       @return
       true if requires v1.1 chunked transfer encoding.
     */
    PBoolean StartResponse(
      StatusCode code,      ///< Status code for the response.
      PMIMEInfo & headers,  ///< MIME variables included in response.
      long bodySize         ///< Size of the rest of the response.
    );

    /** Write an error response for the specified code.

       Depending on the <CODE>code</CODE> parameter this function will also
       send a HTML version of the status code for display on the remote client
       viewer.

       @return
       true if the connection may persist, false if the connection must close
     */
    virtual PBoolean OnError(
      StatusCode code,                         ///< Status code for the error response.
      const PCaselessString & extra,           ///< Extra information included in the response.
      const PHTTPConnectionInfo & connectInfo  ///< HTTP connection information
    );

    /** Set the default mime info
     */
    void SetDefaultMIMEInfo(
      PMIMEInfo & info,      ///< Extra MIME information in command.
      const PHTTPConnectionInfo & connectInfo   ///< Connection information
    );

    /**Get the connection info for this connection.
      */
    PHTTPConnectionInfo & GetConnectionInfo() { return connectInfo; }

    /**Called when a request is received. 
       By default, this intercepts the GET, HEAD and POST commands
       and calls OnGET, OnHEAD and OnPOST. For all other command,
       OnUnknown is called
       */
    virtual bool OnCommand(
      PINDEX cmd,
      const PURL & url, 
      const PString & args,
      PHTTPConnectionInfo & connectInfo
    );

    /**Called when a request indicates a swtch to WebSocket protocol.
       This will complete the WebSocket handshake if m_webSocketNotifiers
       contains and entry for an offerred protocol. If none, a 404 is
       returned and the socket remains in HTTP mode.

       After the start up handshake has completed, if there is a notifier and
       it is non-null, then the notifier is called. The notifier, may then call
       connectInfo.ClearWebSocket() if it wishes to return to HTTP mode. That
       is, ProcessCommand() will return true and the normal persistence rules
       apply. If ClearWebSocket() is not called then ProcessCommand() will
       return false.

       Note, that the latter case does not mean that the underlying socket is
       necessarily closed, it could subsequently be disconnected from the
       PHTTPServer objects and passed to PWebSocket for processing.

       @return true if want to persist with HTTP connection.
      */
    virtual bool OnWebSocket(
      PHTTPConnectionInfo & connectInfo
    );

    typedef PNotifierTemplate<PHTTPConnectionInfo> WebSocketNotifier;

    /** Set the handler for WebSocket sub-protocol.
        A NULL notifier may be set which indiates that the protocol is
        supported, but that ProcessCommand() is to simply return false as
        the caller will handle the channel from then on.
    */
    void SetWebSocketNotifier(
      const PString & protocol,
      const WebSocketNotifier & notifier
    );

    /// Remove a WebSocket notifier
    void ClearWebSocketNotifier(
      const PString & protocol
    );

  protected:
    void Construct();

    PHTTPSpace          urlSpace;
    PHTTPConnectionInfo connectInfo;
    unsigned            transactionCount;
    PTimeInterval       nextTimeout;

    std::map<PString, WebSocketNotifier> m_webSocketNotifiers;

    P_REMOVE_VIRTUAL(PBoolean,OnGET(const PURL&,const PMIMEInfo&, const PHTTPConnectionInfo&),false);
    P_REMOVE_VIRTUAL(PBoolean,OnHEAD(const PURL&,const PMIMEInfo&,const PHTTPConnectionInfo&),false);
    P_REMOVE_VIRTUAL(PBoolean,OnPOST(const PURL&,const PMIMEInfo&,const PStringToString&,const PHTTPConnectionInfo&),false);
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPRequest

/** This object describes a HyperText Transport Protocol request. An individual
   request is passed to handler functions on <code>PHTTPResource</code> descendant
   classes.
 */
class PHTTPRequest : public PObject
{
  PCLASSINFO(PHTTPRequest, PObject)

  public:
    PHTTPRequest(
      const PURL & url,             ///< Universal Resource Locator for document.
      const PMIMEInfo & inMIME,     ///< Extra MIME information in command.
      const PMultiPartList & multipartFormInfo, ///< multipart form information (if any)
      PHTTPResource * resource,     ///< Resource associated with request
      PHTTPServer & server          ///< Server channel that request initiated on
    );

    PHTTPServer & server;           ///< Server channel that request initiated on
    const PURL & url;               ///< Universal Resource Locator for document.
    const PMIMEInfo & inMIME;       ///< Extra MIME information in command.
    const PMultiPartList & multipartFormInfo; ///< multipart form information, if any
    PHTTP::StatusCode code;         ///< Status code for OnError() reply.
    PMIMEInfo outMIME;              ///< MIME information used in reply.
    PString entityBody;             ///< original entity body (POST only)
    PINDEX contentSize;             ///< Size of the body of the resource data.
    PIPSocket::Address origin;      ///< IP address of origin host for request
    PIPSocket::Address localAddr;   ///< IP address of local interface for request
    WORD               localPort;   ///< Port number of local server for request
    PHTTPResource    * m_resource;  ///< HTTP resource found for the request
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPAuthority

/** This abstract class describes the authorisation mechanism for a Universal
   Resource Locator.
 */
class PHTTPAuthority : public PObject
{
  PCLASSINFO(PHTTPAuthority, PObject)

  public:
  // New functions for class.
    /** Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       @return
       String for the authorisation realm name.
     */
    virtual PString GetRealm(
      const PHTTPRequest & request   ///< Request information.
    ) const = 0;

    /** Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       @return
       true if the user and password are authorised in the realm.
     */
    virtual PBoolean Validate(
      const PHTTPRequest & request,  ///< Request information.
      const PString & authInfo       ///< Authority information string.
    ) const = 0;

    /** Determine if the authorisation is to be applied. This could be used to
       distinguish between net requiring authorisation and requiring autorisation
       but having no password.

       The default behaviour is to return true.

       @return
       true if the authorisation in the realm is to be applied.
     */
    virtual PBoolean IsActive() const;

  protected:
    static void DecodeBasicAuthority(
      const PString & authInfo,   ///< Authority information string.
      PString & username,         ///< User name decoded from authInfo
      PString & password          ///< Password decoded from authInfo
    );
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPSimpleAuth

/** This class describes the simplest authorisation mechanism for a Universal
   Resource Locator, a fixed realm, username and password.
 */
class PHTTPSimpleAuth : public PHTTPAuthority
{
  PCLASSINFO(PHTTPSimpleAuth, PHTTPAuthority)

  public:
    PHTTPSimpleAuth(
      const PString & realm,      ///< Name space for the username and password.
      const PString & username,   ///< Username that this object wiull authorise.
      const PString & password    ///< Password for the above username.
    );
    // Construct the simple authorisation structure.


  // Overrides from class PObject.
    /** Create a copy of the class on the heap. This is used by the
       <code>PHTTPResource</code> classes for maintaining authorisation to
       resources.

       @return
       pointer to new copy of the class instance.
     */
    virtual PObject * Clone() const;


  // Overrides from class PHTTPAuthority.
    /** Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       @return
       String for the authorisation realm name.
     */
    virtual PString GetRealm(
      const PHTTPRequest & request   ///< Request information.
    ) const;

    /** Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       @return
       true if the user and password are authorised in the realm.
     */
    virtual PBoolean Validate(
      const PHTTPRequest & request,  ///< Request information.
      const PString & authInfo       ///< Authority information string.
    ) const;

    /** Determine if the authorisation is to be applied. This could be used to
       distinguish between net requiring authorisation and requiring autorisation
       but having no password.

       The default behaviour is to return true.

       @return
       true if the authorisation in the realm is to be applied.
     */
    virtual PBoolean IsActive() const;

    /** Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       @return
       String for the authorisation realm name.
     */
    const PString & GetLocalRealm() const { return realm; }

    /** Get the user name allocated to this simple authorisation.

       @return
       String for the authorisation user name.
     */
    const PString & GetUserName() const { return username; }

    /** Get the password allocated to this simple authorisation.

       @return
       String for the authorisation password.
     */
    const PString & GetPassword() const { return password; }


  protected:
    PString realm;
    PString username;
    PString password;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPMultiSimpAuth

/** This class describes the simple authorisation mechanism for a Universal
   Resource Locator, a fixed realm, multiple usernames and passwords.
 */
class PHTTPMultiSimpAuth : public PHTTPAuthority
{
  PCLASSINFO(PHTTPMultiSimpAuth, PHTTPAuthority)

  public:
    PHTTPMultiSimpAuth(
      const PString & realm      ///< Name space for the username and password.
    );
    PHTTPMultiSimpAuth(
      const PString & realm,           ///< Name space for the usernames.
      const PStringToString & userList ///< List of usernames and passwords.
    );
    // Construct the simple authorisation structure.


  // Overrides from class PObject.
    /** Create a copy of the class on the heap. This is used by the
       <code>PHTTPResource</code> classes for maintaining authorisation to
       resources.

       @return
       pointer to new copy of the class instance.
     */
    virtual PObject * Clone() const;


  // Overrides from class PHTTPAuthority.
    /** Get the realm or name space for the user authorisation name and
       password as required by the basic authorisation system of HTTP/1.0.

       @return
       String for the authorisation realm name.
     */
    virtual PString GetRealm(
      const PHTTPRequest & request   ///< Request information.
    ) const;

    /** Validate the user and password provided by the remote HTTP client for
       the realm specified by the class instance.

       @return
       true if the user and password are authorised in the realm.
     */
    virtual PBoolean Validate(
      const PHTTPRequest & request,  ///< Request information.
      const PString & authInfo       ///< Authority information string.
    ) const;

    /** Determine if the authirisation is to be applied. This could be used to
       distinguish between net requiring authorisation and requiring autorisation
       but having no password.

       The default behaviour is to return true.

       @return
       true if the authorisation in the realm is to be applied.
     */
    virtual PBoolean IsActive() const;

    /** Get the user name allocated to this simple authorisation.

       @return
       String for the authorisation user name.
     */
    void AddUser(
      const PString & username,   ///< Username that this object wiull authorise.
      const PString & password    ///< Password for the above username.
    );


  protected:
    PString realm;
    PStringToString users;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPResource

/** This object describes a HyperText Transport Protocol resource. A tree of
   these resources are available to the <code>PHTTPServer</code> class.
 */
class PHTTPResource : public PObject
{
  PCLASSINFO(PHTTPResource, PObject)

  protected:
    PHTTPResource(
      const PURL & url               ///< Name of the resource in URL space.
    );
    PHTTPResource(
      const PURL & url,              ///< Name of the resource in URL space.
      const PHTTPAuthority & auth    ///< Authorisation for the resource.
    );
    PHTTPResource(
      const PURL & url,              ///< Name of the resource in URL space.
      const PString & contentType    ///< MIME content type for the resource.
    );
    PHTTPResource(
      const PURL & url,              ///< Name of the resource in URL space.
      const PString & contentType,   ///< MIME content type for the resource.
      const PHTTPAuthority & auth    ///< Authorisation for the resource.
    );
    // Create a new HTTP Resource.


  public:
    virtual ~PHTTPResource();
    // Destroy the HTTP Resource.


  // New functions for class.
    /** Get the URL for this resource.

       @return
       The URL for this resource.
     */
    const PURL & GetURL() const { return baseURL; }

    /**Get relative hot link for this resouce.
      */
    PString GetHotLink() const { return baseURL.AsString(PURL::PathOnly); }

    /** Get the current content type for the resource.

       @return
       string for the current MIME content type.
     */
    const PString & GetContentType() const { return contentType; }

    /** Get the current authority for the resource.

       @return
       Pointer to authority or NULL if unrestricted.
     */

    PHTTPAuthority * GetAuthority() const { return authority; }

    /** Set the current authority for the resource.
     */
    void SetAuthority(
      const PHTTPAuthority & auth      ///< authority to set
    );

    /** Set the current authority for the resource to unrestricted.
     */
    void ClearAuthority();

    /** Get the current hit count for the resource. This is the total number of
       times the resource was asked for by a remote client.

       @return
       Hit count for the resource.
     */
    DWORD GetHitCount() const { return hitCount; }

    /// Clear the hit count for the resource.
    void ClearHitCount() { hitCount = 0; }

    /**Called when a request indicates a swtch to WebSocket protocol.
       This will handle a WebScoket protocol change.

       After the start up handshake has completed, if there is a notifier and
       it is non-null, then the notifier is called. The notifier, may then call
       connectInfo.ClearWebSocket() if it wishes to return to HTTP mode. That
       is, ProcessCommand() will return true and the normal persistence rules
       apply. If ClearWebSocket() is not called then ProcessCommand() will
       return false.

       Note, that the latter case does not mean that the underlying socket is
       necessarily closed, it could subsequently be disconnected from the
       PHTTPServer objects and passed to PWebSocket for processing.

       @return true if want to persist with HTTP connection.
      */
    virtual bool OnWebSocket(
      PHTTPServer & server,       ///< HTTP server that received the request
      PHTTPConnectionInfo & connectInfo   ///< HTTP connection information
    );

    /** Handle the GET command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtuals <code>LoadHeaders()</code> and <code>OnGETData()</code> to get
       a resource to be sent to the socket.

       @return
       true if the connection may persist, false if the connection must close.
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
     */
    virtual bool OnGET(
      PHTTPServer & server,       ///< HTTP server that received the request
      const PHTTPConnectionInfo & conInfo  ///< HTTP connection information
    );
    virtual PBoolean OnGET(
      PHTTPServer & server,       ///< HTTP server that received the request
      const PURL & url,           ///< Universal Resource Locator for document.
      const PMIMEInfo & info,     ///< Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo   ///< HTTP connection information
    );

    /**Send the data associated with a GET command.

       The default action calls <code>SendData()</code>.

       @return
       true if the connection may persist, false if the connection must close.
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
    */
    virtual PBoolean OnGETData(
      PHTTPServer & server,                       ///< HTTP server that received the request
      const PURL & url,                           ///< Universal Resource Locator for document
      const PHTTPConnectionInfo & connectInfo,    ///< HTTP connection information
      PHTTPRequest & request                      ///< request state information
    );

    /** Handle the HEAD command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <code>LoadHeaders()</code> to get the header information to
       be sent to the socket.

       @return
       true if the connection may persist, false if the connection must close
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
     */
    virtual bool OnHEAD(
      PHTTPServer & server,       ///< HTTP server that received the request
      const PHTTPConnectionInfo & conInfo ///< HTTP connection information
    );
    virtual PBoolean OnHEAD(
      PHTTPServer & server,       ///< HTTP server that received the request
      const PURL & url,           ///< Universal Resource Locator for document.
      const PMIMEInfo & info,     ///< Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo  ///< HTTP connection information
    );

    /** Handle the POST command passed from the HTTP socket.

       The default action is to check the authorisation for the resource and
       call the virtual <code>Post()</code> function to handle the data being
       received.

       @return
       true if the connection may persist, false if the connection must close
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
     */
    virtual bool OnPOST(
      PHTTPServer & server,       ///< HTTP server that received the request
      const PHTTPConnectionInfo & conInfo ///< HTTP connection information
    );
    virtual PBoolean OnPOST(
      PHTTPServer & server,         ///< HTTP server that received the request
      const PURL & url,             ///< Universal Resource Locator for document.
      const PMIMEInfo & info,       ///< Extra MIME information in command.
      const PStringToString & data, ///< Variables in the POST data.
      const PHTTPConnectionInfo & conInfo  ///< HTTP connection information
    );

    /**Send the data associated with a POST command.

       The default action calls <code>Post()</code>.

       @return
       true if the connection may persist, false if the connection must close.
       If there is no ContentLength field in the response, this value must
       be false for correct operation.
    */
    virtual PBoolean OnPOSTData(
      PHTTPRequest & request,        ///< request information
      const PStringToString & data   ///< Variables in the POST data.
    );

    /** Check to see if the resource has been modified since the date
       specified.

       @return
       true if has been modified since.
     */
    virtual PBoolean IsModifiedSince(
      const PTime & when    ///< Time to see if modified later than
    );

    /** Get a block of data (eg HTML) that the resource contains.

       @return
       Status of load operation.
     */
    virtual PBoolean GetExpirationDate(
      PTime & when          ///< Time that the resource expires
    );

    /** Create a new request block for this type of resource.

       The default behaviour is to create a new PHTTPRequest instance.

       @return
       Pointer to instance of PHTTPRequest descendant class.
     */
    virtual PHTTPRequest * CreateRequest(
      const PURL & url,                   ///< Universal Resource Locator for document.
      const PMIMEInfo & inMIME,           ///< Extra MIME information in command.
      const PMultiPartList & multipartFormInfo,  ///< additional information for multi-part posts
      PHTTPServer & socket                                ///< socket used for request
    );

    /** Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       @return
       true if all OK, false if an error occurred.
     */
    virtual PBoolean LoadHeaders(
      PHTTPRequest & request    ///<  Information on this request.
    );

    /**Send the data associated with a command.

       The default action is to call the virtual <code>LoadData()</code> to get a
       resource to be sent to the socket.
    */
    virtual void SendData(
      PHTTPRequest & request    ///< information for this request
    );

    /** Get a block of data that the resource contains.

       The default behaviour is to call the <code>LoadText()</code> function and
       if successful, call the <code>OnLoadedText()</code> function.

       @return
       true if there is still more to load.
     */
    virtual PBoolean LoadData(
      PHTTPRequest & request,    ///<  Information on this request.
      PCharArray & data          ///<  Data used in reply.
    );

    /** Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <code>LoadText()</code> or
       <code>LoadData()</code> functions must be overridden for correct operation.

       @return
       String for loaded text.
     */
    virtual PString LoadText(
      PHTTPRequest & request    ///< Information on this request.
    );

    /** This is called after the text has been loaded and may be used to
       customise or otherwise mangle a loaded piece of text. Typically this is
       used with HTML responses.

       The default action for this function is to do nothing.
     */
    virtual void OnLoadedText(
      PHTTPRequest & request,    ///<  Information on this request.
      PString & text             ///<  Data used in reply.
    );

    /** Get a block of data (eg HTML) that the resource contains.

       The default action for this function is to do nothing and return
       success.

       @return
       true if the connection may persist, false if the connection must close
     */
    virtual PBoolean Post(
      PHTTPRequest & request,       ///<  Information on this request.
      const PStringToString & data, ///<  Variables in the POST data.
      PHTML & replyMessage          ///<  Reply message for post.
    );


  protected:
    /** See if the resource is authorised given the mime info
     */
    virtual PBoolean CheckAuthority(
      PHTTPServer & server,               ///<  Server to send response to.
      const PHTTPRequest & request,       ///<  Information on this request.
      const PHTTPConnectionInfo & conInfo ///<  Information on the connection
    );
    static PBoolean CheckAuthority(
                   PHTTPAuthority & authority,
                      PHTTPServer & server,
               const PHTTPRequest & request,
        const PHTTPConnectionInfo & connectInfo
    );


    /** common code for GET and HEAD commands */
    virtual PBoolean OnGETOrHEAD(
      PHTTPServer & server,       ///<  HTTP server that received the request
      const PURL & url,           ///<  Universal Resource Locator for document.
      const PMIMEInfo & info,     ///<  Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo,  ///< Connection information
      PBoolean isGet              ///< Flag indicating is GET or HEAD
    );


    PURL             baseURL;     ///< Base URL for the resource, may accept URLS with a longer hierarchy
    PString          contentType; ///< MIME content type for the resource
    PHTTPAuthority * authority;   ///< Authorisation method for the resource
    volatile DWORD   hitCount;    ///< COunt of number of times resource was accessed.
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPString

/** This object describes a HyperText Transport Protocol resource which is a
   string kept in memory. For instance a pre-calculated HTML string could be
   set in this type of resource.
 */
class PHTTPString : public PHTTPResource
{
  PCLASSINFO(PHTTPString, PHTTPResource)

  public:
    /** Contruct a new simple string resource for the HTTP space. If no MIME
       content type is specified then a default type is "text/html".
     */
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


  // Overrides from class PHTTPResource
    /** Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       @return
       true if all OK, false if an error occurred.
     */
    virtual PBoolean LoadHeaders(
      PHTTPRequest & request    // Information on this request.
    );

    /** Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <code>LoadText()</code> or
       <code>LoadData()</code> functions must be overridden for correct operation.

       @return
       String for loaded text.
     */
    virtual PString LoadText(
      PHTTPRequest & request    // Information on this request.
    );

  // New functions for class.
    /** Get the string for this resource.

       @return
       String for resource.
     */
    const PString & GetString() { return string; }

    /** Set the string to be returned by this resource.
     */
    void SetString(
      const PString & str   // New string for the resource.
    ) { string = str; }


  protected:
    PString string;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

/** This object describes a HyperText Transport Protocol resource which is a
   single file. The file can be anywhere in the file system and is mapped to
   the specified URL location in the HTTP name space defined by the
   <code>PHTTPSpace</code> class.
 */
class PHTTPFile : public PHTTPResource
{
  PCLASSINFO(PHTTPFile, PHTTPResource)

  public:
    /** Contruct a new simple file resource for the HTTP space. If no MIME
       content type is specified then a default type is used depending on the
       file type. For example, "text/html" is used of the file type is
       ".html" or ".htm". The default for an unknown type is
       "application/octet-stream".
     */
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


  // Overrides from class PHTTPResource
    /** Create a new request block for this type of resource.

       @return
       Pointer to instance of PHTTPRequest descendant class.
     */
    virtual PHTTPRequest * CreateRequest(
      const PURL & url,                  // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,          // Extra MIME information in command.
      const PMultiPartList & multipartFormInfo,
      PHTTPServer & socket
    );

    /** Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       @return
       true if all OK, false if an error occurred.
     */
    virtual PBoolean LoadHeaders(
      PHTTPRequest & request    // Information on this request.
    );

    /** Get a block of data that the resource contains.

       @return
       true if more to load.
     */
    virtual PBoolean LoadData(
      PHTTPRequest & request,    // Information on this request.
      PCharArray & data          // Data used in reply.
    );

    /** Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <code>LoadText()</code> or
       <code>LoadData()</code> functions must be overridden for correct operation.

       @return
       String for loaded text.
     */
    virtual PString LoadText(
      PHTTPRequest & request    // Information on this request.
    );


  protected:
    PHTTPFile(
      const PURL & url,       // Name of the resource in URL space.
      int dummy
    );
    // Constructor used by PHTTPDirectory


    PFilePath filePath;
};


class PHTTPFileRequest : public PHTTPRequest
{
  PCLASSINFO(PHTTPFileRequest, PHTTPRequest)
  public:
    PHTTPFileRequest(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,     // Extra MIME information in command.
      const PMultiPartList & multipartFormInfo,
      PHTTPResource * resource,
      PHTTPServer & server
    );

    PFile file;
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPTailFile

/** This object describes a HyperText Transport Protocol resource which is a
   single file. The file can be anywhere in the file system and is mapped to
   the specified URL location in the HTTP name space defined by the
   <code>PHTTPSpace</code> class.

   The difference between this and PHTTPFile is that it continually outputs
   the contents of the file, as per the unix "tail -f" command.
 */
class PHTTPTailFile : public PHTTPFile
{
  PCLASSINFO(PHTTPTailFile, PHTTPFile)

  public:
    /** Contruct a new simple file resource for the HTTP space. If no MIME
       content type is specified then a default type is used depending on the
       file type. For example, "text/html" is used of the file type is
       ".html" or ".htm". The default for an unknown type is
       "application/octet-stream".
     */
    PHTTPTailFile(
      const PString & filename     // file in file system and URL name.
    );
    PHTTPTailFile(
      const PString & filename,    // file in file system and URL name.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    PHTTPTailFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file       // Location of file in file system.
    );
    PHTTPTailFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file,      // Location of file in file system.
      const PString & contentType  // MIME content type for the file.
    );
    PHTTPTailFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file,      // Location of file in file system.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );
    PHTTPTailFile(
      const PURL & url,            // Name of the resource in URL space.
      const PFilePath & file,      // Location of file in file system.
      const PString & contentType, // MIME content type for the file.
      const PHTTPAuthority & auth  // Authorisation for the resource.
    );


  // Overrides from class PHTTPResource
    /** Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       @return
       true if all OK, false if an error occurred.
     */
    virtual PBoolean LoadHeaders(
      PHTTPRequest & request    // Information on this request.
    );

    /** Get a block of data that the resource contains.

       @return
       true if more to load.
     */
    virtual PBoolean LoadData(
      PHTTPRequest & request,    // Information on this request.
      PCharArray & data          // Data used in reply.
    );
};


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

/** This object describes a HyperText Transport Protocol resource which is a
   set of files in a directory. The directory can be anywhere in the file
   system and is mapped to the specified URL location in the HTTP name space
   defined by the <code>PHTTPSpace</code> class.

   All subdirectories and files are available as URL names in the HTTP name
   space. This effectively grafts a file system directory tree onto the URL
   name space tree.

   See the <code>PMIMEInfo</code> class for more information on the mappings between
   file types and MIME types.
 */
class PHTTPDirectory : public PHTTPFile
{
  PCLASSINFO(PHTTPDirectory, PHTTPFile)

  public:
    PHTTPDirectory(
      const PURL & url,            ///< Name of the resource in URL space.
      const PDirectory & dir       ///< Location of file in file system.
    );
    PHTTPDirectory(
      const PURL & url,            ///< Name of the resource in URL space.
      const PDirectory & dir,      ///< Location of file in file system.
      const PHTTPAuthority & auth  ///< Authorisation for the resource.
    );
    // Construct a new directory resource for HTTP.


  // Overrides from class PHTTPResource
    /** Create a new request block for this type of resource.

       @return
       Pointer to instance of PHTTPRequest descendant class.
     */
    virtual PHTTPRequest * CreateRequest(
      const PURL & url,                  // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,          // Extra MIME information in command.
      const PMultiPartList & multipartFormInfo,
      PHTTPServer & socket
    );

    /** Get the headers for block of data (eg HTML) that the resource contains.
       This will fill in all the fields of the <CODE>outMIME</CODE> parameter
       required by the resource and return the status for the load.

       @return
       true if all OK, false if an error occurred.
     */
    virtual PBoolean LoadHeaders(
      PHTTPRequest & request    ///< Information on this request.
    );

    /** Get a block of text data (eg HTML) that the resource contains.

       The default behaviour is to assert, one of <code>LoadText()</code> or
       <code>LoadData()</code> functions must be overridden for correct operation.

       @return
       String for loaded text.
     */
    virtual PString LoadText(
      PHTTPRequest & request    ///< Information on this request.
    );

    /** Enable or disable access control using .access files. A directory tree containing
       a _access file will require authorisation to allow access. This file has 
       contains one or more lines, each containing a username and password seperated 
       by a ":" character.

       The parameter sets the realm used for authorisation requests. An empty realm disables
       auhtorisation.
     */
    void EnableAuthorisation(const PString & realm);

    /** Enable or disable directory listings when a default directory file does not exist
     */
    void AllowDirectories(PBoolean enable = true);

  protected:
    PBoolean CheckAuthority(
      PHTTPServer & server,               // Server to send response to.
      const PHTTPRequest & request,       // Information on this request.
      const PHTTPConnectionInfo & conInfo // Information on the connection
    );

    PBoolean FindAuthorisations(const PDirectory & dir, PString & realm, PStringToString & authorisations);

    PDirectory basePath;
    PString authorisationRealm;
    PBoolean allowDirectoryListing;
};


class PHTTPDirRequest : public PHTTPFileRequest
{
  PCLASSINFO(PHTTPDirRequest, PHTTPFileRequest)
  public:
    PHTTPDirRequest(
      const PURL & url,             // Universal Resource Locator for document.
      const PMIMEInfo & inMIME,     // Extra MIME information in command.
      const PMultiPartList & multipartFormInfo, 
      PHTTPResource * resource,
      PHTTPServer & server
    );

    PString fakeIndex;
    PFilePath realPath;
};


#endif // P_HTTP

#endif // PTLIB_HTTP_H


// End Of File ///////////////////////////////////////////////////////////////
