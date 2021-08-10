#pragma once



#ifndef HEXO_THREADING_ENGINE_H
#define HEXO_THREADING_ENGINE_H





using namespace Threading;




//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// Thread-Scope Functions
//////
/////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Func1, typename Func2, typename Func3>
void HX_Threading_ImmediateThread(size_t id, void* data, Func1 function, Func2 callback, Func3 engineCallback){
  function(data);
  callback(data);
  std::free(data);
  engineCallback(id);
}


template<typename Func1>
void HX_Threading_WorkerThread(size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_WorkerTask>* queue, Func1 engineCallback){
  std::unique_lock<std::mutex> lock(tc->mtx);

  while (true){
    tc->cv.wait(lock, [&queue, &tc](){
      return (!queue->empty() || tc->Terminated);
    });
    if (tc->Terminated)break;

    THI_WorkerTask task = queue->front();

    lock.unlock();

    task.WorkerFunction(task.Data);
    task.CallbackFunction(task.Data);
    std::free(task.Data);

    lock.lock();
    queue->pop();

  }

  // lock.lock();
  // engineCallback(id);
}


template<typename Func1, typename Func2, typename Func3>
void HX_Threading_DedicatedThread(
  size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_DedicatedTask>* queue,
  Func1 function, Func2 callback, Func3 engineCallback
){
  std::unique_lock<std::mutex> lock(tc->mtx);

  while (true){
    tc->cv.wait(lock, [&queue, &tc](){
      return (!queue->empty() || tc->Terminated);
    });
    if (tc->Terminated)break;

    THI_DedicatedTask task = queue->front();

    lock.unlock();

    function(task.Data);
    callback(task.Data);
    std::free(task.Data);

    lock.lock();
    queue->pop();
  }

  // lock.lock();
  // engineCallback(id);
}




// template<typename Func1>
// void HX_Threading_WorkerPoolThread(size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_WorkerTask>* queue, Func1 engineCallback){
//
// }

/////////////////////////////////////////////////////////////////////////////////////////////////////











//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// Threading Engine class
//////
/////////////////////////////////////////////////////////////////////////////////////////////////////

/// define aliases
#ifndef HEXO_THREADING_UNTRACKEDTHREADS
  typedef ResourceNode<THI_WorkerThread>* HXWorkerThread;
  typedef ResourceNode<THI_ImmediateThread>* HXImmediateThread;
  typedef ResourceNode<THI_DedicatedThread>* HXDedicatedThread;
#else
  typedef MinimalResourceNode<THI_WorkerThread>* HXWorkerThread;
  typedef MinimalResourceNode<THI_ImmediateThread>* HXImmediateThread;
  typedef MinimalResourceNode<THI_DedicatedThread>* HXDedicatedThread;
#endif

typedef THI_WorkerThreadPool* HXWorkerThreadPool;
typedef THI_DedicatedThreadPool* HXDedicatedThreadPool;



/// NOTE: Engines are declared outside the threading namespace of ease of access
class ThreadingEngine
{
public:
  ThreadingEngine(){
    GlobalCom = new THI_ThreadCommunicator();
  }
  ~ThreadingEngine(){ Release(); }


  /// NOTE: All Engines must have a Release method for safe destruction
  void Release(){
#ifndef HEXO_THREADING_UNTRACKEDTHREADS
    WorkerThreads.Release();
    ImmediateThreads.Release();
    DedicatedThreads.Release();
#endif
    delete GlobalCom;
  }


private:
  THI_ThreadCommunicator* GlobalCom = nullptr;



#ifndef HEXO_THREADING_UNTRACKEDTHREADS
  /// All threads and pools are tracked
  ResourceList<THI_WorkerThread> WorkerThreads;
  ResourceList<THI_ImmediateThread> ImmediateThreads;
  ResourceList<THI_DedicatedThread> DedicatedThreads;


  #define HX_THREADING_WORKER_LIST_INSERT(data) this->WorkerThreads.Insert(data)
  #define HX_THREADING_IMMEDIATE_LIST_INSERT(data) this->ImmediateThreads.Insert(data)
  #define HX_THREADING_DEDICATED_LIST_INSERT(data) this->DedicatedThreads.Insert(data)

