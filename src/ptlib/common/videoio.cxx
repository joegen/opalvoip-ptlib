/*
 * videoio.cxx
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
 * Contributor(s): Mark Cooke (mpc@star.sr.bham.ac.uk)
 *
 * $Log: videoio.cxx,v $
 * Revision 1.25  2002/01/14 02:59:41  robertj
 * Added preferred colour format selection, thanks Walter Whitlock
 *
 * Revision 1.24  2002/01/08 01:32:20  robertj
 * Tidied up some PTRACE debug output.
 *
 * Revision 1.23  2002/01/04 04:11:45  dereks
 * Add video flip code from Walter Whitlock, which flips code at the grabber.
 *
 * Revision 1.22  2001/12/10 22:23:43  dereks
 * Enable resize of CIF source image into QCIF size.
 *
 * Revision 1.21  2001/12/06 22:15:09  dereks
 * Additional debugging lines
 *
 * Revision 1.20  2001/11/28 04:45:14  robertj
 * Added Win32 flipped RGB colour formats.
 *
 * Revision 1.19  2001/11/28 00:07:32  dereks
 * Locking added to PVideoChannel, allowing reader/writer to be changed mid call
 * Enabled adjustment of the video frame rate
 * New fictitous image, a blank grey area
 *
 * Revision 1.18  2001/09/25 04:25:38  dereks
 * Add fix from Stelian Pop to improve the field of view  for
 * the  small image with a  Sony Vaio Motion Eye. Many thanks.
 *
 * Revision 1.17  2001/08/03 04:21:51  dereks
 * Add colour/size conversion for YUV422->YUV411P
 * Add Get/Set Brightness,Contrast,Hue,Colour for PVideoDevice,  and
 * Linux PVideoInputDevice.
 * Add lots of PTRACE statement for debugging colour conversion.
 * Add support for Sony Vaio laptop under linux. Requires 2.4.7 kernel.
 *
 * Revision 1.16  2001/06/27 17:23:33  rogerh
 * Back out my previous change
 *
 * Revision 1.15  2001/06/26 15:48:31  rogerh
 * Do not call GetInputDeviceName if there is no video grabber code
 *
 * Revision 1.14  2001/05/22 23:38:45  robertj
 * Fixed bug in PVideoOutputDevice, removed redundent SetFrameSize.
 *
 * Revision 1.13  2001/03/20 02:21:57  robertj
 * More enhancements from Mark Cooke
 *
 * Revision 1.12  2001/03/08 08:31:34  robertj
 * Numerous enhancements to the video grabbing code including resizing
 *   infrastructure to converters. Thanks a LOT, Mark Cooke.
 *
 * Revision 1.11  2001/03/08 02:18:45  robertj
 * Added improved defaulting of video formats so Open() does not fail.
 * Removed the requirement that the device be open before you can set
 *   formats such as colour, video, channel number etc.
 *
 * Revision 1.10  2001/03/07 01:41:03  dereks
 * Fix memory leak, on destroying PVideoDevice
 * Ensure converter class is resized correctly.
 *
 * Revision 1.9  2001/03/06 23:34:20  robertj
 * Added static function to get input device names.
 * Moved some inline virtuals to non-inline.
 *
 * Revision 1.8  2001/03/05 01:12:41  robertj
 * Added more source formats for conversion, use list. Thanks Mark Cooke.
 *
 * Revision 1.7  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
 * Revision 1.6  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.5  2000/11/09 00:20:58  robertj
 * Added qcif size constants
 *
 * Revision 1.4  2000/07/26 03:50:50  robertj
 * Added last error variable to video device.
 *
 * Revision 1.3  2000/07/26 02:13:48  robertj
 * Added some more "common" bounds checking to video device.
 *
 * Revision 1.2  2000/07/25 13:38:26  robertj
 * Added frame rate parameter to video frame grabber.
 *
 * Revision 1.1  2000/07/15 09:47:35  robertj
 * Added video I/O device classes.
 *
 */

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>



///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoDevice::PVideoDevice()
{
  lastError = 0;

  videoFormat = Auto;
  channelNumber = -1;
  frameWidth = CIFWidth;
  frameHeight = CIFHeight;

  SetCanCaptureVideo(FALSE);    ///This device cannot grab video from a port.

  converter = NULL;

  SetFrameRate(0);
}

