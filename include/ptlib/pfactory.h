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
 * Revision 1.12  2004/06/30 12:17:04  rjongbloed
 * Rewrite of plug in system to use single global variable for all factories to avoid all sorts
 *   of issues with startup orders and Windows DLL multiple instances.
 *
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
 * "concrete" instance that are descended from a abstract base class
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
 */

/** Base class for generic factories.
    This classes reason for existance and the FactoryMap contained within it
    is to resolve issues with static global construction order and Windows DLL
    multiple instances issues. THis mechanism guarantees that the one and one
    only global variable (inside the GetFactories() function) is initialised
    before any other factory related instances of classes.
  */
class PFactoryBase
{
  protected:
    PFactoryBase()
    { }
  public:
    virtual ~PFactoryBase()
    { }

  protected:
    class FactoryMap : public std::map<std::string, PFactoryBase *>
    {
      public:
        FactoryMap() { }
        ~FactoryMap();
    };
    static FactoryMap & GetFactories();

    PMutex mutex;

  private:
    PFactoryBase(const PFactoryBase &) {}
    void operator=(const PFactoryBase &) {}
};


/** Template class for generic factories of an abstract class.
  */
template <class _Abstract_T, typename _Key_T = PString>
class PFactory : PFactoryBase
{
  public:
    typedef _Key_T      Key_T;
    typedef _Abstract_T Abstract_T;

    class WorkerBase
    {
      protected:
        WorkerBase(bool singleton)
          : isDynamic(false),
            isSingleton(singleton),
            singletonInstance(NULL),
            deleteSingleton(false)
        { }
        WorkerBase(Abstract_T * instance)
          : isSingleton(true),
            singletonInstance(instance),
            deleteSingleton(true)
        { }

        virtual ~WorkerBase()
        {
          if (deleteSingleton)
            delete singletonInstance;
        }

        Abstract_T * CreateInstance()
        {
          if (!isSingleton)
            return Create();

          if (singletonInstance == NULL)
            singletonInstance = Create();
          return singletonInstance;
        }

        virtual Abstract_T * Create() const { return singletonInstance; }

        bool         isDynamic;
        bool         isSingleton;
        Abstract_T * singletonInstance;
        bool         deleteSingleton;

      friend class PFactory<_Abstract_T, _Key_T>;
    };

    template <class _Concrete_T>
    class Worker : WorkerBase
    {
      public:
        Worker(const Key_T & key, bool singleton)
          : WorkerBase(singleton)
        {
          PFactory<_Abstract_T>::Register(key, this);
        }

      protected:
        virtual Abstract_T * Create() const { return new _Concrete_T; }
    };

    typedef std::map<_Key_T, WorkerBase *> KeyMap_T;
    typedef std::vector<_Key_T> KeyList_T;

    static void Register(const _Key_T & key, WorkerBase * worker)
    {
      GetInstance().Register_Internal(key, worker);
    }

    static void Register(const _Key_T & key, Abstract_T * instance)
    {
      GetInstance().Register_Internal(key, new WorkerBase(instance));
    }

    static void Unregister(const _Key_T & key)
    {
      GetInstance().Unregister_Internal(key);
    }

    static void UnregisterAll()
    {
      GetInstance().UnregisterAll_Internal();
    }

    static bool IsRegistered(const _Key_T & key)
    {
      return GetInstance().IsRegistered_Internal(key);
    }

    static _Abstract_T * CreateInstance(const _Key_T & key)
    {
      return GetInstance().CreateInstance_Internal(key);
    }

    static BOOL IsSingleton(const _Key_T & key)
    {
      return GetInstance().IsSingleton_Internal(key);
    }

    static KeyList_T GetKeyList()
    { 
      return GetInstance().GetKeyList_Internal();
    }

    static KeyMap_T & GetKeyMap()
    { 
      return GetInstance().keyMap;
    }

    static PMutex & GetMutex()
    {
      return GetInstance().mutex;
    }

  protected:
    PFactory()
    { }

    ~PFactory()
    {
      typename KeyMap_T::const_iterator entry;
      for (entry = keyMap.begin(); entry != keyMap.end(); ++entry) {
        if (entry->second->isDynamic)
          delete entry->second;
      }
    }

    static PFactory & GetInstance()
    {
      std::string className = typeid(PFactory).name();
      FactoryMap & factories = GetFactories();
      FactoryMap::const_iterator entry = factories.find(className);
      if (entry != factories.end())
        return *(dynamic_cast<PFactory*>(entry->second));

      PFactory * factory = new PFactory;
      factories[className] = factory;
      return *factory;
    }

    void Register_Internal(const _Key_T & key, WorkerBase * worker)
    {
      PWaitAndSignal m(mutex);
      if (keyMap.find(key) == keyMap.end())
        keyMap[key] = worker;
    }

    void Unregister_Internal(const _Key_T & key)
    {
      PWaitAndSignal m(mutex);
      keyMap.erase(key);
    }

    void UnregisterAll_Internal()
    {
      PWaitAndSignal m(mutex);
      keyMap.erase(keyMap.begin(), keyMap.end());
    }

    bool IsRegistered_Internal(const _Key_T & key)
    {
      PWaitAndSignal m(mutex);
      return keyMap.find(key) != keyMap.end();
    }

    _Abstract_T * CreateInstance_Internal(const _Key_T & key)
    {
      PWaitAndSignal m(mutex);
      typename KeyMap_T::const_iterator entry = keyMap.find(key);
      if (entry != keyMap.end())
        return entry->second->CreateInstance();
      return NULL;
    }

    bool IsSingleton_Internal(const _Key_T & key)
    {
      PWaitAndSignal m(mutex);
      if (keyMap.find(key) == keyMap.end())
        return false;
      return keyMap[key]->isSingleton;
    }

    KeyList_T GetKeyList_Internal()
    { 
      PWaitAndSignal m(mutex);
      KeyList_T list;
      typename KeyMap_T::const_iterator entry;
      for (entry = keyMap.begin(); entry != keyMap.end(); ++entry)
        list.push_back(entry->first);
      return list;
    }

    KeyMap_T keyMap;

  private:
    PFactory(const PFactory &) {}
    void operator=(const PFactory &) {}
};


#endif // _PFACTORY_H
