/*
 * CONFIG.CXX
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 */

#define _CONFIG_CXX

#pragma implementation "config.h"

#include "ptlib.h"


#define SYS_CONFIG_NAME		"pwlib"

#define	APP_CONFIG_DIR		".pwlib_config/"
#define	SYS_CONFIG_DIR		"/usr/local/pwlib/"

#define	EXTENSION		".ini"

PDECLARE_CLASS (PXConfigValue, PCaselessString)
  public:
    PXConfigValue(const PString & theKey, const PString & theValue = "") 
      : PCaselessString(theKey), value(theValue) { }
    PString GetValue() const { return value; }
    void    SetValue(const PString & theValue) { value = theValue; }

  protected:
    PString  value;
};

PLIST (PXConfigSectionList, PXConfigValue);

PDECLARE_CLASS(PXConfigSection, PCaselessString)
  public:
    PXConfigSection(const PCaselessString & theName) 
      : PCaselessString(theName) { list.AllowDeleteObjects(); }
    PXConfigSectionList & GetList() { return list; }

  protected:
    PXConfigSectionList list;
};

#define	new PNEW

static BOOL ReadConfigFile (PFilePath & filename, PXConfig & config)

{
  PINDEX i;
  PINDEX len;
  PString line;
  PFile::Errors error;

  // attempt to open file
  PTextFile file;
  if (!file.Open(filename, PFile::ReadOnly))
    return FALSE;

  PXConfigSection * currentSection = NULL;

  // read lines in the file
  while (file.ReadLine(line)) {
    line.Trim();
    if ((len = line.GetLength()) > 0) {

      // ignore comments and blank lines 
      char ch = line[0];
      if ((len > 0) && (ch != ';') && (ch != '#')) {
        if (ch == '[') {
          PCaselessString sectionName = line.Mid(1,len-(line[len-1]==']'?2:1));
          PINDEX  index;
          if ((index = config.GetValuesIndex(sectionName)) != P_MAX_INDEX)
            currentSection = &config[index];
          else {
            currentSection = new PXConfigSection(sectionName);
            config.Append(currentSection);
          }
        } else if (currentSection != NULL) {
          PINDEX equals = line.Find('=');
          if (equals > 0) {
            PXConfigValue * value = new PXConfigValue(line.Left(equals), line.Right(len - equals - 1));
            currentSection->GetList().Append(value);
          }
        }
      }
    }
  }
  
  // close the file and return
  file.Close();
  return PFile::NoError;

}

static void GetEnvironment (char **envp, PXConfig & config)

{
  PXConfigSection * currentSection = new PXConfigSection("Options");
  config.Append(currentSection);

  char *line;

  while (*envp != NULL && **envp != '\0') {
    PString line(*envp);
    PINDEX equals = line.Find('=');
    if (equals > 0) {
      PXConfigValue * value = new PXConfigValue(line.Left(equals), line.Right(line.GetLength() - equals - 1));
      currentSection->GetList().Append(value);
    }
    envp++;
  }
}

////////////////////////////////////////////////////////////
//
// PConfig::
//
// Create a new configuration object
//
////////////////////////////////////////////////////////////

void PConfig::Construct(Source src)
{
  PString fname;

  config = new PXConfig;
  config->AllowDeleteObjects();
  dirty = FALSE;
  
  switch (src) {
    case PConfig::Environment:
      GetEnvironment(PProcess::Current()->GetEnvironment(), *config);
      return;

    case PConfig::System:
      filename = PProcess::Current()->GetHomeDir() +
                 APP_CONFIG_DIR + SYS_CONFIG_NAME + EXTENSION;
      if (!PFile::Exists(filename))
        return;
      break;

    case PConfig::Application:
    default:
      // search the current directory
      filename = PProcess::Current()->GetName() + EXTENSION;
      if (!PFile::Exists(filename)) {
        filename = PProcess::Current()->GetHomeDir() +
                   APP_CONFIG_DIR + 
                   PProcess::Current()->GetName() + EXTENSION;
        if (!PFile::Exists(filename))
          return;
      }
      break;
  }
  ReadConfigFile(filename, *config);
}

