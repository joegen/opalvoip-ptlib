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

#ifndef _PWLIB_H
#define _PWLIB_H


#include "osutil.h"

#define P_MSWINDOWS


#define STRICT
#include <windows.h>
#include <commdlg.h>

typedef int PORDINATE;
typedef int PDIMENSION;

#define PPOINT_BASE tagPOINT
#define PRECT_BASE tagRECT
 
#define P_NULL_WINDOW (NULL)


///////////////////////////////////////////////////////////////////////////////
// PDim

#include "../../common/dim.h"
  public:
    PDim(DWORD dw);
      // This is used to convert MS-Windows DWORD forms of size structures to
      // a PDim object.

    DWORD ToDWORD() const;
      // This will pack a PDim object into the MS-Windows DWORD format.
};


///////////////////////////////////////////////////////////////////////////////
// PPoint

#include "../../common/point.h"
  public:
    PPoint(DWORD dw);
      // This is used to convert MS-Windows DWORD forms of point structures to
      // a PPoint object.

    DWORD ToDWORD() const;
      // This will pack a PPoint object into the MS-Windows DWORD format.
};


///////////////////////////////////////////////////////////////////////////////
// PRect

#include "../../common/rect.h"
  public:
    PRect(const RECT & r);
      // Construct a rectangle from MS-Windows RECT structure
};


///////////////////////////////////////////////////////////////////////////////
// PRegion

#include "../../common/region.h"
  protected:
    PRegion(HRGN hRgn);

    HRGN hRegion;
};


///////////////////////////////////////////////////////////////////////////////
// PKeyCode

#include "../../common/keycode.h"
  public:
    PKeyCode(BOOL mouseMessage, WPARAM wParam, int other);
      // Convert a MS-Windows event data for mouse down and key down events
      // to key codes.

    WORD VKCode();
      // Return the MS-Window Virtual Key number for the key code.

    WORD MouseModifiers();
      // Return the modifiers for a mouse click that are set in this key code.
};


//////////////////////////////////////////////////////////////////////////////
// PColour

  #include "../../common/colour.h"
  public:
    COLORREF ToCOLORREF() const;
      // Convert the colour to MS-Windows GDI colour reference

    void FromCOLORREF(COLORREF col);
      // Set the RGB compnents from the MS-Windows GDI colour reference
};


//////////////////////////////////////////////////////////////////////////////
// PRealColour

#include "../../common/rcolour.h"
  public:
    PRealColour(COLORREF col);
      // Create a colour of the specified RGB values

};


///////////////////////////////////////////////////////////////////////////////
// PFont

#include "../../common/font.h"
};


///////////////////////////////////////////////////////////////////////////////
// PRealFont

#include "../../common/rfont.h"
  public:
    PRealFont(PApplication * app, HFONT hFont);
      // Create a real font object given the MS-Windows GDI font handle.
      
    HFONT GetHFONT() const;
      // Return a MS-Windows GDI font handle.
      
      
  protected:
    // New methods for class
    void Construct(HDC dc);
      // Construct the real font for the specified device - part 1


    // member variables
    int height, avgWidth, maxWidth, ascent, descent, leading;
};


///////////////////////////////////////////////////////////////////////////////
// PPalette

#include "../../common/palette.h"
  public:
    // New functions for class
    HPALETTE GetHPALETTE() const;
      // Select the palette for the specified MS-Windows device context.
    

  protected:
    // Member variables
    HPALETTE hPalette;
      // Palette GDI object
};


///////////////////////////////////////////////////////////////////////////////
// PShape

#include "../../common/shape.h"
};


///////////////////////////////////////////////////////////////////////////////
// PLine

#include "../../common/line.h"
};


///////////////////////////////////////////////////////////////////////////////
// POrthoShape

#include "../../common/orthshap.h"
};


///////////////////////////////////////////////////////////////////////////////
// PRectangle

#include "../../common/rectngle.h"
};


///////////////////////////////////////////////////////////////////////////////
// PRoundedRectangle
#include "../../common/rndrect.h"
};


