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
#include <math.h>

static char ControlResourceMismatch[] = "Control class mismatch from resource";


//////////////////////////////////////////////////////////////////////////////
// PControl

PControl::PControl(PInteractor * parent, const PNotifier & notify, void *value)
  : PInteractor(PAssertNULL(parent)),
    callback(notify),
    valuePointer(value)
{
  notifyForStateUpdate = FALSE;
  PRESOURCE_ID id = (PRESOURCE_ID)(parent->GetNumChildren() + 100);
  if (parent->IsDescendant(PInteractorLayout::Class())) {
    while (((PInteractorLayout*)parent)->GetControl(id))
      id++;
  }
  controlID = id;
}


PControl::PControl(PInteractorLayout * parent,
                 PRESOURCE_ID ctlID, const PNotifier & notify, void * valuePtr)
  : PInteractor(NULL)
{
  notifyForStateUpdate = FALSE;
  callback = notify;
  controlID = ctlID;
  valuePointer = valuePtr;
}


BOOL PControl::IsTabStop() const
{
  return FALSE;
}


void PControl::SetControlID(PRESOURCE_ID ID)
{
  controlID = ID;
}


//////////////////////////////////////////////////////////////////////////////
// PNamedControl

PNamedControl::PNamedControl(PInteractor * parent,
           const PString & newName, const PNotifier & notify, void * valuePtr)
  : PControl(parent, notify, valuePtr), name(newName)
{
}


void PNamedControl::SetName(const PString & newName)
{
  name = newName;
}


//////////////////////////////////////////////////////////////////////////////
// PStaticText

PStaticText::PStaticText(PInteractorLayout * parent,
                PRESOURCE_ID ctlID, const PNotifier & notify, void * valuePtr)
  : PNamedControl(parent, ctlID, notify, valuePtr)
{
  alignment = PCanvas::LeftAlign;
}


void PStaticText::SetAlignment(int newAlign)
{
  if (alignment != newAlign) {
    alignment = newAlign;
    Invalidate();
  }
}


void PStaticText::OnRedraw(PCanvas & canvas)
{
  canvas.DrawString(canvas.GetDrawingBounds(), GetName(), alignment);
}


//////////////////////////////////////////////////////////////////////////////
// PEditBox

void PEditBox::SetText(const PString & name)
{
  parent->OnControlNotify(*this, EditChange);
}


//////////////////////////////////////////////////////////////////////////////
// PPasswordEditBox

PPasswordEditBox::PPasswordEditBox(PInteractorLayout * parent,
              PRESOURCE_ID ctlID, const PNotifier & notify, PString * valuePtr)
  : PEditBox(parent, ctlID, notify, valuePtr)
{
}


//////////////////////////////////////////////////////////////////////////////
// PMultiLineEditBox

PMultiLineEditBox::PMultiLineEditBox(PInteractorLayout * parent,
              PRESOURCE_ID ctlID, const PNotifier & notify, PString * valuePtr)
  : PEditBox(parent, ctlID, notify, valuePtr)
{
}


//////////////////////////////////////////////////////////////////////////////
// PIntegerEditBox

PIntegerEditBox::PIntegerEditBox(PInteractorLayout * parent,
                 PRESOURCE_ID ctlID, const PNotifier & notify, long * valuePtr)
  : PNumberEditBox(parent, ctlID, notify, valuePtr)
{
  PStringArray limits;
  PAssert(limits.GetSize() > 1 && limits[0] == "INTEDITBOX", ControlResourceMismatch);
  minimum = limits.GetSize() < 2 ? LONG_MIN : limits[1].AsInteger();
  maximum = limits.GetSize() < 3 ? LONG_MAX : limits[2].AsInteger();
  nudge   = limits.GetSize() < 4 ? 1 : limits[3].AsInteger();
  base    = (BYTE)(limits.GetSize() < 5 ? 1 : limits[4].AsInteger());
  PAssert(base >= 2 && base <= 36, PInvalidParameter);
}


//////////////////////////////////////////////////////////////////////////////
// PFloatEditBox

