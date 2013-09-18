/*
 * pluginmgr.cxx
 *
 * Plugin Manager Class
 *
 * Search path for PLugin is defined in this class, using the following 
 * define variables: P_DEFAULT_PLUGIN_DIR
 *
 * Configure setup in ptlib_config.h.in:
 *   #define P_DEFAULT_PLUGIN_DIR "/usr/local/lib/ptlib-2.13.0"
 * if not defined pluginmgr.cxx set it to:
 *   #define P_DEFAULT_PLUGIN_DIR ".:/usr/lib/ptlib:/usr/lib/pwlib"
 * or windows:
 *   #define P_DEFAULT_PLUGIN_DIR ".;C:\\Program Files\\PTLib Plug Ins;C:\\PTLIB_PLUGINS;C:\\PWLIB_PLUGINS"
 *
 * Also the following Environment variables:  "PTLIBPLUGINDIR" and "PWLIBPLUGINDIR"
 * It check 1st if PTLIBPLUGINDIR is defined, 
 * if not then check PWLIBPLUGINDIR is defined. 
 * If not use P_DEFAULT_PLUGIN_DIR (hard coded as DEFINE).
 *
 * Plugin must have suffixes:
 * #define PTPLUGIN_SUFFIX       "_ptplugin"
 * #define PWPLUGIN_SUFFIX       "_pwplugin"
 * 
 * Debugging plugin issue, set Trace level based on the following:
 * PTRACE
 * level 2 = list plugin that are not compatible (old version, not a PWLIB plugin etc).
 * level 4 = list directories.
 * level 5 = list plugin before checking suffix .
 *  * Portable Windows Library
 *
 * Contributor(s): Snark at GnomeMeeting
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/pluginmgr.h>

#ifndef __BEOS__
#include <algorithm>
#endif

#ifndef P_DEFAULT_PLUGIN_DIR
#  if defined (_WIN32_WCE)
#    define P_DEFAULT_PLUGIN_DIR "\\Program Files\\PTLib Plug Ins"
#  elif defined (_WIN32)
#    define P_DEFAULT_PLUGIN_DIR ".;C:\\Program Files\\PTLib Plug Ins;C:\\PTLIB_PLUGINS;C:\\PWLIB_PLUGINS"
#  elif defined (P_ANDROID)
#    define P_DEFAULT_PLUGIN_DIR ""
#  else
#    define P_DEFAULT_PLUGIN_DIR ".:/usr/lib/ptlib:/usr/lib/pwlib"
#  endif
#endif

#ifdef  _WIN32
#define PATH_SEP   ';'
#else
#define PATH_SEP   ':'
#endif

#ifndef PDIR_SEPARATOR 
#ifdef _WIN32
#define PDIR_SEPARATOR '\\'
#else
#define PDIR_SEPARATOR '/'
#endif
#endif

#define ENV_PTLIB_PLUGIN_DIR  "PTLIBPLUGINDIR"
#define ENV_PWLIB_PLUGIN_DIR  "PWLIBPLUGINDIR"

#define PTPLUGIN_SUFFIX       "_ptplugin"
#define PWPLUGIN_SUFFIX       "_pwplugin"

const char PDevicePluginServiceDescriptor::SeparatorChar = '\t';


class PluginLoaderStartup : public PProcessStartup
{
  PCLASSINFO(PluginLoaderStartup, PProcessStartup);
  public:
    void OnStartup();
    void OnShutdown();
};


#define new PNEW


//////////////////////////////////////////////////////

PPluginManager::PPluginManager()
{
  PString env = ::getenv(ENV_PTLIB_PLUGIN_DIR);
  if (env.IsEmpty())
    env = ::getenv(ENV_PWLIB_PLUGIN_DIR);
  if (env.IsEmpty())
    env = P_DEFAULT_PLUGIN_DIR;
  SetDirectories(env);

#ifdef _WIN32
  TCHAR moduleName[_MAX_PATH];
  if (GetModuleFileName(GetModuleHandle(NULL), moduleName, sizeof(moduleName)) > 0)
    AddDirectory(PFilePath(moduleName).GetDirectory());
#endif // _WIN32

  m_suffixes.AppendString(PTPLUGIN_SUFFIX);
  m_suffixes.AppendString(PWPLUGIN_SUFFIX);
}


PPluginManager & PPluginManager::GetPluginManager()
{
  static PPluginManager systemPluginMgr;
  return systemPluginMgr;
}


void PPluginManager::SetDirectories(const PString & dirs)
{
  SetDirectories(dirs.Tokenise(PATH_SEP, false)); // split into directories on correct seperator
}


void PPluginManager::SetDirectories(const PStringArray & dirs)
{
  m_directories.RemoveAll();

  for (PINDEX i = 0; i < dirs.GetSize(); ++i)
    AddDirectory(dirs[i]);
}


void PPluginManager::AddDirectory(const PDirectory & dir)
{
  if (m_directories.GetValuesIndex(dir) == P_MAX_INDEX)
    m_directories.Append(new PDirectory(dir));
}


void PPluginManager::LoadDirectories()
{
  PTRACE(4, "PLUGIN\tEnumerating plugin directories " << setfill(PATH_SEP) << m_directories);
  for (PList<PDirectory>::iterator it = m_directories.begin(); it != m_directories.end(); ++it)
    LoadDirectory(*it);
}


void PPluginManager::LoadDirectory(const PDirectory & directory)
{
  PDirectory dir = directory;
  if (!dir.Open()) {
    PTRACE(4, "PLUGIN\tCannot open plugin directory " << dir);
    return;
  }
  PTRACE(4, "PLUGIN\tEnumerating plugin directory " << dir);
  do {
    PString entry = dir + dir.GetEntryName();
    PDirectory subdir = entry;
    if (subdir.Open())
      LoadDirectory(entry);
    else {
      PFilePath fn(entry);
      for (PStringList::iterator it = m_suffixes.begin(); it != m_suffixes.end(); ++it) {
        PString suffix = *it;
        PTRACE(5, "PLUGIN\tChecking " << fn << " against suffix " << suffix);
        if ((fn.GetType() *= PDynaLink::GetExtension()) && (fn.GetTitle().Right(strlen(suffix)) *= suffix)) 
          LoadPlugin(entry);
      }
    }
  } while (dir.Next());
}


PBoolean PPluginManager::LoadPlugin(const PString & fileName)
{
  PDynaLink *dll = new PDynaLink(fileName);
  if (!dll->IsLoaded()) {
    PTRACE(4, "PLUGIN\tFailed to open " << fileName << " error: " << dll->GetLastError());
  }

  else {
    PDynaLink::Function fn;
    if (!dll->GetFunction("PWLibPlugin_GetAPIVersion", fn))
      PTRACE(2, "PLUGIN\t" << fileName << " is not a PWLib plugin");

    else {
      unsigned (*GetAPIVersion)() = (unsigned (*)())fn;
      int version = (*GetAPIVersion)();
      switch (version) {
        case 0 : // old-style service plugins, and old-style codec plugins
          {
            // call the register function (if present)
            if (!dll->GetFunction("PWLibPlugin_TriggerRegister", fn)) 
              PTRACE(2, "PLUGIN\t" << fileName << " has no registration-trigger function");
            else {
              void (*triggerRegister)(PPluginManager *) = (void (*)(PPluginManager *))fn;
              (*triggerRegister)(this);
            }
          }
          // fall through to new version

        case 1 : // factory style plugins
          // add the plugin to the list of plugins
          m_pluginsMutex.Wait();
          m_plugins.Append(dll);
          m_pluginsMutex.Signal();

          // call the notifier
          CallNotifier(*dll, LoadingPlugIn);
          return true;

        default:
          PTRACE(2, "PLUGIN\t" << fileName << " uses version " << version << " of the PWLIB PLUGIN API, which is not supported");
          break;
      }
    }
  }

  // loading the plugin failed - return error
  dll->Close();
  delete dll;

  return false;
}


PStringArray PPluginManager::GetPluginTypes() const
{
  PWaitAndSignal mutex(m_servicesMutex);

  PStringArray result;
  for (PINDEX i = 0; i < m_services.GetSize(); i++) {
    PString serviceType = m_services[i].serviceType;
    if (result.GetStringsIndex(serviceType) == P_MAX_INDEX)
      result.AppendString(serviceType);
  }
  return result;
}


PStringArray PPluginManager::GetPluginsProviding(const PString & serviceType) const
{
  PWaitAndSignal mutex(m_servicesMutex);

  PStringArray result;
  for (PINDEX i = 0; i < m_services.GetSize(); i++) {
    if (m_services[i].serviceType *= serviceType)
      result.AppendString(m_services[i].serviceName);
  }
  return result;
}


PPluginServiceDescriptor * PPluginManager::GetServiceDescriptor(const PString & serviceName,
                                                                const PString & serviceType) const
{
  PWaitAndSignal mutex(m_servicesMutex);

  for (PINDEX i = 0; i < m_services.GetSize(); i++) {
    if ((m_services[i].serviceName *= serviceName) &&
        (m_services[i].serviceType *= serviceType))
      return m_services[i].descriptor;
  }
  return NULL;
}


PObject * PPluginManager::CreatePluginsDevice(const PString & serviceName,
                                              const PString & serviceType,
                                              int userData) const
{
  PDevicePluginServiceDescriptor * descr = (PDevicePluginServiceDescriptor *)GetServiceDescriptor(serviceName, serviceType);
  if (descr != NULL)
    return descr->CreateInstance(userData);

  return NULL;
}


PObject * PPluginManager::CreatePluginsDeviceByName(const PString & deviceName,
                                                    const PString & serviceType,
                                                    int userData,
                                                    const PString & serviceName) const
{
  // If have tab character, then have explicit driver name in device
  PINDEX tab = deviceName.Find(PDevicePluginServiceDescriptor::SeparatorChar);
  if (tab != P_MAX_INDEX)
    return CreatePluginsDevice(deviceName.Left(tab), serviceType, userData);

  PWaitAndSignal mutex(m_servicesMutex);

  // If we know the service name of the device we want to create.
  if (!serviceName) {
    PDevicePluginServiceDescriptor * desc = (PDevicePluginServiceDescriptor *)GetServiceDescriptor(serviceName, serviceType);
    if (desc != NULL && desc->ValidateDeviceName(deviceName, userData))
      return desc->CreateInstance(userData);
  }

  for (PINDEX i = 0; i < m_services.GetSize(); i++) {
    const PPluginService & service = m_services[i];
    if (service.serviceType *= serviceType) {
      PDevicePluginServiceDescriptor * descriptor = (PDevicePluginServiceDescriptor *)service.descriptor;
      if (PAssertNULL(descriptor) != NULL && descriptor->ValidateDeviceName(deviceName, userData))
        return descriptor->CreateInstance(userData);
    }
  }

  return NULL;
}


bool PDevicePluginServiceDescriptor::ValidateDeviceName(const PString & deviceName, P_INT_PTR userData) const
{
  PStringArray devices = GetDeviceNames(userData);
  if (
      (deviceName.GetLength() == 2) && 
      (deviceName[0] == '#') && 
       isdigit(deviceName[1]) && 
       ((deviceName[1]-'0') < devices.GetSize())
      )
    return true;
      
  for (PINDEX j = 0; j < devices.GetSize(); j++) {
    if (devices[j] *= deviceName)
      return true;
  }

  return false;
}


bool PDevicePluginServiceDescriptor::GetDeviceCapabilities(const PString & /*deviceName*/,
                                                           void * /*capabilities*/) const
{
  return false;
}


