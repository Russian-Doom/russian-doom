add_subdirectory(textscreen)
add_subdirectory(opl)
add_subdirectory(pcsound)
# thirdparty/* directories are added directly from the root CmakeLists.txt, if no shared versions found in the system

# This is a dummy target to link copy targets to the build tree
add_custom_target(shared_libs)

add_subdirectory(DLL)
