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


#define SYS_CONFIG_NAME		"pwlib"

#define	APP_CONFIG_DIR		".pwlib_config/"
#define	SYS_CONFIG_DIR		"/usr/local/pwlib/"

#define	EXTENSION		".ini"
#define	ENVIRONMENT_CONFIG_STR	"/\~~environment~~\/"

#if defined(P_MACOSX) || defined(P_SOLARIS) || defined(P_FREEBSD)
#define environ (NULL)
#endif

#if defined(P_NETBSD) || defined(P_OPENBSD)
extern char **environ;
#endif


//
// a list of sections
//

class PXConfig : public PDictionary<PCaselessString, PStringToString>
{
    typedef PDictionary<PCaselessString, PStringToString> ParentClass;
    PCLASSINFO(PXConfig, ParentClass);
  public:
    PXConfig(const PString & key, const PFilePath & filename);
    ~PXConfig();

    void Wait()   { mutex.Wait(); }
    void Signal() { mutex.Signal(); }

    PBoolean ReadFromFile (const PFilePath & filename);
    void ReadFromEnvironment (char **envp);

    PBoolean WriteToFile(const PFilePath & filename);
    void Flush();

    void SetDirty()
    {
      PTRACE_IF(4, !dirty, "PTLib\tSetting PXConfig dirty.");
      dirty = PTrue;
    }

  protected:
    PString        m_key;
    PFilePath      m_filename;
    PAtomicInteger instanceCount;
    PMutex         mutex;
    bool           dirty;
    bool           canSave;

  friend class PXConfigDictionary;
};


static PBoolean IsComment(const PString& str)
{
  return str.GetLength() && strchr(";#", str[0]);
}


//
// a dictionary of configurations, keyed by filename
//
class PXConfigDictionary : public PDictionary<PString, PXConfig>
{
    typedef PDictionary<PString, PXConfig> ParentClass;
    PCLASSINFO(PXConfigDictionary, ParentClass)
  public:
    PXConfigDictionary();
    ~PXConfigDictionary();
    PXConfig * GetFileConfigInstance(const PString & key, const PFilePath & filename);
    PXConfig * GetEnvironmentInstance();
    void RemoveInstance(PXConfig * instance);
    void WriteChangedInstances();

  protected:
    PMutex        mutex;
    PXConfig    * environmentInstance;
    PThread     * writeThread;
    PSyncPointAck stopConfigWriteThread;
};


PDECLARE_CLASS(PXConfigWriteThread, PThread)
  public:
    PXConfigWriteThread(PSyncPointAck & stop);
    void Main();
  private:
    PSyncPointAck & stop;
};


static PXConfigDictionary * g_configDict;

#define	new PNEW

//////////////////////////////////////////////////////

void PProcess::CreateConfigFilesDictionary()
{
  configFiles = new PXConfigDictionary;
}


PXConfigWriteThread::PXConfigWriteThread(PSyncPointAck & s)
  : PThread(10000, NoAutoDeleteThread, NormalPriority, "PXConfigWriteThread"),
    stop(s)
{
  Resume();
}


void PXConfigWriteThread::Main()
{
  PTRACE(4, "PTLib\tConfig file cache write back thread started.");
  while (!stop.Wait(30000))  // if stop.Wait() returns PTrue, we are shutting down
    g_configDict->WriteChangedInstances();   // check dictionary for items that need writing

  g_configDict->WriteChangedInstances();

  stop.Acknowledge();
}


//////////////////////////////////////////////////////

PXConfig::PXConfig(const PString & key, const PFilePath & filename)
  : m_key(key)
  , m_filename(filename)
{
  // make sure content gets removed
  AllowDeleteObjects();

  // we start off clean
  dirty = PFalse;

  // normally save on exit (except for environment configs)
  canSave = PTrue;

  PTRACE(4, "PTLib\tCreated PXConfig " << this);
}


PXConfig::~PXConfig()
{
  PTRACE(4, "PTLib\tDestroyed PXConfig " << this);
}


void PXConfig::Flush()
{
  mutex.Wait();

  if (canSave && dirty) {
    WriteToFile(m_filename);
    dirty = PFalse;
  }

  mutex.Signal();
}

