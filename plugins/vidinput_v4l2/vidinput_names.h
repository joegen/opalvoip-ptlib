#ifndef _VIDINPUTNAMES_H
#define _VIDINPUTNAMES_H


#include <ptlib.h>
#include <ptlib/videoio.h>
#ifndef MAJOR
#define MAJOR(a) (int)((unsigned short) (a) >> 8)
#endif

#ifndef MINOR
#define MINOR(a) (int)((unsigned short) (a) & 0xFF)
#endif

class V4LXNames : public PObject
{ 
  PCLASSINFO(V4LXNames, PObject);

 public:

  V4LXNames() {/* nothing */};

  virtual void Update () = 0;
  
  PString GetUserFriendly(PString devName);

  PString GetDeviceName(PString userName);

  PStringList GetInputDeviceNames();

protected:

  void AddUserDeviceName(PString userName, PString devName);

  virtual PString BuildUserFriendly(PString devname) = 0;

  void PopulateDictionary();

  void ReadDeviceDirectory(PDirectory devdir, POrdinalToString & vid);

  PMutex          mutex;
  PStringToString deviceKey;
  PStringToString userKey;
  PStringList     inputDeviceNames;

};

#endif
