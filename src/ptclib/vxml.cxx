/*
 * vxml.cxx
 *
 * VXML engine for pwlib library
 *
 * Copyright (C) 2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: vxml.cxx,v $
 * Revision 1.1  2002/06/27 05:27:49  craigs
 * Initial version
 *
 *
 */

#ifdef __GNUC__
#pragma implementation "vxml.h"
#endif

#include <ptlib.h>
#include <ptclib/vxml.h>
#include <ptclib/memfile.h>

PVXMLSession::PVXMLSession()
{
  activeGrammar   = NULL;
}

BOOL PVXMLSession::Load(const PFilePath & filename)
{
  PWaitAndSignal m(vxmlMutex);

  if (!xmlFile.LoadFile(filename)) {
    PString err = "Cannot open root document " + filename + " - " + GetXMLError();
    PTRACE(2, "PVXML\t" << err);
    return FALSE;
  }

  PXMLElement * root = xmlFile.GetRootElement();
  if (root == NULL)
    return FALSE;

  // find all dialogs in the document
  PINDEX i;
  for (i = 0; i < root->GetSize(); i++) {
    PXMLObject * xmlObject = root->GetElement(i);
    if (xmlObject->IsElement()) {
      PXMLElement * xmlElement = (PXMLElement *)xmlObject;
      PVXMLDialog * dialog = NULL;

      if (xmlElement->GetName() == "form") {
        dialog = new PVXMLFormDialog(*this, *xmlElement);
        dialog->Load();
      }

      if (dialog != NULL)
        dialogArray.SetAt(dialogArray.GetSize(), dialog);
    }
  }

  return TRUE;
}

BOOL PVXMLSession::Execute()
{
  PWaitAndSignal m(vxmlMutex);

  return ExecuteWithoutLock();
}


BOOL PVXMLSession::ExecuteWithoutLock()
{
  // if there is a grammar defined, then no need to look for fields
  if (activeGrammar != NULL)
    return TRUE;

  // find the first dialog that has an undefined form variable
  PINDEX i;
  for (i = 0; i < dialogArray.GetSize(); i++) {
    PVXMLDialog & dialog = dialogArray[i];

    // if this form is not yet defined, then enter it
    if (!dialog.GetGuardCondition()) {

      // execute the form, and clear call if error
      if (!dialog.Execute())
        break;
    }
  }

  // if all forms defined, end of call
  if ((activeGrammar == NULL) && !IsPlaying())
    ClearCall();

  return FALSE;
}


BOOL PVXMLSession::OnUserInput(char ch)
{
  PWaitAndSignal m(vxmlMutex);

  if (activeGrammar != NULL) {

    // if the grammar has not completed, continue
    if (!activeGrammar->OnUserInput(ch))
      return TRUE;

    // if the grammar has completed, save the value and define the field
    activeGrammar->GetField().SetFormValue(activeGrammar->GetValue());

    // remove the grammar
    LoadGrammar(NULL);

    // execute whatever is going on
    ExecuteWithoutLock();
  }

  return TRUE;
}

PString PVXMLSession::GetXMLError() const
{
  return psprintf("(%i:%i) ", xmlFile.GetErrorLine(), xmlFile.GetErrorColumn()) + xmlFile.GetErrorString();
}

BOOL PVXMLSession::LoadGrammar(PVXMLGrammar * grammar)
{
  if (activeGrammar != NULL) {
    delete activeGrammar;
    activeGrammar = FALSE;
  }

  activeGrammar = grammar;

  return TRUE;
}

PString PVXMLSession::GetVar(const PString & ostr) const
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  // process session scope
  if (scope.IsEmpty() || (scope *= "session")) {
    if (sessionVars.Contains(str))
      return sessionVars(str);
  }

  // assume any other scope is actually document or application
  return documentVars(str);
}

void PVXMLSession::SetVar(const PString & ostr, const PString & val)
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  // do session scope
  if (scope.IsEmpty() || (scope *= "session")) {
    sessionVars.SetAt(str, val);
    return;
  }

  PTRACE(3, "PVXML\tDocument: " << str << " = \"" << val << "\"");

  // assume any other scope is actually document or application
  documentVars.SetAt(str, val);
}


///////////////////////////////////////////////////////////////

PVXMLElement::PVXMLElement(PVXMLSession & _vxml, PXMLElement & _xmlElement)
  : vxml(_vxml), xmlElement(_xmlElement)
{
  name = xmlElement.GetAttribute("name");
  if (name.IsEmpty())
    name = psprintf("item_%08x", (int)this);
}

PString PVXMLElement::GetVar(const PString & str) const
{
  return vars(str);
}

