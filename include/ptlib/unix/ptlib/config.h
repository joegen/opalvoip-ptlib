/*
 * $Id: config.h,v 1.3 1995/07/09 00:34:59 craigs Exp $
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
    BOOL       saveOnExit;
};


#endif
