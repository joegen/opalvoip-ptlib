/*
 * $Id: httpsvc.cxx,v 1.38 1998/09/18 01:47:23 robertj Exp $
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Copyright 1995-1996 Equivalence
 *
 * $Log: httpsvc.cxx,v $
 * Revision 1.38  1998/09/18 01:47:23  robertj
 * Fixed bug that made files with signature on first line fail on unix systems.
 *
 * Revision 1.37  1998/08/20 06:01:02  robertj
 * Improved internationalisation, registrationpage override.
 *
 * Revision 1.36  1998/04/21 02:43:40  robertj
 * Fixed conditional around wrong way for requiring signature on HTML files.
 *
 * Revision 1.35  1998/04/01 01:55:41  robertj
 * Fixed bug for automatically including GIF file in HTTP name space.
 *
 * Revision 1.34  1998/03/23 03:21:40  robertj
 * Fixed missing invalid case in register page.
 *
 * Revision 1.33  1998/03/20 03:18:15  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.32  1998/03/17 10:14:39  robertj
 * Rewrite of registration page to allow for HTML file override.
 *
 * Revision 1.31  1998/03/09 07:17:48  robertj
 * Added IP peer/local number macros.
 * Set GetPageGraphic reference to GIF file to be at lop level directory.
 *
 * Revision 1.30  1998/02/16 00:14:09  robertj
 * Added ProductName and BuildDate macros.
 * Major rewrite of application info passed in PHTTPServiceProcess constructor.
 *
 * Revision 1.29  1998/02/03 06:22:45  robertj
 * Allowed PHTTPServiceString to be overridden by html file after ';'.
 *
 * Revision 1.28  1998/01/26 02:49:19  robertj
 * GNU support.
 *
 * Revision 1.27  1998/01/26 02:12:14  robertj
 * GNU warnings.
 *
 * Revision 1.26  1998/01/26 00:45:44  robertj
 * Added option flags to ProcessMacros to automatically load from file etc.
 * Assured that all service HTTP resources are overidable with file, using ; URL field.
 * Added a number of extra #equival macros.
 * Added "Pty. Ltd." to company name.
 *
 * Revision 1.25  1997/11/10 12:40:05  robertj
 * Changed SustituteEquivalSequence so can override standard macros.
 *
 * Revision 1.24  1997/11/04 06:02:46  robertj
 * Allowed help gif file name to overridable in PServiceHTML, so can be in subdirectory.
 *
 * Revision 1.23  1997/10/30 10:21:26  robertj
 * Added ability to customise regisration text by application.
 *
 * Revision 1.22  1997/08/28 14:19:40  robertj
 * Fixed bug where HTTP directory was not processed for macros.
 *
 * Revision 1.20  1997/08/20 08:59:58  craigs
 * Changed macro handling to commonise #equival sequence
 *
 * Revision 1.19  1997/07/26 11:38:22  robertj
 * Support for overridable pages in HTTP service applications.
 *
 * Revision 1.18  1997/07/08 13:11:44  robertj
 * Added standard header and copyright macros to service HTML.
 *
 * Revision 1.17  1997/06/16 13:20:15  robertj
 * Fixed bug where PHTTPThread crashes on exit.
 *
 * Revision 1.16  1997/05/16 12:07:21  robertj
 * Added operating system and version to hidden fields on registration form.
 *
 * Revision 1.15  1997/03/02 03:40:59  robertj
 * Added error logging to standard HTTP Service HTTP Server.
 *
 * Revision 1.14  1997/02/05 11:54:54  robertj
 * Added support for order form page overridiing.
 *
 * Revision 1.13  1997/01/28 11:45:19  robertj
 * .
 *
 * Revision 1.13  1997/01/27 10:22:37  robertj
 * Numerous changes to support OEM versions of products.
 *
 * Revision 1.12  1997/01/03 06:33:23  robertj
 * Removed slash from operating system version string, so says Windows NT rather than Windows/NT
 *
 * Revision 1.11  1996/11/16 10:50:26  robertj
 * ??
 *
 * Revision 1.10  1996/11/04 03:58:23  robertj
 * Changed to accept separate copyright and manufacturer strings.
 *
 * Revision 1.8  1996/10/08 13:08:29  robertj
 * Changed standard graphic to use PHTML class.
 *
 * Revision 1.7  1996/09/14 13:09:33  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.6  1996/08/25 09:39:00  robertj
 * Prevented registration if no user etc entered.
 *
 * Revision 1.5  1996/08/19 13:39:55  robertj
 * Fixed race condition in system restart logic.
 *
 * Revision 1.4  1996/08/08 13:36:39  robertj
 * Fixed Registation page so no longer has static link, ie can be DLLed.
 *
 * Revision 1.3  1996/07/15 10:36:48  robertj
 * Added registration info to bottom of order form so can be faxed to us.
 *
 * Revision 1.2  1996/06/28 13:21:30  robertj
 * Fixed nesting problem in tables.
 * Fixed PConfig page always restarting.
 *
 * Revision 1.1  1996/06/13 13:33:34  robertj
 * Initial revision
 *
 */

#ifdef __GNUC__
#pragma implementation "httpsvc.h"
#endif

#include <ptlib.h>
#include <httpsvc.h>
#include <sockets.h>

#define HOME_PAGE 	"http://www.equival.com"
#define EMAIL     	"equival@equival.com.au"
#define	EQUIVALENCE	"Equivalence Pty. Ltd."


