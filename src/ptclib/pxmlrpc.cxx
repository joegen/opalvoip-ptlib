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
 *
 * $Log: pxmlrpc.cxx,v $
 * Revision 1.20  2002/12/13 01:12:24  robertj
 * Added copy constructor and assignment operator to XML/RPC structs
 *
 * Revision 1.19  2002/12/10 03:51:17  robertj
 * Fixed member variable display in structure
 *
 * Revision 1.18  2002/12/09 04:06:44  robertj
 * Added macros for defining multi-argument functions
 *
 * Revision 1.17  2002/12/04 00:31:13  robertj
 * Fixed GNU compatibility
 *
 * Revision 1.16  2002/12/04 00:15:44  robertj
 * Changed usage of PHTTPClient so supports chunked transfer encoding.
 * Large enhancement to create automatically encoding and decoding structures
 *   using macros to build a class.
 *
 * Revision 1.15  2002/11/06 22:47:25  robertj
 * Fixed header comment (copyright etc)
 *
 * Revision 1.14  2002/10/08 12:26:31  craigs
 * Changed struct members to always contain name/value in that order
 *
 * Revision 1.13  2002/10/08 12:09:28  craigs
 * More fixes for creation of struct params
 *
 * Revision 1.12  2002/10/08 11:58:01  craigs
 * Fixed creation of struct params
 *
 * Revision 1.11  2002/10/08 11:48:37  craigs
 * Added logging of incoming and outgoing XML at highest log level
 *
 * Revision 1.10  2002/10/08 11:36:56  craigs
 * Fixed fault parsing
 *
 * Revision 1.9  2002/10/08 08:22:18  craigs
 * Fixed problem with parsing struct parameters
 *
 * Revision 1.8  2002/10/02 08:54:01  craigs
 * Added support for XMLRPC server
 *
 * Revision 1.7  2002/08/13 03:02:07  robertj
 * Removed previous fix for memory leak, as object was already deleted.
 *
 * Revision 1.6  2002/08/13 01:54:47  craigs
 * Fixed memory leak on PXMLRPCRequest class
 *
 * Revision 1.5  2002/08/06 01:04:03  robertj
 * Fixed missing pragma interface/implementation
 *
 * Revision 1.4  2002/08/02 05:42:10  robertj
 * Fixed confusion between in and out MIME.
 * Improved trace logging and error reporting.
 *
 * Revision 1.3  2002/07/12 05:51:35  craigs
 * Added structs to XMLRPC response types
 *
 * Revision 1.2  2002/03/27 00:50:29  craigs
 * Fixed problems with parsing faults and creating structs
 *
 * Revision 1.1  2002/03/26 07:06:29  craigs
 * Initial version
 *
 */

// This depends on the expat XML library by Jim Clark
// See http://www.jclark.com/xml/expat.html for more information

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "pxmlrpc.h"
#endif

#include <ptclib/pxmlrpc.h>


#if P_EXPAT

#include <ptclib/mime.h>
#include <ptclib/http.h>


/////////////////////////////////////////////////////////////////

PXMLRPCBlock::PXMLRPCBlock()
{
  faultCode = P_MAX_INDEX;
  SetRootElement("methodResponse");
  params = NULL;
}

PXMLRPCBlock::PXMLRPCBlock(const PString & method)
{
  faultCode = P_MAX_INDEX;
  SetRootElement("methodCall");
  rootElement->AddChild(new PXMLElement(rootElement, "methodName", method));
  params = NULL;
}

PXMLRPCBlock::PXMLRPCBlock(const PString & method, const PXMLRPCStructBase & data)
{
  faultCode = P_MAX_INDEX;
  SetRootElement("methodCall");
  rootElement->AddChild(new PXMLElement(rootElement, "methodName", method));
  params = NULL;

  for (PINDEX i = 0; i < data.GetNumVariables(); i++) {
    PXMLRPCVariableBase & variable = data.GetVariable(i);
    PXMLRPCStructBase * structVar = variable.GetStruct();
    if (structVar != NULL)
      AddParam(*structVar);
    else
      AddParam(CreateValueElement(new PXMLElement(NULL, variable.GetType(), variable.ToString())));
  }
}


BOOL PXMLRPCBlock::Load(const PString & str)
{
  if (!PXML::Load(str))
    return FALSE;

  if (rootElement != NULL)
    params = rootElement->GetElement("params", 0);

  return TRUE;
}


PXMLElement * PXMLRPCBlock::GetParams()
{
  if (params == NULL)
    params = rootElement->AddChild(new PXMLElement(rootElement, "params"));

  return params;
}

