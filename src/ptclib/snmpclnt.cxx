/*
 * $Id: snmpclnt.cxx,v 1.4 1996/11/04 04:00:00 robertj Exp $
 *
 * SNMP Client Interface
 *
 * Copyright 1996 Equivalence
 *
 * $Log: snmpclnt.cxx,v $
 * Revision 1.4  1996/11/04 04:00:00  robertj
 * Added support for UDP packet truncation is reading SNMP reply.
 *
 * Revision 1.3  1996/10/08 13:06:24  robertj
 * Fixed SNMP timeout (GNU compatibility).
 *
 * Revision 1.2  1996/09/20 12:20:19  robertj
 * Used read timeout instead of member variable.
 *
 * Revision 1.1  1996/09/14 13:14:59  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <psnmp.h>


#define	SNMP_VERSION		0
#define	SNMP_PORT		"snmp 161"

static const char defaultCommunity[] = "public";


///////////////////////////////////////////////////////////////
//
//  PSNMPClient
//

PSNMPClient::PSNMPClient(PINDEX retry, PINDEX timeout,
                         PINDEX rxSize, PINDEX txSize)
 : community(defaultCommunity),
   version(SNMP_VERSION),
   retryMax(retry),
   maxRxSize(rxSize),
   maxTxSize(txSize)
{
  SetReadTimeout(PTimeInterval(0, timeout));
  requestId = rand() % 0x7fffffff;
}


PSNMPClient::PSNMPClient(const PString & host, PINDEX retry,
                         PINDEX timeout, PINDEX rxSize, PINDEX txSize)
 : hostName(host),
   community(defaultCommunity),
   version(SNMP_VERSION),
   retryMax(retry),
   maxRxSize(rxSize),
   maxTxSize(txSize)
{
  SetReadTimeout(PTimeInterval(0, timeout));
  Open(PNEW PUDPSocket(host, SNMP_PORT));
  requestId = rand() % 0x7fffffff;
}


void PSNMPClient::SetVersion(PASNInt newVersion)
{
  version = newVersion;
}


PASNInt PSNMPClient::GetVersion() const
{
  return version;
}


void PSNMPClient::SetCommunity(const PString & str)
{
  community = str;
}


PString PSNMPClient::GetCommunity() const
{
  return community;
}


void PSNMPClient::SetRequestID(PASNInt newRequestID)
{
  requestId = newRequestID;
}


PASNInt PSNMPClient::GetRequestID() const
{
  return requestId;
}


BOOL PSNMPClient::WriteGetRequest(PSNMPVarBindingList & varsIn,
                                  PSNMPVarBindingList & varsOut)
{
  return WriteRequest(GetRequest, varsIn, varsOut);
}


BOOL PSNMPClient::WriteGetNextRequest(PSNMPVarBindingList & varsIn,
                                      PSNMPVarBindingList & varsOut)
{
  return WriteRequest(GetNextRequest, varsIn, varsOut);
}


BOOL PSNMPClient::WriteSetRequest(PSNMPVarBindingList & varsIn,
                                  PSNMPVarBindingList & varsOut)
{
  return WriteRequest(SetRequest, varsIn, varsOut);
}


PSNMP::ErrorType PSNMPClient::GetLastErrorCode() const
{
  return lastErrorCode;
}


PINDEX PSNMPClient::GetLastErrorIndex() const
{
  return lastErrorIndex;
}


PString PSNMPClient::GetLastErrorText() const
{
  return PSNMP::GetErrorText(lastErrorCode);
}

BOOL PSNMPClient::ReadRequest(PBYTEArray & readBuffer)
{
  readBuffer.SetSize(maxRxSize);
  PINDEX rxSize = 0;

  for (;;) {

    if (!Read(readBuffer.GetPointer()+rxSize, maxRxSize - rxSize)) {

      // if the buffer was too small, then we are receiving datagrams
      // and the datagram was too big
      if (PChannel::GetErrorCode() == PChannel::BufferTooSmall) 
        lastErrorCode = RxBufferTooSmall;
      else
        lastErrorCode = NoResponse;
      return FALSE;

    } else if ((rxSize + GetLastReadCount()) >= 10)
      break;

    else 
      rxSize += GetLastReadCount();
  }

  rxSize += GetLastReadCount();

  PINDEX hdrLen = 1;

  // if not a valid sequence header, then stop reading
  WORD len;
  if ((readBuffer[0] != 0x30) ||
      !PASNObject::DecodeASNLength(readBuffer, hdrLen, len)) {
    lastErrorCode = MalformedResponse;
    return FALSE;
  }

  // length of packet is length of header + length of data
  len += hdrLen;

  // return TRUE if we have the packet, else return FALSE
  if (len <= maxRxSize) 
    return TRUE;

  lastErrorCode = RxBufferTooSmall;
  return FALSE;

#if 0
  // and get a new data ptr
  if (maxRxSize < len) 
    readBuffer.SetSize(len);

  // read the remainder of the packet
  while (rxSize < len) {
    if (!Read(readBuffer.GetPointer()+rxSize, len - rxSize)) {
      lastErrorCode = NoResponse;
      return FALSE;
    }
    rxSize += GetLastReadCount();
  }
  return TRUE;
#endif
}

BOOL PSNMPClient::WriteRequest(PASNInt requestCode,
                               PSNMPVarBindingList & vars,
                               PSNMPVarBindingList & varsOut)
{
  PASNSequence pdu;
  PASNSequence * pduData     = PNEW PASNSequence((BYTE)requestCode);
  PASNSequence * bindingList = PNEW PASNSequence();

  lastErrorIndex = 0;

  // build a get request PDU
  pdu.AppendInteger(version);
  pdu.AppendString(community);
  pdu.Append(pduData);

  // build the PDU data
  PASNInt thisRequestId = requestId;
  requestId = rand() % 0x7fffffff;
  pduData->AppendInteger(thisRequestId);
  pduData->AppendInteger(0);           // error status
  pduData->AppendInteger(0);           // error index
  pduData->Append(bindingList);        // binding list

  // build the binding list
  PINDEX i;
  for (i = 0; i < vars.GetSize(); i++) {
    PASNSequence * binding = PNEW PASNSequence();
    bindingList->Append(binding);
    binding->AppendObjectID(vars.GetObjectID(i));
    binding->Append((PASNObject *)vars[i].Clone());
  }

  // encode the PDU into a buffer
  PBYTEArray sendBuffer;
  pdu.Encode(sendBuffer);

  if (sendBuffer.GetSize() > maxTxSize) {
    lastErrorCode = TxDataTooBig;
    return FALSE;
  }

  varsOut.RemoveAll();

  PINDEX retry = retryMax;

  for (;;) {

    // send the packet
    if (!Write(sendBuffer, sendBuffer.GetSize())) {
      lastErrorCode = SendFailed;
      return FALSE;
    }

    // receive a packet
    if (ReadRequest(readBuffer))
      break;
    else if (lastErrorCode != NoResponse)
      return FALSE;
    else
      retry--;
  }

  // parse the response
  PASNSequence response(readBuffer);
  PINDEX seqLen = response.GetSize();

  // check PDU
  if (seqLen != 3 ||
      response[0].GetType() != PASNObject::Integer ||
      response[1].GetType() != PASNObject::String ||
      response[2].GetType() != PASNObject::Choice) {
    lastErrorCode = MalformedResponse;
    return FALSE;
  }

  // check the PDU data
  const PASNSequence & rPduData = response[2].GetSequence();
  seqLen = rPduData.GetSize();
  if (seqLen != 4 ||
      rPduData.GetChoice()  != GetResponse ||
      rPduData[0].GetType() != PASNObject::Integer ||
      rPduData[1].GetType() != PASNObject::Integer ||
      rPduData[2].GetType() != PASNObject::Integer ||
      rPduData[3].GetType() != PASNObject::Sequence) {
    lastErrorCode = MalformedResponse;
    return FALSE;
  }

  // check the request ID
  PASNInt returnedRequestId = rPduData[0].GetInteger();
  if (returnedRequestId != thisRequestId) {
    lastErrorCode = MalformedResponse;
    return FALSE;
  }
  
  // check the error status and return if non-zero
  PASNInt errorStatus = rPduData[1].GetInteger();
  if (errorStatus != 0) {
    lastErrorIndex = rPduData[2].GetInteger(); 
    lastErrorCode = (ErrorType)errorStatus;
    return FALSE;
  }

  // check the variable bindings
  const PASNSequence & rBindings = rPduData[3].GetSequence();
  PINDEX bindingCount = rBindings.GetSize();

  // create the return list
  for (i = 0; i < bindingCount; i++) {
    if (rBindings[i].GetType() != PASNObject::Sequence) {
      lastErrorIndex = i+1;
      lastErrorCode  = MalformedResponse;
      return FALSE;
    }
    const PASNSequence & rVar = rBindings[i].GetSequence();
    if (rVar.GetSize() != 2 ||
        rVar[0].GetType() != PASNObject::ObjectID) {
      lastErrorIndex = i+1;
      lastErrorCode = MalformedResponse;
      return FALSE;
    }
    varsOut.Append(rVar[0].GetString(), (PASNObject *)rVar[1].Clone());
  }

  lastErrorCode = NoError;
  return TRUE;
}



///////////////////////////////////////////////////////////////
//
//  Trap routines
//

void PSNMP::SendEnterpriseTrap (
                 const PIPSocket::Address & addr,
                            const PString & community,
                            const PString & enterprise,
                                     PINDEX specificTrap,
                               PASNUnsigned timeTicks,
                                       WORD sendPort)
{
  PSNMPVarBindingList vars;
  SendTrap(addr,
           EnterpriseSpecific,
           community,
           enterprise,
           specificTrap,
           timeTicks,
           vars,
           sendPort);
}


void PSNMP::SendEnterpriseTrap (
                 const PIPSocket::Address & addr,
                            const PString & community,
                            const PString & enterprise,
                                     PINDEX specificTrap,
                               PASNUnsigned timeTicks,
                const PSNMPVarBindingList & vars,
                                       WORD sendPort)
{
  SendTrap(addr,
           EnterpriseSpecific,
           community,
           enterprise,
           specificTrap,
           timeTicks,
           vars,
           sendPort);
}


void PSNMP::SendTrap(const PIPSocket::Address & addr,
                                PSNMP::TrapType trapType,
                                const PString & community,
                                const PString & enterprise,
                                         PINDEX specificTrap,
                                   PASNUnsigned timeTicks,
                    const PSNMPVarBindingList & vars,
                                           WORD sendPort)
{
  PIPSocket::Address agentAddress;
  PIPSocket::GetHostAddress(agentAddress);
  SendTrap(addr,
           trapType,
           community,
           enterprise,
           specificTrap,
           timeTicks,
           vars,
           agentAddress,
           sendPort);
}
                            

void PSNMP::SendTrap(const PIPSocket::Address & addr,
                                PSNMP::TrapType trapType,
                                const PString & community,
                                const PString & enterprise,
                                         PINDEX specificTrap,
                                   PASNUnsigned timeTicks,
                    const PSNMPVarBindingList & vars,
                     const PIPSocket::Address & agentAddress,
                                           WORD sendPort)
                            
{
  // send the trap to specified remote host
  PUDPSocket socket(addr, sendPort);
  if (socket.IsOpen())
    WriteTrap(socket, trapType, community, enterprise,
              specificTrap, timeTicks, vars, agentAddress);
}

void PSNMP::WriteTrap(                 PChannel & channel,
                                  PSNMP::TrapType trapType,
                                  const PString & community,
                                  const PString & enterprise,
                                           PINDEX specificTrap,
                                     PASNUnsigned timeTicks,
                      const PSNMPVarBindingList & vars,
                       const PIPSocket::Address & agentAddress)
{
  PASNSequence pdu;
  PASNSequence * pduData     = PNEW PASNSequence((BYTE)Trap);
  PASNSequence * bindingList = PNEW PASNSequence();

  // build a trap PDU PDU
  pdu.AppendInteger(1);
  pdu.AppendString(community);
  pdu.Append(pduData);

  // build the PDU data
  pduData->AppendObjectID(enterprise);               // enterprise
  pduData->Append(PNEW PASNIPAddress(agentAddress)); // agent address
  pduData->AppendInteger((int)trapType);             // trap type
  pduData->AppendInteger(specificTrap);              // specific trap
  pduData->Append(PNEW PASNTimeTicks(timeTicks));    // time of event
  pduData->Append(bindingList);                      // binding list

  // build the binding list
  for (PINDEX i = 0; i < vars.GetSize(); i++) {
    PASNSequence * binding = PNEW PASNSequence();
    bindingList->Append(binding);
    binding->AppendObjectID(vars.GetObjectID(i));
    binding->Append((PASNObject *)vars[i].Clone());
  }

  // encode the PDU into a buffer
  PBYTEArray sendBuffer;
  pdu.Encode(sendBuffer);

  // send the trap to specified remote host
  channel.Write(sendBuffer, sendBuffer.GetSize());
}


// End Of File ///////////////////////////////////////////////////////////////
