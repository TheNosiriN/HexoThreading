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


  /// forgive me for this...
  /// I'll implement it some time later
  template<typename T> using MinimalQueue = std::queue<T>;
  //////



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




    struct THI_ThreadCommunicator {
      bool Terminated = false;
      std::condition_variable cv;
      std::mutex mtx;
    };


    struct THI_WorkerTask {
      void* Data;
      std::function<void(void*)> WorkerFunction;
      std::function<void(void*)> CallbackFunction;
    };

    struct THI_DedicatedTask {
      void* Data;
    };



    struct THI_Thread {
      THI_Thread(HXSIZE id){ this->ID = id; }
      ~THI_Thread(){
        // delete sysThread;
        std::cout << "/* message */" << '\n';
      }
      // THI_Thread(THI_Thread&&) noexcept = default;
      inline void move_thread_constructor(THI_Thread& other){
        sysThread = std::move(other.sysThread);
        ID = other.ID;
        other.ID = 0;
      }

      THI_Thread(THI_Thread& other){
        move_thread_constructor(other);
      }
      THI_Thread(THI_Thread&& other){
        move_thread_constructor(other);
      }

      std::thread sysThread;
      size_t ID = 0;
    };



    struct THI_ImmediateThread : THI_Thread {
      THI_ImmediateThread(HXSIZE id) : THI_Thread(id){}
    };

    struct THI_WorkerThread : THI_Thread {
      THI_WorkerThread(HXSIZE id) : THI_Thread(id){}
      ~THI_WorkerThread(){ delete tc; }

      THI_WorkerThread(THI_WorkerThread&& other) : THI_Thread(other){
        tc = other.tc;
        taskQueue = std::move(taskQueue);
        other.tc = nullptr;
      }

      THI_ThreadCommunicator* tc = nullptr;
      MinimalQueue<THI_WorkerTask> taskQueue;
    };

    struct THI_DedicatedThread : THI_Thread {
      THI_DedicatedThread(HXSIZE id) : THI_Thread(id){}
      ~THI_DedicatedThread(){ delete tc; }

      THI_DedicatedThread(THI_DedicatedThread&& other) : THI_Thread(other){
        tc = other.tc;
        taskQueue = std::move(taskQueue);
        other.tc = nullptr;
      }

      THI_ThreadCommunicator* tc = nullptr;
      MinimalQueue<THI_DedicatedTask> taskQueue;
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
