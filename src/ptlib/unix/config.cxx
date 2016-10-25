/*
 * config.cxx
 *
 * System/application configuration class implementation
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#define _CONFIG_CXX

#pragma implementation "config.h"

#include <ptlib.h>
#include <ptlib/pprocess.h>

#include "../common/pconfig.cxx"


static const char OldSystemConfigFile[] = "/usr/local/pwlib/pwlib.ini";
static const char NewSystemConfigFile[] = "/usr/local/share/ptlib/ptlib.ini";


#if defined(P_MACOSX) || defined(P_IOS) || defined(P_SOLARIS) || defined(P_FREEBSD)
#define environ (NULL)
#endif

#if defined(P_NETBSD) || defined(P_OPENBSD)
extern char **environ;
#endif


//
// a list of sections
//

class PConfig::Cached : public PDictionary<PCaselessString, PStringOptions>
{
    typedef PDictionary<PCaselessString, PStringOptions> ParentClass;
    PCLASSINFO(Cached, ParentClass);
  public:
    Cached(const PFilePath & filename);
    Cached(char **envp);
    ~Cached();

    void SetDirty();

  protected:
    void Flush();
    PDECLARE_NOTIFIER(PTimer, Cached, FlushTimeout);

    PFilePath      m_filePath;
    atomic<uint32_t> m_instanceCount;
    PMutex         m_mutex;
    bool           m_dirty;
    bool           m_canSave;
    PTimer         m_flushTimer;

  friend class PConfigCache;
  friend class PConfig;
};


static bool IsComment(const PString& str)
{
  return str.GetLength() && strchr(";#", str[0]);
}


//
// a dictionary of configurations, keyed by filename
//
class PConfigCache : public PProcessStartup
{
    PCLASSINFO(PConfigCache, PProcessStartup)
  public:
    PConfigCache();
    ~PConfigCache();

    virtual void OnShutdown();

    PConfig::Cached * GetEnvironmentCache();
    PConfig::Cached * GetFileCache(const PFilePath & filename);
    void Detach(PConfig::Cached * cache);

    PFACTORY_GET_SINGLETON(PProcessStartupFactory, PConfigCache);

  protected:
    PMutex   m_mutex;
    PConfig::Cached  * m_environmentCache;

    typedef PDictionary<PFilePath, PConfig::Cached> CacheDict;
    CacheDict m_cache;
};

PFACTORY_CREATE_SINGLETON(PProcessStartupFactory, PConfigCache);



#define PTraceModule() "PConfig"
#define	new PNEW


//////////////////////////////////////////////////////

PConfig::Cached::Cached(const PFilePath & path)
  : m_filePath(path)
  , m_dirty(false)
  , m_canSave(true) // normally save on exit (except for environment configs)
{
  PTRACE(4, "Created " << this << " for " << m_filePath);

  // attempt to open file
  PTextFile file;
  if (!file.Open(m_filePath, PFile::ReadOnly))
    return;

  PStringOptions * currentSection = NULL;

  // read lines in the file
  while (file.good()) {
    PString line;
    file >> line;
    line = line.LeftTrim();
    if (line.IsEmpty())
      continue;

    // Preserve comments
    if (IsComment(line)) {
      SetAt(line, new PStringOptions());
      continue;
    }

    if (line[0] == '[') {
      PCaselessString sectionName = line(1, line.Find(']')-1).Trim();
      if ((currentSection = GetAt(sectionName)) == NULL)
        SetAt(sectionName, currentSection = new PStringOptions());
    }
    else if (currentSection != NULL) {
      PString keyStr, valStr;
      if (line.Split('=', keyStr, valStr, PString::SplitDefaultToBefore|PString::SplitTrimBefore|PString::SplitBeforeNonEmpty)) {
        PStringOptions::iterator it = currentSection->find(keyStr);
        if (it != currentSection->end())
          valStr.Splice(it->second + '\n', 0, 0);
        currentSection->SetAt(keyStr, valStr);
      }
    }
  }

  PTRACE(3, "Read config file " << m_filePath);
}


PConfig::Cached::~Cached()
{
  Flush();
  PTRACE(4, "Destroyed " << this);
}


void PConfig::Cached::SetDirty()
{
  PTRACE_IF(4, !m_dirty, "Setting config cache dirty.");
  m_dirty = true;
  m_flushTimer.SetInterval(0, 10);
}


void PConfig::Cached::FlushTimeout(PTimer&, INT)
{
  m_mutex.Wait();
  Flush();
  m_mutex.Signal();
}


void PConfig::Cached::Flush()
{
  if (!m_dirty)
    return;

  m_dirty = false;
  m_flushTimer.Stop();

  // make sure the directory that the file is to be written into exists
  PDirectory dir = m_filePath.GetDirectory();
  if (!dir.Exists() && !dir.Create(PFileInfo::UserExecute|PFileInfo::UserWrite|PFileInfo::UserRead, true)) {
    PTRACE(1, "Could not create directory: " << dir);
    PProcess::PXShowSystemWarning(2000, "Cannot create config directory");
    return;
  }

  PTextFile file;
  if (!(PFile::Exists(m_filePath) ? file.Open(m_filePath + ".new", PFile::WriteOnly) : file.Open(m_filePath , PFile::WriteOnly))) {
    PTRACE(1, "Could not create file: " << file.GetFilePath() << " - " << file.GetErrorText());
    PProcess::PXShowSystemWarning(2001, "Cannot create config file: " + file.GetErrorText());
    return;
  }

  for (iterator it = begin(); it != end(); ++it) {
    // If the line is a comment, output it as is
    if (IsComment(it->first)) {
      file << it->first << endl;
      continue;
    }

    file << "[" << it->first << "]" << endl;
    for (PStringOptions::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
      PStringArray lines = it2->second.Tokenise('\n', true);
      // Preserve name/value pairs with no value, i.e. of the form "name="
      if (lines.IsEmpty())
        file << it2->first << "=" << endl;
      else {
        for (PINDEX k = 0; k < lines.GetSize(); k++) 
          file << it2->first << "=" << lines[k] << endl;
      }
    }
    file << endl;
  }

  file.Close();

  if (file.GetFilePath() != m_filePath && !file.Rename(file.GetFilePath(), m_filePath.GetFileName(), true)) {
    PTRACE(1, "Could not rename file: " << file.GetFilePath() << " to " << m_filePath << " - " << file.GetErrorText());
    PProcess::PXShowSystemWarning(2001, "Cannot rename config file: " + file.GetErrorText());
    return;
  }

  PTRACE(4, "Flushed config file: " << m_filePath);
}


PConfig::Cached::Cached(char **envp)
  : m_dirty(false)
  , m_canSave(false) // can't save environment configs
{
  PTRACE(4, "Created " << this << " for environment");

  PStringOptions * envSection = new PStringOptions();
  SetAt(PConfig::DefaultSectionName(), envSection);

  if (envp == NULL)
    return;

  while (*envp != NULL && **envp != '\0') {
    PStringStream strm(*envp++);
    strm >> *envSection;
  }
}


////////////////////////////////////////////////////////////

PConfigCache::PConfigCache()
  : m_environmentCache(NULL)
{
}


PConfigCache::~PConfigCache()
{
  delete m_environmentCache;
}


void PConfigCache::OnShutdown()
{
  m_cache.RemoveAll(); // And flush them
}


PConfig::Cached * PConfigCache::GetEnvironmentCache()
{
  m_mutex.Wait();
  if (m_environmentCache == NULL)
    m_environmentCache = new PConfig::Cached(environ);
  m_mutex.Signal();
  return m_environmentCache;
}


PConfig::Cached * PConfigCache::GetFileCache(const PFilePath & filename)
{
  m_mutex.Wait();

  PConfig::Cached * config = m_cache.GetAt(filename);
  if (config == NULL)
    m_cache.SetAt(filename, config = new PConfig::Cached(filename));
  ++config->m_instanceCount;

  m_mutex.Signal();
  return config;
}


void PConfigCache::Detach(PConfig::Cached * config)
{
  m_mutex.Wait();

  if (config != m_environmentCache) {
    CacheDict::iterator it = m_cache.find(config->m_filePath);
    if (it != m_cache.end() && --config->m_instanceCount == 0)
      m_cache.erase(it);
  }

  m_mutex.Signal();
}


void PConfig::Construct(Source src, const PString & appname, const PString & /*manuf*/)
{
  switch (src) {
    case Environment:
      m_config = PConfigCache::GetInstance().GetEnvironmentCache();
      break;

    case System:
      m_config = PConfigCache::GetInstance().GetFileCache(PFile::Exists(OldSystemConfigFile) ? OldSystemConfigFile : NewSystemConfigFile);
      break;

    default :
      m_config = PConfigCache::GetInstance().GetFileCache(PProcess::Current().GetConfigurationFile());
  }
}


