#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <vector>
#include <windows.h>
#include "detours.h"
#pragma comment(lib, "detours.lib")
extern "C" __declspec(dllexport) void hook(void) {}