  #define HX_THREADING_WORKER_LIST_REMOVE(inVar) this->WorkerThreads.Remove(inVar)
  #define HX_THREADING_IMMEDIATE_LIST_REMOVE(inVar) this->ImmediateThreads.Remove(inVar)
  #define HX_THREADING_DEDICATED_LIST_REMOVE(inVar) this->DedicatedThreads.Remove(inVar)
#else
  #define HX_THREADING_WORKER_LIST_INSERT(data) new MinimalResourceNode<THI_WorkerThread>{data}
  #define HX_THREADING_IMMEDIATE_LIST_INSERT(data) new MinimalResourceNode<THI_ImmediateThread>{data}
  #define HX_THREADING_DEDICATED_LIST_INSERT(data) new MinimalResourceNode<THI_DedicatedThread>{data}

  #define HX_THREADING_WORKER_LIST_REMOVE(inVar) delete inVar
  #define HX_THREADING_IMMEDIATE_LIST_REMOVE(inVar) delete inVar
  #define HX_THREADING_DEDICATED_LIST_REMOVE(inVar) delete inVar
#endif





public:

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //////
  ////// Immediate Thread Functions
  //////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  template<typename Func1, typename Func2>
  inline HXImmediateThread SpawnImmediateThread(void* data, size_t size, Func1&& function, Func2&& callback){
    return SpawnImmediateThread(data, size, function, callback);
  }

  template<typename Func1, typename Func2>
  inline HXImmediateThread SpawnImmediateThread(void* data, size_t size, Func1& function, Func2& callback){
    HXImmediateThread th = HX_THREADING_IMMEDIATE_LIST_INSERT( THI_ImmediateThread(1) );

    /// the ID is the memory address
    th->Data.ID = reinterpret_cast<size_t>(th);

    /// copy data
    void* newdata = std::malloc(size);
    memcpy(newdata, data, size);

    auto engineCallback = [this](size_t& id){
      HXImmediateThread nth = reinterpret_cast<HXImmediateThread>(id);
      std::lock_guard<std::mutex> lock(this->GlobalCom->mtx);
      HX_THREADING_IMMEDIATE_LIST_REMOVE(nth);
    };

    th->Data.sysThread = std::thread(
      HX_Threading_ImmediateThread<Func1, Func2, decltype(engineCallback)>,
      th->Data.ID, newdata,
      function, callback, engineCallback
    );

    return th;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////









  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //////
  ////// Worker Thread Functions
  //////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  inline HXWorkerThread SpawnWorkerThread(){
    HXWorkerThread th = HX_THREADING_WORKER_LIST_INSERT( THI_WorkerThread(1) );

    th->Data.ID = reinterpret_cast<size_t>(th);
    th->Data.tc = new THI_ThreadCommunicator();

    auto engineCallback = [this](size_t& id){
      // std::cout << "here1" << '\n';
      // HXWorkerThread nth = reinterpret_cast<HXWorkerThread>(id);
      // std::unique_lock<std::mutex> lock(this->GlobalCom->mtx);
      // HX_THREADING_WORKER_LIST_REMOVE(nth);
      // std::cout << "here2" << '\n';
    };

    th->Data.sysThread = std::thread(
      HX_Threading_WorkerThread<decltype(engineCallback)>, th->Data.ID, th->Data.tc, &th->Data.taskQueue, engineCallback
    );

    return th;
  }


  template<typename Func1, typename Func2>
  inline HXRC SubmitTask(HXWorkerThread& th, void* data, size_t size, Func1&& function, Func2&& callback){
    return SubmitTask(th, data, size, function, callback);
  }

