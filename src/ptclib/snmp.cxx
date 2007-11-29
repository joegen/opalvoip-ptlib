//
// snmp.cxx
//
// Code automatically generated by asnparse.
//

#ifdef P_USE_PRAGMA
#pragma implementation "snmp.h"
#endif

#include <ptlib.h>
#include <ptclib/snmp.h>

#define new PNEW


#ifdef P_SNMP



//
// Message
//

PSNMP_Message::PSNMP_Message(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 0, PFalse, 0)
{
}


#ifndef PASN_NOPRINTON
void PSNMP_Message::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  strm << setw(indent+10) << "version = " << setprecision(indent) << m_version << '\n';
  strm << setw(indent+12) << "community = " << setprecision(indent) << m_community << '\n';
  strm << setw(indent+7) << "data = " << setprecision(indent) << m_data << '\n';
  strm << setw(indent-1) << setprecision(indent-2) << "}";
}
#endif


PObject::Comparison PSNMP_Message::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(&obj, PSNMP_Message), PInvalidCast);
#endif
  const PSNMP_Message & other = (const PSNMP_Message &)obj;

  Comparison result;

  if ((result = m_version.Compare(other.m_version)) != EqualTo)
    return result;
  if ((result = m_community.Compare(other.m_community)) != EqualTo)
    return result;
  if ((result = m_data.Compare(other.m_data)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX PSNMP_Message::GetDataLength() const
{
  PINDEX length = 0;
  length += m_version.GetObjectLength();
  length += m_community.GetObjectLength();
  length += m_data.GetObjectLength();
  return length;
}


PBoolean PSNMP_Message::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return PFalse;

  if (!m_version.Decode(strm))
    return PFalse;
  if (!m_community.Decode(strm))
    return PFalse;
  if (!m_data.Decode(strm))
    return PFalse;

  return UnknownExtensionsDecode(strm);
}


void PSNMP_Message::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  m_version.Encode(strm);
  m_community.Encode(strm);
  m_data.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * PSNMP_Message::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_Message::Class()), PInvalidCast);
#endif
  return new PSNMP_Message(*this);
}



#ifndef PASN_NOPRINTON
const static PASN_Names Names_PSNMP_PDUs[]={
      {"get_request",0}
     ,{"get_next_request",1}
     ,{"get_response",2}
     ,{"set_request",3}
     ,{"trap",4}
};
#endif
//
// PDUs
//

PSNMP_PDUs::PSNMP_PDUs(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Choice(tag, tagClass, 5, PFalse
#ifndef PASN_NOPRINTON
    ,(const PASN_Names *)Names_PSNMP_PDUs,5
#endif
)
{
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
PSNMP_PDUs::operator PSNMP_GetRequest_PDU &() const
#else
PSNMP_PDUs::operator PSNMP_GetRequest_PDU &()
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_GetRequest_PDU), PInvalidCast);
#endif
  return *(PSNMP_GetRequest_PDU *)choice;
}


PSNMP_PDUs::operator const PSNMP_GetRequest_PDU &() const
#endif
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_GetRequest_PDU), PInvalidCast);
#endif
  return *(PSNMP_GetRequest_PDU *)choice;
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
PSNMP_PDUs::operator PSNMP_GetNextRequest_PDU &() const
#else
PSNMP_PDUs::operator PSNMP_GetNextRequest_PDU &()
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_GetNextRequest_PDU), PInvalidCast);
#endif
  return *(PSNMP_GetNextRequest_PDU *)choice;
}


PSNMP_PDUs::operator const PSNMP_GetNextRequest_PDU &() const
#endif
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_GetNextRequest_PDU), PInvalidCast);
#endif
  return *(PSNMP_GetNextRequest_PDU *)choice;
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
PSNMP_PDUs::operator PSNMP_GetResponse_PDU &() const
#else
PSNMP_PDUs::operator PSNMP_GetResponse_PDU &()
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_GetResponse_PDU), PInvalidCast);
#endif
  return *(PSNMP_GetResponse_PDU *)choice;
}


