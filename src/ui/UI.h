#ifndef LIBRARY_MANAGEMENT_SYSTEM_UI_H
#define LIBRARY_MANAGEMENT_SYSTEM_UI_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

class UI {
public:
    enum class DisplayMode {
        SIMPLE,         // Simple text-based menu
        ADVANCED,       // Box-drawing characters with boarders
    };

    enum class MessageType {
        INFO,           // Informational message
        SUCCESS,        // Success confirmation
        WARNING,        // Warning message
        ERR,            // Error message
    };

    enum class Alignment {      // Table alignment options
        LEFT,
        CENTER,
        RIGHT,
    };

    // Constructor
    UI(DisplayMode = DisplayMode::ADVANCED);        // Initial display mode: advanced

    // Display Mode Setter
    void setDisplayMode(DisplayMode mode);

    // Display Mode Getter
    DisplayMode getDisplayMode();

    // Toggle display mode between simple and advanced
    void toggleDisplayMode();

    // Clear screen
    void clearScreen();

    // pause and wait user to press any key
    void pause(const std::string &message = "Press any key to continue...") const;

    // Display the main application header
    void displayHeader(const std::string &title, const std::string &subtitle = "") const;

    // Display section header
    void displaySectionHeader(const std::string &sectionName) const;

    // Display Menu
    void displayMenu(const std::string &title, const std::vector<std::string> &options, bool showExit) const;

    // Display message
    void displayMessage(const std::string &message, MessageType type = MessageType::INFO) const;

    // Get integer input
    int getIntInput(const std::string& prompt, int max, int min) const;

    // Get string input
    std::string getStringInput(const std::string& prompt, bool allowEmpty = false) const;

    // Get password input
    std::string getPasswordInput(const std::string& prompt) const;

    // Get yes/no confirmation
    bool getConfirmation(const std::string &prompt) const;

    // Draw a horizontal line
    void drawLine(int width, char lineChar = '-') const;

    // Draw a box with content
    void drawBox(const std::string& content, int width = 0) const;

    // Draw a box with multiple content
    void drawBox(const std::vector<std::string>& lines, int width = 0) const;

    // Draw a progress bar
    void drawProgressBar(int current, int total, const std::string& label = "", int width = 50) const;

    // Display a loading message with animation
    void displayLoading(const std::string& message, int durationMs = 1000) const;

    // Display current date and time
    void displayDateTime() const;

    // Display a horizontal separator line
    void displaySeparator() const;

    // Center text within a given width
    std::string centerText(const std::string& content, int width) const;

    // Pad text to a specific width
    std::string padText(const std::string& content, int width, Alignment align = Alignment::LEFT) const;

    // Truncate text to fit within width
    std::string truncatedText(const std::string& content, int width, bool addEllipsis = true) const;

    // constants
    static constexpr int DEFAULT_WIDTH = 70;
    static constexpr int DEFAULT_PADDING = 2;

private:
    DisplayMode currentMode;

    // Box drawing characters for ADVANCED mode
    struct BoxChars {
        std::string topLeft;
        std::string topRight;
        std::string bottomLeft;
        std::string bottomRight;
        std::string horizontal;
        std::string vertical;
        std::string teeDown;
        std::string teeLeft;
        std::string teeRight;
        std::string teeUp;
        std::string cross;
    };

    // Get box characters based on current mode
    BoxChars getBoxChars() const;

    // Helper: get message prefix based on type
    std::string getMessagePrefix(MessageType type) const;

    // Helper: Convert string to uppercase
    std::string toUpper(const std::string& str) const;

    // Helper: Convert string to lowercase
    std::string toLower(const std::string& str) const;

    // Helper: Trim whitespace
    std::string trim(const std::string& str) const;
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_UI_H
