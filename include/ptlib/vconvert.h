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
 *                 Thorsten Westheider (thorsten.westheider@teleos-web.de)
 *                 Mark Cooke (mpc@star.sr.bham.ac.uk)
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_CONVERT_H
#define PTLIB_CONVERT_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_VIDEO

#include <ptlib/videoio.h>

struct jdec_private;


/**This class contains a pair of colour formats for conversion.
*/
class PColourPair : public PObject
{
    PCLASSINFO(PColourPair, PObject)
  public:
    PColourPair(const PString & src, const PString & dst)
      : m_srcColourFormat(src)
      , m_dstColourFormat(dst)
    { }

    Comparison Compare(const PObject & other) const;

    /**Get the source colour format.
    */
    const PString & GetSrcColourFormat() const { return m_srcColourFormat; }

    /**Get the destination colour format.
    */
    const PString & GetDstColourFormat() const { return m_dstColourFormat; }

  protected:
    const PString m_srcColourFormat;
    const PString m_dstColourFormat;
};

/**This class defines a means to convert an image from one colour format to another.
   It is an ancestor class for the individual formatting functions.
 */
class PColourConverter : public PColourPair
{
    PCLASSINFO(PColourConverter, PColourPair);
  protected:
    /**Create a new converter.
      */
    PColourConverter(
      const PColourPair & colours
    );

  public:
    /// Print description of converter
    virtual void PrintOn(
      ostream & strm
    ) const;

    /**Get the video conversion vertical flip state
     */
    bool GetVFlipState() const
    { return m_verticalFlip; }
    
    /**Set the video conversion vertical flip state
     */
    void SetVFlipState(
      bool vFlipState  ///< New state for flipping images
    ) { m_verticalFlip = vFlipState; }
    
    /**Set the frame size to be used.

       Default behaviour calls SetSrcFrameSize() and SetDstFrameSize().
    */
    virtual PBoolean SetFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    /**Set the source frame info to be used.

       Default behaviour sets the srcFrameWidth and srcFrameHeight variables and
       recalculates the frame buffer size in bytes then returns true if the size
       was calculated correctly.

       Returns false if the colour formats do not agree.
    */
    virtual PBoolean SetSrcFrameInfo(
      const PVideoFrameInfo & info   ///< New info for frame
    );

    /**Set the destination frame info to be used.

       Default behaviour sets the dstFrameWidth and dstFrameHeight variables,
       and the scale / crop preference. It then recalculates the frame buffer
       size in bytes then returns true if the size was calculated correctly.

       Returns false if the colour formats do not agree.
    */
    virtual PBoolean SetDstFrameInfo(
      const PVideoFrameInfo & info  ///< New info for frame
    );

    /**Get the source frame info to be used.
    */
    virtual void GetSrcFrameInfo(
      PVideoFrameInfo & info   ///< New info for frame
    );

    /**Get the destination frame info to be used.
    */
    virtual void GetDstFrameInfo(
      PVideoFrameInfo & info  ///< New info for frame
    );

    /**Set the source frame size to be used.

       Default behaviour sets the srcFrameWidth and srcFrameHeight variables and
       recalculates the frame buffer size in bytes then returns true if the size
       was calculated correctly.
    */
    virtual PBoolean SetSrcFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    /**Set the destination frame size to be used.

       Default behaviour sets the dstFrameWidth and dstFrameHeight variables,
       and the scale / crop preference. It then recalculates the frame buffer
       size in bytes then returns true if the size was calculated correctly.
    */
    virtual PBoolean SetDstFrameSize(
      unsigned width,   ///< New width of target frame
      unsigned height   ///< New height of target frame
    );
    virtual PBoolean SetDstFrameSize(
      unsigned width,   ///< New width of target frame
      unsigned height,  ///< New height of target frame
      PBoolean bScale   ///< Indicate if scaling or cropping is to be used
    );

    /**Get the maximum frame size in bytes for source frames.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    PINDEX GetMaxSrcFrameBytes() const { return m_srcFrameBytes; }

    /**Set the actual frame size in bytes for source frames.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    void SetSrcFrameBytes(PINDEX frameBytes) { m_srcFrameBytes = frameBytes; }

    /**Get the maximum frame size in bytes for destination frames.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    PINDEX GetMaxDstFrameBytes() const { return m_dstFrameBytes; }


    /**Convert from one colour format to another.
       This version will copy the data from one frame buffer to another.
       An implementation of this function should allow for the case of
       where srcFrameBuffer and dstFrameBuffer are the same, if the conversion
       algorithm allows for that to occur without an intermediate frame store.

       The function should return false if srcFrameBuffer and dstFrameBuffer
       are the same and that form pf conversion is not allowed
    */
    virtual PBoolean Convert(
      const BYTE * srcFrameBuffer,  ///< Frame store for source pixels
      BYTE * dstFrameBuffer,        ///< Frame store for destination pixels
      PINDEX * bytesReturned = NULL ///< Bytes written to dstFrameBuffer
    ) = 0;

