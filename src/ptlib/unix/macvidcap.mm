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
#import <AVFoundation/AVFoundation.h>

#define PTraceModule() "MacVideo"
#define PTRACE_DETAILED(...) PTRACE(4, __VA_ARGS__)


class PLocalMemoryPool
{
  NSAutoreleasePool * m_pool;
public:
   PLocalMemoryPool() {  m_pool = [[NSAutoreleasePool alloc] init]; }
  ~PLocalMemoryPool() { [m_pool drain]; }
};


@interface PVideoInputDevice_MacFrame : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
{
  OSType     m_pixelFormat;
  PBYTEArray m_frameBuffer;
  unsigned   m_frameWidth;
  unsigned   m_frameHeight;
  PSyncPoint m_grabbed;
  bool       m_frameAvailable;
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


// AVCapture delegate method, called when a frame has been loaded by the camera
- (void)captureOutput:        (AVCaptureOutput *)captureOutput
        didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
        fromConnection:       (AVCaptureConnection *)connection
{

  // If we have not processed previous frame, ignore this one
  if (m_frameAvailable) {
    PTRACE_DETAILED("Frame received, but previous frame not yet processed");
    return;
  }

  // The Apple docs state that this action must be synchronized
  // as this method will be run on another thread
  @synchronized (self) {
    CVImageBufferRef pixels = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (pixels == NULL) {
      PTRACE(2, "Could not get image buffer form sample.");
      return;
    }
    
    OSType sourcePixelFormat = CVPixelBufferGetPixelFormatType(pixels);
    size_t actualWidth = CVPixelBufferGetWidth(pixels);
    size_t actualHeight = CVPixelBufferGetHeight(pixels);
    
    if (sourcePixelFormat != m_pixelFormat || actualWidth != m_frameWidth || actualHeight != m_frameHeight) {
      PTRACE(4, "Buffer has incorrect resolution or pixel format.");
      return;
    }
    
    uint8_t * buffer = m_frameBuffer.GetPointer();
    
    CVPixelBufferLockBaseAddress(pixels, kCVPixelBufferLock_ReadOnly);

    size_t planes = CVPixelBufferGetPlaneCount(pixels);
    for (int plane = 0; plane < planes; ++plane) {
      const uint8_t * src = (const uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixels, plane);
      size_t planeWidth = CVPixelBufferGetBytesPerRowOfPlane(pixels, plane);
      size_t planeHeight = CVPixelBufferGetHeightOfPlane(pixels, plane);
      size_t planeSize = planeWidth*planeHeight;
      if (planeSize == actualWidth*actualHeight) {
        memcpy(buffer, src, planeSize);
        buffer += planeSize;
      }
      else {
        size_t copyWidth = std::min(actualWidth, planeWidth);
        for (unsigned y = 0; y < planeHeight; ++y) {
          memcpy(buffer, src, copyWidth);
          buffer += actualWidth;
          src += planeWidth;
        }
      }

      if (plane == 0) {
        switch (m_pixelFormat) {
          case kCVPixelFormatType_420YpCbCr8Planar :
            actualWidth /= 2;
          case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange :
            actualHeight /= 2;
        }
      }
    }
    
    CVPixelBufferUnlockBaseAddress(pixels, kCVPixelBufferLock_ReadOnly);
    
    PTRACE_DETAILED("Frame grabbed: " << actualWidth << 'x' << actualHeight << ", " << planes << " planes, "
                    << (buffer - m_frameBuffer.GetPointer()) << " bytes copied, size=" << m_frameBuffer.GetSize());
    m_frameAvailable = true;
    m_grabbed.Signal();
  }
}


- (void)stop
{
  PTRACE(4, "Breaking grab block");
  m_grabbed.Signal();
}


- (void)waitFrame
{
  m_grabbed.Wait();
}


- (bool)grabFrame:(BYTE *)destFrame withConverter:(PColourConverter *)converter returningBytes:(PINDEX *)bytesReturned
{
  if (!m_frameAvailable)
    m_grabbed.Wait();

  @synchronized (self){
    m_frameAvailable = false;
    
    if (converter != NULL)
      return converter->Convert(m_frameBuffer, destFrame, bytesReturned);

    memcpy(destFrame, m_frameBuffer, m_frameBuffer.GetSize());
    if (bytesReturned != NULL)
      *bytesReturned = m_frameBuffer.GetSize();
    return true;
  }
}


- (void)setFrameSize:(PINDEX)size withWidth:(unsigned)width andHeight:(unsigned)height andFormat:(OSType)pixelFormat
{
  @synchronized (self){
    m_pixelFormat = pixelFormat;
    m_frameBuffer.SetSize(size);
    m_frameWidth = width;
    m_frameHeight = height;
  }
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
    AVCaptureSession * m_session;
    AVCaptureDevice * m_device;
    AVCaptureDeviceInput * m_captureInput;
    AVCaptureVideoDataOutput * m_captureOutput;
    PVideoInputDevice_MacFrame * m_captureFrame;

    typedef std::map<PString, AVCaptureDeviceFormat *> FormatMap;
    FormatMap m_availableFormats;

    PINDEX m_frameSizeBytes;
    PReadWriteMutex m_mutex;
};

PCREATE_VIDINPUT_PLUGIN(Mac);


PVideoInputDevice_Mac::PVideoInputDevice_Mac()
  : m_session(nil)
  , m_device(nil)
  , m_captureInput(nil)
  , m_captureOutput(nil)
  , m_captureFrame(nil)
{
  m_colourFormat.MakeEmpty();
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
  NSError *error = nil;
  
  PWriteWaitAndSignal mutex(m_mutex);
  
  m_session = [[AVCaptureSession alloc] init];

  if (devName.IsEmpty() || (devName *= "default"))
    m_device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
  else {
    NSString * name = [NSString stringWithUTF8String:devName];
    NSArray * devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice * device in devices) {
      if ([[device localizedName] caseInsensitiveCompare:name] == 0) {
        m_device = [AVCaptureDevice deviceWithUniqueID:[device uniqueID]];
        break;
      }
    }
  }