PSNMP_PDUs::operator const PSNMP_GetResponse_PDU &() const
#endif
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_GetResponse_PDU), PInvalidCast);
#endif
  return *(PSNMP_GetResponse_PDU *)choice;
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
PSNMP_PDUs::operator PSNMP_SetRequest_PDU &() const
#else
PSNMP_PDUs::operator PSNMP_SetRequest_PDU &()
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_SetRequest_PDU), PInvalidCast);
#endif
  return *(PSNMP_SetRequest_PDU *)choice;
}


PSNMP_PDUs::operator const PSNMP_SetRequest_PDU &() const
#endif
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_SetRequest_PDU), PInvalidCast);
#endif
  return *(PSNMP_SetRequest_PDU *)choice;
}


#if defined(__GNUC__) && __GNUC__ <= 2 && __GNUC_MINOR__ < 9
PSNMP_PDUs::operator PSNMP_Trap_PDU &() const
#else
PSNMP_PDUs::operator PSNMP_Trap_PDU &()
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_Trap_PDU), PInvalidCast);
#endif
  return *(PSNMP_Trap_PDU *)choice;
}


PSNMP_PDUs::operator const PSNMP_Trap_PDU &() const
#endif
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(PAssertNULL(choice), PSNMP_Trap_PDU), PInvalidCast);
#endif
  return *(PSNMP_Trap_PDU *)choice;
}


PBoolean PSNMP_PDUs::CreateObject()
{
  switch (tag) {
    case e_get_request :
      choice = new PSNMP_GetRequest_PDU();
      return PTrue;
    case e_get_next_request :
      choice = new PSNMP_GetNextRequest_PDU();
      return PTrue;
    case e_get_response :
      choice = new PSNMP_GetResponse_PDU();
      return PTrue;
    case e_set_request :
      choice = new PSNMP_SetRequest_PDU();
      return PTrue;
    case e_trap :
      choice = new PSNMP_Trap_PDU();
      return PTrue;
  }

  choice = NULL;
  return PFalse;
}


PObject * PSNMP_PDUs::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_PDUs::Class()), PInvalidCast);
#endif
  return new PSNMP_PDUs(*this);
}


//
// VarBind
//

PSNMP_VarBind::PSNMP_VarBind(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 0, PFalse, 0)
{
}


#ifndef PASN_NOPRINTON
void PSNMP_VarBind::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  strm << setw(indent+7) << "name = " << setprecision(indent) << m_name << '\n';
  strm << setw(indent+8) << "value = " << setprecision(indent) << m_value << '\n';
  strm << setw(indent-1) << setprecision(indent-2) << "}";
}
#endif


PObject::Comparison PSNMP_VarBind::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(&obj, PSNMP_VarBind), PInvalidCast);
#endif
  const PSNMP_VarBind & other = (const PSNMP_VarBind &)obj;

  Comparison result;

  if ((result = m_name.Compare(other.m_name)) != EqualTo)
    return result;
  if ((result = m_value.Compare(other.m_value)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX PSNMP_VarBind::GetDataLength() const
{
  PINDEX length = 0;
  length += m_name.GetObjectLength();
  length += m_value.GetObjectLength();
  return length;
}


PBoolean PSNMP_VarBind::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return PFalse;

  if (!m_name.Decode(strm))
    return PFalse;
  if (!m_value.Decode(strm))
    return PFalse;

  return UnknownExtensionsDecode(strm);
}


void PSNMP_VarBind::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  m_name.Encode(strm);
  m_value.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * PSNMP_VarBind::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_VarBind::Class()), PInvalidCast);
#endif
  return new PSNMP_VarBind(*this);
}


//
// VarBindList
//

PSNMP_VarBindList::PSNMP_VarBindList(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Array(tag, tagClass)
{
}


PASN_Object * PSNMP_VarBindList::CreateObject() const
{
  return new PSNMP_VarBind;
}


PSNMP_VarBind & PSNMP_VarBindList::operator[](PINDEX i) const
{
  return (PSNMP_VarBind &)array[i];
}


PObject * PSNMP_VarBindList::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_VarBindList::Class()), PInvalidCast);
#endif
  return new PSNMP_VarBindList(*this);
}


