/*
 * wincfg.cxx
 *
 * Miscellaneous implementation of classes for Win32
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
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>

#include <winuser.h>
#include <winnls.h>

#define new PNEW


static PConstString const LocalMachineStr("HKEY_LOCAL_MACHINE\\");
static PConstString const LocalMach64Str ("HKEY_LOCAL_MACH64\\");
static PConstString const CurrentUserStr ("HKEY_CURRENT_USER\\");
static PConstString const ClassesRootStr ("HKEY_CLASSES_ROOT\\");

///////////////////////////////////////////////////////////////////////////////
// Configuration files

class SecurityID
{
  public:
    SecurityID(PSID_IDENTIFIER_AUTHORITY  pIdentifierAuthority,  // pointer to identifier authority
               BYTE nSubAuthorityCount,  // count of subauthorities
               DWORD dwSubAuthority0,  // subauthority 0
               DWORD dwSubAuthority1,  // subauthority 1
               DWORD dwSubAuthority2,  // subauthority 2
               DWORD dwSubAuthority3,  // subauthority 3
               DWORD dwSubAuthority4,  // subauthority 4
               DWORD dwSubAuthority5,  // subauthority 5
               DWORD dwSubAuthority6,  // subauthority 6
               DWORD dwSubAuthority7  // subauthority 7
              )
    {
      if (!AllocateAndInitializeSid(pIdentifierAuthority,  // pointer to identifier authority
                                    nSubAuthorityCount,  // count of subauthorities
                                    dwSubAuthority0,  // subauthority 0
                                    dwSubAuthority1,  // subauthority 1
                                    dwSubAuthority2,  // subauthority 2
                                    dwSubAuthority3,  // subauthority 3
                                    dwSubAuthority4,  // subauthority 4
                                    dwSubAuthority5,  // subauthority 5
                                    dwSubAuthority6,  // subauthority 6
                                    dwSubAuthority7,  // subauthority 7
                                    &sidptr))
        sidptr = NULL;
    }

    SecurityID(LPCTSTR lpSystemName,  // address of string for system name
               LPCTSTR lpAccountName,  // address of string for account name
               LPTSTR ReferencedDomainName,  // address of string for referenced domain 
               LPDWORD cbReferencedDomainName,  // address of size of domain string
               PSID_NAME_USE peUse   // address of SID-type indicator
              )
    {
      DWORD len = 1024;
      sidptr = (PSID)LocalAlloc(LPTR, len);
      if (sidptr != NULL) {
        if (!LookupAccountName(lpSystemName,  // address of string for system name
                               lpAccountName,  // address of string for account name
                               sidptr,  // address of security identifier
                               &len,  // address of size of security identifier
                               ReferencedDomainName,  // address of string for referenced domain 
                               cbReferencedDomainName,  // address of size of domain string
                               peUse   // address of SID-type indicator
                              )) {
          LocalFree(sidptr);
          sidptr = NULL;
        }
      }
    }
    ~SecurityID()
    {
      FreeSid(sidptr);
    }

    operator PSID() const
    {
      return sidptr;
    }

    DWORD GetLength() const
    {
      return GetLengthSid(sidptr);
    }

    PBoolean IsValid() const
    {
      return sidptr != NULL && IsValidSid(sidptr);
    }

  private:
    PSID sidptr;
};


static DWORD SecureCreateKey(HKEY rootKey, const PString & subkey, HKEY & key)
{
  SECURITY_DESCRIPTOR secdesc;
  if (!InitializeSecurityDescriptor(&secdesc, SECURITY_DESCRIPTOR_REVISION))
    return GetLastError();

  static SID_IDENTIFIER_AUTHORITY siaNTAuthority = { SECURITY_NT_AUTHORITY };
  SecurityID adminID(&siaNTAuthority, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS, 
                     0, 0, 0, 0, 0, 0);
  if (!adminID.IsValid())
    return GetLastError();

  static SID_IDENTIFIER_AUTHORITY siaSystemAuthority = { SECURITY_NT_AUTHORITY };
  SecurityID systemID(&siaSystemAuthority, 1,
                      SECURITY_LOCAL_SYSTEM_RID,
                      0, 0, 0, 0, 0, 0, 0);
  if (!systemID.IsValid())
    return GetLastError();

  static SID_IDENTIFIER_AUTHORITY siaCreatorAuthority = { SECURITY_CREATOR_SID_AUTHORITY };
  SecurityID creatorID(&siaCreatorAuthority, 1,
                       SECURITY_CREATOR_OWNER_RID,
                       0, 0, 0, 0, 0, 0, 0);
  if (!creatorID.IsValid())
    return GetLastError();

  SID_NAME_USE snuType;
  char szDomain[100];
  DWORD cchDomainName = sizeof(szDomain);
  SecurityID userID(NULL, PProcess::Current().GetUserName(),
                    szDomain, &cchDomainName, &snuType);
  if (!userID.IsValid())
    return GetLastError();

  DWORD acl_len = sizeof(ACL) + 4*sizeof(ACCESS_ALLOWED_ACE) +
                    adminID.GetLength() + creatorID.GetLength() +
                    systemID.GetLength() + userID.GetLength();
  PBYTEArray dacl_buf(acl_len);
  PACL dacl = (PACL)dacl_buf.GetPointer(acl_len);
  if (!InitializeAcl(dacl, acl_len, ACL_REVISION2))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, adminID))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, systemID))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, creatorID))
    return GetLastError();

  if (!AddAccessAllowedAce(dacl, ACL_REVISION2, KEY_ALL_ACCESS, userID))
    return GetLastError();

  if (!SetSecurityDescriptorDacl(&secdesc, true, dacl, false))
    return GetLastError();

  SECURITY_ATTRIBUTES secattr;
  secattr.nLength = sizeof(secattr);
  secattr.lpSecurityDescriptor = &secdesc;
  secattr.bInheritHandle = false;

  DWORD disposition;

  return RegCreateKeyEx(rootKey, subkey, 0, (char*) "", REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, &secattr, &key, &disposition);
}


RegistryKey::RegistryKey(const PString & subkeyname, OpenMode mode)
{
  PAssert(!subkeyname.IsEmpty(), PInvalidParameter);

  PProcess & proc = PProcess::Current();
  DWORD access = mode == ReadOnly ? KEY_READ : KEY_ALL_ACCESS;
  DWORD error;

  PVarString subkey;
  HKEY basekey;
  if (subkeyname.NumCompare(LocalMachineStr) == PObject::EqualTo) {
    subkey = subkeyname.Mid(LocalMachineStr.GetLength());
    basekey = HKEY_LOCAL_MACHINE;
  }
  else if (subkeyname.NumCompare(LocalMach64Str) == PObject::EqualTo) {
    subkey = subkeyname.Mid(LocalMach64Str.GetLength());
    basekey = HKEY_LOCAL_MACHINE;
    access |= KEY_WOW64_64KEY;
  }
  else if (subkeyname.NumCompare(CurrentUserStr) == PObject::EqualTo) {
    subkey = subkeyname.Mid(CurrentUserStr.GetLength());
    basekey = HKEY_CURRENT_USER;
  }
  else if (subkeyname.NumCompare(ClassesRootStr) == PObject::EqualTo) {
    subkey = subkeyname.Mid(ClassesRootStr.GetLength());
    basekey = HKEY_CLASSES_ROOT;
  }
  else {
    PString adjustedSubkey = subkeyname;
    PINDEX lastCharPos = adjustedSubkey.GetLength()-1;
    while (lastCharPos > 0 && adjustedSubkey[lastCharPos] == '\\')
      adjustedSubkey.Delete(lastCharPos--, 1);
    basekey = NULL;

    subkey = adjustedSubkey;

    if (!proc.GetVersion(false).IsEmpty()) {
      adjustedSubkey.Replace("CurrentVersion", proc.GetVersion(false));
      PVarString keyname = adjustedSubkey;

      error = RegOpenKeyEx(HKEY_CURRENT_USER, keyname, 0, access, &key);
      if (error == ERROR_SUCCESS)
        return;

      PTRACE_IF(1, error == ERROR_ACCESS_DENIED, "PTLib\tAccess denied accessing registry entry HKEY_CURRENT_USER\\" << keyname);

      error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyname, 0, access, &key);
      if (error == ERROR_SUCCESS)
        return;

      PTRACE_IF(1, error == ERROR_ACCESS_DENIED, "PTLib\tAccess denied accessing registry entry HKEY_LOCAL_MACHINE\\" << keyname);
    }

    error = RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, access, &key);
    if (error == ERROR_SUCCESS)
      return;

    PTRACE_IF(1, error == ERROR_ACCESS_DENIED, "PTLib\tAccess denied accessing registry entry HKEY_CURRENT_USER\\" << subkey);
  }

  error = RegOpenKeyEx(basekey != NULL ? basekey : HKEY_LOCAL_MACHINE, subkey, 0, access, &key);
  if (error == ERROR_SUCCESS)
    return;

  PTRACE_IF(1, error == ERROR_ACCESS_DENIED, "PTLib\tAccess denied accessing registry entry "
            << (basekey != NULL ? "" : LocalMachineStr) << subkey);

  key = NULL;
  if (mode != Create)
    return;

  if (basekey == NULL) {
    if (PProcess::Current().IsServiceProcess())
      basekey = HKEY_LOCAL_MACHINE;
    else
      basekey = HKEY_CURRENT_USER;
  }

  if (basekey != HKEY_CURRENT_USER)
    error = SecureCreateKey(basekey, subkey, key);
  if (error != ERROR_SUCCESS) {
    DWORD disposition;
    TCHAR empty[1];
    empty[0] = 0;
    error = RegCreateKeyEx(basekey, subkey, 0, empty, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &disposition);
    if (error != ERROR_SUCCESS) {
      PTRACE(1, "PTLib\tCould not create registry entry "
             << (basekey == HKEY_LOCAL_MACHINE ? LocalMachineStr :
                (basekey == HKEY_CURRENT_USER ? CurrentUserStr : "")) << subkey);
      key = NULL;
    }
  }
}


RegistryKey::~RegistryKey()
{
  if (key != NULL)
    RegCloseKey(key);
}


BOOL RegistryKey::EnumKey(PINDEX idx, PString & str)
{
  if (key == NULL)
    return false;

  TCHAR buffer[_MAX_PATH];
  if (RegEnumKey(key, idx, buffer, sizeof(buffer)) != ERROR_SUCCESS)
    return false;

  str = buffer;
  return true;
}


BOOL RegistryKey::EnumValue(PINDEX idx, PString & str)
{
  if (key == NULL)
    return false;

  TCHAR buffer[_MAX_PATH];
  DWORD size = _MAX_PATH;
  if (RegEnumValue(key, idx, buffer, &size, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    return false;

  str = buffer;
  return true;
}


BOOL RegistryKey::DeleteKey(const PString & subkey)
{
  if (key == NULL)
    return true;

  return RegDeleteKey(key, PVarString(subkey)) == ERROR_SUCCESS;
}


BOOL RegistryKey::DeleteValue(const PString & valueName)
{
  if (key == NULL)
    return true;

  return RegDeleteValue(key, PVarString(valueName)) == ERROR_SUCCESS;
}


BOOL RegistryKey::QueryValue(const PString & valueName, PString & str)
{
  if (key == NULL)
    return false;

  PVarString varValueName = valueName;

  DWORD type, size, num;
  DWORD error = RegQueryValueEx(key, varValueName, NULL, &type, NULL, &size);
  if (error != ERROR_SUCCESS)
    return false;

  if (type == REG_DWORD) {
    size = sizeof(num);
    if (RegQueryValueEx(key, varValueName, NULL, &type, (LPBYTE)&num, &size) != ERROR_SUCCESS)
      return false;

    str = PString(PString::Signed, num);
    return true;
  }

  if (type != REG_SZ && type != REG_MULTI_SZ && type != REG_EXPAND_SZ && type != REG_BINARY) {
    PAssertAlways("Unsupported registry type.");
    return false;
  }

  if (RegQueryValueEx(key, varValueName, NULL, &type,
                      (LPBYTE)str.GetPointerAndSetLength(size), &size) != ERROR_SUCCESS)
    return false;

  if (type == REG_BINARY)
    return true;

  if (str[size-1] == '\0')
    str.GetPointerAndSetLength(size-1);
  return true;
}


BOOL RegistryKey::QueryValue(const PString & valueName, DWORD & num, BOOL boolean)
{
  if (key == NULL)
    return false;

  DWORD type, size;
  PVarString varValueName = valueName;

  if (RegQueryValueEx(key, varValueName, NULL, &type, NULL, &size) != ERROR_SUCCESS)
    return false;


  switch (type) {
    case REG_BINARY :
      if (size > sizeof(DWORD))
        return false;

      num = 0;
      // Do REG_DWORD case

    case REG_DWORD :
      return RegQueryValueEx(key, varValueName, NULL, &type, (LPBYTE)&num, &size) == ERROR_SUCCESS;

    case REG_SZ : {
      TCHAR buffer[100];
      if (RegQueryValueEx(key, varValueName, NULL, &type, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
        PString str = buffer;
        num = str.AsInteger();
        if (num == 0 && boolean) {
          int c = toupper(str[0]);
          num = c == 'T' || c == 'Y';
        }
        return true;
      }
      break;
    }
    default :
      PAssertAlways("Unsupported registry type.");
  }

  return false;
}


BOOL RegistryKey::SetValue(const PString & valueName, const PString & str)
{
  if (key == NULL)
    return false;

  PVarString vstr(str);
  return RegSetValueEx(key, PVarString(valueName), 0, REG_SZ,
                       (LPBYTE)(const TCHAR *)vstr, vstr.GetLength()+1) == ERROR_SUCCESS;
}


BOOL RegistryKey::SetValue(const PString & value, DWORD num)
{
  if (key == NULL)
    return false;

  return RegSetValueEx(key, PVarString(value), 0, REG_DWORD, (LPBYTE)&num, sizeof(num)) == ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////

#if P_CONFIG_FILE

static PBoolean IsRegistryPath(PString & path)
{
  if (path.NumCompare(LocalMachineStr) != PObject::EqualTo &&
      path.NumCompare(LocalMach64Str ) != PObject::EqualTo &&
      path.NumCompare(CurrentUserStr ) != PObject::EqualTo)
    return false;

  if (path[path.GetLength() - 1] != '\\')
    path += '\\';

  return true;
}


static PString GetDefaultRegistryPath(const PString & manuf, const PString & appname)
{
  PStringStream regPath;

  regPath << CurrentUserStr << "SOFTWARE\\" << manuf;

  if (!manuf.IsEmpty() && !appname.IsEmpty())
    regPath << '\\';

  if (!appname.IsEmpty())
    regPath << appname;
  else if (manuf.IsEmpty())
    regPath << "PTLib";

  regPath << "\\CurrentVersion\\";

  return regPath;
}


///////////////////////////////////////////////////////////////////////////////

PString PProcess::GetConfigurationFile()
{
  // No paths set, use defaults
  if (m_configurationPaths.IsEmpty()) {
    m_configurationPaths.AppendString(m_executableFile.GetVolume() + m_executableFile.GetPath());
    m_configurationPaths.AppendString(GetDefaultRegistryPath(GetManufacturer(), GetName()));
  }

  // See if explicit filename
  if (m_configurationPaths.GetSize() == 1 && !PDirectory::Exists(m_configurationPaths[0]))
    return m_configurationPaths[0];

  PString iniFilename = m_executableFile.GetTitle() + ".INI";

  for (PINDEX i = 0; i < m_configurationPaths.GetSize(); i++) {
    PString path = m_configurationPaths[i];
    if (IsRegistryPath(path))
      return path;
    PFilePath cfgFile = PDirectory(path) + iniFilename;
    if (PFile::Exists(cfgFile))
      return cfgFile;
  }

  return PString();
}


///////////////////////////////////////////////////////////////////////////////

void PConfig::Construct(Source src, const PString & appname, const PString & manuf)
{
  source = Application;

  switch (src) {
    case System :
      location = appname;
      if (!IsRegistryPath(location)) {
        TCHAR dir[_MAX_PATH];
        GetWindowsDirectory(dir, sizeof(dir));
        Construct(PDirectory(PString(dir))+"WIN.INI");
      }
      break;

    case Application :
      if (IsRegistryPath(defaultSection))
        location = PString();
      else {
        PProcess & proc = PProcess::Current();
        if (!manuf.IsEmpty() || !appname.IsEmpty())
          location = GetDefaultRegistryPath(manuf, appname);
        else {
          PString cfgPath = proc.GetConfigurationFile();
          if (cfgPath.IsEmpty())
            location = GetDefaultRegistryPath(manuf, appname);
          else {
            if (!IsRegistryPath(cfgPath))
              source = NumSources; // Make a file based config
            location = cfgPath;
          }
        }
      }
      break;

    default :
      source = src;
  }
}


void PConfig::Construct(const PFilePath & filename)
{
  location = filename;
  source = NumSources;
}


static void RecurseRegistryKeys(const PString & location,
                                PINDEX baseLength,
                                PStringList &sections)
{
  RegistryKey registry(location, RegistryKey::ReadOnly);
  PString name;
  for (PINDEX idx = 0; registry.EnumKey(idx, name); idx++) {
    RecurseRegistryKeys(location + name + '\\', baseLength, sections);
    sections.AppendString(location.Mid(baseLength) + name);
  }
}


static PString PGetPrivateProfileString(const char * lpAppName,
                                           const char * lpKeyName,
                                           const char * lpDefault,
                                           const char * lpFileName)
{
  PString buffer;

  DWORD numNulls = lpAppName != NULL && lpKeyName != NULL ? 1 : 2;
  DWORD size = 100;
  while (size <= 100000 &&
                ::GetPrivateProfileString(lpAppName, lpKeyName, lpDefault,
                                          buffer.GetPointerAndSetLength(size+numNulls), size+numNulls,
                                          lpFileName) == size)
    size *= 10;

  return buffer;
}


PStringArray PConfig::GetSections() const
{
  PStringList sections;

  switch (source) {
    case Application :
      RecurseRegistryKeys(location, location.GetLength(), sections);
      break;

    case NumSources :
      {
        PCharArray buffer = PGetPrivateProfileString(NULL, NULL, "", location);
        char * ptr = buffer.GetPointer();
        while (*ptr != '\0') {
          sections.AppendString(ptr);
          ptr += strlen(ptr)+1;
        }
      }
      break;

    case Environment :
    case System :
      break;
  }

  return sections;
}


PStringArray PConfig::GetKeys(const PString & section) const
{
  PStringList keys;

  switch (source) {
    case Environment : {
      char ** ptr = _environ;
      while (*ptr != NULL) {
        PString buf = *ptr++;
        keys.AppendString(buf.Left(buf.Find('=')));
      }
      break;
    }

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::ReadOnly);
      PString name;
      for (PINDEX idx = 0; registry.EnumValue(idx, name); idx++)
        keys.AppendString(name);
      break;
    }

    case NumSources :
      PAssert(!section.IsEmpty(), PInvalidParameter);
      {
        PCharArray buffer = PGetPrivateProfileString(section, NULL, "", location);
        char * ptr = buffer.GetPointer();
        while (*ptr != '\0') {
          keys.AppendString(ptr);
          ptr += strlen(ptr)+1;
        }
      }
      break;

    case System :
      break;
  }

  return keys;
}


void PConfig::DeleteSection(const PString & section)
{
  switch (source) {
    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location, RegistryKey::ReadWrite);
      registry.DeleteKey(section);
      break;
    }

    case NumSources :
      PAssert(!section.IsEmpty(), PInvalidParameter);
      PAssertOS(WritePrivateProfileString(section, NULL, NULL, location));
      break;

    case Environment :
    case System :
      break;
  }
}


void PConfig::DeleteKey(const PString & section, const PString & key)
{
  switch (source) {
    case Environment :
      PAssert(!key.IsEmpty() && key.Find('=') == P_MAX_INDEX, PInvalidParameter);
      _putenv(key + "=");
      break;

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::ReadWrite);
      registry.DeleteValue(key);
      break;
    }

    case NumSources :
      PAssert(!key.IsEmpty(), PInvalidParameter);
      PAssert(!section.IsEmpty(), PInvalidParameter);
      PAssertOS(WritePrivateProfileString(section, key, NULL, location));
      break;

    case System :
      break;
  }
}


PBoolean PConfig::HasKey(const PString & section, const PString & key) const
{
  switch (source) {
    case Environment :
      PAssert(!key.IsEmpty(), PInvalidParameter);
      return getenv(key) != NULL;

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::ReadOnly);
      PString dummy;
      return registry.QueryValue(key, dummy);
    }

    case NumSources :
      {
        PAssert(!key.IsEmpty() && !section.IsEmpty(), PInvalidParameter);
        static const char dflt[] = "<<<<<====---PConfig::DefaultValueString---====>>>>>";
        return PGetPrivateProfileString(section, key, dflt, location) != dflt;
      }

    case System :
      break;
  }

  return false;
}


PString PConfig::GetString(const PString & section,
                               const PString & key, const PString & dflt) const
{
  PString str;

  switch (source) {
    case Environment : {
      PAssert(!key.IsEmpty() && key.Find('=') == P_MAX_INDEX, PInvalidParameter);
      char * env = getenv(key);
      if (env != NULL)
        str = env;
      else
        str = dflt;
      break;
    }

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::ReadOnly);
      if (!registry.QueryValue(key, str))
        str = dflt;
      break;
    }

    case NumSources :
      PAssert(!key.IsEmpty() && !section.IsEmpty(), PInvalidParameter);
      str = PString(PGetPrivateProfileString(section, key, dflt, location));
      str.MakeMinimumSize();
      break;

    case System :
      break;
  }

  return str;
}


void PConfig::SetString(const PString & section,
                                    const PString & key, const PString & value)
{
  switch (source) {
    case Environment :
      PAssert(!key.IsEmpty() && key.Find('=') == P_MAX_INDEX, PInvalidParameter);
      _putenv(key + "=" + value);
      break;

    case Application : {
      PAssert(!section.IsEmpty(), PInvalidParameter);
      RegistryKey registry(location + section, RegistryKey::Create);
      registry.SetValue(key, value);
      break;
    }

    case NumSources :
      PAssert(!key.IsEmpty() && !section.IsEmpty(), PInvalidParameter);
      PAssertOS(WritePrivateProfileString(section, key, value, location));
      break;

    case System :
      break;
  }
}


PBoolean PConfig::GetBoolean(const PString & section,
                                          const PString & key, PBoolean dflt) const
{
  if (source != Application) {
    PString str = GetString(section, key, dflt ? "T" : "F").ToUpper();
    int c = toupper(str[0]);
    return c == 'T' || c == 'Y' || str.AsInteger() != 0;
  }

  PAssert(!section.IsEmpty(), PInvalidParameter);
  RegistryKey registry(location + section, RegistryKey::ReadOnly);

  DWORD value;
  if (!registry.QueryValue(key, value, true))
    return dflt;

  return value != 0;
}


void PConfig::SetBoolean(const PString & section, const PString & key, PBoolean value)
{
  if (source != Application)
    SetString(section, key, value ? "True" : "False");
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    RegistryKey registry(location + section, RegistryKey::Create);
    registry.SetValue(key, value ? 1 : 0);
  }
}


long PConfig::GetInteger(const PString & section,
                                          const PString & key, long dflt) const
{
  if (source != Application) {
    PString str(PString::Signed, dflt);
    return GetString(section, key, str).AsInteger();
  }

  PAssert(!section.IsEmpty(), PInvalidParameter);
  RegistryKey registry(location + section, RegistryKey::ReadOnly);

  DWORD value;
  if (!registry.QueryValue(key, value, false))
    return dflt;

  return value;
}


void PConfig::SetInteger(const PString & section, const PString & key, long value)
{
  if (source != Application) {
    PString str(PString::Signed, value);
    SetString(section, key, str);
  }
  else {
    PAssert(!section.IsEmpty(), PInvalidParameter);
    RegistryKey registry(location + section, RegistryKey::Create);
    registry.SetValue(key, value);
  }
}

#endif // P_CONFIG_FILE


// End Of File ///////////////////////////////////////////////////////////////
