/*
 * snmpserv.cxx
 *
 * SNMP Server (agent) class
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
 * Copyright (c) 2007 ISVO(Asia) Pte. Ltd.
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
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#ifdef P_SNMP

#include <ptclib/psnmp.h>

#define new PNEW


#define SNMP_VERSION 0

static const char defaultCommunity[] = "public";

PSNMPServer::PSNMPServer(PIPSocket::Address binding, WORD localPort, PINDEX timeout, PINDEX rxSize, PINDEX txSize)
 : PThread(1000,AutoDeleteThread),
   community(defaultCommunity),
   version(SNMP_VERSION),
   maxRxSize(rxSize),
   maxTxSize(txSize)
{
  SetReadTimeout(PTimeInterval(0, timeout));

  baseSocket = new PUDPSocket;
  
  if (!baseSocket->Listen(binding, 0, localPort)) {
	  PTRACE(4,"SNMPsrv\tError: Unable to Listen on port " << localPort);
  } else {
      Open(baseSocket);
	  Resume();
  }
}

void PSNMPServer::Main()
{
   if (!HandleChannel())
	     Close();
}

PSNMPServer::~PSNMPServer()
{
	Close();
}

PBoolean PSNMPServer::HandleChannel()
{

  PBYTEArray readBuffer;
  PBYTEArray sendBuffer;

  for (;;) {
   	if (!IsOpen())
	  return PFalse;

		// Reaading
	    PINDEX rxSize = 0;
		for (;;) {
			readBuffer.SetSize(maxRxSize);

			if (!Read(readBuffer.GetPointer()+rxSize, maxRxSize - rxSize)) {

			// if the buffer was too small, then we are receiving datagrams
			// and the datagram was too big
			if (PChannel::GetErrorCode() == PChannel::BufferTooSmall) 
				lastErrorCode = RxBufferTooSmall;
			else
				lastErrorCode = NoResponse;

			PTRACE(4,"SNMPsrv\tRead Error " << lastErrorCode);
			return PFalse;

			} else if ((rxSize + GetLastReadCount()) >= 10)
			break;

			else 
			rxSize += GetLastReadCount();
		}

	     rxSize += GetLastReadCount();

		PIPSocket::Address remoteAddress;
		WORD remotePort;
		baseSocket->GetLastReceiveAddress(remoteAddress, remotePort);

		if (!Authorise(remoteAddress)) {
		  PTRACE(4,"SNMPsrv\tReceived UnAuthorized Message from IP " << remoteAddress);
		  continue;
		} 
	      
		// process the request
		if (ProcessPDU(readBuffer, sendBuffer) >= 0) {
			// send the packet
			baseSocket->SetSendAddress(remoteAddress, remotePort);
			if (!Write(sendBuffer, sendBuffer.GetSize())) {
			    PTRACE(4,"SNMPsrv\tWrite Error.");
			    continue;
			}
		}
  }

}


PBoolean PSNMPServer::Authorise(const PIPSocket::Address & /*received*/)
{
  return PTrue;
}


void PSNMPServer::SetVersion(PASNInt newVersion)
{
  version = newVersion;
}



PSNMP::ErrorType PSNMPServer::SendGetResponse (PSNMPVarBindingList &)
{
  PAssertAlways("SendGetResponse not yet implemented");
  return GenErr;
}


PBoolean PSNMPServer::OnGetRequest (PINDEX , PSNMP::BindingList &, PSNMP::ErrorType &)
{
	return PFalse;
}


PBoolean PSNMPServer::OnGetNextRequest (PINDEX , PSNMP::BindingList &, PSNMP::ErrorType &)
{
	return PFalse;
}


PBoolean PSNMPServer::OnSetRequest (PINDEX , PSNMP::BindingList &,PSNMP::ErrorType &)
{
	return PFalse;
}


template <typename PDUType>
static void DecodeOID(const PDUType & pdu, PINDEX & reqID, PSNMP::BindingList & varlist)
{
   reqID = pdu.m_request_id;
   const PSNMP_VarBindList & vars = pdu.m_variable_bindings;

  // create the Requested list
  for (PINDEX i = 0; i < vars.GetSize(); i++) {
    varlist.push_back(pair<PString,PRFC1155_ObjectSyntax>(vars[i].m_name.AsString(),vars[i].m_value));
  }
}

template <typename PDUType>
static void EncodeOID(PDUType & pdu, const PINDEX & reqID, 
					  const PSNMP::BindingList & varlist, 
					  const PSNMP::ErrorType & errCode)
{
   pdu.m_request_id = reqID;
   pdu.m_error_status = errCode;
   pdu.m_error_index = 0;

   if (errCode == PSNMP::NoError) {
	   // Build the response list
		PSNMP_VarBindList & vars = pdu.m_variable_bindings;
		PINDEX i = 0;
		vars.SetSize(varlist.size());
		PSNMP::BindingList::const_iterator Iter = varlist.begin();
		while (Iter != varlist.end()) {
		  vars[i].m_name.SetValue(Iter->first);
		  vars[i].m_value = Iter->second;
		  i++;
		  ++Iter;
		}
   }
}


int PSNMPServer::ProcessPDU(const PBYTEArray & readBuffer, PBYTEArray & sendBuffer)
{

  PSNMP_PDUs pdu;
  if (!pdu.Decode((PASN_Stream &)readBuffer)) {
	   PTRACE(4,"SNMPsrv\tERROR DECODING PDU");
	  return -1;
  }


  PSNMP::BindingList varlist;
  PINDEX reqID;

  PBoolean success = PTrue;
  PSNMP::ErrorType errCode = PSNMP::NoError;
  switch (pdu.GetTag()) {
    case PSNMP_PDUs::e_get_request:
      DecodeOID<PSNMP_GetRequest_PDU>(pdu,reqID,varlist);
      success = OnGetRequest(reqID,varlist,errCode);
      break;
    case PSNMP_PDUs::e_get_next_request:
      DecodeOID<PSNMP_GetNextRequest_PDU>(pdu,reqID,varlist);
      success = OnGetNextRequest(reqID,varlist,errCode);
      break;

    case PSNMP_PDUs::e_set_request:
      DecodeOID<PSNMP_SetRequest_PDU>(pdu,reqID,varlist);
      success = OnSetRequest(reqID,varlist,errCode);
      break;

    case PSNMP_PDUs::e_get_response:
    case PSNMP_PDUs::e_trap:
    default:
      PTRACE(4,"SNMPsrv\tSNMP Request/Response not supported");
      errCode= PSNMP::GenErr;
      success = PFalse;
  }

  // Write the varlist
  if (success) {
    PSNMP_PDUs sendpdu;
    sendpdu.SetTag(PSNMP_PDUs::e_get_response);
    EncodeOID<PSNMP_GetResponse_PDU>(sendpdu, reqID,varlist,errCode);
    sendpdu.Encode((PASN_Stream &)sendBuffer);
  }

  return success;
}

#endif

// End Of File ///////////////////////////////////////////////////////////////
