/*
 * $Id: pwlib.h,v 1.37 1995/04/02 09:27:25 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: pwlib.h,v $
 * Revision 1.37  1995/04/02 09:27:25  robertj
 * Added "balloon" help.
 *
 * Revision 1.36  1995/02/19  04:19:15  robertj
 * Added dynamically linked command processing.
 *
 * Revision 1.35  1995/01/22  07:29:39  robertj
 * Added font & colour standard dialogs.
 *
 * Revision 1.34  1995/01/07  04:39:38  robertj
 * Redesigned font enumeration code and changed font styles.
 *
 * Revision 1.33  1994/10/23  04:53:25  robertj
 * Added PPixel subclasses
 *
 * Revision 1.32  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.31  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.30  1994/07/25  03:31:41  robertj
 * Renamed common pwlib to pwmisc to avoid name conflict.
 *
 * Revision 1.29  1994/07/17  10:46:06  robertj
 * Reordered classes to fix class references.
 *
 * Revision 1.28  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.27  1994/04/20  12:17:44  robertj
 * Made pwlib.h common
 *
 */

#ifndef _PWLIB_H
#define _PWLIB_H

#ifdef __GNUC__
#pragma interface
#endif


#define P_GUI


///////////////////////////////////////////////////////////////////////////////
// Basic text mode library

#include <ptlib.h>


///////////////////////////////////////////////////////////////////////////////
// PResourceData

#include <resdata.h>


///////////////////////////////////////////////////////////////////////////////
// PResourceString

#include <rstring.h>


///////////////////////////////////////////////////////////////////////////////
// PDim

#include <dim.h>


///////////////////////////////////////////////////////////////////////////////
// PPoint

#include <point.h>


///////////////////////////////////////////////////////////////////////////////
// PRect

#include <rect.h>


///////////////////////////////////////////////////////////////////////////////
// PRegion

#include <region.h>


///////////////////////////////////////////////////////////////////////////////
// PKeyCode

#include <keycode.h>


//////////////////////////////////////////////////////////////////////////////
// PColour

#include <colour.h>


//////////////////////////////////////////////////////////////////////////////
// PRealColour

#include <rcolour.h>


///////////////////////////////////////////////////////////////////////////////
// PFont

#include <font.h>


///////////////////////////////////////////////////////////////////////////////
// PRealFont

#include <rfont.h>


///////////////////////////////////////////////////////////////////////////////
// PFontFamily

#include <fontfam.h>


///////////////////////////////////////////////////////////////////////////////
// PPalette

#include <palette.h>


///////////////////////////////////////////////////////////////////////////////
// PPattern

#include <pattern.h>


///////////////////////////////////////////////////////////////////////////////
// PImage

#include <image.h>


///////////////////////////////////////////////////////////////////////////////
// PPixels

#include <pixels.h>
#include <pixels1.h>
#include <pixels2.h>
#include <pixels4.h>
#include <pixels8.h>
#include <pixels24.h>
#include <pixels32.h>


///////////////////////////////////////////////////////////////////////////////
// PPict

#include <pict.h>


///////////////////////////////////////////////////////////////////////////////
// PCursor

#include <cursor.h>


///////////////////////////////////////////////////////////////////////////////
// PCaret

#include <caret.h>


///////////////////////////////////////////////////////////////////////////////
// PImgIcon

#include <imgicon.h>


///////////////////////////////////////////////////////////////////////////////
// PIcon

#include <icon.h>


///////////////////////////////////////////////////////////////////////////////
// PCanvasState

#include <canstate.h>


///////////////////////////////////////////////////////////////////////////////
// PCanvas

#include <canvas.h>


///////////////////////////////////////////////////////////////////////////////
// PInteractorCanvas

#include <icanvas.h>


///////////////////////////////////////////////////////////////////////////////
// PDrawCanvas

#include <dcanvas.h>


///////////////////////////////////////////////////////////////////////////////
// PRedrawCanvas

#include <rcanvas.h>


///////////////////////////////////////////////////////////////////////////////
// PMemoryCanvas

#include <mcanvas.h>


///////////////////////////////////////////////////////////////////////////////
// PPrintInfo

#include <prinfo.h>


///////////////////////////////////////////////////////////////////////////////
// PPrintCanvas

#include <pcanvas.h>


///////////////////////////////////////////////////////////////////////////////
// PMenuEntry

#include <menuent.h>


///////////////////////////////////////////////////////////////////////////////
// PMenuItem

#include <menuitem.h>


///////////////////////////////////////////////////////////////////////////////
// PMenuSeparator

#include <menusep.h>


///////////////////////////////////////////////////////////////////////////////
// PSubMenu

#include <submenu.h>


///////////////////////////////////////////////////////////////////////////////
// PRootMenu

#include <rootmenu.h>


///////////////////////////////////////////////////////////////////////////////
// PInteractor

#include <interact.h>


///////////////////////////////////////////////////////////////////////////////
// PControl

#include <control.h>


///////////////////////////////////////////////////////////////////////////////
// PNamedControl

