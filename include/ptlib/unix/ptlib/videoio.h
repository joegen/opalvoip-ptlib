/*
 * videoio.h
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2000 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: videoio.h,v $
 * Revision 1.8  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.7  2001/03/20 02:21:57  robertj
 * More enhancements from Mark Cooke
 *
 * Revision 1.6  2001/03/03 06:13:01  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
 * Revision 1.5  2001/01/05 10:50:04  rogerh
 * More BSD Unix support for PVideoInputDevice
 *
 * Revision 1.4  2001/01/03 10:34:18  rogerh
 * Put Linux specific parts in P_LINUX sections and start adding some FreeBSD
 * and OpenBSD code.
 *
 * Revision 1.3  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.2  2000/07/30 03:54:28  robertj
 * Added more colour formats to video device enum.
 *
 * Revision 1.1  2000/07/26 02:40:29  robertj
 * Added video I/O devices.
 *
 */


#ifndef _PVIDEOIO

#if defined(P_LINUX)
#include <linux/videodev.h>     /* change this to "videodev2.h" for v4l2 */
#endif

#if defined(P_FREEBSD)
#include <machine/ioctl_meteor.h>
#endif

#if defined(P_OPENBSD)
#include <i386/ioctl_meteor.h>
#endif


#define _PVIDEOIO_PLATFORM_INCLUDE
#include "../../videoio.h"

#endif
#ifdef _PVIDEOIO_PLATFORM_INCLUDE
#undef _PVIDEOIO_PLATFORM_INCLUDE

  public:
    virtual BOOL SetVideoFormat(VideoFormat videoFormat);
    virtual int  GetNumChannels();
    virtual BOOL SetChannel(int channelNumber);
    virtual BOOL SetColourFormat(const PString & colourFormat);
    virtual BOOL SetFrameRate(unsigned rate);
    virtual BOOL GetFrameSizeLimits(unsigned & minWidth, unsigned & minHeight, unsigned & maxWidth, unsigned & maxHeight) ;
    virtual BOOL SetFrameSize(unsigned width, unsigned height);
  protected:
    void ClearMapping();


#if defined(P_LINUX)
    int    videoFd;
    struct video_capability videoCapability;
    int    canMap;  // -1 = don't know, 0 = no, 1 = yes
    int    colourFormatCode;
    PINDEX hint_index;
    BYTE * videoBuffer;
    PINDEX frameBytes;
    int    currentFrame;
    struct video_mbuf frame;
    struct video_mmap frameBuffer[2];
#endif

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD)
    struct video_capability
    {
        int channels;   /* Num channels */
        int maxwidth;   /* Supported width */
        int maxheight;  /* And height */
        int minwidth;   /* Supported width */
        int minheight;  /* And height */
    };

    int    videoFd;
    struct video_capability videoCapability;
    int    canMap;  // -1 = don't know, 0 = no, 1 = yes
    BYTE * videoBuffer;
    PINDEX frameBytes;
    int    mmap_size;
#endif


#endif


// End Of File ////////////////////////////////////////////////////////////////
