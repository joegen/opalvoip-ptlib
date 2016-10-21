/*
 * pluginmgr.cxx
 *
 * Plugin Manager Class
 *
 * Search path for PLugin is defined in this class, using the following 
 * define variables: P_DEFAULT_PLUGIN_DIR
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
#include <ptlib/pluginmgr.h>

#if P_PLUGINMGR

#include <ptlib/pprocess.h>
#include <algorithm>


#ifndef P_DEFAULT_PLUGIN_DIR
#  if defined (_WIN32_WCE)
#    define P_DEFAULT_PLUGIN_DIR "\\Program Files\\PTLib Plug Ins"
#  elif defined (_WIN32)
#    if _WIN64
#      define P_DEFAULT_PLUGIN_DIR ".;C:\\Program Files\\PTLib Plug Ins;C:\\PTLIB_PLUGINS;C:\\PWLIB_PLUGINS"
#    else
#      define P_DEFAULT_PLUGIN_DIR ".;C:\\Program Files (x86)\\PTLib Plug Ins;C:\\Program Files\\PTLib Plug Ins;C:\\PTLIB_PLUGINS;C:\\PWLIB_PLUGINS"
#    endif
#  elif defined (P_ANDROID)
#    define P_DEFAULT_PLUGIN_DIR ""
#  else
#    define P_DEFAULT_PLUGIN_DIR ".:/usr/lib/ptlib:/usr/lib/pwlib"
#  endif
#endif

#define PTPLUGIN_SUFFIX       "_ptplugin"
#define PWPLUGIN_SUFFIX       "_pwplugin"

const char PPluginServiceDescriptor::SeparatorChar = '\t';


class PluginLoaderStartup : public PProcessStartup
{
  PCLASSINFO(PluginLoaderStartup, PProcessStartup);
  public:
    void OnStartup();
    void OnShutdown();
};


#define new PNEW


////////////////////////////////////////////////////////////////////////////////////

bool PPluginServiceDescriptor::ValidateServiceName(const PString & name, P_INT_PTR) const
{
  return GetServiceName() == name;
}


bool PPluginDeviceDescriptor::ValidateServiceName(const PString & name, P_INT_PTR userData) const
{
  return ValidateDeviceName(name, userData);
}


PStringArray PPluginDeviceDescriptor::GetDeviceNames(P_INT_PTR) const
{
  return GetServiceName();
}


bool PPluginDeviceDescriptor::ValidateDeviceName(const PString & name, P_INT_PTR userData) const
{
  PStringArray devices = GetDeviceNames(userData);
  if (name.GetLength() == 2 && name[0] == '#' && isdigit(name[1]) && (PINDEX)(name[1]-'0') < devices.GetSize())
    return true;

  for (PINDEX i = 0; i < devices.GetSize(); i++) {
    if (devices[i] *= name)
      return true;
  }

  return false;
}


bool PPluginDeviceDescriptor::GetDeviceCapabilities(const PString & /*deviceName*/, void * /*capabilities*/) const
{
  return false;
}


//////////////////////////////////////////////////////

PPluginManager::PPluginManager()
{
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
  SetDirectories(dirs.Tokenise(PPATH_SEPARATOR, false)); // split into directories on correct seperator
}


void PPluginManager::SetDirectories(const PStringArray & dirs)
{
  m_directories.RemoveAll();

  for (PINDEX i = 0; i < dirs.GetSize(); ++i)
    AddDirectory(dirs[i]);
}


void PPluginManager::AddDirectory(const PDirectory & dir)
{
  PTRACE(4, "PLUGIN\tAdding directory \"" << dir << '"');
  if (m_directories.GetValuesIndex(dir) == P_MAX_INDEX)
    m_directories.Append(new PDirectory(dir));
}