PBoolean PXConfig::WriteToFile(const PFilePath & filename)
{
  // make sure the directory that the file is to be written into exists
  PDirectory dir = filename.GetDirectory();
  if (!dir.Exists() && !dir.Create( 
                                   PFileInfo::UserExecute |
                                   PFileInfo::UserWrite |
                                   PFileInfo::UserRead)) {
    PProcess::PXShowSystemWarning(2000, "Cannot create PWLIB config directory");
    return PFalse;
  }

  PTextFile file;
  if (!file.Open(filename + ".new", PFile::WriteOnly))
    file.Open(filename, PFile::WriteOnly);

  if (!file.IsOpen()) {
    PProcess::PXShowSystemWarning(2001, "Cannot create PWLIB config file: " + file.GetErrorText());
    return PFalse;
  }

  for (iterator it = begin(); it != end(); ++it) {
    // If the line is a comment, output it as is
    if (IsComment(it->first)) {
      file << it->first << endl;
      continue;
    }

    file << "[" << it->first << "]" << endl;
    for (PStringToString::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
      PStringArray lines = it2->second.Tokenise('\n', PTrue);
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

  file.flush();
  file.SetLength(file.GetPosition());
  file.Close();

  if (file.GetFilePath() != filename) {
    if (!file.Rename(file.GetFilePath(), filename.GetFileName(), PTrue)) {
      PProcess::PXShowSystemWarning(2001, "Cannot rename config file: " + file.GetErrorText());
      return PFalse;
    }
  }

  PTRACE(4, "PTLib\tSaved config file: " << filename);
  return PTrue;
}


PBoolean PXConfig::ReadFromFile(const PFilePath & filename)
{
  PINDEX len;

  // clear out all information
  RemoveAll();

  PTRACE(4, "PTLib\tReading config file: " << filename);

  // attempt to open file
  PTextFile file;
  if (!file.Open(filename, PFile::ReadOnly))
    return PFalse;

  PStringToString * currentSection = NULL;

  // read lines in the file
  while (file.good()) {
    PString line;
    file >> line;
    line = line.Trim();
    if ((len = line.GetLength()) > 0) {
      // Preserve comments
      if (IsComment(line))
        SetAt(line, new PStringToString());
      else {
        if (line[0] == '[') {
          PCaselessString sectionName = (line.Mid(1,len-(line[len-1]==']'?2:1))).Trim();
          iterator iter;
          if ((iter = find(sectionName)) != end())
            currentSection = &iter->second;
          else {
            currentSection = new PStringToString();
            SetAt(sectionName, currentSection);
          }
        }
        else if (currentSection != NULL) {
          PINDEX equals = line.Find('=');
          if (equals > 0 && equals != P_MAX_INDEX) {
            PString keyStr = line.Left(equals).Trim();
            PString valStr = line.Right(len - equals - 1).Trim();

            if (currentSection->Contains(keyStr))
              (*currentSection)[keyStr] += '\n' + valStr;
            else
              currentSection->SetAt(keyStr, valStr);
          }
        }
      }
    }
  }
  
  // close the file and return
  file.Close();
  return PTrue;
}

void PXConfig::ReadFromEnvironment (char **envp)
{
  // clear out all information
  RemoveAll();

  PStringToString * currentSection = new PStringToString();
  SetAt("Options", currentSection);

  // can't save environment configs
  canSave = PFalse;

  if (envp == NULL)
    return;

  while (*envp != NULL && **envp != '\0') {
    PString line(*envp);
    PINDEX equals = line.Find('=');
    if (equals > 0)
      currentSection->SetAt(line.Left(equals), line.Mid(equals+1));
    envp++;
  }
}


static PBoolean LocateFile(const PString & baseName,
                       PFilePath & readFilename,
                       PFilePath & filename)
{
  // check the user's home directory first
  filename = readFilename = PProcess::Current().GetConfigurationFile();
  if (PFile::Exists(filename))
    return PTrue;

  // otherwise check the system directory for a file to read,
  // and then create 
  readFilename = SYS_CONFIG_DIR + baseName + EXTENSION;
  return PFile::Exists(readFilename);
}

///////////////////////////////////////////////////////////////////////////////

PString PProcess::GetConfigurationFile()
{
  if (configurationPaths.IsEmpty()) {
    configurationPaths.AppendString(PXGetHomeDir() + APP_CONFIG_DIR);
    configurationPaths.AppendString(SYS_CONFIG_DIR);
  }

  // See if explicit filename
  if (configurationPaths.GetSize() == 1 && !PDirectory::Exists(configurationPaths[0]))
    return configurationPaths[0];

  PString iniFilename = executableFile.GetTitle() + ".ini";

  for (PINDEX i = 0; i < configurationPaths.GetSize(); i++) {
    PFilePath cfgFile = PDirectory(configurationPaths[i]) + iniFilename;
    if (PFile::Exists(cfgFile))
      return cfgFile;
  }

  return PDirectory(configurationPaths[0]) + iniFilename;
}


////////////////////////////////////////////////////////////
//
// PXConfigDictionary
//

PXConfigDictionary::PXConfigDictionary()
{
  environmentInstance = NULL;
  writeThread = NULL;
  g_configDict = this;
}


PXConfigDictionary::~PXConfigDictionary()
{
  if (writeThread != NULL) {
    stopConfigWriteThread.Signal();
    writeThread->WaitForTermination();
    delete writeThread;
  }
  delete environmentInstance;
}


PXConfig * PXConfigDictionary::GetEnvironmentInstance()
{
  mutex.Wait();
  if (environmentInstance == NULL) {
    environmentInstance = new PXConfig(PString::Empty(), PString::Empty());
    environmentInstance->ReadFromEnvironment(environ);
  }
  mutex.Signal();
  return environmentInstance;
}


PXConfig * PXConfigDictionary::GetFileConfigInstance(const PString & key, const PFilePath & filename)
{
  mutex.Wait();

  // start write thread, if not already started
  if (writeThread == NULL)
    writeThread = new PXConfigWriteThread(stopConfigWriteThread);

  PXConfig * config = GetAt(key);
  if (config == NULL) {
    config = new PXConfig(key, filename);
    config->ReadFromFile(filename);
    SetAt(key, config);
  }
  ++config->instanceCount;

  mutex.Signal();
  return config;
}

void PXConfigDictionary::RemoveInstance(PXConfig * instance)
{
  mutex.Wait();

  if (instance != environmentInstance) {
    iterator it = find(instance->m_key);
    if (it != end() && --instance->instanceCount == 0) {
      instance->Flush();
      erase(it);
    }
  }

  mutex.Signal();
}

void PXConfigDictionary::WriteChangedInstances()
{
  mutex.Wait();

  for (iterator it = begin(); it != end(); ++it)
    it->second.Flush();

  mutex.Signal();
}

////////////////////////////////////////////////////////////
//
// PConfig::
//
// Create a new configuration object
//
////////////////////////////////////////////////////////////

void PConfig::Construct(Source src,
                        const PString & appname,
                        const PString & /*manuf*/)
{
  // handle cnvironment configs differently
  if (src == PConfig::Environment)  {
    config = g_configDict->GetEnvironmentInstance();
    return;
  }
  
  PString name;
  PFilePath filename, readFilename;
  
  // look up file name to read, and write
  if (src == PConfig::System)
    LocateFile(SYS_CONFIG_NAME, readFilename, filename);
  else
    filename = readFilename = PProcess::Current().GetConfigurationFile();

  // get, or create, the configuration
  config = g_configDict->GetFileConfigInstance(filename, readFilename);
}

PConfig::PConfig(int, const PString & name)
  : defaultSection("Options")
{
  PFilePath readFilename, filename;
  LocateFile(name, readFilename, filename);
  config = g_configDict->GetFileConfigInstance(filename, readFilename);
}


void PConfig::Construct(const PFilePath & theFilename)
{
  config = g_configDict->GetFileConfigInstance(theFilename, theFilename);
}


PConfig::~PConfig()
{
  g_configDict->RemoveInstance(config);
}


////////////////////////////////////////////////////////////
//
// PConfig::
//
// Return a list of all the section names in the file.
//
////////////////////////////////////////////////////////////

PStringArray PConfig::GetSections() const
{
  PAssert(config != NULL, "config instance not set");
  config->Wait();

  PINDEX sz = config->GetSize();
  PStringArray sections(sz);

  PINDEX index = 0;
  for (PXConfig::iterator it = config->begin(); it != config->end(); ++it)
    sections[index++] = it->first;

  config->Signal();

  return sections;
}


////////////////////////////////////////////////////////////
//
// PConfig::
//
// Return a list of all the keys in the section. If the section name is
// not specified then use the default section.
//
////////////////////////////////////////////////////////////

PStringArray PConfig::GetKeys(const PString & theSection) const
{
  PAssert(config != NULL, "config instance not set");
  config->Wait();

  PStringArray keys;

  PXConfig::iterator it;
  if ((it = config->find(theSection)) != config->end()) {
    PStringToString & section = it->second;
    keys.SetSize(section.GetSize());
    PINDEX index = 0;
    for (PStringToString::iterator it2 = section.begin(); it2 != section.end(); ++it2)
      keys[index++] = it2->first;
  }

  config->Signal();
  return keys;
}



////////////////////////////////////////////////////////////
//
// PConfig::
//
// Delete all variables in the specified section. If the section name is
// not specified then use the default section.
//
////////////////////////////////////////////////////////////

void PConfig::DeleteSection(const PString & theSection)
{
  PAssert(config != NULL, "config instance not set");
  config->Wait();

  PXConfig::iterator it;
  if ((it = config->find(theSection)) != config->end()) {
    config->RemoveAt(it->first);
    config->SetDirty();
  }

  config->Signal();
}


////////////////////////////////////////////////////////////
//
// PConfig::
//
// Delete the particular variable in the specified section.
//
////////////////////////////////////////////////////////////

void PConfig::DeleteKey(const PString & theSection, const PString & theKey)
{
  PAssert(config != NULL, "config instance not set");
  config->Wait();

  PXConfig::iterator it;
  if ((it = config->find(theSection)) != config->end()) {
    PStringToString & section = it->second;
    PStringToString::iterator it2;
    if ((it2 = section.find(theKey)) != section.end()) {
      section.RemoveAt(it2->first);
      config->SetDirty();
    }
  }

  config->Signal();
}



////////////////////////////////////////////////////////////
//
// PConfig::
//
// Test if there is a value for the key.
//
////////////////////////////////////////////////////////////

PBoolean PConfig::HasKey(const PString & theSection, const PString & theKey) const
{
  PAssert(config != NULL, "config instance not set");
  config->Wait();

  bool present = false;
  PXConfig::iterator it;
  if ((it = config->find(theSection)) != config->end())
    present = it->second.Contains(theKey);

  config->Signal();
  return present;
}



////////////////////////////////////////////////////////////
//
// PConfig::
//
// Get a string variable determined by the key in the section.
//
////////////////////////////////////////////////////////////

PString PConfig::GetString(const PString & theSection, const PString & theKey, const PString & dflt) const
{
  PAssert(config != NULL, "config instance not set");
  config->Wait();

  PString value = dflt;
  PXConfig::iterator it;
  if ((it = config->find(theSection)) != config->end()) {
    PStringToString & section = it->second;
    PStringToString::iterator it2;
    if ((it2 = section.find(theKey)) != section.end()) 
      value = it2->second;
  }

  config->Signal();
  return value;
}


////////////////////////////////////////////////////////////
//
// PConfig::
//
// Set a string variable determined by the key in the section.
//
////////////////////////////////////////////////////////////

void PConfig::SetString(const PString & theSection,
                        const PString & theKey,
                        const PString & theValue)
{
  PAssert(config != NULL, "config instance not set");
  config->Wait();

  PStringToString * section;

  PXConfig::iterator it;
  if ((it = config->find(theSection)) != config->end())
    section = &it->second;
  else {
    section = new PStringToString;
    config->SetAt(theSection, section);
    config->SetDirty();
  } 

  PStringToString::iterator it2;
  if ((it2 = section->find(theKey)) == section->end() || it2->second != theValue) {
    section->SetAt(theKey, theValue);
    config->SetDirty();
  }

  config->Signal();
}


///////////////////////////////////////////////////////////////////////////////
