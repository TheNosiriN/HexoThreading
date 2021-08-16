
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
#include <functional>
#include <string>
#include <queue>

//////////////////////////////


////// Shared ///////
typedef size_t HXSIZE; // I will provide the HexoDefs header some time later, it's not ready for sharing



// #include "Shared/HexoResourceList.h"
#include <memory>


namespace Hexo
{


	template<typename R>
	struct ResourceNode {
		R Data;
		ResourceNode<R>* PreviousResource = nullptr;
		ResourceNode<R>* NextResource = nullptr;

		~ResourceNode(){
			if (this->PreviousResource){ this->PreviousResource->NextResource = this->NextResource; }
			if (this->NextResource){ this->NextResource->PreviousResource = this->PreviousResource; }
		}
	};

	template<typename R>
	struct MinimalResourceNode {
		R Data;
	};



	template<typename R>
	struct ResourceIterator {
		ResourceNode<R>* Data = nullptr;

		ResourceIterator<R>& operator++(){
			if (Data){ Data = Data->NextResource; }
			return *this;
		}
		ResourceIterator<R>& operator--(){
			if (Data){ Data = Data->LastResource; }
			return *this;
		}
		ResourceIterator<R> operator++(int){
			ResourceIterator<R> t = *this;
			++*this; return t;
		}
		ResourceIterator<R> operator--(int){
			ResourceIterator<R> t = *this;
			--*this; return t;
		}

		bool operator!=(const ResourceIterator<R>& t){ return (this->Data != t.Data); }
		bool operator==(const ResourceIterator<R>& t){ return (this->Data == t.Data); }

		R* operator*() const {
			if (Data){ return &Data->Data; }
			return nullptr;
		}

	};



	template<typename R>
	struct ResourceList
	{
		ResourceList(){}
		~ResourceList(){ Release(); }

		ResourceNode<R>* FirstNode = nullptr;
		ResourceNode<R>* LastNode = nullptr;
		HXSIZE length = 0;

		const ResourceIterator<R> begin() const { return ResourceIterator<R>{ FirstNode }; }
		const ResourceIterator<R> end() const { return ResourceIterator<R>{ nullptr }; }

		const HXSIZE Size() const { return length; }


		ResourceNode<R>* Insert(R&& Data){
			return Insert(Data);
		}

		ResourceNode<R>* Insert(R& Data){
			ResourceNode<R>* n = new ResourceNode<R>{std::move(Data)};
			n->PreviousResource = LastNode;
			if (LastNode){ LastNode->NextResource = n; }
			LastNode = n;

			if (!FirstNode){ FirstNode = LastNode; }

			length += 1;
			return n;
		}

		void Remove(ResourceNode<R>*& pointer){
			if (!pointer)return;

			if (pointer->PreviousResource){ pointer->PreviousResource->NextResource = pointer->NextResource; }
			if (pointer->NextResource){ pointer->NextResource->PreviousResource = pointer->PreviousResource; }

			if (pointer == FirstNode){ FirstNode = pointer->NextResource; }
			if (pointer == LastNode){ LastNode = pointer->PreviousResource; }

			pointer->PreviousResource = nullptr;
			pointer->NextResource = nullptr;

			delete pointer;
			pointer = nullptr;
			if (length > 0){ length -= 1; }
		}

		void Release(){
			ResourceNode<R>* p = FirstNode;
			while (p != nullptr){
				ResourceNode<R>* n = p->NextResource;
				delete p;
				p = n;
			}

			this->FirstNode = nullptr;
			this->LastNode = nullptr;
			this->length = 0;
		}

	};

}




// #include "Shared/HXRC.h"

/////////////////////
#include <functional>
/////////////////////


typedef uint8_t HXRC;




enum : HXRC {
	HXRC_OK,
	HXRC_INFO,
	HXRC_FATAL,
	HXRC_WARNING,
};


struct HXRC_STATE {
	const char* ErrorString = nullptr;
	HXRC Code = 0;
};


struct HXRC_APP {
	std::function<void(HXRC_STATE)> func;
};


struct HXRC_SYS {
	std::vector<HXRC_APP> Apps;
	inline size_t AddApp(std::function<void(HXRC_STATE)> func){
		Apps.push_back(HXRC_APP{func});
		return Apps.size();
	}

	static inline HXRC_SYS* Get(){
		static HXRC_SYS h;
		return &h;
	}
	static inline HXRC_APP GetApp(size_t i){
		return HXRC_SYS::Get()->Apps.at(i-1);
	}
};


static inline size_t HXRC_REGISTER_APP(std::function<void(HXRC_STATE)> func){
	return HXRC_SYS::Get()->AddApp(std::ref(func));
}




