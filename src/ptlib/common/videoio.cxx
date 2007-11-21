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
 * $Id$
 */

#ifdef __GNUC__
#pragma implementation "videoio.h"
#endif 

#include <ptlib.h>

#if P_VIDEO

#include <ptlib/pluginmgr.h>
#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>


namespace PWLib {
  PFactory<PDevicePluginAdapterBase>::Worker< PDevicePluginAdapter<PVideoInputDevice> > vidinChannelFactoryAdapter("PVideoInputDevice", TRUE);
  PFactory<PDevicePluginAdapterBase>::Worker< PDevicePluginAdapter<PVideoOutputDevice> > vidoutChannelFactoryAdapter("PVideoOutputDevice", TRUE);
};

template <> PVideoInputDevice * PDevicePluginFactory<PVideoInputDevice>::Worker::Create(const PString & type) const
{
  return PVideoInputDevice::CreateDevice(type);
}

template <> PVideoOutputDevice * PDevicePluginFactory<PVideoOutputDevice>::Worker::Create(const PString & type) const
{
  return PVideoOutputDevice::CreateDevice(type);
}

///////////////////////////////////////////////////////////////////////////////

#if PTRACING
ostream & operator<<(ostream & strm, PVideoDevice::VideoFormat fmt)
{
  static const char * const VideoFormatNames[PVideoDevice::NumVideoFormats] = {
    "PAL",
    "NTSC",
    "SECAM",
    "Auto"
  };

  if (fmt < PVideoDevice::NumVideoFormats && VideoFormatNames[fmt] != NULL)
    strm << VideoFormatNames[fmt];
  else
    strm << "VideoFormat<" << (unsigned)fmt << '>';

  return strm;
}
#endif


//Colour format bit per pixel table.
// These are in rough order of colour gamut size
static struct {
  const char * colourFormat;
  unsigned     bitsPerPixel;
} colourFormatBPPTab[] = {
  { "RGB32",   32 },
  { "BGR32",   32 },
  { "RGB24",   24 },
  { "BGR24",   24 },
  { "MJPEG",   16 },
  { "JPEG",    16 },
  { "YUY2",    16 },
  { "YUV422",  16 },
  { "YUV422P", 16 },
  { "YUV411",  12 },
  { "YUV411P", 12 },
  { "RGB565",  16 },
  { "RGB555",  16 },
  { "YUV420",  12 },
  { "YUV420P", 12 },
  { "IYUV",    12 },
  { "I420",    12 },
  { "YUV410",  10 },
  { "YUV410P", 10 },
  { "Grey",     8 },
  { "GreyF",    8 },
  { "UYVY422", 16 },
  { "UYV444",  24 },
  { "SBGGR8",   8 }
};


template <class VideoDevice>
static VideoDevice * CreateDeviceWithDefaults(PString & adjustedDeviceName,
                                              const PString & driverName,
                                              PPluginManager * pluginMgr)
{
  PString adjustedDriverName = driverName;

  if (adjustedDeviceName.IsEmpty() || adjustedDeviceName == "*") {
    if (driverName.IsEmpty() || driverName == "*") {
      PStringList drivers = VideoDevice::GetDriverNames(pluginMgr);
      if (drivers.IsEmpty())
        return NULL;

      // Give precedence to drivers like camera grabbers, leave out the fail safe types such as NULL
      PINDEX driverIndex;
      for (driverIndex = drivers.GetSize()-1; driverIndex > 0; --driverIndex) {
        static const char * lowPriorityDrivers[] = {
          "YUVFile", "FakeVideo", "NULLOutput"
        };
        PINDEX i;
        for (i = 0; i < PARRAYSIZE(lowPriorityDrivers); i++) {
          if (drivers[driverIndex] == lowPriorityDrivers[i])
            break;
        }
        if (i == PARRAYSIZE(lowPriorityDrivers))
          break;
      }
      adjustedDriverName = drivers[driverIndex];
    }

    PStringList devices = VideoDevice::GetDriversDeviceNames(adjustedDriverName);
    if (!devices.IsEmpty())
      adjustedDeviceName = devices[0];
  }

  return VideoDevice::CreateDeviceByName(adjustedDeviceName, adjustedDriverName, pluginMgr);
}


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoFrameInfo::PVideoFrameInfo()
  : frameWidth(CIFWidth)
  , frameHeight(CIFHeight)
  , frameRate(25)
  , colourFormat("YUV420P")
  , resizeMode(eScale)
{
}