PHTTPServiceProcess::PHTTPServiceProcess(const Info & inf)
  : PServiceProcess(inf.manufacturerName, inf.productName,
                    inf.majorVersion, inf.minorVersion, inf.buildStatus, inf.buildNumber),
    productKey(inf.productKey),
    securedKeys(inf.securedKeyCount, inf.securedKeys),
    signatureKey(inf.signatureKey),
    compilationDate(inf.compilationDate),
    manufacturersHomePage(inf.manufHomePage != NULL ? inf.manufHomePage : HOME_PAGE),
    manufacturersEmail(inf.email != NULL ? inf.email : EMAIL),
    productNameHTML(inf.productHTML != NULL ? inf.productHTML : inf.productName)
{
  if (inf.gifHTML != NULL)
    gifHTML = inf.gifHTML;
  else {
    gifHTML = psprintf("<img src=\"/%s\" alt=\"%s!\"", inf.gifFilename, inf.productName);
    if (inf.gifWidth != 0 && inf.gifHeight != 0)
      gifHTML += psprintf(" width=%i height=%i", inf.gifWidth, inf.gifHeight);
    gifHTML += " align=absmiddle>";
  }

  if (inf.gifFilename != NULL)
    httpNameSpace.AddResource(new PServiceHTTPFile(inf.gifFilename, GetFile().GetDirectory()+inf.gifFilename));

  restartThread = NULL;
  httpListeningSocket = NULL;
}


PHTTPServiceProcess::~PHTTPServiceProcess()
{
  ShutdownListener();
}


PHTTPServiceProcess & PHTTPServiceProcess::Current() 
{
  PHTTPServiceProcess & process = (PHTTPServiceProcess &)PProcess::Current();
  PAssert(process.IsDescendant(PHTTPServiceProcess::Class()), "Not a HTTP service!");
  return process;
}


BOOL PHTTPServiceProcess::ListenForHTTP(WORD port, BOOL startThread)
{
  if (httpListeningSocket != NULL &&
      httpListeningSocket->GetPort() == port &&
      httpListeningSocket->IsOpen())
    return TRUE;

  return ListenForHTTP(new PTCPSocket(port), startThread);
}


BOOL PHTTPServiceProcess::ListenForHTTP(PSocket * listener, BOOL startThread)
{
  if (httpListeningSocket != NULL)
    ShutdownListener();

  httpListeningSocket = PAssertNULL(listener);
  if (!httpListeningSocket->Listen())
    return FALSE;

  if (startThread)
    new PHTTPServiceThread(*this, *httpListeningSocket, httpNameSpace);

  return TRUE;
}


void PHTTPServiceProcess::ShutdownListener()
{
  if (httpListeningSocket == NULL)
    return;

  if (!httpListeningSocket->IsOpen())
    return;

  httpListeningSocket->Close();
  httpThreadClosed.Wait();
  delete httpListeningSocket;
  httpListeningSocket = NULL;
}


PString PHTTPServiceProcess::GetCopyrightText()
{
  PTime compilationDate = PString(__DATE__);
  PHTML html = PHTML::InBody;
  html << "Copyright &copy;"
       << compilationDate.AsString("yyyy") << " by "
       << PHTML::HotLink(HOME_PAGE)
       << EQUIVALENCE
       << PHTML::HotLink()
       << ", "
       << PHTML::HotLink("mailto:" EMAIL)
       << EMAIL
       << PHTML::HotLink();
  return html;
}


PString PHTTPServiceProcess::GetPageGraphic()
{
  PHTML html = PHTML::InBody;
  html << PHTML::TableStart()
       << PHTML::TableRow()
       << PHTML::TableData()
       << gifHTML
       << PHTML::TableData()
       << GetOSClass() << ' ' << GetOSName()
       << " Version " << GetVersion(TRUE)
       << PHTML::BreakLine()
       << "By "
       << PHTML::HotLink(manufacturersHomePage) << GetManufacturer() << PHTML::HotLink()
       << ", "
       << PHTML::HotLink("mailto:" + manufacturersEmail) << manufacturersEmail << PHTML::HotLink()
       << PHTML::TableEnd();

  return html;
}


void PHTTPServiceProcess::GetPageHeader(PHTML & html)
{
  GetPageHeader(html, GetName());
}


void PHTTPServiceProcess::GetPageHeader(PHTML & html, const PString & title)
{
  html << PHTML::Title(title) 
       << PHTML::Body()
       << GetPageGraphic();
}


void PHTTPServiceProcess::BeginRestartSystem()
{
  if (restartThread == NULL) {
    restartThread = PThread::Current();
    OnConfigChanged();
  }
}


void PHTTPServiceProcess::CompleteRestartSystem()
{
  if (restartThread == NULL)
    return;
  
  if (restartThread != PThread::Current())
    return;

  if (!Initialise("Configuration changed - reloaded"))
    Terminate();
  restartThread = NULL;
}


void PHTTPServiceProcess::AddRegisteredText(PHTML &)
{
}


void PHTTPServiceProcess::AddUnregisteredText(PHTML &)
{
}


BOOL PHTTPServiceProcess::SubstituteEquivalSequence(PHTTPRequest &, const PString &, PString &)
{
  return FALSE;
}


//////////////////////////////////////////////////////////////

PHTTPServiceThread::PHTTPServiceThread(PHTTPServiceProcess & app,
                                       PSocket & listeningSocket,
                                       PHTTPSpace & http)
  : PThread(10000, AutoDeleteThread),
    process(app),
    listener(listeningSocket),
    httpNameSpace(http)
{
  Resume();
}


