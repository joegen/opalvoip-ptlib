//
// asynctest.cxx
//
// Copyright 2011 Vox Lucida Pty. Ltd.
//

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sockets.h>
#include <ptclib/threadpool.h>

#include <vector>


enum Modes
{
  DisabledMode,
  SyncMode,
  AsyncMode,
  NumModes
};

static const char * ModeNames[NumModes] = { "none", "sync", "async" };

static Modes GetMode(const PCaselessString & str)
{
  if (str == ModeNames[AsyncMode])
    return AsyncMode;
  if (str == ModeNames[SyncMode])
    return SyncMode;
  return DisabledMode;
}


class AsyncTest : public PProcess
{
    PCLASSINFO(AsyncTest, PProcess)
  public:
    void Main();

    unsigned m_numTests;
    unsigned m_concurrent;
    PIPSocket::Address m_binding;

    PAtomicInteger m_testsExecuted;
    PAtomicInteger m_testersRunning;
    PSyncPoint     m_finishedTest;

    PArray<PUDPSocket> m_readSockets;
    PArray<PUDPSocket> m_writeSockets;

    void SenderMain(int index);
    PDECLARE_NOTIFIER(PThread, AsyncTest, Receiver);

    class MyContext : public PChannel::AsyncContext
    {
    public:
      MyContext(int index, const PChannel::AsyncNotifier & notifer, WORD port)
        : PChannel::AsyncContext(m_storage, sizeof(m_storage), notifer)
        , m_index(index)
        , m_port(port)
      {
      }

      int  m_index;
      BYTE m_storage[1000];
      WORD m_port;
    };

    vector<MyContext> m_readContexts;
    PDECLARE_AsyncNotifier(AsyncTest, Received);
};

PCREATE_PROCESS(AsyncTest)


class SenderThread : public PThread
{
    PCLASSINFO(SenderThread, PThread)
  protected:
    AsyncTest & m_app;
    int m_index;

  public:
    SenderThread(AsyncTest & app, int index)
      : PThread(65536, NoAutoDeleteThread, NormalPriority, "Sender")
      , m_app(app)
      , m_index(index)
    {
    }

    void Main()
    {
      m_app.SenderMain(m_index);
    }
};


void AsyncTest::Main()
{
  PArgList & args = GetArguments();
  args.Parse("h-help."
             "c-concurrent:"
             "n-num-tests:"
             "i-interface:"
             "p-port:"
#if PTRACING
             "o-output:"
             "t-trace."
#endif
       );

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  if (args.HasOption('h') || args.GetCount() < 2) {
    PError << "usage: " << GetFile().GetTitle() << "[options] <sender-mode> <receiver-mode>\n"
              "\n"
              "   <X-mode> is one of \"none\", \"sync\" or \"async\".\n"
              "\n"
              "Available options are:\n"
              "   -h --help             : print this help message.\n"
              "   -c --concurrent n     : number of concurrent tests (default 100).\n"
              "   -n --num-tests n      : total number of tests (default 100000).\n"
              "   -i --interface if     : interface to use (default 127.0.0.1).\n"
              "   -p --port n           : port base to use (default random).\n"
#if PTRACING
              "   -o or --output file   : file name for output of log messages\n"       
              "   -t or --trace         : degree of verbosity in log (more times for more detail)\n"     
#endif
           << endl;
    return;
  }

  Modes senderMode = GetMode(args[0]);
  Modes receiverMode = GetMode(args[1]);

  m_concurrent = args.GetOptionString('c', "100").AsUnsigned();
  unsigned concurrent;

  m_binding = args.GetOptionString('i', "127.0.0.1");
  WORD port = (WORD)args.GetOptionString('p', "0").AsUnsigned();

  if (receiverMode != DisabledMode) {
    for (concurrent = 0; concurrent < m_concurrent; ++concurrent) {
      PUDPSocket * socket = new PUDPSocket;
      if (!socket->Listen(m_binding, 1, port)) {
        cerr << "Could not create socket listening on " << m_binding << ':' << port << endl;
        return;
      }
      m_readSockets.Append(socket);
      if (port > 0)
        ++port;
    }
  }

  if (senderMode != DisabledMode) {
    for (concurrent = 0; concurrent < m_concurrent; ++concurrent) {
      PUDPSocket * socket = new PUDPSocket;
      if (!socket->Listen(m_binding)) {
        cerr << "Could not create socket listening on " << m_binding << endl;
        return;
      }
      m_writeSockets.Append(socket);
    }
  }

  unsigned totalTests = args.GetOptionString('n', "100000").AsInteger();
  m_numTests = (totalTests+m_concurrent-1)/m_concurrent;

  m_testersRunning = m_concurrent;

  if (receiverMode == AsyncMode) {
    for (concurrent = 0; concurrent < m_concurrent; ++concurrent)
      m_readContexts.push_back(MyContext(concurrent,
                                         PCREATE_AsyncNotifier(Received),
                                         m_writeSockets[concurrent].GetPort()));
  }


  for (concurrent = 0; concurrent < m_concurrent; ++concurrent) {
    switch (receiverMode) {
      case SyncMode :
        PThread::Create(PCREATE_NOTIFIER(Receiver), concurrent, PThread::AutoDeleteThread, PThread::NormalPriority, "Receiver");
        break;
      case AsyncMode :
        m_readSockets[concurrent].ReadAsync(m_readContexts[concurrent]);
        break;
      default :
        break;
    }
  }

  PList<SenderThread> senderThreads;
  for (concurrent = 0; concurrent < m_concurrent; ++concurrent) {
    switch (senderMode) {
      case SyncMode :
        senderThreads.Append(new SenderThread(*this, concurrent));
        break;
      case AsyncMode :
        break;
      default :
        break;
    }
  }

  for (PList<SenderThread>::iterator it = senderThreads.begin(); it != senderThreads.end(); ++it)
    it->Resume();

  PTime start;
  cout << "Testing ..." << endl;
  while (m_testersRunning > 0) {
    if (!m_finishedTest.Wait(5000))
      cout << m_testsExecuted << endl;
  }

  PTimeInterval taken = PTime() - start;
  cout << "Completed: " << taken << " seconds, "
       << (m_numTests*1000/taken.GetMilliSeconds()) << " ops/sec/thread" << endl;
}


