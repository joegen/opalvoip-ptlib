/*
 * object.h
 *
 * Mother of all ancestor classes.
 *
 * Portable Tools Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_OBJECT_H
#define PTLIB_OBJECT_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#if defined(_WIN32)
#include "msos/ptlib/platform.h"
#else
#include "unix/ptlib/platform.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <limits>
#include <typeinfo>
#include <memory>

using namespace std; // Not a good practice (name space polution), but will take too long to fix.

// Somewhere in C headers you get this, which blows up STL version
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


///////////////////////////////////////////////////////////////////////////////

#define P_REMOVE_VIRTUAL_INTERNAL_BASE(fn) __inline virtual struct ptlib_virtual_function_changed_or_removed ****** fn { return 0; }

#if defined(_MSC_VER)
  #if _MSC_VER < 1310
    #define P_DEPRECATED
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
  #elif _MSC_VER < 1400
    #define P_DEPRECATED __declspec(deprecated)
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) __inline virtual __declspec(deprecated) type fn body
  #else
    #define P_DEPRECATED __declspec(deprecated)
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) __inline virtual __declspec(deprecated("Virtual function signature changed or function deprecated")) type fn body
  #endif
#elif defined(__GNUC__)
  #if __GNUC__ < 4
    #define P_DEPRECATED
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
  #else
    #define P_DEPRECATED __attribute__((deprecated))
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) __attribute__((warn_unused_result)) __attribute__((deprecated)) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
  #endif
#else
    #define P_DEPRECATED
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
#endif

#define P_REMOVE_VIRTUAL_VOID(fn)       P_REMOVE_VIRTUAL_INTERNAL(void, fn, {})
#define P_REMOVE_VIRTUAL(type, fn, ret) P_REMOVE_VIRTUAL_INTERNAL(type, fn, { return ret; })


#ifdef _MSC_VER
  #define P_PUSH_MSVC_WARNINGS(warnings) __pragma(warning(push)) __pragma(warning(disable:warnings))
  #define P_POP_MSVC_WARNINGS() __pragma(warning(pop))
#else
  #define P_PUSH_MSVC_WARNINGS(warnings)
  #define P_POP_MSVC_WARNINGS()
#endif // _MSC_VER
#define P_DISABLE_MSVC_WARNINGS(warnings, statement) P_PUSH_MSVC_WARNINGS(warnings) statement P_POP_MSVC_WARNINGS()


// We are gradually converting over to standard C++ names, these
// are for backward compatibility only

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

typedef bool   PBoolean;
#define PTrue  true
#define PFalse false


///////////////////////////////////////////////////////////////////////////////
// Disable inlines when debugging for faster compiles (the compiler doesn't
// actually inline the function with debug on any way).

#ifndef P_USE_INLINES
#ifdef _DEBUG
#define P_USE_INLINES 0
#else
#define P_USE_INLINES 0
#endif
#endif

#if P_USE_INLINES
#define PINLINE __inline
#else
#define PINLINE
#endif


///////////////////////////////////////////////////////////////////////////////
// Handy macros

/// Count the number of arguments passed in macro
#define PARG_COUNT(...) PARG_COUNT_PART1(PARG_COUNT_PART2(__VA_ARGS__,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))
#define PARG_COUNT_PART1(arg) arg
#define PARG_COUNT_PART2(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,N,...) N

/// Turn the argument into a string
#define P_STRINGISE(v) P_STRINGISE_PART2(v)
#define P_STRINGIZE(v) P_STRINGISE_PART2(v)
#define P_STRINGISE_PART2(v) #v

/** This declares a standard enumeration (enum) of symbols with ++ and -- operators.
    The symbols Begin##name and End##name are automatically added to the enumeration
    and is equal to the first value, and one beyond the last value respectively.
    This operates in a similar manner to STL iterators so that a for loop like:
    <PRE><CODE>
       for (MyEnum e = BeginMyEnum; e < EndMyEnum; ++e)
    </CODE></PRE>
    works as epected.

    A symbol for the count of enumerations (End##name - Begin##name) is also defined.
  */
