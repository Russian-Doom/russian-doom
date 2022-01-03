//
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
//     Search for and locate an IWAD file, and initialize according
//     to the IWAD type.
//



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "deh_str.h"
#include "doomkeys.h"
#include "d_iwad.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"
#include "jn.h"

static const iwad_t iwads[] =
{
    { "doom2.wad",    doom2,     commercial, "Doom II" },
    { "doom2unity.wad",    doom2,     commercial, "Doom II" }, // [JN] Unity edition
    { "plutonia.wad", pack_plut, commercial, "Final Doom: Plutonia Experiment" },
    { "plutoniaunity.wad", pack_plut, commercial, "Final Doom: Plutonia Experiment" }, // [JN] Unity edition
    { "tnt.wad",      pack_tnt,  commercial, "Final Doom: TNT: Evilution" },
    { "tntunity.wad",      pack_tnt,  commercial, "Final Doom: TNT: Evilution" }, // [JN] Unity edition
    { "doom.wad",     doom,      retail,     "Doom" },
    { "doomunity.wad",     doom,      retail,     "Doom" }, // [JN] Unity edition
    { "doom1.wad",    doom,      shareware,  "Doom Shareware" },
//  { "chex.wad",     pack_chex, retail,     "Chex Quest" },
//  { "hacx.wad",     pack_hacx, commercial, "Hacx" },
    { "freedm.wad",   doom2,     commercial, "FreeDM" },
    { "freedoom2.wad", doom2,    commercial, "Freedoom: Phase 2" },
    { "freedoom1.wad", doom,     retail,     "Freedoom: Phase 1" },
    { "heretic.wad",  heretic,   retail,     "Heretic" },
    { "heretic1.wad", heretic,   shareware,  "Heretic Shareware" },
    { "hexen.wad",    hexen,     commercial, "Hexen" },
    { "strife0.wad",  strife,    commercial, "Strife" }, // haleyjd: STRIFE-FIXME
    { "strife1.wad",  strife,    commercial, "Strife" },
};

// Array of locations to search for IWAD files
//
// "128 IWAD search directories should be enough for anybody".

#define MAX_IWAD_DIRS 128

static boolean iwad_dirs_built = false;
static char *iwad_dirs[MAX_IWAD_DIRS];
static int num_iwad_dirs = 0;

static void AddIWADDir(char *dir)
{
    if (num_iwad_dirs < MAX_IWAD_DIRS)
    {
        iwad_dirs[num_iwad_dirs] = dir;
        ++num_iwad_dirs;
    }
}

// This is Windows-specific code that automatically finds the location
// of installed IWAD files.  The registry is inspected to find special
// keys installed by the Windows installers for various CD versions
// of Doom.  From these keys we can deduce where to find an IWAD.

#if defined(_WIN32) && !defined(_WIN32_WCE)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

typedef struct 
{
    HKEY root;
    char *path;
    char *value;
} registry_value_t;

#define UNINSTALLER_STRING "\\uninstl.exe /S "

// Keys installed by the various CD editions.  These are actually the 
// commands to invoke the uninstaller and look like this:
//
// C:\Program Files\Path\uninstl.exe /S C:\Program Files\Path
//
// With some munging we can find where Doom was installed.

// [AlexMax] From the persepctive of a 64-bit executable, 32-bit registry
// keys are located in a different spot.
#if _WIN64
#define SOFTWARE_KEY "Software\\Wow6432Node"
#else
#define SOFTWARE_KEY "Software"
#endif

static registry_value_t uninstall_values[] =
{
    // Ultimate Doom, CD version (Depths of Doom trilogy)

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Ultimate Doom for Windows 95",
        "UninstallString",
    },

    // Doom II, CD version (Depths of Doom trilogy)

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Doom II for Windows 95",
        "UninstallString",
    },

    // Final Doom

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Final Doom for Windows 95",
        "UninstallString",
    },

    // Shareware version

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Doom Shareware for Windows 95",
        "UninstallString",
    },
};

// Values installed by the GOG.com and Collector's Edition versions

