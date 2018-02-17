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
 */

// This depends on the expat XML library by Jim Clark
// See http://www.jclark.com/xml/expat.html for more information

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "pxml.h"
#endif

#include <ptclib/pxml.h>

#ifdef P_EXPAT

#define XML_STATIC 1

#include <expat.h>

#define new PNEW

#define CACHE_BUFFER_SIZE   1024
#define XMLSETTINGS_OPTIONS (NewLineAfterElement)


#ifdef P_EXPAT_LIBRARY
  #pragma comment(lib, P_EXPAT_LIBRARY)
#endif


#define MY_CONTEXT ((XML_Parser)m_context)


////////////////////////////////////////////////////

PXMLBase::PXMLBase(Options opts)
  : m_options(opts)
  , m_maxEntityLength(PXML::DEFAULT_MAX_ENTITY_LENGTH)
{
}


////////////////////////////////////////////////////

static void PXML_StartElement(void * userData, const char * name, const char ** attrs)
{
  ((PXMLParserBase *)userData)->StartElement(name, attrs);
}


static void PXML_EndElement(void * userData, const char * name)
{
  ((PXMLParserBase *)userData)->EndElement(name);
}


static void PXML_CharacterDataHandler(void * userData, const char * data, int len)
{
  ((PXMLParserBase *)userData)->AddCharacterData(data, len);
}


static void PXML_XmlDeclHandler(void * userData, const char * version, const char * encoding, int standalone)
{
  ((PXMLParserBase *)userData)->XmlDecl(version, encoding, standalone);
}


static void PXML_StartDocTypeDecl(void * userData,
                                  const char * docTypeName,
                                  const char * sysid,
                                  const char * pubid,
                                  int hasInternalSubSet)
{
  ((PXMLParserBase *)userData)->StartDocTypeDecl(docTypeName, sysid, pubid, hasInternalSubSet);
}


static void PXML_EndDocTypeDecl(void * userData)
{
  ((PXMLParserBase *)userData)->EndDocTypeDecl();
}


static void PXML_StartNamespaceDeclHandler(void *userData, const XML_Char *prefix, const XML_Char *uri)
{
  ((PXMLParserBase *)userData)->StartNamespaceDeclHandler(prefix, uri);
}


static void PXML_EndNamespaceDeclHandler(void *userData, const XML_Char *prefix)
{
  ((PXMLParserBase *)userData)->EndNamespaceDeclHandler(prefix);
}


static void PXML_EntityDeclHandler(void *userData,
                                   const XML_Char *entityName,
                                   int is_parameter_entity,
                                   const XML_Char *value,
                                   int value_length,
                                   const XML_Char *base,
                                   const XML_Char *systemId,
                                   const XML_Char *publicId,
                                   const XML_Char *notationName)
{
  ((PXMLParserBase *)userData)->Entity(entityName, is_parameter_entity, value, value_length, base, systemId, publicId, notationName);
}


PXMLParserBase::PXMLParserBase(PXMLBase::Options options)
  : m_parsing(true)
  , m_total(0)
  , m_consumed(0)
  , m_percent(0)
  , m_userAborted(false)
  , m_expandEntities(options & PXMLBase::ExpandEntities)
{
  if (options & PXMLBase::WithNS)
    m_context = XML_ParserCreateNS(NULL, '|');
  else
    m_context = XML_ParserCreate(NULL);

  XML_SetUserData(MY_CONTEXT, this);

  XML_SetElementHandler      (MY_CONTEXT, PXML_StartElement, PXML_EndElement);
  XML_SetCharacterDataHandler(MY_CONTEXT, PXML_CharacterDataHandler);
  XML_SetXmlDeclHandler      (MY_CONTEXT, PXML_XmlDeclHandler);
  XML_SetDoctypeDeclHandler  (MY_CONTEXT, PXML_StartDocTypeDecl, PXML_EndDocTypeDecl);
  XML_SetNamespaceDeclHandler(MY_CONTEXT, PXML_StartNamespaceDeclHandler, PXML_EndNamespaceDeclHandler);
  XML_SetEntityDeclHandler   (MY_CONTEXT, PXML_EntityDeclHandler);
}


PXMLParserBase::~PXMLParserBase()
{
  XML_ParserFree(MY_CONTEXT);
}


bool PXMLParserBase::Parse(const char * data, size_t dataLen, bool final)
{
  if (m_total != 0) {
    if (final)
      m_consumed = m_total;
    else
      m_consumed += dataLen;

    unsigned newPercent = (unsigned)(m_consumed*100LL/m_total);
    if (m_percent != newPercent) {
      m_percent = newPercent;
      if (!Progress()) {
        m_userAborted = true;
        return false;
      }
    }
  }

  return XML_Parse(MY_CONTEXT, data, dataLen, final) != 0;  
}


void PXMLParserBase::StartDocTypeDecl(const char *, const char *, const char *, int)
{
}


void PXMLParserBase::EndDocTypeDecl()
{
}


void PXMLParserBase::XmlDecl(const char *, const char *, int)
{
}


void PXMLParserBase::StartNamespaceDeclHandler(const char *, const char *)
{
}


void PXMLParserBase::EndNamespaceDeclHandler(const char *)
{
}


void PXMLParserBase::GetFilePosition(unsigned & col, unsigned & line) const
{
  col = XML_GetCurrentColumnNumber(MY_CONTEXT);
  line = XML_GetCurrentLineNumber(MY_CONTEXT);
}


void PXMLParserBase::GetErrorInfo(PString & errorString, unsigned & errorCol, unsigned & errorLine) const
{
  GetFilePosition(errorCol, errorLine);

  if (m_userAborted)
    errorString = "User aborted parsing";
  else
    errorString = XML_ErrorString(XML_GetErrorCode(MY_CONTEXT));
}


