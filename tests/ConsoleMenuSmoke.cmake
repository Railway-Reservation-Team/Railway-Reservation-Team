if(NOT DEFINED APP OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "APP and WORK_DIR must be supplied")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}/data")
file(WRITE "${WORK_DIR}/data/trains.dat"
"77777|Smoke Test Express|Bengaluru|Chennai|10:00|18:00|8h|500|800|1000|400|1|20|20|20|2099-12-31\n")
file(WRITE "${WORK_DIR}/data/bookings.dat"
"1234567890|77777|2099-12-31|Sleeper|Seed Passenger,30,Male|1|500.00|Confirmed\n")

set(input_file "${WORK_DIR}/menu-input.txt")
file(WRITE "${input_file}"
"9\n1\nBob Tester\nbob@test.com\n9999999999\nBobPass123\n2\nbob@test.com\nBobPass123\n3\n1\nBengaluru\nChennai\n2099-12-31\n1\n1\n1\nBob Tester\n30\nFemale\nN\n3\n1\nBengaluru\nChennai\n2099-12-31\n1\n1\n1\nBob Tester\n30\nFemale\nY\n1\n2\n3\n1\nBengaluru\nChennai\n2099-12-31\n1\n1\n1\nBob Tester\n30\nFemale\nY\n1\n1\n7\nBob Tester\n3\n2\n\n3\n2\n1234567890\nN\n3\n2\n1234567890\nY\n6\n8\n")

execute_process(
    COMMAND "${APP}"
    INPUT_FILE "${input_file}"
    WORKING_DIRECTORY "${WORK_DIR}"
    OUTPUT_VARIABLE app_output
    ERROR_VARIABLE app_error
    RESULT_VARIABLE app_result
    TIMEOUT 20
)

if(NOT app_result EQUAL 0)
    message(FATAL_ERROR "Console application returned ${app_result}. stderr: ${app_error}")
endif()

foreach(expected_text
    "Invalid choice"
    "Registration successful"
    "Login successful"
    "Booking cancelled. No seats allocated."
    "Failed payment receipt recorded."
    "BOOKING CONFIRMED"
    "Payment Status: SUCCESS"
    "E-ticket saved successfully"
    "Found 1 booking(s)"
    "Error: PNR cannot be empty."
    "Cancellation request stopped"
    "TICKET CANCELLED SUCCESSFULLY"
    "Passenger logged out successfully"
    "Exiting system")
    string(FIND "${app_output}" "${expected_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Expected console output not found: ${expected_text}. Output: ${app_output}")
    endif()
endforeach()

message(STATUS "Console end-to-end passenger registration, booking, payment, cancellation and administrator login flows passed.")
