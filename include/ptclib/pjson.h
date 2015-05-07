/*
 * pjson.h
 *
 * JSON parser
 *
 * Copyright (C) 2013 Post Increment
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
 * The Original Code is PTLib
 *
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): Craig Southeren (craigs@postincrement.com)
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PJSON_H
#define PTLIB_PJSON_H

#include <ptlib.h>

#include <json/json.h>

class PJSON : public PObject
{
  PCLASSINFO(PJSON, PObject)
  public:
    PJSON()
    { m_json = NULL; }

    ~PJSON()
    { json_object_put(m_json); }

    static PJSON Null()
    { return PJSON(); }

    static PJSON Bool(bool v)
    { return PJSON(v); }

    static PJSON Double(double v)
    { return PJSON(v); }

    static PJSON Int(int v)
    { return PJSON(v); }

    static PJSON String(const char * v)
    { return PJSON(json_object_new_string(v)); }

    static PJSON String(const std::string & v)
    { return PJSON(json_object_new_string(v.c_str())); }

    static PJSON String(const PString & v)
    { return PJSON(json_object_new_string((const char *)v)); }

    static PJSON Object()
    { return PJSON(json_object_new_object()); }

    static PJSON Array()
    { return PJSON(json_object_new_array()); }

    static PJSON Parse(const char * str)
    { return PJSON(json_tokener_parse(str)); }

    bool IsNull() const   { return IsType(json_type_null); }
    bool IsBool() const   { return IsType(json_type_boolean); }
    bool IsDouble() const { return IsType(json_type_double); }
    bool IsInt() const    { return IsType(json_type_int); }
    bool IsString() const { return IsType(json_type_string); }

    bool IsObject() const { return IsType(json_type_object); }
    bool IsArray() const  { return IsType(json_type_array); }

    bool Insert(const char * key, const PJSON & obj)
    { if (!IsObject()) return false; json_object_object_add(m_json, key, json_object_get(obj.m_json)); return true; }

    bool Remove(const char * key)
    { if (!IsObject()) return false; json_object_object_del(m_json, key); return true; }

    bool Append(const PJSON & obj)
    { if (!IsArray()) return false; json_object_array_add(m_json, json_object_get(obj.m_json)); return true; }

    size_t GetSize() const
    { return m_json == NULL ? 0 : json_object_array_length(m_json); }

    ostream & operator << (ostream & strm) const
    { PrintOn(strm); return strm; }

    virtual void PrintOn(ostream & strm) const
    { if (m_json != NULL) strm << json_object_to_json_string(m_json); }

    void push_back(const PJSON & obj)
    { Append(obj); }

    size_t size() const
    { return GetSize(); }

  protected:
    PJSON(json_object * j)
      : m_json(j)
    { }

    PJSON(bool v)
    { m_json = json_object_new_boolean(v); }

    PJSON(double v)
    { m_json = json_object_new_double(v); }

    PJSON(int v)
    { m_json = json_object_new_int(v); }

    PJSON(const char * v)
    { m_json = json_object_new_string(v); }

    bool IsType(enum json_type type) const { return json_object_is_type(m_json, type); }

    json_object * m_json;
};

ostream & operator << (ostream & strm, const PJSON & base)
{ base.PrintOn(strm); return strm; }

#endif  // PTLIB_PJSON_H