void PVXMLElement::SetVar(const PString & str, const PString & val)
{
  vars.SetAt(str, val); 
}

BOOL PVXMLElement::GetGuardCondition() const
{ 
  return !GetFormValue().IsEmpty();
}

PString PVXMLElement::GetFormValue() const
{ 
  return PVXMLElement::GetVar(name);
}

void PVXMLElement::SetFormValue(const PString & v)
{ 
  PVXMLElement::SetVar(name, v);
}

///////////////////////////////////////////////////////////////

PVXMLDialog::PVXMLDialog(PVXMLSession & _vxml, PXMLElement & _xmlForm)
  : PVXMLElement(_vxml, _xmlForm)
{
}

BOOL PVXMLDialog::Load()
{
  // find all items in form
  PINDEX i;
  for (i = 0; i < xmlElement.GetSize(); i++) {
    PXMLObject * object = xmlElement.GetElement(i);
    if (object->IsElement()) {
      PXMLElement * element = (PXMLElement *)object;
      PVXMLFormItem * formItem = NULL;

      if (element->GetName() == "block") {
        PVXMLBlockItem * blockItem = new PVXMLBlockItem(vxml, *element, *this);
        blockItem->Load();
        formItem = blockItem;
      }

      else if (element->GetName() == "var") {
        PVXMLVarItem * varItem = new PVXMLVarItem(vxml, *element, *this);
        varItem->Load();
        formItem = varItem;
      }

      else if (element->GetName() == "field") {
        PVXMLFieldItem * fieldItem = new PVXMLFieldItem(vxml, *element, *this);
        fieldItem->Load();
        formItem = fieldItem;
      }

      itemArray.SetAt(itemArray.GetSize(), formItem);
    }
  }

  return TRUE;
}

BOOL PVXMLDialog::Execute()
{
  // return TRUE if we executed 
  PINDEX i;
  for (i = 0; i < itemArray.GetSize(); i++) {
    PVXMLFormItem & item = itemArray[i];
    if (!item.GetGuardCondition())
      return item.Execute();
  }

  return FALSE;
}

PString PVXMLDialog::GetVar(const PString & ostr) const
{
  PString str = ostr;

  // if the variable has scope, check to see if dialog otherwise move up the chain
  PINDEX pos = ostr.Find('.');
  if (pos != P_MAX_INDEX) {
    PString scope = str.Left(pos);
    if (!(scope *= "dialog"))
      return vxml.GetVar(str);

    str = str.Mid(pos+1);
  }

  // see if local
  if (vars.Contains(str))
    return PVXMLElement::GetVar(str);

  return vxml.GetVar(ostr);
}

void PVXMLDialog::SetVar(const PString & ostr, const PString & val)
{
  PString str = ostr;
  PString scope;

  // get scope if present
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str = str.Mid(pos+1);
  }

  // if scope is not dialog, for
  if (scope.IsEmpty() || (scope *= "dialog")) {
    PVXMLElement::SetVar(str, val);
    return;
  }

  PTRACE(3, "PVXML\tDialog(" << name << "): " << ostr << " = \"" << val << "\"");

  vxml.SetVar(ostr, val);
}

///////////////////////////////////////////////////////////////

PVXMLFormDialog::PVXMLFormDialog(PVXMLSession & vxml, PXMLElement & xmlItem)
  : PVXMLDialog(vxml, xmlItem)
{
}

///////////////////////////////////////////////////////////////

PVXMLFormItem::PVXMLFormItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLElement(_vxml, _xmlItem), parentDialog(_parentDialog)
{
}

PString PVXMLFormItem::GetFormValue() const
{
  return GetVar("dialog." + name);
}

void PVXMLFormItem::SetFormValue(const PString & v)
{
  SetVar("dialog." + name, v);
}

BOOL PVXMLFormItem::ProcessPrompt(PXMLElement & rootElement)
{
  PINDEX i;
  for (i = 0; i < rootElement.GetSize(); i++) {
    PXMLObject * object = rootElement.GetElement(i);
    if (object->IsData())
      vxml.PlayText(((PXMLData *)object)->GetString());

    else {
      PXMLElement * element = (PXMLElement *)object;
      if (element->GetName() *= "value") {
        PString className = element->GetAttribute("class");
        PString value = EvaluateExpr(element->GetAttribute("expr"));
        SayAs(className, value);
      }

      else if (element->GetName() *= "sayas") {
        PString className = element->GetAttribute("class");
        PXMLObject * object = element->GetElement();
        if (object->IsData()) {
          PString text = ((PXMLData *)object)->GetString();
          SayAs(className, text);
        }
      }
    }
  }
  return TRUE;
}