void PHTTPServiceThread::Main()
{
  if (!listener.IsOpen()) {
    process.httpThreadClosed.Signal();
    return;
  }

  // get a socket when a client connects
  PHTTPServer server(httpNameSpace);
  if (!server.Accept(listener)) {
    if (server.GetErrorCode() != PChannel::Interrupted)
      PSYSTEMLOG(Error, "Accept failed for HTTP: " << server.GetErrorText());
    if (listener.IsOpen())
      PNEW PHTTPServiceThread(process, listener, httpNameSpace);
    else
      process.httpThreadClosed.Signal();
    return;
  }

  PNEW PHTTPServiceThread(process, listener, httpNameSpace);

  // process requests
  while (server.ProcessCommand())
    ;

  // always close after the response has been sent
  server.Close();

  // if a restart was requested, then do it
  process.CompleteRestartSystem();
}


//////////////////////////////////////////////////////////////

PConfigPage::PConfigPage(PHTTPServiceProcess & app,
                         const PString & title,
                         const PString & section,
                         const PHTTPAuthority & auth)
  : PHTTPConfig(title, section, auth),
    process(app)
{
}


PConfigPage::PConfigPage(PHTTPServiceProcess & app,
                         const PString & section,
                         const PHTTPAuthority & auth)
  : PHTTPConfig(section.ToLower() + ".html", section, auth),
    process(app)
{
}


void PConfigPage::OnLoadedText(PHTTPRequest & request, PString & text)
{
  PServiceHTML::ProcessMacros(request, text,
                              baseURL.AsString(PURL::PathOnly).Mid(1),
                              PServiceHTML::LoadFromFile);
  PHTTPConfig::OnLoadedText(request, text);
  PServiceHTML::ProcessMacros(request, text, "", PServiceHTML::NoOptions);
}


BOOL PConfigPage::OnPOST(PHTTPServer & server,
                         const PURL & url,
                         const PMIMEInfo & info,
                         const PStringToString & data,
                         const PHTTPConnectionInfo & connectInfo)
{
  PHTTPConfig::OnPOST(server, url, info, data, connectInfo);
  return FALSE;    // Make sure we break any persistent connections
}


BOOL PConfigPage::Post(PHTTPRequest & request,
                       const PStringToString & data,
                       PHTML & reply)
{
  PServiceHTML::ProcessMacros(request, reply,
                              baseURL.AsString(PURL::PathOnly).Mid(1),
                              PServiceHTML::LoadFromFile);

  PSYSTEMLOG(Debug3, "Post to " << request.url << '\n' << data);
  BOOL retval = PHTTPConfig::Post(request, data, reply);

  OnLoadedText(request, reply);

  if (request.code == PHTTP::OK)
    process.BeginRestartSystem();
  return retval;
}


BOOL PConfigPage::GetExpirationDate(PTime & when)
{
  // Well and truly before now....
  when = PTime(0, 0, 0, 1, 1, 1980);
  return TRUE;
}


//////////////////////////////////////////////////////////////

PConfigSectionsPage::PConfigSectionsPage(PHTTPServiceProcess & app,
                                         const PURL & url,
                                         const PHTTPAuthority & auth,
                                         const PString & prefix,
                                         const PString & valueName,
                                         const PURL & editSection,
                                         const PURL & newSection,
                                         const PString & newTitle,
                                         PHTML & heading)
  : PHTTPConfigSectionList(url, auth, prefix, valueName,
                           editSection, newSection, newTitle, heading),
    process(app)
{
}


void PConfigSectionsPage::OnLoadedText(PHTTPRequest & request, PString & text)
{
  PServiceHTML::ProcessMacros(request, text,
                              baseURL.AsString(PURL::PathOnly).Mid(1),
                              PServiceHTML::LoadFromFile);
  PHTTPConfigSectionList::OnLoadedText(request, text);
}


BOOL PConfigSectionsPage::OnPOST(PHTTPServer & server,
                                 const PURL & url,
                                 const PMIMEInfo & info,
                                 const PStringToString & data,
                                 const PHTTPConnectionInfo & connectInfo)
{
  PHTTPConfigSectionList::OnPOST(server, url, info, data, connectInfo);
  return FALSE;    // Make sure we break any persistent connections
}


BOOL PConfigSectionsPage::Post(PHTTPRequest & request,
                               const PStringToString & data,
                               PHTML & reply)
{
  BOOL retval = PHTTPConfigSectionList::Post(request, data, reply);
  if (request.code == PHTTP::OK)
    process.BeginRestartSystem();
  return retval;
}


BOOL PConfigSectionsPage::GetExpirationDate(PTime & when)
{
  // Well and truly before now....
  when = PTime(0, 0, 0, 1, 1, 1980);
  return TRUE;
}


//////////////////////////////////////////////////////////////

PRegisterPage::PRegisterPage(PHTTPServiceProcess & app,
                             const PHTTPAuthority & auth)
  : PConfigPage(app, "register.html", "Secured Options", auth),
    process(app)
{
}


