/*
 * bitwise_enum.h
 *
 * Template class to allow operators on an enum representing bits.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2009 Vox Lucida
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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_BITWISE_ENUM_H
#define PTLIB_BITWISE_ENUM_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

/**This class defines a set of operators for a bit wise enumeration.
   This is typically use within the P_DECLARE_BITWISE_ENUM() which allows the
   enumeration to be correctly defined.
 */
template <typename BaseEnum, BaseEnum MaxValue, typename BaseInt = unsigned>
class PBitwiseEnum
{
  public:
    typedef BaseEnum Enumeration;
    typedef BaseInt  IntType;

  protected:
    Enumeration m_enum;

  public:
    __inline PBitwiseEnum(Enumeration e = (Enumeration)0) : m_enum(e) { }
    __inline PBitwiseEnum(const PBitwiseEnum & e) : m_enum(e.m_enum) { }

    enum IteratorBounds { First, Last };
    __inline PBitwiseEnum(IteratorBounds b) : m_enum(b == First ? (Enumeration)1 : MaxValue) { }

    __inline PBitwiseEnum & operator=(const PBitwiseEnum & e) { m_enum = e.m_enum; return *this; }
    __inline PBitwiseEnum & operator=(Enumeration          e) { m_enum = e; return *this; }

    __inline operator Enumeration() const        { return m_enum; }
    __inline operator Enumeration&()             { return m_enum; }
    __inline operator const Enumeration&() const { return m_enum; }
    __inline Enumeration * operator&()           { return &m_enum; }
    __inline unsigned AsBits() const             { return m_enum; }
    __inline static PBitwiseEnum FromBits(unsigned b) { return (Enumeration)(    b &((MaxValue<<1)-1)); }
    __inline static PBitwiseEnum FromBit (unsigned b) { return (Enumeration)((1<<b)&((MaxValue<<1)-1)); }

    PBitwiseEnum operator++()
    {
      Enumeration previous = m_enum;
      if (m_enum < MaxValue)
        m_enum = static_cast<Enumeration>(m_enum << 1);
      return previous;
    }

    PBitwiseEnum operator++(int)
    {
      if (m_enum < MaxValue)
        m_enum = static_cast<Enumeration>(static_cast<IntType>(m_enum) << 1);
      return *this;
    }

    PBitwiseEnum operator--()
    {
      Enumeration previous = m_enum;
      m_enum = static_cast<Enumeration>(static_cast<IntType>(m_enum) >> 1);
      return previous;
    }

    PBitwiseEnum operator--(int)
    {
      m_enum = static_cast<Enumeration>(static_cast<IntType>(m_enum) >> 1);
      return *this;
    }


#define P_BITWISE_ENUM_INTERNAL_OP1(op) (static_cast<IntType>(m_enum) op static_cast<IntType>(rhs))
#define P_BITWISE_ENUM_INTERNAL_OP2(op) static_cast<Enumeration>(P_BITWISE_ENUM_INTERNAL_OP1(op))

    __inline PBitwiseEnum & operator|=(Enumeration  rhs)       { m_enum = P_BITWISE_ENUM_INTERNAL_OP2( | ); return *this; }
    __inline PBitwiseEnum & operator+=(Enumeration  rhs)       { m_enum = P_BITWISE_ENUM_INTERNAL_OP2( | ); return *this; }
    __inline PBitwiseEnum & operator-=(Enumeration  rhs)       { m_enum = P_BITWISE_ENUM_INTERNAL_OP2(& ~); return *this; }
    __inline PBitwiseEnum & operator^=(Enumeration  rhs)       { m_enum = P_BITWISE_ENUM_INTERNAL_OP2( ^ ); return *this; }

    __inline PBitwiseEnum   operator+ (Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP2( | ); }
    __inline PBitwiseEnum   operator| (Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP2( | ); }
    __inline PBitwiseEnum   operator- (Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP2(& ~); }
    __inline PBitwiseEnum   operator^ (Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP2( ^ ); }

    __inline bool           operator& (Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP1(&) != 0; }
    __inline bool           operator==(Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP1(==); }
    __inline bool           operator<=(Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP1(<=); }
    __inline bool           operator>=(Enumeration  rhs) const { return P_BITWISE_ENUM_INTERNAL_OP1(>=); }

    __inline bool           operator!=(Enumeration  rhs) const { return !operator==(rhs); }
    __inline bool           operator< (Enumeration  rhs) const { return !operator>=(rhs); }
    __inline bool           operator> (Enumeration  rhs) const { return !operator<=(rhs); }