PStringArray PPluginManager::GetPluginsDeviceNames(const PString & serviceName,
                                                   const PString & serviceType,
                                                   int userData) const
{
  PStringArray allDevices;

  if (serviceName.IsEmpty() || serviceName == "*") {
    PWaitAndSignal mutex(m_servicesMutex);

    PINDEX i;
    PStringToString deviceToPluginMap;  

    // First we run through all of the drivers and their lists of devices and
    // use the dictionary to assure all names are unique
    for (i = 0; i < m_services.GetSize(); i++) {
      const PPluginService & service = m_services[i];
      if (service.serviceType *= serviceType) {
        PStringArray devices = ((PDevicePluginServiceDescriptor *)service.descriptor)->GetDeviceNames(userData);
        for (PINDEX j = 0; j < devices.GetSize(); j++) {
          PCaselessString device = devices[j];
          if (deviceToPluginMap.Contains(device)) {
            PString oldPlugin = deviceToPluginMap[device];
            if (!oldPlugin.IsEmpty()) {
              // Make name unique by prepending driver name and a tab character
              deviceToPluginMap.SetAt(oldPlugin+PDevicePluginServiceDescriptor::SeparatorChar+device, service.serviceName);
              // Reset the original to empty string so we dont add it multiple times
              deviceToPluginMap.SetAt(device, "");
            }
            // Now add the new one
            deviceToPluginMap.SetAt(service.serviceName+PDevicePluginServiceDescriptor::SeparatorChar+device, service.serviceName);
          }
          else
            deviceToPluginMap.SetAt(device, service.serviceName);
        }
      }
    }

    for (PStringToString::iterator it = deviceToPluginMap.begin(); it != deviceToPluginMap.end(); ++it) {
      if (!it->second.IsEmpty())
        allDevices.AppendString(it->first);
    }
  }
  else {
    PDevicePluginServiceDescriptor * descr = (PDevicePluginServiceDescriptor *)GetServiceDescriptor(serviceName, serviceType);
    if (descr != NULL)
      allDevices = descr->GetDeviceNames(userData);
  }

  return allDevices;
}


