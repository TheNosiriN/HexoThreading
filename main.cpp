
#define DEBUG
#include "inc/HexoThreading.h"



using namespace Hexo;
using namespace Hexo::Threading;






int main() {

	/// testing error app lambdas
	std::string s = "Errrssss";
	Threading::SetErrorCallback([&](HXRC_STATE state){
		std::cout << state.ErrorString << s << '\n';
		if (state.Code == HXRC_FATAL)exit(0);
	});


	/// testing the error app
	HX_THREADING_ERROR_PRINT("Error App is working");



	//spawn engine
	ThreadingEngine hxt = ThreadingEngine();



	/// testing immediate threads
	int num = 1645;
	void* message = reinterpret_cast<void*>(&num);

	HXThread t = hxt.spawnImmediateThread(message, sizeof(int),
		[](void* data){
			*(reinterpret_cast<int*>(data)) *= 10;
		},

		[](void* data){
			std::cout << *(reinterpret_cast<int*>(data)) << '\n';
		}
	);

	hxt.JoinThread(t);
	//



	return 0;
}
