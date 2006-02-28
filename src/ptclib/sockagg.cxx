/*
 * sockagg.cxx
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
 * $Log: sockagg.cxx,v $
 * Revision 1.11  2006/02/28 02:26:00  csoutheren
 * Renamed variable to be not same as member
 *
 * Revision 1.10  2006/02/28 02:08:02  csoutheren
 * Modified aggregation to load balance better
 *
 * Revision 1.9  2006/02/08 04:02:25  csoutheren
 * Added ability to enable and disable socket aggregation
 *
 * Revision 1.8  2006/01/23 05:57:39  csoutheren
 * More aggegator implementation
 *
 * Revision 1.7  2006/01/18 07:16:56  csoutheren
 * Latest version of socket aggregation code
 *
 * Revision 1.6  2006/01/05 11:39:32  rjongbloed
 * Fixed DevStudio warning
 *
 * Revision 1.5  2006/01/03 04:23:32  csoutheren
 * Fixed Unix implementation
 *
 * Revision 1.4  2005/12/23 07:49:27  csoutheren
 * Fixed Unix implementation
 *
 * Revision 1.3  2005/12/23 06:44:31  csoutheren
 * Working implementation
 *
 * Revision 1.2  2005/12/22 07:27:36  csoutheren
 * More implementation
 *
 * Revision 1.1  2005/12/22 03:55:52  csoutheren
 * Added initial version of socket aggregation classes
 *
 */

#ifdef __GNUC__
#pragma implementation "sockagg.h"
#endif


#include <ptlib.h>
#include <ptclib/sockagg.h>

#include <fcntl.h>

////////////////////////////////////////////////////////////////

#if _WIN32

class LocalEvent : public PHandleAggregator::EventBase
{
  public:
    LocalEvent()
    { 
      event = CreateEvent(NULL, TRUE, FALSE,NULL); 
      PAssert(event != NULL, "CreateEvent failed");
    }

    ~LocalEvent()
    { CloseHandle(event); }

    PAggregatorFD::FD GetHandle()
    { return event; }

    void Set()
    { SetEvent(event);  }

    void Reset()
    { ResetEvent(event); }

  protected:
    HANDLE event;
};

PAggregatorFD::PAggregatorFD(SOCKET v)
  : socket(v) 
{ 
  fd = WSACreateEvent(); 
  PAssert(WSAEventSelect(socket, fd, FD_READ | FD_CLOSE) == 0, "WSAEventSelect failed"); 
}

PAggregatorFD::~PAggregatorFD()
{ 
  WSACloseEvent(fd); 
}

bool PAggregatorFD::IsValid()
{ 
  return socket != INVALID_SOCKET; 
}

#else // #if _WIN32

class LocalEvent : public PHandleAggregator::EventBase
{
  public:
    LocalEvent()
    { ::pipe(fds); }

    virtual ~LocalEvent()
    {
      close(fds[0]);
      close(fds[1]);
    }

    PAggregatorFD::FD GetHandle()
    { return fds[0]; }

    void Set()
    { char ch; write(fds[1], &ch, 1); }

    void Reset()
    { char ch; read(fds[0], &ch, 1); }

  protected:
    int fds[2];
};

PAggregatorFD::PAggregatorFD(int v)
  : fd(v) 
{ 
}

PAggregatorFD::~PAggregatorFD()
{
}

bool PAggregatorFD::IsValid()
{ 
  return fd >= 0; 
}

#endif // #endif _WIN32
  

////////////////////////////////////////////////////////////////

PHandleAggregator::WorkerThreadBase::WorkerThreadBase(EventBase & _event)
  : PThread(100, NoAutoDeleteThread), event(_event), listChanged(TRUE), shutdown(FALSE)
{ 
}

class WorkerThread : public PHandleAggregator::WorkerThreadBase
{
  public:
    WorkerThread()
      : WorkerThreadBase(localEvent)
    { }

