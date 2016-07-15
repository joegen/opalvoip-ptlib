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


#include <ptlib/thread.h>
#include <ptlib/safecoll.h>
#include <map>
#include <queue>


/**

   These classes and templates implement a generic thread pooling mechanism

   There are two forms, low level and high level. For high level, it is assumed
   that there is a pool of threads each with a queue of work items to be
   processed. TO use simply decare a class containing the void Work() function
   and create the poolwith PQueuedThreadPool. e.g.

     class MyWork
     {
       void Work()
       {
         doIt();
       }
     }

     PQueuedThreadPool<MyWork> m_pool;

     m_pool.AddWork(new MyWork());


   To use low level, declare the following:

      - A class that describes a "unit" of work to be performed. 
 
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

       unsigned GetWorkSize()
          Called whenever the thread pool wants to know how "busy" the
          thread is. This is used when deciding how to allocate new work units
             
       void OnAddWork(work_unit *)
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

/** Base class for thread pools.
  */
class PThreadPoolBase : public PObject
{
  public:
    class WorkerThreadBase : public PThread
    {
      protected:
        WorkerThreadBase(Priority priority, const char * threadName)
          : PThread(100, NoAutoDeleteThread, priority, threadName)
          , m_shutdown(false)
        { }

      public:
        virtual void Shutdown() = 0;
        virtual unsigned GetWorkSize() const = 0;

        bool   m_shutdown;
        PMutex m_workerMutex;
    };

    class InternalWorkBase
    {
      public:
        InternalWorkBase(const char * group)
        { 
          if (group != NULL)
            m_group = group;
        }
        std::string m_group;
    };

    ~PThreadPoolBase()
    {
      Shutdown();
    }

    void Shutdown();
    virtual void ReclaimWorkers();

    unsigned GetMaxWorkers() const { return m_maxWorkerCount; }

    void SetMaxWorkers(
      unsigned count
    );

    unsigned GetMaxUnits() const { return m_maxWorkUnitCount; }

    void SetMaxUnits(
      unsigned count
    ) { m_maxWorkUnitCount = count; }

  protected:
    PThreadPoolBase(
      unsigned maxWorkerCount,
      unsigned maxWorkUnitCount,
      const char * threadName,
      PThread::Priority priority
    );

    virtual WorkerThreadBase * AllocateWorker();
    virtual WorkerThreadBase * CreateWorkerThread() = 0;
    virtual void StopWorker(WorkerThreadBase * worker);

    typedef std::vector<WorkerThreadBase *> WorkerList_t;
    WorkerList_t m_workers;
    PMutex       m_mutex;

    unsigned          m_maxWorkerCount;
    unsigned          m_maxWorkUnitCount;
#if PTRACING
    unsigned          m_highWaterMark; // For logging
#endif
    PString           m_threadName;
    PThread::Priority m_priority;
};


/** Low Level thread pool.
  */
template <class Work_T>
class PThreadPool : public PThreadPoolBase
{
  PCLASSINFO(PThreadPool, PThreadPoolBase);
  public:
    //
    //  constructor
    //
    PThreadPool(
      unsigned maxWorkers = 10,
      unsigned maxWorkUnits = 0,
      const char * threadName = NULL,
      PThread::Priority priority = PThread::NormalPriority
    ) : PThreadPoolBase(maxWorkers, maxWorkUnits, threadName, priority)
    { }

    //
    // define the ancestor of the worker thread
    //
    class WorkerThread : public WorkerThreadBase
    {
      protected:
        WorkerThread(PThreadPool & pool, Priority priority = NormalPriority, const char * threadName = NULL)
          : WorkerThreadBase(priority, threadName)
          , m_pool(pool)
        {
        }

      public:
        virtual void AddWork(Work_T * work, const string & group) = 0;
        virtual void RemoveWork(Work_T * work) = 0;
        virtual void Main() = 0;
  
      protected:
        PThreadPool & m_pool;
    };

    //
    // define internal worker wrapper class
    //
    class InternalWork : public InternalWorkBase
    {
      public:
        InternalWork(Work_T * work, const char * group)
          : InternalWorkBase(group)
          , m_work(work)
          , m_worker(NULL)
        { 
        }

        Work_T * m_work;
        WorkerThread * m_worker;
    };

    //
    // define map for external work units to internal work
    //
    typedef std::map<Work_T *, InternalWork> ExternalToInternalWorkMap_T;
    ExternalToInternalWorkMap_T m_externalToInternalWorkMap;