void PVXMLFormItem::SayAs(const PString & className, const PString & text)
{
  if (!text.IsEmpty()) {
    PVXMLSession::TextType type = PVXMLSession::Literal;
    if (className *= "digits")
      type = PVXMLSession::Digits;
    else if (className *= "literal")
      type = PVXMLSession::Literal;
    else if (className *= "number")
      type = PVXMLSession::Number;
    else if (className *= "currency")
      type = PVXMLSession::Currency;
    vxml.PlayText(text, type);
  }
}

PString PVXMLFormItem::EvaluateExpr(const PString & oexpr)
{
  PString expr = oexpr.Trim();

  // see if all digits
  PINDEX i;
  BOOL allDigits = TRUE;
  for (i = 0; i < expr.GetLength(); i++) {
    allDigits = allDigits && isdigit(expr[i]);
  }

  if (allDigits)
    return expr;

  return GetVar(expr);
}

PString PVXMLFormItem::GetVar(const PString & ostr) const
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  if (scope.IsEmpty()) {
    if (vars.Contains(str))
      return PVXMLElement::GetVar(str);
  }

  return parentDialog.GetVar(str);
}

void PVXMLFormItem::SetVar(const PString & ostr, const PString & val)
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  if (scope.IsEmpty())
    PVXMLElement::SetVar(str, val);

  parentDialog.SetVar(ostr, val);
}

///////////////////////////////////////////////////////////////

PVXMLBlockItem::PVXMLBlockItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLFormItem(_vxml, _xmlItem, _parentDialog)
{
}

BOOL PVXMLBlockItem::Execute()
{
  PINDEX i;
  for (i = 0; i < xmlElement.GetSize(); i++) {
    PXMLObject * object = xmlElement.GetElement(i);
    if (object->IsData())
      vxml.PlayText(((PXMLData *)object)->GetString());
    else {
      PXMLElement * element = (PXMLElement *)object;
      if (element->GetName() == "prompt")
        ProcessPrompt(*element);
    }
  }
  this->SetFormValue("1");

  return TRUE;
}

///////////////////////////////////////////////////////////////

PVXMLVarItem::PVXMLVarItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLFormItem(_vxml, _xmlItem, _parentDialog)
{
}

BOOL PVXMLVarItem::Execute()
{
  PString name = xmlElement.GetAttribute("name");
  PString expr = xmlElement.GetAttribute("expr");

  PTRACE(3, "PVXML\tAssigning expr \"" << expr << "\" to var \"" << name << "\" in scope of dialog \"" << parentDialog.GetName());

  parentDialog.SetVar(name, expr);

  return TRUE;
}

///////////////////////////////////////////////////////////////

PVXMLFieldItem::PVXMLFieldItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLFormItem(_vxml, _xmlItem, _parentDialog)
{
}

BOOL PVXMLFieldItem::Execute()
{
  PINDEX i;

  // queue up the prompts
  for (i = 0; i < xmlElement.GetSize(); i++) {
    PXMLObject * object = xmlElement.GetElement(i);
    if (object->IsData())
      vxml.PlayText(((PXMLData *)object)->GetString());
    else {
      PXMLElement * element = (PXMLElement *)object;
      if (element->GetName() == "prompt")
        ProcessPrompt(*element);
    }
  }

  // load the grammar for this field, if we can build it
  PVXMLGrammar * grammar = NULL;
  PString grammarType = xmlElement.GetAttribute("type");
  if (grammarType == "digits") {
    PString lengthStr = xmlElement.GetAttribute("length");
    if (!lengthStr.IsEmpty()) {
      grammar = new PVXMLDigitsGrammar(*this, lengthStr.AsInteger());
    }
  }

  if (grammar != NULL)
    return vxml.LoadGrammar(grammar);

  return FALSE;
}

//////////////////////////////////////////////////////////////////

PVXMLGrammar::PVXMLGrammar(PVXMLFieldItem & _field)
  : field(_field)
{
}

//////////////////////////////////////////////////////////////////

PVXMLMenuGrammar::PVXMLMenuGrammar(PVXMLFieldItem & _field)
  : PVXMLGrammar(_field)
{
}

//////////////////////////////////////////////////////////////////

PVXMLDigitsGrammar::PVXMLDigitsGrammar(PVXMLFieldItem & _field, PINDEX _digitCount)
  : PVXMLGrammar(_field), digitCount(_digitCount)
{
}

BOOL PVXMLDigitsGrammar::OnUserInput(char ch)
{
  value += ch;

  if (value.GetLength() < digitCount)
    return FALSE;

  cout << "grammar \"digits\" completed: value = " << value << endl;

  return TRUE;
}

///////////////////////////////////////////////////////////////

