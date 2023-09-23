//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
// Copyright(C) 2020-2023 Leonid Murin (Dasperal)
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
//



#include <SDL_messagebox.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#endif

#include "SDL.h"
#include "doomtype.h"
#include "d_name.h"
#include "m_argv.h"
#include "m_misc.h"
#include "i_system.h"
#include "jn.h"

// [JN] Объём необходимой памяти увеличен вдвое
#define DEFAULT_RAM 16*2 /* MiB */
#define MIN_RAM     4*4  /* MiB */

typedef struct atexit_listentry_s
{
    struct atexit_listentry_s* next;
    atexit_func_t func;
    boolean run_on_error;
} atexit_listentry_t;

typedef struct message_s
{
    struct message_s* next;
    unsigned int type;
    char msgbuf[512];
} message_t;

static atexit_listentry_t* exit_funcs = NULL;
static message_t* message_queue = NULL;
static boolean already_quitting = false;

void I_AtExit(atexit_func_t func, boolean run_on_error)
{
    atexit_listentry_t *entry;

    entry = malloc(sizeof(*entry));

    entry->func = func;
    entry->run_on_error = run_on_error;
    entry->next = exit_funcs;
    exit_funcs = entry;
}

// Zone memory auto-allocation function that allocates the zone size
// by trying progressively smaller zone sizes until one is found that
// works.

static byte *AutoAllocMemory(int *size, int default_ram, int min_ram)
{
    byte *zonemem;

    // Allocate the zone memory.  This loop tries progressively smaller
    // zone sizes until a size is found that can be allocated.
    // If we used the -mb command line parameter, only the parameter
    // provided is accepted.

    zonemem = NULL;

    while (zonemem == NULL)
    {
        // We need a reasonable minimum amount of RAM to start.

        if (default_ram < min_ram)
        {
            I_QuitWithError(english_language ?
                            "Unable to allocate %i MiB of RAM for zone" :
                            "Невозможно обнаружить %i МБ памяти для распределения",
                            default_ram);
        }

        // Try to allocate the zone memory.

        *size = default_ram * 1024 * 1024;

        zonemem = malloc(*size);

        // Failed to allocate?  Reduce zone size until we reach a size
        // that is acceptable.

        if (zonemem == NULL)
        {
            default_ram -= 1;
        }
    }

    return zonemem;
}

byte *I_ZoneBase (int *size)
{
    byte *zonemem;
    int min_ram, default_ram;
    int p;
    static int i = 1;
    static boolean printed = false;

    //!
    // @arg <mb>
    //
    // Specify the heap size, in MiB (default 16).
    //

    p = M_CheckParmWithArgs("-mb", 1);

    if (p > 0)
    {
        default_ram = atoi(myargv[p+1]);
        min_ram = default_ram;
    }
    else
    {
        default_ram = DEFAULT_RAM;
        min_ram = MIN_RAM;
    }

    // [crispy] do not allocate new zones ad infinitum
    if (i > 16)
    {
        min_ram = default_ram + 1;
    }

    zonemem = AutoAllocMemory(size, default_ram * i, min_ram * i);

    // [crispy] if called again, allocate another zone twice as big
    i *= 2;

    if (!printed)
    {
        printf(english_language ?
               "    zone memory: %p, %x MB allocated for zone\n" :
               "    зональная память: %p, выделено %x Мбайт\n", 
               zonemem, *size >> 20); // [crispy] human-understandable zone heap size
    }

    printed = true;

    return zonemem;
}

// 
// I_ConsoleStdout
//
// Returns true if stdout is a real console, false if it is a file
//

boolean I_ConsoleStdout(void)
{
#ifdef _WIN32
    // SDL "helpfully" always redirects stdout to a file.
    return false;
#else
    return isatty(fileno(stdout));
#endif
}

//
// I_Quit
//

static inline void do_quit(boolean is_error)
{
    if(already_quitting)
    {
        printf(english_language ?
               "Warning: recursive call to I_Quit* detected.\n" :
               "Внимание: обнаружен рекурсивный вызов I_Quit*.\n");
        exit(-1);
    }
    else
    {
        already_quitting = true;
    }

    // Run through all exit functions
    atexit_listentry_t* entry = exit_funcs;
    while(entry != NULL)
    {
        if(!is_error || entry->run_on_error)
        {
            entry->func();
        }
        entry = entry->next;
    }

    // Pop up a GUI dialog box to show the error message
    if(!M_ParmExists("-nogui"))
    {
        while(message_queue != NULL)
        {
            message_t* current_message = message_queue;
#ifdef _WIN32
            // [JN] UTF-8 retranslations of error message and window title.
            wchar_t win_error_message[1024];
            wchar_t win_error_title[128];

            // [JN] Use nicer Windows-styled dialog box.
            MultiByteToWideChar(CP_UTF8, 0, current_message->msgbuf, -1, win_error_message, 1024);
            MultiByteToWideChar(CP_UTF8, 0, RD_Project_String, -1, win_error_title, 128);

            MessageBoxW(NULL, win_error_message, win_error_title, current_message->type);
#else
            SDL_ShowSimpleMessageBox(current_message->type, RD_Project_String, current_message->msgbuf, NULL);
#endif
            message_queue = current_message->next;
            free(current_message);
        }
    }

    SDL_Quit();
    exit(0);
}

void I_Quit(void)
{
    do_quit(false);
}

void I_QuitWithError(char* error, ...)
{
    va_list argptr;
    va_start(argptr, error);
    I_AddError(error, argptr);
    va_end(argptr);

    do_quit(true);
}

