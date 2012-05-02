/*
 * vartype.cxx
 *
 * Interface library for Variable Type class wrapper
 *
 * Portable Tools Library
 *
 * Copyright (C) 2012 by Vox Lucida Pty. Ltd.
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
 * The Original Code is Portable Tools Library.
 *
 * The Initial Developer of the Original Code is Vox Lucida Pty. Ltd.
 *
 * Contributor(s): Robert Jongbloed
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptclib/vartype.h>


#define new PNEW


char * PVarType::Variant::Dynamic::Alloc(size_t sz)
{
  size = sz > 0 ? sz : 1;
  data = new char[size];
  return data;
}


void PVarType::Variant::Dynamic::Copy(const Dynamic & other)
{
  size = other.size;
  data = new char[size];
  memcpy(data, data, size);
}


void PVarType::InternalCopy(const PVarType & other)
{
  if (&other == this)
    return;

  InternalDestroy();

  m_type = other.m_type;

  switch (m_type) {
    case VarFixedString :
    case VarDynamicString :
    case VarDynamicBinary :
      m_.dynamic.Copy(other.m_.dynamic);
      break;

    default :
      m_ = other.m_;
  }
}


void PVarType::InternalDestroy()
{
  switch (m_type) {
    case VarFixedString :
    case VarDynamicString :
    case VarDynamicBinary :
      delete [] m_.dynamic.data;
      break;

    default :
      break;
  }

  m_type = VarNULL;
}


bool PVarType::SetType(BasicType type, PINDEX option)
{
  InternalDestroy();

  m_type = type;
  switch (m_type) {
    case VarTime :
      m_.time.seconds = 0;
      m_.time.format = (PTime::TimeFormat)option;
      break;

    case VarStaticString :
      m_.staticString = "";
      break;

    case VarFixedString :
    case VarDynamicString :
    case VarDynamicBinary :
      memset(m_.dynamic.Alloc(option), 0, option);
      break;

    case VarStaticBinary :
      m_.staticBinary.size = 1;
      m_.staticBinary.data = "";
      break;

    default :
      memset(&m_, 0, sizeof(m_));
  }

  return true;
}


PVarType & PVarType::SetValue(const PString & value)
{
  PStringStream strm(value);
  ReadFrom(strm);
  OnValueChanged();
  return *this;
}


PVarType & PVarType::SetString(const char * value, bool dynamic)
{
  if ((m_type == VarDynamicString || m_type == VarFixedString) && value == m_.dynamic.data)
    return *this;

  if (value == NULL)
    InternalDestroy();
  else if (dynamic) {
    size_t len = strlen(value)+1;
    if (m_type == VarDynamicString && m_.dynamic.size >= len)
      strcpy(m_.dynamic.data, value);
    else if (m_type == VarFixedString)
      strncpy(m_.dynamic.data, value, m_.dynamic.size-1);
    else {
      InternalDestroy();
      m_type = VarDynamicString;
      strcpy(m_.dynamic.Alloc(strlen(value)+1), value);
    }
  }
  else {
    InternalDestroy();
    m_type = VarStaticString;
    m_.staticString = value;
  }

  return *this;
}


PVarType & PVarType::SetBinary(const void * value, PINDEX len, bool dynamic)
{
  if (m_type == VarDynamicBinary && value == m_.dynamic.data)
    return *this;

  if (value == NULL || len == 0)
    InternalDestroy();
  else if (dynamic) {
    if (m_type == VarDynamicBinary && m_.dynamic.size == (size_t)len)
      memcpy(m_.dynamic.data, value, len);
    else {
      InternalDestroy();
      m_type = VarDynamicBinary;
      memcpy(m_.dynamic.Alloc(len), value, len);
    }
  }
  else {
    InternalDestroy();
    m_type = VarStaticBinary;
    m_.staticBinary.size = len;
    m_.staticBinary.data = (const char *)value;
  }

  return *this;
}


void PVarType::PrintOn(ostream & strm) const
{
  const_cast<PVarType *>(this)->OnGetValue();

  switch (m_type) {
    case VarNULL :
      strm << "(null)";
      break;
    case VarBoolean :
      strm << (m_.boolean ? "true" : "false");
      break;
    case VarChar :
      strm << m_.character;
      break;
    case VarInt8 :
      strm << (int)m_.int8;
      break;
    case VarInt16 :
      strm << m_.int16;
      break;
    case VarInt32 :
      strm << m_.int32;
      break;
    case VarInt64 :
      strm << m_.int32;
      break;
    case VarUInt8 :
      strm << (unsigned)m_.uint8;
      break;
    case VarUInt16 :
      strm << m_.uint16;
      break;
    case VarUInt32 :
      strm << m_.uint32;
      break;
    case VarUInt64 :
      strm << m_.uint64;
      break;
    case VarFloatSingle :
      strm << m_.floatSingle;
      break;
    case VarFloatDouble :
      strm << m_.floatDouble;
      break;
    case VarFloatExtended :
      strm << m_.floatExtended;
      break;
    case VarTime :
      strm << PTime(m_.time.seconds);
      break;
    case VarGUID :
      strm << PGloballyUniqueID(m_.guid, sizeof(m_.guid));
      break;
    case VarStaticString :
      strm << m_.staticString;
      break;
    case VarFixedString :
    case VarDynamicString :
      strm << m_.dynamic.data;
      break;
    case VarStaticBinary :
      strm.write(m_.staticBinary.data, m_.staticBinary.size);
      break;
    case VarDynamicBinary :
      strm.write(m_.dynamic.data, m_.dynamic.size);
      break;
    default :
      PAssertAlways("Invalid PVarType");
  }
}


void PVarType::ReadFrom(istream & strm)
{
  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
      strm >> m_.boolean;
      break;
    case VarChar :
      strm >> m_.character;
      break;
    case VarInt8 :
      { int i; strm >> i; m_.int8 = (int8_t)i; }
      break;
    case VarInt16 :
      strm >> m_.int16;
      break;
    case VarInt32 :
      strm >> m_.int32;
      break;
    case VarInt64 :
      strm >> m_.int32;
      break;
    case VarUInt8 :
      { unsigned i; strm >> i; m_.uint8 = (uint8_t)i; }
      break;
    case VarUInt16 :
      strm >> m_.uint16;
      break;
    case VarUInt32 :
      strm >> m_.uint32;
      break;
    case VarUInt64 :
      strm >> m_.uint64;
      break;
    case VarFloatSingle :
      strm >> m_.floatSingle;
      break;
    case VarFloatDouble :
      strm >> m_.floatDouble;
      break;
    case VarFloatExtended :
      strm >> m_.floatExtended;
      break;
    case VarTime :
      { PTime t; strm >> t; m_.time.seconds = t.GetTimeInSeconds(); }
      break;
    case VarGUID :
      { PGloballyUniqueID guid; strm >> guid; memcpy(m_.guid, guid, sizeof(m_.guid)); }
      break;
    case VarStaticString :
      PAssertAlways("Cannot read into PVarType static string");
      break;
    case VarFixedString :
    case VarDynamicString :
      { PString s; strm >> s; *this = s; }
      break;
    case VarStaticBinary :
      PAssertAlways("Cannot read into PVarType static data");
      break;
    case VarDynamicBinary :
      strm.read(m_.dynamic.data, m_.dynamic.size);
      break;
    default :
      PAssertAlways("Invalid PVarType");
  }
}


PObject * PVarType::Clone() const
{
  return new PVarType(*this);
}


bool PVarType::AsBoolean() const
{
  const_cast<PVarType *>(this)->OnGetValue();

  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
      return m_.boolean;
    case VarChar :
      return m_.character != '\0';
    case VarInt8 :
      return m_.int8 != 0;
    case VarInt16 :
      return m_.int16 != 0;
    case VarInt32 :
      return m_.int32 != 0;
    case VarInt64 :
      return m_.int64 != 0;
    case VarUInt8 :
      return m_.uint8 != 0;
    case VarUInt16 :
      return m_.uint16 != 0;
    case VarUInt32 :
      return m_.uint32 != 0;
    case VarUInt64 :
      return m_.uint64 != 0;
    case VarFloatSingle :
      return m_.floatSingle != 0;
    case VarFloatDouble :
      return m_.floatDouble != 0;
    case VarFloatExtended :
      return m_.floatExtended != 0;
    case VarTime :
      return PTime(m_.time.seconds).IsValid();
    case VarGUID :
      return !PGloballyUniqueID(m_.guid, sizeof(m_.guid)).IsNULL();
    case VarStaticString :
      return toupper(*m_.staticString) == 'T';
    case VarFixedString :
    case VarDynamicString :
      return toupper(*m_.dynamic.data) == 'T';
    case VarStaticBinary :
      return *m_.staticBinary.data != 0;
    case VarDynamicBinary :
      return *m_.dynamic.data != 0;
    default :
      PAssertAlways("Invalid PVarType");
  }

  return false;
}


int PVarType::AsInteger() const
{
  const_cast<PVarType *>(this)->OnGetValue();

  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
      return m_.boolean;
    case VarChar :
      return m_.character;
    case VarInt8 :
      return m_.int8;
    case VarInt16 :
      return m_.int16;
    case VarInt32 :
      return m_.int32;
    case VarInt64 :
      if (m_.int64 < INT_MIN)
        return INT_MIN;
      if (m_.int64 > INT_MAX)
        return INT_MAX;
      return (int)m_.int64;
    case VarUInt8 :
      return m_.uint8;
    case VarUInt16 :
      return m_.uint16;
    case VarUInt32 :
      return m_.uint32;
    case VarUInt64 :
      if (m_.uint64 > INT_MAX)
        return INT_MAX;
      return(int)m_.uint64;
    case VarFloatSingle :
      if (m_.floatSingle < INT_MIN)
        return INT_MIN;
      if (m_.floatSingle > INT_MAX)
        return INT_MAX;
      return (int)m_.floatSingle;
    case VarFloatDouble :
      if (m_.floatDouble < INT_MIN)
        return INT_MIN;
      if (m_.floatDouble > INT_MAX)
        return INT_MAX;
      return (int)m_.floatDouble;
    case VarFloatExtended :
      if (m_.floatExtended < INT_MIN)
        return INT_MIN;
      if (m_.floatExtended > INT_MAX)
        return INT_MAX;
      return (int)m_.floatExtended;
    case VarTime :
      if (m_.time.seconds > INT_MAX)
        return INT_MAX;
      return (int)m_.time.seconds;
    case VarGUID :
      return !PGloballyUniqueID(m_.guid, sizeof(m_.guid)).HashFunction();
    case VarStaticString :
      return atoi(m_.staticString);
    case VarFixedString :
    case VarDynamicString :
      return atoi(m_.dynamic.data);
    case VarStaticBinary :
      PAssert(m_.staticBinary.size >= sizeof(int), "Invalid PVarType conversion");
      return *(const int *)m_.staticBinary.data;
    case VarDynamicBinary :
      PAssert(m_.staticBinary.size >= sizeof(int), "Invalid PVarType conversion");
      return *(const int *)m_.dynamic.data;
    default :
      PAssertAlways("Invalid PVarType");
  }

  return 0;
}


unsigned PVarType::AsUnsigned() const
{
  const_cast<PVarType *>(this)->OnGetValue();

  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
      return m_.boolean;
    case VarChar :
      return m_.character;
    case VarInt8 :
      return m_.int8;
    case VarInt16 :
      return m_.int16;
    case VarInt32 :
      return m_.int32;
    case VarInt64 :
      if (m_.int64 < 0)
        return 0;
      if (m_.int64 > UINT_MAX)
        return UINT_MAX;
      return (unsigned)m_.int64;
    case VarUInt8 :
      return m_.uint8;
    case VarUInt16 :
      return m_.uint16;
    case VarUInt32 :
      return m_.uint32;
    case VarUInt64 :
      if (m_.uint64 > UINT_MAX)
        return UINT_MAX;
      return (unsigned)m_.uint64;
    case VarFloatSingle :
      if (m_.floatSingle < 0)
        return 0;
      if (m_.floatSingle > UINT_MAX)
        return UINT_MAX;
      return (unsigned)m_.floatSingle;
    case VarFloatDouble :
      if (m_.floatDouble < 0)
        return 0;
      if (m_.floatDouble > UINT_MAX)
        return UINT_MAX;
      return (unsigned)m_.floatDouble;
    case VarFloatExtended :
      if (m_.floatExtended < 0)
        return 0;
      if (m_.floatExtended > UINT_MAX)
        return UINT_MAX;
      return (unsigned)m_.floatExtended;
    case VarTime :
      if (m_.time.seconds > UINT_MAX)
        return UINT_MAX;
      return (unsigned)m_.time.seconds;
    case VarGUID :
      return !PGloballyUniqueID(m_.guid, sizeof(m_.guid)).HashFunction();
    case VarStaticString :
      return atoi(m_.staticString);
    case VarFixedString :
    case VarDynamicString :
      return atoi(m_.dynamic.data);
    case VarStaticBinary :
      PAssert(m_.staticBinary.size >= sizeof(unsigned), "Invalid PVarType conversion");
      return *(const unsigned *)m_.staticBinary.data;
    case VarDynamicBinary :
      PAssert(m_.staticBinary.size >= sizeof(int), "Invalid PVarType conversion");
      return *(const unsigned *)m_.dynamic.data;
    default :
      PAssertAlways("Invalid PVarType");
  }

  return 0;
}


double PVarType::AsFloat() const
{
  const_cast<PVarType *>(this)->OnGetValue();

  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
      return m_.boolean;
    case VarChar :
      return m_.character;
    case VarInt8 :
      return m_.int8;
    case VarInt16 :
      return m_.int16;
    case VarInt32 :
      return m_.int32;
    case VarInt64 :
      return (double)m_.int64;
    case VarUInt8 :
      return m_.uint8;
    case VarUInt16 :
      return m_.uint16;
    case VarUInt32 :
      return m_.uint32;
    case VarUInt64 :
      return (double)m_.uint64;
    case VarFloatSingle :
      return m_.floatSingle;
    case VarFloatDouble :
      return m_.floatDouble;
    case VarFloatExtended :
      return (double)m_.floatExtended;
    case VarTime :
      return (double)m_.time.seconds;
    case VarGUID :
      return !PGloballyUniqueID(m_.guid, sizeof(m_.guid)).HashFunction();
    case VarStaticString :
      return atoi(m_.staticString);
    case VarFixedString :
    case VarDynamicString :
      return atoi(m_.dynamic.data);
    case VarStaticBinary :
      PAssert(m_.staticBinary.size >= sizeof(double), "Invalid PVarType conversion");
      return *(const double *)m_.staticBinary.data;
    case VarDynamicBinary :
      PAssert(m_.staticBinary.size >= sizeof(int), "Invalid PVarType conversion");
      return *(const double *)m_.dynamic.data;
    default :
      PAssertAlways("Invalid PVarType");
  }

  return 0;
}


PString PVarType::AsString() const
{
  const_cast<PVarType *>(this)->OnGetValue();

  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
      return PString(m_.boolean ? "true" : "false");
    case VarChar :
      return PString(m_.character);
    case VarInt8 :
      return PString((int)m_.int8);
    case VarInt16 :
      return PString(m_.int16);
    case VarInt32 :
      return PString(m_.int32);
    case VarInt64 :
      return PString(m_.int64);
    case VarUInt8 :
      return PString((unsigned)m_.uint8);
    case VarUInt16 :
      return PString(m_.uint16);
    case VarUInt32 :
      return PString(m_.uint32);
    case VarUInt64 :
      return PString(m_.uint64);
    case VarFloatSingle :
      { PStringStream strm; strm << m_.floatSingle; return strm; }
    case VarFloatDouble :
      { PStringStream strm; strm << m_.floatDouble; return strm; }
    case VarFloatExtended :
      { PStringStream strm; strm << m_.floatExtended; return strm; }
    case VarTime :
      return PTime(m_.time.seconds).AsString(m_.time.format);
    case VarGUID :
      return PGloballyUniqueID(m_.guid, sizeof(m_.guid)).AsString();
    case VarStaticString :
      return PConstString(m_.staticString);
    case VarFixedString :
    case VarDynamicString :
      return PString(m_.dynamic.data);
    case VarStaticBinary :
      return PString(m_.staticBinary.data, m_.staticBinary.size);
    case VarDynamicBinary :
      return PString(m_.dynamic.data, m_.dynamic.size);
    default :
      PAssertAlways("Invalid PVarType");
  }

  return PString::Empty();
}


const void * PVarType::GetPointer() const
{
  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
    case VarChar :
    case VarInt8 :
    case VarInt16 :
    case VarInt32 :
    case VarInt64 :
    case VarUInt8 :
    case VarUInt16 :
    case VarUInt32 :
    case VarUInt64 :
    case VarFloatSingle :
    case VarFloatDouble :
    case VarFloatExtended :
    case VarTime :
    case VarGUID :
      return &m_;
    case VarStaticString :
      return m_.staticString;
    case VarFixedString :
    case VarDynamicString :
      return m_.dynamic.data;
    case VarStaticBinary :
      return m_.staticBinary.data;
    case VarDynamicBinary :
      return m_.dynamic.data;
    default :
      PAssertAlways("Invalid PVarType");
  }

  return 0;
}


PINDEX PVarType::GetSize() const
{
  switch (m_type) {
    case VarNULL :
      break;
    case VarBoolean :
      return sizeof(m_.boolean);
    case VarChar :
      return sizeof(m_.character);
    case VarInt8 :
      return sizeof(m_.int8);
    case VarInt16 :
      return sizeof(m_.int16);
    case VarInt32 :
      return sizeof(m_.int32);
    case VarInt64 :
      return sizeof(m_.int64);
    case VarUInt8 :
      return sizeof(m_.uint8);
    case VarUInt16 :
      return sizeof(m_.uint16);
    case VarUInt32 :
      return sizeof(m_.uint32);
    case VarUInt64 :
      return sizeof(m_.uint64);
    case VarFloatSingle :
      return sizeof(m_.floatSingle);
    case VarFloatDouble :
      return sizeof(m_.floatDouble);
    case VarFloatExtended :
      return sizeof(m_.floatExtended);
    case VarTime :
      return sizeof(m_.time);
    case VarGUID :
      return sizeof(m_.guid);
    case VarStaticString :
      return strlen(m_.staticString)+1;
    case VarFixedString :
    case VarDynamicString :
      return m_.dynamic.size;
    case VarStaticBinary :
      return m_.staticBinary.size;
    case VarDynamicBinary :
      return m_.dynamic.size;
    default :
      PAssertAlways("Invalid PVarType");
  }

  return 0;
}


void PVarType::OnGetValue()
{
}


void PVarType::OnValueChanged()
{
}
