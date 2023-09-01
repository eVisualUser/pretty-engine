file(GLOB_RECURSE shaders "${CMAKE_SOURCE_DIR}/shaders/*")

set(shaders_buffer "#pragma once\n\n${file_prefix}\n")

foreach(shader ${shaders})

	get_filename_component(name ${shader} NAME_WE)

	string(TOUPPER ${name} name)

	file(READ ${shader} shader_content)

	string(REPLACE ";" "\;" shader_content "${shader_content}")

	string(APPEND shaders_buffer "\nstatic const char* SHADER_${name} = R\"(${shader_content})\"\;\n")

endforeach()

file(WRITE "include/PrettyEngine/shaders.hpp" ${shaders_buffer})