static registry_value_t root_path_keys[] =
{
    // Doom Collector's Edition

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\Activision\\DOOM Collector's Edition\\v1.0",
        "INSTALLPATH",
    },

    // Doom II

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1435848814",
        "PATH",
    },

    // Final Doom

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1435848742",
        "PATH",
    },

    // Ultimate Doom

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1435827232",
        "PATH",
    },

    // Strife: Veteran Edition

    {
	HKEY_LOCAL_MACHINE,
	SOFTWARE_KEY "\\GOG.com\\Games\\1432899949",
	"PATH",
    },

    // Heretic

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1290366318",
        "PATH",
    },

    // Hexen

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1247951670",
        "PATH",
    },

    // Hexen: Deathkings of a Dark Citadel

    {
        HKEY_LOCAL_MACHINE,
        SOFTWARE_KEY "\\GOG.com\\Games\\1983497091",
        "PATH",
    },
};

// Subdirectories of the above install path, where IWADs are installed.

static char *root_path_subdirs[] =
{
    ".",
    "Doom2",
    "Final Doom",
    "Ultimate Doom",
    "Plutonia",
    "TNT",
};

// Location where Steam is installed

static registry_value_t steam_install_location =
{
    HKEY_LOCAL_MACHINE,
    SOFTWARE_KEY "\\Valve\\Steam",
    "InstallPath",
};

// Subdirs of the steam install directory where IWADs are found

static char *steam_install_subdirs[] =
{
    "steamapps\\common\\doom 2\\base",
    "steamapps\\common\\final doom\\base",
    "steamapps\\common\\ultimate doom\\base",
    "steamapps\\common\\heretic shadow of the serpent riders\\base",
    "steamapps\\common\\hexen\\base",
    "steamapps\\common\\hexen deathkings of the dark citadel\\base",

    // From Doom 3: BFG Edition:

    "steamapps\\common\\DOOM 3 BFG Edition\\base\\wads",

    // From Strife: Veteran Edition:

    "steamapps\\common\\Strife",
};

static char *GetRegistryString(registry_value_t *reg_val)
{
    HKEY key;
    DWORD len;
    DWORD valtype;
    char *result;

    // Open the key (directory where the value is stored)

    if (RegOpenKeyEx(reg_val->root, reg_val->path,
                     0, KEY_READ, &key) != ERROR_SUCCESS)
    {
        return NULL;
    }

    result = NULL;

    // Find the type and length of the string, and only accept strings.

    if (RegQueryValueEx(key, reg_val->value,
                        NULL, &valtype, NULL, &len) == ERROR_SUCCESS
     && valtype == REG_SZ)
    {
        // Allocate a buffer for the value and read the value

        result = malloc(len + 1);

        if (RegQueryValueEx(key, reg_val->value, NULL, &valtype,
                            (unsigned char *) result, &len) != ERROR_SUCCESS)
        {
            free(result);
            result = NULL;
        }
        else
        {
            // Ensure the value is null-terminated
            result[len] = '\0';
        }
    }

    // Close the key

    RegCloseKey(key);

    return result;
}

// Check for the uninstall strings from the CD versions

static void CheckUninstallStrings(void)
{
    unsigned int i;

    for (i=0; i<arrlen(uninstall_values); ++i)
    {
        char *val;
        char *path;
        char *unstr;

        val = GetRegistryString(&uninstall_values[i]);

        if (val == NULL)
        {
            continue;
        }

        unstr = strstr(val, UNINSTALLER_STRING);

        if (unstr == NULL)
        {
            free(val);
        }
        else
        {
            path = unstr + strlen(UNINSTALLER_STRING);

            AddIWADDir(path);
        }
    }
}

// Check for GOG.com and Doom: Collector's Edition

static void CheckInstallRootPaths(void)
{
    unsigned int i;

    for (i=0; i<arrlen(root_path_keys); ++i)
    {
        char *install_path;
        char *subpath;
        unsigned int j;

        install_path = GetRegistryString(&root_path_keys[i]);

        if (install_path == NULL)
        {
            continue;
        }

        for (j=0; j<arrlen(root_path_subdirs); ++j)
        {
            subpath = M_StringJoin(install_path, DIR_SEPARATOR_S,
                                   root_path_subdirs[j], NULL);
            AddIWADDir(subpath);
        }

        free(install_path);
    }
}


// Check for Doom downloaded via Steam

static void CheckSteamEdition(void)
{
    char *install_path;
    char *subpath;
    size_t i;

    install_path = GetRegistryString(&steam_install_location);

    if (install_path == NULL)
    {
        return;
    }

    for (i=0; i<arrlen(steam_install_subdirs); ++i)
    {
        subpath = M_StringJoin(install_path, DIR_SEPARATOR_S,
                               steam_install_subdirs[i], NULL);

        AddIWADDir(subpath);
    }

    free(install_path);
}

// Default install directories for DOS Doom

