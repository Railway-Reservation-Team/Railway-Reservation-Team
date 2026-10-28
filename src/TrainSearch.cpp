#include "TrainSearch.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// Convert text to lowercase and remove surrounding spaces.
static std::string normalizeText(const std::string& text) {
    std::string result = text;

    result.erase(
        result.begin(),
        std::find_if(result.begin(), result.end(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            })
    );

    result.erase(
        std::find_if(result.rbegin(), result.rend(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(),
        result.end()
    );

    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        }
    );

    return result;
}

std::vector<Train> searchTrains(
    const std::vector<Train>& trains,
    const std::string& source,
    const std::string& destination,
    const std::string& journeyDate) {

    std::vector<Train> results;

    const std::string requestedSource = normalizeText(source);
    const std::string requestedDestination = normalizeText(destination);

    if (requestedSource.empty() ||
        requestedDestination.empty() ||
        journeyDate.empty()) {
        return results;
    }

    for (const Train& train : trains) {
        if (train.active &&
            normalizeText(train.source) == requestedSource &&
            normalizeText(train.destination) == requestedDestination &&
            train.journeyDate == journeyDate) {
            results.push_back(train);
        }
    }

    return results;
}