#ifndef LIBRARY_MANAGEMENT_SYSTEM_MEMBERMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_MEMBERMANAGER_H

#include "../models/Member.h"
#include "../utils/FileHandler.h"
#include <vector>
#include <string>

class MemberManager {
private:
    std::vector<Member> members;
    std::string filePath;
    FileHandler fileHandler;

    // Data Persistence
    // Helper: Load Members data from file
    void loadFromFile();

    // Helper: Save Members data to file
    void saveToFile();

    // For Batch Operations
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // Helper: Check autoSave flag to decide whether need to save
    void saveIfNeeded();

public:
    // Constructor
    explicit MemberManager(const std::string& filePath = "../data/members.csv");
    
    // CRUD (Create, Read, Update, Delete)
    bool addMember(const Member& member);
    bool updateMember(const Member& member);
    bool deleteMember(const std::string& memberID);
    
    // Search Functions
    Member* findMemberByID(const std::string &memberID);
    std::vector<const Member*> findByName(const std::string& name, int matchMode = 0) const;
    std::vector<const Member*> findByPhoneNumber(const std::string& phoneNumber, int matchMode = 0) const;
    std::vector<const Member*> findByRegistrationDate(const std::string& registrationDate, int matchMode = 0) const;
    std::vector<const Member*> findByExpiryDate(const std::string& expiryDate, int matchMode = 0) const;
    std::vector<const Member*> findAdmins() const;

    // Authentication
    Member* authenticateUser(const std::string& memberID, const std::string& password);

    // Getters
    const std::vector<Member> &getAllMembers() const;
    int getTotalMembers() const;
    int getAdminCount() const;

    // Utility
    void reload();          // Reload from file
    void clearCache();      // Clear file handler cache
    bool isMemberIDExists(const std::string& memberID) const;

    // Batch Operation (RAII)
    class BatchOperation {
    private:
        MemberManager* memberManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(MemberManager& mmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_MEMBERMANAGER_H