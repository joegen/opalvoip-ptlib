/*
 * pxml.cxx
 *
 * XML parser support library
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 */

// This depends on the expat XML library by Jim Clark
// See http://www.jclark.com/xml/expat.html for more information

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "pxml.h"
#endif

#include <ptclib/pxml.h>


#if P_EXPAT

#include <expat.h>

#define	XMLSETTINGS_OPTIONS	(NewLineAfterElement | CloseExtended)


////////////////////////////////////////////////////
//
static void PXML_StartElement(void * userData, const char * name, const char ** attrs)
{
  ((PXML *)userData)->StartElement(name, attrs);
}

static void PXML_EndElement(void * userData, const char * name)
{
  ((PXML *)userData)->EndElement(name);
}

static void PXML_CharacterDataHandler(void * userData, const char * data, int len)
{
  ((PXML *)userData)->AddCharacterData(data, len);
}

static void PXML_XmlDeclHandler(void * userData, const char * version, const char * encoding, int standalone)
{
  ((PXML *)userData)->XmlDecl(version, encoding, standalone);
}

static void PXML_StartDocTypeDecl(void * userData,
		            const char * docTypeName,
		            const char * sysid,
		            const char * pubid,
			             int hasInternalSubSet)
{
  ((PXML *)userData)->StartDocTypeDecl(docTypeName, sysid, pubid, hasInternalSubSet);
}

static void PXML_EndDocTypeDecl(void * userData)
{
  ((PXML *)userData)->EndDocTypeDecl();
}

////////////////////////////////////////////////////

PXML::PXML(int _options)
{
  Construct();
  if (_options >= 0)
    options = _options;
}

PXML::PXML(const PString & data, int _options)
{
  Construct();
  if (_options >= 0)
    options = _options;
  Load(data);
}

PXML::~PXML()
{
  RemoveAll();
}

PXML::PXML(const PXML & xml)
{
  Construct();

  options      = xml.options;
  loadFromFile = xml.loadFromFile;
  loadFilename = xml.loadFilename;
  version      = xml.version;
  encoding     = xml.encoding;
  standAlone   = xml.standAlone;

  PXMLElement * oldRootElement = xml.rootElement;
  if (oldRootElement != NULL)
    rootElement = (PXMLElement *)oldRootElement->Clone(NULL);
}

void PXML::Construct()
{
  rootElement    = NULL;
  currentElement = NULL;
  lastElement    = NULL;
  options        = 0;
  loadFromFile   = FALSE;
  standAlone     = -2;
}

BOOL PXML::IsDirty() const
{
  if (rootElement == NULL)
    return FALSE;

  return rootElement->IsDirty();
}

PCaselessString PXML::GetDocumentType() const
{ 
  if (rootElement == NULL)
    return PCaselessString();
  return rootElement->GetName();
}

BOOL PXML::LoadFile(const PFilePath & fn, int _options)
{
  if (_options >= 0)
    options = _options;

  loadFilename = fn;
  loadFromFile = TRUE;

  PFile file;
  if (!file.Open(fn, PFile::ReadOnly)) 
    return FALSE;

  off_t len = file.GetLength();
  PString data;
  if (!file.Read(data.GetPointer(len + 1), len))
    return FALSE;

  data[(PINDEX)len] = '\0';

  return Load(data);
}

BOOL PXML::Load(const PString & data, int _options)
{
  if (_options >= 0)
    options = _options;

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);

  XML_SetElementHandler      (parser, PXML_StartElement, PXML_EndElement);
  XML_SetCharacterDataHandler(parser, PXML_CharacterDataHandler);
  XML_SetXmlDeclHandler      (parser, PXML_XmlDeclHandler);
  XML_SetDoctypeDeclHandler  (parser, PXML_StartDocTypeDecl, PXML_EndDocTypeDecl);

  int done = 1;
  BOOL stat = XML_Parse(parser, (const char *)data, data.GetLength(), done) != 0;

  if (!stat) {
    XML_Error err = XML_GetErrorCode(parser);
    errorString = PString(XML_ErrorString(err));
    errorCol    = XML_GetCurrentColumnNumber(parser);
    errorLine   = XML_GetCurrentLineNumber(parser);
  }

  XML_ParserFree(parser);

  return stat;
}

BOOL PXML::Save(int _options)
{
  if (_options >= 0)
    options = _options;

  if (!loadFromFile || !IsDirty())
    return FALSE;

  return SaveFile(loadFilename);
}

