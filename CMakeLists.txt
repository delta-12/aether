cmake_minimum_required(VERSION 3.30)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Aether LANGUAGES C CXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(AETHER_TARGET aether)

option(AETHER_BUILD_TESTS "Build Aether tests" OFF)
option(AETHER_ENABLE_COVERAGE "Build Aether with coverage enabled" OFF)
option(AETHER_ENABLE_ASAN "Build Aether with AddressSanitizer enabled" OFF)

################################################################################
# Versioning
################################################################################
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "^v([0-9]+)" _ "${GIT_TAG}")

if(CMAKE_MATCH_1)
    set(AETHER_GIT_VERSION_MAJOR ${CMAKE_MATCH_1})
else()
    set(AETHER_GIT_VERSION_MAJOR 0)
endif()

message(STATUS "Git tag: ${GIT_TAG}")
message(STATUS "Version: ${AETHER_GIT_VERSION_MAJOR}")

execute_process(
    COMMAND git status --porcelain --untracked-files=no
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_STATUS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_STATUS)
    set(GIT_DIRTY "Dirty")
else()
    set(GIT_DIRTY "Clean")
endif()

################################################################################
# Library
################################################################################
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SRCS
    ${SRC_DIR}/aether.c
    ${SRC_DIR}/buffer.c
    ${SRC_DIR}/cobs.c
    ${SRC_DIR}/err.c
    ${SRC_DIR}/hash.c
    ${SRC_DIR}/hashmap.c
    ${SRC_DIR}/leb128.c
    ${SRC_DIR}/log.c
    ${SRC_DIR}/memory.c
    ${SRC_DIR}/random.c
    ${SRC_DIR}/router.c
    ${SRC_DIR}/serial.c
    ${SRC_DIR}/socket.c
    ${SRC_DIR}/tcp.c
    ${SRC_DIR}/tick.c
    ${SRC_DIR}/transport.c
)

set(GENERATED_VERSION_HEADER ${GENERATED_INCLUDE_DIR}/version.h)
configure_file(
    ${INCLUDE_DIR}/version.h.in
    ${GENERATED_VERSION_HEADER}
    @ONLY
)

add_library(${AETHER_TARGET})
add_library(Aether::Aether ALIAS ${AETHER_TARGET})
target_sources(${AETHER_TARGET}
    PRIVATE
        ${SRCS}
)
target_include_directories(${AETHER_TARGET}
    PUBLIC
        ${INCLUDE_DIR}
        ${GENERATED_INCLUDE_DIR}
)
target_compile_definitions(${AETHER_TARGET}
    PRIVATE
        AETHER_GIT_VERSION_MAJOR=${AETHER_GIT_VERSION_MAJOR}
        $<$<BOOL:${AETHER_LOG_ENABLED}>:AETHER_LOG_ENABLED>
        $<$<BOOL:${AETHER_MEMORY_STATIC_ALLOCATOR}>:AETHER_MEMORY_STATIC_ALLOCATOR>
        $<$<BOOL:${AETHER_TICK_USER}>:AETHER_TICK_USER>
        $<$<BOOL:${AETHER_LOG_LEVEL}>:AETHER_LOG_LEVEL=${AETHER_LOG_LEVEL}>
        $<$<BOOL:${AETHER_TRANSPORT_MTU}>:AETHER_TRANSPORT_MTU=${AETHER_TRANSPORT_MTU}>
        $<$<BOOL:${AETHER_MEMORY_STATIC_ALLOCATOR_SIZE}>:AETHER_MEMORY_STATIC_ALLOCATOR_SIZE=${AETHER_MEMORY_STATIC_ALLOCATOR_SIZE}>
        $<$<BOOL:${AETHER_SESSION_RETRIES}>:AETHER_SESSION_RETRIES=${AETHER_SESSION_RETRIES}>
        $<$<BOOL:${AETHER_SESSION_LEASE}>:AETHER_SESSION_LEASE=${AETHER_SESSION_LEASE}>
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${AETHER_TARGET}
        PRIVATE
            -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color=always
            -fPIC
            $<$<AND:$<CONFIG:Debug>,$<BOOL:${AETHER_ENABLE_COVERAGE}>>:
                -g --coverage
            >
            $<$<CONFIG:Debug>:
                $<IF:$<BOOL:${AETHER_ENABLE_ASAN}>,-O1,-O0>
            >
            $<$<AND:$<CONFIG:Debug>,$<BOOL:${AETHER_ENABLE_ASAN}>>:
                -fsanitize=address -fno-omit-frame-pointer
            >
    )
    target_link_options(${AETHER_TARGET}
        PRIVATE
            $<$<AND:$<CONFIG:Debug>,$<BOOL:${AETHER_ENABLE_COVERAGE}>>:
                --coverage
            >
            $<$<AND:$<CONFIG:Debug>,$<BOOL:${AETHER_ENABLE_ASAN}>>:
                -fsanitize=address
            >
    )
