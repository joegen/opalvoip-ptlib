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

/*
 * These templates implement an Abstract Factory that allows
 * creation of a class "factory" that can be used to create
 * "concrete" instance that are descended from a abstract case class
 *
 * Given an abstract class A with a descendant concrete class B, the 
 * concrete class is registered by instantiating the PAbstractFactory
 * class as follows:
 *
 *       PAbstractFactory<A, B> baseCodecFactory("B");
 *
 * To instantiate the an object of type B, use the following
 *
 *       B * codec = PGenericFactory<BaseCodec>::CreateInstance("B");
 *  
 */

template <class AbstractType, typename TypeKey = std::string>
class PGenericFactory
{
  public:
    typedef AbstractType * (*CreatorFunctionType)();
    typedef std::map<TypeKey, CreatorFunctionType> KeyMap;

    static PGenericFactory & GetFactory()
    { static PGenericFactory factory; return factory; }

    static AbstractType * CreateInstance(const TypeKey & key)
    { return GetFactory()._CreateInstance(key); }

    static void Register(const TypeKey & key, CreatorFunctionType func)
    { return GetFactory()._Register(key, func); }

    PMutex & GetMutex()
    { return mutex; }

    KeyMap & GetKeyMap()
    { return keyMap; }

  protected:
    AbstractType * _CreateInstance(const TypeKey & key) const
    {
      PWaitAndSignal m(mutex);
      AbstractType * instance = NULL;
      typename KeyMap::const_iterator entry = keyMap.find(key);
      if (entry != keyMap.end())
        instance = entry->second();
      return instance;
    }

    void _Register(const TypeKey & key, CreatorFunctionType func)
    { PWaitAndSignal m(mutex); keyMap[key] = func; }

  protected:
    PMutex mutex;
    KeyMap keyMap;
};

template <class AbstractType, class ConcreteType>
class PAbstractFactory
{
  public:
    static AbstractType * CreateInstance()
    { return new ConcreteType; }

    PAbstractFactory(const std::string & key)
    { PGenericFactory<AbstractType>::GetFactory().Register(key, &CreateInstance); }
};


#endif // _PFACTORY_H
