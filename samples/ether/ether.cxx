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

static PMutex g_coutMutex;

class MyProcess : public PProcess
{
    PCLASSINFO(MyProcess, PProcess)
  public:
    MyProcess()
      : PProcess("Vox Lucida", "Ethernet", 1, 0, ReleaseCode, 1)
    { }

    virtual void Main();

    virtual bool OnInterrupt(bool)
    {
      m_exit.Signal();
      return true;
    }

  protected:
    PSyncPoint m_exit;
};


PCREATE_PROCESS(MyProcess)


class TestThread : public PThread
{
  public:
    TestThread(PINDEX idx, PArgList & args);

    bool IsOpen() const { return m_socket.IsOpen(); }

    void Stop();

  protected:
    virtual void Main();

    PINDEX     m_index;
    PEthSocket m_socket;
    bool       m_binary;
    bool       m_running;
};


void MyProcess::Main()
{
  cout << "Ethernet Test Utility" << endl;

  PArgList & args = GetArguments();
  PArgList::ParseResult parseResult = args.Parse(
          "l-list.      List available interfaces\n"
          "b-binary.    Payload is binary\n"
#if PTRACING
          "o-output:    Trace log output file\n"
          "t-trace.     Trace log level\n"
#endif
          "h-help.      Help text\n"
          );
  if (parseResult < PArgList::ParseNoArguments || args.HasOption('h') ||
          (parseResult == PArgList::ParseNoArguments && !args.HasOption('l'))) {
    cerr << "\nusage: " << GetName() << " <interface> [ <filter> ]\n";
    args.Usage(cerr);
    cerr << "\nThe <interface> is either #N for the N'th interface, or the full string name.\n";
    return;
  }

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  if (args.HasOption('l')) {
    cout << "Available interfaces:\n";
    PStringArray interfaces = PEthSocket::EnumInterfaces();
    for (PINDEX i = 0; i < interfaces.GetSize(); ++i)
      cout << "  #" << i << ' ' << interfaces[i] << '\n';
    cout << endl;
    return;
  }

  PList<TestThread> tests;
  do {
    tests.Append(new TestThread(tests.GetSize()+1, args));
    if (!tests.back().IsOpen())
      return;
  } while (args.Parse(NULL) > PArgList::ParseNoArguments);

  m_exit.Wait();

  g_coutMutex.Wait();
  cout << "Exiting sniffers ..." << endl;
  g_coutMutex.Signal();

  for (PList<TestThread>::iterator it = tests.begin(); it != tests.end(); ++it)
    it->Stop();
}


TestThread::TestThread(PINDEX idx, PArgList & args)
  : PThread(0, NoAutoDeleteThread, NormalPriority, "Sniffer")
  , m_index(idx)
  , m_binary(args.HasOption('b'))
  , m_running(true)
{
  PString interf = args[0];
  if (interf[0] == '#') {
    PINDEX idx = interf.Mid(1).AsUnsigned();
    PStringArray interfaces = PEthSocket::EnumInterfaces(false);
    if (idx >= interfaces.GetSize()) {
      cerr << "Invalid index for interface." << endl;
      return;
    }
    interf = interfaces[idx];
  }

  m_socket.SetReadTimeout(1000);
  if (!m_socket.Connect(interf)) {
    cerr << "Could not open interface \"" << interf << '"' << endl;
    return;
  }

  PString filter;
  for (PINDEX arg = 1; arg < args.GetCount(); ++arg)
    filter &= args[arg];
  if (!m_socket.SetFilter(filter)) {
    cerr << "Could not use filter \"" << filter << '"' << endl;
    m_socket.Close();
  }

  Resume();
}


void TestThread::Stop()
{
  m_running = false;
  WaitForTermination();
}


void TestThread::Main()
{
  g_coutMutex.Wait();
  cout << m_index << ": Listening on \"" << m_socket.GetName() << '"';
  if (!m_socket.GetFilter().IsEmpty())
    cout << " filtered by \"" << m_socket.GetFilter() << '"';
  cout << endl;
  g_coutMutex.Signal();

  PBYTEArray buffer;
  PEthSocket::Address src, dst;
  WORD type;
  PINDEX payloadSize;
  BYTE * payloadPtr;
  while (m_running) {
    if (!m_socket.ReadPacket(buffer, dst, src, type, payloadSize, payloadPtr)) {
      if (m_socket.GetErrorCode(PChannel::LastReadError) == PChannel::Timeout)
        continue;

      cerr << m_index << ": Error reading interface \"" << m_socket.GetName() << '"'  << endl;
      break;
    }

    g_coutMutex.Wait();
    cout << m_index << ": ";
    if (type != PEthSocket::TypeIP)
      cout << dst << "<=" << src << ' ' << hex << setfill('0') << setw(4) << type << dec << ' ' << payloadSize;
    else {
      PIPSocket::Address srcIP(4, payloadPtr+12);
      PIPSocket::Address dstIP(4, payloadPtr+16);
      int proto = payloadPtr[9];

      size_t len = (payloadPtr[0]&0xf)*4; // low 4 bits in DWORDS, is this in bytes
      payloadSize -= len;
      payloadPtr += len;

      WORD srcPort = ((PUInt16b *)payloadPtr)[0];
      WORD dstPort = ((PUInt16b *)payloadPtr)[1];
      cout << dstIP << ':' << dstPort << "<=" << srcIP << ':' << srcPort << ' ';

      if (proto != 0x11) {
        payloadSize -= 8;
        payloadPtr += 8;
        cout << "UDP ";
      }
      else {
        payloadSize -= 20;
        payloadPtr += 20;
        cout << "TCP ";
      }

      if (m_binary)
        cout << PBYTEArray(payloadPtr, std::min(payloadSize, 16), false);
      else {
        PString str((char *)payloadPtr, payloadSize);
        cout << str.Left(str.FindOneOf("\r\n"));
      }
    }

    cout << endl;
    g_coutMutex.Signal();
  }

  g_coutMutex.Wait();
  cout << m_index << ": Finished sniffer." << endl;
  g_coutMutex.Signal();
}


// End of ether.cxx
