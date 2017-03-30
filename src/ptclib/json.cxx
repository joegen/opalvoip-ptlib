/*
 * json.cxx
 *
 * JSON parser
 *
 * Portable Tools Library
 *
 * Copyright (C) 2015 by Vox Lucida Pty. Ltd.
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
 *                 Blackboard, inc
 */

#ifdef __GNUC__
#pragma implementation "pjson.h"
#endif

#include <ptlib.h>

#include <ptclib/pjson.h>

#define new PNEW


static PJSON::Base * CreateByType(PJSON::Types type)
{
  switch (type) {
    case PJSON::e_Object :
      return new PJSON::Object;
    case PJSON::e_Array :
      return new PJSON::Array;
    case PJSON::e_String :
      return new PJSON::String;
    case PJSON::e_Number :
      return new PJSON::Number;
    case PJSON::e_Boolean :
      return new PJSON::Boolean;
    case PJSON::e_Null :
      return new PJSON::Null;
  }

  return NULL;
}


PJSON::PJSON()
  : m_root(new Null)
  , m_valid(true)
{
}


PJSON::PJSON(Types type)
  : m_root(CreateByType(type))
  , m_valid(m_root != NULL)
{
    if (m_root == NULL)
        m_root = new Null;
}


PJSON::PJSON(const PString & str)
  : m_root(NULL)
  , m_valid(false)
{
  FromString(str);
}


bool PJSON::FromString(const PString & str)
{
  PStringStream strm(str);
  ReadFrom(strm);
  return m_valid;
}


PString PJSON::AsString(std::streamsize indent) const
{
  PStringStream strm;
  strm.width(indent);
  PrintOn(strm);
  return strm;
}


static PJSON::Base * CreateFromStream(istream & strm)
{
  strm >> ws;
  switch (strm.peek()) {
    case '{' :
      return new PJSON::Object;
    case '[' :
      return new PJSON::Array;
    case '"' :
      return new PJSON::String;
    case '0' :
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' :
      return new PJSON::Number;
    case 'T' :
    case 't' :
    case 'F' :
    case 'f' :
      return new PJSON::Boolean;
    case 'N' :
    case 'n' :
      return new PJSON::Null;
  }

  strm.setstate(ios::failbit);
  return NULL;
}

void PJSON::ReadFrom(istream & strm)
{
  delete m_root;
  m_root = CreateFromStream(strm);
  if (m_root != NULL) {
    m_root->ReadFrom(strm);
    m_valid = !(strm.bad() || strm.fail());
  }
  else {
    m_root = new Null;
    m_valid = false;
  }
}


void PJSON::PrintOn(ostream & strm) const
{
  if (PAssertNULL(m_root) != NULL)
    m_root->PrintOn(strm);
}


static bool Expect(istream & strm, char expected)
{
  char got;
  strm >> ws >> got;
  if (strm.eof())
    return false;

  if (got == expected)
    return true;

  strm.putback(got);
  strm.setstate(ios::failbit);
  return false;
}


static bool ExpectComma(istream & strm, char terminator)
{
  char got;
  strm >> ws >> got;
  if (strm.eof())
    return false;

  if (got == ',')
    return true;
  if (got == terminator)
    return false;

  strm.putback(got);
  strm.setstate(ios::failbit);
  return false;
}


static bool ReadString(istream & strm, PString & str)
{
  if (!Expect(strm, '"'))
    return false;

  while (strm.good()) {
    char c;
    strm.get(c);
    if (c == '"')
      return true;
    if (c != '\\')
      str += c;
    else {
      if (strm.eof())
        return false;
      strm.get(c);
      switch (c) {
        default :
          return false;
        case '"' :
        case '\\' :
        case '/' :
          str += c;
          break;
        case 'b' :
          str += '\b';
          break;
        case 'f' :
          str += '\f';
          break;
        case 'n' :
          str += '\n';
          break;
        case 'r' :
          str += '\r';
          break;
        case 't' :
          str += '\t';
          break;
        case 'u' :
          str += '\b';
          break;
      }
    }
  }

  return false;
}


static void PrintString(ostream & strm, const PString & str)
{
  strm << '"';
  for (PINDEX i = 0; i < str.GetLength(); ++i) {
    switch (str[i]) {
      case '"' :
        strm << "\\\"";
        break;
      case '\\' :
        strm << "\\\\";
        break;
      default :
        strm << str[i];
    }
  }
  strm << '"';
}


PJSON::Object::~Object()
{
  for (iterator it = begin(); it != end(); ++it)
    delete it->second;
}


bool PJSON::Object::IsType(Types type) const
{
  return type == e_Object;
}


