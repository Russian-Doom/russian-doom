# VersionFromGit.cmake
#
# Public domain.

# configure_empty_git_info()
function(configure_empty_git_info)
    set(Hash "<unknown>")
    set(Timestamp "<unknown>")
    set(Version_suffix "")
    set(Display_version_suffix "")
    configure_file("cmake/git_info.h.in" "git_info.h" @ONLY)
    configure_file("cmake/CPackConfig.cmake.in" "CPackProjectConfig.cmake" @ONLY)
endfunction()

# Git info options
find_package(Git)
cmake_dependent_option(RD_GIT_NO_HASH "For Devs only! Don't run 'revision_check' at all" OFF "GIT_FOUND" ON)

# Update git_info.h
if(RD_GIT_NO_HASH)
    # Use a function to avoid accidentally messing the real variables
    configure_empty_git_info()
else()
    add_custom_target(revision_check
        COMMAND "${CMAKE_COMMAND}" -P "\"${PROJECT_SOURCE_DIR}/cmake/scripts/UpdateRevision.cmake\"" "${GIT_EXECUTABLE}" "\"git_info.h\"" "\"CPackProjectConfig.cmake\""
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    )
endif()