PVideoDevice::~PVideoDevice()
{
  if (converter)
    delete converter;
}


BOOL PVideoDevice::SetVideoFormat(VideoFormat videoFmt)
{
  videoFormat = videoFmt;
  return TRUE;
}


PVideoDevice::VideoFormat PVideoDevice::GetVideoFormat() const
{
  return videoFormat;
}


int PVideoDevice::GetNumChannels()
{
  return 1;
}


BOOL PVideoDevice::SetChannel(int channelNum)
{
  if (channelNum < 0) {
    for (int c = 0; c < GetNumChannels(); c++)
      if (SetChannel(c))
        return TRUE;
    return FALSE;
  }

  if (channelNum >= GetNumChannels())
    return FALSE;

  channelNumber = channelNum;
  return TRUE;
}


int PVideoDevice::GetChannel() const
{
  return channelNumber;
}


//Colour format bit per pixel table.
// These are in rough order of colour gamut size
static struct {
  const char * colourFormat;
  unsigned     bitsPerPixel;
} colourFormatBPPTab[] = {
  { "RGB24",   24 },
  { "RGB24F",  24 },
  { "RGB32",   32 },
  { "RGB32F",  32 },
  { "YUV422",  16 },
  { "YUV422P", 16 },
  { "YUV411",  12 },
  { "YUV411P", 12 },
  { "RGB565",  16 },
  { "RGB565F", 16 },
  { "RGB555",  16 },
  { "RGB555F", 16 },
  { "YUV420",  12 },
  { "YUV420P", 12 },
  { "IYUV",    12 },
  { "I420",    12 },
  { "YUV410",  10 },
  { "YUV410P", 10 },
  { "Grey",     8 },
  { "GreyF",    8 }
};


BOOL PVideoDevice::SetColourFormatConverter(const PString & colourFmt)
{
  if (converter) {    
    delete converter;
    converter = NULL;
  }
  
  if (!preferredColourFormat.IsEmpty()) {
    PTRACE(4,"PVidDev\tSetColourFormatConverter, want " << colourFmt << " trying " << preferredColourFormat);
    if (SetColourFormat(preferredColourFormat)) {
      PTRACE(4,"PVidDev\tSetColourFormatConverter set camera to native "<< preferredColourFormat);
      converter = PColourConverter::Create(preferredColourFormat, colourFmt, frameWidth, frameHeight);
      if (converter != NULL) {
        // set converter properties that depend on this color format
        if (SetColourFormat(preferredColourFormat)) {
          PTRACE(3, "PVidDev\tSetColourFormatConverter succeeded for converted " << colourFmt << " camera using " << preferredColourFormat);
          return TRUE;
        }
      } 
    }
  }
  
  if (SetColourFormat(colourFmt)) {
    PTRACE(3, "PVidDev\tSetColourFormatConverter success for native " << colourFmt);    
    return TRUE;
  }
  
  /************************
    Eventually, need something more sophisticated than this, but for the
    moment pick the known colour formats that the device is very likely to
    support and then look for a conversion routine from that to the
    destination format.

    What we really want is some sort of better heuristic that looks at
    computational requirements of each converter and picks a pair of formats
    that the hardware supports and uses the least CPU.
  */

  PINDEX knownFormatIdx = 0;
  while (knownFormatIdx < PARRAYSIZE(colourFormatBPPTab)) {
    PString formatToTry = colourFormatBPPTab[knownFormatIdx].colourFormat;
    PTRACE(4,"PVidDev\tSetColourFormatConverter, want " << colourFmt << " trying " << formatToTry);
    if (SetColourFormat(formatToTry)) {
      PTRACE(4,"PVidDev\tSetColourFormatConverter set camera to "<< formatToTry);
      converter = PColourConverter::Create(formatToTry, colourFmt, frameWidth, frameHeight);
      if (converter != NULL) {
        // set converter properties that depend on this color format
        if (SetColourFormat(formatToTry)) {
          PTRACE(3, "PVidDev\tSetColourFormatConverter succeeded for converted " << colourFmt << " camera using " << formatToTry);
          return TRUE;
        }
      } 
    } 
    knownFormatIdx++;
  }

  PTRACE(2, "PVidDev\tSetColourFormatConverter  FAILED for " << colourFmt);
  return FALSE;
}