  template<typename Func1, typename Func2>
  inline HXRC SubmitTask(HXWorkerThread& th, void* data, size_t size, Func1& function, Func2& callback){
    /// copy data
    void* newdata = std::malloc(size);
    memcpy(newdata, data, size);

    /// I REALLY didn't want to use std::function because of it's size but theres no helping it
    /// Theres no other way to store lambdas :(
    std::lock_guard<std::mutex> lock(th->Data.tc->mtx);
    th->Data.taskQueue.push( THI_WorkerTask{newdata, std::function<void(void*)>(function), std::function<void(void*)>(callback)} );
    th->Data.tc->cv.notify_all();

    return HXRC_OK;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////









  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //////
  ////// Dedicated Thread Functions
  //////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  template<typename Func1, typename Func2>
  inline HXDedicatedThread SpawnDedicatedThread(Func1&& function, Func2&& callback){
    return SpawnDedicatedThread(function, callback);
  }

  template<typename Func1, typename Func2>
  inline HXDedicatedThread SpawnDedicatedThread(Func1& function, Func2& callback){
    HXDedicatedThread th = HX_THREADING_DEDICATED_LIST_INSERT( THI_DedicatedThread(1) );

    th->Data.ID = reinterpret_cast<size_t>(th);
    th->Data.tc = new THI_ThreadCommunicator();

    auto engineCallback = [this](size_t& id){
      // HXDedicatedThread nth = reinterpret_cast<HXDedicatedThread>(id);
      // std::unique_lock<std::mutex> lock(this->GlobalCom->mtx);
      // HX_THREADING_DEDICATED_LIST_REMOVE(nth);
    };

    th->Data.sysThread = std::thread(
      HX_Threading_DedicatedThread<Func1, Func2, decltype(engineCallback)>,
      th->Data.ID, th->Data.tc, &th->Data.taskQueue, function, callback, engineCallback
    );

    return th;
  }


  inline HXRC SubmitTask(HXDedicatedThread& th, void* data, size_t size){
    /// copy data
    void* newdata = std::malloc(size);
    memcpy(newdata, data, size);

    std::lock_guard<std::mutex> lock(th->Data.tc->mtx);
    th->Data.taskQueue.push( THI_DedicatedTask{newdata} );
    th->Data.tc->cv.notify_all();

    return HXRC_OK;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////










  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //////
  ////// Worker Thread Pool Functions
  //////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  inline HXWorkerThreadPool SpawnWorkerPool(const HXSIZE Count){
    HXWorkerThreadPool th = new THI_WorkerThreadPool(1, Count);
    th->ID = reinterpret_cast<size_t>(th);

    th->threads = new std::thread[Count];
    th->tc = new THI_ThreadCommunicator();

    auto engineCallback = [this](size_t& id){
      std::thread* nth = reinterpret_cast<std::thread*>(id);
      delete nth;
    };

    for (HXSIZE i=0; i<Count; ++i){
      th->threads[i] = std::thread(HX_Threading_WorkerThread<decltype(engineCallback)>,
        reinterpret_cast<size_t>(th->threads+i), th->tc, &th->taskQueue, engineCallback
      );
    }

    return th;
  }



  template<typename Func1, typename Func2>
  inline HXRC SubmitTask(HXWorkerThreadPool& th, void* data, size_t size, Func1&& function, Func2&& callback){
    return SubmitTask(th, data, size, function, callback);
  }

  template<typename Func1, typename Func2>
  inline HXRC SubmitTask(HXWorkerThreadPool& th, void* data, size_t size, Func1& function, Func2& callback){
    void* newdata = std::malloc(size);
    memcpy(newdata, data, size);

    std::lock_guard<std::mutex> lock(th->tc.mtx);
    th->taskQueue.push( THI_WorkerTask{newdata, std::function<void(void*)>(function), std::function<void(void*)>(callback)} );
    th->tc.cv.notify_one();

    return HXRC_OK;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////










  /////////////////////////////////////////////////////////////////////////////////////////////////////

  inline HXRC JoinThread(HXImmediateThread th){
    HX_THREADING_WARNING_ASSERT(th->Data.sysThread.joinable(), std::string("Thread: "+std::to_string(th->Data.ID)+" Is not joinable").c_str());
    th->Data.sysThread.join();
    return HXRC_OK;
  }

  inline HXRC DestroyThread(HXImmediateThread th){
    th->Data.sysThread.join();
    HX_THREADING_IMMEDIATE_LIST_REMOVE(th);
    return HXRC_OK;
  }

  inline HXRC DestroyThread(HXWorkerThread th){
    std::unique_lock<std::mutex> lock(th->Data.tc->mtx);
    th->Data.tc->Terminated = true;
    th->Data.tc->cv.notify_all();
    lock.unlock();
    th->Data.sysThread.join();
    HX_THREADING_WORKER_LIST_REMOVE(th);
    return HXRC_OK;
  }

  inline HXRC DestroyThread(HXDedicatedThread th){
    std::unique_lock<std::mutex> lock(th->Data.tc->mtx);
    th->Data.tc->Terminated = true;
    th->Data.tc->cv.notify_all();
    lock.unlock();
    th->Data.sysThread.join();
    HX_THREADING_DEDICATED_LIST_REMOVE(th);
    return HXRC_OK;
  }


  inline HXRC DestoryPool(HXWorkerThreadPool th){
    std::lock_guard<std::mutex> lock(th->tc->mtx);
    th->tc->Terminated = true;
    th->tc->cv.notify_all();
    delete th;
    return HXRC_OK;
  }

  inline HXRC DestoryPool(HXDedicatedThreadPool th){
    std::lock_guard<std::mutex> lock(th->tc->mtx);
    th->tc->Terminated = true;
    th->tc->cv.notify_all();
    delete th;
    return HXRC_OK;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////////////////////////////////





#endif /* end of include guard: HEXO_THREADING_ENGINE_H */
