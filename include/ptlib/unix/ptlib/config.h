/*
 * $Id: config.h,v 1.4 1996/08/03 12:08:19 craigs Exp $
 */

#ifndef _PCONFIG

#pragma interface

class PXConfigSection;
PLIST(PXConfig, PXConfigSection);

///////////////////////////////////////////////////////////////////////////////
// PConfiguration

#include "../../common/ptlib/config.h"
  public:
    ~PConfig();

  protected:
    PFilePath  filename;
    PXConfig * config;
    BOOL       dirty;
    BOOL       saveOnExit;
};


#endif
