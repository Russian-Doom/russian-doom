configure_file("author.man.in" "author.man" @ONLY)

foreach(MODULE ${COMPILE_MODULES})
	generate_man(${MODULE} ru)

	add_dependencies(man-${MODULE}-en man-${MODULE}-ru)

	install(FILES $<TARGET_PROPERTY:man-${MODULE}-ru,OUT_FILES>
		DESTINATION "share/man/man6/ru"
		COMPONENT ${MODULE}
	)
endforeach()
