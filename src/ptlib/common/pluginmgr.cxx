/*
 * pluginmgr.cxx
 *
 * Plugin Manager Class
 *
 * Portable Windows Library
 *
 * Contributor(s): Snark at GnomeMeeting
 *
 * $Log: pluginmgr.cxx,v $
 * Revision 1.3  2003/11/12 06:58:59  csoutheren
 * Added default plugin directory for Windows
 *
 * Revision 1.2  2003/11/12 03:27:25  csoutheren
 * Initial version of plugin code from Snark of GnomeMeeting with changes
 *    by Craig Southeren of Post Increment
 *
 *
 */

#include <ptlib.h>
#include <ptlib/pluginmgr.h>

#ifndef	P_DEFAULT_PLUGIN_DIR
#  ifdef  _WIN32
#    define	P_DEFAULT_PLUGIN_DIR "C:\\PWLIB_PLUGINS"
#  else
#    define	P_DEFAULT_PLUGIN_DIR "/usr/lib/pwlib"
#  endif
#endif

// static vars to help bootstrap the default plugin instance
PPluginManager * PPluginManager::systemPluginMgr;
PMutex           PPluginManager::systemPluginMgrInitMutex;

//////////////////////////////////////////////////////
//
// helper class to ensure DLLs are cleaned up correctly
// when in a list
//

class PPluginDynamic : public PObject
{
  public:
    PPluginDynamic (PDynaLink * _dll)
    { dll = _dll; }

    ~PPluginDynamic()
    {
      dll->Close ();
      delete dll;
      dll = NULL;
    }

    PDynaLink *dll;
};

//////////////////////////////////////////////////////

PPluginManager & PPluginManager::GetPluginManager()
{
  PWaitAndSignal m(systemPluginMgrInitMutex);

  if (systemPluginMgr == NULL) {
    systemPluginMgr = new PPluginManager;
    systemPluginMgr->LoadPluginDirectory(P_DEFAULT_PLUGIN_DIR);
  }

  return *systemPluginMgr;
}

//////////////////////////////////////////////////////

PPluginManager::PPluginManager()
{
}

PPluginManager::~PPluginManager()
{
}

BOOL PPluginManager::LoadPlugin (const PString & fileName)
{
  PWaitAndSignal m(pluginListMutex);

  PDynaLink *dll = new PDynaLink(fileName);
  if (!dll->IsLoaded()) {
    PTRACE(4, "Failed to open " << fileName);
  }

  else {
    unsigned (*GetAPIVersion)();
    if(!dll->GetFunction("PWLibPlugin_GetAPIVersion", (PDynaLink::Function &)GetAPIVersion)) {
      PTRACE(4, "Failed to recognize a plugin in " << fileName);
    }

    else {
      if ((*GetAPIVersion)() != PWLIB_PLUGIN_API_VERSION) {
        PTRACE(4, fileName << " is a plugin, but the version mismatch");
      }

      else {

        // declare local pointer to register function
        void (*triggerRegister)(PPluginManager *);

        if (!dll->GetFunction("PWLibPlugin_TriggerRegister", (PDynaLink::Function &)triggerRegister)) {
          PTRACE(4, "Failed to find the registration-triggering function in " << fileName);
        }

        // call the register function and add the plugin to the list of plugins
        (*triggerRegister)(this);
        pluginList.Append(new PPluginDynamic(dll));
        return TRUE;
      }
    }
  }

  // loading the plugin failed - return error
  dll->Close();
  delete dll;

  return FALSE;
}


void PPluginManager::LoadPluginDirectory (const PDirectory & directory)
{
  PDirectory dir = directory;
 
  if (!dir.Open()) {
    PTRACE(2, "Cannot open plugin directory " << dir);
    return;
  }
 
  PTRACE(4, "Enumerating plugin directory " << dir);

  do {
    PString entry = dir + dir.GetEntryName();
    if (dir.IsSubDir())
      LoadPluginDirectory(entry);
    else
      (void)LoadPlugin(entry);
  } while (dir.Next());
}

  
PStringList PPluginManager::GetPluginTypes() const
{
  PWaitAndSignal n(serviceListMutex);

  PStringList result;
  for (PINDEX i = 0; i < serviceList.GetSize(); i++) {
    PString serviceType = serviceList[i].serviceType;
    if (result.GetStringsIndex(serviceType) == P_MAX_INDEX)
      result.AppendString(serviceList[i].serviceType);
  }
  return result;
}

PStringList PPluginManager::GetPluginsProviding(const PString & serviceType) const
{
  PWaitAndSignal n(serviceListMutex);

  PStringList result;
  for (PINDEX i = 0; i < serviceList.GetSize(); i++) {
    if (serviceList[i].serviceType == serviceType)
      result.AppendString(serviceList[i].serviceName);
  }
  return result;
}

PPluginServiceDescriptor * PPluginManager::GetServiceDescriptor (const PString & serviceName,
					                         const PString & serviceType)
{
  PWaitAndSignal n(serviceListMutex);

  for (PINDEX i = 0; i < serviceList.GetSize(); i++) {
    if (serviceList[i].serviceName == serviceName &&
        serviceList[i].serviceType == serviceType)
      return serviceList[i].descriptor;
  }
  return NULL;
}


BOOL PPluginManager::RegisterService(const PString & serviceName,
				     const PString & serviceType,
				     PPluginServiceDescriptor * descriptor)
{
  PWaitAndSignal m(serviceListMutex);

  // first, check if it something didn't already register that name and type
  for (PINDEX i = 0; i < serviceList.GetSize(); i++) {
    if (serviceList[i].serviceName == serviceName &&
        serviceList[i].serviceType == serviceType)
      return FALSE;
  }  

  PPluginService * service = new PPluginService(serviceName, serviceType, descriptor);
  serviceList.Append(service);

  return TRUE;
}