static void CheckDOSDefaults(void)
{
    // These are the default install directories used by the deice
    // installer program:

    AddIWADDir("\\doom2");              // Doom II
    AddIWADDir("\\plutonia");           // Final Doom
    AddIWADDir("\\tnt");
    AddIWADDir("\\doom_se");            // Ultimate Doom
    AddIWADDir("\\doom");               // Shareware / Registered Doom
    AddIWADDir("\\dooms");              // Shareware versions
    AddIWADDir("\\doomsw");

    AddIWADDir("\\heretic");            // Heretic
    AddIWADDir("\\hrtic_se");           // Heretic Shareware from Quake disc

    AddIWADDir("\\hexen");              // Hexen
    AddIWADDir("\\hexendk");            // Hexen Deathkings of the Dark Citadel

//  AddIWADDir("\\strife");             // Strife
}

#endif

// Returns true if the specified path is a path to a file
// of the specified name.

static boolean DirIsFile(char *path, char *filename)
{
    size_t path_len;
    size_t filename_len;

    path_len = strlen(path);
    filename_len = strlen(filename);

    return path_len >= filename_len + 1
        && path[path_len - filename_len - 1] == DIR_SEPARATOR
        && !strcasecmp(&path[path_len - filename_len], filename);
}

// Check if the specified directory contains the specified IWAD
// file, returning the full path to the IWAD if found, or NULL
// if not found.

static char *CheckDirectoryHasIWAD(char *dir, char *iwadname)
{
    char *filename; 
    char *probe;

    // As a special case, the "directory" may refer directly to an
    // IWAD file if the path comes from DOOMWADDIR or DOOMWADPATH.

    probe = M_FileCaseExists(dir);
    if (DirIsFile(dir, iwadname) && probe != NULL)
    {
        return probe;
    }

    // Construct the full path to the IWAD if it is located in
    // this directory, and check if it exists.

    if (!strcmp(dir, "."))
    {
        filename = M_StringDuplicate(iwadname);
    }
    else
    {
        filename = M_StringJoin(dir, DIR_SEPARATOR_S, iwadname, NULL);
    }

    probe = M_FileCaseExists(filename);
    if (probe != NULL)
    {
        return probe;
    }

    free(filename);

    return NULL;
}

// Search a directory to try to find an IWAD
// Returns the location of the IWAD if found, otherwise NULL.

static char *SearchDirectoryForIWAD(char *dir, int mask, GameMission_t *mission)
{
    char *filename;
    size_t i;

    for (i=0; i<arrlen(iwads); ++i) 
    {
        if (((1 << iwads[i].mission) & mask) == 0)
        {
            continue;
        }

        filename = CheckDirectoryHasIWAD(dir, DEH_String(iwads[i].name));

        if (filename != NULL)
        {
            *mission = iwads[i].mission;

            return filename;
        }
    }

    return NULL;
}

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.

static GameMission_t IdentifyIWADByName(char *name, int mask)
{
    size_t i;
    GameMission_t mission;
    char *p;

    p = strrchr(name, DIR_SEPARATOR);

    if (p != NULL)
    {
        name = p + 1;
    }

    mission = none;

    for (i=0; i<arrlen(iwads); ++i)
    {
        // Check if the filename is this IWAD name.

        // Only use supported missions:

        if (((1 << iwads[i].mission) & mask) == 0)
            continue;

        // Check if it ends in this IWAD name.

        if (!strcasecmp(name, iwads[i].name))
        {
            mission = iwads[i].mission;
            break;
        }
    }

    return mission;
}

// Add IWAD directories parsed from splitting a path string containing
// paths separated by PATH_SEPARATOR. 'suffix' is a string to concatenate
// to the end of the paths before adding them.
static void AddIWADPath(const char *path, char *suffix)
{
    const char *left;
    char *p, *path_copy;

    path_copy = M_StringDuplicate(path);

    // Split into individual dirs within the list.
    left = path_copy;

    for (;;)
    {
        p = strchr(left, PATH_SEPARATOR);
        if (p != NULL)
        {
            // Break at the separator and use the left hand side
            // as another iwad dir
            *p = '\0';

            AddIWADDir(M_StringJoin(left, suffix, NULL));
            left = p + 1;
        }
        else
        {
            break;
        }
    }

    AddIWADDir(M_StringJoin(left, suffix, NULL));

    free(path_copy);
}