# Add flags for other compilers here
endif()

################################################################################
# Arduino
################################################################################
set(ARDUINO_LIB_NAME "AetherArduino")
set(ARDUINO_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/${ARDUINO_LIB_NAME}")
set(ARDUINO_SRC_DIR "${ARDUINO_LIB_DIR}/src")

file(MAKE_DIRECTORY "${ARDUINO_SRC_DIR}")
file(MAKE_DIRECTORY "${ARDUINO_EXAMPLES_DIR}")

set(ARDUINO_COPY_SOURCES ${ARDUINO_LIB_NAME}-copy-sources)
add_custom_command(
    OUTPUT ${ARDUINO_COPY_SOURCES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${SRC_DIR} ${ARDUINO_SRC_DIR}
)

set(ARDUINO_COPY_HEADERS ${ARDUINO_LIB_NAME}-copy-headers)
add_custom_command(
    OUTPUT ${ARDUINO_COPY_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${INCLUDE_DIR} ${ARDUINO_SRC_DIR}
)

set(ARDUINO_VERSION_HEADER ${ARDUINO_SRC_DIR}/version.h)
add_custom_command(
    OUTPUT ${ARDUINO_VERSION_HEADER}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GENERATED_VERSION_HEADER} ${ARDUINO_VERSION_HEADER}
    DEPENDS ${GENERATED_VERSION_HEADER}
)

string(REGEX REPLACE "^v" "" ARUDINO_LIB_VERSION "${GIT_TAG}")
string(REGEX REPLACE "-.*$" "" ARUDINO_LIB_VERSION "${ARUDINO_LIB_VERSION}")
if(NOT ARUDINO_LIB_VERSION)
    set(ARUDINO_LIB_VERSION "0.0.0")
endif()
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/library.properties.in
    ${ARDUINO_LIB_DIR}/library.properties
    @ONLY
)

set(ARDUINO_LIB_ZIP "${CMAKE_CURRENT_BINARY_DIR}/${ARDUINO_LIB_NAME}.zip")
add_custom_target(${ARDUINO_LIB_NAME}
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${ARDUINO_LIB_ZIP}" --format=zip ${ARDUINO_LIB_DIR}
    DEPENDS ${ARDUINO_COPY_SOURCES} ${ARDUINO_COPY_HEADERS} ${ARDUINO_VERSION_HEADER}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

################################################################################
# Tests
################################################################################
if(AETHER_BUILD_TESTS AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
    add_subdirectory(tests)
endif()

################################################################################
# CI and tools
################################################################################
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    set(CPPCHECK_SOURCES ${SRCS})
    set(UNCRUSTIFY_SOURCES ${CPPCHECK_SOURCES})
    set(UNCRUSTIFY_INC_DIRS ${INCLUDE_DIR})
    if (CPPCHECK_SOURCES)
        include(${TOOLS_DIR}/cppcheck/cppcheck.cmake)
    endif()
    if (CPPCHECK_SOURCES AND UNCRUSTIFY_INC_DIRS)
        # TODO add tests to uncrustify sources
        set(UNCRUSTIFY_SOURCES ${CPPCHECK_SOURCES})
        include(${TOOLS_DIR}/uncrustify/uncrustify.cmake)
    endif()
endif()

################################################################################
# Coverage
################################################################################
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND AETHER_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            find_program(GCOVR_PATH gcovr
                HINTS "$ENV{HOME}/.local/bin"
                    "/root/.local/bin"
            )
            if(GCOVR_PATH)
                message(STATUS "Found Gcovr at: ${GCOVR_PATH}")
                set(GCOVR_ARGS -r ${CMAKE_CURRENT_BINARY_DIR} -f ${INCLUDE_DIR} -f ${SRC_DIR} --exclude-throw-branches)
                if(DEFINED ENV{GCOV_EXECUTABLE})
                    set(GCOVR_ARGS --gcov-executable $ENV{GCOV_EXECUTABLE} ${GCOVR_ARGS})
                endif()
                add_custom_target(
                    coverage
                    COMMAND ${GCOVR_PATH} ${GCOVR_ARGS} --html-details --html-theme github.dark-green -o coverage.html
                    COMMENT "Generating code coverage report..."
                    DEPENDS test
                )
                add_custom_target(
                    coverage-no-test
                    COMMAND ${GCOVR_PATH} ${GCOVR_ARGS} --html-details --html-theme github.dark-green -o coverage.html
                    COMMENT "Generating code coverage report..."
                )
                add_custom_target(
                    sonarqube-coverage
                    COMMAND ${GCOVR_PATH} ${GCOVR_ARGS} --sonarqube -o coverage.xml
                    COMMENT "Generating code coverage report..."
                )
            else()
                message(WARNING "Gcovr not found.")
            endif()
        endif()
    else()
        message(WARNING "Gcovr does not support compiler.")
    endif()
endif()
