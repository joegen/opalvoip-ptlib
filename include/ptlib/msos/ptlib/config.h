/*
 * $Id: config.h,v 1.5 1998/08/20 06:03:54 robertj Exp $
 */


#ifndef _PCONFIG


///////////////////////////////////////////////////////////////////////////////
// PConfiguration

#include "../../common/ptlib/config.h"
  protected:
    Source  source;
    PString location;
};


class RegistryKey
{
  public:
    enum OpenMode {
      ReadOnly,
      ReadWrite,
      Create
    };
    RegistryKey(const PString & subkey, OpenMode mode);
    ~RegistryKey();

    BOOL EnumKey(PINDEX idx, PString & str);
    BOOL EnumValue(PINDEX idx, PString & str);
    BOOL DeleteKey(const PString & subkey);
    BOOL DeleteValue(const PString & value);
    BOOL QueryValue(const PString & value, PString & str);
    BOOL QueryValue(const PString & value, DWORD & num, BOOL boolean);
    BOOL SetValue(const PString & value, const PString & str);
    BOOL SetValue(const PString & value, DWORD num);
  private:
    HKEY key;
};


#endif
