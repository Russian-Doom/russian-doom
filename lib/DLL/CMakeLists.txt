if(WIN32)
    if(NOT RD_USE_SELECTED_DLL_SET)
        return()
    endif()
    if(NOT ARCH)
        message(WARNING "Can't use 'Selected DLL set' because target arhitecture is unknown")
        return()
    else()
        message("-- Using 'Selected DLL set' for ${ARCH}")
    endif()
    add_subdirectory("${ARCH}")

    add_dependencies(shared_libs win-libs)
    install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:win-libs,OUT_FILES>>
        DESTINATION .
    )
endif()
