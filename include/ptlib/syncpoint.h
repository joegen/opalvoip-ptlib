/*
 * syncpoint.h
 *
 * Single thread synchronisation point (event) class.
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
 * $Log: syncpoint.h,v $
 * Revision 1.2  1998/09/23 06:21:34  robertj
 * Added open source copyright license.
 *
 * Revision 1.1  1998/03/23 02:41:34  robertj
 * Initial revision
 *
 */


#define _PSYNCPOINT

#ifdef __GNUC__
#pragma interface
#endif

#include <semaphor.h>


PDECLARE_CLASS(PSyncPoint, PSemaphore)
/* This class defines a thread synchonisation object.
 */

  public:
    PSyncPoint();
    /* Create a new sync point.
     */

// Class declaration continued in platform specific header file ///////////////
