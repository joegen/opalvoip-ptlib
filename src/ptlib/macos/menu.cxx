/*
 * $Id: menu.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: menu.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */


#include <pwlib.h>

#ifndef P_USE_INLINES
#include <interact.inl>
#endif


//////////////////////////////////////////////////////////////////////////////
// PMenuEntry

PMenuEntry::~PMenuEntry()
{
  if (itsMenu != NULL) {
    PINDEX pos = GetPosition();
    if (pos != P_MAX_INDEX) {
      if (itsMenu->itsMenu != NULL)
        DelMenuItem(itsMenu->GetMenuHandle(), pos+1);
      itsMenu->entries.DisallowDeleteObjects();
      itsMenu->entries.Remove(this);
      itsMenu->entries.AllowDeleteObjects();
    }
  }
}


//////////////////////////////////////////////////////////////////////////////
// PMenuItem

void PMenuItem::Construct(BOOL setID)
{
  PRootMenu * root = GetRootMenu();
  if (root != NULL) {
    if (setID) {
      menuID = 1;
      while (root->GetItemFromKey(menuID) != NULL)
        menuID++;
    }
    root->keyedItems.SetAt(menuID, this);
  }
  if (setID) {
    InsMenuItem(itsMenu->GetMenuHandle(), (ConstStr255Param)"\001 ", GetPosition());
    SetString(name);
  }
}


void PMenuItem::SetString(const PString & str)
{
  PAssert(!str.IsEmpty(), PInvalidParameter);
  name = str;
  SetItem(itsMenu->GetMenuHandle(), GetPosition()+1, name.ToPascal());
}


void PMenuItem::Enable(BOOL enabled)
{
  if (enabled)
    EnableItem(itsMenu->GetMenuHandle(), GetPosition()+1);
  else
    DisableItem(itsMenu->GetMenuHandle(), GetPosition()+1);
}


void PMenuItem::Check(BOOL checked)
{
  CheckItem(itsMenu->GetMenuHandle(), GetPosition()+1, checked);
}


//////////////////////////////////////////////////////////////////////////////


PMenuSeparator::PMenuSeparator(PSubMenu & menu, PMenuEntry * before)
  : PMenuEntry(menu, before)
{
  InsMenuItem(itsMenu->GetMenuHandle(), (ConstStr255Param)"\001-", GetPosition());
}



//////////////////////////////////////////////////////////////////////////////
// PSubMenu

PSubMenu::PSubMenu(PSubMenu & menu,
                               const PString & menuTitle, PMenuEntry * before)
  : PMenuEntry(menu, before)
{
  title = menuTitle;

  PRootMenu * root = GetRootMenu();

  int menuID = root != itsMenu ? 1 : FirstID+1;
  while (root->subMenus.GetAt(menuID) != NULL) {
    menuID++;
    PAssert(root == itsMenu || menuID < 236, "Cannot create menu item");
  }

  hMenu = NewMenu(menuID, title.ToPascal());
  PAssertNULL(hMenu);
  root->subMenus.SetAt(menuID, this);

  if (root != itsMenu) { // hierarchical sub-menu
  	int pos = GetPosition();
    InsMenuItem(itsMenu->GetMenuHandle(), (ConstStr255Param)"\001 ", pos++);
    SetItem(itsMenu->GetMenuHandle(), pos, title.ToPascal());
    SetItemCmd(itsMenu->GetMenuHandle(), pos, hMenuCmd);
    SetItemMark(itsMenu->GetMenuHandle(), pos, menuID);
    if (root->activeMenu) {
      InsertMenu(hMenu, -1);
      DrawMenuBar();
    }
  }
  else if (root->activeMenu) {
    InsertMenu(hMenu, GetPosition());
    DrawMenuBar();
  }
}


PSubMenu::~PSubMenu()
{
  entries.RemoveAll();
  short menuID = (*itsMenu->GetMenuHandle())->menuID;
  GetRootMenu()->subMenus.SetAt(menuID, NULL);
  if (((PRootMenu*)itsMenu)->activeMenu) {
	  DeleteMenu(menuID);
	  DrawMenuBar();
	}
	DisposeMenu(hMenu);
}


void PSubMenu::SetString(const PString & str)
{
  PAssert(!str.IsEmpty(), PInvalidParameter);
  title = str;
  if (itsMenu->GetMenu() != NULL) // hierarchical sub-menu
    SetItem(itsMenu->GetMenuHandle(), GetPosition()+1, title.ToPascal());
}


void PSubMenu::InsertIntoMenuBar(int pos) const
{
  InsertMenu(hMenu, pos);
  for (PINDEX i = 0; i < entries.GetSize(); i++) {
    PSubMenu & submenu = (PSubMenu &)entries[i];
    if (submenu.IsDescendant(PSubMenu::Class()))
      submenu.InsertIntoMenuBar(-1);
  }
}


//////////////////////////////////////////////////////////////////////////////
// PRootMenu

PRootMenu::PRootMenu()
{
  keyedItems.DisallowDeleteObjects();

  hMenu = NULL;
}

 
PRootMenu::PRootMenu(PRESOURCE_ID resID)
{
  keyedItems.DisallowDeleteObjects();

  activeMenu = FALSE;
  Handle mbar = GetNewMBar(resID);
  SetMenuBar(mbar);
  DisposHandle(mbar);
}


//////////////////////////////////////////////////////////////////////////////

void PTopLevelWindow::SetMenu(PRootMenu * newMenu, BOOL autoDelete)
{
  if (menu != NULL) {
    if (deleteMenu)
      delete menu;
	else
	  menu->activeMenu = FALSE;
  }

  menu = newMenu;
  deleteMenu = autoDelete;

  menu->activeMenu = TRUE;

  ClearMenuBar();
  static MenuHandle appleMenu;
  if (appleMenu == NULL) {
    Str255 appleTitle;
    appleTitle[0] = 1;
    appleTitle[1] = appleMark;
    appleMenu = NewMenu(PSubMenu::FirstID, appleTitle);
    AppendMenu(appleMenu, owner->GetAboutMenuItemString().ToPascal());
    AppendMenu(appleMenu, (ConstStr255Param)"\001-");
    AddResMenu(appleMenu, 'DRVR');	// add DA names to Apple menu
  }
  InsertMenu(appleMenu, 0);
  for (PINDEX i = 0; i < menu->GetSize(); i++)
    ((PSubMenu&)(*menu)[i]).InsertIntoMenuBar(0);
  DrawMenuBar();
}


void PTopLevelWindow::HandleMenu(long menuCmd)
{
  short id = HiWord(menuCmd);
  short pos = LoWord(menuCmd);
  if (id == PSubMenu::FirstID) {
	if (pos == 1) { // About
	}
	else { // all non-About items in this menu are DAs et al
	  Str255 daName;
	  GetItem(GetMHandle(PSubMenu::FirstID), pos, daName);
	  OpenDeskAcc(daName);
	}
  }
  else {
	if (menu->subMenus.GetAt(id) != NULL)
	  OnMenuItemSelect((PMenuItem&)menu->subMenus[id][pos-1]);
  }

  HiliteMenu(0);
}


// End Of File ///////////////////////////////////////////////////////////////
