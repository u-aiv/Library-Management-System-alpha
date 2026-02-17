// Member header

#ifndef LIBRARY_MANAGEMENT_SYSTEM_MEMBER_H
#define LIBRARY_MANAGEMENT_SYSTEM_MEMBER_H

#include <string>
#include <vector>
#include <utility>

class Member {
private:
    std::string memberID;                       // Member Identification Digits
    std::string name;                           // Member's Name
    std::string phoneNumber;                    // Member's Phone Number
    std::vector<std::string> preference;        // Member's Preference Genres (MAX 5)
    std::string registrationDate;               // Member's Registration Date (YYYY-MM-DD)(UTC)
    std::string expiryDate;                     // Member's Expiry Date (YYYY-MM-DD)(UTC)
    int maxBooksAllowed{};                      // Member's max limit of books borrowed (Default 2)
    bool isAdmin{};                             // true = admin, false = member
    std::string passwordHash;                   // for authentication

public:
    // Constructor
    Member(std::string memberID, std::string name, std::string phoneNumber,
        std::vector<std::string> preference, std::string registrationDate, std::string expiryDate,
        int maxBooksAllowed = 2, bool isAdmin = false, std::string passwordHash = ""):
        memberID(std::move(memberID)), name(std::move(name)), phoneNumber(std::move(phoneNumber)),
        preference(std::move(preference)), registrationDate(std::move(registrationDate)),
        expiryDate(std::move(expiryDate)), maxBooksAllowed(maxBooksAllowed),
        isAdmin(isAdmin), passwordHash(std::move(passwordHash)) {}
    Member() = default;

    // Getters and Setters
    std::string getMemberID() const;
    std::string getName() const;
    std::string getPhoneNumber() const;
    std::vector<std::string> getPreference() const;
    std::string getRegistrationDate() const;
    std::string getExpiryDate() const;
    std::string getPasswordHash() const;
    bool isExpired() const;
    bool getAdmin() const;
    int getMaxBooksAllowed() const;
    bool authenticate(const std::string& password) const;

    // CSV function
    std::string toCSV() const;
    static Member fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_MEMBER_H