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
 *		   Thorsten Westheider (thorsten.westheider@teleos-web.de)
 *
 * $Log: vconvert.h,v $
 * Revision 1.7  2001/03/08 23:36:02  robertj
 * Added backward compatibility SetFrameSize() function.
 * Added internal SimpleConvert() function for same type converters.
 * Fixed some documentation.
 *
 * Revision 1.6  2001/03/08 08:31:34  robertj
 * Numerous enhancements to the video grabbing code including resizing
 *   infrastructure to converters. Thanks a LOT, Mark Cooke.
 *
 * Revision 1.5  2001/03/07 01:42:59  dereks
 * miscellaneous video fixes. Works on linux now. Add debug statements
 * (at PTRACE level of 1)
 *
 * Revision 1.4  2001/03/03 23:25:07  robertj
 * Fixed use of video conversion function, returning bytes in destination frame.
 *
 * Revision 1.3  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
 * Revision 1.2  2000/12/19 23:58:14  robertj
 * Fixed MSVC compatibility issues.
 *
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



class PColourConverter;

/**This class registers a colour conversion class.
   There should be one and one only instance of this class for each pair of
   srcColurFormat and dstColourFormat strings. Use the
   PCOLOUR_CONVERTER_REGISTRATION macro to do this.
 */
class PColourConverterRegistration : public PString
{
    PCLASSINFO(PColourConverterRegistration, PString);
  public:
    PColourConverterRegistration(
      const PString & srcColourFormat,  /// Name of source colour format
      const PString & destColourFormat  /// Name of destination colour format
    );

    virtual PColourConverter * Create(
      unsigned width,   /// Width of frame
      unsigned height   /// Height of frame
    ) const = 0;
};


/**Internal list of registered colour conversion classes.
  */
class PColourConverterRegistrations : PSortedStringList
{
    PCLASSINFO(PColourConverterRegistrations, PSortedStringList);
  public:
    PColourConverterRegistrations();

    void Register(PColourConverterRegistration * reg);

  friend class PColourConverter;
};


/**This class defines a means to convert an image from one colour format to another.
   It is an ancestor class for the individual formatting functions.
 */
class PColourConverter : public PObject
{
    PCLASSINFO(PColourConverter, PObject);
  public:
    /**Create a new converter.
      */
    PColourConverter(
      const PString & srcColourFormat,  /// Name of source colour format
      const PString & dstColourFormat,  /// Name of destination colour format
      unsigned width,   /// Width of frame
      unsigned height   /// Height of frame
    );

