cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

include("${CMAKE_SOURCE_DIR}/assets.cmake")

file(COPY "${CMAKE_SOURCE_DIR}/assets/ENGINE_PUBLIC/" DESTINATION "${CMAKE_BINARY_DIR}/public/")

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GLFW_BUILD_EXAMPLES OFF)

project(pretty C CXX)

add_compile_definitions(PRETTY_ENGINE_PROJECT="${CMAKE_SOURCE_DIR}")

include_directories("include")

include_directories("code")

include_directories("PropertyEditor")

file(GLOB code_sources "${CMAKE_SOURCE_DIR}/code/code/*.c" "${CMAKE_SOURCE_DIR}/code/code/*.cpp")

add_library(pretty
	"source/render.cpp"
	"source/assetsManager.cpp"
	"source/command.cpp"
	"source/audio.cpp"
	${code_sources}
)

target_link_libraries(pretty PUBLIC
 	external
)
target_include_directories(pretty PRIVATE "${CMAKE_SOURCE_DIR}/external/openal-soft/include")

include("${CMAKE_SOURCE_DIR}/external.cmake")

include("${CMAKE_SOURCE_DIR}/custom.cmake")

# Debug

project(demo C CXX)

add_executable(demo
	"debug/main.cpp"
)
target_link_libraries(demo custom components renderFeatures)

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

target_link_libraries(game custom components renderFeatures ${extend})
target_compile_definitions(game PRIVATE ENGINE_EDITOR=0)

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

target_link_libraries(editor custom components renderFeatures ${extend})
target_compile_definitions(editor PRIVATE ENGINE_EDITOR=1)

include("${CMAKE_SOURCE_DIR}/test.cmake")