///////////////////////////////////////////////////////////////////////////////
// PEllipse

#include "../../common/ellipse.h"
};


///////////////////////////////////////////////////////////////////////////////
// PArc

#include "../../common/arc.h"
};


///////////////////////////////////////////////////////////////////////////////
// PPie

#include "../../common/pie.h"
};


///////////////////////////////////////////////////////////////////////////////
// PChord

#include "../../common/chord.h"
};


///////////////////////////////////////////////////////////////////////////////
// PTextLine

#include "../../common/textline.h"
};


///////////////////////////////////////////////////////////////////////////////
// PTextBlock

#include "../../common/textblck.h"
};


///////////////////////////////////////////////////////////////////////////////
// PPolyShape

#include "../../common/polyshap.h"
};


///////////////////////////////////////////////////////////////////////////////
// PPolyLine

#include "../../common/polyline.h"
};


///////////////////////////////////////////////////////////////////////////////
// PPolygon

#include "../../common/polygon.h"
};


///////////////////////////////////////////////////////////////////////////////
// PCurve

#include "../../common/curve.h"
};


///////////////////////////////////////////////////////////////////////////////
// PBezier

#include "../../common/bezier.h"
};


///////////////////////////////////////////////////////////////////////////////
// PBSpline

#include "../../common/bspline.h"
};


///////////////////////////////////////////////////////////////////////////////
// PCompositeShape

#include "../../common/compshap.h"
};


///////////////////////////////////////////////////////////////////////////////
// PImage

#include "../../common/image.h"
  public:
    HBITMAP GetHBITMAP() const;

  protected:
    DECLARE_CLASS(Bitmap, PContainer)
      public:
        Bitmap(HBITMAP hBm);
        Bitmap(const Bitmap & bm);
        Bitmap & operator=(const Bitmap & bm);
        virtual ~Bitmap();

      protected:
        virtual Comparison Compare(const PObject & obj) const;
        virtual void DestroyContents();
        virtual BOOL SetSize(PINDEX size);

        HBITMAP hBitmap;
        BITMAP bitmap;

        friend class PImage;
    };
    Bitmap bitmap;
};


///////////////////////////////////////////////////////////////////////////////
// PPattern

#include "../../common/pattern.h"
  public:
    HBITMAP GetHBITMAP() const;

  protected:
    void Construct(HBITMAP hBm);

    HBITMAP hBitmap;
    BITMAP bitmap;
};


///////////////////////////////////////////////////////////////////////////////
// PCursor

#include "../../common/cursor.h"
  public:
    HCURSOR GetHCURSOR() const;
      // Get the MS-Windows cursor

  protected:
    HCURSOR hCursor;
};


///////////////////////////////////////////////////////////////////////////////
// PCaret

#include "../../common/caret.h"
  protected:
    BOOL hasCaret;
};


///////////////////////////////////////////////////////////////////////////////
// PIcon

#include "../../common/icon.h"
  public:
    PIcon(HICON hIcn);
      // Make a PIcon object from the MS-WINDOWS icon handle

    HICON GetHICON() const;
      // Get the MS-WINDOWS icon handle for the loaded icon
      
  protected:
    HICON hIcon;
};


///////////////////////////////////////////////////////////////////////////////
// PCanvas