bool PXMLParserBase::Parse(istream & strm)
{
  do {
    if (!strm.good())
      return false;

    char buffer[10000];
    strm.read(buffer, sizeof(buffer));

    if (!Parse(buffer, (size_t)strm.gcount(), strm.eof())) {
      strm.setstate(ios::badbit);
      return false;
    }
  } while (m_parsing);

  return true;
}


///////////////////////////////////////////////////////////////////////

PXMLParser::PXMLParser(PXML & doc, Options options, off_t progressTotal)
  : PXMLBase(options)
  , PXMLParserBase(options)
  , m_document(doc)
  , m_currentElement(NULL)
  , m_lastData(NULL)
{
  m_total = progressTotal;
}


void PXMLParser::XmlDecl(const char * version, const char * encoding, int standAlone)
{
  m_document.m_version    = version;
  m_document.m_encoding   = encoding;
  m_document.m_standAlone = (StandAloneType)standAlone;
}


bool PXMLParser::Progress()
{
  return m_document.OnLoadProgress(m_percent);
}


void PXMLParser::StartDocTypeDecl(const char * docType,
                                  const char * sysId,
                                  const char * pubId,
                                  int /*hasInternalSubSet*/)
{
  m_document.m_docType = docType;
  m_document.m_dtdURI = sysId;
  m_document.m_publicId = pubId;
}


void PXMLParser::StartNamespaceDeclHandler(const char * prefix, const char * uri)
{
  m_nameSpaces.SetAt(prefix, uri);
}


void PXMLParser::StartElement(const char * name, const char **attrs)
{
  PXMLElement * newElement;
  if (m_currentElement == NULL) {
    PAssert(m_document.m_rootElement == NULL, PLogicError);
    newElement = m_document.m_rootElement = m_document.CreateRootElement(name);
    PAssert(newElement != NULL, PLogicError);
  }
  else {
    newElement = m_currentElement->CreateElement(name);
    if (newElement == NULL)
      return;
    m_currentElement->AddSubObject(newElement, false);
  }

  unsigned col,line;
  GetFilePosition(col, line);
  newElement->SetFilePosition(col, line);

  while (attrs[0] != NULL) {
    newElement->SetAttribute(attrs[0], attrs[1]);
    attrs += 2;
  }

  m_currentElement = newElement;
  m_lastData = NULL;

  for (PStringToString::iterator it = m_nameSpaces.begin(); it != m_nameSpaces.end(); ++it) 
    m_currentElement->AddNamespace(it->first, it->second);

  m_nameSpaces.RemoveAll();
}


void PXMLParser::EndElement(const char * name)
{
  if (m_currentElement == NULL || m_currentElement->GetName() != name)
    return;

  m_currentElement->EndData();

  if (m_currentElement != m_document.m_rootElement)
    m_currentElement = m_currentElement->GetParent();
  else {
    m_currentElement = NULL;
    m_parsing = false;
  }

  m_lastData = NULL;
}


void PXMLParser::AddCharacterData(const char * data, int len)
{
  unsigned checkLen = len + ((m_lastData != NULL) ? m_lastData->GetString().GetLength() : 0);
  if (checkLen >= m_maxEntityLength) {
    PTRACE(2, "PXML\tAborting XML parse at size " << m_maxEntityLength << " - possible 'billion laugh' attack");
    XML_StopParser(MY_CONTEXT, XML_FALSE);
    return;
  }

  if (m_lastData != NULL) {
    m_lastData->SetString(m_lastData->GetString() + PString(data, len), false);
    return;
  }

  if (!(m_options & NoIgnoreWhiteSpace)) {
    while (len > 0 && *data > 0 && isspace(*data)) {
      ++data;
      --len;
    }
  }

  if (len <= 0)
    return;

  PXMLData * newData = m_currentElement->AddData(PString(data, len));
  if (newData == NULL)
    return;

  unsigned col,line;
  GetFilePosition(col, line);
  newData->SetFilePosition(col, line);
  m_lastData = newData;
}


void PXMLParserBase::Entity(const char * /*entityName*/,
                            int /*is_parameter_entity*/,
                            const char * /*value*/,
                            int /*value_length*/,
                            const char * /*base*/,
                            const char * /*systemId*/,
                            const char * /*publicId*/,
                            const char * /*notationName*/)
{
  if (m_expandEntities)
    return;

  // Disable entity expansion completely to prevent billion laughs attack
  PTRACE(2, "PXML\tAborting XML parse at when expanding entity - possible 'billion laughs' attack");
  XML_StopParser(MY_CONTEXT, XML_FALSE);
}


///////////////////////////////////////////////////////////////////////////////////////////////

PXML::PXML(Options options, const char * noIndentElementsParam)
  : PXMLBase(options)
  , m_standAlone(UninitialisedStandAlone)
  , m_rootElement(NULL)
  , m_errorLine(0)
  , m_errorColumn(0)
  , m_noIndentElements(PString(noIndentElementsParam).Tokenise(' ', false))
  , m_totalObjects(0)
  , m_savedObjects(0)
  , m_percent(0)
{
  if (m_options & PXML::FragmentOnly)
    SetRootElement("");
}


PXML::PXML(const PXML & xml)
  : PXMLBase(xml)
  , m_loadFilename(xml.m_loadFilename)
  , m_standAlone(UninitialisedStandAlone)
  , m_rootElement(NULL)
  , m_errorLine(0)
  , m_errorColumn(0)
  , m_noIndentElements(xml.m_noIndentElements)
  , m_defaultNameSpace(xml.m_defaultNameSpace)
  , m_totalObjects(0)
  , m_savedObjects(0)
  , m_percent(0)
{
  if (xml.m_rootElement != NULL)
    m_rootElement = new PXMLRootElement(*this, *xml.m_rootElement);
}


PXML::~PXML()
{
  RemoveAll();
}


