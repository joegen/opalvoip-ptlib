/*
 * pxmlrpc.cxx
 *
 * PWLib application source file for PXMLRPC
 *
 * Main program entry point.
 *
 * Copyright 2002 Equivalence
 *
 * $Log: pxmlrpc.cxx,v $
 * Revision 1.2  2002/03/27 00:50:29  craigs
 * Fixed problems with parsing faults and creating structs
 *
 * Revision 1.1  2002/03/26 07:06:29  craigs
 * Initial version
 *
 */

// This depends on the expat XML library by Jim Clark
// See http://www.jclark.com/xml/expat.html for more information

#ifdef __GNUC__
#pragma implementation "pxmlrpc.h"
#endif

#include <ptlib.h>

#if P_EXPAT

#include <ptclib/mime.h>
#include <ptclib/http.h>
#include <ptclib/pxmlrpc.h>

PXMLRPCRequest::PXMLRPCRequest(const PString & method)
  : PXML()
{
  rootElement = new PXMLElement(NULL, "methodCall");
  rootElement->AddChild(         new PXMLElement  (rootElement, "methodName", method));
  rootElement->AddChild(params = new PXMLRPCParams(rootElement));
}

////////////////////////////////////////////////////////

PXMLRPCElement::PXMLRPCElement(PXMLElement * parent, 
                             const PString & name,
                             const PString & _subName)
  : PXMLElement(parent, name)
{
  subName = _subName;
}

PXMLElement * PXMLRPCElement::AddParam(PXMLElement * element) 
{ 
  PXMLElement * parm = AddChild(new PXMLElement(this, subName));
  parm->AddChild(element); 
  element->SetParent(parm);
  return parm;
}

void PXMLRPCElement::AddParam(const PString & str) 
{ 
  AddParam(new PXMLRPCScalarElement(NULL, str, "string"));
}

void PXMLRPCElement::AddParam(int value) 
{ 
  AddParam(new PXMLRPCScalarElement(NULL, PString(PString::Unsigned, value), "int")); 
}

void PXMLRPCElement::AddParam(double value)
{ 
  AddParam(new PXMLRPCScalarElement(NULL, psprintf("%lf", value), "double")); 
}

void PXMLRPCElement::AddParam(const PTime & time)
{
  AddParam(new PXMLRPCScalarElement(NULL, PXMLRPC::PTimeToISO8601(time), "dateTime.iso8601")); 
}

void PXMLRPCElement::AddBinaryParam(const PString & str)
{
  AddParam(new PXMLRPCScalarElement(NULL, PBase64::Encode(str), "base64")); 
}

void PXMLRPCElement::AddBinaryParam(const char * cstr)
{
  AddParam(new PXMLRPCScalarElement(NULL, PBase64::Encode(cstr), "base64")); 
}

void PXMLRPCElement::AddBinaryParam(const PBYTEArray & data)
{
  AddParam(new PXMLRPCScalarElement(NULL, PBase64::Encode(data), "base64")); 
}

void PXMLRPCElement::AddBinaryParam(const void * dataBlock, PINDEX length)
{
  AddParam(new PXMLRPCScalarElement(NULL, PBase64::Encode(dataBlock, length), "base64")); 
}

void PXMLRPCElement::AddArrayParam(const PStringArray & array, const char * type)
{ 
  AddParam(new PXMLRPCArrayElement(NULL, array, type)); 
}

void PXMLRPCElement::AddStructParam(const PStringToString & dict, const char * type)
{
  AddParam(new PXMLRPCStructElement(NULL, dict, type)); 
}

void PXMLRPCElement::AddStructParam(PXMLRPCStruct * /*structParam*/)
{
}

////////////////////////////////////////////////////////

PXMLRPCScalarElement::PXMLRPCScalarElement(PXMLElement * parent, 
                                         const PString & scalar,
                                            const char * typeStr)
  : PXMLElement(parent, "value")
{
  PString type = "string";
  if (typeStr != NULL)
    type = typeStr;

  AddChild(new PXMLElement(this, type, scalar));
}

////////////////////////////////////////////////////////

PXMLRPCArrayElement::PXMLRPCArrayElement(PXMLElement * parent, 
                                  const PStringArray & stringArray,
                                          const char * typeStr)
  : PXMLElement(parent, "value")
{
  PXMLElement * array =        AddChild(new PXMLElement(this,  "array")); 
  PXMLElement * data  = array->AddChild(new PXMLElement(array, "data" )); 

  PINDEX i;
  for (i = 0; i < stringArray.GetSize(); i++) 
    data->AddChild(new PXMLRPCScalarElement(data, stringArray[i], typeStr));
}

////////////////////////////////////////////////////////

PXMLRPCStructElement::PXMLRPCStructElement(PXMLElement * parent, 
             const PStringToString & dict, 
                        const char * typeStr)
  : PXMLElement(parent, "value")
{ 
  PXMLElement * structElement = AddChild(new PXMLElement(this,  "struct")); 
  PINDEX i;
  for (i = 0; i < dict.GetSize(); i++) {
    PString key = dict.GetKeyAt(i);
    PXMLElement * member = structElement->AddChild(new PXMLElement(structElement, "member"));
    member->AddChild(new PXMLElement         (member, "name",    key));
    member->AddChild(new PXMLRPCScalarElement(member, dict[key], typeStr));
  }
}

