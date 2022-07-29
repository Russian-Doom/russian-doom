//
// Copyright(C) 2021 Roman Fomin
// Copyright(C) 2022 Roman Fomin, Julian Nechaevsky, Dasperal/Leonid Murin, kmeaw
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

#include "rd_io.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include "SDL_syswm.h"

wchar_t* ConvertToUtf8(const char *str)
{
    wchar_t *wstr = NULL;
    int wlen = 0;

    wlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

    wstr = (wchar_t *) malloc(sizeof(wchar_t) * wlen);

    MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, wlen);

    return wstr;
}

FILE* D_fopen(const char *filename, const char *mode)
{
    FILE *f;
    wchar_t *wname = NULL;
    wchar_t *wmode = NULL;

    wname = ConvertToUtf8(filename);
    wmode = ConvertToUtf8(mode);

    f = _wfopen(wname, wmode);

    if(wname) free(wname);
    if(wmode) free(wmode);
    return f;
}

int D_remove(const char *path)
{
    wchar_t *wpath = NULL;
    int ret;

    wpath = ConvertToUtf8(path);

    ret = _wremove(wpath);

    if(wpath) free(wpath);
    return ret;
}

int D_rename(const char *oldname, const char *newname)
{
    wchar_t *wold = NULL;
    wchar_t *wnew = NULL;
    int ret;

    wold = ConvertToUtf8(oldname);
    wnew = ConvertToUtf8(newname);

    ret = _wrename(wold, wnew);

    if(wold) free(wold);
    if(wnew) free(wnew);
    return ret;
}

int D_mkdir(const char *dirname)
{
    wchar_t *wdir;
    int ret;

    wdir = ConvertToUtf8(dirname);

    ret = _wmkdir(wdir);

    if(wdir) free(wdir);
    return ret;
}

typedef long (__stdcall *PRTLGETVERSION)(PRTL_OSVERSIONINFOEXW);

int I_CheckWindows11(void)
{
    PRTLGETVERSION  pRtlGetVersion = (PRTLGETVERSION) GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlGetVersion");

    if(pRtlGetVersion)
    {
        OSVERSIONINFOEXW info;

        memset(&info, 0, sizeof(OSVERSIONINFOEXW));
        info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

        pRtlGetVersion((PRTL_OSVERSIONINFOEXW)&info);

        if(info.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            if(info.dwMajorVersion == 10)
            {
                if(info.dwBuildNumber >= 22000)
                    return 1;
            }
        }
    }

    return 0;
}

typedef HRESULT (WINAPI *PDWMSETWINDOWATTRIBUTE)(HWND, DWORD, LPCVOID, DWORD);

void DisableWinRound(SDL_Window* screen)
{
    HMODULE hDllDwmApi;
    PDWMSETWINDOWATTRIBUTE pDwmSetWindowAttribute;
    SDL_SysWMinfo wmInfo;
    HWND hwnd;
    int noround = 1; // DWMWCP_DONOTROUND

    if(!I_CheckWindows11())
		return;

    pDwmSetWindowAttribute = NULL;
    hDllDwmApi = LoadLibrary("dwmapi.dll");
    if(hDllDwmApi != NULL)
    {
        pDwmSetWindowAttribute = (PDWMSETWINDOWATTRIBUTE) GetProcAddress(hDllDwmApi, "DwmSetWindowAttribute");
    }
    if(pDwmSetWindowAttribute != NULL)
    {
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(screen, &wmInfo);
        hwnd = wmInfo.info.win.window;
        pDwmSetWindowAttribute(hwnd, 33, // DWMWA_WINDOW_CORNER_PREFERENCE
                               &noround, sizeof(noround));
    }
}

#endif