BOOL PVideoFrameInfo::SetFrameSize(unsigned width, unsigned height)
{
  if (width < 8 || height < 8)
    return FALSE;
  frameWidth = width;
  frameHeight = height;
  return TRUE;
}


BOOL PVideoFrameInfo::GetFrameSize(unsigned & width, unsigned & height) const
{
  width = frameWidth;
  height = frameHeight;
  return TRUE;
}


unsigned PVideoFrameInfo::GetFrameWidth() const
{
  unsigned w,h;
  GetFrameSize(w, h);
  return w;
}


unsigned PVideoFrameInfo::GetFrameHeight() const
{
  unsigned w,h;
  GetFrameSize(w, h);
  return h;
}


BOOL PVideoFrameInfo::SetFrameRate(unsigned rate)
{
  if (rate < 1 || rate > 999)
    return FALSE;

  frameRate = rate;
  return TRUE;
}


unsigned PVideoFrameInfo::GetFrameRate() const
{
  return frameRate;
}


BOOL PVideoFrameInfo::SetColourFormat(const PString & colourFmt)
{
  if (!colourFmt) {
    colourFormat = colourFmt.ToUpper();
    return TRUE;
  }

  for (PINDEX i = 0; i < PARRAYSIZE(colourFormatBPPTab); i++) {
    if (SetColourFormat(colourFormatBPPTab[i].colourFormat))
      return TRUE;
  }

  return FALSE;
}


const PString & PVideoFrameInfo::GetColourFormat() const
{
  return colourFormat;
}


PINDEX PVideoFrameInfo::CalculateFrameBytes(unsigned width, unsigned height,
                                              const PString & colourFormat)
{
  for (PINDEX i = 0; i < PARRAYSIZE(colourFormatBPPTab); i++) {
    if (colourFormat *= colourFormatBPPTab[i].colourFormat)
      return width * height * colourFormatBPPTab[i].bitsPerPixel/8;
  }
  return 0;
}
 

