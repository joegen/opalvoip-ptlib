/*
 * $Id: html.h,v 1.1 1996/01/24 23:45:37 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: html.h,v $
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

    void SetHeading1();
    class Heading1 {
      public:
        Heading1()
          { }
        friend ostream & operator<<(ostream & html, const Heading1 &)
          { ((PHTML&)html).SetHeading1(); return html; }
    };

    void SetAnchor(const char * href, const char * text);
    class Anchor {
      public:
        Anchor(const char * href = NULL, const char * text = NULL)
          { hrefString = href; textString = text; }
        friend ostream & operator<<(ostream & html, const Anchor & elmt)
          { ((PHTML&)html).SetAnchor(elmt.hrefString, elmt.textString); return html; }
      private:
        const char * hrefString;
        const char * textString;
    };

    class BreakLine {
      public:
        BreakLine()
          { }
        friend ostream & operator<<(ostream & html, const BreakLine &)
          { return html << "<BL>\r\n"; }
    };

  private:
    enum ElementInSet {
      InHead,
      InBody,
      InTitle,
      InHeading1,
      InAnchor,
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
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
