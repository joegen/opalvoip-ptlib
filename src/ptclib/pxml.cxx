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

#include <ptlib.h>
#include <expat.h>

#include <ptclib/pxml.h>

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
  delete rootElement;
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
  XML_Parse(parser, (const char *)data, data.GetLength(), done);

  return TRUE;
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

PXMLElement * PXML::GetElement(const PCaselessString & name, PINDEX idx) const
{
  if (rootElement == NULL)
    return NULL;

  return rootElement->GetElement(name, idx);
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
    if (currentElement->IsData()) {
      if ((options & NoIgnoreWhiteSpace) == 0)
        str = str.Trim();
      lastElement->SetString(lastElement->GetString() & str, FALSE);
    }
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
    strm << "<!DOCTYPE " << rootElement->GetName() << ">";
    if (newLine)
      strm << endl;
    rootElement->PrintOn(strm, (options & Indent) ? 0 : -1, options);
  }
}

///////////////////////////////////////////////////////
//
void PXMLObject::SetDirty()
{
  dirty = TRUE;
  if (parent != NULL)
    parent->SetDirty();
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
    strm << psprintf("%*s", indent, "");
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

///////////////////////////////////////////////////////

PXMLElement::PXMLElement(PXMLElement * _parent, const char * _name)
 : PXMLObject(_parent)
{
  dirty = FALSE;
  if (_name != NULL)
    name = _name;
}

PXMLElement * PXMLElement::GetElement(const PCaselessString & name, PINDEX start) const
{
  PINDEX idx;
  for (idx = start; idx < subObjects.GetSize(); idx++) {
    if (subObjects[idx].IsElement()) {
      if ( ((PXMLElement &)subObjects[idx]).GetName() *= name)
        return (PXMLElement *)&subObjects[idx];
    }
  }
  return NULL;
}

PString PXMLElement::GetAttribute(const PCaselessString & key)
{
  return attributes(key);
}

BOOL PXMLElement::GetAttribute(const PCaselessString & key) const
{
  return attributes.Contains(key);
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
    strm << psprintf("%*s", indent, "");
  strm << "<" << name;

  PINDEX i;
  if (attributes.GetSize() > 0) {
    for (i = 0; i < attributes.GetSize(); i++) {
      PCaselessString key = attributes.GetKeyAt(i);
      strm << " " << key << "=\"" << attributes[key] << "\"";
    }
  }

  if (((options & PXML::CloseExtended) != 0) &&
      ((options & PXML::NoIgnoreWhiteSpace) == 0) &&
      (subObjects.GetSize() == 0)) {
    strm << " />";
    if (newLine)
      strm << endl;
  } else {
    strm << ">";
    if (indent >= 0 || newLine)
      strm << endl;
  
    for (i = 0; i < subObjects.GetSize(); i++) 
      subObjects[i].PrintOn(strm, (indent < 0) ? -1 : (indent + 2), options);

    if (indent > 0)
      strm << psprintf("%*s", indent, "");
    strm << "</" << name << ">";
    if (indent >= 0 || newLine)
      strm << endl;
  }
}

void PXMLElement::AddSubObject(PXMLObject * elem, BOOL setDirty)
{
  subObjects.SetAt(subObjects.GetSize(), elem);
  if (setDirty)
    SetDirty();
}

///////////////////////////////////////////////////////

#define	XMLSETTINGS_OPTIONS	(NewLineAfterElement | CloseExtended)

PXMLSettings::PXMLSettings()
{
  options = XMLSETTINGS_OPTIONS;
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
  if (element == NULL) 
    element = new PXMLElement(rootElement, section);

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

///////////////////////////////////////////////////////
