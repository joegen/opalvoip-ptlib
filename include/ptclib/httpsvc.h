/*
 * $Id: httpsvc.h,v 1.10 1997/06/16 13:20:14 robertj Exp $
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Copyright 1995-1996 Equivalence
 *
 * $Log: httpsvc.h,v $
 * Revision 1.10  1997/06/16 13:20:14  robertj
 * Fixed bug where PHTTPThread crashes on exit.
 *
 * Revision 1.9  1997/03/02 03:42:19  robertj
 * Added error logging to standard HTTP Service HTTP Server.
 * Removed extraneous variables that set GIF file size to zero.
 *
 * Revision 1.8  1997/02/05 11:54:52  robertj
 * Added support for order form page overridiing.
 *
 * Revision 1.7  1997/01/27 10:22:33  robertj
 * Numerous changes to support OEM versions of products.
 *
 * Revision 1.6  1996/11/04 03:55:20  robertj
 * Changed to accept separate copyright and manufacturer strings.
 *
 * Revision 1.5  1996/09/14 13:09:12  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.4  1996/08/19 13:43:46  robertj
 * Fixed race condition in system restart logic.
 *
 * Revision 1.3  1996/08/08 13:36:38  robertj
 * Fixed Registation page so no longer has static link, ie can be DLLed.
 *
 * Revision 1.2  1996/06/28 13:15:39  robertj
 * Moved HTTP form resource to another compilation module.
 *
 * Revision 1.1  1996/06/13 13:33:14  robertj
 * Initial revision
 *
 */

#ifndef APPCOMM_H
#define APPCOMM_H

#include <httpform.h>
#include <svcproc.h>
#include <cypher.h>


/////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(PHTTPServiceProcess, PServiceProcess)

  public:
    PHTTPServiceProcess(
      const char * name,      // Name of product
      const char * manuf,     // Name of manufacturer
      const char * gifText,   // text for gif file in page headers

      WORD majorVersion,    // Major version number of the product
      WORD minorVersion,    // Minor version number of the product
      CodeStatus status,    // Development status of the product
      WORD buildNumber,     // Build number of the product

      const char * homePage = NULL,  // WWW address of manufacturers home page
      const char * email = NULL,     // contact email for manufacturer
      const PTEACypher::Key * prodKey = NULL,  // Poduct key for registration
      const char * const * securedKeys = NULL, // Product secured keys for registration
      PINDEX securedKeyCount = 0,
      const PTEACypher::Key * sigKey = NULL // Signature key for encryption of HTML files
    );
    ~PHTTPServiceProcess();

    virtual void OnConfigChanged() = 0;
    virtual BOOL Initialise(const char * initMsg) = 0;

    BOOL ListenForHTTP(WORD port, BOOL startThread = TRUE);
    BOOL ListenForHTTP(PSocket * listener, BOOL startThread = TRUE);

    virtual PString GetPageGraphic();
    void GetPageHeader(PHTML &);
    void GetPageHeader(PHTML &, const PString & title);

    virtual PString GetCopyrightText();

    const PString & GetEMailAddress() const { return email; }
    const PString & GetHomePage() const { return homePage; }
    const PTEACypher::Key GetProductKey() const { return productKey; }
    const PStringArray & GetSecuredKeys() const { return securedKeys; }
    const PTEACypher::Key GetSignatureKey() const { return signatureKey; }

    static PHTTPServiceProcess & Current();


  protected:
    PSocket  * httpListeningSocket;
    PHTTPSpace httpNameSpace;

    PString    gifText;

    PString    email;
    PString    homePage;

    PTEACypher::Key productKey;
    PStringArray    securedKeys;
    PTEACypher::Key signatureKey;

  private:
    void ShutdownListener();
    void BeginRestartSystem();
    void CompleteRestartSystem();

    PThread *  restartThread;
    PSemaphore httpThreadClosed;

  friend class PConfigPage;
  friend class PHTTPServiceThread;
};


/////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(PHTTPServiceThread, PThread)
  public:
    PHTTPServiceThread(PHTTPServiceProcess & app,
                       PSocket & listeningSocket,
                       PHTTPSpace & http);

    void Main();

  protected:
    PHTTPServiceProcess & process;
    PSocket & listener;
    PHTTPSpace & httpNameSpace;
};


/////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(PConfigPage, PHTTPConfig)
  public:
    PConfigPage(
      PHTTPServiceProcess & app,
      const PString & section,
      const PHTTPAuthority & auth
    );
    PConfigPage(
      PHTTPServiceProcess & app,
      const PString & title,
      const PString & section,
      const PHTTPAuthority & auth
    );

    BOOL OnPOST(
      PHTTPServer & server,
      const PURL & url,
      const PMIMEInfo & info,
      const PStringToString & data,
      const PHTTPConnectionInfo & connectInfo
    );

    virtual BOOL Post(
      PHTTPRequest & request,       // Information on this request.
      const PStringToString & data, // Variables in the POST data.
      PHTML & replyMessage          // Reply message for post.
    );

  protected:
    virtual BOOL GetExpirationDate(
      PTime & when          // Time that the resource expires
    );

    PHTTPServiceProcess & process;
};


/////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(PRegisterPage, PConfigPage)
  public:
    PRegisterPage(
      PHTTPServiceProcess & app,
      const PHTTPAuthority & auth
    );

    PString LoadText(
      PHTTPRequest & request        // Information on this request.
    );

    virtual BOOL Post(
      PHTTPRequest & request,       // Information on this request.
      const PStringToString & data, // Variables in the POST data.
      PHTML & replyMessage          // Reply message for post.
    );

    virtual void AddFields(
      const PString & prefix        // Prefix on field names
    ) = 0;

  protected:
    PHTTPServiceProcess & process;
};


/////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(POrderPage, PHTTPString)
  public:
    POrderPage(PHTTPServiceProcess & app, PHTTPAuthority & auth);
    PString LoadText(PHTTPRequest & request);

  protected:
    PHTTPServiceProcess & process;
};


/////////////////////////////////////////////////////////////////////S

PDECLARE_CLASS(PServiceHTML, PHTML)
  public:
    PServiceHTML(const char * title, const char * help = NULL);

    PString ExtractSignature(PString & out);
    static PString ExtractSignature(const PString & html,
                                    PString & out,
                                    const char * keyword = "#equival");

    PString CalculateSignature();
    static PString CalculateSignature(const PString & out);
    static PString CalculateSignature(const PString & out, const PTEACypher::Key & sig);

    BOOL CheckSignature();
    static BOOL CheckSignature(const PString & html);

    static BOOL ProcessMacros(PString & text,
                              const PString & filename,
                              BOOL needSignature);
};


///////////////////////////////////////////////////////////////

PDECLARE_CLASS(PServiceHTTPFile, PHTTPFile)
  public:
    PServiceHTTPFile(const PString & filename, BOOL needSig = FALSE)
      : PHTTPFile(filename) { needSignature = needSig; }

    void OnLoadedText(PHTTPRequest &, PString & text);

  protected:
    BOOL needSignature;
};


#endif
