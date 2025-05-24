cmake_minimum_required(VERSION 3.10)

project(cparse LANGUAGES C)

enable_testing()

find_program(LCOV lcov)
find_program(GENHTML genhtml)
add_custom_target(cparse_coverage
	COMMAND ${LCOV} -q -c -d ${CMAKE_BINARY_DIR} -o ${CMAKE_BINARY_DIR}/lcov.info
	COMMAND ${GENHTML} -q ${CMAKE_BINARY_DIR}/lcov.info -o ${CMAKE_SOURCE_DIR}/report/coverage
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(NOT MSVC)
	if(ARCH STREQUAL "x64")
		set(CMAKE_C_FLAGS "-m64")
	elseif(ARCH STREQUAL "x86")
		set(CMAKE_C_FLAGS "-m32")
	endif()
endif()

if(WIN32)
    set(LIB_EXT ".lib")
else()
	set(LIB_EXT ".a")
endif()

include(FetchContent)
FetchContent_Declare(cutils
  GIT_REPOSITORY    https://github.com/cgware/cutils.git
  GIT_TAG           main
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/deps/cutils
)
FetchContent_MakeAvailable(cutils)

file(GLOB_RECURSE cparse_src src/*.h src/*.c)
add_library(cparse ${cparse_src})
target_link_libraries(cparse PUBLIC cutils)
target_include_directories(cparse PUBLIC include PRIVATE src)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(cparse PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(cparse PRIVATE -coverage)
	else()
		target_compile_options(cparse PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(cparse PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/cparse/${ARCH}-Debug/
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/cparse/${ARCH}-Release/
	PREFIX ""
)

file(GLOB_RECURSE cparse_test_src test/*.h test/*.c)
add_executable(cparse_test ${cparse_test_src})
target_include_directories(cparse_test PRIVATE src)
target_link_libraries(cparse_test cparse ctest)
if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(cparse_test PRIVATE -Wall -Wextra -Werror -pedantic -coverage -O0 -ggdb)
		target_link_options(cparse_test PRIVATE -coverage)
	else()
		target_compile_options(cparse_test PRIVATE -Wall -Wextra -Werror -pedantic)
	endif()
endif()
set_target_properties(cparse_test PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/cparse_test/${ARCH}-Debug/
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/cparse_test/${ARCH}-Release/
)
add_test(cparse_build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --config ${CMAKE_BUILD_TYPE} --target cparse_test)
add_test(cparse ${CMAKE_SOURCE_DIR}/bin/cparse_test/${ARCH}-${CMAKE_BUILD_TYPE}/cparse_test)
set_tests_properties(cparse PROPERTIES DEPENDS cparse_build)
