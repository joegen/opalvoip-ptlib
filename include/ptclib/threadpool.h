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


/**

   These classes and templates implement a generic thread pooling mechanism
 
   To use them, declare the following:
      - A class that describes a "unit" of work to be performed. This class must use the PThreadPool
        as an ancestor and define the CreateWorkerThread member function
 
      - A class that described a worker thread within the pool. This class must be a descendant of 
        PThreadPoolWorkerBase and must define the following member functions:
 
            Constructor with one parameter declared as "PThreadPoolBase & threadPool"
            unsigned GetWorkSize() const;
            void OnAddWork(work_unit *);
            void OnRemoveWork(work_unit *);
            void Shutdown();
            void Main();
 
      - A class that describes the thread pool itself. This is defined using PThreadPool template

 
   Example declarations:

      struct MyWorkUnit {
        PString work;
      };

      class MyWorkerThread : public PThreadPoolWorkerBase
      {
        public:
          MyWorkerThread(PThreadPoolBase & threadPool)
            : PThreadPoolWorkerBase(threadPool) { }

          void Main();
          void Shutdown();
          unsigned GetWorkSize() const;
          void OnAddWork(MyWorkUnit * work);
          void OnRemoveWork(MyWorkUnit * work);
      };

      
      class SIPMainThreadPool : public PThreadPool<MyWorkUnit, MyWorkerThread>
      {
        public:
          virtual PThreadPoolWorkerBase * CreateWorkerThread()
          { return new MyWorkerThread(*this); }
      };

    The worker thread member functions operate as follows:

       Constructor 
          Called whenever a new worker thread is required

       void Main()
          Called when the worker thread starts up

       GetWorkSize()
          Called whenever the thread pool wants to know how "busy" the
          thread is. This is used when deciding how to allocate new work units
             
       OnAddWork(work_unit *)
          Called to add a new work unit to the thread

       void OnRemoveWork(work_unit *);
          Called to remove a work unit from the thread

       void Shutdown();
          Called to close down the worker thread

    The thread pool is used simply by instantiation as shown below. 

        MyThreadPool myThreadPool(10, 30);

    If the second parameter is zero, the first paramater sets the maximum number of worker threads that will be created.
    If the second parameter is not zero, this is the maximum number of work units each thread can handle. The first parameter
    is then the "quanta" in which worker threads will be allocated

    Once instantiated, the AddWork and RemoveWork member functions can be used to add and remove
    work units as required. The thread pool code will take care of starting, stopping and load balancing 
    worker threads as required.
   
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
    PThreadPoolBase(unsigned maxWorkerCount = 10, unsigned maxWorkUnitCount = 0);
    ~PThreadPoolBase();

    virtual PThreadPoolWorkerBase * CreateWorkerThread() = 0;

    virtual PThreadPoolWorkerBase * AllocateWorker();

  protected:
    virtual bool CheckWorker(PThreadPoolWorkerBase * worker);
    void StopWorker(PThreadPoolWorkerBase * worker);
    PMutex listMutex;
    typedef std::vector<PThreadPoolWorkerBase *> WorkerList_t;
    WorkerList_t workers;

    unsigned m_maxWorkerCount;
    unsigned m_maxWorkUnitCount;
};


template <class WorkUnit_T, class WorkerThread_T>
class PThreadPool : public PThreadPoolBase
{
  PCLASSINFO(PThreadPool, PThreadPoolBase);
  public:
    typedef typename std::map<WorkUnit_T *, WorkerThread_T *> WorkUnitMap_T;

    PThreadPool(unsigned maxWorkers = 10, unsigned maxWorkUnits = 0)
      : PThreadPoolBase(maxWorkers, maxWorkUnits) { }

    //virtual PThreadPoolWorkerBase * CreateWorkerThread()
    //{ return new WorkerThread_T(*this); }

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