    //
    // define class for storing group informationm
    //
    struct GroupInfo {
      unsigned m_count;
      WorkerThread * m_worker;
      GroupInfo(WorkerThread * worker) : m_count(1), m_worker(worker) { }
    };


    //
    //  define map for group ID to group information
    //
    typedef std::map<std::string, GroupInfo> GroupInfoMap_t;
    GroupInfoMap_t m_groupInfoMap;


    //
    //  add a new unit of work to a worker thread
    //
    bool AddWork(Work_T * work, const char * group = NULL)
    {
      // create internal work structure
      InternalWork internalWork(work, group);

      typename GroupInfoMap_t::iterator iterGroup = m_groupInfoMap.end();

      PWaitAndSignal m(m_mutex);

      // allocate by group if specified, else allocate to least busy
      if (internalWork.m_group.empty() || (iterGroup = m_groupInfoMap.find(group)) == m_groupInfoMap.end()) {
        internalWork.m_worker = (WorkerThread *)AllocateWorker();

        // if cannot allocate worker, return
        if (internalWork.m_worker == NULL) 
          return false;

        // add group ID to map
        if (!internalWork.m_group.empty()) {
          m_groupInfoMap.insert(make_pair(internalWork.m_group, GroupInfo(internalWork.m_worker)));
          PTRACE(6, "ThreadPool", "Setting worker thread \"" << *internalWork.m_worker << "\""
                             " with group Id \"" << internalWork.m_group << '"');
        }
      }
      else {
        internalWork.m_worker = iterGroup->second.m_worker;
        ++iterGroup->second.m_count;
        PTRACE(6, "ThreadPool", "Using existing worker thread \"" << *internalWork.m_worker << "\""
                           " with group Id \"" << internalWork.m_group << "\", count=" << iterGroup->second.m_count);
      }

      // add work to external to internal map
      m_externalToInternalWorkMap.insert(make_pair(work, internalWork));

      // give the work to the worker
      internalWork.m_worker->AddWork(work, internalWork.m_group);
    
      return true;
    }

    //
    //  remove a unit of work from a worker thread
    //
    bool RemoveWork(Work_T * work)
    {
      PWaitAndSignal m(m_mutex);

      // find worker with work unit to remove
      typename ExternalToInternalWorkMap_T::iterator iterWork = m_externalToInternalWorkMap.find(work);
      if (!PAssert(iterWork != m_externalToInternalWorkMap.end(), "Missing work!"))
        return false;

      InternalWork & internalWork = iterWork->second;

      // update group information
      if (!internalWork.m_group.empty()) {
        typename GroupInfoMap_t::iterator iterGroup = m_groupInfoMap.find(internalWork.m_group);
        if (PAssert(iterGroup != m_groupInfoMap.end(), "Unknown work group") && --iterGroup->second.m_count == 0) {
          PTRACE(6, "ThreadPool", "Removing worker thread \"" << *internalWork.m_worker << "\""
                                  " from group Id \"" << internalWork.m_group << '"');
          m_groupInfoMap.erase(iterGroup);
        }
      }

      // tell worker to stop processing work
      internalWork.m_worker->RemoveWork(work);

      // remove element from work unit map
      m_externalToInternalWorkMap.erase(iterWork);

      return true;
    }
};


/** High Level (queued work item) thread pool.
  */
template <class Work_T>
class PQueuedThreadPool : public PThreadPool<Work_T>
{
  protected:
    PTimeInterval m_workerIncreaseLatency;
    unsigned      m_workerIncreaseLimit;
    PTime         m_nextWorkerIncreaseTime;

  public:
    //
    //  constructor
    //
    PQueuedThreadPool(
      unsigned maxWorkers = std::max(PThread::GetNumProcessors(), 10U),
      unsigned maxWorkUnits = 0,
      const char * threadName = NULL,
      PThread::Priority priority = PThread::NormalPriority,
      const PTimeInterval & workerIncreaseLatency = PMaxTimeInterval,
      unsigned workerIncreaseLimit = 0
    ) : PThreadPool<Work_T>(maxWorkers, maxWorkUnits, threadName, priority)
      , m_workerIncreaseLatency(workerIncreaseLatency)
      , m_workerIncreaseLimit(workerIncreaseLimit)
    {
        PTRACE(4, NULL, "ThreadPool", "Thread pool created:"
                                      " maxWorkers=" << maxWorkers << ","
                                      " maxWorkUnits=" << maxWorkUnits << ","
                                      " threadName=" << this->m_threadName << ","
                                      " priority=" << priority << ","
                                      " workerIncreaseLatency=" << workerIncreaseLatency << ","
                                      " workerIncreaseLimit=" << workerIncreaseLimit);
    }