    /**Convert from one colour format to another.
       This version will copy the data from one frame buffer to the same frame
       buffer. Not all conversions can do this so an intermediate store and
       copy may be required. If the noIntermediateFrame parameter is true
       and the conversion cannot be done in place then the function returns
       false. If the in place conversion can be done then that parameter is
       ignored.

       Note that the frame should be large enough to take the destination
       pixels.

       Default behaviour calls Convert() from the frameBuffer to itself, and
       if that returns false then calls it again (provided noIntermediateFrame
       is false) using an intermediate store, copying the intermediate store
       back to the original frame store.
    */
    virtual PBoolean ConvertInPlace(
      BYTE * frameBuffer,               ///< Frame buffer to translate data
      PINDEX * bytesReturned = NULL,    ///< Bytes written to frameBuffer
      PBoolean noIntermediateFrame = false  ///< Flag to use intermediate store
    );


    /**Create an instance of a colour conversion function.
       Returns NULL if there is no registered colour converter between the two
       named formats.
      */
    static PColourConverter * Create(
      const PVideoFrameInfo & src, ///< Source frame info (colour formet, size etc)
      const PVideoFrameInfo & dst  ///< Destination frame info
    );
    static PColourConverter * Create(
      const PString & srcColourFormat,
      const PString & destColourFormat,
      unsigned width,
      unsigned height
    );

    /**Get the output frame size.
      */
    PBoolean GetDstFrameSize(
      unsigned & width, ///< Width of destination frame
      unsigned & height ///< Height of destination frame
    ) const;

    /**Get the input frame size.
      */
    PBoolean GetSrcFrameSize(
      unsigned & width, ///< Width of source frame
      unsigned & height ///< Height of source frame
    ) const;

    unsigned GetSrcFrameWidth()  const { return m_srcFrameWidth;  }
    unsigned GetSrcFrameHeight() const { return m_srcFrameHeight; }
    unsigned GetDstFrameWidth()  const { return m_dstFrameWidth;  }
    unsigned GetDstFrameHeight() const { return m_dstFrameHeight; }

    /**Set the resize mode to be used.
    */
    void SetResizeMode(
      PVideoFrameInfo::ResizeMode mode
    ) { if (mode < PVideoFrameInfo::eMaxResizeMode) m_resizeMode = mode; }

    /**Get the resize mode to be used.
    */
    PVideoFrameInfo::ResizeMode GetResizeMode() const { return m_resizeMode; }

    /**Convert RGB to YUV.
      */
    static void RGBtoYUV(
      unsigned r, unsigned g, unsigned b,
      BYTE   & y, BYTE   & u, BYTE   & v
    );

    /**Convert YUV to RGB.
      */
    static void YUVtoRGB(
      unsigned y, unsigned u, unsigned v,
      BYTE   & r, BYTE   & g, BYTE   & b
    );

    /**Copy a section of the source frame to a section of the destination
       frame with scaling/cropping as required.
      */
    static bool CopyYUV420P(
      unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
      unsigned srcFrameWidth, unsigned srcFrameHeight, const BYTE * srcYUV,
      unsigned dstX, unsigned dstY, unsigned dstWidth, unsigned dstHeight,
      unsigned dstFrameWidth, unsigned dstFrameHeight, BYTE * dstYUV,
      PVideoFrameInfo::ResizeMode resizeMode = PVideoFrameInfo::eScale,
      bool verticalFlip = false, std::ostream * error = NULL
    );

    /**Rotate the video buffer image.
       At this time, the \p angle may be 90, -90 or 180.
       Note: if dstYUV is NULL for an in-place rotation, then there may be a
       performance hit due to allocating an internal buffer and copying back
       to the source memory.
      */
    static bool RotateYUV420P(
      int angle, unsigned width, unsigned height, BYTE * srcYUV, BYTE * dstYUV = NULL
    );

    /**Fill a rectangle of the video buffer with the specified colour.
      */
    static bool FillYUV420P(
      unsigned x, unsigned y, unsigned width, unsigned height,
      unsigned frameWidth, unsigned frameHeight, BYTE * yuv,
      unsigned r, unsigned g, unsigned b
    );

  protected:
    unsigned m_srcFrameWidth;
    unsigned m_srcFrameHeight;
    PINDEX   m_srcFrameBytes;

    // Needed for resizing
    unsigned m_dstFrameWidth;
    unsigned m_dstFrameHeight;
    PINDEX   m_dstFrameBytes;