PFloatEditBox::PFloatEditBox(PInteractorLayout * parent,
               PRESOURCE_ID ctlID, const PNotifier & notify, double * valuePtr)
  : PNumberEditBox(parent, ctlID, notify, valuePtr)
{
  PStringArray limits;
  PAssert(limits.GetSize() > 1 && limits[0] == "FLOATEDITBOX", ControlResourceMismatch);
  minimum  = limits.GetSize() < 2 ? -HUGE_VAL : limits[1].AsReal();
  maximum  = limits.GetSize() < 3 ?  HUGE_VAL : limits[2].AsReal();
  nudge    = limits.GetSize() < 4 ? 1 : limits[3].AsReal();
  decimals = limits.GetSize() < 5 ? 1 : (int)limits[4].AsInteger();
}


//////////////////////////////////////////////////////////////////////////////
// PPushButton

PPushButton::PPushButton(PInteractorLayout * parent,
                PRESOURCE_ID ctlID, const PNotifier & notify, void * valuePtr)
  : PNamedControl(parent, ctlID, notify, valuePtr)
{
  defaultButton = FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PImageButton

void PImageButton::Construct()
{
}


//////////////////////////////////////////////////////////////////////////////
// PCheck3WayBox

PCheck3WayBox::PCheck3WayBox(PInteractorLayout * parent,
          PRESOURCE_ID ctlID, const PNotifier & notify, CheckValues * valuePtr)
  : PNamedControl(parent, ctlID, notify, valuePtr)
{
}


//////////////////////////////////////////////////////////////////////////////
// PCheckBox

PCheckBox::PCheckBox(PInteractorLayout * parent,
                 PRESOURCE_ID ctlID, const PNotifier & notify, BOOL * valuePtr)
  : PCheck3WayBox(parent, ctlID, notify, (CheckValues *)valuePtr)
{
}


//////////////////////////////////////////////////////////////////////////////
// PRadioButton

PRadioButton::PRadioButton(PInteractorLayout * parent,
               PRESOURCE_ID ctlID, const PNotifier & notify, PINDEX * valuePtr)
  : PNamedControl(parent, ctlID, notify, valuePtr)
{
  group.DisallowDeleteObjects();
  group.Append(this);
}


PINDEX PRadioButton::GetValue() const
{
  for (PINDEX i = 0; i < group.GetSize(); i++) {
//    if (SendMessage(group[i].GetHWND(), BM_GETCHECK, 0, 0L) != 0)
//      return i+1;
  }
  return 0;
}


void PRadioButton::SetValue(PINDEX newVal)
{
//  for (PINDEX i = 0; i < group.GetSize(); i++)
//    SendMessage(group[i].GetHWND(), BM_SETCHECK, newVal == i+1, 0L);
}


//////////////////////////////////////////////////////////////////////////////
// PStaticIcon

PStaticIcon::PStaticIcon(PInteractorLayout * parent,
                PRESOURCE_ID ctlID, const PNotifier & notify, void * valuePtr)
  : PControl(parent, ctlID, notify, valuePtr),
    icon(PSTD_ID_ICON_WINDOW)
{
}


void PStaticIcon::SetIcon(const PIcon & icn)
{
  icon = icn;
}


//////////////////////////////////////////////////////////////////////////////
// PStaticBox

PStaticBox::PStaticBox(PInteractorLayout * parent,
                PRESOURCE_ID ctlID, const PNotifier & notify, void * valuePtr)
  : PNamedControl(parent, ctlID, notify, valuePtr)
{
}


//////////////////////////////////////////////////////////////////////////////
// PChoiceBox

PChoiceBox::PChoiceBox(PInteractorLayout * parent,
               PRESOURCE_ID ctlID, const PNotifier & notify, PINDEX * valuePtr)
  : PControl(parent, ctlID, notify, valuePtr)
{
  sort  = FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PListBox

PListBox::PListBox(PInteractorLayout * parent,
               PRESOURCE_ID ctlID, const PNotifier & notify, PINDEX * valuePtr)
  : PControl(parent, ctlID, notify, valuePtr)
{
  sort  = FALSE;
  multi = FALSE;
  width = 0;
}


void PListBox::Construct()
{
}


void PListBox::SetColumnWidth(PDIMENSION newWidth, BOOL update)
{
  PAssert(width != 0 && newWidth != 0, PInvalidParameter);
  width = newWidth;
}


PINDEX PListBox::AddEntry(PObject * obj, BOOL update)
{
  return 0;
}


void PListBox::InsertEntry(PObject * obj,PINDEX index,BOOL update)
{
}


void PListBox::DeleteEntry(PINDEX index, BOOL update)
{
}


void PListBox::DeleteAllEntries(BOOL update)
{
}


PINDEX PListBox::FindEntry(const PObject & obj, PINDEX startIndex) const
{
  return P_MAX_INDEX;
}


void PListBox::SetEntry(PObject * obj, PINDEX index, BOOL update)
{
}


const PObject * PListBox::GetEntry(PINDEX index) const
{
  return NULL;
}


void PListBox::SetTopIndex(PINDEX index, BOOL update)
{
}


PINDEX PListBox::GetSelection() const
{
  return P_MAX_INDEX;
}


void PListBox::SetSelection(PINDEX index)
{
  if (GetSelection() != index) {
    ;
    parent->OnControlNotify(*this, NewSelection);
  }
}


void PListBox::Select(PINDEX index, BOOL update, BOOL sel)
{
  parent->OnControlNotify(*this, NewSelection);
}


//////////////////////////////////////////////////////////////////////////////
// PStringListBox

PINDEX PStringListBox::FindString(
                      const PString & str, PINDEX startIndex, BOOL exact) const
{
  return P_MAX_INDEX;
}


//////////////////////////////////////////////////////////////////////////////
// PComboBox

PComboBox::PComboBox(PInteractorLayout * parent,
              PRESOURCE_ID ctlID, const PNotifier & notify, PString * valuePtr)
  : PControl(parent, ctlID, notify, valuePtr)
{
  sort  = FALSE;
}


BOOL PComboBox::OnEndInput()
{
  parent->OnControlNotify(*this, PEditBox::EndEdit);
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// PScrollBar

PScrollBar::PScrollBar(PInteractorLayout * parent, PRESOURCE_ID ctlID,
                         const PNotifier & notify, PSCROLLBAR_VALUE * valuePtr)
  : PControl(parent, ctlID, notify, valuePtr)
{
  PStringArray limits;

  value = minimum = 0;
  maximum = 100;
  smallNudge = 1;
  largeNudge = 10;

  switch (limits.GetSize()) {
    case 4 :
      largeNudge = (PSCROLLBAR_VALUE)limits[3].AsInteger();
    case 3 :
      smallNudge = (PSCROLLBAR_VALUE)limits[2].AsInteger();
    case 2 :
      maximum = (PSCROLLBAR_VALUE)limits[1].AsInteger();
    case 1 :
      value = minimum = (PSCROLLBAR_VALUE)limits[0].AsInteger();
  }

  foregroundColour = owner->GetScrollBarColour();
  backgroundColour = owner->GetScrollBarColour();
}


void PScrollBar::SetMaximum(PSCROLLBAR_VALUE val, BOOL redraw)
{
  maximum = val;
  if (minimum > maximum)
    minimum = maximum;
  ;
  SetValue(value);
}


void PScrollBar::SetMinimum(PSCROLLBAR_VALUE val, BOOL redraw)
{
  minimum = val;
  if (maximum < minimum)
    maximum = minimum;
  ;
  SetValue(value);
}


void PScrollBar::SetValue(PSCROLLBAR_VALUE val, BOOL redraw)
{
  //SetScrollPos(GetHWND(), SB_CTL, val, redraw);
  PSCROLLBAR_VALUE newValue = 0; //(PSCROLLBAR_VALUE)GetScrollPos(_hWnd, SB_CTL);
  if (value != newValue) {
    parent->OnControlNotify(*this, StartTrack);
    value = newValue;
    parent->OnControlNotify(*this, EndTrack);
  }
}


PVerticalScrollBar::PVerticalScrollBar(PInteractorLayout * parent,
     PRESOURCE_ID ctlID, const PNotifier & notify, PSCROLLBAR_VALUE * valuePtr)
  : PScrollBar(parent, ctlID, notify, valuePtr)
{
}


PHorizontalScrollBar::PHorizontalScrollBar(PInteractorLayout * parent,
     PRESOURCE_ID ctlID, const PNotifier & notify, PSCROLLBAR_VALUE * valuePtr)
  : PScrollBar(parent, ctlID, notify, valuePtr)
{
}


// End Of File ///////////////////////////////////////////////////////////////
