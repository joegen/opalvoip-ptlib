/*
 * vidinput_names.cxx
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2000 Equivalence Pty. Ltd.
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
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *                 Mark Cooke (mpc@star.sr.bham.ac.uk)
 *                 Nicola Orru' <nigu@itadinanta.it>
 *
 * $Revision$
 * $Author$
 * $Date$
 */
#include "vidinput_names.h"

void  V4LXNames::ReadDeviceDirectory(PDirectory devdir, POrdinalToString & vid)
{
  if (!devdir.Open())
    return;

#if defined (P_SOLARIS) || defined (P_NETBSD)
  int devnum = 0;
  do {
    PString filename = devdir.GetEntryName();
    if (!filename.NumCompare("video", 5 , 0)) {
      PString devname = devdir + filename;
      struct stat s;
      if (lstat(devname, &s) == 0) {
        vid.SetAt(devnum++, devname);
      }
    }
  } while (devdir.Next());
#else  
  do {
    PString filename = devdir.GetEntryName();
    PString devname = devdir + filename;
    if (devdir.IsSubDir())
      ReadDeviceDirectory(devname, vid);
    else {

      PFileInfo info;
      if (devdir.GetInfo(info) && info.type == PFileInfo::CharDevice) {
        struct stat s;
        if (lstat(devname, &s) == 0) {
 
          static const int deviceNumbers[] = { 81 };
          for (PINDEX i = 0; i < PARRAYSIZE(deviceNumbers); i++) {
            if (MAJOR(s.st_rdev) == deviceNumbers[i]) {
              PINDEX num = MINOR(s.st_rdev);
              if (num <= 63 && num >= 0) {
                vid.SetAt(num, devname);
              }
            }
          }
        }
      }
    }
  } while (devdir.Next());
#endif  
}

void V4LXNames::PopulateDictionary()
{
  PWaitAndSignal m(mutex);

  PStringToString tempList;

  for (PStringList::iterator it = inputDeviceNames.begin(); it != inputDeviceNames.end(); ++it)
    tempList.SetAt(*it, BuildUserFriendly(*it));

  //Now, we need to cope with the case where there are two video
  //devices available, which both have the same user friendly name.
  //Matching user friendly names have a (X) appended to the name.
  for (PStringToString::iterator it1 = tempList.begin(); it1 != tempList.end(); ++it1) {
    PString userName = it1->second; 

    PINDEX matches = 1;
    for (PStringToString::iterator it2 = tempList.begin(); it2 != tempList.end(); ++it2) {
      if (it2->second == userName) {
        matches++;
        PStringStream revisedUserName;
        revisedUserName << userName << " (" << matches << ")";
        tempList.SetAt(it2->first, revisedUserName);
      }
    }
  }

  //At this stage, we have correctly modified the temp list of names.
  for (PStringToString::iterator it = tempList.begin(); it != tempList.end(); ++it)
    AddUserDeviceName(it->second, it->first);
}

PString V4LXNames::GetUserFriendly(PString devName)
{
  PWaitAndSignal m(mutex);

  
  PString result= deviceKey(devName);
  if (result.IsEmpty())
    return devName;

  return result;
}

PString V4LXNames::GetDeviceName(PString userName)
{
  PWaitAndSignal m(mutex);

  for (PStringToString::iterator it = userKey.begin(); it != userKey.end(); ++it)
    if (it->first.Find(userName) != P_MAX_INDEX)
      return it->second;

  return userName;
}

void V4LXNames::AddUserDeviceName(PString userName, PString devName)
{
  PWaitAndSignal m(mutex);

  if (userName != devName) { // must be a real userName!
    userKey.SetAt(userName, devName);
    deviceKey.SetAt(devName, userName);
  } else { // we didn't find a good userName
    if (!deviceKey.Contains (devName)) { // never met before: fallback
      userKey.SetAt(userName, devName);
      deviceKey.SetAt(devName, userName);
    } // no else: we already know the pair
  }
}


PStringList V4LXNames::GetInputDeviceNames()
{
  PWaitAndSignal m(mutex);
  PStringList result;
  for (PINDEX i = 0; i < inputDeviceNames.GetSize(); i++) {
    result += GetUserFriendly (inputDeviceNames[i]);
  }
  return result;
}
