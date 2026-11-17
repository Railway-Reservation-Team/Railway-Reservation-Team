
if(NOT DEFINED APP OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "APP and WORK_DIR must be supplied")
endif()

function(run_scenario scenario_name input_text)
    set(scenario_dir "${WORK_DIR}/${scenario_name}")

    file(REMOVE_RECURSE "${scenario_dir}")
    file(MAKE_DIRECTORY "${scenario_dir}/data")

    file(WRITE "${scenario_dir}/data/trains.dat"
"77777|Smoke Test Express|Bengaluru|Chennai|10:00|18:00|8h|500|800|1000|400|1|0|20|20|2099-12-31\n")

    file(WRITE "${scenario_dir}/input.txt" "${input_text}")

    execute_process(
        COMMAND "${APP}"
        INPUT_FILE "${scenario_dir}/input.txt"
        WORKING_DIRECTORY "${scenario_dir}"
        OUTPUT_VARIABLE app_output
        ERROR_VARIABLE app_error
        RESULT_VARIABLE app_result
        TIMEOUT 20
    )

    if(NOT "${app_result}" STREQUAL "0")
        message(FATAL_ERROR
            "Scenario ${scenario_name} failed. Result: ${app_result}. Error: ${app_error}. Output: ${app_output}")
    endif()

    foreach(expected IN LISTS ARGN)
        string(FIND "${app_output}" "${expected}" found_at)

        if(found_at EQUAL -1)
            message(FATAL_ERROR
                "Scenario ${scenario_name}: expected output not found: ${expected}. Output: ${app_output}")
        endif()
    endforeach()

    message(STATUS "Scenario passed: ${scenario_name}")
endfunction()

# Scenario 1: Unauthenticated menu options.
run_scenario(
    "UnauthenticatedMenu"
    [=[
6
5
7
Nobody
9
8
]=]
    "No active passenger session"
    "No active administrator session"
    "No bookings found for Nobody."
    "Invalid choice."
    "Exiting system"
)

# Common passenger registration and login.
set(login_input [=[
1
Test Passenger
test@example.com
9876543210
RailwayPass123
2
test@example.com
RailwayPass123
]=])

# Scenario 2: Booking validation and unavailable seats.
run_scenario(
    "BookingValidation"
    "${login_input}3
1

Bengaluru

Chennai
bad-date
2099-12-31
0
1
0
2
5
1
2
2
5
3
8
"
    "Login successful"
    "Train Option: 1"
    "Only 1 seats are available in Sleeper"
    "Sorry, no seats are available in AC 3-Tier."
    "Booking cancelled."
    "Exiting system"
)

# Scenario 3: No trains for selected date and no matching route.
run_scenario(
    "NoTrainAndNoRoute"
    "${login_input}3
1
Bengaluru
Chennai
2099-12-30
3
1
Mumbai
Delhi
2099-12-31
8
"
    "Login successful"
    "No train records found for this date."
    "No trains found for this route and date."
    "Exiting system"
)

# Scenario 4: Passenger declines cancellation.
run_scenario(
    "CancellationDeclined"
    "${login_input}3
2
1234567890
N
8
"
    "Login successful"
    "Cancellation request stopped."
    "Exiting system"
)


# Additional batch: main menu and booking/cancellation error paths.

# Scenario 5: Failed passenger login.
run_scenario(
    "FailedPassengerLogin"
    [=[
2
missing@example.com
WrongPassword123
8
]=]
    "Login failed:"
    "Exiting system"
)

# Scenario 6: Attempt booking without a passenger session.
run_scenario(
    "BookingWithoutLogin"
    [=[
3
8
]=]
    "Please register and log in before booking."
    "Exiting system"
)

# Scenario 7: Invalid booking menu choice.
run_scenario(
    "InvalidBookingMenuChoice"
    "${login_input}3
invalid
8
"
    "Login successful"
    "Error:"
    "Exiting system"
)

# Scenario 8: Empty PNR during cancellation.
run_scenario(
    "BlankCancellationPNR"
    "${login_input}3
2

8
"
    "PNR cannot be empty."
    "Exiting system"
)

# Scenario 9: Cancellation attempt with an unknown PNR.
run_scenario(
    "FailedCancellation"
    "${login_input}3
2
9999999999
Y
8
"
    "Cancellation failed:"
    "Exiting system"
)

# Scenario 10: Invalid passenger registration details.
run_scenario(
    "InvalidPassengerRegistration"
    [=[
1
Bad Name
not-an-email
123
short
8
]=]
    "Registration failed:"
    "Exiting system"
)

# Scenario 11: Duplicate passenger registration.
run_scenario(
    "DuplicatePassengerRegistration"
    "${login_input}1
Test Passenger
test@example.com
9876543210
RailwayPass123
8
"
    "Registration successful"
    "Registration failed:"
    "Exiting system"
)

# Scenario 12: End input while booking is waiting for menu input.
run_scenario(
    "BookingInputEOF"
    "${login_input}3
"
    "Login successful"
)

message(STATUS "All additional console coverage scenarios passed.")