#include <ncontrol.h>


///////////////////////////////////////////////////////////////////////////////
// PStaticText

#include <stattext.h>


///////////////////////////////////////////////////////////////////////////////
// PStaticIcon

#include <staticon.h>


///////////////////////////////////////////////////////////////////////////////
// PStaticBox

#include <statbox.h>


///////////////////////////////////////////////////////////////////////////////
// PPushButton

#include <pbutton.h>


///////////////////////////////////////////////////////////////////////////////
// PTextButton

#include <tbutton.h>


///////////////////////////////////////////////////////////////////////////////
// PImageButton

#include <ibutton.h>


///////////////////////////////////////////////////////////////////////////////
// PCheck3WayBox

#include <check3.h>


///////////////////////////////////////////////////////////////////////////////
// PCheckBox

#include <checkbox.h>


///////////////////////////////////////////////////////////////////////////////
// PRadioButton

#include <rbutton.h>


///////////////////////////////////////////////////////////////////////////////
// PChoiceBox

#include <choicbox.h>


///////////////////////////////////////////////////////////////////////////////
// PListBox

#include <listbox.h>


///////////////////////////////////////////////////////////////////////////////
// PStringListBox

#include <slistbox.h>


///////////////////////////////////////////////////////////////////////////////
// PComboBox

#include <combobox.h>


///////////////////////////////////////////////////////////////////////////////
// PScrollBar

#include <scrollb.h>


///////////////////////////////////////////////////////////////////////////////
// PVerticalScrollBar

#include <vscrollb.h>


///////////////////////////////////////////////////////////////////////////////
// PHorizontalScrollBar

#include <hscrollb.h>


///////////////////////////////////////////////////////////////////////////////
// PEditBox

#include <editbox.h>


///////////////////////////////////////////////////////////////////////////////
// PPasswordEditBox

#include <pwedbox.h>


///////////////////////////////////////////////////////////////////////////////
// PMultiLineEditBox

#include <meditbox.h>


///////////////////////////////////////////////////////////////////////////////
// PNumberEditBox

#include <numedbox.h>


///////////////////////////////////////////////////////////////////////////////
// PIntegerEditBox

#include <intedit.h>


///////////////////////////////////////////////////////////////////////////////
// PFloatEditBox

#include <realedit.h>


///////////////////////////////////////////////////////////////////////////////
// PInteractorLayout

#include <ilayout.h>


///////////////////////////////////////////////////////////////////////////////
// PDialog

#include <dialog.h>


///////////////////////////////////////////////////////////////////////////////
// PModalDialog

#include <modaldlg.h>


///////////////////////////////////////////////////////////////////////////////
// PSimpleDialog

#include <simpdlg.h>


///////////////////////////////////////////////////////////////////////////////
// PFileDialog

#include <filedlg.h>


///////////////////////////////////////////////////////////////////////////////
// POpenFileDialog

#include <opendlg.h>


///////////////////////////////////////////////////////////////////////////////
// PSaveFileDialog

#include <savedlg.h>


///////////////////////////////////////////////////////////////////////////////
// POpenDirDialog

#include <dirdlg.h>


///////////////////////////////////////////////////////////////////////////////
// PPrintDialog

#include <printdlg.h>


///////////////////////////////////////////////////////////////////////////////
// PPrinterSetupDialog

#include <prsetdlg.h>


///////////////////////////////////////////////////////////////////////////////
// PPrintJobDialog

#include <prjobdlg.h>


///////////////////////////////////////////////////////////////////////////////
// PFontDialog

#include <fontdlg.h>


///////////////////////////////////////////////////////////////////////////////
// PColourDialog

#include <colordlg.h>


///////////////////////////////////////////////////////////////////////////////
// PScrollable

#include <scrollab.h>


///////////////////////////////////////////////////////////////////////////////
// PBalloon

#include <balloon.h>


///////////////////////////////////////////////////////////////////////////////
// PTitledWindow

#include <titlewnd.h>


///////////////////////////////////////////////////////////////////////////////
// PTopLevelWindow

#include <toplwnd.h>


///////////////////////////////////////////////////////////////////////////////
// PMDIFrameWindow

#include <mdiframe.h>


///////////////////////////////////////////////////////////////////////////////
// PMDIDocWindow

#include <mdidoc.h>


///////////////////////////////////////////////////////////////////////////////
// PCommandSink, PCommandSource

#include <commands.h>


///////////////////////////////////////////////////////////////////////////////
// PClipboard

#include <clipbrd.h>


///////////////////////////////////////////////////////////////////////////////
// PSound

#include <sound.h>


///////////////////////////////////////////////////////////////////////////////
// PConfiguration

#include <config.h>


///////////////////////////////////////////////////////////////////////////////
// PApplication

#include <applicat.h>


///////////////////////////////////////////////////////////////////////////////

#if defined(P_USE_INLINES)
#include <pwlib.inl>
#include <pwmisc.inl>
#include <graphics.inl>
#include <interact.inl>
#endif

#endif // _PWLIB_H

// PWLIB.H
