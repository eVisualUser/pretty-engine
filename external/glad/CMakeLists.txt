cmake_minimum_required(VERSION 3.20)

include_directories("${CMAKE_SOURCE_DIR}/external/glad/include/")

find_package(OpenGL REQUIRED)

project(glad)

add_library(glad
	"src/glad.c"
)
target_link_libraries(glad PUBLIC ${OPENGL_LIBRARIES})
