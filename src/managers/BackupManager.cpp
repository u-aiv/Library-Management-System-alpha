// BackupManager.cpp Implementation

#include "BackupManager.h"
#include "../config/Config.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace {
std::string joinPath(const std::string& dir, const std::string& sub) {
    if (dir.empty()) return sub;
    char last = dir[dir.size() - 1];
    if (last == '/' || last == '\\') {
        return dir + sub;
    }
    return dir + "/" + sub;
}

std::string getCwd() {
#ifdef _WIN32
    char buf[MAX_PATH];
    if (_getcwd(buf, MAX_PATH) == nullptr) {
        return ".";
    }
    return std::string(buf);
#else
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == nullptr) {
        return ".";
    }
    return std::string(buf);
#endif
}

std::string findDataRoot() {
    FileHandler fh;
    std::string dir = getCwd();
    for (int i = 0; i < 5; ++i) {
        std::string books = joinPath(dir, Config::BOOKS_FILE);
        std::string members = joinPath(dir, Config::MEMBERS_FILE);
        std::string trans = joinPath(dir, Config::TRANSACTIONS_FILE);
        std::string reserv = joinPath(dir, Config::RESERVATIONS_FILE);
        std::string settings = joinPath(dir, Config::SETTINGS_FILE);

        if (fh.isFileExist(books) &&
            fh.isFileExist(members) &&
            fh.isFileExist(trans) &&
            fh.isFileExist(reserv) &&
            fh.isFileExist(settings)) {
            return dir;
        }
        // Move up one directory
        std::string::size_type pos = dir.find_last_of("/\\");
        if (pos == std::string::npos) {
            break;
        }
        dir = dir.substr(0, pos);
    }
    return "";
}
}

BackupInfo::BackupInfo()
    : backupID(""),
      backupTime(""),
      description(""),
      isValid(false),
      backupPath("") {}

std::string BackupInfo::toString() const{
    std::ostringstream oss;
    oss << "Backup ID: " << backupID << "\n"
        << "Backup Time: " << backupTime << "\n"
        << "Description: " << description << "\n"
        << "Status: " << (isValid ? "Valid" : "Invalid") << "\n"
        << "Backup Path: " << backupPath << "\n";

    return oss.str();
}

std::string BackupInfo::toCSV() const {
    std::ostringstream oss;
    oss << backupID << ","
        << backupTime << ","
        << description << ","
        << (isValid ? "1" : "0") << ","
        << backupPath;

    return oss.str();
}

BackupInfo BackupInfo::fromCSV(const std::string& csvLine) {
    BackupInfo backupInfo;
    std::istringstream iss(csvLine);
    std::vector<std::string> fields;
    std::string field;

    while (std::getline(iss, field, ',')) {
        fields.push_back(field);
    }

    if (fields.size() == 5) {
        backupInfo.backupID = fields[0];
        backupInfo.backupTime = fields[1];
        backupInfo.description = fields[2];
        backupInfo.isValid = (fields[3] == "1");
        backupInfo.backupPath = fields[4];
    }

    return backupInfo;
}

BackupManager::BackupManager() {
    initPaths();
    createBackupDirectory();
    loadBackupManifest();
}

BackupManager::~BackupManager() {
    saveBackupManifest();
}


// Create a backup root directory (not existing)
bool BackupManager::initPaths() {
    baseDir = findDataRoot();
    if (baseDir.empty()) {
        return false;
    }
    backupRootDir = joinPath(baseDir, BACKUP_ROOT_DIR);
    return true;
}

