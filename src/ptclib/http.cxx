/*
 * $Id: http.cxx,v 1.3 1996/01/28 02:49:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: http.cxx,v $
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
#include <http.h>
#include <ctype.h>


//////////////////////////////////////////////////////////////////////////////
// PURL

PURL::PURL()
{
  port = 80;
}


PURL::PURL(const char * str)
{
  Parse(str);
}


PURL::PURL(const PString & str)
{
  Parse(str);
}


PObject::Comparison PURL::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PURL::Class()), PInvalidCast);
  const PURL & other = (const PURL &)obj;
  Comparison c = scheme.Compare(other.scheme);
  if (c == EqualTo) {
    c = username.Compare(other.username);
    if (c == EqualTo) {
      c = password.Compare(other.password);
      if (c == EqualTo) {
        c = hostname.Compare(other.hostname);
        if (c == EqualTo) {
          c = path.Compare(other.path);
          if (c == EqualTo) {
            c = parameters.Compare(other.parameters);
            if (c == EqualTo) {
              c = fragment.Compare(other.fragment);
              if (c == EqualTo)
                c = query.Compare(other.query);
            }
          }
        }
      }
    }
  }
  return c;
}


void PURL::PrintOn(ostream & stream) const
{
  stream << AsString(FullURL);
}


void PURL::ReadFrom(istream & stream)
{
  PString s;
  stream >> s;
  Parse(s);
}


void PURL::Parse(const char * cstr)
{
  scheme = hostname = PCaselessString();
  username = password = parameters = fragment = query = PString();
  path.SetSize(0);
  port = 80;
  absolutePath = TRUE;

  // copy the string so we can take bits off it
  PString url = cstr;

  static PString reservedChars = "=;/#?";
  PINDEX pos;

  pos = (PINDEX)-1;
  while ((pos = url.Find('%', pos+1)) != P_MAX_INDEX) {
    int digit1 = url[pos+1];
    int digit2 = url[pos+2];
    if (isxdigit(digit1) && isxdigit(digit2)) {
      url[pos] = (char)(
            (isdigit(digit2) ? (digit2-'0') : (toupper(digit2)-'A'+10)) +
           ((isdigit(digit1) ? (digit1-'0') : (toupper(digit1)-'A'+10)) << 4));
      url.Delete(pos+1, 2);
    }
  }

  // determine if the URL has a scheme
  if (isalpha(url[0])) {
    for (pos = 0; url[pos] != '\0' &&
                          reservedChars.Find(url[pos]) == P_MAX_INDEX; pos++) {
      if (url[pos] == ':') {
        scheme = url.Left(pos);
        url.Delete(0, pos+1);
        break;
      }
    }
  }

  // determine if the URL is absolute or relative - only absolute
  // URLs can have a username/password string
  if (url.GetLength() > 2 && url[0] == '/' && url[1] == '/') {
    // extract username and password
    PINDEX pos2 = url.Find('@');
    if (pos2 != P_MAX_INDEX && pos2 > 0) {
      pos = url.Find(":");

      // if no password...
      if (pos > pos2)
        username = url(2, pos2-1);
      else {
        username = url(2, pos-1);
        password = url(pos+1, pos2-1);
      }
      url.Delete(0, pos2+1);
    }

    // determine if the URL has a port number
    for (pos = 0; url[pos] != '\0'; pos++)
      if (reservedChars.Find(url[pos]) != P_MAX_INDEX)
        break;

    pos2 = url.Find(":");
    if (pos2 >= pos) 
      hostname = url.Left(pos);
    else {
      hostname = url.Left(pos2);
      port = (WORD)url(pos2+1, pos).AsInteger();
    }
    url.Delete(0, pos+1);
  }

  // chop off any trailing fragment
  pos = url.FindLast('#');
  if (pos != P_MAX_INDEX && pos > 0) {
    fragment = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing query
  pos = url.FindLast('?');
  if (pos != P_MAX_INDEX && pos > 0) {
    query = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
  }

  // chop off any trailing parameters
  pos = url.FindLast(';');
  if (pos != P_MAX_INDEX && pos > 0) {
    parameters = url(pos+1, P_MAX_INDEX);
    url.Delete(pos, P_MAX_INDEX);
  }

  // the hierarchy is what is left
  path = url.Tokenise('/', FALSE);
  absolutePath = path[0].IsEmpty();
  if (absolutePath)
    path.RemoveAt(0);
  if (path[path.GetSize()-1].IsEmpty())
    path.RemoveAt(path.GetSize()-1);
  for (pos = 0; pos < path.GetSize(); pos++) {
    if (pos > 0 && path[pos] == ".." && path[pos-1] != "..") {
      path.RemoveAt(pos--);
      path.RemoveAt(pos--);
    }
  }
}


PString PURL::AsString(UrlFormat fmt) const
{
  PStringStream str;

  if (fmt == FullURL) {
    if (!scheme.IsEmpty())
      str << scheme << ':';
    if (!username.IsEmpty() || !password.IsEmpty() ||
                                           !hostname.IsEmpty() || port != 80) {
      str << "//";
      if (!username.IsEmpty() || !password.IsEmpty())
        str << username << ':' << password << '@';
      if (hostname.IsEmpty())
        str << "localhost";
      else
        str << hostname;
      if (port != 80)
        str << ':' << port;
    }
  }

  PINDEX count = path.GetSize();
  if (absolutePath)
    str << '/';
  for (PINDEX i = 0; i < count; i++) {
    str << path[i];
    if (i < count-1)
      str << '/';
  }

  if (fmt == FullURL) {
    if (!parameters.IsEmpty())
      str << ";" << parameters;

    if (!query.IsEmpty())
      str << "?" << query;

    if (!fragment.IsEmpty())
      str << "#" << fragment;
  }

  return str;
}


//////////////////////////////////////////////////////////////////////////////
// PHTML

#if _MSC_VER > 800

PHTML::PHTML()
{
  memset(elementSet, 0, sizeof(elementSet));
}


PHTML::PHTML(const char * cstr)
{
  memset(elementSet, 0, sizeof(elementSet));
  *this << Title(cstr) << Body();
}


PHTML::PHTML(const PString & str)
{
  memset(elementSet, 0, sizeof(elementSet));
  *this << Title(str) << Body();
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
  : Element("HEAD", InHead, BothCRLF)
{
}

void PHTML::Head::Output(PHTML & html) const
{
  PAssert(!html.Is(InBody), "Bad HTML element");
  Element::Output(html);
}


PHTML::Body::Body()
  : Element("BODY", InBody, BothCRLF)
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
  : Element("TITLE", InTitle, CloseCRLF)
{
  titleString = NULL;
}

PHTML::Title::Title(const char * titleCStr)
  : Element("TITLE", InTitle, CloseCRLF)
{
  titleString = titleCStr;
}

PHTML::Title::Title(const PString & titleStr)
  : Element("TITLE", InTitle, CloseCRLF)
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


void PHTML::BodyElement::Output(PHTML & html) const
{
  PAssert(html.Is(InBody), "HTML element out of centext");
  Element::Output(html);
}


PHTML::Banner::Banner()
  : BodyElement("BANNER", NumElementsInSet, BothCRLF)
{
}


PHTML::ClearedElement::ClearedElement(
                             const char * n, ElementInSet elmt, OptionalCRLF c,
                             ClearCodes clear, int distance)
  : BodyElement(n, elmt, c)
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


PHTML::ComplexElement::ComplexElement(
          const char * n, ElementInSet elmt, OptionalCRLF c,
          AlignCodes align, NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ClearedElement(n, elmt, c, clear, distance)
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
  : ClearedElement("DIV", InDivision, BothCRLF, clear, distance)
{
}


PHTML::Heading::Heading(int number, int sequence, int skip,
          AlignCodes align, NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ComplexElement("H", InHeading, BothCRLF, align, noWrap, clear, distance)
{
  num = number;
  srcString = NULL;
  seqNum = sequence;
  skipSeq = skip;
}

PHTML::Heading::Heading(int number, const char * image, int sequence, int skip,
          AlignCodes align, NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ComplexElement("H", InHeading, BothCRLF, align, noWrap, clear, distance)
{
  num = number;
  srcString = image;
  seqNum = sequence;
  skipSeq = skip;
}

PHTML::Heading::Heading(int number,
          const PString & imageStr, int sequence, int skip,
          AlignCodes align, NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ComplexElement("H", InHeading, BothCRLF, align, noWrap, clear, distance)
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
  : ClearedElement("BR", NumElementsInSet, BothCRLF, clear, distance)
{
}


PHTML::Paragraph::Paragraph(AlignCodes align,
                            NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ComplexElement("P", NumElementsInSet, NoCRLF,
                   align, noWrap, clear, distance)
{
}


PHTML::PreFormat::PreFormat(int widthInChars, ClearCodes clear, int distance)
  : ClearedElement("PRE", InPreFormat, CloseCRLF, clear, distance)
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
  : BodyElement("A", NumElementsInSet, NoCRLF)
{
  hrefString = href;
}

PHTML::Anchor::Anchor(const PString & hrefStr)
  : BodyElement("A", NumElementsInSet, NoCRLF)
{
  hrefString = hrefStr;
}

void PHTML::Anchor::AddAttr(PHTML & html) const
{
  PAssert(hrefString != NULL && *hrefString != '\0', PInvalidParameter);
  html << "HREF=\"" << hrefString << '"';
}


PHTML::ImageElement::ImageElement(const char * n, ElementInSet elmt,
                          OptionalCRLF c, const char * image, AlignCodes align,
                          NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ComplexElement(n, elmt, c, align, noWrap, clear, distance)
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
  : ImageElement("IMG", NumElementsInSet, NoCRLF,
                 src, align, WrapWords, ClearDefault, 0)
{
  altString = NULL;
  width = w;
  height = h;
}

PHTML::Image::Image(const char * src,
                    const char * alt, AlignCodes align, int w, int h)
  : ImageElement("IMG", NumElementsInSet, NoCRLF,
                 src, align, WrapWords, ClearDefault, 0)
{
  altString = alt;
  width = w;
  height = h;
}

PHTML::Image::Image(const PString & srcStr,
                    const char * alt, AlignCodes align, int w, int h)
  : ImageElement("IMG", NumElementsInSet, NoCRLF,
                 srcStr, align, WrapWords, ClearDefault, 0)
{
  altString = alt;
  width = w;
  height = h;
}

PHTML::Image::Image(const PString & srcStr,
                    const PString & altStr, AlignCodes align, int w, int h)
  : ImageElement("IMG", NumElementsInSet, NoCRLF,
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
  : ImageElement("HR", NumElementsInSet, NoCRLF,
                 image, AlignDefault, WrapWords, clear, distance)
{
}

PHTML::HRule::HRule(const PString & imageStr, ClearCodes clear, int distance)
  : ImageElement("HR", InNote, CloseCRLF,
                 imageStr, AlignDefault, WrapWords, clear, distance)
{
}


PHTML::Note::Note(const char * image, ClearCodes clear, int distance)
  : ImageElement("NOTE", InNote, CloseCRLF,
                 image, AlignDefault, WrapWords, clear, distance)
{
}

PHTML::Note::Note(const PString & imageStr, ClearCodes clear, int distance)
  : ImageElement("NOTE", InNote, CloseCRLF,
                 imageStr, AlignDefault, WrapWords, clear, distance)
{
}


PHTML::Address::Address(NoWrapCodes noWrap, ClearCodes clear, int distance)
  : ComplexElement("ADDRESS", InAddress, BothCRLF,
                   AlignDefault, noWrap, clear, distance)
{
}


PHTML::BlockQuote::BlockQuote(NoWrapCodes noWrap,ClearCodes clear,int distance)
  : ComplexElement("BQ", InBlockQuote, CloseCRLF,
                   AlignDefault, noWrap, clear, distance)
{
}


PHTML::Credit::Credit()
  : BodyElement("CREDIT", NumElementsInSet, NoCRLF)
{
}

void PHTML::Credit::Output(PHTML & html) const
{
  PAssert(html.Is(InBlockQuote), "HTML element out of context");
  BodyElement::Output(html);
}


PHTML::SetTab::SetTab(const char * id)
  : BodyElement("TAB", NumElementsInSet, NoCRLF)
{
  ident = id;
}

PHTML::SetTab::SetTab(const PString & idStr)
  : BodyElement("TAB", NumElementsInSet, NoCRLF)
{
  ident = idStr;
}

void PHTML::SetTab::AddAttr(PHTML & html) const
{
  PAssert(ident != NULL && *ident != '\0', PInvalidParameter);
  html << " ID=" << ident;
}


PHTML::Tab::Tab(int indent, AlignCodes align, char decimal)
  : ComplexElement("TAB", NumElementsInSet, NoCRLF,
                   align, WrapWords, ClearDefault, 0)
{
  ident = NULL;
  indentSize = indent;
  decimalPoint = decimal;
}

PHTML::Tab::Tab(const char * id, AlignCodes align, char decimal)
  : ComplexElement("TAB", NumElementsInSet, NoCRLF,
                   align, WrapWords, ClearDefault, 0)
{
  ident = id;
  indentSize = 0;
  decimalPoint = decimal;
}

PHTML::Tab::Tab(const PString & idStr, AlignCodes align, char decimal)
  : ComplexElement("TAB", NumElementsInSet, NoCRLF,
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


PHTML::ListElement::ListElement(const char * n, ElementInSet elmt,
                           CompactCodes compact,ClearCodes clear, int distance)
  : ClearedElement(n, elmt, BothCRLF, clear, distance)
{
  compactFlag = compact == Compact;
}


void PHTML::ListElement::AddAttr(PHTML & html) const
{
  if (compactFlag)
    html << " COMPACT";
  ClearedElement::AddAttr(html);
}


PHTML::UnorderedList::UnorderedList(PlainCodes plain, ListWrapCodes wrap,
                          CompactCodes compact, ClearCodes clear, int distance)
  : ListElement("UL", InUnorderedList, compact, clear, distance)
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


PHTML::OrderedList::OrderedList(BOOL contSeq, int seqNum, CompactCodes compact, ClearCodes clear, int distance)
  : ListElement("OL", InOrderedList, compact, clear, distance)
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


PHTML::DefinitionList::DefinitionList(CompactCodes compact, ClearCodes clear, int distance)
  : ListElement("DL", InDefinitionList, compact, clear, distance)
{
}


PHTML::ListHeading::ListHeading()
  : Element("LH", InListHeading, CloseCRLF)
{
}

void PHTML::ListHeading::Output(PHTML & html) const
{
  PAssert(html.Is(InUnorderedList) ||
          html.Is(InOrderedList) ||
          html.Is(InDefinitionList),
          "HTML element out of centext");
  Element::Output(html);
}


PHTML::ListItem::ListItem(int skip, ClearCodes clear, int distance)
  : ClearedElement("LI", InListHeading, NoCRLF, clear, distance)
{
  skipSeq = skip;
}

void PHTML::ListItem::AddAttr(PHTML & html) const
{
  PAssert(html.Is(InUnorderedList) ||
          html.Is(InOrderedList),
          "HTML element out of centext");
  if (skipSeq > 0)
    html << " SKIP=" << skipSeq;
  ClearedElement::AddAttr(html);
}


PHTML::DefinitionTerm::DefinitionTerm(ClearCodes clear, int distance)
  : ClearedElement("DT", NumElementsInSet, NoCRLF, clear, distance)
{
}

void PHTML::DefinitionTerm::Output(PHTML & html) const
{
  PAssert(html.Is(InDefinitionList), "HTML element out of centext");
  PAssert(!html.Is(InDefinitionTerm), "HTML defintion item missing");
  Element::Output(html);
  html.Set(InDefinitionTerm);
}


PHTML::DefinitionItem::DefinitionItem(ClearCodes clear, int distance)
  : ClearedElement("DD", NumElementsInSet, NoCRLF, clear, distance)
{
}

void PHTML::DefinitionItem::Output(PHTML & html) const
{
  PAssert(html.Is(InDefinitionList), "HTML element out of centext");
  PAssert(html.Is(InDefinitionTerm), "HTML defintion term missing");
  Element::Output(html);
  html.Clr(InDefinitionTerm);
}


PHTML::Form::Form(const char * action,
               const char * method, const char * encoding, const char * script)
  : BodyElement("FORM", InForm, BothCRLF)
{
  actionString = action;
  methodString = method;
  encodingString = encoding;
  scriptString = script;
}

PHTML::Form::Form(const PString & action,
               const char * method, const char * encoding, const char * script)
  : BodyElement("FORM", InForm, BothCRLF)
{
  actionString = action;
  methodString = method;
  encodingString = encoding;
  scriptString = script;
}

PHTML::Form::Form(const PString & action,
            const PString & method, const char * encoding, const char * script)
  : BodyElement("FORM", InForm, BothCRLF)
{
  actionString = action;
  methodString = method;
  encodingString = encoding;
  scriptString = script;
}

PHTML::Form::Form(const PString & action,
         const PString & method, const PString & encoding, const char * script)
  : BodyElement("FORM", InForm, BothCRLF)
{
  actionString = action;
  methodString = method;
  encodingString = encoding;
  scriptString = script;
}

PHTML::Form::Form(const PString & action,
      const PString & method, const PString & encoding, const PString & script)
  : BodyElement("FORM", InForm, BothCRLF)
{
  actionString = action;
  methodString = method;
  encodingString = encoding;
  scriptString = script;
}

void PHTML::Form::AddAttr(PHTML & html) const
{
  if (actionString != NULL)
    html << " ACTION=\"" << actionString << '"';
  if (methodString != NULL)
    html << " METHOD=" << methodString;
  if (encodingString != NULL)
    html << " ENCTYPE=\"" << encodingString << '"';
  if (scriptString != NULL)
    html << " SCRIPT=\"" << scriptString << '"';
  BodyElement::AddAttr(html);
}


PHTML::FieldElement::FieldElement(
                             const char * n, ElementInSet elmt, OptionalCRLF c,
                             DisableCodes disabled, const char * error)
  : BodyElement(n, elmt, c)
{
  disabledFlag = disabled == Disabled;
  errorString = error;
}

void PHTML::FieldElement::AddAttr(PHTML & html) const
{
  PAssert(html.Is(InForm), "HTML element out of centext");
  if (disabledFlag)
    html << " DISABLED";
  if (errorString != NULL)
    html << " ERROR=\"" << errorString << '"';
  BodyElement::AddAttr(html);
}


PHTML::FormField::FormField(const char * n, ElementInSet elmt, OptionalCRLF c,
                 const char * fname, DisableCodes disabled, const char * error)
  : FieldElement(n, elmt, c, disabled, error)
{
  nameString = fname;
}

void PHTML::FormField::AddAttr(PHTML & html) const
{
  PAssert(nameString != NULL, PInvalidParameter);
  html << " NAME=\"" << nameString << '"';
  FieldElement::AddAttr(html);
}


PHTML::Select::Select(const char * fname,
                      MultipleCodes multiple,
                      DisableCodes disabled,
                      const char * error)
  : FormField("SELECT", InSelect, BothCRLF, fname, disabled, error)
{
  multipleFlag = multiple == MultipleSelect;
}

PHTML::Select::Select(const PString & fname,
                      MultipleCodes multiple,
                      DisableCodes disabled,
                      const char * error)
  : FormField("SELECT", InSelect, BothCRLF, fname, disabled, error)
{
  multipleFlag = multiple == MultipleSelect;
}

void PHTML::Select::AddAttr(PHTML & html) const
{
  if (multipleFlag)
    html << " MULTIPLE";
  FormField::AddAttr(html);
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
}


PHTML::InputText::InputText(const char * fname,
                            int size,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField("text", fname, disabled, error)
{
  width = size;
  length = maxLength;
}

PHTML::InputText::InputText(const PString & fnameStr,
                            int size,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField("text", fnameStr, disabled, error)
{
  width = size;
  length = maxLength;
}

PHTML::InputText::InputText(const char * type,
                            const char * fname,
                            int size,
                            int maxLength,
                            DisableCodes disabled,
                            const char * error)
  : InputField(type, fname, disabled, error)
{
  width = size;
  length = maxLength;
}

void PHTML::InputText::AddAttr(PHTML & html) const
{
  html << " SIZE=" << width;
  if (length > 0)
    html << " MAXLENGTH=" << length;
}


PHTML::InputPassword::InputPassword(const char * fname,
                                    int size,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * error)
  : InputText("password", fname, size, maxLength, disabled, error)
{
}

PHTML::InputPassword::InputPassword(const PString & fnameStr,
                                    int size,
                                    int maxLength,
                                    DisableCodes disabled,
                                    const char * error)
  : InputText("password", fnameStr, size, maxLength, disabled, error)
{
}


PHTML::CheckBox::CheckBox(const char * fname,
                          CheckedCodes check,
                          DisableCodes disabled,
                          const char * error)
  : InputField("checkbox", fname, disabled, error)
{
  checkedFlag = check = Checked;
}

PHTML::CheckBox::CheckBox(const PString & fnameStr,
                          CheckedCodes check,
                          DisableCodes disabled,
                          const char * error)
  : InputField("checkbox", fnameStr, disabled, error)
{
  checkedFlag = check = Checked;
}

PHTML::CheckBox::CheckBox(const char * type,
                          const char * fname,
                          CheckedCodes check,
                          DisableCodes disabled,
                          const char * error)
  : InputField(type, fname, disabled, error)
{
  checkedFlag = check = Checked;
}

void PHTML::CheckBox::AddAttr(PHTML & html) const
{
  if (checkedFlag)
    html << " CHECKED";
}


PHTML::RadioButton::RadioButton(const char * fname,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * error)
  : CheckBox("radio", fname, check, disabled, error)
{
}

PHTML::RadioButton::RadioButton(const PString & fnameStr,
                                CheckedCodes check,
                                DisableCodes disabled,
                                const char * error)
  : CheckBox("radio", fnameStr, check, disabled, error)
{
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
  html << " MIN=" << minValue << " MAX=" << maxValue << " VALUE=" << initValue;
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


PHTML::ResetButton::ResetButton(const char * fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, src, disabled, error)
{
}

PHTML::ResetButton::ResetButton(const PString & fname,
                                const char * src,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, src, disabled, error)
{
}

PHTML::ResetButton::ResetButton(const char * fname,
                                const PString & srcStr,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, srcStr, disabled, error)
{
}

PHTML::ResetButton::ResetButton(const PString & fname,
                                const PString & srcStr,
                                DisableCodes disabled,
                                const char * error)
  : InputImage("reset", fname, srcStr, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const char * fname,
                                  const char * src,
                                  DisableCodes disabled,
                                  const char * error)
  : InputImage("submit", fname, src, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const PString & fname,
                                  const char * src,
                                  DisableCodes disabled,
                                  const char * error)
  : InputImage("submit", fname, src, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const char * fname,
                                  const PString & srcStr,
                                  DisableCodes disabled,
                                  const char * error)
  : InputImage("submit", fname, srcStr, disabled, error)
{
}

PHTML::SubmitButton::SubmitButton(const PString & fname,
                                  const PString & srcStr,
                                  DisableCodes disabled,
                                  const char * error)
  : InputImage("submit", fname, srcStr, disabled, error)
{
}



#endif


//////////////////////////////////////////////////////////////////////////////
// PHTTPSpace

PHTTPSpace::PHTTPSpace()
{
  resource = NULL;
}


PHTTPSpace::PHTTPSpace(const PString & nam)
  : name(nam)
{
  resource = NULL;
}


PHTTPSpace::~PHTTPSpace()
{
  delete resource;
}


PObject::Comparison PHTTPSpace::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PHTTPSpace::Class()), PInvalidCast);
  return name.Compare(((const PHTTPSpace &)obj).name);
}


BOOL PHTTPSpace::AddResource(PHTTPResource * res)
{
  PAssert(res != NULL, PInvalidParameter);
  const PStringArray & path = res->GetURL().GetPath();
  PHTTPSpace * node = this;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      pos = node->children.Append(PNEW PHTTPSpace(path[i]));
    node = &children[pos];
    if (node->resource != NULL)
      return FALSE;   // Already a resource in tree in partial path
  }
  if (!node->children.IsEmpty())
    return FALSE;   // Already a resource in tree further down path.
  node->resource = res;
  return TRUE;
}


PHTTPResource * PHTTPSpace::FindResource(const PURL & url)
{
  const PStringArray & path = url.GetPath();
  PHTTPSpace * node = this;
  for (PINDEX i = 0; i < path.GetSize(); i++) {
    PINDEX pos = node->children.GetValuesIndex(PHTTPSpace(path[i]));
    if (pos == P_MAX_INDEX)
      return NULL;
    node = &children[pos];
    if (node->resource != NULL)
      return node->resource;
  }
  return NULL;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSocket

static char const * HTTPCommands[PHTTPSocket::NumCommands] = {
  "GET", "HEAD", "POST"
};

static const PString ContentLengthStr = "Content-Length";
static const PString ContentTypeStr = "Content-Type";


PHTTPSocket::PHTTPSocket(WORD port)
  : PApplicationSocket(NumCommands, HTTPCommands, port)
{
}


PHTTPSocket::PHTTPSocket(const PString & address, WORD port)
  : PApplicationSocket(NumCommands, HTTPCommands, address, port)
{
}


PHTTPSocket::PHTTPSocket(const PString & address, const PString & service)
  : PApplicationSocket(NumCommands, HTTPCommands, address, service)
{
}


PHTTPSocket::PHTTPSocket(PSocket & socket)
  : PApplicationSocket(NumCommands, HTTPCommands, socket)
{
}


PHTTPSocket::PHTTPSocket(PSocket & socket, const PHTTPSpace & space)
  : PApplicationSocket(NumCommands, HTTPCommands, socket),
    urlSpace(space)
{
}


BOOL PHTTPSocket::GetDocument(const PString & url)
{
  WriteCommand(GET, url);
  return FALSE;
}


BOOL PHTTPSocket::GetHeader(const PString & url)
{
  WriteCommand(HEAD, url);
  return FALSE;
}


BOOL PHTTPSocket::PostData(const PString & url, const PStringToString & data)
{
  WriteCommand(POST, url);
  for (PINDEX i = 0; i < data.GetSize(); i++)
    WriteString(data.GetKeyAt(i) + " = " + data.GetDataAt(i));
  return FALSE;
}


BOOL PHTTPSocket::ProcessCommand()
{
  PString args;
  PINDEX cmd = ReadCommand(args);
  if (cmd == P_MAX_INDEX)   // Unknown command
    return OnUnknown(args);

  PStringArray tokens = args.Tokenise(" \t", FALSE);

  // if no tokens, error
  if (tokens.IsEmpty()) {
    OnError(BadRequest, args);
    return FALSE;
  }

  PURL url = tokens[0];

  // if only one argument, then it must be a version 0.9 simple request
  if (tokens.GetSize() == 1) {
    majorVersion = 0;
    minorVersion = 9;
  }
  else { // otherwise, attempt to extract a version number
    PString verStr = tokens[1];
    PINDEX dotPos = verStr.Find('.');
    if (dotPos == P_MAX_INDEX
                      || verStr.GetLength() < 8 || verStr.Left(5) != "HTTP/") {
      OnError(BadRequest, "Malformed version number " + verStr);
      return FALSE;
    }

    // should actually check if the text contains only digits, but the
    // chances of matching everything else and it not being a valid number
    // are pretty small, so don't bother
    majorVersion = (int)verStr(5, dotPos-1).AsInteger();
    minorVersion = (int)verStr(dotPos+1, P_MAX_INDEX).AsInteger();
  }

  // If the protocol is version 1.0 or greater, there is MIME info and the
  // prescence of a an entity body is signalled by the inclusion of
  // Content-Length header. If the protocol is less than version 1.0, then the
  // entity body is all remaining bytes until EOF
  PMIMEInfo mimeInfo;
  PString entityBody;
  if (majorVersion >= 1) {
    // at this stage we should be ready to collect the MIME info
    // until an empty line is received, or EOF
    mimeInfo.Read(*this);

    // if there was a Content-Length header, then it gives the exact
    // length of the entity body. Otherwise, read the entity-body until EOF
    long contentLength = mimeInfo.GetInteger(ContentLengthStr, 0);
    if (contentLength > 0) {
      entityBody = ReadString((PINDEX)contentLength);
      if (GetLastReadCount() != contentLength) {
        OnError(BadRequest, "incomplete entity-body received");
        return FALSE;
      }
    }
  }
  else {
    int count = 0;
    while (Read(entityBody.GetPointer(count+100)+count, 100))
      count += GetLastReadCount();
  }

  switch (cmd) {
    case GET :
      OnGET(url, mimeInfo);
      break;

    case HEAD :
      OnHEAD(url, mimeInfo);
      break;

    case POST :
      PStringToString postData;
      // break the string into string/value pairs separated by &
      PStringArray tokens = entityBody.Tokenise("&=", TRUE);
      for (PINDEX i = 0; i < tokens.GetSize(); i += 2) {
        PCaselessString key = tokens[i];
        if (postData.GetAt(key) != NULL) 
          postData.SetAt(key, postData[key] + "," + tokens[i+1]);
        else
          postData.SetAt(key, tokens[i+1]);
      }
      OnPOST(url, mimeInfo, postData);
      break;
  }

  return TRUE;
}


PString PHTTPSocket::GetServerName() const
{
  return "PWLib-HTTP-Server/1.0 PWLib/1.0";
}


void PHTTPSocket::OnGET(const PURL & url, const PMIMEInfo & info)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnGET(*this, url, info);
}


void PHTTPSocket::OnHEAD(const PURL & url, const PMIMEInfo & info)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnHEAD(*this, url, info);
}


void PHTTPSocket::OnPOST(const PURL & url,
                         const PMIMEInfo & info,
                         const PStringToString & data)
{
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL)
    OnError(NotFound, url.AsString());
  else
    resource->OnPOST(*this, url, info, data);
}


BOOL PHTTPSocket::OnUnknown(const PCaselessString & command)
{
  OnError(BadRequest, command);
  return FALSE;
}


static struct httpStatusCodeStruct {
  char *  text;
  int     code;
  BOOL    allowedBody;
} httpStatusDefn[PHTTPSocket::NumStatusCodes] = {
  { "Information",           100, 0 },
  { "OK",                    200, 1 },
  { "Created",               201, 1 },
  { "Accepted",              202, 1 },
  { "No Content",            204, 0 },
  { "Moved Permanently",     301, 1 },
  { "Moved Temporarily",     302, 1 },
  { "Not Modified",          304, 0 },
  { "Bad Request",           400, 1 },
  { "Unauthorised",          401, 0 },
  { "Forbidden",             403, 1 },
  { "Not Found",             404, 1 },
  { "Internal Server Error", 500, 1 },
  { "Not Implemented",       501, 1 },
  { "Bad Gateway",           502, 1 },
};

void PHTTPSocket::StartResponse(StatusCode code,
                                PMIMEInfo & headers,
                                PINDEX bodySize)
{
  if (majorVersion < 1)
    return;

  httpStatusCodeStruct * statusInfo = httpStatusDefn+code;
  *this << "HTTP/" << majorVersion << '.' << minorVersion << ' '
        << statusInfo->code <<  statusInfo->text << "\r\n";

  PTime now;
  headers.SetAt("Date", now.AsString("www, dd MMM yyyy hh:mm:ssg")+" GMT");
  headers.SetAt("MIME-Version", "1.0");
  headers.SetAt("Server", GetServerName());
  headers.SetAt(ContentLengthStr, PString(PString::Unsigned, bodySize));
  headers.Write(*this);
}


void PHTTPSocket::OnError(StatusCode code, const PString & extra)
{
  httpStatusCodeStruct * statusInfo = httpStatusDefn+code;

  PMIMEInfo headers;

  if (!statusInfo->allowedBody) {
    StartResponse(code, headers, 0);
    return;
  }

#if !defined(_MSC_VER) || _MSC_VER > 800
  PHTML reply;
  reply << PHTML::Title()
        << statusInfo->code
        << " "
        << statusInfo->text
        << PHTML::Body()
        << PHTML::Heading(1)
        << statusInfo->code
        << " "
        << statusInfo->text
        << PHTML::Heading(1)
        << extra
        << PHTML::Body();
#else
  PStringStream reply;
  reply << "<TITLE>"
        << statusInfo->code
        << " "
        << statusInfo->text
        << "</TITLE><BODY><H1>"
        << statusInfo->code
        << " "
        << statusInfo->text
        << "</H1>"
        << extra
        << "</BODY>";
#endif

  headers.SetAt(ContentTypeStr, "text/html");
  StartResponse(code, headers, reply.GetLength());
  WriteString(reply);
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPSimpleAuth

PHTTPSimpleAuth::PHTTPSimpleAuth(const PString & realm_,
                                 const PString & username_,
                                 const PString & password_)
  : realm(realm_), username(username_), password(password_)
{
  PAssert(!realm.IsEmpty(), "Must have a realm!");
}


PObject * PHTTPSimpleAuth::Clone() const
{
  return PNEW PHTTPSimpleAuth(realm, username, password);
}


PString PHTTPSimpleAuth::GetRealm() const
{
  return realm;
}


BOOL PHTTPSimpleAuth::Validate(const PString & authInfo) const
{
  PString decoded;
  if (authInfo(0, 5) == PCaselessString("Basic "))
    decoded = PBase64::Decode(authInfo(6, P_MAX_INDEX));
  else
    decoded = PBase64::Decode(authInfo);

  PINDEX colonPos = decoded.Find(':');
  if (colonPos == P_MAX_INDEX) 
    return FALSE;

  return username == decoded.Left(colonPos).Trim() &&
         password == decoded(colonPos+1, P_MAX_INDEX).Trim();
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPResource

PHTTPResource::PHTTPResource(const PURL & url, const PString & type)
  : baseURL(url), contentType(type)
{
  authority = NULL;
}


PHTTPResource::PHTTPResource(const PURL & url,
                             const PString & type,
                             const PHTTPAuthority & auth)
  : baseURL(url), contentType(type)
{
  authority = (PHTTPAuthority *)auth.Clone();
}


PHTTPResource::~PHTTPResource()
{
  delete authority;
}


void PHTTPResource::OnGET(PHTTPSocket & socket,
                          const PURL & url,
                          const PMIMEInfo & info)
{
  if (CheckAuthority(socket, info)) {
    PCharArray data;
    PMIMEInfo outMIME;
    outMIME.SetAt(ContentTypeStr, contentType);
    PHTTPSocket::StatusCode code = OnLoadData(url, info, data, outMIME);
    if (code != PHTTPSocket::OK)
      socket.OnError(code, url.AsString());
    else {
      socket.StartResponse(code, outMIME, data.GetSize());
      socket.Write(data, data.GetSize());
    }
  }
}


void PHTTPResource::OnHEAD(PHTTPSocket & socket,
                                const PURL & url,
                                const PMIMEInfo & info)
{
  if (CheckAuthority(socket, info)) {
    PMIMEInfo reply;
    PCharArray data;
    PHTTPSocket::StatusCode code = OnLoadHead(url, info, data, reply);
    if (code != PHTTPSocket::OK)
      socket.OnError(code, url.AsString());
    else {
      socket.StartResponse(code, reply, data.GetSize());
      socket.Write(data, data.GetSize());
    }
  }
}


void PHTTPResource::OnPOST(PHTTPSocket & socket,
                                const PURL & url,
                                const PMIMEInfo & info,
                                const PStringToString & data)
{
  if (CheckAuthority(socket, info)) {
    Post(url, info, data);
    socket.OnError(PHTTPSocket::OK, "");
  }
}


BOOL PHTTPResource::CheckAuthority(PHTTPSocket & socket,
                                   const PMIMEInfo & info)
{
  if (authority == NULL)
    return TRUE;

  // if this is an authorisation request...
  PString authInfo = info.GetString("Authorization", "");
  if (authInfo.IsEmpty()) {
    // it must be a request for authorisation
    PMIMEInfo reply;
    reply.SetAt("WWW-Authenticate",
                              "Basic realm=\"" + authority->GetRealm() + "\"");
    socket.StartResponse(PHTTPSocket::UnAuthorised, reply, 0);
    return FALSE;
  }

  if (authority->Validate(authInfo))
    return TRUE;

  socket.OnError(PHTTPSocket::Forbidden, "");
  return FALSE;
}


PHTTPSocket::StatusCode PHTTPResource::OnLoadHead(const PURL & url,
                                                  const PMIMEInfo & inMIME,
                                                  PCharArray & data,
                                                  PMIMEInfo & outMIME)
{
  return OnLoadData(url, inMIME, data, outMIME);
}


PHTTPSocket::StatusCode PHTTPResource::Post(const PURL &,
                                            const PMIMEInfo &,
                                            const PStringToString &)
{
  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPString

PHTTPString::PHTTPString(const PURL & url, const PString & str)
  : PHTTPResource(url, "text/html"), string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PString & type)
  : PHTTPResource(url, type), string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, "text/html", auth), string(str)
{
}


PHTTPString::PHTTPString(const PURL & url,
                         const PString & str,
                         const PString & type,
                         const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth), string(str)
{
}


PHTTPSocket::StatusCode PHTTPString::OnLoadData(const PURL &,
                                                const PMIMEInfo &,
                                                PCharArray & data,
                                                PMIMEInfo &)
{
  data = string;
  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPFile

PHTTPFile::PHTTPFile(const PURL & url, const PFilePath & file)
  : PHTTPResource(url, PMIMEInfo::GetContentType(file.GetType())),
    filePath(file)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & file,
                     const PString & type)
  : PHTTPResource(url, type), filePath(file)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & file,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, PMIMEInfo::GetContentType(file.GetType()), auth),
    filePath(file)
{
}


PHTTPFile::PHTTPFile(const PURL & url,
                     const PFilePath & file,
                     const PString & type,
                     const PHTTPAuthority & auth)
  : PHTTPResource(url, type, auth), filePath(file)
{
}


PHTTPSocket::StatusCode PHTTPFile::OnLoadData(const PURL & url,
                                              const PMIMEInfo &,
                                              PCharArray & data,
                                              PMIMEInfo &)
{
  if (baseURL != url)
    return PHTTPSocket::NotFound;

  PFile file;
  if (!file.Open(filePath, PFile::ReadOnly))
    return PHTTPSocket::NotFound;

  PINDEX count = (PINDEX)file.GetLength();
  file.Read(data.GetPointer(count), count);

  return PHTTPSocket::OK;
}


//////////////////////////////////////////////////////////////////////////////
// PHTTPDirectory

PHTTPDirectory::PHTTPDirectory(const PURL & url, const PDirectory & dir)
  : PHTTPResource(url, PString()), basePath(dir)
{
}


PHTTPDirectory::PHTTPDirectory(const PURL & url,
                               const PDirectory & dir,
                               const PHTTPAuthority & auth)
  : PHTTPResource(url, PString(), auth), basePath(dir)
{
}


PHTTPSocket::StatusCode PHTTPDirectory::OnLoadData(const PURL & url,
                                                   const PMIMEInfo &,
                                                   PCharArray & data,
                                                   PMIMEInfo & outMIME)
{
  const PStringArray & path = url.GetPath();
  PFilePath realPath = basePath;
  for (PINDEX i = baseURL.GetPath().GetSize(); i < path.GetSize()-1; i++)
    realPath += path[i] + PDIR_SEPARATOR;

  if (i < path.GetSize())
    realPath += path[i];

  // See if its a normal file
  PFileInfo info;
  if (!PFile::GetInfo(realPath, info))
    return PHTTPSocket::NotFound;

  // Noew try and open it
  PFile file;
  if (info.type != PFileInfo::SubDirectory) {
    if (!file.Open(realPath, PFile::ReadOnly))
      return PHTTPSocket::NotFound;
  }
  else {
    static const char * const IndexFiles[] = {
      "Welcome.html", "welcome.html", "index.html",
      "Welcome.htm",  "welcome.htm",  "index.htm"
    };
    for (i = 0; i < PARRAYSIZE(IndexFiles); i++)
      if (file.Open(realPath + PDIR_SEPARATOR + IndexFiles[i], PFile::ReadOnly))
        break;
  }

  if (file.IsOpen()) {
    PINDEX count = (PINDEX)file.GetLength();
    file.Read(data.GetPointer(count), count);
    outMIME.SetAt(ContentTypeStr,
                      PMIMEInfo::GetContentType(file.GetFilePath().GetType()));

  }
#if _MSC_VER > 800
  else {
    outMIME.SetAt(ContentTypeStr, "text/html");
    PHTML reply = "Directory of " + url.AsString();
    reply << PHTML::Heading(1)
          << "Directory of " << url
          << PHTML::Heading(1);
    PDirectory dir = realPath;
    if (dir.Open()) {
      do {
        const char * imgName;
        if (dir.IsSubDir())
          imgName = "internal-gopher-menu";
        else if (PMIMEInfo::GetContentType(
                      PFilePath(dir.GetEntryName()).GetType())(0,4) == "text/")
          imgName = "internal-gopher-text";
        else
          imgName = "internal-gopher-unknown";
        reply << PHTML::Image(imgName) << ' '
              << PHTML::Anchor(realPath.GetFileName()+'/'+dir.GetEntryName())
              << dir.GetEntryName()
              << PHTML::Anchor()
              << PHTML::BreakLine();
      } while (dir.Next());
    }
    reply << PHTML::Body();
    data = reply;
  }
#endif

  return PHTTPSocket::OK;
}



// End Of File ///////////////////////////////////////////////////////////////