PString PRegisterPage::LoadText(PHTTPRequest & request)
{
  if (fields.GetSize() > 0)
    return PConfigPage::LoadText(request);

  PString mailURL = "mailto:" + process.GetEMailAddress();
  PString orderURL = mailURL;
  PString tempURL = mailURL;
  if (process.GetHomePage() == HOME_PAGE) {
    orderURL = "https://home.equival.com.au/purchase.html";
    tempURL = "http://www.equival.com/" + process.GetName() + "/register.html";
  }

  PServiceHTML regPage(process.GetName() & "Registration", NULL);
  regPage << "<!--#registration start Permanent-->"
             "Your registration key is permanent.<p>"
             "Do not change your registration details or your key will not "
             "operate correctly.<p>"
             "If you need to "
          << PHTML::HotLink(orderURL)
          << "upgrade"
          << PHTML::HotLink()
          << " or "
          << PHTML::HotLink(mailURL)
          << "change"
          << PHTML::HotLink()
          << " your registration, then you may enter the new values sent "
          << " to you from "
          << process.GetManufacturer()
          << " into the fields "
             "below, and then press the Accept button.<p>"
          << PHTML::HRule()
          << "<!--#registration end Permanent-->"
             "<!--#registration start Temporary-->"
             "Your registration key is temporary and will expire on "
             "<!--#registration ExpiryDate-->.<p>"
             "Do not change your registration details or your key will not "
             "operate correctly.<p>"
             "You may "
          << PHTML::HotLink(orderURL)
          << "order a permanent key"
          << PHTML::HotLink()
          << " and enter the new values sent to you from "
          << process.GetManufacturer()
          << " into the fields below, and then press the Accept button.<p>"
          << PHTML::HRule()
          << "<!--#registration end Temporary-->"
             "<!--#registration start Expired-->"
             "Your temporary registration key has expired.<p>"
             "You may "
          << PHTML::HotLink(orderURL)
          << "order a permanent key"
          << PHTML::HotLink()
          << " and enter the new values sent to you from "
          << process.GetManufacturer()
          << " into the fields below, and then press the Accept button.<P>"
          << PHTML::HRule()
          << "<!--#registration end Expired-->";

  PSecureConfig securedConf(process.GetProductKey(), process.GetSecuredKeys());
  PString prefix;
  if (securedConf.GetValidation() != PSecureConfig::IsValid) 
    prefix = securedConf.GetPendingPrefix();

  AddFields(prefix);

  Add(new PHTTPStringField("Validation", 40));
  BuildHTML(regPage, InsertIntoHTML);

  regPage << "<!--#registration start Invalid-->"
             "You have entered the values sent to you from "
          << process.GetManufacturer()
          << " incorrectly. Please enter them again. Note, "
          << PHTML::Emphasis() << PHTML::Strong() << "all" << PHTML::Strong() << PHTML::Emphasis()
          << "the fields must be entered "
          << PHTML::Emphasis() << PHTML::Strong() << "exactly" << PHTML::Strong() << PHTML::Emphasis()
          << " as they appear in the e-mail from "
          << process.GetManufacturer()
          << ". We strongly recommend using copy and paste of all the fields, and then "
             "press the Accept button."
             "<!--#registration end Invalid-->"
             "<!--#registration start Default-->"
             "You may "
          << PHTML::HotLink(orderURL)
          << "order a permanent key"
          << PHTML::HotLink()
          << " or "
          << PHTML::HotLink(tempURL)
          << "obtain a temporary key"
          << PHTML::HotLink()
          << " and enter the values sent to you from "
          << process.GetManufacturer()
          << " into the fields above, and then press the Accept button.<p>"
             "<!--#registration end Default-->"
          << PHTML::HRule()
          << PHTML::Heading(3) << "Disclaimer" << PHTML::Heading(3)
          << PHTML::Paragraph() << PHTML::Bold()
          << "The information and code herein is provided \"as is\" "
             "without warranty of any kind, either expressed or implied, "
             "including but not limited to the implied warrenties of "
             "merchantability and fitness for a particular purpose. In "
             "no event shall " << process.GetManufacturer() << " be liable "
             "for any damages whatsoever including direct, indirect, "
             "incidental, consequential, loss of business profits or special "
             "damages, even if " << process.GetManufacturer() << " has been "
             "advised of the possibility of such damages."
          << PHTML::Bold() << PHTML::Paragraph()
          << process.GetCopyrightText()
          << PHTML::Body();

  SetString(regPage);
  return PConfigPage::LoadText(request);
}


