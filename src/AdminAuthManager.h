#ifndef ADMIN_AUTH_MANAGER_H
#define ADMIN_AUTH_MANAGER_H

#include <string>

namespace AdminAuthManager {

bool hasAdminAccount(std::string& errorMessage);

bool setupAdmin(
    const std::string& username,
    const std::string& password,
    std::string& errorMessage
);

bool login(
    const std::string& username,
    const std::string& password,
    std::string& errorMessage
);

bool isAuthenticated();

void logout();

void resetForTesting();

}

#endif
