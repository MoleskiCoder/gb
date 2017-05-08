#include "stdafx.h"

#include "Configuration.h"
#include "Computer.h"

int main(int, char*[])
{
	Configuration configuration;

#ifdef _DEBUG
	//configuration.setDebugMode(true);
	//configuration.setProfileMode(true);
#endif
	//configuration.setDebugMode(true);

	Computer computer(configuration);
	computer.initialise();

	try {
		computer.runLoop();
	} catch (const std::exception& error) {
		::SDL_LogError(::SDL_LOG_CATEGORY_APPLICATION, "%s", error.what());
		return 2;
	}

	return 0;
}