#ifndef _WIN32
// Add standard directories where IWADs are located on Unix systems.
// To respect the freedesktop.org specification we support overriding
// using standard environment variables. See the XDG Base Directory
// Specification:
// <http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
static void AddXdgDirs(void)
{
    const char *env;
    char *tmp_env;

    // Quote:
    // > $XDG_DATA_HOME defines the base directory relative to which
    // > user specific data files should be stored. If $XDG_DATA_HOME
    // > is either not set or empty, a default equal to
    // > $HOME/.local/share should be used.
    env = getenv("XDG_DATA_HOME");
    tmp_env = NULL;

    if (env == NULL)
    {
        char *homedir = getenv("HOME");
        if (homedir == NULL)
        {
            homedir = "/";
        }

        tmp_env = M_StringJoin(homedir, "/.local/share", NULL);
        env = tmp_env;
    }

    // We support $XDG_DATA_HOME/games/doom (which will usually be
    // ~/.local/share/games/doom) as a user-writeable extension to
    // the usual /usr/share/games/doom location.
    AddIWADDir(M_StringJoin(env, "/games/doom", NULL));
    free(tmp_env);

    // Quote:
    // > $XDG_DATA_DIRS defines the preference-ordered set of base
    // > directories to search for data files in addition to the
    // > $XDG_DATA_HOME base directory. The directories in $XDG_DATA_DIRS
    // > should be seperated with a colon ':'.
    // >
    // > If $XDG_DATA_DIRS is either not set or empty, a value equal to
    // > /usr/local/share/:/usr/share/ should be used.
    env = getenv("XDG_DATA_DIRS");
    if (env == NULL)
    {
        // (Trailing / omitted from paths, as it is added below)
        env = "/usr/local/share:/usr/share";
    }

    // The "standard" location for IWADs on Unix that is supported by most
    // source ports is /usr/share/games/doom - we support this through the
    // XDG_DATA_DIRS mechanism, through which it can be overridden.
    AddIWADPath(env, "/games/doom");
}

#ifndef __MACOSX__
// Steam on Linux allows installing some select Windows games,
// including the classic Doom series (running DOSBox via Wine).  We
// could parse *.vdf files to more accurately detect installation
// locations, but the defaults are likely to be good enough for just
// about everyone.
static void AddSteamDirs(void)
{
    const char *homedir;
    char *steampath;

    homedir = getenv("HOME");
    if (homedir == NULL)
    {
        homedir = "/";
    }
    steampath = M_StringJoin(homedir, "/.steam/root/steamapps/common", NULL);

    AddIWADPath(steampath, "/Doom 2/base");
    AddIWADPath(steampath, "/Master Levels of Doom/doom2");
    AddIWADPath(steampath, "/Ultimate Doom/base");
    AddIWADPath(steampath, "/Final Doom/base");
    AddIWADPath(steampath, "/DOOM 3 BFG Edition/base/wads");
    AddIWADPath(steampath, "/Heretic Shadow of the Serpent Riders/base");
    AddIWADPath(steampath, "/Hexen/base");
    AddIWADPath(steampath, "/Hexen Deathkings of the Dark Citadel/base");
    AddIWADPath(steampath, "/Strife");
    free(steampath);
}
#endif // __MACOSX__
#endif

//
// Build a list of IWAD files
//

static void BuildIWADDirList(void)
{
    char *env;

    if (iwad_dirs_built)
    {
        return;
    }

    // Look in the current directory.  Doom always does this.
    AddIWADDir(".");

    // Next check the directory where the executable is located. This might
    // be different from the current directory.
    AddIWADDir(exedir);

    // Add DOOMWADDIR if it is in the environment
    env = getenv("DOOMWADDIR");
    if (env != NULL)
    {
        AddIWADDir(env);
    }

    // Add dirs from DOOMWADPATH:
    env = getenv("DOOMWADPATH");
    if (env != NULL)
    {
        AddIWADPath(env, "");
    }

#ifdef _WIN32

    // Search the registry and find where IWADs have been installed.

    CheckUninstallStrings();
    CheckInstallRootPaths();
    CheckSteamEdition();
    CheckDOSDefaults();

#else
    AddXdgDirs();
#ifndef __MACOSX__
    AddSteamDirs();
#endif
#endif

    // Don't run this function again.

    iwad_dirs_built = true;
}

//
// Searches WAD search paths for an WAD with a specific filename.
// 