#include "../../common/canvas.h"
  public:
    virtual HDC GetHDC() const;
      // Return the MS-Windows GDI Device Context.

    virtual HDC GetHDC();
      // Return the MS-Windows GDI Device Context, create if does not exist.

    void SetHDC(HDC newDC);
      // Function to associate the MS-Windows GDI Device Context to the canvas
      // object.

    BOOL NullHDC() const;
      // Return TRUE if the hDC is NULL, ie not used by any OnRedraw().


  protected:
    void MakePen();
      // Make a MS-Windows GDI pen for the canvas

    void MakeBrush(const PPattern & pattern);
      // Make a MS-Windows GDI brush for the canvas

    void SetUpDrawModes(DrawingModes mode, const PColour & colour);
      // Set the MS-Windows GDI ROP mode, background colour and background mode.

    LPPOINT MakePOINTArray(const PPointArray & ptArray);
    LPPOINT MakePOINTArray(const PPoint * ptArray, PINDEX numPts);
      // Make an array of POINTs from parameter. Array must be deleted.

    void DrawPolyLine(LPPOINT ptArray, PINDEX numPts);
      // Draw a series of lines represented by the array of points.

    virtual void DrawPolygon(LPPOINT ptArray, PINDEX numPts);
      // Draw a closed polygon represented by the array of points. There is an


    // Member variables
    HDC hDC;
      // The MS-Windows GDI Device Context creates internal to the canvas.

    HPEN hPen;
      // The MS-Windows GDI pen object
      
    HBRUSH hBrush;
      // The MS-Windows GDI brush object
      
    HFONT hFont;
      // The MS-Windows GDI font object
};


///////////////////////////////////////////////////////////////////////////////
// PInteractorCanvas

#include "../../common/icanvas.h"
};


///////////////////////////////////////////////////////////////////////////////
// PDrawCanvas

#include "../../common/dcanvas.h"
  public:
    PDrawCanvas(PInteractor * theInteractor, HDC newDC);
      // Make a PDrawCanvas object from the MS-WINDOWS device context handle

  private:
    // Member variables
    BOOL deleteDC;
};


///////////////////////////////////////////////////////////////////////////////
// PRedrawCanvas

#include "../../common/rcanvas.h"
  public:
    virtual HDC GetHDC() const;
      // Return the MS-Windows GDI Device Context.

    virtual HDC GetHDC();
      // Return the MS-Windows GDI Device Context, create if does not exist.


  private:
    PAINTSTRUCT paint;
      // Internal structure for when responding to a WM_PAINT event.
};


///////////////////////////////////////////////////////////////////////////////
// PPrintCanvas

#include "../../common/pcanvas.h"
  protected:
};


///////////////////////////////////////////////////////////////////////////////
// PMemoryCanvas

#include "../../common/mcanvas.h"
};


///////////////////////////////////////////////////////////////////////////////
// PInteractor

#include "../../common/interact.h"
  public:
    // New functions for class
    virtual HWND GetHWND();
      // Return the MS-Windows handle. If does not exist yet, create it.

    HWND GetHWND() const;
      // Return the MS-Windows handle.


  protected:
    void SetWndText(const PString & str,
                                  UINT setMsg = WM_SETTEXT, WPARAM wParam = 0);
      // Set the MS-Windows text via SendMessage(). Uses the message codes as
      // supplied in the parameters.

    PString GetWndText(UINT lenMsg = WM_GETTEXTLENGTH, UINT getMsg = WM_GETTEXT,
                             WPARAM wParamGet = -1, WPARAM wParamLen = 0) const;
      // Get the MS-Windows text via SendMessage(). Uses the message codes as
      // supplied in the parameters.

    void SetWndFont();
      // Set the windows font when it is created or changed.
      
    void SetWndCursor();
      // Set the windows cursor when it is changed in this interactor.
      
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name string as required by CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style codes as required by CreateWindow().

    virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // Event handler for this interactor. Translates MS-Windows messages into
      // virtual member function calls.
      
    virtual LRESULT DefWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // Default MS-Windows message handler.


    // Member variables
    HWND wnd;
      // MS-Windows handle

    BOOL inPaint;
      // Prevent recursive errors by remembering if we are in WM_PAINT


    friend class PApplication;
};                                     

#define DeadHWND ((HWND)-1)


///////////////////////////////////////////////////////////////////////////////
// PScrollable

#include "../../common/scrollab.h"
  protected:
    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
    virtual const char * GetWinClsName() const;
};


///////////////////////////////////////////////////////////////////////////////
// PControl

