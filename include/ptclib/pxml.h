/*
 * pxml.h
 *
 * XML parser support
 *
 * Portable Windows Library
 *
 * Copyright (c) 2002 Equivalence Pty. Ltd.
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

#ifndef PTLIB_PXML_H
#define PTLIB_PXML_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#ifdef P_EXPAT

#include <ptlib/bitwise_enum.h>
#include <ptclib/http.h>


class PXMLElement;
class PXMLRootElement;


////////////////////////////////////////////////////////////

class PXMLBase : public PObject
{
    PCLASSINFO(PXMLBase, PObject);
  public:
    enum {
      DEFAULT_MAX_ENTITY_LENGTH = 65536
    };

    P_DECLARE_BITWISE_ENUM_EX(
      Options,
      8,
      (
        NoOptions,
        Indent,
        IndentWithTabs,
        NewLineAfterElement,
        NoIgnoreWhiteSpace,
        CloseExtended,
        WithNS,
        FragmentOnly,
        ExpandEntities
      ),
      AllOptions = (1<<8)-1
    );

    enum StandAloneType {
      UninitialisedStandAlone = -2,
      UnknownStandAlone = -1,
      NotStandAlone,
      IsStandAlone
    };

    PXMLBase(Options opts = NoOptions);

    void SetOptions(Options opts) { m_options = opts; }
    Options GetOptions() const { return m_options; }

    void SetMaxEntityLength(unsigned len) { m_maxEntityLength = len; }
    unsigned GetMaxEntityLength() const { return m_maxEntityLength; }

    virtual PBoolean IsNoIndentElement(const PString & /*elementName*/) const
      { return false; }

    virtual bool OutputProgress() const { return true; }

    bool OutputIndent(ostream & strm, int indent, const PString & elementName =  PString::Empty()) const;

  protected:
    Options  m_options;
    unsigned m_maxEntityLength;
};


class PXML : public PXMLBase
{
    PCLASSINFO(PXML, PXMLBase);
  public:
    PXML(
      Options options = NoOptions,
      const char * noIndentElements = NULL
    );
    PXML(const PXML & xml);
    ~PXML();

    void ReadFrom(istream & strm);
    void PrintOn(ostream & strm) const;
    PString AsString() const;

    bool IsDirty() const;

    bool Load(const PString & data);
    bool Load(const PString & data, Options options);
    bool LoadFile(const PFilePath & fn);
    bool LoadFile(const PFilePath & fn, Options options);

    virtual bool OnLoadProgress(unsigned /*percent*/) const { return true; }
    virtual void OnLoaded() { }

    bool Save();
    bool Save(Options options);
    PString AsString();
    PString AsString(Options options);
    bool SaveFile(const PFilePath & fn);
    bool SaveFile(const PFilePath & fn, Options options);
    virtual bool OnSaveProgress(unsigned /*percent*/) const { return true; }
    virtual bool OutputProgress() const;

    virtual PINDEX GetObjectCount() const;

    void RemoveAll();

    virtual PBoolean IsNoIndentElement(
      const PString & elementName
    ) const;


    virtual PXMLElement * CreateElement(const PCaselessString & name, const char * data = NULL);
    virtual PXMLRootElement * CreateRootElement(const PCaselessString & name);

    PXMLElement * GetElement(const PCaselessString & name, const PCaselessString & attr, const PString & attrval) const;
    PXMLElement * GetElement(const PCaselessString & name, PINDEX idx = 0) const;
    PXMLElement * GetElement(PINDEX idx) const;
    PINDEX        GetNumElements() const; 
    PXMLElement * SetRootElement(PXMLRootElement * root);
    PXMLElement * SetRootElement(const PString & documentType);


    enum ValidationOp {
      EndOfValidationList,
      DocType,
      ElementName,
      RequiredAttribute,
      RequiredNonEmptyAttribute,
      RequiredAttributeWithValue,
      RequiredElement,
      Subtree,
      RequiredAttributeWithValueMatching,
      RequiredElementWithBodyMatching,
      OptionalElement,
      OptionalAttribute,
      OptionalNonEmptyAttribute,
      OptionalAttributeWithValue,
      OptionalAttributeWithValueMatching,
      OptionalElementWithBodyMatching,
      SetDefaultNamespace,
      SetNamespace,

      RequiredAttributeWithValueMatchingEx = RequiredAttributeWithValueMatching + 0x8000,
      OptionalAttributeWithValueMatchingEx = OptionalAttributeWithValueMatching + 0x8000,
      RequiredElementWithBodyMatchingEx    = RequiredElementWithBodyMatching    + 0x8000,
      OptionalElementWithBodyMatchingEx    = OptionalElementWithBodyMatching    + 0x8000
    };

