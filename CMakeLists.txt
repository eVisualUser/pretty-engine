cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

include("${CMAKE_SOURCE_DIR}/assets.cmake")
include("${CMAKE_SOURCE_DIR}/shaders.cmake")

file(COPY "${CMAKE_SOURCE_DIR}/assets/ENGINE_PUBLIC/" DESTINATION "./public/")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GLFW_BUILD_EXAMPLES OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-single-bit-bitfield-constant-conversion -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-microsoft-drectve-section -Wno-attributes -ffast-math -Wno-deprecated-non-prototype -Wc++20-attribute-extensions -Wno-inconsistent-missing-override")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-single-bit-bitfield-constant-conversion -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -Wno-microsoft-drectve-section -Wno-attributes -ffast-math -Wno-deprecated-non-prototype -Wno-inconsistent-missing-override")

project(import)

include("${CMAKE_SOURCE_DIR}/external.cmake")

project(pretty C CXX)

include_directories("include")

add_library(pretty
	"source/render.cpp"
)

target_link_libraries(pretty PUBLIC
 	external
)
target_include_directories(pretty PRIVATE "${CMAKE_SOURCE_DIR}/external/openal-soft/include")

include("${CMAKE_SOURCE_DIR}/custom.cmake")

# Debug

project(demo C CXX)

add_executable(demo
	"debug/main.cpp"
)
target_link_libraries(demo pretty custom components)

# Game

project(game C CXX)

include(game/custom.cmake)

file(READ "game/extend.txt" extend)
file(READ "game/source.txt" game_source)

if(NOT ${game_source} STREQUAL "")
string(REPLACE "\n" " " game_source ${game_source})
endif()

add_executable(game
	${game_source}
)

if(NOT ${extend} STREQUAL "")
string(REPLACE "\n" " " extend ${extend})
endif()

target_link_libraries(game pretty custom components ${extend})

# Editor

project(editor C CXX)

include(editor/custom.cmake)

file(READ "editor/extend.txt" extend)
file(READ "editor/source.txt" game_source)

if(NOT ${game_source} STREQUAL "")
string(REPLACE "\n" " " game_source ${game_source})
endif()

add_executable(editor
	${game_source}
)

if(NOT ${extend} STREQUAL "")
string(REPLACE "\n" " " extend ${extend})
endif()

target_link_libraries(editor pretty custom components ${extend})
target_compile_definitions(editor PRIVATE ENGINE_EDITOR=1)
