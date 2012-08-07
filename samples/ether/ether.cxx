/*
 * ether.cxx
 *
 * Sample program to read raw data from ethernet.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2012 Vox Lucida Pty. Ltd.
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
 * The Original Code is Portable Tools Library.
 *
 * The Initial Developer of the Original Code is Vox Lucida
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sockets.h>


class MyProcess : public PProcess
{
    PCLASSINFO(MyProcess, PProcess)
  public:
    MyProcess()
      : PProcess("Vox Lucida", "Ethernet", 1, 0, ReleaseCode, 1)
      , m_running(true)
    { }

    virtual void Main();

    virtual bool OnInterrupt(bool)
    {
      m_running = false;
      return true;
    }

  protected:
    bool m_running;
};


PCREATE_PROCESS(MyProcess)



void MyProcess::Main()
{
  cout << "Ethernet Test Utility" << endl;

  PArgList & args = GetArguments();
  if (args.Parse(
          "i-interface: Interface name to sniff\n"
          "I-ip:        IP address\n"
          "T-tcp:       TCP port\n"
          "U-udp:       UDP port\n"
          "B-binary.    Payload is binary\n"
#if PTRACING
          "o-output:    Trace log output file\n"
          "t-trace.     Trace log level\n"
#endif
          "h-help.      Help text\n"
          ) < PArgList::ParseNoArguments || args.HasOption('h') ||
                     (args.HasOption('T') && args.HasOption('U'))) {
    args.Usage(cerr);
    return;
  }

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  PEthSocket sock;

  if (!args.HasOption('i')) {
    PStringArray interfaces = sock.EnumInterfaces();
    if (interfaces.IsEmpty())
      cout << "No interfaces available for ethernet sniffing." << endl;
    else {
      cout << "Available interfaces:\n";
      for (PINDEX i = 0; i < interfaces.GetSize(); ++i)
        cout << "  #" << i << ' ' << interfaces[i] << '\n';
      cout << endl;
    }
    return;
  }

  PString interf = args.GetOptionString('i');
  if (interf[0] == '#') {
    PINDEX idx = interf.Mid(1).AsUnsigned();
    PStringArray interfaces = sock.EnumInterfaces(false);
    if (idx >= interfaces.GetSize()) {
      cerr << "Invalid index for interface." << endl;
      return;
    }
    interf = interfaces[idx];
  }

  sock.SetReadTimeout(1000);
  if (!sock.Connect(interf)) {
    cerr << "Could not open interface \"" << interf << '"' << endl;
    return;
  }

  cout << "Listening on \"" << interf << '"' << endl;

  PIPSocket::Address ip = PIPSocket::GetDefaultIpAny();
  if (args.HasOption('I'))
    ip = args.GetOptionString('I');

  unsigned tcp = args.GetOptionString('T', "0").AsUnsigned();
  unsigned udp = args.GetOptionString('U', "0").AsUnsigned();

  PBYTEArray buffer;
  PEthSocket::Address src, dst;
  WORD type;
  PINDEX payloadSize;
  BYTE * payloadPtr;
  while (m_running) {
    if (sock.ReadPacket(buffer, dst, src, type, payloadSize, payloadPtr)) {
      if (tcp == 0 && udp == 0)
        cout << dst << "<=" << src << ' ' << hex << setfill('0') << setw(4) << type << dec << ' ' << payloadSize << endl;
      else if (type == PEthSocket::TypeIP && (payloadPtr[9] == (udp != 0 ? 0x11 : 6))) {
        PIPSocket::Address srcIP(4, payloadPtr+12);
        PIPSocket::Address dstIP(4, payloadPtr+16);
        if (ip.IsAny() || srcIP == ip || dstIP == ip) {
          size_t len = (payloadPtr[0]&0xf)*4; // low 4 bits in DWORDS, is this in bytes
          payloadSize -= len;
          payloadPtr += len;

          WORD srcPort = ((PUInt16b *)payloadPtr)[0];
          WORD dstPort = ((PUInt16b *)payloadPtr)[1];
          if (udp != 0) {
            payloadSize -= 8;
            payloadPtr += 8;
          }
          else {
            payloadSize -= 20;
            payloadPtr += 20;
          }

          if (srcPort == tcp || dstPort == tcp || srcPort == udp || dstPort == udp) {
            cout << dstIP << ':' << dstPort << "<=" << srcIP << ':' << dstPort << ' ';
            if (args.HasOption('B'))
              cout << PBYTEArray(payloadPtr, std::min(payloadSize, 16), false);
            else {
              PString str((char *)payloadPtr, payloadSize);
              cout << str.Left(str.FindOneOf("\r\n"));
            }
            cout << endl;
          }
        }
      }
    }
    else if (sock.GetErrorCode(PChannel::LastReadError) != PChannel::Timeout) {
      cerr << "Error reading interface \"" << interf << '"'  << endl;
      break;
    }
  }

  sock.Close();

  cout << "Ended sniff." << endl;
}


// End of ether.cxx
