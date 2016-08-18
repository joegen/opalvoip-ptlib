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

#ifndef PTLIB_FACTORY_H
#define PTLIB_FACTORY_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#include <typeinfo>
#include <string>
#include <map>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif

/*! \page pageFactories Factory handling in the PTLib library
 
  These templates implement an Abstract Factory that allows
  creation of a class "factory" that can be used to create
  "concrete" instance that are descended from a abstract base class
 
  Given an abstract class A with a descendant concrete class B, the 
  concrete class is registered by instantiating the PFactory template
  as follows:
 
        PFactory<A>::Worker<B> aFactory("B");
 
  To instantiate an object of type B, use the following:
 
        A * b = PFactory<A>::CreateInstance("B");
 
  A vector containing the names of all of the concrete classes for an
  abstract type can be obtained as follows:
 
        PFactory<A>::KeyList_T list = PFactory<A>::GetKeyList()
 
  Note that these example assumes that the "key" type for the factory
  registration is of the default type PString. If a different key type
  is needed, then it is necessary to specify the key type:
 
        PFactory<C, unsigned>::Worker<D> aFactory(42);
        C * d = PFactory<C, unsigned>::CreateInstance(42);
        PFactory<C, unsigned>::KeyList_T list = PFactory<C, unsigned>::GetKeyList()
 
  The factory functions also allow the creation of "singleton" factories that return a 
  single instance for all calls to CreateInstance. This can be done by passing a "true"
  as a second paramater to the factory registration as shown below, which will cause a single
  instance to be minted upon the first call to CreateInstance, and then returned for all
  subsequent calls. 
 
       PFactory<A>::Worker<E> eFactory("E", true);
 
  It is also possible to manually set the instance in cases where the object needs to be created non-trivially.
 
  A version that can construct factory concrete classes that take an argument
  is also available.
 
        PParamFactory<C, double, unsigned>::Worker<D> aFactory(42);
        C * d = PParamFactory<C, double, unsigned>::CreateInstance(42, 3.14159);
 
  The argument can, of course, be a class/struct to allow multiple parameters
  during construction.
 
  The following types are defined as part of the PFactory template class:
 
      KeyList_T    a vector<> of the key type (usually std::string)
      Worker       an abstract factory for a specified concrete type
      WorkerMap_T  a map<> that converts from the key type to the Worker instance
                   for each concrete type registered for a specific abstract type
 
  As a side issue, note that the factory lists are all thread safe for addition,
  creation, and obtaining the key lists.
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

    virtual void DestroySingletons() = 0;

    class FactoryMap : public std::map<std::string, PFactoryBase *>, public PMutex
    {
      public:
        ~FactoryMap();
        void DestroySingletons();
    };

    static FactoryMap & GetFactories();

  protected:
    static PFactoryBase & InternalGetFactory(const std::string & className, PFactoryBase * (*createFactory)());

    template <class TheFactory> static PFactoryBase * CreateFactory() { return new TheFactory; }

    template <class TheFactory> static TheFactory & GetFactoryAs()
    {
      return dynamic_cast<TheFactory&>(InternalGetFactory(typeid(TheFactory).name(), CreateFactory<TheFactory>));
    }

  protected:
    PMutex m_mutex;

  private:
    PFactoryBase(const PFactoryBase &) {}
    void operator=(const PFactoryBase &) {}
};


/** Template class for generic factories of an abstract class.
  */
template <class AbstractClass, typename ParamType, typename KeyType>
class PFactoryTemplate : public PFactoryBase
{
  public:
    class WorkerBase;

    typedef AbstractClass                  Abstract_T;
    typedef ParamType                      Param_T;
    typedef KeyType                        Key_T;
    typedef std::vector<Key_T>             KeyList_T;

    struct WorkerWrap
    {
      WorkerWrap(WorkerBase * worker, bool autoDelete) : m_worker(worker), m_autoDelete(autoDelete) { }
      WorkerBase * m_worker;
      bool         m_autoDelete;
    };
    typedef std::map<Key_T, WorkerWrap>    WorkerMap_T;
    typedef typename WorkerMap_T::iterator WorkerIter_T;

    class WorkerBase
    {
      public:
        enum Types {
          NonSingleton,
          StaticSingleton,
          DynamicSingleton
        };

