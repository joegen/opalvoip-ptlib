/*
 * pluginmgr.h
 *
 * Manager for run-time loadable plugins
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
 * $Log: pluginmgr.h,v $
 * Revision 1.1.2.2  2003/10/20 03:25:12  dereksmithies
 * Fix GetFunction method so it is guaranteed to generate good results.
 *
 * Revision 1.1.2.1  2003/10/07 01:33:19  csoutheren
 * Initial checkin of pwlib code to do plugins.
 * Modified from original code and concept provided by Snark of Gnomemeeting
 *
 */

#ifndef _PLUGINMGR_H
#define _PLUGINMGR_H

#include <ptlib.h>
#include <ptlib/dynalink.h>

/**
  Ancestor class for all plugin modules
  This cannot be a descendant of PObject due to multiple inheritance
  */
class PPlugin 
{
  friend class PPluginManager;
  public:
    virtual ~PPlugin() { }

    virtual BOOL    IsValid()   { return TRUE; }
    virtual BOOL    IsDynamic() { return FALSE; }
    virtual PString GetName()   { return PString(); }

    virtual unsigned GetAPIVersion() const = 0;
    virtual PString  GetBaseClassName() const = 0;
    virtual PString  GetClassName() const = 0;

    virtual BOOL GetFunction(
      const PString & name,  
      PDynaLink::Function & func
      ) { cerr << "Call of PPlugin::GetFunction for " << name << endl; return FALSE; } // = 0;

    virtual PString GetFileName() { return fileName; }

  protected:
    PPlugin * next;
    PString   fileName;
};

/**
  Ancestor class for all dynamically loaded plugin modules
  */
class PDynamicPlugin : public PPlugin
{
  public:
    PDynamicPlugin(const PString & str);
    BOOL IsValid();
    virtual BOOL IsDynamic()  { return TRUE; }
    virtual PString GetName() { if (dll == NULL) return PString::Empty(); else return dll->GetName(); }

    // API access functions
    unsigned GetAPIVersion() const
      { if (!dll->IsLoaded()) return 0xffffffff; else return (*versionFn)(); }

    PString GetBaseClassName() const
      { if (!dll->IsLoaded()) return PString::Empty(); else return PString((*baseClassNameFn)()); }

    PString GetClassName() const
      { if (!dll->IsLoaded()) return PString::Empty(); else return PString((*classNameFn)()); }
    BOOL GetFunction(
      const PString & name,  
      PDynaLink::Function & func
    ) 
      { if (!dll->IsLoaded()) return FALSE; else return dll->GetFunction(name, func); }

  protected:
    PDynaLink * dll;

    // pointers to required plugin functions
    unsigned (*versionFn)();
    char * (*baseClassNameFn)();
    char * (*classNameFn)();
};

/**
  Manage the list of plugins
  */
class PPluginManager : public PObject
{
  PCLASSINFO(PPluginManager, PObject);
  public:
    PPluginManager(BOOL loadDefault = TRUE);
    ~PPluginManager();

    BOOL LoadPlugin(const PString & fn);
    BOOL LoadPluginDirectory(const PDirectory & _dir);

    BOOL LoadPlugin(PPlugin * plugin);

    PPlugin * GetPlugin(const PString & baseClassName, const PString & classNameName);

    PStringArray GetPluginNames()
    { return GetPluginNames("*"); }

    PStringArray GetPluginNames(const PString & className);

    void PrintOn(ostream & strm) const;

    PStringArray    GetDevicePluginNames(const PString & type);
    PPlugin       * GetDevicePluginByName(const PString & type, const PString & name);
    PChannel      * CreateDeviceChannelByName(const PString & type, const PString & name);
    PStringArray    GetDevicePluginDeviceNames(const PString & type, const PString & name, int dir);

    static PPluginManager & GetPluginManager();

  protected:
    PMutex mutex;
    PPlugin * first;
    static PPluginManager * globalPluginManager;
};

class PPluginRegistration : PObject
{
  PCLASSINFO(PPluginRegistration, PObject);
  public:
    PPluginRegistration(PPlugin * plugin)
      { PPluginManager::GetPluginManager().LoadPlugin(plugin); }
};

#endif