PXMLElement * PXMLRPCBlock::CreateValueElement(PXMLElement * element) 
{ 
  PXMLElement * value = new PXMLElement(NULL, "value");
  value->AddChild(element);
  element->SetParent(value);

  return value;
}

PXMLElement * PXMLRPCBlock::CreateScalar(const PString & type, 
                                           const PString & scalar)
{ 
  return CreateValueElement(new PXMLElement(NULL, type, scalar));
}

PXMLElement * PXMLRPCBlock::CreateScalar(const PString & str) 
{ 
  return CreateScalar("string", str);
}

PXMLElement * PXMLRPCBlock::CreateScalar(int value) 
{
  return CreateScalar("int", PString(PString::Unsigned, value)); 
}

PXMLElement * PXMLRPCBlock::CreateScalar(double value)
{ 
  return CreateScalar("double", psprintf("%lf", value)); 
}

PXMLElement * PXMLRPCBlock::CreateDateAndTime(const PTime & time)
{
  return CreateScalar("dateTime.iso8601", PXMLRPC::PTimeToISO8601(time)); 
}

PXMLElement * PXMLRPCBlock::CreateBinary(const PBYTEArray & data)
{
  return CreateScalar("base64", PBase64::Encode(data)); 
}

PXMLElement * PXMLRPCBlock::CreateStruct()
{
  PAssertAlways("Not used");
  return NULL;
}


PXMLElement * PXMLRPCBlock::CreateStruct(const PStringToString & dict)
{
  return CreateStruct(dict, "string");
}

PXMLElement * PXMLRPCBlock::CreateStruct(const PStringToString & dict, const PString & typeStr)
{
  PXMLElement * structElement = new PXMLElement(NULL, "struct");
  PXMLElement * valueElement  = CreateValueElement(structElement);

  PINDEX i;
  for (i = 0; i < dict.GetSize(); i++) {
    PString key = dict.GetKeyAt(i);
    structElement->AddChild(CreateMember(key, CreateScalar(typeStr, dict[key])));
  }

  return valueElement;
}


PXMLElement * PXMLRPCBlock::CreateStruct(const PXMLRPCStructBase & data)
{
  PXMLElement * structElement = new PXMLElement(NULL, "struct");
  PXMLElement * valueElement  = PXMLRPCBlock::CreateValueElement(structElement);

  PINDEX i;
  for (i = 0; i < data.GetNumVariables(); i++) {
    PXMLElement * element;
    PXMLRPCVariableBase & variable = data.GetVariable(i);

    PXMLRPCStructBase * nested = variable.GetStruct();
    if (nested != NULL)
      element = CreateStruct(*nested);
    else
      element = CreateScalar(variable.GetType(), variable.ToString());

    structElement->AddChild(CreateMember(variable.GetName(), element));
  }

  return valueElement;
}


PXMLElement * PXMLRPCBlock::CreateMember(const PString & name, PXMLElement * value)
{
  PXMLElement * member = new PXMLElement(NULL, "member");
  member->AddChild(new PXMLElement(member, "name", name));
  member->AddChild(value);

  return member;
}

/////////////////////////////////////////////

void PXMLRPCBlock::AddParam(PXMLElement * parm)
{
  GetParams();
  PXMLElement * child = params->AddChild(new PXMLElement(params, "param"));
  child->AddChild(parm);
  parm->SetParent(child);
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
  AddParam(CreateStruct(dict, "string"));
}

void PXMLRPCBlock::AddStruct(const PStringToString & dict, const PString & typeStr)
{
  AddParam(CreateStruct(dict, typeStr));
}

/////////////////////////////////////////////

BOOL PXMLRPCBlock::ValidateResponse()
{
  // ensure root element exists and has correct name
  if ((rootElement == NULL) || 
      (rootElement->GetName() != "methodResponse")) {
    SetFault(PXMLRPC::ResponseRootNotMethodResponse, "Response root not methodResponse");
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return FALSE;
  }

  // determine if response returned
  params = (PXMLElement *)rootElement->GetElement(0);
  if (params == NULL)
    return TRUE;

  // determine if fault
  if (params->GetName() == "fault") {

    // assume fault is a simple struct
    PStringToString faultInfo;
    PXMLElement * value = params->GetElement("value");
    if (value == NULL) {
      PStringStream txt;
      txt << "Fault does not contain value\n" << *this;
      SetFault(PXMLRPC::FaultyFault, txt);
    } else if (!ParseStruct(value->GetElement("struct"), faultInfo) ||
         (faultInfo.GetSize() != 2) ||
         (!faultInfo.Contains("faultCode")) ||
         (!faultInfo.Contains("faultString"))
         ) {
      PStringStream txt;
      txt << "Fault return is faulty:\n" << *this;
      SetFault(PXMLRPC::FaultyFault, txt);
      PTRACE(2, "XMLRPC\t" << GetFaultText());
      return FALSE;
    }

    // get fault code and string
    SetFault(faultInfo["faultCode"].AsInteger(), faultInfo["faultString"]);

    return FALSE;
  }

  // must be params
  else if (params->GetName() != "params") {
    SetFault(PXMLRPC::ResponseUnknownFormat, PString("Response contains unknown element") & params->GetName());
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return FALSE;
  }

  return TRUE;
}

