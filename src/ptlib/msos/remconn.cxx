/*
 * $Id: remconn.cxx,v 1.8 1996/11/04 03:37:23 robertj Exp $
 *
 * Simple proxy service for internet access under Windows NT.
 *
 * Copyright 1995 Equivalence
 *
 * $Log: remconn.cxx,v $
 * Revision 1.8  1996/11/04 03:37:23  robertj
 * Added more debugging for remote drop outs.
 *
 * Revision 1.7  1996/10/31 12:39:53  robertj
 * Added RCS keywords.
 *
 */

#include <ptlib.h>
#include <remconn.h>


PDECLARE_CLASS(PRASDLL, PDynaLink)
  public:
    PRASDLL();

  DWORD (FAR PASCAL *Dial)(LPRASDIALEXTENSIONS,LPTSTR,LPRASDIALPARAMS,DWORD,LPVOID,LPHRASCONN);
  DWORD (FAR PASCAL *HangUp)(HRASCONN);
  DWORD (FAR PASCAL *GetConnectStatus)(HRASCONN,LPRASCONNSTATUS);
  DWORD (FAR PASCAL *EnumConnections)(LPRASCONN,LPDWORD,LPDWORD);
  DWORD (FAR PASCAL *EnumEntries)(LPTSTR,LPTSTR,LPRASENTRYNAME,LPDWORD,LPDWORD);
} Ras;

PRASDLL::PRASDLL()
#ifdef _WIN32
  : PDynaLink("RASAPI32.DLL")
#else
  : PDynaLink("RASAPI16.DLL")
#endif
{
  if (!GetFunction("RasDialA", (Function &)Dial) ||
      !GetFunction("RasHangUpA", (Function &)HangUp) ||
      !GetFunction("RasGetConnectStatusA", (Function &)GetConnectStatus) ||
      !GetFunction("RasEnumConnectionsA", (Function &)EnumConnections) ||
      !GetFunction("RasEnumEntriesA", (Function &)EnumEntries))
    Close();
}


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


BOOL PRemoteConnection::Open(const PString & name,
                             const PString & user,
                             const PString & pass)
{
  if (name != remoteName) {
    Close();
    remoteName = name;
  }
  userName = user;
  password = pass;
  return Open();
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
  PAssert(obj.IsDescendant(PRemoteConnection::Class()), PInvalidCast);
  return remoteName.Compare(((const PRemoteConnection &)obj).remoteName);
}


PINDEX PRemoteConnection::HashFunction() const
{
  return remoteName.HashFunction();
}


void PRemoteConnection::Construct()
{
  rasConnection = NULL;
  rasError = SUCCESS;
}


BOOL PRemoteConnection::Open()
{
  Close();
  if (!Ras.IsLoaded())
    return FALSE;

  RASCONN connection;
  connection.dwSize = sizeof(RASCONN);

  LPRASCONN connections = &connection;
  DWORD size = sizeof(connection);
  DWORD numConnections;

  rasError = Ras.EnumConnections(connections, &size, &numConnections);

  if (rasError == ERROR_BUFFER_TOO_SMALL) {
    connections = new RASCONN[size/sizeof(RASCONN)];
    connections[0].dwSize = sizeof(RASCONN);
    rasError = Ras.EnumConnections(connections, &size, &numConnections);
  }

  if (rasError == 0) {
    for (DWORD i = 0; i < numConnections; i++) {
      if (remoteName == connections[i].szEntryName) {
        rasConnection = connections[i].hrasconn;
        break;
      }
    }
  }

  if (connections != &connection)
    delete [] connections;

  if (rasConnection != NULL && GetStatus() == Connected) {
    rasError = 0;
    return TRUE;
  }
  rasConnection = NULL;

  RASDIALPARAMS params;
  memset(&params, 0, sizeof(params));
  params.dwSize = sizeof(params);
  if (remoteName[0] != '.') {
    PAssert(remoteName.GetLength() < sizeof(params.szEntryName)-1,
            PInvalidParameter);
    strcpy(params.szEntryName, remoteName);
  }
  else {
    PAssert(remoteName.GetLength() < sizeof(params.szPhoneNumber),
            PInvalidParameter);
    strcpy(params.szPhoneNumber, remoteName(1, P_MAX_INDEX));
  }
  strcpy(params.szUserName, userName);
  strcpy(params.szPassword, password);

  rasError = Ras.Dial(NULL, NULL, &params, 0, NULL, &rasConnection);
  if (rasError == 0)
    return TRUE;

  if (rasConnection != NULL) {
    Ras.HangUp(rasConnection);
    rasConnection = NULL;
  }

  SetLastError(rasError);
  return FALSE;
}


void PRemoteConnection::Close()
{
  if (rasConnection != NULL) {
    Ras.HangUp(rasConnection);
    rasConnection = NULL;
  }
}


PRemoteConnection::Status PRemoteConnection::GetStatus() const
{
  if (!Ras.IsLoaded())
    return NotInstalled;

  if (rasConnection == NULL) {
    switch (rasError) {
      case SUCCESS :
        return Idle;
      case ERROR_CANNOT_FIND_PHONEBOOK_ENTRY :
        return NoNameOrNumber;
      case ERROR_LINE_BUSY :
        return LineBusy;
      case ERROR_NO_DIALTONE :
        return NoDialTone;
      case ERROR_NO_ANSWER :
      case ERROR_NO_CARRIER :
        return NoAnswer;
      case ERROR_PORT_ALREADY_OPEN :
      case ERROR_PORT_NOT_AVAILABLE :
        return PortInUse;
    }
    return GeneralFailure;
  }

  RASCONNSTATUS status;
  status.dwSize = sizeof(status);
  DWORD err = Ras.GetConnectStatus(rasConnection, &status);
  if (err != 0) {
    char msg[100];
    sprintf(msg, "RAS Connection Lost: error %ld", err);
    PAssertAlways(msg);
    return ConnectionLost;
  }

  switch (status.rasconnstate) {
    case RASCS_Connected :
      return Connected;
    case RASCS_Disconnected :
      return Idle;
  }
  return InProgress;
}


PStringArray PRemoteConnection::GetAvailableNames()
{
  PStringArray array;
  if (!Ras.IsLoaded())
    return array;

  RASENTRYNAME entry;
  entry.dwSize = sizeof(RASENTRYNAME);

  LPRASENTRYNAME entries = &entry;
  DWORD size = sizeof(entry);
  DWORD numEntries;

  DWORD rasError = Ras.EnumEntries(NULL, NULL, entries, &size, &numEntries);

  if (rasError == ERROR_BUFFER_TOO_SMALL) {
    entries = new RASENTRYNAME[size/sizeof(RASENTRYNAME)];
    entries[0].dwSize = sizeof(RASENTRYNAME);
    rasError = Ras.EnumEntries(NULL, NULL, entries, &size, &numEntries);
  }

  if (rasError == 0) {
    array.SetSize(numEntries);
    for (DWORD i = 0; i < numEntries; i++)
      array[i] = entries[i].szEntryName;
  }

  if (entries != &entry)
    delete [] entries;

  return array;
}



// End of File ////////////////////////////////////////////////////////////////
