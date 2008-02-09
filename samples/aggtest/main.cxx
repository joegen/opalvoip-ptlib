/*
 * main.cxx
 *
 * PWLib application source file for aggtest
 *
 * Main program entry point.
 *
 * Copyright (C) 2004 Post Increment
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
 * $Revision$
 * $Author$
 * $Date$
 */

#include "precompile.h"
#include "main.h"
#include "version.h"

#include <ptclib/sockagg.h>

PCREATE_PROCESS(AggTest);

AggTest::AggTest()
  : PProcess("Post Increment", "AggTest", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}

class MyUDPSocket : public PUDPSocket
{
  public:
    PBoolean OnRead()
    {
      BYTE buffer[1024];
      Read(buffer, 1024);
      return PTrue;
    }

};

//////////////////////////////////////////////////////////////////////////////

#if _WIN32

#pragma comment(lib, "Ws2_32.lib")

#endif

/*

class Data_T
{
  public:
    void Close(PUDPSocket * socket)
    void PreRead(Socket_T * socket);
    void Read(Socket_T * socket);
};
*/

template <class Data_T, class Socket_T = PIPSocket>
class PIPSocketPool {
  public:

#ifdef _WIN32
    typedef WSAEVENT FDList_Type;
#define	FDLIST_SIZE	WSA_MAXIMUM_WAIT_EVENTS
#else
    typedef int      FDList_Type;
#define	FDLIST_SIZE	64
#endif

    typedef std::vector<FDList_Type> FDList_T;

    struct SocketData_T {
      SocketData_T(Socket_T * _s, Data_T * _d) 
        : ipSocket(_s), data(_d), closed(false)
      { 
        fd = ipSocket->GetHandle();
#ifdef _WIN32
        winEvent = ::WSACreateEvent(); 
        PAssert(WSAEventSelect(fd, winEvent, FD_READ | FD_CLOSE) == 0, "WSAEventSelect failed"); 
#endif
      }

#ifdef _WIN32
      ~SocketData_T()
      {
        ::WSACloseEvent(fd); 
      }
      FDList_Type winEvent;
#endif

      Socket_T  * ipSocket;
      Data_T    * data;
      bool        closed;
      int         fd;
    };

    typedef std::map<Socket_T *, SocketData_T *> SocketDataMap_T;

    PIPSocketPool()
      : listChanged(false)
    {
#if _WIN32
      triggerEvent = ::CreateEvent(NULL, PTrue, PFalse,NULL); 
#else
      ::pipe(triggerPipe);
#endif
    }

    ~PIPSocketPool()
    {
#if _WIN32
      ::CloseHandle(triggerEvent); 
#else
      ::close(triggerPipe[0]);
      ::close(triggerPipe[2]);
#endif
    }

    bool AddSocket(Socket_T * socket, Data_T * data)
    {
      {
        PWaitAndSignal m(mutex);
        SocketData_T * sock = new SocketData_T(socket, data);
        socketMap.insert(SocketDataMap_T::value_type(socket, sock));
        listChanged = true;
      }
      Signal();
      return true;
    }

    Data_T * RemoveSocket(Socket_T * socket)
    {
      Data_T * data = NULL;
      {
        PWaitAndSignal m(mutex);
        SocketDataMap_T::iterator r = socketMap.find(socket);
        if (r == socketMap.end())
          return NULL;
        fdToSocketMap.erase(r->second->fd);
        data = r->second->data;
        delete r->second;
        socketMap.erase(r);
        listChanged = true;
      }
      Signal();
      return data;
    }

    void Signal()
    {
#ifdef _WIN32
      ::SetEvent(triggerEvent);
#else
#endif
    }

