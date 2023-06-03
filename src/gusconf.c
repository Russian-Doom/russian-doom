//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2023 Julian Nechaevsky
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
//     GUS emulation code.
//
//     Actually emulating a GUS is far too much work; fortunately
//     GUS "emulation" already exists in the form of Timidity, which
//     supports GUS patch files. This code therefore converts Doom's
//     DMXGUS lump into an equivalent Timidity configuration file.
//


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL_mixer.h>

#include "gusconf.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"
#include "jn.h"

#define MAX_INSTRUMENTS 256

typedef struct
{
    char *patch_names[MAX_INSTRUMENTS];
    int used[MAX_INSTRUMENTS];
    int mapping[MAX_INSTRUMENTS];
    unsigned int count;
} gus_config_t;

char *gus_patch_path = "";
int gus_ram_kb = 1024;

static unsigned int MappingIndex(void)
{
    unsigned int result = gus_ram_kb / 256;

    if (result < 1)
    {
        return 1;
    }
    else if (result > 4)
    {
        return 4;
    }
    else
    {
        return result;
    }
}

static int SplitLine(char *line, char **fields, unsigned int max_fields)
{
    unsigned int num_fields;
    char *p;

    fields[0] = line;
    num_fields = 1;

    for (p = line; *p != '\0'; ++p)
    {
        if (*p == ',')
        {
            *p = '\0';

            // Skip spaces following the comma.
            do
            {
                ++p;
            } while (*p != '\0' && isspace(*p));

            fields[num_fields] = p;
            ++num_fields;
            --p;

            if (num_fields >= max_fields)
            {
                break;
            }
        }
        else if (*p == '#')
        {
            *p = '\0';
            break;
        }
    }

    // Strip off trailing whitespace from the end of the line.
    p = fields[num_fields - 1] + strlen(fields[num_fields - 1]);
    while (p > fields[num_fields - 1] && isspace(*(p - 1)))
    {
        --p;
        *p = '\0';
    }

    return num_fields;
}

static void ParseLine(gus_config_t *config, char *line)
{
    char *fields[6];
    unsigned int i;
    unsigned int num_fields;
    unsigned int instr_id, mapped_id;

    num_fields = SplitLine(line, fields, 6);

    if (num_fields < 6)
    {
        return;
    }

    instr_id = atoi(fields[0]);

    // Skip non GM percussions.
    if ((instr_id >= 128 && instr_id < 128 + 35) || instr_id > 128 + 81)
    {
        return;
    }

    mapped_id = atoi(fields[MappingIndex()]);

    for (i = 0; i < config->count; i++)
    {
        if (config->used[i] == mapped_id)
        {
            break;
        }
    }
    
    if (i == config->count)
    {
        // DMX uses wrong patch name (we should use name of 'mapped_id'
        // instrument, but DMX uses name of 'instr_id' instead).
        free(config->patch_names[i]);
        config->patch_names[i] = M_StringDuplicate(fields[5]);
        config->used[i] = mapped_id;
        config->count++;
    }
    config->mapping[instr_id] = i;
}

static void ParseDMXConfig(char *dmxconf, gus_config_t *config)
{
    char *p, *newline;
    unsigned int i;

    memset(config, 0, sizeof(gus_config_t));

    for (i = 0; i < MAX_INSTRUMENTS; ++i)
    {
        config->mapping[i] = -1;
        config->used[i] = -1;
    }

    config->count = 0;

    p = dmxconf;

    for (;;)
    {
        newline = strchr(p, '\n');

        if (newline != NULL)
        {
            *newline = '\0';
        }

        ParseLine(config, p);

        if (newline == NULL)
        {
            break;
        }
        else
        {
            p = newline + 1;
        }
    }
}

static void FreeDMXConfig(gus_config_t *config)
{
    unsigned int i;

    for (i = 0; i < MAX_INSTRUMENTS; ++i)
    {
        free(config->patch_names[i]);
    }
}