BOOL PVideoFrameInfo::ParseSize(const PString & str, unsigned & width, unsigned & height)
{
  static struct {
    const char * name;
    unsigned width;
    unsigned height;
  } const sizeTable[] = {
      { "cif",    PVideoDevice::CIFWidth,   PVideoDevice::CIFHeight   },
      { "qcif",   PVideoDevice::QCIFWidth,  PVideoDevice::QCIFHeight  },
      { "sqcif",  PVideoDevice::SQCIFWidth, PVideoDevice::SQCIFHeight },
      { "cif4",   PVideoDevice::CIF4Width,  PVideoDevice::CIF4Height  },
      { "4cif",   PVideoDevice::CIF4Width,  PVideoDevice::CIF4Height  },
      { "cif16",  PVideoDevice::CIF16Width, PVideoDevice::CIF16Height },
      { "16cif",  PVideoDevice::CIF16Width, PVideoDevice::CIF16Height },

      { "ccir601",720,                      486                       },
      { "ntsc",   720,                      480                       },
      { "pal",    768,                      576                       },
      { "hdtvp",  1280,                     720                       },
      { "hd720",  1280,                     720                       },
      { "hdtvi",  1920,                     1080                      },
      { "hd1080", 1920,                     1080                      },

      { "cga",    320,                      240                       },
      { "vga",    640,                      480                       },
      { "wvga",   854,                      480                       },
      { "svga",   800,                      600                       },
      { "xga",    1024,                     768                       },
      { "sxga",   1280,                     1024                      },
      { "wsxga",  1440,                     900                       },
      { "sxga+",  1400,                     1050                      },
      { "wsxga+", 1680,                     1050                      },
      { "uxga",   1600,                     1200                      },
      { "wuxga",  1920,                     1200                      },
      { "qxga",   2048,                     1536                      },
      { "wqxga",  2560,                     1600                      },
      { }
  };

  for (int i = 0; sizeTable[i].name != NULL; i++) {
    if (str *= sizeTable[i].name) {
      width = sizeTable[i].width;
      height = sizeTable[i].height;
      return TRUE;
    }
  }

  return sscanf(str, "%ux%u", &width, &height) == 2 && width > 0 && height > 0;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoDevice::PVideoDevice()
{
  lastError = 0;

  videoFormat = Auto;
  channelNumber = -1;  // -1 will find the first working channel number.
  nativeVerticalFlip = FALSE;

  converter = NULL;
}

PVideoDevice::~PVideoDevice()
{
  if (converter)
    delete converter;
}


PVideoDevice::OpenArgs::OpenArgs()
  : pluginMgr(NULL),
    deviceName("#1"),
    videoFormat(Auto),
    channelNumber(0),
    colourFormat("YUV420P"),
    convertFormat(TRUE),
    rate(0),
    width(CIFWidth),
    height(CIFHeight),
    convertSize(TRUE),
    resizeMode(eScale),
    flip(FALSE),
    brightness(-1),
    whiteness(-1),
    contrast(-1),
    colour(-1),
    hue(-1)
{
}


BOOL PVideoDevice::OpenFull(const OpenArgs & args, BOOL startImmediate)
{
  if (args.deviceName[0] == '#') {
    PStringArray devices = GetDeviceNames();
    PINDEX id = args.deviceName.Mid(1).AsUnsigned();
    if (id == 0 || id > devices.GetSize())
      return FALSE;

    if (!Open(devices[id-1], FALSE))
      return FALSE;
  }
  else {
    if (!Open(args.deviceName, FALSE))
      return FALSE;
  }

  if (!SetVideoFormat(args.videoFormat))
    return FALSE;

  if (!SetChannel(args.channelNumber))
    return FALSE;

  if (args.convertFormat) {
    if (!SetColourFormatConverter(args.colourFormat))
      return FALSE;
  }
  else {
    if (!SetColourFormat(args.colourFormat))
      return FALSE;
  }

  if (args.rate > 0) {
    if (!SetFrameRate(args.rate))
      return FALSE;
  }

  if (args.convertSize) {
    if (!SetFrameSizeConverter(args.width, args.height, args.resizeMode))
      return FALSE;
  }
  else {
    if (!SetFrameSize(args.width, args.height))
      return FALSE;
  }

  if (!SetVFlipState(args.flip))
    return FALSE;

  if (args.brightness >= 0) {
    if (!SetBrightness(args.brightness))
      return FALSE;
  }

  if (args.whiteness >= 0) {
    if (!SetWhiteness(args.whiteness))
      return FALSE;
  }

  if (args.contrast >= 0) {
    if (!SetContrast(args.contrast))
      return FALSE;
  }

  if (args.colour >= 0) {
    if (!SetColour(args.colour))
      return FALSE;
  }

  if (args.hue >= 0) {
    if (!SetColour(args.hue))
      return FALSE;
  }

  if (startImmediate)
    return Start();

  return TRUE;
}


BOOL PVideoDevice::Close()
{
  return TRUE;  
}


BOOL PVideoDevice::Start()
{
  return TRUE;
}


BOOL PVideoDevice::Stop()
{
  return TRUE;
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
  if (channelNum < 0) { // Seek out the first available channel
    for (int c = 0; c < GetNumChannels(); c++) {
      if (SetChannel(c))
        return TRUE;
    }
    return FALSE;
  }

  if (channelNum >= GetNumChannels()) {
    PTRACE(2, "PVidDev\tSetChannel number (" << channelNum << ") too large.");
    return FALSE;
  }

  channelNumber = channelNum;
  return TRUE;
}


int PVideoDevice::GetChannel() const
{
  return channelNumber;
}


BOOL PVideoDevice::SetColourFormatConverter(const PString & newColourFmt)
{
  PVideoFrameInfo src = *this;
  PVideoFrameInfo dst = *this;

  PString colourFmt = newColourFmt; // make copy, just in case newColourFmt is reference to member colourFormat

  if (converter != NULL) {
    if (CanCaptureVideo()) {
      if (converter->GetDstColourFormat() == colourFmt)
        return TRUE;
    }
    else {
      if (converter->GetSrcColourFormat() == colourFmt)
        return TRUE;
    }
    converter->GetSrcFrameInfo(src);
    converter->GetDstFrameInfo(dst);
    delete converter;
    converter = NULL;
  }
  
  if (!preferredColourFormat.IsEmpty()) {
    PTRACE(4,"PVidDev\tSetColourFormatConverter, want " << colourFmt << " trying " << preferredColourFormat);
    if (SetColourFormat(preferredColourFormat)) {
      if (CanCaptureVideo()) {
        PTRACE(4,"PVidDev\tSetColourFormatConverter set camera to native "<< preferredColourFormat);
        if (preferredColourFormat != colourFmt)
          src.SetColourFormat(preferredColourFormat);
      }
      else {
        PTRACE(4,"PVidDev\tSetColourFormatConverter set renderer to "<< preferredColourFormat);
        if (preferredColourFormat != colourFmt)
          dst.SetColourFormat(preferredColourFormat);
      }

      if (nativeVerticalFlip || src.GetColourFormat() != dst.GetColourFormat()) {
        converter = PColourConverter::Create(src, dst);
        if (converter != NULL) {
          converter->SetVFlipState(nativeVerticalFlip);
          return TRUE;
        }
      }
    }
  }
  
  if (SetColourFormat(colourFmt)) {
    if (nativeVerticalFlip) {
      src.SetColourFormat(colourFmt);
      dst.SetColourFormat(colourFmt);
      converter = PColourConverter::Create(src, dst);
      if (PAssertNULL(converter) == NULL)
        return FALSE;
      converter->SetVFlipState(nativeVerticalFlip);
    }

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
      if (CanCaptureVideo()) {
        PTRACE(4,"PVidDev\tSetColourFormatConverter set camera to "<< formatToTry);
        src.SetColourFormat(formatToTry);
        dst.SetColourFormat(colourFmt);
      }
      else {
        PTRACE(4,"PVidDev\tSetColourFormatConverter set renderer to "<< formatToTry);
        dst.SetColourFormat(formatToTry);
        src.SetColourFormat(colourFmt);
      }
      converter = PColourConverter::Create(src, dst);
      if (converter != NULL) {
        // set converter properties that depend on this color format
        PTRACE(3, "PVidDev\tSetColourFormatConverter succeeded for " << colourFmt << " and device using " << formatToTry);
        converter->SetVFlipState(nativeVerticalFlip);
        return TRUE;
      } 
    } 
    knownFormatIdx++;
  }

  PTRACE(2, "PVidDev\tSetColourFormatConverter  FAILED for " << colourFmt);
  return FALSE;
}


