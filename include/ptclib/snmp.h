//
// snmp.h
//
// Code automatically generated by asnparse.
//

#ifdef P_SNMP

#ifndef __PSNMP_H
#define __PSNMP_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/asner.h>
#include <ptclib/rfc1155.h>


//
// Message
//

class PSNMP_Message : public PASN_Sequence
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_Message, PASN_Sequence);
#endif
  public:
    PSNMP_Message(unsigned tag = UniversalSequence, TagClass tagClass = UniversalTagClass);

    PASN_Integer m_version;
    PASN_OctetString m_community;
    PASN_OctetString m_data;

    PINDEX GetDataLength() const;
    BOOL Decode(PASN_Stream & strm);
    void Encode(PASN_Stream & strm) const;
#ifndef PASN_NOPRINTON
    void PrintOn(ostream & strm) const;
#endif
    Comparison Compare(const PObject & obj) const;
    PObject * Clone() const;
};


//
// PDUs
//

class PSNMP_GetRequest_PDU;
class PSNMP_GetNextRequest_PDU;
class PSNMP_GetResponse_PDU;
class PSNMP_SetRequest_PDU;
class PSNMP_Trap_PDU;

class PSNMP_PDUs : public PASN_Choice
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_PDUs, PASN_Choice);
#endif
  public:
    PSNMP_PDUs(unsigned tag = 0, TagClass tagClass = UniversalTagClass);

    enum Choices {
      e_get_request,
      e_get_next_request,
      e_get_response,
      e_set_request,
      e_trap
    };

#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
    operator PSNMP_GetRequest_PDU &() const;
#else
    operator PSNMP_GetRequest_PDU &();
    operator const PSNMP_GetRequest_PDU &() const;
#endif
#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
    operator PSNMP_GetNextRequest_PDU &() const;
#else
    operator PSNMP_GetNextRequest_PDU &();
    operator const PSNMP_GetNextRequest_PDU &() const;
#endif
#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
    operator PSNMP_GetResponse_PDU &() const;
#else
    operator PSNMP_GetResponse_PDU &();
    operator const PSNMP_GetResponse_PDU &() const;
#endif
#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
    operator PSNMP_SetRequest_PDU &() const;
#else
    operator PSNMP_SetRequest_PDU &();
    operator const PSNMP_SetRequest_PDU &() const;
#endif
#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
    operator PSNMP_Trap_PDU &() const;
#else
    operator PSNMP_Trap_PDU &();
    operator const PSNMP_Trap_PDU &() const;
#endif

    BOOL CreateObject();
    PObject * Clone() const;
};


//
// VarBind
//

class PSNMP_VarBind : public PASN_Sequence
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_VarBind, PASN_Sequence);
#endif
  public:
    PSNMP_VarBind(unsigned tag = UniversalSequence, TagClass tagClass = UniversalTagClass);

    PRFC1155_ObjectName m_name;
    PRFC1155_ObjectSyntax m_value;

    PINDEX GetDataLength() const;
    BOOL Decode(PASN_Stream & strm);
    void Encode(PASN_Stream & strm) const;
#ifndef PASN_NOPRINTON
    void PrintOn(ostream & strm) const;
#endif
    Comparison Compare(const PObject & obj) const;
    PObject * Clone() const;
};


//
// VarBindList
//

class PSNMP_VarBind;

class PSNMP_VarBindList : public PASN_Array
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_VarBindList, PASN_Array);
#endif
  public:
    PSNMP_VarBindList(unsigned tag = UniversalSequence, TagClass tagClass = UniversalTagClass);

    PASN_Object * CreateObject() const;
    PSNMP_VarBind & operator[](PINDEX i) const;
    PObject * Clone() const;
};


//
// PDU
//

class PSNMP_PDU : public PASN_Sequence
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_PDU, PASN_Sequence);
#endif
  public:
    PSNMP_PDU(unsigned tag = UniversalSequence, TagClass tagClass = UniversalTagClass);

    PASN_Integer m_request_id;
    PASN_Integer m_error_status;
    PASN_Integer m_error_index;
    PSNMP_VarBindList m_variable_bindings;

    PINDEX GetDataLength() const;
    BOOL Decode(PASN_Stream & strm);
    void Encode(PASN_Stream & strm) const;
#ifndef PASN_NOPRINTON
    void PrintOn(ostream & strm) const;
#endif
    Comparison Compare(const PObject & obj) const;
    PObject * Clone() const;
};


//
// Trap-PDU
//

class PSNMP_Trap_PDU : public PASN_Sequence
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_Trap_PDU, PASN_Sequence);
#endif
  public:
    PSNMP_Trap_PDU(unsigned tag = 4, TagClass tagClass = ContextSpecificTagClass);

    PASN_ObjectId m_enterprise;
    PRFC1155_NetworkAddress m_agent_addr;
    PASN_Integer m_generic_trap;
    PASN_Integer m_specific_trap;
    PRFC1155_TimeTicks m_time_stamp;
    PSNMP_VarBindList m_variable_bindings;

    PINDEX GetDataLength() const;
    BOOL Decode(PASN_Stream & strm);
    void Encode(PASN_Stream & strm) const;
#ifndef PASN_NOPRINTON
    void PrintOn(ostream & strm) const;
#endif
    Comparison Compare(const PObject & obj) const;
    PObject * Clone() const;
};


//
// GetRequest-PDU
//

class PSNMP_GetRequest_PDU : public PSNMP_PDU
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_GetRequest_PDU, PSNMP_PDU);
#endif
  public:
    PSNMP_GetRequest_PDU(unsigned tag = 0, TagClass tagClass = ContextSpecificTagClass);

    PObject * Clone() const;
};


//
// GetNextRequest-PDU
//

class PSNMP_GetNextRequest_PDU : public PSNMP_PDU
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_GetNextRequest_PDU, PSNMP_PDU);
#endif
  public:
    PSNMP_GetNextRequest_PDU(unsigned tag = 1, TagClass tagClass = ContextSpecificTagClass);

    PObject * Clone() const;
};


//
// GetResponse-PDU
//

class PSNMP_GetResponse_PDU : public PSNMP_PDU
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_GetResponse_PDU, PSNMP_PDU);
#endif
  public:
    PSNMP_GetResponse_PDU(unsigned tag = 2, TagClass tagClass = ContextSpecificTagClass);

    PObject * Clone() const;
};


//
// SetRequest-PDU
//

class PSNMP_SetRequest_PDU : public PSNMP_PDU
{
#ifndef PASN_LEANANDMEAN
    PCLASSINFO(PSNMP_SetRequest_PDU, PSNMP_PDU);
#endif
  public:
    PSNMP_SetRequest_PDU(unsigned tag = 3, TagClass tagClass = ContextSpecificTagClass);

    PObject * Clone() const;
};


#endif // __PSNMP_H

#endif // if ! H323_DISABLE_PSNMP


// End of snmp.h
