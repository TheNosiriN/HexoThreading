
#define DEBUG
#define HEXO_THREADING_TRACKTHREADS
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
	HX_THREADING_ERROR_PRINT("Error App");



	//spawn engine
	ThreadingEngine hxt = ThreadingEngine();




	/// testing immediate threads
	{
		int num = 1645;
		void* message = reinterpret_cast<void*>(&num);

		HXImmediateThread t = hxt.SpawnImmediateThread(message, sizeof(int),
			[](void* data){
				*(reinterpret_cast<int*>(data)) *= 10;
			},

			[&](void* data){
				num = *(reinterpret_cast<int*>(data));
			}
		);

		hxt.JoinThread(t);
		std::cout << "Immediate thread: " << num << '\n';
	};
	////






	/// testing worker threads
	{
		int num = 1645;
		volatile bool done = false;
		void* message = reinterpret_cast<void*>(&num);

		HXWorkerThread t = hxt.SpawnWorkerThread();

		hxt.SubmitTask(t, message, sizeof(int),
			[](void* data){
				*(reinterpret_cast<int*>(data)) *= 10;
			},

			[&](void* data){
				num = *(reinterpret_cast<int*>(data));
				done = true;
			}
		);

		//wait for worker thread to callback
		while (!done){}

		std::cout << "Worker thread: " << num << '\n';

	};
	/////






	/// testing dedicated threads
	{
		int num = 1645;
		volatile bool done = false;
		void* message = reinterpret_cast<void*>(&num);

		HXDedicatedThread t = hxt.SpawnDedicatedThread(
			[](void* data){
				*(reinterpret_cast<int*>(data)) *= 10;
			},

			[&](void* data){
				num = *(reinterpret_cast<int*>(data));
				done = true;
			}
		);

		hxt.SubmitTask(t, message, sizeof(int));

		//wait for dedicated thread to callback
		while (!done){}

		std::cout << "Dedicated thread: " << num << '\n';

	};
	/////



	// hxt.Release();


	return 0;
}
