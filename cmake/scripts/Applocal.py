#!/usr/bin/env python3

# The MIT License (MIT)
#
# Copyright (c) 2015 Martin Preisler
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


import subprocess
import os.path
import argparse
import shutil
import re

# The mingw path matches where Fedora 21 installs mingw32; this is the default
# fallback if no other search path is specified in $MINGW_BUNDLEDLLS_SEARCH_PATH
DEFAULT_PATH_PREFIXES = [
    "", "/usr/bin", "/usr/i686-w64-mingw32/sys-root/mingw/bin", "/mingw64/bin",
    "/usr/i686-w64-mingw32/sys-root/mingw/lib",
    "C:\\msys64\\mingw64\\bin"
]

env_path_prefixes = os.environ.get('APPLOCAL_BUNDLEDLLS_SEARCH_PATH', None)
if env_path_prefixes is not None:
    path_prefixes = [path for path in env_path_prefixes.split(';') if path]
else:
    path_prefixes = DEFAULT_PATH_PREFIXES

# This blacklist may need extending
blacklist = [
    "advapi32.dll", "kernel32.dll", "msvcrt.dll", "ole32.dll", "user32.dll",
    "ws2_32.dll", "comdlg32.dll", "gdi32.dll", "imm32.dll", "oleaut32.dll",
    "shell32.dll", "winmm.dll", "winspool.drv", "wldap32.dll",
    "ntdll.dll", "d3d9.dll", "mpr.dll", "crypt32.dll", "dnsapi.dll",
    "shlwapi.dll", "version.dll", "iphlpapi.dll", "msimg32.dll", "setupapi.dll",
    "opengl32.dll", "dwmapi.dll", "uxtheme.dll", "secur32.dll", "gdiplus.dll",
    "usp10.dll", "comctl32.dll", "wsock32.dll", "netapi32.dll", "userenv.dll",
    "avicap32.dll", "avrt.dll", "psapi.dll", "mswsock.dll", "glu32.dll",
    "bcrypt.dll", "rpcrt4.dll", "hid.dll",
    # MSVC crt
    "ucrtbase.dll", "ucrtbased.dll", "vcruntime140.dll", "vcruntime140d.dll",
    # directx 3d 11 
    "d3d11.dll", "dxgi.dll", "dwrite.dll"
]

dll_pattern_dumpbin = re.compile("\\s+([a-zA-Z0-9-_]*\\.[Dd][Ll]{2})")
dll_pattern = re.compile("\\s+DLL Name: ([a-zA-Z0-9-_]*\\.[Dd][Ll]{2})")


def find_full_path(filename, path_prefixes):
    for path_prefix in path_prefixes:
        path = os.path.join(path_prefix, filename)
        path_low = os.path.join(path_prefix, filename.lower())
        if os.path.exists(path):
            return path
        if os.path.exists(path_low):
            return path_low

    else:
        raise RuntimeError(
            "Can't find " + filename + ". If it is an inbuilt Windows DLL, "
            "please add it to the blacklist variable in the script and send "
            "a pull request!"
        )


def gather_deps(path, path_prefixes, seen):
    ret = [path]
    dumpbin = False

    if shutil.which("dumpbin") is not None:
        output = subprocess.check_output(["dumpbin", "/DEPENDENTS", path]).decode(
            "utf-8", "replace").split("\n")
        dumpbin = True
    elif shutil.which("objdump") is not None or shutil.which("llvm-objdump") is not None:
        output = subprocess.check_output(["objdump", "-p", path]).decode(
            "utf-8", "replace").split("\n")
    else:
        print("Neither dumpbin, llvm-objdump nor objdump could be found. Can not take care of dll dependencies")
        return ret

    for line in output:
        if dumpbin:
            regexp = dll_pattern_dumpbin.search(line)
        else:
            regexp = dll_pattern.search(line)
        if regexp is None:
            continue
        dep = regexp.group(1)
        ldep = dep.lower()

        if ldep in blacklist:
            continue

        if ldep in seen:
            continue

        try:
            dep_path = find_full_path(dep, path_prefixes)
            seen.add(ldep)
            subdeps = gather_deps(dep_path, path_prefixes, seen)
            ret.extend(subdeps)
        except RuntimeError as e:
            seen.add(ldep)
            print(e)

    return ret


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "exe_file",
        help="EXE or DLL file that you need to bundle dependencies for"
    )
    parser.add_argument(
        "--copy",
        action="store_true",
        help="In addition to printing out the dependencies, also copy them next to the exe_file"
    )
    parser.add_argument(
        "--upx",
        action="store_true",
        help="Only valid if --copy is provided. Run UPX on all the DLLs and EXE."
    )
    args = parser.parse_args()

    if args.upx and not args.copy:
        raise RuntimeError("Can't run UPX if --copy hasn't been provided.")

    all_deps = set(gather_deps(args.exe_file, path_prefixes, set()))
    all_deps.remove(args.exe_file)

    if not args.copy:
        print("\n".join(all_deps))

    if args.copy:
        # print("Copying enabled, will now copy all dependencies next to the exe_file.\n")

        parent_dir = os.path.dirname(os.path.abspath(args.exe_file))

        for dep in all_deps:
            basename = os.path.basename(dep)
            target = os.path.join(parent_dir, basename)

            try:
                if os.path.exists(target):
                    if os.path.getmtime(target) < os.path.getmtime(dep):
                        # print("%s: Updating from %s" % (basename, dep))
                        shutil.copy2(dep, parent_dir)
                    # else:
                        # print("%s: Already present" % basename)
                else:
                    # print("%s: Copying %s" % (basename, dep))
                    shutil.copy2(dep, parent_dir)

            except shutil.SameFileError:
                print("%s: Was already in target directory, "
                      "skipping..." % basename)

            if args.upx:
                subprocess.call(["upx", target])


if __name__ == "__main__":
    main()
