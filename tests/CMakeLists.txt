cmake_minimum_required(VERSION 3.30)
project(AetherTests LANGUAGES C CXX)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" IS_TOP_LEVEL)

if(IS_TOP_LEVEL)
    enable_testing()
endif()

################################################################################
# GoogleTest
################################################################################
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

################################################################################
# Aether library
################################################################################
if(IS_TOP_LEVEL)
    find_package(Aether REQUIRED)
endif()

################################################################################
# Aether tests
################################################################################
set(${PROJECT_NAME}_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/buffer_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cobs_tests.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/leb128_tests.cc
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_SRCS})
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        ${${PROJECT_NAME}_SRCS}
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Aether
        GTest::gtest_main
        GTest::gmock
)
target_link_options(${PROJECT_NAME}
    PRIVATE
        --coverage
)
gtest_discover_tests(${PROJECT_NAME})
