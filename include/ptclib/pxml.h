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
 * $Log: pxml.h,v $
 * Revision 1.14  2002/11/06 22:47:24  robertj
 * Fixed header comment (copyright etc)
 *
 */

#ifndef _PXML_H
#define _PXML_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptlib.h>


class PXMLObject;
class PXMLElement;
class PXMLData;


////////////////////////////////////////////////////////////

class PXML : public PObject
{
  PCLASSINFO(PXML, PObject);
  public:
    enum Options {
      Indent              = 1,
      NewLineAfterElement = 2,
      NoIgnoreWhiteSpace  = 4,
      CloseExtended       = 8,
    };

    PXML(int options = -1);
    PXML(const PString & data, int options = -1);

    PXML(const PXML & xml);

    ~PXML();

    BOOL IsDirty() const;

    BOOL Load(const PString & data, int options = -1);
    BOOL LoadFile(const PFilePath & fn, int options = -1);

    BOOL Save(int options = -1);
    BOOL Save(PString & data, int options = -1);
    BOOL SaveFile(const PFilePath & fn, int options = -1);

    void RemoveAll();

    void SetOptions(int _options)
      { options = _options; }

    void PrintOn(ostream & strm) const;

    PXMLElement * GetElement(const PCaselessString & name, PINDEX idx = 0) const;
    PXMLElement * GetElement(PINDEX idx) const;
    PINDEX        GetNumElements() const; 
    PXMLElement * GetRootElement() const { return rootElement; }
    PXMLElement * SetRootElement(PXMLElement * p);
    PXMLElement * SetRootElement(const PString & documentType);
    BOOL          RemoveElement(PINDEX idx);

    PCaselessString GetDocumentType() const;

    PString GetErrorString() const { return errorString; }
    PINDEX  GetErrorColumn() const { return errorCol; }
    PINDEX  GetErrorLine() const   { return errorLine; }


    // overrides for expat callbacks
    virtual void StartElement(const char * name, const char **attrs);
    virtual void EndElement(const char * name);
    virtual void AddCharacterData(const char * data, int len);
    virtual void XmlDecl(const char * version, const char * encoding, int standAlone);
    virtual void StartDocTypeDecl(const char * docTypeName,
		                              const char * sysid,
				                          const char * pubid,
				                          int hasInternalSubSet);
    virtual void EndDocTypeDecl();

     // static methods to create XML tags
     static PString CreateStartTag (const PString & text);
     static PString CreateEndTag (const PString & text);
     static PString CreateTagNoData (const PString & text);
     static PString CreateTag (const PString & text, const PString & data);

  protected:
    void Construct();
    PXMLElement * rootElement;
    PXMLElement * currentElement;
    PXMLData * lastElement;
    int options;
    BOOL loadFromFile;
    PFilePath loadFilename;
    PString version, encoding;
    int standAlone;

    PString errorString;
    PINDEX errorCol;
    PINDEX errorLine;
};

////////////////////////////////////////////////////////////

PARRAY(PXMLObjectArray, PXMLObject);

class PXMLObject : public PObject {
  PCLASSINFO(PXMLObject, PObject);
  public:
    PXMLObject(PXMLElement * _parent)
      : parent(_parent) { dirty = FALSE; }

    PXMLElement * GetParent()
      { return parent; }

    PXMLObject * GetNextObject();

    void SetParent(PXMLElement * newParent)
    { 
      PAssert(parent == NULL, "Cannot reparent PXMLElement");
      parent = newParent;
    }

    virtual void PrintOn(ostream & strm, int indent, int options) const = 0;

    virtual BOOL IsElement() const = 0;

    void SetDirty();
    BOOL IsDirty() const { return dirty; }

    virtual PXMLObject * Clone(PXMLElement * parent) const = 0;

  protected:
    PXMLElement * parent;
    BOOL dirty;
};

////////////////////////////////////////////////////////////

class PXMLData : public PXMLObject {
  PCLASSINFO(PXMLData, PXMLObject);
  public:
    PXMLData(PXMLElement * _parent, const PString & data);
    PXMLData(PXMLElement * _parent, const char * data, int len);

    BOOL IsElement() const    { return FALSE; }

    void SetString(const PString & str, BOOL dirty = TRUE);

    PString GetString() const           { return value; }

    void PrintOn(ostream & strm, int indent, int options) const;

    PXMLObject * Clone(PXMLElement * parent) const;

  protected:
    PString value;
};

////////////////////////////////////////////////////////////

class PXMLElement : public PXMLObject {
  PCLASSINFO(PXMLElement, PXMLObject);
  public:
    PXMLElement(PXMLElement * _parent, const char * name = NULL);
    PXMLElement(PXMLElement * _parent, const PString & name, const PString & data);

    BOOL IsElement() const { return TRUE; }

    void PrintOn(ostream & strm, int indent, int options) const;

    PCaselessString GetName() const
      { return name; }

    void SetName(const PString & v)
      { name = v; }

    PINDEX GetSize() const
      { return subObjects.GetSize(); }

    PXMLObject  * AddSubObject(PXMLObject * elem, BOOL dirty = TRUE);

    PXMLElement * AddChild    (PXMLElement * elem, BOOL dirty = TRUE);
    PXMLData    * AddChild    (PXMLData    * elem, BOOL dirty = TRUE);

    void SetAttribute(const PCaselessString & key,
		                  const PString & value,
                      BOOL setDirty = TRUE);

    PString GetAttribute(const PCaselessString & key) const;
    PString GetKeyAttribute(PINDEX idx) const;
    PString GetDataAttribute(PINDEX idx) const;
    BOOL HasAttribute(const PCaselessString & key);
    BOOL HasAttributes() const      { return attributes.GetSize() > 0; }
    PINDEX GetNumAttributes() const { return attributes.GetSize(); }

    PXMLElement * GetElement(const PCaselessString & name, PINDEX idx = 0) const;
    PXMLObject  * GetElement(PINDEX idx = 0) const;
    BOOL          RemoveElement(PINDEX idx);

    PINDEX FindObject(PXMLObject * ptr) const;

    BOOL HasSubObjects() const
      { return subObjects.GetSize() != 0; }

    PXMLObjectArray  GetSubObjects() const
      { return subObjects; }

    PString GetData() const;

    PXMLObject * Clone(PXMLElement * parent) const;

  protected:
    PCaselessString name;
    PStringToString attributes;
    PXMLObjectArray subObjects;
    BOOL dirty;
};

////////////////////////////////////////////////////////////

class PXMLSettings : public PXML
{
  PCLASSINFO(PXMLSettings, PXML);
  public:
    PXMLSettings(int options = NewLineAfterElement | CloseExtended);
    PXMLSettings(const PString & data, int options = NewLineAfterElement | CloseExtended);
    PXMLSettings(const PConfig & data, int options = NewLineAfterElement | CloseExtended);

    BOOL Load(const PString & data);
    BOOL LoadFile(const PFilePath & fn);

    BOOL Save();
    BOOL Save(PString & data);
    BOOL SaveFile(const PFilePath & fn);

    void SetAttribute(const PCaselessString & section, const PString & key, const PString & value);

    PString GetAttribute(const PCaselessString & section, const PString & key) const;
    BOOL    HasAttribute(const PCaselessString & section, const PString & key) const;

    void ToConfig(PConfig & cfg) const;
};

#endif
