/*
 * $Id: httpform.cxx,v 1.5 1997/04/01 06:00:53 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: httpform.cxx,v $
 * Revision 1.5  1997/04/01 06:00:53  robertj
 * Changed PHTTPConfig so if section empty string, does not write PConfig parameters.
 *
 * Revision 1.4  1996/10/08 13:10:34  robertj
 * Fixed bug in boolean (checkbox) html forms, cannot be reset.
 *
 * Revision 1.3  1996/09/14 13:09:31  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.2  1996/08/08 13:34:10  robertj
 * Removed redundent call.
 *
 * Revision 1.1  1996/06/28 12:56:20  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <httpform.h>


//////////////////////////////////////////////////////////////////////////////
// PHTTPField

PHTTPField::PHTTPField(const char * nam, const char * titl, const char * hlp)
  : name(nam),
    title(titl != NULL ? titl : nam),
    help(hlp != NULL ? hlp : "")
{
  notInHTML = TRUE;
}

PObject::Comparison PHTTPField::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PHTTPField::Class()), PInvalidCast);
  return name.Compare(((const PHTTPField &)obj).name);
}


void PHTTPField::SetHelp(const PString & hotLinkURL,
                         const PString & linkText)
{
  help = "<A HREF=\"" + hotLinkURL + "\">" + linkText + "</A>\r\n";
}


void PHTTPField::SetHelp(const PString & hotLinkURL,
                         const PString & imageURL,
                         const PString & imageText)
{
  help = "<A HREF=\"" + hotLinkURL + "\"><IMG SRC=\"" +
             imageURL + "\" ALT=\"" + imageText + "\" ALIGN=absmiddle></A>\r\n";
}


BOOL PHTTPField::Validated(const PString &, PStringStream &) const
{
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPStringField

PHTTPStringField::PHTTPStringField(const char * name,
                                   PINDEX siz,
                                   const char * initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help), value(initVal != NULL ? initVal : "")
{
  size = siz;
}


PHTTPStringField::PHTTPStringField(const char * name,
                                   const char * title,
                                   PINDEX siz,
                                   const char * initVal,
                                   const char * help)
  : PHTTPField(name, title, help), value(initVal != NULL ? initVal : "")
{
  size = siz;
}


void PHTTPStringField::GetHTML(PHTML & html)
{
  html << PHTML::InputText(name, size, value);
  notInHTML = FALSE;
}


void PHTTPStringField::SetValue(const PString & val)
{
  value = val;
}


PString PHTTPStringField::GetValue() const
{
  return value;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPPasswordField

PHTTPPasswordField::PHTTPPasswordField(const char * name,
                                       PINDEX siz,
                                       const char * initVal,
                                       const char * help)
  : PHTTPStringField(name, siz, initVal, help)
{
}


PHTTPPasswordField::PHTTPPasswordField(const char * name,
                                       const char * title,
                                       PINDEX siz,
                                       const char * initVal,
                                       const char * help)
  : PHTTPStringField(name, title, siz, initVal, help)
{
}


void PHTTPPasswordField::GetHTML(PHTML & html)
{
  html << PHTML::InputPassword(name, size, value);
  notInHTML = FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPIntegerField

PHTTPIntegerField::PHTTPIntegerField(const char * nam,
                                     int lo, int hig,
                                     int initVal,
                                     const char * unit,
                                     const char * help)
  : PHTTPField(nam, NULL, help), units(unit != NULL ? unit : "")
{
  low = lo;
  high = hig;
  value = initVal;
}

PHTTPIntegerField::PHTTPIntegerField(const char * nam,
                                     const char * titl,
                                     int lo, int hig,
                                     int initVal,
                                     const char * unit,
                                     const char * help)
  : PHTTPField(nam, titl, help), units(unit != NULL ? unit : "")
{
  low = lo;
  high = hig;
  value = initVal;
}


BOOL PHTTPIntegerField::Validated(const PString & newVal,
                                                     PStringStream & msg) const
{
  int val = newVal.AsInteger();
  if (val >= low && val <= high)
    return TRUE;

  msg << "The field \"" << name << "\" should be between "
      << low << " and " << high << ".<BR>";
  return FALSE;
}


void PHTTPIntegerField::GetHTML(PHTML & html)
{
  html << PHTML::InputRange(name, low, high, value) << "  " << units;
  notInHTML = FALSE;
}


void PHTTPIntegerField::SetValue(const PString & val)
{
  value = val.AsInteger();
}


PString PHTTPIntegerField::GetValue() const
{
  return PString(PString::Signed, value);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPBooleanField

PHTTPBooleanField::PHTTPBooleanField(const char * name,
                                     BOOL initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help)
{
  value = initVal;
}


PHTTPBooleanField::PHTTPBooleanField(const char * name,
                                     const char * title,
                                     BOOL initVal,
                                     const char * help)
  : PHTTPField(name, title, help)
{
  value = initVal;
}


void PHTTPBooleanField::GetHTML(PHTML & html)
{
  html << PHTML::CheckBox(name, value ? PHTML::Checked : PHTML::UnChecked);
  notInHTML = FALSE;
}


void PHTTPBooleanField::SetValue(const PString & val)
{
  value = val[0] == 'T';
}


PString PHTTPBooleanField::GetValue() const
{
  return (value ? "T" : "F");
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPRadioField

PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const PStringArray & valueArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(valueArray),
    titles(valueArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const PStringArray & valueArray,
                                 const PStringArray & titleArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(valueArray),
    titles(titleArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(count, valueStrings),
    titles(count, valueStrings),
    value(valueStrings[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 const char * const * titleStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, NULL, help),
    values(count, valueStrings),
    titles(count, titleStrings),
    value(valueStrings[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 const PStringArray & valueArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(valueArray),
    titles(valueArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 const PStringArray & valueArray,
                                 const PStringArray & titleArray,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(valueArray),
    titles(titleArray),
    value(valueArray[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(count, valueStrings),
    titles(count, valueStrings),
    value(valueStrings[initVal])
{
}


PHTTPRadioField::PHTTPRadioField(const char * name,
                                 const char * groupTitle,
                                 PINDEX count,
                                 const char * const * valueStrings,
                                 const char * const * titleStrings,
                                 PINDEX initVal,
                                 const char * help)
  : PHTTPField(name, groupTitle, help),
    values(count, valueStrings),
    titles(count, titleStrings),
    value(valueStrings[initVal])
{
}


void PHTTPRadioField::GetHTML(PHTML & html)
{
  for (PINDEX i = 0; i < values.GetSize(); i++)
    html << PHTML::RadioButton(name, values[i],
                        values[i] == value ? PHTML::Checked : PHTML::UnChecked)
         << titles[i]
         << PHTML::BreakLine();
  notInHTML = FALSE;
}


PString PHTTPRadioField::GetValue() const
{
  return value;
}


void PHTTPRadioField::SetValue(const PString & val)
{
  value = val;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSelectField

PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const PStringArray & valueArray,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help),
    values(valueArray)
{
  if (initVal < valueArray.GetSize())
    value = valueArray[initVal];
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   PINDEX count,
                                   const char * const * valueStrings,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help),
    values(count, valueStrings)
{
  if (initVal < count)
    value = valueStrings[initVal];
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const char * title,
                                   const PStringArray & valueArray,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, title, help),
    values(valueArray)
{
  if (initVal < valueArray.GetSize())
    value = valueArray[initVal];
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const char * title,
                                   PINDEX count,
                                   const char * const * valueStrings,
                                   PINDEX initVal,
                                   const char * help)
  : PHTTPField(name, title, help),
    values(count, valueStrings),
    value(valueStrings[initVal])
{
}


void PHTTPSelectField::GetHTML(PHTML & html)
{
  html << PHTML::Select(name);
  for (PINDEX i = 0; i < values.GetSize(); i++)
    html << PHTML::Option(
                    values[i] == value ? PHTML::Selected : PHTML::NotSelected)
         << values[i];
  html << PHTML::Select();
  notInHTML = FALSE;
}


PString PHTTPSelectField::GetValue() const
{
  return value;
}


void PHTTPSelectField::SetValue(const PString & val)
{
  value = val;
}



//////////////////////////////////////////////////////////////////////////////
// PHTTPForm

PHTTPForm::PHTTPForm(const PURL & url)
  : PHTTPString(url)
{
}

PHTTPForm::PHTTPForm(const PURL & url, const PHTTPAuthority & auth)
  : PHTTPString(url, auth)
{
}

PHTTPForm::PHTTPForm(const PURL & url, const PString & html)
  : PHTTPString(url, html)
{
}

PHTTPForm::PHTTPForm(const PURL & url,
                     const PString & html,
                     const PHTTPAuthority & auth)
  : PHTTPString(url, html, auth)
{
}


PHTTPField * PHTTPForm::Add(PHTTPField * fld)
{
  PAssertNULL(fld);
  PAssert(!fieldNames[fld->GetName()], "Field already on form!");
  fieldNames += fld->GetName();
  fields.Append(fld);
  return fld;
}


void PHTTPForm::BuildHTML(const char * heading)
{
  PHTML html = heading;
  BuildHTML(html);
}


void PHTTPForm::BuildHTML(const PString & heading)
{
  PHTML html = heading;
  BuildHTML(html);
}


void PHTTPForm::BuildHTML(PHTML & html, BuildOptions option)
{
  if (!html.Is(PHTML::InForm))
    html << PHTML::Form("POST");

  html << PHTML::TableStart();
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (field.NotYetInHTML()) {
      html << PHTML::TableRow()
           << PHTML::TableData("align=right")
           << field.GetTitle()
           << PHTML::TableData("align=left")
           << "<!--PHTTPForm NAME=" << field.GetName() << "-->";
      field.GetHTML(html);
      html << "<!--/PHTTPForm-->"
           << PHTML::TableData()
           << field.GetHelp();
    }
  }
  html << PHTML::TableEnd();
  if (option != InsertIntoForm)
    html << PHTML::Paragraph()
         << ' ' << PHTML::SubmitButton("Accept")
         << ' ' << PHTML::ResetButton("Reset")
         << PHTML::Form();

  if (option == CompleteHTML) {
    html << PHTML::Body();
    string = html;
  }
}


BOOL PHTTPForm::Post(PHTTPRequest & request,
                     const PStringToString & data,
                     PHTML & msg)
{
  msg = "Error in Request";
  if (data.GetSize() == 0) {
    msg << "No parameters changed." << PHTML::Body();
    request.code = PHTTP::NoContent;
    return TRUE;
  }

  BOOL good = TRUE;
  PINDEX fld;
  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    const PCaselessString & name = field.GetName();
    if (data.Contains(name) && !field.Validated(data[name], msg))
      good = FALSE;
  }

  if (!good) {
    msg << PHTML::Body();
    request.code = PHTTP::BadRequest;
    return TRUE;
  }

  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    const PCaselessString & name = field.GetName();
    if (data.Contains(name))
      field.SetValue(data[name]);
    else
      field.SetValue("");
  }

  msg = "Accepted New Configuration";
  msg << PHTML::Body();
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPConfig

PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & sect)
  : PHTTPForm(url), section(sect)
{
  Construct();
}


PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & sect,
                         const PHTTPAuthority & auth)
  : PHTTPForm(url, auth), section(sect)
{
  Construct();
}


PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & html,
                         const PString & sect)
  : PHTTPForm(url, html), section(sect)
{
  Construct();
}


PHTTPConfig::PHTTPConfig(const PURL & url,
                         const PString & html,
                         const PString & sect,
                         const PHTTPAuthority & auth)
  : PHTTPForm(url, html, auth), section(sect)
{
  Construct();
}


void PHTTPConfig::Construct()
{
  sectionField = NULL;
  keyField = NULL;
  valField = NULL;
}


void PHTTPConfig::OnLoadedText(PHTTPRequest & request, PString & text)
{
  if (sectionField != NULL)
    return;

  PString sectionName = request.url.GetQueryVars()("section", section);
  if (sectionName.IsEmpty())
    return;

  PConfig cfg(sectionName);
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    field.SetValue(cfg.GetString(field.GetName(), field.GetValue()));
    PINDEX start = text.Find("<!--PHTTPForm NAME=" + field.GetName() + "-->");
    if (start != P_MAX_INDEX) {
      PINDEX end = text.Find("<!--/PHTTPForm-->", start);
      PAssert(end != P_MAX_INDEX, PLogicError);
      PHTML html = PHTML::InForm;
      field.GetHTML(html);
      text.Splice(html, start, end - start + 17);
    }
  }
}


BOOL PHTTPConfig::Post(PHTTPRequest & request,
                       const PStringToString & data,
                       PHTML & msg)
{
  PHTTPForm::Post(request, data, msg);
  if (request.code != PHTTP::OK)
    return TRUE;

  if (sectionField != NULL)
    section = sectionPrefix + sectionField->GetValue() + sectionSuffix;

  PString sectionName = request.url.GetQueryVars()("section", section);
  if (sectionName.IsEmpty())
    return TRUE;

  PConfig cfg(sectionName);
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (&field == keyField) {
      PString key = field.GetValue();
      if (!key)
        cfg.SetString(key, valField->GetValue());
    }
    else if (&field != valField && &field != sectionField)
      cfg.SetString(field.GetName(), field.GetValue());
  }
  return TRUE;
}


PHTTPField * PHTTPConfig::AddSectionField(PHTTPField * sectionFld,
                                          const char * prefix,
                                          const char * suffix)
{
  sectionField = PAssertNULL(sectionFld);
  Add(sectionField);

  if (prefix != NULL)
    sectionPrefix = prefix;
  if (suffix != NULL)
    sectionSuffix = suffix;

  return sectionField;
}


void PHTTPConfig::AddNewKeyFields(PHTTPField * keyFld,
                                  PHTTPField * valFld)
{
  keyField = PAssertNULL(keyFld);
  Add(keyFld);
  valField = PAssertNULL(valFld);
  Add(valFld);
}


// End Of File ///////////////////////////////////////////////////////////////
