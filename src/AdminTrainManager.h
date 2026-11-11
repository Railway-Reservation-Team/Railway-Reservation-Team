#ifndef ADMIN_TRAIN_MANAGER_H
#define ADMIN_TRAIN_MANAGER_H

#include "Train.h"
#include <string>
#include <vector>

namespace AdminTrainManager {
    bool addTrain(
        const std::string& filename,
        const Train& train,
        std::string& errorMessage
    );

    bool viewTrains(
        const std::string& filename,
        std::vector<Train>& trains,
        std::string& errorMessage
    );

    bool updateTrain(
        const std::string& filename,
        const std::string& trainNumber,
        const Train& updatedTrain,
        std::string& errorMessage
    );

    bool deleteTrain(
        const std::string& filename,
        const std::string& trainNumber,
        std::string& errorMessage
    );

    bool deactivateTrain(
        const std::string& filename,
        const std::string& trainNumber,
        std::string& errorMessage
    );
}

#endif
