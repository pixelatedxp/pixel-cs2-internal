#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <map>
#include <mutex>
#include <atomic>
#include <cstdio>
#include <vector>
#include <cstring>
namespace SCOffsets {
    constexpr std::ptrdiff_t m_AttributeManager       = 0x1378;
    constexpr std::ptrdiff_t m_nFallbackPaintKit      = 0x1850;
    constexpr std::ptrdiff_t m_nFallbackSeed          = 0x1854;
    constexpr std::ptrdiff_t m_flFallbackWear         = 0x1858;
    constexpr std::ptrdiff_t m_nFallbackStatTrak      = 0x185C;
    constexpr std::ptrdiff_t m_Item                   = 0x50;
    constexpr std::ptrdiff_t m_iItemDefinitionIndex   = 0x1BA;
    constexpr std::ptrdiff_t m_iItemIDHigh            = 0x1D0;
    constexpr std::ptrdiff_t m_AttributeList          = 0x208;
    constexpr std::ptrdiff_t m_Attributes             = 0x8;
    constexpr std::ptrdiff_t m_pClippingWeapon        = 0x3DC0;
    constexpr std::ptrdiff_t m_iHealth                = 0x354;
    constexpr std::ptrdiff_t m_lifeState              = 0x35C;
}

namespace SkinChanger {

