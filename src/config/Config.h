#ifndef LIBRARY_MANAGEMENT_SYSTEM_CONFIG_H
#define LIBRARY_MANAGEMENT_SYSTEM_CONFIG_H

#include <string>
#include <unordered_map>

class Config {
public:
    // Singleton pattern
    static Config& getInstance();

    // Copy Denied
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // File path
    static const std::string DATA_DIR;
    static const std::string BOOKS_FILE;
    static const std::string MEMBERS_FILE;
    static const std::string TRANSACTIONS_FILE;
    static const std::string RESERVATIONS_FILE;
    static const std::string SETTINGS_FILE;
    static const std::string REPORTS_DIR;

    // Book genres
    static const std::string GENRES[5];
    static constexpr int GENRES_COUNT = 5;

    // Borrowing policies
    static constexpr int DEFAULT_BORROW_DAYS = 14;
    static constexpr int RENEWAL_DAYS = 7;
    static constexpr int MAX_RENEWALS = 2;      // Max 30 days total (14 + 7 + 7)
    static constexpr int MAX_TOTAL_BORROW_DAYS = 30;

    // Member settings
    static constexpr int DEFAULT_MAX_BOOKS = 2;
    static constexpr int ADMIN_MAX_RENEWALS = 10;
    static constexpr int MEMBERSHIP_DURATION_DAYS = 365 * 4;  // 4 years
    static constexpr int MIN_MAX_BOOKS = 1;
    static constexpr int MAX_MAX_BOOKS = 10;

    // Fine settings
    static const double FINE_PER_DAY;
    static const double MAX_FINE;

    // Validation rules
    static constexpr int ISBN_LENGTH = 13;
    static constexpr int PHONE_LENGTH = 10;
    static constexpr int MIN_TITLE_LENGTH = 1;
    static constexpr int MAX_TITLE_LENGTH = 200;
    static constexpr int MIN_NAME_LENGTH = 1;
    static constexpr int MAX_NAME_LENGTH = 100;
    static constexpr int MIN_AUTHOR_LENGTH = 1;
    static constexpr int MAX_AUTHOR_LENGTH = 100;
    static constexpr int MIN_PUBLISHER_LENGTH = 1;
    static constexpr int MAX_PUBLISHER_LENGTH = 100;

    // ID prefixes
    static const std::string MEMBER_ID_PREFIX;
    static const std::string ADMIN_ID_PREFIX;
    static const std::string TRANSACTION_ID_PREFIX;
    static const std::string RESERVATION_ID_PREFIX;

    // ID formatting
    static constexpr int MEMBER_ID_NUMBER_WIDTH = 8;  // e.g. M20261001
    static constexpr int TRANSACTION_ID_NUMBER_WIDTH = 10;  // e.g. T2026100001
    static constexpr int RESERVATION_ID_NUMBER_WIDTH = 10;  // e.g. R2026100001

    // Date format
    static const std::string DATE_FORMAT;  // YYYY-MM-DD

    // CSV settings
    static constexpr char CSV_DELIMITER = ',';
    static constexpr char CSV_LIST_DELIMITER = ';';  // For lists within a field

    // UI settings
    static constexpr int UI_BOX_WIDTH = 50;
    static constexpr char UI_HORIZONTAL_CHAR = '=';
    static constexpr char UI_VERTICAL_CHAR = '|';
    static constexpr char UI_CORNER_CHAR = '+';

    // Default password for new members (should be changed on first login)
    static const std::string DEFAULT_PASSWORD;

    // System messages
    static const std::string SUCCESS_MESSAGE_PREFIX;
    static const std::string ERROR_MESSAGE_PREFIX;
    static const std::string WARNING_MESSAGE_PREFIX;
    static const std::string INFO_MESSAGE_PREFIX;

    // Load and save settings
    void loadSettings();
    void saveSettings();

    // Getters for configurable settings (can be changed at runtime)
    bool isAdvancedUIMode() const;
    int getBorrowPeriodDays() const;
    double getFinePerDay() const;
    double getMaxFine() const;
    int getDefaultMaxBooks() const;

    // Setters for configurable settings
    void setAdvancedUIMode(bool enabled);
    void setBorrowPeriodDays(int days);
    void setFinePerDay(double fine);
    void setMaxFine(double fine);
    void setDefaultMaxBooks(int maxBooks);

    // Utility methods
    static bool isValidGenre(const std::string& genre);
    static std::string formatMemberID(int number);
    static std::string formatTransactionID(int number);
    static std::string formatReservationID(int number);

private:
    // Private constructor for singleton
    Config();

    // Runtime configurable settings
    bool advancedUIMode;
    int borrowPeriodDays;
    double finePerDay;
    double maxFine;
    int defaultMaxBooks;

    // Settings storage
    std::unordered_map<std::string, std::string> settings;

    // Helper methods
    void initializeDefaults();

    void parseSettingsLine(const std::string &line);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_CONFIG_H
