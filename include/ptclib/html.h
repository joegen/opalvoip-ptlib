/*
 * $Id: html.h,v 1.14 1996/06/28 13:08:41 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: html.h,v $
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

    PHTML(
      ElementInSet initialState = NumElementsInSet
    );
    PHTML(
      const char * cstr     // C string representation of the title string.
    );
    PHTML(
      const PString & str   // String representation of the title string.
    );
    /* Construct a new HTML object. If a title is specified in the
       constructor then the HEAD, TITLE and BODY elements are output and the
       string is used in a H1 element.
     */

    ~PHTML();

    PHTML & operator=(
      const char * cstr    // String for title in restating HTML.
    );
    PHTML & operator=(
      const PString & str    // String for title in restating HTML.
    );
    /* Restart the HTML string output using the specified value as the
       new title. If <CODE>title</CODE> is empty then no HEAD or TITLE
       elements are placed into the HTML.
     */


  // New functions for class.
    BOOL Is(ElementInSet elmt);
    void Set(ElementInSet elmt);
    void Clr(ElementInSet elmt);
    void Toggle(ElementInSet elmt);


    class Element {
      protected:
        enum OptionalCRLF { NoCRLF, OpenCRLF, CloseCRLF, BothCRLF };
        Element(
          const char * nam,
          const char * att,
          ElementInSet elmt,
          ElementInSet req,
          OptionalCRLF opt
        ) { name = nam; attr= att; inElement = elmt; reqElement = req; crlf = opt; }
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
      public:
        HTML(const char * attr = NULL);
    };

    class Head : public Element {
      public:
        Head();
      protected:
        virtual void Output(PHTML & html) const;
    };

    class Body : public Element {
      public:
        Body(const char * attr = NULL);
      protected:
        virtual void Output(PHTML & html) const;
    };

    class Title : public Element {
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
      public:
        Banner(const char * attr = NULL);
    };

    class Division : public Element {
      public:
        Division(const char * attr = NULL);
    };

    class Heading : public Element {
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
      public:
        BreakLine(const char * attr = NULL);
    };

    class Paragraph : public Element {
      public:
        Paragraph(const char * attr = NULL);
    };

    class PreFormat : public Element {
      public:
        PreFormat(int widthInChars = 0,
                  const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int width;
    };

    class HotLink : public Element {
      public:
        HotLink(const char * href = NULL, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * hrefString;
    };

    class Target : public Element {
      public:
        Target(const char * name = NULL, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * nameString;
    };

    class ImageElement : public Element {
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
      public:
        HRule(const char * image = NULL, const char * attr = NULL);
    };

    class Note : public ImageElement {
      public:
        Note(const char * image = NULL, const char * attr = NULL);
    };

    class Address : public Element {
      public:
        Address(const char * attr = NULL);
    };

    class BlockQuote : public Element {
      public:
        BlockQuote(const char * attr = NULL);
    };

    class Credit : public Element {
      public:
        Credit(const char * attr = NULL);
    };

    class SetTab : public Element {
      public:
        SetTab(const char * id, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * ident;
    };

    class Tab : public Element {
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
      public: Bold() : Element("B", NULL, InBold, InBody, NoCRLF) { }
    };
    class Italic : public Element {
      public: Italic() : Element("I", NULL, InItalic, InBody, NoCRLF) { }
    };
    class TeleType : public Element {
      public: TeleType() : Element("TT", NULL, InTeleType, InBody, NoCRLF) { }
    };
    class Underline : public Element {
      public: Underline() : Element("U", NULL, InUnderline, InBody, NoCRLF) { }
    };
    class StrikeThrough : public Element {
      public: StrikeThrough()
                      : Element("S", NULL, InStrikeThrough, InBody, NoCRLF) { }
    };
    class Big : public Element {
      public: Big() : Element("BIG", NULL, InBig, InBody, NoCRLF) { }
    };
    class Small : public Element {
      public: Small() : Element("SMALL", NULL, InSmall, InBody, NoCRLF) { }
    };
    class Subscript : public Element {
      public: Subscript()
                        : Element("SUB", NULL, InSubscript, InBody, NoCRLF) { }
    };
    class Superscript : public Element {
      public: Superscript()
                      : Element("SUP", NULL, InSuperscript, InBody, NoCRLF) { }
    };
    class Emphasis : public Element {
      public: Emphasis() : Element("EM", NULL, InEmphasis, InBody, NoCRLF) { }
    };
    class Cite : public Element {
      public: Cite() : Element("CITE", NULL, InCite, InBody, NoCRLF) { }
    };
    class Strong : public Element {
      public: Strong() : Element("STRONG", NULL, InStrong, InBody, NoCRLF) { }
    };
    class Code : public Element {
      public: Code() : Element("CODE", NULL, InCode, InBody, NoCRLF) { }
    };
    class Sample : public Element {
      public: Sample() : Element("SAMP", NULL, InSample, InBody, NoCRLF) { }
    };
    class Keyboard : public Element {
      public: Keyboard() : Element("KBD", NULL, InKeyboard, InBody, NoCRLF) { }
    };
    class Variable : public Element {
      public: Variable() : Element("VAR", NULL, InVariable, InBody, NoCRLF) { }
    };
    class Definition : public Element {
      public: Definition()
                       : Element("DFN", NULL, InDefinition, InBody, NoCRLF) { }
    };
    class Quote : public Element {
      public: Quote() : Element("Q", NULL, InQuote, InBody, NoCRLF) { }
    };
    class Author : public Element {
      public: Author() : Element("AU", NULL, InAuthor, InBody, NoCRLF) { }
    };
    class Person : public Element {
      public: Person() : Element("PERSON", NULL, InPerson, InBody, NoCRLF) { }
    };
    class Acronym : public Element {
      public: Acronym():Element("ACRONYM", NULL, InAcronym, InBody, NoCRLF) { }
    };
    class Abbrev : public Element {
      public: Abbrev() : Element("ABBREV", NULL, InAbbrev, InBody, NoCRLF) { }
    };
    class InsertedText : public Element {
      public: InsertedText()
                     : Element("INS", NULL, InInsertedText, InBody, NoCRLF) { }
    };
    class DeletedText : public Element {
      public: DeletedText()
                      : Element("DEL", NULL, InDeletedText, InBody, NoCRLF) { }
    };


    class SimpleList : public Element {
      public:
        SimpleList(const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
    };

    class BulletList : public Element {
      public:
        BulletList(const char * attr = NULL);
    };

    class OrderedList : public Element {
      public:
        OrderedList(int seqNum = 0, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int sequenceNum;
    };

    class DefinitionList : public Element {
      public:
        DefinitionList(const char * attr = NULL);
    };

    class ListHeading : public Element {
      public:
        ListHeading(const char * attr = NULL);
    };

    class ListItem : public Element {
      public:
        ListItem(int skip = 0, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int skipSeq;
    };

    class DefinitionTerm : public Element {
      public:
        DefinitionTerm(const char * attr = NULL);
      protected:
        virtual void Output(PHTML & html) const;
    };

    class DefinitionItem : public Element {
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
      public:
        TableEnd();
      protected:
        virtual void Output(PHTML & html) const;
    };
    friend class TableEnd;

    class TableRow : public Element {
      public:
        TableRow(const char * attr = NULL);
    };

    class TableHeader : public Element {
      public:
        TableHeader(const char * attr = NULL);
    };

    class TableData : public Element {
      public:
        TableData(const char * attr = NULL);
    };


    class Form : public Element {
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
      public:
        InputScribble(
          const char * fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
    };

    class ResetButton : public InputImage {
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
      public:
        SubmitButton(
          const char * title,
          const char * fname = NULL,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
    };


  private:
    ElementInSet initialElement;
    BYTE elementSet[NumElementsInSet/8+1];
    PINDEX tableNestLevel;

    PHTML & operator=(
      const PHTML & html    // HTML to copy
    );
    // Cannot do HTML to HTML copy.
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