char *D_FindWADByName(char *name)
{
    char *path;
    char *probe;
    int i;
    
    // Absolute path?

    probe = M_FileCaseExists(name);
    if (probe != NULL)
    {
        // [JN] Normalize slashes.
        // Allows to properly load WADs with slashes (/), 
        // instead of backslashes (\) in given path.
        M_NormalizeSlashes(probe);

        return probe;
    }

    BuildIWADDirList();

    // Search through all IWAD paths for a file with the given name.

    for (i=0; i<num_iwad_dirs; ++i)
    {
        // As a special case, if this is in DOOMWADDIR or DOOMWADPATH,
        // the "directory" may actually refer directly to an IWAD
        // file.

        probe = M_FileCaseExists(iwad_dirs[i]);
        if (DirIsFile(iwad_dirs[i], name) && probe != NULL)
        {
            return probe;
        }

        // Construct a string for the full path

        path = M_StringJoin(iwad_dirs[i], DIR_SEPARATOR_S, name, NULL);

        probe = M_FileCaseExists(path);
        if (probe != NULL)
        {
            return probe;
        }

        free(path);
    }

    // File not found

    return NULL;
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or passes through the filename
// if not found.
//

char *D_TryFindWADByName(char *filename)
{
    char *result;

    result = D_FindWADByName(filename);

    if (result != NULL)
    {
        return result;
    }
    else
    {
        return filename;
    }
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//

char *D_FindIWAD(int mask, GameMission_t *mission)
{
    char *result;
    char *iwadfile;
    int iwadparm;
    int betaparm;   // [JN] Press Beta
    int jaguarparm; // [JN] Atari Jaguar
    int i;

    // Check for the -iwad parameter

    //!
    // Specify an IWAD file to use.
    //
    // @arg <file>
    //

    iwadparm = M_CheckParmWithArgs("-iwad", 1);
    betaparm = M_CheckParm("-beta");
    jaguarparm = M_CheckParm("-jaguar");

    if (iwadparm)
    {
        // Search through IWAD dirs for an IWAD with the given name.

        iwadfile = myargv[iwadparm + 1];

        result = D_FindWADByName(iwadfile);

        if (result == NULL)
        {
            I_Error(english_language ?
                    "IWAD file '%s' not found!" :
                    "IWAD-файл \"%s\" не найден!",
                    iwadfile);
        }
        
        *mission = IdentifyIWADByName(result, mask);
    }
    else
    {
        // Search through the list and look for an IWAD

        result = NULL;

        BuildIWADDirList();
    
        for (i=0; result == NULL && i<num_iwad_dirs; ++i)
        {
            result = SearchDirectoryForIWAD(iwad_dirs[i], mask, mission);
        }
    }

    // [JN] Load Press Beta IWAD if -beta command line parameter is given.
    // All additional -iwad calls will be rejected.
    if (betaparm)
    {
        result = RD_M_FindInternalResource("doom-beta.wad");
        *mission = IdentifyIWADByName(result, mask);
    }

    // [JN] Load Atari Jaguar IWAD if -jaguar command line parameter is given.
    // All additional -iwad calls will be rejected.
    if (jaguarparm)
    {
        result = RD_M_FindInternalResource("doom-jaguar.wad");
        *mission = IdentifyIWADByName(result, mask);
    }
    
    return result;
}

// Find all IWADs in the IWAD search path matching the given mask.

const iwad_t **D_FindAllIWADs(int mask)
{
    const iwad_t **result;
    int result_len;
    char *filename;
    int i;

    result = malloc(sizeof(iwad_t *) * (arrlen(iwads) + 1));
    result_len = 0;

    // Try to find all IWADs

    for (i=0; i<arrlen(iwads); ++i)
    {
        if (((1 << iwads[i].mission) & mask) == 0)
        {
            continue;
        }

        filename = D_FindWADByName(iwads[i].name);

        if (filename != NULL)
        {
            result[result_len] = &iwads[i];
            ++result_len;
        }
    }

    // End of list

    result[result_len] = NULL;

    return result;
}

char *D_SuggestIWADName(GameMission_t mission, GameMode_t mode)
{
    int i;

    for (i = 0; i < arrlen(iwads); ++i)
    {
        if (iwads[i].mission == mission && iwads[i].mode == mode)
        {
            return iwads[i].name;
        }
    }

    return "unknown.wad";
}

char *D_SuggestGameName(GameMission_t mission, GameMode_t mode)
{
    int i;

    for (i = 0; i < arrlen(iwads); ++i)
    {
        if (iwads[i].mission == mission
         && (mode == indetermined || iwads[i].mode == mode))
        {
            return iwads[i].description;
        }
    }

    return "Unknown game?";
}

