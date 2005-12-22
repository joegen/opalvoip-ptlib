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
 * Revision 1.2  2005/12/22 07:27:36  csoutheren
 * More implementation
 *
 * Revision 1.1  2005/12/22 03:55:52  csoutheren
 * Added initial version of socket aggregation classes
 *
 */


#include <ptlib.h>
#include <ptclib/sockagg.h>

#include <fcntl.h>

////////////////////////////////////////////////////////////////

#if _WIN32

class LocalEvent : public PHandleAggregator::EventBase
{
  public:
    LocalEvent()
    { event = CreateEvent(NULL, TRUE, FALSE,NULL); }

    ~LocalEvent()
    { CloseHandle(event); }

    PAggregatorFD::Handle GetHandle()
    { return event; }

    void Set()
    { SetEvent(event);  }

    void Reset()
    { ResetEvent(event); }

  protected:
    HANDLE event;
};

void PAggregatorFD::FdSet::Clear()
{
  erase(begin(), end());
  reserve(MAXIMUM_WAIT_OBJECTS); 
}


void PAggregatorFD::FdSet::AddFD(HANDLE handle, int &)
{ 
  push_back(handle); 
}

BOOL PAggregatorFD::FdSet::IsFDSet(HANDLE)
{ 
  return FALSE;  
}

PAggregatorFD::PAggregatorFD(SOCKET v)
  : socket(v) 
{ 
  fd = WSACreateEvent(); 
  WSAEventSelect(socket, fd, FD_READ | FD_CLOSE); 
}

PAggregatorFD::~PAggregatorFD()
{ 
  WSACloseEvent(fd); 
}

bool PAggregatorFD::IsValid()
{ 
  return socket != INVALID_SOCKET; 
}

#else

class LocalEvent : public PHandleAggregator::EventBase
{
  public:
    LocalEvent()
    { ::pipe(fds); }

    ~LocalEvent()
    {
      close(fds[0]);
      close(fds[1]);
    }

    PAggregatorFD::Handle GetHandle()
    { return fds[0]; }

    void Set())
    { char ch; write(fds[1], &ch, 1); }

    int Reset()
    { char ch; read(fds[0], &ch, 1); }

  protected:
    int fds[2];
};

void PAggregatorFD::FdSet::Clear()
{
  FD_ZERO(this); 
}

void PAggregatorFD::FdSet::AddFD(PAggregatorFD::Handle handle, int & maxFd)
{
  FD_SET(handle, this);
  maxFd = PMAX(maxFd, handle);
}

BOOL PAggregatorFD::FdSet::IsFDSet(PAggregatorFD::Handle handle)
{ 
  return (handle >= 0) && FD_ISSET(handle, this);  
}

PAggregatorFD::PAggregatorFD(int v)
  : fd(v) 
{ 
}

bool PAggregatorFD::IsValid()
{ 
  return fd >= 0; 
}
#endif

////////////////////////////////////////////////////////////////

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
  PTRACE(4, "Adding handle to socket aggregator thread");

  // perform the handle init function
  if (!handle->Init())
    return FALSE;

  PWaitAndSignal m(mutex);

  // look for a worker thread to use that has less than the maximum number of handles
  WorkerList_t::iterator r;
  for (r = workers.begin(); r != workers.end(); ++r) {
    WorkerThreadBase & worker = **r;
    PWaitAndSignal m2(worker.mutex);
    if (worker.handleList.size() < maxWorkerSize) {
      worker.handleList.push_back(handle);
      worker.listChanged = TRUE;
      worker.Trigger();
      return TRUE;
    }
  }

  // no worker threads usable, create a new one
  //cout << "New worker thread created" << endl;
  WorkerThread * worker = new WorkerThread;
  worker->handleList.push_back(handle);
  worker->Resume();
  workers.push_back(worker);

  return TRUE;
}

BOOL PHandleAggregator::RemoveHandle(PAggregatedHandle * handle)
{
  PTRACE(4, "Removing handles from socket aggregator thread");

  PWaitAndSignal m(mutex);

  // look for the thread containing the handle we need to delete
  WorkerList_t::iterator r;
  for (r = workers.begin(); r != workers.end(); ++r) {
    WorkerThreadBase * worker = *r;

    // lock the worker
    worker->mutex.Wait();

    HandleContextList_t & hList = worker->handleList;

    // if handle is not in this thread, then continue searching
    HandleContextList_t::iterator s = find(hList.begin(), hList.end(), handle);
    if (s == worker->handleList.end()) {
      worker->mutex.Signal();
      continue;
    }

    // remove the handle from the worker's list of handled
    worker->handleList.erase(s);

    // if the worker thread has enough handles to keep running, trigger it to update
    if (worker->handleList.size() >= minWorkerSize) {
      worker->listChanged = TRUE;
      worker->Trigger();
      worker->mutex.Signal();
      return TRUE;
    }

    // remove the worker thread from the list of workers
    workers.erase(r);

    // add it's handles to other threads
    {
      HandleContextList_t::iterator t;
      for (t = hList.begin(); t != hList.end(); ++t)
        AddHandle(*t);
    }

    // trigger and unlock the worker
    worker->Trigger();
    worker->mutex.Signal();

    // the worker is now finished
    worker->WaitForTermination();
    delete worker;

    return TRUE;
  }

  return FALSE;
}

