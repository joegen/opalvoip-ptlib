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
 */

#include <ptlib.h>

#ifdef P_SNMP

#include <ptclib/psnmp.h>

#define new PNEW


#define SNMP_VERSION 0

static const char defaultCommunity[] = "public";

PSNMPServer::PSNMPServer(PIPSocket::Address binding, WORD localPort, PINDEX timeout, PINDEX rxSize, PINDEX txSize)
 : P_DISABLE_MSVC_WARNINGS(4355, m_thread(*this, &PSNMPServer::Main, true, "SNMP Server"))
 , m_community(defaultCommunity)
 , m_version(SNMP_VERSION)
 , m_maxRxSize(rxSize)
 , m_maxTxSize(txSize)
{
  SetReadTimeout(PTimeInterval(0, timeout));
  m_baseSocket = new PUDPSocket;

  if (!m_baseSocket->Listen(binding, 0, localPort)) {
    PTRACE(4,"SNMPsrv\tError: Unable to Listen on port " << localPort);
  }
  else {
    Open(m_baseSocket);
    m_thread.Resume();
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
  PBYTEArray sendBuffer(m_maxTxSize);


  for (;;) {
   	if (CheckNotOpen())
      return false;

		// Reading
	    PINDEX rxSize = 0;
		readBuffer.SetSize(m_maxRxSize);
		for (;;) {
			if (!Read(readBuffer.GetPointer()+rxSize, m_maxRxSize - rxSize)) {

			// if the buffer was too small, then we are receiving datagrams
			// and the datagram was too big
			if (PChannel::GetErrorCode() == PChannel::BufferTooSmall) 
				m_lastErrorCode = RxBufferTooSmall;
			else
				m_lastErrorCode = NoResponse;

			PTRACE(4,"SNMPsrv\tRenewing Socket due to timeout" << m_lastErrorCode);

			} else if ((rxSize + GetLastReadCount()) >= 10)
			break;

			else 
			rxSize += GetLastReadCount();
		}

	    rxSize += GetLastReadCount();
		readBuffer.SetSize(rxSize);

		PIPSocket::Address remoteAddress;
		WORD remotePort;
		m_baseSocket->GetLastReceiveAddress(remoteAddress, remotePort);

		if (!Authorise(remoteAddress)) {
		  PTRACE(4,"SNMPsrv\tReceived UnAuthorized Message from IP " << remoteAddress);
		  continue;
		}
		// process the request
		if (ProcessPDU(readBuffer, sendBuffer) == true) {
			// send the packet
			m_baseSocket->SetSendAddress(remoteAddress, remotePort);
			PTRACE(4, "SNMPsrv\tWriting " << sendBuffer.GetSize() << " Bytes to basesocket");
			if (!Write(sendBuffer, sendBuffer.GetSize())) {
			    PTRACE(4,"SNMPsrv\tWrite Error.");
			    continue;
			}
			sendBuffer.SetSize(m_maxTxSize); //revert to max tx
		}
  }

}


PBoolean PSNMPServer::Authorise(const PIPSocket::Address & /*received*/)
{
  return false;
}


void PSNMPServer::SetVersion(PASNInt newVersion)
{
  m_version = newVersion;
}



PSNMP::ErrorType PSNMPServer::SendGetResponse (PSNMPVarBindingList &)
{
  PAssertAlways("SendGetResponse not yet implemented");
  return GenErr;
}


PBoolean PSNMPServer::OnGetRequest (PINDEX , PSNMP::BindingList &, PSNMP::ErrorType &)
{
	return false;
}


PBoolean PSNMPServer::OnGetNextRequest (PINDEX , PSNMP::BindingList &, PSNMP::ErrorType &)
{
	return false;
}


PBoolean PSNMPServer::OnSetRequest (PINDEX , PSNMP::BindingList &,PSNMP::ErrorType &)
{
	return false;
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
		vars.SetSize((int)varlist.size());
		PSNMP::BindingList::const_iterator Iter = varlist.begin();
		while (Iter != varlist.end()) {
		  vars[i].m_name.SetValue(Iter->first);
		  vars[i].m_value = Iter->second;
		  i++;
		  ++Iter;
		}
   }
}