#define HXRC_BUILD(app, string, code) {                  \
	HXRC c=code;                                           \
	if (app < 1){ return c; }                              \
	HXRC_SYS::GetApp(app).func(HXRC_STATE{string, c});     \
	return c;                                              \
}

#define HXRC_BUILD_NO_RETURN(app, string) {              				\
	if (app >= 1){                            						      	\
		HXRC_SYS::GetApp(app).func(HXRC_STATE{string, HXRC_INFO});  \
	}                                                      				\
}


#define HXRC_FAILED(x) (x != HXRC_OK)
#define HXRC_SUCCESS(x) (x == HXRC_OK)

#define HXRC_PRINT(string, app) HXRC_BUILD_NO_RETURN(app, string);

#define HXRC_RETURN(x) { HXRC c=x; if (c != HXRC_OK)return c; }
#define HXRC_RETURN_INFO(string, app) HXRC_BUILD(app, string, HXRC_INFO);
#define HXRC_RETURN_FATAL(string, app) HXRC_BUILD(app, string, HXRC_FATAL);
#define HXRC_RETURN_WARNING(string, app) HXRC_BUILD(app, string, HXRC_WARNING);

#define HXRC_ASSERT(x, string, app, code) if (!(x))HXRC_BUILD(app, string, code);
#define HXRC_ASSERT_INFO(x, string, app) if (!(x))HXRC_BUILD(app, string, HXRC_INFO);
#define HXRC_ASSERT_FATAL(x, string, app) if (!(x))HXRC_BUILD(app, string, HXRC_FATAL);
#define HXRC_ASSERT_WARNING(x, string, app) if (!(x))HXRC_BUILD(app, string, HXRC_WARNING);






////////////////////


