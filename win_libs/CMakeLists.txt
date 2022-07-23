if(WIN32)
    if("${ARCH}" MATCHES "x64")
        add_subdirectory(x64)
    elseif("${ARCH}" MATCHES "x86")
        add_subdirectory(x86)
    else()
        return() # Unknown arch
    endif()

    install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:win-libs,OUT_FILES>>
        DESTINATION .
    )
endif()
