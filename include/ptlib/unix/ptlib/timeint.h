/*
 * timeint.h
 *
 * Time interval (64 bit milliseconds)
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
 * $Log: timeint.h,v $
 * Revision 1.6  1998/09/24 04:12:04  robertj
 * Added open software license.
 *
 * Revision 1.5  1996/08/03 12:09:51  craigs
 * Changed for new common directories
 *
 * Revision 1.4  1996/06/10 11:03:33  craigs
 * Remove unneeded function declarations
 *
 * Revision 1.3  1995/07/09 00:35:00  craigs
 * Latest and greatest omnibus change
 *
 * Revision 1.2  1995/03/25 20:58:24  craigs
 * Removed unnecessary prefix on declaration of PTimerInterval::operator =
 *
 * Revision 1.1  1995/01/23  18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PTIMEINTERVAL

#pragma interface

///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

#include <time.h>

#include "../../common/ptlib/timeint.h"
};

#define PMaxTimeInterval PTimeInterval((long)0x7fffffff)

#endif