////////////////////////////////////////////////////////////////

void PHandleAggregator::WorkerThreadBase::Main()
{
  PTRACE(4, "Socket aggregator thread started");

  PAggregatorFD::FdSet rfds;

  for (;;) {

    // create the list of fds to wait on and minimum timeout
    PTimeInterval timeout(PMaxTimeInterval);
    PAggregatedHandle * timeoutHandle = NULL;

    int maxFd;
    {
      PWaitAndSignal m(mutex);

      // if no handles, then thread is no longer needed
      if (handleList.size() == 0)
        break;

      // if the list of handles has not changed, quickly find the shortest timeout
      if (!listChanged) {
        HandleContextList_t::iterator r;
        for (r = handleList.begin(); r != handleList.end(); ++r) {
          PAggregatedHandle * handle = *r;
          PTimeInterval t = handle->GetTimeout();
          if (t < timeout) {
            timeout = t;
            timeoutHandle = handle;
          }
        }
      }

      // otherwise, create the list of handles
      else
      {
        rfds.Clear();
        HandleContextList_t::iterator r;
        for (r = handleList.begin(); r != handleList.end(); ++r) {
          PAggregatedHandle * handle = *r;
          if (!handle->IsPreReadDone()) {
            handle->PreRead();
            handle->SetPreReadDone();
          }
          handle->AddFD(rfds, maxFd);
          PTimeInterval t = handle->GetTimeout();
          if (t < timeout) {
            timeout = t;
            timeoutHandle = handle;
          }
        }

        // add in the event fd
        rfds.AddFD(event.GetHandle(), maxFd);
      }
    }

    HandleContextList_t handlesToRemove;

#ifdef _WIN32
    DWORD nCount = rfds.size();
    DWORD ret = WSAWaitForMultipleEvents(nCount, &rfds[0], false, timeout.GetMilliSeconds(), FALSE);

    if (ret == WAIT_FAILED) {
      DWORD err = GetLastError();
      PTRACE(1, "WaitForMultipleObjects error " << err);
    }

    PWaitAndSignal m(mutex);
    PAggregatedHandle * handle = NULL;

    if (ret == WAIT_TIMEOUT)
      handle = timeoutHandle;

    else if (WAIT_OBJECT_0 <= ret && ret <= (WAIT_OBJECT_0 + nCount - 1)) {
      DWORD index = ret - WAIT_OBJECT_0;

      // if the event was triggered, redo the select
      if (index == nCount-1) {
        event.Reset();
        continue;
      }

      handle = handleList[index];
    }

    if (handle != NULL) {
      if (handle->OnRead()) 
        handle->SetPreReadDone(FALSE);
      else {
        handlesToRemove.push_back(handle);
        handle->DeInit();
      }
    }

#else
    P_timeval pv = timeout;
    int ret = ::select(maxFd+1, &rfds, NULL, NULL, NULL /* pv */);
    if (ret < 0) {
      int err = WSAGetLastError();
      PTRACE(1, "Select failed with error " << err);
    }

    // loop again if nothing was ready
    if (ret <= 0)
      continue;

    PWaitAndSignal m(mutex);

    // check the event first
    if (rfds.IsFDSet(event.GetHandle())) {
      event.Reset();
      continue;
    }

    // check all handles and collect a list of ones that closed
    {
      HandleContextList_t::iterator r;
      for (r = handleList.begin(); r != handleList.end(); ++r) {
        PAggregatedHandle * handle = *r;
        if (handle->IsFDSet(rfds)) {
          if (handle->OnRead()) 
            handle->preReadDone = FALSE;
          else {
            handlesToRemove.push_back(handle);
            handle->DeInit();
          }
        }
      }
    }
#endif

    // remove handles that are now closed
    while (handlesToRemove.begin() != handlesToRemove.end()) {
      PAggregatedHandle * handle = *handlesToRemove.begin();
      handlesToRemove.erase(handlesToRemove.begin());
      HandleContextList_t::iterator r = find(handleList.begin(), handleList.end(), handle);
      if (r != handleList.end())
        handleList.erase(r);
      if (handle->autoDelete) 
        delete handle;
    }
  }

  PTRACE(4, "Socket aggregator thread finished");
}
