if(NOT DEFINED APP OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "APP and WORK_DIR must be supplied")
endif()

file(REMOVE_RECURSE "\${WORK_DIR}")
file(MAKE_DIRECTORY "\${WORK_DIR}/data")
file(WRITE "\${WORK_DIR}/data/trains.dat"
"77777|Smoke Test Express|Bengaluru|Chennai|10:00|18:00|8h|500|800|1000|400|20|20|20|20|2099-12-31\n")

set(input_file "\${WORK_DIR}/menu-input.txt")
file(WRITE "\${input_file}"
"9\n1\nTest Passenger\ntest@example.com\n9876543210\nRailwayPass123\n2\ntest@example.com\nRailwayPass123\n3\n1\nBengaluru\nChennai\n2099-12-31\n1\n1\n1\nTest Passenger\n20\nFemale\nY\n1\n1\n7\n")

execute_process(
    COMMAND "\${APP}"
    INPUT_FILE "\${input_file}"
    WORKING_DIRECTORY "\${WORK_DIR}"
    OUTPUT_VARIABLE app_output
    ERROR_VARIABLE app_error
    RESULT_VARIABLE app_result
    TIMEOUT 20
)

if(NOT app_result EQUAL 0)
    message(FATAL_ERROR "Console application returned \${app_result}. stderr: \${app_error}")
endif()

foreach(expected_text
    "Invalid choice"
    "Registration successful"
    "Login successful"
    "BOOKING CONFIRMED"
    "Payment Status: SUCCESS"
    "Passenger logged out successfully"
    "Exiting system")
    string(FIND "\${app_output}" "\${expected_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Expected console output not found: \${expected_text}. Output: \${app_output}")
    endif()
endforeach()

message(STATUS "Console menu, passenger registration/login and successful booking paths passed.")
