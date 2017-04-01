/*
 * main.cxx
 *
 * PWLib application header file for scatter read/write test
 *
 * Copyright (c) 2010 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 */

#include <ptlib.h>
#include "main.h"
#include "version.h"

#include <ptlib/udpsock.h>

#include <vector>

PCREATE_PROCESS(ScatterTest);

ScatterTest::ScatterTest()
  : PProcess("Post Increment", "ScatterTest", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}

struct WaitForIncoming
{
  WaitForIncoming(PUDPSocket & socket);

  void operator () (PThread & thread);

  PUDPSocket & m_socket;
};

WaitForIncoming::WaitForIncoming(PUDPSocket & socket)
  : m_socket(socket)
{
}

void WaitForIncoming::operator () (PThread &)
{
  std::vector<PUDPSocket::Slice> slices;

  BYTE buffer1[10];
  BYTE buffer2[10];

  slices.push_back(PUDPSocket::Slice(buffer1, sizeof(buffer1)));
  slices.push_back(PUDPSocket::Slice(buffer2, sizeof(buffer2)));

  for (;;) {
    PIPSocketAddressAndPort addr;
    if (!m_socket.ReadFrom(&slices[0], slices.size(), addr)) {
      if (m_socket.GetErrorCode() == PUDPSocket::Interrupted) {
        PError << "read interrupted" << endl;
        continue;
      }
      else {
        PError << "read failed : " << m_socket.GetErrorText() << endl;
        break;
      }
    }
    PError << "Read from " << addr << " return " << m_socket.GetLastReadCount() << " bytes in " << slices.size() << " slices" << endl;
    size_t i;
    for (i = 0; i < slices.size(); ++i)
      PError << "  slice " << i+1 << " : len = " << slices[i].GetLength() << endl;
  }
}


void ScatterTest::Main()
{
  PArgList & args = GetArguments();
  args.Parse(
#if PTRACING
            "t-trace."       "-no-trace."
            "o-output:"      "-no-output."
#endif
    );

#if PTRACING

  PTrace::Initialise(args.GetOptionCount('t'),
                   args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
                   PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  // open listening socket 
  PUDPSocket rxSocket;
  if (!rxSocket.Listen()) {
    PError << "error: socket listen failed : " << rxSocket.GetErrorText() << endl;
    return;
  }  

  PIPSocketAddressAndPort rxAddr;
  if (!rxSocket.GetLocalAddress(rxAddr)) {
    PError << "error: cannot get local socket address : " << rxSocket.GetErrorText() << endl;
    return;
  }  

  WORD port = rxAddr.GetPort();
  PError << "listening socket opened on port " << port << endl;

  WaitForIncoming waiter(rxSocket);
  PThread * thread = new PThreadFunctor<WaitForIncoming>(waiter);


  // send some test data
  PUDPSocket txSocket;

  {
    std::vector<PIPSocket::Slice> slices;
    BYTE buffer1[7];
    BYTE buffer2[7];
    slices.push_back(PUDPSocket::Slice(buffer1, sizeof(buffer1)));
    slices.push_back(PUDPSocket::Slice(buffer2, sizeof(buffer2)));

    PIPSocketAddressAndPort dest("127.0.0.1", port);

    if (!txSocket.WriteTo(&slices[0], slices.size(), dest)) {
      PError << "error: write failed : " << rxSocket.GetErrorText() << endl;
    }
    else {
      PError << "write succeeded" << endl;
    }
  }

  thread->WaitForTermination();
  delete thread;
}

// End of File ///////////////////////////////////////////////////////////////
