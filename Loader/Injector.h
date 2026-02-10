#pragma once
#include <Windows.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

// Definitions for Manual Map
using f_LoadLibraryA = HINSTANCE(WINAPI *)(const char *lpLibFileName);
using f_GetProcAddress = UINT_PTR(WINAPI *)(HINSTANCE hModule,
                                            const char *lpProcName);
using f_DLL_ENTRY_POINT = BOOL(WINAPI *)(void *hDll, DWORD dwReason,
                                         void *pReserved);

struct MANUAL_MAPPING_DATA {
  f_LoadLibraryA pLoadLibraryA;
  f_GetProcAddress pGetProcAddress;
  HINSTANCE hMod;
};

class CInjector {
public:
  // Configure
  void SetDelay(int seconds) { m_iDelay = seconds; }

  // Main Actions
  bool Attach(const std::string &processName);
  bool ManualMap(const std::string &dllPath);

private:
  int m_iDelay = 0;
  HANDLE m_hProcess = nullptr;
  DWORD m_dwProcessId = 0;

  DWORD GetProcessIdByName(const std::string &processName);
};
