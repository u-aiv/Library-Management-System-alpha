// DateUtils.h Header

#ifndef LIBRARY_MANAGEMENT_SYSTEM_DATEUTILS_H
#define LIBRARY_MANAGEMENT_SYSTEM_DATEUTILS_H

#include <string>
#include <ctime>

namespace DateUtils{
// Convert YYYY-MM-DD to UNIX timestamp
time_t dateToTimestamp(const std::string& dateString);

// Convert UNIX timestamp to YYYY-MM-DD
std::string timestampToDate(time_t timestamp);

// Get current date as YYYY-MM-DD
std::string getCurrentDate();

// Get current precise time as YYYY-MM-DD h:m:s
std::string getCurrentDateTime();

// Get current UNIX timestamp
time_t getCurrentTimestamp();

// Calculate the days interval
unsigned int daysBetween(time_t startTime, time_t endTime);

// Add days
std::string addDays(const std::string& dateString, int addition);
}

#endif //LIBRARY_MANAGEMENT_SYSTEM_DATEUTILS_H