////// Main Header //////
// #include "inc/Threading.h"






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

  // #include "inc/ThreadingEngine.h"


	using namespace Threading;




	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//////
	////// Thread-Scope Functions
	//////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename Func1, typename Func2>
	void HX_Threading_ImmediateThread(size_t id, void* data, Func1 function, Func2 engineCallback){
	  function(data);
	  std::free(data);
	  engineCallback(id);
	}


	template<typename Func1>
	void HX_Threading_WorkerThread(size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_WorkerTask>* queue, Func1 engineCallback){
	  std::unique_lock<std::mutex> lock(tc->mtx);

	  while (true){
	    tc->cv.wait(lock, [&queue, &tc](){
	      return (!queue->empty() || tc->Terminated);
	    });
	    if (tc->Terminated)break;

	    THI_WorkerTask task = std::move(queue->front());

	    lock.unlock();
	    task.WorkerFunction(task.Data);
	    std::free(task.Data);

	    lock.lock();
	    queue->pop();
	  }
	}


	template<typename Func1, typename Func2>
	void HX_Threading_DedicatedThread(
	  size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_DedicatedTask>* queue,
	  Func1 function, Func2 engineCallback
	){
	  std::unique_lock<std::mutex> lock(tc->mtx);

	  while (true){
	    tc->cv.wait(lock, [&queue, &tc](){
	      return (!queue->empty() || tc->Terminated);
	    });
	    if (tc->Terminated)break;

	    THI_DedicatedTask task = std::move(queue->front());

	    lock.unlock();
	    function(task.Data);
	    std::free(task.Data);

	    lock.lock();
	    queue->pop();

	  }
	}




	// template<typename Func1>
	// void HX_Threading_WorkerPoolThread(size_t id, THI_ThreadCommunicator* tc, MinimalQueue<THI_WorkerTask>* queue, Func1 engineCallback){
	//
	// }

	/////////////////////////////////////////////////////////////////////////////////////////////////////











	//////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//////
	////// Threading Engine class
	//////
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	/// define aliases
	#ifndef HEXO_THREADING_UNTRACKEDTHREADS
	  typedef ResourceNode<THI_WorkerThread>* HXWorkerThread;
	  typedef ResourceNode<THI_ImmediateThread>* HXImmediateThread;
	  typedef ResourceNode<THI_DedicatedThread>* HXDedicatedThread;
	#else
	  typedef MinimalResourceNode<THI_WorkerThread>* HXWorkerThread;
	  typedef MinimalResourceNode<THI_ImmediateThread>* HXImmediateThread;
	  typedef MinimalResourceNode<THI_DedicatedThread>* HXDedicatedThread;
	#endif

	typedef THI_WorkerThreadPool* HXWorkerThreadPool;
	typedef THI_DedicatedThreadPool* HXDedicatedThreadPool;



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
	    // std::cout << "Releasing" << '\n';
	#ifndef HEXO_THREADING_UNTRACKEDTHREADS
	    WorkerThreads.Release();
	    ImmediateThreads.Release();
	    DedicatedThreads.Release();
	#endif
	    delete GlobalCom;
	    GlobalCom = nullptr;
	  }


	private:
	  THI_ThreadCommunicator* GlobalCom = nullptr;



	#ifndef HEXO_THREADING_UNTRACKEDTHREADS
	  /// All threads are tracked
	  ResourceList<THI_WorkerThread> WorkerThreads;
	  ResourceList<THI_ImmediateThread> ImmediateThreads;
	  ResourceList<THI_DedicatedThread> DedicatedThreads;


	  #define HX_THREADING_WORKER_LIST_INSERT(data) this->WorkerThreads.Insert(data)
	  #define HX_THREADING_IMMEDIATE_LIST_INSERT(data) this->ImmediateThreads.Insert(data)
	  #define HX_THREADING_DEDICATED_LIST_INSERT(data) this->DedicatedThreads.Insert(data)

	  #define HX_THREADING_WORKER_LIST_REMOVE(inVar) this->WorkerThreads.Remove(inVar)
	  #define HX_THREADING_IMMEDIATE_LIST_REMOVE(inVar) this->ImmediateThreads.Remove(inVar)
	  #define HX_THREADING_DEDICATED_LIST_REMOVE(inVar) this->DedicatedThreads.Remove(inVar)
	#else
	  #define HX_THREADING_WORKER_LIST_INSERT(data) new MinimalResourceNode<THI_WorkerThread>{data}
	  #define HX_THREADING_IMMEDIATE_LIST_INSERT(data) new MinimalResourceNode<THI_ImmediateThread>{data}
	  #define HX_THREADING_DEDICATED_LIST_INSERT(data) new MinimalResourceNode<THI_DedicatedThread>{data}

	  #define HX_THREADING_WORKER_LIST_REMOVE(inVar) delete inVar
	  #define HX_THREADING_IMMEDIATE_LIST_REMOVE(inVar) delete inVar
	  #define HX_THREADING_DEDICATED_LIST_REMOVE(inVar) delete inVar
	#endif





	public:

	  //////////////////////////////////////////////////////////////////////////////////////////////////////
	  /////////////////////////////////////////////////////////////////////////////////////////////////////
	  //////
	  ////// Immediate Thread Functions
	  //////
	  /////////////////////////////////////////////////////////////////////////////////////////////////////

	  template<typename T, typename Func1>
	  inline HXImmediateThread SpawnImmediateThread(T& data, Func1&& function){
	    return SpawnImmediateThread(data, function);
	  }

	  template<typename T, typename Func1>
	  inline HXImmediateThread SpawnImmediateThread(T& data, Func1& function){
	    HXImmediateThread th = HX_THREADING_IMMEDIATE_LIST_INSERT( THI_ImmediateThread(1) );

	    /// the ID is the memory address
	    th->Data.ID = reinterpret_cast<size_t>(th);

	    /// copy data
	    size_t size = sizeof(data);
	    void* newdata = std::malloc(size);
	    memcpy(newdata, reinterpret_cast<void*>(&data), size);

	    auto engineCallback = [this](size_t& id){
	      HXImmediateThread nth = reinterpret_cast<HXImmediateThread>(id);
	      std::lock_guard<std::mutex> lock(this->GlobalCom->mtx);
	      HX_THREADING_IMMEDIATE_LIST_REMOVE(nth);
	    };

	    th->Data.sysThread = std::thread(
	      HX_Threading_ImmediateThread<Func1, decltype(engineCallback)>,
	      th->Data.ID, newdata, function, engineCallback
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
	      // std::cout << "here1" << '\n';
	      // HXWorkerThread nth = reinterpret_cast<HXWorkerThread>(id);
	      // std::unique_lock<std::mutex> lock(this->GlobalCom->mtx);
	      // HX_THREADING_WORKER_LIST_REMOVE(nth);
	      // std::cout << "here2" << '\n';
	    };

	    th->Data.sysThread = std::thread(
	      HX_Threading_WorkerThread<decltype(engineCallback)>, th->Data.ID, th->Data.tc, &th->Data.taskQueue, engineCallback
	    );

	    return th;
	  }


	  template<typename T, typename Func1>
	  inline HXRC SubmitTask(HXWorkerThread& th, T& data, Func1&& function){
	    return SubmitTask(th, data, function);
	  }

	  template<typename T, typename Func1>
	  inline HXRC SubmitTask(HXWorkerThread& th, T& data, Func1& function){
	    /// copy data
	    size_t size = sizeof(data);
	    void* newdata = std::malloc(size);
	    memcpy(newdata, reinterpret_cast<void*>(&data), size);

	    /// I REALLY didn't want to use std::function because of it's size but theres no helping it
	    /// Theres no other way to store lambdas :(
	    std::lock_guard<std::mutex> lock(th->Data.tc->mtx);
	    th->Data.taskQueue.push( THI_WorkerTask{newdata, std::function<void(void*)>(function)} );
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

	  template<typename Func1>
	  inline HXDedicatedThread SpawnDedicatedThread(Func1&& function){
	    return SpawnDedicatedThread(function);
	  }

	  template<typename Func1>
	  inline HXDedicatedThread SpawnDedicatedThread(Func1& function){
	    HXDedicatedThread th = HX_THREADING_DEDICATED_LIST_INSERT( THI_DedicatedThread(1) );

	    th->Data.ID = reinterpret_cast<size_t>(th);
	    th->Data.tc = new THI_ThreadCommunicator();

	    auto engineCallback = [this](size_t& id){
	      // HXDedicatedThread nth = reinterpret_cast<HXDedicatedThread>(id);
	      // std::unique_lock<std::mutex> lock(this->GlobalCom->mtx);
	      // HX_THREADING_DEDICATED_LIST_REMOVE(nth);
	    };

	    th->Data.sysThread = std::thread(
	      HX_Threading_DedicatedThread<Func1, decltype(engineCallback)>,
	      th->Data.ID, th->Data.tc, &th->Data.taskQueue, function, engineCallback
	    );

	    return th;
	  }


	  template<typename T>
	  inline HXRC SubmitTask(HXDedicatedThread& th, T& data){
	    /// copy data
	    size_t size = sizeof(data);
	    void* newdata = std::malloc(size);
	    memcpy(newdata, reinterpret_cast<void*>(&data), size);

	    std::lock_guard<std::mutex> lock(th->Data.tc->mtx);
	    th->Data.taskQueue.push( THI_DedicatedTask{newdata} );
	    th->Data.tc->cv.notify_all();

	    return HXRC_OK;
	  }

	  /////////////////////////////////////////////////////////////////////////////////////////////////////










	  //////////////////////////////////////////////////////////////////////////////////////////////////////
	  /////////////////////////////////////////////////////////////////////////////////////////////////////
	  //////
	  ////// Worker Thread Pool Functions
	  //////
	  /////////////////////////////////////////////////////////////////////////////////////////////////////

	  inline HXWorkerThreadPool SpawnWorkerPool(const HXSIZE Count){
	    HXWorkerThreadPool th = new THI_WorkerThreadPool(1, Count);
	    th->ID = reinterpret_cast<size_t>(th);

	    th->threads = new std::thread[Count];
	    th->tc = new THI_ThreadCommunicator();

	    auto engineCallback = [this](size_t& id){
	      // std::thread* nth = reinterpret_cast<std::thread*>(id);
	      // delete nth;
	    };

	    for (HXSIZE i=0; i<Count; ++i){
	      th->threads[i] = std::thread(HX_Threading_WorkerThread<decltype(engineCallback)>,
	        reinterpret_cast<size_t>(th->threads+i), th->tc, &th->taskQueue, engineCallback
	      );
	    }

	    return th;
	  }



	  template<typename T, typename Func1>
	  inline HXRC SubmitTask(HXWorkerThreadPool& th, T& data, Func1&& function){
	    return SubmitTask(th, data, function);
	  }

	  template<typename T, typename Func1>
	  inline HXRC SubmitTask(HXWorkerThreadPool& th, T& data, Func1& function){
	    /// copy data
	    size_t size = sizeof(data);
	    void* newdata = std::malloc(size);
	    memcpy(newdata, reinterpret_cast<void*>(&data), size);

	    std::lock_guard<std::mutex> lock(th->tc->mtx);
	    th->taskQueue.push( THI_WorkerTask{newdata, std::function<void(void*)>(function)} );
	    th->tc->cv.notify_one();

	    return HXRC_OK;
	  }

	  /////////////////////////////////////////////////////////////////////////////////////////////////////









	  //////////////////////////////////////////////////////////////////////////////////////////////////////
	  /////////////////////////////////////////////////////////////////////////////////////////////////////
	  //////
	  ////// Dedicated Thread Pool Functions
	  //////
	  /////////////////////////////////////////////////////////////////////////////////////////////////////

	  template<typename Func1>
	  inline HXDedicatedThreadPool SpawnDedicatedPool(HXSIZE Count, Func1&& function){
	    return SpawnDedicatedPool(Count, function);
	  }

	  template<typename Func1>
	  inline HXDedicatedThreadPool SpawnDedicatedPool(HXSIZE Count, Func1& function){
	    HXDedicatedThreadPool th = new THI_DedicatedThreadPool(1, Count);
	    th->ID = reinterpret_cast<size_t>(th);

	    th->threads = new std::thread[Count];
	    th->tc = new THI_ThreadCommunicator();

	    auto engineCallback = [this](size_t& id){
	      // std::thread* nth = reinterpret_cast<std::thread*>(id);
	      // delete nth;
	    };

	    for (HXSIZE i=0; i<Count; ++i){
	      th->threads[i] = std::thread(HX_Threading_DedicatedThread<Func1, decltype(engineCallback)>,
	        reinterpret_cast<size_t>(th->threads+i), th->tc, &th->taskQueue, function, engineCallback
	      );
	    }

	    return th;
	  }


	  template<typename T>
	  inline HXRC SubmitTask(HXDedicatedThreadPool& th, T& data){
	    /// copy data
	    size_t size = sizeof(data);
	    void* newdata = std::malloc(size);
	    memcpy(newdata, reinterpret_cast<void*>(&data), size);

	    std::lock_guard<std::mutex> lock(th->tc->mtx);
	    th->taskQueue.push( THI_DedicatedTask{newdata} );
	    th->tc->cv.notify_one();

	    return HXRC_OK;
	  }

	  /////////////////////////////////////////////////////////////////////////////////////////////////////










	  /////////////////////////////////////////////////////////////////////////////////////////////////////

	  inline HXRC JoinThread(HXImmediateThread th){
	    HX_THREADING_WARNING_ASSERT(th->Data.sysThread.joinable(), std::string("Thread: "+std::to_string(th->Data.ID)+" Is not joinable").c_str());
	    th->Data.sysThread.join();
	    return HXRC_OK;
	  }

	  inline HXRC DestroyThread(HXImmediateThread th){
	    th->Data.sysThread.join();
	    HX_THREADING_IMMEDIATE_LIST_REMOVE(th);
	    return HXRC_OK;
	  }

	  inline HXRC DestroyThread(HXWorkerThread th){
	    std::unique_lock<std::mutex> lock(th->Data.tc->mtx);
	    th->Data.tc->Terminated = true;
	    th->Data.tc->cv.notify_all();
	    lock.unlock();
	    th->Data.sysThread.join();
	    HX_THREADING_WORKER_LIST_REMOVE(th);
	    return HXRC_OK;
	  }

	  inline HXRC DestroyThread(HXDedicatedThread th){
	    std::unique_lock<std::mutex> lock(th->Data.tc->mtx);
	    th->Data.tc->Terminated = true;
	    th->Data.tc->cv.notify_all();
	    lock.unlock();
	    th->Data.sysThread.join();
	    HX_THREADING_DEDICATED_LIST_REMOVE(th);
	    return HXRC_OK;
	  }


	  inline HXRC DestroyPool(HXWorkerThreadPool th){
	    std::unique_lock<std::mutex> lock(th->tc->mtx);
	    th->tc->Terminated = true;
	    th->tc->cv.notify_all();
	    lock.unlock();
	    for (HXSIZE i=0; i<th->Count; i++){ th->threads[i].join(); }
	    delete th;
	    return HXRC_OK;
	  }

	  inline HXRC DestroyPool(HXDedicatedThreadPool th){
	    std::unique_lock<std::mutex> lock(th->tc->mtx);
	    th->tc->Terminated = true;
	    th->tc->cv.notify_all();
	    lock.unlock();
	    for (HXSIZE i=0; i<th->Count; i++){ th->threads[i].join(); }
	    delete th;
	    return HXRC_OK;
	  }




	  inline volatile size_t GetQueueSize(const HXWorkerThread th) const {
	    return th->Data.taskQueue.size();
	  }
	  inline volatile size_t GetQueueSize(const HXDedicatedThread th) const {
	    return th->Data.taskQueue.size();
	  }
	  inline volatile size_t GetQueueSize(const HXWorkerThreadPool th) const {
	    return th->taskQueue.size();
	  }
	  inline volatile size_t GetQueueSize(const HXDedicatedThreadPool th) const {
	    return th->taskQueue.size();
	  }

	  /////////////////////////////////////////////////////////////////////////////////////////////////////

	};

	/////////////////////////////////////////////////////////////////////////////////////////////////////



  /////////////////////////////////////////////////////////////////////////////////////////////////////



}; /// namespace Hexo
////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////







#endif /* end of include guard: HEXO_THREADING_H */
