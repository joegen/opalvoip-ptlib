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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef _PFACTORY_H
#define _PFACTORY_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#ifndef _PTLIB_H
#include <ptlib.h>
#endif

#include <string>
#include <map>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif

/**
 *
 * These templates implement an Abstract Factory that allows
 * creation of a class "factory" that can be used to create
 * "concrete" instance that are descended from a abstract base class
 *
 * Given an abstract class A with a descendant concrete class B, the 
 * concrete class is registered by instantiating the PFactory template
 * as follows:
 *
 *       PFactory<A>::Worker<B> aFactory("B");
 *
 * To instantiate an object of type B, use the following:
 *
 *       A * b = PFactory<A>::CreateInstance("B");
 *
 * A vector containing the names of all of the concrete classes for an
 * abstract type can be obtained as follows:
 *
 *       PFactory<A>::KeyList_T list = PFactory<A>::GetKeyList()
 *
 * Note that these example assumes that the "key" type for the factory
 * registration is of the default type PString. If a different key type
 * is needed, then it is necessary to specify the key type:
 *
 *       PFactory<C, unsigned>::Worker<D> aFactory(42);
 *       C * d = PFactory<C, unsigned>::CreateInstance(42);
 *       PFactory<C, unsigned>::KeyList_T list = PFactory<C, unsigned>::GetKeyList()
 *
 * The factory functions also allow the creation of "singleton" factories that return a 
 * single instance for all calls to CreateInstance. This can be done by passing a "true"
 * as a second paramater to the factory registration as shown below, which will cause a single
 * instance to be minted upon the first call to CreateInstance, and then returned for all
 * subsequent calls. 
 *
 *      PFactory<A>::Worker<E> eFactory("E", true);
 *
 * It is also possible to manually set the instance in cases where the object needs to be created non-trivially.
 *
 * The following types are defined as part of the PFactory template class:
 *
 *     KeyList_T    a vector<> of the key type (usually std::string)
 *     Worker       an abstract factory for a specified concrete type
 *     KeyMap_T     a map<> that converts from the key type to the Worker instance
 *                  for each concrete type registered for a specific abstract type
 *
 * As a side issue, note that the factory lists are all thread safe for addition,
 * creation, and obtaining the key lists.
 *
 */

// this define the default class to be used for keys into PFactories
//typedef PString PDefaultPFactoryKey;
typedef std::string PDefaultPFactoryKey;


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

    class FactoryMap : public std::map<std::string, PFactoryBase *>
    {
      public:
        FactoryMap() { }
        ~FactoryMap();
    };

    static FactoryMap & GetFactories();
    static PMutex & GetFactoriesMutex();

    PMutex mutex;

  private:
    PFactoryBase(const PFactoryBase &) {}
    void operator=(const PFactoryBase &) {}
};


/** Template class for generic factories of an abstract class.
  */
template <class _Abstract_T, typename _Key_T = PDefaultPFactoryKey>
class PFactory : PFactoryBase
{
  public:
    typedef _Key_T      Key_T;
    typedef _Abstract_T Abstract_T;

    class WorkerBase
    {
      protected:
        WorkerBase(bool singleton = false)
          : isDynamic(false),
            isSingleton(singleton),
            singletonInstance(NULL),
            deleteSingleton(false)
        { }
        WorkerBase(Abstract_T * instance, bool _deleteSingleton = true)
          : isDynamic(true),
            isSingleton(true),
            singletonInstance(instance),
            deleteSingleton(_deleteSingleton)
        { }

        virtual ~WorkerBase()
        {
          if (deleteSingleton)
            delete singletonInstance;
        }

        Abstract_T * CreateInstance(const Key_T & key)
        {
          if (!isSingleton)
            return Create(key);

          if (singletonInstance == NULL)
            singletonInstance = Create(key);
          return singletonInstance;
        }

        virtual Abstract_T * Create(const Key_T & /*key*/) const { return singletonInstance; }

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
        Worker(const Key_T & key, bool singleton = false)
          : WorkerBase(singleton)
        {
          PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
          PFactory<_Abstract_T, _Key_T>::Register(key, this);   // here
        }

