/*
 * factory.h
 *
 * Abstract Factory Classes
 *
 * Portable Windows Library
 *
 * Copyright (C) 2004 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pfactory.h,v $
 * Revision 1.6  2004/05/19 06:48:39  csoutheren
 * Added new functions to allow handling of singletons without concrete classes
 *
 * Revision 1.5  2004/05/18 06:01:06  csoutheren
 * Deferred plugin loading until after main has executed by using abstract factory classes
 *
 * Revision 1.4  2004/05/18 02:32:08  csoutheren
 * Fixed linking problems with PGenericFactory classes
 *
 * Revision 1.3  2004/05/13 15:10:51  csoutheren
 * Removed warnings under Windows
 *
 * Revision 1.2  2004/05/13 14:59:00  csoutheren
 * Removed warning under gcc
 *
 * Revision 1.1  2004/05/13 14:53:35  csoutheren
 * Add "abstract factory" template classes
 *
 */

#ifndef _PFACTORY_H
#define _PFACTORY_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>
#include <string>
#include <map>
#include <vector>

#ifdef _WIN32
#pragma warning(disable:4786)
#endif

/**
 *
 * These templates implement an Abstract Factory that allows
 * creation of a class "factory" that can be used to create
 * "concrete" instance that are descended from a abstract case class
 *
 * Given an abstract class A with a descendant concrete class B, the 
 * concrete class is registered by instantiating the PAbstractFactory
 * class as follows:
 *
 *       PAbstractFactory<A, B> aFactory("B");
 *
 * To instantiate the an object of type B, use the following
 *
 *       A * b = PGenericFactory<A>::CreateInstance("B");
 *
 * A vector containing the names of all of the concrete classes for an
 * abstract type can be obtained as follows:
 *
 *       std::vector<PString> list = PGenericFactory<A>::GetKeyList()
 *
 * Note that these example assumes that the "key" type for the factory
 * registration is of the default type PString. If a different key type
 * is needed, then it is necessary to specify the key type:
 *
 *       PAbstractFactory<C, D, unsigned> aFactory(42);
 *       C * d = PGenericFactory<C, unsigned>::CreateInstance(42);
 *       std::vector<unsigned> list = PGenericFactory<C, unsigned>::GetKeyList()
 *
 * Finally, note that the factory lists are all thread safe for addition,
 * creation, and obtaining the key lists
 *  
 */

template <class AbstractType, typename TypeKey = PString>
class PGenericFactory
{
  public:
    typedef AbstractType * (*CreatorFunctionType)();
    class AbstractInfo {
      public:
        AbstractInfo()
          : creator(NULL), isSingleton(FALSE), instance(NULL)
        { }
        CreatorFunctionType creator;
        BOOL isSingleton;
        AbstractType * instance;
    };

    typedef std::map<TypeKey, AbstractInfo> KeyMap;
    typedef std::vector<TypeKey> KeyList;

    static PGenericFactory & GetFactory()
    { static PGenericFactory factory; return factory; }

    static AbstractType * CreateInstance(const TypeKey & key)
    {
      PWaitAndSignal m(GetMutex());
      KeyMap & keyMap = GetKeyMap();
      AbstractType * instance = NULL;
      typename KeyMap::const_iterator entry = keyMap.find(key);
      if (entry != keyMap.end()) {
        if (entry->second.isSingleton && entry->second.instance != NULL)
          instance = entry->second.instance;
        else if (entry->second.creator != NULL)
          instance = (*(entry->second.creator))();
      }
      return instance;
    }

    static void Register(const TypeKey & key, CreatorFunctionType func, BOOL isSingleton)
    {
      AbstractInfo info;
      info.creator     = func;
      info.isSingleton = isSingleton;
      Register(key, info);
    }

    static void Register(const TypeKey & key, AbstractInfo info)
    {
      PWaitAndSignal m(GetMutex());
      KeyMap & keyMap = GetKeyMap();
      keyMap[key]     = info;
    }

    static BOOL IsSingleton(const TypeKey & key)
    {
      PWaitAndSignal m(GetMutex());
      KeyMap & keyMap = GetKeyMap();
      if (keyMap.find(key) == keyMap.end())
        return FALSE;
      return keyMap[key].isSingleton;
    }

    static KeyList GetKeyList()
    { 
      PWaitAndSignal m(GetMutex());
      KeyMap & keyMap = GetKeyMap();
      KeyList list;
      typename KeyMap::const_iterator entry;
      for (entry = keyMap.begin(); entry != keyMap.end(); ++entry)
        list.push_back(entry->first);
      return list;
    }

    static PMutex & GetMutex()
    { static PMutex mutex; return mutex; }

    static KeyMap & GetKeyMap()
    { static KeyMap keyMap; return keyMap; }
};

template <class AbstractType, class ConcreteType, typename TypeKey = PString>
class PAbstractFactory
{
  public:
    static AbstractType * CreateInstance()
    { return new ConcreteType; }

    PAbstractFactory(const TypeKey & key)
    { PGenericFactory<AbstractType>::Register(key, &CreateInstance, FALSE); }
};

template <class AbstractType, class ConcreteType, typename TypeKey = PString>
class PAbstractSingletonFactory
{
  public:
    static AbstractType & GetInstance()
    { static ConcreteType instance; return instance; }

    static AbstractType * CreateInstance()
    { return &GetInstance(); }

    PAbstractSingletonFactory(const TypeKey & key)
    { PGenericFactory<AbstractType>::Register(key, &CreateInstance, TRUE); }
};

#endif // _PFACTORY_H