//
// PDU
//

PSNMP_PDU::PSNMP_PDU(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 0, PFalse, 0)
{
}


#ifndef PASN_NOPRINTON
void PSNMP_PDU::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  strm << setw(indent+13) << "request_id = " << setprecision(indent) << m_request_id << '\n';
  strm << setw(indent+15) << "error_status = " << setprecision(indent) << m_error_status << '\n';
  strm << setw(indent+14) << "error_index = " << setprecision(indent) << m_error_index << '\n';
  strm << setw(indent+20) << "variable_bindings = " << setprecision(indent) << m_variable_bindings << '\n';
  strm << setw(indent-1) << setprecision(indent-2) << "}";
}
#endif


PObject::Comparison PSNMP_PDU::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(&obj, PSNMP_PDU), PInvalidCast);
#endif
  const PSNMP_PDU & other = (const PSNMP_PDU &)obj;

  Comparison result;

  if ((result = m_request_id.Compare(other.m_request_id)) != EqualTo)
    return result;
  if ((result = m_error_status.Compare(other.m_error_status)) != EqualTo)
    return result;
  if ((result = m_error_index.Compare(other.m_error_index)) != EqualTo)
    return result;
  if ((result = m_variable_bindings.Compare(other.m_variable_bindings)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX PSNMP_PDU::GetDataLength() const
{
  PINDEX length = 0;
  length += m_request_id.GetObjectLength();
  length += m_error_status.GetObjectLength();
  length += m_error_index.GetObjectLength();
  length += m_variable_bindings.GetObjectLength();
  return length;
}


PBoolean PSNMP_PDU::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return PFalse;

  if (!m_request_id.Decode(strm))
    return PFalse;
  if (!m_error_status.Decode(strm))
    return PFalse;
  if (!m_error_index.Decode(strm))
    return PFalse;
  if (!m_variable_bindings.Decode(strm))
    return PFalse;

  return UnknownExtensionsDecode(strm);
}


void PSNMP_PDU::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  m_request_id.Encode(strm);
  m_error_status.Encode(strm);
  m_error_index.Encode(strm);
  m_variable_bindings.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * PSNMP_PDU::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_PDU::Class()), PInvalidCast);
#endif
  return new PSNMP_PDU(*this);
}


//
// Trap-PDU
//

PSNMP_Trap_PDU::PSNMP_Trap_PDU(unsigned tag, PASN_Object::TagClass tagClass)
  : PASN_Sequence(tag, tagClass, 0, PFalse, 0)
{
}


#ifndef PASN_NOPRINTON
void PSNMP_Trap_PDU::PrintOn(ostream & strm) const
{
  int indent = strm.precision() + 2;
  strm << "{\n";
  strm << setw(indent+13) << "enterprise = " << setprecision(indent) << m_enterprise << '\n';
  strm << setw(indent+13) << "agent_addr = " << setprecision(indent) << m_agent_addr << '\n';
  strm << setw(indent+15) << "generic_trap = " << setprecision(indent) << m_generic_trap << '\n';
  strm << setw(indent+16) << "specific_trap = " << setprecision(indent) << m_specific_trap << '\n';
  strm << setw(indent+13) << "time_stamp = " << setprecision(indent) << m_time_stamp << '\n';
  strm << setw(indent+20) << "variable_bindings = " << setprecision(indent) << m_variable_bindings << '\n';
  strm << setw(indent-1) << setprecision(indent-2) << "}";
}
#endif


PObject::Comparison PSNMP_Trap_PDU::Compare(const PObject & obj) const
{
#ifndef PASN_LEANANDMEAN
  PAssert(PIsDescendant(&obj, PSNMP_Trap_PDU), PInvalidCast);
#endif
  const PSNMP_Trap_PDU & other = (const PSNMP_Trap_PDU &)obj;

  Comparison result;

  if ((result = m_enterprise.Compare(other.m_enterprise)) != EqualTo)
    return result;
  if ((result = m_agent_addr.Compare(other.m_agent_addr)) != EqualTo)
    return result;
  if ((result = m_generic_trap.Compare(other.m_generic_trap)) != EqualTo)
    return result;
  if ((result = m_specific_trap.Compare(other.m_specific_trap)) != EqualTo)
    return result;
  if ((result = m_time_stamp.Compare(other.m_time_stamp)) != EqualTo)
    return result;
  if ((result = m_variable_bindings.Compare(other.m_variable_bindings)) != EqualTo)
    return result;

  return PASN_Sequence::Compare(other);
}


