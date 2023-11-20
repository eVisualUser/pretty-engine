include(FetchContent)

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

message("-- SQLITE3")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/sqlite/sqlite3")

include_directories("${CMAKE_SOURCE_DIR}/external/sqlite")
include_directories("${CMAKE_SOURCE_DIR}/external/Boxer/include")
include_directories("${CMAKE_SOURCE_DIR}/external/tomlplusplus/include")
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

message("--LibCCD")
include_directories("${CMAKE_SOURCE_DIR}/external/libccd/src")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/libccd")

project(external)

add_library(external SHARED "${CMAKE_SOURCE_DIR}/source/external.cpp")
target_link_libraries(external PUBLIC
	Boxer
 	glfw
 	glad
 	glm
 	xg
 	assimp
 	imgui
 	OpenAL
 	sqlite3
 	implot
 	ccd
)