    const PTimeInterval & GetWorkerIncreaseLatency() const { return m_workerIncreaseLatency; }
    void SetWorkerIncreaseLatency(const PTimeInterval & time) { m_workerIncreaseLatency = time; }
    unsigned GetWorkerIncreaseLimit() const { return m_workerIncreaseLimit; }
    void SetWorkerIncreaseLimit(unsigned limit) { m_workerIncreaseLimit = limit; }

    class QueuedWorkerThread : public PThreadPool<Work_T>::WorkerThread
    {
      public:
        QueuedWorkerThread(PThreadPool<Work_T> & pool,
                           PThread::Priority priority = PThread::NormalPriority,
                           const char * threadName = NULL)
          : PThreadPool<Work_T>::WorkerThread(pool, priority, threadName)
          , m_working(false)
        {
        }

        void AddWork(Work_T * work, const string & group)
        {
          if (PAssertNULL(work) != NULL)
            this->m_queue.Enqueue(QueuedWork(work, group));
        }

        void RemoveWork(Work_T * work)
        {
          delete work;
        }

        unsigned GetWorkSize() const
        {
          return (unsigned)this->m_queue.size()+this->m_working;
        }

        void Main()
        {
          QueuedWork item;
          while (this->m_queue.Dequeue(item)) {
            this->m_working = true;

            PQueuedThreadPool & pool = dynamic_cast<PQueuedThreadPool &>(this->m_pool);
            PTimeInterval latency = item.m_time.GetElapsed();

            item.m_work->Work();

            if (!pool.RemoveWork(item.m_work))
              this->RemoveWork(item.m_work);

            this->m_working = false;

            if (latency > pool.m_workerIncreaseLatency)
              pool.OnMaxWaitTime(latency, item.m_group);
          }
        }

        void Shutdown()
        {
          this->m_shutdown = true;
          this->m_queue.Close(true);
        }

      protected:
        struct QueuedWork
        {
          QueuedWork() : m_time(0), m_work(NULL) { }
          explicit QueuedWork(Work_T * work, const string & group) : m_work(work), m_group(group) { }
          PTime    m_time;
          Work_T * m_work;
          string   m_group;
        };
        PSyncQueue<QueuedWork> m_queue;
        bool                   m_working;
    };

    virtual void OnMaxWaitTime(const PTimeInterval & latency, const string & PTRACE_PARAM(group))
    {
      PTime now;
      if (this->m_nextWorkerIncreaseTime > now) {
        PTRACE(3, NULL, "ThreadPool", "Thread pool (group=\"" << group << "\") latency excessive"
               " (" << latency << "s > " << this->m_workerIncreaseLatency << "s),"
               " not increasing threads past " << this->m_maxWorkerCount << " due to recent adjustment");
        return;
      }

      this->m_nextWorkerIncreaseTime = now + latency; // Don't increase again until oafter this blockage removed.

      unsigned newMaxWorkers = std::min((this->m_maxWorkerCount*11+9)/10, this->m_workerIncreaseLimit);
      if (newMaxWorkers == this->m_maxWorkerCount) {
        PTRACE(2, NULL, "ThreadPool", "Thread pool (group=\"" << group << "\") latency excessive"
               " (" << latency << "s > " << this->m_workerIncreaseLatency << "s),"
               " cannot increase threads past " << this->m_workerIncreaseLimit);
        return;
      }

      PTRACE(2, NULL, "ThreadPool", "Thread pool (group=\"" << group << "\") latency excessive"
              " (" << latency << "s > " << this->m_workerIncreaseLatency << "s),"
              " increasing maximum threads from " << this->m_maxWorkerCount << " to " << newMaxWorkers);
      this->m_maxWorkerCount = newMaxWorkers;
    }

    virtual PThreadPoolBase::WorkerThreadBase * CreateWorkerThread()
    {
      return new QueuedWorkerThread(*this, this->m_priority, this->m_threadName);
    }
};


/**A PThreadPool work item template that uses PSafePtr to execute callback
   function.
  */
class PSafeWork : public PSafePtrBase {
  public:
    PSafeWork(
      PSafeObject * ptr
    ) : PSafePtrBase(ptr) { }