    PVideoFrameInfo::ResizeMode m_resizeMode;
    bool                        m_verticalFlip;

    PBYTEArray m_intermediateFrameStore;

  P_REMOVE_VIRTUAL(PBoolean,Convert(const BYTE*,BYTE*,unsigned,PINDEX*),false);
};

typedef PFactory<PColourConverter, PColourPair> PColourConverterFactory;


/**Declare a colour converter class with Convert() function.
   This should only be used once and at the global scope level for each
   converter. It declares everything needs so only the body of the Convert()
   function need be added.
   */
#define PCOLOUR_CONVERTER2(cls,ancestor,srcFmt,dstFmt) \
class cls : public ancestor { \
  public: \
    cls() : ancestor(PColourPair(srcFmt, dstFmt)) { } \
    virtual PBoolean Convert(const BYTE *, BYTE *, PINDEX * = NULL); \
}; \
PFACTORY_CREATE(PColourConverterFactory, cls, PColourPair(srcFmt, dstFmt)); \
PBoolean cls::Convert(const BYTE *srcFrameBuffer, BYTE *dstFrameBuffer, PINDEX * bytesReturned) \


/**Declare a colour converter class with Convert() function.
   This should only be used once and at the global scope level for each
   converter. It declares everything needs so only the body of the Convert()
   function need be added.
  */
#define PCOLOUR_CONVERTER(cls,src,dst) \
        PCOLOUR_CONVERTER2(cls,PColourConverter,src,dst)



/**Define synonym colour format converter.
   This is a class that defines for which no conversion is required between
   the specified colour format names.
  */
class PSynonymColour : public PColourConverter {
  public:
    PSynonymColour(
      const PColourPair & colours
    ) : PColourConverter(colours) { }
    virtual PBoolean Convert(const BYTE *, BYTE *, PINDEX * = NULL);
};


/**Define synonym colour format.
   This is a class that defines for which no conversion is required between
   the specified colour format names.
   */
#define PSYNONYM_COLOUR_CONVERTER(from,to) \
  class PColourConverter_##from##_##to : public PSynonymColour { \
    public: PColourConverter_##from##_##to() : PSynonymColour(PColourPair(#from, #to)) { }\
  }; \
  PColourConverterFactory::Worker<PColourConverter_##from##_##to> PColourConverter_##from##_##to##_instance(PColourPair(#from, #to))


#if P_JPEG_DECODER

/**Class to convert a JPEG image to other formats.
   Simplest usage is to load to a YUV420P buffer:
   <code>
     PBYTEArray yuv;
     PJPEGConverter converter;
     converter.Load(file, yuv)
   </code>
 */
class PJPEGConverter : public PColourConverter
{
  protected:
    struct Context;
    Context * m_context;

  public:
    /**Construct a JPEG converter that outputs YUV420P at same resolution as the JPEG itself.
      */
    PJPEGConverter();
    /**Construct a JPEG converter that outputs YUV420P at same resolution as the JPEG itself.
      */
    PJPEGConverter(
      unsigned width,   ///< Output width, zero indicates same is JPEG input
      unsigned height,  ///< Output height, zero indicates same is JPEG input
      PVideoFrameInfo::ResizeMode resizeMode = PVideoFrameInfo::eScale, ///< How to produce output
      const PString & colourFormat = PVideoFrameInfo::YUV420P() ///< Output colour format
    );
    /**Construct a JPEG converter.
       This is used for the PColourConverter factory.
      */
    PJPEGConverter(
      const PColourPair & colours
    );

    /**Deprecated, used for backward compatibility.
      */
    PJPEGConverter(
      const PVideoFrameInfo & src,
      const PVideoFrameInfo & dst
    );

    /// Destroy the JPEG converter
    ~PJPEGConverter();

    /** Convert JPEG information in a memory buffer to another memory buffer.
        Note if scaling is required
      */
    virtual PBoolean Convert(
      const BYTE * srcFrameBuffer,  ///< Frame store for source pixels
      BYTE * dstFrameBuffer,        ///< Frame store for destination pixels
      PINDEX * bytesReturned = NULL ///< Bytes written to dstFrameBuffer
    );

    /** Load a file and convert to the output format for the converter.
      */
    bool Load(
      const PFilePath & filename,   ///< Name of file to load
      PBYTEArray & dstFrameBuffer   ///< Buffer to receive converted output
    );
    bool Load(
      PFile & file,                 ///< File to read JPEG from.
      PBYTEArray & dstFrameBuffer   ///< Buffer to receive converted output
    );
};

#endif  // P_JPEG_DECODER

#endif // P_VIDEO

#endif // PTLIB_CONVERT_H


// End of file ///////////////////////////////////////////////////////////////