#include "../../common/control.h"
  public:
    // Overrides from class PInteractor
    HWND GetHWND() const;
      // Return the MS-Windows handle for the control.

    virtual HWND GetHWND();
      // Return the MS-Windows handle for the control, call CreateWindow() to
      // create the window if it has not already been created.

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.

  protected:
    virtual LRESULT DefWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // All controls have MS-Windows messages intercepted and passed through
      // this function.


    // Member variables
    WNDPROC wndProc;
      // The default control WndProc, the control is subclassed to allow the
      // PWLib system to intercept messages that it is interested in.
};


///////////////////////////////////////////////////////////////////////////////
// PNamedControl

#include "../../common/ncontrol.h"
  public:
    // Overrides from class PInteractor
    HWND GetHWND() const;
      // Return the MS-Windows handle for the control.

    virtual HWND GetHWND();
      // Return the MS-Windows handle for the control, call CreateWindow() to
      // create the window if it has not already been created.

  private:
    PString initName;
};


///////////////////////////////////////////////////////////////////////////////
// PStaticText

#include "../../common/stattext.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().
};


///////////////////////////////////////////////////////////////////////////////
// PStaticIcon

#include "../../common/staticon.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().
};


///////////////////////////////////////////////////////////////////////////////
// PStaticBox

#include "../../common/statbox.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().
};


///////////////////////////////////////////////////////////////////////////////
// PPushButton

#include "../../common/pbutton.h"
  public:
    // New functions for class
    virtual BOOL IsOwnerDraw() const;
      // Return TRUE if is to handle owner draw messages


  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.
};


///////////////////////////////////////////////////////////////////////////////
// PTextButton

#include "../../common/tbutton.h"
};


///////////////////////////////////////////////////////////////////////////////
// PImageButton

#include "../../common/ibutton.h"
};


///////////////////////////////////////////////////////////////////////////////
// PCheck3WayBox

#include "../../common/check3.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.
};


///////////////////////////////////////////////////////////////////////////////
// PCheckBox

#include "../../common/checkbox.h"
  protected:
    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().
};


///////////////////////////////////////////////////////////////////////////////
// PRadioButton

#include "../../common/rbutton.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.
};


///////////////////////////////////////////////////////////////////////////////
// PChoiceBox

#include "../../common/choicbox.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.
};


///////////////////////////////////////////////////////////////////////////////
// PListBox

#include "../../common/listbox.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.


  public:
    // New functions for class
    virtual BOOL IsOwnerDraw() const;
      // Return TRUE if is to handle owner draw messages
};


///////////////////////////////////////////////////////////////////////////////
// PStringListBox

#include "../../common/slistbox.h"
};


///////////////////////////////////////////////////////////////////////////////
// PComboBox

#include "../../common/combobox.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.
};


///////////////////////////////////////////////////////////////////////////////
// PScrollBar

#include "../../common/scrollb.h"
  public:
    // Overrides from class PInteractor
    HWND GetHWND() const;
      // Return the MS-Windows handle for the control.

    virtual HWND GetHWND();
      // Return the MS-Windows handle for the control, call CreateWindow() to
      // create the window if it has not already been created.


    virtual int TrackScrollBar(WPARAM code, int trackVal);
      // Track changes to a scroll bar. Returns the change in scroll bar value
      // that was caused by the event.


  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().


    // Member variables
  private:
    int initValue, initMin, initMax;
      // These are only used to initialise the MS-Windows scroll bar

    BOOL tracking;
      // Indication that continuous scrolling is occurring.
};


///////////////////////////////////////////////////////////////////////////////
// PVerticalScrollBar

#include "../../common/vscrollb.h"
  protected:
    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().
};


///////////////////////////////////////////////////////////////////////////////
// PHorizontalScrollBar

#include "../../common/hscrollb.h"
  protected:
    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().
};


///////////////////////////////////////////////////////////////////////////////
// PEditBox

#include "../../common/editbox.h"
  protected:
    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual int TranslateOption(WORD msg) const;
      // Translate the windows notification message code to the PWLib
      // notify function code. This returns -1 if the windows message is to be
      // ignored.
};


///////////////////////////////////////////////////////////////////////////////
// PMultiLineEditBox