BOOL PXMLRPCBlock::ParseScalar(PXMLElement * valueElement, 
                                   PString & type, 
                                   PString & value)
{
  if (valueElement == NULL)
    return FALSE;

  if (valueElement->GetName() != "value") {
    SetFault(PXMLRPC::ParamNotValue, "Scalar value does not contain value element");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  PXMLElement * element = (PXMLElement *)valueElement->GetElement(0);
  if ((element == NULL) || !element->IsElement()) {
    SetFault(PXMLRPC::ScalarWithoutElement, "Scalar without sub-element");
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return FALSE;
  }

  type = element->GetName();
  value = element->GetData();

  return TRUE;
}

BOOL PXMLRPCBlock::ParseStruct(PXMLElement * structElement, 
                               PStringToString & structDict)
{
  if ((structElement == NULL) || (structElement->GetName() != "struct")) {
    SetFault(PXMLRPC::ParamNotStruct, "Param is not struct");
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return FALSE;
  }

  for (PINDEX i = 0; i < structElement->GetSize(); i++) {
    PString name;
    PXMLElement * element = ParseStructElement(structElement, i, name);
    if (element == NULL)
      return FALSE;

    PString value;
    PString type;
    if (!ParseScalar(element, type, value))
      return FALSE;

    structDict.SetAt(name, value);
  }

  return TRUE;
}

BOOL PXMLRPCBlock::ParseStruct(PXMLElement * structElement, 
                               PXMLRPCStructBase & data)
{
  if ((structElement == NULL) || (structElement->GetName() != "struct")) {
    SetFault(PXMLRPC::ParamNotStruct, "Param is not struct");
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return FALSE;
  }

  for (PINDEX i = 0; i < structElement->GetSize(); i++) {
    PString name;
    PXMLElement * element = ParseStructElement(structElement, i, name);
    if (element == NULL)
      return FALSE;

    PXMLRPCVariableBase * variable = data.GetVariable(name);
    if (variable != NULL) {
      PXMLRPCStructBase * nested = variable->GetStruct();
      if (nested != NULL) {
        PXMLElement * nestedElement = (PXMLElement *)element->GetElement(0);
        if ((nestedElement == NULL) || !nestedElement->IsElement()) {
          SetFault(PXMLRPC::ScalarWithoutElement, "Scalar without sub-element");
          PTRACE(2, "XMLRPC\t" << GetFaultText());
          return FALSE;
        }

        if (nestedElement->GetName() != "struct") {
          PTRACE(2, "RPCXML\tMember " << i << " is not struct as expected");
          return FALSE;
        }

        if (!ParseStruct(nestedElement, *nested))
          return FALSE;
      }
      else {
        PString value;
        PCaselessString type;
        if (!ParseScalar(element, type, value))
          return FALSE;

        if (type != variable->GetType()) {
          PTRACE(2, "RPCXML\tMember " << i << " is not of expected type: " << variable->GetType());
          return FALSE;
        }

        variable->FromString(value);
      }
    }
  }

  return TRUE;
}

PXMLElement * PXMLRPCBlock::ParseStructElement(PXMLElement * structElement,
                                               PINDEX idx,
                                               PString & name)
{
  if (structElement == NULL)
    return NULL;

  PXMLElement * member = structElement->GetElement("member", idx);
  if (member == NULL) {
    PStringStream txt;
    txt << "Member " << idx << " missing";
    SetFault(PXMLRPC::MemberIncomplete, txt);
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return NULL;
  }

  PXMLElement * nameElement  = member->GetElement("name");
  PXMLElement * valueElement = member->GetElement("value");
  if ((nameElement == NULL) || (valueElement == NULL)) {
    PStringStream txt;
    txt << "Member " << idx << " incomplete";
    SetFault(PXMLRPC::MemberIncomplete, txt);
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return NULL;
  }

  if (nameElement->GetName() != "name") {
    PStringStream txt;
    txt << "Member " << idx << " unnamed";
    SetFault(PXMLRPC::MemberUnnamed, txt);
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return NULL;
  }

  name = nameElement->GetData();
  return valueElement;
}

PXMLElement * PXMLRPCBlock::GetParam(PINDEX idx) const 
{ 
  if (params == NULL) 
    return NULL;
  
  PXMLElement * param = params->GetElement("param", idx); 
  if (param == NULL)
    return FALSE;

  PXMLObject * parm = param->GetElement(0);
  if (!parm->IsElement())
    return NULL;

  return (PXMLElement *)parm;
}


BOOL PXMLRPCBlock::GetParams(PXMLRPCStructBase & data)
{
  if (params == NULL) 
    return FALSE;
  
  if (GetParamCount() == 1) {
    PString type, value;
    if (ParseScalar(GetParam(0), type, value) && type == "struct")
      return GetParam(0, data);
  }

  for (PINDEX i = 0; i < data.GetNumVariables(); i++) {
    PXMLRPCVariableBase & variable = data.GetVariable(i);
    PXMLRPCStructBase * structVar = variable.GetStruct();
    if (structVar != NULL) {
      if (!GetParam(i, *structVar))
        return FALSE;
    }
    else {
      PString value;
      if (!GetExpectedParam(i, variable.GetType(), value))
        return FALSE;

      variable.FromString(value);
    }
  }

  return TRUE;
}


BOOL PXMLRPCBlock::GetParam(PINDEX idx, PString & type, PString & value)
{
  // get the parameter
  if (!ParseScalar(GetParam(idx), type, value)) {
    PTRACE(3, "XMLRPC\tCannot get scalar parm " << idx);
    return FALSE;
  }

  return TRUE;
}


BOOL PXMLRPCBlock::GetExpectedParam(PINDEX idx, const PString & expectedType, PString & value)
{
  PString type;

  // get parameter
  if (!GetParam(idx, type, value))
    return FALSE;

  // see if correct type
  if (!expectedType.IsEmpty() && (type != expectedType)) {
    PTRACE(3, "XMLRPC\tExpected parm " << idx << " to be " << expectedType << ", was " << type);
    return FALSE;
  }

  return TRUE;
}


BOOL PXMLRPCBlock::GetParam(PINDEX idx, PString & result)
{
  return GetExpectedParam(idx, "string", result); 
}

BOOL PXMLRPCBlock::GetParam(PINDEX idx, int & val)
{
  PString type, result; 
  if (!GetExpectedParam(idx, type, result))
    return FALSE;

  if ((type != "i4") && 
      (type != "int") &&
      (type != "boolean"))
    return FALSE;

  val = result.AsInteger();
  return TRUE;
}

BOOL PXMLRPCBlock::GetParam(PINDEX idx, double & val)
{
  PString result; 
  if (!GetExpectedParam(idx, "double", result))
    return FALSE;

  val = result.AsReal();
  return TRUE;
}

// 01234567890123456
// yyyyMMddThh:mm:ss

BOOL PXMLRPCBlock::GetParam(PINDEX idx, PTime & val, int tz)
{
  PString result; 
  if (!GetExpectedParam(idx, "dateTime.iso8601", result))
    return FALSE;

  return PXMLRPC::ISO8601ToPTime(result, val, tz);
}

BOOL PXMLRPCBlock::GetParam(PINDEX idx, PStringToString & result)
{
  PXMLElement * element = GetStructParam(idx);
  if (element == NULL)
    return FALSE;

  return ParseStruct(element, result);
}

BOOL PXMLRPCBlock::GetParam(PINDEX idx, PXMLRPCStructBase & data)
{
  PXMLElement * element = GetStructParam(idx);
  if (element == NULL)
    return FALSE;

  return ParseStruct(element, data);
}


PXMLElement * PXMLRPCBlock::GetStructParam(PINDEX idx)
{
  PXMLElement * param = GetParam(idx);
  if (param == NULL) {
    PTRACE(3, "XMLRPC\tCannot get struct parm " << idx);
    return NULL;
  }

  if (param->GetName() != "value") {
    SetFault(PXMLRPC::ParamNotValue, "Struct value does not contain value element");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return NULL;
  }

  PXMLElement * element = (PXMLElement *)param->GetElement(0);
  if ((element == NULL) || !element->IsElement()) {
    SetFault(PXMLRPC::ScalarWithoutElement, "Scalar without sub-element");
    PTRACE(2, "XMLRPC\t" << GetFaultText());
    return NULL;
  }

  if (element->GetName() != "struct") {
    PTRACE(2, "RPCXML\tParam " << idx << " is not struct as expected");
    return NULL;
  }

  return element;
}


////////////////////////////////////////////////////////

PXMLRPC::PXMLRPC(const PURL & _url)
  : url(_url)
{
  timeout = 10000;
}

BOOL PXMLRPC::MakeRequest(const PString & method)
{
  PXMLRPCBlock request(method);
  PXMLRPCBlock response;

  return MakeRequest(request, response);
}

BOOL PXMLRPC::MakeRequest(const PString & method, PXMLRPCBlock & response)
{
  PXMLRPCBlock request(method);

  return MakeRequest(request, response);
}

BOOL PXMLRPC::MakeRequest(PXMLRPCBlock & request, PXMLRPCBlock & response)
{
  if (PerformRequest(request, response))
    return TRUE;

  faultCode = response.GetFaultCode();
  faultText = response.GetFaultText();

  return FALSE;
}

BOOL PXMLRPC::MakeRequest(const PString & method, const PXMLRPCStructBase & args, PXMLRPCStructBase & reply)
{
  PXMLRPCBlock request(method, args);
  PXMLRPCBlock response;

  if (!MakeRequest(request, response))
    return FALSE;

  return response.GetParams(reply);
}


BOOL PXMLRPC::PerformRequest(PXMLRPCBlock & request, PXMLRPCBlock & response)
{
  // create XML version of request
  PString requestXML;
  if (!request.Save(requestXML)) {
    PStringStream txt;
    txt << "Error creating request XML ("
        << request.GetErrorLine() 
        << ") :" 
        << request.GetErrorString();
    response.SetFault(PXMLRPC::CannotCreateRequestXML, txt);
    PTRACE(2, "XMLRPC\t" << response.GetFaultText());
    return FALSE;
  }

  // make sure the request ends with a newline
  requestXML += "\n";

  // do the request
  PHTTPClient client;
  PMIMEInfo sendMIME, replyMIME;
  sendMIME.SetAt("Server",              url.GetHostName());
  sendMIME.SetAt(PHTTP::ContentTypeTag, "text/xml");

  PTRACE(5, "XMLRPC\tOutgoing XML/RPC:\n" << url << '\n' << sendMIME << requestXML);

  // apply the timeout
  client.SetReadTimeout(timeout);

  PString replyXML;

  // do the request
  BOOL ok = client.PostData(url, sendMIME, requestXML, replyMIME, replyXML);

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
    return FALSE;
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
      if (line >= 0 && line < lines.GetSize())
        txt << lines[(PINDEX)line] << '\n';
    }

    response.SetFault(PXMLRPC::CannotParseResponseXML, txt);
    PTRACE(2, "XMLRPC\t" << response.GetFaultText());
    return FALSE;
  }

  // validate the response
  if (!response.ValidateResponse()) {
    PTRACE(2, "XMLRPC\t" << response.GetFaultText());
    return FALSE;
  }

  return TRUE;
}

