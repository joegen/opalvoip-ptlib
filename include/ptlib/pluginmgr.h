/*
 * pluginmgr.h
 *
 * Plugin Manager Class Declarations
 *
 * Portable Windows Library
 *
 * Contributor(s): Snark at GnomeMeeting
 *
 * $Log: pluginmgr.h,v $
 * Revision 1.7  2004/04/14 10:57:38  csoutheren
 * Removed multiple definition of statc function in generic plugin functions
 *
 * Revision 1.6  2004/04/14 10:01:54  csoutheren
 * Fixed compile problem on Windows
 *
 * Revision 1.5  2004/04/14 08:12:02  csoutheren
 * Added support for generic plugin managers
 *
 * Revision 1.4  2004/03/23 04:43:42  csoutheren
 * Modified plugin manager to allow code modules to be notified when plugins
 * are loaded or unloaded
 *
 * Revision 1.3  2003/11/12 10:24:35  csoutheren
 * Changes to allow operation of static plugins under Windows
 *
 * Revision 1.2  2003/11/12 03:26:17  csoutheren
 * Initial version of plugin code from Snark of GnomeMeeting with changes
 *    by Craig Southeren os Post Increment
 *
 *
 */

#ifndef _PLUGINMGR_H
#define _PLUGINMGR_H

#define DEFAULT_PLUGINDIR "/usr/lib/pwlib"

#include <ptlib/plugin.h>

//////////////////////////////////////////////////////
//
//  Manager for plugins
//

#define PPluginDynamic  PDynaLink

class PPluginManager : public PObject
{
  PCLASSINFO(PPluginManager, PObject);

  public:
    PPluginManager (); 
    ~PPluginManager ();

    // functions to load/unload a dynamic plugin 
    BOOL LoadPlugin (const PString & fileName);
    void LoadPluginDirectory (const PDirectory &directory);

    // functions to access the plugins' services 
    PStringList GetPluginTypes() const;
    PStringList GetPluginsProviding (const PString & serviceType) const;
    PPluginServiceDescriptor * GetServiceDescriptor (const PString & serviceName, const PString & serviceType);

    // function to register a service (used by the plugins themselves)
    BOOL RegisterService (const PString & serviceName, const PString & serviceType, PPluginServiceDescriptor * descriptor);

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
      BOOL existing = FALSE
    );

    void RemoveNotifier(
      const PNotifier & filterFunction
    );

  protected:
    void CallNotifier(PDynaLink & dll, INT code);

    PMutex pluginListMutex;
    PList<PPluginDynamic> pluginList;
    
    PMutex serviceListMutex;
    PList<PPluginService> serviceList;

    PMutex notifierMutex;
    PList<PNotifier> notifierList;
};


// helper classes to make sure plugin manager gets declared
#define PWLIB_PLUGIN_MODULE_LOADER(name) \
class name##_PluginLoader; \
extern name##_PluginLoader name##_PluginLoader_Instance; \
static name##_PluginLoader * name##_PluginLoader_Static = &name##_PluginLoader_Instance; \

#define PWLIB_PLUGIN_MODULE_LOADER_IMPLEMENT(name, mgrclass) \
class name##_PluginLoader  { public: name##_PluginLoader(); } name##_PluginLoader_Instance; \
name##_PluginLoader::name##_PluginLoader() { mgrclass::GetManager(); } \
mgr & mgr::GetManager()\
{\
  static PMutex mutex; \
  static mgr * systemMgr = NULL;\
  PWaitAndSignal m(mutex);\
  if (systemMgr == NULL)\
    systemMgr = new mgr;\
  return *systemMgr;\
}\

class PPluginModuleManager : public PObject
{
  public:
    typedef PDictionary<PString, PDynaLink> PluginListType;

    PPluginModuleManager(const char * _signatureFunctionName, PPluginManager * pluginMgr = NULL);

    BOOL LoadPlugin(const PString & fileName)
    { if (pluginMgr == NULL) return FALSE; else return pluginMgr->LoadPlugin(fileName); }

    void LoadPluginDirectory(const PDirectory &directory)
    { if (pluginMgr != NULL) pluginMgr->LoadPluginDirectory(directory); }

    virtual void OnLoadPlugin(PDynaLink & dll, INT code) = 0;

    virtual PluginListType GetPluginList() const
    { return pluginList; }

  protected:
    PluginListType pluginList;
    PDECLARE_NOTIFIER(PDynaLink, PPluginModuleManager, OnLoadModule);

  protected:
    const char * signatureFunctionName;
    PPluginManager * pluginMgr;
};

#define PWLIB_PLUGIN_MANAGER_CLASS(mgr) \
class mgr : public PPluginModuleManager\
{\
  public: \
    static mgr & GetManager();\

#endif // ifndef _PLUGINMGR_H
