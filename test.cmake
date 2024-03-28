
include(CTest)

file(GLOB TestDirs "${CMAKE_SOURCE_DIR}/test/**/")

project(test)

foreach(dir ${TestDirs})
	file(GLOB TestFiles "${dir}/*.cpp")

	get_filename_component(dirName ${dir} NAME)

	add_executable(${dirName} ${TestFiles})
	target_link_libraries(${dirName} PRIVATE pretty)

	add_test(NAME "${dirName}" COMMAND ${dirName})
endforeach()
