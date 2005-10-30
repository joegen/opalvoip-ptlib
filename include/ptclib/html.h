/*
 * html.h
 *
 * HyperText Markup Language stream classes.
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
 *
 * $Log: html.h,v $
 * Revision 1.25  2005/10/30 19:41:53  dominance
 * fixed most of the warnings occuring during compilation
 *
 * Revision 1.24  2005/03/19 02:52:53  csoutheren
 * Fix warnings from gcc 4.1-20050313 shapshot
 *
 * Revision 1.23  2002/11/06 22:47:23  robertj
 * Fixed header comment (copyright etc)
 *
 * Revision 1.22  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.21  2001/02/13 04:39:08  robertj
 * Fixed problem with operator= in container classes. Some containers will
 *   break unless the copy is virtual (eg PStringStream's buffer pointers) so
 *   needed to add a new AssignContents() function to all containers.
 *
 * Revision 1.20  1999/03/09 08:01:46  robertj
 * Changed comments for doc++ support (more to come).
 *
 * Revision 1.19  1999/02/16 08:07:10  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.18  1998/09/23 06:19:27  robertj
 * Added open source copyright license.
 *
 * Revision 1.17  1997/07/08 13:15:31  robertj
 * DLL support.
 *
 * Revision 1.16  1997/06/16 13:18:02  robertj
 * Set Is() function to be const as it should have been.
 *
 * Revision 1.15  1996/08/17 10:00:18  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.14  1996/06/28 13:08:41  robertj
 * Changed PHTML class so can create html fragments.
 * Fixed nesting problem in tables.
 *
 * Revision 1.13  1996/06/01 04:18:40  robertj
 * Fixed bug in RadioButton, having 2 VALUE fields
 *
 * Revision 1.12  1996/04/14 02:52:02  robertj
 * Added hidden fields to HTML.
 *
 * Revision 1.11  1996/03/12 11:30:00  robertj
 * Fixed resetting of HTML output using operator=.
 *
 * Revision 1.10  1996/03/10 13:14:53  robertj
 * Simplified some of the classes and added catch all string for attributes.
 *
 * Revision 1.9  1996/03/03 07:36:44  robertj
 * Added missing public's to standard character attribute classes.
 *
 * Revision 1.8  1996/02/25 11:14:19  robertj
 * Radio button support for forms.
 *
 * Revision 1.7  1996/02/19 13:18:25  robertj
 * Removed MSC_VER test as now completely removed from WIN16 library.
 *
 * Revision 1.6  1996/02/08 11:50:38  robertj
 * More implementation.
 *
 * Revision 1.5  1996/02/03 11:01:25  robertj
 * Further implementation.
 *
 * Revision 1.4  1996/01/28 14:15:56  robertj
 * More comments.
 *
 * Revision 1.3  1996/01/28 02:45:38  robertj
 * Further implementation.
 *
 * Revision 1.2  1996/01/26 02:24:24  robertj
 * Further implemetation.
 *
 * Revision 1.1  1996/01/24 23:45:37  robertj
 * Initial revision
 *
 */

#ifndef _PHTML
#define _PHTML

#ifdef P_USE_PRAGMA
#pragma interface
#endif



//////////////////////////////////////////////////////////////////////////////
// PHTML

/** This class describes a HyperText markup Language string as used by the
   World Wide Web and the #PURL# and #PHTTPSocket# class.
   
   All of the standard stream I/O operators, manipulators etc will operate on
   the PString class.
 */
class PHTML : public PStringStream
{
  PCLASSINFO(PHTML, PStringStream)

  public:
    enum ElementInSet {
      InHTML,
      InHead,
      InBody,
      InTitle,
      InHeading,
      InDivision,
      InPreFormat,
      InAnchor,
      InNote,
      InAddress,
      InBlockQuote,
      InCredit,
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
      InList,
      InListHeading,
      InDefinitionTerm,
      InTable,
      InForm,
      InSelect,
      InTextArea,
      NumElementsInSet
    };

    /** Construct a new HTML object. If a title is specified in the
       constructor then the HEAD, TITLE and BODY elements are output and the
       string is used in a H1 element.
     */
    PHTML(
      ElementInSet initialState = NumElementsInSet
    );
    PHTML(
      const char * cstr     // C string representation of the title string.
    );
    PHTML(
      const PString & str   // String representation of the title string.
    );

    ~PHTML();

