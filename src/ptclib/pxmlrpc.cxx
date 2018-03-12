/*
 * pxmlrpc.cxx
 *
 * XML/RPC support
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
#pragma implementation "pxmlrpc.h"
#endif

#if P_XMLRPC

#include <ptclib/pxmlrpc.h>

#include <ptclib/mime.h>
#include <ptclib/http.h>


#define new PNEW


static const char NoIndentElements[] = "methodName name string int boolean double dateTime.iso8601";

static PConstString const StringType("string");
static PConstString const IntType("int");
static PConstString const DoubleType("double");
static PConstString const DateTimeType("dateTime.iso8601");
static PConstString const ArrayType("array");
static PConstString const DataType("data");

static PCriticalSection    initialiserMutex;
static PXMLRPCStructBase * initialiserInstance = NULL;


/////////////////////////////////////////////////////////////////

PXMLRPCBlock::PXMLRPCBlock()
  : PXML(PXMLParser::NoOptions, NoIndentElements)
{
  m_faultCode = P_MAX_INDEX;
  SetRootElement("methodResponse");
  m_params = NULL;
}


PXMLRPCBlock::PXMLRPCBlock(const PString & method)
  : PXML(PXMLParser::NoOptions, NoIndentElements)
{
  m_faultCode = P_MAX_INDEX;
  SetRootElement("methodCall")->AddElement("methodName", method);
  m_params = NULL;
}


PXMLRPCBlock::PXMLRPCBlock(const PString & method, const PXMLRPCStructBase & data)
  : PXML(PXMLParser::NoOptions, NoIndentElements)
{
  m_faultCode = P_MAX_INDEX;
  SetRootElement("methodCall")->AddElement("methodName", method);
  m_params = NULL;

  for (PINDEX i = 0; i < data.GetNumVariables(); i++) {
    PXMLRPCVariableBase & variable = data.GetVariable(i);
    if (variable.IsArray())
      AddParam(CreateArray(variable));
    else {
      PXMLRPCStructBase * structVar = variable.GetStruct(0);
      if (structVar != NULL)
        AddParam(*structVar);
      else
        AddParam(CreateScalar(variable.GetType(), variable.ToString(0)));
    }
  }
}


PXMLElement * PXMLRPCBlock::GetParams()
{
  if (PAssertNULL(m_rootElement) == NULL)
    return NULL;

  if (m_params == NULL)
    m_params = m_rootElement->AddElement("params");

  return m_params;
}


PXMLElement * PXMLRPCBlock::CreateValueElement(PXMLElement * element) 
{ 
  PXMLElement * value = CreateElement("value");
  value->AddSubObject(element);
  return value;
}


PXMLElement * PXMLRPCBlock::CreateScalar(const PString & type, const PString & value)
{ 
  return CreateValueElement(CreateElement(type, value));
}


PXMLElement * PXMLRPCBlock::CreateScalar(const PString & value) 
{ 
  return CreateScalar(StringType, value);
}


PXMLElement * PXMLRPCBlock::CreateScalar(int value) 
{
  return CreateScalar(IntType, PString(PString::Unsigned, value)); 
}


PXMLElement * PXMLRPCBlock::CreateScalar(double value)
{ 
  return CreateScalar(DoubleType, psprintf("%lf", value)); 
}


PXMLElement * PXMLRPCBlock::CreateDateAndTime(const PTime & time)
{
  return CreateScalar(DateTimeType, PXMLRPC::PTimeToISO8601(time)); 
}


PXMLElement * PXMLRPCBlock::CreateBinary(const PBYTEArray & data)
{
  return CreateScalar("base64", PBase64::Encode(data)); 
}


PXMLElement * PXMLRPCBlock::CreateStruct(PXMLElement * & structElement)
{
  structElement = CreateElement("struct");
  return CreateValueElement(structElement);
}


PXMLElement * PXMLRPCBlock::CreateStruct(const PStringToString & dict)
{
  return CreateStruct(dict, StringType);
}


PXMLElement * PXMLRPCBlock::CreateStruct(const PStringToString & dict, const PString & typeStr)
{
  PXMLElement * structElement = CreateElement("struct");
  PXMLElement * valueElement  = CreateValueElement(structElement);

  for (PStringToString::const_iterator it = dict.begin(); it != dict.end(); ++it)
    structElement->AddSubObject(CreateMember(it->first, CreateScalar(typeStr, it->second)));

  return valueElement;
}


PXMLElement * PXMLRPCBlock::CreateStruct(const PXMLRPCStructBase & data)
{
  PXMLElement * structElement = CreateElement("struct");
  PXMLElement * valueElement  = PXMLRPCBlock::CreateValueElement(structElement);

  PINDEX i;
  for (i = 0; i < data.GetNumVariables(); i++) {
    PXMLElement * element;
    PXMLRPCVariableBase & variable = data.GetVariable(i);

    if (variable.IsArray())
      element = CreateArray(variable);
    else {
      PXMLRPCStructBase * nested = variable.GetStruct(0);
      if (nested != NULL)
        element = CreateStruct(*nested);
      else
        element = CreateScalar(variable.GetType(), variable.ToString(0));
    }

    structElement->AddSubObject(CreateMember(variable.GetName(), element));
  }

  return valueElement;
}


PXMLElement * PXMLRPCBlock::CreateMember(const PString & name, PXMLElement * value)
{
  PXMLElement * member = CreateElement("member");
  member->AddElement("name", name);
  member->AddSubObject(value);

  return member;
}


PXMLElement * PXMLRPCBlock::CreateArray(PXMLElement * & dataElement)
{
  PXMLElement * arrayElement = CreateElement(ArrayType);
  dataElement  = CreateElement(DataType);
  arrayElement->AddSubObject(dataElement);
  return CreateValueElement(arrayElement);
}


PXMLElement * PXMLRPCBlock::CreateArray(const PStringArray & array)
{
  return CreateArray(array, StringType);
}


PXMLElement * PXMLRPCBlock::CreateArray(const PStringArray & array, const PString & typeStr)
{
  PXMLElement * arrayElement = CreateElement(ArrayType);
  PXMLElement * dataElement  = CreateElement(DataType);
  arrayElement->AddSubObject(dataElement);

  for (PINDEX i = 0; i < array.GetSize(); i++)
    dataElement->AddSubObject(CreateScalar(typeStr, array[i]));

  return CreateValueElement(arrayElement);
}


PXMLElement * PXMLRPCBlock::CreateArray(const PStringArray & array, const PStringArray & types)
{
  PXMLElement * arrayElement = CreateElement(ArrayType);
  PXMLElement * dataElement  = CreateElement(DataType);
  arrayElement->AddSubObject(dataElement);

  PINDEX i;
  for (i = 0; i < array.GetSize(); i++)
    dataElement->AddSubObject(CreateScalar(types[i], array[i]));

  return CreateValueElement(arrayElement);
}


PXMLElement * PXMLRPCBlock::CreateArray(const PArray<PStringToString> & array)
{
  PXMLElement * arrayElement = CreateElement(ArrayType);
  PXMLElement * dataElement = CreateElement(DataType);
  arrayElement->AddSubObject(dataElement);

  PINDEX i;
  for (i = 0; i < array.GetSize(); i++)
    dataElement->AddSubObject(CreateStruct(array[i]));

  return CreateValueElement(arrayElement);
}


PXMLElement * PXMLRPCBlock::CreateArray(const PXMLRPCVariableBase & array)
{
  PXMLElement * arrayElement = CreateElement(ArrayType);
  PXMLElement * dataElement = CreateElement(DataType);
  arrayElement->AddSubObject(dataElement);

  PINDEX i;
  for (i = 0; i < array.GetSize(); i++) {
    PXMLElement * element;
    PXMLRPCStructBase * structure = array.GetStruct(i);
    if (structure != NULL)
      element = CreateStruct(*structure);
    else
      element = CreateScalar(array.GetType(), array.ToString(i));
    dataElement->AddSubObject(element);
  }

  return CreateValueElement(arrayElement);
}


/////////////////////////////////////////////

void PXMLRPCBlock::AddParam(PXMLElement * parm)
{
  PXMLElement * child = (PXMLElement *)GetParams()->AddSubObject(CreateElement("param"));
  child->AddSubObject(parm);
}

void PXMLRPCBlock::AddParam(const PString & str) 
{ 
  AddParam(CreateScalar(str));
}

void PXMLRPCBlock::AddParam(int value) 
{
  AddParam(CreateScalar(value)); 
}

void PXMLRPCBlock::AddParam(double value)
{ 
  AddParam(CreateScalar(value)); 
}

void PXMLRPCBlock::AddParam(const PTime & time)
{
  AddParam(CreateDateAndTime(time)); 
}

void PXMLRPCBlock::AddBinary(const PBYTEArray & data)
{
  AddParam(CreateBinary(data)); 
}

void PXMLRPCBlock::AddParam(const PXMLRPCStructBase & data)
{
  AddParam(CreateStruct(data));
}

void PXMLRPCBlock::AddStruct(const PStringToString & dict)
{
  AddParam(CreateStruct(dict, StringType));
}

void PXMLRPCBlock::AddStruct(const PStringToString & dict, const PString & typeStr)
{
  AddParam(CreateStruct(dict, typeStr));
}

void PXMLRPCBlock::AddArray(const PStringArray & array)
{
  AddParam(CreateArray(array, StringType));
}

void PXMLRPCBlock::AddArray(const PStringArray & array, const PString & typeStr)
{
  AddParam(CreateArray(array, typeStr));
}

void PXMLRPCBlock::AddArray(const PStringArray & array, const PStringArray & types)
{
  AddParam(CreateArray(array, types));
}

void PXMLRPCBlock::AddArray(const PArray<PStringToString> & array)
{
  AddParam(CreateArray(array));
}


/////////////////////////////////////////////

PBoolean PXMLRPCBlock::ValidateResponse()
{
  // ensure root element exists and has correct name
  if (GetDocumentType() != "methodResponse") {
    SetFault(PXMLRPC::ResponseRootNotMethodResponse, "Response root not methodResponse");
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return false;
  }

  // determine if response returned
  if (m_params == NULL)
    m_params = GetElement("params");
  if (m_params == NULL)
    m_params = GetElement("fault");
  if (m_params == NULL)
    return true;

  // determine if fault
  if (m_params->GetName() == "fault") {

    // assume fault is a simple struct
    PStringToString faultInfo;
    PXMLElement * value = m_params->GetElement("value");
    if (value == NULL) {
      PStringStream txt;
      txt << "Fault does not contain value\n" << *this;
      SetFault(PXMLRPC::FaultyFault, txt);
    }
    else if (!ParseStruct(value->GetElement("struct"), faultInfo) ||
             (faultInfo.GetSize() != 2) ||
             (!faultInfo.Contains("faultCode")) ||
             (!faultInfo.Contains("faultString"))
             ) {
      PStringStream txt;
      txt << "Fault return is faulty:\n" << *this;
      SetFault(PXMLRPC::FaultyFault, txt);
      PTRACE(2, "XMLRPC\t" << GetFaultText());
      return false;
    }

    // get fault code and string
    SetFault(faultInfo["faultCode"].AsInteger(), faultInfo["faultString"]);

    return false;
  }

  // must be params
  else if (m_params->GetName() != "params") {
    SetFault(PXMLRPC::ResponseUnknownFormat, PString("Response contains unknown element") & m_params->GetName());
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return false;
  }

  return true;
}

PBoolean PXMLRPCBlock::ParseScalar(PXMLElement * valueElement, 
                                   PString & type, 
                                   PString & value)
{
  if (valueElement == NULL)
    return false;

  if (!valueElement->IsElement())
    return false;

  if (valueElement->GetName() != "value") {
    SetFault(PXMLRPC::ParamNotValue, "Scalar value does not contain value element");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return false;
  }

  for (PINDEX i = 0; i < valueElement->GetSize(); i++) {
    PXMLElement * element = valueElement->GetElement(i);
    if (element != NULL && element->IsElement()) {
      type = element->GetName();
      value = element->GetData();
      return true;
    }
  }

  SetFault(PXMLRPC::ScalarWithoutElement, "Scalar without sub-element");
  PTRACE(2, "XMLRPC\t" << GetFaultText());
  return false;
}


static PBoolean ParseStructBase(PXMLRPCBlock & block, PXMLElement * & element)
{
  if (element == NULL)
    return false;

  if (!element->IsElement())
    return false;

  if (element->GetName() == "struct")
    return true;

  if (element->GetName() != "value")
    block.SetFault(PXMLRPC::ParamNotStruct, "Param is not struct");
  else {
    element = element->GetElement("struct");
    if (element != NULL)
      return true;

    block.SetFault(PXMLRPC::ParamNotStruct, "nested structure not present");
  }

  PTRACE(2, "XMLRPC\t" << block.GetFaultText());
  return false;
}


static PXMLElement * ParseStructElement(PXMLRPCBlock & block,
                                        PXMLElement * structElement,
                                        PINDEX idx,
                                        PString & name)
{
  if (structElement == NULL)
    return NULL;

  PXMLElement * member = structElement->GetElement(idx);
  if (member == NULL)
    return NULL;

  if (!member->IsElement())
    return NULL;

  if (member->GetName() != "member") {
    PStringStream txt;
    txt << "Member " << idx << " missing";
    block.SetFault(PXMLRPC::MemberIncomplete, txt);
    PTRACE(2, "XMLRPC\t" << block.GetFaultText());
    return NULL;
  }

  PXMLElement * nameElement  = member->GetElement("name");
  PXMLElement * valueElement = member->GetElement("value");
  if ((nameElement == NULL) || (valueElement == NULL)) {
    PStringStream txt;
    txt << "Member " << idx << " incomplete";
    block.SetFault(PXMLRPC::MemberIncomplete, txt);
    PTRACE(2, "XMLRPC\t" << block.GetFaultText());
    return NULL;
  }

  if (nameElement->GetName() != "name") {
    PStringStream txt;
    txt << "Member " << idx << " unnamed";
    block.SetFault(PXMLRPC::MemberUnnamed, txt);
    PTRACE(2, "XMLRPC\t" << block.GetFaultText());
    return NULL;
  }

  name = nameElement->GetData();
  return valueElement;
}


PBoolean PXMLRPCBlock::ParseStruct(PXMLElement * structElement, 
                               PStringToString & structDict)
{
  if (!ParseStructBase(*this, structElement))
    return false;

  for (PINDEX i = 0; i < structElement->GetSize(); i++) {
    PString name;
    PXMLElement * element = ParseStructElement(*this, structElement, i, name);
    if (element != NULL) {
      PString value;
      PString type;
      if (ParseScalar(element, type, value))
        structDict.SetAt(name, value);
    }
  }

  return true;
}


PBoolean PXMLRPCBlock::ParseStruct(PXMLElement * structElement, PXMLRPCStructBase & data)
{
  if (!ParseStructBase(*this, structElement))
    return false;

  for (PINDEX i = 0; i < structElement->GetSize(); i++) {
    PString name;
    PXMLElement * element = ParseStructElement(*this, structElement, i, name);
    if (element != NULL) {
      PXMLRPCVariableBase * variable = data.GetVariable(name);
      if (variable != NULL) {
        if (variable->IsArray()) {
          if (!ParseArray(element, *variable))
            return false;
        }
        else {
          PXMLRPCStructBase * nested = variable->GetStruct(0);
          if (nested != NULL) {
            if (!ParseStruct(element, *nested))
              return false;
          }
          else {
            PString value;
            PCaselessString type;
            if (!ParseScalar(element, type, value))
              return false;

            if (type != StringType && type != variable->GetType()) {
              PTRACE(2, "RPCXML\tMember " << i << " is not of expected type: " << variable->GetType());
              return false;
            }

            variable->FromString(0, value);
          }
        }
      }
    }
  }

  return true;
}


static PXMLElement * ParseArrayBase(PXMLRPCBlock & block, PXMLElement * element)
{
  if (element == NULL)
    return NULL;

  if (!element->IsElement())
    return NULL;

  if (element->GetName() == "value")
    element = element->GetElement(ArrayType);

  if (element == NULL)
    block.SetFault(PXMLRPC::ParamNotArray, "array not present");
  else {
    if (element->GetName() != ArrayType)
      block.SetFault(PXMLRPC::ParamNotArray, "Param is not array");
    else {
      element = element->GetElement(DataType);
      if (element != NULL)
        return element;
      block.SetFault(PXMLRPC::ParamNotArray, "Array param has no data");
    }
  }

  PTRACE(2, "XMLRPC\t" << block.GetFaultText());
  return NULL;
}


PBoolean PXMLRPCBlock::ParseArray(PXMLElement * arrayElement, PStringArray & array)
{
  PXMLElement * dataElement = ParseArrayBase(*this, arrayElement);
  if (dataElement == NULL)
    return false;

  array.SetSize(dataElement->GetSize());

  PINDEX count = 0;
  for (PINDEX i = 0; i < dataElement->GetSize(); i++) {
    PString value;
    PString type;
    if (ParseScalar(dataElement->GetElement(i), type, value))
      array[count++] = value;
  }

  array.SetSize(count);
  return true;
}


PBoolean PXMLRPCBlock::ParseArray(PXMLElement * arrayElement, PArray<PStringToString> & array)
{
  PXMLElement * dataElement = ParseArrayBase(*this, arrayElement);
  if (dataElement == NULL)
    return false;

  array.SetSize(dataElement->GetSize());

  PINDEX count = 0;
  for (PINDEX i = 0; i < dataElement->GetSize(); i++) {
    PStringToString values;
    if (!ParseStruct(dataElement->GetElement(i), values))
      return false;

    array[count++] = values;
  }

  array.SetSize(count);
  return true;
}


PBoolean PXMLRPCBlock::ParseArray(PXMLElement * arrayElement, PXMLRPCVariableBase & array)
{
  PXMLElement * dataElement = ParseArrayBase(*this, arrayElement);
  if (dataElement == NULL)
    return false;

  array.SetSize(dataElement->GetSize());

  PINDEX count = 0;
  for (PINDEX i = 0; i < dataElement->GetSize(); i++) {
    PXMLElement * element = dataElement->GetElement(i);

    PXMLRPCStructBase * structure = array.GetStruct(count);
    if (structure != NULL) {
      if (ParseStruct(element, *structure))
        count++;
    }
    else {
      PString value;
      PCaselessString type;
      if (ParseScalar(element, type, value)) {
        if (type != StringType && type != array.GetType())
          PTRACE(2, "RPCXML\tArray entry " << i << " is not of expected type: " << array.GetType());
        else
          array.FromString(count++, value);
      }
    }
  }

  array.SetSize(count);
  return true;
}


PINDEX PXMLRPCBlock::GetParamCount() const
{
  if (m_params == NULL) 
    return 0;

  PINDEX count = 0;
  for (PINDEX i = 0; i < m_params->GetSize(); i++) {
    PXMLElement * element = m_params->GetElement(i);
    if (element != NULL && element->IsElement() && element->GetName() == "param")
      count++;
  }
  return count;
}


PXMLElement * PXMLRPCBlock::GetParam(PINDEX idx) const 
{ 
  if (m_params == NULL) 
    return NULL;

  PXMLElement * param = NULL;
  PINDEX i;
  PINDEX s = m_params->GetSize();
  for (i = 0; i < s; i++) {
    PXMLElement * element = m_params->GetElement(i);
    if (element != NULL && element->IsElement() && element->GetName() == "param") {
      if (idx <= 0) {
        param = element;
        break;
      }
      idx--;
    }
  }

  if (param == NULL)
    return NULL;

  return param->GetElement();
}


PBoolean PXMLRPCBlock::GetParams(PXMLRPCStructBase & data)
{
  if (m_params == NULL) 
    return false;

  // Special case to allow for server implementations that always return
  // values as a struct rather than multiple parameters.
  if (GetParamCount() == 1 &&
          (data.GetNumVariables() > 1 || data.GetVariable((PINDEX)0).GetStruct(0) == NULL)) {
    PString type, value;
    if (ParseScalar(GetParam(0), type, value) && type == "struct")
      return GetParam(0, data);
  }

  for (PINDEX i = 0; i < data.GetNumVariables(); i++) {
    PXMLRPCVariableBase & variable = data.GetVariable(i);
    if (variable.IsArray()) {
      if (!ParseArray(GetParam(i), variable))
        return false;
    }
    else {
      PXMLRPCStructBase * structure = variable.GetStruct(0);
      if (structure != NULL) {
        if (!GetParam(i, *structure))
          return false;
      }
      else {
        PString value;
        if (!GetExpectedParam(i, variable.GetType(), value))
          return false;

        variable.FromString(0, value);
      }
    }
  }

  return true;
}


PBoolean PXMLRPCBlock::GetParam(PINDEX idx, PString & type, PString & value)
{
  // get the parameter
  if (!ParseScalar(GetParam(idx), type, value)) {
    PTRACE(2, "XMLRPC\tCannot get scalar parm " << idx);
    return false;
  }

  return true;
}


PBoolean PXMLRPCBlock::GetExpectedParam(PINDEX idx, const PString & expectedType, PString & value)
{
  PString type;

  // get parameter
  if (!GetParam(idx, type, value))
    return false;

  // see if correct type
  if (!expectedType.IsEmpty() && (type != expectedType)) {
    PTRACE(2, "XMLRPC\tExpected parm " << idx << " to be " << expectedType << ", was " << type);
    return false;
  }

  return true;
}


PBoolean PXMLRPCBlock::GetParam(PINDEX idx, PString & result)
{
  return GetExpectedParam(idx, StringType, result); 
}

PBoolean PXMLRPCBlock::GetParam(PINDEX idx, int & val)
{
  PString type, result; 
  if (!GetParam(idx, type, result))
    return false;

  if ((type != "i4") && 
      (type != IntType) &&
      (type != "boolean")) {
    PTRACE(2, "XMLRPC\tExpected parm " << idx << " to be intger compatible, was " << type);
    return false;
  }

  val = result.AsInteger();
  return true;
}

PBoolean PXMLRPCBlock::GetParam(PINDEX idx, double & val)
{
  PString result; 
  if (!GetExpectedParam(idx, DoubleType, result))
    return false;

  val = result.AsReal();
  return true;
}

// 01234567890123456
// yyyyMMddThh:mm:ss

PBoolean PXMLRPCBlock::GetParam(PINDEX idx, PTime & val, int tz)
{
  PString result; 
  if (!GetExpectedParam(idx, DateTimeType, result))
    return false;

  return PXMLRPC::ISO8601ToPTime(result, val, tz);
}

PBoolean PXMLRPCBlock::GetParam(PINDEX idx, PStringArray & result)
{
  return ParseArray(GetParam(idx), result);
}

PBoolean PXMLRPCBlock::GetParam(PINDEX idx, PArray<PStringToString> & result)
{
  return ParseArray(GetParam(idx), result);
}


PBoolean PXMLRPCBlock::GetParam(PINDEX idx, PStringToString & result)
{
  return ParseStruct(GetParam(idx), result);
}


PBoolean PXMLRPCBlock::GetParam(PINDEX idx, PXMLRPCStructBase & data)
{
  return ParseStruct(GetParam(idx), data);
}


////////////////////////////////////////////////////////

PXMLRPC::PXMLRPC(const PURL & url, PXMLParser::Options opts)
  : m_url(url)
  , m_timeout(0, 10) // Seconds
  , m_options(opts)
{
}

PBoolean PXMLRPC::MakeRequest(const PString & method)
{
  PXMLRPCBlock request(method);
  PXMLRPCBlock response;

  return MakeRequest(request, response);
}

PBoolean PXMLRPC::MakeRequest(const PString & method, PXMLRPCBlock & response)
{
  PXMLRPCBlock request(method);

  return MakeRequest(request, response);
}

PBoolean PXMLRPC::MakeRequest(PXMLRPCBlock & request, PXMLRPCBlock & response)
{
  if (PerformRequest(request, response))
    return true;

  m_faultCode = response.GetFaultCode();
  m_faultText = response.GetFaultText();

  return false;
}

PBoolean PXMLRPC::MakeRequest(const PString & method, const PXMLRPCStructBase & args, PXMLRPCStructBase & reply)
{
  PXMLRPCBlock request(method, args);
  PXMLRPCBlock response;

  if (!MakeRequest(request, response))
    return false;

  if (response.GetParams(reply))
    return true;

  PTRACE(1, "XMLRPC\tParsing response failed: " << response.GetFaultText());
  return false;
}


PBoolean PXMLRPC::PerformRequest(PXMLRPCBlock & request, PXMLRPCBlock & response)
{
  // create XML version of request
  PString requestXML = request.AsString(m_options);
  if (requestXML.IsEmpty()) {
    PStringStream txt;
    txt << "Error creating request XML ("
        << request.GetErrorLine() 
        << ") :" 
        << request.GetErrorString();
    response.SetFault(PXMLRPC::CannotCreateRequestXML, txt);
    PTRACE(2, "XMLRPC\t" << response.GetFaultText());
    return false;
  }

  // make sure the request ends with a newline
  requestXML += "\n";

  // do the request
  PHTTPClient client;
  PMIMEInfo sendMIME, replyMIME;
  sendMIME.SetAt("Server", m_url.GetHostName());
  sendMIME.SetAt(PHTTP::ContentTypeTag(), "text/xml");

  PTRACE(5, "XMLRPC\tOutgoing XML/RPC:\n" << m_url << '\n' << sendMIME << requestXML);

  // apply the timeout
  client.SetReadTimeout(m_timeout);

  PString replyXML;

  // do the request
  PBoolean ok = client.PostData(m_url, sendMIME, requestXML, replyMIME, replyXML);

  PTRACE(5, "XMLRPC\tIncoming XML/RPC:\n" << replyMIME << replyXML);

  // make sure the request worked
  if (!ok) {
    PStringStream txt;
    txt << "HTTP POST failed: "
        << client.GetLastResponseCode() << ' '
        << client.GetLastResponseInfo() << '\n'
        << replyMIME << '\n'
        << replyXML;
    response.SetFault(PXMLRPC::HTTPPostFailed, txt);
    PTRACE(2, "XMLRPC\t" << response.GetFaultText());
    return false;
  }

  // parse the response
  if (!response.Load(replyXML)) {
    PStringStream txt;
    txt << "Error parsing response XML ("
        << response.GetErrorLine() 
        << ") :" 
        << response.GetErrorString() << '\n';

    PStringArray lines = replyXML.Lines();
    for (int offset = -2; offset <= 2; offset++) {
      int line = response.GetErrorLine() + offset;
      if (line >= 0 && (PINDEX)line < lines.GetSize())
        txt << lines[(PINDEX)line] << '\n';
    }

    response.SetFault(PXMLRPC::CannotParseResponseXML, txt);
    PTRACE(2, "XMLRPC\t" << response.GetFaultText());
    return false;
  }

  // validate the response
  if (!response.ValidateResponse()) {
    PTRACE(2, "XMLRPC\tValidation of response failed: " << response.GetFaultText());
    return false;
  }

  return true;
}

PBoolean PXMLRPC::ISO8601ToPTime(const PString & iso8601, PTime & val, int tz)
{
  if ((iso8601.GetLength() != 17) ||
      (iso8601[8]  != 'T') ||
      (iso8601[11] != ':') ||
      (iso8601[14] != ':'))
    return false;

  val = PTime(iso8601.Mid(15,2).AsInteger(),  // seconds
              iso8601.Mid(12,2).AsInteger(),  // minutes
              iso8601.Mid( 9,2).AsInteger(),  // hours
              iso8601.Mid( 6,2).AsInteger(),  // day
              iso8601.Mid( 4,2).AsInteger(),  // month
              iso8601.Mid( 0,4).AsInteger(),  // year
              tz
              );

  return true;
}

PString PXMLRPC::PTimeToISO8601(const PTime & time)
{
  return time.AsString("yyyyMMddThh:mm:ss");

}


/////////////////////////////////////////////////////////////////

PXMLRPCVariableBase::PXMLRPCVariableBase(const char * n, const char * t)
  : m_name(n)
  , m_type(t != NULL ? t : (const char *)StringType)
{
  PXMLRPCStructBase::GetInitialiser().AddVariable(this);
}


PXMLRPCStructBase & PXMLRPCStructBase::GetInitialiser()
{
  return *PAssertNULL(initialiserInstance);
}


PXMLRPCStructBase * PXMLRPCVariableBase::GetStruct(PINDEX) const
{
  return NULL;
}


PBoolean PXMLRPCVariableBase::IsArray() const
{
  return false;
}


PINDEX PXMLRPCVariableBase::GetSize() const
{
  return 1;
}


PBoolean PXMLRPCVariableBase::SetSize(PINDEX)
{
  return true;
}


PString PXMLRPCVariableBase::ToString(PINDEX) const
{
  PStringStream stream;
  PrintOn(stream);
  return stream;
}


void PXMLRPCVariableBase::FromString(PINDEX, const PString & str)
{
  PStringStream stream(str);
  ReadFrom(stream);
}


PString PXMLRPCVariableBase::ToBase64(PAbstractArray & data) const
{
  return PBase64::Encode(data.GetPointer(), data.GetSize());
}


void PXMLRPCVariableBase::FromBase64(const PString & str, PAbstractArray & data)
{
  PBase64 decoder;
  decoder.StartDecoding();
  decoder.ProcessDecoding(str);
  data = decoder.GetDecodedData();
}


/////////////////////////////////////////////////////////////////

PXMLRPCArrayBase::PXMLRPCArrayBase(PContainer & a, const char * n, const char * t)
  : PXMLRPCVariableBase(n, t)
  , m_array(a)
{
}


void PXMLRPCArrayBase::PrintOn(ostream & strm) const
{
  strm << setfill('\n') << m_array << setfill(' ');
}


void PXMLRPCArrayBase::Copy(const PXMLRPCVariableBase & other)
{
  m_array = ((PXMLRPCArrayBase &)other).m_array;
}


PBoolean PXMLRPCArrayBase::IsArray() const
{
  return true;
}


PINDEX PXMLRPCArrayBase::GetSize() const
{
  return m_array.GetSize();
}


PBoolean PXMLRPCArrayBase::SetSize(PINDEX sz)
{
  return m_array.SetSize(sz);
}


/////////////////////////////////////////////////////////////////

PXMLRPCArrayObjectsBase::PXMLRPCArrayObjectsBase(PArrayObjects & a, const char * n, const char * t)
  : PXMLRPCArrayBase(a, n, t)
  , m_array(a)
{
}


PBoolean PXMLRPCArrayObjectsBase::SetSize(PINDEX sz)
{
  if (!m_array.SetSize(sz))
    return false;

  for (PINDEX i = 0; i < sz; i++) {
    if (m_array.GetAt(i) == NULL) {
      PObject * object = CreateObject();
      if (object == NULL)
        return false;
      m_array.SetAt(i, object);
    }
  }

  return true;
}


PString PXMLRPCArrayObjectsBase::ToString(PINDEX i) const
{
  PStringStream stream;
  stream << *m_array.GetAt(i);
  return stream;
}


void PXMLRPCArrayObjectsBase::FromString(PINDEX i, const PString & str)
{
  PObject * object = m_array.GetAt(i);
  if (object == NULL) {
    object = CreateObject();
    m_array.SetAt(i, object);
  }

  PStringStream stream(str);
  stream >> *object;
}


/////////////////////////////////////////////////////////////////

PXMLRPCStructBase::PXMLRPCStructBase()
{
  variablesByOrder.DisallowDeleteObjects();
  variablesByName.DisallowDeleteObjects();

  initialiserMutex.Wait();
  initialiserStack = initialiserInstance;
  initialiserInstance = this;
}


void PXMLRPCStructBase::EndConstructor()
{
  initialiserInstance = initialiserStack;
  initialiserMutex.Signal();
}


PXMLRPCStructBase & PXMLRPCStructBase::operator=(const PXMLRPCStructBase & other)
{
  for (PINDEX i = 0; i < variablesByOrder.GetSize(); i++)
    variablesByOrder[i].Copy(other.variablesByOrder[i]);

  return *this;
}


void PXMLRPCStructBase::PrintOn(ostream & strm) const
{
  for (PINDEX i = 0; i < variablesByOrder.GetSize(); i++) {
    PXMLRPCVariableBase & var = variablesByOrder[i];
    strm << var.GetName() << '=' << var << '\n';
  }
}


void PXMLRPCStructBase::AddVariable(PXMLRPCVariableBase * var)
{
  variablesByOrder.Append(var);
  variablesByName.SetAt(var->GetName(), var);
}


#endif // P_XMLRPC


// End of file ///////////////////////////////////////////////////////////////

