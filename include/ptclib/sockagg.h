/*
 * sockagg.h
 *
 * Generalised Socket Aggregation functions
 *
 * Portable Windows Library
 *
 * Copyright (C) 2005 Post Increment
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
 * Portions of this code were written with the financial assistance of 
 * Metreos Corporation (http://www.metros.com).
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: sockagg.h,v $
 * Revision 1.1  2005/12/22 03:55:52  csoutheren
 * Added initial version of socket aggregation classes
 *
 *
 */


#ifndef _SOCKAGG_H
#define _SOCKAGG_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>
#include <ptlib/sockets.h>

/*

These classes implements a generalised method for aggregating sockets so that they can be handled by a single thread. It is
intended to provide a backwards-compatible mechanism to supplant the "one socket - one thread" model used by OpenH323
and OPAL with a model that provides a better thread to call ratio. A more complete explanation of this model can be
found in the white paper "Increasing the Maximum Call Density of OpenH323 and OPAL" which can be at:

         http://www.voxgratia.org/docs/call%20thread%20handling%20model%201.0.pdf

There are two assumptions made by this code:

  1) The most efficient way to handle I/O is for a thread to be blocked on I/O. Any sort of timer or other
     polling mechanism is less efficient

  2) The time taken to handle a received PDU is relatively small, and will not interfere with the handling of
     other calls that are handled in the same thread

UDP and TCP sockets are aggregated in different ways. UDP sockets are aggregated on the basis of a simple loop that looks
for received datagrams and then processes them. TCP sockets are more complex because there is no intrinsic record-marking 
protocol, so it is difficult to know when a complete PDU has been received. This problem is solved by having the loop collect
received data into a buffer until the handling routine decides that a full PDU has been received.

At the heart of each socket aggregator is a select statement that contains all of the file descriptors that are managed
by the thread. One extra handle for a pipe (or on Windows, a local socket) is added to each handle list so that the thread can
be woken up in order to allow the addition or removal of sockets to the list

The basic aggregator mechanism 

*/

#include <ptlib.h>
#include <functional>
#include <vector>

class PAggregatorFDType 
{
  public:
#ifdef _WIN32
    class FdSet : public std::vector<HANDLE> 
    {
      public:
        FdSet()
        { reserve(MAXIMUM_WAIT_OBJECTS); }

        void AddFD(PAggregatorFDType fd, PAggregatorFDType &)
        { push_back((HANDLE)fd); }

        BOOL IsFDSet(PAggregatorFDType)
        { return FALSE;  }
    };

    PAggregatorFDType()
      : handle(0) { }

    PAggregatorFDType(SOCKET v)
      : handle((HANDLE)v) { }

    PAggregatorFDType(HANDLE v)
      : handle(v) { }

    //operator SOCKET () const
    //{ return handle; }

    operator HANDLE () const
    { return handle; }

    bool IsValid()
    { return (int)handle != -1; }

    HANDLE handle;

#else
    class FdSet : public FD_SET
    {
      public:
        FdSet()
        { FD_ZERO(this); }

        void AddFD(PAggregatorFDType fd, PAggregatorFDType & maxFd);
        {
          FD_SET(fd, this);
          maxFd = PMAX(maxFd, fd);
        }

        BOOL IsFDSet(PAggregatorFDType fd)
        { return fd.IsValid() && FD_ISSET(fd, this);  }
    };

    PAggregatorFDType(int v = 0)
      : handle(v) { }

    operator int () const
    { return handle; }

    bool IsValid()
    { return ((int)*this) >= 0; }

    int handle;
#endif
};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127)
#endif

class PAggregatedHandle
{
  public:
    PAggregatedHandle(BOOL _autoDelete = FALSE)
      : autoDelete(_autoDelete), preReadDone(FALSE)
    { }

    virtual PTimeInterval GetTimeout()
    { return PMaxTimeInterval; }

    virtual void AddFD(PAggregatorFDType::FdSet & fds, PAggregatorFDType & maxFd)
    { fds.AddFD(GetHandle(), maxFd); }

    virtual BOOL IsFDSet(PAggregatorFDType::FdSet & fds)
    { return fds.IsFDSet(GetHandle()); }

    virtual BOOL Init()      { return TRUE; }
    virtual BOOL PreRead()   { return TRUE; }
    virtual BOOL OnRead() = 0;
    virtual void DeInit()    { }

    BOOL autoDelete;
    BOOL preReadDone;

  private:
    virtual PAggregatorFDType GetHandle() const = 0;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif


class PHandleAggregator : public PObject
{
  PCLASSINFO(PHandleAggregator, PObject)
  public:
    class LocalPipeBase
    {
      public:
        virtual PAggregatorFDType GetHandle() = 0;
        virtual void Set() = 0;
        virtual void Reset() = 0;
    };

    typedef std::vector<PAggregatedHandle *> HandleContextList_t;

    class WorkerThreadBase : public PThread
    {
      public:
        WorkerThreadBase(LocalPipeBase & _pipe)
          : PThread(100, NoAutoDeleteThread), pipe(_pipe)
        { }

        virtual void Trigger() = 0;
        void Main();

        LocalPipeBase & pipe;
        PMutex mutex;

        HandleContextList_t handleList;
    };

    typedef std::vector<WorkerThreadBase *> WorkerList_t;

    PHandleAggregator(unsigned _max = 10);

    BOOL AddHandle(PAggregatedHandle * handle);

    BOOL RemoveHandle(PAggregatedHandle * handle);

    PMutex mutex;
    WorkerList_t workers;
    unsigned maxWorkerSize;
    unsigned minWorkerSize;
};


template <class PSocketType>
class PSocketAggregator : public PHandleAggregator
{
  PCLASSINFO(PSocketAggregator, PHandleAggregator)
  public:
    class AggregatedPSocket : public PAggregatedHandle
    {
      public:
        AggregatedPSocket(PSocketType * _s)
          : psocket(_s), fd(_s->GetHandle()) { }

        PAggregatorFDType GetHandle() const
        { return fd; }

        BOOL OnRead()
        { return psocket->OnRead(); }

      protected:
        PSocketType * psocket;
        PAggregatorFDType fd;
    };

    typedef std::map<PSocketType *, AggregatedPSocket *> SocketList_t;
    SocketList_t socketList;

    BOOL AddSocket(PSocketType * sock)
    { 
      PWaitAndSignal m(mutex);

      AggregatedPSocket * handle = new AggregatedPSocket(sock);
      if (AddHandle(handle)) {
        socketList.insert(SocketList_t::value_type(sock, handle));
        return true;
      }

      delete handle;
      return false;
    }

    BOOL RemoveSocket(PSocketType * sock)
    { 
      PWaitAndSignal m(mutex);

      SocketList_t::iterator r = socketList.find(sock);
      if (r == socketList.end()) 
        return FALSE;

      AggregatedPSocket * handle = r->second;
      RemoveHandle(handle);
      delete handle;
      socketList.erase(r);
      return TRUE;
    }
};

typedef PSocketAggregator<PUDPSocket> PUDPSocketAggregator;


#endif
