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

class PPluginDynamic;

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

  protected:
    PMutex pluginListMutex;
    PList<PPluginDynamic> pluginList;
    
    PMutex serviceListMutex;
    PList<PPluginService> serviceList;
};

#endif // ifndef _PLUGINMGR_H
