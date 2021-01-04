cmake_minimum_required(VERSION 3.5.1)

project(script)

option(BUILD_DEBUG "Build a debug release" "ON")

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

#add the framework dependencies
add_library(lm SHARED IMPORTED)
add_library(tools SHARED IMPORTED)

#Virtual library so we don't have to build again and again for all tests...
add_library(applib STATIC ${SOURCE})
set_target_properties(applib PROPERTIES OUTPUT_NAME applib)
add_executable(script src/tests/main.cpp)

set_target_properties(lm PROPERTIES IMPORTED_LOCATION /usr/local/lib/liblm.so)
set_target_properties(tools PROPERTIES IMPORTED_LOCATION /usr/local/lib/libtools.so)

target_link_libraries(script applib dfw lm tools stdc++fs)
