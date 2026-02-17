#ifndef LIBRARY_MANAGEMENT_SYSTEM_FILEHANDLER_H
#define LIBRARY_MANAGEMENT_SYSTEM_FILEHANDLER_H

#include <fstream>
#include <string>
#include <vector>
#include <map>

class FileHandler {
private:
    std::map<std::string, std::vector<std::string>> cache;

public:
    // Constructor
    FileHandler() = default;

    // Methods
    std::vector<std::string> readCSV(const std::string& filePath);
    void writeCSV(const std::string& filePath, const std::vector<std::string>& lines);
    bool isFileExist(const std::string& filePath);
    void createFileIfNotExist(const std::string& filePath);
    void clearCache();
    void clearCache(const std::string& filePath);

    bool createDirectory(const std::string& filePath);                                      // make directory
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_FILEHANDLER_H