/*
 * $Id: httpsvc.cxx,v 1.3 1996/07/15 10:36:48 robertj Exp $
 *
 * Common classes for service applications using HTTP as the user interface.
 *
 * Copyright 1995-1996 Equivalence
 *
 * $Log: httpsvc.cxx,v $
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

#define ORGANISATION "Equivalence"
#define HOME_PAGE "http://www.ozemail.com.au/~equival"
#define EMAIL "mailto:equival@ozemail.com.au"


PHTTPServiceProcess::PHTTPServiceProcess(
                  const char * _gifName,
                  const char * manuf,   // Name of manufacturer
                  const char * name,    // Name of product
                  WORD majorVersion,    // Major version number of the product
                  WORD minorVersion,    // Minor version number of the product
                  CodeStatus status,    // Development status of the product
                  WORD buildNumber      // Build number of the product
                )
  : PServiceProcess(manuf, name, majorVersion,
                    minorVersion, status, buildNumber),
    gifText(_gifName)
{
  restartSystem = FALSE;
}


PString PHTTPServiceProcess::GetPageGraphic()
{
  PString str;
  str = PString("<table><tr><td>") + gifText +
        "<td>" + GetOSClass() + '/' + GetOSName() +
        " Version " + GetVersion(TRUE) +
        ", " + PTime(__DATE__).AsString("d MMM yy") +
        "<br>Copyright &copy;1996 by "
        "<A HREF=\"" HOME_PAGE "\">" ORGANISATION "</A>, "
        "<A HREF=\"" EMAIL "\">equival@ozemail.com.au</A>"
        "</table>";
  return str;
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


//////////////////////////////////////////////////////////////

void PHTTPServiceThread::Main()
{
  // get a socket when a client connects
  PHTTPSocket socket(listener, httpSpace);
  if (!socket.IsOpen())
    return;

  PNEW PHTTPServiceThread(process, listener, httpSpace);

  // process requests
  while (socket.ProcessCommand())
    ;

  // always close after the response has been sent
  socket.Close();

  // if a restart was requested, then do it
  if (process.GetRestartSystem()) {
    process.OnConfigChanged();
    process.SetRestartSystem(FALSE);
    if (!process.Initialise("Configuration changed - reloaded"))
      process.Terminate();
  }
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


BOOL PConfigPage::OnPOST(PHTTPSocket & socket,
                         const PURL & url,
                         const PMIMEInfo & info,
                         const PStringToString & data,
                         const PHTTPConnectionInfo & connectInfo)
{
  PHTTPConfig::OnPOST(socket, url, info, data, connectInfo);
  return FALSE;    // Make sure we break any persistent connections
}


BOOL PConfigPage::Post(PHTTPRequest & request,
                       const PStringToString & data,
                       PHTML & reply)
{
  BOOL retval = PHTTPConfig::Post(request, data, reply);
  if (request.code == PHTTPSocket::OK) {
    process.SetRestartSystem(TRUE);
    process.OnConfigChanged();
  }
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
  : PConfigPage(app, "register.html", securedConf.GetDefaultSection(), auth)
{
  securedKeys = securedConf.GetSecuredKeys();
  securedConf.GetProductKey(productKey);

  static const char disclaimer[] = "The information and code herein is "
      "provided \"as is\" without warranty of any kind, either expressed or "
      "implied, including but not limited to the implied warrenties of "
      "merchantability and fitness for a particular purpose. In no event "
      "shall " ORGANISATION " be liable for any damages whatsoever including "
      "direct, indirect, incidental, consequential, loss of business profits "
      "or special damages, even if " ORGANISATION " has been advised of the "
      "possibility of such damages.";

  PServiceHTML regPage(app.GetName() & "Registration", "reghelp.html");

  PSecureConfig::ValidationState state = securedConf.GetValidation();

  PString prefix;

  if (state != PSecureConfig::IsValid) 
    prefix = securedConf.GetPendingPrefix();

  AddFields(prefix);
  if (state != PSecureConfig::Defaults)
    Add(new PHTTPStringField("Validation", 34));

  if (state == PSecureConfig::Defaults) {
    regPage << PHTML::HRule()
            << PHTML::Heading(3) << "Disclaimer" << PHTML::Heading(3)
            << PHTML::Paragraph() << PHTML::Bold()
            << disclaimer
            << PHTML::Bold() << PHTML::Paragraph();
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
      regPage << "has not yet arrived from " ORGANISATION ".";
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

    regPage << PHTML::HRule()
            << PHTML::Heading(3) << "Disclaimer" << PHTML::Heading(3)
            << PHTML::Paragraph() << PHTML::Bold()
            << disclaimer
            << PHTML::Bold() << PHTML::Paragraph();
  }

  regPage << PHTML::Body();
  SetString(regPage);
}


BOOL PRegisterPage::Post(PHTTPRequest & request,
                         const PStringToString & data,
                         PHTML & reply)
{
  if (!PConfigPage::Post(request, data, reply))
    return FALSE;

  PSecureConfig sconf(productKey, securedKeys);
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
  request.code = PHTTPSocket::InternalServerError;
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
       << PHTML::Form("POST", EMAIL)
       << "If you would like to send your credit card details by email, "
          "please fill out the form below:";

  PSecureConfig sconf(productKey, securedKeys);
  PString prefix;
  if (sconf.GetValidation() != PSecureConfig::IsValid) 
    prefix = sconf.GetPendingPrefix();

  html << PHTML::HiddenField("product", process.GetName())
       << PHTML::HiddenField("digest",  sconf.CalculatePendingDigest());

  PINDEX i;
  for (i = 0; i < securedKeys.GetSize(); i++) 
    html << PHTML::HiddenField(securedKeys[i],
                              sconf.GetString(prefix + securedKeys[i]).Trim());

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
      << sconf.CalculatePendingDigest();
  for (i = 0; i < securedKeys.GetSize(); i++) 
    html << " \"" << sconf.GetString(prefix + securedKeys[i]).Trim() << '"';
  html << PHTML::PreFormat() << PHTML::Small() << PHTML::Body();

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
  text.Replace("<!--Standard_" ORGANISATION "_Header-->",
               PHTTPServiceProcess::Current()->GetPageGraphic());
}


///////////////////////////////////////////////////////////////////