PBoolean PPluginManager::GetPluginsDeviceCapabilities(const PString & serviceType,
                                                      const PString & serviceName,
                                                      const PString & deviceName,
                                                      void * capabilities) const
{
  if (serviceType.IsEmpty() || deviceName.IsEmpty()) 
    return false;

  if (serviceName.IsEmpty() || serviceName == "*") {
    PWaitAndSignal mutex(m_servicesMutex);
    for (PINDEX i = 0; i < m_services.GetSize(); i++) {
      const PPluginService & service = m_services[i];
      if (service.serviceType *= serviceType) { 
        PDevicePluginServiceDescriptor * desc = (PDevicePluginServiceDescriptor *)service.descriptor;
        if (desc != NULL && desc->ValidateDeviceName(deviceName, 0))
          return desc->GetDeviceCapabilities(deviceName,capabilities);
      }
    }
  }
  else {
    PDevicePluginServiceDescriptor * desc = (PDevicePluginServiceDescriptor *)GetServiceDescriptor(serviceName, serviceType);
    if (desc != NULL && desc->ValidateDeviceName(deviceName, 0))
      return desc->GetDeviceCapabilities(deviceName,capabilities);
  }

  return false;
}


PBoolean PPluginManager::RegisterService(const PString & serviceName,
                                         const PString & serviceType,
                                         PPluginServiceDescriptor * descriptor)
{
  PWaitAndSignal mutex(m_servicesMutex);

  // first, check if it something didn't already register that name and type
  for (PINDEX i = 0; i < m_services.GetSize(); i++) {
    if (m_services[i].serviceName == serviceName &&
        m_services[i].serviceType == serviceType)
      return false;
  }  

  PPluginService * service = new PPluginService(serviceName, serviceType, descriptor);
  m_services.Append(service);

  PDevicePluginAdapterBase * adapter = PFactory<PDevicePluginAdapterBase>::CreateInstance(serviceType);
  if (adapter != NULL)
    adapter->CreateFactory(serviceName);

  return true;
}

