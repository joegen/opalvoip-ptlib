/*
 * RemConn.CXX
 *
 * Simple proxy service for internet access under Unix
 *
 * Copyright 1995 Equivalence
 */

#pragma implementation "remconn.h"

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
  return TRUE;
}


void PRemoteConnection::Close()
{
}


PRemoteConnection::Status PRemoteConnection::GetStatus() const
{
//  return Idle;
//  return NoNameOrNumber;
//  return LineBusy;
//  return NoDialTone;
//  return NoAnswer;
//  return PortInUse;
  return Connected;
//  return InProgress
//  return GeneralFailure;
}

// End of File ////////////////////////////////////////////////////////////////
