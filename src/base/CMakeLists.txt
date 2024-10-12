add_subdirectory(gus_patches)

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
    doom-sigil2.wad
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
    DESTINATION $<IF:$<BOOL:${WIN32}>,base,${CMAKE_INSTALL_DATADIR}/${PACKAGE_TARNAME}>
)

add_copy_target(heretic-wads SOURCE
    heretic-common.wad
)

install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:heretic-wads,OUT_FILES>>
    DESTINATION $<IF:$<BOOL:${WIN32}>,base,${CMAKE_INSTALL_DATADIR}/${PACKAGE_TARNAME}>
)

add_copy_target(hexen-wads SOURCE
    hexen-common.wad
#   hexen-psx.wad
)

install(FILES $<GENEX_EVAL:$<TARGET_PROPERTY:hexen-wads,OUT_FILES>>
    DESTINATION $<IF:$<BOOL:${WIN32}>,base,${CMAKE_INSTALL_DATADIR}/${PACKAGE_TARNAME}>
)
