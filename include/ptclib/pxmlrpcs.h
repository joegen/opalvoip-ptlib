/*
 * pxmlrpcs.h
 *
 * PWLib application header file for PXMLRPCS
 *
 * Copyright 2002 Equivalence
 *
 * $Log: pxmlrpcs.h,v $
 * Revision 1.1  2002/10/02 08:54:34  craigs
 * Added support for XMLRPC server
 *
 */

#ifndef _PXMLRPCSRVR_H
#define _PXMLRPCSRVR_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/pxmlrpc.h>
#include <ptclib/http.h>

class PXMLRPCServerMethod : public PString
{
  PCLASSINFO(PXMLRPCServerMethod, PString);
  public:
    PXMLRPCServerMethod(const PString & name)
      : PString(name) { }

    PNotifier methodFunc;
};

class PXMLRPCServerParms : public PObject 
{
  PCLASSINFO(PXMLRPCServerParms, PObject);
  public:
    PXMLRPCServerParms(PXMLRPCBlock & _request)
      : request(_request) { }

    PXMLRPCBlock & request;
    PXMLRPCBlock response;
};

PSORTED_LIST(PXMLRPCServerMethodList, PXMLRPCServerMethod);

class PXMLRPCServerResource : public PHTTPResource
{
  PCLASSINFO(PXMLRPCServerResource, PHTTPResource);
  public:
    PXMLRPCServerResource();
    PXMLRPCServerResource(
      const PHTTPAuthority & auth    // Authorisation for the resource.
    );
    PXMLRPCServerResource(
      const PURL & url               // Name of the resource in URL space.
    );
    PXMLRPCServerResource(
      const PURL & url,              // Name of the resource in URL space.
      const PHTTPAuthority & auth    // Authorisation for the resource.
    );

    // overrides from PHTTPResource
    BOOL LoadHeaders(PHTTPRequest & request);
    BOOL OnPOSTData(PHTTPRequest & request, const PStringToString & data);

    // new functions
    virtual void OnXMLRPCRequest(const PString & body, PString & reply);
    virtual BOOL SetMethod(const PString & methodName, const PNotifier & func);
    void OnXMLRPCRequest(const PString & methodName, PXMLRPCBlock & request, PString & reply);

    virtual PString FormatFault(PINDEX code, const PString & str);

  protected:
    PMutex methodMutex;
    PXMLRPCServerMethodList methodList;
};

#endif