    struct ValidationContext {
      PString m_defaultNameSpace;
      PStringToString m_nameSpaces;
    };

    struct ValidationInfo {
      ValidationOp m_op;
      const char * m_name;

      union {
        const void     * m_placeHolder;
        const char     * m_attributeValues;
        ValidationInfo * m_subElement;
        const char     * m_namespace;
      };

      PINDEX m_minCount;
      PINDEX m_maxCount;
    };

    bool Validate(const ValidationInfo * validator);
    bool ValidateElements(ValidationContext & context, PXMLElement * baseElement, const ValidationInfo * elements);
    bool ValidateElement(ValidationContext & context, PXMLElement * element, const ValidationInfo * elements);
    bool LoadAndValidate(const PString & body, const PXML::ValidationInfo * validator, PString & error, Options options = NoOptions);

    const PCaselessString & GetVersion()    const  { return m_version; }
    const PCaselessString & GetEncoding()   const { return m_encoding; }
    StandAloneType          GetStandAlone() const { return m_standAlone; }

    bool IsLoaded() const { return m_rootElement != NULL; }
    PXMLRootElement * GetRootElement() const { return m_rootElement; }

    PCaselessString GetDocumentType() const;
    const PCaselessString & GetDocType() const { return m_docType; }
    const PCaselessString & GetPubicIdentifier() const { return m_publicId; }
    const PCaselessString & GetDtdURI() const { return m_dtdURI; }

    PString  GetErrorString() const { return m_errorString; }
    unsigned GetErrorColumn() const { return m_errorColumn; }
    unsigned GetErrorLine() const   { return m_errorLine; }

    static PString EscapeSpecialChars(const PString & string);

  protected:
    PFilePath m_loadFilename;

    PCaselessString   m_version;
    PCaselessString   m_encoding;
    StandAloneType    m_standAlone;
    PCaselessString   m_docType;
    PCaselessString   m_publicId;
    PCaselessString   m_dtdURI;

    PXMLRootElement * m_rootElement;

    PStringStream m_errorString;
    unsigned      m_errorLine;
    unsigned      m_errorColumn;

    PSortedStringList m_noIndentElements;

    PCaselessString m_defaultNameSpace;

    PINDEX   m_totalObjects;
    mutable PINDEX   m_savedObjects;
    mutable unsigned m_percent;

  friend class PXMLParser;
};


#if P_HTTP
class PXML_HTTP : public PXML
{
    PCLASSINFO(PXML_HTTP, PXML);
  public:
    PXML_HTTP(
      Options options = NoOptions,
      const char * noIndentElements = NULL
    );

    bool StartAutoReloadURL(
      const PURL & url, 
      const PTimeInterval & timeout, 
      const PTimeInterval & refreshTime,
      Options options = NoOptions
    );
    bool StopAutoReloadURL();
    PString GetAutoReloadStatus() const;
    bool AutoLoadURL();
    virtual void OnAutoLoad(PBoolean ok);

    bool LoadURL(const PURL & url);
    bool LoadURL(const PURL & url, const PTimeInterval & timeout, Options options = NoOptions);
    bool LoadURL(const PURL & url, const PURL::LoadParams & params, Options options = NoOptions);

  protected:
    PDECLARE_NOTIFIER(PTimer,  PXML_HTTP, AutoReloadTimeout);
    PDECLARE_NOTIFIER(PThread, PXML_HTTP, AutoReloadThread);

    PTimer        m_autoLoadTimer;
    PURL          m_autoloadURL;
    PTimeInterval m_autoLoadWaitTime;
    PDECLARE_MUTEX(m_autoLoadMutex);
    PString       m_autoLoadError;
};
#endif // P_HTTP


////////////////////////////////////////////////////////////

class PConfig;      // stupid gcc 4 does not recognize PConfig as a class

class PXMLSettings : public PXML
{
  PCLASSINFO(PXMLSettings, PXML);
  public:
    PXMLSettings(Options options = NewLineAfterElement);

    void SetAttribute(const PCaselessString & section, const PString & key, const PString & value);

    PString GetAttribute(const PCaselessString & section, const PString & key) const;
    bool    HasAttribute(const PCaselessString & section, const PString & key) const;

    void ToConfig(PConfig & cfg) const;
    void FromConfig(const PConfig & cfg);
};


////////////////////////////////////////////////////////////