        WorkerBase(bool singleton = false)
          : m_type(singleton ? DynamicSingleton : NonSingleton)
          , m_singletonInstance(NULL)
        {
        }

        WorkerBase(Abstract_T * instance, bool delSingleton = true)
          : m_type(delSingleton ? DynamicSingleton : StaticSingleton)
          , m_singletonInstance(instance)
        {
        }

        virtual ~WorkerBase()
        {
          DestroySingleton();
        }

        Abstract_T * CreateInstance(Param_T param)
        {
          if (m_type == NonSingleton)
            return Create(param);

          if (m_singletonInstance == NULL)
            m_singletonInstance = Create(param);
          return m_singletonInstance;
        }

        virtual void DestroySingleton()
        {
          if (m_type == DynamicSingleton) {
            delete m_singletonInstance;
            m_singletonInstance = NULL;
          }
        }

        bool IsSingleton() const { return m_type != NonSingleton; }

      protected:
        virtual Abstract_T * Create(Param_T /*param*/) const
        {
          PAssert(this->m_type == StaticSingleton, "Incorrect factory worker descendant");
          return this->m_singletonInstance;
        }

        Types        m_type;
        Abstract_T * m_singletonInstance;

      friend class PFactoryTemplate;
    };

    virtual void DestroySingletons()
    {
      for (WorkerIter_T it = m_workers.begin(); it != m_workers.end(); ++it)
        it->second.m_worker->DestroySingleton();
    }

  protected:
    PFactoryTemplate()
    { }

    ~PFactoryTemplate()
    {
      DestroySingletons();
      InternalUnregisterAll();
    }

    bool InternalRegister(const Key_T & key, WorkerBase * worker, bool autoDeleteWorker)
    {
      PWaitAndSignal mutex(m_mutex);
      typename WorkerMap_T::iterator it = m_workers.find(key);
      if (it != m_workers.end()) {
        return it->second.m_worker == worker;
      }

      PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
      m_workers.insert(make_pair(key, WorkerWrap(PAssertNULL(worker), autoDeleteWorker)));
      return true;
    }

    bool InternalRegister(const Key_T & key, Abstract_T * instance, bool autoDeleteInstance)
    {
      PWaitAndSignal mutex(m_mutex);
      typename WorkerMap_T::iterator it = m_workers.find(key);
      if (it != m_workers.end())
        return it->second.m_worker->m_singletonInstance == instance;

      PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
      m_workers.insert(make_pair(key, WorkerWrap(PNEW WorkerBase(instance, autoDeleteInstance), true)));
      return true;
    }

    PBoolean InternalRegisterAs(const Key_T & newKey, const Key_T & oldKey)
    {
      PWaitAndSignal mutex(m_mutex);
      typename WorkerMap_T::iterator itOld = m_workers.find(oldKey);
      if (itOld == m_workers.end())
        return false;

      typename WorkerMap_T::iterator itNew = m_workers.find(newKey);
      if (itNew != m_workers.end())
        return itNew->second.m_worker == itOld->second.m_worker;

      m_workers.insert(make_pair(newKey, WorkerWrap(itOld->second.m_worker, false)));
      return true;
    }

    void InternalUnregister(const Key_T & key)
    {
      m_mutex.Wait();
      typename WorkerMap_T::iterator it = m_workers.find(key);
      if (it != m_workers.end()) {
        if (it->second.m_autoDelete)
          delete it->second.m_worker;
        m_workers.erase(it);
      }
      m_mutex.Signal();
    }

    void InternalUnregister(WorkerBase * instance)
    {
      m_mutex.Wait();
      for (WorkerIter_T it = m_workers.begin(); it != m_workers.end(); ++it) {
        if (it->second == instance) {
          InternalUnregister(it->first);
          break;
        }
      }
      m_mutex.Signal();
    }

    void InternalUnregisterAll()
    {
      m_mutex.Wait();
      for (typename WorkerMap_T::iterator it = m_workers.begin(); it != m_workers.end(); ++it) {
        if (it->second.m_autoDelete)
          delete it->second.m_worker;
      }
      m_workers.clear();
      m_mutex.Signal();
    }

