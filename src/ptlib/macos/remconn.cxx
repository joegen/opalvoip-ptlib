/*
 * $Id: remconn.cxx,v 1.1 1996/01/02 13:11:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1995 Equivalance
 *
 * $Log: remconn.cxx,v $
 * Revision 1.1  1996/01/02 13:11:52  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <remconn.h>


PRemoteConnection::PRemoteConnection()
{
  Construct();
}


PRemoteConnection::PRemoteConnection(const PString & name)
  : remoteName(name)
{
  Construct();
}


PRemoteConnection::~PRemoteConnection()
{
  Close();
}


BOOL PRemoteConnection::Open(const PString & name)
{
  if (name != remoteName) {
    Close();
    remoteName = name;
  }
  return Open();
}


PObject::Comparison PRemoteConnection::Compare(const PObject & obj) const
{
  return remoteName.Compare(((const PRemoteConnection &)obj).remoteName);
}


PINDEX PRemoteConnection::HashFunction() const
{
  return remoteName.HashFunction();
}


void PRemoteConnection::Construct()
{
}


BOOL PRemoteConnection::Open()
{
  Close();

  return FALSE;
}


void PRemoteConnection::Close()
{
}


PRemoteConnection::Status PRemoteConnection::GetStatus() const
{
#if 0
  return Idle;
  return NoNameOrNumber;
  return LineBusy;
  return NoDialTone;
  return NoAnswer;
  return PortInUse;
  return Connected;
  return InProgress;
#endif
  return GeneralFailure;
}


// End of File ////////////////////////////////////////////////////////////////
