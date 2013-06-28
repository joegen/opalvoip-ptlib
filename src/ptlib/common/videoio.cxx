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
 * $Revision$
 * $Author$
 * $Date$
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
  PFactory<PDevicePluginAdapterBase>::Worker< PDevicePluginAdapter<PVideoInputDevice> > vidinChannelFactoryAdapter("PVideoInputDevice", true);
  PFactory<PDevicePluginAdapterBase>::Worker< PDevicePluginAdapter<PVideoOutputDevice> > vidoutChannelFactoryAdapter("PVideoOutputDevice", true);
};

template <> PVideoInputDevice * PDevicePluginFactory<PVideoInputDevice>::Worker::Create(const PDefaultPFactoryKey & type) const
{
  return PVideoInputDevice::CreateDevice(type);
}

template <> PVideoOutputDevice * PDevicePluginFactory<PVideoOutputDevice>::Worker::Create(const PDefaultPFactoryKey & type) const
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
// These are in rough order of colour gamut size and "popularity"
static struct {
  const char * colourFormat;
  unsigned     bitsPerPixel;
} ColourFormatBPPTab[] = {
  { "YUV420P", 12 },
  { "I420",    12 },
  { "IYUV",    12 },
  { "YUV420",  12 },
  { "RGB32",   32 },
  { "BGR32",   32 },
  { "RGB24",   24 },
  { "BGR24",   24 },
  { "YUY2",    16 },
  { "YUV422",  16 },
  { "YUV422P", 16 },
  { "YUV411",  12 },
  { "YUV411P", 12 },
  { "RGB565",  16 },
  { "RGB555",  16 },
  { "RGB16",   16 },
  { "YUV410",  10 },
  { "YUV410P", 10 },
  { "Grey",     8 },
  { "GreyF",    8 },
  { "UYVY422", 16 },
  { "UYV444",  24 },
  { "SBGGR8",   8 },
  { "JPEG",    24 },
  { "MJPEG",   24 }
};


template <class VideoDevice>
static VideoDevice * CreateDeviceWithDefaults(PString & adjustedDeviceName,
                                              const PString & driverName,
                                              PPluginManager * pluginMgr)
{
  if (adjustedDeviceName == "*")
    adjustedDeviceName.MakeEmpty();

  PString adjustedDriverName = driverName;
  if (adjustedDriverName == "*")
    adjustedDriverName.MakeEmpty();

  if (adjustedDeviceName.IsEmpty()) {
    if (adjustedDriverName.IsEmpty()) {
      PStringArray drivers = VideoDevice::GetDriverNames(pluginMgr);
      if (drivers.IsEmpty())
        return NULL;

      // Give precedence to drivers like camera grabbers, Window
      static const char * prioritisedDrivers[] = {
        "Window", "SDL", "DirectShow", "VideoForWindows", "V4L", "V4L2", "1394DC", "1394AVC", "BSDCAPTURE", "FakeVideo", "NULLOutput"
      };
      for (PINDEX i = 0; i < PARRAYSIZE(prioritisedDrivers); i++) {
        PINDEX driverIndex = drivers.GetValuesIndex(PString(prioritisedDrivers[i]));
        if (driverIndex != P_MAX_INDEX) {
          PStringArray devices = VideoDevice::GetDriversDeviceNames(drivers[driverIndex]);
          if (!devices.IsEmpty()) {
            adjustedDeviceName = devices[0];
            adjustedDriverName = drivers[driverIndex];
            break;
          }
        }
      }

      if (adjustedDriverName.IsEmpty())
        adjustedDriverName = drivers[0];
    }

    if (adjustedDeviceName.IsEmpty()) {
      PStringArray devices = VideoDevice::GetDriversDeviceNames(adjustedDriverName);
      if (devices.IsEmpty())
        return NULL;

      adjustedDeviceName = devices[0];
    }
  }

  return VideoDevice::CreateDeviceByName(adjustedDeviceName, adjustedDriverName, pluginMgr);
}


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

ostream & operator<<(ostream & strm, PVideoFrameInfo::ResizeMode mode)
{
  switch (mode) {
    case PVideoFrameInfo::eScale :
      return strm << "Scaled";
    case PVideoFrameInfo::eCropCentre :
      return strm << "Centred";
    case PVideoFrameInfo::eCropTopLeft :
      return strm << "Cropped";
    default :
      return strm << "ResizeMode<" << (int)mode << '>';
  }
}


