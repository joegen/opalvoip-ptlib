/*
 * configure.cxx
 *
 * Build options generated by the configure script.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: configure.cpp,v $
 * Revision 1.8  2003/10/30 01:17:15  dereksmithies
 * Add fix from Jose Luis Urien. Many thanks.
 *
 * Revision 1.7  2003/10/23 21:49:51  dereksmithies
 * Add very sensible fix to limit extent of search. Thanks Ben Lear.
 *
 * Revision 1.6  2003/08/04 05:13:17  dereksmithies
 * Reinforce the disablement if the command lines specifies --no-XXXX to a feature.
 *
 * Revision 1.5  2003/08/04 05:07:08  dereksmithies
 * Command line option now disables feature when feature found on disk.
 *
 * Revision 1.4  2003/05/16 02:03:07  rjongbloed
 * Fixed being able to manually disable a "feature" when does a full disk search.
 *
 * Revision 1.3  2003/05/05 08:39:52  robertj
 * Added ability to explicitly disable a feature, or tell configure exactly
 *   where features library is so it does not need to search for it.
 *
 * Revision 1.2  2003/04/17 03:32:06  robertj
 * Improved windows configure program to use lines out of configure.in
 *
 * Revision 1.1  2003/04/16 08:00:19  robertj
 * Windoes psuedo autoconf support
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stdlib.h>
#include <windows.h>


using namespace std;


class Feature
{
  public:
    Feature() { found = false; }
    Feature(const string & line)
       : defineValue("1"),
         found(false)
    {
      int pos = 10;
      int comma;
      if ((comma = line.find(',', pos)) == string::npos)
        return;

      displayName = line.substr(pos, comma-pos);
      pos = comma+1;

      if ((comma = line.find(',', pos)) == string::npos)
        return;

      cmdLineArgument = line.substr(pos, comma-pos);
      pos = comma+1;

      if ((comma = line.find(',', pos)) == string::npos) {
        defineName = line.substr(pos);
        found = true;
        return;
      }

      if ((comma = line.find(',', pos)) == string::npos) {
        defineName = line.substr(pos);
        found = true;
        return;
      }

      defineName = line.substr(pos, comma-pos);
      pos = comma+1;

      if ((comma = line.find(',', pos)) == string::npos) {
        defineValue = line.substr(pos);
        found = true;
        return;
      }

      defineValue = line.substr(pos, comma-pos);
      pos = comma+1;

      if ((comma = line.find(',', pos)) == string::npos) {
        directoryName = line.substr(pos);
        return;
      }

      directoryName = line.substr(pos, comma-pos);
      pos = comma+1;

      if ((comma = line.find(',', pos)) == string::npos) {
        includeName = line.substr(pos);
        return;
      }

      includeName = line.substr(pos, comma-pos);
      pos = comma+1;

      if ((comma = line.find(',', pos)) == string::npos) {
        includeText = line.substr(pos);
        return;
      }

      includeText = line.substr(pos, comma-pos);
      pos = comma+1;

      while ((comma = line.find(',', pos)) != string::npos) {
        if (Locate(line.substr(pos, comma-pos).c_str()))
          return;
        pos = comma+1;
      }

      Locate(line.substr(pos).c_str());
    }

    virtual void Adjust(string & line)
    {
      if (found && line.find("#undef") != string::npos && line.find(defineName) != string::npos)
        line = "#define " + defineName + ' ' + defineValue;

      if (directoryName[0] != '\0') {
        int pos = line.find(directoryName);
        if (pos != string::npos)
          line.replace(pos, directoryName.length(), directory);
      }
    }

    virtual bool Locate(const char * testDir)
    {
      if (found)
        return true;

      if (defineName[0] == '\0') {
        found = false;
        return true;
      }

      if (includeName[0] == '\0') {
        found = true;
        return true;
      }

      string testDirectory = testDir;
      if (testDirectory[testDirectory.length()-1] != '\\')
        testDirectory += '\\';

      string filename = testDirectory + includeName;
      ifstream file(filename.c_str(), ios::in);
      if (!file.is_open())
        return false;

      if (includeText[0] == '\0')
        found = true;
      else {
        while (file.good()) {
          string line;
          getline(file, line);
          if (line.find(includeText) != string::npos) {
            found = true;
            break;
          }
        }
      }

      if (!found)
        return false;

      char buf[_MAX_PATH];
      _fullpath(buf, testDirectory.c_str(), _MAX_PATH);
      directory = buf;

      cout << "Located " << displayName << " at " << directory << endl;

      int pos;
      while ((pos = directory.find('\\')) != string::npos)
        directory[pos] = '/';
      pos = directory.length()-1;
      if (directory[pos] == '/')
        directory.erase(pos);

      return true;
    }

    string displayName;
    string cmdLineArgument;
    string defineName;
    string defineValue;
    string directoryName;
    string includeName;
    string includeText;

    bool found;
    string directory;
};


list<Feature> features;


bool TreeWalk(const string & directory)
{
  bool foundAll = false;

  string wildcard = directory;
  wildcard += "*.*";

  WIN32_FIND_DATA fileinfo;
  HANDLE hFindFile = FindFirstFile(wildcard.c_str(), &fileinfo);
  if (hFindFile != NULL) {
    do {
      string subdir = directory;
      subdir += fileinfo.cFileName;

      if ((fileinfo.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                                            fileinfo.cFileName[0] != '.') {
        subdir += '\\';

        foundAll = true;
        list<Feature>::iterator feature;
        for (feature = features.begin(); feature != features.end(); feature++) {
          if (!feature->Locate(subdir.c_str()))
            foundAll = false;
        }

        if (foundAll)
          break;

        TreeWalk(subdir);
      }
    } while (FindNextFile(hFindFile, &fileinfo));

    FindClose(hFindFile);
  }

  return foundAll;
}


bool ProcessHeader(const string & headerFilename)
{
  string inFilename = headerFilename;
  inFilename += ".in";

  ifstream in(inFilename.c_str(), ios::in);
  if (!in.is_open()) {
    cerr << "Could not open " << inFilename << endl;
    return false;
  }

  ofstream out(headerFilename.c_str(), ios::out);
  if (!out.is_open()) {
    cerr << "Could not open " << headerFilename << endl;
    return false;
  }

  while (in.good()) {
    string line;
    getline(in, line);

    list<Feature>::iterator feature;
    for (feature = features.begin(); feature != features.end(); feature++)
      feature->Adjust(line);

    out << line << '\n';
  }

  return !in.bad() && !out.bad();
}


int main(int argc, char* argv[])
{
  ifstream conf("configure.in", ios::in);
  if (!conf.is_open()) {
    cerr << "Could not open configure.in file" << endl;
    return 1;
  }

  list<string> headers;
  while (conf.good()) {
    string line;
    getline(conf, line);
    int pos;
    if ((pos = line.find("AC_CONFIG_HEADERS")) != string::npos) {
      pos = line.find('(', pos);
      if (pos != string::npos) {
        int end = line.find(')', pos);
        if (pos != string::npos)
          headers.push_back(line.substr(pos+1, end-pos-1));
      }
    }
    else if (line.find("dnl MSWIN ") == 0)
      features.push_back(Feature(line));
  }

  list<Feature>::iterator feature;

  const char EXTERN_DIR[] = "--extern-dir=";
  
  bool searchDisk = true;
  char *externDir = 0;
  for (int i = 1; i < argc; i++) {
    if (stricmp(argv[i], "--no-search") == 0)
      searchDisk = false;
    else if (strnicmp(argv[i], EXTERN_DIR, sizeof(EXTERN_DIR) - 1) == 0){
	    externDir = argv[i] + sizeof(EXTERN_DIR) - 1; 	
    }
    else if (stricmp(argv[i], "-h") == 0 || stricmp(argv[i], "--help") == 0) {
      cout << "usage: configure args\n"
              "  --no-search\t\tDo not search disk for libraries.\n"
	"  --extern-dir\t\t specify where to search disk for libraries.\n";
      for (feature = features.begin(); feature != features.end(); feature++) {
        if (feature->cmdLineArgument[0] != '\0') {
          cout << "  --no-" << feature->cmdLineArgument
               << "\t\tDisable " << feature->displayName << '\n';
          if (feature->includeName[0] != '\0')
            cout << "  --" << feature->cmdLineArgument << "-dir=dir"
                    "\tSet directory for " << feature->displayName << '\n';
        }
      }
      return 1;
    }
    else {
      for (feature = features.begin(); feature != features.end(); feature++) {
        if (stricmp(argv[i], ("--no-"+feature->cmdLineArgument).c_str()) == 0) {
	  feature->defineName = "";
          feature->found = false;
	} else if (strstr(argv[i], ("--" + feature->cmdLineArgument+"-dir=").c_str()) == argv[i])
	  if (!feature->Locate(argv[i] + strlen(("--" + feature->cmdLineArgument + "-dir=").c_str())))
	    cerr << feature->displayName << " not found in "
		 << argv[i] + strlen(("--" + feature->cmdLineArgument+"-dir=").c_str()) << endl;
      }
    }
  }

  if (searchDisk) {
    bool foundAll = true;
    for (feature = features.begin(); feature != features.end(); feature++) {
      if (!feature->found)
        foundAll = false;
    }

    if (!foundAll) {
      // Do search of entire system
      char drives[100];
      if (!externDir){
	if (!GetLogicalDriveStrings(sizeof(drives), drives))
	  strcpy(drives, "C:\\");
      } else {
	strcpy(drives, externDir);	
      }

      const char * drive = drives;
      while (*drive != '\0') {
        if (GetDriveType(drive) == DRIVE_FIXED) {
          cout << "Searching " << drive << endl;
          if (TreeWalk(drive))
            break;
        }
        drive += strlen(drive)+1;
      }
    }
  }

  for (feature = features.begin(); feature != features.end(); feature++) {
    cout << "  " << feature->displayName << ' ';
    if (feature->includeName[0] == '\0')
      cout << "set to " << feature->defineValue;
    else if (feature->found)
      cout << "enabled";
    else
      cout << "disabled";
    cout << '\n';
  }
  cout << endl;

  for (list<string>::iterator hdr = headers.begin(); hdr != headers.end(); hdr++)
    ProcessHeader(*hdr);

  cout << "Configuration completed.\n";

  return 0;
}
