if(WIN32)
    if("${ARCH}" MATCHES "x64")
        add_subdirectory(x64)
    elseif("${ARCH}" MATCHES "x86")
        add_subdirectory(x86)
    else()
        return() # Unknown arch
    endif()

    add_dependencies(shared_libs win-libs)
    install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:win-libs,OUT_FILES>>
        DESTINATION .
    )
    # Same install rule but for component based install
    foreach(MODULE ${COMPILE_MODULES})
        install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:win-libs,OUT_FILES>>
            DESTINATION .
            COMPONENT "${MODULE}"
            EXCLUDE_FROM_ALL
        )
    endforeach()
endif()
