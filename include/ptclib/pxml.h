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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PXML_H
#define PTLIB_PXML_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#include <ptbuildopts.h>

#ifdef P_EXPAT

#include <ptclib/http.h>

////////////////////////////////////////////////////////////

class PXMLElement;
class PXMLData;

class PXMLParser : public PObject
{
  PCLASSINFO(PXMLParser, PObject);
  public:
    enum Options {
      Indent              = 1,
      NewLineAfterElement = 2,
      NoIgnoreWhiteSpace  = 4,   ///< ignored
      CloseExtended       = 8,   ///< ignored
      WithNS              = 16,
    };

    PXMLParser(int options = -1);
    ~PXMLParser();
    PBoolean Parse(const char * data, int dataLen, PBoolean final);
    void GetErrorInfo(PString & errorString, PINDEX & errorCol, PINDEX & errorLine);

    virtual void StartElement(const char * name, const char **attrs);
    virtual void EndElement(const char * name);
    virtual void AddCharacterData(const char * data, int len);
    virtual void XmlDecl(const char * version, const char * encoding, int standAlone);
    virtual void StartDocTypeDecl(const char * docTypeName,
                                  const char * sysid,
                                  const char * pubid,
                                  int hasInternalSubSet);
    virtual void EndDocTypeDecl();
    virtual void StartNamespaceDeclHandler(const char * prefix, const char * uri);
    virtual void EndNamespaceDeclHandler(const char * prefix);

    PString GetVersion() const  { return version; }
    PString GetEncoding() const { return encoding; }
    PBoolean GetStandAlone() const  { return standAlone; }

    PXMLElement * GetXMLTree() const;
    PXMLElement * SetXMLTree(PXMLElement * newRoot);

  protected:
    int options;
    void * expat;
    PXMLElement * rootElement;
    bool rootOpen;
    PXMLElement * currentElement;
    PXMLData * lastElement;
    PString version, encoding;
    int standAlone;
};

class PXMLObject;
class PXMLElement;
class PXMLData;

////////////////////////////////////////////////////////////

class PXMLBase : public PObject
{
  public:
    PXMLBase(int opts = -1)
      : options(opts) { if (options < 0) options = 0; }

    void SetOptions(int opts)
      { options = opts; }

    int GetOptions() const { return options; }

    virtual PBoolean IsNoIndentElement(
      const PString & /*elementName*/
    ) const
    {
      return PFalse;
    }

  protected:
    int options;
};


class PXML : public PXMLBase
{
  PCLASSINFO(PXML, PObject);
  public:

    PXML(
      int options = -1,
      const char * noIndentElements = NULL
    );
    PXML(
      const PString & data,
      int options = -1,
      const char * noIndentElements = NULL
    );

    PXML(const PXML & xml);

    ~PXML();

    bool IsLoaded() const { return rootElement != NULL; }
    PBoolean IsDirty() const;

    PBoolean Load(const PString & data, int options = -1);

#if P_HTTP
    PBoolean StartAutoReloadURL(
      const PURL & url, 
      const PTimeInterval & timeout, 
      const PTimeInterval & refreshTime,
      int options = -1
    );
    PBoolean StopAutoReloadURL();
    PString GetAutoReloadStatus() { PWaitAndSignal m(autoLoadMutex); PString str = autoLoadError; return str; }
    PBoolean AutoLoadURL();
    virtual void OnAutoLoad(PBoolean ok);

    PBoolean LoadURL(const PURL & url);
    PBoolean LoadURL(const PURL & url, const PTimeInterval & timeout, int options = -1);
#endif // P_HTTP

    PBoolean LoadFile(const PFilePath & fn, int options = -1);

    virtual void OnLoaded() { }

    PBoolean Save(int options = -1);
    PBoolean Save(PString & data, int options = -1);
    PBoolean SaveFile(const PFilePath & fn, int options = -1);

    void RemoveAll();

    PBoolean IsNoIndentElement(
      const PString & elementName
    ) const;

    void PrintOn(ostream & strm) const;
    void ReadFrom(istream & strm);

    PXMLElement * GetElement(const PCaselessString & name, PINDEX idx = 0) const;
    PXMLElement * GetElement(PINDEX idx) const;
    PINDEX        GetNumElements() const; 
    PXMLElement * GetRootElement() const { return rootElement; }
    PXMLElement * SetRootElement(PXMLElement * p);
    PXMLElement * SetRootElement(const PString & documentType);
    PBoolean          RemoveElement(PINDEX idx);

    PCaselessString GetDocumentType() const;

    PString GetErrorString() const { return errorString; }
    PINDEX  GetErrorColumn() const { return errorCol; }
    PINDEX  GetErrorLine() const   { return errorLine; }

    PString GetDocType() const         { return docType; }
    void SetDocType(const PString & v) { docType = v; }

    PMutex & GetMutex() { return rootMutex; }

#if P_HTTP
    PDECLARE_NOTIFIER(PTimer,  PXML, AutoReloadTimeout);
    PDECLARE_NOTIFIER(PThread, PXML, AutoReloadThread);
#endif // P_HTTP

    // static methods to create XML tags
    static PString CreateStartTag (const PString & text);
    static PString CreateEndTag (const PString & text);
    static PString CreateTagNoData (const PString & text);
    static PString CreateTag (const PString & text, const PString & data);

  protected:
    void Construct(int options, const char * noIndentElements);
    PXMLElement * rootElement;
    PMutex rootMutex;

    PBoolean loadFromFile;
    PFilePath loadFilename;
    PString version, encoding;
    int standAlone;

#if P_HTTP
    PTimer autoLoadTimer;
    PURL autoloadURL;
    PTimeInterval autoLoadWaitTime;
    PMutex autoLoadMutex;
    PString autoLoadError;
#endif // P_HTTP

