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
    set(Timestamp "<unknown>")
    set(Version_suffix "")
    set(Display_version_suffix "")
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
        INTERPROCEDURAL_OPTIMIZATION ${RD_ENABLE_LTO}
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

if(WIN32 AND (Python3_Interpreter_FOUND OR CMAKE_VERSION VERSION_GREATER_EQUAL 3.16))
    # applocal_search_path(<out_var_prefix> TARGETS <targets> ...)
    function(applocal_search_path out_var_prefix)
        cmake_parse_arguments(PARSE_ARGV 1 "ARG"
            ""            # List of Options
            ""            # List of Values
            "TARGETS"     # List of Lists
        )
        get_filename_component(search_paths_release "${CMAKE_C_COMPILER}" DIRECTORY)
        set(search_paths_debug ${search_paths_release})
        foreach(target ${ARG_TARGETS})
            if(TARGET ${target})
                get_target_property(target_type ${target} TYPE)
                if(target_type STREQUAL "SHARED_LIBRARY")
                    get_target_property(target_configs ${target} IMPORTED_CONFIGURATIONS)
                    # Release and Debug
                    if(RELEASE IN_LIST target_configs AND DEBUG IN_LIST target_configs)
                        get_target_property(dll_dir ${target} IMPORTED_LOCATION_RELEASE)
                        get_filename_component(dll_dir "${dll_dir}" DIRECTORY)
                        if(NOT dll_dir IN_LIST search_paths_release)
                            list(PREPEND search_paths_release ${dll_dir})
                        endif()
                        get_target_property(dll_dir ${target} IMPORTED_LOCATION_DEBUG)
                        get_filename_component(dll_dir "${dll_dir}" DIRECTORY)
                        if(NOT dll_dir IN_LIST search_paths_debug)
                            list(PREPEND search_paths_debug ${dll_dir})
                        endif()
                    # Release Only
                    elseif(RELEASE IN_LIST target_configs)
                        get_target_property(dll_dir ${target} IMPORTED_LOCATION_RELEASE)
                        get_filename_component(dll_dir "${dll_dir}" DIRECTORY)
                        if(NOT dll_dir IN_LIST search_paths_release)
                            list(PREPEND search_paths_release ${dll_dir})
                        endif()
                        if(NOT dll_dir IN_LIST search_paths_debug)
                            list(PREPEND search_paths_debug ${dll_dir})
                        endif()
                    # Debug Only
                    elseif(DEBUG IN_LIST target_configs)
                        get_target_property(dll_dir ${target} IMPORTED_LOCATION_DEBUG)
                        get_filename_component(dll_dir "${dll_dir}" DIRECTORY)
                        if(NOT dll_dir IN_LIST search_paths_release)
                            list(PREPEND search_paths_release ${dll_dir})
                        endif()
                        if(NOT dll_dir IN_LIST search_paths_debug)
                            list(PREPEND search_paths_debug ${dll_dir})
                        endif()
                    # Generic
                    else()
                        get_target_property(dll_dir ${target} IMPORTED_LOCATION)
                        get_filename_component(dll_dir "${dll_dir}" DIRECTORY)
                        if(NOT dll_dir IN_LIST search_paths_release)
                            list(PREPEND search_paths_release ${dll_dir})
                        endif()
                        if(NOT dll_dir IN_LIST search_paths_debug)
                            list(PREPEND search_paths_debug ${dll_dir})
                        endif()
                    endif()
                endif()
            endif()
        endforeach()
        set(${out_var_prefix}_DEBUG ${search_paths_debug} PARENT_SCOPE)
        set(${out_var_prefix}_RELEASE ${search_paths_release} PARENT_SCOPE)
    endfunction()

    # applocal_dependencies(<target> <search_paths>)
    function(applocal_dependencies target_name search_paths)
        if(Python3_Interpreter_FOUND) # Applocal.py
            add_custom_command(TARGET "${target_name}" POST_BUILD
                DEPENDS "${PROJECT_SOURCE_DIR}/cmake/Applocal.py"
                COMMAND ${CMAKE_COMMAND} -E env "APPLOCAL_BUNDLEDLLS_SEARCH_PATH=${search_paths}"
                "$<TARGET_FILE:Python3::Interpreter>" "${PROJECT_SOURCE_DIR}/cmake/Applocal.py" --copy
                "$<TARGET_FILE:${target_name}>"
                COMMENT "Copying app dependencies for ${target_name}..."
            )
        else() # Applocal.cmake
            add_custom_command(TARGET "${target_name}" POST_BUILD
                DEPENDS "${PROJECT_SOURCE_DIR}/cmake/Applocal.cmake"
                COMMAND ${CMAKE_COMMAND} -P "${PROJECT_SOURCE_DIR}/cmake/Applocal.cmake"
                "$<TARGET_FILE:${target_name}>" ${search_paths}
                COMMENT "Copying app dependencies for ${target_name} (WARNING: DLLs won't be updated) ..."
            )
        endif()
    endfunction()

    # applocal_install_dependencies(<target> <search_paths> <component>)
    function(applocal_install_dependencies target_name search_paths component_name)
        if(Python3_Interpreter_FOUND) # Applocal.py
            install(CODE "message(STATUS \"Installing app dependencies for ${target_name}...\")
                execute_process(COMMAND ${CMAKE_COMMAND} -E env \"APPLOCAL_BUNDLEDLLS_SEARCH_PATH=${search_paths}\"
                    \"$<TARGET_FILE:Python3::Interpreter>\" \"${PROJECT_SOURCE_DIR}/cmake/Applocal.py\" --copy
                    \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${target_name}>\")"
                COMPONENT "${component_name}"
            )
        else() # Applocal.cmake
            install(CODE "message(STATUS \"Installing app dependencies for ${target_name} (WARNING: DLLs won't be updated)...\")
                execute_process(COMMAND ${CMAKE_COMMAND} -P \"${PROJECT_SOURCE_DIR}/cmake/Applocal.cmake\"
                    \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${target_name}>\" \"${search_paths}\")"
                COMPONENT "${component_name}"
            )
        endif()
    endfunction()
endif()

# configure_desktop_file(<template> <out> <MODULE> <MODULE_NAME>)
# Just set MODULE MODULE_NAME variables for configure_file
function(configure_desktop_file template out MODULE MODULE_NAME)
    configure_file("${template}" "${out}" @ONLY)
endfunction()
