file(STRINGS "miniz.h" MINIZ_VERSION_MAJOR_LINE REGEX "^#define[ \t]+MZ_VER_MAJOR[ \t]+[0-9]+$")
string(REGEX REPLACE "^#define[ \t]+MZ_VER_MAJOR[ \t]+([0-9]+)$" "\\1" MINIZ_VERSION_MAJOR "${MINIZ_VERSION_MAJOR_LINE}")
math(EXPR MINIZ_VERSION_MAJOR "${MINIZ_VERSION_MAJOR} - 8" OUTPUT_FORMAT DECIMAL)

add_library(miniz STATIC EXCLUDE_FROM_ALL
    miniz.c     miniz.h
)
target_include_directories(miniz INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(miniz PRIVATE $<$<C_COMPILER_ID:GNU>:_GNU_SOURCE>)
set_target_properties(miniz PROPERTIES
    INTERFACE_miniz_MAJOR_VERSION "${MINIZ_VERSION_MAJOR}"
    COMPATIBLE_INTERFACE_STRING "miniz_MAJOR_VERSION"
    C_STANDARD 90
    C_STANDARD_REQUIRED ON
)
add_library(miniz::miniz ALIAS miniz)

unset(MINIZ_VERSION_MAJOR_LINE)
unset(MINIZ_VERSION_MAJOR)
