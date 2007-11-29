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

#ifndef _PLUGINMGR_H
#define _PLUGINMGR_H

#define DEFAULT_PLUGINDIR "/usr/lib/pwlib"

#include <ptlib/plugin.h>

class PPluginSuffix {
  private:
    int dummy;
};

template <class C>
void PLoadPluginDirectory(C & obj, const PDirectory & directory, const char * suffix = NULL)
{
  PDirectory dir = directory;
  if (!dir.Open()) {
    PTRACE(4, "Cannot open plugin directory " << dir);
    return;
  }
  PTRACE(4, "Enumerating plugin directory " << dir);
  do {
    PString entry = dir + dir.GetEntryName();
    PDirectory subdir = entry;
    if (subdir.Open())
      PLoadPluginDirectory<C>(obj, entry, suffix);
    else {
      PFilePath fn(entry);
      if (
           (fn.GetType() *= PDynaLink::GetExtension()) &&
           (
             (suffix == NULL) || (fn.GetTitle().Right(strlen(suffix)) *= suffix)
           )
         )
        obj.LoadPlugin(entry);
    }
  } while (dir.Next());
}

//////////////////////////////////////////////////////
//
//  Manager for plugins
//

class PPluginManager : public PObject
{
  PCLASSINFO(PPluginManager, PObject);

  public:
    // functions to load/unload a dynamic plugin 
    PBoolean LoadPlugin (const PString & fileName);
    void LoadPluginDirectory (const PDirectory & dir);
  
    // functions to access the plugins' services 
    PStringList GetPluginTypes() const;
    PStringList GetPluginsProviding(const PString & serviceType) const;
    PPluginServiceDescriptor * GetServiceDescriptor(const PString & serviceName, const PString & serviceType) const;
    PObject * CreatePluginsDevice(const PString & serviceName, const PString & serviceType, int userData = 0) const;
    PObject * CreatePluginsDeviceByName(const PString & deviceName, const PString & serviceType, int userData = 0, const PString & serviceName = PString::Empty()) const;
    PStringList GetPluginsDeviceNames(const PString & serviceName, const PString & serviceType, int userData = 0) const;
    PBoolean GetPluginsDeviceCapabilities(const PString & serviceType,const PString & serviceName,const PString & deviceName,void * capabilities) const;

    // function to register a service (used by the plugins themselves)
    PBoolean RegisterService (const PString & serviceName, const PString & serviceType, PPluginServiceDescriptor * descriptor);

    // Get the list of plugin directories
    static PStringArray GetPluginDirs();

    // static functions for accessing global instances of plugin managers
    static PPluginManager & GetPluginManager();

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
      PBoolean existing = PFalse
    );

    void RemoveNotifier(
      const PNotifier & filterFunction
    );

  protected:
    void LoadPluginDirectory (const PDirectory & directory, const PStringList & suffixes);
    void CallNotifier(PDynaLink & dll, INT code);

    PMutex pluginListMutex;
    PList<PDynaLink> pluginList;
    
    PMutex serviceListMutex;
    PList<PPluginService> serviceList;

    PMutex notifierMutex;
    PList<PNotifier> notifierList;
};

//////////////////////////////////////////////////////
//
//  Manager for plugin modules
//

class PPluginModuleManager : public PObject
{
  public:
    typedef PDictionary<PString, PDynaLink> PluginListType;

    PPluginModuleManager(const char * _signatureFunctionName, PPluginManager * pluginMgr = NULL);

    PBoolean LoadPlugin(const PString & fileName)
    { if (pluginMgr == NULL) return PFalse; else return pluginMgr->LoadPlugin(fileName); }

    void LoadPluginDirectory(const PDirectory &directory)
    { if (pluginMgr != NULL) pluginMgr->LoadPluginDirectory(directory); }

    virtual void OnLoadPlugin(PDynaLink & /*dll*/, INT /*code*/)
    { }

    virtual PluginListType GetPluginList() const
    { return pluginList; }

    virtual void OnShutdown()
    { }

  protected:
    PluginListType pluginList;
    PDECLARE_NOTIFIER(PDynaLink, PPluginModuleManager, OnLoadModule);

  protected:
    const char * signatureFunctionName;
    PPluginManager * pluginMgr;
};

#endif // ifndef _PLUGINMGR_H
