#ifndef LIBRARY_MANAGEMENT_SYSTEM_BACKUPMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_BACKUPMANAGER_H

#include "../utils/DateUtils.h"
#include "../utils/FileHandler.h"


// Record backup metadata
class BackupInfo {
public:
    std::string backupID;               // Backup ID (generated from the timestamp)
    std::string backupTime;             // Backup creation time (YYYY-MM-DD HH:MM:SS format)
    std::string description;            // Description or annotation information
    bool isValid;                       // Backup Integrity
    std::string backupPath;             // Full backup path

    BackupInfo();
    ~BackupInfo() = default;
    std::string toString() const;
    std::string toCSV() const;
    BackupInfo fromCSV(const std::string& csvLine);
};

// Backup and Restore Operations
class BackupManager {
private:
    const std::string BACKUP_ROOT_DIR = "data/backup";                  // Backup root directory path
    const std::string BACKUP_MANIFEST_FILE = "backup_manifest.txt";     // Backup Metadata Filename
    const std::vector<std::string> DATA_FILES =                         // Data needed to back up
        {
        "books.csv",
        "members.csv",
        "transactions.csv",
        "reservations.csv",
        "settings.csv"
    };
    FileHandler filehandler;
    std::vector<BackupInfo> backupInfoList;                             // All backup information
    std::string baseDir;                                                // Project root that contains data/
    std::string backupRootDir;                                          // Absolute backup root path

    bool createBackupDirectory();                                                           // Create a backup root directory (not existing)
    bool initPaths();                                                                       // Resolve baseDir / backupRootDir
    std::string generateBackupID();                                                         // Based on timestamp
    bool copyFile(const std::string& sourcePath, const std::string& destinationPath);       // Copy a single file
    bool loadBackupManifest();                                                              // Load backup from metadata file
    bool saveBackupManifest();                                                              // Save backup from metadata file
    bool isValidBackup(const std::string& backupID);                                        // Check backup integrity

public:
    BackupManager();
    ~BackupManager();
    bool backupData(const std::string& backupDescription = "Manual Backup");            // Full Data Backup
    bool restoreData(const std::string& backupId);                                         // Restore backup
    std::vector<std::string> listBackupIds();                                              // Get available backup ID
    std::vector<BackupInfo> listBackups();                                                 // Get all backup details
    BackupInfo getBackupInfo(const std::string& backupId);                                 // Backup Details for ID
    BackupInfo getLatestBackup();                                                          // Get the latest backup
    bool hasValidBackups();                                                                // Check if has valid updates
    bool autoCleanOldBackups(int keepCount = 5);                                           // Automatically clean up expired backups (Keep the latest N backups)
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_BACKUPMANAGER_H