    /** Restart the HTML string output using the specified value as the
       new title. If <CODE>title</CODE> is empty then no HEAD or TITLE
       elements are placed into the HTML.
     */
    PHTML & operator=(
      const PHTML & html     // HTML stream to make a copy of.
    ) { AssignContents(html); return *this; }
    PHTML & operator=(
      const PString & str    // String for title in restating HTML.
    ) { AssignContents(str); return *this; }
    PHTML & operator=(
      const char * cstr    // String for title in restating HTML.
    ) { AssignContents(PString(cstr)); return *this; }
    PHTML & operator=(
      char ch    // String for title in restating HTML.
    ) { AssignContents(PString(ch)); return *this; }


  // New functions for class.
    BOOL Is(ElementInSet elmt) const;
    void Set(ElementInSet elmt);
    void Clr(ElementInSet elmt);
    void Toggle(ElementInSet elmt);


    class Element {
      public: virtual ~Element() throw () {}
      protected:
        enum OptionalCRLF { NoCRLF, OpenCRLF, CloseCRLF, BothCRLF };
        Element(
          const char * nam,
          const char * att,
          ElementInSet elmt,
          ElementInSet req,
          OptionalCRLF opt
        ) { name = nam; attr= att; inElement = elmt; reqElement = req; crlf = opt; }
        virtual ~Element() { }
        virtual void Output(PHTML & html) const;
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * name;
        const char * attr;
        ElementInSet inElement;
        ElementInSet reqElement;
        OptionalCRLF crlf;
      friend ostream & operator<<(ostream & strm, const Element & elmt)
        { elmt.Output((PHTML&)strm); return strm; }
    };

    class HTML : public Element {
      public: virtual ~HTML() throw () {}
      public:
        HTML(const char * attr = NULL);
    };

    class Head : public Element {
      public: virtual ~Head() throw () {}
      public:
        Head();
      protected:
        virtual void Output(PHTML & html) const;
    };

    class Body : public Element {
      public: virtual ~Body() throw () {}
      public:
        Body(const char * attr = NULL);
      protected:
        virtual void Output(PHTML & html) const;
    };

    class Title : public Element {
      public: virtual ~Title() throw () {}
      public:
        Title();
        Title(const char * titleCStr);
        Title(const PString & titleStr);
      protected:
        virtual void Output(PHTML & html) const;
      private:
        const char * titleString;
    };

    class Banner : public Element {
      public: virtual ~Banner() throw () {}
      public:
        Banner(const char * attr = NULL);
    };

    class Division : public Element {
      public: virtual ~Division() throw () {}
      public:
        Division(const char * attr = NULL);
    };

    class Heading : public Element {
      public: virtual ~Heading() throw () {}
      public:
        Heading(int number,
                int sequence = 0,
                int skip = 0,
                const char * attr = NULL);
        Heading(int number,
                const char * image,
                int sequence = 0,
                int skip = 0,
                const char * attr = NULL);
        Heading(int number,
                const PString & imageStr,
                int sequence = 0,
                int skip = 0,
                const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int num;
        const char * srcString;
        int seqNum, skipSeq;
    };

    class BreakLine : public Element {
      public: virtual ~BreakLine() throw () {}
      public:
        BreakLine(const char * attr = NULL);
    };

    class Paragraph : public Element {
      public: virtual ~Paragraph() throw () {}
      public:
        Paragraph(const char * attr = NULL);
    };

    class PreFormat : public Element {
      public: virtual ~PreFormat() throw () {}
      public:
        PreFormat(int widthInChars = 0,
                  const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int width;
    };

    class HotLink : public Element {
      public: virtual ~HotLink() throw () {}
      public:
        HotLink(const char * href = NULL, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * hrefString;
    };

    class Target : public Element {
      public: virtual ~Target() throw () {}
      public:
        Target(const char * name = NULL, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * nameString;
    };

    class ImageElement : public Element {
      public: virtual ~ImageElement() throw () {}
      protected:
        ImageElement(const char * nam,
                     const char * attr,
                     ElementInSet elmt,
                     ElementInSet req,
                     OptionalCRLF opt,
                     const char * image);
        virtual void AddAttr(PHTML & html) const;
        const char * srcString;
    };

    class Image : public ImageElement {
      public: virtual ~Image() throw () {}
      public:
        Image(const char * src,
              int width = 0,
              int height = 0,
              const char * attr = NULL);
        Image(const char * src,
              const char * alt,
              int width = 0,
              int height = 0,
              const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * altString;
        int width, height;
    };

    class HRule : public ImageElement {
      public: virtual ~HRule() throw () {}
      public:
        HRule(const char * image = NULL, const char * attr = NULL);
    };

