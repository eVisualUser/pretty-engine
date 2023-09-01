file(GLOB customFiles "${CMAKE_SOURCE_DIR}/custom/*.hpp")

file(READ "${CMAKE_SOURCE_DIR}/custom/template.hxx" template)

set(customOut ${template})
foreach(file ${customFiles})

	get_filename_component(id ${file} NAME_WE)

	string(TOUPPER ${id} id)

	set(customOut "${customOut}#define ${id} \"${id}\"")

endforeach()
file(WRITE "${CMAKE_SOURCE_DIR}/custom/custom.hpp" ${customOut})

project(custom)

add_library(custom
	SHARED
	${customFiles}
)
target_link_libraries(custom PRIVATE pretty)