BOOL PVideoDevice::GetVFlipState()
{
  if (converter != NULL)
    return converter->GetVFlipState() ^ nativeVerticalFlip;

  return nativeVerticalFlip;
}


BOOL PVideoDevice::SetVFlipState(BOOL newVFlip)
{
  if (newVFlip && converter == NULL) {
    converter = PColourConverter::Create(*this, *this);
    if (PAssertNULL(converter) == NULL)
      return FALSE;
  }

  if (converter != NULL)
    converter->SetVFlipState(newVFlip ^ nativeVerticalFlip);

  return TRUE;
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
    unsigned asked_width, asked_height, device_width, device_height;
} framesizeTab[] = {    
    { 704, 576,    640, 480 },
    { 640, 480,    704, 576 },
    { 640, 480,    352, 288 },

    { 352, 288,    704, 576 },
    { 352, 288,    640, 480 },
    { 352, 288,    352, 240 },
    { 352, 288,    320, 240 },
    { 352, 288,    176, 144 },
    { 352, 288,   1024, 576 }, /* High resolution need to be set at the end */
    { 352, 288,   1280, 960 },

    { 352, 240,    352, 288 },
    { 352, 240,    320, 240 },
    { 352, 240,    640, 480 },

    { 320, 240,    352, 288 },
    { 320, 240,    352, 240 },
    { 320, 240,    640, 480 },

    { 176, 144,    352, 288 },
    { 176, 144,    352, 240 },
    { 176, 144,    320, 240 },
    { 176, 144,    176, 120 },
    { 176, 144,    160, 120 },
    { 176, 144,    640, 480 },
    { 176, 144,   1024, 576 },
    { 176, 144,   1280, 960 }, /* High resolution need to be set at the end */

    { 176, 120,    352, 288 },
    { 176, 120,    352, 240 },
    { 176, 120,    320, 240 },
    { 176, 120,    176, 144 },
    { 176, 120,    160, 120 },
    { 176, 120,    640, 480 },

    { 160, 120,    352, 288 },
    { 160, 120,    352, 240 },
    { 160, 120,    320, 240 },
    { 160, 120,    176, 144 },
    { 160, 120,    176, 120 },
    { 160, 120,    640, 480 },
};

