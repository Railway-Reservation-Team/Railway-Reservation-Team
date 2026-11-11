cmake_minimum_required(VERSION 3.16)

project(RailwayReservationSystem
    VERSION 1.0.0
    DESCRIPTION "Console-based Railway Reservation System"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RRS_ENABLE_COVERAGE "Enable GCC coverage instrumentation" OFF)

find_package(OpenSSL REQUIRED COMPONENTS Crypto)

set(RRS_CORE_SOURCES
    src/AdminAuthManager.cpp
    src/AdminFareReservationManager.cpp
    src/AdminTrainManager.cpp
    src/AdminTrainMenu.cpp
    src/AuthManager.cpp
    src/BookingManager.cpp
    src/Cancellation.cpp
    src/FareCalculator.cpp
    src/PassengerDetails.cpp
    src/Payment.cpp
    src/SeatInventory.cpp
    src/TrainSearch.cpp
    src/Validation.cpp
)

function(rrs_enable_coverage target)
    if(RRS_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE --coverage -O0 -g)
        target_link_options(${target} PRIVATE --coverage)
    endif()
endfunction()

add_library(rrs_core STATIC ${RRS_CORE_SOURCES})
target_include_directories(rrs_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(rrs_core PUBLIC OpenSSL::Crypto)

if(MSVC)
    target_compile_options(rrs_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(rrs_core PRIVATE -Wall -Wextra -Wpedantic)
endif()
rrs_enable_coverage(rrs_core)

add_executable(railway src/main.cpp)
target_link_libraries(railway PRIVATE rrs_core)
rrs_enable_coverage(railway)

add_custom_command(TARGET railway POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:railway>/data"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/data/trains.dat"
            "$<TARGET_FILE_DIR:railway>/data/trains.dat"
)

include(CTest)
if(BUILD_TESTING)
    set(RRS_TEST_SOURCES
        tests/AdminFareReservationTests.cpp
        tests/AdminTrainManagerTests.cpp
        tests/AuthManagerTests.cpp
        tests/BookingIntegrationTest.cpp
        tests/BookingManagerTests.cpp
        tests/FileStoreTests.cpp
        tests/PaymentStructureTests.cpp
        tests/SeatInventoryTests.cpp
        tests/US05Tests.cpp
        tests/US13Tests.cpp
        tests/test_modules.cpp
        tests/TrainDataFileTests.cpp
    )

    foreach(test_source IN LISTS RRS_TEST_SOURCES)
        get_filename_component(test_name "${test_source}" NAME_WE)
        set(test_target "rrs_${test_name}")
        add_executable("${test_target}" "${test_source}")
        target_link_libraries("${test_target}" PRIVATE rrs_core)
        rrs_enable_coverage("${test_target}")

        if(MSVC)
            target_compile_options("${test_target}" PRIVATE /W4 /permissive-)
        else()
            target_compile_options("${test_target}" PRIVATE -Wall -Wextra -Wpedantic)
        endif()

        set(test_work_dir "${CMAKE_CURRENT_BINARY_DIR}/test-work/${test_name}")
        file(MAKE_DIRECTORY "${test_work_dir}/data")
        add_test(NAME "${test_name}" COMMAND "${test_target}")
        set_tests_properties("${test_name}" PROPERTIES
            WORKING_DIRECTORY "${test_work_dir}"
            TIMEOUT 30
        )
    endforeach()
endif()