#include "../../common/meditbox.h"
  protected:
    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
      // Return the MS-Windows style code used in CreateWindow().
};


///////////////////////////////////////////////////////////////////////////////
// PNumberEditBox

#include "../../common/numedbox.h"
};


///////////////////////////////////////////////////////////////////////////////
// PIntegerEditBox

#include "../../common/intedit.h"
};


///////////////////////////////////////////////////////////////////////////////
// PFloatEditBox

#include "../../common/realedit.h"
};


///////////////////////////////////////////////////////////////////////////////
// PToolBar

#include "../../common/toolbar.h"
};


///////////////////////////////////////////////////////////////////////////////
// PButtonBar

#include "../../common/butbar.h"
};


///////////////////////////////////////////////////////////////////////////////
// PStatusBar

#include "../../common/statbar.h"
};


///////////////////////////////////////////////////////////////////////////////
// PTitledWindow

#include "../../common/titlewnd.h"
  public:
    // Overrides from class PInteractor
    HWND GetHWND() const;
      // Return the MS-Windows handle for the control.

    virtual HWND GetHWND();
      // Return the MS-Windows handle for the control, call CreateWindow() to
      // create the window if it has not already been created.


  protected:
    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
      // Return the MS-Windows style for CreateWindow().

    virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // Event handler for this interactor. Translates MS-Windows messages into
      // virtual member function calls.


  private:
    // Member variables
    BOOL canBeClosed;
};


///////////////////////////////////////////////////////////////////////////////
// PDialog

#include "../../common/dialog.h"
  public:
    // Overrides from class PInteractor
    HWND GetHWND() const;
      // Return the MS-Windows handle for the control.

    virtual HWND GetHWND();
      // Return the MS-Windows handle for the control, call CreateWindow() to
      // create the window if it has not already been created.


  protected:
    // Overrides from class PContainer
    virtual void DestroyContents();

    // Overrides from class PInteractor
    virtual const char * GetWinClsName() const;
      // Return the MS-Windows class name used in CreateWindow().

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    virtual LRESULT DefWndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // All controls have MS-Windows messages intercepted and passed through
      // this function.

    // Member variables
    short resourceID;
    const PControlCreators * resourceCreators;
    PINDEX numResourceCreators;
      // Resource information needed for delayed creation of MS-Windows dialog.

    friend BOOL CALLBACK _export
                              EnumDlgChildren(HWND hWnd, PDialog FAR * dialog);
};


///////////////////////////////////////////////////////////////////////////////
// PModalDialog

#include "../../common/modaldlg.h"

  protected:
    enum { Initialising, Running, Ended } runState;
    int returnValue;

    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
      // Return the MS-Windows style used in CreateWindow().

    friend BOOL CALLBACK _export EnumDlgChildren(HWND hWnd, PDialog FAR * dialog);
};


///////////////////////////////////////////////////////////////////////////////
// PSimpleDialog

#include "../../common/simpdlg.h"
};


///////////////////////////////////////////////////////////////////////////////
// PFileDialog

#include "../../common/filedlg.h"
  protected:
    // Overrides from class PContainer
    virtual void DestroyContents();


    // Overrides from class PInteractor
    virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // Event handler for this interactor. Translates MS-Windows messages into
      // virtual member function calls.

    HWND GetHWND();
      // Return the MS-Windows handle for the control.


    // Overrides from class PModalDialog
    virtual void OnOk();
      // Function called when the dialog OK button has been pressed. The
      // default behaviour is to end the modal dialog returning TRUE.

    virtual void OnCancel();
      // Function called when the dialog Cancel button has been pressed. The
      // default behaviour is to end the modal dialog returning FALSE.


    // New member functions for class
    virtual void OnListSelection(UINT listBox, UINT item, UINT operation);
      // Called whenever the standard file dialog changes the selected file in
      // its list boxes


    // Member variables
    OPENFILENAME fileDlgInfo;
    char fileBuffer[P_MAX_PATH];
    UINT selChangeMessage;
};