PCaselessString PXML::GetDocumentType() const
{ 
  return m_rootElement == NULL ? PString::Empty() : m_rootElement->GetName();
}


PXMLElement * PXML::SetRootElement(const PString & documentType)
{
  return SetRootElement(CreateRootElement(documentType));
}


PXMLElement * PXML::SetRootElement(PXMLRootElement * root)
{
  delete m_rootElement;
  m_rootElement = root;
  m_errorString.MakeEmpty();
  m_errorLine = m_errorColumn = 0;

  return m_rootElement;
}


bool PXML::IsDirty() const
{
  return m_rootElement != NULL && m_rootElement->IsDirty();
}


bool PXML::LoadFile(const PFilePath & fn, PXML::Options options)
{
  m_options = options;
  return LoadFile(fn);
}


bool PXML::LoadFile(const PFilePath & fn)
{
  PTRACE(4, "XML\tLoading file " << fn);

  RemoveAll();

  m_loadFilename = fn;

  PFile file;
  if (!file.Open(fn, PFile::ReadOnly)) {
    m_errorString << "File open error " << file.GetErrorText();
    return false;
  }

  PXMLParser parser(*this, m_options, file.GetLength());
  parser.SetMaxEntityLength(m_maxEntityLength);
  if (!parser.Parse(file))
    return false;

  PTRACE(4, "XML\tRead XML <" << GetDocumentType() << '>');

  OnLoaded();

  return m_rootElement != NULL;
}


bool PXML::Load(const PString & data, PXML::Options options)
{
  m_options = options;
  return Load(data);
}


bool PXML::Load(const PString & data)
{
  RemoveAll();

  m_loadFilename.MakeEmpty();

  m_errorString.MakeEmpty();
  m_errorLine = m_errorColumn = 0;

  PXMLParser parser(*this, m_options, data.GetLength());
  parser.SetMaxEntityLength(m_maxEntityLength);
  if (!parser.Parse(data, data.GetLength(), true)) {
    parser.GetErrorInfo(m_errorString, m_errorColumn, m_errorLine);
    return false;
  }

  if (!IsLoaded())
    return false;

  PTRACE(4, "XML\tLoaded XML <" << GetDocumentType() << '>');

  OnLoaded();

  return true;
}


bool PXML::Save(PXML::Options options)
{
  m_options = options;
  return Save();
}


bool PXML::Save()
{
  if (m_loadFilename.IsEmpty() || !IsDirty())
    return false;

  return SaveFile(m_loadFilename);
}


bool PXML::SaveFile(const PFilePath & fn, PXML::Options options)
{
  m_options = options;
  return SaveFile(fn);
}


bool PXML::SaveFile(const PFilePath & fn)
{
  PFile file;
  if (!file.Open(fn, PFile::WriteOnly)) 
    return false;

  m_totalObjects = GetObjectCount();
  m_savedObjects = 0;
  m_percent = 0;

  file << *this;
  return file.good();
}


PString PXML::AsString(PXML::Options options)
{
  m_options = options;
  return AsString();
}


PString PXML::AsString()
{
  PStringStream strm;
  strm << *this;
  return strm;
}


bool PXML::OutputProgress() const
{
  ++m_savedObjects;

  unsigned newPercent = (m_totalObjects == 0) ? 0 : (unsigned)(m_savedObjects*100LL/m_totalObjects);
  if (m_percent != newPercent) {
    m_percent = newPercent;
    if (!OnSaveProgress(newPercent))
      return true;
  }

  return false;
}


PINDEX PXML::GetObjectCount() const
{
  return m_rootElement != NULL ? m_rootElement->GetObjectCount() : 0;
}


void PXML::RemoveAll()
{
  delete m_rootElement;
  m_rootElement = NULL;
}


PXMLElement * PXML::CreateElement(const PCaselessString & name, const char * data)
{
  return new PXMLElement(name, data);
}


PXMLRootElement * PXML::CreateRootElement(const PCaselessString & name)
{
  return new PXMLRootElement(*this, name);
}


PXMLElement * PXML::GetElement(const PCaselessString & name, const PCaselessString & attr, const PString & attrval) const
{
  return m_rootElement != NULL ? m_rootElement->GetElement(name, attr, attrval) : NULL;
}


PXMLElement * PXML::GetElement(const PCaselessString & name, PINDEX idx) const
{
  return m_rootElement != NULL ? m_rootElement->GetElement(name, idx) : NULL;
}


PXMLElement * PXML::GetElement(PINDEX idx) const
{
  return m_rootElement != NULL ? m_rootElement->GetElement(idx) : NULL;
}


PINDEX PXML::GetNumElements() const
{
  return m_rootElement == NULL ? 0 : m_rootElement->GetSize();
}


PBoolean PXML::IsNoIndentElement(const PString & elementName) const
{
  return m_noIndentElements.GetValuesIndex(elementName) != P_MAX_INDEX;
}


PString PXML::AsString() const
{
  PStringStream strm;
  PrintOn(strm);
  return strm;
}


void PXML::PrintOn(ostream & strm) const
{
//<?xml version="1.0" encoding="UTF-8" standalone="yes"?>

  if (!(m_options & PXML::FragmentOnly)) {
    strm << "<?xml version=\"";
    if (m_version.IsEmpty())
      strm << "1.0";
    else
      strm << m_version;

    strm << "\" encoding=\"";
    if (m_encoding.IsEmpty())
      strm << "UTF-8";
    else
      strm << m_encoding;
    strm << '"';

    switch (m_standAlone) {
      case NotStandAlone:
        strm << " standalone=\"no\"";
        break;
      case IsStandAlone:
        strm << " standalone=\"yes\"";
        break;
      default:
        break;
    }

    strm << "?>";
    if (m_options & NewLineAfterElement)
      strm << '\n';

    if (!m_docType.IsEmpty()) {
      strm << "<!DOCTYPE " << m_docType;
      if (m_publicId.IsEmpty())
        strm << " SYSTEM";
      else
        strm << " PUBLIC \"" << m_publicId << '"';
      if (!m_dtdURI.IsEmpty())
        strm << " \"" << m_dtdURI << '"';
      strm << '>';
      if (m_options & NewLineAfterElement)
        strm << '\n';
    }
  }

  if (m_rootElement != NULL)
    m_rootElement->Output(strm, *this, 1);
}