BOOL PVideoDevice::SetFrameSizeConverter(unsigned width, unsigned height, ResizeMode resizeMode)
{
  if (SetFrameSize(width, height)) {
    if (nativeVerticalFlip && converter == NULL) {
      converter = PColourConverter::Create(*this, *this);
      if (PAssertNULL(converter) == NULL)
        return FALSE;
    }
    if (converter != NULL) {
      converter->SetFrameSize(frameWidth, frameHeight);
      converter->SetVFlipState(nativeVerticalFlip);
    }
    return TRUE;
  }

  // Try and get the most compatible physical frame size to convert from/to
  PINDEX i;
  for (i = 0; i < PARRAYSIZE(framesizeTab); i++) {
    if (framesizeTab[i].asked_width == width && framesizeTab[i].asked_height == height &&
        SetFrameSize(framesizeTab[i].device_width, framesizeTab[i].device_height))
      break;
  }
  if (i >= PARRAYSIZE(framesizeTab)) {
    // Failed to find a resolution the device can do so far, so try
    // using the maximum width and height it claims it can do.
    unsigned minWidth, minHeight, maxWidth, maxHeight;
    if (GetFrameSizeLimits(minWidth, minHeight, maxWidth, maxHeight))
      SetFrameSize(maxWidth, maxHeight);
  }

  // Now create the converter ( if not already exist)
  if (converter == NULL) {
    PVideoFrameInfo src = *this;
    PVideoFrameInfo dst = *this;
    if (CanCaptureVideo())
      dst.SetFrameSize(width, height);
    else
      src.SetFrameSize(width, height);
    dst.SetResizeMode(resizeMode);
    converter = PColourConverter::Create(src, dst);
    if (converter == NULL) {
      PTRACE(1, "PVidDev\tSetFrameSizeConverter Colour converter creation failed");
      return FALSE;
    }
  }
  else
  {
    if (CanCaptureVideo())
      converter->SetDstFrameSize(width, height);
    else
      converter->SetSrcFrameSize(width, height);
    converter->SetResizeMode(resizeMode);
  }

  PTRACE(3,"PVidDev\tColour converter used from " << converter->GetSrcFrameWidth() << 'x' << converter->GetSrcFrameHeight() << " [" << converter->GetSrcColourFormat() << "]" << " to " << converter->GetDstFrameWidth() << 'x' << converter->GetDstFrameHeight() << " [" << converter->GetDstColourFormat() << "]");

  return TRUE;
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
        !converter->SetDstFrameSize(width, height)) {
      PTRACE(1, "PVidDev\tSetFrameSize with converter failed with " << width << 'x' << height);
      return FALSE;
    }
  }

  PTRACE_IF(2, oldWidth != frameWidth || oldHeight != frameHeight,
            "PVidDev\tSetFrameSize to " << frameWidth << 'x' << frameHeight);
  return TRUE;
}


BOOL PVideoDevice::GetFrameSize(unsigned & width, unsigned & height) const
{
  // Channels get very upset at this not returning the output size.
  return converter != NULL ? converter->GetDstFrameSize(width, height) : PVideoFrameInfo::GetFrameSize(width, height);
}


