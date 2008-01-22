/*
 * plugin.h
 *
 * Plugin Class Declarations
 *
 * Portable Windows Library
 *
 * Contributor(s): Snark at GnomeMeeting
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef _PLUGIN_H
#define _PLUGIN_H

//////////////////////////////////////////////////////
//
//  these templates implement an adapter to make the old style device plugins appear in the new factory system
//

#include <ptlib/pfactory.h>

template <class _Abstract_T, typename _Key_T = PString>
class PDevicePluginFactory : public PFactory<_Abstract_T, _Key_T>
{
  public:
    class Worker : public PFactory<_Abstract_T, _Key_T>::WorkerBase 
    {
      public:
        Worker(const _Key_T & key, bool singleton = false)
          : PFactory<_Abstract_T, _Key_T>::WorkerBase(singleton)
        {
          PFactory<_Abstract_T, _Key_T>::Register(key, this);
        }

        ~Worker()
        {
          typedef typename PFactory<_Abstract_T, _Key_T>::WorkerBase WorkerBase_T;
          typedef std::map<_Key_T, WorkerBase_T *> KeyMap_T;
          _Key_T key;

          KeyMap_T km = PFactory<_Abstract_T, _Key_T>::GetKeyMap();

          typename KeyMap_T::const_iterator entry;
          for (entry = km.begin(); entry != km.end(); ++entry) {
            if (entry->second == this) {
              key = entry->first;
              break;
            }
          }
          if (key != NULL)
            PFactory<_Abstract_T, _Key_T>::Unregister(key);
        }

      protected:
        virtual _Abstract_T * Create(const _Key_T & key) const;
    };
};

class PDevicePluginAdapterBase
{
  public:
    PDevicePluginAdapterBase()
    { }
    virtual ~PDevicePluginAdapterBase()
    { }
    virtual void CreateFactory(const PString & device) = 0;
};

template <typename DeviceBase>
class PDevicePluginAdapter : public PDevicePluginAdapterBase
{
  public:
    typedef PDevicePluginFactory<DeviceBase> Factory_T;
    typedef typename Factory_T::Worker Worker_T;
    void CreateFactory(const PString & device)
    {
      if (!(Factory_T::IsRegistered(device)))
        new Worker_T(device, PFalse);
    }
};


#ifndef PWLIB_PLUGIN_API_VERSION
#define PWLIB_PLUGIN_API_VERSION 0
#endif


//////////////////////////////////////////////////////
//
//  Ancestor Service descriptor for plugins
//

class PPluginServiceDescriptor 
{
  public:
    PPluginServiceDescriptor() { version = PWLIB_PLUGIN_API_VERSION; }
    virtual ~PPluginServiceDescriptor() { }

    virtual unsigned GetPluginAPIVersion() const { return version; }

  protected:
    unsigned version;
};


class PDevicePluginServiceDescriptor : public PPluginServiceDescriptor
{
  public:
    static const char SeparatorChar;

    virtual PObject *   CreateInstance(int userData) const = 0;
    virtual PStringList GetDeviceNames(int userData) const = 0;
    virtual bool        ValidateDeviceName(const PString & deviceName, int userData) const;
	virtual bool        GetDeviceCapabilities(const PString & deviceName, 
											         void * capabilities) const;
};


//////////////////////////////////////////////////////
//
// Define a service provided by a plugin, which consists of the following:
//
//    serviceType - the base class name of the service which is used to identify
//                  the service type, such as PSoundChannel, 
//
//    serviceName - the name of the service provided by the plugin. This will usually
//                  be related to the class implementing the service, such as:
//                       service name = OSS, class name = PSoundChannelOSS
//
//    descriptor  - a pointer to a class containing pointers to any static functions
//                  for this class
//   
//

class PPluginService: public PObject
{
  public:
    PPluginService(const PString & _serviceName,
                   const PString & _serviceType,
                   PPluginServiceDescriptor *_descriptor)
    {
      serviceName = _serviceName;
      serviceType = _serviceType;
      descriptor  = _descriptor;
    }

    PString serviceName;
    PString serviceType;
    PPluginServiceDescriptor * descriptor;
};


//////////////////////////////////////////////////////
//
//  These crazy macros are needed to cause automatic registration of 
//  static plugins. They are made more complex by the arcane behaviour
//  of the Windows link system that requires an external reference in the
//  object module before it will instantiate any globals in in it
//

#define PCREATE_PLUGIN_REGISTERER(serviceName, serviceType, descriptor) \
class PPlugin_##serviceType##_##serviceName##_Registration { \
  public: \
    PPlugin_##serviceType##_##serviceName##_Registration(PPluginManager * pluginMgr) \
    { \
      static PDevicePluginFactory<serviceType>::Worker factory(#serviceName); \
      pluginMgr->RegisterService(#serviceName, #serviceType, descriptor); \
    } \
    int kill_warning; \
}; \

#ifdef _WIN32

#define PCREATE_PLUGIN_STATIC(serviceName, serviceType, descriptor) \
PCREATE_PLUGIN_REGISTERER(serviceName, serviceType, descriptor) \
PPlugin_##serviceType##_##serviceName##_Registration \
  PPlugin_##serviceType##_##serviceName##_Registration_Instance(&PPluginManager::GetPluginManager()); \

#define PWLIB_STATIC_LOAD_PLUGIN(serviceName, serviceType) \
  class PPlugin_##serviceType##_##serviceName##_Registration; \
  extern PPlugin_##serviceType##_##serviceName##_Registration PPlugin_##serviceType##_##serviceName##_Registration_Instance; \
  static PPlugin_##serviceType##_##serviceName##_Registration * PPlugin_##serviceType##_##serviceName##_Registration_Static_Library_Loader = &PPlugin_##serviceType##_##serviceName##_Registration_Instance;

// Win32 only has static plugins at present, maybe one day ...
#define P_FORCE_STATIC_PLUGIN 1

#else

#ifdef USE_GCC
#define PCREATE_PLUGIN_STATIC(serviceName, serviceType, descriptor) \
static void __attribute__ (( constructor )) PWLIB_StaticLoader_##serviceName##_##serviceType() \
{ PPluginManager::GetPluginManager().RegisterService(#serviceName, #serviceType, descriptor); } \

#else
#define PCREATE_PLUGIN_STATIC(serviceName, serviceType, descriptor) \
extern int PWLIB_gStaticLoader__##serviceName##_##serviceType; \
static int PWLIB_StaticLoader_##serviceName##_##serviceType() \
{ PPluginManager::GetPluginManager().RegisterService(#serviceName, #serviceType, descriptor); return 1; } \
int PWLIB_gStaticLoader__##serviceName##_##serviceType =  PWLIB_StaticLoader_##serviceName##_##serviceType(); 
#endif
#define PWLIB_STATIC_LOAD_PLUGIN(serviceName, serviceType) 

#endif


//////////////////////////////////////////////////////

#if defined(P_HAS_PLUGINS) && ! defined(P_FORCE_STATIC_PLUGIN)

#  define PCREATE_PLUGIN(serviceName, serviceType, descriptor) \
    PCREATE_PLUGIN_REGISTERER(serviceName, serviceType, descriptor) \
    extern "C" void PWLibPlugin_TriggerRegister (PPluginManager * pluginMgr) { \
    PPlugin_##serviceType##_##serviceName##_Registration \
        pplugin_##serviceType##_##serviceName##_Registration_Instance(pluginMgr); \
        pplugin_##serviceType##_##serviceName##_Registration_Instance.kill_warning = 0; \
    } \
    extern "C" unsigned PWLibPlugin_GetAPIVersion (void) \
    { return PWLIB_PLUGIN_API_VERSION; }

#else

#  define PCREATE_PLUGIN(serviceName, serviceType, descriptor) \
    PCREATE_PLUGIN_STATIC(serviceName, serviceType, descriptor)

#endif

//////////////////////////////////////////////////////

#endif
