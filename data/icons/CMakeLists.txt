if(UNIX)
    install(FILES "doom.png"
        RENAME "${PACKAGE_ID}.doom.png"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps"
    )
    install(FILES "heretic.png"
        RENAME "${PACKAGE_ID}.heretic.png"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps"
    )
    install(FILES "hexen.png"
        RENAME "${PACKAGE_ID}.hexen.png"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps"
    )
endif()
