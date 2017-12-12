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
 */

#ifndef PTLIB_HTML_H
#define PTLIB_HTML_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif



//////////////////////////////////////////////////////////////////////////////
// PHTML

/** This class describes a HyperText markup Language string as used by the
   World Wide Web and the <code>PURL</code> and <code>PHTTP</code> class.
   
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
      InStyle,
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
      InCustom1,
      InCustom2,
      InCustom3,
      InCustom4,
      InCustom5,
      InCustom6,
      InCustom7,
      InCustom8,
      InCustom9,
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
      const char * cstr     ///< C string representation of the title string.
    );
    PHTML(
      const PString & str   ///< String representation of the title string.
    );

    ~PHTML();

    /** Restart the HTML string output using the specified value as the
       new title. If <CODE>title</CODE> is empty then no HEAD or TITLE
       elements are placed into the HTML.
     */
    PHTML & operator=(
      const PHTML & html     ///< HTML stream to make a copy of.
    ) { AssignContents(html); return *this; }
    PHTML & operator=(
      const PString & str    ///< String for title in restating HTML.
    ) { AssignContents(str); return *this; }
    PHTML & operator=(
      const char * cstr    ///< String for title in restating HTML.
    ) { AssignContents(PString(cstr)); return *this; }
    PHTML & operator=(
      char ch    ///< String for title in restating HTML.
    ) { AssignContents(PString(ch)); return *this; }


  // New functions for class.
    PBoolean Is(ElementInSet elmt) const;
    void Set(ElementInSet elmt);
    void Clr(ElementInSet elmt);
    void Toggle(ElementInSet elmt);


    class Escaped {
      public:
        Escaped(const char * str) : m_str(str) { }
      private:
        void Output(ostream & strm) const;
        PString m_str;
      friend ostream & operator<<(ostream & strm, const Escaped & e) { e.Output(strm); return strm; }
    };
    static PString Escape(const char * str);

    static const PString & GetNonBreakSpace();
    class NonBreakSpace {
      public:
        NonBreakSpace(unsigned count = 1) : m_count(count) { }
      private:
        void Output(ostream & strm) const;
        unsigned m_count;
      friend ostream & operator<<(ostream & strm, const NonBreakSpace & e) { e.Output(strm); return strm; }
    };

    class Element {
      public: 
        virtual ~Element() {}
        enum OptionalCRLF { NoCRLF, OpenCRLF, CloseCRLF, BothCRLF };
        Element(
          const char * nam,
          ElementInSet elmt = NumElementsInSet,
          ElementInSet req = InBody,
          OptionalCRLF opt = BothCRLF
        );
        Element(
          const char * nam,
          const char * att,
          ElementInSet elmt = NumElementsInSet,
          ElementInSet req = InBody,
          OptionalCRLF opt = BothCRLF
        );
      protected:
        virtual void Output(PHTML & html) const;
        virtual void AddAttr(PHTML & html) const;
      private:
        PString      m_name;
        PString      m_attr;
        ElementInSet m_inElement;
        ElementInSet m_reqElement;
        OptionalCRLF m_crlf;
      friend ostream & operator<<(ostream & strm, const Element & elmt)
        { elmt.Output(dynamic_cast<PHTML&>(strm)); return strm; }
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
        PString m_titleString;
    };

    class Style : public Element {
      public:
        Style();
        Style(const char * cssText);
        Style(const PString & titleStr);
      protected:
        virtual void Output(PHTML & html) const;
      private:
        PString m_cssString;
    };

    class StyleLink : public Element {
      public:
        StyleLink(const char * linkCStr);
        StyleLink(const PString & linkStr);
      protected:
        virtual void AddAttr(PHTML & html) const;
        virtual void Output(PHTML & html) const;
      private:
        PString m_styleLink;
    };

    class Banner : public Element {
      public:
        Banner(const char * attr = NULL);
    };

    class DivisionStart : public Element {
      public:
        DivisionStart(const char * attr = NULL);
      protected:
        virtual void Output(PHTML & html) const;
        virtual void AddAttr(PHTML & html) const;
    };

    class DivisionEnd : public Element {
      public:
        DivisionEnd();
      protected:
        virtual void Output(PHTML & html) const;
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
        int     m_level;
        PString m_srcString;
        int     m_seqNum;
        int     m_skipSeq;
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
        int m_width;
    };

    class HotLink : public Element {
      public:
        HotLink(const char * href = NULL, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        PString m_hrefString;
    };

    class Target : public Element {
      public:
        Target(const char * name = NULL, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        PString m_nameString;
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

        PString m_srcString;
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
        PString m_altString;
        int m_width, m_height;
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
        PString m_ident;
    };

    class Tab : public Element {
      public:
        Tab(int indent, const char * attr = NULL);
        Tab(const char * id, const char * attr = NULL);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        PString m_ident;
        int m_indentSize;
    };


    class Bold : public Element {
      public: Bold() : Element("B", NULL, InBold, InBody, NoCRLF) { }
    };
    class Italic : public Element {
      public: 
        Italic() 
          : Element("I", NULL, InItalic, InBody, NoCRLF) { }
    };
    class TeleType : public Element {
      public: 
        TeleType() 
          : Element("TT", NULL, InTeleType, InBody, NoCRLF) { }
    };
    class Underline : public Element {
      public: 
        Underline() 
          : Element("U", NULL, InUnderline, InBody, NoCRLF) { }
    };
    class StrikeThrough : public Element {
      public: 
        StrikeThrough()
          : Element("S", NULL, InStrikeThrough, InBody, NoCRLF) { }
    };
    class Big : public Element {
      public: 
        Big() 
          : Element("BIG", NULL, InBig, InBody, NoCRLF) { }
    };
    class Small : public Element {
      public: 
        Small() 
          : Element("SMALL", NULL, InSmall, InBody, NoCRLF) { }
    };
    class Subscript : public Element {
      public: 
        Subscript()
          : Element("SUB", NULL, InSubscript, InBody, NoCRLF) { }
    };
    class Superscript : public Element {
      public: 
        Superscript()
          : Element("SUP", NULL, InSuperscript, InBody, NoCRLF) { }
    };
    class Emphasis : public Element {
      public: 
        Emphasis() 
          : Element("EM", NULL, InEmphasis, InBody, NoCRLF) { }
    };
    class Cite : public Element {
      public: 
        Cite() 
          : Element("CITE", NULL, InCite, InBody, NoCRLF) { }
    };
    class Strong : public Element {
      public: 
        Strong() 
          : Element("STRONG", NULL, InStrong, InBody, NoCRLF) { }
    };
    class Code : public Element {
      public: 
        Code() 
          : Element("CODE", NULL, InCode, InBody, NoCRLF) { }
    };
    class Sample : public Element {
      public: 
        Sample() 
          : Element("SAMP", NULL, InSample, InBody, NoCRLF) { }
    };
    class Keyboard : public Element {
      public: 
        Keyboard() 
          : Element("KBD", NULL, InKeyboard, InBody, NoCRLF) { }
    };
    class Variable : public Element {
      public: 
        Variable() 
          : Element("VAR", NULL, InVariable, InBody, NoCRLF) { }
    };
    class Definition : public Element {
      public: 
        Definition()
          : Element("DFN", NULL, InDefinition, InBody, NoCRLF) { }
    };
    class Quote : public Element {
      public: 
        Quote() 
          : Element("Q", NULL, InQuote, InBody, NoCRLF) { }
    };
    class Author : public Element {
      public: 
        Author() 
          : Element("AU", NULL, InAuthor, InBody, NoCRLF) { }
    };
    class Person : public Element {
      public: 
        Person() 
          : Element("PERSON", NULL, InPerson, InBody, NoCRLF) { }
    };
    class Acronym : public Element {
      public: 
        Acronym()
          : Element("ACRONYM", NULL, InAcronym, InBody, NoCRLF) { }
    };
    class Abbrev : public Element {
      public: 
        Abbrev() 
          : Element("ABBREV", NULL, InAbbrev, InBody, NoCRLF) { }
    };
    class InsertedText : public Element {
      public: 
        InsertedText()
          : Element("INS", NULL, InInsertedText, InBody, NoCRLF) { }
    };
    class DeletedText : public Element {
      public: 
        DeletedText()
          : Element("DEL", NULL, InDeletedText, InBody, NoCRLF) { }
      public: 
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
        int m_sequenceNum;
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
        int m_skipSeq;
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


    enum TableAttr {
      NoWrap,
      Border1,
      Border2,
      NoPadding,
      CellPad1,
      CellPad2,
      CellPad4,
      CellPad8,
      NoCellSpacing,
      CellSpace1,
      CellSpace2,
      CellSpace4,
      CellSpace8,
      AlignLeft,
      AlignCentre,
      AlignCenter = AlignCentre,
      AlignRight,
      AlignJustify,
      AlignBaseline,
      AlignBottom,
      AlignMiddle,
      AlignTop
    };
    #define P_DECL_HTML_TABLE_CTOR(cls) \
        cls(const char * attr = NULL); \
        cls(TableAttr attr1, const char * attr = NULL); \
        cls(TableAttr attr1, TableAttr attr2, const char * attr = NULL); \
        cls(TableAttr attr1, TableAttr attr2, TableAttr attr3, const char * attr = NULL); \
        cls(TableAttr attr1, TableAttr attr2, TableAttr attr3, TableAttr attr4, const char * attr = NULL); \
        cls(TableAttr attr1, TableAttr attr2, TableAttr attr3, TableAttr attr4, TableAttr attr5, const char * attr = NULL); \

    class TableElement : public Element {
      public:
        TableElement(
          const char * nam,
          const char * att,
          ElementInSet elmt,
          ElementInSet req,
          OptionalCRLF opt
        ) : Element(nam, att, elmt, req, opt) { }
        virtual void Output(PHTML & html) const;
    };

    class TableStart : public TableElement
    {
      public:
        P_DECL_HTML_TABLE_CTOR(TableStart)
      protected:
        virtual void Output(PHTML & html) const;
        virtual void AddAttr(PHTML & html) const;
    };
    friend class TableStart;

    class TableEnd : public TableElement {
      public:
        TableEnd();
      protected:
        virtual void Output(PHTML & html) const;
    };
    friend class TableEnd;

    class TableRow : public TableElement {
      public:
        P_DECL_HTML_TABLE_CTOR(TableRow)
    };

    class TableHeader : public TableElement {
      public:
        P_DECL_HTML_TABLE_CTOR(TableHeader)
    };

    class TableData : public TableElement {
      public:
        P_DECL_HTML_TABLE_CTOR(TableData)
    };


    class Form : public Element {
      public:
        Form(
          const char * method = NULL,
          const char * action = NULL,
          const char * encoding = NULL,
          const char * script = NULL,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        PString m_methodString;
        PString m_actionString;
        PString m_mimeTypeString;
        PString m_scriptString;
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
        bool m_disabledFlag;
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
        PString m_nameString;
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
        bool m_selectedFlag;
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
        PString m_nameString;
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
        int m_numRows, m_numCols;
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
        PString m_typeString;
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
        PString m_valueString;
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
        PString m_value;
        int m_width, m_length;
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
        PString m_valueString;
        bool m_checkedFlag;
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


    class InputNumber : public InputField {
      public:
        InputNumber(
          const char * fname,
          int min, int max,
          int value = 0,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        InputNumber(
          const char * type,
          const char * fname,
          int min, int max,
          int value,
          DisableCodes disabled,
          const char * attr
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        void Construct(int min, int max, int value);
        int m_minValue, m_maxValue, m_initValue;
    };

    class InputReal : public InputField {
      public:
        InputReal(
          const char * fname,
          double minimum, double maximum,
          double value = 0,
          int decimals = 2,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        double m_minValue, m_maxValue, m_initValue;
        int m_decimals;
    };

    class InputRange : public InputNumber {
      public:
        InputRange(
          const char * fname,
          int min, int max,
          int value = 0,
          DisableCodes disabled = Enabled,
          const char * attr = NULL
        );
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
        PString m_acceptString;
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
        PString m_srcString;
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
        PString m_titleString;
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


  protected:
    virtual void AssignContents(const PContainer & c);

  private:
    ElementInSet m_initialElement;
    BYTE         m_elementSet[NumElementsInSet/8+1];
    PINDEX       m_tableNestLevel;
    PINDEX       m_divisionNestLevel;
};


#endif // PTLIB_HTML_H


// End Of File ///////////////////////////////////////////////////////////////
