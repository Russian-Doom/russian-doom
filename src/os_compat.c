//
// Copyright(C) 2021-2022 Roman Fomin
// Copyright(C) 2022 kmeaw
// Copyright(C) 2022-2023 Julian Nechaevsky
// Copyright(C) 2022-2024 Leonid Murin (Dasperal)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  Several compatibility functions for Windows OS

#include "os_compat.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

static os_version_t osVersion = 0;

os_version_t OS_getVersion(void)
{
    if(osVersion >= Unknown)
        return osVersion;
#ifdef _WIN32
    typedef long (__stdcall *PRTLGETVERSION)(PRTL_OSVERSIONINFOEXW);

    PRTLGETVERSION  pRtlGetVersion = (PRTLGETVERSION) GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion");
    if(pRtlGetVersion)
    {
        OSVERSIONINFOEXW info;

        memset(&info, 0, sizeof(OSVERSIONINFOEXW));
        info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
        pRtlGetVersion((PRTL_OSVERSIONINFOEXW)&info);

        if(info.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if(info.dwMajorVersion >= 10)
            {
                // if(info.dwMinorVersion >= 0)
                if(info.dwBuildNumber >= 22000)
                    return osVersion = Windows_11;
                else
                    return osVersion = Windows_10;
            }
            else if(info.dwMajorVersion >= 6)
            {
                if(info.dwMinorVersion >= 3)
                    return osVersion = Windows_8_1;
                else if(info.dwMinorVersion == 2)
                    return osVersion = Windows_8;
                else if(info.dwMinorVersion == 1)
                    return osVersion = Windows_7;
                else if(info.dwMinorVersion == 0)
                    return osVersion = Windows_Vista;
            }
            else if(info.dwMajorVersion == 5)
            {
                if(info.dwMinorVersion >= 2)
                    return osVersion = Windows_XP64;
                else if(info.dwMinorVersion == 1)
                    return osVersion = Windows_XP;
            }
        }
    }
#endif
    return osVersion = Unknown;
}

#ifdef _WIN32
typedef HRESULT (WINAPI *PDWMSETWINDOWATTRIBUTE)(HWND, DWORD, LPCVOID, DWORD);

void OS_DisableWindowCornersRounding(SDL_Window* window)
{
    if(OS_getVersion() != Windows_11)
        return;

    PDWMSETWINDOWATTRIBUTE pDwmSetWindowAttribute = NULL;
    HMODULE hDllDwmApi = LoadLibrary("dwmapi.dll");
    if(hDllDwmApi != NULL)
    {
        pDwmSetWindowAttribute = (PDWMSETWINDOWATTRIBUTE) GetProcAddress(hDllDwmApi, "DwmSetWindowAttribute");
    }
    if(pDwmSetWindowAttribute != NULL)
    {
        SDL_SysWMinfo wmInfo;
        int noround = 1; // DWMWCP_DONOTROUND

        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);
        HWND hwnd = wmInfo.info.win.window;
        pDwmSetWindowAttribute(hwnd, 33, // DWMWA_WINDOW_CORNER_PREFERENCE
            &noround, sizeof(noround));
    }
}
#endif
