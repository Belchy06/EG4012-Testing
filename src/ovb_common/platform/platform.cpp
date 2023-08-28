#include <windows.h>

#include "ovb_common/platform/platform.h"

void* Platform::GetDllHandle(std::string InString)
{
	return LoadLibrary(InString.c_str());
}

void* Platform::GetDllExport(void* InHandle, std::string InFuncName)
{
	return (void*)GetProcAddress((HMODULE)InHandle, InFuncName.c_str());
}

void Platform::FreeDllHandle(void* InHandle)
{
	FreeLibrary((HMODULE)InHandle);
}