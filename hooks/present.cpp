#include "pch.h"
#include "../skin_changer.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>
namespace Visuals {
    struct Tracer {
        Vector3 startPos;
        Vector3 endPos;
        float spawnTime;
        float totalDist;
    };
    inline std::vector<Tracer> g_tracers;
    inline std::mutex g_tracerMutex;
    inline int g_lastShotsFired = 0;
    inline void AngleToDir(float pitch, float yaw, float out[3]) {
        constexpr float PI = 3.14159265358979323846f;
        float cp = cosf(pitch * (PI / 180.0f));
        float sp = sinf(pitch * (PI / 180.0f));
        float cy = cosf(yaw * (PI / 180.0f));
        float sy = sinf(yaw * (PI / 180.0f));
        out[0] = cp * cy;
        out[1] = cp * sy;
        out[2] = -sp;
    }
}
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace Hooks {
    typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
    typedef HRESULT(__stdcall* ResizeBuffers_t)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    typedef LRESULT(__stdcall* WndProc_t)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    Present_t oPresent = nullptr;
    ResizeBuffers_t oResizeBuffers = nullptr;
    WndProc_t oWndProc = nullptr;
    ID3D11Device* g_pDevice = nullptr;
    ID3D11DeviceContext* g_pContext = nullptr;
    ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
    HWND g_hWnd = nullptr;
    bool g_bInitialized = false;
    bool g_bMenuOpen = true;
    bool g_bEspEnabled = true;
    bool g_bEspBoxes = true;
    bool g_bEspHealth = true;
    bool g_bEspNames = true;
    bool g_bEspFovCircle = true;
    bool g_bAimbotEnabled = true;   
    bool g_bTriggerbotEnabled = false;
    float g_fAimbotFov = 5.0f;
    float g_fAimbotSmooth = 5.0f;
    bool g_bAimbotTeamCheck = true;
    bool g_bAimbotVisCheck = true; 
    bool g_bAimbotZCheck = true; 
    bool g_bAimbotRcs = true;
    int g_nAimbotBone = 6; 
    bool g_bLegitMode = true;
    float g_fHeadshotPercent = 50.0f;
    bool g_bClosestBone = false;
    int  g_nLegitBoneSelected = 6;
    uintptr_t g_nLegitLastPawn = 0;
    float g_fSwitchDelay = 1.0f;
    DWORD g_dwLastSwitchTime = 0;
    HANDLE g_hAimbotThread = nullptr;
    HANDLE g_hTriggerbotThread = nullptr;
    bool g_bEspTeamCheck = true;
    bool g_bEspSkeleton = false;
    bool g_bEspSnaplines = false;
    bool g_bAntiFlash = false;
    bool g_bEspOffscreen = false;
    float g_fEspOffscreenRadius = 150.0f;
    float g_fEspOffscreenSize = 15.0f;
    ImVec4 g_colEspOffscreen = ImVec4(1.0f, 0.3f, 0.0f, 1.0f);
    bool g_bEspTeamColors = false;
    ImVec4 g_colEspT = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); 
    ImVec4 g_colEspCT = ImVec4(0.2f, 0.5f, 1.0f, 1.0f); 
    ImVec4 g_colEspBox = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 g_colEspName = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 g_colEspSkeleton = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);
    ImVec4 g_colEspSnaplines = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 g_colFovCircle = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
    bool g_bEspCornerBox = false;
    bool g_bEspDistance = false;
    float g_fEspMaxDistance = 150.0f; 
    int g_nSnaplineOrigin = 0; 
    bool g_bBombTimer = false;
    bool g_bBulletTracers = false;
    float g_fTracerLife = 2.5f;
    float g_fTracerSpeed = 8000.0f;
    float g_fTracerThickness = 2.0f;
    const char* g_szKeyNames[] = { "Left Mouse", "Right Mouse", "Middle Mouse", "Mouse 4", "Mouse 5", "Shift", "Ctrl", "Alt", "Space", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Insert", "Home", "End" };
    int g_nKeyCodes[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2, VK_SHIFT, VK_CONTROL, VK_MENU, VK_SPACE, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12, VK_INSERT, VK_HOME, VK_END };
    int g_nAimbotKeyIndex = 1;      
    int g_nTriggerbotKeyIndex = 4;  
    int g_nConsoleKeyIndex = 20;    
    int g_nMenuKeyIndex = 21;       
    int g_nAimbotKey = VK_RBUTTON;
    int g_nTriggerbotKey = VK_XBUTTON2;
    int g_nConsoleKey = VK_F12;
    int g_nMenuKey = VK_INSERT;
    int g_nAimbotPriority = 0;      
    bool g_bSkinMenuOpen = false; 
    bool g_bRunning = true;
    void RenderMenu();
    void RenderSkinMenu();
    void RenderESP();
    void RunAimbot();
    void RunTriggerbot();
    void SaveConfig();
    void LoadConfig();
    DWORD WINAPI AimbotThread(LPVOID lpParam);
    DWORD WINAPI TriggerbotThread(LPVOID lpParam);
    LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_KEYDOWN) {
            if (wParam == g_nMenuKey) {
                g_bMenuOpen = !g_bMenuOpen;
                return 0;
            }
        }
        if (g_bMenuOpen) {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
                return 0;
            }
        }
        return CallWindowProcA(oWndProc, hWnd, msg, wParam, lParam);
    }
    HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
        if (!g_bInitialized) {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pDevice))) {
                g_pDevice->GetImmediateContext(&g_pContext);
                DXGI_SWAP_CHAIN_DESC desc;
                pSwapChain->GetDesc(&desc);
                g_hWnd = desc.OutputWindow;
                ID3D11Texture2D* pBackBuffer = nullptr;
                pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
                if (pBackBuffer) {
                    g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
                    pBackBuffer->Release();
                }
                oWndProc = (WndProc_t)SetWindowLongPtrA(g_hWnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                ImGui::StyleColorsDark();
                ImGui_ImplWin32_Init(g_hWnd);
                ImGui_ImplDX11_Init(g_pDevice, g_pContext);
                g_bInitialized = true;
            }
        }
        if (g_bInitialized) {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            if (g_bMenuOpen) {
                RenderMenu();
            }
            static bool bFirstInitConsole = true;
            if (bFirstInitConsole) {
                HWND hc = GetConsoleWindow();
                if (hc) ShowWindow(hc, SW_HIDE);
                bFirstInitConsole = false;
            }
            static bool s_keyConsole = false;
            bool isConsole = (GetAsyncKeyState(g_nConsoleKey) & 0x8000) != 0;
            if (isConsole && !s_keyConsole) {
                HWND hConsole = GetConsoleWindow();
                if (IsWindowVisible(hConsole)) ShowWindow(hConsole, SW_HIDE);
                else ShowWindow(hConsole, SW_SHOW);
            }
            s_keyConsole = isConsole;
            {
                uintptr_t cb = (uintptr_t)GetModuleHandleA("client.dll");
                if (cb) {
                    uintptr_t localPawn = *(uintptr_t*)(cb + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
                    SkinChanger::Tick(cb, localPawn);
                }
            }
            if (g_bEspEnabled) {
                RenderESP();
            }
            ImGui::Render();
            g_pContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
        return oPresent(pSwapChain, SyncInterval, Flags);
    }
    HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
        if (g_pRenderTargetView) {
            g_pRenderTargetView->Release();
            g_pRenderTargetView = nullptr;
        }
        HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
        ID3D11Texture2D* pBackBuffer = nullptr;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
        if (pBackBuffer) {
            g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
            pBackBuffer->Release();
        }
        return hr;
    }
    IDXGISwapChain* GetSwapChain() {
        WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, DefWindowProcA, 0, 0, GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr, "DummyClass", nullptr };
        RegisterClassExA(&wc);
        HWND hWnd = CreateWindowA("DummyClass", "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        ID3D11Device* pDevice = nullptr;
        ID3D11DeviceContext* pContext = nullptr;
        IDXGISwapChain* pSwapChain = nullptr;
        D3D_FEATURE_LEVEL featureLevel;
        D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &featureLevel, &pContext);
        void** pVTable = *(void***)pSwapChain;
        static void* presentAddr = pVTable[8];
        static void* resizeBuffersAddr = pVTable[13];
        pContext->Release();
        pDevice->Release();
        pSwapChain->Release();
        DestroyWindow(hWnd);
        UnregisterClassA("DummyClass", wc.hInstance);
        return (IDXGISwapChain*)presentAddr; 
    }
    void Initialize() {
        WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, DefWindowProcA, 0, 0, GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr, "DX11Hook", nullptr };
        RegisterClassExA(&wc);
        HWND hWnd = CreateWindowA("DX11Hook", "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        ID3D11Device* pDevice = nullptr;
        ID3D11DeviceContext* pContext = nullptr;
        IDXGISwapChain* pSwapChain = nullptr;
        D3D_FEATURE_LEVEL featureLevel;
        if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &featureLevel, &pContext))) {
            DestroyWindow(hWnd);
            UnregisterClassA("DX11Hook", wc.hInstance);
            return;
        }
        void** pVTable = *(void***)pSwapChain;
        void* pPresent = pVTable[8];
        void* pResizeBuffers = pVTable[13];
        pContext->Release();
        pDevice->Release();
        pSwapChain->Release();
        DestroyWindow(hWnd);
        UnregisterClassA("DX11Hook", wc.hInstance);
        MH_Initialize();
        MH_CreateHook(pPresent, &hkPresent, (void**)&oPresent);
        MH_EnableHook(pPresent);
        MH_CreateHook(pResizeBuffers, &hkResizeBuffers, (void**)&oResizeBuffers);
        MH_EnableHook(pResizeBuffers);
        g_bRunning = true;
        g_hAimbotThread = CreateThread(nullptr, 0, AimbotThread, nullptr, 0, nullptr);
        g_hTriggerbotThread = CreateThread(nullptr, 0, TriggerbotThread, nullptr, 0, nullptr);
        printf("[+] Feature threads started\n");
    }
    void Shutdown() {
        g_bRunning = false;
        if (g_hAimbotThread) {
            WaitForSingleObject(g_hAimbotThread, 1000);
            CloseHandle(g_hAimbotThread);
            g_hAimbotThread = nullptr;
        }
        if (g_hTriggerbotThread) {
            WaitForSingleObject(g_hTriggerbotThread, 1000);
            CloseHandle(g_hTriggerbotThread);
            g_hTriggerbotThread = nullptr;
        }
        printf("[+] Feature threads stopped\n");
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        if (oWndProc) {
            SetWindowLongPtrA(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        }
        if (g_bInitialized) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
        if (g_pRenderTargetView) g_pRenderTargetView->Release();
        if (g_pContext) g_pContext->Release();
        if (g_pDevice) g_pDevice->Release();
    }
    void RenderMenu() {
        ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_FirstUseEver);
        ImGui::Begin("Pixel's Internal", &g_bMenuOpen, ImGuiWindowFlags_NoCollapse);
        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Aimbot")) {
                ImGui::Separator();
                ImGui::Checkbox("Enable Aimbot", &g_bAimbotEnabled);
                ImGui::Combo("Aimbot Key", &g_nAimbotKeyIndex, g_szKeyNames, IM_ARRAYSIZE(g_szKeyNames));
                g_nAimbotKey = g_nKeyCodes[g_nAimbotKeyIndex];
                ImGui::SliderFloat("FOV",       &g_fAimbotFov,    1.0f, 50.0f);
                ImGui::SliderFloat("Smoothing", &g_fAimbotSmooth,  1.0f, 50.0f);
                ImGui::Checkbox("Team Check", &g_bAimbotTeamCheck);
                ImGui::Checkbox("(Very Unreliable) Wallcheck", &g_bAimbotVisCheck);
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Warning: Uses m_bSpotted radar flag.");
                ImGui::TextWrapped("This is a server-wide flag. Only use this if you really want to, as your aimbot will aim through walls if a teammate spots the enemy.");
                ImGui::Checkbox("150 Z-Unit Height Check (Vertigo Floor Clamp)", &g_bAimbotZCheck);
                ImGui::Checkbox("Recoil Control System", &g_bAimbotRcs);
                ImGui::Spacing(); ImGui::Separator(); ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Triggerbot"); ImGui::Spacing();
                ImGui::Checkbox("Enable Triggerbot", &g_bTriggerbotEnabled);
                ImGui::Combo("Triggerbot Key", &g_nTriggerbotKeyIndex, g_szKeyNames, IM_ARRAYSIZE(g_szKeyNames));
                g_nTriggerbotKey = g_nKeyCodes[g_nTriggerbotKeyIndex];
                ImGui::Spacing(); ImGui::Separator(); ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Targeting"); ImGui::Spacing();
                const char* prio[] = { "Crosshair (FOV)", "Closest Distance" };
                ImGui::Combo("Target Priority", &g_nAimbotPriority, prio, 2);
                ImGui::Spacing(); ImGui::Separator(); ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Legit Mode"); ImGui::Spacing();
                ImGui::Checkbox("Enable Legit Base", &g_bLegitMode);
                if (g_bLegitMode) {
                    ImGui::SliderFloat("Headshot %%",    &g_fHeadshotPercent, 0.0f, 100.0f);
                    ImGui::SliderFloat("Switch Delay (s)", &g_fSwitchDelay, 0.0f, 5.0f);
                    ImGui::Checkbox("Closest Bone to Crosshair", &g_bClosestBone);
                } else {
                    ImGui::SliderFloat("Switch Delay (s)", &g_fSwitchDelay, 0.0f, 5.0f);
                    const char* bones[] = {"Head","Neck","Chest"};
                    static int boneIdx = 0;
                    if (ImGui::Combo("Target Bone", &boneIdx, bones, 3)) {
                        if (boneIdx==0) g_nAimbotBone=6;
                        else if (boneIdx==1) g_nAimbotBone=5;
                        else g_nAimbotBone=4;
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Visuals")) {
                ImGui::Separator();
                ImGui::Columns(2, nullptr, false);
                ImGui::Checkbox("Enable ESP", &g_bEspEnabled);
                ImGui::Checkbox("Team Check (Show Enemies Only)", &g_bEspTeamCheck);
                if (g_bEspTeamCheck) {
                    ImGui::BeginDisabled();
                    bool f = false;
                    ImGui::Checkbox("Use Team Colors (T=Red, CT=Blue)", &f);
                    ImGui::EndDisabled();
                } else {
                    ImGui::Checkbox("Use Team Colors (T=Red, CT=Blue)", &g_bEspTeamColors);
                }
                ImGui::Checkbox("Bounding Boxes", &g_bEspBoxes);
                ImGui::Checkbox("Corner Boxes", &g_bEspCornerBox);
                ImGui::Checkbox("Health Bars", &g_bEspHealth);
                ImGui::Checkbox("Names", &g_bEspNames);
                ImGui::Checkbox("Distance", &g_bEspDistance);
                if (g_bEspDistance) ImGui::SliderFloat("Max Distance (m)", &g_fEspMaxDistance, 10.0f, 500.0f);
                ImGui::Checkbox("FOV Circle", &g_bEspFovCircle);
                ImGui::Checkbox("Skeleton Lines", &g_bEspSkeleton);
                ImGui::Checkbox("Snaplines", &g_bEspSnaplines);
                if (g_bEspSnaplines) {
                    const char* origins[] = { "Bottom", "Center", "Top" };
                    ImGui::Combo("Snap Origin", &g_nSnaplineOrigin, origins, 3);
                }
                ImGui::Checkbox("Offscreen Indicators", &g_bEspOffscreen);
                if (g_bEspOffscreen) {
                    ImGui::SliderFloat("Offscreen Radius", &g_fEspOffscreenRadius, 50.0f, 1000.0f);
                    ImGui::SliderFloat("Offscreen Size", &g_fEspOffscreenSize, 10.0f, 100.0f);
                }
                ImGui::Spacing(); ImGui::Separator(); ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "World"); ImGui::Spacing();
                ImGui::Checkbox("Anti-Flash (!)", &g_bAntiFlash);
                ImGui::Checkbox("Bomb Timer", &g_bBombTimer);
                ImGui::Spacing(); ImGui::Separator(); ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Bullet Tracers"); ImGui::Spacing();
                ImGui::Checkbox("Enable Tracers", &g_bBulletTracers);
                if (g_bBulletTracers) {
                    ImGui::SliderFloat("Trail Life (s)", &g_fTracerLife, 0.5f, 5.0f);
                    ImGui::SliderFloat("Bullet Speed", &g_fTracerSpeed, 1000.0f, 15000.0f);
                    ImGui::SliderFloat("Thickness", &g_fTracerThickness, 1.0f, 5.0f);
                }
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Custom Colors");
                ImGui::ColorEdit4("Box Color", (float*)&g_colEspBox, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::ColorEdit4("Name Color", (float*)&g_colEspName, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                bool bOverridden = (!g_bEspTeamCheck && g_bEspTeamColors);
                if (!bOverridden) {
                    ImGui::ColorEdit4("Skeleton Color", (float*)&g_colEspSkeleton, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                }
                ImGui::ColorEdit4("Snapline Color", (float*)&g_colEspSnaplines, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::ColorEdit4("FOV Circle Color", (float*)&g_colFovCircle, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                if (!bOverridden) {
                    ImGui::ColorEdit4("Offscreen Color", (float*)&g_colEspOffscreen, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                }
                if (bOverridden) {
                    ImGui::Spacing(); ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Team Colors (Overrides Skeleton & Offscreen)");
                    ImGui::ColorEdit4("T Color", (float*)&g_colEspT, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::ColorEdit4("CT Color", (float*)&g_colEspCT, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                }
                ImGui::Columns(1);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Misc")) {
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                ImGui::Combo("Debugging Console Hotkey", &g_nConsoleKeyIndex, g_szKeyNames, IM_ARRAYSIZE(g_szKeyNames));
                g_nConsoleKey = g_nKeyCodes[g_nConsoleKeyIndex];
                ImGui::Combo("Menu Toggle Hotkey", &g_nMenuKeyIndex, g_szKeyNames, IM_ARRAYSIZE(g_szKeyNames));
                g_nMenuKey = g_nKeyCodes[g_nMenuKeyIndex];
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                ImGui::TextDisabled("Saves to: Pixel_Internal.cfg (in CS2 directory)");
                if (ImGui::Button("Save Config", ImVec2(180, 0))) SaveConfig();
                ImGui::SameLine();
                if (ImGui::Button("Load Config", ImVec2(180, 0))) LoadConfig();
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                ImGui::TextDisabled("INSERT = Toggle Cheat Interface");
                ImGui::TextDisabled("END    = Unload from game");
                ImGui::TextDisabled("F12    = Toggle Debug Console");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Skin Changer")) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[!] Knife Skin Notice:");
                ImGui::TextWrapped("This cheat safely modifies the 'Paint Kit' (texture) of your weapon without hooking the engine to change 3D models. Because of this, you must physically equip the correct knife model in your CS2 loadout for the skin to match! (e.g. Applying a Butterfly Knife skin to a Default Knife will just paint the default blade).");
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                static int selectedWeapon = 0;
                const char* wepNames[SkinChanger::WeaponCount];
                for (int i = 0; i < SkinChanger::WeaponCount; i++)
                    wepNames[i] = SkinChanger::GetWeaponName(SkinChanger::WeaponList[i]);
                ImGui::Combo("Weapon", &selectedWeapon, wepNames, SkinChanger::WeaponCount);
                int def = SkinChanger::WeaponList[selectedWeapon];
                SkinChanger::SkinConfig& skin = SkinChanger::weaponSkins[def];
                ImGui::Checkbox("Enable Skin for this weapon", &skin.enabled);
                ImGui::InputInt("Paint Kit ID", &skin.paintKit);
                if (skin.paintKit < 0) skin.paintKit = 0;
                ImGui::SliderFloat("Wear", &skin.wear, 0.001f, 1.0f);
                ImGui::SliderInt("Seed", &skin.seed, 0, 999);
                bool hasST = (skin.statTrak >= 0);
                if (ImGui::Checkbox("StatTrak", &hasST)) skin.statTrak = hasST ? 0 : -1;
                if (hasST) ImGui::InputInt("StatTrak Count", &skin.statTrak);
                ImGui::Spacing();
                if (ImGui::Button("Apply Skins", ImVec2(140, 0))) {
                    SkinChanger::forceUpdate.store(true);
                }
                ImGui::SameLine();
                if (ImGui::Button("Randomize All", ImVec2(140, 0))) {
                    int knifeKits[] = { 38, 415, 416, 417, 44, 42, 59, 175, 409, 410, 411, 413, 568 };
                    for (int w : SkinChanger::WeaponList) {
                        SkinChanger::weaponSkins[w].enabled = true;
                        if (w == 42 || w == 59 || (w >= 500 && w < 600)) {
                            SkinChanger::weaponSkins[w].paintKit = knifeKits[rand() % 13];
                        } else {
                            SkinChanger::weaponSkins[w].paintKit = (rand() % 500) + 1; 
                        }
                        SkinChanger::weaponSkins[w].wear = 0.01f;
                    }
                    SkinChanger::forceUpdate.store(true);
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
    Vector3 GetBonePosition(uintptr_t pawn, int boneId);
    void RenderESPImpl() {
            uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
            if (!clientBase) return;
        uintptr_t entityList = *(uintptr_t*)(clientBase + cs2_dumper::offsets::client_dll::dwEntityList);
        if (!entityList) return;
        uintptr_t localPawn = *(uintptr_t*)(clientBase + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
        if (!localPawn) return;
        int localTeam = *(uint8_t*)(localPawn + 0x3F3); 
        uintptr_t localSceneNode = *(uintptr_t*)(localPawn + 0x338);
        Vector3 localOrigin = {0, 0, 0};
        if (localSceneNode) localOrigin = *(Vector3*)(localSceneNode + 0xD0);
        view_matrix_t viewMatrix;
        memcpy(&viewMatrix, (void*)(clientBase + cs2_dumper::offsets::client_dll::dwViewMatrix), sizeof(view_matrix_t)); 
        ImGuiIO& io = ImGui::GetIO();
        int screenWidth = (int)io.DisplaySize.x;
        int screenHeight = (int)io.DisplaySize.y;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 screenCenter(screenWidth / 2.0f, screenHeight / 2.0f);
        QAngle viewAngles = *(QAngle*)(clientBase + cs2_dumper::offsets::client_dll::dwViewAngles);
        if (g_bAntiFlash && localPawn) {
            float currentAlpha = *(float*)(localPawn + 0x15F4); 
            if (currentAlpha > 0.0f) {
                *(float*)(localPawn + 0x15F4) = 0.0f;
            }
        }
        if (g_bBombTimer) {
            uintptr_t pBombSystem = *(uintptr_t*)(clientBase + cs2_dumper::offsets::client_dll::dwPlantedC4);
            if (pBombSystem) {
                uintptr_t bomb = *(uintptr_t*)pBombSystem;
                if (bomb) {
                    uintptr_t bombNode = *(uintptr_t*)(bomb + 0x338);
                    if (bombNode) {
                        Vector3 bombOrigin = *(Vector3*)(bombNode + 0xD0);
                        if (bombOrigin.x != 0.0f || bombOrigin.y != 0.0f) {
                            Vector2 bSc;
                            if (WorldToScreen(bombOrigin, bSc, viewMatrix, screenWidth, screenHeight)) {
                                bool isTicking = *(bool*)(bomb + 0x1170); 
                                if (isTicking) {
                                    float currentTime = *(float*)(clientBase + cs2_dumper::offsets::client_dll::dwGlobalVars); 
                                    float simTime = *(float*)(localPawn + 0x3C0); 
                                    float c4BlowTime = *(float*)(bomb + 0x11A0); 
                                    float remaining = c4BlowTime - simTime;
                                    if (remaining > 0) {
                                        char bombText[64];
                                        sprintf_s(bombText, "C4: %.1f", remaining);
                                        ImVec2 bSize = ImGui::CalcTextSize(bombText);
                                        ImVec2 bPos(bSc.x - (bSize.x / 2.0f), bSc.y);
                                        drawList->AddText(ImVec2(bPos.x + 1, bPos.y + 1), IM_COL32(0,0,0,255), bombText);
                                        drawList->AddText(bPos, remaining < 10.0f ? IM_COL32(255,0,0,255) : IM_COL32(255,255,0,255), bombText);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        int shotsFired = *(int*)(localPawn + 0x270C); 
        if (shotsFired > Visuals::g_lastShotsFired && Visuals::g_lastShotsFired >= 0) {
            if (g_bBulletTracers) {
                QAngle viewAngles = *(QAngle*)(clientBase + cs2_dumper::offsets::client_dll::dwViewAngles);
                float dir[3];
                Visuals::AngleToDir(viewAngles.x, viewAngles.y, dir);
                Vector3 viewOffset = *(Vector3*)(localPawn + 0xD58);
                if (viewOffset.z < 10.0f) viewOffset.z = 64.0f;
                Vector3 eyePos = localOrigin + viewOffset;
                Visuals::Tracer t;
                t.startPos = eyePos;
                t.endPos = { eyePos.x + dir[0] * g_fTracerSpeed, eyePos.y + dir[1] * g_fTracerSpeed, eyePos.z + dir[2] * g_fTracerSpeed };
                t.spawnTime = GetTickCount() / 1000.0f;
                float dx = t.endPos.x - eyePos.x;
                float dy = t.endPos.y - eyePos.y;
                float dz = t.endPos.z - eyePos.z;
                t.totalDist = sqrtf(dx*dx + dy*dy + dz*dz);
                std::lock_guard<std::mutex> lock(Visuals::g_tracerMutex);
                Visuals::g_tracers.push_back(t);
            }
        }
        Visuals::g_lastShotsFired = shotsFired;
        if (g_bBulletTracers) {
            float now = GetTickCount() / 1000.0f;
            std::lock_guard<std::mutex> lock(Visuals::g_tracerMutex);
            float maxAge = g_fTracerLife + 2.0f;
            for (auto it = Visuals::g_tracers.begin(); it != Visuals::g_tracers.end(); ) {
                if ((now - it->spawnTime) > maxAge) {
                    it = Visuals::g_tracers.erase(it);
                } else {
                    ++it;
                }
            }
            for (const auto& t : Visuals::g_tracers) {
                float age = now - t.spawnTime;
                float travelTime = t.totalDist / g_fTracerSpeed;
                if (travelTime < 0.01f) travelTime = 0.01f;
                float bulletFrac = age / travelTime;
                if (bulletFrac > 1.0f) bulletFrac = 1.0f;
                float trailAge = age - travelTime;
                float trailAlpha = 1.0f;
                if (trailAge > 0.0f) {
                    trailAlpha = 1.0f - (trailAge / g_fTracerLife);
                    if (trailAlpha <= 0.0f) continue;
                    trailAlpha *= trailAlpha;
                }
                constexpr int SEGMENTS = 16;
                ImVec2 pts[SEGMENTS + 1];
                bool ok[SEGMENTS + 1] = {};
                for (int s = 0; s <= SEGMENTS; s++) {
                    float segFrac = (float)s / (float)SEGMENTS * bulletFrac;
                    Vector3 pos3d = {
                        t.startPos.x + (t.endPos.x - t.startPos.x) * segFrac,
                        t.startPos.y + (t.endPos.y - t.startPos.y) * segFrac,
                        t.startPos.z + (t.endPos.z - t.startPos.z) * segFrac
                    };
                    Vector2 sc;
                    if (WorldToScreen(pos3d, sc, viewMatrix, screenWidth, screenHeight)) {
                        pts[s] = ImVec2(sc.x, sc.y);
                        ok[s] = true;
                    }
                }
                for (int s = 0; s < SEGMENTS; s++) {
                    if (!ok[s] || !ok[s+1]) continue;
                    float segFrac = (float)s / (float)SEGMENTS;
                    float brightness = 0.2f + 0.8f * segFrac;
                    int alpha = (int)(trailAlpha * brightness * 220.0f);
                    if (alpha <= 0) continue;
                    if (alpha > 255) alpha = 255;
                    drawList->AddLine(pts[s], pts[s+1], IM_COL32(255,255,255,alpha), g_fTracerThickness);
                    int glowA = (int)(trailAlpha * brightness * 40.0f);
                    if (glowA > 255) glowA = 255;
                    drawList->AddLine(pts[s], pts[s+1], IM_COL32(180,200,255,glowA), g_fTracerThickness * 3.5f);
                }
            }
        }
        for (int i = 1; i <= 64; i++) {
            uintptr_t listEntry = *(uintptr_t*)(entityList + 0x10 + 8 * (i >> 9));
            if (!listEntry) continue;
            uintptr_t controller = *(uintptr_t*)(listEntry + 0x70 * (i & 0x1FF));
            if (!controller) continue;
            uint32_t pawnHandle = *(uint32_t*)(controller + 0x6C4); 
            if (!pawnHandle) continue;
            uintptr_t pawnEntry = *(uintptr_t*)(entityList + 0x10 + 8 * ((pawnHandle & 0x7FFF) >> 9));
            if (!pawnEntry) continue;
            uintptr_t pawn = *(uintptr_t*)(pawnEntry + 0x70 * (pawnHandle & 0x1FF));
            if (!pawn || pawn == localPawn) continue;
            int health = *(int*)(pawn + 0x354); 
            int team = *(uint8_t*)(pawn + 0x3F3); 
            uint8_t lifeState = *(uint8_t*)(pawn + 0x35C); 
            if (health <= 0 || lifeState != 0) continue;
            if (g_bEspTeamCheck && team == localTeam) continue;
            uintptr_t gameSceneNode = *(uintptr_t*)(pawn + 0x338); 
            if (!gameSceneNode) continue;
            Vector3 origin = *(Vector3*)(gameSceneNode + 0xD0);
            Vector2 screenPos;
            bool onScreen = WorldToScreen(origin, screenPos, viewMatrix, screenWidth, screenHeight);
            if (!onScreen && g_bEspOffscreen) {
                Vector3 viewOffset = *(Vector3*)(localPawn + 0xD58);
                if (viewOffset.z < 10.0f) viewOffset.z = 64.0f;
                Vector3 eyePos = localOrigin + viewOffset;
                Vector3 rawAngle = Vector3::CalculateAngle(eyePos, origin);
                float angleRad = -1.57079633f - (rawAngle.y - viewAngles.y) * 0.01745329f;
                float cosA = cosf(angleRad);
                float sinA = sinf(angleRad);
                ImVec2 point1(screenCenter.x + g_fEspOffscreenRadius * cosA, screenCenter.y + g_fEspOffscreenRadius * sinA);
                float baseR = g_fEspOffscreenRadius - g_fEspOffscreenSize;
                ImVec2 baseCenter(screenCenter.x + baseR * cosA, screenCenter.y + baseR * sinA);
                float orthoX = -sinA;
                float orthoY = cosA;
                float halfWidth = g_fEspOffscreenSize * 0.5f;
                ImVec2 point2(baseCenter.x + orthoX * halfWidth, baseCenter.y + orthoY * halfWidth);
                ImVec2 point3(baseCenter.x - orthoX * halfWidth, baseCenter.y - orthoY * halfWidth);
                ImU32 offColor = ImGui::GetColorU32(g_colEspOffscreen);
                if (!g_bEspTeamCheck && g_bEspTeamColors) {
                    if (team == 2) offColor = ImGui::GetColorU32(g_colEspT);
                    else if (team == 3) offColor = ImGui::GetColorU32(g_colEspCT);
                }
                drawList->AddTriangleFilled(point1, point2, point3, offColor);
            }
            if (!onScreen) continue;
            Vector2 headScreenPos;
            Vector3 headPos = origin;
            headPos.z += 72.0f; 
            if (!WorldToScreen(headPos, headScreenPos, viewMatrix, screenWidth, screenHeight)) continue;
            float distanceMeters = sqrtf(powf(origin.x - localOrigin.x, 2) + powf(origin.y - localOrigin.y, 2) + powf(origin.z - localOrigin.z, 2)) * 0.0254f;
            if (g_bEspDistance && distanceMeters > g_fEspMaxDistance) continue;
            float boxHeight = screenPos.y - headScreenPos.y;
            float boxWidth = boxHeight * 0.5f;
            ImU32 colBox = ImGui::GetColorU32(g_colEspBox);
            ImU32 colName = ImGui::GetColorU32(g_colEspName);
            ImU32 colSkel = ImGui::GetColorU32(g_colEspSkeleton);
            ImU32 colSnap = ImGui::GetColorU32(g_colEspSnaplines);
            if (!g_bEspTeamCheck && g_bEspTeamColors) {
                if (team == 2) {
                    ImU32 tCol = ImGui::GetColorU32(g_colEspT);
                    colSkel = tCol;
                } else if (team == 3) {
                    ImU32 ctCol = ImGui::GetColorU32(g_colEspCT);
                    colSkel = ctCol;
                }
            }
            if (g_bEspBoxes) {
                if (g_bEspCornerBox) {
                    float lineW = (boxWidth / 3.0f);
                    float lineH = (boxHeight / 4.0f);
                    float pX = headScreenPos.x - boxWidth / 2.0f;
                    float pY = headScreenPos.y;
                    drawList->AddLine(ImVec2(pX, pY), ImVec2(pX + lineW, pY), colBox, 2.0f);
                    drawList->AddLine(ImVec2(pX, pY), ImVec2(pX, pY + lineH), colBox, 2.0f);
                    drawList->AddLine(ImVec2(pX + boxWidth, pY), ImVec2(pX + boxWidth - lineW, pY), colBox, 2.0f);
                    drawList->AddLine(ImVec2(pX + boxWidth, pY), ImVec2(pX + boxWidth, pY + lineH), colBox, 2.0f);
                    drawList->AddLine(ImVec2(pX, screenPos.y), ImVec2(pX + lineW, screenPos.y), colBox, 2.0f);
                    drawList->AddLine(ImVec2(pX, screenPos.y), ImVec2(pX, screenPos.y - lineH), colBox, 2.0f);
                    drawList->AddLine(ImVec2(pX + boxWidth, screenPos.y), ImVec2(pX + boxWidth - lineW, screenPos.y), colBox, 2.0f);
                    drawList->AddLine(ImVec2(pX + boxWidth, screenPos.y), ImVec2(pX + boxWidth, screenPos.y - lineH), colBox, 2.0f);
                } else {
                    drawList->AddRect(
                        ImVec2(headScreenPos.x - boxWidth / 2, headScreenPos.y),
                        ImVec2(headScreenPos.x + boxWidth / 2, screenPos.y),
                        colBox, 0.0f, 0, 2.0f
                    );
                }
            }
            if (g_bEspHealth) {
                float healthPercent = health / 100.0f;
                ImU32 healthColor = IM_COL32(255 * (1 - healthPercent), 255 * healthPercent, 0, 255);
                drawList->AddRectFilled(
                    ImVec2(headScreenPos.x - boxWidth / 2 - 6, screenPos.y),
                    ImVec2(headScreenPos.x - boxWidth / 2 - 2, screenPos.y - boxHeight * healthPercent),
                    healthColor
                );
                drawList->AddRect(
                    ImVec2(headScreenPos.x - boxWidth / 2 - 6, headScreenPos.y),
                    ImVec2(headScreenPos.x - boxWidth / 2 - 2, screenPos.y),
                    IM_COL32(0, 0, 0, 255)
                );
            }
            if (g_bEspNames || g_bEspDistance) {
                const char* name = (const char*)(controller + 0x6F8); 
                char textObj[128];
                if (name && name[0]) {
                    if (g_bEspNames && g_bEspDistance) sprintf_s(textObj, sizeof(textObj), "%s [%dm]", name, (int)distanceMeters);
                    else if (g_bEspNames) sprintf_s(textObj, sizeof(textObj), "%s", name);
                    else sprintf_s(textObj, sizeof(textObj), "[%dm]", (int)distanceMeters);
                    ImVec2 textSize = ImGui::CalcTextSize(textObj);
                    drawList->AddText(
                        ImVec2(headScreenPos.x - textSize.x / 2, headScreenPos.y - 15),
                        colName,
                        textObj
                    );
                }
            }
            if (g_bEspSnaplines) {
                ImVec2 originPos;
                if (g_nSnaplineOrigin == 0)      originPos = ImVec2(screenWidth / 2.0f, screenHeight); 
                else if (g_nSnaplineOrigin == 1) originPos = ImVec2(screenWidth / 2.0f, screenHeight / 2.0f); 
                else                             originPos = ImVec2(screenWidth / 2.0f, 0.0f); 
                drawList->AddLine(originPos, ImVec2(screenPos.x, screenPos.y), colSnap, 1.0f);
            }
            if (g_bEspSkeleton) {
                int bonePairs[][2] = {
                    {6,5}, {5,4}, {4,2}, {2,0}, 
                    {5,9}, {9,10}, {10,11},     
                    {5,14}, {14,15}, {15,16},   
                    {0,25}, {25,26}, {26,27},   
                    {0,22}, {22,23}, {23,24}    
                };
                for (auto& pair : bonePairs) {
                    Vector3 p1 = GetBonePosition(pawn, pair[0]);
                    Vector3 p2 = GetBonePosition(pawn, pair[1]);
                    Vector2 s1, s2;
                    if (WorldToScreen(p1, s1, viewMatrix, screenWidth, screenHeight) && 
                        WorldToScreen(p2, s2, viewMatrix, screenWidth, screenHeight)) {
                        drawList->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), colSkel, 1.0f);
                    }
                }
            }
        } 
        if (g_bEspFovCircle && g_bAimbotEnabled) {
            drawList->AddCircle(
                ImVec2(screenWidth / 2.0f, screenHeight / 2.0f),
                g_fAimbotFov * 10.0f,
                ImGui::GetColorU32(g_colFovCircle),
                64, 1.5f
            );
        }
    } 
    void RenderESP() {
        __try {
            RenderESPImpl();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    Vector3 GetBonePosition(uintptr_t pawn, int boneId) {
        uintptr_t gameSceneNode = *(uintptr_t*)(pawn + 0x338); 
        if (!gameSceneNode) return { 0, 0, 0 };
        uintptr_t boneArray = *(uintptr_t*)(gameSceneNode + 0x1E0); 
        if (!boneArray) boneArray = *(uintptr_t*)(gameSceneNode + 0x1F0); 
        if (!boneArray) return { 0, 0, 0 };
        return *(Vector3*)(boneArray + boneId * 32);
    }
    static uintptr_t g_lockedPawn = 0;
    int GetEffectiveBone(uintptr_t pawn, const view_matrix_t& viewMatrix, int screenWidth, int screenHeight) {
        if (!g_bLegitMode) return g_nAimbotBone;
        static bool seeded = false;
        if (!seeded) { srand((unsigned int)GetTickCount()); seeded = true; }
        if (pawn != g_nLegitLastPawn) {
            g_nLegitLastPawn = pawn;
            if (g_bClosestBone) {
                int bones[] = { 6, 5, 4 };
                Vector2 center(screenWidth / 2.0f, screenHeight / 2.0f);
                float bestD = 99999.0f;
                g_nLegitBoneSelected = 6;
                for (int b : bones) {
                    Vector3 pos = GetBonePosition(pawn, b);
                    if (pos.x == 0.0f && pos.y == 0.0f) continue;
                    Vector2 sc;
                    if (WorldToScreen(pos, sc, viewMatrix, screenWidth, screenHeight)) {
                        float d = sqrtf(powf(sc.x - center.x, 2) + powf(sc.y - center.y, 2));
                        if (d < bestD) { bestD = d; g_nLegitBoneSelected = b; }
                    }
                }
            } else {
                float r = (float)(rand() % 100);
                g_nLegitBoneSelected = (r < g_fHeadshotPercent) ? 6 : 4;
            }
        }
        return g_nLegitBoneSelected;
    }
    void RunAimbot() {
        if (!(GetAsyncKeyState(g_nAimbotKey) & 0x8000)) {
            g_lockedPawn = 0; 
            g_dwLastSwitchTime = 0; 
            return;
        }
        __try {
            uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
            if (!clientBase) return;
            uintptr_t entityList = *(uintptr_t*)(clientBase + cs2_dumper::offsets::client_dll::dwEntityList);
            if (!entityList) return;
        uintptr_t localPawn = *(uintptr_t*)(clientBase + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
        if (!localPawn) return;
        uintptr_t localSceneNode = *(uintptr_t*)(localPawn + 0x338);
        if (!localSceneNode) return;
        Vector3 localOrigin = *(Vector3*)(localSceneNode + 0xD0);
        Vector3 viewOffset = *(Vector3*)(localPawn + 0xD58);
        if (viewOffset.z < 10.0f) viewOffset.z = 64.0f;
        Vector3 eyePos = localOrigin + viewOffset;
        static DWORD lastAimLog = 0;
        bool logThisFrame = (GetTickCount() - lastAimLog > 1000);
        if (logThisFrame) {
            printf("[Aimbot] EyeZ: %.1f (OriginZ: %.1f, ViewOffZ: %.1f)\n", 
                eyePos.z, localOrigin.z, viewOffset.z);
        }
        QAngle currentAngles = *(QAngle*)(clientBase + cs2_dumper::offsets::client_dll::dwViewAngles);
        Vector3 aimPunch = *(Vector3*)(localPawn + 0x16CC);
        int localTeam = *(uint8_t*)(localPawn + 0x3F3);
        view_matrix_t viewMatrix;
        memcpy(&viewMatrix, (void*)(clientBase + cs2_dumper::offsets::client_dll::dwViewMatrix), sizeof(view_matrix_t));
        ImGuiIO& io = ImGui::GetIO();
        int screenWidth = (int)io.DisplaySize.x;
        int screenHeight = (int)io.DisplaySize.y;
        Vector2 screenCenter(screenWidth / 2.0f, screenHeight / 2.0f);
        float bestScore = 999999.0f;
        QAngle bestTargetAngle;
        bool foundTarget = false;
        int validCount = 0;
        int boneFoundCount = 0;
        if (g_lockedPawn) {
            int hp = *(int*)(g_lockedPawn + 0x354);
            int team = *(uint8_t*)(g_lockedPawn + 0x3F3);
            bool isVis = true;
            if (g_bAimbotVisCheck) {
                isVis = *(bool*)(g_lockedPawn + 0x26E8); 
            }
            if (hp > 0 && isVis && (localTeam != team || !g_bAimbotTeamCheck)) {
                Vector3 targetPos = GetBonePosition(g_lockedPawn, g_nAimbotBone);
                bool validDistance = true;
                if (g_bAimbotZCheck) {
                    validDistance = (abs(targetPos.z - eyePos.z) < 150.0f);
                }
                if (!validDistance) isVis = false;
                if (isVis && (targetPos.x != 0.0f || targetPos.y != 0.0f)) {
                    Vector2 screenPos;
                    if (WorldToScreen(targetPos, screenPos, viewMatrix, screenWidth, screenHeight)) {
                        float dist = sqrtf(powf(screenPos.x - screenCenter.x, 2) + powf(screenPos.y - screenCenter.y, 2));
                        if (dist < g_fAimbotFov * 15.0f) { 
                            bestScore = dist; 
                            bestTargetAngle = Vector3::CalculateAngle(eyePos, targetPos);
                            foundTarget = true;
                        } else {
                            g_lockedPawn = 0;
                        }
                    } else {
                        g_lockedPawn = 0;
                    }
                } else {
                    g_lockedPawn = 0;
                }
            } else {
                g_lockedPawn = 0;
                if (hp <= 0) {
                    g_dwLastSwitchTime = GetTickCount(); 
                }
            }
        }
        if (!foundTarget) {
            for (int i = 0; i < 64; i++) {
                uintptr_t listEntry = *(uintptr_t*)(entityList + 0x10 + 8 * (i >> 9));
                if (!listEntry) continue;
                uintptr_t controller = *(uintptr_t*)(listEntry + 0x70 * (i & 0x1FF));
                if (!controller) continue;
                uint32_t pawnHandle = *(uint32_t*)(controller + 0x6C4);
                if (!pawnHandle) continue;
                uintptr_t pawnEntry = *(uintptr_t*)(entityList + 0x10 + 8 * ((pawnHandle & 0x7FFF) >> 9));
                if (!pawnEntry) continue;
                uintptr_t pawn = *(uintptr_t*)(pawnEntry + 0x70 * (pawnHandle & 0x1FF));
                if (!pawn || pawn == localPawn) continue;
                int health = *(int*)(pawn + 0x354);
                uint8_t lifeState = *(uint8_t*)(pawn + 0x35C);
                if (health <= 0 || lifeState != 0) continue;
                uintptr_t gameSceneNode = *(uintptr_t*)(pawn + 0x338);
                uintptr_t boneArray = 0;
                if (gameSceneNode) {
                    bool isDormant = *(bool*)(gameSceneNode + 0x10B); 
                    if (isDormant) continue;
                    boneArray = *(uintptr_t*)(gameSceneNode + 0x1E0);
                    if (!boneArray) boneArray = *(uintptr_t*)(gameSceneNode + 0x1F0);
                }
                if (boneArray) boneFoundCount++;
                if (g_bAimbotTeamCheck) {
                    int enemyTeamNum = *(uint8_t*)(pawn + 0x3F3);
                    if (localTeam == enemyTeamNum) continue;
                }
                if (g_bAimbotVisCheck) {
                    bool isSpotted = *(bool*)(pawn + 0x26E8); 
                    if (!isSpotted) continue;
                }
                if (!boneArray) continue;
                Vector3 targetPos = GetBonePosition(pawn, g_nAimbotBone);
                if (g_bAimbotZCheck) {
                    if (abs(targetPos.z - eyePos.z) > 150.0f) continue;
                }
                if (targetPos.x == 0.0f && targetPos.y == 0.0f) continue;
                Vector2 targetScreen;
                bool onScreen = WorldToScreen(targetPos, targetScreen, viewMatrix, screenWidth, screenHeight);
                float distToCrosshair = sqrtf(powf(targetScreen.x - screenCenter.x, 2) + powf(targetScreen.y - screenCenter.y, 2));
                if (distToCrosshair > g_fAimbotFov * 10.0f) continue; 
                float score = 0;
                if (g_nAimbotPriority == 0) {
                    score = distToCrosshair;
                } else {
                    score = sqrtf(powf(targetPos.x - eyePos.x, 2) + powf(targetPos.y - eyePos.y, 2) + powf(targetPos.z - eyePos.z, 2));
                }
                if (score < bestScore) {
                    if (g_lockedPawn == 0 && g_dwLastSwitchTime > 0) {
                        DWORD elapsed = GetTickCount() - g_dwLastSwitchTime;
                        DWORD needed  = (DWORD)(g_fSwitchDelay * 1000.0f);
                        if (elapsed < needed) continue; 
                    }
                    bestScore = score;
                    bestTargetAngle = Vector3::CalculateAngle(eyePos, targetPos);
                    foundTarget = true;
                    if (pawn != g_lockedPawn) {
                        g_dwLastSwitchTime = GetTickCount(); 
                    }
                    g_lockedPawn = pawn;
                    g_nLegitLastPawn = 0; 
                }
            }
        }
        if (logThisFrame) {
            if (foundTarget) {
                printf("[Debug] Target Angle: %.2f, %.2f | Cur Angle: %.2f, %.2f | Locked: %s\n", 
                    bestTargetAngle.x, bestTargetAngle.y, currentAngles.x, currentAngles.y, 
                    g_lockedPawn ? "YES" : "NO");
            }
            lastAimLog = GetTickCount();
        }
        if (foundTarget) {
            if (g_bLegitMode) {
                int legitBone = GetEffectiveBone(g_lockedPawn, viewMatrix, screenWidth, screenHeight);
                Vector3 legitPos = GetBonePosition(g_lockedPawn, legitBone);
                if (legitPos.x != 0.0f || legitPos.y != 0.0f) {
                    bestTargetAngle = Vector3::CalculateAngle(eyePos, legitPos);
                }
            }
            if (g_bAimbotRcs) {
                bestTargetAngle.x -= aimPunch.x * 2.0f;
                bestTargetAngle.y -= aimPunch.y * 2.0f;
            }
            bestTargetAngle.Clamp();
            if (g_fAimbotSmooth > 1.0f) {
                QAngle delta = bestTargetAngle - currentAngles;
                delta.Clamp();
                currentAngles.x += delta.x / g_fAimbotSmooth;
                currentAngles.y += delta.y / g_fAimbotSmooth;
            } else {
                currentAngles = bestTargetAngle;
            }
            currentAngles.Clamp();
            *(QAngle*)(clientBase + cs2_dumper::offsets::client_dll::dwViewAngles) = currentAngles;
        }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            std::printf("[Aimbot] SEH Handled!\n");
        }
    }
    static DWORD lastTriggerbotLog = 0;
    static int lastCrosshairId = -1;
    void RunTriggerbot() {
        if (!g_bTriggerbotEnabled) return;
        bool isKeyDown = (GetAsyncKeyState(g_nTriggerbotKey) & 0x8000) != 0;
        if (!isKeyDown) return;
        __try {
            uintptr_t clientBase = (uintptr_t)GetModuleHandleA("client.dll");
            if (!clientBase) return;
            uintptr_t localPawn = *(uintptr_t*)(clientBase + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
            if (!localPawn) return;
            int crosshairId = *(int*)(localPawn + 0x3EAC);
            DWORD now = GetTickCount();
            if (now - lastTriggerbotLog > 500 || crosshairId != lastCrosshairId) {
                if (crosshairId > 0) {
                    printf("[Triggerbot] CrosshairID: %d\n", crosshairId);
                }
                lastTriggerbotLog = now;
                lastCrosshairId = crosshairId;
            }
            if (crosshairId <= 0) return;
            uintptr_t entityList = *(uintptr_t*)(clientBase + cs2_dumper::offsets::client_dll::dwEntityList);
            if (!entityList) return;
            int entityIndex = crosshairId & 0x7FFF;
            uintptr_t listEntry = *(uintptr_t*)(entityList + 0x10 + 8 * (entityIndex >> 9));
            if (!listEntry) return;
            uintptr_t entity = *(uintptr_t*)(listEntry + 0x70 * (entityIndex & 0x1FF));
            if (!entity) return;
            int entityHealth = *(int*)(entity + 0x354);
            if (entityHealth <= 0) return;
            int localTeam = *(uint8_t*)(localPawn + 0x3F3);
            int entityTeam = *(uint8_t*)(entity + 0x3F3);
            if (entityTeam == localTeam) return;
            if (entityTeam != 0) {
                INPUT inputs[2] = {};
                inputs[0].type = INPUT_MOUSE;
                inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                inputs[1].type = INPUT_MOUSE;
                inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &inputs[0], sizeof(INPUT));
                Sleep(30);
                SendInput(1, &inputs[1], sizeof(INPUT));
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            std::printf("[Triggerbot] SEH Handled!\n");
        }
    }
    inline const char* CONFIG_PATH = "Pixel_Internal.cfg";
    void SaveConfig() {
        std::ofstream f(CONFIG_PATH);
        if (!f.is_open()) { printf("[Config] Could not write %s\n", CONFIG_PATH); return; }
        f << "esp_enabled="       << g_bEspEnabled       << "\n";
        f << "esp_boxes="         << g_bEspBoxes         << "\n";
        f << "esp_health="        << g_bEspHealth        << "\n";
        f << "esp_names="         << g_bEspNames         << "\n";
        f << "esp_fov_circle="    << g_bEspFovCircle     << "\n";
        f << "aimbot_enabled="    << g_bAimbotEnabled    << "\n";
        f << "aimbot_fov="        << g_fAimbotFov        << "\n";
        f << "aimbot_smooth="     << g_fAimbotSmooth     << "\n";
        f << "aimbot_teamcheck="  << g_bAimbotTeamCheck  << "\n";
        f << "aimbot_rcs="        << g_bAimbotRcs        << "\n";
        f << "aimbot_bone="       << g_nAimbotBone       << "\n";
        f << "legit_mode="        << g_bLegitMode        << "\n";
        f << "headshot_pct="      << g_fHeadshotPercent  << "\n";
        f << "closest_bone="      << g_bClosestBone      << "\n";
        f << "switch_delay="      << g_fSwitchDelay      << "\n";
        f << "triggerbot="        << g_bTriggerbotEnabled << "\n";
        f << "esp_teamcheck="     << g_bEspTeamCheck     << "\n";
        f << "esp_skeleton="      << g_bEspSkeleton      << "\n";
        f << "esp_snaplines="     << g_bEspSnaplines     << "\n";
        f << "anti_flash="        << g_bAntiFlash        << "\n";
        f << "aimbot_vischeck="   << g_bAimbotVisCheck   << "\n";
        f << "esp_cornerbox="     << g_bEspCornerBox     << "\n";
        f << "esp_distance="      << g_bEspDistance      << "\n";
        f << "esp_max_dist="      << g_fEspMaxDistance   << "\n";
        f << "snapline_orig="     << g_nSnaplineOrigin   << "\n";
        f << "bomb_timer="        << g_bBombTimer        << "\n";
        f << "tracers="           << g_bBulletTracers    << "\n";
        f << "tracer_life="       << g_fTracerLife       << "\n";
        f << "tracer_speed="      << g_fTracerSpeed      << "\n";
        f << "tracer_thick="      << g_fTracerThickness  << "\n";
        f << "aimbot_key_idx="    << g_nAimbotKeyIndex   << "\n";
        f << "trigger_key_idx="   << g_nTriggerbotKeyIndex<< "\n";
        f << "console_key_idx="   << g_nConsoleKeyIndex  << "\n";
        f << "menu_key_idx="      << g_nMenuKeyIndex     << "\n";
        f << "aimbot_prio="       << g_nAimbotPriority   << "\n";
        f << "col_box=" << g_colEspBox.x<<","<<g_colEspBox.y<<","<<g_colEspBox.z<<","<<g_colEspBox.w<<"\n";
        f << "col_name=" << g_colEspName.x<<","<<g_colEspName.y<<","<<g_colEspName.z<<","<<g_colEspName.w<<"\n";
        f << "col_skel=" << g_colEspSkeleton.x<<","<<g_colEspSkeleton.y<<","<<g_colEspSkeleton.z<<","<<g_colEspSkeleton.w<<"\n";
        f << "col_snap=" << g_colEspSnaplines.x<<","<<g_colEspSnaplines.y<<","<<g_colEspSnaplines.z<<","<<g_colEspSnaplines.w<<"\n";
        f << "col_fov="  << g_colFovCircle.x<<","<<g_colFovCircle.y<<","<<g_colFovCircle.z<<","<<g_colFovCircle.w<<"\n";
        for (int w : SkinChanger::WeaponList) {
            SkinChanger::SkinConfig& sc = SkinChanger::weaponSkins[w];
            if (sc.enabled || sc.paintKit > 0) {
                f << "skin_" << w << "=" << sc.enabled << "," << sc.paintKit << "," << sc.seed << "," << sc.statTrak << "," << sc.wear << "\n";
            }
        }
        f.close();
        printf("[Config] Saved to %s\n", CONFIG_PATH);
    }
    void LoadConfig() {
        std::ifstream f(CONFIG_PATH);
        if (!f.is_open()) { printf("[Config] Could not read %s\n", CONFIG_PATH); return; }
        std::string line;
        while (std::getline(f, line)) {
            auto sep = line.find('=');
            if (sep == std::string::npos) continue;
            std::string key = line.substr(0, sep);
            std::string val = line.substr(sep + 1);
            int   iv = std::stoi(val.empty() ? "0" : val);
            float fv = std::stof(val.empty() ? "0" : val);
            bool  bv = (iv != 0);
            if      (key=="esp_enabled")      g_bEspEnabled      = bv;
            else if (key=="esp_boxes")        g_bEspBoxes        = bv;
            else if (key=="esp_health")       g_bEspHealth       = bv;
            else if (key=="esp_names")        g_bEspNames        = bv;
            else if (key=="esp_fov_circle")   g_bEspFovCircle    = bv;
            else if (key=="aimbot_enabled")   g_bAimbotEnabled   = bv;
            else if (key=="aimbot_vischeck")  g_bAimbotVisCheck  = bv;
            else if (key=="aimbot_fov")       g_fAimbotFov       = fv;
            else if (key=="aimbot_smooth")    g_fAimbotSmooth    = fv;
            else if (key=="aimbot_teamcheck") g_bAimbotTeamCheck = bv;
            else if (key=="aimbot_rcs")       g_bAimbotRcs       = bv;
            else if (key=="aimbot_bone")      g_nAimbotBone      = iv;
            else if (key=="legit_mode")       g_bLegitMode       = bv;
            else if (key=="headshot_pct")     g_fHeadshotPercent = fv;
            else if (key=="closest_bone")     g_bClosestBone     = bv;
            else if (key=="switch_delay")     g_fSwitchDelay     = fv;
            else if (key=="triggerbot")       g_bTriggerbotEnabled = bv;
            else if (key=="esp_teamcheck")    g_bEspTeamCheck    = bv;
            else if (key=="esp_skeleton")     g_bEspSkeleton     = bv;
            else if (key=="esp_snaplines")    g_bEspSnaplines    = bv;
            else if (key=="anti_flash")       g_bAntiFlash       = bv;
            else if (key=="esp_cornerbox")    g_bEspCornerBox    = bv;
            else if (key=="esp_distance")     g_bEspDistance     = bv;
            else if (key=="esp_max_dist")     g_fEspMaxDistance  = fv;
            else if (key=="snapline_orig")    g_nSnaplineOrigin  = iv;
            else if (key=="bomb_timer")       g_bBombTimer       = bv;
            else if (key=="tracers")          g_bBulletTracers   = bv;
            else if (key=="tracer_life")      g_fTracerLife      = fv;
            else if (key=="tracer_speed")     g_fTracerSpeed     = fv;
            else if (key=="tracer_thick")     g_fTracerThickness = fv;
            else if (key=="aimbot_key_idx")   g_nAimbotKeyIndex  = iv;
            else if (key=="trigger_key_idx")  g_nTriggerbotKeyIndex = iv;
            else if (key=="console_key_idx")  g_nConsoleKeyIndex = iv;
            else if (key=="menu_key_idx")     g_nMenuKeyIndex = iv;
            else if (key=="aimbot_prio")      g_nAimbotPriority  = iv;
            else if (key=="col_box" || key=="col_name" || key=="col_skel" || key=="col_snap" || key=="col_fov") {
                float rgba[4] = {1,1,1,1};
                int i=0; size_t pos=0;
                std::string s = val;
                while((pos = s.find(',')) != std::string::npos && i<3) {
                    rgba[i++] = std::stof(s.substr(0, pos)); s.erase(0, pos+1);
                }
                rgba[i] = std::stof(s);
                if (key=="col_box") g_colEspBox = ImVec4(rgba[0],rgba[1],rgba[2],rgba[3]);
                else if (key=="col_name") g_colEspName = ImVec4(rgba[0],rgba[1],rgba[2],rgba[3]);
                else if (key=="col_skel") g_colEspSkeleton = ImVec4(rgba[0],rgba[1],rgba[2],rgba[3]);
                else if (key=="col_snap") g_colEspSnaplines = ImVec4(rgba[0],rgba[1],rgba[2],rgba[3]);
                else if (key=="col_fov") g_colFovCircle = ImVec4(rgba[0],rgba[1],rgba[2],rgba[3]);
            }
            else if (key.find("skin_") == 0) {
                try {
                    int w = std::stoi(key.substr(5));
                    std::stringstream ss(val);
                    std::string item;
                    std::getline(ss, item, ','); SkinChanger::weaponSkins[w].enabled = std::stoi(item);
                    std::getline(ss, item, ','); SkinChanger::weaponSkins[w].paintKit = std::stoi(item);
                    std::getline(ss, item, ','); SkinChanger::weaponSkins[w].seed = std::stoi(item);
                    std::getline(ss, item, ','); SkinChanger::weaponSkins[w].statTrak = std::stoi(item);
                    std::getline(ss, item, ','); SkinChanger::weaponSkins[w].wear = std::stof(item);
                } catch (...) {} 
            }
        }
        f.close();
        g_nAimbotKey = g_nKeyCodes[g_nAimbotKeyIndex];
        g_nTriggerbotKey = g_nKeyCodes[g_nTriggerbotKeyIndex];
        g_nConsoleKey = g_nKeyCodes[g_nConsoleKeyIndex];
        g_nMenuKey = g_nKeyCodes[g_nMenuKeyIndex];
        SkinChanger::forceUpdate.store(true);
        printf("[Config] Loaded from %s\n", CONFIG_PATH);
    }
    DWORD WINAPI AimbotThread(LPVOID lpParam) {
        printf("[+] Aimbot thread started\n");
        while (g_bRunning) {
            if (g_bAimbotEnabled) {
                RunAimbot();
            }
            Sleep(1); 
        }
        printf("[+] Aimbot thread exiting\n");
        return 0;
    }
    DWORD WINAPI TriggerbotThread(LPVOID lpParam) {
        printf("[+] Triggerbot thread started\n");
        while (g_bRunning) {
            if (g_bTriggerbotEnabled) {
                RunTriggerbot();
            }
            Sleep(5); 
        }
        printf("[+] Triggerbot thread exiting\n");
        return 0;
    }
}
