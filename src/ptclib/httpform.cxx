/*
 * $Id: httpform.cxx,v 1.10 1997/07/26 11:38:20 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: httpform.cxx,v $
 * Revision 1.10  1997/07/26 11:38:20  robertj
 * Support for overridable pages in HTTP service applications.
 *
 * Revision 1.9  1997/07/14 11:49:51  robertj
 * Put "Add" and "Keep" on check boxes in array fields.
 *
 * Revision 1.8  1997/07/08 13:12:29  robertj
 * Major HTTP form enhancements for lists and arrays of fields.
 *
 * Revision 1.7  1997/06/08 04:47:27  robertj
 * Adding new llist based form field.
 *
 * Revision 1.6  1997/04/12 02:07:26  robertj
 * Fixed boolean check boxes being more flexible on string values.
 *
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
#include <cypher.h>


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


PCaselessString PHTTPField::GetNameAt(PINDEX) const
{
  return name;
}


void PHTTPField::SetName(const PString & newName)
{
  name = newName;
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


PINDEX PHTTPField::GetSize() const
{
  return 1;
}


PString PHTTPField::GetHTMLKeyword() const
{
  return "value";
}


void PHTTPField::GetHTMLHeading(PHTML &)
{
}


PString PHTTPField::GetValueAt(PINDEX) const
{
  return GetValue();
}


void PHTTPField::SetValueAt(PINDEX, const PString & value)
{
  SetValue(value);
}


BOOL PHTTPField::Validated(const PString &, PStringStream &) const
{
  return TRUE;
}


void PHTTPField::SetAllValues(const PStringToString & data)
{
  SetValue(data(name));
}


BOOL PHTTPField::ValidateAll(const PStringToString & data, PStringStream & msg) const
{
  return Validated(data(name), msg);
}


static BOOL FindSplice(const PString & keyword,
                       const PString & name,
                       const PString & text,
                       PINDEX offset,
                       PINDEX & pos,
                       PINDEX & len)
{
  PRegularExpression regex = "<!--#form[ \t\n]*" + keyword + "[ \t\n]*" + name + "[ \t\n]*-->";
  PIntArray starts;
  PIntArray ends;
  if (!regex.Execute(offset+(const char *)text, starts, ends)) {
    len = 0;
    return FALSE;
  }
  pos = starts[0] + offset;
  len = ends[0] - starts[0];
  return TRUE;
//  PString arg = "<!--#form " + keyword + " " + name + "-->";
//  len = arg.GetLength();
//  pos = text.Find(arg, offset);
//  return pos != P_MAX_INDEX;
}


static void SpliceFields(PINDEX prefix, const PHTTPFieldList & fields, PString & text)
{
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    PString name = field.GetName().Mid(prefix);
    PINDEX pos, len;
    if (FindSplice("html", name, text, 0, pos, len)) {
      PHTML html = PHTML::InForm;
      field.GetHTMLTag(html);
      text.Splice(html, pos, len);
    }
    if (FindSplice(field.GetHTMLKeyword(), name, text, 0, pos, len)) {
      PINDEX endpos, endlen;
      if (!FindSplice("end", name, text, pos, endpos, endlen))
        endpos = pos + len;
      text.Splice(field.GetHTMLValue(text(pos + len, endpos-1)),
                  pos, endpos - pos + endlen);
    }
  }
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPCompositeField

PHTTPCompositeField::PHTTPCompositeField(const char * nam,
                                         const char * titl,
                                         const char * hlp)
  : PHTTPField(nam, titl, hlp)
{
}


PCaselessString PHTTPCompositeField::GetNameAt(PINDEX idx) const
{
  PINDEX base = 0;
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    PINDEX fieldSize = fields[i].GetSize();
    if (idx - base < fieldSize)
      return fields[i].GetNameAt(idx - base);
    base += fieldSize;
  }

  return GetName();
}


void PHTTPCompositeField::SetName(const PString & newName)
{
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    PHTTPField & field = fields[i];
    PINDEX pos = 0;
    if (field.GetName().Find(name) == 0)
      pos = name.GetLength();
    field.SetName(newName & field.GetName().Mid(pos));
  }

  name = newName;
}


PINDEX PHTTPCompositeField::GetSize() const
{
  PINDEX total = 0;
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    total += fields[i].GetSize();
  return total;
}


PHTTPField * PHTTPCompositeField::NewField() const
{
  PHTTPCompositeField * fld = new PHTTPCompositeField(name, title, help);
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    fld->Append(fields[i].NewField());
  return fld;
}


void PHTTPCompositeField::GetHTMLTag(PHTML & html)
{
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    if (i != 0)
      html << PHTML::TableData();
    fields[i].GetHTMLTag(html);
  }
}


PString PHTTPCompositeField::GetHTMLValue(const PString & original)
{
  PString text = original;
  SpliceFields(name.GetLength()+1, fields, text);
  return text;
}


PString PHTTPCompositeField::GetHTMLKeyword() const
{
  return "";
}


void PHTTPCompositeField::GetHTMLHeading(PHTML & html)
{
  html << PHTML::TableRow();
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    html << PHTML::TableData() << fields[i].GetName();
}


PString PHTTPCompositeField::GetValue() const
{
  PString value;
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    value += fields[i].GetValue();
  return value;
}


PString PHTTPCompositeField::GetValueAt(PINDEX idx) const
{
  PINDEX base = 0;
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    PINDEX fieldSize = fields[i].GetSize();
    if (idx - base < fieldSize)
      return fields[i].GetValueAt(idx - base);
    base += fieldSize;
  }

  return PString();
}


void PHTTPCompositeField::SetValue(const PString &)
{
  PAssertAlways("PHTTPCompositeField::SetValue() called");
}


void PHTTPCompositeField::SetValueAt(PINDEX idx, const PString & value)
{
  PINDEX base = 0;
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    PINDEX fieldSize = fields[i].GetSize();
    if (idx - base < fieldSize) {
      fields[i].SetValueAt(idx - base, value);
      break;
    }
    base += fieldSize;
  }
}


void PHTTPCompositeField::SetAllValues(const PStringToString & data)
{
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    fields[i].SetAllValues(data);
}


BOOL PHTTPCompositeField::ValidateAll(const PStringToString & data,
                                      PStringStream & msg) const
{
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    if (!fields[i].ValidateAll(data, msg))
      return FALSE;

  return TRUE;
}


void PHTTPCompositeField::Append(PHTTPField * fld)
{
  fields.Append(fld);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFieldArray

PHTTPFieldArray::PHTTPFieldArray(PHTTPField * fld)
  : PHTTPCompositeField(fld->GetName(), fld->GetTitle(), fld->GetHelp()),
    baseField(fld)
{
  AddBlankField();
}


PHTTPFieldArray::~PHTTPFieldArray()
{
  delete baseField;
}


PCaselessString PHTTPFieldArray::GetNameAt(PINDEX idx) const
{
  if (idx > 0)
    return PHTTPCompositeField::GetNameAt(idx-1);

  return name & "Array Size";
}


PINDEX PHTTPFieldArray::GetSize() const
{
  return PHTTPCompositeField::GetSize() + 1;
}


PHTTPField * PHTTPFieldArray::NewField() const
{
  return new PHTTPFieldArray(baseField->NewField());
}


static const char IncludeCheckBox[] = "Include";

void PHTTPFieldArray::GetHTMLTag(PHTML & html)
{
  html << PHTML::TableStart();
  baseField->GetHTMLHeading(html);
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    html << PHTML::TableRow() << PHTML::TableData();
    fields[i].GetHTMLTag(html);
    html << PHTML::TableData();
    PString chkboxName = IncludeCheckBox & fields[i].GetName();
    if (i < fields.GetSize()-1)
      html << PHTML::CheckBox(chkboxName, PHTML::Checked) << " Keep";
    else
      html << PHTML::CheckBox(chkboxName, PHTML::UnChecked) << " Add";
  }
  html << PHTML::TableEnd();
}


PString PHTTPFieldArray::GetHTMLValue(const PString & original)
{
  PString text;
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PString row = fields[fld].GetHTMLValue(original);
    PINDEX pos,len;
    if (FindSplice("rowcheck", "", row, 0, pos, len)) {
      PHTML html = PHTML::InForm;
      PString chkboxName = IncludeCheckBox & fields[fld].GetName();
      if (fld < fields.GetSize()-1)
        html << PHTML::CheckBox(chkboxName, PHTML::Checked) << " Keep";
      else
        html << PHTML::CheckBox(chkboxName, PHTML::UnChecked) << " Add";
      row.Splice(html, pos, len);
    }
    if (FindSplice("rownum", "", row, 0, pos, len))
      row.Splice(psprintf("%u", fld+1), pos, len);
    text += row;
  }
  return text;
}


PString PHTTPFieldArray::GetHTMLKeyword() const
{
  return "array";
}


PString PHTTPFieldArray::GetValueAt(PINDEX idx) const
{
  if (idx > 0)
    return PHTTPCompositeField::GetValueAt(idx-1);

  return psprintf("%u", fields.GetSize());
}


void PHTTPFieldArray::SetValueAt(PINDEX idx, const PString & value)
{
  if (idx > 0)
    PHTTPCompositeField::SetValueAt(idx-1, value);
  else {
    PINDEX newSize = value.AsInteger();
    if (newSize == 0)
      newSize = 1;
    while (fields.GetSize() > newSize)
      fields.RemoveAt(fields.GetSize()-1);
    while (fields.GetSize() < newSize)
      AddBlankField();
  }
}


void PHTTPFieldArray::SetAllValues(const PStringToString & data)
{
  BOOL lastFieldIsSet = TRUE;
  PINDEX i;
  for (i = 0; i < fields.GetSize(); i++) {
    if (data.Contains(IncludeCheckBox & fields[i].GetName()))
      fields[i].SetAllValues(data);
    else if (i < fields.GetSize()-1)
      fields.SetAt(i, NULL);
    else
      lastFieldIsSet = FALSE;
  }
  for (i = 0; i < fields.GetSize(); i++) {
    if (fields.GetAt(i) == NULL) {
      fields.RemoveAt(i);
      for (PINDEX j = i; j < fields.GetSize(); j++)
        fields[j].SetName(name + psprintf(" %u", j+1));
      i--;
    }
  }
  if (lastFieldIsSet)
    AddBlankField();
}


void PHTTPFieldArray::AddBlankField()
{
  PHTTPField * fld = baseField->NewField();
  fields.Append(fld);
  fld->SetName(name + psprintf(" %u", fields.GetSize()));
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


PHTTPField * PHTTPStringField::NewField() const
{
  return new PHTTPStringField(name, title, size, "", help);
}


void PHTTPStringField::GetHTMLTag(PHTML & html)
{
  html << PHTML::InputText(name, size, value);
}


PString PHTTPStringField::GetHTMLValue(const PString &)
{
  return value;
}


void PHTTPStringField::SetValue(const PString & newVal)
{
  value = newVal;
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


PHTTPField * PHTTPPasswordField::NewField() const
{
  return new PHTTPPasswordField(name, title, size, "", help);
}


void PHTTPPasswordField::GetHTMLTag(PHTML & html)
{
  html << PHTML::InputPassword(name, size, value);
}


static const PTEACypher::Key PasswordKey = {
  103,  60, 222,  17, 128, 157,  31, 137,
  133,  64,  82, 148,  94, 136,   4, 209
};

void PHTTPPasswordField::SetValue(const PString & newVal)
{
  value = Decrypt(newVal);
}


PString PHTTPPasswordField::GetValue() const
{
  PTEACypher crypt(PasswordKey);
  return crypt.Encode(value);;
}


PString PHTTPPasswordField::Decrypt(const PString & pword)
{
  PTEACypher crypt(PasswordKey);
  PString clear = crypt.Decode(pword);
  return clear.IsEmpty() ? pword : clear;
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


PHTTPField * PHTTPIntegerField::NewField() const
{
  return new PHTTPIntegerField(name, title, low, high, 0, units, help);
}


void PHTTPIntegerField::GetHTMLTag(PHTML & html)
{
  html << PHTML::InputRange(name, low, high, value) << "  " << units;
}


PString PHTTPIntegerField::GetHTMLValue(const PString &)
{
  return PString(PString::Signed, value);
}


void PHTTPIntegerField::SetValue(const PString & newVal)
{
  value = newVal.AsInteger();
}


PString PHTTPIntegerField::GetValue() const
{
  return PString(PString::Signed, value);
}


BOOL PHTTPIntegerField::Validated(const PString & newVal, PStringStream & msg) const
{
  int val = newVal.AsInteger();
  if (val >= low && val <= high)
    return TRUE;

  msg << "The field \"" << name << "\" should be between "
      << low << " and " << high << ".<BR>";
  return FALSE;
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


PHTTPField * PHTTPBooleanField::NewField() const
{
  return new PHTTPBooleanField(name, title, FALSE, help);
}


void PHTTPBooleanField::GetHTMLTag(PHTML & html)
{
  html << PHTML::CheckBox(name, value ? PHTML::Checked : PHTML::UnChecked);
}


PString PHTTPBooleanField::GetHTMLValue(const PString &)
{
  return value ? "CHECKED" : "";
}


void PHTTPBooleanField::SetValue(const PString & val)
{
  value = toupper(val[0]) == 'T' || toupper(val[0]) == 'y' || val.AsInteger() != 0;
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


PHTTPField * PHTTPRadioField::NewField() const
{
  return new PHTTPRadioField(name, title, values, titles, 0, help);
}


void PHTTPRadioField::GetHTMLTag(PHTML & html)
{
  for (PINDEX i = 0; i < values.GetSize(); i++)
    html << PHTML::RadioButton(name, values[i],
                        values[i] == value ? PHTML::Checked : PHTML::UnChecked)
         << titles[i]
         << PHTML::BreakLine();
}


PString PHTTPRadioField::GetHTMLValue(const PString & original)
{
  PString text = original;
  for (PINDEX i = 0; i < values.GetSize(); i++) {
    PString arg = "<!--#form radio " + values[i] + "-->";
    PINDEX pos = text.Find(arg);
    if (pos == P_MAX_INDEX) {
      if (values[i] == value)
        text.Splice("CHECKED", pos, arg.GetLength());
      else
        text.Delete(pos, arg.GetLength());
    }
  }
  return text;
}


PString PHTTPRadioField::GetValue() const
{
  return value;
}


void PHTTPRadioField::SetValue(const PString & newVal)
{
  value = newVal;
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


PHTTPField * PHTTPSelectField::NewField() const
{
  return new PHTTPSelectField(name, title, values, 0, help);
}


void PHTTPSelectField::GetHTMLTag(PHTML & html)
{
  html << PHTML::Select(name);
  for (PINDEX i = 0; i < values.GetSize(); i++)
    html << PHTML::Option(values[i] == value ? PHTML::Selected : PHTML::NotSelected)
         << values[i];
  html << PHTML::Select();
}


PString PHTTPSelectField::GetHTMLValue(const PString & original)
{
  PString text = original;
  for (PINDEX i = 0; i < values.GetSize(); i++) {
    PString arg = "<!--#form select " + values[i] + "-->";
    PINDEX pos = text.Find(arg);
    if (pos == P_MAX_INDEX) {
      if (values[i] == value)
        text.Splice("SELECTED", pos, arg.GetLength());
      else
        text.Delete(pos, arg.GetLength());
    }
  }
  return text;
}


PString PHTTPSelectField::GetValue() const
{
  return value;
}


void PHTTPSelectField::SetValue(const PString & newVal)
{
  value = newVal;
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


void PHTTPForm::OnLoadedText(PHTTPRequest &, PString & text)
{
  SpliceFields(0, fields, text);
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
           << "<!--#form html " << field.GetName() << "-->"
           << PHTML::TableData()
           << field.GetHelp();
      field.SetInHTML();
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
    if (!fields[fld].ValidateAll(data, msg))
      good = FALSE;
  }

  if (!good) {
    msg << PHTML::Body();
    request.code = PHTTP::BadRequest;
    return TRUE;
  }

  for (fld = 0; fld < fields.GetSize(); fld++)
    fields[fld].SetAllValues(data);

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
  if (sectionField == NULL) {
    PString sectionName = request.url.GetQueryVars()("section", section);
    if (!sectionName) {
      PConfig cfg(sectionName);
      for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
        PHTTPField & field = fields[fld];
        for (PINDEX i = 0; i < field.GetSize(); i++) {
          if (section != sectionName)
            field.SetValueAt(i, cfg.GetString(field.GetNameAt(i)));
          else
            field.SetValueAt(i, cfg.GetString(field.GetNameAt(i), field.GetValueAt(i)));
        }
      }
      section = sectionName;
    }
  }

  PHTTPForm::OnLoadedText(request, text);
}


BOOL PHTTPConfig::Post(PHTTPRequest & request,
                       const PStringToString & data,
                       PHTML & msg)
{
  PSortedStringList oldValues;

  PINDEX fld;
  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (&field != keyField && &field != valField && &field != sectionField) {
      for (PINDEX i = 0; i < field.GetSize(); i++)
        oldValues.AppendString(field.GetNameAt(i));
    }
  }

  PHTTPForm::Post(request, data, msg);
  if (request.code != PHTTP::OK)
    return TRUE;

  if (sectionField != NULL)
    section = sectionPrefix + sectionField->GetValue() + sectionSuffix;

  PString sectionName = request.url.GetQueryVars()("section", section);
  if (sectionName.IsEmpty())
    return TRUE;

  PConfig cfg(sectionName);

  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (&field == keyField) {
      PString key = field.GetValue();
      if (!key)
        cfg.SetString(key, valField->GetValue());
    }
    else if (&field != valField && &field != sectionField) {
      for (PINDEX i = 0; i < field.GetSize(); i++)
        cfg.SetString(field.GetNameAt(i), field.GetValueAt(i));
    }
  }

  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (&field != keyField && &field != valField && &field != sectionField) {
      for (PINDEX i = 0; i < field.GetSize(); i++) {
        PINDEX idx = oldValues.GetStringsIndex(field.GetNameAt(i));
        if (idx != P_MAX_INDEX)
          oldValues.RemoveAt(idx);
      }
    }
  }

  for (fld = 0; fld < oldValues.GetSize(); fld++)
    cfg.DeleteKey(oldValues[fld]);

  section = sectionName;
  return TRUE;
}


PHTTPField * PHTTPConfig::AddSectionField(PHTTPField * sectionFld,
                                          const char * prefix,
                                          const char * suffix)
{
  sectionField = PAssertNULL(sectionFld);
  PAssert(sectionField->GetSize() == 1, "Section field is list");
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


//////////////////////////////////////////////////////////////////////////////
// PHTTPConfigSectionList

static const char FormListInclude[] = "<!--#form pagelist-->";

PHTTPConfigSectionList::PHTTPConfigSectionList(const PURL & url,
                                               const PHTTPAuthority & auth,
                                               const PString & prefix,
                                               const PString & valueName,
                                               const PURL & editSection,
                                               const PURL & newSection,
                                               const PString & newTitle,
                                               PHTML & heading)
  : PHTTPString(url, auth),
    sectionPrefix(prefix),
    additionalValueName(valueName),
    newSectionLink(newSection.AsString(PURL::URIOnly)),
    newSectionTitle(newTitle),
    editSectionLink(editSection.AsString(PURL::URIOnly) +
                      "?section=" + PURL::TranslateString(prefix, PURL::QueryTranslation))
{
  if (heading.Is(PHTML::InBody))
    heading << FormListInclude << PHTML::Body();
  SetString(heading);
}


void PHTTPConfigSectionList::OnLoadedText(PHTTPRequest &, PString & text)
{
  PConfig cfg;
  PStringList nameList = cfg.GetSections();

  PINDEX pos = text.Find(FormListInclude);
  if (pos != P_MAX_INDEX) {
    PINDEX endpos = text.Find(FormListInclude, pos + sizeof(FormListInclude)-1);
    if (endpos == P_MAX_INDEX) {
      PHTML html = PHTML::InBody;
      html << PHTML::Form() << PHTML::TableStart();

      PINDEX i;
      for (i = 0; i < nameList.GetSize(); i++) {
        if (nameList[i].Find(sectionPrefix) == 0) {
          PString name = nameList[i].Mid(sectionPrefix.GetLength());
          html << PHTML::TableRow()
               << PHTML::TableData()
               << PHTML::HotLink(editSectionLink + name)
               << name
               << PHTML::HotLink();
          if (!additionalValueName)
            html << PHTML::TableData() << cfg.GetString(nameList[i], additionalValueName, "");
          html << PHTML::TableData() << PHTML::SubmitButton("Remove", name);
        }
      }

      html << PHTML::TableRow()
           << PHTML::TableData()
           << PHTML::HotLink(newSectionLink)
           << newSectionTitle
           << PHTML::HotLink()
           << PHTML::TableEnd()
           << PHTML::Form();

      text.Splice(html, pos, sizeof(FormListInclude)-1);
    }
    else {
      PString repeat = text(pos + sizeof(FormListInclude)-1, endpos-1);
      text.Delete(pos, endpos - pos);

      PINDEX i;
      for (i = 0; i < nameList.GetSize(); i++) {
        if (nameList[i].Find(sectionPrefix) == 0) {
          PString name = nameList[i].Mid(sectionPrefix.GetLength());
          text.Splice(repeat, pos, 0);
          text.Replace("<!--#form hotlink-->",
                       editSectionLink + name,
                       FALSE, pos);
          if (!additionalValueName)
            text.Replace("<!--#form additional-->",
                         cfg.GetString(nameList[i], additionalValueName, ""),
                         FALSE, pos);
          text.Replace("<!--#form section-->", name, FALSE, pos);
          pos = text.Find(FormListInclude, pos);
        }
      }
      text.Delete(text.Find(FormListInclude, pos), sizeof(FormListInclude)-1);
    }
  }
}


BOOL PHTTPConfigSectionList::Post(PHTTPRequest &,
                                  const PStringToString & data,
                                  PHTML & replyMessage)
{
  PConfig cfg;
  PStringList nameList = cfg.GetSections();
  PINDEX i; 
  for (i = 0; i < nameList.GetSize(); i++) {
    if (nameList[i].Find(sectionPrefix) == 0) {
      PString name = nameList[i].Mid(sectionPrefix.GetLength());
      if (data.Contains(name)) {
        cfg.DeleteSection(nameList[i]);
        replyMessage << name << " removed.";
      }
    }
  }

  return TRUE;
}



// End Of File ///////////////////////////////////////////////////////////////
