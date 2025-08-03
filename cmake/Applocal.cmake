# Applocal.cmake
#
# Public domain.

if(Python3_Interpreter_FOUND)
    message(STATUS "Using Python 3 to get DLLs for applocal and install")
else()
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
        message(STATUS "Using Cmake 3.16 native method to get DLLs for applocal and install."
            "DLLs won't be updated once installed/copied!"
            "Install Python 3 for more efficient implementation"
        )
    else()
        message(FATAL_ERROR "Python 3 or Cmake 3.16 is required")
    endif()
endif()

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
            COMMAND ${CMAKE_COMMAND} -E env "APPLOCAL_BUNDLEDLLS_SEARCH_PATH=${search_paths}"
            "$<TARGET_FILE:Python3::Interpreter>" "${PROJECT_SOURCE_DIR}/cmake/scripts/Applocal.py" --copy
            "$<TARGET_FILE:${target_name}>"
            COMMENT "Copying app dependencies for ${target_name}..."
        )
    else() # Applocal.cmake
        add_custom_command(TARGET "${target_name}" POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P "${PROJECT_SOURCE_DIR}/cmake/scripts/Applocal.cmake"
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
                \"$<TARGET_FILE:Python3::Interpreter>\" \"${PROJECT_SOURCE_DIR}/cmake/scripts/Applocal.py\" --copy
                \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${target_name}>\")"
            COMPONENT "${component_name}"
        )
    else() # Applocal.cmake
        install(CODE "message(STATUS \"Installing app dependencies for ${target_name} (WARNING: DLLs won't be updated)...\")
            execute_process(COMMAND ${CMAKE_COMMAND} -P \"${PROJECT_SOURCE_DIR}/cmake/scripts/Applocal.cmake\"
                \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${target_name}>\" \"${search_paths}\")"
            COMPONENT "${component_name}"
        )
    endif()
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
