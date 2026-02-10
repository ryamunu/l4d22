#include "Injector.h"
#include <TlHelp32.h>
#include <Windows.h>

// Shellcode struct
#include "Injector.h"
#include <TlHelp32.h>
#include <Windows.h>


#define RELOC_FLAG(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)

// Shellcode Function (Must be position independent)
void __stdcall Shellcode(MANUAL_MAPPING_DATA *pData) {
  if (!pData)
    return;

  BYTE *pBase = reinterpret_cast<BYTE *>(pData->hMod);
  auto *pOpt =
      &reinterpret_cast<IMAGE_NT_HEADERS *>(
           pBase + reinterpret_cast<IMAGE_DOS_HEADER *>(pData->hMod)->e_lfanew)
           ->OptionalHeader;

  auto _LoadLibraryA = pData->pLoadLibraryA;
  auto _GetProcAddress = pData->pGetProcAddress;
  auto _DllMain =
      reinterpret_cast<f_DLL_ENTRY_POINT>(pBase + pOpt->AddressOfEntryPoint);

  // Location delta
  BYTE *LocationDelta = pBase - pOpt->ImageBase;
  if (LocationDelta) {
    if (!pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
      return;

    auto *pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
        pBase +
        pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    while (pRelocData->VirtualAddress) {
      UINT AmountOfEntries =
          (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
          sizeof(WORD);
      WORD *pRelativeInfo = reinterpret_cast<WORD *>(pRelocData + 1);

      for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
        if (RELOC_FLAG(*pRelativeInfo)) {
          UINT_PTR *pPatch = reinterpret_cast<UINT_PTR *>(
              pBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
          *pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
        }
      }
      pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
          reinterpret_cast<BYTE *>(pRelocData) + pRelocData->SizeOfBlock);
    }
  }

  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
    auto *pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(
        pBase +
        pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (pImportDescr->Name) {
      char *szMod = reinterpret_cast<char *>(pBase + pImportDescr->Name);
      HINSTANCE hDll = _LoadLibraryA(szMod);

      ULONG_PTR *pThunkRef = reinterpret_cast<ULONG_PTR *>(
          pBase + pImportDescr->OriginalFirstThunk);
      ULONG_PTR *pFuncRef =
          reinterpret_cast<ULONG_PTR *>(pBase + pImportDescr->FirstThunk);

      if (!pThunkRef)
        pThunkRef = pFuncRef;

      for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
        if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
          *pFuncRef = _GetProcAddress(
              hDll, reinterpret_cast<char *>(*pThunkRef & 0xFFFF));
        } else {
          auto *pImport =
              reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(pBase + (*pThunkRef));
          *pFuncRef = _GetProcAddress(hDll, pImport->Name);
        }
      }
      ++pImportDescr;
    }
  }

  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
    auto *pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY *>(
        pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
    auto *pCallback =
        reinterpret_cast<PIMAGE_TLS_CALLBACK *>(pTLS->AddressOfCallBacks);
    for (; pCallback && *pCallback; ++pCallback)
      (*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
  }

  _DllMain(pBase, DLL_PROCESS_ATTACH, nullptr);

  pData->hMod = reinterpret_cast<HINSTANCE>(pBase);
}

bool CInjector::Attach(const std::string &processName) {
  m_dwProcessId = GetProcessIdByName(processName);
  if (!m_dwProcessId)
    return false;

  m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwProcessId);
  return (m_hProcess != nullptr);
}

DWORD CInjector::GetProcessIdByName(const std::string &processName) {
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

  if (Process32First(snapshot, &entry) == TRUE) {
    while (Process32Next(snapshot, &entry) == TRUE) {
      if (std::string(entry.szExeFile) == processName) {
        CloseHandle(snapshot);
        return entry.th32ProcessID;
      }
    }
  }

  CloseHandle(snapshot);
  return 0;
}

