/*
 * httpsvc.h
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: httpsvc.h,v $
 * Revision 1.24  1998/09/23 06:19:34  robertj
 * Added open source copyright license.
 *
 * Revision 1.23  1998/04/01 01:56:47  robertj
 * Added PServiceHTTPFile constructor so file path and URL can be different.
 *
 * Revision 1.22  1998/03/20 03:16:09  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.21  1998/03/17 10:16:00  robertj
 * Allowed registration page to have HTML override.
 *
 * Revision 1.20  1998/02/16 00:15:13  robertj
 * Major rewrite of application info passed in PHTTPServiceProcess constructor.
 *
 * Revision 1.19  1998/01/26 00:28:32  robertj
 * Removed POrderPage completely from httpsvc.
 * Added PHTTPAuthority to PHTTPServiceString constructor.
 * Added option flags to ProcessMacros to automatically load from file etc.
 *
 * Revision 1.18  1997/11/10 12:40:05  robertj
 * Changed SustituteEquivalSequence so can override standard macros.
 *
 * Revision 1.17  1997/11/04 06:02:56  robertj
 * Allowed help gif file name to overridable in PServiceHTML, so can be in subdirectory.
 *
 * Revision 1.16  1997/10/30 10:22:35  robertj
 * Added ability to customise regisration text by application.
 *
 * Revision 1.15  1997/08/28 13:56:34  robertj
 * Fixed bug where HTTP directory was not processed for macros.
 *
 * Revision 1.13  1997/08/20 08:48:18  craigs
 * Added PHTTPServiceDirectory & PHTTPServiceString
 *
 * Revision 1.12  1997/07/26 11:38:18  robertj
 * Support for overridable pages in HTTP service applications.
 *
 * Revision 1.11  1997/06/16 14:12:55  robertj
 * Changed private to protected.
 *
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
    enum {
      MaxSecuredKeys = 10
    };
    struct Info {
      const char * productName;
      const char * manufacturerName;

      WORD majorVersion;
      WORD minorVersion;
      CodeStatus buildStatus;    // AlphaCode, BetaCode or ReleaseCode
      WORD buildNumber;
      const char * compilationDate;

      PTEACypher::Key productKey;  // Poduct key for registration
      const char * securedKeys[MaxSecuredKeys]; // Product secured keys for registration
      PINDEX securedKeyCount;

      PTEACypher::Key signatureKey;   // Signature key for encryption of HTML files

      const char * manufHomePage; // WWW address of manufacturers home page
      const char * email;         // contact email for manufacturer
      const char * productHTML;   // HTML for the product name, if NULL defaults to
                                  //   the productName variable.
      const char * gifHTML;       // HTML to show GIF image in page headers, if NULL
                                  //   then the following are used to build one
      const char * gifFilename;   // File name for the products GIF file
      int gifWidth;               // Size of GIF image, if zero then none is used
      int gifHeight;              //   in the generated HTML.
    };

    PHTTPServiceProcess(const Info & inf);
    ~PHTTPServiceProcess();

    virtual void OnConfigChanged() = 0;
    virtual BOOL Initialise(const char * initMsg) = 0;

    BOOL ListenForHTTP(WORD port, BOOL startThread = TRUE);
    BOOL ListenForHTTP(PSocket * listener, BOOL startThread = TRUE);

    virtual PString GetPageGraphic();
    void GetPageHeader(PHTML &);
    void GetPageHeader(PHTML &, const PString & title);

    virtual PString GetCopyrightText();

    const PTime & GetCompilationDate() const { return compilationDate; }
    const PString & GetHomePage() const { return manufacturersHomePage; }
    const PString & GetEMailAddress() const { return manufacturersEmail; }
    const PString & GetProductName() const { return productNameHTML; }
    const PTEACypher::Key & GetProductKey() const { return productKey; }
    const PStringArray & GetSecuredKeys() const { return securedKeys; }
    const PTEACypher::Key & GetSignatureKey() const { return signatureKey; }

    static PHTTPServiceProcess & Current();

    virtual void AddRegisteredText(PHTML & html);
    virtual void AddUnregisteredText(PHTML & html);
    virtual BOOL SubstituteEquivalSequence(PHTTPRequest & request, const PString &, PString &);

  protected:
    PSocket  * httpListeningSocket;
    PHTTPSpace httpNameSpace;

    PTEACypher::Key productKey;
    PStringArray    securedKeys;
    PTEACypher::Key signatureKey;

    PTime      compilationDate;
    PString    manufacturersHomePage;
    PString    manufacturersEmail;
    PString    productNameHTML;
    PString    gifHTML;


    void ShutdownListener();
    void BeginRestartSystem();
    void CompleteRestartSystem();

    PThread *  restartThread;
    PSyncPoint httpThreadClosed;

  friend class PConfigPage;
  friend class PConfigSectionsPage;
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

    void OnLoadedText(PHTTPRequest &, PString & text);

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

PDECLARE_CLASS(PConfigSectionsPage, PHTTPConfigSectionList)
  public:
    PConfigSectionsPage(
      PHTTPServiceProcess & app,
      const PURL & url,
      const PHTTPAuthority & auth,
      const PString & prefix,
      const PString & valueName,
      const PURL & editSection,
      const PURL & newSection,
      const PString & newTitle,
      PHTML & heading
    );

    void OnLoadedText(PHTTPRequest &, PString & text);

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
    void OnLoadedText(PHTTPRequest & request, PString & text);

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


/////////////////////////////////////////////////////////////////////S

PDECLARE_CLASS(PServiceHTML, PHTML)
  public:
    PServiceHTML(const char * title,
                 const char * help = NULL,
                 const char * helpGif = "help.gif");

    PString ExtractSignature(PString & out);
    static PString ExtractSignature(const PString & html,
                                    PString & out,
                                    const char * keyword = "#equival");

    PString CalculateSignature();
    static PString CalculateSignature(const PString & out);
    static PString CalculateSignature(const PString & out, const PTEACypher::Key & sig);

    BOOL CheckSignature();
    static BOOL CheckSignature(const PString & html);

    enum MacroOptions {
      NoOptions           = 0,
      NeedSignature       = 1,
      LoadFromFile        = 2,
      NoURLOverride       = 4,
      NoSignatureForFile  = 8
    };
    static BOOL ProcessMacros(PHTTPRequest & request,
                              PString & text,
                              const PString & filename,
                              unsigned options);
};


///////////////////////////////////////////////////////////////

PDECLARE_CLASS(PServiceHTTPString, PHTTPString)
  public:
    PServiceHTTPString(const PURL & url, const PString & string)
      : PHTTPString(url, string) { }

    PServiceHTTPString(const PURL & url, const PString & string, const PHTTPAuthority & auth)
      : PHTTPString(url, string, auth) { }

    PString LoadText(PHTTPRequest &);
};


PDECLARE_CLASS(PServiceHTTPFile, PHTTPFile)
  public:
    PServiceHTTPFile(const PString & filename, BOOL needSig = FALSE)
      : PHTTPFile(filename) { needSignature = needSig; }
    PServiceHTTPFile(const PString & filename, const PFilePath & file, BOOL needSig = FALSE)
      : PHTTPFile(filename, file) { needSignature = needSig; }
    PServiceHTTPFile(const PString & filename, const PHTTPAuthority & auth, BOOL needSig = FALSE)
      : PHTTPFile(filename, auth) { needSignature = needSig; }

    void OnLoadedText(PHTTPRequest &, PString & text);

  protected:
    BOOL needSignature;
};

PDECLARE_CLASS(PServiceHTTPDirectory, PHTTPDirectory)
  public:
    PServiceHTTPDirectory(const PURL & url, const PDirectory & dirname, BOOL needSig = FALSE)
      : PHTTPDirectory(url, dirname) { needSignature = needSig; }

    PServiceHTTPDirectory(const PURL & url, const PDirectory & dirname, const PHTTPAuthority & auth, BOOL needSig = FALSE)
      : PHTTPDirectory(url, dirname, auth) { needSignature = needSig; }

    void OnLoadedText(PHTTPRequest &, PString & text);

  protected:
    BOOL needSignature;
};


#endif