    /**Set the frame size to be used.

       Default behaviour calls SetSrcFrameSize() and SetDstFrameSize().
    */
    virtual BOOL SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );

    /**Set the source frame size to be used.

       Default behaviour sets the srcFrameWidth and srcFrameHeight variables and
       recalculates the frame buffer size in bytes then returns TRUE if the size
       was calculated correctly.
    */
    virtual BOOL SetSrcFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );

    /**Set the destination frame size to be used.

       Default behaviour sets the dstFrameWidth and dstFrameHeight variables,
       and the scale / crop preference. It then recalculates the frame buffer
       size in bytes then returns TRUE if the size was calculated correctly.
    */
    virtual BOOL SetDstFrameSize(
      unsigned width,   /// New width of target frame
      unsigned height,  /// New height of target frame
      BOOL     bScale   /// TRUE if scaling is preferred over crop
    );

    /**Get the source colour format.
      */
    const PString & GetSrcColourFormat() { return srcColourFormat; }

    /**Get the destination colour format.
      */
    const PString & GetDstColourFormat() { return dstColourFormat; }

    /**Get the maximum frame size in bytes for source frames.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    PINDEX GetMaxSrcFrameBytes() { return srcFrameBytes; }

    /**Get the maximum frame size in bytes for destination frames.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    PINDEX GetMaxDstFrameBytes() { return dstFrameBytes; }


    /**Convert from one colour format to another.
       This version will copy the data from one frame buffer to another.
       An implementation of this function should allow for the case of
       where srcFrameBuffer and dstFrameBuffer are the same, if the conversion
       algorithm allows for that to occur without an intermediate frame store.

       The function should return FALSE if srcFrameBuffer and dstFrameBuffer
       are the same and that form pf conversion is not allowed
    */
    virtual BOOL Convert(
      const BYTE * srcFrameBuffer,  /// Frame store for source pixels
      BYTE * dstFrameBuffer,        /// Frame store for destination pixels
      PINDEX * bytesReturned = NULL /// Bytes written to dstFrameBuffer
    ) = 0;

    /**Convert from one colour format to another.
       This version will copy the data from one frame buffer to the same frame
       buffer. Not all conversions can do this so an intermediate store and
       copy may be required. If the noIntermediateFrame parameter is TRUE
       and the conversion cannot be done in place then the function returns
       FALSE. If the in place conversion can be done then that parameter is
       ignored.

       Note that the frame should be large enough to take the destination
       pixels.

       Default behaviour calls Convert() from the frameBuffer to itself, and
       if that returns FALSE then calls it again (provided noIntermediateFrame
       is FALSE) using an intermediate store, copying the intermediate store
       back to the original frame store.
    */
    virtual BOOL ConvertInPlace(
      BYTE * frameBuffer,               /// Frame buffer to translate data
      PINDEX * bytesReturned = NULL,    /// Bytes written to frameBuffer
      BOOL noIntermediateFrame = FALSE  /// Flag to use intermediate store
    );


    /**Create an instance of a colour conversion function.
       Returns NULL if there is no registered colour converter between the two
       named formats.
      */
    static PColourConverter * Create(
      const PString & srcColourFormat,  /// Name of source colour format
      const PString & dstColourFormat,  /// Name of destination colour format
      unsigned width,   /// Width of frame (used for both src and dst)
      unsigned height   /// Height of frame (used for both src and dst)
    );


  protected:
    virtual BOOL SimpleConvert(
      const BYTE * srcFrameBuffer,  /// Frame store for source pixels
      BYTE * dstFrameBuffer,        /// Frame store for destination pixels
      PINDEX * bytesReturned        /// Bytes written to dstFrameBuffer
    );

    PString  srcColourFormat;
    PString  dstColourFormat;
    unsigned srcFrameWidth;
    unsigned srcFrameHeight;
    unsigned srcFrameBytes;
    unsigned dstFrameBytes;

    // Needed for resizing
    unsigned dstFrameWidth;
    unsigned dstFrameHeight;
    BOOL     scaleNotCrop;

    PBYTEArray intermediateFrameStore;

    static PColourConverterRegistrations converters;

  friend class PColourConverterRegistration;
};


/**Declare a colour converter class with Convert() function.
   This should only be used once and at the global scope level for each
   converter. It declares everything needs so only the body of the Convert()
   function need be added.
  */
#define PCOLOUR_CONVERTER(cls,src,dst) \
class cls : public PColourConverter { \
  public: \
  cls(const PString & srcFmt, const PString & dstFmt, unsigned w, unsigned h) \
    : PColourConverter(srcFmt, dstFmt, w, h) { } \
  virtual BOOL Convert(const BYTE *, BYTE *, PINDEX * = NULL); \
}; \
class cls##_Registration : public PColourConverterRegistration { \
  public: \
  cls##_Registration() \
    : PColourConverterRegistration(src,dst) { } \
  virtual PColourConverter * Create(unsigned w, unsigned h) const; \
} cls##_registration_instance; \
PColourConverter * cls##_Registration::Create(unsigned w, unsigned h) const \
  { PINDEX tab = Find('\t'); return new cls(Left(tab), Mid(tab+1), w, h); } \
BOOL cls::Convert(const BYTE *srcFrameBuffer, BYTE *dstFrameBuffer, PINDEX * bytesReturned)



// End of file ///////////////////////////////////////////////////////////////
