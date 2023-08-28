#pragma once

#include <string>

class Platform
{
public:
	static void* GetDllHandle(std::string InString);
	static void* GetDllExport(void* InHandle, std::string InFuncName);
	static void	 FreeDllHandle(void* InHandle);
};
