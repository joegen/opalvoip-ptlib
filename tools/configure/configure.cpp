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
    Feature() : defineValue("1") { found = false; }

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

    bool IsFound() const { return found; }

    friend ostream & operator<<(ostream & stream, const Feature & feature)
    {
      stream << feature.displayName << ' ';
      if (feature.found)
        stream << "enabled";
      else
        stream << "disabled";
      return stream;
    }

    string displayName;
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
  bool searchDisk = true;
  for (int i = 1; i < argc; i++) {
    if (stricmp(argv[i], "--no-search") == 0)
      searchDisk = false;
  }

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
    else if ((pos = line.find("dnl MSWIN ")) != string::npos) {
      pos += 10;
      int comma = line.find(',', pos);
      if (comma != string::npos) {
        Feature feature;
        feature.displayName = line.substr(pos, comma-pos);
        pos = comma+1;
        if ((comma = line.find(',', pos)) == string::npos) {
          feature.defineName = line.substr(pos);
          feature.found = true;
        }
        else {
          feature.defineName = line.substr(pos, comma-pos);
          pos = comma+1;
          if ((comma = line.find(',', pos)) == string::npos) {
            feature.defineValue = line.substr(pos);
            feature.found = true;
          }
          else {
            feature.defineValue = line.substr(pos, comma-pos);
            pos = comma+1;
            if ((comma = line.find(',', pos)) == string::npos)
              feature.directoryName = line.substr(pos);
            else {
              feature.directoryName = line.substr(pos, comma-pos);
              pos = comma+1;
              if ((comma = line.find(',', pos)) == string::npos)
                feature.includeName = line.substr(pos);
              else {
                feature.includeName = line.substr(pos, comma-pos);
                pos = comma+1;
                if ((comma = line.find(',', pos)) == string::npos)
                  feature.includeText = line.substr(pos);
                else {
                  feature.includeText = line.substr(pos, comma-pos);
                  feature.Locate(line.substr(comma+1).c_str());
                }
              }
            }
          }
        }
        features.push_back(feature);
      }
    }
  }

  list<Feature>::iterator feature;

  if (searchDisk) {
    bool foundAll = true;
    for (feature = features.begin(); feature != features.end(); feature++) {
      if (!feature->IsFound())
        foundAll = false;
    }

    if (!foundAll) {
      // Do search of entire system
      char drives[100];
      if (!GetLogicalDriveStrings(sizeof(drives), drives))
        strcpy(drives, "C:\\");

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

  for (feature = features.begin(); feature != features.end(); feature++)
    cout << "  " << *feature << '\n';
  cout << endl;

  for (list<string>::iterator hdr = headers.begin(); hdr != headers.end(); hdr++)
    ProcessHeader(*hdr);

  cout << "Configuration completed:\n";

  return 0;
}
