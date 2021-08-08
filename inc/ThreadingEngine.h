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
void HX_Threading_SimpleWorkerThread(size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_WorkerTask>* queue, Func1 engineCallback){
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

    engineCallback(id);
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
typedef ResourceNode<THI_WorkerThread>* HXWorkerThread;
typedef ResourceNode<THI_ImmediateThread>* HXImmediateThread;



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

  /// All threads and pools are tracked
  ResourceList<THI_WorkerThread> WorkerThreads;
  ResourceList<THI_ImmediateThread> ImmediateThreads;



public:

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //////
  ////// Immediate Thread Functions
  //////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  template<typename Func1, typename Func2>
  inline HXImmediateThread spawnImmediateThread(void* data, size_t size, Func1&& function, Func2&& callback){
    return spawnImmediateThread(data, size, function, callback);
  }

  template<typename Func1, typename Func2>
  inline HXImmediateThread spawnImmediateThread(void* data, size_t size, Func1& function, Func2& callback){
    HXImmediateThread th = ImmediateThreads.Insert( THI_ImmediateThread(1) );

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

  inline HXWorkerThread spawnWorkerThread(){
    HXWorkerThread th = WorkerThreads.Insert( THI_WorkerThread(1) );

    th->Data.ID = reinterpret_cast<size_t>(th);
    th->Data.tc = new THI_ThreadCommunicator();
    // nth->tc.Terminated = false;

    auto engineCallback = [this](size_t& id){
      HXWorkerThread nth = reinterpret_cast<HXWorkerThread>(id);
      // detachThread(nth);
    };

    th->Data.sysThread = std::thread(
      HX_Threading_SimpleWorkerThread<decltype(engineCallback)>, th->Data.ID, th->Data.tc, &th->Data.taskQueue, engineCallback
    );
    
    return th;
  }


  inline HXRC AddWorkerTask(THI_WorkerThread* th, THI_WorkerTask&& tsk){
    std::lock_guard<std::mutex> lock(th->tc->mtx);

    th->taskQueue.push(tsk);

    th->tc->cv.notify_all();
    return HXRC_OK;
  }


  template<typename Func1, typename Func2>
  inline HXRC SubmitWorkerTask(HXWorkerThread th, void* data, size_t size, Func1&& function, Func2&& callback){
    return SubmitWorkerTask(th, data, size, function, callback);
  }

  template<typename Func1, typename Func2>
  inline HXRC SubmitWorkerTask(HXWorkerThread th, void* data, size_t size, Func1& function, Func2& callback){
    /// copy data
    void* newdata = std::malloc(size);
    memcpy(newdata, data, size);

    /// I REALLY didn't want to use std::function because of it's size but theres no helping it
    /// Theres no other way to store lambdas :(
    HXRC_RETURN(AddWorkerTask(
      &th->Data, THI_WorkerTask{ newdata, std::function<void(void*)>(function), std::function<void(void*)>(callback) }
    ));

    return HXRC_OK;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////








  /////////////////////////////////////////////////////////////////////////////////////////////////////

  template<class T>
  inline HXRC JoinThread(T t){
    HX_THREADING_WARNING_ASSERT(t->Data.sysThread.joinable(), std::string("Thread: "+std::to_string(t->Data.ID)+" Is not joinable").c_str());
    t->Data.sysThread.join();
    return HXRC_OK;
  }

  template<class T>
  inline HXRC detachThread(T t){
    t->Data.sysThread.detach();
    return HXRC_OK;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////////////////////////////////





#endif /* end of include guard: HEXO_THREADING_ENGINE_H */
