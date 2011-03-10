//
// httptest.cxx
//
// Copyright 2011 Vox Lucida Pty. Ltd.
//

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sockets.h>
#include <ptclib/http.h>
#include <ptclib/threadpool.h>


class HTTPConnection
{
  public:
    HTTPConnection(PHTTPSpace & httpNameSpace)
      :  m_httpNameSpace(httpNameSpace)
    {
    }

    void Work();

    PHTTPSpace & m_httpNameSpace;
    PTCPSocket m_socket;
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
  args.Parse("p:T:Q:");

  m_pool.SetMaxWorkers(args.GetOptionString('T', "10").AsUnsigned());

  PTCPSocket listener((WORD)args.GetOptionString('p', "80").AsUnsigned());
  if (!listener.Listen(args.GetOptionString('Q', "100").AsUnsigned())) {
    cerr << "Could not listen on port " << listener.GetPort() << endl;
    return;
  }

  PHTTPSpace httpNameSpace;
  httpNameSpace.AddResource(new PHTTPString("index.html", "Hello", "text/plain"));

  cout << "Listening for HTTP on port " << listener.GetPort() << endl;

  for (;;) {
    HTTPConnection * connection = new HTTPConnection(httpNameSpace);
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
  PHTTPServer httpServer(m_httpNameSpace);
  if (!httpServer.Open(m_socket))
    return;

  while (httpServer.ProcessCommand())
    ;
}


// End of hello.cxx