void PPluginManager::LoadDirectories()
{
  PTRACE(4, "PLUGIN\tEnumerating plugin directories " << setfill(PPATH_SEPARATOR) << m_directories);
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
      PTRACE(5, "PLUGIN\t" << fileName << " API version " << version);
      switch (version) {
        case 0 : // old-style service plugins, and old-style codec plugins
          {
            // call the register function (if present)
            if (!dll->GetFunction("PWLibPlugin_TriggerRegister", fn)) 
              PTRACE(2, "PLUGIN\t" << fileName << " has no registration-trigger function");
            else {
              void (*triggerRegister)(PPluginManager *) = (void (*)(PPluginManager *))fn;
              (*triggerRegister)(this);
              PTRACE(4, "PLUGIN\t" << fileName << " has been triggered");
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


PStringArray PPluginManager::GetServiceTypes() const
{
  PWaitAndSignal mutex(m_servicesMutex);

  PStringSet distinct;
  for (ServiceMap::const_iterator it = m_services.begin(); it != m_services.end(); ++it)
    distinct += it->first;

  PINDEX count = 0;
  PStringArray result(distinct.GetSize());
  for (PStringSet::iterator it = distinct.begin(); it != distinct.end(); ++it)
    result[count++] = *it;

  return result;
}


PStringArray PPluginManager::GetPluginsProviding(const PString & serviceType, bool friendlyName) const
{
  PWaitAndSignal mutex(m_servicesMutex);

  PStringArray result;
  for (ServiceMap::const_iterator it = m_services.find(serviceType); it != m_services.end() && it->first == serviceType; ++it)
    result.AppendString(friendlyName ? it->second->GetFriendlyName() : it->second->GetServiceName());

  return result;
}


const PPluginServiceDescriptor * PPluginManager::GetServiceDescriptor(const PString & serviceName,
                                                                      const PString & serviceType) const
{
  PWaitAndSignal mutex(m_servicesMutex);

  for (ServiceMap::const_iterator it = m_services.find(serviceType); it != m_services.end() && it->first == serviceType; ++it) {
    if (it->second->GetServiceName() == serviceName)
      return it->second;
  }
  return NULL;
}


PObject * PPluginManager::CreatePlugin(const PString & serviceName,
                                       const PString & serviceType,
                                       P_INT_PTR userData) const
{
  PWaitAndSignal mutex(m_servicesMutex);

  {
    // If have tab character, then have explicit driver name in device
    PINDEX tab = serviceName.Find(PPluginServiceDescriptor::SeparatorChar);
    const PPluginServiceDescriptor * descriptor = GetServiceDescriptor(tab != P_MAX_INDEX ? serviceName.Left(tab) : serviceName, serviceType);
    if (descriptor != NULL)
      return descriptor->CreateInstance(userData);
  }

  for (ServiceMap::const_iterator it = m_services.find(serviceType); it != m_services.end() && it->first == serviceType; ++it) {
    if (it->second->ValidateServiceName(serviceName, userData))
      return it->second->CreateInstance(userData);
  }

  return NULL;
}


PStringArray PPluginManager::GetPluginDeviceNames(const PString & serviceName,
                                                  const PString & serviceType,
                                                  P_INT_PTR userData,
                                                  const char * const * prioritisedDrivers) const
{
  PWaitAndSignal mutex(m_servicesMutex);

  if (!serviceName.IsEmpty() && serviceName != "*") {
    const PPluginDeviceDescriptor * descriptor = dynamic_cast<const PPluginDeviceDescriptor *>(GetServiceDescriptor(serviceName, serviceType));
    return descriptor != NULL ? descriptor->GetDeviceNames(userData) : PStringArray();
  }

  PStringToString deviceToPluginMap;  

  // First we run through all of the drivers and their lists of devices and
  // use the dictionary to assure all names are unique
  for (ServiceMap::const_iterator it = m_services.find(serviceType); it != m_services.end() && it->first == serviceType; ++it) {
    const PPluginDeviceDescriptor * descriptor = dynamic_cast<const PPluginDeviceDescriptor *>(it->second);
    if (descriptor != NULL) {
      PCaselessString driver = descriptor->GetServiceName();
      PStringArray devices = descriptor->GetDeviceNames(userData);
      for (PINDEX j = 0; j < devices.GetSize(); j++) {
        PCaselessString device = devices[j];
        if (deviceToPluginMap.Contains(device)) {
          PString oldDriver = deviceToPluginMap[device];
          if (!oldDriver.IsEmpty()) {
            // Make name unique by prepending driver name and a tab character
            deviceToPluginMap.SetAt(oldDriver+PPluginServiceDescriptor::SeparatorChar+device, driver);
            // Reset the original to empty string so we dont add it multiple times
            deviceToPluginMap.SetAt(device, "");
          }
          // Now add the new one
          deviceToPluginMap.SetAt(driver+PPluginServiceDescriptor::SeparatorChar+device, driver);
        }
        else
          deviceToPluginMap.SetAt(device, driver);
      }
    }
  }

  PStringArray allDevices;

  if (prioritisedDrivers != NULL) {
    while (*prioritisedDrivers != NULL) {
      for (PStringToString::iterator it = deviceToPluginMap.begin(); it != deviceToPluginMap.end(); ++it) {
        if (it->second == *prioritisedDrivers)
          allDevices.AppendString(it->first);
      }
      ++prioritisedDrivers;
    }
  }

  for (PStringToString::iterator it = deviceToPluginMap.begin(); it != deviceToPluginMap.end(); ++it) {
    if (!it->second.IsEmpty() && allDevices.GetValuesIndex(it->first) == P_MAX_INDEX)
      allDevices.AppendString(it->first);
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
    for (ServiceMap::const_iterator it = m_services.find(serviceType); it != m_services.end() && it->first == serviceType; ++it) {
      const PPluginDeviceDescriptor * desc = dynamic_cast<const PPluginDeviceDescriptor *>(it->second);
      if (desc != NULL && desc->ValidateDeviceName(deviceName, 0))
        return desc->GetDeviceCapabilities(deviceName,capabilities);
    }
  }
  else {
    const PPluginDeviceDescriptor * desc = dynamic_cast<const PPluginDeviceDescriptor *>(GetServiceDescriptor(serviceName, serviceType));
    if (desc != NULL && desc->ValidateDeviceName(deviceName, 0))
      return desc->GetDeviceCapabilities(deviceName,capabilities);
  }

  return false;
}


bool PPluginManager::RegisterService(const char * name)
{
  PTRACE(5, "PLUGIN\tRegistering \"" << name << '"');
  PPluginServiceDescriptor * descriptor = PPluginFactory::CreateInstance(name);
  if (descriptor == NULL) {
    PTRACE(3, "PLUGIN\tCould not create factory instance for \"" << name << '"');
    return false;
  }

  PWaitAndSignal mutex(m_servicesMutex);

  // first, check if it something didn't already register that name and type
  if (GetServiceDescriptor(descriptor->GetServiceName(), descriptor->GetServiceType()) != NULL) {
    PTRACE(3, "PLUGIN\tDuplicate \"" << name << '"');
    return false;
  }

  m_services.insert(ServiceMap::value_type(descriptor->GetServiceType(), descriptor));

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

  m_services.clear();

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
  PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
  if (pluginMgr.GetDirectories().IsEmpty()) {
    PString env = ::getenv(P_PTLIB_PLUGIN_DIR_ENV_VAR);
    if (env.IsEmpty())
      env = ::getenv(P_PWLIB_PLUGIN_DIR_ENV_VAR);
    if (env.IsEmpty()) {
      env = P_DEFAULT_PLUGIN_DIR;
      PDirectory appdir = PProcess::Current().GetFile().GetDirectory();
      if (!appdir.IsRoot())
        env += PPATH_SEPARATOR + appdir;
    }
    pluginMgr.SetDirectories(env);
  }

  // load the plugin module managers
  PFactory<PPluginModuleManager>::KeyList_T keyList = PFactory<PPluginModuleManager>::GetKeyList();
  for (PFactory<PPluginModuleManager>::KeyList_T::const_iterator it = keyList.begin(); it != keyList.end(); ++it)
    PFactory<PPluginModuleManager>::CreateInstance(*it)->OnStartup();

  // load the actual DLLs, which will also load the system plugins
  pluginMgr.LoadDirectories();

  // load the static plug in services/devices
  PPluginFactory::KeyList_T keys = PPluginFactory::GetKeyList();
  for (PPluginFactory::KeyList_T::iterator it = keys.begin(); it != keys.end(); ++it)
    pluginMgr.RegisterService(it->c_str());
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

#endif // P_PLUGINMGR