static BOOL FindSpliceBlock(const PRegularExpression & regex,
                            const PString & text,
                            PINDEX & pos,
                            PINDEX & len,
                            PINDEX & start,
                            PINDEX & finish)
{
  if (!text.FindRegEx(regex, pos, len, 0))
    return FALSE;

  PINDEX endpos, endlen;
  static PRegularExpression EndBlock("<?!--#registration[ \t\n]*end[ \t\n]*[a-z]*[ \t\n]*-->?",
                                     PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  if (text.FindRegEx(EndBlock, endpos, endlen, pos)) {
    start = pos+len;
    finish = endpos-1;
    len = endpos - pos + endlen;
  }

  return TRUE;
}


void PRegisterPage::OnLoadedText(PHTTPRequest & request, PString & text)
{
  PString block;
  PINDEX pos, len, start, finish;
  PSecureConfig securedConf(process.GetProductKey(), process.GetSecuredKeys());
  PTime expiry = securedConf.GetTime(securedConf.GetExpiryDateKey());

  static PRegularExpression Default("<?!--#registration[ \t\n]*start[ \t\n]*Default[ \t\n]*-->?",
                                    PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression Permanent("<?!--#registration[ \t\n]*start[ \t\n]*Permanent[ \t\n]*-->?",
                                      PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression Temporary("<?!--#registration[ \t\n]*start[ \t\n]*Temporary[ \t\n]*-->?",
                                      PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression Expired("<?!--#registration[ \t\n]*start[ \t\n]*Expired[ \t\n]*-->?",
                                    PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression Invalid("<?!--#registration[ \t\n]*start[ \t\n]*Invalid[ \t\n]*-->?",
                                    PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression Pending("name[ \t\n]*=[ \t\n]*\"" +
                                    securedConf.GetPendingPrefix() +
                                    "[^\"]+\"",
                                    PRegularExpression::Extended|PRegularExpression::IgnoreCase);

  PServiceHTML::ProcessMacros(request, text,
                              baseURL.AsString(PURL::PathOnly).Mid(1),
                              PServiceHTML::LoadFromFile);

  switch (securedConf.GetValidation()) {
    case PSecureConfig::Defaults :
      while (FindSpliceBlock(Default, text, pos, len, start, finish))
        text.Splice(text(start, finish), pos, len);
      while (FindSpliceBlock(Permanent, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Temporary, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Expired, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Invalid, text, pos, len, start, finish))
        text.Delete(pos, len);
      break;

    case PSecureConfig::Invalid :
    case PSecureConfig::Pending :
      while (FindSpliceBlock(Default, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Permanent, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Temporary, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Expired, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Invalid, text, pos, len, start, finish))
        text.Splice(text(start, finish), pos, len);
      break;

    case PSecureConfig::Expired :
      while (FindSpliceBlock(Default, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Permanent, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Temporary, text, pos, len, start, finish))
        text.Delete(pos, len);
      while (FindSpliceBlock(Expired, text, pos, len, start, finish))
        text.Splice(text(start, finish), pos, len);
      while (FindSpliceBlock(Invalid, text, pos, len, start, finish))
        text.Delete(pos, len);
      break;

    case PSecureConfig::IsValid :
      start = 0;
      while (text.FindRegEx(Pending, pos, len)) {
        static PINDEX pendingLength = securedConf.GetPendingPrefix().GetLength();
        text.Delete(text.Find('"', pos)+1, pendingLength);
        start = pos + len - pendingLength;
      }
      if (expiry.GetYear() < 2011) {
        while (FindSpliceBlock(Default, text, pos, len, start, finish))
          text.Delete(pos, len);
        while (FindSpliceBlock(Permanent, text, pos, len, start, finish))
          text.Delete(pos, len);
        while (FindSpliceBlock(Temporary, text, pos, len, start, finish))
          text.Splice(text(start, finish), pos, len);
        while (FindSpliceBlock(Expired, text, pos, len, start, finish))
          text.Delete(pos, len);
        while (FindSpliceBlock(Invalid, text, pos, len, start, finish))
          text.Delete(pos, len);
      }
      else {
        while (FindSpliceBlock(Default, text, pos, len, start, finish))
          text.Delete(pos, len);
        while (FindSpliceBlock(Permanent, text, pos, len, start, finish))
          text.Splice(text(start, finish), pos, len);
        while (FindSpliceBlock(Temporary, text, pos, len, start, finish))
          text.Delete(pos, len);
        while (FindSpliceBlock(Expired, text, pos, len, start, finish))
          text.Delete(pos, len);
        while (FindSpliceBlock(Invalid, text, pos, len, start, finish))
          text.Delete(pos, len);
      }
  }

  static PRegularExpression ExpiryDate("<?!--#registration[ \t\n]*ExpiryDate[ \t\n]*-->?",
                                       PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  while (text.FindRegEx(ExpiryDate, pos, len, 0))
    text.Splice(expiry.AsString(PTime::LongDate), pos, len);

  PHTTPConfig::OnLoadedText(request, text);
  PServiceHTML::ProcessMacros(request, text, "", PServiceHTML::NoOptions);
}


BOOL PRegisterPage::Post(PHTTPRequest & request,
                         const PStringToString & data,
                         PHTML & reply)
{
  if (fields.GetSize() == 0)
    LoadText(request);

  BOOL retval = PHTTPConfig::Post(request, data, reply);
  if (request.code != PHTTP::OK)
    return FALSE;

  PSecureConfig sconf(process.GetProductKey(), process.GetSecuredKeys());
  switch (sconf.GetValidation()) {
    case PSecureConfig::Defaults :
      sconf.ResetPending();
      break;

    case PSecureConfig::IsValid :
      break;

    case PSecureConfig::Pending :
      sconf.ValidatePending();
      break;

    default :
      sconf.ResetPending();
  }

  RemoveAllFields();
  LoadText(request);
  OnLoadedText(request, reply);

  return retval;
}


///////////////////////////////////////////////////////////////////

static void DigestSecuredKeys(PHTTPServiceProcess & process,
                              PString & reginfo,
                              PHTML * html)
{
  const PStringArray & securedKeys = process.GetSecuredKeys();
  PSecureConfig sconf(process.GetProductKey(), securedKeys);

  PString prefix;
  if (sconf.GetValidation() != PSecureConfig::IsValid) 
    prefix = sconf.GetPendingPrefix();

  PMessageDigest5 digestor;

  PStringStream info;
  info << '"' << process.GetName() << "\" ===";

  PINDEX i;
  for (i = 0; i < securedKeys.GetSize(); i++) {
    PString val = sconf.GetString(prefix + securedKeys[i]).Trim();
    info << " \"" << val << '"';
    if (html != NULL)
      *html << PHTML::HiddenField(securedKeys[i], val);
    digestor.Process(val);
  }

  PString digest = digestor.Complete();
  if (html != NULL)
    *html << PHTML::HiddenField("digest", digest);

  info.Replace("===", digest);
  reginfo = info;
}


///////////////////////////////////////////////////////////////////

PServiceHTML::PServiceHTML(const char * title, const char * help, const char * helpGif)
{
  PHTTPServiceProcess::Current().GetPageHeader(*this, title);

  *this << PHTML::Heading(1) << title;
  
  if (help != NULL)
    *this << "&nbsp;"
          << PHTML::HotLink(help)
          << PHTML::Image(helpGif, "Help", 48, 23, "align=absmiddle")
          << PHTML::HotLink();

  *this << PHTML::Heading(1) << PHTML::Paragraph();
}


PString PServiceHTML::ExtractSignature(PString & out)
{
  return ExtractSignature(*this, out);
}


PString PServiceHTML::ExtractSignature(const PString & html,
                                       PString & out,
                                       const char * keyword)
{
  PString signature;
  out = PString();

  // search for all comment blocks
  PINDEX  lastPos = 0, endPos = 0;
  PINDEX  pos;
  while ((pos    = html.Find("<!--", lastPos)) != P_MAX_INDEX &&
         (endPos = html.Find("-->", pos))      != P_MAX_INDEX) {

    // add in the text before the comment and move the ptr to the end of
    // the comment
    if (pos > lastPos)
      out += html(lastPos, pos-1);
    lastPos = endPos+3;

    // tokenise the text inside the comment
    PStringArray tokens = html(pos+4, endPos-1).Trim().Tokenise(" \n", FALSE);

    // if this is a signature, then retreive it
    if (tokens[0] *= keyword) {
      PINDEX len = tokens.GetSize();
      if (tokens[1] != "signature" || len != 3)
        out += html(pos, endPos+2);
      else
        signature = tokens[2];
    }
  }

  out += html(lastPos, P_MAX_INDEX);
  return signature;
}


PString PServiceHTML::CalculateSignature()
{
  return CalculateSignature(*this);
}


PString PServiceHTML::CalculateSignature(const PString & out)
{
  return CalculateSignature(out, PHTTPServiceProcess::Current().GetSignatureKey());
}


PString PServiceHTML::CalculateSignature(const PString & out,
                                         const PTEACypher::Key & sig)
{
  // calculate the MD5 digest of the HTML data
  PMessageDigest5 digestor;

  PINDEX p1 = 0;
  PINDEX p2;
  while ((p2 = out.FindOneOf("\r\n", p1)) != P_MAX_INDEX) {
    if (p2 > p1)
      digestor.Process(out(p1, p2-1));
    digestor.Process("\r\n", 2);
    p1 = p2 + 1;
    if (out[p2] == '\r' && out[p1] == '\n') // CR LF pair
      p1++;
  }
  digestor.Process(out(p1, P_MAX_INDEX));

  PMessageDigest5::Code md5;
  digestor.Complete(md5);

  // encode it
  PTEACypher cypher(sig);
  return cypher.Encode(&md5, sizeof(md5));
}


BOOL PServiceHTML::CheckSignature()
{
  return CheckSignature(*this);
}


BOOL PServiceHTML::CheckSignature(const PString & html)
{
  // extract the signature from the file
  PString out;
  PString signature = ExtractSignature(html, out);

  // calculate the signature on the data
  PString checkSignature = CalculateSignature(out);

  // return TRUE or FALSE
  return checkSignature == signature;
}


static BOOL FindBrackets(const PString & args, PINDEX & open, PINDEX & close)
{
  open = args.FindOneOf("[{(", close);
  if (open == P_MAX_INDEX)
    return FALSE;

  switch (args[open]) {
    case '[' :
      close = args.Find(']', open+1);
      break;
    case '{' :
      close = args.Find('}', open+1);
      break;
    case '(' :
      close = args.Find(')', open+1);
      break;
  }
  return close != P_MAX_INDEX;
}


static BOOL ExtractVariables(const PString & args,
                             PString & variable,
                             PString & value)
{
  PINDEX open;
  PINDEX close = 0;
  if (FindBrackets(args, open, close))
    variable = args(open+1, close-1);
  else {
    variable = args.Trim();
    close = P_MAX_INDEX-1;
  }
  if (variable.IsEmpty())
    return FALSE;

  if (FindBrackets(args, open, close))
    value = args(open+1, close-1);

  return TRUE;
}


class PServiceMacro : public PCaselessString
{
  public:
    PServiceMacro(const char * name);
    virtual PString Translate(PHTTPRequest & request, const PString & args) const = 0;
};


PSORTED_LIST(PServiceMacros_base, PServiceMacro);


class PServiceMacros_list : public PServiceMacros_base
{
  public:
    PServiceMacros_list() { DisallowDeleteObjects(); }
};


static PServiceMacros_list ServiceMacros;


PServiceMacro::PServiceMacro(const char * name)
  : PCaselessString(name)
{
  ServiceMacros.Append(this);
}


#define EMPTY

#define CREATE_MACRO(name, request, args) \
  static const class PServiceMacro_##name : public PServiceMacro { \
    public: \
      PServiceMacro_##name() : PServiceMacro(#name) { } \
      PString Translate(PHTTPRequest & request, const PString & args) const; \
  } serviceMacro_##name; \
  PString PServiceMacro_##name::Translate(PHTTPRequest & request, const PString & args) const


CREATE_MACRO(Header,request,EMPTY)
{
  PString hdr = PHTTPServiceProcess::Current().GetPageGraphic();
  PServiceHTML::ProcessMacros(request, hdr, "header.html",
                PServiceHTML::LoadFromFile|PServiceHTML::NoURLOverride);
  return hdr;
}


CREATE_MACRO(Copyright,EMPTY,EMPTY)
{
  return PHTTPServiceProcess::Current().GetCopyrightText();
}


CREATE_MACRO(ProductName,EMPTY,EMPTY)
{
  return PHTTPServiceProcess::Current().GetProductName();
}


CREATE_MACRO(Manufacturer,EMPTY,EMPTY)
{
  return PHTTPServiceProcess::Current().GetManufacturer();
}


CREATE_MACRO(Version,EMPTY,EMPTY)
{
  return PHTTPServiceProcess::Current().GetVersion(TRUE);
}


CREATE_MACRO(BuildDate,EMPTY,args)
{
  const PTime & date = PHTTPServiceProcess::Current().GetCompilationDate();
  if (args.IsEmpty())
    return date.AsString("d MMMM yyyy");

  return date.AsString(args);
}


CREATE_MACRO(OS,EMPTY,EMPTY)
{
  PHTTPServiceProcess & process = PHTTPServiceProcess::Current();
  return process.GetOSClass() & process.GetOSName();
}


CREATE_MACRO(LongDateTime,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::LongDateTime);
}


CREATE_MACRO(LongDate,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::LongDate);
}


CREATE_MACRO(LongTime,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::LongTime);
}


CREATE_MACRO(MediumDateTime,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::MediumDateTime);
}


CREATE_MACRO(MediumDate,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::MediumDate);
}


CREATE_MACRO(ShortDateTime,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::ShortDateTime);
}


