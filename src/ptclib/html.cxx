/*
 * $Id: html.cxx,v 1.4 1996/02/25 11:14:22 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: html.cxx,v $
 * Revision 1.4  1996/02/25 11:14:22  robertj
 * Radio button support for forms.
 *
 * Revision 1.3  1996/02/19 13:31:51  robertj
 * Removed MSC_VER test as now completely removed from WIN16 library.
 *
 * Revision 1.2  1996/02/08 12:24:30  robertj
 * Further implementation.
 *
 * Revision 1.1  1996/02/03 11:18:46  robertj
 * Initial revision
 *
 * Revision 1.3  1996/01/28 02:49:16  robertj
 * Further implementation.
 *
 * Revision 1.2  1996/01/26 02:24:30  robertj
 * Further implemetation.
 *
 * Revision 1.1  1996/01/23 13:04:32  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <html.h>


//////////////////////////////////////////////////////////////////////////////
// PHTML

PHTML::PHTML()
{
  memset(elementSet, 0, sizeof(elementSet));
}


PHTML::PHTML(const char * cstr)
{
  memset(elementSet, 0, sizeof(elementSet));
  *this << Title(cstr) << Body() << Heading(1) << cstr << Heading(1);
}


PHTML::PHTML(const PString & str)
{
  memset(elementSet, 0, sizeof(elementSet));
  *this << Title(str) << Body() << Heading(1) << str << Heading(1);
}


PHTML::~PHTML()
{
  for (PINDEX i = 0; i < PARRAYSIZE(elementSet); i++)
    PAssert(elementSet[i] == 0, "Failed to close elements");
}


BOOL PHTML::Is(ElementInSet elmt)
{
  return (elementSet[elmt>>3]&(1<<(elmt&7))) != 0;
}


void PHTML::Set(ElementInSet elmt)
{
  elementSet[elmt>>3] |= (1<<(elmt&7));
}


void PHTML::Clr(ElementInSet elmt)
{
  elementSet[elmt>>3] &= ~(1<<(elmt&7));
}


void PHTML::Toggle(ElementInSet elmt)
{
  elementSet[elmt>>3] ^= (1<<(elmt&7));
}


void PHTML::Element::Output(PHTML & html) const
{
  PAssert(reqElement == NumElementsInSet || html.Is(reqElement),
                                                "HTML element out of centext");

  if (crlf == OpenCRLF && !html.Is(inElement))
    html << "\r\n";

  html << '<';
  if (html.Is(inElement))
    html << '/';
  html << name;

  AddAttr(html);

  html << '>';
  if (crlf != NoCRLF && (crlf == BothCRLF || html.Is(inElement)))
    html << "\r\n";

  if (inElement != NumElementsInSet)
    html.Toggle(inElement);
}


void PHTML::Element::AddAttr(PHTML &) const
{
}


PHTML::Head::Head()
  : Element("HEAD", InHead, NumElementsInSet, BothCRLF)
{
}

void PHTML::Head::Output(PHTML & html) const
{
  PAssert(!html.Is(InBody), "HTML element out of centext");
  Element::Output(html);
}


PHTML::Body::Body()
  : Element("BODY", InBody, NumElementsInSet, BothCRLF)
{
}


void PHTML::Body::Output(PHTML & html) const
{
  if (html.Is(InTitle))
    html << Title();
  if (html.Is(InHead))
    html << Head();
  Element::Output(html);
}


PHTML::Title::Title()
  : Element("TITLE", InTitle, InHead, CloseCRLF)
{
  titleString = NULL;
}

PHTML::Title::Title(const char * titleCStr)
  : Element("TITLE", InTitle, InHead, CloseCRLF)
{
  titleString = titleCStr;
}

PHTML::Title::Title(const PString & titleStr)
  : Element("TITLE", InTitle, InHead, CloseCRLF)
{
  titleString = titleStr;
}

void PHTML::Title::Output(PHTML & html) const
{
  PAssert(!html.Is(InBody), "HTML element out of centext");
  if (!html.Is(InHead))
    html << Head();
  if (html.Is(InTitle)) {
    if (titleString != NULL)
      html << titleString;
    Element::Output(html);
  }
  else {
    Element::Output(html);
    if (titleString != NULL) {
      html << titleString;
      Element::Output(html);
    }
  }
}


PHTML::Banner::Banner()
  : Element("BANNER", NumElementsInSet, InBody, BothCRLF)
{
}


PHTML::ClearedElement::ClearedElement(const char * n,
                                      ElementInSet elmt,
                                      ElementInSet req,
                                      OptionalCRLF c,
                                      ClearCodes clear,
                                      int distance)
  : Element(n, elmt, req, c)
{
  clearCode = clear;
  clearDistance = distance;
}


void PHTML::ClearedElement::AddAttr(PHTML & html) const
{
  static const char * const ClearCodeString[PHTML::NumClearCodes] = {
    "", "left", "right", "all", " en", " pixels"
  };
  if (*ClearCodeString[clearCode] != '\0') {
    html << " CLEAR=";
    if (*ClearCodeString[clearCode] == ' ')
      html << clearDistance;
    html << ClearCodeString[clearCode];
  }
}


PHTML::ComplexElement::ComplexElement(const char * n,
                                      ElementInSet elmt,
                                      ElementInSet req,
                                      OptionalCRLF c,
                                      AlignCodes align,
                                      NoWrapCodes noWrap,
                                      ClearCodes clear,
                                      int distance)
  : ClearedElement(n, elmt, req, c, clear, distance)
{
  alignCode = align;
  noWrapFlag = noWrap == NoWrap;
}


void PHTML::ComplexElement::AddAttr(PHTML & html) const
{
  static const char * const AlignCodeString[PHTML::NumAlignCodes] = {
    "", "left", "centre", "right", "justify", "top", "bottom", "decimal"
  };
  if (*AlignCodeString[alignCode] != '\0')
    html << " ALIGN=" << AlignCodeString[alignCode];

  if (noWrapFlag)
    html << " NOWRAP";

  ClearedElement::AddAttr(html);
}


PHTML::Division::Division(ClearCodes clear, int distance)
  : ClearedElement("DIV", InDivision, InBody, BothCRLF, clear, distance)
{
}


PHTML::Heading::Heading(int number,
                        int sequence,
                        int skip,
                        AlignCodes align,
                        NoWrapCodes noWrap,
                        ClearCodes clear,
                        int distance)
  : ComplexElement("H", InHeading, InBody, CloseCRLF,
                   align, noWrap, clear, distance)
{
  num = number;
  srcString = NULL;
  seqNum = sequence;
  skipSeq = skip;
}

PHTML::Heading::Heading(int number,
                        const char * image,
                        int sequence,
                        int skip,
                        AlignCodes align,
                        NoWrapCodes noWrap,
                        ClearCodes clear,
                        int distance)
  : ComplexElement("H", InHeading, InBody, CloseCRLF,
                   align, noWrap, clear, distance)
{
  num = number;
  srcString = image;
  seqNum = sequence;
  skipSeq = skip;
}

PHTML::Heading::Heading(int number,
                        const PString & imageStr,
                        int sequence,
                        int skip,
                        AlignCodes align,
                        NoWrapCodes noWrap,
                        ClearCodes clear,
                        int distance)
  : ComplexElement("H", InHeading, InBody, CloseCRLF,
                   align, noWrap, clear, distance)
{
  num = number;
  srcString = imageStr;
  seqNum = sequence;
  skipSeq = skip;
}

void PHTML::Heading::AddAttr(PHTML & html) const
{
  PAssert(num >= 1 && num <= 6, "Bad heading number");
  html << num;
  if (srcString != NULL)
    html << " SRC=\"" << srcString << '"';
  if (seqNum > 0)
    html << " SEQNUM=" << seqNum;
  if (skipSeq > 0)
    html << " SKIP=" << skipSeq;
  ComplexElement::AddAttr(html);
}


PHTML::BreakLine::BreakLine(ClearCodes clear, int distance)
  : ClearedElement("BR", NumElementsInSet, InBody, BothCRLF, clear, distance)
{
}


PHTML::Paragraph::Paragraph(AlignCodes align,
                            NoWrapCodes noWrap,
                            ClearCodes clear,
                            int distance)
  : ComplexElement("P", NumElementsInSet, InBody, OpenCRLF,
                   align, noWrap, clear, distance)
{
}


PHTML::PreFormat::PreFormat(int widthInChars, ClearCodes clear, int distance)
  : ClearedElement("PRE", InPreFormat, InBody, CloseCRLF, clear, distance)
{
  width = widthInChars;
}


void PHTML::PreFormat::AddAttr(PHTML & html) const
{
  if (width > 0)
    html << " WIDTH=" << width;
  ClearedElement::AddAttr(html);
}


PHTML::Anchor::Anchor(const char * href)
  : Element("A", InAnchor, InBody, NoCRLF)
{
  hrefString = href;
}

PHTML::Anchor::Anchor(const PString & hrefStr)
  : Element("A", InAnchor, InBody, NoCRLF)
{
  hrefString = hrefStr;
}

void PHTML::Anchor::AddAttr(PHTML & html) const
{
  if (hrefString != NULL && *hrefString != '\0')
    html << " HREF=\"" << hrefString << '"';
  else
    PAssert(html.Is(InAnchor), PInvalidParameter);
}


PHTML::ImageElement::ImageElement(const char * n,
                                  ElementInSet elmt,
                                  ElementInSet req,
                                  OptionalCRLF c,
                                  const char * image,
                                  AlignCodes align,
                                  NoWrapCodes noWrap,
                                  ClearCodes clear,
                                  int distance)
  : ComplexElement(n, elmt, req, c, align, noWrap, clear, distance)
{
  srcString = image;
}


void PHTML::ImageElement::AddAttr(PHTML & html) const
{
  if (srcString != NULL)
    html << " SRC=\"" << srcString << '"';
  ComplexElement::AddAttr(html);
}


PHTML::Image::Image(const char * src, AlignCodes align, int w, int h)
  : ImageElement("IMG", NumElementsInSet, InBody, NoCRLF,
                 src, align, WrapWords, ClearDefault, 0)
{
  altString = NULL;
  width = w;
  height = h;
}

PHTML::Image::Image(const char * src,
                    const char * alt,
                    AlignCodes align,
                    int w, int h)
  : ImageElement("IMG", NumElementsInSet, InBody, NoCRLF,
                 src, align, WrapWords, ClearDefault, 0)
{
  altString = alt;
  width = w;
  height = h;
}

PHTML::Image::Image(const PString & srcStr,
                    const char * alt,
                    AlignCodes align,
                    int w, int h)
  : ImageElement("IMG", NumElementsInSet, InBody, NoCRLF,
                 srcStr, align, WrapWords, ClearDefault, 0)
{
  altString = alt;
  width = w;
  height = h;
}

PHTML::Image::Image(const PString & srcStr,
                    const PString & altStr,
                    AlignCodes align,
                    int w, int h)
  : ImageElement("IMG", NumElementsInSet, InBody, NoCRLF,
                 srcStr, align, WrapWords, ClearDefault, 0)
{
  altString = altStr;
  width = w;
  height = h;
}

void PHTML::Image::AddAttr(PHTML & html) const
{
  PAssert(srcString != NULL && *srcString != '\0', PInvalidParameter);
  if (altString != NULL)
    html << " ALT=\"" << altString << '"';
  if (width != 0)
    html << " WIDTH=" << width;
  if (height != 0)
    html << " HEIGHT=" << height;
  ImageElement::AddAttr(html);
}


PHTML::HRule::HRule(const char * image, ClearCodes clear, int distance)
  : ImageElement("HR", NumElementsInSet, InBody, BothCRLF,
                 image, AlignDefault, WrapWords, clear, distance)
{
}

PHTML::HRule::HRule(const PString & imageStr, ClearCodes clear, int distance)
  : ImageElement("HR", NumElementsInSet, InBody, BothCRLF,
                 imageStr, AlignDefault, WrapWords, clear, distance)
{
}


PHTML::Note::Note(const char * image, ClearCodes clear, int distance)
  : ImageElement("NOTE", InNote, InBody, BothCRLF,
                 image, AlignDefault, WrapWords, clear, distance)
{
}

PHTML::Note::Note(const PString & imageStr, ClearCodes clear, int distance)
  : ImageElement("NOTE", InNote, InBody, BothCRLF,
                 imageStr, AlignDefault, WrapWords, clear, distance)
{
}


PHTML::Address::Address(NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ComplexElement("ADDRESS", InAddress, InBody, BothCRLF,
                   AlignDefault, noWrap, clear, distance)
{
}


PHTML::BlockQuote::BlockQuote(NoWrapCodes noWrap,ClearCodes clear,int distance)
  : ComplexElement("BQ", InBlockQuote, InBody, BothCRLF,
                   AlignDefault, noWrap, clear, distance)
{
}


PHTML::Credit::Credit()
  : Element("CREDIT", NumElementsInSet, InBlockQuote, OpenCRLF)
{
}

PHTML::SetTab::SetTab(const char * id)
  : Element("TAB", NumElementsInSet, InBody, NoCRLF)
{
  ident = id;
}

PHTML::SetTab::SetTab(const PString & idStr)
  : Element("TAB", NumElementsInSet, InBody, NoCRLF)
{
  ident = idStr;
}

void PHTML::SetTab::AddAttr(PHTML & html) const
{
  PAssert(ident != NULL && *ident != '\0', PInvalidParameter);
  html << " ID=" << ident;
}


PHTML::Tab::Tab(int indent, AlignCodes align, char decimal)
  : ComplexElement("TAB", NumElementsInSet, InBody, NoCRLF,
                   align, WrapWords, ClearDefault, 0)
{
  ident = NULL;
  indentSize = indent;
  decimalPoint = decimal;
}

PHTML::Tab::Tab(const char * id, AlignCodes align, char decimal)
  : ComplexElement("TAB", NumElementsInSet, InBody, NoCRLF,
                   align, WrapWords, ClearDefault, 0)
{
  ident = id;
  indentSize = 0;
  decimalPoint = decimal;
}

PHTML::Tab::Tab(const PString & idStr, AlignCodes align, char decimal)
  : ComplexElement("TAB", NumElementsInSet, InBody, NoCRLF,
                   align, WrapWords, ClearDefault, 0)
{
  ident = idStr;
  indentSize = 0;
  decimalPoint = decimal;
}

void PHTML::Tab::AddAttr(PHTML & html) const
{
  PAssert(indentSize!=0 || (ident!=NULL && *ident!='\0'), PInvalidParameter);
  if (indentSize > 0)
    html << " INDENT=" << indentSize;
  else
    html << " TO=" << ident;
  if (decimalPoint != '\0')
    html << " DP=\"" << decimalPoint << '"';
  ComplexElement::AddAttr(html);
}


PHTML::ListElement::ListElement(const char * n,
                                CompactCodes compact,
                                ClearCodes clear,
                                int distance)
  : ClearedElement(n, InList, InBody, BothCRLF, clear, distance)
{
  compactFlag = compact == Compact;
}


void PHTML::ListElement::AddAttr(PHTML & html) const
{
  if (compactFlag)
    html << " COMPACT";
  ClearedElement::AddAttr(html);
}


PHTML::UnorderedList::UnorderedList(PlainCodes plain,
                                    ListWrapCodes wrap,
                                    CompactCodes compact,
                                    ClearCodes clear,
                                    int distance)
  : ListElement("UL", compact, clear, distance)
{
  plainFlag  = plain == Plain;
  wrapColumn = wrap;
}


void PHTML::UnorderedList::AddAttr(PHTML & html) const
{
  if (plainFlag)
    html << " PLAIN";
  if (wrapColumn != WrapDefault)
    html << " WRAP=" << (wrapColumn != WrapHoriz ? "vert" : "horiz");
  ListElement::AddAttr(html);
}


PHTML::OrderedList::OrderedList(BOOL contSeq,
                                int seqNum,
                                CompactCodes compact,
                                ClearCodes clear,
                                int distance)
  : ListElement("OL", compact, clear, distance)
{
  continueSeq = contSeq;
  sequenceNum = seqNum;
}

void PHTML::OrderedList::AddAttr(PHTML & html) const
{
  if (continueSeq)
    html << " CONTINUE";
  if (sequenceNum > 0)
    html << " SEQNUM=" << sequenceNum;
  ListElement::AddAttr(html);
}


PHTML::DefinitionList::DefinitionList(CompactCodes compact,
                                      ClearCodes clear,
                                      int distance)
  : ListElement("DL", compact, clear, distance)
{
}


PHTML::ListHeading::ListHeading()
  : Element("LH", InListHeading, InList, CloseCRLF)
{
}

PHTML::ListItem::ListItem(int skip, ClearCodes clear, int distance)
  : ClearedElement("LI", NumElementsInSet, InList, OpenCRLF, clear, distance)
{
  skipSeq = skip;
}

void PHTML::ListItem::AddAttr(PHTML & html) const
{
  if (skipSeq > 0)
    html << " SKIP=" << skipSeq;
  ClearedElement::AddAttr(html);
}


PHTML::DefinitionTerm::DefinitionTerm(ClearCodes clear, int distance)
  : ClearedElement("DT", NumElementsInSet, InList, NoCRLF, clear, distance)
{
}

void PHTML::DefinitionTerm::Output(PHTML & html) const
{
  PAssert(!html.Is(InDefinitionTerm), "HTML definition item missing");
  Element::Output(html);
  html.Set(InDefinitionTerm);
}


PHTML::DefinitionItem::DefinitionItem(ClearCodes clear, int distance)
  : ClearedElement("DD", NumElementsInSet, InList, NoCRLF, clear, distance)
{
}

void PHTML::DefinitionItem::Output(PHTML & html) const
{
  PAssert(html.Is(InDefinitionTerm), "HTML definition term missing");
  Element::Output(html);
  html.Clr(InDefinitionTerm);
}


PHTML::Table::Table(BorderCodes border)
  : Element("TABLE", InTable, InBody, BothCRLF)
{
  borderFlag = border == Border;
}

void PHTML::Table::AddAttr(PHTML & html) const
{
  if (borderFlag)
    html << " BORDER";
}


PHTML::TableElement::TableElement(const char * nam,
                                  ElementInSet elmt,
                                  OptionalCRLF opt,
                                  const char * attr)
  : Element(nam, elmt, InTable, opt)
{
  attributes = attr;
}

void PHTML::TableElement::AddAttr(PHTML & html) const
{
  PAssert(html.Is(InTable), "HTML element out of context");
  if (attributes != NULL)
    html << ' ' << attributes;
}


PHTML::TableRow::TableRow(const char * attr)
  : TableElement("TR", NumElementsInSet, OpenCRLF, attr)
{
}


PHTML::TableHeader::TableHeader(const char * attr)
  : TableElement("TH", NumElementsInSet, CloseCRLF, attr)
{
}


PHTML::TableData::TableData(const char * attr)
  : TableElement("TD", NumElementsInSet, NoCRLF, attr)
{
}


PHTML::Form::Form(const char * method,
                  const char * action,
                  const char * mimeType,
                  const char * script)
  : Element("FORM", InForm, InBody, BothCRLF)
{
  actionString = action;
  methodString = method;
  mimeTypeString = mimeType;
  scriptString = script;
}

PHTML::Form::Form(const PString & method,
                  const char * action,
                  const char * mimeType,
                  const char * script)
  : Element("FORM", InForm, InBody, BothCRLF)
{
  methodString = method;
  actionString = action;
  mimeTypeString = mimeType;
  scriptString = script;
}

PHTML::Form::Form(const PString & method,
                  const PString & action,
                  const char * mimeType,
                  const char * script)
  : Element("FORM", InForm, InBody, BothCRLF)
{
  methodString = method;
  actionString = action;
  mimeTypeString = mimeType;
  scriptString = script;
}

PHTML::Form::Form(const PString & method,
                  const PString & action,
                  const PString & mimeType,
                  const char * script)
  : Element("FORM", InForm, InBody, BothCRLF)
{
  methodString = method;
  actionString = action;
  mimeTypeString = mimeType;
  scriptString = script;
}

PHTML::Form::Form(const PString & method,
                  const PString & action,
                  const PString & mimeType,
                  const PString & script)
  : Element("FORM", InForm, InBody, BothCRLF)
{
  methodString = method;
  actionString = action;
  mimeTypeString = mimeType;
  scriptString = script;
}

void PHTML::Form::AddAttr(PHTML & html) const
{
  if (methodString != NULL)
    html << " METHOD=" << methodString;
  if (actionString != NULL)
    html << " ACTION=\"" << actionString << '"';
  if (mimeTypeString != NULL)
    html << " ENCTYPE=\"" << mimeTypeString << '"';
  if (scriptString != NULL)
    html << " SCRIPT=\"" << scriptString << '"';
  Element::AddAttr(html);
}


PHTML::FieldElement::FieldElement(const char * n,
                                  ElementInSet elmt,
                                  OptionalCRLF c,
                                  DisableCodes disabled,
                                  const char * error)
  : Element(n, elmt, InForm, c)
{
  disabledFlag = disabled == Disabled;
  errorString = error;
}

void PHTML::FieldElement::AddAttr(PHTML & html) const
{
  if (disabledFlag)
    html << " DISABLED";
  if (errorString != NULL)
    html << " ERROR=\"" << errorString << '"';
  Element::AddAttr(html);
}


PHTML::Select::Select(const char * fname,
                      MultipleCodes multiple,
                      DisableCodes disabled,
                      const char * error)
  : FieldElement("SELECT", InSelect, BothCRLF, disabled, error)
{
  nameString = fname;
  multipleFlag = multiple == MultipleSelect;
}

PHTML::Select::Select(const PString & fname,
                      MultipleCodes multiple,
                      DisableCodes disabled,
                      const char * error)
  : FieldElement("SELECT", InSelect, BothCRLF, disabled, error)
{
  nameString = fname;
  multipleFlag = multiple == MultipleSelect;
}

void PHTML::Select::AddAttr(PHTML & html) const
{
  if (!html.Is(InSelect)) {
    PAssert(nameString != NULL && *nameString != '\0', PInvalidParameter);
    html << " NAME=\"" << nameString << '"';
  }
  if (multipleFlag)
    html << " MULTIPLE";
  FieldElement::AddAttr(html);
}


PHTML::Option::Option(SelectionCodes select,
                      DisableCodes disabled,
                      const char * error)
  : FieldElement("OPTION", NumElementsInSet, NoCRLF, disabled, error)
{
  selectedFlag = select == Selected;
}

void PHTML::Option::AddAttr(PHTML & html) const
{
  if (selectedFlag)
    html << " SELECTED";
}


PHTML::FormField::FormField(const char * n, ElementInSet elmt, OptionalCRLF c,
                 const char * fname, DisableCodes disabled, const char * error)
  : FieldElement(n, elmt, c, disabled, error)
{
  nameString = fname;
}

void PHTML::FormField::AddAttr(PHTML & html) const
{
  PAssert(nameString != NULL && *nameString != '\0', PInvalidParameter);
  html << " NAME=\"" << nameString << '"';
  FieldElement::AddAttr(html);
}


PHTML::TextArea::TextArea(const char * fname,
                          int rows, int cols,
                          DisableCodes disabled,
                          const char * error)
  : FormField("TEXTAREA", InSelect, BothCRLF, fname, disabled, error)
{
  numRows = rows;
  numCols = cols;
}

PHTML::TextArea::TextArea(const PString & fname,
                          int rows, int cols,
                          DisableCodes disabled,
                          const char * error)
  : FormField("TEXTAREA", InSelect, BothCRLF, fname, disabled, error)
{
  numRows = rows;
  numCols = cols;
}

void PHTML::TextArea::AddAttr(PHTML & html) const
{
  if (numRows > 0)
    html << " ROWS=" << numRows;
  if (numCols > 0)
    html << " COLS=" << numCols;
  FormField::AddAttr(html);
}


PHTML::InputField::InputField(const char * type,
                              const char * fname,
                              DisableCodes disabled,
                              const char * error)
  : FormField("INPUT", NumElementsInSet, NoCRLF, fname, disabled, error)
{
  typeString = type;
}

void PHTML::InputField::AddAttr(PHTML & html) const
{
  PAssert(typeString != NULL && *typeString != '\0', PInvalidParameter);
  html << " TYPE=" << typeString;
  FormField::AddAttr(html);
}


PHTML::InputText::InputText(const char * fname,
                            int size,
                            const char * init,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField("text", fname, disabled, error)
{
  width = size;
  length = maxLength;
  value = init;
}

PHTML::InputText::InputText(const PString & fname,
                            int size,
                            const char * init,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField("text", fname, disabled, error)
{
  width = size;
  length = maxLength;
  value = init;
}

PHTML::InputText::InputText(const char * fnameStr,
                            int size,
                            const PString & init,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField("text", fnameStr, disabled, error)
{
  width = size;
  length = maxLength;
  value = init;
}

PHTML::InputText::InputText(const PString & fnameStr,
                            int size,
                            const PString & init,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField("text", fnameStr, disabled, error)
{
  width = size;
  length = maxLength;
  value = init;
}

PHTML::InputText::InputText(const char * type,
                            const char * fname,
                            int size,
                            const char * init,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField(type, fname, disabled, error)
{
  width = size;
  length = maxLength;
  value = init;
}

void PHTML::InputText::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  html << " SIZE=" << width;
  if (length > 0)
    html << " MAXLENGTH=" << length;
  if (value != NULL)
    html << " VALUE=\"" << value << '"';
}


PHTML::InputPassword::InputPassword(const char * fname,
                                    int size,
                                    const char * init,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * error)
  : InputText("password", fname, size, init, maxLength, disabled, error)
{
}

PHTML::InputPassword::InputPassword(const PString & fnameStr,
                                    int size,
                                    const char * init,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * error)
  : InputText("password", fnameStr, size, init, maxLength, disabled, error)
{
}

PHTML::InputPassword::InputPassword(const char * fname,
                                    int size,
                                    const PString & init,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * error)
  : InputText("password", fname, size, init, maxLength, disabled, error)
{
}

PHTML::InputPassword::InputPassword(const PString & fnameStr,
                                    int size,
                                    const PString & init,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * error)
  : InputText("password", fnameStr, size, init, maxLength, disabled, error)
{
}


PHTML::CheckBox::CheckBox(const char * fname,
                          CheckedCodes check,
                          DisableCodes disabled,
                          const char * error)
  : InputField("checkbox", fname, disabled, error)
{
  checkedFlag = check == Checked;
}

PHTML::CheckBox::CheckBox(const PString & fnameStr,
                          CheckedCodes check,
                          DisableCodes disabled,
                          const char * error)
  : InputField("checkbox", fnameStr, disabled, error)
{
  checkedFlag = check == Checked;
}

PHTML::CheckBox::CheckBox(const char * type,
                          const char * fname,
                          CheckedCodes check,
                          DisableCodes disabled,
                          const char * error)
  : InputField(type, fname, disabled, error)
{
  checkedFlag = check == Checked;
}

void PHTML::CheckBox::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  if (checkedFlag)
    html << " CHECKED";
}


PHTML::RadioButton::RadioButton(const char * fname,
                                const char * value,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * error)
  : CheckBox("radio", fname, check, disabled, error)
{
  valueString = value;
}

PHTML::RadioButton::RadioButton(const PString & fnameStr,
                                const char * value,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * error)
  : CheckBox("radio", fnameStr, check, disabled, error)
{
  valueString = value;
}

PHTML::RadioButton::RadioButton(const char * fname,
                                const PString & value,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * error)
  : CheckBox("radio", fname, check, disabled, error)
{
  valueString = value;
}

PHTML::RadioButton::RadioButton(const PString & fnameStr,
                                const PString & value,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * error)
  : CheckBox("radio", fnameStr, check, disabled, error)
{
  valueString = value;
}

void PHTML::RadioButton::AddAttr(PHTML & html) const
{
  PAssert(valueString != NULL, PInvalidParameter);
  CheckBox::AddAttr(html);
  html << " VALUE=" << valueString;
}


PHTML::InputRange::InputRange(const char * fname,
                              int min, int max, int value,
                              DisableCodes disabled,
                              const char * error)
  : InputField("range", fname, disabled, error)
{
  PAssert(min <= max, PInvalidParameter);
  minValue = min;
  maxValue = max;
  if (value < min)
    initValue = min;
  else if (value > max)
    initValue = max;
  else
    initValue = value;
}

PHTML::InputRange::InputRange(const PString & fnameStr,
                              int min, int max, int value,
                              DisableCodes disabled,
                              const char * error)
  : InputField("range", fnameStr, disabled, error)
{
  PAssert(min <= max, PInvalidParameter);
  minValue = min;
  maxValue = max;
  if (value < min)
    initValue = min;
  else if (value > max)
    initValue = max;
  else
    initValue = value;
}

void PHTML::InputRange::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  PINDEX max = PMAX(-minValue, maxValue);
  PINDEX width = 1;
  while (max > 10) {
    width++;
    max /= 10;
  }
  html << " SIZE=" << width
       << " MIN=" << minValue
       << " MAX=" << maxValue
       << " VALUE=" << initValue;
}


PHTML::InputFile::InputFile(const char * fname,
                            const char * accept,
                            DisableCodes disabled,
                            const char * error)
  : InputField("file", fname, disabled, error)
{
  acceptString = accept;
}

PHTML::InputFile::InputFile(const PString & fnameStr,
                            const char * accept,
                            DisableCodes disabled,
                            const char * error)
  : InputField("file", fnameStr, disabled, error)
{
  acceptString = accept;
}

PHTML::InputFile::InputFile(const char * fname,
                            const PString & acceptStr,
                            DisableCodes disabled,
                            const char * error)
  : InputField("file", fname, disabled, error)
{
  acceptString = acceptStr;
}

PHTML::InputFile::InputFile(const PString & fnameStr,
                            const PString & acceptStr,
                            DisableCodes disabled,
                            const char * error)
  : InputField("file", fnameStr, disabled, error)
{
  acceptString = acceptStr;
}

void PHTML::InputFile::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  if (acceptString != NULL)
    html << " ACCEPT=\"" << acceptString << '"';
}


PHTML::InputImage::InputImage(const char * fname,
                              const char * src,
                              DisableCodes disabled,
                              const char * error)
  : InputField("image", fname, disabled, error)
{
  srcString = src;
}

PHTML::InputImage::InputImage(const PString & fnameStr,
                              const char * src,
                              DisableCodes disabled,
                              const char * error)
  : InputField("image", fnameStr, disabled, error)
{
  srcString = src;
}

PHTML::InputImage::InputImage(const char * fname,
                              const PString & srcStr,
                              DisableCodes disabled,
                              const char * error)
  : InputField("image", fname, disabled, error)
{
  srcString = srcStr;
}

PHTML::InputImage::InputImage(const PString & fname,
                              const PString & srcStr,
                              DisableCodes disabled,
                              const char * error)
  : InputField("image", fname, disabled, error)
{
  srcString = srcStr;
}

PHTML::InputImage::InputImage(const char * type,
                              const char * fname,
                              const char * src,
                              DisableCodes disabled,
                              const char * error)
  : InputField(type, fname, disabled, error)
{
  srcString = src;
}

void PHTML::InputImage::AddAttr(PHTML & html) const
{
  InputField::AddAttr(html);
  if (srcString != NULL)
    html << " SRC=\"" << srcString << '"';
}


PHTML::InputScribble::InputScribble(const char * fname,
                                    const char * src,
                                    DisableCodes disabled,
                                    const char * error)
  : InputImage("scribble", fname, src, disabled, error)
{
}

PHTML::InputScribble::InputScribble(const PString & fname,
                                    const char * src,
                                    DisableCodes disabled,
                                    const char * error)
  : InputImage("scribble", fname, src, disabled, error)
{
}

PHTML::InputScribble::InputScribble(const char * fname,
                                    const PString & srcStr,
                                    DisableCodes disabled,
                                    const char * error)
  : InputImage("scribble", fname, srcStr, disabled, error)
{
}

PHTML::InputScribble::InputScribble(const PString & fname,
                                    const PString & srcStr,
                                    DisableCodes disabled,
                                    const char * error)
  : InputImage("scribble", fname, srcStr, disabled, error)
{
}


PHTML::ResetButton::ResetButton(const char * title,
                                const char * fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname != NULL ? fname : "reset", src, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const PString & title,
                                const char * fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname != NULL ? fname : "reset", src, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const char * title,
                                const PString & fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, src, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const PString & title,
                                const PString & fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, src, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const char * title,
                                const char * fname,
                                const PString & srcStr,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname != NULL ? fname : "reset", srcStr, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const PString & title,
                                const char * fname,
                                const PString & srcStr,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname != NULL ? fname : "reset", srcStr, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const char * title,
                                const PString & fname,
                                const PString & srcStr,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, srcStr, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const PString & title,
                                const PString & fname,
                                const PString & srcStr,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, srcStr, disabled, error)
{
  titleString = title;
}

PHTML::ResetButton::ResetButton(const char * type,
                                const char * fname,
                                const char * title,
                                const char * src,
                                DisableCodes disabled,
                                const char * error)
  : InputImage(type, fname != NULL ? fname : type, src, disabled, error)
{
  titleString = title;
}

void PHTML::ResetButton::AddAttr(PHTML & html) const
{
  InputImage::AddAttr(html);
  if (titleString != NULL)
    html << " VALUE=\"" << titleString << '"';
}


PHTML::SubmitButton::SubmitButton(const char * title,
                                  const char * fname,
                                  const char * src,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, src, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const PString & title,
                                  const char * fname,
                                  const char * src,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, src, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const char * title,
                                  const PString & fname,
                                  const char * src,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, src, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const PString & title,
                                  const PString & fname,
                                  const char * src,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, src, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const char * title,
                                  const char * fname,
                                  const PString & srcStr,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, srcStr, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const PString & title,
                                  const char * fname,
                                  const PString & srcStr,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, srcStr, disabled, error)
{
}


PHTML::SubmitButton::SubmitButton(const char * title,
                                  const PString & fname,
                                  const PString & srcStr,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, srcStr, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const PString & title,
                                  const PString & fname,
                                  const PString & srcStr,
                                  DisableCodes disabled,
                                  const char * error)
  : ResetButton("submit", fname, title, srcStr, disabled, error)
{
}


// End Of File ///////////////////////////////////////////////////////////////