void PJSON::Object::ReadFrom(istream & strm)
{
  if (!Expect(strm, '{'))
    return;

  char close;
  strm >> ws >> close;
  if (close == '}')
      return;
  strm.putback(close);

  do {
    PString name;
    if (!ReadString(strm, name))
      return;

    if (!Expect(strm, ':'))
      return;

    Base * value = CreateFromStream(strm);
    if (value == NULL)
      return;

    insert(make_pair(name, value));
    value->ReadFrom(strm);
    if (strm.fail())
      return;

  } while (ExpectComma(strm, '}'));
}


void PJSON::Object::PrintOn(ostream & strm) const
{
  std::streamsize indent = strm.width();

  if (indent > 0)
    strm << std::right << std::setw(indent + 1);
  strm << '{';
  for (const_iterator it = begin(); it != end(); ++it) {
    if (it != begin())
      strm << ',';
    if (indent > 0)
      strm << '\n' << std::setw(indent+2) << ' ';
    PrintString(strm, it->first);
    if (indent > 0) {
      strm << " :";
      if (it->second->IsType(e_Object) || it->second->IsType(e_Array))
        strm << '\n' << std::setw(indent + 2);
      else
        strm << ' ';
    }
    else
      strm << ':';
    it->second->PrintOn(strm);
  }
  if (indent > 0 && !empty())
    strm << '\n' << std::setw(indent+1);
  strm << '}';
}


PJSON::Object & PJSON::Object::GetObject(const PString & name) const
{
  Object * obj = Get<Object>(name);
  return *PAssertNULL(obj);
}


PJSON::Array & PJSON::Object::GetArray(const PString & name) const
{
  Array * arr = Get<Array>(name);
  return *PAssertNULL(arr);
}


PString PJSON::Object::GetString(const PString & name) const
{
  String * str = Get<String>(name);
  return str != NULL ? *str : PString::Empty();
}


int PJSON::Object::GetInteger(const PString & name) const
{
  Number * num = Get<Number>(name);
  return num != NULL ? (int)num->GetValue() : 0;
}


unsigned PJSON::Object::GetUnsigned(const PString & name) const
{
  Number * num = Get<Number>(name);
  return num != NULL ? (unsigned)num->GetValue() : 0;
}


double PJSON::Object::GetNumber(const PString & name) const
{
  Number * num = Get<Number>(name);
  return num != NULL ? num->GetValue() : 0;
}


bool PJSON::Object::GetBoolean(const PString & name) const
{
    Boolean * flag = Get<Boolean>(name);
    return flag != NULL && flag->GetValue();
}


bool PJSON::Object::Set(const PString & name, Types type)
{
  if (find(name) != end())
    return false;

  Base * ptr = CreateByType(type);
  if (ptr == NULL)
    return false;

  insert(make_pair(name, ptr));
  return true;
}


PJSON::Object & PJSON::Object::SetObject(const PString & name)
{
  Set(name, e_Object);
  return GetObject(name);
}


PJSON::Array & PJSON::Object::SetArray(const PString & name)
{
  Set(name, e_Array);
  return GetArray(name);
}


bool PJSON::Object::SetString(const PString & name, const PString & value)
{
  if (!Set(name, e_String))
    return false;
  *Get<String>(name) = value;
  return true;
}


bool PJSON::Object::SetNumber(const PString & name, double value)
{
  if (!Set(name, e_Number))
    return false;
  *Get<Number>(name) = value;
  return true;
}


bool PJSON::Object::SetBoolean(const PString & name, bool value)
{
  if (!Set(name, e_Boolean))
    return false;
  *Get<Boolean>(name) = value;
  return true;
}


PJSON::Array::~Array()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
}


bool PJSON::Array::IsType(Types type) const
{
  return type == e_Array;
}


void PJSON::Array::ReadFrom(istream & strm)
{
  if (!Expect(strm, '['))
    return;

  char close;
  strm >> ws >> close;
  if (close == ']')
      return;
  strm.putback(close);

  do {
    Base * value = CreateFromStream(strm);
    if (value == NULL)
      return;

    push_back(value);

    value->ReadFrom(strm);
    if (strm.fail())
      return;

  } while (ExpectComma(strm, ']'));
}


void PJSON::Array::PrintOn(ostream & strm) const
{
  std::streamsize indent = strm.width();

  if (indent > 0)
    strm << std::right << std::setw(indent + 1);
  strm << '[';
  for (const_iterator it = begin(); it != end(); ++it) {
    const PJSON::Base & item = **it;
    if (it != begin())
      strm << ',';
    if (indent > 0) {
      strm << '\n';
      if (item.IsType(e_Object) || item.IsType(e_Array))
        strm.width(indent + 2);
      else
        strm << std::setw(indent + 2) << ' ';
    }
    item.PrintOn(strm);
  }
  if (indent > 0 && !empty())
    strm << '\n' << std::setw(indent + 1);
  strm << ']';
}


