/*
 * pxmlrpc.h
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
 * $Log: pxmlrpc.h,v $
 * Revision 1.10  2002/12/04 00:15:56  robertj
 * Large enhancement to create automatically encoding and decoding structures
 *   using macros to build a class.
 *
 * Revision 1.9  2002/11/06 22:47:24  robertj
 * Fixed header comment (copyright etc)
 *
 * Revision 1.8  2002/10/02 08:54:34  craigs
 * Added support for XMLRPC server
 *
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


class PXMLRPCBlock;
class PXMLRPCStructBase;


/////////////////////////////////////////////////////////////////

class PXMLRPC : public PObject
{
  PCLASSINFO(PXMLRPC, PObject);
  public:
    enum {
      CannotCreateRequestXML          = 100,
      CannotParseResponseXML,
      CannotParseRequestXML,
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
      RequestHasWrongDocumentType,
      RequestHasNoMethodName,
      RequestHasNoParms,
      MethodNameIsEmpty,
      UnknownMethod,

      UserFault                       = 1000,
    };

    PXMLRPC(const PURL & url);

    void SetTimeout(const PTimeInterval & _timeout) { timeout = _timeout; }

    BOOL MakeRequest(const PString & method);
    BOOL MakeRequest(const PString & method,  PXMLRPCBlock & response);
    BOOL MakeRequest(PXMLRPCBlock  & request, PXMLRPCBlock & response);

    PString GetFaultText() const { return faultText; }
    PINDEX  GetFaultCode() const { return faultCode; }

    static BOOL    ISO8601ToPTime(const PString & iso8601, PTime & val, int tz = PTime::GMT);
    static PString PTimeToISO8601(const PTime & val);

  protected:
    BOOL PerformRequest(PXMLRPCBlock & request, PXMLRPCBlock & response);

    PURL url;
    PINDEX  faultCode;
    PString faultText;
    PTimeInterval timeout;
};

/////////////////////////////////////////////////////////////////

class PXMLRPCBlock : public PXML
{
  PCLASSINFO(PXMLRPCBlock, PXML);
  public:
    PXMLRPCBlock();
    PXMLRPCBlock(const PString & method);

    BOOL Load(const PString & str);

    PXMLElement * GetParams();
    PXMLElement * GetParam(PINDEX idx) const;
    PINDEX GetParamCount() const             { return (params == NULL) ? 0 : params->GetSize(); }

    // used when used as a response
    PINDEX  GetFaultCode() const                     { return faultCode; }
    PString GetFaultText() const                     { return faultText; }
    void SetFault(PINDEX code, const PString & text) { faultCode = code; faultText = text; }
    BOOL ValidateResponse();

    // helper functions for getting parameters
    BOOL GetParam(PINDEX idx, PString & type, PString & result);
    BOOL GetExpectedParam(PINDEX idx, const PString & expectedType, PString & value);

    BOOL GetParam(PINDEX idx, PString & result);
    BOOL GetParam(PINDEX idx, int & result);
    BOOL GetParam(PINDEX idx, double & result);
    BOOL GetParam(PINDEX idx, PTime & result, int tz = PTime::GMT);
    BOOL GetParam(PINDEX idx, PStringToString & result);
    BOOL GetParam(PINDEX idx, PXMLRPCStructBase & result);

    PXMLElement * GetStructParam(PINDEX idx);

    // static functions for parsing values
    BOOL ParseScalar(PXMLElement * element, PString & type, PString & value);
    BOOL ParseStruct(PXMLElement * element, PStringToString & structDict);
    BOOL ParseStruct(PXMLElement * element, PXMLRPCStructBase & structData);
    PXMLElement * ParseStructElement(PXMLElement * structElement, PINDEX idx, PString & name);

    // static functions for creating values
    static PXMLElement * CreateValueElement(PXMLElement * element);
    static PXMLElement * CreateScalar(const PString & type, const PString & scalar);
    static PXMLElement * CreateMember(const PString & name, PXMLElement * value);

    static PXMLElement * CreateScalar(const PString & str);
    static PXMLElement * CreateScalar(int value);
    static PXMLElement * CreateScalar(double value);
    static PXMLElement * CreateDateAndTime(const PTime & time);
    static PXMLElement * CreateBinary(const PBYTEArray & data);

    static PXMLElement * CreateStruct();
    static PXMLElement * CreateStruct(const PStringToString & dict);
    static PXMLElement * CreateStruct(const PStringToString & dict, const PString & typeStr);
    static PXMLElement * CreateStruct(const PXMLRPCStructBase & structData);

    // helper functions for adding parameters
    void AddParam(PXMLElement * parm);
    void AddParam(const PString & str);
    void AddParam(int value);
    void AddParam(double value);
    void AddParam(const PTime & time);
    void AddParam(const PXMLRPCStructBase & structData);
    void AddBinary(const PBYTEArray & data);
    void AddStruct(const PStringToString & dict);
    void AddStruct(const PStringToString & dict, const PString & typeStr);

  protected:
    PXMLElement * params;
    PString faultText;
    PINDEX  faultCode;
};


/////////////////////////////////////////////////////////////////

class PXMLRPCStructBase;

class PXMLRPCVariableBase : public PObject {
    PCLASSINFO(PXMLRPCVariableBase, PObject);
  public:
    PXMLRPCVariableBase(const char * name, const char * type = NULL);

    const char * GetName() const { return name; }
    const char * GetType() const { return type; }

    virtual PString ToString() const;
    virtual void FromString(const PString & str);
    virtual PXMLRPCStructBase * GetStruct() const { return NULL; }

    PString ToBase64(PAbstractArray & data) const;
    void FromBase64(const PString & str, PAbstractArray & data);

  protected:
    const char * name;
    const char * type;
};


class PXMLRPCStructBase : public PObject {
    PCLASSINF(PXMLRPCStructBase, PObject);
  protected:
    PXMLRPCStructBase();
    void EndConstructor();

  public:
    void PrintOn(ostream & strm) const;

    PINDEX GetNumVariables() const { return variables.GetSize(); }
    PXMLRPCVariableBase & GetVariable(PINDEX idx) const { return variables.GetDataAt(idx); }
    PXMLRPCVariableBase * GetVariable(const char * name) const { return variables.GetAt(name); }

    void AddVariable(PXMLRPCVariableBase * var) { variables.SetAt(var->GetName(), var); }
    static PXMLRPCStructBase & GetInitialiser() { return *PAssertNULL(initialiserInstance); }

  protected:
    PDictionary<PString, PXMLRPCVariableBase> variables;

    PXMLRPCStructBase        * initialiserStack;
    static PMutex              initialiserMutex;
    static PXMLRPCStructBase * initialiserInstance;
};


#define PXML_STRUCT_BEGIN(name) \
  class name : public PXMLRPCStructBase { \
    public: name() { EndConstructor(); }

#define PXML_VARIABLE_CUSTOM(base, type, variable, xmltype, extras) \
    public: type variable; \
    private: struct PXmlVar_##variable : public PXMLRPCVariableBase { \
      PXmlVar_##variable() \
        : PXMLRPCVariableBase(#variable, #xmltype), \
          instance(((base &)base::GetInitialiser()).variable) \
        { } \
      virtual void PrintOn (ostream & s) const { s << instance; } \
      virtual void ReadFrom(istream & s)       { s >> instance; } \
      extras \
      type & instance; \
    } pxmlvar_##variable

#define PXML_STRUCT_END() \
  };

#define PXML_VARIABLE(base, type, variable, xmltype) \
        PXML_VARIABLE_CUSTOM(base, type, variable, xmltype, ;)

#define PXML_STRING(base, type, variable)  PXML_VARIABLE(base, type, variable, string)
#define PXML_INTEGER(base, type, variable) PXML_VARIABLE(base, type, variable, int)
#define PXML_BOOLEAN(base, type, variable) PXML_VARIABLE(base, type, variable, boolean)
#define PXML_DOUBLE(base, type, variable)  PXML_VARIABLE(base, type, variable, double)

#define PXML_DATETIME(base, type, variable) \
        PXML_VARIABLE_CUSTOM(base, type, variable, dateTime.iso8601, \
             PString ToString() const { return instance.AsString("yyyyMMddThh:mm:ss"); } )

#define PXML_BINARY(base, type, variable) \
        PXML_VARIABLE_CUSTOM(base, type, variable, base64, \
                     PString ToString() const { return ToBase64(instance); } \
                     void FromString(const PString & str) { FromBase64(str, instance); } )

#define PXML_STRUCT(base, type, variable) \
        PXML_VARIABLE_CUSTOM(base, type, variable, struct, \
                             PXMLRPCStructBase * GetStruct() const { return &instance; } )


/////////////////////////////////////////////////////////////////


#endif
