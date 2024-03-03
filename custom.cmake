# Entities

set(CUSTOM_DIRECTORY "${CMAKE_SOURCE_DIR}/entities")

file(GLOB customFiles "${CUSTOM_DIRECTORY}/*.hpp")
file(GLOB customSources "${CUSTOM_DIRECTORY}/*.cpp")

file(READ "${CUSTOM_DIRECTORY}/template.hxx" template)

include_directories("${CUSTOM_DIRECTORY}")

set(customOut "#pragma once\n")

foreach(file ${customFiles})
		file(RELATIVE_PATH relative "${CUSTOM_DIRECTORY}" ${file})
		if(NOT(${relative} STREQUAL "custom.hpp"))
			set(customOut "${customOut}\n#include <${relative}>")
		endif()
endforeach()

set(customOut "${customOut}\n${template}")

set(customList "")

foreach(file ${customFiles})

	get_filename_component(id ${file} NAME_WE)
	if(NOT(${id} STREQUAL "custom"))
		set(customOut "${customOut}    if(name == \"${id}\") {\n        world->RegisterEntity(std::make_shared<Custom::${id}>())\;\n    } ")
		set(customList "${customList}${id}\;")
	endif()

endforeach()
set(customOut "${customOut}\n}")
file(WRITE "${CUSTOM_DIRECTORY}/custom.hpp" ${customOut})
file(WRITE "${CUSTOM_DIRECTORY}/list.csv" ${customList})

project(custom)

add_library(custom
	${customFiles}
	${customSources}
)
set_target_properties(custom PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
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
set(componentList "")

foreach(file ${customFiles})
	get_filename_component(id ${file} NAME_WE)
	if(NOT(${id} STREQUAL "components"))
		set(customOut "${customOut}    if(name == \"${id}\") {\n        return std::make_shared<Custom::${id}>()\;\n    } ")
		set(componentList "${componentList}${id}\;")
	endif()
endforeach()
set(customOut "${customOut}\n return nullptr\;")
set(customOut "${customOut}\n}")
file(WRITE "${CMAKE_SOURCE_DIR}/components/components.hpp" ${customOut})
file(WRITE "${CMAKE_SOURCE_DIR}/components/list.csv" ${componentList})

project(components)

add_library(components
	${customFiles}
	${customSources}
)
target_link_libraries(components PRIVATE pretty)

# Render Features

set(customFiles "")

file(GLOB customFiles "${CMAKE_SOURCE_DIR}/RenderFeatures/*.hpp")
file(GLOB customSources "${CMAKE_SOURCE_DIR}/RenderFeatures/*.cpp")

file(READ "${CMAKE_SOURCE_DIR}/RenderFeatures/template.hxx" template)

include_directories("${CMAKE_SOURCE_DIR}/RenderFeatures")

set(customOut "#pragma once\n")

foreach(file ${customFiles})
		file(RELATIVE_PATH relative "${CMAKE_SOURCE_DIR}/RenderFeatures" ${file})
		if(NOT(${relative} STREQUAL "RenderFeatures.hpp"))
			set(customOut "${customOut}\n#include <${relative}>")
		endif()
endforeach()

set(customOut "${customOut}\n${template}")
set(componentList "")

foreach(file ${customFiles})
	get_filename_component(id ${file} NAME_WE)
	if(NOT(${id} STREQUAL "RenderFeatures"))
		set(customOut "${customOut}    if(name == \"${id}\") {\n        return std::make_shared<Custom::${id}>()\;\n    } ")
		set(componentList "${componentList}${id}\;")
	endif()
endforeach()
set(customOut "${customOut}\n return nullptr\;")
set(customOut "${customOut}\n}")
file(WRITE "${CMAKE_SOURCE_DIR}/RenderFeatures/RenderFeatures.hpp" ${customOut})
file(WRITE "${CMAKE_SOURCE_DIR}/RenderFeatures/RenderFeaturesList.csv" ${componentList})
file(COPY "${CMAKE_SOURCE_DIR}/RenderFeatures/RenderFeaturesList.csv" DESTINATION "${CMAKE_SOURCE_DIR}/assets/ENGINE_PUBLIC")

project(renderFeatures)

add_library(renderFeatures
	${customFiles}
	${customSources}
)
target_link_libraries(renderFeatures PRIVATE pretty)

# External GO

execute_process(
	COMMAND go run ./
	OUTPUT_VARIABLE external_go
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tools/generateFiles"
)

execute_process(
	COMMAND go run ./
	OUTPUT_VARIABLE external_go
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tools/clearAssets"
)