    void Trigger()
    { localEvent.Set(); }

    LocalEvent localEvent;
};


////////////////////////////////////////////////////////////////

PHandleAggregator::PHandleAggregator(unsigned _max)
  : maxWorkerSize(_max), minWorkerSize(1)
{ 
}

BOOL PHandleAggregator::AddHandle(PAggregatedHandle * handle)
{
  // perform the handle init function
  if (!handle->Init())
    return FALSE;

  PWaitAndSignal m(listMutex);

  // if the maximum number of worker threads has been reached, then
  // use the worker thread with the minimum number of handles
  if (workers.size() >= maxWorkerSize) {
    WorkerList_t::iterator minWorker = workers.end();
    size_t minSizeFound = 0x7ffff;
    WorkerList_t::iterator r;
    for (r = workers.begin(); r != workers.end(); ++r) {
      WorkerThreadBase & worker = **r;
      PWaitAndSignal m2(worker.workerMutex);
      if (!worker.shutdown && (worker.handleList.size() <= minSizeFound)) {
        minSizeFound = worker.handleList.size();
        minWorker     = r;
      }
    }

    // add the worker to the thread
    PAssert(minWorker != workers.end(), "could not find minimum worker");

    WorkerThreadBase & worker = **minWorker;
    PWaitAndSignal m2(worker.workerMutex);
    worker.handleList.push_back(handle);
    PTRACE(4, "SockAgg: Adding handle " << (void *)handle << " to aggregator - " << worker.handleList.size() << " handles");
    worker.listChanged = TRUE;
    worker.Trigger();
    return TRUE;
  }

  PTRACE(4, "SockAgg: Creating new aggregator for " << (void *)handle);

  // no worker threads usable, create a new one
  WorkerThread * worker = new WorkerThread;
  worker->handleList.push_back(handle);
  worker->Resume();
  workers.push_back(worker);

  return TRUE;
}

BOOL PHandleAggregator::RemoveHandle(PAggregatedHandle * handle)
{
  PWaitAndSignal m(listMutex);

  // look for the thread containing the handle we need to delete
  WorkerList_t::iterator r;
  for (r = workers.begin(); r != workers.end(); ++r) {
    WorkerThreadBase * worker = *r;

    // lock the worker
    worker->workerMutex.Wait();

    HandleContextList_t & hList = worker->handleList;

    // if handle is not in this thread, then continue searching
    HandleContextList_t::iterator s = find(hList.begin(), hList.end(), handle);
    if (s == hList.end()) {
      worker->workerMutex.Signal();
      continue;
    }

    // remove the handle from the worker's list of handles
    worker->handleList.erase(s);

    // do the de-init action
    handle->DeInit();

    // delete the handle if autodelete enabled
    if (handle->autoDelete)
      delete handle;

    // if the worker thread has enough handles to keep running, triger it to update
    if (worker->handleList.size() >= minWorkerSize) {
      PTRACE(4, "SockAgg: Removed handle " << (void *)handle << " from aggregator - " << worker->handleList.size() << " handles remaining");
      worker->listChanged = TRUE;
      worker->Trigger();
      worker->workerMutex.Signal();
      return TRUE;
    }

    PTRACE(4, "SockAgg: Removed handle " << (void *)handle << " from aggregator - merging remaining " << worker->handleList.size() << " handles to other aggregators");

    // make a copy of the list of handles controlled by this worker thread
    HandleContextList_t handlesToCopy = worker->handleList;
    while (worker->handleList.size() > 0)
      worker->handleList.erase(worker->handleList.begin());

    // shutdown the thread
    worker->shutdown = TRUE;
    worker->Trigger();
    worker->workerMutex.Signal();

    // the worker is now finished
    worker->WaitForTermination();
    delete worker;

    // remove the worker thread from the list of workers
    workers.erase(r);

    // add it's handles to other threads
    {
      HandleContextList_t::iterator t;
      for (t = handlesToCopy.begin(); t != handlesToCopy.end(); ++t)
        AddHandle(*t);
    }

    return TRUE;
  }

  PAssertAlways("Cannot find aggregator handle");

  return FALSE;
}

