// MemberManager.cpp Implementation

#include "MemberManager.h"
#include "../utils/FileHandler.h"
#include "../authentication/auth.h"
#include <algorithm>
#include <iostream>

// Constructor
MemberManager::MemberManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "MemberID,Name,PhoneNumber,Preference,RegistrationDate,ExpiryDate,MaxBooksAllowed,IsAdmin,PasswordHash"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
}

// private: Helper: Load members data from file
void MemberManager::loadFromFile() {
    members.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // Skip header (line 1)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                members.push_back(Member::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("Failed to load members file: " + std::string(e.what()));
    }
}

// private: Helper: Save members data to file
void MemberManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("MemberID,Name,PhoneNumber,Preference,RegistrationDate,ExpiryDate,MaxBooksAllowed,isAdmin,PasswordHash");

    // Add all members
    for (const auto& member : members) {
        lines.push_back(member.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to save members file: " + std::string(e.what()));
    }
}

// private: Helper: Check autoSave flag to decide whether need to save
void MemberManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// Add a new Member
bool MemberManager::addMember(const Member& member) {
    // Check if MemberID already exists
    if (isMemberIDExists(member.getMemberID())) {
        return false;
    }
    members.push_back(member);
    saveIfNeeded();
    return true;
}

// Delete Member by MemberID
bool MemberManager::deleteMember(const std::string& MemberID) {
    auto it = std::find_if(members.begin(), members.end(),
        [&](const Member& member) { return member.getMemberID() == MemberID; });

    if (it != members.end()) {
        members.erase(it);
        saveIfNeeded();
        return true;
    }
    return false;
}

// Update existing book
bool MemberManager::updateMember(const Member& member) {
    Member *existingMember = findMemberByID(member.getMemberID());

    if (existingMember == nullptr) {
        return false;
    }
    *existingMember = member;
    saveIfNeeded();
    return true;
}

// Find member by MemberID
Member* MemberManager::findMemberByID(const std::string &MemberID) {
    for (auto& member : members) {
        if (member.getMemberID() == MemberID) {
            return &member;
        }
    }
    return nullptr;
}

// Find members Template
// matchMode = 0 --> exact matching (case-sensitive) (default)
// matchMode = 1 --> fuzzy matching (case-insensitive)
template<typename Getter>
std::vector<const Member*> findByField(
    const std::vector<Member>& members,
    const std::string& key,
    Getter getter,
    int matchMode = 0
    ) {
        if (matchMode != 0 && matchMode != 1) {
            throw std::runtime_error("Invalid match mode");
        }

        std::vector<const Member*> results;

        if (matchMode == 0) {
            for (const auto& member : members) {
                if ((member.*getter)() == key) {
                    results.push_back(&member);
                }
            }
        }
        else {
            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

            for (const auto& member : members) {
                std::string memberKey = (member.*getter)();
                std::transform(memberKey.begin(), memberKey.end(), memberKey.begin(), ::tolower);

                if (memberKey.find(lowerKey) != std::string::npos)
                    results.push_back(&member);
            }
        }
        return results;
}

// Find member by name
std::vector<const Member*> MemberManager::findByName(const std::string& name, int matchMode) const {
    return findByField(members, name, &Member::getName , matchMode);
}

// Find member by author
std::vector<const Member*> MemberManager::findByPhoneNumber(const std::string& phoneNumber, int matchMode) const {
    return findByField(members, phoneNumber, &Member::getPhoneNumber, matchMode);
}

// Find member by registration date
std::vector<const Member*> MemberManager::findByRegistrationDate(const std::string& registrationDate, int matchMode) const {
    return findByField(members, registrationDate, &Member::getRegistrationDate, matchMode);
}

std::vector<const Member*> MemberManager::findByExpiryDate(const std::string& expiryDate, int matchMode) const {
    return findByField(members, expiryDate, &Member::getExpiryDate, matchMode);
}

// Find admins
std::vector<const Member*> MemberManager::findAdmins() const {
    std::vector<const Member*> results;

    for (const auto& member : members) {
        if (member.getAdmin()) {
            results.push_back(&member);
        }
    }
    return results;
}

// Authentication
Member* MemberManager::authenticateUser(const std::string& memberID, const std::string& password) {
    Member *toBeVerifiedMember = findMemberByID(memberID);

    if (toBeVerifiedMember == nullptr) {
        return nullptr;
    }

    if (auth::verifyPassword(password, toBeVerifiedMember->getPasswordHash())) {
        return toBeVerifiedMember;
    }
    return nullptr;
}

// Get all members
const std::vector<Member>& MemberManager::getAllMembers() const {
    return members;
}

// Get total number of members
int MemberManager::getTotalMembers() const {
    return static_cast<int>(members.size());
}

// Get count of admin members
int MemberManager::getAdminCount() const {
    int count = 0;
    for (const auto& member : members) {
        if (member.getAdmin()) {
            count++;
        }
    }
    return count;
}

// Reload from file
void MemberManager::reload() {
    loadFromFile();
}

// Clear file handler cache
void MemberManager::clearCache() {
    fileHandler.clearCache();
}

// Check if MemberID exists
bool MemberManager::isMemberIDExists(const std::string& memberID) const {
    return std::find_if(members.begin(), members.end(),
        [&](const Member& member) { return member.getMemberID() == memberID; }) != members.end();
}

// Batch Operations (RAII)
void MemberManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool MemberManager::isAutoSaveEnabled() const {
    return autoSave;
}

MemberManager::BatchOperation::BatchOperation(MemberManager& mmgr) :
                    memberManager(&mmgr), originalAutoSave(mmgr.autoSave), active(true) {
    mmgr.setAutoSave(false);
}

MemberManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    memberManager(other.memberManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

MemberManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        memberManager->saveToFile();
        memberManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "Error when trying to save members during batch operations." << std::endl;
    }
}

MemberManager::BatchOperation MemberManager::beginBatch() {
    return BatchOperation(*this);
}