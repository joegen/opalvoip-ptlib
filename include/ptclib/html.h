/*
 * $Id: html.h,v 1.2 1996/01/26 02:24:24 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: html.h,v $
 * Revision 1.2  1996/01/26 02:24:24  robertj
 * Further implemetation.
 *
 * Revision 1.1  1996/01/24 23:45:37  robertj
 * Initial revision
 *
 */

#ifndef _PHTML
#define _PHTML

#ifdef __GNUC__
#pragma interface
#endif



//////////////////////////////////////////////////////////////////////////////
// PHTML

PDECLARE_CLASS(PHTML, PStringStream)
/* This class describes a HyperText markup Language string as used by the
   World Wide Web and the <A>PURL</A> and <A>PHTTPSocket</A> class.
   
   All of the standard stream I/O operators, manipulators etc will operate on
   the PString class.
 */

  public:
    PHTML();
    PHTML(
      const char * cstr     // C string representation of the title string.
    );
    PHTML(
      const PString & str   // String representation of the title string.
    );
    /* Construct a new HTML object.
     */

    ~PHTML();

  // New functions for class.
    void SetHead();
    class Head {
      public:
        Head()
          { }
        friend ostream & operator<<(ostream & html, const Head &)
          { ((PHTML&)html).SetHead(); return html; }
    };

    void SetBody();
    class Body {
      public:
        Body()
          { }
        friend ostream & operator<<(ostream & html, const Body &)
          { ((PHTML&)html).SetBody(); return html; }
    };

    void SetTitle(const char * title);
    class Title {
      public:
        Title()
          { titleString = NULL; }
        Title(const char * title)
          { titleString = title; }
        Title(const PString & title)
          { titleString = title; }
        friend ostream & operator<<(ostream & html, const Title & elmt)
          { ((PHTML&)html).SetTitle(elmt.titleString); return html; }
      private:
        const char * titleString;
    };

    void SetHeading(int num);
    class Heading {
      public:
        Heading(int num)
          { number = num; }
        friend ostream & operator<<(ostream & html, const Heading & elmt)
          { ((PHTML&)html).SetHeading(elmt.number); return html; }
      private:
        int number;
    };

    enum ClearCodes {
      ClearDefault, ClearLeft, ClearRight, ClearAll, ClearEn, ClearPixels
    };
    void SetLineBreak(ClearCodes clear, int distance = 1);
    class BreakLine {
      public:
        BreakLine(ClearCodes clear = ClearDefault, int distance = 1)
          { clearCode = clear; clearDistance = distance; }
        friend ostream & operator<<(ostream & html, const BreakLine & elmt)
          { ((PHTML&)html).SetLineBreak(elmt.clearCode, elmt.clearDistance); return html; }
      private:
        ClearCodes clearCode;
        int clearDistance;
    };

    enum AlignCodes {
      AlignDefault, AlignLeft, AlignCentre, AlignCenter = AlignCentre, AlignRight, AlignJustify, AlignTop, AlignBottom,
      NumAlignCodes
    };
    void SetParagraph(AlignCodes align, BOOL noWrap, ClearCodes clear, int distance = 1);
    class Paragraph {
      public:
        Paragraph(AlignCodes align = AlignDefault, BOOL noWrap = FALSE, ClearCodes clear = ClearDefault, int distance = 1)
          { alignCode = align; noWrapFlag = noWrap; clearCode = clear; clearDistance = distance; }
        friend ostream & operator<<(ostream & html, const Paragraph & elmt)
          { ((PHTML&)html).SetParagraph(elmt.alignCode, elmt.noWrapFlag, elmt.clearCode, elmt.clearDistance); return html; }
      private:
        AlignCodes alignCode;
        BOOL noWrapFlag;
        ClearCodes clearCode;
        int clearDistance;
    };

    void SetAnchor(const char * href, const char * text = NULL);
    class Anchor {
      public:
        Anchor(const char * href = NULL, const char * text = NULL)
          { hrefString = href; textString = text; }
        Anchor(const PString & hrefStr, const char * text = NULL)
          { hrefString = hrefStr; textString = text; }
        Anchor(const PString & hrefStr, const PString & textStr)
          { hrefString = hrefStr; textString = textStr; }
        friend ostream & operator<<(ostream & html, const Anchor & elmt)
          { ((PHTML&)html).SetAnchor(elmt.hrefString, elmt.textString); return html; }
      private:
        const char * hrefString;
        const char * textString;
    };

    void SetImage(const char * src, const char * alt = NULL, AlignCodes align = AlignDefault, int width = 0, int height = 0);
    class Image {
      public:
        Image(const char * src, const char * alt = NULL, AlignCodes align = AlignDefault, int w = 0, int h = 0)
          { srcString = src; altString = alt; alignCode = align; width = w; height = h; }
        Image(const PString & srcStr, const char * alt = NULL, AlignCodes align = AlignDefault, int w = 0, int h = 0)
          { srcString = srcStr; altString = alt; alignCode = align; width = w; height = h; }
        Image(const PString & srcStr, const PString & altStr, AlignCodes align = AlignDefault, int w = 0, int h = 0)
          { srcString = srcStr; altString = altStr; alignCode = align; width = w; height = h; }
        friend ostream & operator<<(ostream & html, const Image & elmt)
          { ((PHTML&)html).SetImage(elmt.srcString, elmt.altString, elmt.alignCode, elmt.width, elmt.height); return html; }
      private:
        const char * srcString;
        const char * altString;
        AlignCodes alignCode;
        int width, height;
    };