void PPluginManager::OnShutdown()
{
  PWaitAndSignal mutex(m_pluginsMutex);

  for (PINDEX i = 0; i < m_plugins.GetSize(); i++)
    CallNotifier(m_plugins[i], UnloadingPlugIn);

  m_notifiersMutex.Wait();
  m_notifiers.RemoveAll();
  m_notifiersMutex.Signal();
  
  m_plugins.RemoveAll();
}


void PPluginManager::AddNotifier(const PNotifier & notifyFunction, PBoolean existing)
{
  m_notifiersMutex.Wait();
  m_notifiers.Append(new PNotifier(notifyFunction));
  m_notifiersMutex.Signal();

  if (existing) {
    PWaitAndSignal mutex(m_pluginsMutex);
    for (PINDEX i = 0; i < m_plugins.GetSize(); i++) 
      CallNotifier(m_plugins[i], LoadingPlugIn);
  }
}


void PPluginManager::RemoveNotifier(const PNotifier & notifyFunction)
{
  PWaitAndSignal mutex(m_notifiersMutex);
  PList<PNotifier>::iterator it = m_notifiers.begin();
  while (it != m_notifiers.end()) {
    if (*it != notifyFunction)
      ++it;
    else
      m_notifiers.erase(it++);
  }
}


void PPluginManager::CallNotifier(PDynaLink & dll, NotificationCode code)
{
  PWaitAndSignal mutex(m_notifiersMutex);
  for (PList<PNotifier>::iterator it = m_notifiers.begin(); it != m_notifiers.end(); ++it)
    (*it)(dll, code);
}


