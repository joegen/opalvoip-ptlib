/*
 * $Id
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log
 */


#include <pwlib.h>

#ifndef P_USE_INLINES
#include <pwmisc.inl>
#endif


void PInteractor::Construct(PInteractor * par, BOOL hiddenChild)
{
  parent = par;

  if (parent != NULL) {
    if (!hiddenChild)
      parent->children.Append(this);
    owner = parent->owner;
    cursorMode = UseParentCursor;
  }
  else {
    owner = PApplication::Current();
    cursorMode = UseCurrentCursor;
  }

  foregroundColour = owner->GetWindowFgColour();
  backgroundColour = owner->GetWindowBkColour();
}


PInteractor::~PInteractor()
{
  if (parent!=NULL && parent->children.GetObjectsIndex(this) != P_MAX_INDEX) {
    parent->children.DisallowDeleteObjects();
    parent->children.Remove(this);
    parent->children.AllowDeleteObjects();
  }
}


PObject::Comparison PInteractor::Compare(const PObject & obj) const
{
  PAssert(obj.IsDescendant(PInteractor::Class()), PInvalidParameter);
	return EqualTo;
}


void PInteractor::SetFont(const PFont & newFont, BOOL toChildren)
{
  font = PRealFont(newFont);

  if (toChildren) {
    for (PINDEX i = 0; i < children.GetSize(); i++)
      children[i].SetFont(font, TRUE);
  }
}


void PInteractor::_SetPosition(PORDINATE x, PORDINATE y,
                                 PositionOrigin xOrigin, PositionOrigin yOrigin)
{
  position.SetX(x);
  position.SetY(y);
}


PPoint PInteractor::GetPosition(BOOL inPixels) const
{
  if (inPixels)
    return position;

  const PInteractor * p = parent != NULL ? parent : this;
  return p->FromPixels(position);
}


PPoint PInteractor::ToScreen(const PPoint & pt, BOOL inPixels) const
{
  PPoint p = inPixels ? pt : ToPixels(pt);
  LocalToGlobal(p);
  return p;
}


PPoint PInteractor::FromScreen(const PPoint & pt, BOOL inPixels) const
{
  PPoint p = pt;
  GlobalToLocal(p);
  return inPixels ? p : ToPixels(p);
}


void PInteractor::_SetDimensions(PDIMENSION width, PDIMENSION height, BOOL inPixels)
{
  if (!inPixels) {
    const PInteractor * p = parent != NULL ? parent : this;
    width = p->ToPixelsX(width);
    height = p->ToPixelsY(height);
  }

  dimensions.SetWidth(width);
  dimensions.SetHeight(height);
}


PDim PInteractor::GetDimensions(BOOL inPixels) const
{
  if (inPixels)
    return dimensions;

  const PInteractor * p = parent != NULL ? parent : this;
  return p->FromPixels(dimensions);
}


PRect PInteractor::GetStructureBounds(BOOL inPixels) const
{
  if (inPixels)
    return structureBounds;

  PPoint p1 = structureBounds.Origin();
  GlobalToLocal(p1);
  PPoint p2 = structureBounds.Corner();
  GlobalToLocal(p2);

  return PRect(FromPixels(p1), FromPixels(p2));
}


void PInteractor::_Invalidate(PORDINATE x, PORDINATE y,
                                 PDIMENSION dx, PDIMENSION dy, BOOL inPixels)
{
}


void PInteractor::_Validate(PORDINATE x, PORDINATE y,
                                 PDIMENSION dx, PDIMENSION dy, BOOL inPixels)
{
}


void PInteractor::ReleaseMouse()
{
}


void PInteractor::SetCursorToParent(BOOL useParent)
{
  if (cursorMode == UseParentCursor || cursorMode == UseCurrentCursor) {
    cursorMode = useParent ? UseParentCursor : UseCurrentCursor;
  }
}


void PInteractor::SetCursor(const PCursor & newCursor)
{
  if (cursorMode == UseParentCursor || cursorMode == UseCurrentCursor) {
    cursorMode = UseCurrentCursor;
    cursor = newCursor;
  }
}


void PInteractor::_SetCursorPos(PORDINATE x, PORDINATE y, BOOL inPixels)
{
  if (inPixels)
    ;
  else {
    Point p;
    p.h = ToPixelsX(x);
    p.v = ToPixelsY(y);
    LocalToGlobal(&p);
    ;
  }
}


PPoint PInteractor::GetCursorPos(BOOL inPixels) const
{
  PPoint p;
  ;
  if (inPixels)
    return p;
  return FromPixels(p);
}


PDim PInteractor::GetBorderSize() const
{
  return owner->GetBorderSize();
}


