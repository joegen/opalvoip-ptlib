/*
 * pxml.cxx
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
 * $Log: pxml.cxx,v $
 * Revision 1.25  2002/12/10 04:41:16  robertj
 * Added test for URL being empty, don't try and run auto load in background.
 *
 * Revision 1.24  2002/11/26 05:53:45  craigs
 * Added ability to auto-reload from URL
 *
 * Revision 1.23  2002/11/21 08:08:52  craigs
 * Changed to not overwrite XML data if load fails
 *
 * Revision 1.22  2002/11/19 07:37:25  craigs
 * Added locking functions and LoadURL function
 *
 * Revision 1.21  2002/11/06 22:47:25  robertj
 * Fixed header comment (copyright etc)
 *
 */

// This depends on the expat XML library by Jim Clark
// See http://www.jclark.com/xml/expat.html for more information

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "pxml.h"
#endif

#include <ptclib/pxml.h>

#define CACHE_BUFFER_SIZE   1024

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
  autoLoadTimer.Stop();
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

  PWaitAndSignal m(xml.rootMutex);

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

PXMLElement * PXML::SetRootElement(const PString & documentType)
{
  PWaitAndSignal m(rootMutex);

  if (rootElement != NULL)
    delete rootElement;

  rootElement = new PXMLElement(rootElement, documentType);

  return rootElement;
}

PXMLElement * PXML::SetRootElement(PXMLElement * element)
{
  PWaitAndSignal m(rootMutex);

  if (rootElement != NULL)
    delete rootElement;

  rootElement = element;

  return rootElement;
}

BOOL PXML::IsDirty() const
{
  PWaitAndSignal m(rootMutex);

  if (rootElement == NULL)
    return FALSE;

  return rootElement->IsDirty();
}

PCaselessString PXML::GetDocumentType() const
{ 
  PWaitAndSignal m(rootMutex);

  if (rootElement == NULL)
    return PCaselessString();
  return rootElement->GetName();
}

BOOL PXML::LoadFile(const PFilePath & fn, int _options)
{
  PWaitAndSignal m(rootMutex);

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

BOOL PXML::LoadURL(const PURL & url)
{
  return LoadURL(url, PMaxTimeInterval, -1);
}


BOOL PXML::LoadURL(const PURL & url, const PTimeInterval & timeout, int _options)
{
  if (url.IsEmpty()) {
    errorString = "Cannot load empty URL";
    errorCol = errorLine = 0;
    return FALSE;
  }

  PString data;
  if (url.GetScheme() == "file") 
    return LoadFile(url.AsFilePath());

  PHTTPClient client;
  PINDEX contentLength;
  PMIMEInfo outMIME, replyMIME;

  // make sure we do not hang around for ever
  client.SetReadTimeout(timeout);

  // get the resource header information
  if (!client.GetDocument(url, outMIME, replyMIME)) {
    errorString = PString("Cannot load URL") & url.AsString();
    errorCol = errorLine = 0;
    return FALSE;
  }

  // get the length of the data
  if (!replyMIME.Contains(PHTTPClient::ContentLengthTag))
    contentLength = (PINDEX)replyMIME[PHTTPClient::ContentLengthTag].AsUnsigned();
  else
    contentLength = P_MAX_INDEX;

  // download the resource into memory
  PINDEX offs = 0;
  for (;;) {
    PINDEX len;
    if (contentLength == P_MAX_INDEX)
      len = CACHE_BUFFER_SIZE;
    else if (offs == contentLength)
      break;
    else
      len = PMIN(contentLength = offs, CACHE_BUFFER_SIZE);

    if (!client.Read(offs + data.GetPointer(offs + len), len))
      break;

    len = client.GetLastReadCount();

    offs += len;
  }

  return Load(data, _options);
}

BOOL PXML::StartAutoReloadURL(const PURL & url, 
                              const PTimeInterval & timeout, 
                              const PTimeInterval & refreshTime,
                              int _options)
{
  if (url.IsEmpty()) {
    autoLoadError = "Cannot auto-load empty URL";
    return FALSE;
  }

  PWaitAndSignal m(autoLoadMutex);
  autoLoadTimer.Stop();

  SetOptions(_options);
  autoloadURL      = url;
  autoLoadWaitTime = timeout;
  autoLoadError    = PString::Empty();
  autoLoadTimer.SetNotifier(PCREATE_NOTIFIER(AutoReloadTimeout));

  BOOL stat = AutoLoadURL();

  autoLoadTimer = refreshTime;

  return stat;
}

void PXML::AutoReloadTimeout(PTimer &, INT)
{
  PThread::Create(PCREATE_NOTIFIER(AutoReloadThread), PThread::AutoDeleteThread);
}

void PXML::AutoReloadThread(PThread &, INT)
{
  PWaitAndSignal m(autoLoadMutex);
  OnAutoLoad(AutoLoadURL());
  autoLoadTimer.Reset();
}

BOOL PXML::AutoLoadURL()
{
  BOOL stat = LoadURL(autoloadURL, autoLoadWaitTime);
  if (stat)
    autoLoadError = PString::Empty();
  else 
    autoLoadError = GetErrorString() + psprintf(" at line %i, column %i", GetErrorLine(), GetErrorColumn());
  return stat;
}

BOOL PXML::StopAutoReloadURL()
{
  PWaitAndSignal m(autoLoadMutex);
  autoLoadTimer.Stop();
  return TRUE;
}


BOOL PXML::Load(const PString & data, int _options)
{
  if (_options >= 0)
    options = _options;

  loadingRootElement = NULL;

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

  if (stat) {
    {
    PWaitAndSignal m(rootMutex);
      if (rootElement != NULL) {
        delete rootElement;
        rootElement = NULL;
      }
      rootElement = loadingRootElement;
    }
    OnLoaded();
  }

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
  PWaitAndSignal m(rootMutex);

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
  PWaitAndSignal m(rootMutex);

  if (_options >= 0)
    options = _options;

  PStringStream strm;
  strm << *this;
  data = strm;
  return TRUE;
}

void PXML::RemoveAll()
{
  PWaitAndSignal m(rootMutex);

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

BOOL PXML::RemoveElement(PINDEX idx)
{
  if (rootElement == NULL)
    return FALSE;

  if (idx >= rootElement->GetSize())
    return FALSE;

  rootElement->RemoveElement(idx);
  return TRUE;
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

  if (loadingRootElement == NULL) 
    loadingRootElement = currentElement;
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

BOOL PXMLElement::RemoveElement(PINDEX idx)
{
  if (idx >= subObjects.GetSize())
    return FALSE;

  subObjects.RemoveAt(idx);
  return TRUE;
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

