/*
 * RemConn.CXX
 *
 * Simple proxy service for internet access under Windows NT.
 *
 * Copyright 1995 Equivalence
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

  RASCONN connection;
  connection.dwSize = sizeof(RASCONN);

  LPRASCONN connections = &connection;
  DWORD size = sizeof(connection);
  DWORD numConnections;

  rasError = RasEnumConnections(connections, &size, &numConnections);

  if (rasError == ERROR_BUFFER_TOO_SMALL) {
    connections = new RASCONN[size/sizeof(RASCONN)];
    connections[0].dwSize = sizeof(RASCONN);
    rasError = RasEnumConnections(connections, &size, &numConnections);
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

  rasError = RasDial(NULL, NULL, &params, 0, NULL, &rasConnection);
  if (rasError == 0)
    return TRUE;

  if (rasConnection != NULL) {
    RasHangUp(rasConnection);
    rasConnection = NULL;
  }

  SetLastError(rasError);
  return FALSE;
}


void PRemoteConnection::Close()
{
  if (rasConnection != NULL) {
    RasHangUp(rasConnection);
    rasConnection = NULL;
  }
}


PRemoteConnection::Status PRemoteConnection::GetStatus() const
{
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
  }
  else {
    RASCONNSTATUS status;
    status.dwSize = sizeof(status);
    if (RasGetConnectStatus(rasConnection, &status) == 0) {
      switch (status.rasconnstate) {
        case RASCS_Connected :
          return Connected;
        case RASCS_Disconnected :
          return Idle;
      }
      return InProgress;
    }
  }
  return GeneralFailure;
}


PStringArray PRemoteConnection::GetAvailableNames()
{
  RASENTRYNAME entry;
  entry.dwSize = sizeof(RASENTRYNAME);

  LPRASENTRYNAME entries = &entry;
  DWORD size = sizeof(entry);
  DWORD numEntries;

  DWORD rasError = RasEnumEntries(NULL, NULL, entries, &size, &numEntries);

  if (rasError == ERROR_BUFFER_TOO_SMALL) {
    entries = new RASENTRYNAME[size/sizeof(RASENTRYNAME)];
    entries[0].dwSize = sizeof(RASENTRYNAME);
    rasError = RasEnumEntries(NULL, NULL, entries, &size, &numEntries);
  }

  PStringArray array;
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
