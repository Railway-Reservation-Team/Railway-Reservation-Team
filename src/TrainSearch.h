#ifndef TRAINSEARCH_H
#define TRAINSEARCH_H

#include "Train.h"

#include <string>
#include <vector>

// Search trains by source, destination and journey date.
// Source and destination matching is case-insensitive.
// Leading and trailing whitespace is ignored.
// Only active trains matching the requested journey date are returned.
// Invalid or empty search inputs produce an empty result.
std::vector<Train> searchTrains(
    const std::vector<Train>& trains,
    const std::string& source,
    const std::string& destination,
    const std::string& journeyDate);

#endif