void PConfig::Construct(const PFilePath & theFilename)

{
  config = new PXConfig;
  config->AllowDeleteObjects();
  
  ReadConfigFile(filename = theFilename, *config);
  dirty = FALSE;
}

PConfig::~PConfig()

{
  if (dirty) {

    PTextFile file;
    if (!file.Open(filename, PFile::WriteOnly, PFile::Create))
      return;

    for (PINDEX i = 0; i < config->GetSize(); i++) {
      PXConfigSectionList & section = (*config)[i].GetList();
      file << "[" << (*config)[i] << "]" << endl;
      for (PINDEX j = 0; j < section.GetSize(); j++) {
        PXConfigValue & value = section[j];
        file << value << "=" << value.GetValue() << endl;
      }
      file << endl;
    }

    file.Close();
  }

  delete config;
}

////////////////////////////////////////////////////////////
//
// PConfig::
//
// Return a list of all the section names in the file.
//
////////////////////////////////////////////////////////////

PStringList PConfig::GetSections()
{
  PStringList list;

  for (PINDEX i = 0; i < list.GetSize(); i++)
    list.AppendString((*config)[i]);

  return list;
}


////////////////////////////////////////////////////////////
//
// PConfig::
//
// Return a list of all the keys in the section. If the section name is
// not specified then use the default section.
//
////////////////////////////////////////////////////////////

PStringList PConfig::GetKeys(const PString & theSection) const
{
  PINDEX index;
  PStringList list;

  if ((index = config->GetValuesIndex(theSection)) != P_MAX_INDEX) {
    PXConfigSectionList & section = (*config)[index].GetList();
    for (PINDEX i = 0; i < section.GetSize(); i++)
      list.AppendString(section[i]);
  }

  return list;
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
  PStringList list;

  PINDEX index;
  if ((index = config->GetValuesIndex(theSection)) != P_MAX_INDEX) 
    config->RemoveAt(index);
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
  PINDEX index;
  if ((index = config->GetValuesIndex(theSection)) != P_MAX_INDEX) {
    PXConfigSectionList & section = (*config)[index].GetList();
    PINDEX index_2;
    if ((index_2 = section.GetValuesIndex(theKey)) != P_MAX_INDEX) 
      section.RemoveAt(index_2);
  }
}



////////////////////////////////////////////////////////////
//
// PConfig::
//
// Get a string variable determined by the key in the section.
//
////////////////////////////////////////////////////////////

PString PConfig::GetString(const PString & theSection,
                                    const PString & theKey, const PString & dflt)
{
  PINDEX index;
  if ((index = config->GetValuesIndex(theSection)) == P_MAX_INDEX) 
    return dflt;

  PXConfigSectionList & section = (*config)[index].GetList();
  if ((index = section.GetValuesIndex(theKey)) == P_MAX_INDEX) 
    return dflt;

  return section[index].GetValue();
}


////////////////////////////////////////////////////////////
//
// PConfig::
//
// Set a string variable determined by the key in the section.
//
////////////////////////////////////////////////////////////

void PConfig::SetString(const PString & theSection,
                                   const PString & theKey, const PString & theValue)
{
  PINDEX index;
  PXConfigSection * section;
  PXConfigValue   * value;

  if ((index = config->GetValuesIndex(theSection)) != P_MAX_INDEX) 
    section = &(*config)[index];
  else {
    section = new PXConfigSection(theSection);
    config->Append(section);
    dirty = TRUE;
  } 

  if ((index = section->GetList().GetValuesIndex(theKey)) != P_MAX_INDEX) 
    value = &(section->GetList()[index]);
  else {
    value = new PXConfigValue(theKey);
    section->GetList().Append(value);
    dirty = TRUE;
  }

  if (theValue == value->GetValue())
    return;

  value->SetValue(theValue);
  dirty = TRUE;
}

#undef NEW