class PXMLObject : public PObject
{
    PCLASSINFO(PXMLObject, PObject);
  protected:
    PXMLObject();

  public:
    PXMLElement * GetParent() const
      { return m_parent; }

    bool SetParent(PXMLElement * parent);

    virtual PINDEX GetObjectCount() const { return 1; }

    PXMLObject * GetNextObject() const;

    PString AsString() const;

    virtual void Output(ostream & strm, const PXMLBase & xml, int indent) const = 0;

    virtual PBoolean IsElement() const = 0;

    void SetDirty();
    bool IsDirty() const { return m_dirty; }

    void GetFilePosition(unsigned & col, unsigned & line) const { col = m_column; line = m_lineNumber; }
    void SetFilePosition(unsigned   col, unsigned   line)       { m_column = col; m_lineNumber = line; }

    virtual PXMLObject * Clone() const = 0;

  protected:
    PXMLElement * m_parent;
    bool          m_dirty;
    unsigned      m_lineNumber;
    unsigned      m_column;

  P_REMOVE_VIRTUAL(PXMLObject *, Clone(PXMLElement *) const, 0);
};

PARRAY(PXMLObjectArray, PXMLObject);

////////////////////////////////////////////////////////////

class PXMLData : public PXMLObject
{
    PCLASSINFO(PXMLData, PXMLObject);
  public:
    PXMLData(const PString & data);
    PXMLData(const char * data, int len);

    PBoolean IsElement() const    { return false; }

    void SetString(const PString & str, bool dirty = true);

    const PString & GetString() const { return m_value; }

    void Output(ostream & strm, const PXMLBase & xml, int indent) const;

    PXMLObject * Clone() const;

  protected:
    PString m_value;
};


////////////////////////////////////////////////////////////

class PXMLElement : public PXMLObject
{
    PCLASSINFO(PXMLElement, PXMLObject);
  protected:
    PXMLElement(const PXMLElement & copy);
  public:
    PXMLElement(const char * name = NULL, const char * data = NULL);

    virtual PINDEX GetObjectCount() const;

    PBoolean IsElement() const { return true; }

    void PrintOn(ostream & strm) const;
    void Output(ostream & strm, const PXMLBase & xml, int indent) const;

    const PCaselessString & GetName() const
      { return m_name; }

    void SetName(const PString & v)
      { m_name = v; }

    /**
        Get the completely qualified name for the element inside the
        XML tree, for example "root:trunk:branch:subbranch:leaf".
     */
    PCaselessString GetPathName() const;

    PINDEX GetSize() const
      { return m_subObjects.GetSize(); }

    PINDEX FindObject(const PXMLObject * ptr) const;

    bool HasSubObjects() const
      { return !m_subObjects.IsEmpty(); }

    virtual PXMLObject * AddSubObject(PXMLObject * elem, bool dirty = true);
    bool RemoveSubObject(PINDEX idx, bool dispose = true);

    virtual PXMLElement * CreateElement(const PCaselessString & name, const char * data = NULL);

    PXMLElement * AddElement(const char * name);
    PXMLElement * AddElement(const PString & name, const PString & data);
    PXMLElement * AddElement(const PString & name, const PString & attrName, const PString & attrVal);

    void SetAttribute(const PCaselessString & key,
                      const PString & value,
                      bool setDirty = true);

    PString GetAttribute(const PCaselessString & key) const;
    bool HasAttribute(const PCaselessString & key) const;
    bool HasAttributes() const      { return m_attributes.GetSize() > 0; }
    const PStringToString & GetAttributes() const { return m_attributes; }

    PXMLObject  * GetSubObject(PINDEX idx) const;
    PXMLElement * GetElement(PINDEX idx = 0) const;
    PXMLElement * GetElement(const PCaselessString & name, PINDEX idx = 0) const;
    PXMLElement * GetElement(const PCaselessString & name, const PCaselessString & attr, const PString & attrval) const;

    template <class T> T * GetElementAs(PINDEX idx = 0) const { return dynamic_cast<T *>(GetElement(idx)); }
    template <class T> T * GetElementAs(const PCaselessString & name, PINDEX idx = 0) const { return dynamic_cast<T *>(GetElement(name, idx)); }
    template <class T> T * GetElementAs(const PCaselessString & name, const PCaselessString & attr, const PString & attrval) const { return dynamic_cast<T *>(GetElement(name, attr, attrval)); }

    PString GetData(bool trim = true) const;

    PXMLObjectArray  GetSubObjects() const
      { return m_subObjects; }

