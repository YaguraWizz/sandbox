cmake_minimum_required(VERSION 3.15)

project(sandbox-vector VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" OFF)
set(CLANG_FORMAT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vector/v1/vector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vector/v1/allocator.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/include/vector/v2/vector.hpp
)

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>  
) 

if(BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(${PROJECT_NAME}_tests         
        ${CLANG_FORMAT_FILE}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_vector.v1.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tests/test_vector.v2.cpp 
    )

    target_link_libraries(${PROJECT_NAME}_tests
        PRIVATE
            ${PROJECT_NAME}
            GTest::gtest
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()