#define SIMPLE_ELEMENT(elmt, str) \
    class elmt { \
      public: \
        elmt() { } \
        friend ostream & operator<<(ostream & html, const elmt &) \
          { ((PHTML&)html).SetSimpleElement(str, In##elmt); return html; } \
    }; \
    friend ostream & operator<<(ostream & html, const elmt &)
    SIMPLE_ELEMENT(Bold, "B");
    SIMPLE_ELEMENT(Italic, "I");
    SIMPLE_ELEMENT(TeleType, "TT");
    SIMPLE_ELEMENT(Underline, "U");
    SIMPLE_ELEMENT(StrikeThrough, "S");
    SIMPLE_ELEMENT(Big, "BIG");
    SIMPLE_ELEMENT(Small, "SMALL");
    SIMPLE_ELEMENT(Subscript, "SUB");
    SIMPLE_ELEMENT(Superscript, "SUP");
    SIMPLE_ELEMENT(Emphasis, "EM");
    SIMPLE_ELEMENT(Cite, "CITE");
    SIMPLE_ELEMENT(Strong, "STRONG");
    SIMPLE_ELEMENT(Code, "CODE");
    SIMPLE_ELEMENT(Sample, "SAMP");
    SIMPLE_ELEMENT(Keyboard, "KBD");
    SIMPLE_ELEMENT(Variable, "VAR");
    SIMPLE_ELEMENT(Definition, "DFN");
    SIMPLE_ELEMENT(Quote, "Q");
    SIMPLE_ELEMENT(Author, "AU");
    SIMPLE_ELEMENT(Person, "PERSON");
    SIMPLE_ELEMENT(Acronym, "ACRONYM");
    SIMPLE_ELEMENT(Abbrev, "ABBREV");
    SIMPLE_ELEMENT(InsertedText, "INS");
    SIMPLE_ELEMENT(DeletedText, "DEL");
#undef SIMPLE_ELEMENT

  private:
    enum ElementInSet {
      InHead,
      InBody,
      InTitle,
      InHeading,
      InAnchor,
      InBold,
      InItalic,
      InTeleType,
      InUnderline,
      InStrikeThrough,
      InBig,
      InSmall,
      InSubscript,
      InSuperscript,
      InEmphasis,
      InCite,
      InStrong,
      InCode,
      InSample,
      InKeyboard,
      InVariable,
      InDefinition,
      InQuote,
      InAuthor,
      InPerson,
      InAcronym,
      InAbbrev,
      InInsertedText,
      InDeletedText,
      NumElementsInSet
    };
    BOOL Is(ElementInSet elmt)
      { return (elementSet[elmt>>3]&(1<<(elmt&7))) != 0; }
    void Set(ElementInSet elmt)
      { elementSet[elmt>>3] |= (1<<(elmt&7)); }
    void Not(ElementInSet elmt)
      { elementSet[elmt>>3] &= ~(1<<(elmt&7)); }
    void Toggle(ElementInSet elmt)
      { elementSet[elmt>>3] ^= (1<<(elmt&7)); }
    BYTE elementSet[(NumElementsInSet+7)/8];

    void SetSimpleElement(const char * name, ElementInSet elmt);
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