BOOL PXMLRPC::ISO8601ToPTime(const PString & iso8601, PTime & val, int tz)
{
  if ((iso8601.GetLength() != 17) ||
      (iso8601[8]  != 'T') ||
      (iso8601[11] != ':') ||
      (iso8601[14] != ':'))
    return FALSE;

  val = PTime(iso8601.Mid(15,2).AsInteger(),  // seconds
              iso8601.Mid(12,2).AsInteger(),  // minutes
              iso8601.Mid( 9,2).AsInteger(),  // hours
              iso8601.Mid( 6,2).AsInteger(),  // day
              iso8601.Mid( 4,2).AsInteger(),  // month
              iso8601.Mid( 0,4).AsInteger(),  // year
              tz
              );

  return TRUE;
}

PString PXMLRPC::PTimeToISO8601(const PTime & time)
{
  return time.AsString("yyyyMMddThh:mm:ss");

}


/////////////////////////////////////////////////////////////////

PXMLRPCVariableBase::PXMLRPCVariableBase(const char * n, const char * t)
  : name(n),
    type(t != NULL ? t : "string")
{
  PXMLRPCStructBase::GetInitialiser().AddVariable(this);
}


PString PXMLRPCVariableBase::ToString() const
{
  PStringStream stream;
  PrintOn(stream);
  return stream;
}


void PXMLRPCVariableBase::FromString(const PString & str)
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

PMutex              PXMLRPCStructBase::initialiserMutex;
PXMLRPCStructBase * PXMLRPCStructBase::initialiserInstance = NULL;


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


#endif 


// End of file ///////////////////////////////////////////////////////////////

