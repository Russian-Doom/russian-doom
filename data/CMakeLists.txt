add_subdirectory(icons)

if(UNIX)
    add_subdirectory(man)

    configure_desktop_file("desktop.in" "${PACKAGE_ID}.desktop" "doom" "Doom")
    configure_desktop_file("desktop.in" "${PACKAGE_ID}.heretic.desktop" "heretic" "Heretic")
    configure_desktop_file("desktop.in" "${PACKAGE_ID}.hexen.desktop" "hexen" "Hexen")
    configure_file(metainfo.xml.in "${PACKAGE_ID}.metainfo.xml" @ONLY)

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_ID}.desktop"
        "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_ID}.heretic.desktop"
        "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_ID}.hexen.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_ID}.metainfo.xml"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo"
    )
endif()
