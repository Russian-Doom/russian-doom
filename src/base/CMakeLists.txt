add_subdirectory(gus_patches)

if(COMPILE_DOOM)
    add_copy_target(doom-wads SOURCE
        doom-beta.wad
        doom-common.wad
        doom-extra.wad
        doom-freedm.wad
        doom-freedoom.wad
        doom-jaguar.wad
        doom-mlevels.wad
        doom-nerve.wad
        doom-sigil.wad
        doom-skies-doom1.wad
        doom-skies-doom2.wad
        doom-skies-plutonia.wad
        doom-skies-tnt.wad
        doom-wide-doom1.wad
        doom-wide-doom2.wad
        doom-wide-plutonia.wad
        doom-wide-tnt.wad
    )

    install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:doom-wads,OUT_FILES>>
        DESTINATION $<IF:$<OR:$<BOOL:${WIN32}>,$<BOOL:${BUILD_PORTABLE}>>,base,${CMAKE_INSTALL_DATADIR}/${PACKAGE_TARNAME}>
        COMPONENT doom
    )
endif()

if(COMPILE_HERETIC)
    add_copy_target(heretic-wads SOURCE
        heretic-common.wad
    )

    install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:heretic-wads,OUT_FILES>>
        DESTINATION $<IF:$<OR:$<BOOL:${WIN32}>,$<BOOL:${BUILD_PORTABLE}>>,base,${CMAKE_INSTALL_DATADIR}/${PACKAGE_TARNAME}>
        COMPONENT heretic
    )
endif()

if(COMPILE_HEXEN)
    add_copy_target(hexen-wads SOURCE
        hexen-common.wad
#        hexen-psx.wad
    )

    install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:hexen-wads,OUT_FILES>>
        DESTINATION $<IF:$<OR:$<BOOL:${WIN32}>,$<BOOL:${BUILD_PORTABLE}>>,base,${CMAKE_INSTALL_DATADIR}/${PACKAGE_TARNAME}>
        COMPONENT hexen
    )
endif()

if(COMPILE_STRIFE)
    add_copy_target(strife-wads SOURCE
        strife-common.wad
        strife-russian.wad
    )

    install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:strife-wads,OUT_FILES>>
        DESTINATION $<IF:$<OR:$<BOOL:${WIN32}>,$<BOOL:${BUILD_PORTABLE}>>,base,${CMAKE_INSTALL_DATADIR}/${PACKAGE_TARNAME}>
        COMPONENT strife
    )
endif()
