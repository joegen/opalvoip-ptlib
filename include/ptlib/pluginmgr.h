/*
 * pluginmgr.h
 *
 * Plugin Manager Class Declarations
 *
 * Portable Windows Library
 *
 * Contributor(s): Snark at GnomeMeeting
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PLUGINMGR_H
#define PTLIB_PLUGINMGR_H

#include <ptlib/plugin.h>


//////////////////////////////////////////////////////
//
//  Manager for plugins
//

class PPluginManager : public PObject
{
    PCLASSINFO(PPluginManager, PObject);
  public:
    // static functions for accessing global instances of plugin managers
    static PPluginManager & GetPluginManager();

    // Add a directory to the list of plugin directories (used by OPAL)
    void AddDirectory(const PDirectory & dir);

    /* Set the list of plugin directories using ':' (or ';' for Windows) separated string. */
    void SetDirectories(const PString & dirs);

    /* Set the list of plugin directories. */
    void SetDirectories(const PStringArray & dirs);

    // Load the plugins in the directories.
    void LoadDirectories();
    void LoadDirectory(const PDirectory & dir);

    // functions to load/unload a dynamic plugin 
    PBoolean LoadPlugin(const PString & fileName);

    void OnShutdown();
  
    // functions to access the plugins' services 
    PStringArray GetPluginTypes() const;
    PStringArray GetPluginsProviding(const PString & serviceType) const;
    PPluginServiceDescriptor * GetServiceDescriptor(const PString & serviceName, const PString & serviceType) const;
    PObject * CreatePluginsDevice(const PString & serviceName, const PString & serviceType, int userData = 0) const;
    PObject * CreatePluginsDeviceByName(const PString & deviceName, const PString & serviceType, int userData = 0, const PString & serviceName = PString::Empty()) const;
    PStringArray GetPluginsDeviceNames(const PString & serviceName, const PString & serviceType, int userData = 0) const;
    PBoolean GetPluginsDeviceCapabilities(const PString & serviceType,const PString & serviceName,const PString & deviceName,void * capabilities) const;

    // function to register a service (used by the plugins themselves)
    PBoolean RegisterService (const PString & serviceName, const PString & serviceType, PPluginServiceDescriptor * descriptor);


    enum NotificationCode {
      LoadingPlugIn,
      UnloadingPlugIn
    };

    /**Add a notifier to the plugin manager.
       The call back function is executed just after loading, or 
       just after unloading, a plugin. 

       To use define:
         PDECLARE_NOTIFIER(PDynaLink, YourClass, YourFunction);
       and
         void YourClass::YourFunction(PDynaLink & dll, INT code)
         {
           // code == 0 means loading
           // code == 1 means unloading
         }
       and to connect to the plugin manager:
         PPluginManager & mgr = PPluginManager::GetPluginManager();
         mgr->AddNotifier((PCREATE_NOTIFIER(YourFunction));
      */

    void AddNotifier(
      const PNotifier & filterFunction,
      PBoolean existing = false
    );

    void RemoveNotifier(
      const PNotifier & filterFunction
    );

  protected:
    PPluginManager();

    void CallNotifier(PDynaLink & dll, NotificationCode code);

    PList<PDirectory> m_directories;
    PStringList       m_suffixes;

    PMutex            m_pluginsMutex;
    PArray<PDynaLink> m_plugins;
    
    PMutex                 m_servicesMutex;
    PArray<PPluginService> m_services;

    PMutex           m_notifiersMutex;
    PList<PNotifier> m_notifiers;
};

//////////////////////////////////////////////////////
//
//  Manager for plugin modules
//

class PPluginModuleManager : public PObject
{
  public:
    typedef PDictionary<PString, PDynaLink> PluginListType;

    PPluginModuleManager(const char * signatureFunctionName, PPluginManager * pluginMgr = NULL);

    virtual void OnLoadPlugin(PDynaLink & /*dll*/, P_INT_PTR /*code*/)
    { }

    virtual PluginListType GetPluginList() const
    { return pluginDLLs; }

    virtual void OnStartup()
    { }
    virtual void OnShutdown()
    { }

  protected:
    PluginListType pluginDLLs;
    PDECLARE_NOTIFIER(PDynaLink, PPluginModuleManager, OnLoadModule);

  protected:
    const char * signatureFunctionName;
    PPluginManager * pluginMgr;
};


#define PLUGIN_LOADER_STARTUP_NAME "PluginLoaderStartup"

PFACTORY_LOAD(PluginLoaderStartup);


#endif // PTLIB_PLUGINMGR_H


// End Of File ///////////////////////////////////////////////////////////////
