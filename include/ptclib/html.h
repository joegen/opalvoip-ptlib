/*
 * $Id: html.h,v 1.4 1996/01/28 14:15:56 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: html.h,v $
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
#if _MSC_VER > 800
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
    /* Construct a new HTML object. If a title is specified in the
       constructor then the HEAD, TITLE and BODY elements are output and the
       string is used in a H1 element.
     */

    ~PHTML();


  // New functions for class.
    enum ElementInSet {
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
      InUnorderedList,
      InOrderedList,
      InDefinitionList,
      InListHeading,
      InDefinitionTerm,
      InForm,
      InSelect,
      InTextArea,
      NumElementsInSet
    };
    BOOL Is(ElementInSet elmt);
    void Set(ElementInSet elmt);
    void Clr(ElementInSet elmt);
    void Toggle(ElementInSet elmt);


    class Element {
      protected:
        enum OptionalCRLF { NoCRLF, CloseCRLF, BothCRLF };
        Element(
          const char * nam,
          ElementInSet elmt,
          OptionalCRLF opt
        ) { name = nam; inElement = elmt; crlf = opt; }
        virtual void Output(PHTML & html) const;
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * name;
        ElementInSet inElement;
        OptionalCRLF crlf;
      friend ostream & operator<<(ostream & strm, const Element & elmt)
        { elmt.Output((PHTML&)strm); return strm; }
    };

    class Head : public Element {
      public:
        Head();
      protected:
        virtual void Output(PHTML & html) const;
    };

    class Body : public Element {
      public:
        Body();
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

    class BodyElement : public Element {
      protected:
        BodyElement(const char * nam, ElementInSet elmt, OptionalCRLF opt)
          : Element(nam, elmt, opt) { }
        virtual void Output(PHTML & html) const;
    };

    class Banner : public BodyElement {
      public:
        Banner();
    };

    enum ClearCodes {
      ClearDefault, ClearLeft, ClearRight, ClearAll, ClearEn, ClearPixels,
      NumClearCodes
    };
    class ClearedElement : public BodyElement {
      protected:
        ClearedElement(const char * nam, ElementInSet elmt, OptionalCRLF opt,
                       ClearCodes clear, int distance);
        virtual void AddAttr(PHTML & html) const;
      private:
        ClearCodes clearCode;
        int clearDistance;
    };

    enum AlignCodes {
      AlignDefault, AlignLeft, AlignCentre, AlignCenter = AlignCentre,
      AlignRight, AlignJustify, AlignTop, AlignBottom, AlignDecimal,
      NumAlignCodes
    };
    enum NoWrapCodes {
      WrapWords,
      NoWrap
    };
    class ComplexElement : public ClearedElement {
      protected:
        ComplexElement(const char * nam, ElementInSet elmt, OptionalCRLF opt,
                AlignCodes align, NoWrapCodes noWrap, ClearCodes clear, int distance);
        virtual void AddAttr(PHTML & html) const;
      private:
        AlignCodes alignCode;
        BOOL noWrapFlag;
    };

    class Division : public ClearedElement {
      public:
        Division(ClearCodes clear = ClearDefault, int distance = 1);
    };

    class Heading : public ComplexElement {
      public:
        Heading(int number,
                int sequence = 0,
                int skip = 0,
                AlignCodes align = AlignDefault,
                NoWrapCodes noWrap = WrapWords,
                ClearCodes clear = ClearDefault,
                int distance = 1);
        Heading(int number,
                const char * image,
                int sequence = 0,
                int skip = 0,
                AlignCodes align = AlignDefault,
                NoWrapCodes noWrap = WrapWords,
                ClearCodes clear = ClearDefault,
                int distance = 1);
        Heading(int number,
                const PString & imageStr,
                int sequence = 0,
                int skip = 0,
                AlignCodes align = AlignDefault,
                NoWrapCodes noWrap = WrapWords,
                ClearCodes clear = ClearDefault,
                int distance = 1);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int num;
        const char * srcString;
        int seqNum, skipSeq;
    };

    class BreakLine : public ClearedElement {
      public:
        BreakLine(ClearCodes clear = ClearDefault, int distance = 1);
    };

    class Paragraph : public ComplexElement {
      public:
        Paragraph(AlignCodes align = AlignDefault,
                  NoWrapCodes noWrap = WrapWords,
                  ClearCodes clear = ClearDefault, int distance = 1);
    };

    class PreFormat : public ClearedElement {
      public:
        PreFormat(int widthInChars = 0,
                  ClearCodes clear = ClearDefault,
                  int distance = 1);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int width;
    };

    class Anchor : public BodyElement {
      public:
        Anchor(const char * href = NULL);
        Anchor(const PString & hrefStr);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * hrefString;
    };

    class ImageElement : public ComplexElement {
      protected:
        ImageElement(const char * nam, ElementInSet elmt, OptionalCRLF opt,
                     const char * image, AlignCodes align, NoWrapCodes noWrap,
                     ClearCodes clear, int distance);
        virtual void AddAttr(PHTML & html) const;
        const char * srcString;
    };

    class Image : public ImageElement {
      public:
        Image(const char * src,
              AlignCodes align = AlignDefault,
              int width = 0,
              int height = 0);
        Image(const char * src,
              const char * alt,
              AlignCodes align = AlignDefault,
              int width = 0,
              int height = 0);
        Image(const PString & srcStr,
              const char * alt = NULL,
              AlignCodes align = AlignDefault,
              int width = 0,
              int height = 0);
        Image(const PString & srcStr,
              const PString & altStr,
              AlignCodes align = AlignDefault,
              int width = 0,
              int height = 0);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * altString;
        int width, height;
    };

    class HRule : public ImageElement {
      public:
        HRule(const char * image = NULL,
              ClearCodes clear = ClearDefault,
              int distance = 1);
        HRule(const PString & imageStr,
              ClearCodes clear = ClearDefault,
              int distance = 1);
    };

    class Note : public ImageElement {
      public:
        Note(const char * image = NULL,
             ClearCodes clear = ClearDefault,
             int distance = 1);
        Note(const PString & imageStr,
             ClearCodes clear = ClearDefault,
             int distance = 1);
    };

    class Address : public ComplexElement {
      public:
        Address(NoWrapCodes noWrap = WrapWords,
                ClearCodes clear = ClearDefault,
                int distance = 1);
    };

    class BlockQuote : public ComplexElement {
      public:
        BlockQuote(NoWrapCodes noWrap = WrapWords,
                   ClearCodes clear = ClearDefault,
                   int distance = 1);
    };

    class Credit : public BodyElement {
      public:
        Credit();
      protected:
        virtual void Output(PHTML & html) const;
    };

    class SetTab : public BodyElement {
      public:
        SetTab(const char * id);
        SetTab(const PString & idStr);
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * ident;
    };

    class Tab : public ComplexElement {
      public:
        Tab(int indent,
            AlignCodes align = AlignDefault,
            char decimal = '\0');
        Tab(const char * id,
            AlignCodes align = AlignDefault,
            char decimal = '\0');
        Tab(const PString & idStr,
            AlignCodes align = AlignDefault,
            char decimal = '\0');
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * ident;
        int indentSize;
        char decimalPoint;
    };


    class Bold : BodyElement {
      public: Bold() : BodyElement("B", InBold, NoCRLF) { }
    };
    class Italic : BodyElement {
      public: Italic() : BodyElement("I", InItalic, NoCRLF) { }
    };
    class TeleType : BodyElement {
      public: TeleType() : BodyElement("TT", InTeleType, NoCRLF) { }
    };
    class Underline : BodyElement {
      public: Underline() : BodyElement("U", InUnderline, NoCRLF) { }
    };
    class StrikeThrough : BodyElement {
      public: StrikeThrough() : BodyElement("S", InStrikeThrough, NoCRLF) { }
    };
    class Big : BodyElement {
      public: Big() : BodyElement("BIG", InBig, NoCRLF) { }
    };
    class Small : BodyElement {
      public: Small() : BodyElement("SMALL", InSmall, NoCRLF) { }
    };
    class Subscript : BodyElement {
      public: Subscript() : BodyElement("SUB", InSubscript, NoCRLF) { }
    };
    class Superscript : BodyElement {
      public: Superscript() : BodyElement("SUP", InSuperscript, NoCRLF) { }
    };
    class Emphasis : BodyElement {
      public: Emphasis() : BodyElement("EM", InEmphasis, NoCRLF) { }
    };
    class Cite : BodyElement {
      public: Cite() : BodyElement("CITE", InCite, NoCRLF) { }
    };
    class Strong : BodyElement {
      public: Strong() : BodyElement("STRONG", InStrong, NoCRLF) { }
    };
    class Code : BodyElement {
      public: Code() : BodyElement("CODE", InCode, NoCRLF) { }
    };
    class Sample : BodyElement {
      public: Sample() : BodyElement("SAMP", InSample, NoCRLF) { }
    };
    class Keyboard : BodyElement {
      public: Keyboard() : BodyElement("KBD", InKeyboard, NoCRLF) { }
    };
    class Variable : BodyElement {
      public: Variable() : BodyElement("VAR", InVariable, NoCRLF) { }
    };
    class Definition : BodyElement {
      public: Definition() : BodyElement("DFN", InDefinition, NoCRLF) { }
    };
    class Quote : BodyElement {
      public: Quote() : BodyElement("Q", InQuote, NoCRLF) { }
    };
    class Author : BodyElement {
      public: Author() : BodyElement("AU", InAuthor, NoCRLF) { }
    };
    class Person : BodyElement {
      public: Person() : BodyElement("PERSON", InPerson, NoCRLF) { }
    };
    class Acronym : BodyElement {
      public: Acronym() : BodyElement("ACRONYM", InAcronym, NoCRLF) { }
    };
    class Abbrev : BodyElement {
      public: Abbrev() : BodyElement("ABBREV", InAbbrev, NoCRLF) { }
    };
    class InsertedText : BodyElement {
      public: InsertedText() : BodyElement("INS", InInsertedText, NoCRLF) { }
    };
    class DeletedText : BodyElement {
      public: DeletedText() : BodyElement("DEL", InDeletedText, NoCRLF) { }
    };


    enum CompactCodes {
      NotCompact,
      Compact
    };
    class ListElement : public ClearedElement {
      protected:
        ListElement(
          const char * nam, ElementInSet elmt,
          CompactCodes compact = NotCompact,
          ClearCodes clear = ClearDefault,
          int distance = 1
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL compactFlag;
    };

    enum PlainCodes {
      NotPlain,
      Plain
    };
    enum ListWrapCodes {
      WrapDefault, WrapVert, WrapHoriz
    };
    class UnorderedList : public ListElement {
      public:
        UnorderedList(
          PlainCodes plain = NotPlain,
          ListWrapCodes wrap = WrapDefault,
          CompactCodes compact = NotCompact,
          ClearCodes clear = ClearDefault,
          int distance = 1
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL plainFlag;
        ListWrapCodes wrapColumn;
    };

    class OrderedList : public ListElement {
      public:
        OrderedList(
          BOOL contSeq = FALSE,
          int seqNum = 0,
          CompactCodes compact = NotCompact,
          ClearCodes clear = ClearDefault,
          int distance = 1
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL continueSeq;
        int sequenceNum;
    };

    class DefinitionList : public ListElement {
      public:
        DefinitionList(
          CompactCodes compact = NotCompact,
          ClearCodes clear = ClearDefault,
          int distance = 1
        );
    };

    class ListHeading : public Element {
      public:
        ListHeading();
      protected:
        virtual void Output(PHTML & html) const;
    };

    class ListItem : public ClearedElement {
      public:
        ListItem(
          int skip = 0,
          ClearCodes clear = ClearDefault,
          int distance = 1
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        int skipSeq;
    };

    class DefinitionTerm : public ClearedElement {
      public:
        DefinitionTerm(
          ClearCodes clear = ClearDefault,
          int distance = 1
        );
      protected:
        virtual void Output(PHTML & html) const;
    };

    class DefinitionItem : public ClearedElement {
      public:
        DefinitionItem(
          ClearCodes clear = ClearDefault,
          int distance = 1
        );
      protected:
        virtual void Output(PHTML & html) const;
    };


    class Form : public BodyElement {
      public:
        Form(
          const char * action = NULL,
          const char * method = NULL,
          const char * encoding = NULL,
          const char * script = NULL
        );
        Form(
          const PString & actionStr,
          const char * method = NULL,
          const char * encoding = NULL,
          const char * script = NULL
        );
        Form(
          const PString & actionStr,
          const PString & methodStr,
          const char * encoding = NULL,
          const char * script = NULL
        );
        Form(
          const PString & actionStr,
          const PString & methodStr,
          const PString & encoding,
          const char * script = NULL
        );
        Form(
          const PString & actionStr,
          const PString & methodStr,
          const PString & encoding,
          const PString & script
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * actionString;
        const char * methodString;
        const char * encodingString;
        const char * scriptString;
    };

    enum DisableCodes {
      Enabled,
      Disabled
    };
    class FieldElement : public BodyElement {
      protected:
        FieldElement(
          const char * nam,
          ElementInSet elmt,
          OptionalCRLF opt,
          DisableCodes disabled,
          const char * error
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL disabledFlag;
        const char * errorString;
    };

    class FormField : public FieldElement {
      protected:
        FormField(
          const char * nam,
          ElementInSet elmt,
          OptionalCRLF opt,
          const char * fname,
          DisableCodes disabled,
          const char * error
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * nameString;
    };

    enum MultipleCodes {
      SingleSelect,
      MultipleSelect
    };
    class Select : public FormField {
      public:
        Select(
          const char * fname,
          MultipleCodes multiple = SingleSelect,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        Select(
          const PString & fnameStr,
          MultipleCodes multiple = SingleSelect,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL multipleFlag;
    };

    enum SelectionCodes {
      NotSelected,
      Selected
    };
    class Option : public FieldElement {
      public:
        Option(
          SelectionCodes select = NotSelected,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
      protected:
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL selectedFlag;
    };

    class TextArea : public FormField {
      public:
        TextArea(
          const char * fname,
          int rows = 0, int cols = 0,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        TextArea(
          const PString & fnameStr,
          int rows = 0, int cols = 0,
          DisableCodes disabled = Enabled,
          const char * error = NULL
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
          const char * error
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        const char * typeString;
    };

    class InputText : public InputField {
      public:
        InputText(
          const char * fname,
          int size,
          int maxLength = 0,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputText(
          const PString & fnameStr,
          int size,
          int maxLength = 0,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
      protected:
        InputText(
          const char * type,
          const char * fname,
          int size,
          int maxLength,
          DisableCodes disabled,
          const char * error
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        int width, length;
    };

    class InputPassword : public InputText {
      public:
        InputPassword(
          const char * fname,
          int size,
          int maxLength = 0, 
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputPassword(
          const PString & fnameStr,
          int size,
          int maxLength = 0, 
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
    };

    enum CheckedCodes {
      UnChecked,
      Checked
    };
    class CheckBox : public InputField {
      public:
        CheckBox(
          const char * fname,
          CheckedCodes check = Checked,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        CheckBox(
          const PString & fnameStr,
          CheckedCodes check = Checked,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
      protected:
        CheckBox(
          const char * type,
          const char * fname,
          CheckedCodes check,
          DisableCodes disabled,
          const char * error
        );
        virtual void AddAttr(PHTML & html) const;
      private:
        BOOL checkedFlag;
    };

    class RadioButton : public CheckBox {
      public:
        RadioButton(
          const char * fname,
          CheckedCodes check = Checked,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        RadioButton(
          const PString & fnameStr,
          CheckedCodes check = Checked,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
    };

    class InputRange : public InputField {
      public:
        InputRange(
          const char * fname,
          int min, int max, int value = 0,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputRange(
          const PString & fnameStr,
          int min, int max, int value = 0,
          DisableCodes disabled = Enabled,
          const char * error = NULL
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
          const char * error = NULL
        );
        InputFile(
          const PString & fnameStr,
          const char * accept = NULL,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputFile(
          const char * fname,
          const PString & acceptStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputFile(
          const PString & fnameStr,
          const PString & acceptStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
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
          const char * error = NULL
        );
        InputImage(
          const PString & fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputImage(
          const char * fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputImage(
          const PString & fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
      protected:
        InputImage(
          const char * type,
          const char * fname,
          const char * src,
          DisableCodes disabled,
          const char * error
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
          const char * error = NULL
        );
        InputScribble(
          const PString & fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputScribble(
          const char * fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        InputScribble(
          const PString & fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
    };

    class ResetButton : public InputImage {
      public:
        ResetButton(
          const char * fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        ResetButton(
          const PString & fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        ResetButton(
          const char * fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        ResetButton(
          const PString & fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
    };

    class SubmitButton : public InputImage {
      public:
        SubmitButton(
          const char * fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        SubmitButton(
          const PString & fname,
          const char * src = NULL,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        SubmitButton(
          const char * fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
        SubmitButton(
          const PString & fname,
          const PString & srcStr,
          DisableCodes disabled = Enabled,
          const char * error = NULL
        );
    };


  private:
    BYTE elementSet[NumElementsInSet/8+1];
};


#endif
#endif


// End Of File ///////////////////////////////////////////////////////////////
