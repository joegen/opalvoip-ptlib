/*
 * $Id: ethsock.cxx,v 1.1 1998/08/20 06:04:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: ethsock.cxx,v $
 * Revision 1.1  1998/08/20 06:04:52  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>

#include <winioctl.h>
#include <winsvc.h>
#include <snmp.h>


#pragma warning(disable:4201 4245 4514)
#include "ndis.h"


static const char PacketServiceName[] = "PACKET";


///////////////////////////////////////////////////////////////////////////////

class PWin32AsnAny : public AsnAny
{
  public:
    PWin32AsnAny();
    PWin32AsnAny(const AsnAny & any)  { memcpy(this, &any, sizeof(*this)); }
    ~PWin32AsnAny();
};


///////////////////////////////////////////////////////////////////////////////

class PWin32AsnOid : public AsnObjectIdentifier
{
  public:
    PWin32AsnOid();
    PWin32AsnOid(const char * str);
    PWin32AsnOid(const AsnObjectIdentifier & oid);
    PWin32AsnOid(const PWin32AsnOid & oid)              { SnmpUtilOidCpy(this, (AsnObjectIdentifier *)&oid); }
    void Free()                                         { SnmpUtilOidFree(this); }
    PWin32AsnOid & operator=(const AsnObjectIdentifier&);
    PWin32AsnOid & operator=(const PWin32AsnOid & oid)  { SnmpUtilOidCpy(this, (AsnObjectIdentifier *)&oid); return *this; }
    PWin32AsnOid & operator+=(const PWin32AsnOid & oid) { SnmpUtilOidAppend(this, (AsnObjectIdentifier *)&oid); return *this; }
    UINT & operator[](int idx)                          { return ids[idx]; }
    UINT   operator[](int idx) const                    { return ids[idx]; }
    bool operator==(const PWin32AsnOid & oid)           { return SnmpUtilOidCmp(this, (AsnObjectIdentifier *)&oid) == 0; }
    bool operator!=(const PWin32AsnOid & oid)           { return SnmpUtilOidCmp(this, (AsnObjectIdentifier *)&oid) != 0; }
    bool operator< (const PWin32AsnOid & oid)           { return SnmpUtilOidCmp(this, (AsnObjectIdentifier *)&oid) <  0; }
    bool operator<=(const PWin32AsnOid & oid)           { return SnmpUtilOidCmp(this, (AsnObjectIdentifier *)&oid) <= 0; }
    bool operator> (const PWin32AsnOid & oid)           { return SnmpUtilOidCmp(this, (AsnObjectIdentifier *)&oid) >  0; }
    bool operator>=(const PWin32AsnOid & oid)           { return SnmpUtilOidCmp(this, (AsnObjectIdentifier *)&oid) >= 0; }
    bool operator*=(const PWin32AsnOid & oid)           { return SnmpUtilOidNCmp(this, (AsnObjectIdentifier *)&oid, idLength) == 0; }
};


/////////////////////////////////////////////////////////////////////////////

class PWin32SnmpLibrary : public PDynaLink
{
  PCLASSINFO(PWin32SnmpLibrary, PDynaLink)
  public:
    PWin32SnmpLibrary();

    BOOL GetOid(AsnObjectIdentifier & oid, AsnInteger & value);
    BOOL GetOid(AsnObjectIdentifier & oid, in_addr & ip_address);
    BOOL GetOid(AsnObjectIdentifier & oid, void * value, UINT valSize);
    BOOL GetOid(AsnObjectIdentifier & oid, AsnAny & value);

    BOOL GetNextOid(AsnObjectIdentifier & oid, AsnAny & value);

  private:
    PFNSNMPEXTENSIONINIT Init;
    PFNSNMPEXTENSIONQUERY Query;

    HANDLE hEvent;
    AsnObjectIdentifier baseOid;
};


///////////////////////////////////////////////////////////////////////////////

class PWin32OidBuffer
{
  public:
    PWin32OidBuffer(UINT oid, UINT len, const BYTE * data = NULL);
    ~PWin32OidBuffer() { delete buffer; }

    operator void *()         { return buffer; }
    operator DWORD ()         { return size; }
    DWORD operator [](int i)  { return buffer[i]; }

    void Move(BYTE * data, DWORD received);

  private:
    DWORD * buffer;
    UINT size;
};


///////////////////////////////////////////////////////////////////////////////

class PWin32PacketDriver
{
  public:
    virtual ~PWin32PacketDriver();

    BOOL IsOpen() const;
    void Close();
    DWORD GetLastError() const;

    virtual BOOL EnumInterfaces(PINDEX idx, PString & name) = 0;
    virtual BOOL BindInterface(const PString & interfaceName) = 0;

    virtual BOOL BeginRead(void * buf, DWORD size, DWORD & received, PWin32Overlapped & overlap) = 0;
    virtual BOOL BeginWrite(const void * buf, DWORD len, PWin32Overlapped & overlap) = 0;
    BOOL CompleteIO(DWORD & received, PWin32Overlapped & overlap);

    BOOL IoControl(UINT func,
                   const void * input, DWORD inSize,
                   void * output, DWORD outSize,
                   DWORD & received);

    BOOL QueryOid(UINT oid, DWORD & data);
    BOOL QueryOid(UINT oid, UINT len, BYTE * data);
    BOOL SetOid(UINT oid, DWORD data);
    BOOL SetOid(UINT oid, UINT len, const BYTE * data);

  protected:
    PWin32PacketDriver();

    DWORD dwError;
    HANDLE hDriver;
};


///////////////////////////////////////////////////////////////////////////////

class PWin32PacketVxD : public PWin32PacketDriver
{
  public:
    virtual BOOL EnumInterfaces(PINDEX idx, PString & name);
    virtual BOOL BindInterface(const PString & interfaceName);

    virtual BOOL BeginRead(void * buf, DWORD size, DWORD & received, PWin32Overlapped & overlap);
    virtual BOOL BeginWrite(const void * buf, DWORD len, PWin32Overlapped & overlap);
};


///////////////////////////////////////////////////////////////////////////////

class PWin32PacketSYS : public PWin32PacketDriver
{
  public:
    PWin32PacketSYS();

    virtual BOOL EnumInterfaces(PINDEX idx, PString & name);
    virtual BOOL BindInterface(const PString & interfaceName);

    virtual BOOL BeginRead(void * buf, DWORD size, DWORD & received, PWin32Overlapped & overlap);
    virtual BOOL BeginWrite(const void * buf, DWORD len, PWin32Overlapped & overlap);
};


#define FILE_DEVICE_PROTOCOL        0x8000

#define IOCTL_PROTOCOL_QUERY_OID    CTL_CODE(FILE_DEVICE_PROTOCOL, 0 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_SET_OID      CTL_CODE(FILE_DEVICE_PROTOCOL, 1 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_STATISTICS   CTL_CODE(FILE_DEVICE_PROTOCOL, 2 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_RESET        CTL_CODE(FILE_DEVICE_PROTOCOL, 3 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_READ         CTL_CODE(FILE_DEVICE_PROTOCOL, 4 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_WRITE        CTL_CODE(FILE_DEVICE_PROTOCOL, 5 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_MACNAME      CTL_CODE(FILE_DEVICE_PROTOCOL, 6 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_BIND         CTL_CODE(FILE_DEVICE_PROTOCOL, 7 , METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTOCOL_UNBIND       CTL_CODE(FILE_DEVICE_PROTOCOL, 8 , METHOD_BUFFERED, FILE_ANY_ACCESS)


/////////////////////////////////////////////////////////////////////////////

class PWin32PacketBuffer
{
  public:
    enum Statuses {
      Uninitialised,
      Progressing,
      Completed
    };

    PWin32PacketBuffer();

    void SetSize(PINDEX sz);
    PINDEX GetData(void * buf, PINDEX size);
    PINDEX PutData(const void * buf, PINDEX length);
    HANDLE GetEvent() const { return overlap.hEvent; }

    BOOL ReadAsync(PWin32PacketDriver & pkt);
    BOOL ReadComplete(PWin32PacketDriver & pkt);
    BOOL WriteAsync(PWin32PacketDriver & pkt);
    BOOL WriteComplete(PWin32PacketDriver & pkt);

    BOOL InProgress() const { return status == Progressing; }
    BOOL IsCompleted() const { return status == Completed; }

  protected:
    Statuses         status;
    PWin32Overlapped overlap;
    PBYTEArray       data;
    DWORD            count;
};


/////////////////////////////////////////////////////////////////////////////

PWin32AsnAny::PWin32AsnAny()
{
  asnType = ASN_INTEGER;
  asnValue.number = 0;
}


PWin32AsnAny::~PWin32AsnAny()
{
  switch (asnType) {
    case ASN_OCTETSTRING :
      SnmpUtilMemFree(asnValue.string.stream);
      break;
    case ASN_BITS :
      SnmpUtilMemFree(asnValue.bits.stream);
      break;
    case ASN_OBJECTIDENTIFIER :
      SnmpUtilMemFree(asnValue.object.ids);
      break;
    case ASN_SEQUENCE :
      SnmpUtilMemFree(asnValue.sequence.stream);
      break;
    case ASN_IPADDRESS :
      SnmpUtilMemFree(asnValue.address.stream);
      break;
    case ASN_OPAQUE :
      SnmpUtilMemFree(asnValue.arbitrary.stream);
      break;
  }
}


///////////////////////////////////////////////////////////////////////////////

PWin32AsnOid::PWin32AsnOid()
{
  ids = NULL;
  idLength = 0;
}


PWin32AsnOid::PWin32AsnOid(const AsnObjectIdentifier & oid)
{
  ids = oid.ids;
  idLength = oid.idLength;
}


PWin32AsnOid::PWin32AsnOid(const char * str)
{
  idLength = 0;
  ids = NULL;

  AsnObjectIdentifier oid;
  oid.idLength = 0;
  const char * dot = strchr(str, '.');
  while (dot != NULL) {
    oid.idLength++;
    dot = strchr(dot+1, '.');
  }

  if (oid.idLength > 0) {
    oid.ids = new UINT[++oid.idLength];
    char * next = (char *)str;
    for (UINT i = 0; i < oid.idLength; i++) {
      oid.ids[i] = strtoul(next, &next, 10);
      if (*next != '.')
        break;
      next++;
    }

    if (*next == '\0')
      SnmpUtilOidCpy(this, &oid);

    delete [] oid.ids;
  }
}


PWin32AsnOid & PWin32AsnOid::operator=(const AsnObjectIdentifier & oid)
{
  ids = oid.ids;
  idLength = oid.idLength;
  return *this;
}


///////////////////////////////////////////////////////////////////////////////

PWin32SnmpLibrary::PWin32SnmpLibrary()
  : PDynaLink("inetmib1.dll")
{
  if (!GetFunction("SnmpExtensionInit", (Function &)Init) ||
      !GetFunction("SnmpExtensionQuery", (Function &)Query) ||
      !Init(0, &hEvent, &baseOid))
    Close();
}


BOOL PWin32SnmpLibrary::GetOid(AsnObjectIdentifier & oid, AsnInteger & value)
{
  if (!IsLoaded())
    return FALSE;

  PWin32AsnAny any;
  if (!GetOid(oid, any))
    return FALSE;

  if (any.asnType != ASN_INTEGER)
    return FALSE;

  value = any.asnValue.number;
  return TRUE;
}


BOOL PWin32SnmpLibrary::GetOid(AsnObjectIdentifier & oid, in_addr & value)
{
  if (!IsLoaded())
    return FALSE;

  PWin32AsnAny any;
  if (!GetOid(oid, any))
    return FALSE;

  if (any.asnType != ASN_IPADDRESS)
    return FALSE;

  memcpy(&value, any.asnValue.address.stream, sizeof(value));
  return TRUE;
}


BOOL PWin32SnmpLibrary::GetOid(AsnObjectIdentifier & oid, void * value, UINT valSize)
{
  if (!IsLoaded())
    return FALSE;

  PWin32AsnAny any;
  if (!GetOid(oid, any))
    return FALSE;

  if (any.asnType != ASN_OCTETSTRING)
    return FALSE;

  if (any.asnValue.string.length > valSize)
    return FALSE;

  memcpy(value, any.asnValue.string.stream, any.asnValue.string.length);
  if (valSize > any.asnValue.string.length)
    ((char *)value)[any.asnValue.string.length] = '\0';
  return TRUE;
}


BOOL PWin32SnmpLibrary::GetOid(AsnObjectIdentifier & oid, AsnAny & value)
{
  if (!IsLoaded())
    return FALSE;

  RFC1157VarBind var;
  var.name = oid;
  var.value = value;

  RFC1157VarBindList vars;
  vars.len = 1;
  vars.list = &var;

  AsnInteger status, error;
  if (!Query(SNMP_PDU_GET, &vars, &status, &error))
    return FALSE;

  if (status != SNMP_ERRORSTATUS_NOERROR)
    return FALSE;

  value = var.value;
  oid = var.name;
  return TRUE;
}


BOOL PWin32SnmpLibrary::GetNextOid(AsnObjectIdentifier & oid, AsnAny & value)
{
  if (!IsLoaded())
    return FALSE;

  SnmpVarBind var;
  var.name = oid;
  var.value = value;

  SnmpVarBindList vars;
  vars.len = 1;
  vars.list = &var;

  AsnInteger status, error;
  if (!Query(SNMP_PDU_GETNEXT, &vars, &status, &error))
    return FALSE;

  if (status != SNMP_ERRORSTATUS_NOERROR)
    return FALSE;

  value = var.value;
  oid = var.name;
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////

PWin32OidBuffer::PWin32OidBuffer(UINT oid, UINT len, const BYTE * data)
{
  size = sizeof(DWORD)*2 + len;
  buffer = new DWORD[(size+sizeof(DWORD)-1)/sizeof(DWORD)];

  buffer[0] = oid;
  buffer[1] = len;
  if (data != NULL)
    memcpy(&buffer[2], data, len);
}


void PWin32OidBuffer::Move(BYTE * data, DWORD received)
{
  memcpy(data, &buffer[2], received-sizeof(DWORD)*2);
}


///////////////////////////////////////////////////////////////////////////////

PWin32PacketDriver::PWin32PacketDriver()
{
  hDriver = INVALID_HANDLE_VALUE;
  dwError = ERROR_OPEN_FAILED;
}


PWin32PacketDriver::~PWin32PacketDriver()
{
  Close();
}


void PWin32PacketDriver::Close()
{
  if (hDriver != INVALID_HANDLE_VALUE) {
    CloseHandle(hDriver);
    hDriver = INVALID_HANDLE_VALUE;
  }
}


BOOL PWin32PacketDriver::IsOpen() const
{
  return hDriver != INVALID_HANDLE_VALUE;
}


DWORD PWin32PacketDriver::GetLastError() const
{
  return dwError;
}


BOOL PWin32PacketDriver::IoControl(UINT func,
                              const void * input, DWORD inSize,
                              void * output, DWORD outSize, DWORD & received)
{
  PWin32Overlapped overlap;

  if (DeviceIoControl(hDriver, func,
                      (LPVOID)input, inSize, output, outSize,
                      &received, &overlap)) {
    dwError = ERROR_SUCCESS;
    return TRUE;
  }

  dwError = ::GetLastError();
  if (dwError != ERROR_IO_PENDING)
    return FALSE;

  return CompleteIO(received, overlap);
}


BOOL PWin32PacketDriver::CompleteIO(DWORD & received, PWin32Overlapped & overlap)
{
  received = 0;
  if (GetOverlappedResult(hDriver, &overlap, &received, TRUE)) {
    dwError = ERROR_SUCCESS;
    return TRUE;
  }

  dwError = ::GetLastError();
  return FALSE;
}


BOOL PWin32PacketDriver::QueryOid(UINT oid, UINT len, BYTE * data)
{
  PWin32OidBuffer buf(oid, len);
  DWORD rxsize;
  if (!IoControl(oid >= 0x01000000 ? IOCTL_PROTOCOL_QUERY_OID : IOCTL_PROTOCOL_STATISTICS,
                 buf, buf, buf, buf, rxsize))
    return FALSE;

  if (rxsize == 0)
    return FALSE;

  buf.Move(data, rxsize);
  return TRUE;
}


BOOL PWin32PacketDriver::QueryOid(UINT oid, DWORD & data)
{
  DWORD oidData[3];
  oidData[0] = oid;
  oidData[1] = sizeof(data);
  oidData[2] = 0x12345678;

  DWORD rxsize;
  if (!IoControl(oid >= 0x01000000 ? IOCTL_PROTOCOL_QUERY_OID : IOCTL_PROTOCOL_STATISTICS,
                 oidData, sizeof(oidData), oidData, sizeof(oidData), rxsize))
    return FALSE;

  if (rxsize == 0)
    return FALSE;

  data = oidData[2];
  return TRUE;
}


BOOL PWin32PacketDriver::SetOid(UINT oid, UINT len, const BYTE * data)
{
  DWORD rxsize;
  PWin32OidBuffer buf(oid, len, data);
  return IoControl(IOCTL_PROTOCOL_SET_OID, buf, buf, buf, buf, rxsize);
}


BOOL PWin32PacketDriver::SetOid(UINT oid, DWORD data)
{
  DWORD oidData[3];
  oidData[0] = oid;
  oidData[1] = sizeof(data);
  oidData[2] = data;
  DWORD rxsize;
  return IoControl(IOCTL_PROTOCOL_SET_OID,
                   oidData, sizeof(oidData), oidData, sizeof(oidData), rxsize);
}


///////////////////////////////////////////////////////////////////////////////

BOOL PWin32PacketVxD::EnumInterfaces(PINDEX idx, PString & name)
{
  static const PString RegBase = "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Class\\Net";

  PString keyName;
  RegistryKey registry(RegBase, RegistryKey::ReadOnly);
  if (!registry.EnumKey(idx, keyName))
    return FALSE;

  PString description;
  RegistryKey subkey(RegBase + "\\" + keyName, RegistryKey::ReadOnly);
  if (subkey.QueryValue("DriverDesc", description))
    name = keyName + ": " + description;
  else
    name = keyName;

  return TRUE;
}


BOOL PWin32PacketVxD::BindInterface(const PString & interfaceName)
{
  if (hDriver == INVALID_HANDLE_VALUE) {
    hDriver = CreateFile("\\\\.\\VPACKET.VXD",
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL |
                             FILE_FLAG_OVERLAPPED |
                             FILE_FLAG_DELETE_ON_CLOSE,
                         NULL);
    if (hDriver == INVALID_HANDLE_VALUE) {
      dwError = ::GetLastError();
      return FALSE;
    }
  }

  BYTE rxbuf[20];
  DWORD rxsize;
  if (!IoControl(IOCTL_PROTOCOL_BIND,
                 (const char *)interfaceName, interfaceName.GetLength()+1,
                 rxbuf, sizeof(rxbuf), rxsize)) {
    dwError = ::GetLastError();
    return FALSE;
  }

  rxsize = 0;
  if (!IoControl(IOCTL_PROTOCOL_MACNAME,
                 rxbuf, sizeof(rxbuf),
                 rxbuf, sizeof(rxbuf), rxsize)) {
    dwError = ::GetLastError();
    return FALSE;
  }

  if (rxsize == 0) {
    dwError = ERROR_PATH_NOT_FOUND;
    return FALSE;
  }

  dwError = ERROR_SUCCESS;
  return TRUE;
}


BOOL PWin32PacketVxD::BeginRead(void * buf, DWORD size, DWORD & received, PWin32Overlapped & overlap)
{
  received = 0;
  if (DeviceIoControl(hDriver, IOCTL_PROTOCOL_READ,
                      buf, size, buf, size, &received, &overlap)) {
    dwError = ERROR_SUCCESS;
    return TRUE;
  }

  dwError = ::GetLastError();
  return dwError == ERROR_IO_PENDING;
}


BOOL PWin32PacketVxD::BeginWrite(const void * buf, DWORD len, PWin32Overlapped & overlap)
{
  DWORD rxsize = 0;
  BYTE dummy[2];
  if (DeviceIoControl(hDriver, IOCTL_PROTOCOL_WRITE,
                      (void *)buf, len, dummy, sizeof(dummy), &rxsize, &overlap)) {
    dwError = ERROR_SUCCESS;
    return TRUE;
  }

  dwError = ::GetLastError();
  return dwError == ERROR_IO_PENDING;
}


///////////////////////////////////////////////////////////////////////////////

PWin32PacketSYS::PWin32PacketSYS()
{
  // Start the packet driver service
  SC_HANDLE hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (hManager != NULL) {
    HANDLE hService = OpenService(hManager, PacketServiceName, SERVICE_START);
    if (hService != NULL) {
      StartService(hService, 0, NULL);
      dwError = ::GetLastError();
      CloseServiceHandle(hService);
    }
    CloseServiceHandle(hManager);
  }
}


static const char DevicePacketStr[] = "\\Device\\Packet_";

BOOL PWin32PacketSYS::EnumInterfaces(PINDEX idx, PString & name)
{
  RegistryKey registry("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Packet\\Linkage",
                       RegistryKey::ReadOnly);
  PString exports;
  if (!registry.QueryValue("Export", exports))
    return FALSE;

  const char * ptr = exports;
  while (*ptr != '\0' && idx-- > 0)
    ptr += strlen(ptr)+1;

  if (*ptr == '\0') {
    dwError = ERROR_NO_MORE_ITEMS;
    return FALSE;
  }

  if (strncmp(ptr, DevicePacketStr, sizeof(DevicePacketStr)-1) == 0)
    ptr += sizeof(DevicePacketStr)-1;

  name = ptr;
  return TRUE;
}


BOOL PWin32PacketSYS::BindInterface(const PString & interfaceName)
{
  Close();

  if (!DefineDosDevice(DDD_RAW_TARGET_PATH,
                       "Packet_" + interfaceName,
                       DevicePacketStr + interfaceName)) {
    dwError = ::GetLastError();
    return FALSE;
  }

  hDriver = CreateFile("\\\\.\\Packet_" + interfaceName,
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_FLAG_OVERLAPPED,
                       NULL);
  if (hDriver == INVALID_HANDLE_VALUE) {
    dwError = ::GetLastError();
    return FALSE;
  }

  dwError = ERROR_SUCCESS;
  return TRUE;
}


BOOL PWin32PacketSYS::BeginRead(void * buf, DWORD size, DWORD & received, PWin32Overlapped & overlap)
{
  overlap.Reset();
  received = 0;

  if (ReadFile(hDriver, buf, size, &received, &overlap)) {
    dwError = ERROR_SUCCESS;
    return TRUE;
  }

  return (dwError = ::GetLastError()) == ERROR_IO_PENDING;
}


BOOL PWin32PacketSYS::BeginWrite(const void * buf, DWORD len, PWin32Overlapped & overlap)
{
  overlap.Reset();
  DWORD sent = 0;
  if (WriteFile(hDriver, buf, len, &sent, &overlap)) {
    dwError = ERROR_SUCCESS;
    return TRUE;
  }

  dwError = ::GetLastError();
  return dwError == ERROR_IO_PENDING;
}


///////////////////////////////////////////////////////////////////////////////

PEthSocket::Address::Address()
{
  memset(b, 0xff, sizeof(b));
}


PEthSocket::Address::Address(const BYTE * addr)
{
  memcpy(b, PAssertNULL(addr), sizeof(b));
}


PEthSocket::Address::Address(const Address & addr)
{
  l = addr.l;
  s = addr.s;
}


PEthSocket::Address::Address(const PString & str)
{
  operator=(str);
}


PEthSocket::Address & PEthSocket::Address::operator=(const Address & addr)
{
  l = addr.l;
  s = addr.s;
  return *this;
}


PEthSocket::Address & PEthSocket::Address::operator=(const PString & str)
{
  memset(b, 0, sizeof(b));

  int shift = 0;
  PINDEX byte = 5;
  PINDEX pos = str.GetLength();
  while (pos-- > 0) {
    int c = str[pos];
    if (c != '-') {
      if (isdigit(c))
        b[byte] |= (c - '0') << shift;
      else if (isxdigit(c))
        b[byte] |= (toupper(c) - 'A' + 10) << shift;
      else {
        memset(this, 0, sizeof(*this));
        return *this;
      }
      if (shift == 0)
        shift = 4;
      else {
        shift = 0;
        byte--;
      }
    }
  }

  return *this;
}


PEthSocket::Address::operator PString() const
{
  return psprintf("%02X-%02X-%02X-%02X-%02X-%02X", b[0], b[1], b[2], b[3], b[4], b[5]);
}


///////////////////////////////////////////////////////////////////////////////

static PWin32PacketDriver * CreatePacketDriver()
{
  OSVERSIONINFO info;
  info.dwOSVersionInfoSize = sizeof(info);
  GetVersionEx(&info);
  if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
    return new PWin32PacketSYS;
  else
    return new PWin32PacketVxD;
}


PEthSocket::PEthSocket(PINDEX nBuffers, PINDEX size)
{
  driver = CreatePacketDriver();
  snmp = new PWin32SnmpLibrary;

  numBuffers = min(nBuffers, MAXIMUM_WAIT_OBJECTS);
  buffers = new PWin32PacketBuffer[nBuffers];
  for (PINDEX i = 0; i < nBuffers; i++)
    buffers[i].SetSize(size);
}


PEthSocket::~PEthSocket()
{
  Close();

  delete [] buffers;
  delete driver;
  delete snmp;
}


BOOL PEthSocket::Close()
{
  driver->Close();
  os_handle = -1;
  return TRUE;
}


PString PEthSocket::GetName() const
{
  return interfaceName;
}


BOOL PEthSocket::Connect(const PString & interfaceName)
{
  Close();

  if (!driver->BindInterface(interfaceName))
    return FALSE;

  Address myAddr;
  if (!GetAddress(myAddr))
    return FALSE;

  os_handle = 1;
  for (;;) {
    Address itsAddr;
    PWin32AsnOid ifPhysAddressOid = "1.3.6.1.2.1.2.2.1.6.0";
    ifPhysAddressOid[ifPhysAddressOid.idLength-1] = os_handle;
    if (!snmp->GetOid(ifPhysAddressOid, &itsAddr, sizeof(itsAddr)))
      break;

    ifPhysAddressOid.Free();
    if (itsAddr == myAddr)
      return TRUE;

    os_handle++;
  }

  os_handle = -1;
  return FALSE;
}


BOOL PEthSocket::OpenSocket()
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


const char * PEthSocket::GetProtocolName() const
{
  return "eth";
}


BOOL PEthSocket::Listen(unsigned, WORD, Reusability)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


BOOL PEthSocket::EnumInterfaces(PINDEX idx, PString & name)
{
  return driver->EnumInterfaces(idx, name);
}


PString PEthSocket::GetGatewayInterface() const
{
  AsnInteger ifNum;
  PWin32AsnOid gatewayOid = "1.3.6.1.2.1.4.21.1.2.0.0.0.0";
  if (snmp->GetOid(gatewayOid, ifNum)) {
    gatewayOid.Free();

    Address gwAddr;
    PWin32AsnOid ifPhysAddressOid = "1.3.6.1.2.1.2.2.1.6.0";
    ifPhysAddressOid[ifPhysAddressOid.idLength-1] = ifNum;
    if (snmp->GetOid(ifPhysAddressOid, &gwAddr, sizeof(gwAddr))) {
      ifPhysAddressOid.Free();

      PWin32PacketDriver * tempDriver = CreatePacketDriver();
      PINDEX idx = 0;
      PString name;
      while (tempDriver->EnumInterfaces(idx++, name)) {
        if (tempDriver->BindInterface(name)) {
          Address ifAddr;
          if (tempDriver->BindInterface(name) &&
              tempDriver->QueryOid(OID_802_3_CURRENT_ADDRESS, sizeof(ifAddr), ifAddr.b) &&
              ifAddr == gwAddr) {
            delete tempDriver;
            return name;
          }
        }
      }
      delete tempDriver;
    }
  }
  return PString();
}


BOOL PEthSocket::GetAddress(Address & addr)
{
  return driver->QueryOid(OID_802_3_CURRENT_ADDRESS, sizeof(addr), addr.b);
}


BOOL PEthSocket::GetIpAddress(PIPSocket::Address & addr)
{
  PIPSocket::Address net_mask;
  return EnumIpAddress(0, addr, net_mask);
}


BOOL PEthSocket::GetIpAddress(PIPSocket::Address & addr, PIPSocket::Address & net_mask)
{
  return EnumIpAddress(0, addr, net_mask);
}


BOOL PEthSocket::EnumIpAddress(PINDEX idx,
                               PIPSocket::Address & addr,
                               PIPSocket::Address & net_mask)
{
  if (!IsOpen())
    return FALSE;

  AsnInteger ifNum = os_handle;
  PWin32AsnAny value;
  PWin32AsnOid baseOid = "1.3.6.1.2.1.4.20.1";
  PWin32AsnOid oid = baseOid;
  while (snmp->GetNextOid(oid, value)) {
    if (!(baseOid *= oid))
      return FALSE;
    if (value.asnType != ASN_IPADDRESS)
      return FALSE;

    oid[9] = 2;
    AsnInteger ifIndex;
    if (!snmp->GetOid(oid, ifIndex))
      return FALSE;

    if (ifIndex == ifNum && idx-- == 0) {
      memcpy(&addr, value.asnValue.address.stream, sizeof(addr));
      oid[9] = 3;
      if (snmp->GetOid(oid, net_mask))
        oid.Free();
      return TRUE;
    }
    oid[9] = 1;
  }

  return FALSE;
}


static const struct {
  unsigned pwlib;
  DWORD    ndis;
} FilterMasks[] = {
  { PEthSocket::FilterDirected,     NDIS_PACKET_TYPE_DIRECTED },
  { PEthSocket::FilterMulticast,    NDIS_PACKET_TYPE_MULTICAST },
  { PEthSocket::FilterAllMulticast, NDIS_PACKET_TYPE_ALL_MULTICAST },
  { PEthSocket::FilterBroadcast,    NDIS_PACKET_TYPE_BROADCAST },
  { PEthSocket::FilterPromiscuous,  NDIS_PACKET_TYPE_PROMISCUOUS }
};


unsigned PEthSocket::GetFilter()
{
  DWORD filter;
  if (!driver->QueryOid(OID_GEN_CURRENT_PACKET_FILTER, filter))
    return 0;

  unsigned bits = 0;
  for (PINDEX i = 0; i < PARRAYSIZE(FilterMasks); i++) {
    if ((filter&FilterMasks[i].ndis) != 0)
      bits |= FilterMasks[i].pwlib;
  }
  return bits;
}


BOOL PEthSocket::SetFilter(unsigned filter)
{
  DWORD bits = 0;
  for (PINDEX i = 0; i < PARRAYSIZE(FilterMasks); i++) {
    if ((filter&FilterMasks[i].pwlib) != 0)
      bits |= FilterMasks[i].ndis;
  }
  return driver->SetOid(OID_GEN_CURRENT_PACKET_FILTER, bits);
}


PEthSocket::MediumTypes PEthSocket::GetMedium()
{
  DWORD medium;
  if (!driver->QueryOid(OID_GEN_MEDIA_SUPPORTED, medium))
    return NumMediumTypes;

  static const DWORD MediumValues[NumMediumTypes] = {
    NdisMedium802_3, NdisMediumWan
  };

  for (int type = Medium802_3; type < NumMediumTypes; type++) {
    if (MediumValues[type] == medium)
      return (MediumTypes)type;
  }

  return MediumUnknown;
}


BOOL PEthSocket::ResetAdaptor()
{
  DWORD received;
  return driver->IoControl(IOCTL_PROTOCOL_RESET, NULL, 0, NULL, 0, received);
}


BOOL PEthSocket::Read(void * data, PINDEX length)
{
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];

  PINDEX idx;
  for (idx = 0; idx < numBuffers; idx++) {
    PWin32PacketBuffer & buffer = buffers[idx];
    if (!buffer.InProgress()) {
      if (!buffer.ReadAsync(*driver))
        return FALSE;
    }

    if (buffer.IsCompleted()) {
      lastReadCount = buffer.GetData(data, length);
      return TRUE;
    }
    handles[idx] = buffer.GetEvent();
  }

  DWORD result = WaitForMultipleObjects(numBuffers, handles, FALSE, INFINITE);
  if (result < WAIT_OBJECT_0 || result >= WAIT_OBJECT_0+numBuffers) {
    DWORD dwError = ::GetLastError();
    return FALSE;
  }

  idx = result - WAIT_OBJECT_0;
  if (!buffers[idx].ReadComplete(*driver))
    return FALSE;

  lastReadCount = buffers[idx].GetData(data, length);
  return TRUE;
}


BOOL PEthSocket::Write(const void * data, PINDEX length)
{
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];

  PINDEX idx;
  for (idx = 0; idx < numBuffers; idx++) {
    PWin32PacketBuffer & buffer = buffers[idx];
    if (buffer.InProgress()) {
      if (WaitForSingleObject(buffer.GetEvent(), 0) == WAIT_OBJECT_0)
        buffer.WriteComplete(*driver);
    }
    if (!buffer.InProgress()) {
      lastWriteCount = buffer.PutData(data, length);
      return buffer.WriteAsync(*driver);
    }
    handles[idx] = buffer.GetEvent();
  }

  DWORD result = WaitForMultipleObjects(numBuffers, handles, FALSE, INFINITE);
  if (result < WAIT_OBJECT_0 || result >= WAIT_OBJECT_0+numBuffers) {
    DWORD dwError = ::GetLastError();
    return FALSE;
  }

  idx = result - WAIT_OBJECT_0;
  if (!buffers[idx].WriteComplete(*driver))
    return FALSE;

  lastWriteCount = buffers[idx].PutData(data, length);
  return buffers[idx].WriteAsync(*driver);
}


///////////////////////////////////////////////////////////////////////////////

PWin32PacketBuffer::PWin32PacketBuffer()
{
  status = Uninitialised;
  count = 0;
}


void PWin32PacketBuffer::SetSize(PINDEX sz)
{
  data.SetSize(sz);
  count = 0;
}


PINDEX PWin32PacketBuffer::GetData(void * buf, PINDEX size)
{
  if (count > (DWORD)size)
    count = size;

  memcpy(buf, data, count);

  return count;
}


PINDEX PWin32PacketBuffer::PutData(const void * buf, PINDEX length)
{
  count = min(data.GetSize(), length);

  memcpy(data.GetPointer(), buf, count);

  return count;
}


BOOL PWin32PacketBuffer::ReadAsync(PWin32PacketDriver & pkt)
{
  if (status == Progressing)
    return FALSE;

  status = Uninitialised;
  if (!pkt.BeginRead(data.GetPointer(), data.GetSize(), count, overlap))
    return FALSE;

  if (pkt.GetLastError() == ERROR_SUCCESS)
    status = Completed;
  else
    status = Progressing;
  return TRUE;
}


BOOL PWin32PacketBuffer::ReadComplete(PWin32PacketDriver & pkt)
{
  if (status != Progressing)
    return status == Completed;

  if (!pkt.CompleteIO(count, overlap)) {
    status = Uninitialised;
    return FALSE;
  }

  status = Completed;
  return TRUE;
}


BOOL PWin32PacketBuffer::WriteAsync(PWin32PacketDriver & pkt)
{
  if (status == Progressing)
    return FALSE;

  status = Uninitialised;
  if (!pkt.BeginWrite(data, count, overlap))
    return FALSE;

  if (pkt.GetLastError() == ERROR_SUCCESS)
    status = Completed;
  else
    status = Progressing;
  return TRUE;
}


BOOL PWin32PacketBuffer::WriteComplete(PWin32PacketDriver & pkt)
{
  if (status != Progressing)
    return status == Completed;

  DWORD dummy;
  if (pkt.CompleteIO(dummy, overlap)) {
    status = Completed;
    return TRUE;
  }

  status = Uninitialised;
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