BOOL PXML::SaveFile(const PFilePath & fn, int _options)
{
  PFile file;
  if (!file.Open(fn, PFile::WriteOnly)) 
    return FALSE;

  PString data;
  if (!Save(data, _options))
    return FALSE;

  return file.Write((const char *)data, data.GetLength());
}

BOOL PXML::Save(PString & data, int _options)
{
  if (_options >= 0)
    options = _options;

  PStringStream strm;
  strm << *this;
  data = strm;
  return TRUE;
}

void PXML::RemoveAll()
{
  if (rootElement != NULL) {
    delete rootElement;
    rootElement = NULL;
  }
}

PXMLElement * PXML::GetElement(const PCaselessString & name, PINDEX idx) const
{
  if (rootElement == NULL)
    return NULL;

  return rootElement->GetElement(name, idx);
}

PXMLElement * PXML::GetElement(PINDEX idx) const
{
  if (rootElement == NULL)
    return NULL;
  if (idx >= rootElement->GetSize())
    return NULL;

  return (PXMLElement *)(rootElement->GetElement(idx));
}

PINDEX PXML::GetNumElements() const
{
	if (rootElement == NULL) return 0;
	else return rootElement->GetSize();
}

void PXML::StartElement(const char * name, const char **attrs)
{
  PXMLElement * newElement = new PXMLElement(currentElement, name);
  if (currentElement != NULL)
    currentElement->AddSubObject(newElement, FALSE);

  while (attrs[0] != NULL) {
    newElement->SetAttribute(PString(attrs[0]), PString(attrs[1]));
    attrs += 2;
  }

  currentElement = newElement;
  lastElement    = NULL;

  if (rootElement == NULL) 
    rootElement = currentElement;
}

void PXML::EndElement(const char * /*name*/)
{
  currentElement = currentElement->GetParent();
  lastElement    = NULL;
}

void PXML::AddCharacterData(const char * data, int len)
{
  PString str(data, len);

  if (lastElement != NULL) {
    PAssert(!lastElement->IsElement(), "lastElement set by non-data element");
    if ((options & NoIgnoreWhiteSpace) == 0)
      str = str.Trim();
    lastElement->SetString(lastElement->GetString() & str, FALSE);
  } else {
    if ((options & NoIgnoreWhiteSpace) == 0) {
      str = str.Trim();
      if (str.IsEmpty()) 
        return;
    }
    PXMLData * newElement = new PXMLData(currentElement, str);
    currentElement->AddSubObject(newElement, FALSE);
    lastElement = newElement;
  }
}

void PXML::XmlDecl(const char * _version, const char * _encoding, int _standAlone)
{
  version    = _version;
  encoding   = _encoding;
  standAlone = _standAlone;
}

void PXML::StartDocTypeDecl(const char * /*_docTypeName*/,
	                    const char * /*sysid*/,
			    const char * /*pubid*/,
			             int /*hasInternalSubSet*/)
{
}

void PXML::EndDocTypeDecl()
{
}

void PXML::PrintOn(ostream & strm) const
{
  BOOL newLine = (options & PXML::NewLineAfterElement) != 0;

//<?xml version="1.0" encoding="UTF-8" standalone="yes"?>

  PString ver = version;
  PString enc = encoding;
  int salone = standAlone;

  if (ver.IsEmpty())
    ver= "1.0";
  if (enc.IsEmpty())
    enc = "UTF-8";
  if (salone == -2)
    salone = -1;

  strm << "<?xml version=\"" << ver << "\" encoding=\"" << enc << "\"";
  switch (salone) {
    case 0:
      strm << " standalone=\"no\"";
      break;
    case 1:
      strm << " standalone=\"yes\"";
      break;
    default:
      break;
  }

  strm << "?>";
  if (newLine)
    strm << endl;

  if (rootElement != NULL) {
    strm << "<!DOCTYPE " << rootElement->GetName() << '>';
    if (newLine)
      strm << endl;
    rootElement->PrintOn(strm, (options & Indent) ? 0 : -1, options);
  }
}

PString PXML::CreateStartTag(const PString & text)
{
  return '<' + text + '>';
}


PString PXML::CreateEndTag(const PString & text)
{
  return "</" + text + '>';
}


PString PXML::CreateTagNoData(const PString & text)
{
  return '<' + text + "/>";
}


PString PXML::CreateTag(const PString & text, const PString & data)
{
  return CreateStartTag(text) + data + CreateEndTag(text);
}