    class Note : public ImageElement {
      public: virtual ~Note() throw () {}
      public:
        Note(const char * image = NULL, const char * attr = NULL);
    };

    class Address : public Element {
      public: virtual ~Address() throw () {}
      public:
        Address(const char * attr = NULL);
    };

    class BlockQuote : public Element {
      public: virtual ~BlockQuote() throw () {}
      public:
        BlockQuote(const char * attr = NULL);
    };

    class Credit : public Element {
      public: virtual ~Credit() throw () {}
      public:
        Credit(const char * attr = NULL);
    };

    class SetTab : public Element {
      public: virtual ~SetTab() throw () {}
      public:
        SetTab(const char * id, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * ident;
    };

    class Tab : public Element {
      public: virtual ~Tab() throw () {}
      public:
        Tab(int indent, const char * attr = NULL);
        Tab(const char * id, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * ident;
        int indentSize;
    };


    class Bold : public Element {
      public: virtual ~Bold() throw () {}
      public: Bold() : Element("B", NULL, InBold, InBody, NoCRLF) { }
    };
    class Italic : public Element {
      public: virtual ~Italic() throw () {}
      public: Italic() : Element("I", NULL, InItalic, InBody, NoCRLF) { }
    };
    class TeleType : public Element {
      public: virtual ~TeleType() throw () {}
      public: TeleType() : Element("TT", NULL, InTeleType, InBody, NoCRLF) { }
    };
    class Underline : public Element {
      public: virtual ~Underline() throw () {}
      public: Underline() : Element("U", NULL, InUnderline, InBody, NoCRLF) { }
    };
    class StrikeThrough : public Element {
      public: virtual ~StrikeThrough() throw () {}
      public: StrikeThrough()
                      : Element("S", NULL, InStrikeThrough, InBody, NoCRLF) { }
    };
    class Big : public Element {
      public: virtual ~Big() throw () {}
      public: Big() : Element("BIG", NULL, InBig, InBody, NoCRLF) { }
    };
    class Small : public Element {
      public: virtual ~Small() throw () {}
      public: Small() : Element("SMALL", NULL, InSmall, InBody, NoCRLF) { }
    };
    class Subscript : public Element {
      public: virtual ~Subscript() throw () {}
      public: Subscript()
                        : Element("SUB", NULL, InSubscript, InBody, NoCRLF) { }
    };
    class Superscript : public Element {
      public: virtual ~Superscript() throw () {}
      public: Superscript()
                      : Element("SUP", NULL, InSuperscript, InBody, NoCRLF) { }
    };
    class Emphasis : public Element {
      public: virtual ~Emphasis() throw () {}
      public: Emphasis() : Element("EM", NULL, InEmphasis, InBody, NoCRLF) { }
    };
    class Cite : public Element {
      public: virtual ~Cite() throw () {}
      public: Cite() : Element("CITE", NULL, InCite, InBody, NoCRLF) { }
    };
    class Strong : public Element {
      public: virtual ~Strong() throw () {}
      public: Strong() : Element("STRONG", NULL, InStrong, InBody, NoCRLF) { }
    };
    class Code : public Element {
      public: virtual ~Code() throw () {}
      public: Code() : Element("CODE", NULL, InCode, InBody, NoCRLF) { }
    };
    class Sample : public Element {
      public: virtual ~Sample() throw () {}
      public: Sample() : Element("SAMP", NULL, InSample, InBody, NoCRLF) { }
    };
    class Keyboard : public Element {
      public: virtual ~Keyboard() throw () {}
      public: Keyboard() : Element("KBD", NULL, InKeyboard, InBody, NoCRLF) { }
    };
    class Variable : public Element {
      public: virtual ~Variable() throw () {}
      public: Variable() : Element("VAR", NULL, InVariable, InBody, NoCRLF) { }
    };
    class Definition : public Element {
      public: virtual ~Definition() throw () {}
      public: Definition()
                       : Element("DFN", NULL, InDefinition, InBody, NoCRLF) { }
    };
    class Quote : public Element {
      public: virtual ~Quote() throw () {}
      public: Quote() : Element("Q", NULL, InQuote, InBody, NoCRLF) { }
    };
    class Author : public Element {
      public: virtual ~Author() throw () {}
      public: Author() : Element("AU", NULL, InAuthor, InBody, NoCRLF) { }
    };
    class Person : public Element {
      public: virtual ~Person() throw () {}
      public: Person() : Element("PERSON", NULL, InPerson, InBody, NoCRLF) { }
    };
    class Acronym : public Element {
      public: virtual ~Acronym() throw () {}
      public: Acronym():Element("ACRONYM", NULL, InAcronym, InBody, NoCRLF) { }
    };
    class Abbrev : public Element {
      public: virtual ~Abbrev() throw () {}
      public: Abbrev() : Element("ABBREV", NULL, InAbbrev, InBody, NoCRLF) { }
    };
    class InsertedText : public Element {
      public: virtual ~InsertedText() throw () {}
      public: InsertedText()
                     : Element("INS", NULL, InInsertedText, InBody, NoCRLF) { }
    };
    class DeletedText : public Element {
      public: virtual ~DeletedText() throw () {}
      public: DeletedText()
                      : Element("DEL", NULL, InDeletedText, InBody, NoCRLF) { }
    };