////////////////////////////////////////////////////////////////

typedef std::vector<PAggregatorFD::FD> fdList_t;
typedef std::vector<PAggregatorFD * > aggregatorFdList_t;
typedef std::map<PAggregatorFD::FD, PAggregatedHandle *> aggregatorFdToHandleMap_t;

#ifdef _WIN32
#define	FDLIST_SIZE	WSA_MAXIMUM_WAIT_EVENTS
#else
#define	FDLIST_SIZE	64
#endif

void PHandleAggregator::WorkerThreadBase::Main()
{
  PTRACE(4, "SockAgg: aggregator started");

  fdList_t                  fdList;
  aggregatorFdList_t        aggregatorFdList;
  aggregatorFdToHandleMap_t aggregatorFdToHandleMap;

  while (!shutdown) {

    // create the list of fds to wait on and find minimum timeout
    PTimeInterval timeout(PMaxTimeInterval);
    PAggregatedHandle * timeoutHandle = NULL;

#ifndef _WIN32
    fd_set rfds;
    FD_ZERO(&rfds);
    int maxFd = 0;
#endif

    {
      PWaitAndSignal m(workerMutex);

      // if the list of handles has changed, clear the list of handles
      if (listChanged) {
        aggregatorFdList.erase       (aggregatorFdList.begin(),      aggregatorFdList.end());
        aggregatorFdList.reserve     (FDLIST_SIZE);
        fdList.erase                 (fdList.begin(),                fdList.end());
        fdList.reserve               (FDLIST_SIZE);
        aggregatorFdToHandleMap.erase(aggregatorFdToHandleMap.begin(),         aggregatorFdToHandleMap.end());
      }

      HandleContextList_t::iterator r;
      for (r = handleList.begin(); r != handleList.end(); ++r) {
        PAggregatedHandle * handle = *r;

        if (handle->closed)
          continue;

        if (listChanged) {
          PAggregatorFDList_t fds = handle->GetFDs();
          PAggregatorFDList_t::iterator s;
          for (s = fds.begin(); s != fds.end(); ++s) {
            fdList.push_back          ((*s)->fd);
            aggregatorFdList.push_back((*s));
            aggregatorFdToHandleMap.insert(aggregatorFdToHandleMap_t::value_type((*s)->fd, handle));
          }
        }

        if (!handle->IsPreReadDone()) {
          handle->PreRead();
          handle->SetPreReadDone();
        }

        PTimeInterval t = handle->GetTimeout();
        if (t < timeout) {
          timeout = t;
          timeoutHandle = handle;
        }
      }

      // add in the event fd
      if (listChanged) {
        fdList.push_back(event.GetHandle());
        listChanged = FALSE;
      }

#ifndef _WIN32
      // create the list of FDs
      fdList_t::iterator s;
      for (s = fdList.begin(); s != fdList.end(); ++s) {
        FD_SET(*s, &rfds);
        maxFd = PMAX(maxFd, *s);
      }
#endif
    } // workerMutex goes out of scope

#ifdef _WIN32
    PTime wstart;
    DWORD nCount = fdList.size();
    DWORD ret = WSAWaitForMultipleEvents(nCount, &fdList[0], false, (DWORD)timeout.GetMilliSeconds(), FALSE);

    if (shutdown)
      break;

    if (ret == WAIT_FAILED) {
      DWORD err = WSAGetLastError();
      PTRACE(1, "SockAgg: WSAWaitForMultipleEvents error " << err);
    }

    {
      PWaitAndSignal m(workerMutex);

      if (ret == WAIT_TIMEOUT) {
        PTime start;
        timeoutHandle->closed = !timeoutHandle->OnRead();
        unsigned duration = (unsigned)(PTime() - start).GetMilliSeconds();
        if (duration > 50) {
          PTRACE(4, "SockAgg: Warning - aggregator read routine was of extended duration = " << duration << " msecs");
        }
        if (!timeoutHandle->closed)
          timeoutHandle->SetPreReadDone(FALSE);
      }

      else if (WAIT_OBJECT_0 <= ret && ret <= (WAIT_OBJECT_0 + nCount - 1)) {
        DWORD index = ret - WAIT_OBJECT_0;

        // if the event was triggered, redo the select
        if (index == nCount-1) {
          event.Reset();
          continue;
        }

        PAggregatorFD * fd = aggregatorFdList[index];
        PAssert(fdList[index] == fd->fd, "Mismatch in fd lists");
        aggregatorFdToHandleMap_t::iterator r = aggregatorFdToHandleMap.find(fd->fd);
        if (r != aggregatorFdToHandleMap.end()) {
          PAggregatedHandle * handle = r->second;

#ifdef _DEBUG
          {
            HandleContextList_t::iterator t = find(handleList.begin(), handleList.end(), handle);
            PAssert(t != handleList.end(), "reverse FD map failed");
          }
#endif
          WSANETWORKEVENTS events;
          WSAEnumNetworkEvents(fd->socket, fd->fd, &events);
          if (events.lNetworkEvents != 0) {
            if ((events.lNetworkEvents & FD_CLOSE) != 0)
              handle->closed = TRUE;
            else if ((events.lNetworkEvents & FD_READ) != 0) {
              PTime start;
              handle->closed = !handle->OnRead();
              unsigned duration = (unsigned)(PTime() - start).GetMilliSeconds();
              if (duration > 50) {
                PTRACE(4, "SockAgg: Warning - aggregator read routine was of extended duration = " << duration << " msecs");
              }
            }
            if (!handle->closed)
              handle->SetPreReadDone(FALSE);
            else {
              //handle->DeInit();
              //handlesToRemove.push_back(handle);
              listChanged = TRUE;
            }
          }
        }
      }

#else

    P_timeval pv = timeout;
    int ret = ::select(maxFd+1, &rfds, NULL, NULL, pv);

    if (ret < 0) {
      PTRACE(1, "SockAgg - Select failed with error " << errno);
    }

    // loop again if nothing was ready
    if (ret <= 0)
      continue;

    {
      PWaitAndSignal m(workerMutex);

      if (ret == 0) {
        PTime start;
        BOOL closed = !timeoutHandle->OnRead();
        unsigned duration = (unsigned)(PTime() - start).GetMilliSeconds();
        if (duration > 50) {
          PTRACE(4, "SockAgg: Warning - aggregator read routine was of extended duration = " << duration << " msecs");
        }
        if (!closed)
          timeoutHandle->SetPreReadDone(FALSE);
      }

      // check the event first
      else if (FD_ISSET(event.GetHandle(), &rfds)) {
        event.Reset();
        continue;
      }

      else {
        PAggregatorFD * fd = aggregatorFdList[ret];
        PAssert(fdList[ret] == fd->fd, "Mismatch in fd lists");
        aggregatorFdToHandleMap_t::iterator r = aggregatorFdToHandleMap.find(fd->fd);
        if (r != aggregatorFdToHandleMap.end()) {
          PAggregatedHandle * handle = r->second;
          PTime start;
          BOOL closed = !handle->OnRead();
          unsigned duration = (unsigned)(PTime() - start).GetMilliSeconds();
          if (duration > 50) {
            PTRACE(4, "SockAgg: Warning - aggregator read routine was of extended duration = " << duration << " msecs");
          }
          if (!closed)
            handle->SetPreReadDone(FALSE);
        }
      }

#endif
    } // workerMutex goes out of scope
  }

  PTRACE(4, "SockAgg: aggregator finished");
}

