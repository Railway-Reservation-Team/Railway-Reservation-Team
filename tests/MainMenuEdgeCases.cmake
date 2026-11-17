if(NOT DEFINED APP OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "APP and WORK_DIR must be supplied")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}/data")

file(WRITE "${WORK_DIR}/data/bookings.dat"
"1234567890|77777|2099-12-31|Sleeper|Seed Passenger,30,Male|1|500.00|Confirmed\n")

set(input_file "${WORK_DIR}/menu-input.txt")

file(WRITE "${input_file}"
"4\nab\nSecureRailwayPass123\n4\nrailadmin\nSecureRailwayPass123\n6\n5\n1\nTest Passenger\ntest@example.com\n9876543210\nRailwayPass123\n1\nTest Passenger\ntest@example.com\n9876543210\nRailwayPass123\n2\nwrong@example.com\nWrongPassword123\n2\ntest@example.com\nWrongPassword123\n2\ntest@example.com\nRailwayPass123\n3\ninvalid\n3\n2\n\n3\n2\n1234567890\nN\n6\n7\nNo Such Passenger\n8\n")

execute_process(
    COMMAND "${APP}"
    INPUT_FILE "${input_file}"
    WORKING_DIRECTORY "${WORK_DIR}"
    OUTPUT_VARIABLE app_output
    ERROR_VARIABLE app_error
    RESULT_VARIABLE app_result
    TIMEOUT 30
)

if(NOT app_result EQUAL 0)
    message(FATAL_ERROR
        "Application returned ${app_result}. stderr: ${app_error}\n${app_output}")
endif()

foreach(expected_text
    "Setup failed"
    "Administrator account created"
    "Administrator login successful"
    "Administrator session ended"
    "No active administrator session"
    "Registration successful"
    "Login successful"
    "Error:"
    "Cancellation request stopped"
    "Passenger logged out successfully"
    "No bookings found for No Such Passenger"
    "Exiting system")
    string(FIND "${app_output}" "${expected_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR
            "Missing expected output: ${expected_text}\n${app_output}")
    endif()
endforeach()

message(STATUS "Main menu edge-case scenarios passed.")
