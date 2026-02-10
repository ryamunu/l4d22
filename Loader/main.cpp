#include "Injector.h"
#include <Windows.h>
#include <chrono>
#include <fstream>
#include <iostream>

void Log(const std::string &msg) {
  std::cout << "[ZeniiLoader] " << msg << std::endl;
}

int main() {
  SetConsoleTitleA("ZeniiLoader - L4D2 Undetected Injector");
  Log("Welcome to ZeniiLoader.");
  Log("Status: Waiting for 'left4dead2.exe'...");

  CInjector injector;

  // 1. Wait for Process
  while (!injector.Attach("left4dead2.exe")) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  Log("Found Left 4 Dead 2!");

  // 2. Initial Delay (Anti-Anti-Cheat Startup Scan)
  int delay = 15;
  std::cout << "[>] Configured Delay: " << delay << " seconds." << std::endl;
  std::cout << "[>] Waiting..." << std::endl;

  for (int i = delay; i > 0; i--) {
    std::cout << "\r[Time Remaining] " << i << "s   " << std::flush;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << std::endl;

  // 3. Inject
  Log("Injecting DLL...");

  // Assume DLL is in same folder
  char buffer[MAX_PATH];
  GetModuleFileNameA(NULL, buffer, MAX_PATH);
  std::string::size_type pos = std::string(buffer).find_last_of("\\/");
  std::string dllPath =
      std::string(buffer).substr(0, pos) + "\\astolfowareforl4d2.dll";

  std::fstream file(dllPath);
  if (!file.good()) {
    Log("[ERROR] DLL 'astolfowareforl4d2.dll' not found!");
    system("pause");
    return 0;
  }
  file.close();

  if (injector.ManualMap(dllPath)) {
    Log("[SUCCESS] Injection Complete!");
    Log("You may close this window.");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
  } else {
    Log("[FAILED] Injection Failed.");
    system("pause");
  }

  return 0;
}