bool CInjector::ManualMap(const std::string &dllPath) {
  if (!m_hProcess)
    return false;

  // 1. Read File
  std::ifstream File(dllPath, std::ios::binary | std::ios::ate);
  if (File.fail()) {
    std::cout << "[!] Failed to open DLL file!" << std::endl;
    return false;
  }

  auto FileSize = File.tellg();
  if (FileSize < 0x1000) {
    std::cout << "[!] Invalid File Size!" << std::endl;
    File.close();
    return false;
  }

  BYTE *pSrcData = new BYTE[(UINT_PTR)FileSize];
  if (!pSrcData) {
    File.close();
    return false;
  }

  File.seekg(0, std::ios::beg);
  File.read(reinterpret_cast<char *>(pSrcData), FileSize);
  File.close();

  // 2. Parse Headers
  auto *pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER *>(pSrcData);
  if (pDosHeader->e_magic != 0x5A4D) {
    delete[] pSrcData;
    return false;
  }

  auto *pOldNtHeader =
      reinterpret_cast<IMAGE_NT_HEADERS *>(pSrcData + pDosHeader->e_lfanew);
  auto *pOldOptHeader = &pOldNtHeader->OptionalHeader;
  auto *pOldFileHeader = &pOldNtHeader->FileHeader;

  // 3. Allocate Memory in Target
  BYTE *pTargetBase = reinterpret_cast<BYTE *>(
      VirtualAllocEx(m_hProcess, nullptr, pOldOptHeader->SizeOfImage,
                     MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
  if (!pTargetBase) {
    std::cout << "[!] Memory Allocation Failed (0x" << GetLastError() << ")"
              << std::endl;
    delete[] pSrcData;
    return false;
  }

  // 4. Copy Sections
  MANUAL_MAPPING_DATA data{0};
  data.pLoadLibraryA = LoadLibraryA;
  data.pGetProcAddress = reinterpret_cast<f_GetProcAddress>(GetProcAddress);

  // Copy Headers
  if (!WriteProcessMemory(m_hProcess, pTargetBase, pSrcData, 0x1000,
                          nullptr)) { // Standard headers size
    VirtualFreeEx(m_hProcess, pTargetBase, 0, MEM_RELEASE);
    delete[] pSrcData;
    return false;
  }

  auto *pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
  for (UINT i = 0; i != pOldFileHeader->NumberOfSections;
       ++i, ++pSectionHeader) {
    if (pSectionHeader->SizeOfRawData) {
      if (!WriteProcessMemory(m_hProcess,
                              pTargetBase + pSectionHeader->VirtualAddress,
                              pSrcData + pSectionHeader->PointerToRawData,
                              pSectionHeader->SizeOfRawData, nullptr)) {
        VirtualFreeEx(m_hProcess, pTargetBase, 0, MEM_RELEASE);
        delete[] pSrcData;
        return false;
      }
    }
  }

  // 5. Inject Shellcode/Loader
  // We allocate memory for the loader code itself
  void *pShellcode =
      VirtualAllocEx(m_hProcess, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE,
                     PAGE_EXECUTE_READWRITE);
  if (!pShellcode) {
    VirtualFreeEx(m_hProcess, pTargetBase, 0, MEM_RELEASE);
    delete[] pSrcData;
    return false;
  }

  WriteProcessMemory(m_hProcess, pShellcode, Shellcode, 0x1000,
                     nullptr); // Write function

  // 6. Inject Data
  // Write Mapping Data structure
  void *pData = VirtualAllocEx(m_hProcess, nullptr, sizeof(MANUAL_MAPPING_DATA),
                               MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  data.hMod = reinterpret_cast<HINSTANCE>(pTargetBase);
  WriteProcessMemory(m_hProcess, pData, &data, sizeof(MANUAL_MAPPING_DATA),
                     nullptr);

  // 7. Execute Shellcode
  HANDLE hThread = CreateRemoteThread(
      m_hProcess, nullptr, 0,
      reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode), pData, 0, nullptr);
  if (!hThread) {
    std::cout << "[!] Thread Creation Failed" << std::endl;
    VirtualFreeEx(m_hProcess, pTargetBase, 0, MEM_RELEASE);
    VirtualFreeEx(m_hProcess, pShellcode, 0, MEM_RELEASE);
    VirtualFreeEx(m_hProcess, pData, 0, MEM_RELEASE);
    delete[] pSrcData;
    return false;
  }

  CloseHandle(hThread);
  delete[] pSrcData;
  return true;
}
