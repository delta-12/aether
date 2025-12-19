cmake_minimum_required(VERSION 3.30)
project(Aether LANGUAGES C CXX)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
# Add global flags for other compilers here
endif()

set(CMAKE_CXX_STANDARD 20)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
# Add global flags for other compilers here
endif()

option(AETHER_BUILD_TESTS "Build Aether tests" OFF)

################################################################################
# Versioning
################################################################################
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git status --porcelain
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
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
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SRCS
    ${SRC_DIR}/buffer.c
    ${SRC_DIR}/cobs.c
    ${SRC_DIR}/err.c
    ${SRC_DIR}/hashmap.c
    ${SRC_DIR}/leb128.c
    ${SRC_DIR}/router.c
    ${SRC_DIR}/serial.c
    ${SRC_DIR}/session.c
    ${SRC_DIR}/socket.c
    ${SRC_DIR}/transport.c
)

configure_file(
    ${INCLUDE_DIR}/version.h.in
    ${INCLUDE_DIR}/version.h
)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        ${SRCS}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${INCLUDE_DIR}
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME}
        PRIVATE
            -Wall -Wextra -Werror -Wpedantic
            $<$<CONFIG:Debug>:-g -O0 --coverage>
    )
    target_link_options(${PROJECT_NAME}
        PRIVATE
            $<$<CONFIG:Debug>:--coverage>
    )
# Add flags for other compilers here
endif()

################################################################################
# Tests
################################################################################
if(AETHER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

################################################################################
# CI and tools
################################################################################
set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
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

################################################################################
# Coverage
################################################################################
find_program(GCOVR_PATH gcovr
    HINTS "$ENV{HOME}/.local/bin"
          "/root/.local/bin"
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(GCOVR_PATH)
        message(STATUS "Found Gcovr at: ${GCOVR_PATH}")
        set(GCOVR_ARGS -r ${CMAKE_BINARY_DIR} -f ${INCLUDE_DIR} -f ${SRC_DIR} --exclude-throw-branches)
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
else()
    message(WARNING "Gcovr does not support compiler.")
endif()
