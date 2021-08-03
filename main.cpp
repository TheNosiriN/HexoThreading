
#define DEBUG
#include "inc/HexoThreading.h"



using namespace Hexo;
using namespace Hexo::Threading;




static void threading_error_callback(HXRC_STATE state){
	std::cout << state.ErrorString << '\n';
	if (state.Code == HXRC_FATAL)exit(0);
}




int main() {

	Threading::SetErrorCallback(threading_error_callback);

	/// testing the error app
	HX_THREADING_ERROR_PRINT("Error App is working");

	return 0;
}
