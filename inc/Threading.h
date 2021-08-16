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


    struct THI_WorkerTask_I {
      virtual ~THI_WorkerTask_I() = 0;
    };


    struct THI_WorkerTask{
      void* Data;
      std::function<void(void*)> WorkerFunction;
    };

    struct THI_DedicatedTask {
      void* Data;
    };




    struct THI_Thread {
      THI_Thread(size_t id){ this->ID = id; }
      ~THI_Thread(){
        // std::cout << "help!!: " << ID << '\n';
      }

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
      THI_ImmediateThread(size_t id) : THI_Thread(id){}
      ~THI_ImmediateThread(){

        ///TODO: Find a better way to stop thread destuctor from being called
        /// THIS IS A MEMORY LEAK!!!!!!
        std::thread* temp = new std::thread();
        *temp = std::move(sysThread);
        ///////

      }
    };


    struct THI_WorkerThread : THI_Thread {
      THI_WorkerThread(size_t id) : THI_Thread(id){}
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
      THI_DedicatedThread(size_t id) : THI_Thread(id){}
      ~THI_DedicatedThread(){ delete tc; }

      THI_DedicatedThread(THI_DedicatedThread&& other) : THI_Thread(other){
        tc = other.tc;
        taskQueue = std::move(taskQueue);
        other.tc = nullptr;
      }

      THI_ThreadCommunicator* tc = nullptr;
      MinimalQueue<THI_DedicatedTask> taskQueue;
    };






    struct THI_WorkerThreadPool {
      THI_WorkerThreadPool(size_t id, HXSIZE count){
        this->ID = id;
        this->Count = count;
      }
      ~THI_WorkerThreadPool(){
        delete[] threads;
        delete tc;
      }

      THI_WorkerThreadPool(THI_WorkerThreadPool&& other){
        taskQueue = std::move(taskQueue);
        threads = other.threads;
        tc = other.tc;
        ID = other.ID;

        other.threads = nullptr;
        other.tc = nullptr;
        other.ID = 0;
      }

      std::thread* threads;
      THI_ThreadCommunicator* tc = nullptr;
      MinimalQueue<THI_WorkerTask> taskQueue;

      HXSIZE Count = 0;
      size_t ID = 0;
    };







    struct THI_DedicatedThreadPool {
      THI_DedicatedThreadPool(size_t id, HXSIZE count){
        this->ID = id;
        this->Count = count;
      }
      ~THI_DedicatedThreadPool(){
        delete[] threads;
        delete tc;
      }

      THI_DedicatedThreadPool(THI_DedicatedThreadPool&& other){
        taskQueue = std::move(taskQueue);
        threads = other.threads;
        tc = other.tc;
        ID = other.ID;

        other.threads = nullptr;
        other.tc = nullptr;
        other.ID = 0;
      }

      std::thread* threads;
      THI_ThreadCommunicator* tc = nullptr;
      MinimalQueue<THI_DedicatedTask> taskQueue;

      HXSIZE Count = 0;
      size_t ID = 0;
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