    PString errorString;
    PINDEX errorCol;
    PINDEX errorLine;

    PSortedStringList noIndentElements;

    PString docType;
};

////////////////////////////////////////////////////////////

PARRAY(PXMLObjectArray, PXMLObject);

class PXMLObject : public PObject {
  PCLASSINFO(PXMLObject, PObject);
  public:
    PXMLObject(PXMLElement * par)
      : parent(par) { dirty = false; }

    PXMLElement * GetParent() const
      { return parent; }

    PXMLObject * GetNextObject() const;

    void SetParent(PXMLElement * newParent)
    { 
      PAssert(parent == NULL, "Cannot reparent PXMLElement");
      parent = newParent;
    }

    virtual void Output(ostream & strm, const PXMLBase & xml, int indent) const = 0;

    virtual PBoolean IsElement() const = 0;

    void SetDirty();
    PBoolean IsDirty() const { return dirty; }

    virtual PXMLObject * Clone(PXMLElement * parent) const = 0;

  protected:
    PXMLElement * parent;
    PBoolean dirty;
};

////////////////////////////////////////////////////////////

class PXMLData : public PXMLObject {
  PCLASSINFO(PXMLData, PXMLObject);
  public:
    PXMLData(PXMLElement * parent, const PString & data);
    PXMLData(PXMLElement * parent, const char * data, int len);

    PBoolean IsElement() const    { return PFalse; }

    void SetString(const PString & str, PBoolean dirty = PTrue);

    PString GetString() const           { return value; }

    void Output(ostream & strm, const PXMLBase & xml, int indent) const;

    PXMLObject * Clone(PXMLElement * parent) const;

  protected:
    PString value;
};

////////////////////////////////////////////////////////////

class PXMLElement : public PXMLObject {
  PCLASSINFO(PXMLElement, PXMLObject);
  public:
    PXMLElement(PXMLElement * parent, const char * name = NULL);
    PXMLElement(PXMLElement * parent, const PString & name, const PString & data);

    PBoolean IsElement() const { return PTrue; }

    void PrintOn(ostream & strm) const;
    void Output(ostream & strm, const PXMLBase & xml, int indent) const;

    PCaselessString GetName() const
      { return name; }

    /**
        Get the completely qualified name for the element inside the
        XML tree, for example "root:trunk:branch:subbranch:leaf".
     */
    PCaselessString GetPathName() const;

    void SetName(const PString & v)
      { name = v; }

    PINDEX GetSize() const
      { return subObjects.GetSize(); }

    PXMLObject  * AddSubObject(PXMLObject * elem, PBoolean dirty = PTrue);

    PXMLElement * AddChild    (PXMLElement * elem, PBoolean dirty = PTrue);
    PXMLData    * AddChild    (PXMLData    * elem, PBoolean dirty = PTrue);

    void SetAttribute(const PCaselessString & key,
                      const PString & value,
                      PBoolean setDirty = PTrue);

    PString GetAttribute(const PCaselessString & key) const;
    PString GetKeyAttribute(PINDEX idx) const;
    PString GetDataAttribute(PINDEX idx) const;
    PBoolean HasAttribute(const PCaselessString & key) const;
    PBoolean HasAttributes() const      { return attributes.GetSize() > 0; }
    PINDEX GetNumAttributes() const { return attributes.GetSize(); }

    PXMLElement * GetElement(const PCaselessString & name, PINDEX idx = 0) const;
    PXMLObject  * GetElement(PINDEX idx = 0) const;
    PBoolean          RemoveElement(PINDEX idx);

    PINDEX FindObject(const PXMLObject * ptr) const;

    PBoolean HasSubObjects() const
      { return subObjects.GetSize() != 0; }

    PXMLObjectArray  GetSubObjects() const
      { return subObjects; }

    PString GetData() const;

    PXMLObject * Clone(PXMLElement * parent) const;

    void GetFilePosition(unsigned & col, unsigned & line) const { col = column; line = lineNumber; }
    void SetFilePosition(unsigned col,   unsigned line)         { column = col; lineNumber = line; }

  protected:
    PCaselessString name;
    PStringToString attributes;
    PXMLObjectArray subObjects;
    PBoolean dirty;
    unsigned column;
    unsigned lineNumber;
};

////////////////////////////////////////////////////////////

class PConfig;      // stupid gcc 4 does not recognize PConfig as a class

class PXMLSettings : public PXML
{
  PCLASSINFO(PXMLSettings, PXML);
  public:
    PXMLSettings(int options = PXMLParser::NewLineAfterElement);
    PXMLSettings(const PString & data, int options = PXMLParser::NewLineAfterElement);
    PXMLSettings(const PConfig & data, int options = PXMLParser::NewLineAfterElement);

    PBoolean Load(const PString & data);
    PBoolean LoadFile(const PFilePath & fn);

    PBoolean Save();
    PBoolean Save(PString & data);
    PBoolean SaveFile(const PFilePath & fn);

    void SetAttribute(const PCaselessString & section, const PString & key, const PString & value);

    PString GetAttribute(const PCaselessString & section, const PString & key) const;
    PBoolean    HasAttribute(const PCaselessString & section, const PString & key) const;

    void ToConfig(PConfig & cfg) const;
};

////////////////////////////////////////////////////////////

class PXMLStreamParser : public PXMLParser
{
  PCLASSINFO(PXMLStreamParser, PXMLParser);
  public:
    PXMLStreamParser();

    virtual void EndElement(const char * name);
    virtual PXML * Read(PChannel * channel);

  protected:
    PQueue<PXML> messages;
};

#endif // P_EXPAT

#endif // PTLIB_PXML_H


// End Of File ///////////////////////////////////////////////////////////////
