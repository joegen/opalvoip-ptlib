/*
 * pipechan.h
 *
 * Sub-process communicating via pipe I/O channel.
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
 * $Log: pipechan.h,v $
 * Revision 1.5  1998/09/24 04:11:44  robertj
 * Added open software license.
 *
 * Revision 1.4  1996/12/30 03:23:52  robertj
 * Commonised kill and wait functions.
 *
 * Revision 1.3  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.2  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 */

#ifndef _PPIPECHANNEL

#pragma interface

#include <signal.h>

#include "../../common/ptlib/pipechan.h"
  protected:
    int toChildPipe[2];
    int fromChildPipe[2];
    int childPid;
    int retVal;
};

#ifdef P_USE_INLINES
#include "../../common/pipechan.inl"
#endif

#endif
