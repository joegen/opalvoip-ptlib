/*
 * $Id: httpsvc.cxx,v 1.23 1997/10/30 10:21:26 robertj Exp $
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Copyright 1995-1996 Equivalence
 *
 * $Log: httpsvc.cxx,v $
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

#include <ptlib.h>
#include <httpsvc.h>
#include <sockets.h>

#define HOME_PAGE 	"http://www.ozemail.com.au/~equival"
#define EMAIL     	"equival@ozemail.com.au"
#define	EQUIVALENCE	"Equivalence"


PHTTPServiceProcess::PHTTPServiceProcess(
                  const char * name,      // Name of product
                  const char * manuf,     // Name of manufacturer
                  const char * _gifName,  // text for gif file in page headers

                  WORD majorVersion,    // Major version number of the product
                  WORD minorVersion,    // Minor version number of the product
                  CodeStatus status,    // Development status of the product
                  WORD buildNumber,     // Build number of the product

                  const char * _homePage,  // WWW address of manufacturers home page
                  const char * _email,     // contact email for manufacturer
                  const PTEACypher::Key * prodKey,  // Poduct key for registration
                  const char * const * securedKeys, // Product secured keys for registration
                  PINDEX securedKeyCount,
                  const PTEACypher::Key * sigKey // Signature key for encryption of HTML files
                )
  : PServiceProcess(manuf, name, majorVersion,
                    minorVersion, status, buildNumber),
    gifText(_gifName),
    securedKeys(securedKeyCount, securedKeys),
    httpThreadClosed(0)
{
  if (_email != NULL)
    email = _email;
  else
    email = EMAIL;

  if (_homePage != NULL)
    homePage = _homePage;
  else
    homePage = HOME_PAGE;

  if (prodKey != NULL)
    productKey = *prodKey;

  if (sigKey != NULL)
    signatureKey = *sigKey;

  if (_gifName != NULL)
    httpNameSpace.AddResource(new PServiceHTTPFile(_gifName));

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
       << gifText
       << PHTML::TableData()
       << GetOSClass() << ' ' << GetOSName()
       << " Version " << GetVersion(TRUE)
       << PHTML::BreakLine()
       << "By "
       << PHTML::HotLink(homePage) << GetManufacturer() << PHTML::HotLink()
       << ", "
       << PHTML::HotLink("mailto:" + email) << email << PHTML::HotLink()
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


void PHTTPServiceProcess::SubstituteEquivalSequence(PHTTPRequest &, const PString &, PString &)
{
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
  PString filePath = baseURL.AsString(PURL::PathOnly).Mid(1);
  PFile file;
  if (!file.Open(filePath, PFile::ReadOnly))
    PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly), FALSE);
  else {
    text = file.ReadString(file.GetLength());
    PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly), TRUE);
  }

  PHTTPConfig::OnLoadedText(request, text);
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
  BOOL retval = PHTTPConfig::Post(request, data, reply);
  if (request.code == PHTTP::OK)
    process.BeginRestartSystem();
  return retval;
}


BOOL PConfigPage::GetExpirationDate(PTime & when)
{
  // As early as possible, but because of time zones & daylight time make it the
  // second day...
  when = PTime(90000);
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
  PString filePath = baseURL.AsString(PURL::PathOnly).Mid(1);
  PFile file;
  if (!file.Open(filePath, PFile::ReadOnly))
    PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly), FALSE);
  else {
    text = file.ReadString(file.GetLength());
    PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly), TRUE);
  }
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
  // As early as possible, but because of time zones & daylight time make it the
  // second day...
  when = PTime(90000);
  return TRUE;
}


//////////////////////////////////////////////////////////////

PRegisterPage::PRegisterPage(PHTTPServiceProcess & app,
                             const PHTTPAuthority & auth)
  : PConfigPage(app, "register.html", "Secured Options", auth),
    process(app)
{
}


static void AddDisclaimer(PHTML & regPage, const PString & manuf)
{
  regPage << PHTML::HRule()
          << PHTML::Heading(3) << "Disclaimer" << PHTML::Heading(3)
          << PHTML::Paragraph() << PHTML::Bold()
          << "The information and code herein is provided \"as is\" "
             "without warranty of any kind, either expressed or implied, "
             "including but not limited to the implied warrenties of "
             "merchantability and fitness for a particular purpose. In "
             "no event shall " << manuf << " be liable for any damages "
             "whatsoever including direct, indirect, incidental, "
             "consequential, loss of business profits or special "
             "damages, even if " << manuf << " has been advised of the "
             "possibility of such damages."
          << PHTML::Bold() << PHTML::Paragraph();
}


PString PRegisterPage::LoadText(PHTTPRequest & request)
{
  if (fields.GetSize() > 0)
    return PConfigPage::LoadText(request);

  PServiceHTML regPage(process.GetName() & "Registration", "reghelp.html");

  PSecureConfig securedConf(process.GetProductKey(), process.GetSecuredKeys());
  PSecureConfig::ValidationState state = securedConf.GetValidation();

  PString prefix;

  if (state != PSecureConfig::IsValid) 
    prefix = securedConf.GetPendingPrefix();

  AddFields(prefix);

  Add(new PHTTPStringField("Validation", 34));

  if (state == PSecureConfig::Defaults) {
    AddDisclaimer(regPage, process.GetManufacturer());
    BuildHTML(regPage, InsertIntoHTML);
  }
  else {
    BuildHTML(regPage, InsertIntoHTML);

    BOOL doReorder = FALSE;

    regPage << "Your registration key ";
    if (state == PSecureConfig::IsValid) {
      PTime expiry = securedConf.GetTime(securedConf.GetExpiryDateKey());
      if (expiry.GetYear() < 2011)
        regPage << "is temporary, and will expire on "
                << expiry.AsString(PTime::LongDate);
      else
        regPage << "is permanent.";
      doReorder = TRUE;

    } else if (state == PSecureConfig::Pending) {
      regPage << "has not yet arrived from "
              << process.GetManufacturer() << '.';
      doReorder = TRUE;

    } else {
      if (state == PSecureConfig::Expired)
        regPage << "is no longer valid.";
      else
        regPage << "has been changed or is not valid.";
      regPage << PHTML::Paragraph()
              << "You will have to enter valid registration information, or "
                 "remove the contents of the Validation entry field, and then"
                 "press the Accept button";
    } 

    if (doReorder) {
      regPage << PHTML::Paragraph()
              << PHTML::Form("GET", "/order.html")
              << "Do not change your registration details or "
                 "your key will not operate correctly."
              << PHTML::Paragraph()
              << "If you need to upgrade or change your registration, "
                 " then enter the new values into the fields above, and "
                 "then press the Accept button. Then return to this screen and "
                 " press the Order Form button below"
              << PHTML::Paragraph()
              << PHTML::SubmitButton("Order Form")
              << PHTML::Form();
    }

    AddDisclaimer(regPage, process.GetManufacturer());
  }

  regPage << process.GetCopyrightText()
          << PHTML::Body();

  SetString(regPage);
  return PConfigPage::LoadText(request);
}


BOOL PRegisterPage::Post(PHTTPRequest & request,
                         const PStringToString & data,
                         PHTML & reply)
{
  if (fields.IsEmpty())
    LoadText(request);

  if (!PConfigPage::Post(request, data, reply))
    return FALSE;

  PSecureConfig sconf(process.GetProductKey(), process.GetSecuredKeys());

  BOOL anyEmpty = FALSE;
  for (PINDEX i = 0; i < fields.GetSize(); i++) 
    anyEmpty = anyEmpty || fields[i].GetValue().Trim().IsEmpty();
  if (anyEmpty) {
    reply = "Registration Error";
    reply << "Your registration information contains at least one empty "
              "parameter. Please fill in all fields and try again."
          << PHTML::Body();
    request.code = PHTTP::InternalServerError;
    return FALSE;
  }

  BOOL good = FALSE;
  switch (sconf.GetValidation()) {
    case PSecureConfig::Defaults :
      sconf.ResetPending();
      // Then do IsValid case
    case PSecureConfig::IsValid :
      good = TRUE;
      break;
    case PSecureConfig::Pending :
      good = sconf.ValidatePending();
      break;
    default :
      sconf.ResetPending();
  }

  if (good)
    return TRUE;

  reply = "Registration Error";
  reply << "The entered security key is invalid." << PHTML::Body();
  request.code = PHTTP::InternalServerError;
  return FALSE;
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

POrderPage::POrderPage(PHTTPServiceProcess & app, PHTTPAuthority & auth)
  : PHTTPString("/order.html", auth),
    process(app)
{
}


PString POrderPage::LoadText(PHTTPRequest & request)
{
  PFile file;
  if (file.Open("order.html", PFile::ReadOnly)) {
    PString text = file.ReadString(file.GetLength());
    PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly), TRUE);
    return text;
  }

  PHTML html;
  process.GetPageHeader(html, process.GetName() & "Order Page");

  html << PHTML::Heading(1)
       << "Order Form"
       << PHTML::Heading(1)
       << PHTML::Paragraph()
       << PHTML::Form("POST", "mailto:" + process.GetEMailAddress())
       << "If you would like to send your credit card details by email, "
          "please fill out the form below:";

  html << PHTML::HiddenField("product", process.GetName())
	   << PHTML::HiddenField("os", process.GetOSClass() & process.GetOSName())
	   << PHTML::HiddenField("version", process.GetVersion(TRUE));

  PString reginfo;
  DigestSecuredKeys(process, reginfo, &html);

  html << PHTML::TableStart()
       << PHTML::TableRow("valign=baseline")
         << PHTML::TableHeader("align=right")
           << "Card Type:"
         << PHTML::TableData("align=left")
           << PHTML::RadioButton("PaymentType", "VISA")
           << "VISA "
           << PHTML::RadioButton("PaymentType", "Mastercard")
           << "Mastercard "
           << PHTML::RadioButton("PaymentType", "Bankcard")
           << "Bankcard "
           << PHTML::RadioButton("PaymentType", "Amex")
           << "Amex "
       << PHTML::TableRow("valign=baseline")
         << PHTML::TableHeader("align=right")
           << "Card Holder:"
         << PHTML::TableData("align=left")
           << PHTML::InputText("CardName", 40)

       << PHTML::TableRow("valign=baseline")
         << PHTML::TableHeader("align=right")
           << "Card No:"
         << PHTML::TableData("align=left")
           << PHTML::InputText("CardNum", 20)

       << PHTML::TableRow("valign=baseline")
         << PHTML::TableHeader("align=right")
           << "Expiry Date:"
         << PHTML::TableData("align=left")
           << PHTML::InputText("ExpiryDate", 8)

       << PHTML::TableRow("valign=baseline")
         << PHTML::TableHeader("align=right")
           << "Validation Number (Amex only):"
         << PHTML::TableData("align=left")
           << PHTML::InputText("ValNum", 8)

      << PHTML::TableEnd()
      << PHTML::Paragraph()
      << "If you are paying by some other method, please enter the details "
         "below. We recommend paying by credit card - you can fax or mail the "
         "information to us."
      << PHTML::BreakLine()
      << "<textarea name=\"Info\" rows=5 cols=60></textarea>"
      << PHTML::BreakLine()
      << "Upon receipt of your order, we will issue you a temporary key. "
         "We will issue a permanent key when payment has cleared."
      << PHTML::Paragraph()
      << PHTML::SubmitButton("Send Order Now")
      << PHTML::Form()
      << PHTML::Paragraph()
      << PHTML::Small()
      << PHTML::PreFormat()
      << reginfo
      << PHTML::PreFormat() << PHTML::Small()
      << PHTML::HRule()
      << process.GetCopyrightText()
      << PHTML::Body();

  return html;
}


///////////////////////////////////////////////////////////////////

PServiceHTML::PServiceHTML(const char * title, const char * help)
{
  PHTTPServiceProcess::Current().GetPageHeader(*this, title);

  *this << PHTML::Heading(1);
  
  if (help != NULL)
    *this << title
          << ' '
          << PHTML::HotLink(help)
          << PHTML::Image("/help.gif", "Help", 48, 23, "align=absmiddle")
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
  PMessageDigest5::Code md5;
  PMessageDigest5::Encode(out, md5);

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

static void ReplaceIncludes(PHTTPRequest & request, PString & text)
{
  PHTTPServiceProcess & process = PHTTPServiceProcess::Current();

  for (;;) {
    PINDEX pos = text.Find("<!--#equival");
    PINDEX end = text.Find("-->", pos);
    if (pos == P_MAX_INDEX || end == P_MAX_INDEX)
      break;

    PString subs;
    PCaselessString cmd = text(pos+12, end-1).Trim();
    if (cmd == "header") {
      subs = process.GetPageGraphic();
      ReplaceIncludes(request, subs);
    }

    else if (cmd == "copyright")
      subs = process.GetCopyrightText();
    else if (cmd == "os")
      subs = process.GetOSClass() & process.GetOSName();
    else if (cmd == "version")
      subs = process.GetVersion(TRUE);
    else if (cmd == "localhost")
      subs = PIPSocket::GetHostName();
    else if (cmd == "peerhost")
      subs = PIPSocket::GetHostName(request.origin);
    else if (cmd == "reginfo")
      DigestSecuredKeys(process, subs, NULL);
    else if (cmd == "registration") {
      PSecureConfig sconf(process.GetProductKey(), process.GetSecuredKeys());
      PString pending = sconf.GetPendingPrefix();
      PHTML out = PHTML::InBody;
      out << PHTML::Heading(3)
          << sconf.GetString("Name", sconf.GetString(pending+"Name",
                             "*** Unregistered Demonstration Copy ***"))
          << PHTML::Heading(3)
          << PHTML::Heading(4)
          << sconf.GetString("Company", sconf.GetString(pending+"Company"))
          << PHTML::Heading(4)
          << PHTML::Paragraph();

      if (sconf.GetString("Name").IsEmpty())
        process.AddUnregisteredText(out);
      else
        process.AddRegisteredText(out);

      out << PHTML::HotLink("/register.html")
          << (sconf.GetString("Name").IsEmpty()
                                   ? "Register Now!" : "View Registration")
          << PHTML::HotLink();
      subs = out;
    } else 
      process.SubstituteEquivalSequence(request, cmd, subs);

    text.Splice(subs, pos, end-pos+3);
  }
}



BOOL PServiceHTML::ProcessMacros(PHTTPRequest & request,
                                 PString & text,
                                 const PString & filename,
                                 BOOL needSignature)
{
  PHTTPServiceProcess & process = PHTTPServiceProcess::Current();

  if (needSignature) {
    if (!CheckSignature(text)) {
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

  ReplaceIncludes(request, text);

  return TRUE;
}


///////////////////////////////////////////////////////////////////

static void ServiceOnLoadedText(PString & text)
{
  PHTTPServiceProcess & process = PHTTPServiceProcess::Current();

  text.Replace("<!--Standard_" + process.GetManufacturer() + "_Header-->",
               process.GetPageGraphic(), TRUE);
  text.Replace("<!--Standard_Equivalence_Header-->",
               process.GetPageGraphic(), TRUE);
  text.Replace("<!--Standard_Copyright_Header-->",
               process.GetCopyrightText(), TRUE);
}

PString PServiceHTTPString::LoadText(PHTTPRequest & request)
{
  PString text = PHTTPString::LoadText(request);
  ServiceOnLoadedText(text);
  PServiceHTML::ProcessMacros(request, text, "", FALSE);

  return text;
}

void PServiceHTTPFile::OnLoadedText(PHTTPRequest & request, PString & text)
{
  ServiceOnLoadedText(text);
  PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly), needSignature);
}

void PServiceHTTPDirectory::OnLoadedText(PHTTPRequest & request, PString & text)
{
  ServiceOnLoadedText(text);
  PServiceHTML::ProcessMacros(request, text, baseURL.AsString(PURL::PathOnly), needSignature);
}


///////////////////////////////////////////////////////////////////