///////////////////////////////////////////////////////
//
void PXMLObject::SetDirty()
{
  dirty = TRUE;
  if (parent != NULL)
    parent->SetDirty();
}

PXMLObject * PXMLObject::GetNextObject()
{
	if (parent == NULL)
		return NULL;

  // find our index in our parent's list
  PINDEX idx = parent->FindObject(this);
  if (idx == P_MAX_INDEX)
    return NULL;

  // get the next object
  ++idx;
  if (idx >= parent->GetSize())
    return NULL;

  return (*parent).GetElement(idx);
}

///////////////////////////////////////////////////////

PXMLData::PXMLData(PXMLElement * _parent, const PString & _value)
 : PXMLObject(_parent)
{
  value = _value;
}

PXMLData::PXMLData(PXMLElement * _parent, const char * data, int len)
 : PXMLObject(_parent)
{
  value = PString(data, len);
}

void PXMLData::PrintOn(ostream & strm, int indent, int options) const
{
  if (indent > 0)
    strm << setw(indent) << " ";
  strm << value;
  if (indent >= 0 || ((options & PXML::NewLineAfterElement) != 0))
    strm << endl;
}

void PXMLData::SetString(const PString & str, BOOL setDirty)
{
  value = str;
  if (setDirty)
    SetDirty();
}

PXMLObject * PXMLData::Clone(PXMLElement * _parent) const
{
  return new PXMLData(_parent, value);
}

///////////////////////////////////////////////////////

PXMLElement::PXMLElement(PXMLElement * _parent, const char * _name)
 : PXMLObject(_parent)
{
  dirty = FALSE;
  if (_name != NULL)
    name = _name;
}

PXMLElement::PXMLElement(PXMLElement * _parent, const PString & _name, const PString & data)
 : PXMLObject(_parent), name(_name)
{
  dirty = FALSE;
  AddSubObject(new PXMLData(this, data));
}

PINDEX PXMLElement::FindObject(PXMLObject * ptr) const
{
  return subObjects.GetObjectsIndex(ptr);
}


PXMLElement * PXMLElement::GetElement(const PCaselessString & name, PINDEX start) const
{
  PINDEX idx;
  PINDEX size = subObjects.GetSize();
  PINDEX count = 0;
  for (idx = 0; idx < size; idx++) {
    if (subObjects[idx].IsElement()) {
      PXMLElement & subElement = ((PXMLElement &)subObjects[idx]);
      if (subElement.GetName() *= name) {
        if (count++ == start)
          return (PXMLElement *)&subObjects[idx];
      }
    }
  }
  return NULL;
}

PXMLObject * PXMLElement::GetElement(PINDEX idx) const
{
  if (idx >= subObjects.GetSize())
    return NULL;

  return &subObjects[idx];
}

PString PXMLElement::GetAttribute(const PCaselessString & key) const
{
  return attributes(key);
}

PString PXMLElement::GetKeyAttribute(PINDEX idx) const
{
  if (idx < attributes.GetSize())
    return attributes.GetKeyAt(idx);
  else
    return PString();
}

PString PXMLElement::GetDataAttribute(PINDEX idx) const
{
  if (idx < attributes.GetSize())
    return attributes.GetDataAt(idx);
  else
    return PString();
}

void PXMLElement::SetAttribute(const PCaselessString & key,
		                       const PString & value,
				                  BOOL setDirty)
{
  attributes.SetAt(key, value);
  if (setDirty)
    SetDirty();
}

BOOL PXMLElement::HasAttribute(const PCaselessString & key)
{
  return attributes.Contains(key);
}

void PXMLElement::PrintOn(ostream & strm, int indent, int options) const
{
  BOOL newLine = (options & PXML::NewLineAfterElement) != 0;

  if (indent > 0)
    strm << setw(indent) << " ";
  strm << '<' << name;

  PINDEX i;
  if (attributes.GetSize() > 0) {
    for (i = 0; i < attributes.GetSize(); i++) {
      PCaselessString key = attributes.GetKeyAt(i);
      strm << ' ' << key << "=\"" << attributes[key] << '"';
    }
  }

  if (((options & PXML::CloseExtended) != 0) &&
      ((options & PXML::NoIgnoreWhiteSpace) == 0) &&
      (subObjects.GetSize() == 0)) {
    strm << " />";
    if (newLine)
      strm << endl;
  } else {
    strm << '>';
    if (indent >= 0 || newLine)
      strm << endl;
  
    for (i = 0; i < subObjects.GetSize(); i++) 
      subObjects[i].PrintOn(strm, (indent < 0) ? -1 : (indent + 2), options);

    if (indent > 0)
      strm << setw(indent) << " ";
    strm << "</" << name << '>';
    if (indent >= 0 || newLine)
      strm << endl;
  }
}