    void SetData(const PString & data);
    virtual PXMLData * AddData(const PString & data);
    virtual void EndData() { }

    PXMLObject * Clone() const;

    void AddNamespace(const PString & prefix, const PString & uri);
    void RemoveNamespace(const PString & prefix);

    bool GetDefaultNamespace(PCaselessString & str) const;
    bool GetNamespace(const PCaselessString & prefix, PCaselessString & str) const;
    PCaselessString PrependNamespace(const PCaselessString & name) const;
    bool GetURIForNamespace(const PCaselessString & prefix, PCaselessString & uri) const;

  protected:
    PCaselessString m_name;
    PStringToString m_attributes;
    PStringToString m_nameSpaces;
    PCaselessString m_defaultNamespace;

    PArray<PXMLObject> m_subObjects;
};


////////////////////////////////////////////////////////////

class PXMLRootElement : public PXMLElement
{
    PCLASSINFO(PXMLRootElement, PXMLElement);
  public:
    PXMLRootElement(PXML & doc, const char * name = NULL)
      : PXMLElement(name)
      , m_document(doc)
    { }

    PXMLRootElement(PXML & doc, const PXMLElement & copy)
      : PXMLElement(copy)
      , m_document(doc)
    { }

    virtual PObject * Clone();
    virtual PXMLElement * CreateElement(const PCaselessString & name, const char * data = NULL);

  protected:
    PXML & m_document;
};


////////////////////////////////////////////////////////////

class PXMLParserBase
{
  protected:
    PXMLParserBase(PXMLBase::Options options);

  public:
    ~PXMLParserBase();

    bool Parse(istream & strm);
    bool Parse(const char * data, size_t dataLen, bool final);

    virtual void StartDocTypeDecl(const char * docType, const char * sysid, const char * pubid, int hasInternalSubSet);
    virtual void EndDocTypeDecl();
    virtual void XmlDecl(const char * version, const char * encoding, int standAlone);
    virtual void StartNamespaceDeclHandler(const char * prefix, const char * uri);
    virtual void EndNamespaceDeclHandler(const char * prefix);
    virtual void StartElement(const char * name, const char **attrs) = 0;
    virtual void EndElement(const char * name) = 0;
    virtual void AddCharacterData(const char * data, int len) = 0;
    virtual void Entity(const char *entityName,
                        int is_parameter_entity,
                        const char *value,
                        int value_length,
                        const char *base,
                        const char *systemId,
                        const char *publicId,
                        const char *notationName);

    virtual bool Progress() { return true; }

    void GetFilePosition(unsigned & col, unsigned & line) const;
    void GetErrorInfo(PString & errorString, unsigned & errorCol, unsigned & errorLine) const;

    bool IsParsing() const { return m_parsing; }

  protected:
    void *   m_context;
    bool     m_parsing;
    off_t    m_total;
    off_t    m_consumed;
    unsigned m_percent;
    bool     m_userAborted;
    bool     m_expandEntities;
};


////////////////////////////////////////////////////////////

class PXMLParser : public PXMLBase, public PXMLParserBase
{
    PCLASSINFO(PXMLParser, PXMLBase);
  public:
    PXMLParser(
      PXML & doc,
      Options options,
      off_t progressTotal
    );

    virtual void StartDocTypeDecl(const char * docType, const char * sysid, const char * pubid, int hasInternalSubSet);
    virtual void XmlDecl(const char * version, const char * encoding, int standAlone);
    virtual void StartNamespaceDeclHandler(const char * prefix, const char * uri);
    virtual void StartElement(const char * name, const char **attrs);
    virtual void EndElement(const char * name);
    virtual void AddCharacterData(const char * data, int len);

    virtual bool Progress();

    PXML & GetDocument() const { return m_document; }

  protected:
    PXML & m_document;

    PXMLElement   * m_currentElement;
    PXMLData      * m_lastData;
    PStringToString m_nameSpaces;
};


////////////////////////////////////////////////////////////

class PXMLStreamParser : public PXMLParser
{
    PCLASSINFO(PXMLStreamParser, PXMLParser);
  public:
    PXMLStreamParser(PXML & doc, Options options = NoOptions);

    virtual void EndElement(const char * name);
    virtual PXMLElement * Read(PChannel * channel);

  protected:
    PQueue<PXMLElement> messages;
};


#else

namespace PXML {
  extern PString EscapeSpecialChars(const PString & str);
};

#endif // P_EXPAT

#endif // PTLIB_PXML_H


// End Of File ///////////////////////////////////////////////////////////////