    class SimpleList : public Element {
      public: virtual ~SimpleList() throw () {}
      public:
        SimpleList(const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
    };

    class BulletList : public Element {
      public: virtual ~BulletList() throw () {}
      public:
        BulletList(const char * attr = NULL);
    };

    class OrderedList : public Element {
      public: virtual ~OrderedList() throw () {}
      public:
        OrderedList(int seqNum = 0, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int sequenceNum;
    };

    class DefinitionList : public Element {
      public: virtual ~DefinitionList() throw () {}
      public:
        DefinitionList(const char * attr = NULL);
    };

    class ListHeading : public Element {
      public: virtual ~ListHeading() throw () {}
      public:
        ListHeading(const char * attr = NULL);
    };

    class ListItem : public Element {
      public: virtual ~ListItem() throw () {}
      public:
        ListItem(int skip = 0, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int skipSeq;
    };

    class DefinitionTerm : public Element {
      public: virtual ~DefinitionTerm() throw () {}
      public:
        DefinitionTerm(const char * attr = NULL);
      protected:
        virtual void Output(PHTML & html) const;
    };

    class DefinitionItem : public Element {
      public: virtual ~DefinitionItem() throw () {}
      public:
        DefinitionItem(const char * attr = NULL);
      protected:
        virtual void Output(PHTML & html) const;
    };


    enum BorderCodes {
      NoBorder,
      Border
    };
    class TableStart : public Element {
      public: virtual ~TableStart() throw () {}
      public:
        TableStart(const char * attr = NULL);
        TableStart(BorderCodes border, const char * attr = NULL);
      protected:
        virtual void Output(PHTML & html) const;
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL borderFlag;
    };
    friend class TableStart;

    class TableEnd : public Element {
      public: virtual ~TableEnd() throw () {}
      public:
        TableEnd();
      protected:
        virtual void Output(PHTML & html) const;
    };
    friend class TableEnd;

    class TableRow : public Element {
      public: virtual ~TableRow() throw () {}
      public:
        TableRow(const char * attr = NULL);
    };

    class TableHeader : public Element {
      public: virtual ~TableHeader() throw () {}
      public:
        TableHeader(const char * attr = NULL);
    };

    class TableData : public Element {
      public: virtual ~TableData() throw () {}
      public:
        TableData(const char * attr = NULL);
    };


    class Form : public Element {
      public: virtual ~Form() throw () {}
      public:
        Form(
          const char * method = NULL,
          const char * action = NULL,
          const char * encoding = NULL,
          const char * script = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * methodString;
        const char * actionString;
        const char * mimeTypeString;
        const char * scriptString;
    };

    enum DisableCodes {
      Enabled,
      Disabled
    };
    class FieldElement : public Element {
      public: virtual ~FieldElement() throw () {}
      protected:
        FieldElement(
          const char * nam,
          const char * attr,
          ElementInSet elmt,
          OptionalCRLF opt,
          DisableCodes disabled
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL disabledFlag;
    };

    class Select : public FieldElement {
      public: virtual ~Select() throw () {}
      public:
        Select(
          const char * fname = NULL,
          const char * attr = NULL
        );
        Select(
          const char * fname,
          DisableCodes disabled,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * nameString;
    };

    enum SelectionCodes {
      NotSelected,
      Selected
    };
    class Option : public FieldElement {
      public: virtual ~Option() throw () {}
      public:
        Option(
          const char * attr = NULL
        );
        Option(
          SelectionCodes select,
          const char * attr = NULL
        );
        Option(
          DisableCodes disabled,
          const char * attr = NULL
        );
        Option(
          SelectionCodes select,
          DisableCodes disabled,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL selectedFlag;
    };

