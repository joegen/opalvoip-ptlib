/*
 * threadpool.cxx
 *
 * Generalised Thead Pool functions
 *
 * Portable Windows Library
 *
 * Copyright (C) 2009 Post Increment
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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "threadpool.h"
#endif


#include <ptlib.h>
#include <ptclib/threadpool.h>

#define new PNEW


PThreadPoolBase::PThreadPoolBase(unsigned int maxWorkerCount,
                                 unsigned int maxWorkUnitCount,
                                 const char * threadName,
                                 PThread::Priority priority)
  : m_maxWorkerCount(maxWorkerCount)
  , m_maxWorkUnitCount(maxWorkUnitCount)
#if PTRACING
  , m_highWaterMark(0)
#endif
  , m_threadName(threadName != NULL ? threadName : "Pool")
  , m_priority(priority)
{
}


void PThreadPoolBase::Shutdown()
{
  PTRACE(3, PThreadPoolTraceModule, "Shutting down thread pool \"" << m_threadName << '"');

  while (!m_workers.empty()) {
    m_mutex.Wait();
    WorkerThreadBase * worker = m_workers.front();
    m_workers.erase(m_workers.begin());
    m_mutex.Signal();

    StopWorker(worker);
  }
}


void PThreadPoolBase::SetMaxWorkers(unsigned count)
{
  m_mutex.Wait();
  bool needReclamation = m_maxWorkerCount > count;
  m_maxWorkerCount = count;
  m_mutex.Signal();

  if (needReclamation)
    ReclaimWorkers();
}


PThreadPoolBase::WorkerThreadBase * PThreadPoolBase::AllocateWorker()
{
  // Assumes m_mutex already locked

  // find the worker thread with the minimum number of work units
  // shortcut the search if we find an empty one
  WorkerList_t::iterator minWorker = m_workers.end();
  size_t minSizeFound = INT_MAX;
  for (WorkerList_t::iterator iter = m_workers.begin(); iter != m_workers.end(); ++iter) {
    WorkerThreadBase & worker = **iter;
    PWaitAndSignal m2(worker.m_workerMutex);
    if (!worker.m_shutdown && (worker.GetWorkSize() <= minSizeFound)) {
      minSizeFound = worker.GetWorkSize();
      if (minSizeFound == 0)
        return &worker; // if there is an idle worker, use it
      minWorker = iter;
    }
  }

  if (minWorker != m_workers.end()) {
    // if there is a per-worker limit, increase workers in quanta of the max worker count
    // otherwise only allow maximum number of workers
    if (m_maxWorkUnitCount > 0) {
      if (((m_workers.size() % m_maxWorkerCount) == 0) && (minSizeFound < m_maxWorkUnitCount))
        return *minWorker;
    }
    else {
      if (m_workers.size() >= m_maxWorkerCount)
        return *minWorker;
    }
  }

  // create a new worker thread
  WorkerThreadBase * worker = CreateWorkerThread();
  m_workers.push_back(PAssertNULL(worker));
  worker->Resume();
  return worker;
}


void PThreadPoolBase::ReclaimWorkers()
{
  WorkerThreadBase * stoppedWorker;
  do {
    stoppedWorker = NULL;

    m_mutex.Wait();

    // don't shut down the last thread, so we don't have the overhead of starting it up again
    if (m_workers.size() > 1) {
      for (WorkerList_t::iterator iter = m_workers.begin(); iter != m_workers.end(); ++iter) {
        WorkerThreadBase * worker = *iter;

        // if the worker thread has work, leave it alone
        // don't try and kill ourselves - just leave the thread for someone else to use
        if (worker->GetWorkSize() == 0 && worker != PThread::Current()) {
          stoppedWorker = worker;
          m_workers.erase(iter);
          break;
        }
      }
    }

    m_mutex.Signal();

    // Outside of mutex to assure deadlock freedom
    StopWorker(stoppedWorker);
  } while (stoppedWorker != NULL);

}


void PThreadPoolBase::StopWorker(WorkerThreadBase * worker)
{
  if (worker == NULL)
    return;

  PTRACE(4, PThreadPoolTraceModule, "Shutting down pool thread " << worker);
  worker->Shutdown();
  PAssert(worker->WaitForTermination(10000), "Worker did not terminate promptly");
  delete worker;
}


bool PThreadPoolBase::OnWorkerStarted(WorkerThreadBase & PTRACE_PARAM(thread))
{
#if PTRACING
  bool higherWatermark = m_workers.size() > m_highWaterMark;
  PTRACE(higherWatermark ? 2 : 3, PThreadPoolTraceModule,
         "Started new pool thread \"" << thread << "\", high water mark=" << m_workers.size());
  if (higherWatermark)
    m_highWaterMark = m_workers.size();
#endif
  return true;
}


PThreadPoolBase::WorkerThreadBase::WorkerThreadBase(PThreadPoolBase & pool, Priority priority, const char * threadName)
  : PThread(100, NoAutoDeleteThread, priority, threadName)
  , m_pool(pool)
  , m_shutdown(false)
{
}

void PThreadPoolBase::WorkerThreadBase::Main()
{
  if (m_pool.OnWorkerStarted(*this)) {
    while (Work())
      PTRACE(6, PThreadPoolTraceModule, "Processed work.");
  }
  PTRACE(2, PThreadPoolTraceModule, "Finished pool thread.");
}
