file(GLOB_RECURSE SIIAM_SRC *.cpp *.c)

set(COMPONENT_SRCS ${SIIAM_SRC})
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()