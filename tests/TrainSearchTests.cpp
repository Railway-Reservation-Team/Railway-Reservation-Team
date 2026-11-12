#include <string>
#include "TrainSearch.h"

#include <iostream>
#include <vector>

int main() {
    int passed = 0;
    int failed = 0;

    auto check = [&](bool condition, const char* name) {
        if (condition) {
            std::cout << "[PASS] " << name << '\n';
            ++passed;
        } else {
            std::cout << "[FAIL] " << name << '\n';
            ++failed;
        }
    };

    std::vector<Train> trains;

    Train t1;
    t1.trainNumber = "12627";
    t1.source = "Bangalore";
    t1.destination = "Chennai";
    t1.journeyDate = "2026-11-13";
    t1.active = true;

    Train t2 = t1;
    t2.trainNumber = "12628";

    Train t3 = t1;
    t3.trainNumber = "12629";
    t3.active = false;

    Train t4 = t1;
    t4.trainNumber = "12630";
    t4.journeyDate = "2026-11-14";

    Train t5 = t1;
    t5.trainNumber = "12631";
    t5.destination = "Hyderabad";

    trains = {t1, t2, t3, t4, t5};

    check(
        searchTrains(trains, "Bangalore", "Chennai", "2026-11-13").size() == 2,
        "Find multiple matching trains"
    );

    check(
        searchTrains(trains, "  BANGALORE ", " chennai  ",
                     "2026-11-13").size() == 2,
        "Case-insensitive search and whitespace handling"
    );

    check(
        searchTrains(trains, "", "Chennai", "2026-11-13").empty(),
        "Reject empty source"
    );

    check(
        searchTrains(trains, "Bangalore", "", "2026-11-13").empty(),
        "Reject empty destination"
    );

    check(
        searchTrains(trains, "Bangalore", "Chennai", "").empty(),
        "Reject empty journey date"
    );

    check(
    searchTrains(trains, "Bangalore", "Chennai",
                 "2026-11-14").size() == 1,
    "Filter by journey date"
);

    auto results =
        searchTrains(trains, "Bangalore", "Chennai", "2026-11-13");

    bool inactiveExcluded = true;
    for (const Train& train : results) {
        if (train.trainNumber == "12629") {
            inactiveExcluded = false;
        }
    }

    check(inactiveExcluded, "Exclude inactive trains");

    check(
        searchTrains(trains, "Bangalore", "Hyderabad",
                     "2026-11-13").size() == 1,
        "Match correct destination"
    );
    check(
        searchTrains(trains, "Bangalore", "Chennai",
                     "2026-99-45").empty(),
        "Reject invalid month and day"
    );

    check(
        searchTrains(trains, "Bangalore", "Chennai",
                     "2026-02-29").empty(),
        "Reject invalid non-leap-year date"
    );

    Train leapTrain = t1;
    leapTrain.journeyDate = "2024-02-29";
    std::vector<Train> leapTrains = {leapTrain};

    check(
        searchTrains(leapTrains, "Bangalore", "Chennai",
                     "2024-02-29").size() == 1,
        "Accept valid leap-year date"
    );

    check(
        searchTrains(trains, "Bangalore", "Chennai",
                     "2026/11/13").empty(),
        "Reject incorrect date format"
    );
    check(
        searchTrains(trains, "Mumbai", "Chennai",
                     "2026-11-13").empty(),
        "Return empty result when no trains match"
    );

    std::vector<Train> duplicateTrains = {t1, t1};

    check(
        searchTrains(duplicateTrains, "Bangalore", "Chennai",
                     "2026-11-13").size() == 2,
        "Preserve duplicate matching records"
    );

    std::vector<Train> largeTrains;

    for (int i = 0; i < 1000; ++i) {
        Train generated = t1;
        generated.trainNumber = std::to_string(20000 + i);
        largeTrains.push_back(generated);
    }

    check(
        searchTrains(largeTrains, "Bangalore", "Chennai",
                     "2026-11-13").size() == 1000,
        "Search correctly through 1000 trains"
    );

    check(
        searchTrains(trains, "  Bangalore  ", "  Chennai  ",
                     "2026-11-13").size() == 2,
        "Handle additional surrounding whitespace"
    );

    check(
        searchTrains(trains, "Bangalore", "Chennai",
                     "2026-11-15").empty(),
        "Return no results for unavailable journey date"
    );
 
   std::vector<Train> emptyTrains;

   check(
       searchTrains(emptyTrains, "Bangalore", "Chennai",
                    "2026-11-13").empty(),
       "Handle empty train collection"
   );

   check(
       searchTrains(trains, "Bangalore", "Bangalore",
                    "2026-11-13").empty(),
       "Handle identical source and destination"
   );

   check(
       searchTrains(trains, "   ", "Chennai",
                    "2026-11-13").empty(),
       "Reject whitespace-only source"
   );

   std::cout << "\nPassed: " << passed
              << "\nFailed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