PVideoFrameInfo::PVideoFrameInfo()
  : frameWidth(CIFWidth)
  , frameHeight(CIFHeight)
  , sarWidth(1)
  , sarHeight(1)
  , frameRate(25)
  , colourFormat("YUV420P")
  , resizeMode(eScale)
{
}


PVideoFrameInfo::PVideoFrameInfo(unsigned        width,
                                 unsigned        height,
                                 const PString & format,
                                 unsigned        rate,
                                 ResizeMode      resize)
  : frameWidth(width)
  , frameHeight(height)
  , sarWidth(1)
  , sarHeight(1)
  , frameRate(rate)
  , colourFormat(format)
  , resizeMode(resize)
{
}


PObject::Comparison PVideoFrameInfo::Compare(const PObject & obj) const
{
  const PVideoFrameInfo & other = dynamic_cast<const PVideoFrameInfo &>(obj);

  unsigned area = frameWidth*frameHeight;
  unsigned otherArea = other.frameWidth*other.frameHeight;
  if (area < otherArea)
    return LessThan;
  if (area > otherArea)
    return GreaterThan;

  if (frameRate < other.frameRate)
    return LessThan;
  if (frameRate > other.frameRate)
    return GreaterThan;

  return colourFormat.Compare(other.colourFormat);
}


void PVideoFrameInfo::PrintOn(ostream & strm) const
{
  if (!colourFormat.IsEmpty())
    strm << colourFormat << ':';

  strm << AsString(frameWidth, frameHeight);

  if (frameRate > 0)
    strm << '@' << frameRate;

  if (resizeMode < eMaxResizeMode)
    strm << '/' << resizeMode;
}


PBoolean PVideoFrameInfo::SetFrameSize(unsigned width, unsigned height)
{
  if (!PAssert(width >= 16 && height >= 16 && width < 65536 && height < 65536, PInvalidParameter))
    return false;

  frameWidth = width;
  frameHeight = height;
  return true;
}


