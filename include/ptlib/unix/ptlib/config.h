/*
 * $Id: config.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 */

#ifndef _PCONFIG

#pragma interface

class PXConfig;

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
