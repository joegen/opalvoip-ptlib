/*
 * $Id: miscellaneous.cxx,v 1.2 2003/11/25 08:28:14 rjongbloed Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: miscellaneous.cxx,v $
 * Revision 1.2  2003/11/25 08:28:14  rjongbloed
 * Removed ability to have platform without threads, win16 finally deprecated
 *
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <pwlib.h>

#ifndef P_USE_INLINES
#include <pwlib.inl>
#endif


///////////////////////////////////////////////////////////////////////////////
// PResourceData

PResourceData::PResourceData(const PString & resType, PRESOURCE_ID resID)
{
}


void PResourceData::CopyContents(const PResourceData & res)
{
}


PObject::Comparison PResourceData::Compare(const PObject & obj) const
{
  return EqualTo;
}


BOOL PResourceData::SetSize(PINDEX)
{
  return TRUE;
}


void PResourceData::DestroyContents()
{
}


///////////////////////////////////////////////////////////////////////////////
// PResourceString

void PResourceString::Construct(PRESOURCE_ID resID)
{
}


///////////////////////////////////////////////////////////////////////////////
// PCursor

PCursor::PCursor(PRESOURCE_ID resID)
{
  switch ((unsigned)resID) {
    case PSTD_ID_CURSOR_ARROW :
      break;

    case PSTD_ID_CURSOR_IBEAM :
      break;

    case PSTD_ID_CURSOR_WAIT :
      break;

    case PSTD_ID_CURSOR_CROSS :
      break;

    case PSTD_ID_CURSOR_MOVELEFT :
    case PSTD_ID_CURSOR_MOVERIGHT :
    case PSTD_ID_CURSOR_LEFTRIGHT :
      break;

    case PSTD_ID_CURSOR_MOVETOP :
    case PSTD_ID_CURSOR_MOVEBOTTOM :
    case PSTD_ID_CURSOR_UPDOWN :
      break;

    case PSTD_ID_CURSOR_MOVETOPRIGHT :
    case PSTD_ID_CURSOR_MOVEBOTLEFT :
      break;

    case PSTD_ID_CURSOR_MOVETOPLEFT :
    case PSTD_ID_CURSOR_MOVEBOTRIGHT :
      break;

    default :
      ;
  }
}


PCursor::PCursor(const PPixelImage & andMask,
                          const PPixelImage & xorMask, const PPoint & hotSpot)
{
}


void PCursor::DestroyContents()
{
}


void PCursor::CopyContents(const PCursor & curs)
{
}


//////////////////////////////////////////////////////////////////////////////
// PCaret

void PCaret::DestroyContents()
{
}


void PCaret::CopyContents(const PCaret & c)
{
}


void PCaret::Activate(PInteractor * activator, BOOL display)
{
}


void PCaret::Deactivate(PInteractor * activator)
{
}


void PCaret::Show(PInteractor * activator)
{
}


void PCaret::Hide(PInteractor * activator)
{
}


void PCaret::SetPosition(PInteractor * activator)
{
}


void PCaret::SetDimensions(PDIMENSION dx, PDIMENSION dy)
{
}


///////////////////////////////////////////////////////////////////////////////
// PImgIcon

PImgIcon::PImgIcon(PRESOURCE_ID resID)
{
}


PImgIcon::PImgIcon(PCanvas & canvas, const PPixelImage & pix)
{
}


void PImgIcon::DestroyContents()
{
}


PDim PImgIcon::GetDimensions() const
{
  return PDim();
}


///////////////////////////////////////////////////////////////////////////////
// PIcon

PIcon::PIcon(PRESOURCE_ID resID)
  : PImgIcon(NULL)
{
  switch ((unsigned)resID) {
    case PSTD_ID_ICON_INFORMATION :
      break;

    case PSTD_ID_ICON_QUESTION :
      break;

    case PSTD_ID_ICON_EXCLAMATION :
      break;

    case PSTD_ID_ICON_STOPSIGN :
      break;

    case PSTD_ID_ICON_WINDOW :
      break;

    default :
      ;
  }
}


void PIcon::DestroyContents()
{
}


void PIcon::CopyContents(const PIcon & icon)
{
}


///////////////////////////////////////////////////////////////////////////////
// PClipboard

PClipboard::PClipboard(const PInteractor * wnd)
  : owner(PAssertNULL(wnd))
{
}


PClipboard::~PClipboard()
{
}


BOOL PClipboard::HasFormat(Format fmt)
{
  return FALSE;
}


BOOL PClipboard::HasFormat(const PString & fmt)
{
  return FALSE;
}


DWORD PClipboard::GetSize(const PString & fmt)
{
  return 0;
}


BOOL PClipboard::GetData(const PString & fmt, void * data, DWORD max)
{
  return FALSE;
}

    
BOOL PClipboard::SetData(const PString & fmt, const void * data, DWORD len)
{
  return FALSE;
}


PString PClipboard::GetText()
{
  PString str;
  return str;
}


BOOL PClipboard::SetText(const PString & str)
{
  return FALSE;
}


PPixelImage PClipboard::GetPixels()
{
  return PPixelImage();
}


BOOL PClipboard::SetPixels(const PPixelImage & pix)
{
  return FALSE;
}


PPictImage PClipboard::GetPict()
{
  PPictImage dwg;

  return dwg;
}


BOOL PClipboard::SetPict(const PPictImage & pic)
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PApplication

PApplication::PApplication(const char * manuf, const char * name,
                           WORD major, WORD minor, CodeStatus stat, WORD build)
  : PProcess(manuf, name, major, minor, stat, build),
    systemFont("Chicago", 12),
    balloonFont("Helvetica", 8)
{
}


int PApplication::_main(int argc, char ** argv, char **)
{
  InitGraf((Ptr) &qd.thePort);
  InitFonts();
  InitWindows();
  InitMenus();
  TEInit();
  InitDialogs(nil);
  InitCursor();

  for (int count = 1; count <= 3; count++) {
  	EventRecord event;
  	EventAvail(everyEvent, &event);
  }

  SysEnvirons(1, &environment);
  PAssert(environment.machineType >= 0, "Illegal Machine type");

  PreInitialise(argc, argv);
  Construct();

//  PErrorStream = new PDebugStream;  // Use OutputDebugString() stream

  blowUpTimeout = 1000;
  doubleClick = 250;
  mainWindow = NULL;
  applicationRunning = FALSE;

  defaultHelpFile = GetFile();
  defaultHelpFile.SetType(".hlp");

  screenSize.SetWidth(qd.screenBits.bounds.right);
  screenSize.SetHeight(qd.screenBits.bounds.bottom);
  screenResImperial = PDim(72, 72);
  screenResMetric = screenResImperial*394L;
  screenResMetric = screenResImperial/1000L;
  cursorSize = PDim(16, 16);
  iconSize = PDim(32, 32);
  titledBorder = PDim(2, 2);
  dlgBorder = PDim(4, 4);
  border = PDim(1, 1);
  heightTitle = 20;
  heightMenu = 22;
  heightHScroll = 16;
  widthVScroll = 16;
  defMainWindow.SetWidth(qd.screenBits.bounds.right*2/3);
  defMainWindow.SetHeight(qd.screenBits.bounds.bottom*2/3);
  defTitledWindow = defMainWindow;
  screenColours = 256;
  screenDepth = 8;
  dblClkRect = PRect(-2, -2, 4, 4);
  windowFg = PColour::Black;
  windowBk = PColour::White;
  highlightFg = PColour::White;
  highlightBk = PColour::Black;
  menuFg = PColour::Black;
  menuBk = PColour::White;
  balloonFg = menuFg;
  balloonBk = menuBk;
  grayText = PColour(0xA000, 0xA000, 0xA000);
  buttonFg = PColour::Black;
  buttonBk = PColour(0x8000, 0x8000, 0x8000);
  buttonLighting = PColour::White;
  buttonShadow = PColour(0x2000, 0x2000, 0x2000);
  scrollBar = PColour::Black;
  activeTitleFg = PColour::Black;
  activeTitleBk = PColour::White;
  inactiveTitleFg = PColour::Black;
  inactiveTitleBk = PColour::White;
  activeBorder = PColour::Black;
  inactiveBorder = PColour::White;

  lastMouseUpTime = TickCount();
  focusInteractor = NULL;
  return GetTerminationValue();
}


void PApplication::Main()
{
  RgnHandle multiFinderCursorRgn = NewRgn(); // we'll pass WNE an empty region the 1st time thru

  applicationRunning = TRUE;
  while (applicationRunning) {
    EventRecord event;
	OSEventAvail(0, &event);	// we aren't interested in any events yet
	HandleCursor(event.where);	// just the mouse position
	if (WaitNextEvent(everyEvent, &event, GetCaretTime(), multiFinderCursorRgn))
	HandleEvent(event);
    PassMainLoop();
  }
}
  

void PApplication::HandleEvent(EventRecord & event)
{
  switch (event.what) {
    case mouseDown : {
	  WindowPtr theWindowPtr;
	  short part = FindWindow(event.where, &theWindowPtr);
	
	  switch (part) {
		case inMenuBar :
		  mainWindow->HandleMenu(MenuSelect(event.where));
		  break;
	
		case inSysWindow :
		  SystemClick(&event, theWindowPtr);
		  break;
	
		default :
		  if (theWindowPtr != NULL) {
			PTitledWindow * window = (PTitledWindow *)GetWRefCon(theWindowPtr);
			if (window != NULL)
			  window->HandleMouseDown(part, event.where, event.modifiers);
		  }
	  }
	  break;
	}

	case mouseUp :
	  lastMouseUpTime = TickCount();
	  break;

	case keyDown :
	case autoKey :
	  if (event.modifiers&cmdKey)	/* Command key down */
		mainWindow->HandleMenu(MenuKey(event.message&charCodeMask));
	  break;
			
	case updateEvt : {
	  WindowPtr theWindowPtr = (WindowPtr)event.message;
	  BeginUpdate(theWindowPtr);
	
	  PTitledWindow * window = (PTitledWindow *)GetWRefCon(theWindowPtr);
	  if (window != NULL)
		window->HandleUpdateEvent(theWindowPtr->visRgn);
	
	  EndUpdate(theWindowPtr);
	  break;
	}

	case diskEvt :
	  // It is not a bad idea to at least call DIBadMount in response
	  // to a diskEvt, so that the user can format a floppy.
	  if (HiWord(event.message) != noErr)
		DIBadMount((Point)PPoint(0x0070, 0x0050), event.message);
	  break;
  }
}


void PApplication::HandleCursor(Point where)
{
}


void PApplication::Terminate()
{
	applicationRunning = FALSE;
}


void PApplication::DoContextHelp(const PString & context,
                                                        const PFilePath & file)
{
}


void PApplication::DoContextHelp(PINDEX context, const PFilePath & file)
{
  switch ((unsigned)context) {
    case PSTD_ID_HELP_CONTENTS :
      break;

    case PSTD_ID_HELP_SEARCH :
      break;

    case PSTD_ID_HELP_ON_HELP :
      break;

    default :
      ;
  }
}



// End Of File ///////////////////////////////////////////////////////////////
