#ifndef TRAINSEARCH_H
#define TRAINSEARCH_H

#include "Train.h"

#include <string>
#include <vector>

// Search trains by source, destination and journey date.
std::vector<Train> searchTrains(
    const std::vector<Train>& trains,
    const std::string& source,
    const std::string& destination,
    const std::string& journeyDate);

#endif