#define P_DECLARE_ENUM_EX(name, countName, firstName, firstValue, ...) \
  enum name { firstName = firstValue, Begin##name = firstName, __VA_ARGS__, End##name, countName = End##name-Begin##name }; \
  friend __inline name operator++(name & e     ) { PAssert(e <    End##name, PInvalidParameter); return    e = (name)(e+1);           } \
  friend __inline name operator++(name & e, int) { PAssert(e <    End##name, PInvalidParameter); name o=e; e = (name)(e+1); return o; } \
  friend __inline name operator--(name & e     ) { PAssert(e >= Begin##name, PInvalidParameter); return    e = (name)(e-1);           } \
  friend __inline name operator--(name & e, int) { PAssert(e >= Begin##name, PInvalidParameter); name o=e; e = (name)(e-1); return o; } \
  static __inline name name##FromInt(int v) { return (name)(v < Begin##name ? Begin##name : v >= End##name ? (End##name-1) : v); }

/** This declares a standard enumeration (enum) of symbols with ++ and -- operators.
    The symbols Begin##name and End##name are automatically added to the enumeration
    and is equal to the first value, and one beyond the last value respectively.
    This operates in a similar manner to STL iterators so that a for loop like:
    <PRE><CODE>
       for (MyEnum e = BeginMyEnum; e < EndMyEnum; ++e)
    </CODE></PRE>
    works as epected.

    A symbol Num##name for the count of enumerations (End##name - Begin##name) is also defined.
  */
#define P_DECLARE_ENUM(name, first, ...) P_DECLARE_ENUM_EX(name, Num##name, first, 0, __VA_ARGS__)

extern void PPrintEnum(std::ostream & strm, int e, int begin, int end, char const * const * names);
extern int PReadEnum(std::istream & strm, int begin, int end, char const * const * names, bool matchCase = true);
extern int PParseEnum(const char * str, int begin, int end, char const * const * names, bool matchCase = true);

#define P_ENUM_NAMES_PART1(narg, args)P_ENUM_NAMES_PART2(narg, args)
#define P_ENUM_NAMES_PART2(narg, args) P_ENUM_NAMES_ARG_##narg args
#define P_ENUM_NAMES_ARG_1(_1                                                                                                                                                     )#_1
#define P_ENUM_NAMES_ARG_2(_1,_2                                                                                                                                                  )#_1,#_2
#define P_ENUM_NAMES_ARG_3(_1,_2,_3                                                                                                                                               )#_1,#_2,#_3
#define P_ENUM_NAMES_ARG_4(_1,_2,_3,_4                                                                                                                                            )#_1,#_2,#_3,#_4
#define P_ENUM_NAMES_ARG_5(_1,_2,_3,_4,_5                                                                                                                                         )#_1,#_2,#_3,#_4,#_5
#define P_ENUM_NAMES_ARG_6(_1,_2,_3,_4,_5,_6                                                                                                                                      )#_1,#_2,#_3,#_4,#_5,#_6
#define P_ENUM_NAMES_ARG_7(_1,_2,_3,_4,_5,_6,_7                                                                                                                                   )#_1,#_2,#_3,#_4,#_5,#_6,#_7
#define P_ENUM_NAMES_ARG_8(_1,_2,_3,_4,_5,_6,_7,_8                                                                                                                                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8
#define P_ENUM_NAMES_ARG_9(_1,_2,_3,_4,_5,_6,_7,_8,_9                                                                                                                             )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9
#define P_ENUM_NAMES_ARG_10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10                                                                                                                        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10
#define P_ENUM_NAMES_ARG_11(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11                                                                                                                    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11
#define P_ENUM_NAMES_ARG_12(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12                                                                                                                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12
#define P_ENUM_NAMES_ARG_13(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13                                                                                                            )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13
#define P_ENUM_NAMES_ARG_14(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14                                                                                                        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14
#define P_ENUM_NAMES_ARG_15(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15                                                                                                    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15
#define P_ENUM_NAMES_ARG_16(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16                                                                                                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16
#define P_ENUM_NAMES_ARG_17(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17                                                                                            )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17
#define P_ENUM_NAMES_ARG_18(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18                                                                                        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18
#define P_ENUM_NAMES_ARG_19(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19                                                                                    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19
#define P_ENUM_NAMES_ARG_20(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20                                                                                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20
#define P_ENUM_NAMES_ARG_21(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21                                                                            )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21
#define P_ENUM_NAMES_ARG_22(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22                                                                        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22
#define P_ENUM_NAMES_ARG_23(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23                                                                    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23
#define P_ENUM_NAMES_ARG_24(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24                                                                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24
#define P_ENUM_NAMES_ARG_25(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25                                                            )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25
#define P_ENUM_NAMES_ARG_26(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26                                                        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26
#define P_ENUM_NAMES_ARG_27(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27                                                    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27
#define P_ENUM_NAMES_ARG_28(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28                                                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28
#define P_ENUM_NAMES_ARG_29(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29                                            )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29
#define P_ENUM_NAMES_ARG_30(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30                                        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30
#define P_ENUM_NAMES_ARG_31(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31                                    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31
#define P_ENUM_NAMES_ARG_32(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32                                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32
#define P_ENUM_NAMES_ARG_33(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33                            )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33
#define P_ENUM_NAMES_ARG_34(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34                        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33,#_34
#define P_ENUM_NAMES_ARG_35(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35                    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33,#_34,#_35
#define P_ENUM_NAMES_ARG_36(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36                )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33,#_34,#_35,#_36
#define P_ENUM_NAMES_ARG_37(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37            )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33,#_34,#_35,#_36,#_37
#define P_ENUM_NAMES_ARG_38(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38        )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33,#_34,#_35,#_36,#_37,#_38
#define P_ENUM_NAMES_ARG_39(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39    )#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33,#_34,#_35,#_36,#_37,#_38,#_39
#define P_ENUM_NAMES_ARG_40(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40)#_1,#_2,#_3,#_4,#_5,#_6,#_7,#_8,#_9,#_10,#_11,#_12,#_13,#_14,#_15,#_16,#_17,#_18,#_19,#_20,#_21,#_22,#_23,#_24,#_25,#_26,#_27,#_28,#_29,#_30,#_31,#_32,#_33,#_34,#_35,#_36,#_37,#_38,#_39,#_40

/** This declares the functions and string table for the text names of an enum.
    The enum is expected to be defined using P_DECLARE_ENUM() or P_DECLARE_ENUM_EX()
    and adds functions MyEnumToString() and MyEnumFromString() to convert between
    the string representation and the enum. The iostream operator<< and operator>>
    is also defined.
  */
#define P_DECLARE_ENUM_NAMES(name, ...) \
  struct PEnumNames_##name { \
    static char const * const * Names() { static char const * const Strings[] = { __VA_ARGS__ }; return Strings; } \
  }; \
  friend __inline std::ostream & operator<<(std::ostream & strm, name e) \
    { PPrintEnum(strm, e, Begin##name, End##name, PEnumNames_##name::Names()); return strm; } \
  friend __inline std::istream & operator>>(std::istream & strm, name & e) \
    { e = (name)PReadEnum(strm, Begin##name, End##name, PEnumNames_##name::Names()); return strm; } \
  static __inline const char * name##ToString(name e) { return e >= Begin##name && e < End##name ? PAssertNULL(PEnumNames_##name::Names()[e-Begin##name]) : ""; } \
  static __inline name name##FromString(const char * str, bool matchCase = true) { return (name)PParseEnum(str, Begin##name, End##name, PEnumNames_##name::Names(), matchCase); }

/** This declares a standard enumeration using P_DECLARE_ENUM_EX() and adds
    the text names so can be streamed using operator<< or operator>>. See
    P_DECLARE_ENUM_NAMES() for more
  */
#define P_DECLARE_STREAMABLE_ENUM_EX(name, countName, firstName, firstValue, ...) \
  P_DECLARE_ENUM_EX(name, countName, firstName, firstValue, __VA_ARGS__) \
  P_DECLARE_ENUM_NAMES(name, #firstName, P_ENUM_NAMES_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__)))

/** This declares a standard enumeration using P_DECLARE_ENUM() and adds
    the text names so can be streamed using operator<< or operator>>. See
    P_DECLARE_ENUM_NAMES() for more
  */
#define P_DECLARE_STREAMABLE_ENUM(name, first, ...) P_DECLARE_STREAMABLE_ENUM_EX(name, Num##name, first, 0, __VA_ARGS__)


///////////////////////////////////////////////////////////////////////////////
// Declare the debugging support

#ifndef P_USE_ASSERTS
#define P_USE_ASSERTS 1
#endif

#if !P_USE_ASSERTS

#define PAssert(b, m) (b)
#define PAssert2(b, c, m) (b)
#define PAssertOS(b) (b)
#define PAssertNULL(p) (p)
#define PAssertAlways(m) {}
#define PAssertAlways2(c, m) {}

#else // P_USE_ASSERTS

/// Standard assert messages for the PAssert macro.
enum PStandardAssertMessage {
  PLogicError,              ///< A logic error occurred.
  POutOfMemory,             ///< A new or malloc failed.
  PNullPointerReference,    ///< A reference was made through a NULL pointer.
  PInvalidCast,             ///< An invalid cast to descendant is required.
  PInvalidArrayIndex,       ///< An index into an array was negative.
  PInvalidArrayElement,     ///< A NULL array element object was accessed.
  PStackEmpty,              ///< A Pop() was made of a stack with no elements.
  PUnimplementedFunction,   ///< Funtion is not implemented.
  PInvalidParameter,        ///< Invalid parameter was passed to a function.
  POperatingSystemError,    ///< Error was returned by Operating System.
  PChannelNotOpen,          ///< Operation attempted when channel not open.
  PUnsupportedFeature,      ///< Feature is not supported.
  PInvalidWindow,           ///< Access through invalid window.
  PMaxStandardAssertMessage ///< Number of standard assert message.
};

#define __CLASS__ NULL

bool PAssertFunc(const char * file, int line, const char * className, PStandardAssertMessage msg);
bool PAssertFunc(const char * file, int line, const char * className, const char * msg);
bool PAssertFunc(const char * full_msg);


/** This macro is used to assert that a condition must be true.
If the condition is false then an assert function is called with the source
file and line number the macro was instantiated on, plus the message described
by the <code>msg</code> parameter. This parameter may be either a standard value
from the <code>PStandardAssertMessage</code> enum or a literal string.
*/
#define PAssert(b, msg) ((b)?true:PAssertFunc(__FILE__,__LINE__,__CLASS__,(msg)))

/** This macro is used to assert that a condition must be true.
If the condition is false then an assert function is called with the source
file and line number the macro was instantiated on, plus the message described
by the <code>msg</code> parameter. This parameter may be either a standard value
from the <code>PStandardAssertMessage</code> enum or a literal string.
The <code>cls</code> parameter specifies the class name that the error occurred in
*/
#define PAssert2(b, cls, msg) ((b)?true:PAssertFunc(__FILE__,__LINE__,(cls),(msg)))

/** This macro is used to assert that an operating system call succeeds.
If the condition is false then an assert function is called with the source
file and line number the macro was instantiated on, plus the message
described by the <code>POperatingSystemError</code> value in the <code>PStandardAssertMessage</code>
enum.
 */
#define PAssertOS(b) ((b)?true:PAssertFunc(__FILE__,__LINE__,__CLASS__,POperatingSystemError))

/** This macro is used to assert that a pointer must be non-null.
If the pointer is NULL then an assert function is called with the source file
and line number the macro was instantiated on, plus the message described by
the PNullPointerReference value in the <code>PStandardAssertMessage</code> enum.

Note that this evaluates the expression defined by <code>ptr</code> twice. To
prevent incorrect behaviour with this, the macro will assume that the
<code>ptr</code> parameter is an L-Value.
 */
#define PAssertNULL(ptr) (((ptr)!=NULL)?(ptr): \
                     (PAssertFunc(__FILE__,__LINE__, __CLASS__, PNullPointerReference),(ptr)))

/** This macro is used to assert immediately.
The assert function is called with the source file and line number the macro
was instantiated on, plus the message described by the <code>msg</code> parameter. This
parameter may be either a standard value from the <code>PStandardAssertMessage</code>
enum or a literal string.
*/
#define PAssertAlways(msg) PAssertFunc(__FILE__,__LINE__,__CLASS__,(msg))

/** This macro is used to assert immediately.
The assert function is called with the source file and line number the macro
was instantiated on, plus the message described by the <code>msg</code> parameter. This
parameter may be either a standard value from the <code>PStandardAssertMessage</code>
enum or a literal string.
*/
#define PAssertAlways2(cls, msg) PAssertFunc(__FILE__,__LINE__,(cls),(msg))

#endif // P_USE_ASSERTS


/** Get the stream being used for error output.
This stream is used for all trace output using the various trace functions
and macros.
*/
ostream & PGetErrorStream();

/** Set the stream to be used for error output.
This stream is used for all error output using the <code>PError</code> macro.
*/
void PSetErrorStream(ostream * strm /** New stream for error output */ );

/** This macro is used to access the platform specific error output stream.
This is to be used in preference to assuming <code>cerr</code> is always available. On
Unix platforms this {\b is} <code>cerr</code> but for MS-Windows this is another stream
that uses the OutputDebugString() Windows API function. Note that a MS-DOS or
Windows NT console application would still use <code>cerr</code>.

The <code>PError</code> stream would normally only be used for debugging information as
a suitable display is not always available in windowed environments.
   
The macro is a wrapper for a global variable error stream. The internal variable
is initialised to <i>std::cerr</i> for all but MS-Windows and NT GUI applications.
An application could change this pointer to a <i>std::ofstream</i> variable of
#PError output is wished to be redirected to a file.
*/
#define PError (PGetErrorStream())


///////////////////////////////////////////////////////////////////////////////
// Debug and tracing

#ifndef PTRACING
#define PTRACING 2
#endif

#if PTRACING

class PObject;
class PArgList;

/**Class to encapsulate tracing functions.
   This class does not require any instances and is only being used as a
   method of grouping functions together in a name space.

   There are a number of macros for supporting tracing. These will all
   evaluate as empty in a "No Trace" build of the system:
     - PTRACE()
     - PTRACE_IF()
     - PTRACE_PARAM()
     - PTRACE_BLOCK()
     - PTRACE_LINE()
  */
class PTrace
{
public:
  /// Options for trace output.
  enum Options {
    Blocks            = 0x0001,   /**< Include PTrace::Block constructs in output
                                       If this is bit is clear, all PTrace::Block output is inhibited
                                       regardless of the trace level. If set, the PTrace::Block may occur
                                       provided the trace level is greater than zero.
                                    */
    DateAndTime       = 0x0002,   ///< Include date and time in all output
    Timestamp         = 0x0004,   ///< Include (millisecond) timestamp in all output
    Thread            = 0x0008,   ///< Include identifier for thread trace is made from in all output
    TraceLevel        = 0x0010,   ///< Include trace level, as a numeric value, in all output
    FileAndLine       = 0x0020,   ///< Include the file and line for the trace call in all output
    ThreadAddress     = 0x0040,   ///< Include thread object pointer address in all trace output
    AppendToFile      = 0x0080,   ///< Append to log file rather than resetting every time
    GMTTime           = 0x0100,   ///< Output timestamps in GMT time rather than local time
    RotateDaily       = 0x0200,   ///< If set, log file will be rotated daily
    RotateHourly      = 0x0400,   ///< If set, log file will be rotated hourly
    RotateMinutely    = 0x0800,   ///< If set, log file will be rotated every minute
    RotateLogMask     = RotateDaily + RotateHourly + RotateMinutely,
                                  ///< Mask for all the rotate bits
    ObjectInstance    = 0x1000,   ///< Include object instance in all trace output
    ContextIdentifier = 0x2000,   ///< Include context identifier in all trace output
    SystemLogStream   = 0x8000,   /**< SystemLog flag for tracing within a PServiceProcess
                                       application. Setting this flag will automatically
                                       execute <code>#SetStream(new PSystemLog)</code>. */
    HasFilePermissions = 0x8000000, ///< Flag indicating file permissions are to be set
    FilePermissionMask = 0x7ff0000, /**< Mask for setting standard file permission mask as used in
                                         open() or creat() system function calls. */
    FilePermissionShift = 16
  };


  #define PTRACE_ARGLIST_OPT_HELP \
    "use +X or -X to add/remove option where X is one of:\r" \
    "  block    PTrace::Block constructs in output\r" \
    "  time     time since prgram start\r" \
    "  date     date and time\r" \
    "  gmt      Date/time is in UTC\r" \
    "  thread   thread name and identifier\r" \
    "  level    log level\r" \
    "  file     source file name and line number\r" \
    "  object   PObject pointer\r" \
    "  context  context identifier\r" \
    "  daily    rotate output file daily\r" \
    "  hour     rotate output file hourly\r" \
    "  minute   rotate output file every minute\r" \
    "  append   append to output file, otherwise overwrites\r" \
    "  <perm>   file permission similar to unix chmod, but starts\r" \
    "           with +/- and only has one combination at a time,\r" \
    "           e.g. +uw is user write, +or is other read, etc"

  #define PTRACE_ARG_TRACE    "trace"
  #define PTRACE_ARG_LEVEL    "trace-level"
  #define PTRACE_ARG_OUTPUT   "output"
  #define PTRACE_ARG_ROLLOVER "trace-rollover"
  #define PTRACE_ARG_OPTION   "trace-option"

  #define PTRACE_ARGLIST_EXT(t,l,o,r,O) \
    t "-" PTRACE_ARG_TRACE ".     Trace enable (use multiple times for more detail).\n" \
    l "-" PTRACE_ARG_LEVEL ":     Specify trace detail level.\n" \
    o "-" PTRACE_ARG_OUTPUT ":    Specify filename for trace output\rMay be special value such as \"stderr\" dependent on platform.\n" \
    r "-" PTRACE_ARG_ROLLOVER ":  Specify trace file rollover file name pattern.\n" \
    O "-" PTRACE_ARG_OPTION ":    Specify trace option(s),\r" PTRACE_ARGLIST_OPT_HELP "\n"

  #define PTRACE_ARGLIST PTRACE_ARGLIST_EXT("t","","o","","")
  
  #define PTRACE_INITIALISE(...) PTrace::Initialise(__VA_ARGS__)

  /**Set the most common trace options.
     This sets trace options based on command line arguments.
    */
  static void Initialise(
    const PArgList & args,                   ///< Command line arguments
    unsigned options =
#ifdef _DEBUG
          FileAndLine |
#endif
                Timestamp | Thread | Blocks, ///< Default #Options for tracing
    const char * traceCount = PTRACE_ARG_TRACE,       ///< Argument option name for trace count
    const char * outputFile = PTRACE_ARG_OUTPUT,      ///< Argument option name for log output file
    const char * traceOpts  = PTRACE_ARG_OPTION,      ///< Argument option name for trace options
    const char * traceRollover = PTRACE_ARG_ROLLOVER, ///< Argument option name for trace file roll over pattern
    const char * traceLevel = PTRACE_ARG_LEVEL        ///< Argument option name for trace level
  );

  /**Set the most common trace options.
     If \p filename is not NULL then a PTextFile is created and attached the
     trace output stream. This object is never closed or deleted until the
     termination of the program.

     There are several special values for \p filename:
       <dl>
       <dt>"stderr"      <dd>Output to standard error
       <dt>"stdout"      <dd>Output to standard output
       <dt>"DEBUGSTREAM" <dd>Output to debugger (Windows only)
       <dt>"syslog,ident,priority,options,facility"
            <dd>Output to syslog (Unix variants only)
                The ident, priority, options & facility components are optional,
                so "syslog", "syslog,myappname", "syslog,myappname,3" are all
                acceptable. See PSystemLogToSyslog for their exact meaning.
       <dt>"network,host:port,facility"
            <dd>Output to RFC3164 network log server.
                The server and facility components are optional. The default
                server would be "localhost" Also, the port is an optional
                field for the server component, defaulting to the RFC3164
                standard value. So, "network,10.0.1.1" or "network,fred:1234"
                are acceptable. See PSystemLogToNetwork for more.
       </dl>

     If \p rolloverPattern is not NULL it is used as the time format pattern
     appended to filename if the #RotateLogMask bits are set. Default is
     "yyyy_MM_dd".

     A trace output of the program name, version, OS abnd other information is
     written to the log immediately.
    */
  static void Initialise(
    unsigned level,                               ///< Level for tracing
    const char * filename = NULL,                 ///< Filename for log output
    unsigned options = Timestamp | Thread | Blocks, ///< #Options for tracing
    const char * rolloverPattern = NULL             ///< Pattern for rolling over trace files
  );

  // Deprecated - for backward compatibility
  static void Initialise(
    unsigned level,
    const char * filename,
    const char * rolloverPattern,
    unsigned options = Timestamp | Thread | Blocks
  ) { Initialise(level, filename, options, rolloverPattern); }

  /** Set the trace options.
      The PTRACE(), PTRACE_BLOCK() and PTRACE_LINE() macros output trace text that
      may contain assorted values. These are defined by the #Options enum.

      Note this function OR's the bits included in the options parameter.
  */
  static void SetOptions(
    unsigned options ///< New option bits for tracing
  );

  /** Clear the trace options.
      The <code>PTRACE()</code>, <code>PTRACE_BLOCK()</code> and
      <code>PTRACE_LINE()</code> macros output trace text that
      may contain assorted values. These are defined by the #Options enum.

      Note this function AND's the complement of the bits included in the options
      parameter.
  */
  static void ClearOptions(
    unsigned options ///< Option bits to turn off
  );

  /** Get the current trace options.
      The <code>PTRACE()</code>, <code>PTRACE_BLOCK()</code> and
      <code>PTRACE_LINE()</code> macros output trace text that
      may contain assorted values. These are defined by the #Options enum.
  */
  static unsigned GetOptions();

  /** Set the trace level.
      The <code>PTRACE()</code> macro checks to see if its level is equal to or lower then the
      level set by this function. If so then the trace text is output to the trace
      stream.
  */
  static void SetLevel(
    unsigned level ///< New level for tracing
  );

  /** Get the trace level.
      The <code>PTRACE()</code> macro checks to see if its level is equal to or lower then the
      level set by this function. If so then the trace text is output to the trace
      stream.
  */
  static unsigned GetLevel();

  /** Determine if the level may cause trace output.
      This checks against the current global trace level set by SetLevel()
      for if the trace output may be emitted. This is used by the PTRACE() macro.
  */
  static PBoolean CanTrace(
    unsigned level ///< Trace level to check
  );

  /** Set the stream to be used for trace output.
      This stream is used for all trace output using the various trace functions
      and macros.
  */
  static void SetStream(
    ostream * out ///< New output stream from trace.
  );

  /** Get the stream being used for trace output.
  */
  static ostream * GetStream();

  /** Output trace parameters (level, output, options etc) to stream.
    */
  static ostream & PrintInfo(
    ostream & strm,
    bool crlf = true
  );

  /** Begin a trace output.
      If the trace stream output is used outside of the provided macros, it
      should be noted that a mutex is obtained on the call to Begin() which
      will prevent any other threads from using the trace stream until the
      End() function is called.

      So a typical usage would be:
      <pre><code>
        ostream & s = PTrace::Begin(3, __FILE__, __LINE__);
        s << "hello";
        if (want_there)
          s << " there";
        s << '!' << PTrace::End;
      </code></pre>
  */
  static ostream & Begin(
    unsigned level,         ///< Log level for output
    const char * fileName,  ///< Filename of source file being traced
    int lineNum,            ///< Line number of source file being traced.
    const PObject * instance = NULL,  ///< Instance for object logging occurred in
    const char * module = NULL        ///< Module or subsection string
  );

  static ostream & Begin(
    unsigned level,           ///< Log level for output
    const char * fileName,    ///< Filename of source file being traced
    int lineNum,              ///< Line number of source file being traced.
    const char * module,      ///< Module or subsection string
    const PObject * instance, ///< Instance for object logging occurred in
    const char * defModule
  ) { return Begin(level, fileName, lineNum, instance, module != NULL ? module : defModule); }

  static ostream & Begin(
    unsigned level,           ///< Log level for output
    const char * fileName,    ///< Filename of source file being traced
    int lineNum,              ///< Line number of source file being traced.
    const PObject * instance, ///< Instance for object logging occurred in
    const PObject * defInstance,
    const char * module       ///< Module or subsection string
  ) { return Begin(level, fileName, lineNum, instance != NULL ? instance : defInstance, module); }

  /** End a trace output.
      If the trace stream output is used outside of the provided macros, the
      End() function must be used at the end of the section of trace
      output. A mutex is obtained on the call to Begin() which will prevent
      any other threads from using the trace stream until the End(). The
      End() is used in a similar manner to <code>std::endl</code> or
      <code>std::flush</code>.

      So a typical usage would be:
      <pre><code>
        ostream & s = PTrace::Begin();
        s << "hello";
        if (want_there)
          s << " there";
        s << '!' << PTrace::End;
      </code></pre>
  */
  static ostream & End(
    ostream & strm ///< Trace output stream being completed
  );

  /** Class to trace Execution blocks.
      This class is used for tracing the entry and exit of program blocks. Upon
      construction it outputs an entry trace message and on destruction outputs an
      exit trace message. This is normally only used from in the <code>PTRACE_BLOCK()</code> macro.
  */
  class Block {
    public:
      /** Output entry trace message. */
      Block(
        const char * fileName, ///< Filename of source file being traced
        int lineNum,           ///< Line number of source file being traced.
        const char * traceName ///< String to be output with trace, typically it is the function name.
      );
      Block(const Block & obj);

      /// Output exit trace message.
      ~Block();

    protected:
      Block & operator=(const Block &)
      { return *this; }
      const char * file;
      int          line;
      const char * name;
  };

  /** Class to reduce noise level for some logging.
      A log is emitted at the lowLevel every interval milliseconds. All logs
      within the time interval are emitted at highLevel.

      An optional log output of the count of times the log occurred during may
      achived by simply including the throttle instance at the end of the log,
      e.g.
      <pre><code>
        PTRACE_THROTTLE_STATIC(m_throttleIt, 2, 2000);
        PTRACE(m_throttleIt, "A very frequent log" << m_throttleIt);
      </code></pre>
    */
  class ThrottleBase
  {
    public:
      ThrottleBase(
        unsigned lowLevel,          ///< Level at which low frequency logs made
        unsigned interval = 60000,  ///< TIme between low frequency logs
        unsigned highLevel = 6      ///> Level for high frequency (every) logs
      );

      bool CanTrace();
      operator unsigned() const { return m_currentLevel; }

      friend ostream & operator<<(ostream & strm, const ThrottleBase & throttle);

    protected:
      unsigned m_interval;
      unsigned m_lowLevel;
      unsigned m_highLevel;
      unsigned m_currentLevel;
      uint64_t m_lastLog;
      unsigned m_count;
  };

  template <unsigned lowLevel, unsigned interval = 60000, unsigned highLevel = 6> struct Throttle : ThrottleBase
  {
    Throttle() : ThrottleBase(lowLevel, interval, highLevel) { }
  };

  static bool CanTrace(const ThrottleBase & throttle) { return const_cast<ThrottleBase &>(throttle).CanTrace(); }

  static void WalkStack(
    ostream & strm,
    PThreadIdentifier id = PNullThreadIdentifier
  );

  static unsigned MaxStackWalk; // Default 20

#if PTRACING==2
  static unsigned GetNextContextIdentifier();
#endif
};

/* Macro to conditionally declare a parameter to a function to avoid compiler
   warning due that parameter only being used in a <code>PTRACE()</code> */
#define PTRACE_PARAM(...) __VA_ARGS__

/** Trace an execution block.
This macro creates a trace variable for tracking the entry and exit of program
blocks. It creates an instance of the PTraceBlock class that will output a
trace message at the line <code>PTRACE_BLOCK()</code> is called and then on exit from the
scope it is defined in.
*/
#define PTRACE_BLOCK(name) PTrace::Block __trace_block_instance(__FILE__, __LINE__, name)

/** Trace the execution of a line.
This macro outputs a trace of a source file line execution.
*/
#define PTRACE_LINE() \
    if (PTrace::CanTrace(1)) \
      PTrace::Begin(1, __FILE__, __LINE__) << __FILE__ << '(' << __LINE__ << ')' << PTrace::End; \
    else (void)0



#define PTRACE_INTERNAL(level, condition, args, ...) \
    if (PTrace::CanTrace(level) condition) \
      PTrace::Begin(level, __FILE__, __LINE__, __VA_ARGS__) << args << PTrace::End; \
    else (void)0

#define PTRACE_NO_CONDITION


#define PTRACE_PART1(narg, args) PTRACE_PART2(narg, args)
#define PTRACE_PART2(narg, args) PTRACE_ARG_##narg args

#define PTRACE_ARG_4(level, object, module, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, object, module)

#define PTRACE_ARG_3(level, objectOrModule, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, objectOrModule, PTraceObjectInstance(objectOrModule), PTraceModule())

#define PTRACE_ARG_2(level, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, PTraceObjectInstance(), PTraceModule())


#define PTRACE_IF_PART1(narg, args) PTRACE_IF_PART2(narg, args)
#define PTRACE_IF_PART2(narg, args) PTRACE_IF_ARG_##narg args

#define PTRACE_IF_ARG_5(level, condition, object, module, args) \
        PTRACE_INTERNAL(level, && (condition), args, object, module)

#define PTRACE_IF_ARG_4(level, condition, objectOrModule, args) \
        PTRACE_INTERNAL(level, && (condition), args, objectOrModule, PTraceObjectInstance(objectOrModule), PTraceModule())

#define PTRACE_IF_ARG_3(level, condition, args) \
        PTRACE_INTERNAL(level, && (condition), args, PTraceObjectInstance(), PTraceModule())


#define PTRACE_BEGIN_PART1(narg, args) PTRACE_BEGIN_PART2(narg, args)
#define PTRACE_BEGIN_PART2(narg, args) PTRACE_BEGIN_ARG_##narg args

#define PTRACE_BEGIN_ARG_3(level, object, module) \
      PTrace::Begin(level, __FILE__, __LINE__, object, module)

#define PTRACE_BEGIN_ARG_2(level, objectOrModule) \
      PTrace::Begin(level, __FILE__, __LINE__, objectOrModule, PTraceObjectInstance(objectOrModule), PTraceModule())

#define PTRACE_BEGIN_ARG_1(level) \
      PTrace::Begin(level, __FILE__, __LINE__, PTraceObjectInstance(), PTraceModule())


// Backward compatibility
#define PTRACE2(level, object, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, object, PTraceModule())
#define PTRACE_IF2(level, condition, object, args) \
  PTRACE_INTERNAL(level, && (condition), args, object, PTraceModule())

/** Output trace.
This macro outputs a trace of any information needed, using standard stream
output operators. The output is only made if the trace level set by the
SetLevel() function is greater than or equal to the first argument.

There can be variable numbers of arguments to this macro. Its full form is:

  PTRACE(level, instance, module, stream)

The level is the level for this trace log, instance is a PObject instance to
generate a context for the trace, module is a string representing a subsytem
for filtering and stream is a standard C++ stream output expression.

Both instance and module can by NULL, or absent. If only one of instance or
module is present it is determined by its type (PObject * or char *) as to
which it is.

The stream is only evaluated if level is below the PTrace::GetLevel()
threshold, so do not expect any functions to always be executed.

Note: If this is used with a static function of a PObject descendant there
will be an issue with the default usage of PTraceObjectInstance(). To avoid
the issue you will need to make sure the PTRACE macro has four parameters, as
in the full form descibed above.

The general policy for levels in trace logs is:
  Level 0 - Fatal error - program will likely crash
  Level 1 - Error       - program should continue, but call will likely fail.
  Level 2 - Warning     - something not right, call might continue with something missing
  Level 3 - Info        - all should be well, but enough information to track something odd
  Level 4 - Debug1      - More info on what's ahppening, including protocol packet dumps
  Level 5 - Debug2      - A lot more information, including more details on protocol
  Level 6 - Debug3      - A ridiculous amount of debugging, includes media packets.
  Level 7 - Debug4      - A hard disk filler. Used only to track down the nastiest of problems
*/
#define PTRACE(...) PTRACE_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))

/** Output trace on condition.
This macro conditionally outputs a trace of any information needed.

This macro has variable arguments, and is of the form:

  PTRACE_IF(level, condition, instance, module, stream)

Note condition is only evaluated if level is below the PTrace::GetLevel()
threshold, and stream is only evaluated if condition is evaluated to true.

See PTRACE() for more information on level, instance, module and stream.
*/
#define PTRACE_IF(...) PTRACE_IF_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))

/** Begin output trace.
This macro returns a ostream & for trace output.

This macro has variable arguments, and is of the form:

  PTRACE_BEGIN(level, instance, module)

See PTRACE() for more information on level, instance, module.
*/
#define PTRACE_BEGIN(...) PTRACE_BEGIN_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))


/* Macro to create a throttle context for use in <code>PTRACE()</code> */
#define PTRACE_THROTTLE(var, ...) PTrace::Throttle<__VA_ARGS__> var
#define PTRACE_THROTTLE_STATIC(var, ...) static PTrace::Throttle<__VA_ARGS__> var


__inline const PObject * PTraceObjectInstance() { return NULL; }
__inline static const PObject * PTraceObjectInstance(const void *) { return NULL; }
__inline const char * PTraceModule() { return NULL; }


#if PTRACING==2

/**Propagate PTRACE context identifier in an object from another.
   The context identifier can group objects together with a single
   identifier which can aid in debugging highly threaded systems where
   the logs from various threads become very interleaved.
  */
#define PTRACE_CONTEXT_ID_NEW() SetTraceContextIdentifier(PTrace::GetNextContextIdentifier())
#define PTRACE_CONTEXT_ID_SET(to, from) PObject::SetTraceContextIdentifier(to, from)
#define PTRACE_CONTEXT_ID_FROM(obj) SetTraceContextIdentifier(obj)
#define PTRACE_CONTEXT_ID_TO(obj) GetTraceContextIdentifier(obj)

class PTraceSaveContextIdentifier
{
  private:
    class PThread * m_currentThread;
    unsigned        m_savedContextIdentifier;
  public:
    PTraceSaveContextIdentifier(const PObject & obj);
    PTraceSaveContextIdentifier(const PObject * obj);
    ~PTraceSaveContextIdentifier();
};

#define PTRACE_CONTEXT_ID_PUSH_THREAD(obj) PTraceSaveContextIdentifier ptraceSavedContextIdentifier(obj)

#endif // PTRACING==2


#define P_DECLARE_TRACED_ENUM    P_DECLARE_STREAMABLE_ENUM
#define P_DECLARE_TRACED_ENUM_EX P_DECLARE_STREAMABLE_ENUM_EX

#endif // PTRACING

#ifndef PTRACE_ARGLIST_EXT
#define PTRACE_ARGLIST_EXT(...) ""
#endif

#ifndef PTRACE_ARGLIST
#define PTRACE_ARGLIST ""
#endif

#ifndef PTRACE_INITIALISE
#define PTRACE_INITIALISE(...)
#endif

#ifndef PTRACE_PARAM
#define PTRACE_PARAM(...)
#endif

#ifndef PTRACE_BLOCK
#define PTRACE_BLOCK(n)
#endif

#ifndef PTRACE_LINE
#define PTRACE_LINE()
#endif

#ifndef PTRACE
#define PTRACE(...)
#endif

#ifndef PTRACE_IF
#define PTRACE_IF(...)
#endif

#ifndef PTRACE_BEGIN
#define PTRACE_BEGIN(...)
#endif

#ifndef PTRACE_THROTTLE
#define PTRACE_THROTTLE(...)
#endif

#ifndef PTRACE2
#define PTRACE2(level, obj, arg)
#endif

#ifndef PTRACE_IF2
#define PTRACE_IF2(level, cond, obj, args)
#endif

#ifndef PTRACE_CONTEXT_ID_NEW
#define PTRACE_CONTEXT_ID_NEW()
#endif

#ifndef PTRACE_CONTEXT_ID_SET
#define PTRACE_CONTEXT_ID_SET(to, from)
#endif

#ifndef PTRACE_CONTEXT_ID_FROM
#define PTRACE_CONTEXT_ID_FROM(obj)
#endif

#ifndef PTRACE_CONTEXT_ID_TO
#define PTRACE_CONTEXT_ID_TO(obj)
#endif

#ifndef PTRACE_CONTEXT_ID_PUSH_THREAD
#define PTRACE_CONTEXT_ID_PUSH_THREAD(obj)
#endif

#ifndef P_DECLARE_TRACED_ENUM
#define P_DECLARE_TRACED_ENUM P_DECLARE_ENUM
#endif

#ifndef P_DECLARE_TRACED_ENUM_EX
#define P_DECLARE_TRACED_ENUM_EX P_DECLARE_ENUM_EX
#endif


///////////////////////////////////////////////////////////////////////////////
// Profiling

#if defined( __GNUC__) && !defined(__clang__)
  #define PPROFILE_EXCLUDE(func)  func  __attribute__((no_instrument_function))
#else
  #define PPROFILE_EXCLUDE(func) func
  #ifdef _MSC_VER
    #define __PRETTY_FUNCTION__ __FUNCSIG__
  #else
    #define __PRETTY_FUNCTION__ __FUNCTION__
  #endif
#endif

#if P_PROFILING

class PThread;
class PTimeInterval;

namespace PProfiling
{
  #if defined(__i386__) || defined(__x86_64__)
    #define PProfilingGetCycles(when) { uint32_t l,h; __asm__ __volatile__ ("rdtsc" : "=a"(l), "=d"(h)); when = ((uint64_t)h<<32)|l; }
  #elif defined(_M_IX86) || defined(_M_X64)
    #define PProfilingGetCycles(when) when = __rdtsc()
  #elif defined(_WIN32)
    #define PProfilingGetCycles(when) { LARGE_INTEGER li; QueryPerformanceCounter(&li); when = li.QuadPart; }
  #elif defined(CLOCK_MONOTONIC)
    #define PProfilingGetCycles(when) { timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); when = ts.ts_sec*1000000000ULL+ts.ts_nsec; }
  #else
    #define PProfilingGetCycles(when) { timeval tv; gettimeofday(&tv, NULL); when = tv.tv_sec*1000000ULL+tv.tv_usec; }
  #endif

  PPROFILE_EXCLUDE(void GetFrequency(uint64_t & freq));

  struct Function
  {
    unsigned    m_count;
    uint64_t    m_sum;
    uint64_t    m_minimum;
    uint64_t    m_maximum;

    Function()
      : m_count(0)
      , m_sum(0)
      , m_minimum(std::numeric_limits<uint64_t>::max())
      , m_maximum(0)
    {
    }
  };
  typedef std::map<std::string, Function> FunctionMap;

  struct Thread
  {
    std::string             m_name;
    PThreadIdentifier       m_threadId;
    PUniqueThreadIdentifier m_uniqueId;
    float                   m_realTime;
    float                   m_systemCPU;
    float                   m_userCPU;
    bool                    m_running;
    FunctionMap             m_functions;

    Thread(
      PThreadIdentifier       threadId = PNullThreadIdentifier,
      PUniqueThreadIdentifier uniqueId = 0,
      const char * name = "",
      float realTime = 0,
      float systemCPU = 0,
      float userCPU = 0
    ) : m_name(name)
      , m_threadId(threadId)
      , m_uniqueId(uniqueId)
      , m_realTime(realTime)
      , m_systemCPU(systemCPU)
      , m_userCPU(userCPU)
      , m_running(false)
    {
    }
  };
  typedef std::map<PUniqueThreadIdentifier, Thread> ThreadByID;
  typedef std::multimap<float, Thread, std::greater<double> > ThreadByUsage; // percentage

  struct Analysis
  {
    uint64_t      m_durationCycles;
    uint64_t      m_frequency;
    unsigned      m_functionCount;
    ThreadByID    m_threadByID;
    ThreadByUsage m_threadByUsage;

    Analysis()
      : m_durationCycles(0)
      , m_frequency(0)
      , m_functionCount(0)
    {
    }

    float CyclesToSeconds(uint64_t cycles) const;
    void ToText(ostream & strm) const;
    void ToHTML(ostream & strm) const;
  };

  void Analyse(Analysis & analysis);
  void Analyse(ostream & strm, bool html);

  PPROFILE_EXCLUDE(
    void Enable(bool enab)
  );
  PPROFILE_EXCLUDE(
    bool IsEnabled()
  );

  PPROFILE_EXCLUDE(
    void Reset()
  );
  PPROFILE_EXCLUDE(
    void Dump(ostream & strm)
  );

  PPROFILE_EXCLUDE(
    void OnThreadEnded(const PThread & thread, const PTimeInterval & realTime, const PTimeInterval & systemCPU, const PTimeInterval & userCPU)
  );

  PPROFILE_EXCLUDE(
    void PreSystem()
  );

  PPROFILE_EXCLUDE(
    void PostSystem()
  );

  class Block
  {
    public:
      PPROFILE_EXCLUDE(
        Block(
          const char * name,
          const char * file,
          unsigned line
        )
      );
      PPROFILE_EXCLUDE(
        ~Block()
      );

    protected:
      const char * m_name;
  };

  #define PPROFILE_BLOCK(name) ::PProfiling::Block p_profile_block_instance(name, __FILE__, __LINE__)
  #define PPROFILE_FUNCTION() PPROFILE_BLOCK(__PRETTY_FUNCTION__)

  #define PPROFILE_PRE_SYSTEM()  ::PProfiling::PreSystem()
  #define PPROFILE_POST_SYSTEM() ::PProfiling::PostSystem()
  #define PPROFILE_SYSTEM(...)  PPROFILE_PRE_SYSTEM(); __VA_ARGS__; PPROFILE_POST_SYSTEM()
};
#else
  #define PPROFILE_BLOCK(...)
  #define PPROFILE_FUNCTION()
  #define PPROFILE_PRE_SYSTEM()
  #define PPROFILE_POST_SYSTEM()
  #define PPROFILE_SYSTEM(...) __VA_ARGS__
#endif


///////////////////////////////////////////////////////////////////////////////
// Memory management

#if PMEMORY_CHECK || (defined(_MSC_VER) && defined(_DEBUG) && !defined(_WIN32_WCE)) 

#define PMEMORY_HEAP 1

/** Memory heap checking class.
This class implements the memory heap checking and validation functions. It
maintains lists of allocated block so that memory leaks can be detected. It
also initialises memory on allocation and deallocation to help catch errors
involving the use of dangling pointers.
*/
class PMemoryHeap {
  protected:
    /// Initialise the memory checking subsystem.
    PMemoryHeap();

  public:
    // Clear up the memory checking subsystem, dumping memory leaks.
    ~PMemoryHeap();

    /** Allocate a memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the <code>allocFillChar</code> member variable
       to help detect uninitialised structures.
       @return pointer to newly allocated memory block.
     */
    static void * Allocate(
      size_t nSize,           ///< Number of bytes to allocate.
      const char * file,      ///< Source file name for allocating function.
      int line,               ///< Source file line for allocating function.
      const char * className  ///< Class name for allocating function.
    );

    /** Allocate a memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the <code>allocFillChar</code> member variable
       to help detect uninitialised structures.
       @return pointer to newly allocated memory block.
     */
    static void * Allocate(
      size_t count,       ///< Number of items to allocate.
      size_t iSize,       ///< Size in bytes of each item.
      const char * file,  ///< Source file name for allocating function.
      int line            ///< Source file line for allocating function.
    );

    /** Change the size of an allocated memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the <code>allocFillChar</code> member variable
       to help detect uninitialised structures.
      @return pointer to reallocated memory block. Note this may
      {\em not} be the same as the pointer passed into the function.
     */
    static void * Reallocate(
      void * ptr,         ///< Pointer to memory block to reallocate.
      size_t nSize,       ///< New number of bytes to allocate.
      const char * file,  ///< Source file name for allocating function.
      int line            ///< Source file line for allocating function.
    );

    /** Free a memory block.
      The memory is deallocated, a warning is displayed if it was never
      allocated. The block of memory is filled with the value in the
      <code>freeFillChar</code> member variable.
     */
    static void Deallocate(
      void * ptr,             ///< Pointer to memory block to deallocate.
      const char * className  ///< Class name for deallocating function.
    );

    /** Validation result.
     */
    enum Validation {
      Ok,         // All good.
      Bad,        // Bad pointer, not allocated by us, double deletion or just random address.
      Trashed,    // Heap is trashed, linked list failed.
      Corrupt,    // Corrupted guard bytes or object.
      Inactive    // Heap checking disabled or after final destruction.
    };
    /** Validate the memory pointer.
        The <code>ptr</code> parameter is validated as a currently allocated heap
        variable.
        @return Ok for pointer is in heap, Bad for pointer is not in the heap
        or Trashed if the pointer is in the heap but has overwritten the guard
        bytes before or after the actual data part of the memory block.
     */
    static Validation Validate(
      const void * ptr,       ///< Pointer to memory block to check
      const char * className, ///< Class name it should be.
      ostream * error         ///< Stream to receive error message (may be NULL)
    );

    /** Validate all objects in memory.
       This effectively calls Validate() on every object in the heap.
        @return true if every object in heap is Ok.
     */
    static PBoolean ValidateHeap(
      ostream * error = NULL  ///< Stream to output, use default if NULL
    );

    /** Ignore/Monitor allocations.
       Set internal flag so that allocations are not included in the memory
       leak check on program termination.
       Returns the previous state.
     */
    static PBoolean SetIgnoreAllocations(
      PBoolean ignore  ///< New flag for allocation ignoring.
    );

    /** Get memory check system statistics.
        Dump statistics output to the default stream.
     */
    static void DumpStatistics();
    /** Get memory check system statistics.
        Dump statistics output to the specified stream.
     */
    static void DumpStatistics(ostream & strm /** Stream to output to */);

#if PMEMORY_CHECK
    struct State {
      DWORD allocationNumber;
    };
#else
	typedef _CrtMemState State;
#endif

    /* Get memory state.
      This returns a state that may be used to determine where to start dumping
      objects from.
     */
    static void GetState(
      State & state  ///< Memory state
    );

    /** Dump allocated objects.
       Dump ojects allocated and not deallocated since the specified object
       number. This would be a value returned by the <code>GetAllocationRequest()</code>
       function.

       Output is to the default stream.
     */
    static void DumpObjectsSince(
      const State & when    ///< Memory state to begin dump from.
    );

    /** Dump allocated objects.
       Dump ojects allocated and not deallocated since the specified object
       number. This would be a value returned by the <code>GetAllocationRequest()</code>
       function.
     */
    static void DumpObjectsSince(
      const State & when,   ///< Memory state to begin dump from.
      ostream & strm        ///< Stream to output dump
    );

    /** Set break point allocation number.
      Set the allocation request number to cause an assert. This allows a
      developer to cause a halt in a debugger on a certain allocation allowing
      them to determine memory leaks allocation point.
     */
    static void SetAllocationBreakpoint(
      DWORD point   ///< Allocation number to stop at.
    );

#if PMEMORY_CHECK

  protected:
    void * InternalAllocate(
      size_t nSize,           // Number of bytes to allocate.
      const char * file,      // Source file name for allocating function.
      int line,               // Source file line for allocating function.
      const char * className  // Class name for allocating function.
    );
    Validation InternalValidate(
      const void * ptr,       // Pointer to memory block to check
      const char * className, // Class name it should be.
      ostream * error         // Stream to receive error message (may be NULL)
    );
    void InternalDumpStatistics(ostream & strm);
    void InternalDumpObjectsSince(DWORD objectNumber, ostream & strm);

    class Wrapper {
      public:
        Wrapper();
        ~Wrapper();
        PMemoryHeap * operator->() const { return instance; }
      private:
        PMemoryHeap * instance;
    };
    friend class Wrapper;

    enum Flags {
      NoLeakPrint = 1
    };

#pragma pack(1)
    struct Header {
      enum {
        // Assure that the Header struct is aligned to 8 byte boundary
        NumGuardBytes = 16 - (sizeof(Header *) +
                              sizeof(Header *) +
                              sizeof(const char *) +
                              sizeof(const char *) +
                              sizeof(size_t) +
                              sizeof(DWORD) +
                              sizeof(WORD) +
                              sizeof(BYTE) +
                              sizeof(PThreadIdentifier)
                              )%8
      };

      Header     * prev;
      Header     * next;
      const char * className;
      const char * fileName;
      size_t       size;
      DWORD        request;
      WORD         line;
      BYTE         flags;
      PThreadIdentifier threadId;
      char         guard[NumGuardBytes];

      static char GuardBytes[NumGuardBytes];
    };
#pragma pack()

    enum {
      e_Disabled,
      e_Destroyed,
      e_Active
    } m_state;

    Header * listHead;
    Header * listTail;

    static DWORD allocationBreakpoint;
    DWORD allocationRequest;
    DWORD firstRealObject;
    BYTE  flags;

    char  allocFillChar;
    char  freeFillChar;

    DWORD currentMemoryUsage;
    DWORD peakMemoryUsage;
    DWORD currentObjects;
    DWORD peakObjects;
    DWORD totalObjects;

    ostream * leakDumpStream;

#if defined(_WIN32)
    CRITICAL_SECTION mutex;
#elif defined(P_PTHREADS)
    pthread_mutex_t mutex;
#elif defined(P_VXWORKS)
    void * mutex;
#endif

#else

    static void CreateInstance();
#define P_CLIENT_BLOCK (_CLIENT_BLOCK|(0x61<<16)) // This identifies a PObject derived class
    _CrtMemState initialState;

#endif // PMEMORY_CHECK
};


/** Allocate memory for the run time library.
This version of free is used for data that is not to be allocated using the
memory check system, ie will be free'ed inside the C run time library.
*/
inline void * runtime_malloc(size_t bytes /** Size of block to allocate */ ) { return malloc(bytes); }

/** Free memory allocated by run time library.
This version of free is used for data that is not allocated using the
memory check system, ie was malloc'ed inside the C run time library.
*/
inline void runtime_free(void * ptr /** Memory block to free */ ) { free(ptr); }


/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define malloc(s) PMemoryHeap::Allocate(s, __FILE__, __LINE__, NULL)

/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define calloc(n,s) PMemoryHeap::Allocate(n, s, __FILE__, __LINE__)

/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define realloc(p,s) PMemoryHeap::Reallocate(p, s, __FILE__, __LINE__)


/** Override of system call for memory check system.
This macro is used to deallocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define free(p) PMemoryHeap::Deallocate(p, NULL)


/** Override of system call for memory check system.
This macro is used to deallocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define cfree(p) PMemoryHeap::Deallocate(p, NULL)


/** Macro for overriding system default <code>new</code> operator.
This macro is used to allocate memory via the memory check system selected
with the PMEMORY_CHECK compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.

This macro could be used instead of the system <code>new</code> operator. Or you can place
the line
<pre><code>
  #define new PNEW
</code></pre>
at the begining of the source file, after all declarations that use the
<code>#PCLASSINFO</code> macro.
*/
#define PNEW  new (__FILE__, __LINE__)

#if !defined(_MSC_VER) || _MSC_VER<1200
#define PSPECIAL_DELETE_FUNCTION
#else
#define PSPECIAL_DELETE_FUNCTION \
    void operator delete(void * ptr, const char *, int) \
      { PMemoryHeap::Deallocate(ptr, Class()); } \
    void operator delete[](void * ptr, const char *, int) \
      { PMemoryHeap::Deallocate(ptr, Class()); }
#endif

#define PNEW_AND_DELETE_FUNCTIONS(align) \
    void * operator new(size_t nSize, const char * file, int line) \
      { return PMemoryHeap::Allocate(nSize, file, line, Class()); } \
    void * operator new(size_t nSize) \
      { return PMemoryHeap::Allocate(nSize, NULL, 0, Class()); } \
    void operator delete(void * ptr) \
      { PMemoryHeap::Deallocate(ptr, Class()); } \
    void * operator new(size_t, void * placement) \
      { return placement; } \
    void operator delete(void *, void *) \
      { } \
    void * operator new[](size_t nSize, const char * file, int line) \
      { return PMemoryHeap::Allocate(nSize, file, line, Class()); } \
    void * operator new[](size_t nSize) \
      { return PMemoryHeap::Allocate(nSize, NULL, 0, Class()); } \
    void operator delete[](void * ptr) \
      { PMemoryHeap::Deallocate(ptr, Class()); } \
    PSPECIAL_DELETE_FUNCTION


inline void * operator new(size_t nSize, const char * file, int line)
  { return PMemoryHeap::Allocate(nSize, file, line, NULL); }

inline void * operator new[](size_t nSize, const char * file, int line)
  { return PMemoryHeap::Allocate(nSize, file, line, NULL); }

#ifndef __GNUC__
void * operator new(size_t nSize);
void * operator new[](size_t nSize);

void operator delete(void * ptr);
void operator delete[](void * ptr);

#if defined(_MSC_VER) && _MSC_VER>=1200
inline void operator delete(void * ptr, const char *, int)
  { PMemoryHeap::Deallocate(ptr, NULL); }

inline void operator delete[](void * ptr, const char *, int)
  { PMemoryHeap::Deallocate(ptr, NULL); }
#endif
#endif


class PMemoryHeapIgnoreAllocationsForScope {
public:
  PMemoryHeapIgnoreAllocationsForScope() : previousIgnoreAllocations(PMemoryHeap::SetIgnoreAllocations(true)) { }
  ~PMemoryHeapIgnoreAllocationsForScope() { PMemoryHeap::SetIgnoreAllocations(previousIgnoreAllocations); }
private:
  PBoolean previousIgnoreAllocations;
};

#define PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE PMemoryHeapIgnoreAllocationsForScope instance_PMemoryHeapIgnoreAllocationsForScope

class PMemoryAllocationBreakpoint {
public:
  PMemoryAllocationBreakpoint(DWORD point)
  {
    PMemoryHeap::SetAllocationBreakpoint(point);
  }
};

#define PMEMORY_ALLOCATION_BREAKPOINT(point) PMemoryAllocationBreakpoint PMemoryAllocationBreakpointInstance(point)


#else // PMEMORY_CHECK || (defined(_MSC_VER) && defined(_DEBUG))

#define PMEMORY_HEAP 0

#define PNEW new

#if _MSC_VER < 1800
  #define PNEW_AND_DELETE_FUNCTIONS(align)
#else
  #define PNEW_AND_DELETE_FUNCTIONS_ALIGNED(align) \
      void * operator new(size_t nSize) \
        { return _aligned_malloc(nSize, align); } \
      void operator delete(void * ptr) \
        { _aligned_free(ptr); } \
      void * operator new(size_t, void * placement) \
        { return placement; } \
      void operator delete(void *, void *) \
        { } \
      void * operator new[](size_t nSize) \
        { return _aligned_malloc(nSize, align); } \
      void operator delete[](void * ptr) \
        { _aligned_free(ptr); }

  #define PNEW_AND_DELETE_FUNCTIONS64 PNEW_AND_DELETE_FUNCTIONS_ALIGNED(64)
  #define PNEW_AND_DELETE_FUNCTIONS32 PNEW_AND_DELETE_FUNCTIONS_ALIGNED(32)
  #define PNEW_AND_DELETE_FUNCTIONS16 PNEW_AND_DELETE_FUNCTIONS_ALIGNED(16)
  #define PNEW_AND_DELETE_FUNCTIONS8  PNEW_AND_DELETE_FUNCTIONS_ALIGNED(8)
  #define PNEW_AND_DELETE_FUNCTIONS4  PNEW_AND_DELETE_FUNCTIONS_ALIGNED(4)
  #define PNEW_AND_DELETE_FUNCTIONS2  PNEW_AND_DELETE_FUNCTIONS_ALIGNED(2)
  #define PNEW_AND_DELETE_FUNCTIONS0
  #define PNEW_AND_DELETE_FUNCTIONS(align) PNEW_AND_DELETE_FUNCTIONS##align
#endif

#define runtime_malloc(s) malloc(s)
#define runtime_free(p) free(p)

#define PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE
#define PMEMORY_ALLOCATION_BREAKPOINT(point)

#endif // PMEMORY_CHECK || (defined(_MSC_VER) && defined(_DEBUG))


void PThreadYield();

///////////////////////////////////////////////////////////////////////////////

template<class Type> Type * PSingletonCreatorDefault()
{
  return new Type();
}

/** Template class for a simple singleton object.
     Usage is typically like:
<pre><code>
        typedef PSingleton<MyClass> MySingleton;
        MySingleton()->DoSomething();
</code></pre>
     Default is not thread safe, however the following is:
<pre><code>
        typedef PSingleton<MyClass, atomic<uint32_t> > MySafeSingleton;
        MySafeSingleton()->DoSomething();
</code></pre>
     If the singleton class requires parameters on construction, then a creator
     function may be included in the template parameters:
 <pre><code>
        MyClass * CreateMyClass() { return new MyClass("fred"); }
        typedef PSingleton<MyClass, atomic<uint32_t>, CreateMyClass> MySafeSingleton;
        MySafeSingleton()->DoSomething();
</code></pre>
*/
template <class Type, typename GuardType = unsigned, Type * (*Creator)() = PSingletonCreatorDefault<Type> >
class PSingleton
{
  protected:
    Type * m_instance;
  public:
    PSingleton()
    {
      static auto_ptr<Type> s_pointer;
      static GuardType s_guard(0);
      if (s_guard++ != 0) {
        s_guard = 1;
        while ((m_instance = s_pointer.get()) == NULL)
          PThreadYield();
      }
      else {
#if PMEMORY_HEAP
        // Do this to make sure debugging is initialised as early as possible
        PMemoryHeap::Validate(NULL, NULL, NULL);
#endif
        s_pointer.reset(Creator());
        m_instance = s_pointer.get();
      }
    }

    Type * operator->() const { return  m_instance; }
    Type & operator* () const { return *m_instance; }
};


///////////////////////////////////////////////////////////////////////////////
// Memory pool allocators

#if P_GNU_ALLOCATOR

  #include <ext/mt_allocator.h>

  /* Need this tempalte class specialisation of standard class to do the
     de-allocation of the pool memory. As per:
     http://gcc.gnu.org/viewcvs/trunk/libstdc++-v3/testsuite/ext/mt_allocator/deallocate_local-6.cc?view=markup
   */
  template <bool _Thread>
  struct PMemoryPool : public __gnu_cxx::__pool<_Thread>
    {
    PMemoryPool()
      : __gnu_cxx::__pool<_Thread>()
    {
    }

    PMemoryPool(const __gnu_cxx::__pool_base::_Tune& t) 
      : __gnu_cxx::__pool<_Thread>(t)
    {
    }
  };

  #if P_GNU_ALLOCATOR==1
    /*Do this template class specialisation so each type has it's own separate
      memory block for the pool. */
    template <class Type>
    struct PCommonPool : public __gnu_cxx::__common_pool_policy<PMemoryPool, true>
    {
    };

    template <class Type>
    struct PFixedPoolAllocator : public PSingleton<__gnu_cxx::__mt_alloc<Type, PCommonPool<Type> > >
    {
    };
  #else
    #include <ext/bitmap_allocator.h>
    template <class Type>
    struct PFixedPoolAllocator : public PSingleton<__gnu_cxx::bitmap_allocator<Type> >
    {
    };
  #endif

  #define PDECLARE_POOL_ALLOCATOR(cls) \
    void * cls::operator new(size_t)                           { return PFixedPoolAllocator<cls>()->allocate(1);               } \
    void * cls::operator new(size_t, const char *, int)        { return PFixedPoolAllocator<cls>()->allocate(1);               } \
    void   cls::operator delete(void * ptr)                    {        PFixedPoolAllocator<cls>()->deallocate((cls *)ptr, 1); } \
    void   cls::operator delete(void * ptr, const char *, int) {        PFixedPoolAllocator<cls>()->deallocate((cls *)ptr, 1); }

#else

  #define PDECLARE_POOL_ALLOCATOR(cls) \
    virtual ~cls() { } \
    __inline static const char * Class() { return typeid(cls).name(); } \
    PNEW_AND_DELETE_FUNCTIONS(0)

  #define PDEFINE_POOL_ALLOCATOR(cls)

#endif


#define PCLASSINFO_ALIGNED(cls, par, align) \
  public: \
    typedef cls P_thisClass; \
    __inline static const char * Class() { return typeid(cls).name(); } \
    __inline bool IsClass(const char * name) const { return strcmp(name, Class()) == 0; } \
    virtual PObject::Comparison CompareObjectMemoryDirect(const PObject & obj) const \
      { return PObject::InternalCompareObjectMemoryDirect(this, dynamic_cast<const cls *>(&obj), sizeof(cls)); } \
    PNEW_AND_DELETE_FUNCTIONS(align)


/** Declare all the standard PTLib class information.
This macro is used to provide the basic run-time typing capability needed
by the library. All descendent classes from the <code>PObject</code> class require
these functions for correct operation. Either use this macro or the
<code>#PDECLARE_CLASS</code> macro.

The use of the <code>#PDECLARE_CLASS</code> macro is no longer recommended for reasons
of compatibility with documentation systems.
*/

#define PCLASSINFO(cls, par) PCLASSINFO_ALIGNED(cls, par, 0)

/// Declare all the standard PTLib class information, plus Clone().
#define PCLASSINFO_WITH_CLONE(cls, par) \
    PCLASSINFO(cls, par) \
    virtual PObject * Clone() const { return new cls(*this); }


#define PIsDescendant(ptr, cls)    (dynamic_cast<const cls *>(ptr) != NULL) 
#define PRemoveConst(cls, ptr)  (const_cast<cls*>(ptr))

#if P_USE_ASSERTS
template<class BaseClass> inline BaseClass * PAssertCast(BaseClass * obj, const char * file, int line) 
  { if (obj == NULL) PAssertFunc(file, line, obj->Class(), PInvalidCast); return obj; }
#define PDownCast(cls, ptr) PAssertCast<cls>(dynamic_cast<cls*>(ptr),__FILE__,__LINE__)
#else
#define PDownCast(cls, ptr) (dynamic_cast<cls*>(ptr))
#endif


/** Declare a class with PWLib class information.
This macro is used to declare a new class with a single public ancestor. It
starts the class declaration and then uses the <code>#PCLASSINFO</code> macro to
get all the run-time type functions.

The use of this macro is no longer recommended for reasons of compatibility
with documentation systems.
*/
#define PDECLARE_CLASS(cls, par) class cls : public par { PCLASSINFO(cls, par)
#ifdef DOC_PLUS_PLUS
} Match previous opening brace in doc++
#endif

///////////////////////////////////////////////////////////////////////////////
// The root of all evil ... umm classes

/** Ultimate parent class for all objects in the class library.
This provides functionality provided to all classes, eg run-time types,
default comparison operations, simple stream I/O and serialisation support.
*/
class PObject {
  protected:
    unsigned m_traceContextIdentifier;
#if PTRACING==2
  public:
    /**Get PTRACE context identifier
      */
    __inline unsigned GetTraceContextIdentifier() const { return m_traceContextIdentifier; }
    __inline void SetTraceContextIdentifier(unsigned id) { m_traceContextIdentifier = id; }
    __inline void GetTraceContextIdentifier(PObject & obj) const { obj.m_traceContextIdentifier = m_traceContextIdentifier; }
    __inline void GetTraceContextIdentifier(PObject * obj) const { if (obj != NULL) obj->m_traceContextIdentifier = m_traceContextIdentifier; }
    __inline void SetTraceContextIdentifier(const PObject & obj) { m_traceContextIdentifier = obj.m_traceContextIdentifier; }
    __inline void SetTraceContextIdentifier(const PObject * obj) { if (obj != NULL) m_traceContextIdentifier = obj->m_traceContextIdentifier; }
    __inline static void SetTraceContextIdentifier(PObject & to, const PObject & from) { to.SetTraceContextIdentifier(from); }
    __inline static void SetTraceContextIdentifier(PObject & to, const PObject * from) { to.SetTraceContextIdentifier(from); }
    __inline static void SetTraceContextIdentifier(PObject * to, const PObject & from) { from.GetTraceContextIdentifier(to); }
    __inline static void SetTraceContextIdentifier(PObject * to, const PObject * from) { if (to != NULL) to->SetTraceContextIdentifier(from); }
#endif // PTRACING==2

  protected:
    /** Constructor for PObject, made protected so cannot ever create one on
       its own.
     */
    PObject()
      : m_traceContextIdentifier(0)
    { }

  public:
    /* Destructor required to get the "virtual". A PObject really has nothing
       to destroy.
     */
    virtual ~PObject() { }

    // Backward compatibility, use RTTI from now on!
    __inline static const char * Class() { return typeid(PObject).name(); }
    __inline const char * GetClass() const { return typeid(const_cast<PObject &>(*this)).name(); }
    __inline bool IsClass(const char * name) const { return strcmp(name, Class()) == 0; }

    __inline const PObject * PTraceObjectInstance() const { return this; }
    __inline static const PObject * PTraceObjectInstance(const char *) { return NULL; }
    __inline static const PObject * PTraceObjectInstance(const PObject * obj) { return obj; }

  /**@name Comparison functions */
  //@{
    /** Result of the comparison operation performed by the <code>Compare()</code>
       function.
      */
    enum Comparison {
      LessThan = -1,
      EqualTo = 0,
      GreaterThan = 1
    };

    /// Compare two types, returning Comparison type.
    template<typename T> static Comparison Compare2(T v1, T v2)
    {
      if (v1 < v2)
        return LessThan;
      if (v1 > v2)
        return GreaterThan;
      return EqualTo;
    }

    /** Compare the two objects and return their relative rank. This function is
       usually overridden by descendent classes to yield the ranking according
       to the semantics of the object.
       
       The default function is to use the <code>CompareObjectMemoryDirect()</code>
       function to do a byte wise memory comparison of the two objects.

       @return
       <code>LessThan</code>, <code>EqualTo</code> or <code>GreaterThan</code>
       according to the relative rank of the objects.
     */
    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;
    
    /** Determine the byte wise comparison of two objects. This is the default
       comparison operation for objects that do not explicitly override the
       <code>Compare()</code> function.
    
       The <code>#PCLASSINFO</code> macro declares an override of this function for
       the particular class. The user need not implement it.

       @return
       <code>LessThan</code>, <code>EqualTo</code> or <code>GreaterThan</code>
       according to the result <code>memcpy()</code> function.
     */
    virtual Comparison CompareObjectMemoryDirect(
      const PObject & obj   // Object to compare against.
    ) const;

    /// Internal function caled from CompareObjectMemoryDirect()
    static Comparison InternalCompareObjectMemoryDirect(
      const PObject * obj1,
      const PObject * obj2,
      PINDEX size
    );

    /** Compare the two objects.
    
       @return
       true if objects are equal.
     */
    bool operator==(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == EqualTo; }

    /** Compare the two objects.
    
       @return
       true if objects are not equal.
     */
    bool operator!=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != EqualTo; }

    /** Compare the two objects.
    
       @return
       true if objects are less than.
     */
    bool operator<(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == LessThan; }

    /** Compare the two objects.
    
       @return
       true if objects are greater than.
     */
    bool operator>(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == GreaterThan; }

    /** Compare the two objects.
    
       @return
       true if objects are less than or equal.
     */
    bool operator<=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != GreaterThan; }

    /** Compare the two objects.
    
       @return
       true if objects are greater than or equal.
     */
    bool operator>=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != LessThan; }
  //@}

  /**@name I/O functions */
  //@{
    /** Output the contents of the object to the stream. The exact output is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard <code>#operator<<</code> function.

       The default behaviour is to print the class name.
     */
    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;

    /** Input the contents of the object from the stream. The exact input is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard <code>#operator>></code> function.

       The default behaviour is to do nothing.
     */
    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );


    /** Global function for using the standard << operator on objects descended
       from PObject. This simply calls the objects <code>PrintOn()</code> function.
       
       @return the \p strm parameter.
     */
    inline friend ostream & operator<<(
      ostream &strm,       ///< Stream to print the object into.
      const PObject & obj  ///< Object to print to the stream.
    ) { obj.PrintOn(strm); return strm; }

    /** Global function for using the standard >> operator on objects descended
       from PObject. This simply calls the objects <code>ReadFrom()</code> function.

       @return the \p strm parameter.
     */
    inline friend istream & operator>>(
      istream &strm,   ///< Stream to read the objects contents from.
      PObject & obj    ///< Object to read inormation into.
    ) { obj.ReadFrom(strm); return strm; }


  /**@name Miscellaneous functions */
  //@{
    /** Create a copy of the class on the heap. The exact semantics of the
       descendent class determine what is required to make a duplicate of the
       instance. Not all classes can even \b do a clone operation.
       
       The main user of the clone function is the <code>PDictionary</code> class as
       it requires copies of the dictionary keys.

       The default behaviour is for this function to assert.

       @return
       pointer to new copy of the class instance.
     */
    virtual PObject * Clone() const;

    /** As for Clone() but converts to specified type.
      */
    template <class CLS>
    CLS * CloneAs() const
    {
      PObject * clone = Clone();
      CLS * obj = dynamic_cast<CLS *>(clone);
      if (obj != NULL)
        return obj;
      delete clone;
      return NULL;
    }

    /** This function yields a hash value required by the <code>PDictionary</code>
       class. A descendent class that is required to be the key of a dictionary
       should override this function. The precise values returned is dependent
       on the semantics of the class. For example, the <code>PString</code> class
       overrides it to provide a hash function for distinguishing text strings.

       The default behaviour is to return the value zero.

       @return
       hash function value for class instance.
     */
    virtual PINDEX HashFunction() const;
  //@}
};

///////////////////////////////////////////////////////////////////////////////
// Platform independent types

// All these classes encapsulate primitive types such that they may be
// transfered in a platform independent manner. In particular it is used to
// do byte swapping for little endien and big endien processor architectures
// as well as accommodating structure packing rules for memory structures.

#define PANSI_CHAR 1
#define PLITTLE_ENDIAN 2
#define PBIG_ENDIAN 3


template <typename type>
struct PIntSameOrder {
  __inline PIntSameOrder()                            : data(0)              { }
  __inline PIntSameOrder(type value)                  : data(value)          { }
  __inline PIntSameOrder(const PIntSameOrder & value) : data(value.data)     { }
  __inline PIntSameOrder & operator=(type value)                             { data = value; return *this; }
  __inline PIntSameOrder & operator=(const PIntSameOrder & value)            { data = value.data; return *this; }
  __inline operator type() const                                             { return data; }
  __inline friend ostream & operator<<(ostream & s, const PIntSameOrder & v) { return s << v.data; }
  __inline friend istream & operator>>(istream & s, PIntSameOrder & v)       { return s >> v.data; }

  private:
    type data;
};


template <typename type>
struct PIntReversedOrder {
  __inline PIntReversedOrder()                                : data(0)              { }
  __inline PIntReversedOrder(type value)                                             { ReverseBytes(value, data); }
  __inline PIntReversedOrder(const PIntReversedOrder & value) : data(value.data)     { }
  __inline PIntReversedOrder & operator=(type value)                                 { ReverseBytes(value, data); return *this; }
  __inline PIntReversedOrder & operator=(const PIntReversedOrder & value)            { data = value.data; return *this; }
  __inline operator type() const                                                     { type value; ReverseBytes(data, value); return value; }
  __inline friend ostream & operator<<(ostream & s, const PIntReversedOrder & value) { return s << (type)value; }
  __inline friend istream & operator>>(istream & s, PIntReversedOrder & v)           { type val; s >> val; v = val; return s; }

  private:
    type data;

  static __inline void ReverseBytes(const type & src, type & dst)
  {
    size_t s = sizeof(type)-1;
    for (size_t d = 0; d < sizeof(type); ++d,--s)
      ((BYTE *)&dst)[d] = ((const BYTE *)&src)[s];
  }
};

#ifndef PCHAR8
#define PCHAR8 PANSI_CHAR
#endif

#if PCHAR8==PANSI_CHAR
typedef PIntSameOrder<char> PChar8;
#endif

typedef PIntSameOrder<char> PInt8;

typedef PIntSameOrder<unsigned char> PUInt8;

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PInt16> PInt16l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PInt16> PInt16l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PInt16> PInt16b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PInt16> PInt16b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<WORD> PUInt16l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<WORD> PUInt16l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<WORD> PUInt16b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<WORD> PUInt16b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PInt32> PInt32l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PInt32> PInt32l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PInt32> PInt32b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PInt32> PInt32b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<DWORD> PUInt32l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<DWORD> PUInt32l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<DWORD> PUInt32b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<DWORD> PUInt32b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PInt64> PInt64l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PInt64> PInt64l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PInt64> PInt64b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PInt64> PInt64b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PUInt64> PUInt64l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PUInt64> PUInt64l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PUInt64> PUInt64b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PUInt64> PUInt64b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<float> PFloat32l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<float> PFloat32l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<float> PFloat32b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<float> PFloat32b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<double> PFloat64l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<double> PFloat64l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<double> PFloat64b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<double> PFloat64b;
#endif

#ifndef NO_LONG_DOUBLE // stupid OSX compiler
#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<long double> PFloat80l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<long double> PFloat80l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<long double> PFloat80b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<long double> PFloat80b;
#endif
#endif

typedef intptr_t P_INT_PTR;

#if defined(_MSC_VER)
#define P_ALIGN_FIELD(fieldType,fieldName,alignment) fieldType __declspec(align(alignment)) fieldName
#elif defined(__GNUC__)
#define P_ALIGN_FIELD(fieldType,fieldName,alignment) fieldType fieldName __attribute__ ((aligned(alignment)))
#endif

typedef intptr_t P_INT_PTR;

///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

/*$MACRO PARRAYSIZE(array)
   This macro is used to calculate the number of array elements in a static
   array.
 */
#define PARRAYSIZE(array) ((PINDEX)(sizeof(array)/sizeof(array[0])))

/*$MACRO PMIN(v1, v2)
   This macro is used to calculate the minimum of two values.
   Maps to std::min and is for backward compatibility only.
 */
#define PMIN(v1, v2) std::min(v1, v2)

/*$MACRO PMAX(v1, v2)
   This macro is used to calculate the maximum of two values.
   Maps to std::max and is for backward compatibility only.
 */
#define PMAX(v1, v2) std::max(v1, v2)

/*$MACRO PABS(val)
   This macro is used to calculate an absolute value.
   Maps to std::abs and is for backward compatibility only.
 */
#define PABS(v) std::abs(v)


#endif // PTLIB_OBJECT_H


// End Of File ///////////////////////////////////////////////////////////////
