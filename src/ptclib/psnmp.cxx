/*
 * SNMP Library
 *
 * $Id: psnmp.cxx,v 1.2 1996/11/04 03:59:19 robertj Exp $
 *
 * Copyright 1996 by Equivalence
 *
 * $Log: psnmp.cxx,v $
 * Revision 1.2  1996/11/04 03:59:19  robertj
 * Added selectable read buffer size.
 *
 * Revision 1.1  1996/09/14 13:02:18  robertj
 * Initial revision
 *
 * Revision 1.9  1996/05/29 10:44:51  craigs
 * Latest version wil traps and discovery
 *
 * Revision 1.8  1996/05/09 13:23:49  craigs
 * Added trap functions
 *
 * Revision 1.7  1996/04/23 12:12:46  craigs
 * Changed to use GetErrorText function
 *
 * Revision 1.6  1996/04/16 13:20:43  craigs
 * Final version prior to beta1 release
 *
 * Revision 1.5  1996/04/15 09:05:30  craigs
 * Latest version prior to integration with Robert's changes
 *
 * Revision 1.4  1996/04/06 11:38:35  craigs
 * Lots of changes - working version prior to discover changes
 *
 * Revision 1.3  1996/04/01 12:50:44  craigs
 * CHanged for clean compile under NT
 *
 * Revision 1.2  1996/04/01 12:34:06  craigs
 * Added RCS header
 *
 *
 */

#include <ptlib.h>
#include <psnmp.h>


static char *SnmpErrorCodeTable[] = 
{
  "no error",
  "too big",
  "no such name",
  "bad value",
  "read only",
  "gen err",

  "no response",
  "malformed response",
  "send failed",
  "rx buff too small",
  "tx data too big"
};

static char * TrapCodeToText[PSNMP::NumTrapTypes] = {
  "Cold Start",
  "Warm Start",
  "Link Down",
  "Link Up",
  "Auth Fail",
  "EGP Loss",
  "Enterprise"
};


///////////////////////////////////////////////////////////////
//
//  PSNMPVarBindingList
//

void PSNMPVarBindingList::Append(const PString & objectID)
{
  objectIds.AppendString(objectID);
  values.Append(PNEW PASNString(""));
}


void PSNMPVarBindingList::Append(const PString & objectID, PASNObject * obj)
{
  objectIds.AppendString(objectID);
  values.Append(obj);
}


void PSNMPVarBindingList::AppendString(const PString & objectID, const PString & str)
{
  Append(objectID, PNEW PASNString(str));
}


void PSNMPVarBindingList::RemoveAll()
{
  objectIds.RemoveAll();
  values.RemoveAll();
}


PINDEX PSNMPVarBindingList::GetSize() const
{
  return objectIds.GetSize();
}


PASNObject & PSNMPVarBindingList::operator[](PINDEX idx) const
{
  return values[idx];
}


PString PSNMPVarBindingList::GetObjectID(PINDEX idx) const
{ 
  return objectIds[idx];
}


void PSNMPVarBindingList::PrintOn(ostream & strm) const
{
  for (PINDEX i = 0; i < GetSize(); i++) 
    strm << objectIds[i] 
         << " = "
         << values[i];
}


PString PSNMP::GetTrapTypeText(PINDEX code)
{
  PString str;
  if (code >= NumTrapTypes)
    return "Unknown";
  else
    return TrapCodeToText[code];
}


PString PSNMP::GetErrorText(ErrorType err) 
{
  if (err >= NumErrors)
    return "unknown error";
  else
    return SnmpErrorCodeTable[err];
}


// End Of File ///////////////////////////////////////////////////////////////
