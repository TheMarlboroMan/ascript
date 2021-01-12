cmake_minimum_required(VERSION 3.5.1)

project(ascript)

option(BUILD_TESTS "Build test code" ON)
option(BUILD_DEBUG "Build a debug release" "OFF")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${BUILD_DEBUG})
	message("DEBUG BUILD")
	set(CMAKE_BUILD_TYPE Debug)
else()
	message("RELEASE BUILD")
	set(CMAKE_BUILD_TYPE Release)
endif()

#-Werror
add_compile_options( -Wall -Wextra -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-deprecated -ansi -pedantic -fno-rtti)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include/script")
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/src")

add_library(ascript_static STATIC ${SOURCE})
set_target_properties(ascript_static PROPERTIES OUTPUT_NAME ascript)

add_library(ascript_shared SHARED ${SOURCE})
set_target_properties(ascript_shared PROPERTIES OUTPUT_NAME ascript)

install(TARGETS ascript_static DESTINATION lib)
install(TARGETS ascript_shared DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

if(${BUILD_TESTS})

	add_library(lm SHARED IMPORTED)
	set_target_properties(lm PROPERTIES IMPORTED_LOCATION /usr/local/lib/liblm.so)

	add_library(tools SHARED IMPORTED)
	set_target_properties(tools PROPERTIES IMPORTED_LOCATION /usr/local/lib/libtools.so)

	add_executable(ascript src/tests/ascript.cpp)
	add_executable(interactive src/tests/interactive.cpp)
	add_executable(print_tokens src/tests/print_tokens.cpp)
	add_executable(print_code src/tests/print_code.cpp)

	target_link_libraries(ascript ascript_shared dfw lm tools stdc++fs)
	target_link_libraries(interactive ascript_shared dfw lm tools stdc++fs)
	target_link_libraries(print_tokens ascript_shared dfw lm tools stdc++fs)
	target_link_libraries(print_code ascript_shared dfw lm tools stdc++fs)
endif()