BOOL PVideoDevice::SetColourFormat(const PString & colourFmt)
{
  if (!colourFmt) {
    colourFormat = colourFmt;
    return TRUE;
  }

  for (PINDEX i = 0; i < PARRAYSIZE(colourFormatBPPTab); i++) {
    if (SetColourFormat(colourFormatBPPTab[i].colourFormat))
      return TRUE;
  }

  return FALSE;
}


const PString & PVideoDevice::GetColourFormat() const
{
  return colourFormat;
}


BOOL PVideoDevice::SetFrameRate(unsigned rate)
{

  if (rate < 1) {
    frameRate = 0;
    return TRUE;
  }

  frameRate = rate;
  previousFrameTime = PTime();
  msBetweenFrames = 1000/rate;
  frameTimeError  = 0;

  return TRUE;
}


unsigned PVideoDevice::GetFrameRate() const
{
  return frameRate;
}


BOOL PVideoDevice::GetFrameSizeLimits(unsigned & minWidth,
                                      unsigned & minHeight,
                                      unsigned & maxWidth,
                                      unsigned & maxHeight) 
{
  minWidth = minHeight = 1;
  maxWidth = maxHeight = UINT_MAX;
  return FALSE;
}


static struct {
    unsigned dest_width, dest_height, device_width, device_height;
} prefResizeTable[] = {    
    { 352, 288,    320, 240 },
    { 176, 144,    160, 120 },
    { 176, 144,    320, 240 },
    { 176, 144,    352, 288 }   //Generate small video when camera only does large.
};

BOOL PVideoDevice::SetFrameSizeConverter(unsigned width, unsigned height,
					 BOOL bScaleNotCrop)
{
  if (SetFrameSize(width, height))
    return TRUE;
  
  if (!converter)
    converter = PColourConverter::Create(colourFormat, colourFormat, width, height);
  if (!converter) {
    PTRACE(1, "PVidDev\tSetFrameSizeConverter Colour converter creation failed");
    return FALSE;
  }
  
  PTRACE(3,"PVidDev\tColour converter created for " << width << 'x' << height);
  
  PINDEX prefResizeIdx = 0;
  while (prefResizeIdx < PARRAYSIZE(prefResizeTable)) {
    if ((prefResizeTable[prefResizeIdx].dest_width == width) &&
        (prefResizeTable[prefResizeIdx].dest_height == height)) {

      if (SetFrameSize(prefResizeTable[prefResizeIdx].device_width,
                       prefResizeTable[prefResizeIdx].device_height)) {
	BOOL converterOK= converter->SetDstFrameSize(width, height, bScaleNotCrop);
	if (converterOK){
  	  PTRACE(4,"PVidDev\tSetFrameSizeConverter succceded for "
                 << prefResizeTable[prefResizeIdx].device_width << 'x'
                 << prefResizeTable[prefResizeIdx].device_height
                 << " --> " << width<< 'x' <<height);	       

	  return TRUE;
	}
	PTRACE(2,"PVidDev\tSetFrameSizeConverter FAILED for "
               << prefResizeTable[prefResizeIdx].device_width << 'x'
               << prefResizeTable[prefResizeIdx].device_height 
               << " --> " << width << 'x' << height);	       
      }
    }    
    prefResizeIdx++;
  }

  // Failed to find a resolution the device can do so far, so try
  // using the maximum width and height it claims it can do.
  
  // QUESTION: DO WE WANT A MAX SIZE INSANITY CHECK HERE.

  unsigned minWidth, minHeight, maxWidth, maxHeight;
  GetFrameSizeLimits(minWidth, minHeight, maxWidth, maxHeight);

  if (SetFrameSize(maxWidth, maxHeight)){
    PTRACE(4,"PVidDev\tSuccess set hardware size to " << maxWidth << 'x' << maxHeight);
    if (converter->SetDstFrameSize(width, height, bScaleNotCrop)){
      PTRACE(3,"PVidDev\tSetFrameSizeConvert SUCCEEDED for " << width << 'x' << height);
      return TRUE;
    }
  }

  PTRACE(2,"PVidDev\tSetFrameSizeConverter FAILED for " << width << 'x' << height);
  return FALSE;
}


