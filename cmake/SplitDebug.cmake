if(APPLE)
    find_program(DSYMUTIL_executable NAMES dsymutil llvm-dsymutil)
    if(NOT DSYMUTIL_executable)
        message(FATAL_ERROR "Split debug info requires dsymutil on macOS")
    endif()
else()
    find_program(OBJCOPY_executable NAMES llvm-objcopy objcopy)
    if(NOT OBJCOPY_executable)
        message(FATAL_ERROR "Split debug info requires objcopy or llvm-objcopy")
    endif()
    if(NOT CMAKE_STRIP)
        message(FATAL_ERROR "Split debug info requires a strip tool")
    endif()
endif()

function(attach_split_debug target)
    if(APPLE)
        set(debug_path "$<TARGET_FILE:${target}>.dSYM")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${CMAKE_COMMAND}>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:-E>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:rm>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:-rf>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${debug_path}>"
            COMMAND
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${DSYMUTIL_executable}>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:$<TARGET_FILE:${target}>>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:-o>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${debug_path}>"
            COMMAND
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${CMAKE_STRIP}>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:-S>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:$<TARGET_FILE:${target}>>"
        )
        set_target_properties(${target} PROPERTIES
            ADDITIONAL_CLEAN_FILES "${debug_path}"
        )
    else()
        set(debug_path "$<TARGET_FILE:${target}>.debug")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${OBJCOPY_executable}>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:--only-keep-debug>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:$<TARGET_FILE:${target}>>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${debug_path}>"
            COMMAND
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${CMAKE_STRIP}>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:--strip-unneeded>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:$<TARGET_FILE:${target}>>"
            COMMAND
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:${OBJCOPY_executable}>"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:--add-gnu-debuglink=\"${debug_path}\">"
            "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:$<TARGET_FILE:${target}>>"
        )
        set_target_properties(${target} PROPERTIES
            ADDITIONAL_CLEAN_FILES "${debug_path}"
        )
    endif()
endfunction()
