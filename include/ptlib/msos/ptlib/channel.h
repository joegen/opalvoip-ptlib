/*
 * channel.h
 *
 * I/O channel ancestor class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: channel.h,v $
 * Revision 1.4  1998/09/24 03:29:55  robertj
 * Added open software license.
 *
 * Revision 1.3  1998/08/20 06:03:44  robertj
 * Allowed Win32 class to be used in other compilation modules
 *
 * Revision 1.2  1996/08/08 10:08:56  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.1  1994/07/02 03:18:09  robertj
 * Initial revision
 *
 */


#ifndef _PCHANNEL

///////////////////////////////////////////////////////////////////////////////
// PChannel

#include "../../common/ptlib/channel.h"
};


class PWin32Overlapped : public OVERLAPPED
{
  // Support class for overlapped I/O in Win32.
  public:
    PWin32Overlapped();
    ~PWin32Overlapped();
    void Reset();
};



#endif
