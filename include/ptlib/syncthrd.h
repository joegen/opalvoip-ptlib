/*
 * syncptack.h
 *
 * Two way thread synchronisation point class.
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
 * $Log: syncthrd.h,v $
 * Revision 1.2  1998/09/23 06:21:35  robertj
 * Added open source copyright license.
 *
 * Revision 1.1  1998/05/30 13:26:15  robertj
 * Initial revision
 *
 */


#define _PSYNCPOINTACK

#ifdef __GNUC__
#pragma interface
#endif

#include <syncpoint.h>


PDECLARE_CLASS(PSyncPointAck, PSyncPoint)
/* This class defines a thread synchonisation object.

   This may be used to send signals to a thread and await an acknowldegement
   that the signal was processed.
 */

  public:
    virtual void Signal();
    void Signal(const PTimeInterval & waitTime);

    void Acknowledge();

  protected:
    PSyncPoint ack;
};

////////////////////////////////////////////////////////////////////////////////

