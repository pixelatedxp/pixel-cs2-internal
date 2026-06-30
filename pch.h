#ifndef PCH_H
#define PCH_H
    const int WeaponCount = 57;
    const int WeaponList[WeaponCount] = {
        1, 2, 3, 4, 7, 8, 9, 10, 11, 13, 14, 16, 17, 19, 23, 24, 25, 26, 27, 28, 
        29, 30, 31, 32, 33, 34, 35, 36, 38, 39, 40, 42, 59, 60, 61, 63, 64,
        500, 503, 505, 506, 507, 508, 509, 512, 514, 515, 516, 519, 520, 521, 522, 523, 524, 525, 526
    };
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
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include "minhook-master/include/MinHook.h"
#include "imgui-master/imgui.h"
#include "imgui-master/backends/imgui_impl_dx11.h"
#include "imgui-master/backends/imgui_impl_win32.h"
#include <cstdint>
#include <string>
#include <cmath>
#include "Cs2-Offsets/offsets.hpp"
#include "Cs2-Offsets/client_dll.hpp"
#include "Cs2-Offsets/buttons.hpp"
#include "sdk/structs.h"
#endif 
