set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_BUILD_TYPE release)

if(PORT STREQUAL "libsamplerate")
    list(APPEND VCPKG_CXX_FLAGS "/fp:fast")
    list(APPEND VCPKG_C_FLAGS "/fp:fast")
endif()
