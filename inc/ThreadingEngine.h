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
void HX_Threading_SimpleThread(HXThreadCommunicator* tc, size_t id, void* data, Func1 function, Func2 callback, Func3 engineCallback){
  function(data);
  callback(data);
  engineCallback(id);
  std::free(data);
}


void HX_Threading_SimpleWorkerThread(HXThreadCommunicator* tc, void** currentTask){
  std::unique_lock<std::mutex> lock(tc->mtx);

  while (true){
    tc->cv.wait(lock);
    if (tc->Terminated)break;

    // tc->
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////











//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// Threading Engine class
//////
/////////////////////////////////////////////////////////////////////////////////////////////////////

/// define alternate types
typedef ResourceNode<HXThread_I>* HXThread;



/// Standard: Engines are declared outside the threading namespace of ease of access
class ThreadingEngine
{
public:
  ThreadingEngine(){
    GlobalCom = new HXThreadCommunicator();
  }
  ~ThreadingEngine(){ Release(); }


  /// Standard: All Engines must have a Release method for safe destruction
  void Release(){
    Threads.Release();
    delete GlobalCom;
  }


private:
  HXThreadCommunicator* GlobalCom;

  /// All threads and pools are tracked
  ResourceList<HXThread_I> Threads;



public:

  /// spawn Immediate thread
  template<typename Func1, typename Func2>
  inline HXThread spawnImmediateThread(void* data, size_t size, Func1&& function, Func2&& callback){
    HXThread th = Threads.Insert( HXThread_I(1) );

    /// the ID is the memory address
    th->Data.ID = reinterpret_cast<size_t>(th);

    /// copy goes here
    void* newdata = std::malloc(size);
    memcpy(newdata, data, size);

    auto engineCallback = [this](size_t id){
      HXThread nth = reinterpret_cast<HXThread>(id);
      // this->Threads.Remove(nth);
    };

    th->Data.sysThread = std::thread(
      HX_Threading_SimpleThread<Func1, Func2, decltype(engineCallback)>,
      this->GlobalCom, th->Data.ID, newdata,
      function, callback, engineCallback
    );

    return th;
  }



  /// spawn Worker thread
  inline HXThread spawnWorkerThread(){
    HXThread th = Threads.Insert( HXWorkerThread_I(Threads.Size()) );
    auto nth = reinterpret_cast<HXWorkerThread_I*>(&th->Data);
    th->Data.sysThread = std::thread(HX_Threading_SimpleWorkerThread, &(nth->tc), &(nth->task));
    return th;
  }

  template<typename Func1, typename Func2>
  inline HXRC GiveWorkerTask(HXThread th, void* data, size_t size, Func1&& function, Func2&& callback){
    // th->Data.task = 

    return HXRC_OK;
  }




  inline HXRC JoinThread(HXThread t){
    HX_THREADING_WARNING_ASSERT(t->Data.sysThread.joinable(), std::string("Thread: "+std::to_string(t->Data.ID)+" Is not joinable").c_str());
    t->Data.sysThread.join();
    return HXRC_OK;
  }

  inline HXRC detachThread(HXThread t){
    t->Data.sysThread.detach();
    return HXRC_OK;
  }


};

/////////////////////////////////////////////////////////////////////////////////////////////////////





#endif /* end of include guard: HEXO_THREADING_ENGINE_H */