    bool Poll()
    {
      mutex.Wait();

      // erase the lists of sockets if they have have been invalidated
      if (listChanged) {
        listChanged = false;
#ifdef _WIN32
        fdList.erase(fdList.begin(), fdList.end());
        fdList.reserve(FDLIST_SIZE);
        fdList.push_back(triggerEvent);

        socketOrderList.erase(socketOrderList.begin(), socketOrderList.end());
        socketOrderList.reserve(FDLIST_SIZE);
#else
        FD_ZERO(&fdSet);
#endif
      }

      // create the list of sockets (if required) and do general startup stuff
      SocketDataMap_T::iterator r;
      for (r = socketMap.begin(); r != socketMap.end(); ++r) {
        SocketData_T * socket = r->second;

        if (socket->closed)
          continue;

        if (listChanged) {
#ifdef _WIN32
          fdList.push_back(socket->winEvent);
          socketOrderList.push_back(socket);
#else
          FD_SET(socket->fd, &fdSet);
#endif
        }

        socket->data->PreRead(socket->ipSocket);

        //PTimeInterval t = handle->GetTimeout();
        //if (t < timeout) {
        //  timeout = t;
        //  timeoutHandle = handle;
        //}
      }

      // perform select, or equivalent
      mutex.Signal();

      PTimeInterval timeout;

#if _WIN32
      // Windows - do WaitForMultipleEvents
      DWORD nCount = (DWORD)fdList.size();
      
      DWORD ret = WSAWaitForMultipleEvents(nCount, 
                                           &fdList[0], 
                                           false, 
                                           (timeout == PMaxTimeInterval) ? WSA_INFINITE : (DWORD)timeout.GetMilliSeconds(), 
                                           PFalse);

      mutex.Wait();

      if (WAIT_OBJECT_0 <= ret && ret <= (WAIT_OBJECT_0 + nCount - 1)) {
        DWORD index = ret - WAIT_OBJECT_0;

        // reset and ignore trigger events
        if (index == 0) {
          ::ResetEvent(triggerEvent); 
          return true;
        }

        // get the read event
        if (index >  0 && index < fdList.size()) {   // note off by one as socketOrderList does not contain trigger
          if (index <= socketOrderList.size()-1) {   
            SocketData_T * socket = socketOrderList[index-1];   
            WSANETWORKEVENTS events;
            WSAEnumNetworkEvents(socket->fd, socket->winEvent, &events);
            if (events.lNetworkEvents != 0) {

              // check for socket close. Must do first so Read can check status
              if ((events.lNetworkEvents & FD_CLOSE) != 0) {
                r->second->data->Close(r->second->ipSocket);
                socket->closed = true;
              }

              // check for read events first so we process any data that arrives before closing
              if ((events.lNetworkEvents & FD_READ) != 0) 
                r->second->data->Read(r->second->ipSocket);
    
              // make sure the list is refreshed without the closed socket
              if (socket->closed) 
                listChanged = PTrue;
            }
          }
        }
        return true;
      }

      if (ret != WSA_WAIT_TIMEOUT) {
        PTRACE(1, "SockPool\tWSAWaitForMultipleEvents error " << WSAGetLastError());
        return true;
      }

      // timeout - fall through
#else
      // Linux - do select here
      int stat = ::select(nfds, &fdSet, NULL, NULL, timeout);

      mutex.Wait();

      if (ret < 0) {
        PTRACE(1, "SockPool\tselect error " << errno());
        return true;
      }

      if (ret > 0) {
        int i;
        for (i = 0; i < stat; ++i) {
          if (FD_ISSET(i, &fdSet)) {
            FDToSocketData_T::iterator * r = fdToSocketMap.find((FD)i);
            if (r != fdToSocketMap.end())
              r->second->data->Read(r->second->ipsocket);
          }
        }
      }
      // timeout - fall through
#endif

      // make sure the handle did not disappear while we were waiting
      //SocketDataMap_T::iterator s = socketMap.find(timeoutHandle);
      //if (s == socketMap.end()) {
      //  PTRACE(4, "SockPool\tHandle was removed while waiting");
      //} 

      return true;
    } 

  protected:
    PMutex mutex;
    SocketDataMap_T socketMap;
    bool listChanged;
#if _WIN32
    typedef std::vector<SocketData_T *> SocketOrderList_T;
    SocketOrderList_T socketOrderList;
    FDList_T fdList;
    HANDLE triggerEvent;
#else
    typedef std::map<int, SocketData_T *> FDToSocketMap_T;
    FDToSocketMap_T fdToSocketMap;
    fd_set fdSet;
    int triggerPipe[2];
#endif
};

