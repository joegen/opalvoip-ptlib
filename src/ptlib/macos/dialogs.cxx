/*
 * $Id: dialogs.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: dialogs.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <pwlib.h>


///////////////////////////////////////////////////////////////////////////////
// PInteractorLayout

PInteractorLayout::PInteractorLayout(PInteractor * parent)
  : PInteractor(parent)
{
  focusInteractor = this;
}


PInteractorLayout::PInteractorLayout(PInteractor * parent, PRESOURCE_ID resID)
  : PInteractor(parent)
{
  focusInteractor = this;
}


PInteractorLayout::~PInteractorLayout()
{
}


void PInteractorLayout::ConstructEnd(PRESOURCE_ID resID)
{
}


///////////////////////////////////////////////////////////////////////////////
// PFloatingDialog

void PFloatingDialog::Construct()
{
  titleFont = PRealFont(PFont("Helvetica", 1));  // Smallest font possible
  borderWidth = owner->GetBorderSize().Width();
  borderHeight = owner->GetBorderSize().Height();
  titleHeight = titleFont.GetHeight(TRUE) + borderHeight*2;
}


///////////////////////////////////////////////////////////////////////////////
// PModalDialog

void PModalDialog::Construct()
{
  ok = cancel = NULL;
}


int PModalDialog::RunModal()
{
  owner->GetWindow()->Disable();

  OnInit();
  Show();

  return 0;
}


void PModalDialog::EndModal(int retVal)
{
  owner->GetWindow()->Enable();

  Hide();
}


///////////////////////////////////////////////////////////////////////////////
// PFileDialog

PFileDialog::PFileDialog(PInteractor * parent, PRESOURCE_ID resID)
  : PModalDialog(parent, resID)
{
}


PFileDialog::~PFileDialog()
{
}


void PFileDialog::SetTitle(const PString & title)
{
}


///////////////////////////////////////////////////////////////////////////////
// POpenFileDialog

POpenFileDialog::POpenFileDialog(PInteractor * parent, PRESOURCE_ID resID)
  : PFileDialog(parent, resID)
{
}


int POpenFileDialog::RunModal()
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PSaveFileDialog

PSaveFileDialog::PSaveFileDialog(PInteractor * parent, PRESOURCE_ID resID)
  : PFileDialog(parent, resID)
{
}


int PSaveFileDialog::RunModal()
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// POpenDirDialog

POpenDirDialog::POpenDirDialog(PInteractor * parent, PRESOURCE_ID resID)
  : PFileDialog(parent, resID)
{
}


int POpenDirDialog::RunModal()
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PPrintDialog

void PPrintDialog::Construct()
{
}


PPrintDialog::~PPrintDialog()
{
}


int PPrintDialog::RunModal()
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PPrinterSetupDialog

void PPrinterSetupDialog::Construct()
{
}


///////////////////////////////////////////////////////////////////////////////
// PPrintJobDialog

void PPrintJobDialog::Construct()
{
}


///////////////////////////////////////////////////////////////////////////////
// PFontDialog

void PFontDialog::Construct()
{
}


PFontDialog::~PFontDialog()
{
}


void PFontDialog::SetDefaultFont(const PFont & fon)
{
  font = fon;
}


int PFontDialog::RunModal()
{
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// PColourDialog

void PColourDialog::Construct(PRESOURCE_ID)
{
}


PColourDialog::~PColourDialog()
{
}


void PColourDialog::SetColour(const PColour & col)
{
  colour = col;
}


int PColourDialog::RunModal()
{
  return FALSE;
}


// End Of File ///////////////////////////////////////////////////////////////