PINDEX PVideoDevice::GetMaxFrameBytesConverted(PINDEX rawFrameBytes) const
{
  if (converter == NULL)
    return rawFrameBytes;

  PINDEX srcFrameBytes = converter->GetMaxSrcFrameBytes();
  PINDEX dstFrameBytes = converter->GetMaxDstFrameBytes();
  PINDEX convertedFrameBytes = PMAX(srcFrameBytes, dstFrameBytes);
  return PMAX(rawFrameBytes, convertedFrameBytes);
}


int PVideoDevice::GetBrightness()
{
  return frameBrightness;
}


BOOL PVideoDevice::SetBrightness(unsigned newBrightness)
{
  frameBrightness = newBrightness;
  return TRUE;
}


int PVideoDevice::GetWhiteness()
{
  return frameWhiteness;
}


BOOL PVideoDevice::SetWhiteness(unsigned newWhiteness)
{
  frameWhiteness = newWhiteness;
  return TRUE;
}


int PVideoDevice::GetColour()
{
  return frameColour;
}


BOOL PVideoDevice::SetColour(unsigned newColour)
{
  frameColour=newColour;
  return TRUE;
}


int PVideoDevice::GetContrast()
{
  return frameContrast;
}


BOOL PVideoDevice::SetContrast(unsigned newContrast)
{
  frameContrast=newContrast;
  return TRUE;
}


int PVideoDevice::GetHue()
{
  return frameHue;
}


BOOL PVideoDevice::SetHue(unsigned newHue)
{
  frameHue=newHue;
  return TRUE;
}

    
BOOL PVideoDevice::GetParameters (int *whiteness,
                                  int *brightness, 
                                  int *colour,
                                  int *contrast,
                                  int *hue)
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

BOOL PVideoDevice::SetVideoChannelFormat (int newNumber, VideoFormat newFormat) 
{
  BOOL err1, err2;

  err1 = SetChannel (newNumber);
  err2 = SetVideoFormat (newFormat);
  
  return (err1 && err2);
}

