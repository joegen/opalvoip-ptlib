/*
 * $Id: config.h,v 1.2 1995/04/04 18:31:51 craigs Exp $
 */

#ifndef _PCONFIG

#pragma interface

class PXConfigSection;
PLIST(PXConfig, PXConfigSection);

///////////////////////////////////////////////////////////////////////////////
// PConfiguration

#include "../../common/config.h"
  public:
    ~PConfig();

  protected:
    PFilePath  filename;
    PXConfig * config;
    BOOL       dirty;
};


#endif
