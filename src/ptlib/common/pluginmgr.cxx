/*
 * pluginmgr.cxx
 *
 * Code for pluigns sound device
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Post Increment
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
 * Contributor(s): Craig Southeren
 *                 Snark at GnomeMeeting
 *
 * $Log: pluginmgr.cxx,v $
 * Revision 1.1.2.2  2003/10/20 03:29:48  dereksmithies
 * Add some seriously defensive programming tests.
 *
 * Revision 1.1.2.1  2003/10/07 01:33:19  csoutheren
 * Initial checkin of pwlib code to do plugins.
 * Modified from original code and concept provided by Snark of Gnomemeeting
 *
 */

#include <ptlib.h>
#include <ptlib/pluginmgr.h>

#define	APIVERSION_FUNCTIONNAME	 "PWLibPlugin_GetAPIVersion"
#define	BASECLASS_FUNCTIONNAME	 "PWLibPlugin_GetBaseClassName"
#define	CLASSNAME_FUNCTIONNAME	 "PWLibPlugin_GetClassName"
#define TYPE_FUNCTIONNAME        "PWLibPlugin_GetType"

PPluginManager * PPluginManager::globalPluginManager = NULL;

/////////////////////////////////////////////////////////////////////////////

PPluginManager::PPluginManager(BOOL loadDefault)
{
  PWaitAndSignal m(mutex);
  first = NULL;
  if (loadDefault)
    LoadPluginDirectory(P_DEFAULTPLUGINDIR);
}

PPluginManager::~PPluginManager()
{
}

BOOL PPluginManager::LoadPluginDirectory(const PDirectory & _dir)
{
  PWaitAndSignal m(mutex);

  PDirectory dir = _dir;

  if (!dir.Open()) {
    PTRACE(2, "Cannot open plugin directory " << dir);
    return FALSE;
  }

  PTRACE(4, "Enumerating plugin directory " << dir);

  BOOL result = FALSE;

  do {
    PString entry = dir + dir.GetEntryName();
    if (dir.IsSubDir())
      result = result || LoadPluginDirectory(entry);
    else
      result = result || LoadPlugin(entry);
  } while (dir.Next());

  return result;
}

BOOL PPluginManager::LoadPlugin(const PString & name)
{
  PPlugin * plugin = new PDynamicPlugin(name);
  if (LoadPlugin(plugin)) 
    return TRUE;


  delete plugin;
  return FALSE;
}

BOOL PPluginManager::LoadPlugin(PPlugin * plugin)
{
  PWaitAndSignal m(mutex);

  // check for required functions
  if (!plugin->IsValid()) 
    return FALSE;

  PTRACE(4, "Loaded plugin " << plugin->GetClassName() << " -> " << plugin->GetBaseClassName());

  // add the plugin to the list
  plugin->next = first;
  first = plugin;

  return TRUE;
}

PPlugin * PPluginManager::GetPlugin(const PString & className, const PString & baseClassName)
{
  PWaitAndSignal m(mutex);

  PPlugin * plugin = first;
  while (plugin != NULL) {
    if (plugin->GetBaseClassName() == baseClassName &&
        plugin->GetClassName() == className) 
      return plugin;
    plugin = plugin->next;
  }

  return NULL;
}

PStringArray PPluginManager::GetPluginNames(const PString & baseClassName)
{
  PWaitAndSignal m(mutex);

  PStringArray names;
  PPlugin * plugin = first;
  while (plugin != NULL) {
    if (baseClassName == "*" ||
        baseClassName == plugin->GetBaseClassName())
      names.AppendString(plugin->GetClassName());
    plugin = plugin->next;
  }

  return names;
}

void PPluginManager::PrintOn(ostream & strm) const
{
  PPluginManager * thisP = (PPluginManager *)this;
  strm << thisP->GetPluginNames("*");
}

PPluginManager & PPluginManager::GetPluginManager()
{
  static PMutex globalMutex;
  PWaitAndSignal m(globalMutex);
  if (globalPluginManager == NULL)
    globalPluginManager = new PPluginManager();
  return *globalPluginManager;
}

PStringArray PPluginManager::GetDevicePluginNames(const PString & type)
{
  PStringArray pluginNames = GetPluginNames(type);

  // now get their device names
  PStringArray devNames;
  PINDEX i;
  for (i = 0; i < pluginNames.GetSize(); i++) {
    PPlugin * plugin = GetPlugin(pluginNames[i], type);
    PString (*typeFn)() = NULL;
    if (plugin->GetFunction("GetType", (PDynaLink::Function &)typeFn)) 
      if (typeFn != NULL) {
	PString str = (*typeFn)();
	devNames.AppendString(str);
      }
  }
  return devNames;
}

PPlugin * PPluginManager::GetDevicePluginByName(const PString & type, const PString & name)
{
  PStringArray pluginNames = GetPluginNames(type);

  PINDEX i;
  for (i = 0; i < pluginNames.GetSize(); i++) {
    PPlugin * plugin = GetPlugin(pluginNames[i], type);
    PString (*typeFn)() = NULL;
    if (plugin->GetFunction("GetType", (PDynaLink::Function &)typeFn))
      if (typeFn != NULL) {
	if ((*typeFn)() == name) 
	  return plugin;
      }
  }
  return NULL;
}

PChannel * PPluginManager::CreateDeviceChannelByName(const PString & name,
                                                     const PString & type)
{
  PPlugin * plugin = GetDevicePluginByName(name, type);
  if (plugin == NULL)
    return NULL;

  PChannel * (*createFn)() = NULL;
  if (!plugin->GetFunction("Create", (PDynaLink::Function &)createFn))
    return NULL;

  if (createFn == NULL)
    return NULL;
  else
    return (*createFn)();
}

PStringArray PPluginManager::GetDevicePluginDeviceNames(const PString & name,
                                                        const PString & type,
                                                                    int dir)
{
  PPlugin * plugin = GetDevicePluginByName(name, type);
  if (plugin == NULL)
    return NULL;

  PStringArray (*deviceNamesFn)(int) = NULL;
  if (!plugin->GetFunction("GetDeviceNames", (PDynaLink::Function &)deviceNamesFn))
    return FALSE;
  
  if (deviceNamesFn == NULL)
    return FALSE;
  else
    return (*deviceNamesFn)(dir);
}

/////////////////////////////////////////////////////////////////////////////

PDynamicPlugin::PDynamicPlugin(const PString & name)
{
  fileName = name;

  dll = new PDynaLink(name);
  if (!dll->IsLoaded() ||
      !dll->GetFunction(APIVERSION_FUNCTIONNAME, (PDynaLink::Function &)versionFn) ||
      !dll->GetFunction(BASECLASS_FUNCTIONNAME,  (PDynaLink::Function &)baseClassNameFn) ||
      !dll->GetFunction(CLASSNAME_FUNCTIONNAME,  (PDynaLink::Function &)classNameFn)) {
      cerr << "Reject " << name << " as a plugin as it does not have the requisite functions." << endl;
      dll->Close();
      delete dll;
      dll = NULL;
  }
}

BOOL PDynamicPlugin::IsValid()
{
  return dll != NULL;
}

/////////////////////////////////////////////////////////////////////////////