      protected:
        virtual Abstract_T * Create(const Key_T & /*key*/) const
        {
#if PMEMORY_HEAP
          // Singletons are never deallocated, so make sure they arenot reported as a leak
          PBoolean previousIgnoreAllocations = PMemoryHeap::SetIgnoreAllocations(WorkerBase::isSingleton);
#endif
          Abstract_T * instance = new _Concrete_T;
#if PMEMORY_HEAP
          PMemoryHeap::SetIgnoreAllocations(previousIgnoreAllocations);
#endif
          return instance;
        }
    };

    typedef std::map<_Key_T, WorkerBase *> KeyMap_T;
    typedef std::vector<_Key_T> KeyList_T;

    static void Register(const _Key_T & key, WorkerBase * worker)
    {
      GetInstance().Register_Internal(key, worker);
    }

    static void Register(const _Key_T & key, Abstract_T * instance, bool autoDeleteInstance = true)
    {
      WorkerBase * w = PNEW WorkerBase(instance, autoDeleteInstance);
      GetInstance().Register_Internal(key, w);
    }

    static PBoolean RegisterAs(const _Key_T & newKey, const _Key_T & oldKey)
    {
      return GetInstance().RegisterAs_Internal(newKey, oldKey);
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

    template <class Derived_T>
    static Derived_T * CreateInstanceAs(const _Key_T & key)
    {
      return dynamic_cast<Derived_T *>(GetInstance().CreateInstance_Internal(key));
    }

    static PBoolean IsSingleton(const _Key_T & key)
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
      PWaitAndSignal m(GetFactoriesMutex());
      FactoryMap & factories = GetFactories();
      FactoryMap::const_iterator entry = factories.find(className);
      if (entry != factories.end()) {
        PAssert(entry->second != NULL, "Factory map returned NULL for existing key");
        PFactoryBase * b = entry->second;
        // don't use the following dynamic cast, because gcc does not like it
        //PFactory * f = dynamic_cast<PFactory*>(b);
        return *(PFactory *)b;
      }

      PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
      PFactory * factory = new PFactory;
      factories[className] = factory;
      return *factory;
    }


    void Register_Internal(const _Key_T & key, WorkerBase * worker)
    {
      PWaitAndSignal m(mutex);
      if (keyMap.find(key) == keyMap.end()) {
        keyMap[key] = worker;
        if (worker->isSingleton)
          worker->CreateInstance(key);
      }
    }

    PBoolean RegisterAs_Internal(const _Key_T & newKey, const _Key_T & oldKey)
    {
      PWaitAndSignal m(mutex);
      if (keyMap.find(oldKey) == keyMap.end())
        return PFalse;
      keyMap[newKey] = keyMap[oldKey];
      return PTrue;
    }

    void Unregister_Internal(const _Key_T & key)
    {
      PWaitAndSignal m(mutex);
      typename KeyMap_T::iterator r = keyMap.find(key);
      if (r != keyMap.end()) {
        if (r->second->isDynamic)
          delete r->second;
        keyMap.erase(r);
      }
    }

    void UnregisterAll_Internal()
    {
      PWaitAndSignal m(mutex);
      while (keyMap.size() > 0)
        keyMap.erase(keyMap.begin());
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
        return entry->second->CreateInstance(key);
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

//
//  this macro is used to initialise the static member variable used to force factories to instantiate
//
#define PLOAD_FACTORY(AbstractType, KeyType) \
  namespace PWLibFactoryLoader { \
    extern int AbstractType##_##KeyType##_loader; \
    static int AbstractType##_##KeyType##_loader_instance = AbstractType##_##KeyType##_loader; \
  };


//
//  this macro is used to instantiate a static variable that accesses the static member variable 
//  in a factory forcing it to load
//
#define PINSTANTIATE_FACTORY(AbstractType, KeyType) \
  namespace PWLibFactoryLoader { int AbstractType##_##KeyType##_loader; }


#endif // _PFACTORY_H
