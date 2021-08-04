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


void HX_Threading_SimpleWorkerThread(void){
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////











//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// Threading Engine class
//////
/////////////////////////////////////////////////////////////////////////////////////////////////////

/// define alternate types
typedef ResourceNode<HXThread_I> HXThread;



/// Standard: Engines are declared outside the threading namespace of ease of access
class ThreadingEngine
{
  ThreadingEngine(){}
  ~ThreadingEngine(){ Release(); }

  /// Standard: All Engines must have a Release method for safe destruction
  void Release(){
    Threads.Release();
  }


  /// All threads and pools are tracked
  ResourceList<HXThread_I> Threads;



  /// spawn Immediate thread
  template<typename Func1, typename Func2>
  inline HXThread spawnImmediateThread(void* data, Func1&& function, Func2&& callback){
    HXThread th = Threads.Insert(HXThread_I{});
    th.data.ID = th.size() - 1;
    th.data.sysThread = std::thread(HX_Threading_SimpleThread<Func1, Func2>, data, function, callback);
    return th;
  }


  /// spawn Worker thread
  inline HXThread spawnWorkerThread(){
    HXThread th = Threads.Insert(HXThread_I{});
    th.data.ID = th.size() - 1;
    th.data.sysThread = std::thread(HX_Threading_SimpleWorkerThread);
    return th;
  }


};

/////////////////////////////////////////////////////////////////////////////////////////////////////





#endif /* end of include guard: HEXO_THREADING_ENGINE_H */
