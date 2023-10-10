# Entities

file(GLOB customFiles "${CMAKE_SOURCE_DIR}/custom/*.hpp")
file(GLOB customSources "${CMAKE_SOURCE_DIR}/custom/*.cpp")

file(READ "${CMAKE_SOURCE_DIR}/custom/template.hxx" template)

include_directories("${CMAKE_SOURCE_DIR}/custom")

set(customOut "#pragma once\n")

foreach(file ${customFiles})
		file(RELATIVE_PATH relative "${CMAKE_SOURCE_DIR}/custom" ${file})
		if(NOT(${relative} STREQUAL "custom.hpp"))
			set(customOut "${customOut}\n#include <${relative}>")
		endif()
endforeach()

set(customOut "${customOut}\n${template}")

foreach(file ${customFiles})

	get_filename_component(id ${file} NAME_WE)
	if(NOT(${id} STREQUAL "custom"))
		set(customOut "${customOut}    if(name == \"${id}\") {\n        world->RegisterEntity(std::make_shared<Custom::${id}>())\;\n    } ")
	endif()

endforeach()
set(customOut "${customOut}\n}")
file(WRITE "${CMAKE_SOURCE_DIR}/custom/custom.hpp" ${customOut})

project(custom)

add_library(custom
	${customFiles}
	${customSources}
)
target_link_libraries(custom PRIVATE pretty)

# Components

set(customFiles "")

file(GLOB customFiles "${CMAKE_SOURCE_DIR}/components/*.hpp")
file(GLOB customSources "${CMAKE_SOURCE_DIR}/components/*.cpp")

file(READ "${CMAKE_SOURCE_DIR}/components/template.hxx" template)

include_directories("${CMAKE_SOURCE_DIR}/components")

set(customOut "#pragma once\n")

foreach(file ${customFiles})
		file(RELATIVE_PATH relative "${CMAKE_SOURCE_DIR}/components" ${file})
		if(NOT(${relative} STREQUAL "components.hpp"))
			set(customOut "${customOut}\n#include <${relative}>")
		endif()
endforeach()

set(customOut "${customOut}\n${template}")

foreach(file ${customFiles})

	get_filename_component(id ${file} NAME_WE)
	if(NOT(${id} STREQUAL "components"))
		set(customOut "${customOut}    if(name == \"${id}\") {\n        return std::make_shared<Custom::${id}>()\;\n    } ")
	endif()

endforeach()
set(customOut "${customOut}\n return nullptr\;")
set(customOut "${customOut}\n}")
file(WRITE "${CMAKE_SOURCE_DIR}/components/components.hpp" ${customOut})

project(components)

add_library(components
	${customFiles}
	${customSources}
)
target_link_libraries(components PRIVATE pretty)
