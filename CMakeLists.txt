cmake_minimum_required(VERSION 3.15)
project(dolphin_tools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(Boost 1.66.0 REQUIRED)

if (${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
    option(PACKAGE_TESTS "Build the tests" ON)
endif()

add_subdirectory(src/Yaz0)

add_executable(dolphin_tools
        src/dolphin-tools.cpp)

target_include_directories(dolphin_tools PRIVATE src/Yaz0)
target_link_libraries(dolphin_tools yaz0)

if (${PACKAGE_TESTS} STREQUAL ON)
    enable_testing()
    find_package(GTest)
    add_subdirectory(test)
endif()