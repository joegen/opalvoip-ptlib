/*
 * html.cxx
 *
 * HTML classes.
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
#pragma implementation "html.h"
#endif

#include <ptlib.h>
#include <ptclib/html.h>

#include <math.h>


//////////////////////////////////////////////////////////////////////////////
// PHTML

PHTML::PHTML(ElementInSet initialState)
  : m_initialElement(initialState)
  , m_tableNestLevel(0)
  , m_divisionNestLevel(0)
{
  memset(m_elementSet, 0, sizeof(m_elementSet));

  switch (initialState) {
    case NumElementsInSet :
      break;
    case InBody :
      Set(InBody);
      break;
    case InForm :
      Set(InBody);
      Set(InForm);
      break;
    default :
      PAssertAlways(PInvalidParameter);
  }
}


PHTML::PHTML(const char * cstr)
  : m_initialElement(NumElementsInSet)
  , m_tableNestLevel(0)
  , m_divisionNestLevel(0)
{
  memset(m_elementSet, 0, sizeof(m_elementSet));

  ostream & this_stream = *this;
  this_stream << Title(cstr) << Body() << Heading(1) << cstr << Heading(1);
}


PHTML::PHTML(const PString & str)
  : m_initialElement(NumElementsInSet)
  , m_tableNestLevel(0)
  , m_divisionNestLevel(0)
{
  memset(m_elementSet, 0, sizeof(m_elementSet));

  ostream & this_stream = *this;
  this_stream << Title(str) << Body() << Heading(1) << str << Heading(1);
}


PHTML::~PHTML()
{
#ifndef NDEBUG
  if (m_initialElement != NumElementsInSet) {
    Clr(m_initialElement);
    Clr(InBody);
  }
  for (PINDEX i = 0; i < PARRAYSIZE(m_elementSet); i++)
    PAssert(m_elementSet[i] == 0, psprintf("Failed to close element %u", i));
#endif
}


void PHTML::AssignContents(const PContainer & cont)
{
  PStringStream::AssignContents(cont);
  memset(m_elementSet, 0, sizeof(m_elementSet));
}


PBoolean PHTML::Is(ElementInSet elmt) const
{
  return (m_elementSet[elmt>>3]&(1<<(elmt&7))) != 0;
}


void PHTML::Set(ElementInSet elmt)
{
  m_elementSet[elmt>>3] |= (1<<(elmt&7));
}


void PHTML::Clr(ElementInSet elmt)
{
  m_elementSet[elmt>>3] &= ~(1<<(elmt&7));
}


void PHTML::Toggle(ElementInSet elmt)
{
  m_elementSet[elmt>>3] ^= (1<<(elmt&7));
}


void PHTML::Escaped::Output(ostream & strm) const
{
  for (const char * ptr = m_str; *ptr != '\0'; ++ptr) {
    switch (*ptr) {
      case '"' :
        strm << "&quot;";
        break;
      case '<' :
        strm << "&lt;";
        break;
      case '>' :
        strm << "&gt;";
        break;
      case '&' :
        strm << "&amp;";
        break;
      default :
        strm << *ptr;
    }
  }
}


PString PHTML::Escape(const char * str)
{
  PStringStream strm;
  strm << Escaped(str);
  return strm;
}


const PString & PHTML::GetNonBreakSpace() { static const PConstString s("&nbsp;"); return s; }

void PHTML::NonBreakSpace::Output(ostream & strm) const
{
  for (unsigned i = 0; i < m_count; ++i)
    strm << GetNonBreakSpace();
}


PHTML::Element::Element(const char * nam,
                        ElementInSet elmt,
                        ElementInSet req,
                        OptionalCRLF opt)
  : m_name(nam)
  , m_inElement(elmt)
  , m_reqElement(req)
  , m_crlf(opt)
{
}


PHTML::Element::Element(const char * nam,
                        const char * att,
                        ElementInSet elmt,
                        ElementInSet req,
                        OptionalCRLF opt)
  : m_name(nam)
  , m_attr(att)
  , m_inElement(elmt)
  , m_reqElement(req)
  , m_crlf(opt)
{
}


void PHTML::Element::Output(PHTML & html) const
{
  PAssert(m_reqElement == NumElementsInSet || html.Is(m_reqElement),
                                                "HTML element out of context");

  if (m_crlf == BothCRLF || (m_crlf == OpenCRLF && !html.Is(m_inElement)))
    html << "\r\n";

  html << '<';
  if (html.Is(m_inElement))
    html << '/';
  html << m_name;

  AddAttr(html);

  if (m_attr != NULL)
    html << ' ' << m_attr;

  html << '>';
  if (m_crlf == BothCRLF || (m_crlf == CloseCRLF && html.Is(m_inElement)))
    html << "\r\n";

  if (m_inElement != NumElementsInSet)
    html.Toggle(m_inElement);
}


void PHTML::Element::AddAttr(PHTML &) const
{
}


PHTML::HTML::HTML(const char * attr)
  : Element("HTML", attr, InHTML, NumElementsInSet, BothCRLF)
{
}

PHTML::Head::Head()
  : Element("HEAD", NULL, InHead, NumElementsInSet, BothCRLF)
{
}

void PHTML::Head::Output(PHTML & html) const
{
  PAssert(!html.Is(InBody), "HTML element out of context");
  if (!html.Is(InHTML))
    html << HTML();
  Element::Output(html);
}


PHTML::Body::Body(const char * attr)
  : Element("BODY", attr, InBody, NumElementsInSet, BothCRLF)
{
}


void PHTML::Body::Output(PHTML & html) const
{
  if (!html.Is(InHTML))
    html << HTML();
  if (html.Is(InTitle))
    html << Title();
  if (html.Is(InHead))
    html << Head();
  Element::Output(html);
  if (!html.Is(InBody))
    html << HTML();
}


PHTML::Title::Title()
  : Element("TITLE", NULL, InTitle, InHead, CloseCRLF)
{
}

PHTML::Title::Title(const char * titleCStr)
  : Element("TITLE", NULL, InTitle, InHead, CloseCRLF)
  , m_titleString(titleCStr)
{
}

PHTML::Title::Title(const PString & titleStr)
  : Element("TITLE", NULL, InTitle, InHead, CloseCRLF)
  , m_titleString(titleStr)
{
}

void PHTML::Title::Output(PHTML & html) const
{
  PAssert(!html.Is(InBody), "HTML element out of context");
  if (!html.Is(InHead))
    html << Head();
  if (html.Is(InTitle)) {
    html << m_titleString;
    Element::Output(html);
  }
  else {
    Element::Output(html);
    if (!m_titleString.IsEmpty()) {
      html << m_titleString;
      Element::Output(html);
    }
  }
}


PHTML::Style::Style()
  : Element("STYLE", NULL, InTitle, InHTML, BothCRLF)
{
}

PHTML::Style::Style(const char * cssCStr)
  : Element("STYLE", NULL, InTitle, InHTML, BothCRLF)
  , m_cssString(cssCStr)
{
}

PHTML::Style::Style(const PString & cssStr)
  : Element("STYLE", NULL, InTitle, InHTML, BothCRLF)
  , m_cssString(cssStr)
{
}

void PHTML::Style::Output(PHTML & html) const
{
  if (!html.Is(InHTML))
    html << HTML();
  if (html.Is(InStyle)) {
    html << m_cssString;
    Element::Output(html);
  }
  else {
    Element::Output(html);
    if (!m_cssString.IsEmpty()) {
      html << m_cssString;
      Element::Output(html);
    }
  }
}


PHTML::StyleLink::StyleLink(const char * linkCStr)
  : Element("LINK", NULL, NumElementsInSet, InHTML, CloseCRLF)
  , m_styleLink(linkCStr)
{
}

PHTML::StyleLink::StyleLink(const PString & linkStr)
  : Element("LINK", NULL, NumElementsInSet, InHTML, CloseCRLF)
  , m_styleLink(linkStr)
{
}

void PHTML::StyleLink::AddAttr(PHTML & html) const
{
  html << " rel=\"stylesheet\" type=\"text/css\" href=\"" << m_styleLink << '"';
}

void PHTML::StyleLink::Output(PHTML & html) const
{
  if (!html.Is(InHTML))
    html << HTML();
  Element::Output(html);
}


PHTML::Banner::Banner(const char * attr)
  : Element("BANNER", attr, NumElementsInSet, InBody, BothCRLF)
{
}


PHTML::DivisionStart::DivisionStart(const char * attr)
  : Element("DIV", attr, InDivision, InBody, BothCRLF)
{
}


void PHTML::DivisionStart::Output(PHTML & html) const
{
  if (html.m_divisionNestLevel > 0)
    html.Clr(InDivision);
  Element::Output(html);
}

void PHTML::DivisionStart::AddAttr(PHTML & html) const
{
  html.m_divisionNestLevel++;
}


PHTML::DivisionEnd::DivisionEnd()
  : Element("DIV", "", InDivision, InBody, BothCRLF)
{
}


void PHTML::DivisionEnd::Output(PHTML & html) const
{
  PAssert(html.m_divisionNestLevel > 0, "Table nesting error");
  Element::Output(html);
  html.m_divisionNestLevel--;
  if (html.m_divisionNestLevel > 0)
    html.Set(InDivision);
}


PHTML::Heading::Heading(int number,
                        int sequence,
                        int skip,
                        const char * attr)
  : Element("H", attr, InHeading, InBody, CloseCRLF)
  , m_level(number)
  , m_seqNum(sequence)
  , m_skipSeq(skip)
{
}

PHTML::Heading::Heading(int number,
                        const char * image,
                        int sequence,
                        int skip,
                        const char * attr)
  : Element("H", attr, InHeading, InBody, CloseCRLF)
  , m_level(number)
  , m_srcString(image)
  , m_seqNum(sequence)
  , m_skipSeq(skip)
{
}

PHTML::Heading::Heading(int number,
                        const PString & imageStr,
                        int sequence,
                        int skip,
                        const char * attr)
  : Element("H", attr, InHeading, InBody, CloseCRLF)
  , m_level(number)
  , m_srcString(imageStr)
  , m_seqNum(sequence)
  , m_skipSeq(skip)
{
}

void PHTML::Heading::AddAttr(PHTML & html) const
{
  PAssert(m_level >= 1 && m_level <= 6, "Bad heading number");
  html << m_level;
  if (!m_srcString.IsEmpty())
    html << " SRC=\"" << Escaped(m_srcString) << '"';
  if (m_seqNum > 0)
    html << " SEQNUM=" << m_seqNum;
  if (m_skipSeq > 0)
    html << " SKIP=" << m_skipSeq;
}


PHTML::BreakLine::BreakLine(const char * attr)
  : Element("BR", attr, NumElementsInSet, InBody, CloseCRLF)
{
}


PHTML::Paragraph::Paragraph(const char * attr)
  : Element("P", attr, NumElementsInSet, InBody, OpenCRLF)
{
}


PHTML::PreFormat::PreFormat(int widthInChars, const char * attr)
  : Element("PRE", attr, InPreFormat, InBody, CloseCRLF)
  , m_width(widthInChars)
{
}


void PHTML::PreFormat::AddAttr(PHTML & html) const
{
  if (m_width > 0)
    html << " WIDTH=" << m_width;
}


PHTML::HotLink::HotLink(const char * href, const char * attr)
  : Element("A", attr, InAnchor, InBody, NoCRLF)
  , m_hrefString(href)
{
}

void PHTML::HotLink::AddAttr(PHTML & html) const
{
  if (!m_hrefString.IsEmpty())
    html << " HREF=\"" << Escaped(m_hrefString) << '"';
  else
    PAssert(html.Is(InAnchor), PInvalidParameter);
}


PHTML::Target::Target(const char * name, const char * attr)
  : Element("A", attr, NumElementsInSet, InBody, NoCRLF)
  , m_nameString(name)
{
}

void PHTML::Target::AddAttr(PHTML & html) const
{
  if (!m_nameString.IsEmpty())
    html << " NAME=\"" << Escaped(m_nameString) << '"';
}


PHTML::ImageElement::ImageElement(const char * n,
                                  const char * attr,
                                  ElementInSet elmt,
                                  ElementInSet req,
                                  OptionalCRLF c,
                                  const char * image)
  : Element(n, attr, elmt, req, c)
  , m_srcString(image)
{
}


void PHTML::ImageElement::AddAttr(PHTML & html) const
{
  if (!m_srcString.IsEmpty())
    html << " SRC=\"" << Escaped(m_srcString) << '"';
}


PHTML::Image::Image(const char * src, int w, int h, const char * attr)
  : ImageElement("IMG", attr, NumElementsInSet, InBody, NoCRLF, src)
  , m_width(w)
  , m_height(h)
{
}

PHTML::Image::Image(const char * src,
                    const char * alt,
                    int w, int h,
                    const char * attr)
  : ImageElement("IMG", attr, NumElementsInSet, InBody, NoCRLF, src)
  , m_altString(alt)
  , m_width(w)
  , m_height(h)
{
}

void PHTML::Image::AddAttr(PHTML & html) const
{
  PAssert(!m_srcString.IsEmpty(), PInvalidParameter);
  if (!m_altString.IsEmpty())
    html << " ALT=\"" << Escaped(m_altString) << '"';
  if (m_width != 0)
    html << " WIDTH=" << m_width;
  if (m_height != 0)
    html << " HEIGHT=" << m_height;
  ImageElement::AddAttr(html);
}


PHTML::HRule::HRule(const char * image, const char * attr)
  : ImageElement("HR", attr, NumElementsInSet, InBody, BothCRLF, image)
{
}


PHTML::Note::Note(const char * image, const char * attr)
  : ImageElement("NOTE", attr, InNote, InBody, BothCRLF, image)
{
}


PHTML::Address::Address(const char * attr)
  : Element("ADDRESS", attr, InAddress, InBody, BothCRLF)
{
}


PHTML::BlockQuote::BlockQuote(const char * attr)
  : Element("BQ", attr, InBlockQuote, InBody, BothCRLF)
{
}


PHTML::Credit::Credit(const char * attr)
  : Element("CREDIT", attr, NumElementsInSet, InBlockQuote, OpenCRLF)
{
}

PHTML::SetTab::SetTab(const char * id, const char * attr)
  : Element("TAB", attr, NumElementsInSet, InBody, NoCRLF)
  , m_ident(id)
{
}

void PHTML::SetTab::AddAttr(PHTML & html) const
{
  PAssert(!m_ident.IsEmpty(), PInvalidParameter);
  html << " ID=" << m_ident;
}


PHTML::Tab::Tab(int indent, const char * attr)
  : Element("TAB", attr, NumElementsInSet, InBody, NoCRLF)
  , m_indentSize(indent)
{
}

PHTML::Tab::Tab(const char * id, const char * attr)
  : Element("TAB", attr, NumElementsInSet, InBody, NoCRLF)
  , m_ident(id)
  , m_indentSize(0)
{
}

void PHTML::Tab::AddAttr(PHTML & html) const
{
  PAssert(m_indentSize != 0 || !m_ident.IsEmpty(), PInvalidParameter);
  if (m_indentSize > 0)
    html << " INDENT=" << m_indentSize;
  else
    html << " TO=" << m_ident;
}


PHTML::SimpleList::SimpleList(const char * attr)
  : Element("UL", attr, InList, InBody, BothCRLF)
{
}

void PHTML::SimpleList::AddAttr(PHTML & html) const
{
  html << " PLAIN";
}


PHTML::BulletList::BulletList(const char * attr)
  : Element("UL", attr, InList, InBody, BothCRLF)
{
}


PHTML::OrderedList::OrderedList(int seqNum, const char * attr)
  : Element("OL", attr, InList, InBody, BothCRLF)
  , m_sequenceNum(seqNum)
{
}

void PHTML::OrderedList::AddAttr(PHTML & html) const
{
  if (m_sequenceNum > 0)
    html << " SEQNUM=" << m_sequenceNum;
  if (m_sequenceNum < 0)
    html << " CONTINUE";
}


PHTML::DefinitionList::DefinitionList(const char * attr)
  : Element("DL", attr, InList, InBody, BothCRLF)
{
}


PHTML::ListHeading::ListHeading(const char * attr)
  : Element("LH", attr, InListHeading, InList, CloseCRLF)
{
}

PHTML::ListItem::ListItem(int skip, const char * attr)
  : Element("LI", attr, NumElementsInSet, InList, OpenCRLF)
  , m_skipSeq(skip)
{
}

void PHTML::ListItem::AddAttr(PHTML & html) const
{
  if (m_skipSeq > 0)
    html << " SKIP=" << m_skipSeq;
}


PHTML::DefinitionTerm::DefinitionTerm(const char * attr)
  : Element("DT", attr, NumElementsInSet, InList, NoCRLF)
{
}

void PHTML::DefinitionTerm::Output(PHTML & html) const
{
  PAssert(!html.Is(InDefinitionTerm), "HTML definition item missing");
  Element::Output(html);
  html.Set(InDefinitionTerm);
}


PHTML::DefinitionItem::DefinitionItem(const char * attr)
  : Element("DD", attr, NumElementsInSet, InList, NoCRLF)
{
}

void PHTML::DefinitionItem::Output(PHTML & html) const
{
  PAssert(html.Is(InDefinitionTerm), "HTML definition term missing");
  Element::Output(html);
  html.Clr(InDefinitionTerm);
}


static PConstString const AttStr[] =
{
  "nowrap",
  "border=1",
  "border=2",
  "cellpadding=0",
  "cellpadding=1",
  "cellpadding=2",
  "cellpadding=4",
  "cellpadding=8",
  "cellspacing=0",
  "cellspacing=1",
  "cellspacing=2",
  "cellspacing=4",
  "cellspacing=8",
  "align=left",
  "align=center",
  "align=right",
  "align=justify",
  "valign=baseline",
  "valign=bottom",
  "valign=middle",
  "valign=top"
};

#define P_DEF_HTML_TABLE_CTOR(cls, name, elmt, req, opt) \
  PHTML::cls::cls(const char * attr) \
    : TableElement(name, attr, elmt, req, opt) { } \
  PHTML::cls::cls(TableAttr attr1, const char * attr) \
    : TableElement(name, AttStr[attr1]&attr, elmt, req, opt) { } \
  PHTML::cls::cls(TableAttr attr1, TableAttr attr2, const char * attr) \
    : TableElement(name, AttStr[attr1]&AttStr[attr2]&attr, elmt, req, opt) { } \
  PHTML::cls::cls(TableAttr attr1, TableAttr attr2, TableAttr attr3, const char * attr) \
    : TableElement(name, AttStr[attr1]&AttStr[attr2]&AttStr[attr3]&attr, elmt, req, opt) { } \
  PHTML::cls::cls(TableAttr attr1, TableAttr attr2, TableAttr attr3, TableAttr attr4, const char * attr) \
    : TableElement(name, AttStr[attr1]&AttStr[attr2]&AttStr[attr3]&AttStr[attr4]&attr, elmt, req, opt) { } \
  PHTML::cls::cls(TableAttr attr1, TableAttr attr2, TableAttr attr3, TableAttr attr4, TableAttr attr5, const char * attr) \
    : TableElement(name, AttStr[attr1]&AttStr[attr2]&AttStr[attr3]&AttStr[attr4]&AttStr[attr5]&attr, elmt, req, opt) { } \


P_DEF_HTML_TABLE_CTOR(TableStart, "TABLE", InTable, InBody, BothCRLF)
P_DEF_HTML_TABLE_CTOR(TableRow, "TR", NumElementsInSet, InTable, OpenCRLF)
P_DEF_HTML_TABLE_CTOR(TableHeader, "TH", NumElementsInSet, InTable, CloseCRLF)
P_DEF_HTML_TABLE_CTOR(TableData, "TD", NumElementsInSet, InTable, NoCRLF)


void PHTML::TableElement::Output(PHTML & html) const
{
  // All these are reset by a table data/row boundary
  for (int i = InNote; i < InList; ++i)
    html.Clr((ElementInSet)i);
  Element::Output(html);
}

void PHTML::TableStart::Output(PHTML & html) const
{
  if (html.m_tableNestLevel > 0)
    html.Clr(InTable);
  TableElement::Output(html);
}

void PHTML::TableStart::AddAttr(PHTML & html) const
{
  html.m_tableNestLevel++;
}


PHTML::TableEnd::TableEnd()
  : TableElement("TABLE", "", InTable, InBody, BothCRLF)
{
}

void PHTML::TableEnd::Output(PHTML & html) const
{
  PAssert(html.m_tableNestLevel > 0, "Table nesting error");
  TableElement::Output(html);
  html.m_tableNestLevel--;
  if (html.m_tableNestLevel > 0)
    html.Set(InTable);
}


PHTML::Form::Form(const char * method,
                  const char * action,
                  const char * mimeType,
                  const char * script,
                  const char * attr)
  : Element("FORM", attr, InForm, InBody, BothCRLF)
  , m_methodString(method)
  , m_actionString(action)
  , m_mimeTypeString(mimeType)
  , m_scriptString(script)
{
}

void PHTML::Form::AddAttr(PHTML & html) const
{
  if (m_methodString != NULL)
    html << " METHOD=" << m_methodString;
  if (m_actionString != NULL)
    html << " ACTION=\"" << m_actionString << '"';
  if (m_mimeTypeString != NULL)
    html << " ENCTYPE=\"" << m_mimeTypeString << '"';
  if (m_scriptString != NULL)
    html << " SCRIPT=\"" << Escaped(m_scriptString) << '"';
}


PHTML::FieldElement::FieldElement(const char * n,
                                  const char * attr,
                                  ElementInSet elmt,
                                  OptionalCRLF c,
                                  DisableCodes disabled)
  : Element(n, attr, elmt, InForm, c)
  , m_disabledFlag(disabled == Disabled)
{
}

void PHTML::FieldElement::AddAttr(PHTML & html) const
{
  if (m_disabledFlag)
    html << " DISABLED";
}


PHTML::Select::Select(const char * fname, const char * attr)
  : FieldElement("SELECT", attr, InSelect, BothCRLF, Enabled)
  , m_nameString(fname)
{
}

PHTML::Select::Select(const char * fname,
                      DisableCodes disabled,
                      const char * attr)
  : FieldElement("SELECT", attr, InSelect, BothCRLF, disabled)
  , m_nameString(fname)
{
}

void PHTML::Select::AddAttr(PHTML & html) const
{
  if (!html.Is(InSelect)) {
    PAssert(!m_nameString.IsEmpty(), PInvalidParameter);
    html << " NAME=\"" << Escaped(m_nameString) << '"';
  }
  FieldElement::AddAttr(html);
}


PHTML::Option::Option(const char * attr)
  : FieldElement("OPTION", attr, NumElementsInSet, NoCRLF, Enabled)
  , m_selectedFlag(false)
{
}

PHTML::Option::Option(SelectionCodes select,
                      const char * attr)
  : FieldElement("OPTION", attr, NumElementsInSet, NoCRLF, Enabled)
  , m_selectedFlag(select == Selected)
{
}

PHTML::Option::Option(DisableCodes disabled,
                      const char * attr)
  : FieldElement("OPTION", attr, NumElementsInSet, NoCRLF, disabled)
  , m_selectedFlag(false)
{
}

PHTML::Option::Option(SelectionCodes select,
                      DisableCodes disabled,
                      const char * attr)
  : FieldElement("OPTION", attr, NumElementsInSet, NoCRLF, disabled)
  , m_selectedFlag(select == Selected)
{
}

void PHTML::Option::AddAttr(PHTML & html) const
{
  if (m_selectedFlag)
    html << " SELECTED";
  FieldElement::AddAttr(html);
}


PHTML::FormField::FormField(const char * n,
                            const char * attr,
                            ElementInSet elmt,
                            OptionalCRLF c,
                            DisableCodes disabled,
                            const char * fname)
  : FieldElement(n, attr, elmt, c, disabled)
  , m_nameString(fname)
{
}

void PHTML::FormField::AddAttr(PHTML & html) const
{
  PAssert(!m_nameString.IsEmpty(), PInvalidParameter);
  html << " NAME=\"" << Escaped(m_nameString) << '"';
  FieldElement::AddAttr(html);
}


PHTML::TextArea::TextArea(const char * fname,
                          DisableCodes disabled,
                          const char * attr)
  : FormField("TEXTAREA", attr, InSelect, BothCRLF, disabled, fname)
  , m_numRows(0)
  , m_numCols(0)
{
}

PHTML::TextArea::TextArea(const char * fname,
                          int rows, int cols,
                          DisableCodes disabled,
                          const char * attr)
  : FormField("TEXTAREA", attr, InSelect, BothCRLF, disabled, fname)
  , m_numRows(rows)
  , m_numCols(cols)
{
}

void PHTML::TextArea::AddAttr(PHTML & html) const
{
  if (m_numRows > 0)
    html << " ROWS=" << m_numRows;
  if (m_numCols > 0)
    html << " COLS=" << m_numCols;
  FormField::AddAttr(html);
}


PHTML::InputField::InputField(const char * type,
                              const char * fname,
                              DisableCodes disabled,
                              const char * attr)
  : FormField("INPUT", attr, NumElementsInSet, NoCRLF, disabled, fname)
  , m_typeString(type)
{
}

void PHTML::InputField::AddAttr(PHTML & html) const
{
  PAssert(!m_typeString.IsEmpty(), PInvalidParameter);
  html << " TYPE=" << m_typeString;
  FormField::AddAttr(html);
}


PHTML::HiddenField::HiddenField(const char * fname,
                                const char * value,
                                const char * attr)
  : InputField("hidden", fname, Enabled, attr)
  , m_valueString(value)
{
}

void PHTML::HiddenField::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  html << " VALUE=\"" << Escaped(m_valueString) << '"';
}


PHTML::InputText::InputText(const char * fname,
                            int size,
                            const char * init,
                            const char * attr)
  : InputField("text", fname, Enabled, attr)
  , m_width(size)
  , m_length(0)
  , m_value(init)
{
}

PHTML::InputText::InputText(const char * fname,
                            int size,
                            DisableCodes disabled,
                            const char * attr)
  : InputField("text", fname, disabled, attr)
  , m_width(size)
  , m_length(0)
{
}

PHTML::InputText::InputText(const char * fname,
                            int size,
                            int maxLength,
                            DisableCodes disabled,
                            const char * attr)
  : InputField("text", fname, disabled, attr)
  , m_width(size)
  , m_length(maxLength)
{
}

PHTML::InputText::InputText(const char * fname,
                            int size,
                            const char * init,
                            int maxLength,
                            DisableCodes disabled,
                            const char * attr)
  : InputField("text", fname, disabled, attr)
  , m_width(size)
  , m_length(maxLength)
  , m_value(init)
{
}

PHTML::InputText::InputText(const char * type,
                            const char * fname,
                            int size,
                            const char * init,
                            int maxLength,
                            DisableCodes disabled,
                            const char * attr)
  : InputField(type, fname, disabled, attr)
  , m_width(size)
  , m_length(maxLength)
  , m_value(init)
{
}

void PHTML::InputText::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  if (m_width > 0)
    html << " SIZE=" << m_width;
  if (m_length > 0)
    html << " MAXLENGTH=" << m_length;
  if (m_value != NULL)
    html << " VALUE=\"" << Escaped(m_value) << '"';
}


PHTML::InputPassword::InputPassword(const char * fname,
                                    int size,
                                    const char * init,
                                    const char * attr)
  : InputText("password", fname, size, init, 0, Enabled, attr)
{
}

PHTML::InputPassword::InputPassword(const char * fname,
                                    int size,
                                    DisableCodes disabled,
                                    const char * attr)
  : InputText("password", fname, size, NULL, 0, disabled, attr)
{
}

PHTML::InputPassword::InputPassword(const char * fname,
                                    int size,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * attr)
  : InputText("password", fname, size, NULL, maxLength, disabled, attr)
{
}

PHTML::InputPassword::InputPassword(const char * fname,
                                    int size,
                                    const char * init,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * attr)
  : InputText("password", fname, size, init, maxLength, disabled, attr)
{
}


PHTML::RadioButton::RadioButton(const char * fname,
                                const char * value,
                                const char * attr)
  : InputField("radio", fname, Enabled, attr)
  , m_valueString(value)
  , m_checkedFlag(false)
{
}

PHTML::RadioButton::RadioButton(const char * fname,
                                const char * value,
                                DisableCodes disabled,
                                const char * attr)
  : InputField("radio", fname, disabled, attr)
  , m_valueString(value)
  , m_checkedFlag(false)
{
}

PHTML::RadioButton::RadioButton(const char * fname,
                                const char * value,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * attr)
  : InputField("radio", fname, disabled, attr)
  , m_valueString(value)
  , m_checkedFlag(check == Checked)
{
}

PHTML::RadioButton::RadioButton(const char * type,
                                const char * fname,
                                const char * value,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * attr)
  : InputField(type, fname, disabled, attr)
  , m_valueString(value)
  , m_checkedFlag(check == Checked)
{
}

void PHTML::RadioButton::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  PAssert(!m_valueString.IsEmpty(), PInvalidParameter);
  html << " VALUE=\"" << Escaped(m_valueString) << '"';
  if (m_checkedFlag)
    html << " CHECKED";
}


PHTML::CheckBox::CheckBox(const char * fname, const char * attr)
  : RadioButton("checkbox", fname, "true", UnChecked, Enabled, attr)
{
}

PHTML::CheckBox::CheckBox(const char * fname,
                          DisableCodes disabled,
                          const char * attr)
  : RadioButton("checkbox", fname, "true", UnChecked, disabled, attr)
{
}

PHTML::CheckBox::CheckBox(const char * fname,
                          CheckedCodes check,
                          DisableCodes disabled,
                          const char * attr)
  : RadioButton("checkbox", fname, "true", check, disabled, attr)
{
}


PHTML::InputNumber::InputNumber(const char * fname,
                                int min, int max, int value,
                                DisableCodes disabled,
                                const char * attr)
  : InputField("number", fname, disabled, attr)
{
  Construct(min, max, value);
}

PHTML::InputNumber::InputNumber(const char * type,
                                const char * fname,
                                int min, int max,
                                int value,
                                DisableCodes disabled,
                                const char * attr)
  : InputField(type, fname, disabled, attr)
{
  Construct(min, max, value);
}

void PHTML::InputNumber::Construct(int min, int max, int value)
{
  PAssert(min <= max, PInvalidParameter);
  m_minValue = min;
  m_maxValue = max;
  if (value < min)
    m_initValue = min;
  else if (value > max)
    m_initValue = max;
  else
    m_initValue = value;
}

void PHTML::InputNumber::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  int range = std::max(-m_minValue, m_maxValue);
  int width = 3;
  while (range > 10) {
    width++;
    range /= 10;
  }
  html << " SIZE=" << width
       << " MIN=" << m_minValue
       << " MAX=" << m_maxValue
       << " VALUE=\"" << m_initValue << '"';
}


PHTML::InputReal::InputReal(const char * fname,
                            double minimum, double maximum, double value,
                            int decs,
                            DisableCodes disabled,
                            const char * attr)
  : InputField("number", fname, disabled, attr)
{
  PAssert(minimum <= maximum, PInvalidParameter);
  m_minValue = minimum;
  m_maxValue = maximum;
  if (value < minimum)
    m_initValue = minimum;
  else if (value > maximum)
    m_initValue = maximum;
  else
    m_initValue = value;
  m_decimals = decs;
}

void PHTML::InputReal::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  html << std::fixed << std::setprecision(m_decimals)
       << " MIN=" << m_minValue
       << " MAX=" << m_maxValue
       << " VALUE=" << m_initValue
       << " STEP=" << pow(10, -m_decimals);
}


PHTML::InputRange::InputRange(const char * fname,
                              int min, int max, int value,
                              DisableCodes disabled,
                              const char * attr)
  : InputNumber("range", fname, min, max, value, disabled, attr)
{
}


PHTML::InputFile::InputFile(const char * fname,
                            const char * accept,
                            DisableCodes disabled,
                            const char * attr)
  : InputField("file", fname, disabled, attr)
  , m_acceptString(accept)
{
}

void PHTML::InputFile::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  if (!m_acceptString.IsEmpty())
    html << " ACCEPT=\"" << Escaped(m_acceptString) << '"';
}


PHTML::InputImage::InputImage(const char * fname,
                              const char * src,
                              DisableCodes disabled,
                              const char * attr)
  : InputField("image", fname, disabled, attr)
  , m_srcString(src)
{
}

PHTML::InputImage::InputImage(const char * type,
                              const char * fname,
                              const char * src,
                              DisableCodes disabled,
                              const char * attr)
  : InputField(type, fname, disabled, attr)
  , m_srcString(src)
{
}

void PHTML::InputImage::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  if (!m_srcString.IsEmpty())
    html << " SRC=\"" << Escaped(m_srcString) << '"';
}


PHTML::InputScribble::InputScribble(const char * fname,
                                    const char * src,
                                    DisableCodes disabled,
                                    const char * attr)
  : InputImage("scribble", fname, src, disabled, attr)
{
}

PHTML::ResetButton::ResetButton(const char * title,
                                const char * fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * attr)
  : InputImage("reset", fname != NULL ? fname : "reset", src, disabled, attr)
  , m_titleString(title)
{
}

PHTML::ResetButton::ResetButton(const char * type,
                                const char * title,
                                const char * fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * attr)
  : InputImage(type, fname, src, disabled, attr)
  , m_titleString(title)
{
}

void PHTML::ResetButton::AddAttr(PHTML & html) const
{
  InputImage::AddAttr(html);
  if (!m_titleString.IsEmpty())
    html << " VALUE=\"" << Escaped(m_titleString) << '"';
}


PHTML::SubmitButton::SubmitButton(const char * title,
                                  const char * fname,
                                  const char * src,
                                  DisableCodes disabled,
                                  const char * attr)
  : ResetButton("submit",
                  title, fname != NULL ? fname : "submit", src, disabled, attr)
{
}

// End Of File ///////////////////////////////////////////////////////////////