const PColour & PInteractor::GetBorderColour() const
{
  return owner->GetActiveBorderColour();
}


void PInteractor::SetBackgroundColour(const PColour & newColour)
{
  backgroundColour = newColour;
  Invalidate();
}


WindowPtr PInteractor::GetWindowPtr()
{
  return PAssertNULL(parent)->GetWindowPtr();
}


void PInteractor::HandleUpdateEvent(RgnHandle visRgn)
{
  if (RectInRgn(PRect(position, dimensions), visRgn)) {
    PRedrawCanvas canvas(this, TRUE);
    OnRedraw(canvas);
  }
  for (PINDEX i = 0; i < GetNumChildren(); i++)
    children[i].HandleUpdateEvent(visRgn);
}


void PInteractor::HandleMouseDown(short part, Point where, short modifiers)
{
  if (part != inContent)
    return;

  int keyModifier = PKeyCode::NoModifier;
  if ((modifiers&cmdKey) != 0)
    keyModifier |= PKeyCode::Command|PKeyCode::Accelerator1;
  if ((modifiers&optionKey) != 0)
    keyModifier |= PKeyCode::Option|PKeyCode::Accelerator2;
  if ((modifiers&shiftKey) != 0)
    keyModifier |= PKeyCode::Shift;
  if ((modifiers&alphaLock) != 0)
    keyModifier |= PKeyCode::CapsLock;

  PPoint pt(position.X()+where.h, position.Y()+where.v);
  OnMouseDown(PKeyCode(PKeyCode::LeftButton, keyModifier), pt, FALSE);
}


void PInteractor::OnMouseDown(PKeyCode, const PPoint &, BOOL)
{
}


void PInteractor::OnMouseUp(PKeyCode, const PPoint &)
{
}


void PInteractor::OnMouseMove(PKeyCode, const PPoint &)
{
}


BOOL PInteractor::OnKeyDown(PKeyCode, unsigned)
{
  return TRUE;
}


void PInteractor::OnKeyUp(PKeyCode key)
{
}


void PInteractor::OnKeyInput(const PString &)
{
}


void PInteractor::OnGainFocus()
{
}


void PInteractor::OnLostFocus()
{
}


void PInteractor::OnRedraw(PCanvas & canvas)
{
}



//////////////////////////////////////////////////////////////////////////////


PTitledWindow::PTitledWindow(PInteractor * par, unsigned CanDo)
  : PInteractor(par),
    icon(PSTD_ID_ICON_WINDOW)
{
  Construct(CanDo, noGrowDocProc, owner->GetTitledWindowSize());
}


PTitledWindow::PTitledWindow(unsigned CanDo, int)
  : icon(PSTD_ID_ICON_WINDOW)
{
  Construct(CanDo, noGrowDocProc, owner->GetTitledWindowSize());
}


void PTitledWindow::Construct(unsigned CanDo, short docProc, PDim size)
{
  initFlags = CanDo;
  focusInteractor = this;

  minSize = PDim(50, 20),
  maxSize.SetWidth(qd.screenBits.bounds.right);
  maxSize.SetHeight(qd.screenBits.bounds.bottom);
  zoomSize = maxSize;
  
  short proc = documentProc;
  if (CanDo&CanZoom)
    proc += 8;

  position.SetX(owner->GetTitledBorderSize().Width());
  position.SetY(owner->GetMenuHeight() +
  					        owner->GetTitleHeight() + owner->GetTitledBorderSize().Height());
  dimensions = size;
  PRect bounds(position, dimensions);
  Str255 title;
  title[0] = 0;

  wnd = NewWindow(NULL, bounds, title, FALSE, docProc, (WindowPtr)-1, TRUE, (long)this);
  PAssertNULL(wnd);

  structureBounds = (*((WindowPeek)wnd)->strucRgn)->rgnBBox;
}


void PTitledWindow::_SetPosition(PORDINATE x,PORDINATE y,
                                 PositionOrigin xOrigin,
                                 PositionOrigin yOrigin)
{
  PInteractor::_SetPosition(x, y, xOrigin, yOrigin);
  MoveWindow(PAssertNULL(wnd), position.X(), position.Y(), FALSE);
}


void PTitledWindow::_SetDimension(PDIMENSION width, PDIMENSION height,BOOL inPixels)
{
  PInteractor::_SetDimensions(width, height, inPixels);
  SizeWindow(PAssertNULL(wnd), dimensions.Width(), dimensions.Height(), TRUE);
}


void PTitledWindow::Show(BOOL show)
{
  if (show)
    ShowWindow(wnd);
  else
    HideWindow(wnd);
}