////////////////////////////////////////////////////////

PXMLRPCResponse::PXMLRPCResponse()
  : PXML()
{
  params = NULL;
  faultCode = P_MAX_INDEX;
}

BOOL PXMLRPCResponse::Validate()
{
  // ensure root element exists and has correct name
  if ((rootElement == NULL) || 
      (rootElement->GetName() != "methodResponse")) {
    SetFault(ResponseRootNotMethodResponse, "Response root not methodResponse");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  // determine if response returned
  params = (PXMLElement *)rootElement->GetElement(0);
  if (params == NULL) {
    SetFault(ResponseEmpty, "Response empty");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  // determine if fault
  if (params->GetName() == "fault") {

    // assume fault is a simple struct
    PStringToString faultInfo;
    PXMLElement * value = params->GetElement("value");
    if ((value == NULL) ||
         !ParseStruct(*value, faultInfo) ||
         (faultInfo.GetSize() != 2) ||
         (!faultInfo.Contains("faultCode")) ||
         (!faultInfo.Contains("faultString"))
         ) {
      PStringStream txt;
      txt << "Fault return is faulty:\n" << *this;
      SetFault(FaultyFault, txt);
      PTRACE(2, "RPCXML\t" << GetFaultText());
      return FALSE;
    }

    // get fault code and string
    SetFault(faultInfo["faultCode"].AsInteger(), faultInfo["faultString"]);

    return FALSE;
  }

  // must be params
  else if (params->GetName() != "params") {
    SetFault(ResponseUnknownFormat, PString("Response contains unknown element") & params->GetName());
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  return TRUE;
}

BOOL PXMLRPCResponse::ParseScalar(PXMLElement & valueElement, 
                                      PString & type, 
                                      PString & value)
{
  if (valueElement.GetName() != "value") {
    SetFault(ParamNotValue, "Scalar value does not contain value element");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  PXMLElement * element = (PXMLElement *)valueElement.GetElement(0);
  if ((element == NULL) || !element->IsElement()) {
    SetFault(ScalarWithoutElement, "Scalar without sub-element");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  type = element->GetName();
  value = element->GetData();

  return TRUE;
}

BOOL PXMLRPCResponse::ParseStruct(PXMLElement & valueElement, 
                              PStringToString & structDict)
{
  if (valueElement.GetName() != "value") {
    SetFault(ParamNotValue, "Struct value does not contain value element");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  PXMLElement * structElement = valueElement.GetElement("struct");
  if (structElement == NULL) {
    SetFault(ParamNotStruct, "Param is not struct");
    PTRACE(2, "RPCXML\t" << GetFaultText());
    return FALSE;
  }

  PINDEX i = 0;
  while (i < structElement->GetSize()) {
    PXMLElement * member = structElement->GetElement("member", i++);
    if (member != NULL) {
      PXMLElement * nameElement = member->GetElement("name");
      PXMLElement * element     = member->GetElement("value");
      if ((nameElement == NULL) || (element == NULL)) {
        PStringStream txt;
        txt << "Member " << i << " incomplete";
        SetFault(MemberIncomplete, txt);
        PTRACE(2, "RPCXML\t" << GetFaultText());
        return FALSE;
      }

      if (nameElement->GetName() != "name") {
        PStringStream txt;
        txt << "Member " << i << " unnamed";
        SetFault(MemberUnnamed, txt);
        PTRACE(2, "RPCXML\t" << GetFaultText());
        return FALSE;
      }

      PString name = nameElement->GetData();
      PString value;
      PString type;
      if (!ParseScalar(*element, type, value))
        return FALSE;

      structDict.SetAt(name, value);
    }
  }

  return TRUE;
}

BOOL PXMLRPCResponse::GetParam(PINDEX idx, PString & result)
{
  return GetExpectedParam(idx, "string", result); 
}

BOOL PXMLRPCResponse::GetParam(PINDEX idx, int & val)
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

BOOL PXMLRPCResponse::GetParam(PINDEX idx, double & val)
{
  PString type, result; 
  if (!GetExpectedParam(idx, type, result))
    return FALSE;

  if (type != "double")
    return FALSE;

  val = result.AsReal();
  return TRUE;
}

// 01234567890123456
// yyyyMMddThh:mm:ss

BOOL PXMLRPCResponse::GetParam(PINDEX idx, PTime & val, int tz)
{
  PString type, result; 
  if (!GetExpectedParam(idx, "dateTime.iso8601", result))
    return FALSE;

  return PXMLRPC::ISO8601ToPTime(result, val, tz);
}

BOOL PXMLRPCResponse::GetBinaryParam(PINDEX idx, PString & str)
{
  PString type, result; 
  if (!GetExpectedParam(idx, "base64", result))
    return FALSE;

  str = PBase64::Decode(result);

  return TRUE;
}

BOOL PXMLRPCResponse::GetBinaryParam(PINDEX idx, PBYTEArray & data)
{
  PString type, result; 
  if (!GetExpectedParam(idx, "base64", result))
    return FALSE;

  return PBase64::Decode(result, data);
}

BOOL PXMLRPCResponse::GetBinaryParam(PINDEX idx, void * dataBlock, PINDEX & length)
{
  PString type, result; 
  if (!GetExpectedParam(idx, "base64", result))
    return FALSE;

  return PBase64::Decode(result, dataBlock, length);
}

BOOL PXMLRPCResponse::GetExpectedParam(PINDEX idx, 
                              const PString & expectedType, 
                                    PString & result)
{
  PString type; 
  if (!GetParam(idx, type, result))
    return FALSE;

  return type == expectedType;
}

BOOL PXMLRPCResponse::GetParam(PINDEX idx, 
                            PString & type, 
                            PString & result)
{
  if (idx >= params->GetSize()) {
    PTRACE(2, "RPCXML\tParam " << idx << " not in response");
    return FALSE;
  }

  PXMLObject * param = params->GetElement(idx);
  if (!param->IsElement()) {
    PTRACE(2, "RPCXML\tParam " << idx << " malformed");
    return FALSE;
  }

  PXMLElement * value = ((PXMLElement *)param)->GetElement("value");
  if (value == NULL) {
    PTRACE(2, "RPCXML\tParam " << idx << " is malformed");
    return FALSE;
  }

  return ParseScalar(*value, type, result);
}

////////////////////////////////////////////////////////

PXMLRPC::PXMLRPC(const PURL & _url)
  : url(_url)
{
  timeout = 10000;
}

BOOL PXMLRPC::MakeRequest(const PString & method)
{
  PXMLRPCRequest request(method);
  PXMLRPCResponse response;

  return MakeRequest(request, response);
}

BOOL PXMLRPC::MakeRequest(const PString & method, PXMLRPCResponse & response)
{
  PXMLRPCRequest request(method);

  return MakeRequest(request, response);
}

BOOL PXMLRPC::MakeRequest(PXMLRPCRequest & request, PXMLRPCResponse & response)
{
  if (PerformRequest(request, response))
    return TRUE;

  faultCode = response.GetFaultCode();
  faultText = response.GetFaultText();

  return FALSE;
}

BOOL PXMLRPC::PerformRequest(PXMLRPCRequest & request, PXMLRPCResponse & response)
{
  // create XML version of request
  PString requestXML;
  if (!request.Save(requestXML)) {
    PStringStream txt;
    txt << "Error creating request XML ("
        << request.GetErrorLine() 
        << ") :" 
        << request.GetErrorString();
    response.SetFault(PXMLRPCResponse::CannotCreateRequestXML, txt);
    PTRACE(2, "RPCXML\t" << response.GetFaultText());
    return FALSE;
  }

  // make sure the request ends with a newline
  requestXML += "\n";

  // do the request
  PHTTPClient client;
  PMIMEInfo outMIME, inMIME;
  outMIME.SetAt("Server",              url.GetHostName());
  outMIME.SetAt(PHTTP::ContentTypeTag, "text/xml");

  // apply the timeout
  client.SetReadTimeout(timeout);

  // do the request
  BOOL ok = client.PostData(url, inMIME, requestXML, outMIME);

  // get length of response
  PINDEX contentLength;
  if (outMIME.Contains(PHTTP::ContentLengthTag))
    contentLength = (PINDEX)outMIME[PHTTP::ContentLengthTag].AsUnsigned();
  else
    contentLength = P_MAX_INDEX;

  // read the response
  PString reply = client.ReadString(contentLength);
  if (reply.IsEmpty()) {
    response.SetFault(PXMLRPCResponse::CannotReadResponseContentBody, 
                      "Cannot read response content body");
    PTRACE(2, "RPCXML\t" << response.GetFaultText());
    return FALSE;
  }

  // make sure the request worked
  if (!ok) {
    PStringStream txt;
    txt << "HTTP POST failed: " 
        << client.GetLastResponseCode() 
        << " " 
        << client.GetLastResponseInfo() 
        << "\n" 
        << outMIME 
        << "\n" 
        << reply;
    response.SetFault(PXMLRPCResponse::HTTPPostFailed, txt);
    PTRACE(2, "RPCXML\t" << response.GetFaultText());
    return FALSE;
  }

  // parse the response
  if (!response.Load(reply)) {
    PStringStream txt;
    txt << "Error parsing response XML ("
        << response.GetErrorLine() 
        << ") :" 
        << response.GetErrorString();
    response.SetFault(PXMLRPCResponse::CannotParseResponseXML, txt);
    PTRACE(2, "RPCXML\t" << response.GetFaultText());
    return FALSE;
  }

  // validate the response
  if (!response.Validate()) {
    PTRACE(2, "RPCXML\t" << response.GetFaultText());
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

#endif 
