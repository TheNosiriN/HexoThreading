
#pragma once


#ifndef HEXO_THREADING_H
#define HEXO_THREADING_H




//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// Precompiled headers part
////// It's not on pch.h yet because of the messed up shared headers system
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>


////// Shared ///////
typedef size_t HXSIZE; // I will provide the HexoDefs header some time later, it's not ready for sharing
#include "Shared/HexoResourceList.h"
#include "Shared/HXRC.h"
////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////












//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// HXRC Wrappers
//////
/////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(HEXO_THREADING_STRICTMODE) || defined(HEXO_THREADING_ENABLE_ERRORS) || defined(DEBUG) || defined(_DEBUG)
    #define HX_THREADING_ERROR_PRINT(str) HXRC_PRINT(str, HX_THREADING_ERROR)
    #define HX_THREADING_ERROR_RETURN(str) HXRC_RETURN_FATAL(str, HX_THREADING_ERROR)
    #define HX_THREADING_WARNING_RETURN(str) HXRC_RETURN_WARNING(str, HX_THREADING_ERROR)
    #define HX_THREADING_ERROR_ASSERT(bool, str) HXRC_ASSERT_FATAL(bool, str, HX_THREADING_ERROR)
    #define HX_THREADING_WARNING_ASSERT(bool, str) HXRC_ASSERT_WARNING(bool, str, HX_THREADING_ERROR)
#else
    #define HX_THREADING_ERROR_PRINT(str)
    #define HX_THREADING_ERROR_RETURN(str) return HXRC_FATAL
    #define HX_THREADING_WARNING_RETURN(str) return HXRC_WARNING
    #define HX_THREADING_ERROR_ASSERT(bool, str)
    #define HX_THREADING_WARNING_ASSERT(bool, str)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////













namespace Hexo {


  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //////
  ////// Stuff thats meant to be under Threading namespace
  //////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  namespace Threading {

    /// Register Error App for threading engine
    struct ErrorApp {
      size_t hxrc_error_app = 0;
      static inline ErrorApp* Get(){ static ErrorApp e; return &e; }
    };

    #define HX_THREADING_ERROR Threading::ErrorApp::Get()->hxrc_error_app

    static inline void SetErrorCallback(void (*func)(HXRC_STATE)){
      HX_THREADING_ERROR = HXRC_REGISTER_APP(func);
    }




    struct HXThread_I {
      std::thread sysThread;
      HXSIZE ID = 0;
    };

    struct HXThreadEngineMinimal {
      bool Terminated = false;

    };



  };

  using namespace Threading;

  /////////////////////////////////////////////////////////////////////////////////////////////////////










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

    }


  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////




}; /// namespace Hexo











#endif /* end of include guard: HEXO_THREADING_H */
