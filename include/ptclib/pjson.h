/*
 * pjson.h
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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PJSON_H
#define PTLIB_PJSON_H

#include <ptlib.h>

class PJSON : public PObject
{
  PCLASSINFO(PJSON, PObject)
  public:
    enum Types
    {
      e_Object,
      e_Array,
      e_String,
      e_Number,
      e_Boolean,
      e_Null
    };

    class Base
    {
      public:
        Base() { }
        virtual ~Base() { }
        virtual bool IsType(Types type) const = 0;
        virtual void ReadFrom(istream & strm) = 0;
        virtual void PrintOn(ostream & strm) const = 0;
      private:
        Base(const Base &) { }
        void operator=(const Base &);
    };

    class Array;

    class Object : public Base, public std::map<PString, Base *>
    {
      public:
        ~Object();
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;

        template <class T> T * Get(const PString & name) const
        {
          const_iterator it = find(name);
          return it != end() ? dynamic_cast<T *>(it->second) : NULL;
        }
        Object & GetObject(const PString & name) const;
        Array & GetArray(const PString & name) const;
        PString GetString(const PString & name) const;
        int GetInteger(const PString & name) const;
        unsigned GetUnsigned(const PString & name) const;
        double GetNumber(const PString & name) const;
        bool GetBoolean(const PString & name) const;

        bool Set(const PString & name, Types type);
        Object & SetObject(const PString & name);
        Array & SetArray(const PString & name);
        bool SetString(const PString & name, const PString & value);
        bool SetNumber(const PString & name, double value);
        bool SetBoolean(const PString & name, bool value);
    };

    class Array : public Base, public std::vector<Base *>
    {
      public:
        ~Array();
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;

        template <class T> T * Get(size_t index) const
        {
          return index < size() ? dynamic_cast<T *>(at(index)) : NULL;
        }
        Object & GetObject(size_t index) const;
        Array & GetArray(size_t index) const;
        PString GetString(size_t index) const;
        int GetInteger(size_t index) const;
        unsigned GetUnsigned(size_t index) const;
        double GetNumber(size_t index) const;
        bool GetBoolean(size_t index) const;

        void Append(Types type);
        Object & AppendObject();
        Array & AppendArray();
        void AppendString(const PString & value);
        void AppendNumber(double value);
        void AppendBoolean(bool value);
    };

    class String : public Base, public PString
    {
      public:
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        String & operator=(const char * str) { PString::operator=(str); return *this; }
        String & operator=(const PString & str) { PString::operator=(str); return *this; }
    };

    class Number : public Base
    {
      protected:
        double m_value;
      public:
        explicit Number(double value = 0);
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        Number & operator=(double value) { m_value = value; return *this; }
        void SetValue(double value) { m_value = value; }
        double GetValue() const { return m_value; }
    };

    class Boolean : public Base
    {
      protected:
        bool m_value;
      public:
        explicit Boolean(bool value = false);
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        Boolean & operator=(bool value) { m_value = value; return *this; }
        void SetValue(bool value) { m_value = value; }
        bool GetValue() const { return m_value; }
    };

    class Null : public Base
    {
      public:
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
    };

    ///< Constructor
    PJSON();
    explicit PJSON(Types type);
    explicit PJSON(const PString & str);

    ~PJSON() { delete m_root; }

    virtual void ReadFrom(istream & strm);
    virtual void PrintOn(ostream & strm) const;

    bool FromString(
      const PString & str
    );

    PString AsString(std::streamsize indent = 0) const;

    bool IsValid() const { return m_valid; }

    template <class T> T & GetAs() const { return dynamic_cast<T &>(*PAssertNULL(m_root)); }
    Object  & GetObject()  const { return GetAs<Object>();  }
    Array   & GetArray ()  const { return GetAs<Array>();   }
    String  & GetString()  const { return GetAs<String>();  }
    Number  & GetNumber()  const { return GetAs<Number>();  }
    Boolean & GetBoolean() const { return GetAs<Boolean>(); }

  protected:
    Base * m_root;
    bool   m_valid;
};


#endif  // PTLIB_PJSON_H