void PConfig::Construct(const PFilePath & theFilename)
{
  m_config = PConfigCache::GetInstance().GetFileCache(theFilename);
}


PConfig::~PConfig()
{
  PConfigCache::GetInstance().Detach(m_config);
}


PStringArray PConfig::GetSections() const
{
  PAssert(m_config != NULL, "config instance not set");
  PWaitAndSignal(m_config->m_mutex);

  PStringArray sections(m_config->GetSize());

  PINDEX index = 0;
  for (Cached::iterator it = m_config->begin(); it != m_config->end(); ++it)
    sections[index++] = it->first;

  return sections;
}


PStringArray PConfig::GetKeys(const PString & theSection) const
{
  PAssert(m_config != NULL, "config instance not set");
  PWaitAndSignal(m_config->m_mutex);

  PStringArray keys;

  PStringOptions * section = m_config->GetAt(theSection);
  if (section != NULL) {
    keys.SetSize(section->GetSize());
    PINDEX index = 0;
    for (PStringOptions::iterator it = section->begin(); it!= section->end(); ++it)
      keys[index++] = it->first;
  }

  return keys;
}


void PConfig::DeleteSection(const PString & theSection)
{
  PAssert(m_config != NULL, "config instance not set");
  PWaitAndSignal(m_config->m_mutex);

  PConfig::Cached::iterator it = m_config->find(theSection);
  if (it != m_config->end()) {
    m_config->erase(it);
    m_config->SetDirty();
  }
}