#if 0

typedef std::vector<PIPSocket *> PIPSocketAggregatorSocketList;

struct PIPSocketAggregatorFunctions {
  virtual void WaitForSocket(PIPSocketAggregatorSocketList & socketList) = 0;
};

class PIPSocketAggregatorThread : public PThreadPoolWorkerBase
{
  public
    PIPSocketAggregatorThread(PIPSocketAggregatorFunctions * _handler, PThreadPoolBase & _pool)
      : PThreadPoolWorkerBase(_pool), handler(_handler) 
    { }

    virtual unsigned GetWorkSize() const 
    { return (unsigned)socketList.size(); }

    void OnAddWork(PIPSocket * work)
    {
      PWaitAndSignal m(mutex);
      socketList.push_back(work);
      sync.Signal();
    }

    void OnRemoveWork(PIPSocket * work) 
    { 
      PWaitAndSignal m(mutex);
      PIPSocketAggregatorSocketList::iterator r = ::find(socketList.begin(), socketList.end(), work);
      if (r != socketList.end()) 
        socketList.erase(r);
      sync.Signal();
    }

    virtual void Shutdown()
    {
      shutdown = true;
      sync.Signal();
    }

    void Main()
    {
      while (!shutdown) {
        mutex.Wait();
        if (socketList.size() == 0) {
          mutex.Signal();
          sync.Wait();
          continue;
        }

        handler.WaitForSocket(socketList);

        mutex.Signal();
      }
    }

  protected:
    PIPSocketAggregatorFunctions & handler;
    PMutex mutex;
    PSyncPoint sync;
    PIPSocketAggregatorSocketList socketList;
};

struct PIPSocketAggregatorFunctions {
  virtual void WaitForSockets(PIPSocketAggregatorSocketList & socketList) = 0;
};

#define PIPSOCKET_AGGREGATOR(cls, handler) \
class cls : public PThreadPool<PIPSocket, PIPSocketAggregatorThread<handler> > \
{ \
  typedef PThreadPool<PIPSocket, PIPSocketAggregatorThread<handler> > Ancestor_T; \
  public: \
    cls(unsigned m = 10) : Ancestor_T(m) { } \
    void AddSocket(PIPSocket * s)    { AddWork(s); } \
    void RemoveSocket(PIPSocket * s) { RemoveWork(s); } \
}; \

class MySocketHandlerClass
{
  public:
    static void WaitForSocket(PIPSocketAggregatorSocketList & socketList)
    {
    }
};

#endif

//////////////////////////////////////////////////////////////////////////////

class MySocketHandler {
  public:
    MySocketHandler()
      : closed(false), preReadDone(false) { }

    void PreRead(PUDPSocket * socket)
    {
      if (!preReadDone) {
        //handle->PreRead();
        preReadDone = true;
      }
    }

    void Close(PUDPSocket * socket)
    {
      closed = true;
    }

    void Read(PUDPSocket * socket)
    {
      if (!closed) {
        preReadDone = false;
      } else {
        //handle->beingProcessed = PTrue;
        //handle->OnClose();
        //handle->beingProcessed = PFalse;

        // make sure the list is refreshed without the closed socket
        //listChanged = PTrue;
      }
    }

  protected:
    bool closed;
    bool preReadDone;
};

void AggTest::Main()
{
  PArgList & args = GetArguments();

  args.Parse(
             "-server:"
             "-to:"
             "-from:"
             "-re:"
             "-attachment:"

#if PTRACING
             "o-output:"             "-no-output."
             "t-trace."              "-no-trace."
#endif
  );

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif


  PIPSocketPool<MySocketHandler, PUDPSocket> pool;

  MyUDPSocket * sockets[100];
  memset(sockets, 0, sizeof(sockets));
  const unsigned count = sizeof(sockets) / sizeof(sockets[0]);

  for (PINDEX i = 0; i < count; ++i) {
    sockets[i] = new MyUDPSocket(); 
    pool.AddSocket(sockets[i], new MySocketHandler());
  }

  pool.Poll();

  cout << "handler finished" << endl;
}


// End of File ///////////////////////////////////////////////////////////////
