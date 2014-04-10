//
// httptest.cxx
//
// Copyright 2011 Vox Lucida Pty. Ltd.
//

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sockets.h>
#include <ptclib/pssl.h>
#include <ptclib/http.h>
#include <ptclib/threadpool.h>


class HTTPConnection
{
  public:
    HTTPConnection(
      PHTTPSpace & httpNameSpace
#if P_SSL
      , PSSLContext * context
#endif
    ) : m_httpNameSpace(httpNameSpace)
#if P_SSL
      , m_context(context)
#endif
    {
    }

    void Work();

    PHTTPSpace  & m_httpNameSpace;
#if P_SSL
    PSSLContext * m_context;
#endif
    PTCPSocket    m_socket;
};


class HTTPTest : public PProcess
{
    PCLASSINFO(HTTPTest, PProcess)
  public:
    void Main();

    PQueuedThreadPool<HTTPConnection> m_pool;
};

PCREATE_PROCESS(HTTPTest)


void HTTPTest::Main()
{
  PArgList & args = GetArguments();
  args.Parse("h-help.    print this help message.\n"
             "p-port:    port number to listen on(default 80 or 443).\n"
#if P_SSL
             "s-secure.     SSL/TLS mode.\n"
             "-ca:          SSL/TLS client certificate authority file/directory.\n"
             "-certificate: SSL/TLS server certificate.\n"
             "-private-key: SSL/TLS server private key.\n"
#endif
             "T-theads:  max number of threads in pool(default 10)\n"
             "Q-queue:   max queue size for listening sockets(default 100).\n"
             PTRACE_ARGLIST
       );

  PTRACE_INITIALISE(args);

  if (!args.IsParsed() || args.HasOption('h')) {
    args.Usage(PError);
    return;
  }

  m_pool.SetMaxWorkers(args.GetOptionString('T', "10").AsUnsigned());

#if P_SSL
  PSSLContext * sslContext = args.HasOption('s') ? new PSSLContext : NULL;
  if (sslContext != NULL) {
    if (!sslContext->SetCredentials(args.GetOptionString("ca", "."),
                                    args.GetOptionString("certificate", "certificate.pem"),
                                    args.GetOptionString("private-key", "privatekey.pem"),
                                    true))
    {
      cerr << "Could not set credentials for SSL" << endl;
      return;
    }
  }
#endif

    PTCPSocket listener(args.GetOptionAs('p', (WORD)(sslContext != NULL ? 443 : 80)));
  if (!listener.Listen(args.GetOptionString('Q', "100").AsUnsigned())) {
    cerr << "Could not listen on port " << listener.GetPort() << endl;
    return;
  }

  PHTTPSpace httpNameSpace;
  httpNameSpace.AddResource(new PHTTPString("index.html", "Hello", "text/plain"));

  cout << "Listening for " << (sslContext != NULL ? "https" : "http") << " on port " << listener.GetPort() << endl;

  for (;;) {
    HTTPConnection * connection = new HTTPConnection(httpNameSpace, sslContext);
    if (connection->m_socket.Accept(listener))
      m_pool.AddWork(connection);
    else {
      delete connection;
      cerr << "Error in accept: " << listener.GetErrorText() << endl;
      break;
    }
  }

  cout << "Exiting HTTP test" << endl;
}


void HTTPConnection::Work()
{
  PTRACE(3, "HTTPTest\tStarted work on " << m_socket.GetPeerAddress());

  PHTTPServer httpServer(m_httpNameSpace);

#if P_SSL
  if (m_context != NULL) {
    PSSLChannel * ssl = new PSSLChannel(m_context);
    if (!ssl->Open(m_socket))
      return;
    if (!ssl->Accept())
      return;
    if (!httpServer.Open(ssl))
      return;
  }
  else
#endif
  if (!httpServer.Open(m_socket))
    return;

  unsigned count = 0;
  while (httpServer.ProcessCommand())
    ++count;

  PTRACE(3, "HTTPTest\tEnded work on " << m_socket.GetPeerAddress() << ", " << count << " transactions.");
}


// End of hello.cxx