void PXML::ReadFrom(istream & strm)
{
  delete m_rootElement;
  m_rootElement = NULL;

  PXMLParser parser(*this, m_options, 0);
  parser.SetMaxEntityLength(m_maxEntityLength);
  if (!parser.Parse(strm))
    return;

  PTRACE(4, "XML\tRead XML <" << GetDocumentType() << '>');
}


bool PXML::Validate(const ValidationInfo * validator)
{
  if (PAssertNULL(validator) == NULL)
    return false;

  m_errorString.MakeEmpty();

  if (m_rootElement != NULL) {
    ValidationContext context;
    return ValidateElements(context, m_rootElement, validator);
  }

  m_errorString << "No root element";
  return false;
}


bool PXML::ValidateElements(ValidationContext & context, PXMLElement * baseElement, const ValidationInfo * validator)
{
  if (PAssertNULL(validator) == NULL)
    return false;

  while (validator->m_op != EndOfValidationList) {
    if (!ValidateElement(context, baseElement, validator))
      return false;
    ++validator;
  }

  return true;
}


bool PXML::ValidateElement(ValidationContext & context, PXMLElement * baseElement, const ValidationInfo * validator)
{
  if (PAssertNULL(validator) == NULL)
    return false;

  PCaselessString elementNameWithNs(validator->m_name);
  {
    PINDEX pos;
    if ((pos = elementNameWithNs.FindLast(':')) == P_MAX_INDEX) {
      if (!context.m_defaultNameSpace.IsEmpty())
        elementNameWithNs = context.m_defaultNameSpace + "|" + elementNameWithNs.Right(pos);
    }
    else {
      PString * uri = context.m_nameSpaces.GetAt(elementNameWithNs.Left(pos));
      if (uri != NULL)
        elementNameWithNs = *uri + "|" + elementNameWithNs.Right(pos);
    }
  }

  bool checkValue = false;
  bool extendedRegex = false;

  switch (validator->m_op) {

    case SetDefaultNamespace:
      context.m_defaultNameSpace = validator->m_name;
      break;

    case SetNamespace:
      context.m_nameSpaces.SetAt(validator->m_name, validator->m_namespace);;
      break;

    case ElementName:
      {
        if (elementNameWithNs != baseElement->GetName()) {
          m_errorString << "Expected element with name \"" << elementNameWithNs << '"';
          baseElement->GetFilePosition(m_errorColumn, m_errorLine);
          return false;
        }
      }
      break;

    case Subtree:
      {
        if (baseElement->GetElement(elementNameWithNs) == NULL) {
          if (validator->m_minCount == 0)
            break;

          m_errorString << "Must have at least " << validator->m_minCount << " instances of '" << elementNameWithNs << "'";
          baseElement->GetFilePosition(m_errorColumn, m_errorLine);
          return false;
        }

        // verify each matching element
        PINDEX index = 0;
        PXMLElement * subElement;
        while ((subElement = baseElement->GetElement(elementNameWithNs, index)) != NULL) {
          if (validator->m_maxCount > 0 && index > validator->m_maxCount) {
            m_errorString << "Must have at no more than " << validator->m_maxCount << " instances of '" << elementNameWithNs << "'";
            baseElement->GetFilePosition(m_errorColumn, m_errorLine);
            return false;
          }

          if (!ValidateElement(context, subElement, validator->m_subElement))
            return false;

          ++index;
        }
      }
      break;

    case RequiredElementWithBodyMatching:
      extendedRegex = true;
    case RequiredElementWithBodyMatchingEx:
      checkValue = true;
    case RequiredElement:
      if (baseElement->GetElement(elementNameWithNs) == NULL) {
        m_errorString << "Element \"" << baseElement->GetName() << "\" missing required subelement \"" << elementNameWithNs << '"';
        baseElement->GetFilePosition(m_errorColumn, m_errorLine);
        return false;
      }
      // fall through

    case OptionalElementWithBodyMatchingEx:
      extendedRegex = extendedRegex || validator->m_op == OptionalElementWithBodyMatchingEx;
      checkValue    = checkValue    || validator->m_op == OptionalElementWithBodyMatching;
    case OptionalElementWithBodyMatching:
      checkValue    = checkValue    || validator->m_op == OptionalElementWithBodyMatching;
    case OptionalElement:
      {
        if (baseElement->GetElement(validator->m_name) == NULL) 
          break;

        // verify each matching element
        PINDEX index = 0;
        PXMLElement * subElement;
        while ((subElement = baseElement->GetElement(elementNameWithNs, index)) != NULL) {
          if (validator->m_maxCount > 0 && index > validator->m_maxCount) {
            m_errorString << "Must have at no more than " << validator->m_maxCount << " instances of '" << elementNameWithNs << "'";
            baseElement->GetFilePosition(m_errorColumn, m_errorLine);
            return false;
          }
          if (validator->m_op == RequiredElementWithBodyMatching) {
            PString toMatch(subElement->GetData());
            PRegularExpression regex(validator->m_attributeValues,
                                     extendedRegex ? PRegularExpression::Extended : PRegularExpression::Simple);
            if (!toMatch.MatchesRegEx(regex)) {
              m_errorString << "Element \"" << subElement->GetName() << "\" has body with value \"" << toMatch.Trim() << "\" that does not match regex \"" << PString(validator->m_attributeValues) << '"';
              return false;
            }
          }
          ++index;
        }
      }
      break;

    case OptionalAttributeWithValueMatchingEx:
      extendedRegex = true;
    case OptionalAttributeWithValueMatching:
    case OptionalAttributeWithValue:
    case OptionalNonEmptyAttribute:
    case OptionalAttribute:
      if (!baseElement->HasAttribute(validator->m_name)) 
        break;
      // fall through
    case RequiredAttributeWithValueMatchingEx:
      extendedRegex = extendedRegex || validator->m_op == RequiredAttributeWithValueMatchingEx;
    case RequiredAttributeWithValueMatching:
    case RequiredAttributeWithValue:
    case RequiredNonEmptyAttribute:
    case RequiredAttribute:
      if (!baseElement->HasAttribute(validator->m_name)) {
        m_errorString << "Element \"" << baseElement->GetName() << "\" missing required attribute \"" << validator->m_name << '"';
        baseElement->GetFilePosition(m_errorColumn, m_errorLine);
        return false;
      }

      switch (validator->m_op) {
        case RequiredNonEmptyAttribute:
        case OptionalNonEmptyAttribute:
          if (baseElement->GetAttribute(validator->m_name).IsEmpty()) {
            m_errorString << "Element \"" << baseElement->GetName() << "\" has attribute \"" << validator->m_name << "\" which cannot be empty";
            baseElement->GetFilePosition(m_errorColumn, m_errorLine);
            return false;
          }
          break;

        case RequiredAttributeWithValue:
        case OptionalAttributeWithValue:
          {
            PString toMatch(baseElement->GetAttribute(validator->m_name));
            PStringArray values = PString(validator->m_attributeValues).Lines();
            PINDEX i = 0;
            for (i = 0; i < values.GetSize(); ++i) {
              if (toMatch *= values[i])
                break;
            }
            if (i == values.GetSize()) {
              m_errorString << "Element \"" << baseElement->GetName() << "\" has attribute \"" << validator->m_name << "\" which is not one of required values ";
              for (i = 0; i < values.GetSize(); ++i) {
                if (i != 0)
                  m_errorString << " | ";
                m_errorString << "'" << values[i] << "'";
              }
              baseElement->GetFilePosition(m_errorColumn, m_errorLine);
              return false;
            }
          }
          break;

        case RequiredAttributeWithValueMatching:
        case OptionalAttributeWithValueMatching:
        case RequiredAttributeWithValueMatchingEx:
        case OptionalAttributeWithValueMatchingEx:
          {
            PString toMatch(baseElement->GetAttribute(validator->m_name));
            PRegularExpression regex(validator->m_attributeValues,
                                     extendedRegex ? PRegularExpression::Extended : PRegularExpression::Simple);
            if (!toMatch.MatchesRegEx(regex)) {
              m_errorString << "Element \"" << baseElement->GetName() << "\" has attribute \"" << validator->m_name << "\" with value \"" << baseElement->GetAttribute(validator->m_name) << "\" that does not match regex \"" << PString(validator->m_attributeValues) << '"';
              return false;
            }
          }
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }

  return true;
}


bool PXML::LoadAndValidate(const PString & body, const PXML::ValidationInfo * validator, PString & error, Options options)
{
  PStringStream err;

  // load the XML
  if (!Load(body, options))
    err << "XML parse";
  else if (!Validate(validator))
    err << "XML validation";
  else
    return true;

  err << " error\n"
         "Error at line " << GetErrorLine() << ", column " << GetErrorColumn() << '\n'
      << GetErrorString() << '\n';
  error = err;
  return false;
}


///////////////////////////////////////////////////////

#if P_HTTP

PXML_HTTP::PXML_HTTP(Options options, const char * noIndentElements)
  : PXML(options, noIndentElements)
{
}


bool PXML_HTTP::LoadURL(const PURL & url)
{
  return LoadURL(url, PMaxTimeInterval, PXML::NoOptions);
}


bool PXML_HTTP::LoadURL(const PURL & url, const PTimeInterval & timeout, Options options)
{
  return LoadURL(url, PURL::LoadParams(PString::Empty(), timeout), options);
}


bool PXML_HTTP::LoadURL(const PURL & url, const PURL::LoadParams & params, Options options)
{
  m_errorString.MakeEmpty();
  m_errorLine = m_errorColumn = 0;

  if (url.IsEmpty()) {
    m_errorString << "Cannot load empty URL.";
    return false;
  }

  PTRACE(4, "XML\tLoading URL " << url);

  PString str;
  PHTTPClient http;
  http.SetReadTimeout(params.m_timeout);
  http.SetAuthenticationInfo(params.m_username, params.m_password);
#if P_SSL
  http.SetSSLCredentials(params.m_authority, params.m_certificate, params.m_privateKey);
#endif
  if (http.GetTextDocument(url, str, params.m_requiredContentType))
    return Load(str, options);

  m_errorString << "Error loading \"" << url << "\", code=" << http.GetLastResponseCode() << ' ' << http.GetLastResponseInfo();
  return false;
}


bool PXML_HTTP::StartAutoReloadURL(const PURL & url, 
                                   const PTimeInterval & timeout, 
                                   const PTimeInterval & refreshTime,
                                   PXML::Options options)
{
  if (url.IsEmpty()) {
    m_autoLoadError = "Cannot auto-load empty URL";
    return false;
  }

  PWaitAndSignal m(m_autoLoadMutex);
  m_autoLoadTimer.Stop();

  SetOptions(options);
  m_autoloadURL      = url;
  m_autoLoadWaitTime = timeout;
  m_autoLoadError.MakeEmpty();
  m_autoLoadTimer.SetNotifier(PCREATE_NOTIFIER(AutoReloadTimeout), "XMLReload");

  bool stat = AutoLoadURL();

  m_autoLoadTimer = refreshTime;

  return stat;
}


void PXML_HTTP::AutoReloadTimeout(PTimer &, P_INT_PTR)
{
  PThread::Create(PCREATE_NOTIFIER(AutoReloadThread), "XmlReload");
}


void PXML_HTTP::AutoReloadThread(PThread &, P_INT_PTR)
{
  PWaitAndSignal m(m_autoLoadMutex);
  OnAutoLoad(AutoLoadURL());
  m_autoLoadTimer.Reset();
}


void PXML_HTTP::OnAutoLoad(bool PTRACE_PARAM(ok))
{
  PTRACE_IF(3, !ok, "XML\tFailed to load XML: " << GetErrorString());
}


PString PXML_HTTP::GetAutoReloadStatus() const
{
  PWaitAndSignal m(m_autoLoadMutex);
  PString str = m_autoLoadError;
  str.MakeEmpty();
  return str;
}


bool PXML_HTTP::AutoLoadURL()
{
  bool stat = LoadURL(m_autoloadURL, m_autoLoadWaitTime);
  if (stat)
    m_autoLoadError.MakeEmpty();
  else 
    m_autoLoadError = GetErrorString() + psprintf(" at line %i, column %i", GetErrorLine(), GetErrorColumn());
  return stat;
}


bool PXML_HTTP::StopAutoReloadURL()
{
  PWaitAndSignal m(m_autoLoadMutex);
  m_autoLoadTimer.Stop();
  return true;
}

#endif // P_HTTP


///////////////////////////////////////////////////////

PXMLObject::PXMLObject()
  : m_parent(NULL)
  , m_dirty(false)
  , m_lineNumber(1)
  , m_column(1)
{
}


void PXMLObject::SetDirty()
{
  m_dirty = true;
  if (m_parent != NULL)
    m_parent->SetDirty();
}


bool PXMLObject::SetParent(PXMLElement * parent)
{
  if (!PAssert(m_parent == NULL, "XML object alread in another element"))
    return false;

  m_parent = PAssertNULL(parent);
  return true;
}


PXMLObject * PXMLObject::GetNextObject() const
{
  if (m_parent == NULL)
    return NULL;

  // find our index in our parent's list
  PINDEX idx = m_parent->FindObject(this);
  if (idx == P_MAX_INDEX)
    return NULL;

  // get the next object
  return m_parent->GetSubObject(idx+1);
}


PString PXMLObject::AsString() const
{
  PStringStream strm;
  PrintOn(strm);
  return strm;
}


///////////////////////////////////////////////////////

bool PXMLBase::OutputIndent(ostream & strm, int indent, const PString & elementName) const
{
  if (!elementName.IsEmpty() && IsNoIndentElement(elementName))
    return false;

  if (indent == 0)
    return false;

  if (m_options & IndentWithTabs) {
    for (int tab = 0; tab < indent; ++tab)
      strm << '\t';
    return true;
  }

  if (indent > 1 && m_options & PXML::Indent) {
    strm << setw((indent-1)*2) << " ";
    return true;
  }

 return m_options & NewLineAfterElement;
}


///////////////////////////////////////////////////////

PXMLData::PXMLData(const PString & value)
 : m_value(value)
{
}


PXMLData::PXMLData(const char * data, int len)
 : m_value(data, len)
{
}


void PXMLData::Output(ostream & strm, const PXMLBase & xml, int indent) const
{
  xml.OutputProgress();
  bool newLine = xml.OutputIndent(strm, indent, m_parent->GetName());

  strm << m_value;

  if (newLine)
    strm << endl;
}


void PXMLData::SetString(const PString & str, bool setDirty)
{
  m_value = str;
  if (setDirty)
    SetDirty();
}


PXMLObject * PXMLData::Clone() const
{
  return new PXMLData(m_value);
}


///////////////////////////////////////////////////////

PXMLElement::PXMLElement(const char * name, const char * data)
 : m_name(name)
{
  if (data != NULL)
    AddData(data);
}


PXMLElement::PXMLElement(const PXMLElement & copy)
  : m_name(copy.m_name)
  , m_attributes(copy.m_attributes)
{
  m_attributes.MakeUnique();
  m_dirty = copy.m_dirty;

  for (PINDEX idx = 0; idx < copy.m_subObjects.GetSize(); idx++)
    AddSubObject(copy.m_subObjects[idx].Clone(), false);
}


PINDEX PXMLElement::FindObject(const PXMLObject * ptr) const
{
  return m_subObjects.GetObjectsIndex(ptr);
}


bool PXMLElement::GetDefaultNamespace(PCaselessString & str) const
{
  if (!m_defaultNamespace.IsEmpty()) {
    str = m_defaultNamespace;
    return true;
  }

  if (m_parent != NULL)
    return m_parent->GetDefaultNamespace(str);

  return false;
}


bool PXMLElement::GetNamespace(const PCaselessString & prefix, PCaselessString & str) const
{
  if (m_nameSpaces.GetValuesIndex(prefix) != P_MAX_INDEX) {
    str = m_nameSpaces[prefix];
    return true;
  }

  if (m_parent != NULL)
    return m_parent->GetNamespace(prefix, str);

  return false;
}


bool PXMLElement::GetURIForNamespace(const PCaselessString & prefix, PCaselessString & uri) const
{
  if (prefix.IsEmpty()) {
    if (!m_defaultNamespace.IsEmpty()) {
      uri = m_defaultNamespace + "|"; 
      return true;
    }
  }
  else {
    for (PStringToString::const_iterator it = m_nameSpaces.begin(); it != m_nameSpaces.end(); ++it) {
      if (prefix == it->second) {
        uri = it->first + "|";
        return true;
      }
    }
  }

  if (m_parent != NULL)
    return m_parent->GetNamespace(prefix, uri);

  uri = prefix + ":";

  return false;
}


PCaselessString PXMLElement::PrependNamespace(const PCaselessString & name) const
{
  if (name.Find('|') == P_MAX_INDEX) {
    PCaselessString newPrefix;
    PINDEX pos = name.FindLast(':');
    if (pos == P_MAX_INDEX) {
      if (GetDefaultNamespace(newPrefix))
        return newPrefix + '|' + name;
    }
    else {
      if (GetNamespace(name.Left(pos), newPrefix))
        return newPrefix + '|' + name.Mid(pos+1);
    }
  }

  return name;
}


PXMLObject * PXMLElement::GetSubObject(PINDEX idx) const
{
  return idx < m_subObjects.GetSize() ? &m_subObjects[idx] : NULL;
}


PXMLElement * PXMLElement::GetElement(PINDEX index) const
{
  for (PINDEX i = 0; i < m_subObjects.GetSize(); i++) {
    PXMLElement * element = dynamic_cast<PXMLElement *>(m_subObjects.GetAt(i));
    if (element != NULL && index-- == 0)
      return element;
  }
  return NULL;
}


PXMLElement * PXMLElement::GetElement(const PCaselessString & name, PINDEX index) const
{
  PCaselessString extendedName(PrependNamespace(name));
  for (PINDEX i = 0; i < m_subObjects.GetSize(); i++) {
    PXMLElement * element = dynamic_cast<PXMLElement *>(m_subObjects.GetAt(i));
    if (element != NULL && extendedName == element->GetName() && index-- == 0)
      return element;
  }
  return NULL;
}


PXMLElement * PXMLElement::GetElement(const PCaselessString & name, const PCaselessString & attr, const PString & attrval) const
{
  PCaselessString extendedName(PrependNamespace(name));
  for (PINDEX i = 0; i < m_subObjects.GetSize(); i++) {
    PXMLElement * element = dynamic_cast<PXMLElement *>(m_subObjects.GetAt(i));
    if (element != NULL && extendedName == element->GetName() && attrval == element->GetAttribute(attr))
      return element;
  }
  return NULL;
}


bool PXMLElement::RemoveSubObject(PINDEX idx, bool dispose)
{
  if (idx >= m_subObjects.GetSize())
    return false;

  if (dispose)
    m_subObjects.RemoveAt(idx);
  else {
    m_subObjects.DisallowDeleteObjects();
    m_subObjects.RemoveAt(idx);
    m_subObjects.AllowDeleteObjects();
  }
  return true;
}


PString PXMLElement::GetAttribute(const PCaselessString & key) const
{
  return m_attributes(key);
}

void PXMLElement::SetAttribute(const PCaselessString & key,
                               const PString & value,
                               bool setDirty)
{
  m_attributes.SetAt(key, value);
  if (setDirty)
    SetDirty();
}


bool PXMLElement::HasAttribute(const PCaselessString & key) const
{
  return m_attributes.Contains(key);
}


PINDEX PXMLElement::GetObjectCount() const
{
  PINDEX count = 1;
  for (PINDEX i = 0; i < m_subObjects.GetSize(); i++) 
    count += m_subObjects[i].GetObjectCount();
  return count;
}


void PXMLElement::PrintOn(ostream & strm) const
{
  PXMLBase xml;
  Output(strm, xml, 0);
}


void PXMLElement::Output(ostream & strm, const PXMLBase & xml, int indent) const
{
  xml.OutputProgress();

  PString elementName;
  if (m_parent != NULL)
    elementName = m_parent->GetName();
  bool newLine = xml.OutputIndent(strm, indent, elementName);

  strm << '<' << m_name;

  if (m_attributes.GetSize() > 0) {
    for (PStringToString::const_iterator it = m_attributes.begin(); it != m_attributes.end(); ++it)
      strm << ' ' << it->first << "=\"" << it->second << '"';
  }

  // this ensures empty elements use the shortened form
  if (m_subObjects.IsEmpty())
    strm << "/>";
  else {
    strm << '>';

    if (!m_subObjects.IsEmpty()) {
      if (m_subObjects.GetSize() == 1 && !m_subObjects[0].IsElement())
        m_subObjects[0].Output(strm, xml, 0);
      else {
        if (newLine)
          strm << endl;

        for (PINDEX i = 0; i < m_subObjects.GetSize(); i++)
          m_subObjects[i].Output(strm, xml, indent + 1);

        xml.OutputIndent(strm, indent, elementName);
      }
    }
    strm << "</" << m_name << '>';
  }

  if (newLine)
    strm << endl;
}


PXMLObject * PXMLElement::AddSubObject(PXMLObject * obj, bool setDirty)
{
  if (PAssertNULL(obj) == NULL)
    return NULL;

  if (obj->SetParent(this))
    m_subObjects.SetAt(m_subObjects.GetSize(), obj);

  if (setDirty)
    SetDirty();

  return obj;
}


PXMLElement * PXMLElement::CreateElement(const PCaselessString & name, const char * data)
{
  return m_parent != NULL ? m_parent->CreateElement(name, data) : new PXMLElement(name, data);
}


PXMLElement * PXMLElement::AddElement(const char * name)
{
  return static_cast<PXMLElement *>(AddSubObject(CreateElement(name)));
}


PXMLElement * PXMLElement::AddElement(const PString & name, const PString & data)
{
  return static_cast<PXMLElement *>(AddSubObject(CreateElement(name, data)));
}


PXMLElement * PXMLElement::AddElement(const PString & name, const PString & attrName, const PString & attrVal)
{
  PXMLElement * element = static_cast<PXMLElement *>(AddSubObject(CreateElement(name)));
  element->SetAttribute(attrName, attrVal);
  return element;
}


PXMLObject * PXMLElement::Clone() const
{
  return new PXMLElement(*this);
}


PString PXMLElement::GetData(bool trim) const
{
  PString str;

  for (PINDEX i = 0; i < m_subObjects.GetSize(); i++) {
    PXMLData * data = dynamic_cast<PXMLData *>(m_subObjects.GetAt(i));
    if (data != NULL) {
      if (trim) {
        PStringArray lines = data->GetString().Lines();
        for (PINDEX j = 0; j < lines.GetSize(); j++)
          str &= lines[j];
      }
      else
        str += data->GetString();
    }
  }
  return str;
}


void PXMLElement::SetData(const PString & data)
{
  for (PINDEX idx = 0; idx < m_subObjects.GetSize(); idx++) {
    if (!m_subObjects[idx].IsElement())
      m_subObjects.RemoveAt(idx--);
  }
  AddData(data);
}


PXMLData * PXMLElement::AddData(const PString & data)
{
  return static_cast<PXMLData *>(AddSubObject(new PXMLData(data)));
}


PCaselessString PXMLElement::GetPathName() const
{
    PCaselessString s;

    s = GetName();
    const PXMLElement* el = this;
    while ((el = el->GetParent()) != NULL)
        s = el->GetName() + ":" + s;
    return s;
}


void PXMLElement::AddNamespace(const PString & prefix, const PString & uri)
{
  if (prefix.IsEmpty())
    m_defaultNamespace = uri;
  else
    m_nameSpaces.SetAt(prefix, uri);
}


void PXMLElement::RemoveNamespace(const PString & prefix)
{
  if (prefix.IsEmpty())
    m_defaultNamespace.MakeEmpty();
  else
    m_nameSpaces.RemoveAt(prefix);
}


///////////////////////////////////////////////////////

PObject * PXMLRootElement::Clone()
{
  return m_document.CreateRootElement(m_name);
}


PXMLElement * PXMLRootElement::CreateElement(const PCaselessString & name, const char * data)
{
  return m_document.CreateElement(name, data);
}


///////////////////////////////////////////////////////

PXMLSettings::PXMLSettings(PXML::Options options)
  : PXML(options)
{
}


PString PXMLSettings::GetAttribute(const PCaselessString & section, const PString & key) const
{
  PXMLElement * element = GetElement(section);
  if (element == NULL)
    return PString::Empty();

  return element->GetAttribute(key);
}


void PXMLSettings::SetAttribute(const PCaselessString & section, const PString & key, const PString & value)
{
  if (m_rootElement == NULL) 
    SetRootElement("settings");

  PXMLElement * element = m_rootElement->GetElement(section);
  if (element == NULL)
    element = m_rootElement->AddElement(section);

  element->SetAttribute(key, value);
}


bool PXMLSettings::HasAttribute(const PCaselessString & section, const PString & key) const
{
  PXMLElement * element = GetElement(section);
  if (element == NULL)
    return false;

  return element->HasAttribute(key);
}


#if P_CONFIG_FILE
void PXMLSettings::ToConfig(PConfig & cfg) const
{
  for (PINDEX i = 0;i < (PINDEX)GetNumElements();++i) {
    PXMLElement * el = GetElement(i);
    PString sectionName = el->GetName();
    for (PStringToString::const_iterator it = el->GetAttributes().begin(); it != el->GetAttributes().end(); ++it)
      cfg.SetString(sectionName, it->first, it->second);
  }
}

void PXMLSettings::FromConfig(const PConfig & data)
{
  PStringList sects = data.GetSections();

  for (PStringList::iterator i = sects.begin(); i != sects.end(); ++i) {
    PStringToString keyvals = data.GetAllKeyValues(*i);
    for (PStringToString::iterator it = keyvals.begin(); it != keyvals.end(); ++it)
      SetAttribute(*i, it->first, it->second);
  }
}
#endif // P_CONFIG_FILE


///////////////////////////////////////////////////////

PXMLStreamParser::PXMLStreamParser(PXML & doc, Options options)
  : PXMLParser(doc, options, 0)
{
}


void PXMLStreamParser::EndElement(const char * name)
{
  PXMLElement * element = m_currentElement;

  PXMLParser::EndElement(name);

  if (!m_parsing)
    return;

  PINDEX i = m_document.GetRootElement()->FindObject(element);
  if (i == P_MAX_INDEX)
    return;

  messages.Enqueue(element);
  m_document.GetRootElement()->RemoveSubObject(i, false);
}


PXMLElement * PXMLStreamParser::Read(PChannel * channel)
{
  char buf[256];

  channel->SetReadTimeout(1000);

  while (m_parsing) {
    if (messages.GetSize() != 0)
      return messages.Dequeue();

    if (!channel->Read(buf, sizeof(buf) - 1) || !channel->IsOpen())
      return 0;

    buf[channel->GetLastReadCount()] = 0;

    if (!Parse(buf, channel->GetLastReadCount(), false))
      return 0;
  }

  channel->Close();
  return 0;
}

///////////////////////////////////////////////////////
#endif


#ifdef P_EXPAT
PString PXML::EscapeSpecialChars(const PString & str)
#else
namespace PXML {
PString EscapeSpecialChars(const PString & str)
#endif
{
  // http://www.w3.org/TR/2008/REC-xml-20081126/#charsets

  PStringStream escaped;

  for (PINDEX i = 0; i < str.GetLength(); ++i) {
    char c = str[i];
    switch (c) {
      case '"' :
        escaped << "&quot;";
        break;
      case '\'' :
        escaped << "&apos;";
        break;
      case '&' :
        escaped << "&amp;";
        break;
      case '<' :
        escaped << "&lt;";
        break;
      case '>' :
        escaped << "&gt;";
        break;
      case '\t' :
      case '\r' :
      case '\n' :
        escaped << c;
        break;
      default :
        if (c >= '\0' && c < ' ')
          escaped << "&#" << (unsigned)c << ';';
        else
          escaped << c;
    }
  }

  return escaped;
}

#ifndef P_EXPAT
}; // namespace PXML {
#endif
