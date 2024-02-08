cmake_minimum_required(VERSION 3.5.1)
project(ascript)

option(BUILD_STATIC "generate static release" OFF)
option(BUILD_SHARED "generate shared release" ON)
option(BUILD_DEBUG "Build a shared debug release" OFF)
option(BUILD_TESTS "Build test code" ON)

#library version
set(MAJOR_VERSION 1)
set(MINOR_VERSION 0)
set(PATCH_VERSION 0)

if(${BUILD_DEBUG})

	set(RELEASE_VERSION "debug")

else()

	set(RELEASE_VERSION "prod")
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.22.0")

	add_definitions(-DMAJOR_VERSION=${MAJOR_VERSION})
	add_definitions(-DMINOR_VERSION=${MINOR_VERSION})
	add_definitions(-DPATCH_VERSION=${PATCH_VERSION})
	add_definitions(-DRELEASE_VERSION=\"${RELEASE_VERSION}\")
else()

	add_compile_definitions(MAJOR_VERSION=${MAJOR_VERSION})
	add_compile_definitions(MINOR_VERSION=${MINOR_VERSION})
	add_compile_definitions(PATCH_VERSION=${PATCH_VERSION})
	add_compile_definitions(RELEASE_VERSION=\"${RELEASE_VERSION}\")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options( -Wall -Wextra -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-deprecated -ansi -pedantic -fno-rtti)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include/script")
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

set(SOURCE "")
add_subdirectory("${PROJECT_SOURCE_DIR}/src")

#library type and filenames.
if(${BUILD_DEBUG})

	set(CMAKE_BUILD_TYPE Debug)
	set(LIB_FILENAME "ascript_debug")
else()

	set(CMAKE_BUILD_TYPE Release)
	set(LIB_FILENAME "ascript")
endif()

if(${BUILD_STATIC})
	add_library(ascript_static STATIC ${SOURCE})
	set_target_properties(ascript_static PROPERTIES OUTPUT_NAME ${LIB_FILENAME})
	target_compile_definitions(ascript_static PUBLIC "-DLIB_VERSION=\"static\"")
	install(TARGETS ascript_static DESTINATION lib)

	message("will build ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}-${RELEASE_VERSION}-shared")
endif()

if(${BUILD_SHARED}) 
	add_library(ascript_shared SHARED ${SOURCE})
	set_target_properties(ascript_shared PROPERTIES OUTPUT_NAME ${LIB_FILENAME})
	target_compile_definitions(ascript_shared PUBLIC "-DLIB_VERSION=\"shared\"")
	install(TARGETS ascript_shared DESTINATION lib)

	message("will build ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}-${RELEASE_VERSION}-static")
endif()

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
	add_executable(version src/tests/version.cpp)

	target_link_libraries(ascript ascript_shared dfw lm tools stdc++fs)
	target_link_libraries(interactive ascript_shared dfw lm tools stdc++fs)
	target_link_libraries(print_tokens ascript_shared dfw lm tools stdc++fs)
	target_link_libraries(print_code ascript_shared dfw lm tools stdc++fs)
	target_link_libraries(version ascript_shared dfw lm tools stdc++fs)
endif()