////////////////////////////////////////////////////////////////////////////////////

PPluginModuleManager::PPluginModuleManager(const char * _signatureFunctionName, PPluginManager * _pluginMgr)
  : signatureFunctionName(_signatureFunctionName)
{
  pluginDLLs.DisallowDeleteObjects();
  pluginMgr = _pluginMgr;;
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();
}

void PPluginModuleManager::OnLoadModule(PDynaLink & dll, P_INT_PTR code)
{
  PDynaLink::Function dummyFunction;
  if (!dll.GetFunction(signatureFunctionName, dummyFunction))
    return;

  switch (code) {
    case 0:
      pluginDLLs.SetAt(dll.GetName(), &dll); 
      break;

    case 1: 
      pluginDLLs.RemoveAt(dll.GetName());
      break;

    default:
      break;
  }

  OnLoadPlugin(dll, code);
}


////////////////////////////////////////////////////////////////////////////////////

void PluginLoaderStartup::OnStartup()
{ 
  // load the actual DLLs, which will also load the system plugins
  PPluginManager::GetPluginManager().LoadDirectories();

  // load the plugin module managers
  PFactory<PPluginModuleManager>::KeyList_T keyList = PFactory<PPluginModuleManager>::GetKeyList();
  PFactory<PPluginModuleManager>::KeyList_T::const_iterator it;
  for (it = keyList.begin(); it != keyList.end(); ++it)
    PFactory<PPluginModuleManager>::CreateInstance(*it)->OnStartup();
}


void PluginLoaderStartup::OnShutdown()
{
  PPluginManager::GetPluginManager().OnShutdown();

  // load the plugin module managers, and construct the list of suffixes
  PFactory<PPluginModuleManager>::KeyList_T keyList = PFactory<PPluginModuleManager>::GetKeyList();
  PFactory<PPluginModuleManager>::KeyList_T::const_iterator it;
  for (it = keyList.begin(); it != keyList.end(); ++it)
    PFactory<PPluginModuleManager>::CreateInstance(*it)->OnShutdown();
}

PFACTORY_CREATE(PProcessStartupFactory, PluginLoaderStartup, PLUGIN_LOADER_STARTUP_NAME, true);

