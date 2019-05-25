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
        virtual Base * DeepClone() const = 0;
      private:
        Base(const Base &) { }
        void operator=(const Base &);

      friend ostream & operator<<(ostream & s, const Base & b) { b.PrintOn(s); return s; }
    };

    class Array;

    typedef long double NumberType;

    class Object : public Base, public std::map<PString, Base *>
    {
      public:
        Object() { }
        ~Object();
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        virtual Base * DeepClone() const;

        bool IsType(const PString & name, Types type) const;

        template <class T> const T * Get(const PString & name) const
        {
          const_iterator it = find(name);
          return it != end() ? dynamic_cast<const T *>(it->second) : NULL;
        }
        template <class T> T * Get(const PString & name)
        {
          iterator it = find(name);
          return it != end() ? dynamic_cast<T *>(it->second) : NULL;
        }
        const Object & GetObject(const PString & name) const { return *Get<Object>(name); }
              Object & GetObject(const PString & name)       { return *Get<Object>(name); }
        const Array & GetArray(const PString & name)   const { return *Get<Array>(name); }
              Array & GetArray(const PString & name)         { return *Get<Array>(name); }

        PString GetString(const PString & name) const;
        int GetInteger(const PString & name) const;
        int64_t GetInteger64(const PString & name) const;
        unsigned GetUnsigned(const PString & name) const;
        uint64_t GetUnsigned64(const PString & name) const;
        NumberType GetNumber(const PString & name) const;
        bool GetBoolean(const PString & name) const;

        bool Set(const PString & name, Types type);
        bool Set(const PString & name, const Base & toInsert);
        bool Set(const PString & name, const PJSON & toInsert);
        Object & SetObject(const PString & name);
        Array & SetArray(const PString & name);
        bool SetString(const PString & name, const PString & value);
        bool SetNumber(const PString & name, NumberType value);
        bool SetBoolean(const PString & name, bool value);

      private:
        Object(const Object &) { }
        void operator=(const Object &) { }
    };

    class Array : public Base, public std::vector<Base *>
    {
      public:
        Array() { }
        ~Array();
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        virtual Base * DeepClone() const;

        bool IsType(size_t index, Types type) const;

        template <class T> const T * Get(size_t index) const { return index < size() ? dynamic_cast<const T *>(at(index)) : NULL; }
        template <class T>       T * Get(size_t index)       { return index < size() ? dynamic_cast<      T *>(at(index)) : NULL; }
        const Object & GetObject(size_t index)         const { return *Get<Object>(index); }
              Object & GetObject(size_t index)               { return *Get<Object>(index); }
        const Array & GetArray(size_t index)           const { return *Get<Array>(index); }
              Array & GetArray(size_t index)                 { return *Get<Array>(index); }

        PString GetString(size_t index) const;
        int GetInteger(size_t index) const;
        int64_t GetInteger64(size_t index) const;
        unsigned GetUnsigned(size_t index) const;
        uint64_t GetUnsigned64(size_t index) const;
        NumberType GetNumber(size_t index) const;
        bool GetBoolean(size_t index) const;

        void Append(Types type);
        void Append(const Base & toAppend);
        void Append(const PJSON & toAppend);
        Object & AppendObject();
        Array & AppendArray();
        void AppendString(const PString & value);
        void AppendNumber(NumberType value);
        void AppendBoolean(bool value);

      private:
        Array(const Array &) { }
        void operator=(const Array &) { }
    };

    class String : public Base, public PString
    {
      public:
        String(const char * str = NULL) : PString(str) { }
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        virtual Base * DeepClone() const;
        String & operator=(const char * str) { PString::operator=(str); return *this; }
        String & operator=(const PString & str) { PString::operator=(str); return *this; }
    };

    class Number : public Base
    {
      protected:
        NumberType m_value;
      public:
        explicit Number(NumberType value = 0);
        virtual bool IsType(Types type) const;
        virtual void ReadFrom(istream & strm);
        virtual void PrintOn(ostream & strm) const;
        virtual Base * DeepClone() const;
        Number & operator=(NumberType value) { m_value = value; return *this; }
        void SetValue(NumberType value) { m_value = value; }
        NumberType GetValue() const { return m_value; }
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
        virtual Base * DeepClone() const;
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
        virtual Base * DeepClone() const;
    };

    ///< Constructor
    PJSON();
    explicit PJSON(Types type);
    explicit PJSON(const PString & str);
    PJSON(const PJSON & other);
    PJSON & operator=(const PJSON & other);

    ~PJSON() { delete m_root; }

    virtual void ReadFrom(istream & strm);
    virtual void PrintOn(ostream & strm) const;

    bool FromString(
      const PString & str
    );

    PString AsString(
      std::streamsize initialIndent = 0,
      std::streamsize subsequentIndent = 0) const;

    bool IsValid() const { return m_valid; }

    bool IsType(Types type) const { return PAssertNULL(m_root)->IsType(type); }

    void Set(Types type);

    template <class T> const T & GetAs() const { return dynamic_cast<const T &>(*PAssertNULL(m_root)); }
    template <class T>       T & GetAs()       { return dynamic_cast<      T &>(*PAssertNULL(m_root)); }
    const Object  & GetObject()  const { return GetAs<Object>();  }
          Object  & GetObject()        { return GetAs<Object>();  }
    const Array   & GetArray ()  const { return GetAs<Array>();   }
          Array   & GetArray ()        { return GetAs<Array>();   }
    const String  & GetString()  const { return GetAs<String>();  }
          String  & GetString()        { return GetAs<String>();  }
    const Number  & GetNumber()  const { return GetAs<Number>();  }
          Number  & GetNumber()        { return GetAs<Number>();  }
    const Boolean & GetBoolean() const { return GetAs<Boolean>(); }
          Boolean & GetBoolean()       { return GetAs<Boolean>(); }

  protected:
    Base * m_root;
    bool   m_valid;
};


