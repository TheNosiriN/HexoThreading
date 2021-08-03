
#pragma once


#ifndef HEXO_THREADING_H
#define HEXO_THREADING_H




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

  };


  using namespace Threading;



  ///Standard: Engines are declared outside the threading namespace of ease of access
  class ThreadingEngine
  {
    ThreadingEngine(){}
    ~ThreadingEngine(){ Release(); }

    /// Standard: All Engines must have a Release method for safe destruction
    void Release(){

    }



  };




}; /// namespace Hexo











#endif /* end of include guard: HEXO_THREADING_H */