PINDEX PSNMP_Trap_PDU::GetDataLength() const
{
  PINDEX length = 0;
  length += m_enterprise.GetObjectLength();
  length += m_agent_addr.GetObjectLength();
  length += m_generic_trap.GetObjectLength();
  length += m_specific_trap.GetObjectLength();
  length += m_time_stamp.GetObjectLength();
  length += m_variable_bindings.GetObjectLength();
  return length;
}


PBoolean PSNMP_Trap_PDU::Decode(PASN_Stream & strm)
{
  if (!PreambleDecode(strm))
    return PFalse;

  if (!m_enterprise.Decode(strm))
    return PFalse;
  if (!m_agent_addr.Decode(strm))
    return PFalse;
  if (!m_generic_trap.Decode(strm))
    return PFalse;
  if (!m_specific_trap.Decode(strm))
    return PFalse;
  if (!m_time_stamp.Decode(strm))
    return PFalse;
  if (!m_variable_bindings.Decode(strm))
    return PFalse;

  return UnknownExtensionsDecode(strm);
}


void PSNMP_Trap_PDU::Encode(PASN_Stream & strm) const
{
  PreambleEncode(strm);

  m_enterprise.Encode(strm);
  m_agent_addr.Encode(strm);
  m_generic_trap.Encode(strm);
  m_specific_trap.Encode(strm);
  m_time_stamp.Encode(strm);
  m_variable_bindings.Encode(strm);

  UnknownExtensionsEncode(strm);
}


PObject * PSNMP_Trap_PDU::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_Trap_PDU::Class()), PInvalidCast);
#endif
  return new PSNMP_Trap_PDU(*this);
}


//
// GetRequest-PDU
//

PSNMP_GetRequest_PDU::PSNMP_GetRequest_PDU(unsigned tag, PASN_Object::TagClass tagClass)
  : PSNMP_PDU(tag, tagClass)
{
}


PObject * PSNMP_GetRequest_PDU::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_GetRequest_PDU::Class()), PInvalidCast);
#endif
  return new PSNMP_GetRequest_PDU(*this);
}


//
// GetNextRequest-PDU
//

PSNMP_GetNextRequest_PDU::PSNMP_GetNextRequest_PDU(unsigned tag, PASN_Object::TagClass tagClass)
  : PSNMP_PDU(tag, tagClass)
{
}


PObject * PSNMP_GetNextRequest_PDU::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_GetNextRequest_PDU::Class()), PInvalidCast);
#endif
  return new PSNMP_GetNextRequest_PDU(*this);
}


//
// GetResponse-PDU
//

PSNMP_GetResponse_PDU::PSNMP_GetResponse_PDU(unsigned tag, PASN_Object::TagClass tagClass)
  : PSNMP_PDU(tag, tagClass)
{
}


PObject * PSNMP_GetResponse_PDU::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_GetResponse_PDU::Class()), PInvalidCast);
#endif
  return new PSNMP_GetResponse_PDU(*this);
}


//
// SetRequest-PDU
//

PSNMP_SetRequest_PDU::PSNMP_SetRequest_PDU(unsigned tag, PASN_Object::TagClass tagClass)
  : PSNMP_PDU(tag, tagClass)
{
}


PObject * PSNMP_SetRequest_PDU::Clone() const
{
#ifndef PASN_LEANANDMEAN
  PAssert(IsClass(PSNMP_SetRequest_PDU::Class()), PInvalidCast);
#endif
  return new PSNMP_SetRequest_PDU(*this);
}


#endif // if ! H323_DISABLE_PSNMP


// End of snmp.cxx