    struct SkinConfig {
        int   paintKit  = 0;
        float wear      = 0.001f;
        int   seed      = 0;
        int   statTrak  = -1;
        bool  enabled   = false;
    };
    template<typename T>
    inline T SCRead(uintptr_t addr) {
        if (!addr) return T{};
        __try { return *reinterpret_cast<T*>(addr); }
        __except (EXCEPTION_EXECUTE_HANDLER) { return T{}; }
    }
    template<typename T>
    inline void SCWrite(uintptr_t addr, const T& val) {
        if (!addr) return;
        __try { *reinterpret_cast<T*>(addr) = val; }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    struct CPtrGameVector {
        uint64_t  size;
        uintptr_t ptr;
    };
#pragma pack(push,1)
    struct CEconItemAttribute {
        uintptr_t vtable;
        uintptr_t owner;
        char      pad[32];
        uint16_t  defIndex;
        char      pad2[2];
        float     value;
        float     initValue;
        int32_t   refundableCurrency;
        bool      setBonus;
        char      pad3[7];
    }; 
#pragma pack(pop)
    inline std::map<int, SkinConfig> weaponSkins;
    inline std::atomic<bool>         forceUpdate  = false;
    inline std::mutex                configMutex;
    inline CEconItemAttribute*       g_attrBuffer = nullptr;
    inline uintptr_t                 regenAddr    = 0;
    inline bool                      regenPatched = false;
    inline uintptr_t                 lastWeapon   = 0;
    inline int                       lastKit      = 0;
    inline const char* GetWeaponName(int d) {
        switch (d) {
        case 1: return "Desert Eagle"; case 2: return "Dual Berettas"; case 3: return "Five-SeveN";
        case 4: return "Glock-18"; case 7: return "AK-47"; case 8: return "AUG";
        case 9: return "AWP"; case 10: return "FAMAS"; case 11: return "G3SG1";
        case 13: return "Galil AR"; case 14: return "M249"; case 16: return "M4A4";
        case 17: return "MAC-10"; case 19: return "P90"; case 23: return "MP5-SD";
        case 24: return "UMP-45"; case 25: return "XM1014"; case 26: return "PP-Bizon";
        case 27: return "MAG-7"; case 28: return "Negev"; case 29: return "Sawed-Off";
        case 30: return "Tec-9"; case 31: return "Zeus x27"; case 32: return "P2000";
        case 33: return "MP7"; case 34: return "MP9"; case 35: return "Nova";
        case 36: return "P250"; case 38: return "SCAR-20"; case 39: return "SG 553";
        case 40: return "SSG 08"; case 42: return "CT Knife"; case 59: return "T Knife";
        case 60: return "M4A1-S"; case 61: return "USP-S"; case 63: return "CZ75-Auto"; 
        case 64: return "R8 Revolver";
        case 500: return "Bayonet"; case 503: return "Classic Knife"; case 505: return "Flip Knife";
        case 506: return "Gut Knife"; case 507: return "Karambit"; case 508: return "M9 Bayonet";
        case 509: return "Huntsman Knife"; case 512: return "Falchion Knife"; case 514: return "Bowie Knife";
        case 515: return "Butterfly Knife"; case 516: return "Shadow Daggers"; case 519: return "Ursus Knife";
        case 520: return "Navaja Knife"; case 521: return "Nomad Knife"; case 522: return "Stiletto Knife";
        case 523: return "Talon Knife"; case 524: return "Paracord Knife"; case 525: return "Skeleton Knife";
        case 526: return "Survival Knife";
        default: return "Weapon";
        }
    }
    inline const int WeaponCount = 57;
    inline const int WeaponList[WeaponCount] = {
        1, 2, 3, 4, 7, 8, 9, 10, 11, 13, 14, 16, 17, 19, 23, 24, 25, 26, 27, 28, 
        29, 30, 31, 32, 33, 34, 35, 36, 38, 39, 40, 42, 59, 60, 61, 63, 64,
        500, 503, 505, 506, 507, 508, 509, 512, 514, 515, 516, 519, 520, 521, 522, 523, 524, 525, 526
    };
    inline CEconItemAttribute MakeAttr(uint16_t def, float val) {
        CEconItemAttribute a{};
        a.defIndex  = def;
        a.value     = val;
        a.initValue = val;
        return a;
    }

    inline void CreateAttributes(uintptr_t item, int kit, int seed, float wear) {
        if (kit <= 0) return;
        uintptr_t listAddr = item + SCOffsets::m_AttributeList + SCOffsets::m_Attributes;
        CPtrGameVector existing = SCRead<CPtrGameVector>(listAddr);
        if (existing.size > 0 || existing.ptr != 0) return;

        if (!g_attrBuffer) {
            g_attrBuffer = reinterpret_cast<CEconItemAttribute*>(
                VirtualAlloc(nullptr, sizeof(CEconItemAttribute) * 3,
                    MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
            if (!g_attrBuffer) return;
        }
        g_attrBuffer[0] = MakeAttr(6, (float)kit);
        g_attrBuffer[1] = MakeAttr(7, (float)seed);
        g_attrBuffer[2] = MakeAttr(8, wear);

        CPtrGameVector nv;
        nv.size = 3;
        nv.ptr  = reinterpret_cast<uintptr_t>(g_attrBuffer);
        SCWrite<CPtrGameVector>(listAddr, nv);
    }

    inline void RemoveAttributes(uintptr_t item) {
        uintptr_t listAddr = item + SCOffsets::m_AttributeList + SCOffsets::m_Attributes;
        CPtrGameVector existing = SCRead<CPtrGameVector>(listAddr);
        if (existing.size == 0) return;
        SCWrite<CPtrGameVector>(listAddr, CPtrGameVector{});
    }
    inline uintptr_t SigScanLocal(uintptr_t base, size_t sz, const char* pat) {
        struct ByteEntry { uint8_t byte; bool wildcard; };
        std::vector<ByteEntry> sig;
        const char* p = pat;
        while (*p) {
            if (*p == ' ') { p++; continue; }
            if (*p == '?') {
                sig.push_back({0, true});
                while (*p == '?') p++;
            } else {
                sig.push_back({(uint8_t)strtoul(p, const_cast<char**>(&p), 16), false});
            }
        }
        uint8_t* bytes = reinterpret_cast<uint8_t*>(base);
        for (size_t i = 0; i < sz - sig.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < sig.size(); j++) {
                if (!sig[j].wildcard && bytes[i+j] != sig[j].byte) { found = false; break; }
            }
            if (found) return base + i;
        }
        return 0;
    }

    inline void InitRegen() {
        if (regenAddr) return;
        HMODULE hClient = GetModuleHandleW(L"client.dll");
        if (!hClient) return;
        MODULEINFO mi{};
        if (!GetModuleInformation(GetCurrentProcess(), hClient, &mi, sizeof(mi))) return;

        regenAddr = SigScanLocal(reinterpret_cast<uintptr_t>(hClient), mi.SizeOfImage,
            "48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10");

        if (regenAddr) {
            uint16_t combined = (uint16_t)(SCOffsets::m_AttributeManager + SCOffsets::m_Item +
                SCOffsets::m_AttributeList + SCOffsets::m_Attributes);
            DWORD old;
            if (VirtualProtect(reinterpret_cast<void*>(regenAddr + 0x52), 2, PAGE_EXECUTE_READWRITE, &old)) {
                *reinterpret_cast<uint16_t*>(regenAddr + 0x52) = combined;
                VirtualProtect(reinterpret_cast<void*>(regenAddr + 0x52), 2, old, &old);
                regenPatched = true;
                printf("[SkinChanger] Regen patched at 0x%llX\n", regenAddr);
            }
        } else {
            printf("[SkinChanger] RegenerateWeaponSkins NOT found\n");
        }
    }

    inline void CallRegen() {
        if (!regenAddr || !regenPatched) return;
        __try {
            typedef void(__fastcall* RegenFn)();
            reinterpret_cast<RegenFn>(regenAddr)();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            printf("[SkinChanger] Regen crashed (SEH)\n");
        }
    }

    inline void ApplyAndRegen(uintptr_t weapon, const SkinConfig& skin, uint16_t def) {
        uintptr_t item = weapon + SCOffsets::m_AttributeManager + SCOffsets::m_Item;
        uint32_t origIDHigh = SCRead<uint32_t>(item + SCOffsets::m_iItemIDHigh);

        SCWrite<uint32_t>(item  + SCOffsets::m_iItemIDHigh, 0xFFFFFFFF);
        SCWrite<int32_t>(weapon + SCOffsets::m_nFallbackPaintKit, skin.paintKit);
        SCWrite<float>  (weapon + SCOffsets::m_flFallbackWear,    skin.wear);
        SCWrite<int32_t>(weapon + SCOffsets::m_nFallbackSeed,     skin.seed);
        SCWrite<int32_t>(weapon + SCOffsets::m_nFallbackStatTrak, skin.statTrak);

        CreateAttributes(item, skin.paintKit, skin.seed, skin.wear);
        CallRegen();
        RemoveAttributes(item);
        SCWrite<uint32_t>(item  + SCOffsets::m_iItemIDHigh, origIDHigh);
        SCWrite<int32_t>(weapon + SCOffsets::m_nFallbackPaintKit, 0);
        SCWrite<float>  (weapon + SCOffsets::m_flFallbackWear,    0.0f);
        SCWrite<int32_t>(weapon + SCOffsets::m_nFallbackSeed,     0);
        SCWrite<int32_t>(weapon + SCOffsets::m_nFallbackStatTrak, -1);
    }
    inline void Tick(uintptr_t clientBase, uintptr_t localPawn) {
        if (!clientBase || !localPawn) return;

        uint8_t  life   = SCRead<uint8_t> (localPawn + SCOffsets::m_lifeState);
        int32_t  hp     = SCRead<int32_t> (localPawn + SCOffsets::m_iHealth);
        if (life != 0 || hp <= 0) { lastWeapon = 0; lastKit = 0; return; }

        InitRegen();

        static int tickCounter = 0;
        if (++tickCounter >= 2000) {
            forceUpdate.store(true);
            tickCounter = 0;
        }

        bool force = forceUpdate.load();
        std::lock_guard<std::mutex> lock(configMutex);

        uintptr_t activeWeapon = SCRead<uintptr_t>(localPawn + SCOffsets::m_pClippingWeapon);
        if (!activeWeapon) return;

        uintptr_t item    = activeWeapon + SCOffsets::m_AttributeManager + SCOffsets::m_Item;
        uint16_t  def     = SCRead<uint16_t>(item + SCOffsets::m_iItemDefinitionIndex);
        bool      isWep   = (def > 0 && def < 70) || (def >= 500 && def < 600);
        if (!isWep || def == 31) return;

        int lookupIndex = def;
        if (def == 42 || def == 59 || (def >= 500 && def < 600)) { 
            lookupIndex = 42; 
            for (auto& [k, cfg] : weaponSkins) {
                if ((k == 42 || k == 59 || (k >= 500 && k < 600)) && cfg.enabled) { 
                    lookupIndex = k; 
                    break; 
                }
            }
        }

        auto it = weaponSkins.find(lookupIndex);
        if (it == weaponSkins.end() || !it->second.enabled || it->second.paintKit <= 0) return;

        const SkinConfig& skin = it->second;
        bool needApply = force || (activeWeapon != lastWeapon) || (skin.paintKit != lastKit);
        if (!needApply) return;
        life = SCRead<uint8_t>(localPawn + SCOffsets::m_lifeState);
        hp   = SCRead<int32_t>(localPawn + SCOffsets::m_iHealth);
        if (life != 0 || hp <= 0) return;

        SCWrite<uint32_t>(item + SCOffsets::m_iItemIDHigh, 0);
        ApplyAndRegen(activeWeapon, skin, def);
        lastWeapon = activeWeapon;
        lastKit    = skin.paintKit;

        if (force) forceUpdate.store(false);
    }
}
