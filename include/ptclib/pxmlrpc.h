/*
 * pxmlrpc.h
 *
 * PWLib application header file for PXMLRPC
 *
 * Copyright 2002 Equivalence
 *
 * $Log: pxmlrpc.h,v $
 * Revision 1.7  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.6  2002/08/13 03:02:07  robertj
 * Removed previous fix for memory leak, as object was already deleted.
 *
 * Revision 1.5  2002/08/13 01:55:00  craigs
 * Fixed memory leak on PXMLRPCRequest class
 *
 * Revision 1.4  2002/08/06 01:04:03  robertj
 * Fixed missing pragma interface/implementation
 *
 * Revision 1.3  2002/07/12 05:51:14  craigs
 * Added structs to XMLRPC response types
 *
 * Revision 1.2  2002/03/27 00:50:44  craigs
 * Fixed problems with parsing faults and creating structs
 *
 * Revision 1.1  2002/03/26 07:06:50  craigs
 * Initial version
 *
 */

#ifndef _PXMLRPC_H
#define _PXMLRPC_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/pxml.h>
#include <ptclib/url.h>


class PXMLRPCStruct;

class PXMLRPCElement : public PXMLElement
{
  PCLASSINFO(PXMLRPCElement, PXMLElement);

  public:
    PXMLRPCElement(PXMLElement * parent, const PString & name, const PString & subName);

    void AddParam(const PString & str);
    void AddParam(int value);
    void AddParam(double value);
    void AddParam(const PTime & time);

    void AddBinaryParam(const PString & str);
    void AddBinaryParam(const char * cstr);
    void AddBinaryParam(const PBYTEArray & data);
    void AddBinaryParam(const void * dataBlock, PINDEX length);

    void AddArrayParam(const PStringArray & array, const char * type = NULL);

    void AddStructParam(const PStringToString & dict, const char * type = NULL);
    void AddStructParam(PXMLRPCStruct * structParam);

  protected:
    PXMLElement * AddParam(PXMLElement * element);
    PString subName;
};

/////////////////////////////////////////////////////////////////

class PXMLRPCParams : public PXMLRPCElement
{
  PCLASSINFO(PXMLRPCParams, PXMLRPCElement);
  public:
    PXMLRPCParams(PXMLElement * parent = NULL)
      : PXMLRPCElement(parent, "params", "param")
    { }
};

/////////////////////////////////////////////////////////////////

class PXMLRPCScalarElement : public PXMLElement
{
  PCLASSINFO(PXMLRPCScalarElement, PXMLElement);
  public:
    PXMLRPCScalarElement(PXMLElement * parent, 
                       const PString & value,
                       const char * typeStr = NULL);
};

/////////////////////////////////////////////////////////////////

class PXMLRPCArrayElement : public PXMLElement
{
  PCLASSINFO(PXMLRPCArrayElement, PXMLElement);
  public:
    PXMLRPCArrayElement(PXMLElement * parent, 
                       const PStringArray & array,
                       const char * typeStr = NULL);
};

/////////////////////////////////////////////////////////////////

class PXMLRPCStructElement : public PXMLElement
{
  PCLASSINFO(PXMLRPCStructElement, PXMLElement);
  public:
    PXMLRPCStructElement(PXMLElement * parent, 
               const PStringToString & dict, 
                          const char * type = NULL);
};

/////////////////////////////////////////////////////////////////

class PXMLRPCRequest : public PXML
{
  PCLASSINFO(PXMLRPCRequest, PXML);
  public:
    PXMLRPCRequest(const PString & method);

    PXMLRPCParams * GetParams()  { return params; }

  protected:
    PXMLRPCParams * params;
};

/////////////////////////////////////////////////////////////////

class PXMLRPCResponse : public PXML
{
  PCLASSINFO(PXMLRPCResponse, PXML);
  public:
    enum {
      CannotCreateRequestXML          = 100,
      CannotParseResponseXML,
      HTTPPostFailed,
      CannotReadResponseContentBody,
      ResponseRootNotMethodResponse,
      ResponseEmpty,
      ResponseUnknownFormat,
      ParamNotValue,
      ScalarWithoutElement,
      ParamNotStruct,
      MemberIncomplete,
      MemberUnnamed,
      FaultyFault,

      UserFault                       = 1000,
    };

    PXMLRPCResponse();

    BOOL GetParam(PINDEX idx, PString & result);
    BOOL GetParam(PINDEX idx, int & result);
    BOOL GetParam(PINDEX idx, double & result);
    BOOL GetParam(PINDEX idx, PTime & result, int tz = PTime::GMT);
    BOOL GetParam(PINDEX idx, PString & type, PString & result);
    BOOL GetParam(PINDEX idx, PStringToString & result);

    BOOL GetBinaryParam(PINDEX idx, PString & str);
    BOOL GetBinaryParam(PINDEX idx, PBYTEArray & data);
    BOOL GetBinaryParam(PINDEX idx, void * dataBlock, PINDEX & length);

    PINDEX GetParamCount() const   { return (params == 0) ? 0 : params->GetSize(); }

    PINDEX  GetFaultCode() const { return faultCode; }
    PString GetFaultText() const { return faultText; }

    void SetFault(PINDEX code, const PString & text) 
      { faultCode = code; faultText = text; }

    BOOL Validate();

  protected:
    BOOL GetExpectedParam(PINDEX idx, const PString & expectedType, PString & result);

    BOOL ParseScalar(PXMLElement & valueElement, PString & type, PString & value);
    BOOL ParseStruct(PXMLElement & valueElement, PStringToString & structDict);

    PString faultText;
    PINDEX  faultCode;
    PXMLElement * params;
};

/////////////////////////////////////////////////////////////////

class PXMLRPC : public PObject
{
  PCLASSINFO(PXMLRPC, PObject);
  public:
    PXMLRPC(const PURL & url);

    void SetTimeout(const PTimeInterval & _timeout) { timeout = _timeout; }

    BOOL MakeRequest(const PString & method);
    BOOL MakeRequest(const PString & method, PXMLRPCResponse & response);
    BOOL MakeRequest(PXMLRPCRequest & request, PXMLRPCResponse & response);

    PString GetFaultText() const { return faultText; }
    PINDEX  GetFaultCode() const { return faultCode; }

    static BOOL    ISO8601ToPTime(const PString & iso8601, PTime & val, int tz = PTime::GMT);
    static PString PTimeToISO8601(const PTime & val);

  protected:
    BOOL PerformRequest(PXMLRPCRequest & request, PXMLRPCResponse & response);

    PURL url;
    PINDEX  faultCode;
    PString faultText;
    PTimeInterval timeout;
};

#endif
