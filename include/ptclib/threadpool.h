/*
 * threadpool.h
 *
 * Generalised Thread Pooling functions
 *
 * Portable Tools Library
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


#ifndef PTLIB_THREADPOOL_H
#define PTLIB_THREADPOOL_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <map>


/*
 *  These classes and templates implement a generic thread pooling mechanism
 */

class PThreadPoolBase;

class PThreadPoolWorkerBase : public PThread
{
  public:
    PThreadPoolWorkerBase(PThreadPoolBase & threadPool);

    virtual unsigned GetWorkSize() const = 0;
    virtual void Shutdown() = 0;

  protected:
    PThreadPoolBase & pool;
    PBoolean shutdown;
    PMutex workerMutex;

  friend class PThreadPoolBase;
};


class PThreadPoolBase : public PObject
{
  public:
    PThreadPoolBase(unsigned maximum = 10);
    ~PThreadPoolBase();

    virtual PThreadPoolWorkerBase * CreateWorkerThread() = 0;

    virtual PThreadPoolWorkerBase * AllocateWorker();

  protected:
    virtual bool CheckWorker(PThreadPoolWorkerBase * worker);
    void StopWorker(PThreadPoolWorkerBase * worker);
    PMutex listMutex;
    typedef std::vector<PThreadPoolWorkerBase *> WorkerList_t;
    WorkerList_t workers;

    unsigned maxWorkerSize;
};


template <class WorkUnit_T, class WorkerThread_T>
class PThreadPool : public PThreadPoolBase
{
  PCLASSINFO(PThreadPool, PThreadPoolBase);
  public:
    typedef typename std::map<WorkUnit_T *, WorkerThread_T *> WorkUnitMap_T;

    PThreadPool(unsigned maximum = 10)
      : PThreadPoolBase(maximum) { }

    virtual PThreadPoolWorkerBase * CreateWorkerThread()
    { return new WorkerThread_T(*this); }

    bool AddWork(WorkUnit_T * workUnit)
    {
      PWaitAndSignal m(listMutex);

      PThreadPoolWorkerBase * a_worker = AllocateWorker();
      if (a_worker == NULL)
        return false;

      WorkerThread_T * worker = dynamic_cast<WorkerThread_T *>(a_worker);
      workUnitMap.insert(typename WorkUnitMap_T::value_type(workUnit, worker));

      worker->OnAddWork(workUnit);

      return true;
    }

    bool RemoveWork(WorkUnit_T * workUnit)
    {
      PWaitAndSignal m(listMutex);

      // find worker with work unit to remove
      typename WorkUnitMap_T::iterator r = workUnitMap.find(workUnit);
      if (r == workUnitMap.end())
        return false;

      WorkerThread_T * worker = dynamic_cast<WorkerThread_T *>(r->second);

      workUnitMap.erase(r);

      worker->OnRemoveWork(workUnit);

      CheckWorker(worker);

      return true;
    }

  protected:
    WorkUnitMap_T workUnitMap;
};


#endif // PTLIB_THREADPOOL_H


// End Of File ///////////////////////////////////////////////////////////////
