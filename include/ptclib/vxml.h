/*
 * vxml.h
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
 * $Log: vxml.h,v $
 * Revision 1.1  2002/06/27 05:28:17  craigs
 * Initial version
 *
 *
 */

#ifndef _VXML_H
#define _VXML_H

#ifdef __GNUC__
#pragma interface
#endif

#include <ptclib/pxml.h>
#include <ptclib/delaychan.h>
#include <ptclib/pwavfile.h>

class PVXMLSession;


class PVXMLDialog;
class PVXMLSession;

class PVXMLElement : public PObject
{
  PCLASSINFO(PVXMLElement, PObject);
  public:
    PVXMLElement(PVXMLSession & vxml, PXMLElement & xmlElement);

    virtual BOOL Load()     { return TRUE; }
    virtual BOOL Execute()  { return TRUE; }

    virtual PString GetVar(const PString & str) const;
    virtual void    SetVar(const PString & str, const PString & val);

    virtual BOOL GetGuardCondition() const;

    virtual PString GetFormValue() const;
    virtual void SetFormValue(const PString & v);

    PString GetName() const { return name; }

  protected:
    PVXMLSession & vxml;
    PXMLElement  & xmlElement;
    PStringToString vars;
    PString name;
};

//////////////////////////////////////////////////////////////////

class PVXMLFormItem : public PVXMLElement 
{
  PCLASSINFO(PVXMLFormItem, PObject);
  public:
    PVXMLFormItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL ProcessPrompt(PXMLElement & element);
    void SayAs(const PString & className, const PString & text);

    PVXMLDialog & GetParentDialog() { return parentDialog; }

    PString EvaluateExpr(const PString & oexpr);

    PString GetFormValue() const;
    void SetFormValue(const PString & v);

    PString GetVar(const PString & str) const;
    void    SetVar(const PString & str, const PString & val);

  protected:
    PVXMLDialog & parentDialog;
    PStringToString formVars;
};

PARRAY(PVXMLFormItemArray, PVXMLFormItem);

//////////////////////////////////////////////////////////////////

class PVXMLBlockItem : public PVXMLFormItem
{
  PCLASSINFO(PVXMLBlockItem, PVXMLFormItem);
  public:
    PVXMLBlockItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL Execute();
};

//////////////////////////////////////////////////////////////////

class PVXMLFieldItem : public PVXMLFormItem
{
  PCLASSINFO(PVXMLFieldItem, PVXMLFormItem);
  public:
    PVXMLFieldItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL Execute();
};

//////////////////////////////////////////////////////////////////

class PVXMLVarItem : public PVXMLFormItem
{
  PCLASSINFO(PVXMLVarItem, PVXMLFormItem);
  public:
    PVXMLVarItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL Execute();
};

//////////////////////////////////////////////////////////////////

class PVXMLDialog : public PVXMLElement
{
  PCLASSINFO(PVXMLDialog, PObject);
  public:
    PVXMLDialog(PVXMLSession & vxml, PXMLElement & xmlItem);
    BOOL Load();
    BOOL Execute();

    PString GetVar(const PString & str) const;
    void SetVar(const PString & ostr, const PString & val);

  protected:
    PVXMLFormItemArray itemArray;
};

PARRAY(PVXMLDialogArray, PVXMLDialog);

//////////////////////////////////////////////////////////////////

class PVXMLFormDialog : public PVXMLDialog
{
  PCLASSINFO(PVXMLFormDialog, PVXMLDialog);
  public:
    PVXMLFormDialog(PVXMLSession & vxml, PXMLElement & xmlItem);
};

//////////////////////////////////////////////////////////////////

class PVXMLGrammar : public PObject
{
  PCLASSINFO(PVXMLGrammar, PObject);
  public:
    PVXMLGrammar(PVXMLFieldItem & field);
    virtual BOOL OnUserInput(char /*ch*/) { return TRUE; }

    PString GetValue() const { return value; }
    PVXMLFieldItem & GetField() { return field; }

  protected:
    PVXMLFieldItem & field;
    PString value;
};

//////////////////////////////////////////////////////////////////

class PVXMLMenuGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLMenuGrammar, PVXMLGrammar);
  public:
    PVXMLMenuGrammar(PVXMLFieldItem & field);
};

//////////////////////////////////////////////////////////////////

class PVXMLDigitsGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLDigitsGrammar, PVXMLGrammar);
  public:
    PVXMLDigitsGrammar(PVXMLFieldItem & field, PINDEX digitCount);
    BOOL OnUserInput(char ch);

  protected:
    PINDEX digitCount;
    PString digits;
};

//////////////////////////////////////////////////////////////////

class PVXMLSession : public PObject 
{
  PCLASSINFO(PVXMLSession, PObject);
  public:
    enum TextType {
      Default,
      Literal,
      Digits,
      Number,
      Currency
    };

    PVXMLSession();

    virtual BOOL Load(const PFilePath & xmlSource);

    BOOL Execute();

    BOOL LoadGrammar(PVXMLGrammar * grammar);

    virtual BOOL PlayText(const PString & text, TextType type = Default) = 0;
    virtual BOOL PlayFile(const PString & fn) = 0;
    virtual BOOL PlayData(const PBYTEArray & data) = 0;

    virtual BOOL IsPlaying() const = 0;

    virtual BOOL OnUserInput(char ch);

    PString GetXMLError() const;

    virtual void ClearCall() { };

    virtual PString GetVar(const PString & str) const;
    virtual void SetVar(const PString & ostr, const PString & val);

  protected:
    BOOL ExecuteWithoutLock();

    PMutex vxmlMutex;

    PXML xmlFile;
    PVXMLDialogArray dialogArray;
    PStringToString textMap;
    PStringToString digitMap;

    PVXMLGrammar * activeGrammar;

    PStringToString sessionVars;
    PStringToString documentVars;
};

//////////////////////////////////////////////////////////////////

#endif