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

        bool Set(const PString & name, Types type);
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

        void Append(Types type);
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
        Number(double value = 0);
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        Number & operator=(double value) { m_value = value; return *this; }
    };

    class Boolean : public Base
    {
      protected:
        bool m_value;
      public:
        Boolean(bool value = false);
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        Boolean & operator=(bool value) { m_value = value; return *this; }
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

    PString AsString() const;

    bool IsValid() const { return m_root != NULL; }

    Object & GetObject() const { return dynamic_cast<Object &>(*m_root); }
    Array  & GetArray () const { return dynamic_cast<Array &>(*m_root); }
    String & GetString() const { return dynamic_cast<String &>(*m_root); }
    Number & GetNumber() const { return dynamic_cast<Number &>(*m_root); }
    Boolean & GetBoolean() const { return dynamic_cast<Boolean &>(*m_root); }

  protected:
    Base * m_root;
};


#endif  // PTLIB_PJSON_H