void PConfig::DeleteKey(const PString & theSection, const PString & theKey)
{
  PAssert(m_config != NULL, "config instance not set");
  PWaitAndSignal(m_config->m_mutex);

  PStringOptions * section = m_config->GetAt(theSection);
  if (section != NULL) {
    PStringOptions::iterator it = section->find(theKey);
    if (it != section->end()) {
      section->erase(it);
      m_config->SetDirty();
    }
  }
}


PBoolean PConfig::HasKey(const PString & theSection, const PString & theKey) const
{
  PAssert(m_config != NULL, "config instance not set");
  PWaitAndSignal(m_config->m_mutex);

  PStringOptions * section = m_config->GetAt(theSection);
  return section != NULL && section->Contains(theKey);
}


PString PConfig::GetString(const PString & theSection, const PString & theKey, const PString & dflt) const
{
  PAssert(m_config != NULL, "config instance not set");
  PWaitAndSignal(m_config->m_mutex);

  PStringOptions * section = m_config->GetAt(theSection);
  return section != NULL ? section->GetString(theKey, dflt) : dflt;
}


void PConfig::SetString(const PString & theSection,
                        const PString & theKey,
                        const PString & theValue)
{
  PAssert(m_config != NULL, "config instance not set");
  PWaitAndSignal(m_config->m_mutex);

  PStringOptions * section = m_config->GetAt(theSection);
  if (section == NULL)
    m_config->SetAt(theSection, section = new PStringOptions);

  section->SetAt(theKey, theValue);
  m_config->SetDirty();
}


///////////////////////////////////////////////////////////////////////////////

PString PProcess::GetConfigurationFile()
{
  if (m_configurationPaths.IsEmpty()) {
    m_configurationPaths.AppendString(GetHomeDirectory() + ".ptlib_config/");
    m_configurationPaths.AppendString(GetHomeDirectory() + ".pwlib_config/");
    m_configurationPaths.AppendString("/usr/local/share/ptlib/");
    m_configurationPaths.AppendString("/usr/local/pwlib/");
  }

  // See if explicit filename
  if (m_configurationPaths.GetSize() == 1 && !PDirectory::Exists(m_configurationPaths[0]))
    return m_configurationPaths[0];

  PString iniFilename = m_executableFile.GetTitle() + ".ini";

  for (PINDEX i = 0; i < m_configurationPaths.GetSize(); i++) {
    PFilePath cfgFile = PDirectory(m_configurationPaths[i]) + iniFilename;
    if (PFile::Exists(cfgFile))
      return cfgFile;
  }

  return PDirectory(m_configurationPaths[0]) + iniFilename;
}


///////////////////////////////////////////////////////////////////////////////