    virtual void Work()
    {
      PSafeObject * ptr = this->GetObject();
      if (ptr != NULL) {
        PTRACE_CONTEXT_ID_PUSH_THREAD(ptr);
        CallFunction(*ptr);
      }
    }

    virtual void CallFunction(PSafeObject & obj) = 0;
};


/// The thread pool for PSafeWork items.
typedef PQueuedThreadPool<PSafeWork> PSafeThreadPool;


/// A PSafeWork thread pool item where call back has no arguments.
template <class PtrClass, typename FuncRet = void>
class PSafeWorkNoArg : public PSafeWork
{
  PCLASSINFO_ALIGNED(PSafeWorkNoArg, PSafeWork, 16);

  public:
    typedef FuncRet (PtrClass::*Function)();

  protected:
    P_ALIGN_FIELD(Function,m_function,16);

  public:
    PSafeWorkNoArg(
      PtrClass * ptr,
      Function function
    ) : PSafeWork(ptr)
      , m_function(function)
    { }

    virtual void CallFunction(PSafeObject & obj)
    {
      (dynamic_cast<PtrClass&>(obj).*(this->m_function))();
    }
};


/// A PSafeWork thread pool item where call back has 1 argument.
template <
  class PtrClass,
  typename Arg1Type,
  typename FuncRet = void
>
class PSafeWorkArg1 : public PSafeWork
{
  PCLASSINFO_ALIGNED(PSafeWorkArg1, PSafeWork, 16);

  public:
    typedef FuncRet (PtrClass::*Function)(Arg1Type arg1);

  protected:
    P_ALIGN_FIELD(Function,m_function,16);
    Arg1Type m_arg1;

  public:
    PSafeWorkArg1(
      PtrClass * ptr,
      Arg1Type arg1,
      Function function
    ) : PSafeWork(ptr)
      , m_function(function)
      , m_arg1(arg1)
    { }

    virtual void CallFunction(PSafeObject & obj)
    {
      (dynamic_cast<PtrClass&>(obj).*(this->m_function))(m_arg1);
    }
};


/// A PSafeWork thread pool item where call back has 2 arguments.
template <
  class PtrClass,
  typename Arg1Type,
  typename Arg2Type,
  typename FuncRet = void
>
class PSafeWorkArg2 : public PSafeWork
{
  PCLASSINFO_ALIGNED(PSafeWorkArg2, PSafeWork, 16);

  public:
    typedef FuncRet (PtrClass::*Function)(Arg1Type arg1, Arg2Type arg2);

  protected:
    P_ALIGN_FIELD(Function,m_function,16);
    Arg1Type m_arg1;
    Arg2Type m_arg2;

  public:
    PSafeWorkArg2(
      PtrClass * ptr,
      Arg1Type arg1,
      Arg2Type arg2,
      Function function
    ) : PSafeWork(ptr)
      , m_function(function)
      , m_arg1(arg1)
      , m_arg2(arg2)
    { }

    virtual void CallFunction(PSafeObject & obj)
    {
      (dynamic_cast<PtrClass&>(obj).*(this->m_function))(m_arg1, m_arg2);
    }
};


/// A PSafeWork thread pool item where call back has 3 arguments.
template <
  class PtrClass,
  typename Arg1Type,
  typename Arg2Type,
  typename Arg3Type,
  typename FuncRet = void
>
class PSafeWorkArg3 : public PSafeWork
{
  PCLASSINFO_ALIGNED(PSafeWorkArg3, PSafeWork, 16);

  public:
    typedef FuncRet (PtrClass::*Function)(Arg1Type arg1, Arg2Type arg2, Arg3Type arg3);

  protected:
    P_ALIGN_FIELD(Function,m_function,16);
    Arg1Type m_arg1;
    Arg2Type m_arg2;
    Arg3Type m_arg3;

  public:
    PSafeWorkArg3(
      PtrClass * ptr,
      Arg1Type arg1,
      Arg2Type arg2,
      Arg2Type arg3,
      Function function
    ) : PSafeWork(ptr)
      , m_function(function)
      , m_arg1(arg1)
      , m_arg2(arg2)
      , m_arg3(arg3)
    { }

    virtual void CallFunction(PSafeObject & obj)
    {
      (dynamic_cast<PtrClass&>(obj).*(this->m_function))(m_arg1, m_arg2, m_arg3);
    }
};


#endif // PTLIB_THREADPOOL_H


// End Of File ///////////////////////////////////////////////////////////////