PJSON::Object & PJSON::Array::GetObject(size_t index) const
{
  Object * obj = Get<Object>(index);
  return *PAssertNULL(obj);
}


PJSON::Array & PJSON::Array::GetArray(size_t index) const
{
  Array * arr = Get<Array>(index);
  return *PAssertNULL(arr);
}


PString PJSON::Array::GetString(size_t index) const
{
  String * str = Get<String>(index);
  return str != NULL ? *str : PString::Empty();
}


int PJSON::Array::GetInteger(size_t index) const
{
  Number * num = Get<Number>(index);
  return num != NULL ? (int)num->GetValue() : 0;
}


unsigned PJSON::Array::GetUnsigned(size_t index) const
{
  Number * num = Get<Number>(index);
  return num != NULL ? (unsigned)num->GetValue() : 0;
}


double PJSON::Array::GetNumber(size_t index) const
{
  Number * num = Get<Number>(index);
  return num != NULL ? num->GetValue() : 0;
}


bool PJSON::Array::GetBoolean(size_t index) const
{
  Boolean * flag = Get<Boolean>(index);
  return flag != NULL && flag->GetValue();
}


void PJSON::Array::Append(Types type)
{
  Base * ptr = CreateByType(type);
  if (ptr != NULL)
    push_back(ptr);
}


PJSON::Object & PJSON::Array::AppendObject()
{
  Append(e_Object);
  return *dynamic_cast<Object *>(back());
}


PJSON::Array & PJSON::Array::AppendArray()
{
  Append(e_Array);
  return *dynamic_cast<Array *>(back());
}


void PJSON::Array::AppendString(const PString & value)
{
  Append(e_String);
  dynamic_cast<String &>(*back()) = value;
}


void PJSON::Array::AppendNumber(double value)
{
  Append(e_Number);
  dynamic_cast<Number &>(*back()) = value;
}


void PJSON::Array::AppendBoolean(bool value)
{
  Append(e_Boolean);
  dynamic_cast<Boolean &>(*back()) = value;
}


bool PJSON::String::IsType(Types type) const
{
  return type == e_String;
}


void PJSON::String::ReadFrom(istream & strm)
{
  ReadString(strm, *this);
}


void PJSON::String::PrintOn(ostream & strm) const
{
  PrintString(strm, *this);
}


PJSON::Number::Number(double value)
  : m_value(value)
{
}


bool PJSON::Number::IsType(Types type) const
{
  return type == e_Number;
}


void PJSON::Number::ReadFrom(istream & strm)
{
  strm >> m_value;
}


void PJSON::Number::PrintOn(ostream & strm) const
{
  if (m_value < 0) {
    int intval = (int)m_value;
    if (intval == m_value) {
      strm << intval;
      return;
    }
  }
  else if (m_value < UINT_MAX) {
    unsigned uintval = (unsigned)m_value;
    if (uintval == m_value) {
      strm << uintval;
      return;
    }
  }
    strm << m_value;
}


PJSON::Boolean::Boolean(bool value)
  : m_value(value)
{
}


bool PJSON::Boolean::IsType(Types type) const
{
  return type == e_Boolean;
}


void PJSON::Boolean::ReadFrom(istream & strm)
{
  char c;
  strm >> ws >> c;
  switch (c) {
    case 'T' :
    case 't' :
      m_value = true;
      if (tolower(strm.get()) == 'r' &&
          tolower(strm.get()) == 'u' &&
          tolower(strm.get()) == 'e')
        return;
      break;
    case 'F' :
    case 'f' :
      m_value = false;
      if (tolower(strm.get()) == 'a' &&
          tolower(strm.get()) == 'l' &&
          tolower(strm.get()) == 's' &&
          tolower(strm.get()) == 'e')
        return;
      break;
  }
  strm.setstate(ios::failbit);
}


void PJSON::Boolean::PrintOn(ostream & strm) const
{
  strm << std::boolalpha << m_value;
}


bool PJSON::Null::IsType(Types type) const
{
  return type == e_String;
}


void PJSON::Null::ReadFrom(istream & strm)
{
  if (tolower(strm.get()) == 'n' &&
      tolower(strm.get()) == 'u' &&
      tolower(strm.get()) == 'l' &&
      tolower(strm.get()) == 'l')
    return;

  strm.setstate(ios::failbit);
}


void PJSON::Null::PrintOn(ostream & strm) const
{
  strm << "null";
}
