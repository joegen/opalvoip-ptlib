/*
 * pxmlrpcs.cxx
 *
 * PWLib application source file for PXMLRPCS
 *
 * Main program entry point.
 *
 * Copyright 2002 Equivalence
 *
 * $Log: pxmlrpcs.cxx,v $
 * Revision 1.3  2002/10/17 12:51:01  rogerh
 * Add a newline at the of the file to silence a gcc compiler warning.
 *
 * Revision 1.2  2002/10/10 04:43:44  robertj
 * VxWorks port, thanks Martijn Roest
 *
 * Revision 1.1  2002/10/02 08:54:01  craigs
 * Added support for XMLRPC server
 *
 */

// This depends on the expat XML library by Jim Clark
// See http://www.jclark.com/xml/expat.html for more information

#include <ptlib.h>

#ifdef __GNUC__
#pragma implementation "pxmlrpcs.h"
#endif

#define   DEFAULT_XMPRPC_URL  "/RPC2"

#include <ptclib/pxmlrpcs.h>

#if P_EXPAT

PXMLRPCServerResource::PXMLRPCServerResource()
  : PHTTPResource(DEFAULT_XMPRPC_URL)
{
}

PXMLRPCServerResource::PXMLRPCServerResource(
      const PHTTPAuthority & auth)    // Authorisation for the resource.
  : PHTTPResource(DEFAULT_XMPRPC_URL, auth)
{
}
PXMLRPCServerResource::PXMLRPCServerResource(
      const PURL & url)               // Name of the resource in URL space.
  : PHTTPResource(url)
{
}

PXMLRPCServerResource::PXMLRPCServerResource(
      const PURL & url,              // Name of the resource in URL space.
      const PHTTPAuthority & auth    // Authorisation for the resource.
    )
  : PHTTPResource(url, auth)
{
}

BOOL PXMLRPCServerResource::SetMethod(const PString & methodName, const PNotifier & func)
{
  PWaitAndSignal m(methodMutex);

  // find the method, or create a new one
  PXMLRPCServerMethod * methodInfo;
  PINDEX pos = methodList.GetValuesIndex(methodName);
  if (pos != P_MAX_INDEX)
    methodInfo = (PXMLRPCServerMethod *)methodList.GetAt(pos);
  else {
    methodInfo = new PXMLRPCServerMethod(methodName);
    methodList.Append(methodInfo);
  }

  // set the function
  methodInfo->methodFunc = func;

  return TRUE;
}

BOOL PXMLRPCServerResource::LoadHeaders(PHTTPRequest & /*request*/)    // Information on this request.
{
  return TRUE;
}

BOOL PXMLRPCServerResource::OnPOSTData(PHTTPRequest & request,
                                const PStringToString & /*data*/)
{
  PString reply;

  OnXMLRPCRequest(request.entityBody, reply);

  request.code = PHTTP::RequestOK;
  request.outMIME.SetAt(PHTTP::ContentTypeTag, "text/xml");

  PINDEX len = reply.GetLength();
  request.server.StartResponse(request.code, request.outMIME, len);
  return request.server.Write((const char *)reply, len);
}


void PXMLRPCServerResource::OnXMLRPCRequest(const PString & body, PString & reply)
{
  // get body of message here
  PXMLRPCBlock request;
  BOOL ok = request.Load(body);

  // if cannot parse XML, set return 
  if (!ok) { 
    reply = FormatFault(PXMLRPC::CannotParseRequestXML, "XML error:" + request.GetErrorString());
    return;
  }

  // make sure methodCall is specified as top level
  if ((request.GetDocumentType() != "methodCall") || (request.GetNumElements() < 1)) {
    reply = FormatFault(PXMLRPC::RequestHasWrongDocumentType, "document type is not methodCall");
    return;
  }

  // make sure methodName is speciified
  PXMLElement * methodName = request.GetElement("methodName");
  if (methodName == NULL) {
    reply = FormatFault(PXMLRPC::RequestHasNoMethodName, "methodCall has no methodName");
    return;
  }

  // make sure params are specified
  PXMLElement * params = request.GetElement("params");
  if (params == NULL) {
    reply = FormatFault(PXMLRPC::RequestHasNoParms, "methodCall has no parms");
    return;
  }

  // extract method name
  if ((methodName->GetSize() != 1) || (methodName->GetElement(0)->IsElement())) {
    reply = FormatFault(PXMLRPC::MethodNameIsEmpty, "methodName is empty");
    return;
  }
  PString method = ((PXMLData *)methodName->GetElement(0))->GetString();

  // extract params
  PTRACE(3, "XMLRPC\tReceived XMLRPC request for method " << method);

  OnXMLRPCRequest(method, request, reply);
}

void PXMLRPCServerResource::OnXMLRPCRequest(const PString & methodName, 
                                            PXMLRPCBlock & request,
                                            PString & reply)
{
  methodMutex.Wait();

  // find the method information
  PINDEX pos = methodList.GetValuesIndex(methodName);
  if (pos == P_MAX_INDEX) {
    reply = FormatFault(PXMLRPC::UnknownMethod, "unknown method " + methodName);
    return;
  }
  PXMLRPCServerMethod * methodInfo = (PXMLRPCServerMethod *)methodList.GetAt(pos);
  PNotifier notifier = methodInfo->methodFunc;
  methodMutex.Signal();

  // create paramaters
  PXMLRPCServerParms p(request);

  // call the notifier
  notifier(p, 0);

  // get the reply
  if (request.GetFaultCode() != P_MAX_INDEX)
    reply = FormatFault(request.GetFaultCode(), request.GetFaultText());
  else {
    PStringStream r; r << p.response;
    reply = r;
  }
}


PString PXMLRPCServerResource::FormatFault(PINDEX code, const PString & str)
{
  PTRACE(2, "XMLRPC\trequest failed: " << str);

  PStringStream reply;
  reply << "<?xml version=\"1.0\"?>\n"
           "<methodResponse>"
             "<fault>"
               "<value>"
                 "<struct>"
                   "<member>"
                     "<name>faultCode</name>"
                     "<value><int>" << code << "</int></value>"
                   "</member>"
                   "<member>"
                     "<name>faultString</name>"
                     "<value><string>" << str << "</string></value>"
                   "</member>"
                 "</struct>"
               "</value>"
             "</fault>"
           "</methodResponse>";
  return reply;
}

#endif