void PTitledWindow::Invalidate(const PRect & rect)
{
  SetPort(wnd);
  InvalRect(rect);
}


void PTitledWindow::Validate(const PRect & rect)
{
  SetPort(wnd);
  ValidRect(rect);
}


PString PTitledWindow::GetTitle() const
{
  Str255 title;
  GetWTitle(wnd, title);
  return PString(PString::Pascal, (const char *)title);
}


void PTitledWindow::SetMinSize(const PDim & dim, BOOL inPixels)
{
  if (inPixels)
    minSize = dim;
  else
    minSize = ToPixels(dim);
  SetDimensions(GetDimensions(TRUE), TRUE);
}


void PTitledWindow::SetMaxSize(const PDim & dim, BOOL inPixels)
{
  if (inPixels)
    maxSize = dim;
  else
    maxSize = ToPixels(dim);
  SetDimensions(GetDimensions(TRUE), TRUE);
}


void PTitledWindow::SetIcon(const PIcon & icn)
{
  icon = icn;
}


void PTitledWindow::OnReposition(const PPoint &)
{
}


void PTitledWindow::OnActivate(BOOL)
{
}


void PTitledWindow::OnResize(const PDim &, ResizeType)
{
}


WindowPtr PTitledWindow::GetWindowPtr()
{
  return wnd;
}


void PTitledWindow::HandleMouseDown(short part, Point where, short modifiers)
{
  PRect bounds;

  switch (part) {
    case inGrow :
  	  bounds.SetLeft  (minSize.Width());
  	  bounds.SetTop   (minSize.Height());
  	  bounds.SetRight (maxSize.Width());
  	  bounds.SetBottom(maxSize.Height());

  	  long growResult = GrowWindow(wnd, where, bounds);
  	  if (growResult != 0) {
  	    SetDimensions(PDim(LoWord(growResult), HiWord(growResult)));
  	    OnResize(GetDimensions(), Normalised);
  	  }
  	  break;

  	case inZoomIn:
  	case inZoomOut:
  	  if (TrackBox(wnd, where, part)) {
    		SetPort(wnd);
    		EraseRect(&wnd->portRect);
    		ZoomWindow(wnd, part, wnd == FrontWindow());
    		InvalRect(&wnd->portRect);
    		*(Rect *)bounds = (*((WindowPeek)wnd)->contRgn)->rgnBBox;
    		SetDimensions(bounds.Dimensions());
    		OnResize(GetDimensions(), part == inZoomOut ? Zoomed : Normalised);
  	  }
  	  break;

    case inContent :
  	  if (wnd != FrontWindow())
    		SelectWindow(wnd);
  	  else
  	    PInteractor::HandleMouseDown(part, where, modifiers);
  	  break;

  	case inDrag :
  	  DragWindow(wnd, where, &qd.screenBits.bounds);
  	  Rect * bounds = &(*((WindowPeek)wnd)->strucRgn)->rgnBBox;
  	  SetPosition(bounds->left, bounds->top, TopLeftScreen, TopLeftScreen);
  	  OnReposition(GetPosition());
  	  break;

  	case inGoAway :
  	  if (TrackGoAway(wnd, where))
    	  OnClose();
  	  break;

    default :
      PInteractor::HandleMouseDown(part, where, modifiers);
  }
}


//////////////////////////////////////////////////////////////////////////////


PTopLevelWindow::PTopLevelWindow(unsigned CanDo)
  : PTitledWindow(CanDo, 0)
{
  menu = NULL;
  deleteMenu = FALSE;

  if (owner->mainWindow == NULL)
    owner->mainWindow = this;

  SetTitle(owner->GetName());
}


PTopLevelWindow::~PTopLevelWindow()
{
  if (deleteMenu)
    delete menu;

  if (owner->mainWindow == this)
    owner->mainWindow = NULL;
}


//////////////////////////////////////////////////////////////////////////////


PMDIFrameWindow::PMDIFrameWindow(unsigned CanDo)
  : PTopLevelWindow(CanDo)
{
}


PMDIDocWindow * PMDIFrameWindow::GetActiveDocument()
{
  return NULL;
}


void PMDIFrameWindow::SetDocumentArea(const PRect & rect, BOOL inPixels)
{
  if (inPixels)
    documentArea = rect;
  else
    documentArea = ToPixels(rect);
  PRect r = documentArea;
  if (documentArea.IsEmpty())
    r = GetDimensions(TRUE);
}


///////////////////////////////////////////////////////////////////////////////
// PPopUp

PPopUp::PPopUp(PInteractor * parent)
  : PInteractor(parent)
{
}


// End Of File ///////////////////////////////////////////////////////////////