PStringList PVideoDevice::GetDeviceNames() const
{
  return PStringList();
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice

PVideoOutputDevice::PVideoOutputDevice()
{
}


BOOL PVideoOutputDevice::CanCaptureVideo() const
{
  return FALSE;
}


BOOL PVideoOutputDevice::GetPosition(int &, int &) const
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDeviceRGB

PVideoOutputDeviceRGB::PVideoOutputDeviceRGB()
{
  PTRACE(6, "RGB\t Constructor of PVideoOutputDeviceRGB");

  colourFormat = "RGB24";
  bytesPerPixel = 3;
  swappedRedAndBlue = false;
//  SetFrameSize(frameWidth, frameHeight);
}


BOOL PVideoOutputDeviceRGB::SetColourFormat(const PString & colourFormat)
{
  PWaitAndSignal m(mutex);

  PINDEX newBytesPerPixel;

  if (colourFormat *= "RGB32") {
    newBytesPerPixel = 4;
    swappedRedAndBlue = false;
  }
  else if (colourFormat *= "RGB24") {
    newBytesPerPixel = 3;
    swappedRedAndBlue = false;
  }
  else if (colourFormat *= "BGR32") {
    newBytesPerPixel = 4;
    swappedRedAndBlue = true;
  }
  else if (colourFormat *= "BGR24") {
    newBytesPerPixel = 3;
    swappedRedAndBlue = true;
  }
  else
    return FALSE;

  if (!PVideoOutputDevice::SetColourFormat(colourFormat))
    return FALSE;

  bytesPerPixel = newBytesPerPixel;
  scanLineWidth = ((frameWidth*bytesPerPixel+3)/4)*4;
  return frameStore.SetSize(frameHeight*scanLineWidth);
}


BOOL PVideoOutputDeviceRGB::SetFrameSize(unsigned width, unsigned height)
{
  PWaitAndSignal m(mutex);

  if (!PVideoOutputDevice::SetFrameSize(width, height))
    return FALSE;

  scanLineWidth = ((frameWidth*bytesPerPixel+3)/4)*4;
  return frameStore.SetSize(frameHeight*scanLineWidth);
}


PINDEX PVideoOutputDeviceRGB::GetMaxFrameBytes()
{
  PWaitAndSignal m(mutex);
  return GetMaxFrameBytesConverted(frameStore.GetSize());
}


BOOL PVideoOutputDeviceRGB::SetFrameData(unsigned x, unsigned y,
                                         unsigned width, unsigned height,
                                         const BYTE * data,
                                         BOOL endFrame)
{
  PWaitAndSignal m(mutex);

  if (x+width > frameWidth || y+height > frameHeight)
    return FALSE;

  if (x == 0 && width == frameWidth && y == 0 && height == frameHeight) {
    if (converter != NULL)
      converter->Convert(data, frameStore.GetPointer());
    else
      memcpy(frameStore.GetPointer(), data, height*scanLineWidth);
  }
  else {
    if (converter != NULL) {
      PAssertAlways("Converted output of partial RGB frame not supported");
      return FALSE;
    }

    if (x == 0 && width == frameWidth)
      memcpy(frameStore.GetPointer() + y*scanLineWidth, data, height*scanLineWidth);
    else {
      for (unsigned dy = 0; dy < height; dy++)
        memcpy(frameStore.GetPointer() + (y+dy)*scanLineWidth + x*bytesPerPixel,
               data + dy*width*bytesPerPixel, width*bytesPerPixel);
    }
  }

  if (endFrame)
    return FrameComplete();

  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevicePPM

#ifdef SHOULD_BE_MOVED_TO_PLUGIN

PVideoOutputDevicePPM::PVideoOutputDevicePPM()
{
  PTRACE(6, "PPM\t Constructor of PVideoOutputDevicePPM");
  frameNumber = 0;
}


BOOL PVideoOutputDevicePPM::Open(const PString & name,
                                 BOOL /*startImmediate*/)
{
  Close();

  PFilePath path = name;
  if (!PDirectory::Exists(path.GetDirectory()))
    return FALSE;

  if (path != psprintf(path, 12345))
    deviceName = path;
  else
    deviceName = path.GetDirectory() + path.GetTitle() + "%u" + path.GetType();

  return TRUE;
}


BOOL PVideoOutputDevicePPM::IsOpen()
{
  return !deviceName;
}


BOOL PVideoOutputDevicePPM::Close()
{
  deviceName.MakeEmpty();
  return TRUE;
}


PStringList PVideoOutputDevicePPM::GetDeviceNames() const
{
  PStringList list;
  list += PDirectory();
  return list;
}


BOOL PVideoOutputDevicePPM::EndFrame()
{
  PFile file;
  if (!file.Open(psprintf(deviceName, frameNumber++), PFile::WriteOnly)) {
    PTRACE(1, "PPMVid\tFailed to open PPM output file \""
           << file.GetName() << "\": " << file.GetErrorText());
    return FALSE;
  }

  file << "P6 " << frameWidth  << " " << frameHeight << " " << 255 << "\n";

  if (!file.Write(frameStore, frameStore.GetSize())) {
    PTRACE(1, "PPMVid\tFailed to write frame data to PPM output file " << file.GetName());
    return FALSE;
  }

  PTRACE(6, "PPMVid\tFinished writing PPM file " << file.GetName());
  return file.Close();
}

#endif // SHOULD_BE_MOVED_TO_PLUGIN


///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice

BOOL PVideoInputDevice::CanCaptureVideo() const
{
  return TRUE;
}

static const char videoInputPluginBaseClass[] = "PVideoInputDevice";


PStringList PVideoInputDevice::GetDriverNames(PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsProviding(videoInputPluginBaseClass);
}


PStringList PVideoInputDevice::GetDriversDeviceNames(const PString & driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsDeviceNames(driverName, videoInputPluginBaseClass);
}


PVideoInputDevice * PVideoInputDevice::CreateDevice(const PString &driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PVideoInputDevice *)pluginMgr->CreatePluginsDevice(driverName, videoInputPluginBaseClass);
}


PVideoInputDevice * PVideoInputDevice::CreateDeviceByName(const PString & deviceName, const PString & driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PVideoInputDevice *)pluginMgr->CreatePluginsDeviceByName(deviceName, videoInputPluginBaseClass,0,driverName);
}

