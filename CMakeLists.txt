cmake_minimum_required(VERSION 3.30)
project(Aether LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
# Add global flags for other compilers here
endif()

option(AETHER_BUILD_TESTS "Build Aether tests" OFF)

################################################################################
# Library
################################################################################
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SRCS
    # TODO
    ${SRC_DIR}/cobs.c
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
# CI and tools
################################################################################
# TODO
# set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
# set(CPPCHECK_SOURCES ${COMMON_SRCS} ${CPP_SRCS} ${C_SRCS})
# include(${TOOLS_DIR}/cppcheck/cppcheck.cmake)
# set(UNCRUSTIFY_SOURCES ${CPPCHECK_SOURCES})
# set(UNCRUSTIFY_INC_DIRS ${COMMON_INC_DIR} ${CPP_INC_DIR} ${C_INC_DIR})
# include(${TOOLS_DIR}/uncrustify/uncrustify.cmake)

################################################################################
# Tests
################################################################################
if(AETHER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

################################################################################
# Coverage
################################################################################
find_program(GCOVR_PATH gcovr
    HINTS ~/.local/bin/
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