void I_QuitWithMessage(char* error, ...)
{
    va_list argptr;
    va_start(argptr, error);
    I_AddMessage(error, argptr);
    va_end(argptr);

    do_quit(false);
}

void I_AddError(char* error, ...)
{
    va_list argptr;

    // Message first.
    va_start(argptr, error);
    vprintf(error, argptr);
    printf("\n\n");
    va_end(argptr);
    fflush(stdout);

    // Write a copy of the msg into buffer.
    message_t* msg = malloc(sizeof(message_t));
    va_start(argptr, error);
    memset(msg->msgbuf, 0, sizeof(msg->msgbuf));
    M_vsnprintf(msg->msgbuf, sizeof(msg->msgbuf), error, argptr);
    va_end(argptr);

    msg->type = SDL_MESSAGEBOX_ERROR;
    msg->next = message_queue;
    message_queue = msg;
}

void I_AddMessage(char* message, ...)
{
    va_list argptr;

    // Message first.
    va_start(argptr, message);
    vprintf(message, argptr);
    printf("\n\n");
    va_end(argptr);
    fflush(stdout);

    // Write a copy of the message into buffer.
    message_t* msg = malloc(sizeof(message_t));
    va_start(argptr, message);
    memset(msg->msgbuf, 0, sizeof(msg->msgbuf));
    M_vsnprintf(msg->msgbuf, sizeof(msg->msgbuf), message, argptr);
    va_end(argptr);

    msg->type = SDL_MESSAGEBOX_INFORMATION;
    msg->next = message_queue;
    message_queue = msg;
}


//
// I_Realloc
//

void *I_Realloc(void *ptr, size_t size)
{
    void *new_ptr;

    new_ptr = realloc(ptr, size);

    if (size != 0 && new_ptr == NULL)
    {
        I_QuitWithError(english_language ?
                        "I_Realloc: failed on reallocation of %zu bytes" :
                        "I_Realloc: ошибка переобнаружения %zu байт",
                        size);
    }

    return new_ptr;
}

//
// Read Access Violation emulation.
//
// From PrBoom+, by entryway.
//

// C:\>debug
// -d 0:0
//
// DOS 6.22:
// 0000:0000  (57 92 19 00) F4 06 70 00-(16 00)
// DOS 7.1:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// Win98:
// 0000:0000  (9E 0F C9 00) 65 04 70 00-(16 00)
// DOSBox under XP:
// 0000:0000  (00 00 00 F1) ?? ?? ?? 00-(07 00)

#define DOS_MEM_DUMP_SIZE 10

static const unsigned char mem_dump_dos622[DOS_MEM_DUMP_SIZE] = {
  0x57, 0x92, 0x19, 0x00, 0xF4, 0x06, 0x70, 0x00, 0x16, 0x00};
static const unsigned char mem_dump_win98[DOS_MEM_DUMP_SIZE] = {
  0x9E, 0x0F, 0xC9, 0x00, 0x65, 0x04, 0x70, 0x00, 0x16, 0x00};
static const unsigned char mem_dump_dosbox[DOS_MEM_DUMP_SIZE] = {
  0x00, 0x00, 0x00, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00};
static unsigned char mem_dump_custom[DOS_MEM_DUMP_SIZE];

static const unsigned char *dos_mem_dump = mem_dump_dos622;

boolean I_GetMemoryValue(unsigned int offset, void *value, int size)
{
    static boolean firsttime = true;

    if (firsttime)
    {
        int p, i, val;

        firsttime = false;
        i = 0;

        //!
        // @category compat
        // @arg <version>
        //
        // Specify DOS version to emulate for NULL pointer dereference
        // emulation.  Supported versions are: dos622, dos71, dosbox.
        // The default is to emulate DOS 7.1 (Windows 98).
        //

        p = M_CheckParmWithArgs("-setmem", 1);

        if (p > 0)
        {
            if (!strcasecmp(myargv[p + 1], "dos622"))
            {
                dos_mem_dump = mem_dump_dos622;
            }
            if (!strcasecmp(myargv[p + 1], "dos71"))
            {
                dos_mem_dump = mem_dump_win98;
            }
            else if (!strcasecmp(myargv[p + 1], "dosbox"))
            {
                dos_mem_dump = mem_dump_dosbox;
            }
            else
            {
                for (i = 0; i < DOS_MEM_DUMP_SIZE; ++i)
                {
                    ++p;

                    if (p >= myargc || myargv[p][0] == '-')
                    {
                        break;
                    }

                    M_StrToInt(myargv[p], &val);
                    mem_dump_custom[i++] = (unsigned char) val;
                }

                dos_mem_dump = mem_dump_custom;
            }
        }
    }

    switch (size)
    {
    case 1:
        *((unsigned char *) value) = dos_mem_dump[offset];
        return true;
    case 2:
        *((unsigned short *) value) = dos_mem_dump[offset]
                                    | (dos_mem_dump[offset + 1] << 8);
        return true;
    case 4:
        *((unsigned int *) value) = dos_mem_dump[offset]
                                  | (dos_mem_dump[offset + 1] << 8)
                                  | (dos_mem_dump[offset + 2] << 16)
                                  | (dos_mem_dump[offset + 3] << 24);
        return true;
    }

    return false;
}

#ifdef _WIN32
void I_ConsolePause(void)
{
    printf("Press any key to continue . . .");
    getch();
    printf("\n");
}
#endif

