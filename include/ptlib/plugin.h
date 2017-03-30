/*
 * plugin.h
 *
 * Plugin Class Declarations
 *
 * Portable Windows Library
 *
 * Contributor(s): Snark at GnomeMeeting
 */

#ifndef PTLIB_PLUGIN_H
#define PTLIB_PLUGIN_H

//////////////////////////////////////////////////////
//
//  these templates implement an adapter to make the old style device plugins appear in the new factory system
//

#include <ptlib/pfactory.h>

#ifdef PWLIB_PLUGIN_API_VERSION
  #ifdef PTLIB_PLUGIN_API_VERSION
    #warning Ignoring PWLIB_PLUGIN_API_VERSION as PTLIB_PLUGIN_API_VERSION is defined!
    #undef PWLIB_PLUGIN_API_VERSION
  #else
    #define PTLIB_PLUGIN_API_VERSION PWLIB_PLUGIN_API_VERSION
  #endif
#else
  #ifndef PTLIB_PLUGIN_API_VERSION
    #define PTLIB_PLUGIN_API_VERSION 1
  #endif
#endif


//////////////////////////////////////////////////////
//
//  Ancestor Service descriptor for plugins
//

class PPluginServiceDescriptor 
{
  public:
    static const char SeparatorChar;

    PPluginServiceDescriptor()
      : m_version(PTLIB_PLUGIN_API_VERSION)
      { }

    virtual ~PPluginServiceDescriptor() { }

    virtual unsigned GetPluginAPIVersion() const { return m_version; }
    virtual const char * GetServiceType() const = 0;
    virtual const char * GetServiceName() const = 0;
    virtual const char * GetFriendlyName() const { return GetServiceName(); }
    virtual bool  ValidateServiceName(const PString & name, P_INT_PTR userData) const;
    virtual PObject * CreateInstance(P_INT_PTR userData) const = 0;

  protected:
    unsigned m_version;
};


typedef PFactory<PPluginServiceDescriptor> PPluginFactory;


class PPluginDeviceDescriptor : public PPluginServiceDescriptor
{
  public:
    virtual bool         ValidateServiceName(const PString & name, P_INT_PTR userData) const;

    virtual PStringArray GetDeviceNames(P_INT_PTR userData) const;
    virtual bool         ValidateDeviceName(const PString & deviceName, P_INT_PTR userData) const;
    virtual bool         GetDeviceCapabilities(const PString & deviceName, void * capabilities) const;
};

typedef PPluginDeviceDescriptor PDevicePluginServiceDescriptor;   // Backward compatibility


//////////////////////////////////////////////////////

#define PCREATE_PLUGIN_SERVICE_EX(serviceType, BaseClass, extra) \
        class PPlugin_##serviceType : public BaseClass { \
          public: \
            static  const char * ServiceType() { return #serviceType; } \
            virtual const char * GetServiceType() const { return ServiceType(); } \
            extra \
        }

#define PCREATE_PLUGIN_SERVICE(serviceType) \
        PCREATE_PLUGIN_SERVICE_EX(serviceType, PPluginServiceDescriptor, )

#define PCREATE_PLUGIN_DEVICE(serviceType) \
        PCREATE_PLUGIN_SERVICE_EX(serviceType, PPluginDeviceDescriptor, )


//////////////////////////////////////////////////////

#define PCREATE_PLUGIN_ARG_5(serviceName, serviceType, InstanceClass, DescriptorClass, extra) \
        class PPlugin_##serviceType##_##serviceName : public DescriptorClass { \
          public: \
            static  const char * ServiceName() { return #serviceName; } \
            virtual const char * GetServiceName() const { return ServiceName(); } \
            virtual PObject * CreateInstance(P_INT_PTR) const { return new InstanceClass; } \
            extra \
        }; \
        PFACTORY_CREATE(PPluginFactory, PPlugin_##serviceType##_##serviceName, #serviceType #serviceName, true)

#define PCREATE_PLUGIN_ARG_4(serviceName, serviceType, InstanceClass, DescriptorClass) \
        PCREATE_PLUGIN_ARG_5(serviceName, serviceType, InstanceClass, DescriptorClass, )

#define PCREATE_PLUGIN_ARG_3(serviceName, serviceType, InstanceClass) \
        PCREATE_PLUGIN_ARG_4(serviceName, serviceType, InstanceClass, PPlugin_##serviceType)

#define PCREATE_PLUGIN_ARG_2(serviceName, serviceType) \
        PCREATE_PLUGIN_ARG_3(serviceName, serviceType, serviceType##_##serviceName)

#define PCREATE_PLUGIN_PART1(narg, args) PCREATE_PLUGIN_PART2(narg, args)
#define PCREATE_PLUGIN_PART2(narg, args) PCREATE_PLUGIN_ARG_##narg args

#define PCREATE_PLUGIN_STATIC(...) PCREATE_PLUGIN_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))


#define PPLUGIN_STATIC_LOAD(serviceName, serviceType) PFACTORY_LOAD(PPlugin_##serviceType##_##serviceName)


#if defined(_WIN32) && !defined(P_FORCE_STATIC_PLUGIN)
  #define P_FORCE_STATIC_PLUGIN 1  // always define static plugins in Windows, since otherwise they seem not to work
#endif

#if defined(P_PLUGINS) && !defined(P_FORCE_STATIC_PLUGIN)

  #define PCREATE_PLUGIN(serviceName, serviceType, ...) \
    PCREATE_PLUGIN_STATIC(serviceName, serviceType, __VA_ARGS__) \
    extern "C" void PWLibPlugin_TriggerRegister(PPluginManager * pluginMgr) \
      { pluginMgr->RegisterService(#serviceType #serviceName); } \
    extern "C" unsigned PWLibPlugin_GetAPIVersion (void) \
      { return PTLIB_PLUGIN_API_VERSION; }

#else

  #define PCREATE_PLUGIN(...) \
          PCREATE_PLUGIN_STATIC(__VA_ARGS__)

#endif


//////////////////////////////////////////////////////


#endif // PTLIB_PLUGIN_H


// End Of File ///////////////////////////////////////////////////////////////