CREATE_MACRO(ShortDate,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::ShortDate);
}


CREATE_MACRO(ShortTime,EMPTY,EMPTY)
{
  return PTime().AsString(PTime::ShortTime);
}


CREATE_MACRO(Time,EMPTY,args)
{
  PTime now;
  if (args.IsEmpty())
    return now.AsString();

  return now.AsString(args);
}


CREATE_MACRO(LocalHost,request,EMPTY)
{
  if (request.localAddr != 0)
    return PIPSocket::GetHostName(request.localAddr);
  else
    return PIPSocket::GetHostName();
}


CREATE_MACRO(LocalIP,request,EMPTY)
{
  if (request.localAddr != 0)
    return request.localAddr;
  else
    return "127.0.0.1";
}


CREATE_MACRO(LocalPort,request,EMPTY)
{
  if (request.localPort != 0)
    return psprintf("%u", request.localPort);
  else
    return "80";
}


CREATE_MACRO(PeerHost,request,EMPTY)
{
  if (request.origin != 0)
    return PIPSocket::GetHostName(request.origin);
  else
    return "N/A";
}


CREATE_MACRO(PeerIP,request,EMPTY)
{
  if (request.origin != 0)
    return request.origin;
  else
    return "N/A";
}


CREATE_MACRO(RegInfo,EMPTY,EMPTY)
{
  PString subs;
  DigestSecuredKeys(PHTTPServiceProcess::Current(), subs, NULL);
  return subs;
}


