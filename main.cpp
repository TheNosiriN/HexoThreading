
#define DEBUG
///if you enable this you will have to destroy all threads and pools yourself
// #define HEXO_THREADING_UNTRACKEDTHREADS

#include "inc/HexoThreading.h"



using namespace Hexo;
using namespace Hexo::Threading;






int main() {
	/// testing error app with lambdas
	std::string s = "Threading Engine: ";

	Threading::SetErrorCallback([&](HXRC_STATE state){
		std::cout << s << state.ErrorString << '\n';
		if (state.Code == HXRC_FATAL)exit(0);
	});


	/// testing the error app
	HX_THREADING_ERROR_PRINT("Error App is Working");



	//spawn engine
	ThreadingEngine hxt = ThreadingEngine();





	/// testing immediate threads
	{
		int num =1645;

		HXImmediateThread t = hxt.SpawnImmediateThread(num,
			[](void* data){
				*(reinterpret_cast<int*>(data)) *= 10;
			}
		);

		hxt.JoinThread(t);
		std::cout << "Immediate thread: " << num << '\n';
	};
	////






	/// testing worker threads
	{
		HXWorkerThread t = hxt.SpawnWorkerThread();

		for (int i=0; i<10; i++){
			int num = 1645 + i;
			// void* message = reinterpret_cast<void*>(&num);

			hxt.SubmitTask(t, num,
				[](void* data){
					int num = *(reinterpret_cast<int*>(data)) * 10;
					std::cout << "Worker thread: " << num << '\n';
				}
			);
		}

		//wait for worker thread to callback
		while (true){
			volatile size_t s = hxt.GetQueueSize(t);
			if (!s)break;
		}


		hxt.DestroyThread(t);

	};
	/////





	/// testing dedicated threads
	{
		HXDedicatedThread t = hxt.SpawnDedicatedThread(
			[](void* data){
				int num = *(reinterpret_cast<int*>(data)) * 10;
				std::cout << "Dedicated thread: " << num << '\n';
			}
		);

		for (int i=0; i<10; i++){
			int num = 1645 + i;
			hxt.SubmitTask(t, num);
		}

		//wait for dedicated thread to callback
		while (true){
			volatile size_t s = hxt.GetQueueSize(t);
			if (!s)break;
		}


		hxt.DestroyThread(t);

	};
	/////





	/// testing worker threadpools
	{
		HXWorkerThreadPool t = hxt.SpawnWorkerPool(4);

		int* numarray = new int[10];

		for (int i=0; i<10; i++){
			numarray[i] = 1645 + i;

			hxt.SubmitTask(t, i,
				[&numarray](void* data){
					int i = *(reinterpret_cast<int*>(data));
					numarray[i] *= 10;
				}
			);
		}

		//wait for worker thread pool to callback
		while (true){
			volatile size_t s = hxt.GetQueueSize(t);
			if (!s)break;
		}

		for (size_t i = 0; i < 10; i++) {
			std::cout << "Worker pool: " << numarray[i] << '\n';
		}

	};
	/////





	/// testing dedicated threadpools
	{

	};
	/////





	/// testing engine destruction
	hxt.Release();


	return 0;
}
