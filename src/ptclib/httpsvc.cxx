/*
 * $Id: httpsvc.cxx,v 1.12 1997/01/03 06:33:23 robertj Exp $
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Copyright 1995-1996 Equivalence
 *
 * $Log: httpsvc.cxx,v $
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
                  const char * _email      // contact email for manufacturer
                )
  : PServiceProcess(manuf, name, majorVersion,
                    minorVersion, status, buildNumber),
    gifText(_gifName) 
{
  if (_email != NULL)
    email = _email;
  else
    email = EMAIL;
  if (_homePage != NULL)
    homePage = _homePage;
  else
    homePage = HOME_PAGE;

  restartThread = NULL;
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
  PTime compilationDate = PString(__DATE__);

  PHTML html = PHTML::InBody;
  html << PHTML::TableStart()
       << PHTML::TableRow()
       << PHTML::TableData()
       << gifText
       << PHTML::TableData()
       << GetOSClass() << ' ' << GetOSName()
       << " Version " << GetVersion(TRUE)
       << ", " << compilationDate.AsString("d MMM yy")
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


//////////////////////////////////////////////////////////////

void PHTTPServiceThread::Main()
{
  // get a socket when a client connects
  PHTTPServer server(httpSpace);
  if (!server.Accept(listener))
    return;

  PNEW PHTTPServiceThread(process, listener, httpSpace);

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
  when = PTime();  // Now
  return TRUE;
}


//////////////////////////////////////////////////////////////

PRegisterPage::PRegisterPage(PHTTPServiceProcess & app,
                             const PSecureConfig & securedConf,
                             const PHTTPAuthority & auth)
  : PConfigPage(app, "register.html", securedConf.GetDefaultSection(), auth),
    process(app)
{
  securedKeys = securedConf.GetSecuredKeys();
  securedConf.GetProductKey(productKey);
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

  PSecureConfig securedConf(productKey, securedKeys);
  PSecureConfig::ValidationState state = securedConf.GetValidation();

  PString prefix;

  if (state != PSecureConfig::IsValid) 
    prefix = securedConf.GetPendingPrefix();

  AddFields(prefix);

  if (state != PSecureConfig::Defaults)
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
  if (!PConfigPage::Post(request, data, reply))
    return FALSE;

  PSecureConfig sconf(productKey, securedKeys);

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

POrderPage::POrderPage(PHTTPAuthority & auth, const PSecureConfig & securedConf)
  : PHTTPString("/order.html", auth)
{
  securedKeys = securedConf.GetSecuredKeys();
  securedConf.GetProductKey(productKey);
}


PString POrderPage::LoadText(PHTTPRequest &)
{
  PHTML html;

  PHTTPServiceProcess & process = *PHTTPServiceProcess::Current();
  process.GetPageHeader(html, process.GetName() & "Order Page");

  html << PHTML::Heading(1)
       << "Order Form"
       << PHTML::Heading(1)
       << PHTML::Paragraph()
       << PHTML::Form("POST", "mailto:" + process.email)
       << "If you would like to send your credit card details by email, "
          "please fill out the form below:";

  PSecureConfig sconf(productKey, securedKeys);
  PString prefix;
  if (sconf.GetValidation() != PSecureConfig::IsValid) 
    prefix = sconf.GetPendingPrefix();

  html << PHTML::HiddenField("product", process.GetName());

  PMessageDigest5 digestor;

  PINDEX i;
  for (i = 0; i < securedKeys.GetSize(); i++) {
    PString val = sconf.GetString(prefix + securedKeys[i]).Trim();
    html << PHTML::HiddenField(securedKeys[i], val);
    digestor.Process(val);
  }

  PString digest = digestor.Complete();

  html << PHTML::HiddenField("digest", digest)

       << PHTML::TableStart()
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
      << '"' << process.GetName() << "\" "
      << digest;

  for (i = 0; i < securedKeys.GetSize(); i++) 
    html << " \"" << sconf.GetString(prefix + securedKeys[i]).Trim() << '"';

  html << PHTML::PreFormat() << PHTML::Small()
       << PHTML::HRule()
       << process.GetCopyrightText()
       << PHTML::Body();

  return html;
}


///////////////////////////////////////////////////////////////////

PServiceHTML::PServiceHTML(const char * title, const char * help)
{
  PHTTPServiceProcess::Current()->GetPageHeader(*this, title);

  *this << PHTML::Heading(1);
  
  if (help != NULL)
    *this << title
          << ' '
          << PHTML::HotLink(help)
          << PHTML::Image("/help.gif", "Help", 48, 23, "align=absmiddle")
          << PHTML::HotLink();

  *this << PHTML::Heading(1) << PHTML::Paragraph();
}


///////////////////////////////////////////////////////////////////

void PServiceHTTPFile::OnLoadedText(PHTTPRequest &, PString & text)
{
  PHTTPServiceProcess & process = *PHTTPServiceProcess::Current();
  text.Replace("<!--Standard_" + process.GetManufacturer() + "_Header-->",
               process.GetPageGraphic());
  text.Replace("<!--Standard_Copyright_Header-->",
               process.GetCopyrightText());
}


///////////////////////////////////////////////////////////////////
