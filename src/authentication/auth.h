#ifndef LIBRARY_MANAGEMENT_SYSTEM_AUTH_H
#define LIBRARY_MANAGEMENT_SYSTEM_AUTH_H

#include <string>

namespace auth {
// MAX length limit of password
constexpr size_t MAX_PASSWORD_LENGTH = 64;

// Hash the password for secure storage
std::string hashPassword(const std::string& password);

// Verify the password
bool verifyPassword(const std::string& password, const std::string& storedHash);
}

#endif //LIBRARY_MANAGEMENT_SYSTEM_AUTH_H