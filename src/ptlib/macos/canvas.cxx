/*
 * $Id: canvas.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: canvas.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <pwlib.h>

#ifndef P_USE_INLINES
#include <graphics.inl>
#endif


///////////////////////////////////////////////////////////////////////////////


PRect::PRect(PORDINATE x, PORDINATE y, PDIMENSION dx, PDIMENSION dy)
{
  r.left = x;
  r.top = y;
  r.right = x + dx;
  r.bottom = y + dy;
}


PRect::PRect(const PDim & dim)
{
  r.left = r.top = 0;
  r.right = dim.Width();
  r.bottom = dim.Height();
}


PRect::PRect(const PPoint & p1, const PPoint & p2)
{
  if (p1.X() > p2.X()) {
    r.left = p2.X();
    r.right = p1.X();
  }
  else {
    r.left = p1.X();
    r.right = p2.X();
  }

  if (p1.Y() > p2.Y()) {
    r.top = p2.Y();
    r.bottom = p1.Y();
  }
  else {
    r.top = p1.Y();
    r.bottom = p2.Y();
  }
}


PRect::PRect(const PPoint & topLeft, const PDim & dim)
{
  r.left = topLeft.X();
  r.top = topLeft.Y();
  r.right = topLeft.X() + dim.Width();
  r.bottom = topLeft.Y() + dim.Height();
}


void PRect::SetX(PORDINATE nx)
{
  PORDINATE diff = nx - (PORDINATE)r.left;
  r.left = nx;
  r.right += diff;
}


void PRect::SetY(PORDINATE ny)
{
  PORDINATE diff = ny - (PORDINATE)r.top;
  r.top = ny;
  r.bottom += diff;
}


void PRect::SetTop(PORDINATE t)
{
  r.top = t;
  if (t > r.bottom)
    r.bottom = t;
}


void PRect::SetLeft(PORDINATE l)
{
  r.left = l;
  if (l > r.right)
    r.right = l;
}


void PRect::SetBottom(PORDINATE b)
{
  r.bottom = b;
  if (b < r.top)
    r.top = b;
}

void PRect::SetRight(PORDINATE rt)
{
  r.right = rt;
  if (rt < r.left)
    r.left = rt;
}

PRect PRect::Intersection(const PRect & rect) const
{
  PRect newRect;
  SectRect(&r, rect, newRect);
  return newRect;
}


PRect PRect::Union(const PRect & rect) const
{
  PRect newRect;
  UnionRect(&r, rect, newRect);
  return newRect;
}


///////////////////////////////////////////////////////////////////////////////


PRegion::PRegion(const PRect & rect)
{
  hRegion = NewRgn();
  RectRgn(PAssertNULL(hRegion), rect);
}


PRegion::PRegion(const PRegion & rgn)
{
  CopyRgn(rgn.hRegion, hRegion);
}


PRegion & PRegion::operator=(const PRegion & rgn)
{
  CopyRgn(rgn.hRegion, hRegion);
  return *this;
}


void PRegion::Add(const PRect & rect)
{
  RgnHandle hRgnRect = NewRgn();
  RectRgn(PAssertNULL(hRgnRect), rect);
  UnionRgn(hRgnRect, hRegion, hRegion);
  DisposeRgn(hRgnRect);
}


void PRegion::Add(const PRegion & rgn)
{
  UnionRgn(rgn.hRegion, hRegion, hRegion);
}


BOOL PRegion::OverlapsRect(const PRect & rect) const
{
  RgnHandle hRgnRect = NewRgn();
  RectRgn(PAssertNULL(hRgnRect), rect);
  RgnHandle hRgn = NewRgn();
  SectRgn(hRgnRect, hRegion, PAssertNULL(hRgn));
  BOOL empty = EmptyRgn(hRgn);
  DisposeRgn(hRgn);
  DisposeRgn(hRgnRect);
  return !empty;
}


BOOL PRegion::IsEmpty() const
{
  return EmptyRgn(hRegion);
}


PRect PRegion::GetBounds() const
{
  return (*hRegion)->rgnBBox;
}


PRegion PRegion::Intersection(const PRegion & rgn) const
{
  RgnHandle hRgnNew = NewRgn();
  SectRgn(rgn.hRegion, hRegion, PAssertNULL(hRgnNew));
  return PRegion(hRgnNew);
}


PRegion PRegion::Union(const PRegion & rgn) const
{
  RgnHandle hRgnNew = NewRgn();
  UnionRgn(rgn.hRegion, hRegion, PAssertNULL(hRgnNew));
  return PRegion(hRgnNew);
}


///////////////////////////////////////////////////////////////////////////////


PColour::PColour(const PString & description)
{
  memset(component, 0, sizeof(component));
  PStringArray compstr = description.Tokenise(" ,");
  switch (compstr.GetSize()) {
    case 4 :
      component[AlphaComponent] = (BYTE)compstr[3].AsInteger();
      // Then execute next case

    case 3 :
      component[BlueComponent] = (BYTE)compstr[2].AsInteger();
      // Then execute next case

    case 2 :
      component[GreenComponent] = (BYTE)compstr[1].AsInteger();
      // Then execute next case

    case 1 :
      component[RedComponent] = (BYTE)compstr[0].AsInteger();
  }
}


PString PColour::GetDescription() const
{
  return psprintf("%u,%u,%u,%u", GetRed(), GetGreen(), GetBlue(), GetAlpha());
}


//////////////////////////////////////////////////////////////////////////////

PRealColour::PRealColour(PCanvas & canvas, const PColour & colour)
  : PColour(colour)
{
}


//////////////////////////////////////////////////////////////////////////////

PPalette::PPalette()
{
  hPalette = NewPalette(0, NULL, 0, 0);
}


void PPalette::DestroyContents()
{
  DisposePalette(hPalette);
}


PINDEX PPalette::AddColour(const PColour & colour)
{
  return 0;
}


BOOL PPalette::RemoveColour(const PColour & colour)
{
  return FALSE;
}


BOOL PPalette::RemoveColour(PINDEX index)
{
  return FALSE;
}


BOOL PPalette::HasColour(const PColour & colour) const
{
  return FALSE;
}


PINDEX PPalette::GetIndex(const PColour & colour) const
{
  return 0;
}


PColour PPalette::GetColour(PINDEX index) const
{
  return PColour::Black;
}


BOOL PPalette::SetColour(PINDEX index, const PColour & colour)
{
  return FALSE;
}


PINDEX PPalette::GetSize() const
{
  return 0;
}


BOOL PPalette::SetSize(PINDEX size)
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////


PRealFont::PRealFont()
  : PFont(PApplication::Current()->GetSystemFont())
{
}


PRealFont::PRealFont(const PFont & font)
  : PFont(font)
{
}


PRealFont::PRealFont(PCanvas & canvas, const PFont & font)
  : PFont(font)
{
}


PDIMENSION PRealFont::GetHeight(BOOL inPixels) const
{
  return 0; //inPixels ? PointsToPixelsY(height) : height;
}


PDIMENSION PRealFont::GetAvgWidth(BOOL inPixels) const
{
  return 0; //inPixels ? PointsToPixelsX(avgWidth) : avgWidth;
}


PDIMENSION PRealFont::GetMaxWidth(BOOL inPixels) const
{
  return 0; //inPixels ? PointsToPixelsX(maxWidth) : maxWidth;
}


PDIMENSION PRealFont::GetAscent(BOOL inPixels) const
{
  return 0; //inPixels ? PointsToPixelsY(ascent) : ascent;
}


PDIMENSION PRealFont::GetDescent(BOOL inPixels) const
{
  return 0; //inPixels ? PointsToPixelsY(descent) : descent;
}


PDIMENSION PRealFont::GetLeading(BOOL inPixels) const
{
  return 0; //inPixels ? PointsToPixelsY(leading) : leading;
}


///////////////////////////////////////////////////////////////////////////////

PPattern::PPattern()
{
  hPattern = NULL;
}


PPattern::PPattern(PRESOURCE_ID resID)
{
  hPattern = GetPixPat(resID);
}


PPattern::PPattern(Bits bits)
{
  hPattern = NULL;
}


void PPattern::DestroyContents()
{
  if (hPattern != NULL)
    DisposPixPat(hPattern);
}


void PPattern::CopyContents(const PPattern & pat)
{
  hPattern = pat.hPattern;
}


///////////////////////////////////////////////////////////////////////////////

PPixelBase::PPixelBase(PDIMENSION dx, PDIMENSION dy, BYTE depth)
  : PImageBase(dx, dy)
{
}


PPixelBase::~PPixelBase()
{
  free(pixels);
}


PPixelDataPtr PPixelBase::GetRasterDataPtr(PORDINATE y) const
{
  PAssert(y >= 0 && (PDIMENSION)y < Height(), "Pixel out of bounds");
  PAssert(pixels != NULL, "No pixel data");
  return pixels + (long)(Height()-y-1)*pixelLineBytes;
}


PPixelImage::PPixelImage(PRESOURCE_ID resID)
{
}


BOOL PPixelBase::Write(PFile & bmpFile)
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////


PPictBase::PPictBase()
  : PImageBase(0, 0)
{
  hPict = NULL;
}


PPictBase::PPictBase(PFile & dwg)
  : PImageBase(0, 0)
{
}


PPictBase::PPictBase(PRESOURCE_ID)
  : PImageBase(0, 0)
{
  hPict = NULL;
}


PPictBase::PPictBase(PicHandle hP)
  : PImageBase(0, 0)
{
  hPict = PAssertNULL(hP);
}


PPictBase::~PPictBase()
{
  KillPicture(hPict);
}


BOOL PPictBase::Write(PFile & dwg)
{
  return FALSE;
}



///////////////////////////////////////////////////////////////////////////////

PPrintInfo::PPrintInfo()
{
}


PPrintInfo::PPrintInfo(const PString & printerType,const PString & devicePort)
{
}


///////////////////////////////////////////////////////////////////////////////

PCanvas::PCanvas()
{
}


PCanvas::~PCanvas()
{
}


PCanvasState & PCanvas::operator=(const PCanvasState & state)
{
  PCanvasState::operator=(state);
  return *this;
}


PFontFamilyList PCanvas::GetAvailableFonts() const
{
  PFontFamilyList list;

  return list;
}


BOOL PCanvas::SetPenStyle(PenStyles style)
{
  if (PCanvasState::SetPenStyle(style)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetPenWidth(int width)
{
  if (PCanvasState::SetPenWidth(width)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetPenFgColour(const PColour & colour)
{
  if (PCanvasState::SetPenFgColour(colour)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetFillPattern(const PPattern & pattern)
{
  if (PCanvasState::SetFillPattern(pattern)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetPatternOrigin(const PPoint & pt)
{
  if (PCanvasState::SetPatternOrigin(pt)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetFillFgColour(const PColour & colour)
{
  if (PCanvasState::SetFillFgColour(colour)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetFont(const PFont & newFont)
{
  if (!PCanvasState::SetFont(newFont))
    return FALSE;

  return TRUE;
}


BOOL PCanvas::SetPalette(const PPalette & newPal)
{
  if (PCanvasState::SetPalette(newPal)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetPolyFillMode(PolyFillMode newMode)
{
  if (PCanvasState::SetPolyFillMode(newMode)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetViewportRect(const PRect & rect)
{
  if (PCanvasState::SetViewportRect(rect)) {
    return TRUE;
  }
  return FALSE;
}


BOOL PCanvas::SetMappingRect(const PRect & rect)
{
  if (PCanvasState::SetMappingRect(rect)) {
    return TRUE;
  }
  return FALSE;
}


void PCanvas::MoveCurrentPosition(PORDINATE x, PORDINATE y)
{
}


PPoint PCanvas::GetCurrentPosition() const
{
  PPoint p;
  return p;
}


void PCanvas::DrawLine(PORDINATE x, PORDINATE y)
{
}


void PCanvas::DrawLine(PORDINATE x1, PORDINATE y1, PORDINATE x2, PORDINATE y2)
{
}


void PCanvas::DrawLineRelative(PORDINATE x, PORDINATE y)
{
}


void PCanvas::internalDrawRect(PORDINATE x, PORDINATE y, PDIMENSION dx, PDIMENSION dy)
{
}


void PCanvas::internalFillRect(PORDINATE x, PORDINATE y, PDIMENSION dx, PDIMENSION dy)
{
}


void PCanvas::DrawFocusRect(const PRect & rect)
{
}


void PCanvas::DrawRoundRect(const PRect & rect,
                               PDIMENSION cornerWidth, PDIMENSION cornerHeight)
{
}


void PCanvas::DrawEllipse(const PRect & rect)
{
}


void PCanvas::DrawArc(const PRect & rect,
                                  const PPoint & startPt, const PPoint & endPt)
{
}


void PCanvas::DrawArc(const PRect & rect, int startAngle, int endAngle)
{
}


void PCanvas::DrawPie(const PRect & rect,
                                  const PPoint & startPt, const PPoint & endPt)
{
}


void PCanvas::DrawPie(const PRect & rect, int startAngle, int endAngle)
{
}


void PCanvas::DrawChord(const PRect & rect,
                                  const PPoint & startPt, const PPoint & endPt)
{
}


void PCanvas::DrawChord(const PRect & rect, int startAngle, int endAngle)
{
}


void PCanvas::DrawPolyLine(const PPointArray & ptArray)
{
}


void PCanvas::DrawPolyLine(const PPoint * ptArray, PINDEX numPts)
{
}


void PCanvas::DrawPolygon(const PPointArray & ptArray)
{
}


void PCanvas::DrawPolygon(const PPoint * ptArray, PINDEX numPts)
{
}


void PCanvas::DrawImgIcon(PORDINATE x, PORDINATE y, const PImgIcon & icn)
{
}


void PCanvas::DrawIcon(PORDINATE x, PORDINATE y, const PIcon & icn)
{
}


void PCanvas::DrawPixels(PORDINATE x, PORDINATE y, const PPixelImage & pix)
{
}


void PCanvas::DrawPixels(const PRect & rect, const PPixelImage & pix)
{
}


void PCanvas::DrawPict(PORDINATE x, PORDINATE y, const PPictImage & pic)
{
}


void PCanvas::DrawPict(const PRect & rect, const PPictImage & pic)
{
}


void PCanvas::DrawString(PORDINATE x, PORDINATE y,
                                              const PString & str, int options)
{
  PDim dim;
  switch (options&VerticalAlignmentMask) {
    case BottomAlign :
      if (dim.Height() == 0)
        dim = MeasureString(str);
      y -= dim.Height();
      break;

    case CentreVertical :
      if (dim.Height() == 0)
        dim = MeasureString(str);
      y -= dim.Height()/2;
      break;

    case BaseLine :
      y -= FromPointsY(realFont.GetAscent());
  }

  switch (options&HorizontalAlignmentMask) {
    case RightAlign :
      if (dim.Width() == 0)
        dim = MeasureString(str);
      x -= dim.Width();
      break;

    case Centred :
      if (dim.Width() == 0)
        dim = MeasureString(str);
      x -= dim.Width()/2;
  }

#if 0
  if (str.FindOneOf("\r\n") == P_MAX_INDEX)
    TabbedTextOut(GetHDC(), x, y, str, str.GetLength(), 0, NULL, 0);
  else {
    PRect r(x, y, 10000, 10000);
    DrawText(GetHDC(), str, -1, r,
                 DT_LEFT|DT_TOP|DT_NOPREFIX|DT_EXPANDTABS|DT_EXTERNALLEADING);
  }
#endif
}


PDim PCanvas::MeasureString(const PString & str)
{
  PDim dim;
  return dim;
}


void PCanvas::DrawTextLine(PORDINATE x, PORDINATE y,
                           const char * textChars,
                           const PDIMENSION * charWidths,
                           PINDEX len)
{
}


void PCanvas::SetClipRect(const PRect & rect)
{
}


PRect PCanvas::GetClipRect() const
{
  PRect r;
  return r;
}


void PCanvas::SetClipRegion(const PRegion & rgn)
{
}


PRegion PCanvas::GetClipRegion() const
{
  return PRegion();
}


//////////////////////////////////////////////////////////////////////////////


PRect PInteractorCanvas::GetDrawingBounds() const
{
  return FromPixels(interactor->GetDrawingBounds(TRUE));
}


//////////////////////////////////////////////////////////////////////////////


PDrawCanvas::PDrawCanvas(PInteractor * theInteractor,
                                     BOOL inPixels, BOOL overDrawChildWindows)
  : PInteractorCanvas(theInteractor, inPixels)
{
}


PDrawCanvas::~PDrawCanvas()
{
}


//////////////////////////////////////////////////////////////////////////////


PRedrawCanvas::PRedrawCanvas(PInteractor * theInteractor, BOOL inPixels)
  : PInteractorCanvas(theInteractor, inPixels)
{
}


PRedrawCanvas::~PRedrawCanvas()
{
}


//////////////////////////////////////////////////////////////////////////////


PPrintCanvas::~PPrintCanvas()
{
}


void PPrintCanvas::Construct()
{
}


PRect PPrintCanvas::GetDrawingBounds() const
{
  return PRect(0, 0, 1, 1);
}


BOOL PPrintCanvas::NewPage()
{
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////


void PMemoryCanvas::Construct()
{
}


// End Of File ///////////////////////////////////////////////////////////////