  if (m_device == nil || error != nil) {
    PTRACE(2, "Could not open device \"" << devName << "\": " << [error localizedDescription]);
    return false;
  }

  PString oldColourFormat = m_colourFormat;
  
  m_availableFormats.clear();
  for (AVCaptureDeviceFormat * format in m_device.formats) {
    FourCharCode subType = CMFormatDescriptionGetMediaSubType(format.formatDescription);
    char subTypeStr[5] = {(subType >> 24) & 0xFF, (subType >> 16) & 0xFF, (subType >> 8) & 0xFF, subType & 0xFF, 0};
    // Do the SetColourFormat to get the PTLib synonym name for known FourCharCode.
    if (PVideoInputDevice::SetColourFormat(subTypeStr) && CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat))
      m_availableFormats[m_colourFormat] = format;
  }
  
  if (m_availableFormats.empty()) {
    PTRACE(2, "Camera does not provide a sub type (colour format) we understand");
    return false;
  }

  if (!SetColourFormat(oldColourFormat) && !SetColourFormat(PString::Empty()))
    return false;

  // Must be OK, or above would have failed
  AVCaptureDeviceFormat * selectedFormat = m_availableFormats[m_colourFormat];
  OSType pixelFormat = CMFormatDescriptionGetMediaSubType(selectedFormat.formatDescription);
  
  if ([m_device lockForConfiguration:NULL] != YES) {
    PTRACE(2, "Camera could not be locked");
    return false;
  }
  
  m_device.activeFormat = selectedFormat;
  m_device.activeVideoMinFrameDuration = CMTimeMake(1, m_frameRate);
  m_device.activeVideoMaxFrameDuration = CMTimeMake(1, m_frameRate);
  [m_device unlockForConfiguration];

  m_captureInput = [AVCaptureDeviceInput deviceInputWithDevice:m_device error:&error];
  if (error != nil) {
    PTRACE(2, "Could not get input device "
           "\"" << devName << "\": " << [error localizedDescription]);
    return false;
  }
  
  if (![m_session canAddInput:m_captureInput]) {
    PTRACE(2, "Could not add input device \"" << devName << '"');
    return false;
  }

  [m_session addInput:m_captureInput];
  
  m_captureOutput = [[AVCaptureVideoDataOutput alloc] init];
  m_captureOutput.alwaysDiscardsLateVideoFrames = NO;
  m_captureOutput.videoSettings =
    @{
      (id)kCVPixelBufferPixelFormatTypeKey: [NSNumber numberWithUnsignedInt:pixelFormat],
      (id)kCVPixelBufferWidthKey: [NSNumber numberWithUnsignedInt:m_frameWidth],
      (id)kCVPixelBufferHeightKey: [NSNumber numberWithUnsignedInt:m_frameHeight]
    };

  if (![m_session canAddOutput:m_captureOutput]) {
    PTRACE(2, "Could not add output capture for \"" << devName << '"');
    return false;
  }
  
  [m_session addOutput:m_captureOutput];
  
  [m_session commitConfiguration];
  
  m_captureFrame = [[PVideoInputDevice_MacFrame alloc] init];
  dispatch_queue_t captureQueue = dispatch_queue_create( "captureQueue", DISPATCH_QUEUE_SERIAL );
  [m_captureOutput setSampleBufferDelegate:m_captureFrame queue:captureQueue];
  
  m_frameSizeBytes = CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat);
  [m_captureFrame setFrameSize:m_frameSizeBytes withWidth:m_frameWidth andHeight:m_frameHeight andFormat:pixelFormat];
  
  m_deviceName = devName;

  PTRACE(3, "Opened \"" << devName << "\""
            " res=" << m_frameWidth << 'x' << m_frameHeight << '@' << m_frameRate);
  
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
  PTRACE_IF(4, IsOpen(), "Closing \"" << m_deviceName << '"');
  
  Stop();

  m_mutex.StartWrite();

  if (m_captureOutput != nil) {
    [m_session removeOutput:m_captureOutput];
    [m_captureOutput setSampleBufferDelegate:nil queue:nil];
    [m_captureOutput release];
    m_captureOutput = nil;
  }

  if (m_captureFrame != nil) {
    [m_captureFrame release];
    m_captureFrame = nil;
  }
  
  if (m_captureInput != nil) {
    [m_session removeInput:m_captureInput];
    // No need to release captureInput as owned by session
    m_captureInput = nil;
  }
  
  // Do not release this as is global object
  m_device = nil;

  if (m_session != nil) {
    [m_session release];
    m_session = nil;
  }

  m_mutex.EndWrite();

  PTRACE(5, "Closed \"" << m_deviceName << '"');
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

  PTRACE(3, "Starting \"" << m_deviceName << '"');
  [m_session startRunning];
  for (int retry = 0; retry < 50; ++retry) {
    if ([m_session isRunning])
      return true;
    PThread::Sleep(20);
  }
  
  [m_session stopRunning];
  PTRACE(2, "Could not start \"" << m_deviceName << '"');
  return false;
}