    __inline PBitwiseEnum & operator|=(PBitwiseEnum rhs)       { return operator|=(rhs.m_enum); }
    __inline PBitwiseEnum & operator+=(PBitwiseEnum rhs)       { return operator+=(rhs.m_enum); }
    __inline PBitwiseEnum & operator-=(PBitwiseEnum rhs)       { return operator-=(rhs.m_enum); }
    __inline PBitwiseEnum & operator^=(PBitwiseEnum rhs)       { return operator^=(rhs.m_enum); }

    __inline PBitwiseEnum   operator| (PBitwiseEnum rhs) const { return operator|(rhs.m_enum); }
    __inline PBitwiseEnum   operator+ (PBitwiseEnum rhs) const { return operator+(rhs.m_enum); }
    __inline PBitwiseEnum   operator- (PBitwiseEnum rhs) const { return operator-(rhs.m_enum); }
    __inline PBitwiseEnum   operator^ (PBitwiseEnum rhs) const { return operator^(rhs.m_enum); }

    __inline bool           operator& (PBitwiseEnum rhs) const { return operator& (rhs.m_enum); }
    __inline bool           operator==(PBitwiseEnum rhs) const { return operator==(rhs.m_enum); }
    __inline bool           operator<=(PBitwiseEnum rhs) const { return operator<=(rhs.m_enum); }
    __inline bool           operator>=(PBitwiseEnum rhs) const { return operator>=(rhs.m_enum); }

    __inline bool           operator!=(PBitwiseEnum rhs) const { return !operator==(rhs); }
    __inline bool           operator< (PBitwiseEnum rhs) const { return !operator>=(rhs); }
    __inline bool           operator> (PBitwiseEnum rhs) const { return !operator<=(rhs); }
};


#define P_DECLARE_BITWISE_ENUM_1(_0,_1)_0=0,_1=1
#define P_DECLARE_BITWISE_ENUM_2(_0,_1,_2)P_DECLARE_BITWISE_ENUM_1(_0,_1),_2=2
#define P_DECLARE_BITWISE_ENUM_3(_0,_1,_2,_3)P_DECLARE_BITWISE_ENUM_2(_0,_1,_2),_3=4
#define P_DECLARE_BITWISE_ENUM_4(_0,_1,_2,_3,_4)P_DECLARE_BITWISE_ENUM_3(_0,_1,_2,_3),_4=8
#define P_DECLARE_BITWISE_ENUM_5(_0,_1,_2,_3,_4,_5)P_DECLARE_BITWISE_ENUM_4(_0,_1,_2,_3,_4),_5=16
#define P_DECLARE_BITWISE_ENUM_6(_0,_1,_2,_3,_4,_5,_6)P_DECLARE_BITWISE_ENUM_5(_0,_1,_2,_3,_4,_5),_6=32
#define P_DECLARE_BITWISE_ENUM_7(_0,_1,_2,_3,_4,_5,_6,_7)P_DECLARE_BITWISE_ENUM_6(_0,_1,_2,_3,_4,_5,_6),_7=64
#define P_DECLARE_BITWISE_ENUM_8(_0,_1,_2,_3,_4,_5,_6,_7,_8)P_DECLARE_BITWISE_ENUM_7(_0,_1,_2,_3,_4,_5,_6,_7),_8=128
#define P_DECLARE_BITWISE_ENUM_9(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9)P_DECLARE_BITWISE_ENUM_8(_0,_1,_2,_3,_4,_5,_6,_7,_8),_9=256

#define P_DECLARE_BITWISE_NAMES_1(_0,_1)#_0,#_1
#define P_DECLARE_BITWISE_NAMES_2(_0,_1,_2)P_DECLARE_BITWISE_NAMES_1(_0,_1),#_2
#define P_DECLARE_BITWISE_NAMES_3(_0,_1,_2,_3)P_DECLARE_BITWISE_NAMES_2(_0,_1,_2),#_3
#define P_DECLARE_BITWISE_NAMES_4(_0,_1,_2,_3,_4)P_DECLARE_BITWISE_NAMES_3(_0,_1,_2,_3),#_4
#define P_DECLARE_BITWISE_NAMES_5(_0,_1,_2,_3,_4,_5)P_DECLARE_BITWISE_NAMES_4(_0,_1,_2,_3,_4),#_5
#define P_DECLARE_BITWISE_NAMES_6(_0,_1,_2,_3,_4,_5,_6)P_DECLARE_BITWISE_NAMES_5(_0,_1,_2,_3,_4,_5),#_6
#define P_DECLARE_BITWISE_NAMES_7(_0,_1,_2,_3,_4,_5,_6,_7)P_DECLARE_BITWISE_NAMES_6(_0,_1,_2,_3,_4,_5,_6),#_7
#define P_DECLARE_BITWISE_NAMES_8(_0,_1,_2,_3,_4,_5,_6,_7,_8)P_DECLARE_BITWISE_NAMES_7(_0,_1,_2,_3,_4,_5,_6,_7),#_8
#define P_DECLARE_BITWISE_NAMES_9(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9)P_DECLARE_BITWISE_NAMES_8(_0,_1,_2,_3,_4,_5,_6,_7,_8),#_9