bool BackupManager::createBackupDirectory() {
    try {
        FileHandler fileHandler;
        if (backupRootDir.empty()) {
            return false;
        }
        if (!fileHandler.isFileExist(backupRootDir)) {
            fileHandler.createDirectory(backupRootDir);
            std::cout << "Backup directory created: " << backupRootDir << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating backup directory " + std::string(e.what());
        return false;
    }
}

// Generate BackupID Based on timestamp
std::string BackupManager::generateBackupID() {
    time_t now = DateUtils::getCurrentTimestamp();
    std::tm *tm = std::localtime(&now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tm);
    return {buffer};
}

// Copy a single file
bool BackupManager::copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    try {
        std::ifstream ifs(sourcePath, std::ios::binary);
        if (!ifs.is_open()) {
            std::cerr << "Error opening source file " << sourcePath << std::endl;
            return false;
        }
        std::ofstream ofs(destinationPath, std::ios::binary);
        if (!ofs.is_open()) {
            ifs.close();
            std::cerr << "Error opening destination file " << destinationPath << std::endl;
            return false;
        }

        // Block I/O
        const int BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];

        while (ifs.read(buffer, BUFFER_SIZE) && ifs.gcount() > 0) {
            ofs.write(buffer, ifs.gcount());
        }

        ofs.close();
        ifs.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error copying file: " << destinationPath << std::endl;
        return false;
    }
}

// Load backup from metadata file
bool BackupManager::loadBackupManifest() {
    FileHandler fileHandler;
    if (backupRootDir.empty()) {
        return false;
    }
    std::string manifestPath = joinPath(backupRootDir, BACKUP_MANIFEST_FILE);
    backupInfoList.clear();         // Clear the list of backup information in memory
    if (!fileHandler.isFileExist(manifestPath)) {
        return true;                // Normal conditions on the first run
    }

    try {
        std::ifstream ifs(manifestPath);
        if (!ifs.is_open()) {
            std::cerr << "Error opening backup manifest file " << manifestPath << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) {
                continue;
            }

            BackupInfo backupInfo;
            backupInfo = backupInfo.fromCSV(line);
            backupInfo.isValid = isValidBackup(backupInfo.backupID);
            backupInfoList.emplace_back(backupInfo);
        }
        ifs.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading backup manifest: " << e.what() << std::endl;
        return false;
    }
}

// Save backup from metadata file
bool BackupManager::saveBackupManifest() {
    FileHandler fileHandler;
    if (backupRootDir.empty()) {
        return false;
    }
    std::string manifestPath = joinPath(backupRootDir, BACKUP_MANIFEST_FILE);
    try {
        std::ofstream ofs(manifestPath);
        if (!ofs.is_open()) {
            std::cerr << "Error opening backup manifest file " << manifestPath << std::endl;
            return false;
        }

        for (const auto& backupInfo : backupInfoList) {
            std::string csvLine = backupInfo.toCSV();
            ofs << csvLine << "\n";
        }
        ofs.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving backup manifest: " << e.what() << std::endl;
        return false;
    }
}

// Check backup integrity
bool BackupManager::isValidBackup(const std::string& backupID) {
    FileHandler fileHandler;
    if (backupRootDir.empty()) {
        return false;
    }
    std::string backupPath = joinPath(backupRootDir, backupID);
    if (!fileHandler.isFileExist(backupPath)) {
        std::cerr << "Error opening backup path directory " << backupPath << std::endl;
        return false;
    }

    for (const auto& datafile : DATA_FILES) {
        std::string datafilePath = backupPath + "/" + datafile;
        if (!fileHandler.isFileExist(datafilePath)) {
            std::cerr << "Error opening data file directory " << datafilePath << std::endl;
            return false;
        }
    }
    return true;
}