PBoolean PVideoFrameInfo::GetFrameSize(unsigned & width, unsigned & height) const
{
  width = frameWidth;
  height = frameHeight;
  return true;
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


PBoolean PVideoFrameInfo::SetFrameSar(unsigned width, unsigned height)
{
  if (!PAssert(width < 65536 && height < 65536, PInvalidParameter))
    return false;

  sarWidth  = width;
  sarHeight = height;
  return true;
}


PBoolean PVideoFrameInfo::GetSarSize(unsigned & width, unsigned & height) const
{
  width  = sarWidth;
  height = sarHeight;
  return true;
}

unsigned PVideoFrameInfo::GetSarWidth() const
{
  unsigned w,h;
  GetSarSize(w, h);
  return w;
}


unsigned PVideoFrameInfo::GetSarHeight() const
{
  unsigned w,h;
  GetSarSize(w, h);
  return h;
}

PBoolean PVideoFrameInfo::SetFrameRate(unsigned rate)
{
  if (!PAssert(rate > 0 && rate < 1000, PInvalidParameter))
    return false;

  frameRate = rate;
  return true;
}


unsigned PVideoFrameInfo::GetFrameRate() const
{
  return frameRate;
}


PBoolean PVideoFrameInfo::SetColourFormat(const PString & colourFmt)
{
  if (!colourFmt) {
    colourFormat = colourFmt.ToUpper();
    return true;
  }

  for (PINDEX i = 0; i < PARRAYSIZE(ColourFormatBPPTab); i++) {
    if (SetColourFormat(ColourFormatBPPTab[i].colourFormat))
      return true;
  }

  return false;
}


const PString & PVideoFrameInfo::GetColourFormat() const
{
  return colourFormat;
}


PINDEX PVideoFrameInfo::CalculateFrameBytes(unsigned width, unsigned height,
                                              const PString & colourFormat)
{
  for (PINDEX i = 0; i < PARRAYSIZE(ColourFormatBPPTab); i++) {
    if (colourFormat *= ColourFormatBPPTab[i].colourFormat)
      return width * height * ColourFormatBPPTab[i].bitsPerPixel/8;
  }
  return 0;
}
 

bool PVideoFrameInfo::Parse(const PString & str)
{
  PString newFormat = colourFormat;
  PINDEX formatOffset = str.Find(':');
  if (formatOffset == 0)
    return false;

  if (formatOffset == P_MAX_INDEX)
    formatOffset = 0;
  else
    newFormat = str.Left(formatOffset++);


  ResizeMode newMode = resizeMode;
  PINDEX resizeOffset = str.Find('/', formatOffset);
  if (resizeOffset != P_MAX_INDEX) {
    static struct {
      const char * name;
      ResizeMode   mode;
    } const ResizeNames[] = {
      { "scale",   eScale },
      { "resize",  eScale },
      { "scaled",  eScale },
      { "centre",  eCropCentre },
      { "centred", eCropCentre },
      { "center",  eCropCentre },
      { "centered",eCropCentre },
      { "crop",    eCropTopLeft },
      { "cropped", eCropTopLeft },
      { "topleft", eCropTopLeft }
    };

    PCaselessString crop = str.Mid(resizeOffset+1);
    PINDEX resizeIndex = 0;
    while (crop != ResizeNames[resizeIndex].name) {
      if (++resizeIndex >= PARRAYSIZE(ResizeNames))
        return false;
    }
    newMode = ResizeNames[resizeIndex].mode;
  }


  int newRate = frameRate;
  PINDEX rateOffset = str.Find('@', formatOffset);
  if (rateOffset == P_MAX_INDEX)
    rateOffset = resizeOffset;
  else {
    newRate = str.Mid(rateOffset+1).AsInteger();
    if (newRate < 1 || newRate > 100)
      return false;
  }

  if (!ParseSize(str(formatOffset, rateOffset-1), frameWidth, frameHeight))
    return false;

  colourFormat = newFormat;
  frameRate = newRate;
  resizeMode = newMode;
  return true;
}


static struct {
  const char * name;
  unsigned width;
  unsigned height;
} const SizeTable[] = {
    { "CIF",    PVideoDevice::CIFWidth,   PVideoDevice::CIFHeight   },
    { "QCIF",   PVideoDevice::QCIFWidth,  PVideoDevice::QCIFHeight  },
    { "SQCIF",  PVideoDevice::SQCIFWidth, PVideoDevice::SQCIFHeight },
    { "CIF4",   PVideoDevice::CIF4Width,  PVideoDevice::CIF4Height  },
    { "4CIF",   PVideoDevice::CIF4Width,  PVideoDevice::CIF4Height  },
    { "CIF16",  PVideoDevice::CIF16Width, PVideoDevice::CIF16Height },
    { "16CIF",  PVideoDevice::CIF16Width, PVideoDevice::CIF16Height },

    { "CCIR601",720,                      486                       },
    { "NTSC",   720,                      480                       },
    { "PAL",    768,                      576                       },
    { "HD480",  PVideoDevice::HD480Width, PVideoDevice::HD480Height },
    { "HDTVP",  PVideoDevice::HD720Width, PVideoDevice::HD720Height },
    { "HD720",  PVideoDevice::HD720Width, PVideoDevice::HD720Height },
    { "720p",   PVideoDevice::HD720Width, PVideoDevice::HD720Height },
    { "HDTVI",  PVideoDevice::HD1080Width,PVideoDevice::HD1080Height},
    { "HD1080", PVideoDevice::HD1080Width,PVideoDevice::HD1080Height},
    { "1080p",  PVideoDevice::HD1080Width,PVideoDevice::HD1080Height},

    { "CGA",    320,                      240                       },
    { "VGA",    640,                      480                       },
    { "WVGA",   854,                      480                       },
    { "SVGA",   800,                      600                       },
    { "XGA",    1024,                     768                       },
    { "SXGA",   1280,                     1024                      },
    { "WSXGA",  1440,                     900                       },
    { "SXGA+",  1400,                     1050                      },
    { "WSXGA+", 1680,                     1050                      },
    { "UXGA",   1600,                     1200                      },
    { "WUXGA",  1920,                     1200                      },
    { "QXGA",   2048,                     1536                      },
    { "WQXGA",  2560,                     1600                      },
};

bool PVideoFrameInfo::ParseSize(const PString & str, unsigned & width, unsigned & height)
{
  for (size_t i = 0; i < PARRAYSIZE(SizeTable); i++) {
    if (str *= SizeTable[i].name) {
      width = SizeTable[i].width;
      height = SizeTable[i].height;
      return true;
    }
  }

  return sscanf(str, "%ux%u", &width, &height) == 2 && width > 0 && height > 0;
}


PString PVideoFrameInfo::AsString(unsigned width, unsigned height)
{
  for (size_t i = 0; i < PARRAYSIZE(SizeTable); i++) {
    if (SizeTable[i].width == width && SizeTable[i].height == height)
      return SizeTable[i].name;
  }

  return psprintf("%ux%u", width, height);
}


PStringArray PVideoFrameInfo::GetSizeNames()
{
  PStringArray names(PARRAYSIZE(SizeTable));
  for (size_t i = 0; i < PARRAYSIZE(SizeTable); i++)
    names[i] = SizeTable[i].name;
  return names;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoDevice::PVideoDevice()
{
  lastError = 0;

  videoFormat = Auto;
  channelNumber = -1;  // -1 will find the first working channel number.
  nativeVerticalFlip = false;

  converter = NULL;
}

PVideoDevice::~PVideoDevice()
{
  if (converter)
    delete converter;
}


PVideoDevice::Attributes::Attributes()
  : m_brightness(-1)
  , m_contrast(-1)
  , m_saturation(-1)
  , m_hue(-1)
  , m_gamma(-1)
  , m_exposure(-1)
{
}


PVideoDevice::OpenArgs::OpenArgs()
  : pluginMgr(NULL),
    deviceName("#1"),
    videoFormat(Auto),
    channelNumber(-1),
    colourFormat("YUV420P"),
    convertFormat(true),
    rate(0),
    width(CIFWidth),
    height(CIFHeight),
    convertSize(true),
    resizeMode(eScale),
    flip(false)
{
}


PBoolean PVideoDevice::OpenFull(const OpenArgs & args, PBoolean startImmediate)
{
  if (args.deviceName[0] == '#') {
    PStringArray devices = GetDeviceNames();
    PINDEX id = args.deviceName.Mid(1).AsUnsigned();
    if (id == 0 || id > devices.GetSize())
      return false;

    if (!Open(devices[id-1], false))
      return false;
  }
  else {
    if (!Open(args.deviceName, false))
      return false;
  }

  if (!SetVideoFormat(args.videoFormat))
    return false;

  if (!SetChannel(args.channelNumber))
    return false;

  if (args.convertFormat) {
    if (!SetColourFormatConverter(args.colourFormat))
      return false;
  }
  else {
    if (!SetColourFormat(args.colourFormat))
      return false;
  }

  if (args.rate > 0) {
    if (!SetFrameRate(args.rate))
      return false;
  }

  if (args.convertSize) {
    if (!SetFrameSizeConverter(args.width, args.height, args.resizeMode))
      return false;
  }
  else {
    if (!SetFrameSize(args.width, args.height))
      return false;
  }

  if (!SetVFlipState(args.flip))
    return false;

  SetAttributes(args.m_attributes);

  if (startImmediate)
    return Start();

  return true;
}


PBoolean PVideoDevice::Close()
{
  return true;  
}


PBoolean PVideoDevice::Start()
{
  return true;
}


PBoolean PVideoDevice::Stop()
{
  return true;
}


PBoolean PVideoDevice::SetVideoFormat(VideoFormat videoFmt)
{
  videoFormat = videoFmt;
  return true;
}


PVideoDevice::VideoFormat PVideoDevice::GetVideoFormat() const
{
  return videoFormat;
}


int PVideoDevice::GetNumChannels()
{
  return 1;
}


PBoolean PVideoDevice::SetChannel(int channelNum)
{
  if (channelNum < 0) { // Seek out the first available channel
    for (int c = 0; c < GetNumChannels(); c++) {
      if (SetChannel(c))
        return true;
    }
    return false;
  }

  if (channelNum >= GetNumChannels()) {
    PTRACE(2, "PVidDev\tSetChannel number (" << channelNum << ") too large.");
    return false;
  }

  channelNumber = channelNum;
  return true;
}


int PVideoDevice::GetChannel() const
{
  return channelNumber;
}


bool PVideoDevice::SetFrameInfoConverter(const PVideoFrameInfo & info)
{
  if (!SetColourFormatConverter(info.GetColourFormat())) {
    PTRACE(1, "PVidDev\tCould not set colour format in "
           << (CanCaptureVideo() ? "grabber" : "display") << " to " << info);
    return false;
  }

  if (!SetFrameSizeConverter(info.GetFrameWidth(), info.GetFrameHeight())) {
    PTRACE(1, "PVidDev\tCould not set frame size in "
           << (CanCaptureVideo() ? "grabber" : "display") << " to " << info);
    return false;
  }

  if (info.GetFrameRate() != 0) {
    if (!SetFrameRate(info.GetFrameRate())) {
      PTRACE(1, "PVidDev\tCould not set frame rate in "
           << (CanCaptureVideo() ? "grabber" : "display") << " to " << info);
      return false;
    }
  }

  PTRACE(4, "PVidDev\tVideo " << (CanCaptureVideo() ? "grabber" : "display") << " set to " << info);
  return true;
}


PBoolean PVideoDevice::SetColourFormatConverter(const PString & newColourFmt)
{
  if (converter != NULL) {
    if (CanCaptureVideo()) {
      if (converter->GetDstColourFormat() == newColourFmt)
        return true;
    }
    else {
      if (converter->GetSrcColourFormat() == newColourFmt)
        return true;
    }
  }
  else {
    if (colourFormat == newColourFmt)
      return true;
  }

  PString newColourFormat = newColourFmt; // make copy, just in case newColourFmt is reference to member colourFormat

  if (!SetColourFormat(newColourFormat) &&
        (preferredColourFormat.IsEmpty() || !SetColourFormat(preferredColourFormat))) {
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
    while (!SetColourFormat(ColourFormatBPPTab[knownFormatIdx].colourFormat)) {
      if (++knownFormatIdx >= PARRAYSIZE(ColourFormatBPPTab)) {
        PTRACE(2, "PVidDev\tSetColourFormatConverter FAILED for " << newColourFormat);
        return false;
      }
    }
  }

  PTRACE(3, "PVidDev\tSetColourFormatConverter success for native " << colourFormat);

  PVideoFrameInfo src = *this;
  PVideoFrameInfo dst = *this;

  if (converter != NULL) {
    converter->GetSrcFrameInfo(src);
    converter->GetDstFrameInfo(dst);
    delete converter;
    converter = NULL;
  }

  if (nativeVerticalFlip || colourFormat != newColourFormat) {
    if (CanCaptureVideo()) {
      src.SetColourFormat(colourFormat);
      dst.SetColourFormat(newColourFormat);
    }
    else {
      src.SetColourFormat(newColourFormat);
      dst.SetColourFormat(colourFormat);
    }

    converter = PColourConverter::Create(src, dst);
    if (converter == NULL) {
      PTRACE(2, "PVidDev\tSetColourFormatConverter failed to create converter from " << src << " to " << dst);
      return false;
    }

    converter->SetVFlipState(nativeVerticalFlip);
  }

  return true;
}


PBoolean PVideoDevice::GetVFlipState()
{
  if (converter != NULL)
    return converter->GetVFlipState() ^ nativeVerticalFlip;

  return nativeVerticalFlip;
}


PBoolean PVideoDevice::SetVFlipState(PBoolean newVFlip)
{
  if (newVFlip && converter == NULL) {
    converter = PColourConverter::Create(*this, *this);
    if (PAssertNULL(converter) == NULL)
      return false;
  }

  if (converter != NULL)
    converter->SetVFlipState(newVFlip ^ nativeVerticalFlip);

  return true;
}


PBoolean PVideoDevice::GetFrameSizeLimits(unsigned & minWidth,
                                      unsigned & minHeight,
                                      unsigned & maxWidth,
                                      unsigned & maxHeight) 
{
  minWidth = minHeight = 1;
  maxWidth = maxHeight = UINT_MAX;
  return false;
}


PBoolean PVideoDevice::SetFrameSizeConverter(unsigned width, unsigned height, ResizeMode resizeMode)
{
  bool isMJPEGCapture = (PVideoFrameInfo::GetColourFormat() == "MJPEG") && CanCaptureVideo();

  if (!isMJPEGCapture || (((width | height) & 0xf) == 0)) {
    if (SetFrameSize(width, height)) {
      if (nativeVerticalFlip && (converter == NULL))
        converter = new PSynonymColour(*this, *this);
      if (converter != NULL) {
        converter->SetFrameSize(frameWidth, frameHeight);
        converter->SetVFlipState(nativeVerticalFlip);
      }
      return true;
    }
  }

  int outW = width;
  int outH = height;

  if (isMJPEGCapture) {
    outW = ((width + 15) / 16) * 16;
    outH = ((height + 15) / 16) * 16;
    if (resizeMode == eMaxResizeMode)
      resizeMode = eCropCentre;
  }

  // Try and get the most compatible physical frame size to convert from/to
  if (!SetNearestFrameSize(outW, outH)) {
    PTRACE(1, "PVidDev\tCannot set an apropriate size to scale from.");
    return false;
  }

  // Now create the converter ( if not already exist)
  if (converter == NULL) {
    PVideoFrameInfo src = *this;
    PVideoFrameInfo dst = *this;
    if (CanCaptureVideo()) {
      dst.SetFrameSize(width, height);
    }
    else
      src.SetFrameSize(width, height);
    dst.SetResizeMode(resizeMode);
    converter = PColourConverter::Create(src, dst);
    if (converter == NULL) {
      PTRACE(1, "PVidDev\tSetFrameSizeConverter Colour converter creation failed");
      return false;
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

  return true;
}


PBoolean PVideoDevice::SetNearestFrameSize(unsigned width, unsigned height)
{
  unsigned minWidth, minHeight, maxWidth, maxHeight;
  if (GetFrameSizeLimits(minWidth, minHeight, maxWidth, maxHeight)) {
    if (width < minWidth)
      width = minWidth;
    else if (width > maxWidth)
      width = maxWidth;

    if (height < minHeight)
      height = minHeight;
    else if (height > maxHeight)
      height = maxHeight;
  }

  return SetFrameSize(width, height);
}


PBoolean PVideoDevice::SetFrameSize(unsigned width, unsigned height)
{
#if PTRACING
  unsigned oldWidth = frameWidth;
  unsigned oldHeight = frameHeight;
#endif

  if (!PVideoFrameInfo::SetFrameSize(width, height))
    return false;

  if (converter != NULL && !converter->SetFrameSize(width, height)) {
    PTRACE(1, "PVidDev\tSetFrameSize with converter failed with " << width << 'x' << height);
    return false;
  }

  PTRACE_IF(3, oldWidth != frameWidth || oldHeight != frameHeight,
            "PVidDev\tSetFrameSize to " << frameWidth << 'x' << frameHeight);
  return true;
}


PBoolean PVideoDevice::GetFrameSize(unsigned & width, unsigned & height) const
{
  // Channels get very upset at this not returning the output size.
  if (converter == NULL)
    return PVideoFrameInfo::GetFrameSize(width, height);
  return CanCaptureVideo() ? converter->GetDstFrameSize(width, height)
                           : converter->GetSrcFrameSize(width, height);
}


const PString& PVideoDevice::GetColourFormat() const
{
  if (converter == NULL)
    return PVideoFrameInfo::GetColourFormat();
  return CanCaptureVideo() ? converter->GetDstColourFormat()
                           : converter->GetSrcColourFormat();
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


bool PVideoDevice::GetAttributes(Attributes &)
{
  return false;
}


bool PVideoDevice::SetAttributes(const Attributes &)
{
  return false;
}


PBoolean PVideoDevice::SetVideoChannelFormat (int newNumber, VideoFormat newFormat) 
{
  PBoolean err1, err2;

  err1 = SetChannel (newNumber);
  err2 = SetVideoFormat (newFormat);
  
  return (err1 && err2);
}

PStringArray PVideoDevice::GetDeviceNames() const
{
  return PStringArray();
}

////////////////////////////////////////////////////////////////////////////////////////////

PString PVideoInteractionInfo::AsString(const InputInteractType & ctype)
{
	switch (ctype) {
		case InteractKey:
			return "Remote Key Press";
		case InteractMouse:
			return "Remote Mouse Move/Click";
		case InteractNavigate:
			return "Remote Navigation";
		case InteractRTSP:
			return "Remote RTSP Commands";
		case InteractOther:
			return "Custom/Other";
	}
	return PString();
}

/////////////////////////////////////////////////////////////////////////////////////////////

PVideoInputControl::~PVideoInputControl()  
{ 
  Reset(); 
}


PBoolean PVideoInputControl::Pan(long /*value*/, bool /*absolute*/)  
{
  return false; 
}


PBoolean PVideoInputControl::Tilt(long /*value*/, bool /*absolute*/)  
{ 
  return false; 
}


PBoolean PVideoInputControl::Zoom(long /*value*/, bool /*absolute*/)  
{ 
  return false; 
}


void PVideoInputControl::Reset()
{
  PTRACE(4,"CC\tResetting camera to default position.");

  Pan (m_control[PVideoControlInfo::Pan ].m_default, true);
  Tilt(m_control[PVideoControlInfo::Tilt].m_default, true);
  Zoom(m_control[PVideoControlInfo::Zoom].m_default, true);
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice

PVideoOutputDevice::PVideoOutputDevice()
{
}


PBoolean PVideoOutputDevice::CanCaptureVideo() const
{
  return false;
}


PBoolean PVideoOutputDevice::GetPosition(int &, int &) const
{
  return false;
}


bool PVideoOutputDevice::SetPosition(int, int)
{
  return false;
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


PBoolean PVideoOutputDeviceRGB::SetColourFormat(const PString & colourFormat)
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
    return false;

  if (!PVideoOutputDevice::SetColourFormat(colourFormat))
    return false;

  bytesPerPixel = newBytesPerPixel;
  scanLineWidth = ((frameWidth*bytesPerPixel+3)/4)*4;
  return frameStore.SetSize(frameHeight*scanLineWidth);
}


PBoolean PVideoOutputDeviceRGB::SetFrameSize(unsigned width, unsigned height)
{
  PWaitAndSignal m(mutex);

  if (frameWidth == width && frameHeight == height)
    return true;

  if (!PVideoOutputDevice::SetFrameSize(width, height))
    return false;

  scanLineWidth = ((frameWidth*bytesPerPixel+3)/4)*4;
  return frameStore.SetSize(frameHeight*scanLineWidth);
}


PINDEX PVideoOutputDeviceRGB::GetMaxFrameBytes()
{
  PWaitAndSignal m(mutex);
  return GetMaxFrameBytesConverted(frameStore.GetSize());
}


PBoolean PVideoOutputDeviceRGB::SetFrameData(unsigned x, unsigned y,
                                         unsigned width, unsigned height,
                                         const BYTE * data,
                                         PBoolean endFrame)
{
  PWaitAndSignal m(mutex);

  if (x+width > frameWidth || y+height > frameHeight || PAssertNULL(data) == NULL)
    return false;

  if (x == 0 && width == frameWidth && y == 0 && height == frameHeight) {
    if (converter != NULL)
      converter->Convert(data, frameStore.GetPointer());
    else
      memcpy(frameStore.GetPointer(), data, height*scanLineWidth);
  }
  else {
    if (converter != NULL) {
      PAssertAlways("Converted output of partial RGB frame not supported");
      return false;
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

  return true;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevicePPM

#ifdef SHOULD_BE_MOVED_TO_PLUGIN

PVideoOutputDevicePPM::PVideoOutputDevicePPM()
{
  PTRACE(6, "PPM\t Constructor of PVideoOutputDevicePPM");
  frameNumber = 0;
}


PBoolean PVideoOutputDevicePPM::Open(const PString & name,
                                 PBoolean /*startImmediate*/)
{
  Close();

  PFilePath path = name;
  if (!PDirectory::Exists(path.GetDirectory()))
    return false;

  if (path != psprintf(path, 12345))
    deviceName = path;
  else
    deviceName = path.GetDirectory() + path.GetTitle() + "%u" + path.GetType();

  return true;
}


PBoolean PVideoOutputDevicePPM::IsOpen()
{
  return !deviceName;
}


PBoolean PVideoOutputDevicePPM::Close()
{
  deviceName.MakeEmpty();
  return true;
}


PStringArray PVideoOutputDevicePPM::GetDeviceNames() const
{
  return PDirectory();
}


PBoolean PVideoOutputDevicePPM::EndFrame()
{
  PFile file;
  if (!file.Open(psprintf(deviceName, frameNumber++), PFile::WriteOnly)) {
    PTRACE(1, "PPMVid\tFailed to open PPM output file \""
           << file.GetName() << "\": " << file.GetErrorText());
    return false;
  }

  file << "P6 " << frameWidth  << " " << frameHeight << " " << 255 << "\n";

  if (!file.Write(frameStore, frameStore.GetSize())) {
    PTRACE(1, "PPMVid\tFailed to write frame data to PPM output file " << file.GetName());
    return false;
  }

  PTRACE(6, "PPMVid\tFinished writing PPM file " << file.GetName());
  return file.Close();
}

#endif // SHOULD_BE_MOVED_TO_PLUGIN


///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice

PBoolean PVideoInputDevice::CanCaptureVideo() const
{
  return true;
}

static const char videoInputPluginBaseClass[] = "PVideoInputDevice";


PStringArray PVideoInputDevice::GetDriverNames(PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsProviding(videoInputPluginBaseClass);
}


PStringArray PVideoInputDevice::GetDriversDeviceNames(const PString & driverName, PPluginManager * pluginMgr)
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


PBoolean PVideoInputDevice::GetDeviceCapabilities(const PString & deviceName, Capabilities * caps, PPluginManager * pluginMgr)
{
  return GetDeviceCapabilities(deviceName, "*", caps, pluginMgr);
}


PBoolean PVideoInputDevice::GetDeviceCapabilities(const PString & deviceName, const PString & driverName, Capabilities * caps, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsDeviceCapabilities(videoInputPluginBaseClass,driverName,deviceName, caps);
}

PVideoInputControl * PVideoInputDevice::GetVideoInputControls()
{
	return NULL;
}


PVideoInputDevice * PVideoInputDevice::CreateOpenedDevice(const PString & driverName,
                                                          const PString & deviceName,
                                                          PBoolean startImmediate,
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
                                                          PBoolean startImmediate)
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


PBoolean PVideoInputDevice::SetNearestFrameSize(unsigned width, unsigned height)
{
  if (PVideoDevice::SetNearestFrameSize(width, height))
    return true;

  // Get the discrete sizes grabber is capable of
  Capabilities caps;
  if (!GetDeviceCapabilities(&caps))
    return false;

  std::list<PVideoFrameInfo>::iterator it;

  bool isMJPEG = PVideoFrameInfo::GetColourFormat() == "MJPEG";

  if (!isMJPEG) {

    // First try and pick one with the same width
    for (it = caps.framesizes.begin(); it != caps.framesizes.end(); ++it) {
      if (it->GetFrameWidth() == width)
        return SetFrameSize(width, it->GetFrameHeight());
    }

    // Then try for the same height
    for (it = caps.framesizes.begin(); it != caps.framesizes.end(); ++it) {
      if (it->GetFrameHeight() == height)
        return SetFrameSize(it->GetFrameWidth(), height);
    }

    // Then try for double the size
    for (it = caps.framesizes.begin(); it != caps.framesizes.end(); ++it) {
      unsigned w, h;
      it->GetFrameSize(w, h);
      if (w == width*2 && h == height*2)
        return SetFrameSize(w, h);
    }

    // Then try for half the size
    for (it = caps.framesizes.begin(); it != caps.framesizes.end(); ++it) {
      unsigned w, h;
      it->GetFrameSize(w, h);
      if (w == width/2 && h == height/2)
        return SetFrameSize(w, h);
    }
  }

  // Now try and pick one that has the nearest number of pixels in total.
  unsigned pixels = width*height;
  unsigned widthToUse = 0, heightToUse = 0;
  int diff = INT_MAX;
  for (it = caps.framesizes.begin(); it != caps.framesizes.end(); ++it) {
    unsigned w, h;
    it->GetFrameSize(w, h);
    int d = w*h - pixels;
    if (d < 0)
      d = -d;
    if (diff > d && 
          // Ensure we don't pick one dimension greater and one 
          // lower because we can't shrink one and grow the other.
          ((w < width && h < height) || 
	       (w > width && h > height))
    ) {
      if (!isMJPEG || (((w | h) & 0xf) == 0)) {
        diff = d;
        widthToUse = w;
        heightToUse = h;
      }
    }
  }

  if (widthToUse == 0)
    return false;

  return SetFrameSize(widthToUse, heightToUse);
}


PBoolean PVideoInputDevice::GetFrame(PBYTEArray & frame)
{
  PINDEX returned;
  if (!GetFrameData(frame.GetPointer(GetMaxFrameBytes()), &returned))
    return false;

  frame.SetSize(returned);
  return true;
}


PBoolean PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned, bool & keyFrame)
{
  keyFrame = true;
  return GetFrameData(buffer, bytesReturned);
}


PBoolean PVideoInputDevice::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned, bool & keyFrame)
{
  keyFrame = true;
  return GetFrameDataNoDelay(buffer, bytesReturned);
}


bool PVideoInputDevice::FlowControl(const void * /*flowData*/)
{
    return false;
}


bool PVideoInputDevice::SetCaptureMode(unsigned)
{
  return false;
}


int PVideoInputDevice::GetCaptureMode() const
{
  return -1;
}


PBoolean PVideoOutputDevice::SetFrameData(
      unsigned x,
      unsigned y,
      unsigned width,
      unsigned height,
      const BYTE * data,
      PBoolean endFrame,
      bool & /*keyFrameNeeded*/
)
{
  return SetFrameData(x, y, width, height, data, endFrame);
}


PBoolean PVideoOutputDevice::SetFrameData(
      unsigned x,
      unsigned y,
      unsigned width,
      unsigned height,
      unsigned /*saWidth*/,
      unsigned /*sarHeight*/,
      const BYTE * data,
      PBoolean endFrame,
      bool & keyFrameNeeded,
	  const void * /*mark*/
)
{
  return SetFrameData(x, y, width, height, data, endFrame, keyFrameNeeded);
}


PBoolean PVideoOutputDevice::DisableDecode() 
{
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////

static const char videoOutputPluginBaseClass[] = "PVideoOutputDevice";


PStringArray PVideoOutputDevice::GetDriverNames(PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsProviding(videoOutputPluginBaseClass);
}


PStringArray PVideoOutputDevice::GetDriversDeviceNames(const PString & driverName, PPluginManager * pluginMgr)
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
                                                            PBoolean startImmediate,
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
                                                            PBoolean startImmediate)
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
