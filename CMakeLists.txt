cmake_minimum_required(VERSION 3.16)

project(RailwayReservationSystem
    VERSION 1.0.0
    DESCRIPTION "Team 12 Railway Reservation System"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep the application sources in src/ and compile every module into one executable.
file(GLOB RRS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(railway ${RRS_SOURCES})
target_include_directories(railway PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(MSVC)
    target_compile_options(railway PRIVATE /W4)
else()
    target_compile_options(railway PRIVATE -Wall -Wextra -pedantic)
endif()

# The application reads data files relative to its working directory.
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

include(CTest)
if(BUILD_TESTING)
    add_executable(test_modules
        tests/test_modules.cpp
        src/Validation.cpp
        src/TrainSearch.cpp
        src/FareCalculator.cpp
    )
    target_include_directories(test_modules PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    if(MSVC)
        target_compile_options(test_modules PRIVATE /W4)
    else()
        target_compile_options(test_modules PRIVATE -Wall -Wextra -pedantic)
    endif()

    add_test(NAME module_tests COMMAND test_modules)
endif()
