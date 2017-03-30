/*
 * httpform.cxx
 *
 * Forms using HTTP user interface.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 */

#ifdef __GNUC__
#pragma implementation "httpform.h"
#endif

#include <ptlib.h>
#include <ptclib/httpform.h>

#if P_HTTPFORMS

#include <ptclib/cypher.h>


#define new PNEW


//////////////////////////////////////////////////////////////////////////////
// PHTTPField

PHTTPField::PHTTPField(const char * nam, const char * titl, const char * hlp)
  : baseName(nam), fullName(nam),
    title(titl != NULL ? titl : nam),
    help(hlp != NULL ? hlp : "")
{
  notInHTML = true;
}


PObject::Comparison PHTTPField::Compare(const PObject & obj) const
{
  PAssert(PIsDescendant(&obj, PHTTPField), PInvalidCast);
  return fullName.Compare(((const PHTTPField &)obj).fullName);
}


void PHTTPField::SetName(const PString & newName)
{
  fullName = newName;
}


const PHTTPField * PHTTPField::LocateName(const PString & name) const
{
  if (fullName == name)
    return this;

  return NULL;
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


static PBoolean FindSpliceBlock(const PRegularExpression & startExpr,
                            const PRegularExpression & endExpr,
                            const PString & text,
                            PINDEX offset,
                            PINDEX & pos,
                            PINDEX & len,
                            PINDEX & start,
                            PINDEX & finish)
{
  start = finish = P_MAX_INDEX;

  if (!text.FindRegEx(startExpr, pos, len, offset))
    return false;

  PINDEX endpos, endlen;
  if (!text.FindRegEx(endExpr, endpos, endlen, pos+len))
    return true;

  start = pos + len;
  finish = endpos - 1;
  len = endpos - pos + endlen;
  return true;
}


static PBoolean FindSpliceBlock(const PRegularExpression & startExpr,
                            const PString & text,
                            PINDEX offset,
                            PINDEX & pos,
                            PINDEX & len,
                            PINDEX & start,
                            PINDEX & finish)
{
  static PRegularExpression EndBlock("<?!--#form[ \t\r\n]+end[ \t\r\n]*-->?",
                                     PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  return FindSpliceBlock(startExpr, EndBlock, text, offset, pos, len, start, finish);
}


static PBoolean FindSpliceName(const PCaselessString & text,
                           PINDEX start,
                           PINDEX finish,
                           PINDEX & pos,
                           PINDEX & end)
{
  if (text[start+1] != '!') {
    static PRegularExpression NameExpr("name[ \t\r\n]*=[ \t\r\n]*\"[^\"]*\"",
                                       PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    if ((pos = text.FindRegEx(NameExpr, start)) == P_MAX_INDEX)
      return false;

    if (pos >= finish)
      return false;

    pos = text.Find('"', pos) + 1;
    end = text.Find('"', pos) - 1;
  }
  else {
    pos = start + 9;            // Skip over the <!--#form
    while (isspace(text[pos]))  // Skip over blanks
      pos++;
    while (pos < finish && !isspace(text[pos])) // Skip over keyword
      pos++;
    while (isspace(text[pos]))  // Skip over more blanks
      pos++;
    
    end = text.Find("--", pos) - 1;
  }

  return end < finish;
}


static PBoolean FindSpliceFieldName(const PString & text,
                            PINDEX offset,
                            PINDEX & pos,
                            PINDEX & len,
                            PString & name)
{
  static PRegularExpression FieldName("<?!--#form[ \t\r\n]+[a-z0-9]+[ \t\r\n]+(-?[^-])+-->?"
                                      "|"
                                      "<[a-z]+[ \t\r\n][^>]*name[ \t\r\n]*=[ \t\r\n]*\"[^\"]*\"[^>]*>",
                                      PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  if (!text.FindRegEx(FieldName, pos, len, offset))
    return false;

  PINDEX nameStart, nameEnd;
  if (!FindSpliceName(text, pos, pos+len-1, nameStart, nameEnd))
    return false;

  name = text(nameStart, nameEnd);
  pos = nameStart;
  len = nameEnd - nameStart + 1;
  return true;
}


static void SpliceAdjust(const PString & str,
                         PString & text,
                         PINDEX pos,
                         PINDEX & len,
                         PINDEX & finish)
{
  text.Splice(str, pos, len);
  PINDEX newLen = str.GetLength();
  if (finish != P_MAX_INDEX)
    finish += newLen - len;
  len = newLen;
}


void PHTTPField::ExpandFieldNames(PString & text, PINDEX start, PINDEX & finish) const
{
  PString name;
  PINDEX pos, len;
  while (start < finish && FindSpliceFieldName(text, start, pos, len, name)) {
    if (pos > finish)
      break;
    if (baseName == name)
      SpliceAdjust(fullName, text, pos, len, finish);
    start = pos + len;
  }
}


static PBoolean FindInputValue(const PString & text, PINDEX & before, PINDEX & after)
{
  static PRegularExpression Value("value[ \t\r\n]*=[ \t\r\n]*(\"[^\"]*\"|[^> \t\r\n]+)",
                                  PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  PINDEX pos = text.FindRegEx(Value);
  if (pos == P_MAX_INDEX)
    return false;

  before = text.Find('"', pos);
  if (before != P_MAX_INDEX)
    after = text.Find('"', before+1);
  else {
    before = text.Find('=', pos);
    while (isspace(text[before+1]))
      before++;
    after = before + 1;
    while (text[after] != '\0' && text[after] != '>' && !isspace(text[after]))
      after++;
  }
  return true;
}


PString PHTTPField::GetHTMLInput(const PString & input) const
{
  PStringStream adjusted;
  PINDEX before, after;
  if (FindInputValue(input, before, after))
    adjusted << input(0, before) << PHTML::Escaped(GetValue(false)) << input.Mid(after);
  else
    adjusted << "<INPUT VALUE=\"" << PHTML::Escaped(GetValue(false)) << '"' << input.Mid(6);
  return adjusted;
}


static void AdjustSelectOptions(PString & text, PINDEX begin, PINDEX end,
                                const PString & myValue, PStringArray & validValues,
                                PINDEX & finishAdjust)
{
  PINDEX start, finish;
  PINDEX pos = begin;
  PINDEX len = 0;
  static PRegularExpression StartOption("<[ \t\r\n]*option[^>]*>",
                                        PRegularExpression::IgnoreCase);
  static PRegularExpression EndOption("<[ \t\r\n]*/?option[^>]*>",
                                      PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  while (FindSpliceBlock(StartOption, EndOption, text, pos+len, pos, len, start, finish) && pos < end) {
    if (start == P_MAX_INDEX)
      start = text.Find('>', pos)+1;
    else {
      // Check for if option was not closed by </option> but another <option>
      PINDEX optpos = text.FindRegEx(StartOption, start);
      if (optpos < pos+len) // Adjust back to before <option> if so.
        len = optpos-pos;
    }
    PCaselessString option = text(pos, start-1);
    PINDEX before, after;
    if (FindInputValue(option, before, after)) {
      start = pos + before + 1;
      finish = pos + after - 1;
    }
    PINDEX selpos = option.Find("selected");
    PString thisValue = text(start, finish).Trim();
    if (thisValue == myValue) {
      if (selpos == P_MAX_INDEX) {
        text.Splice(" selected", pos+7, 0);
        if (finishAdjust != P_MAX_INDEX)
          finishAdjust += 9;
        if (end != P_MAX_INDEX)
          end += 9;
        len += 9;
      }
    }
    else {
      if (validValues.GetSize() > 0) {
        PINDEX valid;
        for (valid = 0; valid < validValues.GetSize(); valid++) {
          if (thisValue == validValues[valid])
            break;
        }
        if (valid >= validValues.GetSize()) {
          text.Delete(pos, len);
          selpos = P_MAX_INDEX;
          if (finishAdjust != P_MAX_INDEX)
            finishAdjust -= len;
          if (end != P_MAX_INDEX)
            end -= len;
          len = 0;
        }
      }
      if (selpos != P_MAX_INDEX) {
        selpos += pos;
        PINDEX sellen = 8;
        if (text[selpos-1] == ' ') {
          selpos--;
          sellen++;
        }
        text.Delete(selpos, sellen);
        if (finishAdjust != P_MAX_INDEX)
          finishAdjust -= sellen;
        if (end != P_MAX_INDEX)
          end -= sellen;
        len -= sellen;
      }
    }
  }
}

PString PHTTPField::GetHTMLSelect(const PString & selection) const
{
  PString text = selection;
  PStringArray dummy1;
  PINDEX dummy2 = P_MAX_INDEX;
  AdjustSelectOptions(text, 0, P_MAX_INDEX, GetValue(false), dummy1, dummy2);
  return text;
}


void PHTTPField::GetHTMLHeading(PHTML &) const
{
}


static int SplitConfigKey(const PString & fullName,
                          PString & section, PString & key)
{
  if (fullName.IsEmpty())
    return 0;

  PINDEX slash = fullName.FindLast('\\');
  if (slash == 0 || slash >= fullName.GetLength()-1) {
    key = fullName;
    return 1;
  }

  section = fullName.Left(slash);
  key = fullName.Mid(slash+1);
  if (section.IsEmpty() || key.IsEmpty())
    return 0;

  return 2;
}


bool PHTTPField::LoadFromConfig(PConfig & cfg)
{
  PString section, key;
  switch (SplitConfigKey(fullName, section, key)) {
    case 1 :
      if (!cfg.HasKey(key))
        return true;
      SetValue(cfg.GetString(key));
      break;

    case 2 :
      if (!cfg.HasKey(section, key))
        return true;
      SetValue(cfg.GetString(section, key, PString::Empty()));
  }

  return false;
}


void PHTTPField::SaveToConfig(PConfig & cfg) const
{
  PString section, key;
  switch (SplitConfigKey(fullName, section, key)) {
    case 1 :
      cfg.SetString(key, GetValue());
      break;
    case 2 :
      cfg.SetString(section, key, GetValue());
  }
}


PBoolean PHTTPField::Validated(const PString &, PStringStream &) const
{
  return true;
}


void PHTTPField::GetAllNames(PStringArray & list) const
{
  list.AppendString(fullName);
}


void PHTTPField::SetAllValues(const PStringToString & data)
{
  if (!baseName && data.Contains(fullName))
    SetValue(data[fullName]);
}


PBoolean PHTTPField::ValidateAll(const PStringToString & data, PStringStream & msg) const
{
  if (data.Contains(fullName))
    return Validated(data[fullName], msg);
  return true;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPCompositeField

PHTTPCompositeField::PHTTPCompositeField(const char * nam,
                                         const char * titl,
                                         const char * hlp,
                                         bool includeHeaders)
  : PHTTPField(nam, titl, hlp)
  , m_includeHeaders(includeHeaders)
{
}


void PHTTPCompositeField::SetName(const PString & newName)
{
  if (fullName.IsEmpty() || newName.IsEmpty())
    return;

  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    PHTTPField & field = fields[i];

    PString firstPartOfName = psprintf(fullName, i+1);
    PString subFieldName;
    if (field.GetName().Find(firstPartOfName) == 0)
      subFieldName = field.GetName().Mid(firstPartOfName.GetLength());
    else
      subFieldName = field.GetName();

    firstPartOfName = psprintf(newName, i+1);
    if (subFieldName[0] == '\\' || firstPartOfName[firstPartOfName.GetLength()-1] == '\\')
      field.SetName(firstPartOfName + subFieldName);
    else
      field.SetName(firstPartOfName & subFieldName);
  }

  PHTTPField::SetName(newName);
}


const PHTTPField * PHTTPCompositeField::LocateName(const PString & name) const
{
  if (fullName == name)
    return this;

  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    const PHTTPField * field = fields[i].LocateName(name);
    if (field != NULL)
      return field;
  }

  return NULL;
}


PHTTPField * PHTTPCompositeField::NewField() const
{
  PHTTPCompositeField * fld = new PHTTPCompositeField(baseName, title, help);
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    fld->Append(fields[i].NewField());
  return fld;
}


void PHTTPCompositeField::GetHTMLTag(PHTML & html) const
{
  if (m_includeHeaders) {
    html << PHTML::TableStart("border=1 cellspacing=0 cellpadding=8");
    GetHTMLHeading(html);
    html << PHTML::TableRow();
  }

  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    if (m_includeHeaders || (i != 0 && html.Is(PHTML::InTable)))
      html << PHTML::TableData("NOWRAP ALIGN=CENTER");
    fields[i].GetHTMLTag(html);
  }

  if (m_includeHeaders)
    html << PHTML::TableEnd();
}


PString PHTTPCompositeField::GetHTMLInput(const PString & input) const
{
  return input;
}


void PHTTPCompositeField::ExpandFieldNames(PString & text, PINDEX start, PINDEX & finish) const
{
  static PRegularExpression FieldName( "!--#form[ \t\r\n]+(-?[^-])+[ \t\r\n]+(-?[^-])+--"
                                       "|"
                                       "<[a-z]*[ \t\r\n][^>]*name[ \t\r\n]*=[ \t\r\n]*\"[^\"]*\"[^>]*>",
                                       PRegularExpression::IgnoreCase);

  PString name;
  PINDEX pos, len;
  while (start < finish && FindSpliceFieldName(text, start, pos, len, name)) {
    if (pos > finish)
      break;
    for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
      if (fields[fld].GetBaseName() *= name) {
        SpliceAdjust(fields[fld].GetName(), text, pos, len, finish);
        break;
      }
    }
    start = pos + len;
  }
}


void PHTTPCompositeField::GetHTMLHeading(PHTML & html) const
{
  html << PHTML::TableRow();
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    html << PHTML::TableHeader() << PHTML::Escaped(fields[i].GetTitle());
}


PString PHTTPCompositeField::GetValue(PBoolean dflt) const
{
  PStringStream value;
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    value << fields[i].GetValue(dflt) << '\n';
  return value;
}


void PHTTPCompositeField::SetValue(const PString &)
{
  PAssertAlways(PLogicError);
}


bool PHTTPCompositeField::LoadFromConfig(PConfig & cfg)
{
  bool useDefault = true;

  SetName(fullName);

  for (PINDEX i = 0; i < GetSize(); i++) {
    if (!fields[i].LoadFromConfig(cfg))
      useDefault = false;
  }

  return useDefault;
}


void PHTTPCompositeField::SaveToConfig(PConfig & cfg) const
{
  for (PINDEX i = 0; i < GetSize(); i++)
    fields[i].SaveToConfig(cfg);
}


void PHTTPCompositeField::GetAllNames(PStringArray & list) const
{
  for (PINDEX i = 0; i < GetSize(); i++)
    fields[i].GetAllNames(list);
}


void PHTTPCompositeField::SetAllValues(const PStringToString & data)
{
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    fields[i].SetAllValues(data);
}


PBoolean PHTTPCompositeField::ValidateAll(const PStringToString & data,
                                      PStringStream & msg) const
{
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    if (!fields[i].ValidateAll(data, msg))
      return false;

  return true;
}


PINDEX PHTTPCompositeField::GetSize() const
{
  return fields.GetSize();
}


void PHTTPCompositeField::Append(PHTTPField * fld)
{
  fields.Append(fld);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSubForm

PHTTPSubForm::PHTTPSubForm(const PString & subForm,
                           const char * name,
                           const char * title,
                           PINDEX prim,
                           PINDEX sec)
  : PHTTPCompositeField(name, title, NULL),
    subFormName(subForm)
{
  primary = prim;
  secondary = sec;
}


PHTTPField * PHTTPSubForm::NewField() const
{
  PHTTPCompositeField * fld = new PHTTPSubForm(subFormName, baseName, title, primary, secondary);
  for (PINDEX i = 0; i < fields.GetSize(); i++)
    fld->Append(fields[i].NewField());
  return fld;
}


void PHTTPSubForm::GetHTMLTag(PHTML & html) const
{
  PString value = fields[primary].GetValue();
  if (value.IsEmpty())
    value = "New";
  html << PHTML::HotLink(subFormName +
            "?subformprefix=" + PURL::TranslateString(fullName, PURL::QueryTranslation))
       << PHTML::Escaped(value) << PHTML::HotLink();

  if (secondary != P_MAX_INDEX)
    html << PHTML::TableData("NOWRAP") << PHTML::Escaped(fields[secondary].GetValue());
}


void PHTTPSubForm::GetHTMLHeading(PHTML &) const
{
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFieldArray

PHTTPFieldArray::PHTTPFieldArray(PHTTPField * fld, PBoolean ordered, PINDEX fixedSize)
  : PHTTPCompositeField(fld->GetName(), fld->GetTitle(), fld->GetHelp()),
    baseField(fld)
{
  orderedArray = ordered;
  canAddElements = fixedSize == 0;
  SetSize(fixedSize);
}


PHTTPFieldArray::~PHTTPFieldArray()
{
  delete baseField;
}


void PHTTPFieldArray::SetSize(PINDEX newSize)
{
  while (fields.GetSize() > newSize)
    fields.RemoveAt(fields.GetSize()-1);
  while (fields.GetSize() < newSize)
    AddBlankField();
  if (canAddElements)
    AddBlankField();
}


PHTTPField * PHTTPFieldArray::NewField() const
{
  return new PHTTPFieldArray(baseField->NewField(), orderedArray);
}


static const char ArrayControlBox[] = " Array Control";
static const char ArrayControlKeep[] = "Keep";
static const char ArrayControlRemove[] = "Remove";
static const char ArrayControlMoveUp[] = "Move Up";
static const char ArrayControlMoveDown[] = "Move Down";
static const char ArrayControlToTop[] = "To Top";
static const char ArrayControlToBottom[] = "To Bottom";
static const char ArrayControlIgnore[] = "Ignore";
static const char ArrayControlAddTop[] = "Add Top";
static const char ArrayControlAddBottom[] = "Add Bottom";
static const char ArrayControlAdd[] = "Add";

static PStringArray GetArrayControlOptions(PINDEX fld, PINDEX size, PBoolean orderedArray)
{
  PStringArray options;

  if (fld >= size) {
    options.AppendString(ArrayControlIgnore);
    if (size == 0 || !orderedArray)
      options.AppendString(ArrayControlAdd);
    else {
      options.AppendString(ArrayControlAddTop);
      options.AppendString(ArrayControlAddBottom);
    }
  }
  else {
    options.AppendString(ArrayControlKeep);
    options.AppendString(ArrayControlRemove);
    if (orderedArray) {
      if (fld > 0)
        options.AppendString(ArrayControlMoveUp);
      if (fld < size-1)
        options.AppendString(ArrayControlMoveDown);
      if (fld > 0)
        options.AppendString(ArrayControlToTop);
      if (fld < size-1)
        options.AppendString(ArrayControlToBottom);
    }
  }

  return options;
}

void PHTTPFieldArray::AddArrayControlBox(PHTML & html, PINDEX fld) const
{
  PStringArray options = GetArrayControlOptions(fld, fields.GetSize()-1, orderedArray);
  html << PHTML::Select(fields[fld].GetName() + ArrayControlBox);
  for (PINDEX i = 0; i < options.GetSize(); i++)
    html << PHTML::Option(i == 0 ? PHTML::Selected : PHTML::NotSelected) << PHTML::Escaped(options[i]);
  html << PHTML::Select();
}


void PHTTPFieldArray::GetHTMLTag(PHTML & html) const
{
  html << PHTML::TableStart("border=1 cellspacing=0 cellpadding=8");
  baseField->GetHTMLHeading(html);
  for (PINDEX i = 0; i < fields.GetSize(); i++) {
    html << PHTML::TableRow() << PHTML::TableData("NOWRAP");
    fields[i].GetHTMLTag(html);
    html << PHTML::TableData("NOWRAP");
    if (canAddElements)
      AddArrayControlBox(html, i);
  }
  html << PHTML::TableEnd();
}


void PHTTPFieldArray::ExpandFieldNames(PString & text, PINDEX start, PINDEX & finish) const
{
  PString original = text(start, finish);
  PINDEX origFinish = finish;
  PINDEX finalFinish = finish;

  PINDEX fld = fields.GetSize();
  while (fld > 0) {
    fields[--fld].ExpandFieldNames(text, start, finish);

    PINDEX pos,len;
    static PRegularExpression RowNum("<?!--#form[ \t\r\n]+rownum[ \t\r\n]*-->?",
                                     PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    while (text.FindRegEx(RowNum, pos, len, start, finish))
      SpliceAdjust(psprintf("%u", fld+1), text, pos, len, finish);

    static PRegularExpression SubForm("<?!--#form[ \t\r\n]+subform[ \t\r\n]*-->?",
                                      PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    while (text.FindRegEx(SubForm, pos, len, start, finish)) {
      PString fmt = fullName;
      if (fmt.Find("%u") == P_MAX_INDEX)
        fmt += " %u";
      SpliceAdjust("subformprefix=" + PURL::TranslateString(psprintf(fmt, fld+1), PURL::QueryTranslation),
                   text, pos, len, finish);
    }

    static PRegularExpression RowControl("<?!--#form[ \t\r\n]+rowcontrol[ \t\r\n]*-->?",
                                         PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    while (text.FindRegEx(RowControl, pos, len, start, finish)) {
      PHTML html(PHTML::InForm);
      if (canAddElements)
        AddArrayControlBox(html, fld);
      SpliceAdjust(html, text, pos, len, finish);
    }

    static PRegularExpression RowCheck("<?!--#form[ \t\r\n]+row(add|delete)[ \t\r\n]*(-?[^-])*-->?",
                                         PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    while (text.FindRegEx(RowCheck, pos, len, start, finish)) {
      PStringStream checkbox;
      if (canAddElements) {
        PINDEX titlepos = text.Find("row", start)+3;
        PBoolean adding = text[titlepos] == 'a';
        if (( adding && fld >= fields.GetSize()-1) ||
            (!adding && fld <  fields.GetSize()-1)) {
          titlepos += adding ? 3 : 6;
          PINDEX dashes = text.Find("--", titlepos);
          PString title = text(titlepos, dashes-1).Trim();
          if (title.IsEmpty() && adding)
            title = "Add";
          checkbox << title
                   << "<INPUT TYPE=checkbox NAME=\""
                   << fields[fld].GetName()
                   << ArrayControlBox
                   << "\" VALUE="
                   << (adding ? ArrayControlAdd : ArrayControlRemove)
                   << '>';
        }
      }
      SpliceAdjust(checkbox, text, pos, len, finish);
    }

    static PRegularExpression SelectRow("<select[ \t\r\n][^>]*name[ \t\r\n]*=[ \t\r\n]*\"!--#form[ \t\r\n]+rowselect[ \t\r\n]*--\"[^>]*>",
                                        PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    static PRegularExpression SelEndRegEx("</select[^>]*>",
                                          PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    PINDEX begin, end;
    while (FindSpliceBlock(SelectRow, SelEndRegEx, text, 0, pos, len, begin, end)) {
      PStringArray options = GetArrayControlOptions(fld, fields.GetSize()-1, orderedArray);
      AdjustSelectOptions(text, begin, end, options[0], options, finish);
      static PRegularExpression RowSelect("!--#form[ \t\r\n]+rowselect[ \t\r\n]*--",
                                          PRegularExpression::Extended|PRegularExpression::IgnoreCase);
      if (text.FindRegEx(RowSelect, pos, len, pos, begin))
        SpliceAdjust(fields[fld].GetName() + ArrayControlBox, text, pos, len, finish);
    }

    finalFinish += finish - origFinish;

    if (fld > 0) {
      text.Splice(original, start, 0);
      finish = origFinish;
      finalFinish += finish - start;
    }
  }

  finish = finalFinish;
}


static int SplitArraySizeKey(const PString & fullName,
                             PString & section, PString & key)
{
  static const char ArraySize[] = "Array Size";
  PINDEX pos = fullName.Find("%u");
  if (pos == P_MAX_INDEX)
    return SplitConfigKey(fullName & ArraySize, section, key);

  PINDEX endPos = fullName.GetLength() - 1;
  if (fullName[endPos] == '\\')
    endPos--;
  return SplitConfigKey(fullName.Left(pos) & ArraySize & fullName(pos+2, endPos), section, key);
}


bool PHTTPFieldArray::LoadFromConfig(PConfig & cfg)
{
  if (canAddElements) {
    PString section, key;
    switch (SplitArraySizeKey(fullName, section, key)) {
      case 1 :
        if (!cfg.HasKey(key))
          return true;

        SetSize(cfg.GetInteger(key));
        break;

      case 2 :
        if (!cfg.HasKey(section, key))
          return true;

        SetSize(cfg.GetInteger(section, key));
    }
  }

  PHTTPCompositeField::LoadFromConfig(cfg);
  return false;
}


void PHTTPFieldArray::SaveToConfig(PConfig & cfg) const
{
  if (canAddElements) {
    PString section, key;
    switch (SplitArraySizeKey(fullName, section, key)) {
      case 1 :
        cfg.SetInteger(key, GetSize());
        break;
      case 2 :
        cfg.SetInteger(section, key, GetSize());
    }
  }
  PHTTPCompositeField::SaveToConfig(cfg);
}


void PHTTPFieldArray::SetArrayFieldName(PINDEX idx) const
{
  PString fmt = fullName;
  if (fmt.Find("%u") == P_MAX_INDEX)
    fmt += " %u";
  fields[idx].SetName(psprintf(fmt, idx+1));
}


void PHTTPFieldArray::SetAllValues(const PStringToString & data)
{
  PHTTPFields newFields(fields.GetSize());
  newFields.DisallowDeleteObjects();
  PINDEX i;
  for (i = 0; i < fields.GetSize(); i++)
    newFields.SetAt(i, fields.GetAt(i));

  PBoolean lastFieldIsSet = false;
  PINDEX size = fields.GetSize();
  for (i = 0; i < size; i++) {
    PHTTPField * fieldPtr = &fields[i];
    PINDEX pos = newFields.GetObjectsIndex(fieldPtr);
    fieldPtr->SetAllValues(data);

    PString control = data(fieldPtr->GetName() + ArrayControlBox);
    if (control == ArrayControlMoveUp) {
      if (pos > 0) {
        newFields.SetAt(pos, newFields.GetAt(pos-1));
        newFields.SetAt(pos-1, fieldPtr);
      }
    }
    else if (control == ArrayControlMoveDown) {
      if (size > 2 && pos < size-2) {
        newFields.SetAt(pos, newFields.GetAt(pos+1));
        newFields.SetAt(pos+1, fieldPtr);
      }
    }
    else if (control == ArrayControlToTop) {
      newFields.RemoveAt(pos);
      newFields.InsertAt(0, fieldPtr);
    }
    else if (control == ArrayControlToBottom) {
      newFields.RemoveAt(pos);
      newFields.Append(fieldPtr);
    }
    else if (control == ArrayControlAddTop) {
      if (i == size-1) {
        newFields.RemoveAt(pos);
        newFields.InsertAt(0, fieldPtr);
        lastFieldIsSet = true;
      }
    }
    else if (control == ArrayControlAddBottom || control == ArrayControlAdd) {
      if (i == size-1) {
        newFields.RemoveAt(pos);
        newFields.Append(fieldPtr);
        lastFieldIsSet = true;
      }
    }
    else if (control == ArrayControlIgnore) {
      newFields.RemoveAt(pos);
      newFields.Append(fieldPtr);
    }
    else if (control == ArrayControlRemove)
      newFields.RemoveAt(pos);
  }

  fields.DisallowDeleteObjects();
  for (i = 0; i < newFields.GetSize(); i++)
    fields.Remove(newFields.GetAt(i));
  fields.AllowDeleteObjects();
  fields.RemoveAll();

  for (i = 0; i < newFields.GetSize(); i++) {
    fields.Append(newFields.GetAt(i));
    SetArrayFieldName(i);
  }

  if (lastFieldIsSet && canAddElements)
    AddBlankField();
}


PINDEX PHTTPFieldArray::GetSize() const
{
  PINDEX size = fields.GetSize();
  PAssert(size > 0, PLogicError);
  if (canAddElements)
    size--;
  return size;
}


void PHTTPFieldArray::AddBlankField()
{
  fields.Append(baseField->NewField());
  SetArrayFieldName(fields.GetSize()-1);
}


PStringArray PHTTPFieldArray::GetStrings(PConfig & cfg, const PStringArray & defaultValues)
{
  if (LoadFromConfig(cfg))
    SetStrings(cfg, defaultValues);

  PStringArray values(GetSize());

  for (PINDEX i = 0; i < GetSize(); i++)
    values[i] = fields[i].GetValue(false);

  return values;
}


void PHTTPFieldArray::SetStrings(PConfig & cfg, const PStringArray & values)
{
  SetSize(values.GetSize());
  for (PINDEX i = 0; i < values.GetSize(); i++)
    fields[i].SetValue(values[i]);

  SaveToConfig(cfg);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPStringField

PHTTPStringField::PHTTPStringField(const char * name,
                                   PINDEX maxLen,
                                   const char * initVal,
                                   const char * help,
                                   int r,
                                   int c)
  : PHTTPField(name, NULL, help)
  , value(initVal != NULL ? initVal : "")
  , initialValue(value)
  , maxLength(maxLen)
  , rows(r)
  , columns(c)
{
}


PHTTPStringField::PHTTPStringField(const char * name,
                                   const char * title,
                                   PINDEX maxLen,
                                   const char * initVal,
                                   const char * help,
                                   int r,
                                   int c)
  : PHTTPField(name, title, help)
  , value(initVal != NULL ? initVal : "")
  , initialValue(value)
  , maxLength(maxLen)
  , rows(r)
  , columns(c)
{
}


PHTTPField * PHTTPStringField::NewField() const
{
  return new PHTTPStringField(baseName, title, maxLength, initialValue, help, rows, columns);
}


void PHTTPStringField::GetHTMLTag(PHTML & html) const
{
  const int DefaultColumns = 80;
  int r, c;
  if (rows == 0) {
    if (columns != 0) {
      c = columns;
      r = (maxLength+c-1)/c;
    }
    else if (maxLength < DefaultColumns*2) {
      c = maxLength;
      r = 1;
    }
    else {
      c = DefaultColumns;
      r = (maxLength+c-1)/c;
    }
  }
  else {
    r = rows;
    if (columns != 0)
      c = columns;
    else
      c = (maxLength+r-1)/r;
  }

  if (r <= 1)
    html << PHTML::InputText(fullName, c, maxLength);
  else
    html << PHTML::TextArea(fullName, r, c) << PHTML::Escaped(value) << PHTML::TextArea(fullName);
}


void PHTTPStringField::SetValue(const PString & newVal)
{
  value = newVal;
}


PString PHTTPStringField::GetValue(PBoolean dflt) const
{
  if (dflt)
    return initialValue;
  else
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
  return new PHTTPPasswordField(baseName, title, maxLength, initialValue, help);
}


void PHTTPPasswordField::GetHTMLTag(PHTML & html) const
{
  html << PHTML::InputPassword(fullName, maxLength);
}


static const PTEACypher::Key PasswordKey = {
  {
    103,  60, 222,  17, 128, 157,  31, 137,
    133,  64,  82, 148,  94, 136,   4, 209
  }
};

void PHTTPPasswordField::SetValue(const PString & newVal)
{
  value = Decrypt(newVal);
}


PString PHTTPPasswordField::GetValue(PBoolean dflt) const
{
  if (dflt)
    return initialValue;

  PTEACypher crypt(PasswordKey);
  return crypt.Encode(value);
}


PString PHTTPPasswordField::Decrypt(const PString & pword)
{
  PString clear;
  PTEACypher crypt(PasswordKey);
  return crypt.Decode(pword, clear) ? clear : pword;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPDateField

PHTTPDateField::PHTTPDateField(const char * name, const PTime & initVal, PTime::TimeFormat fmt)
  : PHTTPStringField(name, 30, initVal.AsString(fmt))
  , m_format(fmt)
{
}


PHTTPField * PHTTPDateField::NewField() const
{
  return new PHTTPDateField(baseName, value);
}


void PHTTPDateField::SetValue(const PString & newValue)
{
  PTime test(newValue);
  if (test.IsValid())
    value = test.AsString(m_format);
  else
    value = newValue;
}


PBoolean PHTTPDateField::Validated(const PString & newValue, PStringStream & msg) const
{
  if (newValue.IsEmpty())
    return true;

  PTime test(newValue);
  if (test.IsValid())
    return true;

  msg << "Invalid time specification.";
  return false;
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
  value = initialValue = initVal;
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
  value = initialValue = initVal;
}


PHTTPField * PHTTPIntegerField::NewField() const
{
  return new PHTTPIntegerField(baseName, title, low, high, initialValue, units, help);
}


void PHTTPIntegerField::GetHTMLTag(PHTML & html) const
{
  html << PHTML::InputNumber(fullName, low, high, value) << "  " << PHTML::Escaped(units);
}


void PHTTPIntegerField::SetValue(const PString & newVal)
{
  value = newVal.AsInteger();
}


PString PHTTPIntegerField::GetValue(PBoolean dflt) const
{
  return PString(PString::Signed, dflt ? initialValue : value);
}


bool PHTTPIntegerField::LoadFromConfig(PConfig & cfg)
{
  PString section, key;
  switch (SplitConfigKey(fullName, section, key)) {
    case 1 :
      if (!cfg.HasKey(key))
        return true;
      value = cfg.GetInteger(key, initialValue);
      break;
    case 2 :
      if (!cfg.HasKey(section, key))
        return true;
      value = cfg.GetInteger(section, key, initialValue);
  }
  return false;
}


void PHTTPIntegerField::SaveToConfig(PConfig & cfg) const
{
  PString section, key;
  switch (SplitConfigKey(fullName, section, key)) {
    case 1 :
      cfg.SetInteger(key, value);
      break;
    case 2 :
      cfg.SetInteger(section, key, value);
  }
}


PBoolean PHTTPIntegerField::Validated(const PString & newVal, PStringStream & msg) const
{
  int val = newVal.AsInteger();
  if (val >= low && val <= high)
    return true;

  msg << "The field \"" << GetName() << "\" should be between "
      << low << " and " << high << ".<BR>";
  return false;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPBooleanField

PHTTPBooleanField::PHTTPBooleanField(const char * name,
                                     PBoolean initVal,
                                   const char * help)
  : PHTTPField(name, NULL, help)
{
  value = initialValue = initVal;
}


PHTTPBooleanField::PHTTPBooleanField(const char * name,
                                     const char * title,
                                     PBoolean initVal,
                                     const char * help)
  : PHTTPField(name, title, help)
{
  value = initialValue = initVal;
}


PHTTPField * PHTTPBooleanField::NewField() const
{
  return new PHTTPBooleanField(baseName, title, initialValue, help);
}


void PHTTPBooleanField::GetHTMLTag(PHTML & html) const
{
  html << PHTML::HiddenField(fullName, "false")
       << PHTML::CheckBox(fullName, value ? PHTML::Checked : PHTML::UnChecked);
}


static void SpliceChecked(PString & text, PBoolean value)
{
  PINDEX pos = text.Find("checked");
  if (value) {
    if (pos == P_MAX_INDEX)
      text.Splice(" checked", 6, 0);
  }
  else {
    if (pos != P_MAX_INDEX) {
      PINDEX len = 7;
      if (text[pos-1] == ' ') {
        pos--;
        len++;
      }
      text.Delete(pos, len);
    }
  }
}


PString PHTTPBooleanField::GetHTMLInput(const PString & input) const
{
  static PRegularExpression checkboxRegEx("type[ \t\r\n]*=[ \t\r\n]*\"?checkbox\"?",
                                          PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  if (input.FindRegEx(checkboxRegEx) != P_MAX_INDEX) {
    PCaselessString text;
    PINDEX before, after;
    if (FindInputValue(input, before, after)) 
      text = input(0, before) + "true" + input.Mid(after);
    else
      text = "<input value=\"true\"" + input.Mid(6);
    SpliceChecked(text, value);
    return "<input type=hidden name=\"" + fullName + "\">" + text;
  }

  static PRegularExpression radioRegEx("type[ \t\r\n]*=[ \t\r\n]*\"?radio\"?",
                                       PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  if (input.FindRegEx(radioRegEx) != P_MAX_INDEX) {
    PINDEX before, after;
    if (FindInputValue(input, before, after)) {
      PCaselessString text = input;
      PString val = input(before+1, after-1);
      SpliceChecked(text, (value && (val *= "true")) || (!value && (val *= "false")));
      return text;
    }
    return input;
  }

  return PHTTPField::GetHTMLInput(input);
}


void PHTTPBooleanField::SetValue(const PString & val)
{
  value = toupper(val[0]) == 'T' || toupper(val[0]) == 'y' ||
          val.AsInteger() != 0 || val.Find("true") != P_MAX_INDEX;
}


PString PHTTPBooleanField::GetValue(PBoolean dflt) const
{
  return ((dflt ? initialValue : value) ? "True" : "False");
}


bool PHTTPBooleanField::LoadFromConfig(PConfig & cfg)
{
  PString section, key;
  switch (SplitConfigKey(fullName, section, key)) {
    case 1 :
      if (!cfg.HasKey(key))
        return true;
      value = cfg.GetBoolean(key, initialValue);
      break;
    case 2 :
      if (!cfg.HasKey(section, key))
        return true;
      value = cfg.GetBoolean(section, key, initialValue);
  }
  return false;
}


void PHTTPBooleanField::SaveToConfig(PConfig & cfg) const
{
  PString section, key;
  switch (SplitConfigKey(fullName, section, key)) {
    case 1 :
      cfg.SetBoolean(key, value);
      break;
    case 2 :
      cfg.SetBoolean(section, key, value);
  }
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
    value(valueArray[initVal]),
    initialValue(value)
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
    value(valueArray[initVal]),
    initialValue(value)
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
    value(valueStrings[initVal]),
    initialValue(value)
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
    value(valueStrings[initVal]),
    initialValue(value)
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
    value(valueArray[initVal]),
    initialValue(value)
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
    value(valueArray[initVal]),
    initialValue(value)
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
    value(valueStrings[initVal]),
    initialValue(value)
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
    value(valueStrings[initVal]),
    initialValue(value)
{
}


PHTTPField * PHTTPRadioField::NewField() const
{
  return new PHTTPRadioField(*this);
}


void PHTTPRadioField::GetHTMLTag(PHTML & html) const
{
  for (PINDEX i = 0; i < values.GetSize(); i++)
    html << PHTML::RadioButton(fullName, values[i],
                        values[i] == value ? PHTML::Checked : PHTML::UnChecked)
         << PHTML::Escaped(titles[i])
         << PHTML::BreakLine();
}


PString PHTTPRadioField::GetHTMLInput(const PString & input) const
{
  PString inval;
  PINDEX before, after;
  if (FindInputValue(input, before, after))
    inval = input(before+1, after-1);
  else
    inval = baseName;

  bool checked = input.Find("CHECKED>") != P_MAX_INDEX;
  bool sameValue = inval == value;
  if (checked == sameValue)
    return input;

  PINDEX len = input.GetLength();
  if (sameValue)
    return input.Left(len-1) + "CHECKED>";

  return input.Left(len-8) + '>';
}


PString PHTTPRadioField::GetValue(PBoolean dflt) const
{
  if (dflt)
    return initialValue;
  else
    return value;
}


void PHTTPRadioField::SetValue(const PString & newVal)
{
  PINDEX idx;
  if ((idx = values.GetValuesIndex(newVal)) != P_MAX_INDEX ||
      (idx = titles.GetValuesIndex(newVal)) != P_MAX_INDEX)
    value = values[idx];
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSelectField

PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const PStringArray & valueArray,
                                   PINDEX initVal,
                                   const char * help,
                                   bool enumeration)
  : PHTTPField(name, NULL, help)
  , m_values(valueArray)
{
  Construct(initVal, enumeration);
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   PINDEX count,
                                   const char * const * valueStrings,
                                   PINDEX initVal,
                                   const char * help,
                                   bool enumeration)
  : PHTTPField(name, NULL, help)
  , m_values(count, valueStrings)
{
  Construct(initVal, enumeration);
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const char * title,
                                   const PStringArray & valueArray,
                                   PINDEX initVal,
                                   const char * help,
                                   bool enumeration)
  : PHTTPField(name, title, help)
  , m_values(valueArray)
{
  Construct(initVal, enumeration);
}


PHTTPSelectField::PHTTPSelectField(const char * name,
                                   const char * title,
                                   PINDEX count,
                                   const char * const * valueStrings,
                                   PINDEX initVal,
                                   const char * help,
                                   bool enumeration)
  : PHTTPField(name, title, help)
  , m_values(count, valueStrings)
{
  Construct(initVal, enumeration);
}


void PHTTPSelectField::Construct(PINDEX initVal, bool enumeration)
{
  m_initialValue = initVal;
  m_enumeration = enumeration;

  if (enumeration)
    m_value.sprintf("%u", initVal);
  else if (initVal < m_values.GetSize())
    m_value = m_values[initVal];
}


PHTTPField * PHTTPSelectField::NewField() const
{
  return new PHTTPSelectField(baseName, title, m_values, m_initialValue, help, m_enumeration);
}


void PHTTPSelectField::GetHTMLTag(PHTML & html) const
{
  html << PHTML::Select(fullName);
  PINDEX selection = m_value.AsUnsigned();
  for (PINDEX i = 0; i < m_values.GetSize(); i++) {
    if (m_enumeration)
      html << PHTML::Option(i == selection ? PHTML::Selected : PHTML::NotSelected, psprintf("VALUE=\"%u\"", i));
    else
      html << PHTML::Option(m_values[i] == m_value ? PHTML::Selected : PHTML::NotSelected);
    html << PHTML::Escaped(m_values[i]);
  }
  html << PHTML::Select();
}


PString PHTTPSelectField::GetValue(PBoolean dflt) const
{
  if (!dflt)
    return m_value;

  if (m_enumeration)
    return m_initialValue;

  if (m_initialValue < m_values.GetSize())
    return m_values[m_initialValue];

  return PString::Empty();
}


void PHTTPSelectField::SetValue(const PString & newVal)
{
  m_value = newVal;
}



//////////////////////////////////////////////////////////////////////////////
// PHTTPForm

PHTTPForm::PHTTPForm(const PURL & url)
  : PHTTPString(url),
    fields("")
{
}

PHTTPForm::PHTTPForm(const PURL & url, const PHTTPAuthority & auth)
  : PHTTPString(url, auth),
    fields("")
{
}

PHTTPForm::PHTTPForm(const PURL & url, const PString & html)
  : PHTTPString(url, html),
    fields("")
{
}

PHTTPForm::PHTTPForm(const PURL & url,
                     const PString & html,
                     const PHTTPAuthority & auth)
  : PHTTPString(url, html, auth),
    fields("")
{
}


static PBoolean FindSpliceAccepted(const PString & text,
                              PINDEX offset,
                              PINDEX & pos,
                              PINDEX & len,
                              PINDEX & start,
                              PINDEX & finish)
{
  static PRegularExpression Accepted("<?!--#form[ \t\r\n]+accepted[ \t\r\n]*-->?",
                                     PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  return FindSpliceBlock(Accepted, text, offset, pos, len, start, finish);
}


static PBoolean FindSpliceErrors(const PString & text,
                            PINDEX offset,
                            PINDEX & pos,
                            PINDEX & len,
                            PINDEX & start,
                            PINDEX & finish)
{
  static PRegularExpression Errors("<?!--#form[ \t\r\n]+errors[ \t\r\n]*-->?",
                                     PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  return FindSpliceBlock(Errors, text, offset, pos, len, start, finish);
}


static PBoolean FindSpliceField(const PRegularExpression & startExpr,
                            const PRegularExpression & endExpr,
                            const PString & text,
                            PINDEX offset,
                            const PHTTPField & rootField,
                            PINDEX & pos,
                            PINDEX & len,
                            PINDEX & start,
                            PINDEX & finish,
                            const PHTTPField * & field)
{
  field = NULL;

  if (!FindSpliceBlock(startExpr, endExpr, text, offset, pos, len, start, finish))
    return false;

  PINDEX endBlock = start != finish ? (start-1) : (pos+len-1);
  PINDEX namePos, nameEnd;
  if (FindSpliceName(text, pos, endBlock, namePos, nameEnd))
    field = rootField.LocateName(text(namePos, nameEnd));
  return true;
}


static const char ListFieldDeleteBox[] = "List Row Delete ";

void PHTTPForm::OnLoadedText(PHTTPRequest & request, PString & text)
{
  PINDEX pos, len, start, finish;
  const PHTTPField * field;

  // Remove the subsections for POST command
  pos = 0;
  while (FindSpliceAccepted(text, pos, pos, len, start, finish))
    text.Delete(pos, len);

  pos = 0;
  while (FindSpliceErrors(text, pos, pos, len, start, finish))
    text.Delete(pos, len);

  // See if are a subform, set root composite field accordingly
  PString prefix = request.url.GetQueryVars()("subformprefix");
  if (!prefix) {
    static PRegularExpression SubFormPrefix("<?!--#form[ \t\r\n]+subformprefix[ \t\r\n]*-->?",
                                            PRegularExpression::Extended|PRegularExpression::IgnoreCase);
    while (text.FindRegEx(SubFormPrefix, pos, len))
      text.Splice("subformprefix=" +
                  PURL::TranslateString(prefix, PURL::QueryTranslation),
                  pos, len);
    field = fields.LocateName(prefix);
    if (field != NULL) {
      finish = P_MAX_INDEX;
      field->ExpandFieldNames(text, 0, finish);
    }
  }

  // Locate <!--#form list name--> macros and expand them
  static PRegularExpression ListRegEx("<!--#form[ \t\r\n]+listfields[ \t\r\n]+(-?[^-])+-->",
                                       PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression EndBlock("<?!--#form[ \t\r\n]+end[ \t\r\n]+(-?[^-])+-->?",
                                     PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  pos = len = 0;
  while (FindSpliceBlock(ListRegEx, EndBlock, text, pos+len, pos, len, start, finish)) {
    if (start != finish) {
      PString repeat = text(start, finish);

      PINDEX namePos, nameEnd;
      PRegularExpression fieldsRegEx;
      if (FindSpliceName(text, pos, start-1, namePos, nameEnd))
        fieldsRegEx.Compile(text(namePos, nameEnd), PRegularExpression::Extended|PRegularExpression::IgnoreCase);
      else
        fieldsRegEx.Compile(".*");

      PString insert;
      for (PINDEX f = 0; f < fields.GetSize(); f++) {
        if (fields[f].GetName().FindRegEx(fieldsRegEx) != P_MAX_INDEX) {
          PString iteration = repeat;
          PINDEX npos, nlen;

          static PRegularExpression FieldNameRegEx("<?!--#form[ \t\r\n]+fieldname[ \t\r\n]*-->?",
                                                   PRegularExpression::Extended|PRegularExpression::IgnoreCase);
          while (iteration.FindRegEx(FieldNameRegEx, npos, nlen))
            iteration.Splice(fields[f].GetName(), npos, nlen);

          static PRegularExpression RowDeleteRegEx("<?!--#form[ \t\r\n]+rowdelete[ \t\r\n]*-->?",
                                                   PRegularExpression::Extended|PRegularExpression::IgnoreCase);
          while (iteration.FindRegEx(RowDeleteRegEx, npos, nlen)) {
            PHTML html(PHTML::InForm);
            html << PHTML::CheckBox(ListFieldDeleteBox + fields[f].GetName());
            iteration.Splice(html, npos, nlen);
          }

          insert += iteration;
        }
      }
      text.Splice(insert, pos, len);
    }
  }

  // Locate <!--#form array name--> macros and expand them
  static PRegularExpression ArrayRegEx("<!--#form[ \t\r\n]+array[ \t\r\n]+(-?[^-])+-->",
                                       PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  pos = len = 0;
  while (FindSpliceField(ArrayRegEx, EndBlock, text, pos+len, fields, pos, len, start, finish, field)) {
    if (start != finish && field != NULL)
      field->ExpandFieldNames(text, start, finish);
  }

  // Have now expanded all field names to be fully qualified

  static PRegularExpression HTMLRegEx("<!--#form[ \t\r\n]+html[ \t\r\n]+(-?[^-])+-->",
                                      PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  while (FindSpliceField(HTMLRegEx, PRegularExpression(), text, 0, fields, pos, len, start, finish, field)) {
    if (field != NULL) {
      PHTML html(PHTML::InForm);
      field->GetHTMLTag(html);
      text.Splice(html, pos, len);
    }
  }

  pos = len = 0;
  static PRegularExpression ValueRegEx("<!--#form[ \t\r\n]+value[ \t\r\n]+(-?[^-])+-->",
                                       PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  while (FindSpliceField(ValueRegEx, PRegularExpression(), text, pos+len, fields, pos, len, start, finish, field)) {
    if (field != NULL)
      text.Splice(PHTML::Escape(field->GetValue()), pos, len);
  }

  pos = len = 0;
  static PRegularExpression InputRegEx("<input[ \t\r\n][^>]*name[ \t\r\n]*=[ \t\r\n]*\"[^\"]*\"[^>]*>",
                                       PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  while (FindSpliceField(InputRegEx, PRegularExpression(), text, pos+len, fields, pos, len, start, finish, field)) {
    if (field != NULL) {
      static PRegularExpression HiddenRegEx("type[ \t\r\n]*=[ \t\r\n]*\"?hidden\"?",
                                            PRegularExpression::Extended|PRegularExpression::IgnoreCase);
      PString substr = text.Mid(pos, len);
      if (substr.FindRegEx(HiddenRegEx) == P_MAX_INDEX)
        text.Splice(field->GetHTMLInput(substr), pos, len);
    }
  }

  pos = len = 0;
  static PRegularExpression SelectRegEx("<select[ \t\r\n][^>]*name[ \t\r\n]*=[ \t\r\n]*\"[^\"]*\"[^>]*>",
                                        PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression SelEndRegEx("</select[^>]*>",
                                        PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  while (FindSpliceField(SelectRegEx, SelEndRegEx, text, pos+len, fields, pos, len, start, finish, field)) {
    if (field != NULL)
      text.Splice(field->GetHTMLSelect(text(start, finish)), start, finish-start+1);
  }

  pos = len = 0;
  static PRegularExpression TextRegEx("<textarea[ \t\r\n][^>]*name[ \t\r\n]*=[ \t\r\n]*\"[^\"]*\"[^>]*>",
                                      PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression TextEndRegEx("</textarea[^>]*>", PRegularExpression::IgnoreCase);
  while (FindSpliceField(TextRegEx, TextEndRegEx, text, pos+len, fields, pos, len, start, finish, field)) {
    if (field != NULL)
      text.Splice(PHTML::Escape(field->GetValue()), start, finish-start+1);
  }
}


PHTTPField * PHTTPForm::Add(PHTTPField * fld)
{
  if (PAssertNULL(fld) == NULL)
    return NULL;

  PAssert(!fieldNames[fld->GetName()], "Field " + fld->GetName() + " already on form!");
  fieldNames += fld->GetName();
  fields.Append(fld);
  return fld;
}


void PHTTPForm::BuildHTML(const char * heading)
{
  PHTML html(heading);
  BuildHTML(html);
}


void PHTTPForm::BuildHTML(const PString & heading)
{
  PHTML html(heading);
  BuildHTML(html);
}


void PHTTPForm::BuildHTML(PHTML & html, BuildOptions option)
{
  if (!html.Is(PHTML::InForm))
    html << PHTML::Form("POST");

  html << PHTML::TableStart("cellspacing=8");
  for (PINDEX fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (field.NotYetInHTML()) {
      html << PHTML::TableRow()
           << PHTML::TableData("align=right")
           << PHTML::Escaped(field.GetTitle())
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


PBoolean PHTTPForm::Post(PHTTPRequest & request,
                     const PStringToString & data,
                     PHTML & msg)
{
  //PString prefix = request.url.GetQueryVars()("subformprefix");
  const PHTTPField * field = NULL;
  //if (!prefix)
  //  field = fields.LocateName(prefix);
  //if (field == NULL)
    field = &fields;

  PStringStream errors;
  if (field->ValidateAll(data, errors)) {
    ((PHTTPField *)field)->SetAllValues(data);

    if (msg.IsEmpty()) {
      msg << PHTML::Title() << "Accepted New Configuration" << PHTML::Body()
          << PHTML::Heading(1) << "Accepted New Configuration" << PHTML::Heading(1)
          << PHTML::HotLink(request.url.AsString()) << "Reload page" << PHTML::HotLink()
          << PHTML::NonBreakSpace(4)
          << PHTML::HotLink("/") << "Home page" << PHTML::HotLink();
    }
    else {
      PString block;
      PINDEX pos = 0;
      PINDEX len, start, finish;
      while (FindSpliceAccepted(msg, pos, pos, len, start, finish))
        msg.Splice(msg(start, finish), pos, len);
      pos = 0;
      while (FindSpliceErrors(msg, pos, pos, len, start, finish))
        msg.Delete(pos, len);
    }
  }
  else {
    if (msg.IsEmpty()) {
      msg << PHTML::Title() << "Validation Error in Request" << PHTML::Body()
          << PHTML::Heading(1) << "Validation Error in Request" << PHTML::Heading(1)
          << errors
          << PHTML::Paragraph()
          << PHTML::HotLink(request.url.AsString()) << "Reload page" << PHTML::HotLink()
          << PHTML::NonBreakSpace(4)
          << PHTML::HotLink("/") << "Home page" << PHTML::HotLink();
    }
    else {
      PINDEX pos = 0;
      PINDEX len, start, finish;
      while (FindSpliceAccepted(msg, pos, pos, len, start, finish))
        msg.Delete(pos, len);

      PBoolean appendErrors = true;
      pos = 0;
      while (FindSpliceErrors(msg, pos, pos, len, start, finish)) {
        PString block = msg(start, finish);
        PINDEX vPos, vLen;
        static PRegularExpression Validation("<?!--#form[ \t\r\n]+validation[ \t\r\n]*-->?",
                                             PRegularExpression::Extended|PRegularExpression::IgnoreCase);
        if (block.FindRegEx(Validation, vPos, vLen))
          block.Splice(errors, vPos, vLen);
        else
          block += errors;
        msg.Splice(block, pos, len);
        appendErrors = false;
      }

      if (appendErrors)
        msg << errors;
    }
  }

  return true;
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


bool PHTTPConfig::LoadFromConfig()
{
  PConfig cfg(section);
  return fields.LoadFromConfig(cfg);
}


void PHTTPConfig::OnLoadedText(PHTTPRequest & request, PString & text)
{
  if (sectionField == NULL) {
    PString sectionName = request.url.GetQueryVars()("section", section);
    if (!sectionName) {
      section = sectionName;
      LoadFromConfig();
    }
  }

  PHTTPForm::OnLoadedText(request, text);
}


PBoolean PHTTPConfig::Post(PHTTPRequest & request,
                       const PStringToString & data,
                       PHTML & msg)
{
  // Make sure the internal structure is up to date before accepting new data
  if (!section)
    LoadFromConfig();


  PSortedStringList oldValues;

  // Remember fields that are here now, so can delete removed array fields
  PINDEX fld;
  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (&field != keyField && &field != valField && &field != sectionField) {
      PStringArray names;
      field.GetAllNames(names);
      oldValues = names;
    }
  }

  PHTTPForm::Post(request, data, msg);
  if (request.code != PHTTP::RequestOK)
    return true;

  if (sectionField != NULL)
    section = sectionPrefix + sectionField->GetValue() + sectionSuffix;

  PString sectionName = request.url.GetQueryVars()("section", section);
  if (sectionName.IsEmpty())
    return true;

  PConfig cfg(sectionName);

  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (&field == keyField) {
      PString key = field.GetValue();
      if (!key)
        cfg.SetString(key, valField->GetValue());
    }
    else if (&field != valField && &field != sectionField)
      field.SaveToConfig(cfg);
  }

  // Find out which fields have been removed (arrays elements deleted)
  for (fld = 0; fld < fields.GetSize(); fld++) {
    PHTTPField & field = fields[fld];
    if (&field != keyField && &field != valField && &field != sectionField) {
      PStringArray names;
      field.GetAllNames(names);
      for (PINDEX i = 0; i < names.GetSize(); i++) {
        PINDEX idx = oldValues.GetStringsIndex(names[i]);
        if (idx != P_MAX_INDEX)
          oldValues.RemoveAt(idx);
      }
    }
  }

  for (fld = 0; fld < oldValues.GetSize(); fld++) {
    PString section, key;
    switch (SplitConfigKey(oldValues[fld], section, key)) {
      case 1 :
        cfg.DeleteKey(key);
        break;
      case 2 :
        cfg.DeleteKey(section, key);
        if (cfg.GetKeys(section).IsEmpty())
          cfg.DeleteSection(section);
    }
  }

  section = sectionName;
  return true;
}


PHTTPField * PHTTPConfig::AddSectionField(PHTTPField * sectionFld,
                                          const char * prefix,
                                          const char * suffix)
{
  sectionField = PAssertNULL(sectionFld);
  PAssert(!PIsDescendant(sectionField, PHTTPCompositeField), "Section field is composite");
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


bool PHTTPConfig::AddBooleanField(const char * name, bool defaultValue, const char * help)
{
  PConfig cfg(section);
  bool currentValue = cfg.GetBoolean(name, defaultValue);
  Add(new PHTTPBooleanField(name, currentValue, help));
  return currentValue;
}


int PHTTPConfig::AddIntegerField(const char * name, int low, int high, int defaultValue, const char * units, const char * help)
{
  PConfig cfg(section);
  int currentValue = cfg.GetInteger(name, defaultValue);
  Add(new PHTTPIntegerField(name, low, high, currentValue, units, help));
  return currentValue;
}


PString PHTTPConfig::AddStringField(const char * name, PINDEX maxLength, const char * defaultValue, const char * help, int rows, int columns)
{
  PConfig cfg(section);
  PString currentValue = cfg.GetString(name, defaultValue);
  Add(new PHTTPStringField(name, maxLength, currentValue, help, rows, columns));
  return currentValue;
}


PStringArray PHTTPConfig::AddStringArrayField(const char * name,
                                              bool sorted,
                                              PINDEX maxLength,
                                              const PStringArray & defaultValues,
                                              const char * help,
                                              int rows,
                                              int columns)
{
  PConfig cfg(section);
  PHTTPFieldArray* fieldArray = new PHTTPFieldArray(new PHTTPStringField(name, maxLength, "", help, rows, columns), sorted);
  Add(fieldArray);
  return fieldArray->GetStrings(cfg, defaultValues);
}


PString PHTTPConfig::AddSelectField(const char * name,
                                    const PStringArray & valueArray,
                                    const char * defaultValue,
                                    const char * help,
                                    bool enumeration)
{
  PConfig cfg(section);
  PString currentValue = cfg.GetString(name, defaultValue);
  PINDEX selection = valueArray.GetValuesIndex(currentValue);
  if (selection == P_MAX_INDEX)
    selection = 0;
  Add(new PHTTPSelectField(name, valueArray, selection, help, enumeration));
  return currentValue;
}


PStringArray PHTTPConfig::AddSelectArrayField(const char * name,
                                              bool sorted,
                                              const PStringArray & defaultValues,
                                              const PStringArray & possibleValues,
                                              const char * help,
                                              bool enumeration)
{
  PConfig cfg(section);
  PHTTPFieldArray* fieldArray = new PHTTPFieldArray(new PHTTPSelectField(name, possibleValues, 0, help, enumeration), sorted);
  Add(fieldArray);
  return fieldArray->GetStrings(cfg, defaultValues);
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
    newSectionLink(newSection.AsString(PURL::RelativeOnly)),
    newSectionTitle(newTitle),
    editSectionLink(editSection.AsString(PURL::RelativeOnly) +
                      "?section=" + PURL::TranslateString(prefix, PURL::QueryTranslation))
{
  if (heading.Is(PHTML::InBody))
    heading << FormListInclude << PHTML::Body();
  SetString(heading);
}


void PHTTPConfigSectionList::OnLoadedText(PHTTPRequest &, PString & text)
{
  PConfig cfg;
  PStringArray nameList = cfg.GetSections();

  PINDEX pos = text.Find(FormListInclude);
  if (pos != P_MAX_INDEX) {
    PINDEX endpos = text.Find(FormListInclude, pos + sizeof(FormListInclude)-1);
    if (endpos == P_MAX_INDEX) {
      PHTML html(PHTML::InBody);
      html << PHTML::Form("POST") << PHTML::TableStart();

      PINDEX i;
      for (i = 0; i < nameList.GetSize(); i++) {
        if (nameList[i].Find(sectionPrefix) == 0) {
          PString name = nameList[i].Mid(sectionPrefix.GetLength());
          html << PHTML::TableRow()
               << PHTML::TableData()
               << PHTML::HotLink(editSectionLink + PURL::TranslateString(name, PURL::QueryTranslation))
               << PHTML::Escaped(name)
               << PHTML::HotLink();
          if (!additionalValueName)
            html << PHTML::TableData()
                 << PHTML::HotLink(editSectionLink + PURL::TranslateString(name, PURL::QueryTranslation))
                 << PHTML::Escaped(cfg.GetString(nameList[i], additionalValueName, ""))
                 << PHTML::HotLink();
          html << PHTML::TableData() << PHTML::SubmitButton("Remove", name);
        }
      }

      html << PHTML::TableRow()
           << PHTML::TableData()
           << PHTML::HotLink(newSectionLink)
           << PHTML::Escaped(newSectionTitle)
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
                       editSectionLink + PURL::TranslateString(name, PURL::QueryTranslation),
                       true, pos);
          if (!additionalValueName)
            text.Replace("<!--#form additional-->",
                         cfg.GetString(nameList[i], additionalValueName, ""),
                         true, pos);
          text.Replace("<!--#form section-->", name, true, pos);
          pos = text.Find(FormListInclude, pos);
        }
      }
      text.Delete(text.Find(FormListInclude, pos), sizeof(FormListInclude)-1);
    }
  }
}


PBoolean PHTTPConfigSectionList::Post(PHTTPRequest &,
                                  const PStringToString & data,
                                  PHTML & replyMessage)
{
  PConfig cfg;
  PStringArray nameList = cfg.GetSections();
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

  return true;
}


#endif // P_HTTPFORMS

// End Of File ///////////////////////////////////////////////////////////////
