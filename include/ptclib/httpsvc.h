/*
 * $Id: httpsvc.h,v 1.6 1996/11/04 03:55:20 robertj Exp $
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Copyright 1995-1996 Equivalence
 *
 * $Log: httpsvc.h,v $
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

class POrderPage;

PDECLARE_CLASS(PHTTPServiceProcess, PServiceProcess)

  friend class POrderPage;

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
      const char * email = NULL      // contact email for manufacturer
    );

    virtual void OnConfigChanged() = 0;
    virtual BOOL Initialise(const char * initMsg) = 0;

    void BeginRestartSystem();
    void CompleteRestartSystem();

    virtual PString GetPageGraphic();
    void GetPageHeader(PHTML &);
    void GetPageHeader(PHTML &, const PString & title);

    virtual PString GetCopyrightText();

    static PHTTPServiceProcess * Current() 
      { return (PHTTPServiceProcess *)PServiceProcess::Current(); }

  protected:
    PString    gifText;
    PINDEX     gifWidth, gifHeight;

    PString    email;
    PString    homePage;

  private:
    PThread *  restartThread;
};


/////////////////////////////////////////////////////////////////////

class PTCPSocket;

PDECLARE_CLASS(PHTTPServiceThread, PThread)
  public:
    PHTTPServiceThread(PHTTPServiceProcess & app,
                       PSocket & listeningSocket,
                       PHTTPSpace & http)
      : PThread(10000, AutoDeleteThread),
        process(app), listener(listeningSocket), httpSpace(http)
      { Resume(); }

    void Main();

  protected:
    PHTTPServiceProcess & process;
    PSocket & listener;
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
