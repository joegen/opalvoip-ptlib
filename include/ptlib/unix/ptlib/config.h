/*
 * $Id: config.h,v 1.6 1998/03/29 10:42:52 craigs Exp $
 */

#ifndef _PCONFIG

#pragma interface

class PXConfig;

///////////////////////////////////////////////////////////////////////////////
// PConfiguration

#include "../../common/ptlib/config.h"
  public:
    PConfig(int, const PString & name);
    ~PConfig();

  protected:
    PXConfig * config;
};


#endif
