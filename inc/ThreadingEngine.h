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
void HX_Threading_ImmediateThread(size_t id, THI_ThreadCommunicator* tc, void* data, Func1 function, Func2 callback, Func3 engineCallback){
  function(data);
  callback(data);
  engineCallback(id);
  std::free(data);
}


template<typename Func1>
void HX_Threading_WorkerThread(size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_WorkerTask>* queue, Func1 engineCallback){
  std::unique_lock<std::mutex> lock(tc->mtx);

  while (true){
    tc->cv.wait(lock, [&](){
      return (!queue->empty() || tc->Terminated);
    });
    if (tc->Terminated)break;

    THI_WorkerTask task = queue->front();
    task.WorkerFunction(task.Data);
    task.CallbackFunction(task.Data);
    std::free(task.Data);
    queue->pop();

    // engineCallback(id);
  }
}


template<typename Func1, typename Func2, typename Func3>
void HX_Threading_DedicatedThread(
  size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_DedicatedTask>* queue,
  Func1 function, Func2 callback, Func3 engineCallback
){
  std::unique_lock<std::mutex> lock(tc->mtx);

  while (true){
    tc->cv.wait(lock, [&](){
      return (!queue->empty() || tc->Terminated);
    });
    if (tc->Terminated)break;

    THI_DedicatedTask task = queue->front();
    function(task.Data);
    callback(task.Data);
    std::free(task.Data);
    queue->pop();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////











//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// Threading Engine class
//////
/////////////////////////////////////////////////////////////////////////////////////////////////////

/// define aliases
#ifdef HEXO_THREADING_TRACKTHREADS
  typedef ResourceNode<THI_WorkerThread>* HXWorkerThread;
  typedef ResourceNode<THI_ImmediateThread>* HXImmediateThread;
  typedef ResourceNode<THI_DedicatedThread>* HXDedicatedThread;
#else
  typedef MinimalResourceNode<THI_WorkerThread>* HXWorkerThread;
  typedef MinimalResourceNode<THI_ImmediateThread>* HXImmediateThread;
  typedef MinimalResourceNode<THI_DedicatedThread>* HXDedicatedThread;
#endif



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
    WorkerThreads.Release();
    ImmediateThreads.Release();
    delete GlobalCom;
  }


private:
  THI_ThreadCommunicator* GlobalCom = nullptr;



#ifdef HEXO_THREADING_TRACKTHREADS
  /// All threads and pools are tracked
  ResourceList<THI_WorkerThread> WorkerThreads;
  ResourceList<THI_ImmediateThread> ImmediateThreads;
  ResourceList<THI_DedicatedThread> DedicatedThreads;

  #define HX_THREADING_WORKER_LIST_INSERT(data) this->WorkerThreads.Insert(data)
  #define HX_THREADING_IMMEDIATE_LIST_INSERT(data) this->ImmediateThreads.Insert(data)
  #define HX_THREADING_DEDICATED_LIST_INSERT(data) this->DedicatedThreads.Insert(data)

  #define HX_THREADING_WORKER_LIST_REMOVE(th) this->WorkerThreads.Remove(th)
  #define HX_THREADING_IMMEDIATE_LIST_REMOVE(th) this->ImmediateThreads.Remove(th)
  #define HX_THREADING_DEDICATED_LIST_REMOVE(th) this->DedicatedThreads.Remove(th)
#else
  #define HX_THREADING_WORKER_LIST_INSERT(data) new HXWorkerThread{data}
  #define HX_THREADING_IMMEDIATE_LIST_INSERT(data) new HXImmediateThread{data}
  #define HX_THREADING_DEDICATED_LIST_INSERT(data) new HXDedicatedThread{data}

  #define HX_THREADING_WORKER_LIST_REMOVE(th) delete th;
  #define HX_THREADING_IMMEDIATE_LIST_REMOVE(th) delete th;
  #define HX_THREADING_DEDICATED_LIST_REMOVE(th) delete th;
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
      // this->Threads.Remove(nth);
    };

    th->Data.sysThread = std::thread(
      HX_Threading_ImmediateThread<Func1, Func2, decltype(engineCallback)>,
      th->Data.ID, this->GlobalCom, newdata,
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
      HXWorkerThread nth = reinterpret_cast<HXWorkerThread>(id);
      // detachThread(nth);
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
      HXDedicatedThread nth = reinterpret_cast<HXDedicatedThread>(id);
      // this->Threads.Remove(nth);
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










  /////////////////////////////////////////////////////////////////////////////////////////////////////

  inline HXRC JoinThread(HXImmediateThread t){
    HX_THREADING_WARNING_ASSERT(t->Data.sysThread.joinable(), std::string("Thread: "+std::to_string(t->Data.ID)+" Is not joinable").c_str());
    t->Data.sysThread.join();
    return HXRC_OK;
  }

  inline HXRC DestroyThread(HXImmediateThread th){
    HX_THREADING_IMMEDIATE_LIST_REMOVE(th);
    return HXRC_OK;
  }

  inline HXRC DestroyThread(HXWorkerThread th){
    std::lock_guard<std::mutex> lock(th->Data.tc->mtx);
    th->Data.tc->Terminated = true;
    th->Data.tc->cv.notify_all();
    HX_THREADING_WORKER_LIST_REMOVE(th);

    return HXRC_OK;
  }

  inline HXRC DestroyThread(HXDedicatedThread th){
    std::lock_guard<std::mutex> lock(th->Data.tc->mtx);
    th->Data.tc->Terminated = true;
    th->Data.tc->cv.notify_all();
    HX_THREADING_DEDICATED_LIST_REMOVE(th);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////////////////////////////////





#endif /* end of include guard: HEXO_THREADING_ENGINE_H */
