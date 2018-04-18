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
#include <ptclib/cypher.h>


#define new PNEW
#define PTraceModule() "JSON"


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


PJSON::PJSON(const PJSON & other)
  : m_root(other.m_root->DeepClone())
  , m_valid(other.m_valid)
{
}


PJSON & PJSON::operator=(const PJSON & other)
{
  delete m_root;
  m_root = other.m_root->DeepClone();
  m_valid = other.m_valid;
  return *this;
}


bool PJSON::FromString(const PString & str)
{
  PStringStream strm(str);
  ReadFrom(strm);
  return m_valid;
}


PString PJSON::AsString(std::streamsize initialIndent, std::streamsize subsequentIndent) const
{
  PStringStream strm;
  strm.width(initialIndent);
  strm.precision(subsequentIndent != 0 ? subsequentIndent : (initialIndent != 0 ? 2 : 6));
  PrintOn(strm);
  return strm;
}


void PJSON::Set(Types type)
{
  delete m_root;
  m_root = CreateByType(type);
  if (m_root == NULL)
    m_root = new Null;
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
    case '-' :
    case '.' :
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
          char hex[5];
          strm.read(hex, 4);
          hex[4] = '\0';
          str += (char)strtoul(hex, NULL, 16);
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
    unsigned c = str[i] & 0xff;
    switch (c) {
      case '"' :
        strm << "\\\"";
        break;
      case '\\' :
        strm << "\\\\";
        break;
      case '\t' :
        strm << "\\t";
        break;
      case '\r' :
        strm << "\\r";
        break;
      case '\n' :
        strm << "\\n";
        break;
      default :
        if (c >= ' ')
          strm << str[i];
        else {
          char oldFill = strm.fill('0');
          ios_base::fmtflags oldFlags = strm.setf(ios_base::hex, ios_base::dec);

          strm << "\\u" << setw(4) << c;

          strm.setf(oldFlags);
          strm.fill(oldFill);
        }
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
  std::streamsize increment = strm.precision();
  bool pretty = indent != 0 || increment != 6; // std::ios defaults
  if (increment == 6)
    increment = 2;

  if (pretty)
    strm << std::right << std::setw(indent + 1);
  strm << '{';
  for (const_iterator it = begin(); it != end(); ++it) {
    if (it != begin())
      strm << ',';
    if (pretty)
      strm << '\n' << std::setw(indent + increment) << ' ';
    PrintString(strm, it->first);
    if (pretty) {
      strm << " :";
      if (it->second->IsType(e_Object) || it->second->IsType(e_Array))
        strm << '\n' << std::setw(indent + increment);
      else
        strm << ' ';
    }
    else
      strm << ':';
    it->second->PrintOn(strm);
  }
  if (pretty && !empty())
    strm << '\n' << std::setw(indent+1);
  strm << '}';
}


PJSON::Base * PJSON::Object::DeepClone() const
{
  PJSON::Object * obj = new Object();
  for (std::map<PString, Base *>::const_iterator it = begin(); it != end(); ++it)
      obj->insert(make_pair(it->first, it->second->DeepClone()));
  return obj;
}


bool PJSON::Object::IsType(const PString & name, Types type) const
{
  const_iterator it = find(name);
  return it != end() ? it->second->IsType(type) : (type == e_Null);
}


PString PJSON::Object::GetString(const PString & name) const
{
  const String * str = Get<String>(name);
  return str != NULL ? *str : PString::Empty();
}


int PJSON::Object::GetInteger(const PString & name) const
{
  const Number * num = Get<Number>(name);
  return num != NULL ? (int)num->GetValue() : 0;
}


unsigned PJSON::Object::GetUnsigned(const PString & name) const
{
  const Number * num = Get<Number>(name);
  return num != NULL ? (unsigned)num->GetValue() : 0;
}


double PJSON::Object::GetNumber(const PString & name) const
{
  const Number * num = Get<Number>(name);
  return num != NULL ? num->GetValue() : 0;
}


bool PJSON::Object::GetBoolean(const PString & name) const
{
  const Boolean * flag = Get<Boolean>(name);
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


bool PJSON::Object::Set(const PString & name, const Base & toInsert)
{
  if (find(name) != end())
    return false;

  insert(make_pair(name, toInsert.DeepClone()));
  return true;
}


bool PJSON::Object::Set(const PString & name, const PJSON & toInsert)
{
  return Set(name, toInsert.GetAs<Base>());
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
  std::streamsize increment = strm.precision();
  bool pretty = indent != 0 || increment != 6; // std::ios defaults
  if (increment == 6)
    increment = 2;

  if (pretty)
    strm << std::right << std::setw(indent + 1);
  strm << '[';
  for (const_iterator it = begin(); it != end(); ++it) {
    const PJSON::Base & item = **it;
    if (it != begin())
      strm << ',';
    if (pretty) {
      strm << '\n';
      if (item.IsType(e_Object) || item.IsType(e_Array))
        strm.width(indent + increment);
      else
        strm << std::setw(indent + increment) << ' ';
    }
    item.PrintOn(strm);
  }
  if (pretty && !empty())
    strm << '\n' << std::setw(indent + 1);
  strm << ']';
}


PJSON::Base * PJSON::Array::DeepClone() const
{
  PJSON::Array * arr = new Array();
  arr->resize(size());
  for (size_t i = 0; i < size(); ++i)
      arr->at(i) = at(i)->DeepClone();
  return arr;
}


bool PJSON::Array::IsType(size_t index, Types type) const
{
    if (index < size()) {
        Base * obj = at(index);
        if (obj != NULL)
            return obj->IsType(type);
    }

    return type == e_Null;
}


PString PJSON::Array::GetString(size_t index) const
{
  const String * str = Get<String>(index);
  return str != NULL ? *str : PString::Empty();
}


int PJSON::Array::GetInteger(size_t index) const
{
  const Number * num = Get<Number>(index);
  return num != NULL ? (int)num->GetValue() : 0;
}


unsigned PJSON::Array::GetUnsigned(size_t index) const
{
  const Number * num = Get<Number>(index);
  return num != NULL ? (unsigned)num->GetValue() : 0;
}


double PJSON::Array::GetNumber(size_t index) const
{
  const Number * num = Get<Number>(index);
  return num != NULL ? num->GetValue() : 0;
}


bool PJSON::Array::GetBoolean(size_t index) const
{
  const Boolean * flag = Get<Boolean>(index);
  return flag != NULL && flag->GetValue();
}


void PJSON::Array::Append(Types type)
{
  Base * ptr = CreateByType(type);
  if (ptr != NULL)
    push_back(ptr);
}


void PJSON::Array::Append(const Base & toAppend)
{
  push_back(toAppend.DeepClone());
}


void PJSON::Array::Append(const PJSON & toAppend)
{
  Append(toAppend.GetAs<Base>());
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


PJSON::Base * PJSON::String::DeepClone() const
{
  return new String(GetPointer());
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
    int64_t intval = (int64_t)m_value;
    if (intval == m_value) {
      strm << intval;
      return;
    }
  }
  else {
    uint64_t uintval = (uint64_t)m_value;
    if (uintval == m_value) {
      strm << uintval;
      return;
    }
  }

  strm << m_value;
}


PJSON::Base * PJSON::Number::DeepClone() const
{
  return new Number(m_value);
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


PJSON::Base * PJSON::Boolean::DeepClone() const
{
  return new Boolean(m_value);
}


bool PJSON::Null::IsType(Types type) const
{
  return type == e_Null;
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


PJSON::Base * PJSON::Null::DeepClone() const
{
  return new Null();
}


#if P_SSL

PJWT::PJWT()
  : PJSON(e_Object)
{
}


PJWT::PJWT(const PString & str, const PString & secret, const PTime & verifyTime)
{
  m_valid = Decode(str, secret, verifyTime);
}


static PHMAC * CreateHMAC(const PJWT::Algorithm algorithm)
{
  switch (algorithm) {
    case PJWT::HS256:
      return new PHMAC_SHA256;
      break;
    case PJWT::HS384:
      return new PHMAC_SHA384;
      break;
    case PJWT::HS512:
      return new PHMAC_SHA512;
    default :
      PAssertAlways("Unknown algorithm");
      return NULL;
  }
}


PString PJWT::Encode(const PString & secret, const Algorithm algorithm)
{
  std::auto_ptr<PHMAC> hmac(CreateHMAC(algorithm));
  if (!hmac.get())
    return PString::Empty();

  hmac->SetKey(secret);

  PJSON hdr(e_Object);
  Object & hdrObj = hdr.GetObject();
  hdrObj.SetString("typ", "JWT");
  hdrObj.SetString("alg", PSTRSTRM(algorithm));

  PStringStream token;
  token << PBase64::Encode(hdr.AsString(), PBase64::e_URL) << '.' << PBase64::Encode(AsString(), PBase64::e_URL);
  PINDEX prefixLength = token.GetLength();

  token << '.' << hmac->Encode(token.GetPointer(), prefixLength, PBase64::e_URL);

  return token;
}


bool PJWT::Decode(const PString & str, const PString & secret, const PTime & verifyTime)
{
  PStringArray section = str.Tokenise(".");
  if (section.size() != 3) {
    PTRACE(2, "Invalid JWT header: \"" << str.Left(100) << '"');
    return false;
  }

  PJSON hdr(PBase64::Decode(section[0]));
  if (!hdr.IsValid()) {
    PTRACE(2, "Invalid JWT header JSON.");
    return false;
  }

  if (!hdr.IsType(e_Object)) {
    PTRACE(2, "Invalid JWT header JSON type.");
    return false;
  }

  Object & hdrObj = hdr.GetObject();
  if (hdrObj.GetString("typ") != "JWT") {
    PTRACE(2, "Unsupported JWT type.");
    return false;
  }

  std::auto_ptr<PHMAC> hmac(CreateHMAC(AlgorithmFromString(hdrObj.GetString("alg"))));
  if (!hmac.get()) {
    PTRACE(2, "Unsupported JWT algorithm.");
    return false;
  }

  hmac->SetKey(secret);

  PHMAC::Result theirs, ours;
  PBase64::Decode(section[2], theirs);
  hmac->Process(str.GetPointer(), str.FindLast('.'), ours);
  if (!ours.ConstantTimeCompare(theirs)) {
    PTRACE(2, "JWT signature does not match.");
    return false;
  }

  if (!FromString(PBase64::Decode(section[1]))) {
    PTRACE(2, "Invalid JWT payload JSON.");
    return false;
  }

  if (verifyTime.IsValid()) {
    PTime exp = GetExpiration();
    if (exp.IsValid() && exp < verifyTime) {
      PTRACE(2, "JWT expired at " << exp);
      return false;
    }

    PTime nbf = GetNotBefore();
    if (nbf.IsValid() && nbf >= verifyTime) {
      PTRACE(2, "JWT not available before " << nbf);
      return false;
    }
  }

  return true;
}

void PJWT::SetIssuer(const PString & str)
{
  GetObject().SetString("iss", str);
}


PString PJWT::GetIssuer() const
{
  return GetObject().GetString("iss");
}


void PJWT::SetSubject(const PString & str)
{
  GetObject().SetString("sub", str);
}


PString PJWT::GetSubject() const
{
  return GetObject().GetString("sub");
}


void PJWT::SetAudience(const PString & str)
{
  GetObject().SetString("aud", str);
}


PString PJWT::GetAudience() const
{
  return GetObject().GetString("aud");
}


void PJWT::SetExpiration(const PTime & when)
{
  GetObject().SetNumber("exp", (double)when.GetTimeInSeconds());
}


PTime PJWT::GetExpiration() const
{
  return GetObject().GetUnsigned("exp");
}


void PJWT::SetNotBefore(const PTime & when)
{
  GetObject().SetNumber("nbf", (double)when.GetTimeInSeconds());
}


PTime PJWT::GetNotBefore() const
{
  return GetObject().GetUnsigned("nbf");
}


void PJWT::SetIssuedAt(const PTime & when)
{
  GetObject().SetNumber("iat", (double)when.GetTimeInSeconds());
}


PTime PJWT::GetIssuedAt() const
{
  return GetObject().GetUnsigned("iat");
}


void PJWT::SetTokenId(const PString & str)
{
  GetObject().SetString("jti", str);
}


PString PJWT::GetTokenId() const
{
  return GetObject().GetString("jti");
}


void PJWT::SetPrivate(const PString & key, const PString & str)
{
  GetObject().SetString(key, str);
}


PString PJWT::GetPrivate(const PString & key) const
{
  return GetObject().GetString(key);
}


#endif // P_SSL
