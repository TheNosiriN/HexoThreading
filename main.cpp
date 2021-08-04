
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

	return 0;
}