BOOL PVideoDevice::SetFrameSize(unsigned width, unsigned height)
{
#if PTRACING
  unsigned oldWidth = frameWidth;
  unsigned oldHeight = frameHeight;
#endif

  unsigned minWidth, minHeight, maxWidth, maxHeight;
  GetFrameSizeLimits(minWidth, minHeight, maxWidth, maxHeight);

  if (width < minWidth)
    frameWidth = minWidth;
  else if (width > maxWidth)
    frameWidth = maxWidth;
  else
    frameWidth = width;

  if (height < minHeight)
    frameHeight = minHeight;
  else if (height > maxHeight)
    frameHeight = maxHeight;
  else
    frameHeight = height;

  if (converter != NULL) {
    if (!converter->SetSrcFrameSize(width, height) ||
        !converter->SetDstFrameSize(width, height, FALSE)) {
      PTRACE(1, "PVidDev\tSetFrameSize with converter failed with " << width << 'x' << height);
      return FALSE;
    }
  }

  PTRACE_IF(2, oldWidth != frameWidth || oldHeight != frameHeight,
            "PVidDev\tSetFrameSize to " << frameWidth << 'x' << frameHeight);
  return TRUE;
}


BOOL PVideoDevice::GetFrameSize(unsigned & width, unsigned & height) 
{
#if 1
    // Channels get very upset at this not returning the output size.
    if (converter)
	return converter->GetDstFrameSize(width, height);
#endif
  width = frameWidth;
  height = frameHeight;
  return TRUE;
}


unsigned PVideoDevice::GetFrameWidth() const
{
#if 1
    unsigned w,h;

    // Channels get very upset at this not returning the output size.
    if (converter) {
	converter->GetDstFrameSize(w, h);
	return w;
    }
#endif
  return frameWidth;
}


unsigned PVideoDevice::GetFrameHeight() const
{
#if 1
    unsigned w,h;

    // Channels get very upset at this not returning the output size.
    if (converter) {
	converter->GetDstFrameSize(w, h);
	return h;
    }
#endif
  return frameHeight;
}


unsigned PVideoDevice::CalculateFrameBytes(unsigned width, unsigned height,
                                           const PString & colourFormat)
{
  for (PINDEX i = 0; i < PARRAYSIZE(colourFormatBPPTab); i++) {
    if (colourFormat == colourFormatBPPTab[i].colourFormat)
      return width * height * colourFormatBPPTab[i].bitsPerPixel/8;
  }
  return 0;
}
 

BOOL PVideoDevice::Close()
{
  return TRUE;  
}

BOOL PVideoDevice::GetParameters (int *whiteness, int *brightness, 
                                      int *colour, int *contrast, int *hue)
{
  if (!IsOpen())
    return FALSE;

  *brightness = frameBrightness;
  *colour     = frameColour;
  *contrast   = frameContrast;
  *hue        = frameHue;
  *whiteness  = frameWhiteness;

  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice

PVideoOutputDevice::PVideoOutputDevice()
{
  now = 0;
  suppress = FALSE;
}


BOOL PVideoOutputDevice::Redraw(const void * /*frame*/)
{
  return FALSE;
}


void PVideoOutputDevice::SetNow(int _now)
{
  now = _now;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice

PStringList PVideoInputDevice::GetDeviceNames() const
{
  return GetInputDeviceNames();
}


BOOL PVideoInputDevice::GetFrame(PBYTEArray & frame)
{
  PINDEX returned;
  if (!GetFrameData(frame.GetPointer(GetMaxFrameBytes()), &returned))
    return FALSE;

  frame.SetSize(returned);
  return TRUE;
}


BOOL PVideoInputDevice::GetVFlipState() 
{
  if ( converter == NULL )
    return FALSE;

  return converter->GetVFlipState();
} 

BOOL PVideoInputDevice::SetVFlipState(BOOL newVFlip) 
{
  if ( converter == NULL )
    return FALSE;
  converter->SetVFlipState(newVFlip);
  return TRUE;
} 

BOOL PVideoInputDevice::ToggleVFlipState() 
{
  if ( converter == NULL )
    return FALSE;
  converter->ToggleVFlipState();
  return TRUE;
} 

// End Of File ///////////////////////////////////////////////////////////////