PBoolean PVideoInputDevice_Mac::Stop()
{
  PReadWaitAndSignal mutex(m_mutex);
  
  if (!IsOpen())
    return false;
  
  PLocalMemoryPool localPool;
  
  PTRACE(3, "Stopping \"" << m_deviceName << '"');
  [m_session stopRunning];
  [m_captureFrame stop];
  
  for (int retry = 0; retry < 50; ++retry) {
    if (![m_session isRunning]) {
      PTRACE(5, "Stopped \"" << m_deviceName << '"');
      return true;
    }
    PThread::Sleep(20);
  }
  
  PTRACE(2, "Could not stop \"" << m_deviceName << '"');
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

  NSArray * devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
  for (AVCaptureDevice * device in devices)
    names += [[device localizedName] cStringUsingEncoding:NSUTF8StringEncoding];

  return names;
}


PBoolean PVideoInputDevice_Mac::SetColourFormat(const PString & newFormat)
{
  if (m_colourFormat *= newFormat)
    return true;
  
  if (newFormat.IsEmpty())
    return PVideoInputDevice::SetColourFormat(newFormat);

  if (m_availableFormats.find(newFormat) == m_availableFormats.end()) {
    PTRACE(4, "Cannot use format \"" << newFormat << '"');
    return false;
  }

  m_colourFormat = newFormat;
  return !IsOpen() || Open(m_deviceName, IsCapturing());
}


PBoolean PVideoInputDevice_Mac::SetFrameRate(unsigned rate)
{
  if (rate < 1)
    rate = 1;
  else if (rate > 50)
    rate = 50;

  if (rate == m_frameRate)
    return true;

  if (!PVideoDevice::SetFrameRate(rate))
    return false;

  return !IsOpen() || Open(m_deviceName, IsCapturing());
}


PBoolean PVideoInputDevice_Mac::SetFrameSize(unsigned width, unsigned height)
{
  if (width == m_frameWidth && height == m_frameHeight)
    return true;

  if (!PVideoDevice::SetFrameSize(width, height))
    return false;

  return !IsOpen() || Open(m_deviceName, IsCapturing());
}


bool PVideoInputDevice_Mac::GetDeviceCapabilities(const PString &, Capabilities * caps)
{
  PVideoFrameInfo frameInfo;
  frameInfo.SetFrameSize(160, 120);
  caps->m_frameSizes.push_back(frameInfo);
  frameInfo.SetFrameSize(320, 240);
  caps->m_frameSizes.push_back(frameInfo);
  frameInfo.SetFrameSize(640, 480);
  caps->m_frameSizes.push_back(frameInfo);
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
  
  PTRACE_DETAILED("Waiting");
  [m_captureFrame waitFrame];
  return GetFrameDataNoDelay(buffer, bytesReturned);
}

 
PBoolean PVideoInputDevice_Mac::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * bytesReturned)
{
  PTRACE_DETAILED("Get frame: " << *this << ", converter=" << m_converter);
  
  if (!IsCapturing())
    return false;

  PReadWaitAndSignal mutex(m_mutex);
  return [m_captureFrame grabFrame:destFrame withConverter:m_converter returningBytes:bytesReturned];
}


#endif // P_VIDEO

// End Of File ///////////////////////////////////////////////////////////////

