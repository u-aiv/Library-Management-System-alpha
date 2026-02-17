// DateUtils.h Instance

#include "DateUtils.h"
#include <ctime>
#include <string>
#include <sstream>

namespace DateUtils {
    time_t dateToTimestamp(const std::string& dateString) {
        int year = 0;
        int month = 0;
        int day = 0;

        if (std::sscanf(dateString.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
            return 0;                       // Returns 0 if parsing fails, and the caller will get 1970-01-01
        }
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_isdst = -1;

        return mktime(&tm);
    }

    std::string timestampToDate(time_t timestamp) {
        std::tm *tm = std::localtime(&timestamp);
        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);

        return buffer;
    }

    std::string getCurrentDate() {
        return timestampToDate(std::time(nullptr));
        // return timestampToDate(time(0));
    }

    std::string getCurrentDateTime() {
        time_t timestamp = std::time(nullptr);
        std::tm *tm = std::localtime(&timestamp);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);

        return buffer;
    }

    time_t getCurrentTimestamp() {
        return std::time(nullptr);
        // return time(0);
    }

    unsigned int daysBetween(time_t startTimestamp, time_t endTimestamp) {
        time_t incrementTimestamp = endTimestamp - startTimestamp;
        // The maximum year corresponding to INT is 2038, while
        // the maximum year corresponding to UNSIGNED INT is 2106.
        return static_cast<unsigned int>(incrementTimestamp) / 86400;
    }

    std::string addDays(const std::string& dateString, int addition) {
        time_t dateTimestamp = dateToTimestamp(dateString);
        time_t targetTimestamp = dateTimestamp + (addition * 86400);
        return timestampToDate(targetTimestamp);
    }
}