PXMLObject * PXMLElement::AddSubObject(PXMLObject * elem, BOOL setDirty)
{
  subObjects.SetAt(subObjects.GetSize(), elem);
  if (setDirty)
    SetDirty();

  return elem;
}

PXMLElement * PXMLElement::AddChild(PXMLElement * elem, BOOL dirty)
{
  return (PXMLElement *)AddSubObject(elem, dirty);
}

PXMLData * PXMLElement::AddChild(PXMLData * elem, BOOL dirty)
{
  return (PXMLData *)AddSubObject(elem, dirty);
}

PXMLObject * PXMLElement::Clone(PXMLElement * _parent) const
{
  PXMLElement * elem = new PXMLElement(_parent);

  elem->SetName(name);
  elem->attributes = attributes;
  elem->dirty      = dirty;

  PINDEX idx;
  for (idx = 0; idx < subObjects.GetSize(); idx++)
    elem->AddSubObject(subObjects[idx].Clone(elem), FALSE);

  return elem;
}

PString PXMLElement::GetData() const
{
  PString str;
  PINDEX idx;
  for (idx = 0; idx < subObjects.GetSize(); idx++) {
    if (!subObjects[idx].IsElement()) {
      PXMLData & dataElement = ((PXMLData &)subObjects[idx]);
      str = str & dataElement.GetString();
    }
  }
  return str;
}


///////////////////////////////////////////////////////

PXMLSettings::PXMLSettings(int options)
  :PXML(options)
{
}

PXMLSettings::PXMLSettings(const PString & data, int options)
  : PXML(data,options) 
{
}

PXMLSettings::PXMLSettings(const PConfig & data, int options)
  : PXML(options) 
{
  PStringList sects = data.GetSections();

  for (PINDEX i = 0;i < (PINDEX)sects.GetSize();++i) {
    PStringToString keyvals = data.GetAllKeyValues(sects[i]);
    for (PINDEX j = 0; j < (PINDEX)keyvals.GetSize(); ++j) {
      SetAttribute(sects[i],keyvals.GetKeyAt(j),keyvals.GetDataAt(j));
	 }
  }
}

BOOL PXMLSettings::Load(const PString & data)
{
  return PXML::Load(data);
}

BOOL PXMLSettings::LoadFile(const PFilePath & fn)
{
  return PXML::LoadFile(fn);
}

BOOL PXMLSettings::Save()
{
  return PXML::Save();
}

BOOL PXMLSettings::Save(PString & data)
{
  return PXML::Save(data);
}

BOOL PXMLSettings::SaveFile(const PFilePath & fn)
{
  return PXML::SaveFile(fn);
}

PString PXMLSettings::GetAttribute(const PCaselessString & section, const PString & key) const
{
  if (rootElement == NULL)
    return PString();

  PXMLElement * element = rootElement->GetElement(section);
  if (element == NULL)
    return PString();

  return element->GetAttribute(key);
}

void PXMLSettings::SetAttribute(const PCaselessString & section, const PString & key, const PString & value)
{
  if (rootElement == NULL) 
    rootElement = new PXMLElement(NULL, "settings");

  PXMLElement * element = rootElement->GetElement(section);
  if (element == NULL) {
    element = new PXMLElement(rootElement, section);
    rootElement->AddSubObject(element);
  }
  element->SetAttribute(key, value);
}

BOOL PXMLSettings::HasAttribute(const PCaselessString & section, const PString & key) const
{
  if (rootElement == NULL)
    return FALSE;

  PXMLElement * element = rootElement->GetElement(section);
  if (element == NULL)
    return FALSE;

  return element->HasAttribute(key);
}

void PXMLSettings::ToConfig(PConfig & cfg) const
{
  for (PINDEX i = 0;i < (PINDEX)GetNumElements();++i) {
    PXMLElement * el = GetElement(i);
    PString sectionName = el->GetName();
    for (PINDEX j = 0; j < (PINDEX)el->GetNumAttributes(); ++j) {
      PString key = el->GetKeyAttribute(j);
      PString dat = el->GetDataAttribute(j);
      if (!key && !dat)
        cfg.SetString(sectionName, key, dat);
    }
  }	
}

///////////////////////////////////////////////////////

#endif 

