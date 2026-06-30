#include "pch.h"
namespace Hooks {
    void Initialize();
    void Shutdown();
}
HMODULE g_hModule = nullptr;
DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    printf("[+] Pixel's Internal Loaded!\n");
    printf("[+] Waiting for game modules...\n");
    while (!GetModuleHandleA("client.dll") || !GetModuleHandleA("engine2.dll")) {
        Sleep(100);
    }
    printf("[+] client.dll: 0x%p\n", GetModuleHandleA("client.dll"));
    printf("[+] engine2.dll: 0x%p\n", GetModuleHandleA("engine2.dll"));
    Sleep(1000); 
    printf("[+] Initializing hooks...\n");
    Hooks::Initialize();
    printf("[+] Hooks initialized!\n");
    printf("[+] Press INSERT to toggle menu\n");
    printf("[+] Press END to unload\n");
    while (true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }
        Sleep(100);
    }
    printf("[+] Unloading...\n");
    Hooks::Shutdown();
    FreeConsole();
    FreeLibraryAndExitThread(g_hModule, 0);
    return 0;
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        g_hModule = hModule;
        HANDLE hThread = CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        if (hThread) {
            CloseHandle(hThread);
        }
    }
    return TRUE;
}