static PString GetRegInfo(const char * info)
{
  PHTTPServiceProcess & process = PHTTPServiceProcess::Current();
  PSecureConfig sconf(process.GetProductKey(), process.GetSecuredKeys());
  PString pending = sconf.GetPendingPrefix();
  return sconf.GetString(info, sconf.GetString(pending+info));
}

CREATE_MACRO(RegUser,EMPTY,EMPTY)
{
  return GetRegInfo("Name");
}


CREATE_MACRO(RegCompany,EMPTY,EMPTY)
{
  return GetRegInfo("Company");
}


CREATE_MACRO(RegEmail,EMPTY,EMPTY)
{
  return GetRegInfo("EMail");
}


CREATE_MACRO(Registration,EMPTY,args)
{
  PHTTPServiceProcess & process = PHTTPServiceProcess::Current();
  PSecureConfig sconf(process.GetProductKey(), process.GetSecuredKeys());
  PString pending = sconf.GetPendingPrefix();

  PString regNow = "Register Now!";
  PString viewReg = "View Registration";
  PString demoCopy = "Unregistered Demonstration Copy ***";
  PINDEX open;
  PINDEX close = 0;
  if (FindBrackets(args, open, close)) {
    regNow = args(open+1, close-1);
    if (FindBrackets(args, open, close)) {
      viewReg = args(open+1, close-1);
      if (FindBrackets(args, open, close))
        demoCopy = args(open+1, close-1);
    }
  }

  PHTML out = PHTML::InBody;
  out << "<font size=5>"
      << sconf.GetString("Name", sconf.GetString(pending+"Name", "*** "+demoCopy+" ***"))
      << PHTML::BreakLine()
      << "<font size=4>"
      << sconf.GetString("Company", sconf.GetString(pending+"Company"))
      << PHTML::BreakLine()
      << PHTML::BreakLine()
      << "<font size=3>";

  if (sconf.GetString("Name").IsEmpty())
    process.AddUnregisteredText(out);
  else
    process.AddRegisteredText(out);

  out << PHTML::HotLink("/register.html")
      << (sconf.GetString("Name").IsEmpty() ? regNow : viewReg)
      << PHTML::HotLink();
  return out;
}