    bool InternalIsRegistered(const Key_T & key)
    {
      PWaitAndSignal mutex(m_mutex);
      return m_workers.find(key) != m_workers.end();
    }

    Abstract_T * InternalCreateInstance(const Key_T & key, Param_T param)
    {
      PWaitAndSignal mutex(m_mutex);
      WorkerIter_T entry = m_workers.find(key);
      return entry == m_workers.end() ? NULL : entry->second.m_worker->CreateInstance(param);
    }

    void InternalDestroy(const Key_T & key, Abstract_T * instance)
    {
      PWaitAndSignal mutex(m_mutex);
      WorkerIter_T entry = m_workers.find(key);
      if (entry != m_workers.end() && !entry->second.m_worker->IsSingleton())
        delete instance;
    }

    bool InternalIsSingleton(const Key_T & key)
    {
      PWaitAndSignal mutex(m_mutex);
      WorkerIter_T entry = m_workers.find(key);
      return entry != m_workers.end() && entry->second.m_worker->IsSingleton();
    }

    KeyList_T InternalGetKeyList()
    { 
      PWaitAndSignal mutex(m_mutex);
      KeyList_T list;
      for (WorkerIter_T entry = m_workers.begin(); entry != m_workers.end(); ++entry)
        list.push_back(entry->first);
      return list;
    }

  protected:
    WorkerMap_T m_workers;

  private:
    PFactoryTemplate(const PFactoryTemplate &) {}
    void operator=(const PFactoryTemplate &) {}

  friend class PFactoryBase;
};


/// this define the default class to be used for keys into PFactories
typedef std::string PDefaultPFactoryKey;


#define PFACTORY_STATICS(cls) \
  static cls & GetFactory()                                            { return PFactoryBase::GetFactoryAs<cls>(); } \
  static bool Register(const Key_T & k, WorkerBase_T *w, bool a=false) { return GetFactory().InternalRegister(k, w, a); } \
  static bool Register(const Key_T & k, Abstract_T * i, bool a = true) { return GetFactory().InternalRegister(k, i, a); } \
  static bool RegisterAs(const Key_T & synonym, const Key_T & original){ return GetFactory().InternalRegisterAs(synonym, original); } \
  static void Unregister(const Key_T & k)                              {        GetFactory().InternalUnregister(k); } \
  static void Unregister(WorkerBase_T * a)                             {        GetFactory().InternalUnregister(a); } \
  static void UnregisterAll()                                          {        GetFactory().InternalUnregisterAll(); } \
  static bool IsRegistered(const Key_T & k)                            { return GetFactory().InternalIsRegistered(k); } \
  static bool IsSingleton(const Key_T & k)                             { return GetFactory().InternalIsSingleton(k); } \
  static typename Base_T::KeyList_T GetKeyList()                       { return GetFactory().InternalGetKeyList(); } \
  static PMutex & GetMutex()                                           { return GetFactory().m_mutex; } \


/** Class for a factory to create concrete class instances without parameters
    during construction.
  */
template <class AbstractClass, typename KeyType = PDefaultPFactoryKey>
class PFactory : public PFactoryTemplate<AbstractClass, const KeyType &, KeyType>
{
  public:
    typedef PFactoryTemplate<AbstractClass, const KeyType &, KeyType> Base_T;
    typedef typename Base_T::WorkerBase WorkerBase_T;
    typedef typename Base_T::Abstract_T Abstract_T;
    typedef typename Base_T::Param_T    Param_T;
    typedef typename Base_T::Key_T      Key_T;

    PFACTORY_STATICS(PFactory);

    template <class ConcreteClass>
    class Worker : protected WorkerBase_T
    {
      private:
        Key_T * m_key;
      public:
        Worker(const Key_T & key, bool singleton = false)
          : WorkerBase_T(singleton)
          , m_key(new Key_T(key))
        {
          PAssert(Register(key, this), "Factory Worker already registered");
        }

        ~Worker()
        {
          if (m_key == NULL)
            return;
          Unregister(*m_key);
          delete m_key;
          m_key = NULL;
        }

        const Key_T & GetKey() const { return *m_key; }

      protected:
        virtual Abstract_T * Create(Param_T) const
        {
          return new ConcreteClass();
        }
    };

    static Abstract_T * CreateInstance(const Key_T & key)
    {
      return GetFactory().InternalCreateInstance(key, key);
    }