// Full Data Backup
bool BackupManager::backupData(const std::string& backupDescription) {
    if (backupRootDir.empty() && !initPaths()) {
        std::cerr << "Backup failed: data directory not found." << std::endl;
        return false;
    }
    std::string backupID = generateBackupID();
    std::string backupPath = joinPath(backupRootDir, backupID);
    try {
        FileHandler fileHandler;
        if (fileHandler.isFileExist(backupPath)) {
            std::cerr << "Backup already exists: " << backupPath << std::endl;
            return false;
        }

        if (!fileHandler.createDirectory(backupPath)) {
            std::cerr << "Failed to create backup directory: " << backupPath << std::endl;
            return false;
        }
        bool allCopied = true;
        for (const auto& datafile : DATA_FILES) {
            std::string sourcePath = joinPath(baseDir, Config::DATA_DIR + datafile);
            std::string destinationPath = joinPath(backupPath, datafile);
            if (!fileHandler.isFileExist(sourcePath)) {
                std::cerr << "Error opening source directory " << sourcePath << std::endl;
                allCopied = false;
                continue;
            }
            if (!copyFile(sourcePath, destinationPath)) {
                std::cerr << "Error copying files " << sourcePath << std::endl;
                allCopied = false;
            }
        }
        if (!allCopied) {
            std::cerr << "Backup failed: one or more source files missing or copy failed." << std::endl;
            return false;
        }

        BackupInfo backupInfo;
        backupInfo.backupID = backupID;
        backupInfo.backupTime = DateUtils::getCurrentDateTime();
        backupInfo.description = backupDescription;
        backupInfo.isValid = true;
        backupInfo.backupPath = backupPath;

        backupInfoList.emplace_back(backupInfo);

        if (!saveBackupManifest()) {
            std::cerr << "Error saving backup manifest file " << backupPath << std::endl;
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading backup manifest: " << e.what() << std::endl;
        return false;
    }
}

// Restore backup
bool BackupManager::restoreData(const std::string& backupId) {
    if (backupRootDir.empty() && !initPaths()) {
        std::cerr << "Restore failed: data directory not found." << std::endl;
        return false;
    }
    std::string backupPath = joinPath(backupRootDir, backupId);
    FileHandler fileHandler;

    try {
        if (!fileHandler.isFileExist(backupPath)) {
            std::cerr << "Error opening backup path directory " << backupPath << std::endl;
            return false;
        }
        if (!isValidBackup(backupId)) {
            std::cerr << "Backup ID is INVALID: " << backupId << std::endl;
            return false;
        }

        for (const auto& datafile : DATA_FILES) {
            std::string sourcePath = joinPath(backupPath, datafile);
            std::string destinationPath = joinPath(baseDir, Config::DATA_DIR + datafile);
            if (!fileHandler.isFileExist(sourcePath)) {
                std::cerr << "Error opening source directory " << sourcePath << std::endl;
                continue;
            }
            if (!copyFile(sourcePath, destinationPath)) {
                std::cerr << "Error copying backup manifest file " << sourcePath << std::endl;
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error restoring data: " << e.what() << std::endl;
        return false;
    }
}

// Get available backup ID
std::vector<std::string> BackupManager::listBackupIds() {
    std::vector<std::string> backupIds;

    backupIds.reserve(backupInfoList.size());
    for (const auto& backupInfo : backupInfoList) {
        backupIds.emplace_back(backupInfo.backupID);
    }
    return backupIds;
}

// Get all backup details
std::vector<BackupInfo> BackupManager::listBackups() {
    return backupInfoList;
}

// Backup Details for ID
BackupInfo BackupManager::getBackupInfo(const std::string& backupId) {
    for (const auto& backupInfo : backupInfoList) {
        if (backupInfo.backupID == backupId) {
            return backupInfo;
        }
    }
    return {};      // if not found
}

// Get the latest backup
BackupInfo BackupManager::getLatestBackup() {
    if (backupInfoList.empty()) {
        return {};
    }
    return backupInfoList.back();
}

// Check if has valid updates
bool BackupManager::hasValidBackups() {
    for (const auto& backupInfo : backupInfoList) {
        if (backupInfo.isValid) {
            return true;
        }
    }
    return false;
}

// Automatically clean up expired backups (Keep the latest N backups)
bool BackupManager::autoCleanOldBackups(int keepCount) {
    try {
        while (static_cast<int>(backupInfoList.size()) > keepCount) {
            backupInfoList.erase(backupInfoList.begin());
        }
        saveBackupManifest();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error auto cleaning old backups: " << e.what() << std::endl;
        return false;
    }
}
