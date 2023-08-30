cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

include("${CMAKE_SOURCE_DIR}/assets.cmake")

file(COPY "${CMAKE_SOURCE_DIR}/assets/ENGINE_PUBLIC/" DESTINATION "./public/")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(GLFW_BUILD_EXAMPLES OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")

project(import)

find_package(OpenCL REQUIRED)

# Rendering

message("-- GLFW")
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glfw")
message("-- GLAD")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glad")
message("-- GLM")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glm")

include_directories("${CMAKE_SOURCE_DIR}/external/glad/include")
include_directories("${CMAKE_SOURCE_DIR}/external/glm/include")
include_directories("${CMAKE_SOURCE_DIR}/external/stb")

# Header-Only

include_directories("${CMAKE_SOURCE_DIR}/external/dr_libs")

# Other

message("-- PortAudio")
set(PORTAUDIO_LIBRARY "${CMAKE_SOURCE_DIR}/external/portaudio") 
set(PORTAUDIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/portaudio/include")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	message("-- PulseAudio")
	set(PULSEAUDIO_LIBRARY "${CMAKE_SOURCE_DIR}/external/pulseaudio")
	set(PULSEAUDIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/pulseaudio/src")
endif()

message("-- Boxer")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/Boxer")
message("-- TomlPlusPlus")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/tomlplusplus")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/crossguid")

message("-- Bullet3")
set(BUILD_BULLET3_DEMOS OFF)
set(BUILD_BULLET2_DEMOS OFF)
set(BUILD_CPU_DEMOS OFF)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_BULLET3_TOOLS ON)
set(BUILD_BULLET3 ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/bullet3")

message("-- SQLITE3")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/sqlite/sqlite3")

include_directories("${CMAKE_SOURCE_DIR}/external/sqlite")
include_directories("${CMAKE_SOURCE_DIR}/external/Boxer/include")
include_directories("${CMAKE_SOURCE_DIR}/external/tomlplusplus/include")
include_directories("${CMAKE_SOURCE_DIR}/external/bullet3/src")
include_directories("${CMAKE_SOURCE_DIR}/external/crossguid")

# ImGUI

project(imgui)

include_directories("${CMAKE_SOURCE_DIR}/external/imgui")

file(GLOB sources "${CMAKE_SOURCE_DIR}/external/imgui/*.c" "${CMAKE_SOURCE_DIR}/external/imgui/*.cpp")

message("-- ImGUI")
add_library(imgui ${sources})
target_link_libraries(imgui PRIVATE glfw glad)

project(implot)

message("-- ImPlot")
add_library(implot
	"${CMAKE_SOURCE_DIR}/external/implot/implot.cpp"
	"${CMAKE_SOURCE_DIR}/external/implot/implot_items.cpp"
)
target_link_libraries(implot PRIVATE imgui)

include_directories("${CMAKE_SOURCE_DIR}/external/implot")

# ASSIMP

message("-- Assimp")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/assimp")
include_directories("${CMAKE_SOURCE_DIR}/external/assimp/include")

project(pretty C CXX)

include_directories("include")

set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)
set(MYSOFA_LIBRARY "${CMAKE_SOURCE_DIR}/external/libmysofa")
set(MYSOFA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/libmysofa/src")

set(ZLIB_LIBRARY "${CMAKE_SOURCE_DIR}/external/zlib")
set(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/zlib")

message("-- OpenAL")
set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)
set(LIBTYPE STATIC)
set(ALSOFT_INSTALL ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/openal-soft")

include_directories(${OpenCL_INCLUDE_DIR})

add_library(pretty
	"source/base.cpp"
	"source/render.cpp"
)

target_link_libraries(pretty PUBLIC
 	Boxer
 	glfw
 	glad
 	glm
 	LinearMath
 	Bullet3Common
 	BulletInverseDynamics
 	BulletCollision
 	BulletDynamics
 	BulletSoftBody
 	xg
 	assimp
 	imgui
 	OpenAL
 	${OpenCL_LIBRARY}
 	sqlite3
 	implot
)
target_include_directories(pretty PRIVATE "${CMAKE_SOURCE_DIR}/external/openal-soft/include")

# Debug

project(demo C CXX)

add_executable(demo
	"debug/main.cpp"
)
target_link_libraries(demo pretty)

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

target_link_libraries(game pretty ${extend})

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

target_link_libraries(editor pretty ${extend})
