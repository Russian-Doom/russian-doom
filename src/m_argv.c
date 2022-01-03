//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
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



#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_misc.h"
#include "m_argv.h"  // haleyjd 20110212: warning fix
#include "jn.h"

int		myargc;
char**		myargv;




//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
//

int M_CheckParmWithArgs(char *check, int num_args)
{
    int i;

    for (i = 1; i < myargc - num_args; i++)
    {
	if (!strcasecmp(check, myargv[i]))
	    return i;
    }

    return 0;
}

//
// M_ParmExists
//
// Returns true if the given parameter exists in the program's command
// line arguments, false if not.
//

boolean M_ParmExists(char *check)
{
    return M_CheckParm(check) != 0;
}

int M_CheckParm(char *check)
{
    return M_CheckParmWithArgs(check, 0);
}

#define MAXARGVS        100

static void LoadResponseFile(int argv_index, const char *filename)
{
    FILE *handle;
    int size;
    char *infile;
    char *file;
    char **newargv;
    int newargc;
    int i, k;

    // Read the response file into memory
    handle = fopen(filename, "rb");

    if (handle == NULL)
    {
        printf (english_language ?
                "\nNo such response file!" :
                "\nОтветный файл не обнаружен!");
        exit(1);
    }

    printf(english_language ?
           "Found response file %s!\n" :
           "Ответный файл %s обнаружен!\n",
           filename);

    size = M_FileLength(handle);

    // Read in the entire file
    // Allocate one byte extra - this is in case there is an argument
    // at the end of the response file, in which case a '\0' will be
    // needed.

    file = malloc(size + 1);

    i = 0;

    while (i < size)
    {
        k = fread(file + i, 1, size - i, handle);

        if (k < 0)
        {
            I_Error(english_language ?
                    "Failed to read full contents of '%s'" :
                    "Невозможно прочитать содержимое \"%s\"",
                    filename);
        }

        i += k;
    }

    fclose(handle);

    // Create new arguments list array

    newargv = malloc(sizeof(char *) * MAXARGVS);
    newargc = 0;
    memset(newargv, 0, sizeof(char *) * MAXARGVS);

    // Copy all the arguments in the list up to the response file

    for (i=0; i<argv_index; ++i)
    {
        newargv[i] = myargv[i];
        myargv[i] = NULL;
        ++newargc;
    }

    infile = file;
    k = 0;

    while(k < size)
    {
        // Skip past space characters to the next argument

        while(k < size && isspace(infile[k]))
        {
            ++k;
        }

        if (k >= size)
        {
            break;
        }

        // If the next argument is enclosed in quote marks, treat
        // the contents as a single argument.  This allows long filenames
        // to be specified.

        if (infile[k] == '\"')
        {
            char *argstart;
            // Skip the first character(")
            ++k;

            argstart = &infile[k];

            // Read all characters between quotes

            while (k < size && infile[k] != '\"' && infile[k] != '\n')
            {
                ++k;
            }

            if (k >= size || infile[k] == '\n')
            {
                I_Error(english_language ?
                        "Quotes are not closed in the response file \"%s\"" :
                        "Кавычки не закрыты в ответном файле \"%s\"",
                        filename);
            }

            // Cut off the string at the closing quote

            infile[k] = '\0';
            ++k;
            newargv[newargc++] = M_StringDuplicate(argstart);
        }
        else
        {
            char *argstart;
            // Read in the next argument until a space is reached

            argstart = &infile[k];

            while(k < size && !isspace(infile[k]))
            {
                ++k;
            }

            // Cut off the end of the argument at the first space

            infile[k] = '\0';

            ++k;
            newargv[newargc++] = M_StringDuplicate(argstart);
        }
    }

    // Add arguments following the response file argument

    for (i=argv_index + 1; i<myargc; ++i)
    {
        newargv[newargc] = myargv[i];
        ++newargc;
    }

    // Free any old strings in myargv which were not moved to newargv
    for (i = 0; i < myargc; ++i)
    {
        if (myargv[i] != NULL)
        {
            free(myargv[i]);
            myargv[i] = NULL;
        }
    }

    free(myargv);
    myargv = newargv;
    myargc = newargc;

#if 0
    // Disabled - Vanilla Doom does not do this.
    // Display arguments

    printf("%d command-line args:\n", myargc);

    for (k=1; k<myargc; k++)
    {
        printf("'%s'\n", myargv[k]);
    }
#endif
}

//
// Find a Response File
//

void M_FindResponseFile(void)
{
    int i;

    for (i = 1; i < myargc; i++)
    {
        if (myargv[i][0] == '@')
        {
            LoadResponseFile(i, myargv[i] + 1);
        }
    }

    for (;;)
    {
        //!
        // @arg filename
        //
        // Load extra command line arguments from the given response file.
        // Arguments read from the file will be inserted into the command
        // line replacing this argument. A response file can also be loaded
        // using the abbreviated syntax '@filename.rsp'.
        //
        i = M_CheckParmWithArgs("-response", 1);
        if (i <= 0)
        {
            break;
        }
        // Replace the -response argument so that the next time through
        // the loop we'll ignore it. Since some parameters stop reading when
        // an argument beginning with a '-' is encountered, we keep something
        // that starts with a '-'.
        free(myargv[i]);
        myargv[i] = M_StringDuplicate("-_");
        LoadResponseFile(i + 1, myargv[i + 1]);
    }
}

// Return the name of the executable used to start the program:

char *M_GetExecutableName(void)
{
    char *sep;

    sep = strrchr(myargv[0], DIR_SEPARATOR);

    if (sep == NULL)
    {
        return myargv[0];
    }
    else
    {
        return sep + 1;
    }
}

char* exedir = NULL;
#ifdef __APPLE__
char* packageResourcesDir = NULL;
#endif