    template <class Derived_T>
    static Derived_T * CreateInstanceAs(const Key_T & key)
    {
      Abstract_T * instance = GetFactory().InternalCreateInstance(key, key);
      Derived_T * derived = dynamic_cast<Derived_T *>(instance);
      if (derived != NULL)
        return derived;

      GetFactory().InternalDestroy(key, instance);
      return NULL;
    }
};


/** Class for a factory to create concrete class instances which have a single
    parameter during construction.
  */
template <class AbstractClass, typename ParamType, typename KeyType = PDefaultPFactoryKey>
class PParamFactory : public PFactoryTemplate<AbstractClass, ParamType, KeyType>
{
  public:
    typedef PFactoryTemplate<AbstractClass, ParamType, KeyType> Base_T;
    typedef typename Base_T::WorkerBase WorkerBase_T;
    typedef typename Base_T::Abstract_T Abstract_T;
    typedef typename Base_T::Param_T    Param_T;
    typedef typename Base_T::Key_T      Key_T;

    PFACTORY_STATICS(PParamFactory);

    template <class ConcreteClass>
    class Worker : protected WorkerBase_T
    {
      private:
        Key_T * m_key;
      public:
        Worker(const Key_T & key, bool singleton = false)
          : WorkerBase_T(singleton)
          , m_key(new Key_T(key))
        {
          PAssert(Register(key, this), "Factory Worker already registered");
        }

        ~Worker()
        {
          if (m_key == NULL)
            return;
          Unregister(*m_key);
          delete m_key;
          m_key = NULL;
        }

        const Key_T & GetKey() const { return *m_key; }

      protected:
        virtual Abstract_T * Create(Param_T param) const
        {
          return new ConcreteClass(param);
        }
    };

    static Abstract_T * CreateInstance(const Key_T & key, Param_T param)
    {
      return GetFactory().InternalCreateInstance(key, param);
    }

    template <class Derived_T>
    static Derived_T * CreateInstanceAs(const Key_T & key, Param_T param)
    {
      Abstract_T * instance = GetFactory().InternalCreateInstance(key, param);
      Derived_T * derived = dynamic_cast<Derived_T *>(instance);
      if (derived != NULL)
        return derived;
      GetFactory().InternalDestroy(key, instance);
      return NULL;
    }
};


/** This macro is used to create a factory.
    This is mainly used for factories that exist inside a library and works in
    conjunction with the PFACTORY_LOAD() macro.

    When a factory is contained wholly within a single compilation module of
    a library, it is typical that a linker does not include ANY of the code in
    that module. To avoid this the header file that declares the abstract type
    should include a PFACTORY_LOAD() macro call for all concrete classes that
    are in the library. Then whan an application includes the abstract types
    header, it will force the load of all the possible concrete classes.
  */
#define PFACTORY_CREATE(factory, ConcreteClass, ...) \
  namespace PFactoryLoader { \
    int ConcreteClass##_link(int const *) { return 0; } \
    factory::Worker<ConcreteClass> ConcreteClass##_instance(__VA_ARGS__); \
  }

#define PFACTORY_SYNONYM(factory, ConcreteClass, name, key) \
  namespace PFactoryLoader { \
    bool ConcreteClass##name##_synonym = factory::RegisterAs(key, ConcreteClass##_instance.GetKey()); \
  }

#define PFACTORY_CREATE_SINGLETON(factory, ConcreteClass) \
        PFACTORY_CREATE(factory, ConcreteClass, typeid(ConcreteClass).name(), true)

#define PFACTORY_GET_SINGLETON(factory, ConcreteClass) \
        static ConcreteClass & GetInstance() { \
          return *factory::CreateInstanceAs<ConcreteClass>(typeid(ConcreteClass).name()); \
        }




/* This macro is used to force linking of factories.
   See PFACTORY_CREATE() for more information
 */
#define PFACTORY_LOAD(ConcreteType) \
  namespace PFactoryLoader { \
    extern int ConcreteType##_link(int const *); \
    static int const ConcreteType##_loader = ConcreteType##_link(&ConcreteType##_loader); \
  }


#endif // PTLIB_FACTORY_H


// End Of File ///////////////////////////////////////////////////////////////