///////////////////////////////////////////////////////////////////////////////
// POpenFileDialog

#include "../../common/opendlg.h"
  public:
    // Overrides from class PModalDialog
    virtual int RunModal();
      // Execute the dialog in a mode.


    // Overrides from class PFileDialog
    virtual void OnListSelection(UINT listBox, UINT item, UINT operation);
      // Called whenever the standard file dialog changes the selected file in
      // its list boxes
};


///////////////////////////////////////////////////////////////////////////////
// PSaveFileDialog

#include "../../common/savedlg.h"
  public:
    // Overrides from class PModalDialog
    virtual int RunModal();
      // Execute the dialog in a mode.
};


///////////////////////////////////////////////////////////////////////////////
// POpenDirDialog

#include "../../common/dirdlg.h"
  public:
    // Overrides from class PModalDialog
    virtual int RunModal();
      // Execute the dialog in a mode.


  protected:      
    // Overrides from class PInteractor
    virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // Event handler for this interactor. Translates MS-Windows messages into
      // virtual member function calls.


    // Overrides from class PFileDialog
    virtual void OnListSelection(UINT listBox, UINT item, UINT operation);
      // Called whenever the standard file dialog changes the selected file in
      // its list boxes
};


///////////////////////////////////////////////////////////////////////////////
// PSizableWindow

#include "../../common/sizewnd.h"
  protected:
    // Overrides from class PInteractor
    virtual DWORD GetStyle() const;
      // Return the MS-Windows style for CreateWindow().

    virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // Event handler for this interactor. Translates MS-Windows messages into
      // virtual member function calls.


    // Member Variables
    BOOL canBeIcon, canBeZoomed;
      // Variables used to specify the options when the window is created.
};


///////////////////////////////////////////////////////////////////////////////
// PTopLevelWindow

#include "../../common/toplwnd.h"
  protected:
    // Overrides from class PContainer
    void DestroyContents();
      // Destroy the top level window and menu.
      
      
    // Overrides from class PInteractor
    void SetPosition(PORDINATE x, PORDINATE y,
                     PositionOrigin xOrigin = TopLeftScreen,
                     PositionOrigin yOrigin = TopLeftScreen);
      // Set the window position based on the origin and coordinate system
      // specified.

    virtual DWORD GetStyle() const;
      // Return the MS-Windows style flags for CreateWindow().

    LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
      // This function is called for each MS-Windows message received by the
      // window.
};


///////////////////////////////////////////////////////////////////////////////
// PMDIFrameWindow

#include "../../common/mdiframe.h"
  protected:
    HWND GetMDIClient() const;
    HWND GetMDIClient();
    HWND mdiClient;
};


///////////////////////////////////////////////////////////////////////////////
// PMDIDocWindow

#include "../../common/mdidoc.h"
};


///////////////////////////////////////////////////////////////////////////////
// PMenuEntry

#include "../../common/menuent.h"
};


///////////////////////////////////////////////////////////////////////////////
// PMenuSeparator

#include "../../common/menusep.h"
  protected:
    PMenuSeparator(int dummy, PSubMenu * menu);
      // Helper constructor for when creating menu from a resource. Creates
      // the object given the menu ID as used in the resource template.
};


///////////////////////////////////////////////////////////////////////////////
// PMenuItem

#include "../../common/menuitem.h"
};


///////////////////////////////////////////////////////////////////////////////
// PSubMenu

#include "../../common/submenu.h"
  public:
    HMENU GetHMENU() const;
      // Return the menu handle for the menu as required by PTopLevelWindow.


  protected:
    PSubMenu(HMENU hSubMenu, PSubMenu * menu, const char * menuTitle,
                                                  const PMenuBindings * funcs);
      // Helper constructor for when creating menu from a resource. Creates
      // the object given the menu ID as used in the resource template.


    // Overrides from class PContainer
    virtual void DestroyContents();
      // Destroy the window.


    // New functions for class
    void LoadSubMenu(const PMenuBindings * funcs);
      // Function for recursively creating menu items and sub-menus from the
      // menu loaded from a resource.


    // Memeber variables
    HMENU hMenu;
      // MS-Windows menu handle
};


