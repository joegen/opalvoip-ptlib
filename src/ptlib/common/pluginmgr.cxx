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
 * Revision 1.10  2004/03/23 04:43:42  csoutheren
 * Modified plugin manager to allow code modules to be notified when plugins
 * are loaded or unloaded
 *
 * Revision 1.9  2004/02/23 23:56:01  csoutheren
 * Removed unneeded class
 *
 * Revision 1.8  2004/01/18 21:00:15  dsandras
 * Fixed previous commit thanks to Craig Southeren!
 *
 * Revision 1.7  2004/01/17 17:40:57  csoutheren
 * Changed to only attempt loading of files with the correct library file extension
 * Changed to handle plugins without a register function
 *
 * Revision 1.6  2004/01/17 16:02:59  dereksmithies
 * make test for plugin names case insensitive.
 *
 * Revision 1.5  2003/11/18 10:39:56  csoutheren
 * Changed PTRACE levels to give better output at trace level 3
 *
 * Revision 1.4  2003/11/12 10:27:11  csoutheren
 * Changes to allow operation of static plugins under Windows
 *
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

//////////////////////////////////////////////////////

PPluginManager & PPluginManager::GetPluginManager()
{
  static PMutex mutex;
  static PPluginManager * systemPluginMgr = NULL;

  PWaitAndSignal m(mutex);

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

BOOL PPluginManager::LoadPlugin(const PString & fileName)
{
  PWaitAndSignal m(pluginListMutex);

  PDynaLink *dll = new PDynaLink(fileName);
  if (!dll->IsLoaded()) {
    PTRACE(4, "Failed to open " << fileName);
  }

  else {
    unsigned (*GetAPIVersion)();
    if(!dll->GetFunction("PWLibPlugin_GetAPIVersion", (PDynaLink::Function &)GetAPIVersion)) {
      PTRACE(3, "Failed to recognize a plugin in " << fileName);
    }

    else {
      if ((*GetAPIVersion)() != PWLIB_PLUGIN_API_VERSION) {
        PTRACE(3, fileName << " is a plugin, but the version mismatch");
      }

      else {

        // declare local pointer to register function
        void (*triggerRegister)(PPluginManager *);

        // call the register function (if present)
        if (dll->GetFunction("PWLibPlugin_TriggerRegister", (PDynaLink::Function &)triggerRegister)) 
          (*triggerRegister)(this);
        else {
          PTRACE(3, "Failed to find the registration-triggering function in " << fileName);
        }

        // call the notifier
        CallNotifier(*dll, 0);

        // add the plugin to the list of plugins
        pluginList.Append(dll);
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
    PTRACE(4, "Cannot open plugin directory " << dir);
    return;
  }
 
  PTRACE(4, "Enumerating plugin directory " << dir);

  do {
    PString entry = dir + dir.GetEntryName();
    if (dir.IsSubDir())
      LoadPluginDirectory(entry);
    else if (PFilePath(entry).GetType() *= PDynaLink::GetExtension()) 
      LoadPlugin(entry);
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
    if (serviceList[i].serviceType *= serviceType)
      result.AppendString(serviceList[i].serviceName);
  }
  return result;
}

PPluginServiceDescriptor * PPluginManager::GetServiceDescriptor (const PString & serviceName,
					                         const PString & serviceType)
{
  PWaitAndSignal n(serviceListMutex);

  for (PINDEX i = 0; i < serviceList.GetSize(); i++) {
    if ((serviceList[i].serviceName *= serviceName) &&
        (serviceList[i].serviceType *= serviceType))
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


void PPluginManager::AddNotifier(const PNotifier & notifyFunction, BOOL existing)
{
  PWaitAndSignal m(notifierMutex);
  notifierList.Append(new PNotifier(notifyFunction));

  if (existing)
    for (PINDEX i = 0; i < pluginList.GetSize(); i++) 
      CallNotifier(pluginList[i], 0);
}

void PPluginManager::RemoveNotifier(const PNotifier & notifyFunction)
{
  PWaitAndSignal m(notifierMutex);
  for (PINDEX i = 0; i < notifierList.GetSize(); i++) {
    if (notifierList[i] == notifyFunction) {
      notifierList.RemoveAt(i);
      i = 0;
      continue;
    }
  }
}

void PPluginManager::CallNotifier(PDynaLink & dll, INT code)
{
  PWaitAndSignal m(notifierMutex);
  for (PINDEX i = 0; i < notifierList.GetSize(); i++)
    notifierList[i](dll, code);
}
