# add_copy_target(<target_name> SOURCE <file> ... [DESTINATION <dir>])
# Copy given files to given dir at build time
function(add_copy_target target_name)
    cmake_parse_arguments(PARSE_ARGV 1 "ARG"
        ""            # List of Options
        "DESTINATION" # List of Values
        "SOURCE"      # List of Lists
    )

    if(NOT ARG_DESTINATION)
        set(ARG_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    if(MSVC OR CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
        string(REPLACE "src" "src/$<CONFIG>" output_dir "${ARG_DESTINATION}")
    else()
        set(output_dir "${ARG_DESTINATION}")
    endif()

    foreach(source_file ${ARG_SOURCE})
        add_custom_command(OUTPUT "${output_dir}/${source_file}"
            MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}"
            "${output_dir}/${source_file}"
            COMMENT "Copying ${output_dir}/${source_file}"
        )
        list(APPEND destination_files "${output_dir}/${source_file}")
    endforeach()

    add_custom_target(${target_name} DEPENDS ${destination_files})
    set_target_properties(${target_name} PROPERTIES
        OUT_FILES "${destination_files}"
    )
endfunction()

# copy_files(SOURCE <file> ... [DESTINATION <dir>])
# Copy given files to given dir at config time
function(copy_files)
    cmake_parse_arguments(PARSE_ARGV 0 "ARG"
        ""            # List of Options
        "DESTINATION" # List of Values
        "SOURCE"      # List of Lists
    )
    if(NOT ARG_SOURCE)
        return()
    endif()
    if(NOT ARG_DESTINATION)
        set(ARG_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    if(MSVC OR CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
        foreach(_config ${CMAKE_CONFIGURATION_TYPES})
            string(REPLACE "src" "src/${_config}" output_dir_config "${ARG_DESTINATION}")
            list(APPEND output_dirs "${output_dir_config}")
        endforeach()
    else()
        set(output_dirs "${ARG_DESTINATION}")
    endif()

    foreach(output_dir ${output_dirs})
        file(COPY ${ARG_SOURCE} DESTINATION "${output_dir}")
    endforeach()
endfunction()

include(CheckCCompilerFlag)

# add_compile_options_checked(<option> ...)
# Don't support generator expressions
function(add_compile_options_checked)
    foreach(flag ${ARGV})
        # Turn flag into suitable internal cache variable.
        string(REGEX REPLACE "-(.*)" "CFLAG_\\1" flag_found ${flag})
        string(REPLACE "=" "_" flag_found "${flag_found}")

        check_c_compiler_flag(${flag} ${flag_found})
        if(${flag_found})
            list(APPEND flag_list ${flag})
        endif()
    endforeach()
    set(RD_compile_options ${RD_compile_options} ${flag_list} PARENT_SCOPE)
endfunction()

# configure_empty_git_info(<Template> <Output>)
function(configure_empty_git_info Template Output)
    set(Hash "<unknown>")
    set(Hash_suffix "")
    set(Timestamp "<unknown>")
    configure_file("${Template}" "${Output}" @ONLY)
endfunction()

# add_common_compile_definitions(<definition> ...)
function(add_common_compile_definitions)
    set(RD_compile_definitions ${RD_compile_definitions} ${ARGV} PARENT_SCOPE)
endfunction()

# target_common_settings(<target>)
function(target_common_settings Target)
    target_compile_options(${Target} PRIVATE ${RD_compile_options})
    target_compile_definitions(${Target} PRIVATE ${RD_compile_definitions})
    set_target_properties(${Target} PROPERTIES
        C_STANDARD "${RD_C_STANDARD}"
        C_STANDARD_REQUIRED ON
        INTERPROCEDURAL_OPTIMIZATION ${ENABLE_LTO}
        INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF
    )
endfunction()

# Helper for Find modules.
# get_flags_from_pkg_config(<STATIC|SHARED> <pc_prefix> <out_prefix>)
function(get_flags_from_pkg_config _library_type _pc_prefix _out_prefix)
    if(NOT ${_pc_prefix}_FOUND)
        set(${_out_prefix}_compile_options
            ""
            PARENT_SCOPE)
        set(${_out_prefix}_link_libraries
            ""
            PARENT_SCOPE)
        set(${_out_prefix}_link_options
            ""
            PARENT_SCOPE)
        set(${_out_prefix}_link_directories
            ""
            PARENT_SCOPE)
        return()
    endif()

    if("${_library_type}" STREQUAL "STATIC")
        set(_cflags ${_pc_prefix}_STATIC_CFLAGS_OTHER)
        set(_link_libraries ${_pc_prefix}_STATIC_LIBRARIES)
        set(_link_options ${_pc_prefix}_STATIC_LDFLAGS_OTHER)
        set(_library_dirs ${_pc_prefix}_STATIC_LIBRARY_DIRS)
    else()
        set(_cflags ${_pc_prefix}_CFLAGS_OTHER)
        set(_link_libraries ${_pc_prefix}_LIBRARIES)
        set(_link_options ${_pc_prefix}_LDFLAGS_OTHER)
        set(_library_dirs ${_pc_prefix}_LIBRARY_DIRS)
    endif()

    # The *_LIBRARIES lists always start with the library itself
    list(REMOVE_AT "${_link_libraries}" 0)

    # Work around CMake's flag deduplication when pc files use `-framework A` instead of `-Wl,-framework,A`
    string(REPLACE "-framework;" "-Wl,-framework," "_filtered_link_options" "${${_link_options}}")

    set(${_out_prefix}_compile_options
        "${${_cflags}}"
        PARENT_SCOPE)
    set(${_out_prefix}_link_libraries
        "${${_link_libraries}}"
        PARENT_SCOPE)
    set(${_out_prefix}_link_options
        "${_filtered_link_options}"
        PARENT_SCOPE)
    set(${_out_prefix}_link_directories
        "${${_library_dirs}}"
        PARENT_SCOPE)
endfunction()
