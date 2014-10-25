/*
 * macvideo.mm
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

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>

#ifdef BOOL
#undef BOOL
#endif

#include <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSLock.h>
#import <QTKit/QTKit.h>

#define PTraceModule() "MacVideo"
#define PTRACE_DETAILED(...) PTRACE(__VA_ARGS__)


class PLocalMemoryPool
{
  NSAutoreleasePool * m_pool;
public:
   PLocalMemoryPool() {  m_pool = [[NSAutoreleasePool alloc] init]; }
  ~PLocalMemoryPool() { [m_pool drain]; }
};


@interface PVideoInputDevice_MacFrame : NSObject
{
  CVImageBufferRef m_videoFrame;
  PSyncPoint       m_grabbed;
  bool             m_frameAvailable;
}

@end


@implementation PVideoInputDevice_MacFrame

- (id)init
{
  if ((self = [super init]))
  {
    m_frameAvailable = false;
  }
  
  return self;
}


// QTCapture delegate method, called when a frame has been loaded by the camera
- (void)captureOutput:(QTCaptureOutput *)captureOutput
        didOutputVideoFrame:(CVImageBufferRef)videoFrame
        withSampleBuffer:(QTSampleBuffer *)sampleBuffer
        fromConnection:(QTCaptureConnection *)connection
{
  PTRACE_DETAILED(5, "Frame received: m_frameAvailable=" << m_frameAvailable);

  // If we have not processed previous frame, ignore this one
  if (m_frameAvailable)
    return;

  // Retain the videoFrame so it won't disappear
  CVBufferRetain(videoFrame);
  CVImageBufferRef imageBufferToRelease = m_videoFrame;

  // The Apple docs state that this action must be synchronized
  // as this method will be run on another thread
  @synchronized (self) {
    m_videoFrame = videoFrame;
    m_frameAvailable = true;
    m_grabbed.Signal();
  }
  
  CVBufferRelease(imageBufferToRelease);
}


- (void)stop
{
  PTRACE(4, "Breaking grab block");
  m_grabbed.Signal();
}


- (void)waitFrame
{
  PTRACE_DETAILED(5, "Waiting");
  m_grabbed.Wait();
}


- (void)grabFrame:(BYTE *)buffer withWidth:(unsigned)expectedWidth andHeight:(unsigned)expectedHeight
{
  if (!m_frameAvailable)
    m_grabbed.Wait();

  CVPixelBufferRef pixels;
  
  @synchronized (self){
    pixels = CVBufferRetain(m_videoFrame);
    m_frameAvailable = false;
  }

  unsigned actualWidth = CVPixelBufferGetWidth(pixels);
  unsigned actualHeight = CVPixelBufferGetHeight(pixels);
  
  PTRACE_IF(3, expectedWidth != actualWidth || expectedHeight != actualHeight,
            "Not grabbing the size we expected: "
            << actualWidth << 'x' << actualHeight << "!=" << expectedWidth << 'x' << expectedHeight);

  PTRACE_DETAILED(5, "Frame grabbed: " << actualWidth << 'x' << actualHeight);

  if (actualWidth > expectedWidth)
    actualWidth = expectedWidth;
  if (actualHeight > expectedHeight)
    actualHeight = expectedHeight;
  
  CVPixelBufferLockBaseAddress(pixels, 0);

  const uint8_t * src = (const uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixels, 0);
  size_t rowBytes = CVPixelBufferGetBytesPerRowOfPlane(pixels, 0);
  for (unsigned y = 0; y < actualHeight; ++y) {
    memcpy(buffer, src, actualWidth);
    buffer += expectedWidth;
    src += rowBytes;
  }

  actualWidth /= 2;
  actualHeight /= 2;
  expectedWidth /= 2;
  expectedHeight /= 2;
  
  src = (const uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixels, 1);
  rowBytes = CVPixelBufferGetBytesPerRowOfPlane(pixels, 1);
  for (unsigned y = 0; y < actualHeight; ++y) {
    memcpy(buffer, src, actualWidth);
    buffer += expectedWidth;
    src += rowBytes;
  }
  
  src = (const uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixels, 2);
  rowBytes = CVPixelBufferGetBytesPerRowOfPlane(pixels, 2);
  for (unsigned y = 0; y < actualHeight; ++y) {
    memcpy(buffer, src, actualWidth);
    buffer += expectedWidth;
    src += rowBytes;
  }
    
  CVPixelBufferUnlockBaseAddress(pixels, 0);
  CVBufferRelease(pixels);
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
    static bool GetDeviceCapabilities(const PString &, Capabilities *);
    virtual bool GetDeviceCapabilities(Capabilities * caps) { return GetDeviceCapabilities(PString::Empty(), caps); }
    virtual PINDEX GetMaxFrameBytes();
    virtual PBoolean GetFrameData(BYTE * buffer, PINDEX * bytesReturned);
    virtual PBoolean GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned);
    virtual PBoolean SetColourFormat(const PString & colourFormat);
    virtual PBoolean SetFrameRate(unsigned rate);
    virtual PBoolean SetFrameSize(unsigned width, unsigned height);

  protected:
    QTCaptureSession * m_session;
    QTCaptureDevice * m_device;
    QTCaptureDeviceInput * m_captureInput;
    QTCaptureDecompressedVideoOutput * m_captureOutput;
    PVideoInputDevice_MacFrame * m_captureFrame;
  
    PINDEX m_frameSizeBytes;
    PReadWriteMutex m_mutex;
    PBYTEArray m_tempFrame;
};

PCREATE_VIDINPUT_PLUGIN(Mac);


PVideoInputDevice_Mac::PVideoInputDevice_Mac()
  : m_session(nil)
  , m_device(nil)
  , m_captureInput(nil)
  , m_captureOutput(nil)
  , m_captureFrame(nil)
{
  colourFormat = "YUV420P";
  m_frameSizeBytes = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  PTRACE(5, "Constructed.");
}


PVideoInputDevice_Mac::~PVideoInputDevice_Mac()
{
  Close();
  PTRACE(5, "Destroyed.");
}


PBoolean PVideoInputDevice_Mac::Open(const PString & devName, PBoolean startImmediate)
{
  Close();
  
  PLocalMemoryPool localPool;
  
  PWriteWaitAndSignal mutex(m_mutex);
  
  bool opened;
  NSError *error = nil;

  m_session = [[QTCaptureSession alloc] init];

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
    PTRACE(2, "Could not open device \"" << devName << "\": " << [error localizedDescription]);
    return false;
  }
  
  m_captureInput = [[QTCaptureDeviceInput alloc] initWithDevice:m_device];
  
  if (![m_session addInput:m_captureInput error:&error] || error != nil) {
    PTRACE(2, "Could not add input device "
           "\"" << devName << "\": " << [error localizedDescription]);
    return false;
  }
  
  m_captureOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
  
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

  if (![m_session addOutput:m_captureOutput error:&error] || error != nil) {
    PTRACE(2, "Could not add output for device "
           "\"" << devName << "\": " << [error localizedDescription]);
    return false;
  }
  
  m_captureFrame = [[PVideoInputDevice_MacFrame alloc] init];
  [m_captureOutput setDelegate:m_captureFrame];
  
  deviceName = devName;
  
  PTRACE(3, "Opened \"" << devName << "\""
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
  PTRACE_IF(4, IsOpen(), "Closing \"" << deviceName << '"');
  
  Stop();

  m_mutex.StartWrite();

  PLocalMemoryPool localPool;

  if (m_device != nil) {
    [m_device close];
    m_device = nil;
  }

  if (m_captureInput != nil) {
    [m_session removeInput:m_captureInput];
    [m_captureInput release];
    m_captureInput = nil;
  }

  if (m_captureOutput != nil) {
    [m_session removeOutput:m_captureOutput];
    [m_captureOutput setDelegate:nil];
    [m_captureOutput release];
    m_captureOutput = nil;
  }

  if (m_captureFrame != nil) {
    [m_captureFrame release];
    m_captureFrame = nil;
  }
  
  if (m_session != nil) {
    [m_session release];
    m_session = nil;
  }

  m_mutex.EndWrite();

  PTRACE(5, "Closed \"" << deviceName << '"');
  return true;
}


PBoolean PVideoInputDevice_Mac::Start()
{
  PReadWaitAndSignal mutex(m_mutex);
  
  if (!IsOpen())
    return false;

  PLocalMemoryPool localPool;
  
  if ([m_session isRunning])
    return true;

  PTRACE(3, "Starting \"" << deviceName << '"');
  [m_session startRunning];
  for (int retry = 0; retry < 50; ++retry) {
    if ([m_session isRunning])
      return true;
    PThread::Sleep(20);
  }
  
  [m_session stopRunning];
  PTRACE(2, "Could not start \"" << deviceName << '"');
  return false;
}


PBoolean PVideoInputDevice_Mac::Stop()
{
  PReadWaitAndSignal mutex(m_mutex);
  
  if (!IsOpen())
    return false;
  
  PLocalMemoryPool localPool;
  
  PTRACE(3, "Stopping \"" << deviceName << '"');
  [m_session stopRunning];
  [m_captureFrame stop];
  
  for (int retry = 0; retry < 50; ++retry) {
    if (![m_session isRunning]) {
      PTRACE(5, "Stopped \"" << deviceName << '"');
      return true;
    }
    PThread::Sleep(20);
  }
  
  PTRACE(2, "Could not stop \"" << deviceName << '"');
  return false;
}


PBoolean PVideoInputDevice_Mac::IsCapturing()
{
  PReadWaitAndSignal mutex(m_mutex);
  return IsOpen() && [m_session isRunning];
}


PStringArray PVideoInputDevice_Mac::GetInputDeviceNames()
{
  PVideoInputDevice_Mac dev;
  return dev.GetDeviceNames();
}


PStringArray PVideoInputDevice_Mac::GetDeviceNames() const
{
  PLocalMemoryPool localPool;
  
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

  if (!IsOpen())
    return true;
  
  bool restart = IsCapturing();
  if (restart)
    Stop();
  
  PTRACE(3, "Setting frame rate of \"" << deviceName << "\" to " << rate);
  
  PLocalMemoryPool localPool;
  
  m_mutex.StartRead();
  
  if ([m_captureOutput respondsToSelector:@selector(setMinimumVideoFrameInterval)])
    [m_captureOutput setMinimumVideoFrameInterval:(1.0/rate)];
  
  m_mutex.EndRead();
  
  if (restart)
    return Start();
  
  return true;
}


PBoolean PVideoInputDevice_Mac::SetFrameSize(unsigned width, unsigned height)
{
  if (width == frameWidth && height == frameHeight)
    return true;

  // Searched and searched but cannot figure out how to do this programmatically.
  if (!((width == 160 && height == 120) ||
        (width == 320 && height == 240) ||
        (width == 640 && height == 480) ||
        (width == 176 && height == 144) ||
        (width == 352 && height == 288)))
    return false;

  if (!PVideoDevice::SetFrameSize(width, height))
    return false;

  m_frameSizeBytes = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  
  if (IsOpen())
    return Open(deviceName, IsCapturing());
  
  return true;
}


bool PVideoInputDevice_Mac::GetDeviceCapabilities(const PString &, Capabilities * caps)
{
  PVideoFrameInfo frameInfo;
  frameInfo.SetFrameSize(160, 120);
  caps->framesizes.push_back(frameInfo);
  frameInfo.SetFrameSize(320, 240);
  caps->framesizes.push_back(frameInfo);
  frameInfo.SetFrameSize(640, 480);
  caps->framesizes.push_back(frameInfo);
  frameInfo.SetFrameSize(176, 144);
  caps->framesizes.push_back(frameInfo);
  frameInfo.SetFrameSize(352, 288);
  caps->framesizes.push_back(frameInfo);
  return true;
}


PINDEX PVideoInputDevice_Mac::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(m_frameSizeBytes);
}


PBoolean PVideoInputDevice_Mac::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  PReadWaitAndSignal mutex(m_mutex);
  
  if (!IsCapturing())
    return false;
  
  [m_captureFrame waitFrame];
  return GetFrameDataNoDelay(buffer, bytesReturned);
}

 
PBoolean PVideoInputDevice_Mac::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * bytesReturned)
{
  PTRACE_DETAILED(5, "Get frame, converter=" << converter);
  
  if (!IsCapturing())
    return false;

  PReadWaitAndSignal mutex(m_mutex);

  if (converter != NULL) {
    [m_captureFrame grabFrame:m_tempFrame.GetPointer(m_frameSizeBytes) withWidth:frameWidth andHeight:frameHeight];
    if (!converter->Convert(m_tempFrame, destFrame, bytesReturned))
      return false;
  }
  else {
    [m_captureFrame grabFrame:destFrame withWidth:frameWidth andHeight:frameHeight];
    if (bytesReturned != NULL)
      *bytesReturned = m_frameSizeBytes;
  }
  
  return true;
}


#endif // P_VIDEO

// End Of File ///////////////////////////////////////////////////////////////