CREATE_MACRO(InputsFromQuery,request,EMPTY)
{
  PStringToString vars = request.url.GetQueryVars();
  PStringStream subs;
  for (PINDEX i = 0; i < vars.GetSize(); i++)
    subs << "<INPUT TYPE=hidden NAME=\"" << vars.GetKeyAt(i)
         << "\" VALUE=\"" << vars.GetDataAt(i) << "\">\r\n";
  return subs;
}


CREATE_MACRO(Query,request,args)
{
  if (args.IsEmpty())
    return request.url.GetQuery();

  PString variable, value;
  if (ExtractVariables(args, variable, value)) {
    value = request.url.GetQueryVars()(variable, value);
    if (!value)
      return value;
  }
  return PString();
}


CREATE_MACRO(Get,request,args)
{
  PString variable, value;
  if (ExtractVariables(args, variable, value)) {
    PString section = request.url.GetQueryVars()("section");
    PINDEX slash = variable.FindLast('\\');
    if (slash != P_MAX_INDEX) {
      section += variable.Left(slash);
      variable = variable.Mid(slash+1);
    }
    if (!section && !variable) {
      PConfig config(section);
      return config.GetString(variable, value);
    }
  }
  return PString();
}


CREATE_MACRO(URL,request,EMPTY)
{
  return request.url.AsString();
}


BOOL PServiceHTML::ProcessMacros(PHTTPRequest & request,
                                 PString & text,
                                 const PString & defaultFile,
                                 unsigned options)
{
  PINDEX alreadyLoadedPrefixLength = 0;

  PString filename = defaultFile;
  if ((options&LoadFromFile) != 0) {
    if ((options&NoURLOverride) == 0) {
      filename = request.url.GetParameters();
      if (filename.IsEmpty())
        filename = defaultFile;
    }

    if (!filename) {
      PString alreadyLoaded = "<!--#loadedfrom " + filename + "-->\r\n";
      alreadyLoadedPrefixLength = alreadyLoaded.GetLength();

      if (text.Find(alreadyLoaded) != 0) {
        PFile file;
        if (file.Open(filename, PFile::ReadOnly)) {
          text = alreadyLoaded + file.ReadString(file.GetLength());
          if ((options&NoSignatureForFile) == 0)
            options |= NeedSignature;
        }
      }
    }
  }

  if ((options&NeedSignature) != 0) {
    if (!CheckSignature(text.Mid(alreadyLoadedPrefixLength))) {
      PHTTPServiceProcess & process = PHTTPServiceProcess::Current();
      PHTML html = "Invalid OEM Signature";
      html << "The HTML file \""
           << filename
           << "\" contains an invalid signature for \""
           << process.GetName()
           << "\" by \""
           << process.GetManufacturer()
           << '"'
           << PHTML::Body();
      text = html;
      return FALSE;
    }
  }

  PINDEX pos = 0;
  for (;;) {
    pos = text.Find("!--#equival", pos);
    if (pos == P_MAX_INDEX)
      break;
    PINDEX end = text.Find("--", pos+3);
    if (end == P_MAX_INDEX)
      break;

    PString include = text(pos+12, end-1).Trim();

    if (text[pos-1] == '<')
      pos--;
    end += 2;
    if (text[end] == '>')
      end++;

    PString subs;
    if (!PHTTPServiceProcess::Current().SubstituteEquivalSequence(request, include, subs)) {
      PCaselessString cmd = include.Left(include.Find(' '));
      PINDEX idx = ServiceMacros.GetValuesIndex(cmd);
      if (idx != P_MAX_INDEX)
        subs = ServiceMacros[idx].Translate(request, include.Mid(cmd.GetLength()).LeftTrim());
    }

    text.Splice(subs, pos, end-pos);
  }

  return TRUE;
}


///////////////////////////////////////////////////////////////////

static void ServiceOnLoadedText(PString & text)
{
  PHTTPServiceProcess & process = PHTTPServiceProcess::Current();

  PString manuf = "<!--Standard_" + process.GetManufacturer() + "_Header-->";
  if (text.Find(manuf) != P_MAX_INDEX)
    text.Replace(manuf, process.GetPageGraphic(), TRUE);

  static const char equiv[] = "<!--Standard_Equivalence_Header-->";
  if (text.Find(equiv) != P_MAX_INDEX)
    text.Replace(equiv, process.GetPageGraphic(), TRUE);

  static const char copy[] = "<!--Standard_Copyright_Header-->";
  if (text.Find(copy) != P_MAX_INDEX)
    text.Replace(copy, process.GetCopyrightText(), TRUE);
}

PString PServiceHTTPString::LoadText(PHTTPRequest & request)
{
  PString text = PHTTPString::LoadText(request);
  ServiceOnLoadedText(text);
  PServiceHTML::ProcessMacros(request, text, "", PServiceHTML::LoadFromFile);

  return text;
}

void PServiceHTTPFile::OnLoadedText(PHTTPRequest & request, PString & text)
{
  ServiceOnLoadedText(text);
  PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly),
          needSignature ? PServiceHTML::NeedSignature : PServiceHTML::NoOptions);
}

void PServiceHTTPDirectory::OnLoadedText(PHTTPRequest & request, PString & text)
{
  ServiceOnLoadedText(text);
  PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly),
          needSignature ? PServiceHTML::NeedSignature : PServiceHTML::NoOptions);
}


///////////////////////////////////////////////////////////////////
