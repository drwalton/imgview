#ifdef _MSC_VER
#include <Windows.h>
#endif
#include "ImgviewApp.hpp"
#include <iostream>
#include <chrono>

int main(int argc, char *argv[])
{
	if(argc != 2) {
		std::cout << "Usage: $ imgview [imageName]" << std::endl;
		return 0;
	}

	try {
		ImgviewApp app(argv[1]);
		app.run();
	} catch(std::runtime_error &err) {
		std::stringstream errMsg;
		errMsg << "Error encountered:\n" << err.what()
			 << "\nExiting..." << std::endl;
		std::cerr << errMsg.str();
#ifdef _MSC_VER
		MessageBox(NULL, errMsg.str().c_str(), "imgview Error", MB_OK);
#endif
		return 1;
	}

	return 0;
}