    class FormField : public FieldElement {
      public: virtual ~FormField() throw () {}
      protected:
        FormField(
          const char * nam,
          const char * attr,
          ElementInSet elmt,
          OptionalCRLF opt,
          DisableCodes disabled,
          const char * fname
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * nameString;
    };

    class TextArea : public FormField {
      public: virtual ~TextArea() throw () {}
      public:
        TextArea(
          const char * fname,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
        TextArea(
          const char * fname,
          int rows, int cols,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int numRows, numCols;
    };

    class InputField : public FormField {
      public: virtual ~InputField() throw () {}
      protected:
        InputField(
          const char * type,
          const char * fname,
          DisableCodes disabled,
          const char * attr
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * typeString;
    };

    class HiddenField : public InputField {
      public: virtual ~HiddenField() throw () {}
      public:
        HiddenField(
          const char * fname,
          const char * value,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * valueString;
    };

    class InputText : public InputField {
      public: virtual ~InputText() throw () {}
      public:
        InputText(
          const char * fname,
          int size,
          const char * init = NULL,
          const char * attr = NULL
        );
        InputText(
          const char * fname,
          int size,
          DisableCodes disabled,
          const char * attr = NULL
        );
        InputText(
          const char * fname,
          int size,
          int maxLength,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
        InputText(
          const char * fname,
          int size,
          const char * init,
          int maxLength,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        InputText(
          const char * type,
          const char * fname,
          int size,
          const char * init,
          int maxLength,
          DisableCodes disabled,
          const char * attr
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * value;
        int width, length;
    };

    class InputPassword : public InputText {
      public: virtual ~InputPassword() throw () {}
      public:
        InputPassword(
          const char * fname,
          int size,
          const char * init = NULL,
          const char * attr = NULL
        );
        InputPassword(
          const char * fname,
          int size,
          DisableCodes disabled,
          const char * attr = NULL
        );
        InputPassword(
          const char * fname,
          int size,
          int maxLength,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
        InputPassword(
          const char * fname,
          int size,
          const char * init,
          int maxLength,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
    };

    enum CheckedCodes {
      UnChecked,
      Checked
    };
    class RadioButton : public InputField {
      public: virtual ~RadioButton() throw () {}
      public:
        RadioButton(
          const char * fname,
          const char * value,
          const char * attr = NULL
        );
        RadioButton(
          const char * fname,
          const char * value,
          DisableCodes disabled,
          const char * attr = NULL
        );
        RadioButton(
          const char * fname,
          const char * value,
          CheckedCodes check,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        RadioButton(
          const char * type,
          const char * fname,
          const char * value,
          CheckedCodes check,
          DisableCodes disabled,
          const char * attr
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * valueString;
        BOOL checkedFlag;
    };

    class CheckBox : public RadioButton {
      public: virtual ~CheckBox() throw () {}
      public:
        CheckBox(
          const char * fname,
          const char * attr = NULL
        );
        CheckBox(
          const char * fname,
          DisableCodes disabled,
          const char * attr = NULL
        );
        CheckBox(
          const char * fname,
          CheckedCodes check,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
    };


    class InputRange : public InputField {
      public: virtual ~InputRange() throw () {}
      public:
        InputRange(
          const char * fname,
          int min, int max,
          int value = 0,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int minValue, maxValue, initValue;
    };

    class InputFile : public InputField {
      public: virtual ~InputFile() throw () {}
      public:
        InputFile(
          const char * fname,
          const char * accept = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * acceptString;
    };

    class InputImage : public InputField {
      public: virtual ~InputImage() throw () {}
      public:
        InputImage(
          const char * fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        InputImage(
          const char * type,
          const char * fname,
          const char * src,
          DisableCodes disabled,
          const char * attr
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * srcString;
    };

    class InputScribble : public InputImage {
      public: virtual ~InputScribble() throw () {}
      public:
        InputScribble(
          const char * fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
    };

    class ResetButton : public InputImage {
      public: virtual ~ResetButton() throw () {}
      public:
        ResetButton(
          const char * title,
          const char * fname = NULL,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        ResetButton(
          const char * type,
          const char * title,
          const char * fname = NULL,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * titleString;
    };

    class SubmitButton : public ResetButton {
      public: virtual ~SubmitButton() throw () {}
      public:
        SubmitButton(
          const char * title,
          const char * fname = NULL,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
    };


  protected:
    virtual void AssignContents(const PContainer & c);

  private:
    ElementInSet initialElement;
    BYTE elementSet[NumElementsInSet/8+1];
    PINDEX tableNestLevel;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