BOOL PVideoInputDevice::GetDeviceCapabilities(const PString & deviceName,InputDeviceCapabilities * caps, PPluginManager * pluginMgr)
{
	return GetDeviceCapabilities(deviceName, "*",caps,pluginMgr);
}

BOOL PVideoInputDevice::GetDeviceCapabilities(const PString & deviceName,const PString & driverName, InputDeviceCapabilities * caps, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsDeviceCapabilities(videoInputPluginBaseClass,driverName,deviceName, (void *)caps);
}



PVideoInputDevice * PVideoInputDevice::CreateOpenedDevice(const PString & driverName,
                                                          const PString & deviceName,
                                                          BOOL startImmediate,
                                                          PPluginManager * pluginMgr)
{
  PString adjustedDeviceName = deviceName;
  PVideoInputDevice * device = CreateDeviceWithDefaults<PVideoInputDevice>(adjustedDeviceName, driverName, pluginMgr);
  if (device == NULL)
    return NULL;

  if (device->Open(adjustedDeviceName, startImmediate))
    return device;

  delete device;
  return NULL;
}


PVideoInputDevice * PVideoInputDevice::CreateOpenedDevice(const OpenArgs & args,
                                                          BOOL startImmediate)
{
  OpenArgs adjustedArgs = args;
  PVideoInputDevice * device = CreateDeviceWithDefaults<PVideoInputDevice>(adjustedArgs.deviceName, args.driverName, NULL);
  if (device == NULL)
    return NULL;

  if (device->OpenFull(adjustedArgs, startImmediate))
    return device;

  delete device;
  return NULL;
}

BOOL PVideoInputDevice::GetFrame(PBYTEArray & frame)
{
  PINDEX returned;
  if (!GetFrameData(frame.GetPointer(GetMaxFrameBytes()), &returned))
    return FALSE;

  frame.SetSize(returned);
  return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////

static const char videoOutputPluginBaseClass[] = "PVideoOutputDevice";


PStringList PVideoOutputDevice::GetDriverNames(PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsProviding(videoOutputPluginBaseClass);
}


PStringList PVideoOutputDevice::GetDriversDeviceNames(const PString & driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsDeviceNames(driverName, videoOutputPluginBaseClass);
}


PVideoOutputDevice * PVideoOutputDevice::CreateDevice(const PString & driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PVideoOutputDevice *)pluginMgr->CreatePluginsDevice(driverName, videoOutputPluginBaseClass);
}


PVideoOutputDevice * PVideoOutputDevice::CreateDeviceByName(const PString & deviceName, const PString & driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PVideoOutputDevice *)pluginMgr->CreatePluginsDeviceByName(deviceName, videoOutputPluginBaseClass, 0, driverName);
}


PVideoOutputDevice * PVideoOutputDevice::CreateOpenedDevice(const PString &driverName,
                                                            const PString &deviceName,
                                                            BOOL startImmediate,
                                                            PPluginManager * pluginMgr)
{
  PString adjustedDeviceName = deviceName;
  PVideoOutputDevice * device = CreateDeviceWithDefaults<PVideoOutputDevice>(adjustedDeviceName, driverName, pluginMgr);
  if (device == NULL)
    return NULL;

  if (device->Open(adjustedDeviceName, startImmediate))
    return device;

  delete device;
  return NULL;
}


PVideoOutputDevice * PVideoOutputDevice::CreateOpenedDevice(const OpenArgs & args,
                                                            BOOL startImmediate)
{
  OpenArgs adjustedArgs = args;
  PVideoOutputDevice * device = CreateDeviceWithDefaults<PVideoOutputDevice>(adjustedArgs.deviceName, args.driverName, NULL);
  if (device == NULL)
    return NULL;

  if (device->OpenFull(adjustedArgs, startImmediate))
    return device;

  delete device;
  return NULL;
}

#endif // P_VIDEO

// End Of File ///////////////////////////////////////////////////////////////