static char *ReadDMXConfig(void)
{
    int lumpnum;
    unsigned int len;
    char *data;

    // TODO: This should be chosen based on gamemode == commercial:

    lumpnum = W_CheckNumForName("DMXGUS");

    if (lumpnum < 0)
    {
        lumpnum = W_GetNumForName("DMXGUSC");
    }

    len = W_LumpLength(lumpnum);
    data = Z_Malloc(len + 1, PU_STATIC, NULL);
    W_ReadLump(lumpnum, data);

    data[len] = '\0';
    return data;
}

static boolean WriteTimidityConfig(char *path, gus_config_t *config, char* gus_patches_path)
{
    FILE *fstream;
    unsigned int i;
    const SDL_version* sdl_mixer_version = Mix_Linked_Version();

    if(!LIB_VERSION_ATLEAST(sdl_mixer_version, 2, 5, 0) && strchr(gus_patches_path, ' '))
    {
        printf(english_language ?
               "\tError: The path contains spaces, which are not supported by your SDL_mixer library. Update SDL_mixer to at least 2.5.0\n" :
               "\tОшибка: Путь содержит пробелы, что не поддерживается вашей версией библиотеки SDL_mixer. Обновите SDL_mixer хотя бы до версии 2.5.0\n");
    }

    fstream = M_fopen(path, "w");

    if (fstream == NULL)
    {
        printf(english_language ?
               "\tError: Could not write Timidity config\n" :
               "\tОшибка: Не удалось записать конфигурацию Timidity\n");
        return false;
    }

    fprintf(fstream, "# Autogenerated Timidity config.\n\n");
    if(LIB_VERSION_ATLEAST(sdl_mixer_version, 2, 5, 0))
    {
        fprintf(fstream, "dir \"%s\"\n", gus_patches_path);
    }
    else
    {
        fprintf(fstream, "dir %s\n", gus_patches_path);
    }

    fprintf(fstream, "\nbank 0\n\n");

    for (i = 0; i < 128; ++i)
    {
        if (config->mapping[i] >= 0 && config->mapping[i] < MAX_INSTRUMENTS
         && config->patch_names[config->mapping[i]] != NULL)
        {
            fprintf(fstream, "%u %s\n",
                    i, config->patch_names[config->mapping[i]]);
        }
    }

    fprintf(fstream, "\ndrumset 0\n\n");
    
    for (i = 128 + 35; i <= 128 + 81; ++i)
    {
        if (config->mapping[i] >= 0 && config->mapping[i] < MAX_INSTRUMENTS
         && config->patch_names[config->mapping[i]] != NULL)
        {
            fprintf(fstream, "%u %s\n",
                    i - 128, config->patch_names[config->mapping[i]]);
        }
    }

    fprintf(fstream, "\n");

    fclose(fstream);

    return true;
}

boolean GUS_WriteConfig(char *path)
{
    boolean result;
    char *dmxconf;
    char* gus_patches_path;
    gus_config_t config;

    int p = M_CheckParmWithArgs("-gus_patches", 1);
    if (p)
    {
        gus_patches_path = M_StringDuplicate(myargv[p + 1]);
    }
    else if (strcmp(gus_patch_path, "") == 0)
    {
        gus_patches_path = RD_M_FindInternalResource("gus_patches");
    }
    else
    {
        gus_patches_path = M_StringDuplicate(gus_patch_path);
    }

    printf(english_language ?
           "I_SDLMusic: Using GUS patch set from:\n    %s\n" :
           "I_SDLMusic: Используются патчи GUS из папки:\n    %s\n",
           gus_patches_path);

    dmxconf = ReadDMXConfig();
    ParseDMXConfig(dmxconf, &config);

    result = WriteTimidityConfig(path, &config, gus_patches_path);

    FreeDMXConfig(&config);
    Z_Free(dmxconf);
    free(gus_patches_path);

    return result;
}

