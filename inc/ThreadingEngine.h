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

template<typename Func1, typename Func2>
void HX_Threading_SimpleThread(void* data, Func1 function, Func2 callback){
  function(data);
  callback(data);
}


void HX_Threading_SimpleWorkerThread(HXThreadCommunicator* tc){

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
  ThreadingEngine(){}
  ~ThreadingEngine(){ Release(); }

  /// Standard: All Engines must have a Release method for safe destruction
  void Release(){
    Threads.Release();
  }


private:
  /// All threads and pools are tracked
  ResourceList<HXThread_I> Threads;



public:

  /// spawn Immediate thread
  template<typename Func1, typename Func2>
  inline HXThread spawnImmediateThread(void* data, size_t size, Func1&& function, Func2&& callback){
    HXThread th = Threads.Insert( HXThread_I(Threads.Size()) );

    /// copy goes here
    void* newdata = std::malloc(size);
    memcpy(newdata, data, size);
    
    th->Data.sysThread = std::thread(HX_Threading_SimpleThread<Func1, Func2>, newdata, function, callback);
    return th;
  }



  /// spawn Worker thread
  inline HXThread spawnWorkerThread(){
    HXThread th = Threads.Insert( HXWorkerThread_I(Threads.Size()) );
    auto nth = reinterpret_cast<HXWorkerThread_I*>(&th->Data);
    th->Data.sysThread = std::thread(HX_Threading_SimpleWorkerThread, &(nth->tc) );
    return th;
  }

  // inline HXRC GiveWorkerTask(HXThreadTask )



  inline HXRC JoinThread(HXThread t){
    const char* str = std::string("Thread: "+std::to_string(t->Data.ID)+" Is not joinable").c_str();
    HX_THREADING_WARNING_ASSERT(t->Data.sysThread.joinable(), str);
    t->Data.sysThread.join();
    return HXRC_OK;
  }


};

/////////////////////////////////////////////////////////////////////////////////////////////////////





#endif /* end of include guard: HEXO_THREADING_ENGINE_H */