PBoolean PSNMPServer::MIB_LocalMatch(PSNMP_PDU & pdu)
{
  PBoolean found = false;
  PSNMP_VarBindList & vars = pdu.m_variable_bindings;
  PINDEX size = vars.GetSize();
 
  for(PINDEX x = 0 ;x < size; x++){
    PRFC1155_ObjectSyntax *obj = (PRFC1155_ObjectSyntax*) m_objList.GetAt(vars[x].m_name);
    if (obj != NULL){
      vars[x].m_value = *obj;
      found = true;
    }
    else{
      pdu.m_error_status = PSNMP::NoSuchName;
    }
  }
  return found;
}

PBoolean PSNMPServer::ConfirmCommunity(PASN_OctetString & /*community*/)
{
	return false;
}

PBoolean PSNMPServer::ConfirmVersion(PASN_Integer vers)
{
  return m_version == vers ? true : false;
}

PBoolean PSNMPServer::ProcessPDU(const PBYTEArray & readBuffer, PBYTEArray & sendBuffer)
{

  PSNMP_Message msg;
  
  if (!msg.Decode((PASN_Stream &)readBuffer)) {
	  PTRACE(4,"SNMPsrv\tERROR DECODING PDU");
	  return false;
  }

  PTRACE(4, "SNMPsrv\tEncoded message" << msg);

  if (!ConfirmVersion(msg.m_version)){
    PTRACE(4, "SNMPsrv\tVersion mismatch on request, ignoring");
    return false;
  }

  if (!ConfirmCommunity(msg.m_community)){
    PTRACE(4, "SNMPsrv\tCommunity string mismatch on request, ignoring");
    return false;
  }

  PSNMP::BindingList varlist;
  PINDEX reqID;

  PSNMP_Message resp;
  PSNMP_PDUs sendpdu;

  PBoolean retval = true;
  PSNMP::ErrorType errCode = PSNMP::NoError;
  switch (msg.m_pdu.GetTag()) {
    case PSNMP_PDUs::e_get_request:
      DecodeOID<PSNMP_GetRequest_PDU>(msg.m_pdu, reqID, varlist);
      retval = OnGetRequest(reqID, varlist, errCode);
      if (retval == true){
        sendpdu.SetTag(PSNMP_PDUs::e_get_response);
        EncodeOID<PSNMP_GetResponse_PDU>(sendpdu, reqID,varlist,errCode);
        resp.m_pdu = sendpdu;
        resp.m_version = msg.m_version;
        resp.m_community = msg.m_community;
        PSNMP_GetResponse_PDU & mpdu = (PSNMP_GetResponse_PDU &) resp.m_pdu;
        if (!MIB_LocalMatch(mpdu)){
          retval = false;
          break;
        }
        resp.Encode((PASN_Stream &)sendBuffer);
      }
      
    break;
    case PSNMP_PDUs::e_get_next_request:
      DecodeOID<PSNMP_GetNextRequest_PDU>(msg.m_pdu, reqID, varlist);
      retval = OnGetNextRequest(reqID,varlist,errCode);
      if (retval == true){
        sendpdu.SetTag(PSNMP_PDUs::e_get_response);   
        EncodeOID<PSNMP_GetResponse_PDU>(sendpdu, reqID,varlist,errCode);
        resp.m_pdu = sendpdu;
        resp.m_version = msg.m_version;
        resp.m_community = msg.m_community;
        PSNMP_GetResponse_PDU & mpdu = (PSNMP_GetResponse_PDU &) resp.m_pdu;
        if (!MIB_LocalMatch(mpdu)){
          retval = false;
          break;
        }
        resp.Encode((PASN_Stream &)sendBuffer);
      }
      break;

    case PSNMP_PDUs::e_set_request:
      DecodeOID<PSNMP_SetRequest_PDU>(msg.m_pdu, reqID, varlist);
      retval = OnSetRequest(reqID, varlist, errCode);
      break;

    case PSNMP_PDUs::e_get_response:
    case PSNMP_PDUs::e_trap:
    default:
      PTRACE(4,"SNMPsrv\tSNMP Request/Response not supported");
      errCode= PSNMP::GenErr;
      retval = false;
  }

  if (retval) {
    PTRACE(4, "SNMPSrv\tSNMP Response " << resp);
  }

  return retval;
}

#endif

// End Of File ///////////////////////////////////////////////////////////////