void AsyncTest::SenderMain(int index)
{
  PUDPSocket & socket = m_writeSockets[index];

  PTRACE(4, "Async\tStarted sender thread " << index << ", socket=" << socket.GetLocalAddress());

  BYTE buffer[1000];
  PIPSocket::Address ip;
  WORD port = m_readSockets[index].GetPort();

  for (unsigned i = 0; i < m_numTests; ++i) {
    if (!socket.WriteTo(buffer, sizeof(buffer), m_binding, port)) {
      PTRACE(1, "Async\tSender " << index << " write error: " << socket.GetErrorText(PChannel::LastWriteError));
      break;
    }

    if (!socket.ReadFrom(buffer, sizeof(buffer), ip, port)) {
      PTRACE(1, "Async\tSender " << index << " read error: " << socket.GetErrorText(PChannel::LastReadError));
      break;
    }
  }

  m_readSockets[index].Close();

  --m_testersRunning;
  m_finishedTest.Signal();

  PTRACE(4, "Async\tEnded sender thread " << index);
}


void AsyncTest::Receiver(PThread &, P_INT_PTR index)
{
  PTRACE(4, "Async\tStarted receiver thread " << index);

  PUDPSocket & socket = m_readSockets[index];

  BYTE buffer[1000];
  PIPSocket::Address ip;
  WORD port;

  for (;;) {
    if (!socket.ReadFrom(buffer, sizeof(buffer), ip, port)) {
      PTRACE_IF(1, socket.GetErrorCode() != PChannel::Interrupted,
                "Async\tReceiver " << index << " read error: " << socket.GetErrorText(PChannel::LastReadError));
      break;
    }

    if (!socket.WriteTo(buffer, socket.GetLastReadCount(), ip, port)) {
      PTRACE(1, "Async\tReceiver " << index << " write error: " << socket.GetErrorText(PChannel::LastWriteError));
      break;
    }

    ++m_testsExecuted;
  }

  PTRACE(4, "Async\tEnded receiver thread " << index);
}


void AsyncTest::Received(PChannel & channel, PChannel::AsyncContext & asyncContext)
{
  PUDPSocket & socket = dynamic_cast<PUDPSocket &>(channel);
  MyContext & context = static_cast<MyContext &>(asyncContext);

  PTRACE(5, "Async\tReceived data for " << context.m_index << ", sending to " << context.m_port);

  if (!socket.WriteTo(context.m_buffer, context.m_length, m_binding, context.m_port)) {
    PTRACE_IF(1, socket.GetErrorCode() != PChannel::Interrupted,
              "Async\tReceived " << context.m_index << " write error: "
              << socket.GetErrorText(PChannel::LastWriteError));
  }

  if (!socket.ReadAsync(context)) {
    PTRACE_IF(1, socket.GetErrorCode() != PChannel::Interrupted,
              "Async\tRead " << context.m_index << " error: "
              << socket.GetErrorText(PChannel::LastReadError));
  }

  ++m_testsExecuted;
}


// End of asynctest.cxx