#if P_SSL

/** Encode/Decode JSON payload as a JSON Web Token.
    The JSON is always of Object type.
  */
class PJWT : public PJSON
{
    PCLASSINFO(PJWT, PJSON);
  public:
    /// Construct empty Object type JSON
    PJWT();

    /** Create and decode the JWT.
        Use IsValid() afterward to cerify if decode was successful.
     */
    explicit PJWT(
      const PString & str,
      const PString & secret = PString::Empty(),
      const PTime & verifyTime = PTime(0)
    );

    /// Available token algorithms 
    P_DECLARE_STREAMABLE_ENUM(Algorithm,
      none,
      HS256,  // HMAC SHA-256
      HS384,  // HMAC SHA-384
      HS512   // HMAC SHA-512
    );

    /**Encode the JWT using the shared secret and algorithm.
      */
    PString Encode(
      const PString & secret = PString::Empty(),  ///< Shared secret
      const Algorithm algorithm = HS256           ///< Algorithm
    );

    /**Decode the JWT using the shared secret and algorithm.
      */
    bool Decode(
      const PString & str,                        ///< Encoded JWT string
      const PString & secret = PString::Empty(),  ///< Shared secret
      const PTime & verifyTime = PTime(0)         ///< Optional time to use for verification
    );

    void SetIssuer(const PString & str);
    PString GetIssuer() const;
    void SetSubject(const PString & str);
    PString GetSubject() const;
    void SetAudience(const PString & str);
    PString GetAudience() const;
    void SetExpiration(const PTime & when);
    PTime GetExpiration() const;
    void SetNotBefore(const PTime & when);
    PTime GetNotBefore() const;
    void SetIssuedAt(const PTime & when);
    PTime GetIssuedAt() const;
    void SetTokenId(const PString & str);
    PString GetTokenId() const;

    void SetPrivate(const PString & key, const PString & str);
    PString GetPrivate(const PString & key) const;
};

#endif // P_SSL

#endif  // PTLIB_PJSON_H