#define P_DECLARE_BITWISE_ENUM_FRIENDS(name) \
  __inline friend name##_Bits operator+(name##_Bits lhs, name##_Bits rhs) \
    { return static_cast<name##_Bits>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)); } \
  __inline friend name##_Bits operator|(name##_Bits lhs, name##_Bits rhs) \
    { return static_cast<name##_Bits>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)); } \
  __inline friend name##_Bits operator-(name##_Bits lhs, name##_Bits rhs) \
    { return static_cast<name##_Bits>(static_cast<unsigned>(lhs) & ~static_cast<unsigned>(rhs)); }

#define P_DECLARE_BITWISE_ENUM_END(name, count) \
  P_DECLARE_BITWISE_ENUM_FRIENDS(name) \
  typedef PBitwiseEnum<name##_Bits, (name##_Bits)(1<<(count-1))> name


/**This macro can be used to declare a bit wise enumeration, using the
   PBitWiseEnum template class.

   Example:
     P_DECLARE_BITWISE_ENUM(MyBitWiseEnum, 3, (ZeroValue, FirstBit, SecondBit, ThirdBit));

   Which will decclare the values for an enum MyBitWiseEnum_Bits of
   ZeroValue = 0, FirstBit = 1, SecondBit = 2, ThirdBit = 4. And the PBitWiseEnum
   template class will have a typedef to \p name.
  */
#define P_DECLARE_BITWISE_ENUM(name, count, values) \
  enum name##_Bits { P_DECLARE_BITWISE_ENUM_##count values }; \
  P_DECLARE_BITWISE_ENUM_END(name, count)

/**This macro can be used to declare a bit wise enumeration, using the
   PBitWiseEnum template class.

   This si similar to P_DECLARE_BITWISE_ENUM but allows extra value to be
   added to the enum for bit combinations, for example:
     P_DECLARE_BITWISE_ENUM_EX(MyBitWiseEnum, 3,
                              (ZeroValue, FirstBit, SecondBit, ThirdBit),
                               LowBits = FirstBit|SecondBit,
                               AllBits = FirstBit|SecondBit|ThirdBit);

  */
#define P_DECLARE_BITWISE_ENUM_EX(name, count, values, ...) \
  enum name##_Bits { P_DECLARE_BITWISE_ENUM_##count values , ##__VA_ARGS__ }; \
  P_DECLARE_BITWISE_ENUM_END(name, count)


extern void PPrintBitwiseEnum(std::ostream & strm, unsigned bits, char const * const * names);
extern unsigned PReadBitwiseEnum(std::istream & strm, char const * const * names);


#define P_DECLARE_STREAMABLE_BITWISE_ENUM_EX(name, count, values, ...) \
  enum name##_Bits { P_DECLARE_BITWISE_ENUM_##count values }; \
  P_DECLARE_BITWISE_ENUM_FRIENDS(name) \
  class name : public PBitwiseEnum<name##_Bits, (name##_Bits)(1<<count)>{ \
    public: typedef PBitwiseEnum<name##_Bits, (name##_Bits)(1<<count)> BaseClass; \
    __inline name(BaseClass::Enumeration e = (BaseClass::Enumeration)0) : BaseClass(e) { } \
    __inline explicit name(const PString & s) { FromString(s); } \
    __inline name(IteratorBounds b) : BaseClass(b) { } \
    static char const * const * Names() { static char const * const Strings[] = { __VA_ARGS__, NULL }; return Strings; } \
    friend __inline std::ostream & operator<<(std::ostream & strm, const name & e) { PPrintBitwiseEnum(strm, e.AsBits(), name::Names()); return strm; } \
    friend __inline std::istream & operator>>(std::istream & strm, name & e) { e = (BaseClass::Enumeration)PReadBitwiseEnum(strm, name::Names()); return strm; } \
    PString ToString() { PStringStream strm; strm >> *this; return strm; } \
    bool FromString(const PString & s) { PStringStream strm(s); strm >> *this; return strm.good(); } \
  }

#define P_DECLARE_STREAMABLE_BITWISE_ENUM(name, count, values) \
        P_DECLARE_STREAMABLE_BITWISE_ENUM_EX(name, count, values, P_DECLARE_BITWISE_NAMES_##count values)


#endif // PTLIB_BITWISE_ENUM_H


// End Of File ///////////////////////////////////////////////////////////////
