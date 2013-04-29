/*
 * macvideo.cxx
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2013 Equivalence Pty. Ltd.
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
 * Contributor(s):
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#if P_VIDEO

#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>

#include <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSLock.h>
#import <QTKit/QTKit.h>


@interface PVideoInputDevice_MacFrame : NSObject
{
  NSCondition * m_grabbed;
  const void  * m_frameData;
  size_t        m_frameSize;
}

@end


@implementation PVideoInputDevice_MacFrame

- (id)init
{
  if ((self = [super init]))
  {
    m_frameData = nil;
    m_grabbed = [[NSCondition alloc] init];
  }
  
  return self;
}


// QTCapture delegate method, called when a frame has been loaded by the camera
- (void)captureOutput:(QTCaptureOutput *)captureOutput
        didOutputVideoFrame:(CVImageBufferRef)videoFrame
        withSampleBuffer:(QTSampleBuffer *)sampleBuffer
        fromConnection:(QTCaptureConnection *)connection
{
  PTRACE(6, "MacVideo", "Frame received");
  
  // If we already have an image we should use that instead
  if (m_frameData != nil)
    return;
  
  // Retain the videoFrame so it won't disappear
  // don't forget to release!
  CVBufferRetain(videoFrame);
  
  // The Apple docs state that this action must be synchronized
  // as this method will be run on another thread
  @synchronized (self) {
    m_frameData = [sampleBuffer bytesForAllSamples];
    m_frameSize = [sampleBuffer lengthForAllSamples];
    [m_grabbed signal];
  }
}


- (void)close
{
  PTRACE(6, "MacVideo", "Breaking grab block");
  [m_grabbed signal];
}


- (void)waitFrame
{
  PTRACE(6, "MacVideo", "Waiting");
  [m_grabbed wait];
}


- (bool)grabFrame:(BYTE *)buffer withWidth:(unsigned)width andHeight:(unsigned)height
{
  @synchronized (self) {
    if (m_frameData == nil)
      return false;

    const CVPlanarPixelBufferInfo_YCbCrPlanar * info =
                  (const CVPlanarPixelBufferInfo_YCbCrPlanar *)m_frameData;
    
    int32_t  offY = *(const  PInt32b *)&info->componentInfoY.offset;
    uint32_t rowY = *(const PUInt32b *)&info->componentInfoY.rowBytes;
    int32_t  offU = *(const  PInt32b *)&info->componentInfoCb.offset;
    uint32_t rowU = *(const PUInt32b *)&info->componentInfoCb.rowBytes;
    int32_t  offV = *(const  PInt32b *)&info->componentInfoCr.offset;
    uint32_t rowV = *(const PUInt32b *)&info->componentInfoCr.rowBytes;
    PTRACE(5, "MacVideo", "Frame grabbed: "
           <<offY<<' '<<rowY<<' '<<offU<<' '<<rowU<<' '<<offV<<' '<<rowV);

    const BYTE * src = (const BYTE *)m_frameData+offY;
    for (unsigned y = 0; y < height; ++y) {
      memcpy(buffer, src, width);
      buffer += width;
      src += rowY;
    }
    
    width /= 2;
    height /= 2;
    
    src = (const BYTE *)m_frameData+offU;
    for (unsigned y = 0; y < height; ++y) {
      memcpy(buffer, src, width);
      buffer += width;
      src += rowU;
    }
    
    src = (const BYTE *)m_frameData+offV;
    for (unsigned y = 0; y < height; ++y) {
      memcpy(buffer, src, width);
      buffer += width;
      src += rowV;
    }
    
    m_frameData = nil;
  }
  
  return true;
}

@end



class PVideoInputDevice_Mac : public PVideoInputDevice
{
    PCLASSINFO(PVideoInputDevice_Mac, PVideoInputDevice);
  public:
    PVideoInputDevice_Mac();
    ~PVideoInputDevice_Mac();

    PBoolean Open(const PString & deviceName, PBoolean startImmediate);
    PBoolean IsOpen() ;
    PBoolean Close();
    PBoolean Start();
    PBoolean Stop();
    PBoolean IsCapturing();
    static PStringArray GetInputDeviceNames();
    virtual PStringArray GetDeviceNames() const;
    static bool GetDeviceCapabilities(const PString &, Capabilities *) { return false; }
    virtual PINDEX GetMaxFrameBytes();
    virtual PBoolean GetFrameData(BYTE * buffer, PINDEX * bytesReturned);
    virtual PBoolean GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned);
    virtual PBoolean SetColourFormat(const PString & colourFormat);
    virtual PBoolean SetFrameRate(unsigned rate);
    virtual PBoolean SetFrameSize(unsigned width, unsigned height);

  protected:
    NSAutoreleasePool * m_pool;
    QTCaptureSession * m_session;
    QTCaptureDevice * m_device;
    QTCaptureDeviceInput * m_captureInput;
    QTCaptureDecompressedVideoOutput * m_captureOutput;
    PVideoInputDevice_MacFrame * m_captureFrame;
  
    PINDEX m_frameSizeBytes;
};

PCREATE_VIDINPUT_PLUGIN(Mac);


PVideoInputDevice_Mac::PVideoInputDevice_Mac()
  : m_device(nil)
  , m_captureInput(nil)
  , m_captureOutput(nil)
  , m_captureFrame(nil)
{
  m_pool = [[NSAutoreleasePool alloc] init];
  m_session = [[QTCaptureSession alloc] init];
  
  colourFormat = "YUV420P";
  m_frameSizeBytes = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
}


PVideoInputDevice_Mac::~PVideoInputDevice_Mac()
{
  Close();

  [m_session release];
  //[m_pool release];
}


PBoolean PVideoInputDevice_Mac::Open(const PString & devName, PBoolean startImmediate)
{
  bool opened;
  NSError *error = nil;

  if (devName.IsEmpty() || (devName *= "default")) {
    m_device = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
    opened = [m_device open:&error];
  }
  else {
    NSString * name = [NSString stringWithUTF8String:devName];
    m_device = [QTCaptureDevice deviceWithUniqueID:name];
    opened = [m_device open:&error];
    if (!opened || error != nil) {
      NSArray * devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];
      for (QTCaptureDevice * device in devices) {
        if ([[device localizedDisplayName] caseInsensitiveCompare:name] == 0) {
          m_device = [QTCaptureDevice deviceWithUniqueID:[device uniqueID]];
          opened = [m_device open:&error];
          break;
        }
      }
    }
  }

  if (!opened || error != nil) {
    PTRACE(2, "MacVideo", "Could not open device "
              "\"" << devName << "\": " << [error localizedDescription]);
    return false;
  }
  
  m_captureInput = [[QTCaptureDeviceInput alloc] initWithDevice:m_device];
  
  if (![m_session addInput:m_captureInput error:&error] || error != nil) {
    PTRACE(2, "MacVideo", "Could not add input device "
           "\"" << devName << "\": " << [error localizedDescription]);
    return false;
  }
  
  m_captureOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
  
  if (![m_session addOutput:m_captureOutput error:&error] || error != nil) {
    PTRACE(2, "MacVideo", "Could not add output for device "
           "\"" << devName << "\": " << [error localizedDescription]);
    return false;
  }

  [m_captureOutput setPixelBufferAttributes:
      [NSDictionary dictionaryWithObjectsAndKeys:
          [NSNumber numberWithInt:frameWidth],
                  (id)kCVPixelBufferWidthKey,
          [NSNumber numberWithInt:frameHeight],
                  (id)kCVPixelBufferHeightKey,
          [NSNumber numberWithUnsignedInt:kCVPixelFormatType_420YpCbCr8Planar],
                  (id)kCVPixelBufferPixelFormatTypeKey,
          nil
      ]
  ];

  if ([m_captureOutput respondsToSelector:@selector(setMinimumVideoFrameInterval)])
    [m_captureOutput setMinimumVideoFrameInterval:(1.0/frameRate)];

  m_captureFrame = [[PVideoInputDevice_MacFrame alloc] init];
  [m_captureOutput setDelegate:m_captureFrame];
  
  deviceName = devName;
  
  PTRACE(3, "MacVideo", "Opened \"" << devName << "\""
            " res=" << frameWidth << 'x' << frameHeight << '@' << frameRate);
  
  if (startImmediate)
    return Start();
  
  return true;
}


PBoolean PVideoInputDevice_Mac::IsOpen() 
{
  return m_captureFrame != nil;
}


PBoolean PVideoInputDevice_Mac::Close()
{
  PTRACE_IF(4, IsOpen(), "MacVideo", "Closing \"" << deviceName << '"');
  
  Stop();

  [m_captureFrame close];
  
  if (m_device != nil && [m_device isOpen])
    [m_device close];

  if (m_captureInput != nil) {
    [m_session removeInput:m_captureInput];
    [m_captureInput release];
    m_captureInput = nil;
  }
  
  if (m_captureOutput != nil) {
    [m_session removeOutput:m_captureOutput];
    [m_captureOutput release];
    m_captureOutput = nil;
  }

  if (m_captureFrame != nil) {
    [m_captureFrame release];
    m_captureFrame = nil;
  }
  
  if (m_device != nil) {
    [m_device release];
    m_device = nil;
  }
  
  return true;
}


PBoolean PVideoInputDevice_Mac::Start()
{
  if (!IsOpen())
    return false;
  
  [m_session startRunning];
  PTRACE(3, "MacVideo", "Started \"" << deviceName << '"');
  return [m_session isRunning];
}


PBoolean PVideoInputDevice_Mac::Stop()
{
  if (!IsOpen())
    return false;
  
  PTRACE(3, "MacVideo", "Stopping \"" << deviceName << '"');
  [m_session stopRunning];
  return ![m_session isRunning];
}


PBoolean PVideoInputDevice_Mac::IsCapturing()
{
  return IsOpen() && [m_session isRunning];
}


PStringArray PVideoInputDevice_Mac::GetInputDeviceNames()
{
  PVideoInputDevice_Mac dev;
  return dev.GetDeviceNames();
}


PStringArray PVideoInputDevice_Mac::GetDeviceNames() const
{
  PStringArray names;

  NSArray * devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];
  for (QTCaptureDevice * device in devices)
    names += [[device localizedDisplayName] cStringUsingEncoding:NSUTF8StringEncoding];

  return names;
}


PBoolean PVideoInputDevice_Mac::SetColourFormat(const PString & newFormat)
{
  return newFormat == "YUV420P";
}


PBoolean PVideoInputDevice_Mac::SetFrameRate(unsigned rate)
{
  if (rate < 1)
    rate = 1;
  else if (rate > 50)
    rate = 50;

  if (rate == frameRate)
    return true;

  if (!PVideoDevice::SetFrameRate(rate))
    return false;

  bool restart = IsCapturing();
  if (restart)
    Stop();
  
  PTRACE(3, "MacVideo", "Setting frame rate of \"" << deviceName << "\" to " << rate);
  
  if ([m_captureOutput respondsToSelector:@selector(setMinimumVideoFrameInterval)])
    [m_captureOutput setMinimumVideoFrameInterval:(1.0/rate)];
  
  if (restart)
    return Start();
  
  return true;
}


PBoolean PVideoInputDevice_Mac::SetFrameSize(unsigned width, unsigned height)
{
  if (width == frameWidth && height == frameHeight)
    return true;
  
  if (!PVideoDevice::SetFrameSize(width, height))
    return false;

  if (IsOpen()) {
    bool restart = IsCapturing();
    if (restart)
      Stop();
    
    PTRACE(3, "MacVideo", "Setting resolution of \"" << deviceName << "\""
              " to " << frameWidth << 'x' << frameHeight);
    
    [m_captureOutput setPixelBufferAttributes:
        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:frameWidth],
                (id)kCVPixelBufferWidthKey,
            [NSNumber numberWithInt:frameHeight],
                (id)kCVPixelBufferHeightKey,
            [NSNumber numberWithUnsignedInt:kCVPixelFormatType_420YpCbCr8Planar],
                (id)kCVPixelBufferPixelFormatTypeKey,
            nil
        ]
    ];
    
    m_frameSizeBytes = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
    
    if (restart)
      return Start();
  }
  else
    m_frameSizeBytes = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  
  return true;
}


PINDEX PVideoInputDevice_Mac::GetMaxFrameBytes()
{
  return m_frameSizeBytes;
}


PBoolean PVideoInputDevice_Mac::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  if (!IsCapturing())
    return false;
  
  [m_captureFrame waitFrame];
  return GetFrameDataNoDelay(buffer, bytesReturned);
}

 
PBoolean PVideoInputDevice_Mac::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * bytesReturned)
{
  if (!IsCapturing())
    return false;
  
  if (converter == NULL)
    [m_captureFrame grabFrame:destFrame withWidth:frameWidth andHeight:frameHeight];
  else {
    if (!converter->Convert(destFrame, destFrame, bytesReturned))
      return false;
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_frameSizeBytes;

  return true;
}


#endif // P_VIDEO

// End Of File ///////////////////////////////////////////////////////////////

