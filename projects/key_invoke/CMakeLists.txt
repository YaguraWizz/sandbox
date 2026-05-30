cmake_minimum_required(VERSION 3.15)
project(key_invoke VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" OFF)
set(CLANG_FORMAT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format)

add_library(${PROJECT_NAME} INTERFACE)
target_sources(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/key_invoke/key_invoke.hpp
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
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(googletest nlohmann_json)
    include(GoogleTest)


    set(TEST_CXX_STANDARDS 17 20 23)
    foreach(STD IN LISTS TEST_CXX_STANDARDS)
        set(TEST_TARGET ${PROJECT_NAME}_tests_cpp${STD})

        add_executable(${TEST_TARGET}         
            ${CLANG_FORMAT_FILE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_key_invoke.cpp
        )

        set_target_properties(${TEST_TARGET} PROPERTIES
            CXX_STANDARD ${STD}
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
        )

        target_link_libraries(${TEST_TARGET}
            PRIVATE
                ${PROJECT_NAME}
                $<BUILD_INTERFACE:nlohmann_json::nlohmann_json>
                GTest::gtest_main
        )

        gtest_discover_tests(${TEST_TARGET} 
            EXTRA_ARGS "--gtest_filter=*" 
            TEST_PREFIX "cpp${STD}_"
        )
    endforeach()
endif()