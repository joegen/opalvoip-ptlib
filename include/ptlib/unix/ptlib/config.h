/*
 * $Id: config.h,v 1.5 1996/09/21 05:42:12 craigs Exp $
 */

#ifndef _PCONFIG

#pragma interface

class PXConfigSection;
PLIST(PXConfig, PXConfigSection);

///////////////////////////////////////////////////////////////////////////////
// PConfiguration

#include "../../common/ptlib/config.h"
  public:
    PConfig(int, const PString & name);
    ~PConfig();

  protected:
    PFilePath  filename;
    PXConfig * config;
    BOOL       dirty;
    BOOL       saveOnExit;
};


#endif
