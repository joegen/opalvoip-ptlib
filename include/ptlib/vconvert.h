/*
 * vconvert.h
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
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *
 * $Log: vconvert.h,v $
 * Revision 1.1  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 *
 */


#define _PCONVERT

#ifdef __GNUC__
#pragma interface
#endif



/**This class defines a means to convert an image from one format.
 */
class PVideoConvert 
{
//  PCLASSINFO(PVideoConvert, PObject);

  public:
    /** Create a new video output device.
        Allocates internal buffer memory. Open fails if cannot alloc memory.
     */
    PVideoConvert(
      int       srcColourFormat,
      int       destColourFormat,
      PINDEX    width,
      PINDEX    height );

    /** Destructor. Frees up internal buffer memory.
     */
    ~PVideoConvert() { Close(); }

     /**Open operation is required to be a descendant of PVideoDevice.
        Open process occurs in the constructor, so this function does nothing.
	     */
    BOOL Open(
        const PString & /*deviceName*/,   /// Device name to open
        BOOL /*startImmediate = TRUE*/    /// Immediately start device
        ) { return TRUE; }

    /** Do the conversion. (first copy data to an internal buffer )
    */
      BOOL ConvertWithCopy(BYTE * src, BYTE * dest);

      /**Convert image at src pointer into buffer pointed to by dest
      */      
      BOOL ConvertDirectly(BYTE * src, BYTE * dest);
            
     /** Obtain pointer to the internal buffer.
         Necessary when processing the output of ::ioctl(READ
       */    
      BYTE * GetInternalBuffer() 
           { return internalBuffer; }
  
      /** Take the data in the internal buffer and convert it to the
         required format and put into the region pointed by dest.
	    */
      BOOL ConvertInternalBuffer(BYTE *dest);
     
      /** YUV422 to YUV411P format 
      */
      BOOL Yuv422ToYuv411p(BYTE *src, BYTE * dest);

      BOOL Close();

      
  private:
      BYTE * internalBuffer;
    
      PINDEX  frameWidth;
      PINDEX  frameHeight;
      PINDEX  srcFrameSize;
      PINDEX  destFrameSize;

      int srcColourFormat;
      int destColourFormat;
 };
 
////////////////////////////////////////////////////////////////////////
// End of file

