/*
 * video.h
 *
 * Video class.
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
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *
 * $Log: video.h,v $
 * Revision 1.5  2001/11/28 00:07:32  dereks
 * Locking added to PVideoChannel, allowing reader/writer to be changed mid call
 * Enabled adjustment of the video frame rate
 * New fictitous image, a blank grey area
 *
 * Revision 1.4  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.3  2001/01/03 08:30:36  rogerh
 * Typo sound -> video
 *
 * Revision 1.2  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.1  2000/11/09 00:28:34  dereks
 * Initial release. Required for implementation of PVideoChannel.
 *
 *
 */

#ifndef _PVIDEO

#pragma interface


///////////////////////////////////////////////////////////////////////////////
// PVideo

#define _PVIDEO_PLATFORM_INCLUDE
#include "../../video.h"

#endif
#ifdef _PVIDEO_PLATFORM_INCLUDE
#undef _PVIDEO_PLATFORM_INCLUDE

  public:
    virtual BOOL Close();

    virtual PString GetName() const;
    // Return the name of the channel.

    PString GetErrorText() const;
    // Get a text form of the last error encountered.

  protected:
    static PMutex dictMutex;

#endif


// End Of File ////////////////////////////////////////////////////////////////
