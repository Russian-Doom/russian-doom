if(WIN32)
    if(NOT ID_USE_SELECTED_DLL_SET)
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
    # Same install rule but for component based install
    foreach(MODULE ${COMPILE_MODULES})
        install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:win-libs,OUT_FILES>>
            DESTINATION .
            COMPONENT "${MODULE}"
            EXCLUDE_FROM_ALL
        )
    endforeach()
endif()
