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
 * Revision 1.11  2004/06/17 06:35:12  csoutheren
 * Use attribute (( constructor )) to guarantee that factories are
 * instantiated when loaded from a shared library
 *
 * Revision 1.10  2004/06/03 13:30:57  csoutheren
 * Renamed INSTANTIATE_FACTORY to avoid potential namespace collisions
 * Added documentaton on new PINSTANTIATE_FACTORY macro
 * Added generic form of PINSTANTIATE_FACTORY
 *
 * Revision 1.9  2004/06/03 12:47:58  csoutheren
 * Decomposed PFactory declarations to hopefully avoid problems with Windows DLLs
 *
 * Revision 1.8  2004/06/01 05:44:12  csoutheren
 * Added typedefs to allow access to types
 * Changed singleton class to use new so as to allow full cleanup
 *
 * Revision 1.7  2004/05/23 12:33:56  rjongbloed
 * Made some subtle changes to the way the static variables are instantiated in
 *   the factoris to fix problems with DLL's under windows. May not be final solution.
 *
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
 * creation, and obtaining the key lists.
 *
 * On Windows, static variables declared within functions in DLLs are put into
 * seperate data spaces. As the factory code uses a template that declares
 * the factory instance within a static function, this results in each DLL and
 * the main program all getting seperate factories. As a result, the factory
 * instance must be defined seperately using the PINSTANTIATE_FACTORY macro.
 * This ensures that the static function that contains the factory instance
 * is only declared in one compile unit, and hence, all usages must use the
 * same factory instance.
 *
 * Using the example above, the factory is instantiated as follows:
 * 
 *      PINSTANTIATE_FACTORY(A)
 *
 * Note that INSTANTIATE_FACTORY assumes the default key type is being used (PString)
 * If another key type is used, then the PINSTANTIATE_GENERIC_FACTORY macro
 * can be used as follows
 *
 *      PINSTANTIATE_GENERIC_FACTORY(C, unsigned)
 *  
 */

template <class _Abstract_T, typename _Key_T = PString>
class PGenericFactory
{
  public:
    typedef _Key_T      Key_T;
    typedef _Abstract_T Abstract_T;
    typedef _Abstract_T * (*CreatorFunction_T)();

    class AbstractInfo {
      public:
        AbstractInfo()
          : creator(NULL), isSingleton(FALSE), instance(NULL)
        { }
        AbstractInfo(CreatorFunction_T creat, BOOL single)
          : creator(creat), isSingleton(single), instance(NULL)
        { }
        AbstractInfo(_Abstract_T * inst, BOOL single)
          : creator(NULL), isSingleton(single), instance(inst)
        { }
        CreatorFunction_T creator;
        BOOL isSingleton;
        _Abstract_T * instance;
    };

    typedef std::map<_Key_T, AbstractInfo> KeyMap_T;
    typedef std::vector<_Key_T> KeyList_T;

    static void Register(const _Key_T & key, CreatorFunction_T func, BOOL isSingleton)
    { Register(key, AbstractInfo(func, isSingleton)); }

    static void Register(const _Key_T & key, _Abstract_T * instance, BOOL isSingleton)
    { Register(key, AbstractInfo(instance, isSingleton)); }

    static void Register(const _Key_T & key, const AbstractInfo & info)
    {
      PWaitAndSignal m(GetMutex());
      KeyMap_T & keyMap = GetKeyMap();
      if (keyMap.find(key) == keyMap.end())
        keyMap[key] = info;
    }

    static void Unregister(const _Key_T & key)
    {
      PWaitAndSignal m(GetMutex());
      KeyMap_T & keyMap = GetKeyMap();
      keyMap.erase(key);
    }

    static _Abstract_T * CreateInstance(const _Key_T & key)
    {
      PWaitAndSignal m(GetMutex());
      KeyMap_T & keyMap = GetKeyMap();
      _Abstract_T * instance = NULL;
      typename KeyMap_T::const_iterator entry = keyMap.find(key);
      if (entry != keyMap.end()) {
        if (entry->second.isSingleton && entry->second.instance != NULL)
          instance = entry->second.instance;
        else if (entry->second.creator != NULL)
          instance = (*(entry->second.creator))();
      }
      return instance;
    }

    static BOOL IsSingleton(const _Key_T & key)
    {
      PWaitAndSignal m(GetMutex());
      KeyMap_T & keyMap = GetKeyMap();
      if (keyMap.find(key) == keyMap.end())
        return FALSE;
      return keyMap[key].isSingleton;
    }

    static KeyList_T GetKeyList()
    { 
      PWaitAndSignal m(GetMutex());
      KeyMap_T & keyMap = GetKeyMap();
      KeyList_T list;
      typename KeyMap_T::const_iterator entry;
      for (entry = keyMap.begin(); entry != keyMap.end(); ++entry)
        list.push_back(entry->first);
      return list;
    }

    static PMutex & GetMutex()
    { return GetFactory().mutex; }

    static KeyMap_T & GetKeyMap()
    { return GetFactory().keyMap; }

  protected:
    PGenericFactory()
    { }

    static PGenericFactory & GetFactory();

    PMutex mutex;
    KeyMap_T keyMap;

  private:
    PGenericFactory(const PGenericFactory &) {}
    void operator=(const PGenericFactory &) {}
};

#ifdef __GNUC__
#define	P_LOAD_FACTORY(Abstract_T, Key_T) \
void __attribute__ ((constructor)) PWLib_LoadFactory##Abstract_T##_##Key_T(void) \
{ PGenericFactory<Abstract_T, Key_T>::GetMutex(); } 
#else
#define	P_LOAD_FACTORY(Abstract_T, Key_T)
#endif

#define PINSTANTIATE_GENERIC_FACTORY(Abstract_T, Key_T) \
PGenericFactory<Abstract_T, Key_T> & PGenericFactory<Abstract_T, Key_T>::GetFactory() \
{ \
  static PGenericFactory<Abstract_T, Key_T> factory; \
  return factory; \
} \
P_LOAD_FACTORY(Abstract_T, Key_T) \

#define PINSTANTIATE_FACTORY(Abstract_T) \
PINSTANTIATE_GENERIC_FACTORY(Abstract_T, PString)

template <class _Abstract_T, class ConcreteType, typename _Key_T = PString>
class PAbstractFactory
{
  public:
    PAbstractFactory(const _Key_T & key)
    { PGenericFactory<_Abstract_T>::Register(key, &CreateInstance, FALSE); }

    static _Abstract_T * CreateInstance()
    { return new ConcreteType; }

  private:
    PAbstractFactory(const PAbstractFactory &) {}
    void operator=(const PAbstractFactory &) {}
};


template <class _Abstract_T, class ConcreteType, typename _Key_T = PString>
class PAbstractSingletonFactory
{
  public:
    PAbstractSingletonFactory(const _Key_T & key)
    { PGenericFactory<_Abstract_T>::Register(key, &CreateInstance, TRUE); }

    static _Abstract_T * CreateInstance()
    { return &GetInstance(); }

    static ConcreteType & GetInstance()
    {
      static ConcreteType * instance = NULL;
      if (instance == NULL)
        instance = new ConcreteType;
      return *instance;
    }

  private:
    PAbstractSingletonFactory(const PAbstractSingletonFactory &) {}
    void operator=(const PAbstractSingletonFactory &) {}
};

#endif // _PFACTORY_H
