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
 * $Revision: 25564 $
 * $Author: csoutheren $
 * $Date: 2011-04-14 15:49:56 +1000 (Thu, 14 Apr 2011) $
 */

#include <ptlib.h>
#include "main.h"
#include "version.h"

#include <ptlib/udpsock.h>

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
  PUDPSocket::VectorOfSlice slices;

  BYTE buffer1[10];
  BYTE buffer2[10];

  slices.push_back(PUDPSocket::Slice(buffer1, sizeof(buffer1)));
  slices.push_back(PUDPSocket::Slice(buffer2, sizeof(buffer2)));

  for (;;) {
    PIPSocketAddressAndPort addr;
    if (!m_socket.ReadFrom(slices, addr)) {
      PError << "read failed" << endl;
      break;
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
    PError << "error: socket listen failed" << endl;
    return;
  }  

  PIPSocketAddressAndPort rxAddr;
  if (!rxSocket.GetLocalAddress(rxAddr)) {
    PError << "error: cannot get local socket address" << endl;
    return;
  }  

  WORD port = rxAddr.GetPort();
  PError << "listening socket opened on port " << port << endl;

  WaitForIncoming waiter(rxSocket);
  PThread * thread = new PThreadFunctor<WaitForIncoming>(waiter);


  // send some test data
  PUDPSocket txSocket;

  {
    PIPSocket::VectorOfSlice slices;
    BYTE buffer1[10];
    BYTE buffer2[10];
    slices.push_back(PUDPSocket::Slice(buffer1, sizeof(buffer1)));
    slices.push_back(PUDPSocket::Slice(buffer2, sizeof(buffer2)));

    PIPSocketAddressAndPort dest("127.0.0.1", port);

    if (!txSocket.WriteTo(slices, dest)) {
      PError << "error: write failed" << endl;
    }
    else {
      PError << "error: write succeeded" << endl;
    }
  }

  thread->WaitForTermination();
  delete thread;
}

// End of File ///////////////////////////////////////////////////////////////
