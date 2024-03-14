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

# Header-Only
include_directories("${CMAKE_SOURCE_DIR}/external/stb")
include_directories("${CMAKE_SOURCE_DIR}/external/dr_libs")

# Other

message("-- PortAudio")
set(PORTAUDIO_LIBRARY "${CMAKE_SOURCE_DIR}/external/portaudio") 
set(PORTAUDIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/portaudio/include")

# if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
# 	message("-- PulseAudio")
# 	set(PULSEAUDIO_LIBRARY "${CMAKE_SOURCE_DIR}/external/pulseaudio")
# 	set(PULSEAUDIO_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/pulseaudio/src")
# 	include_directories("${CMAKE_SOURCE_DIR}/external/pulseaudio/src")
# endif()

message("-- Boxer")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/Boxer")
message("-- TomlPlusPlus")
set(TOML_EXCEPTIONS 0)
set(TOML_HEADER_ONLY 0)
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

set(BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)

set(ZLIB_LIBRARY "${CMAKE_SOURCE_DIR}/external/zlib")
set(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/zlib")

message("-- OpenAL")
# Set CMake options to control OpenAL Soft features
option(ALSOFT_UTILS "Build utility programs" OFF)
option(ALSOFT_TESTS "Build test programs" OFF)
option(ALSOFT_EXAMPLES "Build example programs" OFF)
option(ALSOFT_EMBED_HRTF_DATA "Embed HRTF data in the library" OFF)
option(ALSOFT_INSTALL_HRTF_DATA "Build definitions for HRTF data" OFF)

# Additional options related to 3D audio playback
option(ALSOFT_NO_CONFIG_UTIL "Disable alsoft-config utility" OFF)
option(ALSOFT_NO_ASM "Disable assembly optimizations" OFF)
option(ALSOFT_NO_LOOPBACK "Disable Loopback devices" OFF)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)
set(LIBTYPE STATIC)
set(ALSOFT_INSTALL ON)
set(ALSOFT_INSTALL_CONFIG ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/openal-soft")

project(external)

add_library(external SHARED "${CMAKE_SOURCE_DIR}/source/external.cpp")
target_link_libraries(external PUBLIC
	Boxer
 	glfw
 	glad
 	glm
 	xg
 	imgui
 	OpenAL
 	sqlite3
 	implot
)
