/*
 * $Id: httpsvc.h,v 1.3 1996/08/08 13:36:38 robertj Exp $
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Copyright 1995-1996 Equivalence
 *
 * $Log: httpsvc.h,v $
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
      const char * gifText, // text for gif file in page headers
      const char * manuf,   // Name of manufacturer
      const char * name,    // Name of product
      WORD majorVersion,    // Major version number of the product
      WORD minorVersion,    // Minor version number of the product
      CodeStatus status,    // Development status of the product
      WORD buildNumber      // Build number of the product
    );

    virtual void OnConfigChanged() = 0;
    virtual BOOL Initialise(const char * initMsg) = 0;

    BOOL GetRestartSystem() const { return restartSystem; }
    void SetRestartSystem(BOOL b) { restartSystem = b; }

    PString GetPageGraphic();
    void GetPageHeader(PHTML &);
    void GetPageHeader(PHTML &, const PString & title);

    static PHTTPServiceProcess * Current() 
      { return (PHTTPServiceProcess *)PServiceProcess::Current(); }

  protected:
    PString    gifText;
    BOOL       restartSystem;
};


/////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(PHTTPServiceThread, PThread)
  public:
    PHTTPServiceThread(PHTTPServiceProcess & app,
                       PTCPSocket & listeningSocket,
                       PHTTPSpace & http)
      : PThread(10000, AutoDeleteThread),
        process(app), listener(listeningSocket), httpSpace(http)
      { Resume(); }

    void Main();

  protected:
    PHTTPServiceProcess & process;
    PTCPSocket & listener;
    PHTTPSpace & httpSpace;
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
      PHTTPSocket & socket,
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
      const PSecureConfig & sconf,
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
    PStringArray    securedKeys;
    PTEACypher::Key productKey;
};


/////////////////////////////////////////////////////////////////////

PDECLARE_CLASS(POrderPage, PHTTPString)
  public:
    POrderPage(PHTTPAuthority & auth, const PSecureConfig & sconf);
    PString LoadText(PHTTPRequest & request);

  protected:
    PStringArray    securedKeys;
    PTEACypher::Key productKey;
};


/////////////////////////////////////////////////////////////////////S

PDECLARE_CLASS(PServiceHTML, PHTML)
  public:
    PServiceHTML(const char * title, const char * help = NULL);
};


///////////////////////////////////////////////////////////////

PDECLARE_CLASS(PServiceHTTPFile, PHTTPFile)
  public:
    PServiceHTTPFile(const PString & filename)
      : PHTTPFile(filename) { }
    void OnLoadedText(PHTTPRequest &, PString & text);
};


#endif
