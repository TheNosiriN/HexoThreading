#pragma once



#ifndef HEXO_THREADING_MAIN_H
#define HEXO_THREADING_MAIN_H








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

    static inline void SetErrorCallback(std::function<void(HXRC_STATE)>&& func){
      HX_THREADING_ERROR = HXRC_REGISTER_APP(std::ref(func));
    }




    struct HXThreadCommunicator {
      bool Terminated = false;
      std::condition_variable cv;
      std::mutex mtx;
    };

    struct HXThread_I {
      HXThread_I(HXSIZE id){}
      ~HXThread_I(){}

      HXThread_I(HXThread_I&&) noexcept = default;

      std::thread sysThread;
      size_t ID = 0;
    };

    struct HXWorkerThread_I : HXThread_I {
      HXWorkerThread_I(HXSIZE id) : HXThread_I(id){}
      HXThreadCommunicator tc;
    };




  };

  /////////////////////////////////////////////////////////////////////////////////////////////////////










  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //////
  ////// Other stuff under the hexo namespace
  //////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  #include "inc/ThreadingEngine.h"

  /////////////////////////////////////////////////////////////////////////////////////////////////////



}; /// namespace Hexo




#endif /* end of include guard: HEXO_THREADING_MAIN_H */