///////////////////////////////////////////////////////////////////////////////
// PRootMenu

#include "../../common/rootmenu.h"
};


///////////////////////////////////////////////////////////////////////////////
// PSound

#include "../../common/sound.h"
};


///////////////////////////////////////////////////////////////////////////////
// PResourceData

#include "../../common/resdata.h"
  protected:
    // Member variables
    HGLOBAL hResource;
    LPSTR lpResource;
};


///////////////////////////////////////////////////////////////////////////////
// PResourceString

#include "../../common/rstring.h"
};


///////////////////////////////////////////////////////////////////////////////
// PApplication

#include "../../common/applicat.h"
  private:
    HINSTANCE hInstance;
      // The MS-Windows instance handle for the running image.

    DECLARE_CLASS(HWNDKey, PObject)
      // This class is used in the hash table lookup for getting a PInteractor
      // pointer given a MS-Windows window handle.
      public:
        HWNDKey(HWND newKey)
          { theKey = newKey; }
        inline PObject * Clone() const
          { return new HWNDKey(theKey); }
        inline Comparison Compare(const PObject & obj) const
          { return theKey != ((const HWNDKey &)obj).theKey
                                                     ? GreaterThan : EqualTo; }
        inline PINDEX HashFunction() const
          { return ((WORD)theKey/8)%23; }
    
      private:
        HWND theKey;
    };
    PDictionary(WindowDict, HWNDKey, PInteractor);
    WindowDict CreatedWindows;

    PList(NonModalDict, PDialog);
    NonModalDict NonModalDialogs;


    friend LRESULT FAR PASCAL _export
                    WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
      // Core function for receiving messages. This dispatches messages to the
      // interactors found in the PApplication::CreatedWindows dictionary. Once
      // attached to an object, its own window message handling takes over.

    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
      // Called by the static function above but is now bound to the
      // application object data.

    friend BOOL FAR PASCAL _export
                    DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
      // Core function for receiving messages. This dispatches messages to the
      // dialogs found in the PApplication::CreatedWindows dictionary. Once
      // attached to an object, its own window message handling takes over.

    BOOL DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
      // Called by the static function above but is now bound to the
      // application object data.

    friend int PASCAL
            WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmd, int show);
      // The MS-Windows entry point.

    int Main(HINSTANCE hInstance, int argc, char **_argv);
      // Application specific version of WinMain


  public:
    HINSTANCE GetInstance() const;
      // Return the MS-Windows instance handle.

    // Support for looking up C++ objects from HWNDs
    void AddWindowHandle(HWND hWnd, PInteractor * pWnd);
    void RemoveWindowHandle(HWND hWnd);
    PInteractor * GetWindowObject(HWND hWnd);

    // Support for non-modal dialogs
    void AddDialog(PDialog * pWnd);
    void RemoveDialog(PDialog * pWnd);
};


#ifdef __BORLANDC__
extern "C" int _argc;
extern "C" char **_argv;
#define __argc _argc
#define __argv _argv
#else
extern "C" int __argc;
extern "C" char **__argv;
#endif


#define DECLARE_MAIN(cls) \
  static PApplication * PApplicationInstance = NULL; \
  LRESULT FAR PASCAL _export WndProc(HWND hW, UINT msg, WPARAM wP, LPARAM lP) \
    { return PApplicationInstance->WndProc(hW, msg, wP, lP); } \
  BOOL FAR PASCAL _export DlgProc(HWND hW, UINT msg, WPARAM wP, LPARAM lP) \
    { return PApplicationInstance->DlgProc(hW, msg, wP, lP); } \
  extern "C" int PASCAL WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) \
    { return (PApplicationInstance = new cls)->Main(hInst, __argc, __argv); }



///////////////////////////////////////////////////////////////////////////////


#include "../../common/pwlib.inl"
#include "pwlib.inl"


#endif // _PWLIB_H

// PWLIB.H
