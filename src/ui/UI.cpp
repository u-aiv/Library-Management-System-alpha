#include "UI.h"
#include "../utils/DateUtils.h"
#include <algorithm>
#include <sstream>
#include <chrono>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <thread>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

// Constructor
UI::UI(DisplayMode mode) : currentMode(mode) {}

// Display Mode Setter
void UI::setDisplayMode(DisplayMode mode) {
    currentMode = mode;
}

// Display Mode Getter
UI::DisplayMode UI::getDisplayMode() {
    return currentMode;
}

// Toggle display mode between simple and advanced
void UI::toggleDisplayMode() {
    currentMode = (currentMode == DisplayMode::SIMPLE) ? DisplayMode::ADVANCED : DisplayMode::SIMPLE;
}

// Clear screen
void UI::clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// pause and wait user to press any key
void UI::pause(const std::string &message) const {
    std::cout << "\n" << message;
    #ifdef _WIN32
        _getch();
    #else
        struct termios oldSettings, newSettings;
        tcgetattr(STDIN_FILENO, &oldSettings);
        newSettings = oldSettings;
        newSettings.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
        getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
#endif
    std::cout << std::endl;
}

// Get box characters based on current mode
UI::BoxChars UI::getBoxChars() const {
    if (currentMode == DisplayMode::ADVANCED) {
        return {
            // UTF-8 box drawing characters
            "\xE2\x95\x94",  // ╔ top-left
            "\xE2\x95\x97",  // ╗ top-right
            "\xE2\x95\x9A",  // ╚ bottom-left
            "\xE2\x95\x9D",  // ╝ bottom-right
            "\xE2\x95\x90",  // ═ horizontal
            "\xE2\x95\x91",  // ║ vertical
            "\xE2\x95\xA6",  // ╦ tee-down
            "\xE2\x95\xA9",  // ╩ tee-up
            "\xE2\x95\xA0",  // ╠ tee-right
            "\xE2\x95\xA3",  // ╣ tee-left
            "\xE2\x95\xAC"   // ╬ cross
            };
        } else {
            return {
            "+",  // top-left
            "+",  // top-right
            "+",  // bottom-left
            "+",  // bottom-right
            "-",  // horizontal
            "|",  // vertical
            "+",  // tee-down
            "+",  // tee-up
            "+",  // tee-right
            "+",  // tee-left
            "+"   // cross
            };
    }
}

// Display the main application header
void UI::displayHeader(const std::string &title, const std::string &subtitle) const {
    BoxChars boxChars = getBoxChars();
    int width = DEFAULT_WIDTH;

    // Top border
    std::cout << boxChars.topLeft;
    for (size_t i = 1; i < width - 1; i++) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.topRight << std::endl;

    // Title
    std::cout << boxChars.vertical << centerText(title, width - 2) << boxChars.vertical << std::endl;

    // Subtitle (if provided)
    if (!subtitle.empty()) {
        std::cout << boxChars.vertical << centerText(subtitle, width - 2) << boxChars.vertical << std::endl;
    }

    // Bottom border
    std::cout << boxChars.bottomLeft;
    for (size_t i = 1; i < width - 1; i++) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.bottomRight << std::endl;
}

// Display section header
void UI::displaySectionHeader(const std::string &sectionName) const {
    std::cout << '\n';
    if (currentMode == DisplayMode::ADVANCED) {
        BoxChars boxChars = getBoxChars();
        std::cout << boxChars.teeDown << " " << sectionName << " ";
        for (size_t i = 0; i < DEFAULT_WIDTH - sectionName.length(); i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << std::endl;
    } else {
        std::cout << "=== " << sectionName << " ===" << std::endl;
    }
    std::cout << std::endl;
}

// Display Menu
void UI::displayMenu(const std::string &title, const std::vector<std::string> &options, bool showExit) const {
    if (currentMode == DisplayMode::ADVANCED) {
        BoxChars boxChars = getBoxChars();
        int width = DEFAULT_WIDTH;

        // Top border
        std::cout << boxChars.topLeft;
        for (size_t i = 1; i < width - 1; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.topRight << std::endl;

        // Title
        std::cout << boxChars.vertical << centerText(title, width - 2) << std::endl;

        // Middle separator
        std::cout << boxChars.teeLeft;
        for (size_t i = 0; i < width - 2; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.teeRight << std::endl;

        // Menu Options
        for (size_t i = 0; i < options.size(); i++) {
            std::ostringstream oss;
            oss << std::setw(3) << std::right << (i + 1) << ". " << options[i] << std::endl;
            std::cout << boxChars.vertical << " " << std::left << std::setw(width - 3) << oss.str() << boxChars.vertical <<std::endl;
        }
        std::cout << std::endl;

        // Exit option for advanced mode
        if (showExit) {
            std::ostringstream oss;
            oss << std::setw(3) << std::right << "0" << ". " << "Exit/Back";
            std::cout << boxChars.vertical << " " << std::left << std::setw(width - 4)
                      << oss.str() << " " << boxChars.vertical << std::endl;
        }

        // Bottom border
        std::cout << boxChars.bottomLeft;
        for (size_t i = 0; i < width - 2; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.bottomRight << std::endl;
    } else {
        // Simple mode
        std::cout << "\n" << title << "\n";
        for (size_t i = 0; i < title.length(); i++) {
            std::cout << "=";
        }
        std::cout << "\n\n";

        for (size_t i = 0; i < options.size(); i++) {
            std::cout << std::setw(3) << std::right << (i + 1) << ". " << options[i];
        }

        // Exit option for simple mode
        if (showExit) {
            std::cout << std::setw(3) << std::right << "0" << ". " << "Exit/Back" << std::endl;
        }
        std::cout << std::endl;
    }
}

// Message Display
// Helper: get message prefix based on type
std::string UI::getMessagePrefix(MessageType type) const {
    switch (type) {
        case MessageType::SUCCESS:
            return "[SUCCESS]";
        case MessageType::WARNING:
            return "[WARNING]";
        case MessageType::ERR:
            return "[ERROR]";
        case MessageType::INFO:
        default:
            return "[INFO]";
    }
}

// Message Display
void UI::displayMessage(const std::string &message, MessageType type) const {
    std::string prefix = getMessagePrefix(type);

    if (currentMode == DisplayMode::ADVANCED) {
        BoxChars boxChars = getBoxChars();
        int width = DEFAULT_WIDTH;

        std::cout << "\n" << boxChars.topLeft;
        for (size_t i = 1; i < width - 1; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.topRight << std::endl;

        std::cout << boxChars.vertical << " " << std::left
                  << std::setw(width - 3) << (prefix + message) << boxChars.vertical << std::endl;

        std::cout << boxChars.bottomLeft;
        for (size_t i = 0; i < width - 2; i++) {
            std::cout << boxChars.horizontal;
        }
        std::cout << boxChars.bottomRight << std::endl;
    } else {
        std::cout << "\n" << (prefix + message) << "\n" << std::endl;
    }
}

// Input Functions
// Get integer input
int UI::getIntInput(const std::string &prompt, int max, int min) const {
    int value;
    bool valid = false;

    while (!valid) {
        std::cout << prompt << ": ";
        if (std::cin >> value) {
            if (value >= min && value <= max) {
                valid = true;
            } else {
                displayMessage("Please enter a number between " +
                    std::to_string(min) + " and " + std::to_string(max),
                    MessageType::WARNING);
            }
        } else {
            displayMessage("Invalid input. Please enter a number.", MessageType::ERR);
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
    }
    std::cin.ignore(10000, '\n');       // Clear trailing newline
    return value;
}

// Get string input
std::string UI::getStringInput(const std::string& prompt, bool allowEmpty) const {
    std::string input;
    bool valid = false;

    while (!valid) {
        std::cout << prompt << ": ";
        std::getline(std::cin, input);

        input = trim(input);

        if (!input.empty() || allowEmpty) {
            valid = true;
        } else {
            displayMessage("Please enter a valid input.", MessageType::ERR);
        }
    }
    return input;
}

// Get password input
std::string UI::getPasswordInput(const std::string& prompt) const {
    std::cout << prompt << ": ";
    std::string password;

    #ifdef _WIN32
        char ch;
        while ((ch = _getch()) != '\r') {  // Enter key
            if (ch == '\b') {  // Backspace
                if (!password.empty()) {
                    password.pop_back();
                    std::cout << "\b \b";
                }
            } else {
                password.push_back(ch);
                std::cout << '*';
            }
        }
    #else
        struct termios oldSettings, newSettings;
        tcgetattr(STDIN_FILENO, &oldSettings);
        newSettings = oldSettings;
        newSettings.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

        std::getline(std::cin, password);

        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    #endif

    std::cout << std::endl;
    return password;
}

// Get yes/no confirmation
bool UI::getConfirmation(const std::string &prompt) const {
    std::string input;
    while (true) {
        std::cout << prompt << " (y/n) ";
        std::getline(std::cin, input);

        input = trim(toLower(input));

        if (input == "y" || input == "yes") {
            return true;
        } else if (input == "n" || input == "no") {
            return false;
        } else {
            displayMessage("Please enter 'y' or 'n'.", MessageType::WARNING);
        }
    }
}

// Box Drawing Functions
// Draw a horizontal line
void UI::drawLine(int width, char lineChar) const {
    for (size_t i = 0; i < width; i++) {
        std::cout << lineChar;
    }
    std::cout << std::endl;
}

// Draw a box with content
void UI::drawBox(const std::string &content, int width) const {
    drawBox(std::vector<std::string>{content}, width);
}

// Draw a box with multiple content
void UI::drawBox(const std::vector<std::string> &lines, int width) const {
    BoxChars boxChars = getBoxChars();

    // Automatically calculate width
    if (width == 0) {
        for (const auto& line : lines) {
            width = std::max(width, static_cast<int>(line.length()) + 4);
        }
    }

    // Top border
    std::cout << boxChars.topLeft;
    for (size_t i = 1; i < width - 1; ++i) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.topRight << std::endl;

    // Content
    for (const auto& line : lines) {
        std::cout << boxChars.vertical << " " << std::left << std::setw(width - 3)
                 << line << boxChars.vertical << std::endl;
    }

    // Bottom border
    std::cout << boxChars.bottomLeft;
    for (size_t i = 1; i < width - 1; ++i) {
        std::cout << boxChars.horizontal;
    }
    std::cout << boxChars.bottomRight << std::endl;
}

// Draw a progress bar
void UI::drawProgressBar(int current, int total, const std::string& label, int width) const {
    double percentage = (total > 0) ? static_cast<double>(current) / total * 100.0 : 0.0;
    int filled = static_cast<int>(width * current / total);

    std::cout << "\r";
    if (!label.empty()) {
        std::cout << label << " ";
    }

    std::cout << "[";
    for (size_t i = 0; i < width; ++i) {
        if (i < filled) {
            std::cout << '=';
        } else if (i == filled) {
            std::cout << '>';
        } else {
            std::cout << ' ';
        }
    }
    std::cout << ']' << std::fixed << std::setprecision(1) << percentage << std::endl;
    std::cout.flush();

    if (current >= total) {
        std::cout << std::endl;
    }
}

// Display a loading message with animation
void UI::displayLoading(const std::string& message, int durationMs) const {
    const char spinChars[] = {'|', '/', '-', '\\'};
    int iterations = durationMs / 100;

    for (size_t i = 0; i < iterations; ++i) {
        std::cout << "\r" << message << " " << spinChars[i % 4] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\r" << message << "Done." << std::endl;
}

// Display current date and time
void UI::displayDateTime() const {
    std::cout << DateUtils::getCurrentDateTime() << std::endl;
}

// Display a horizontal separator line
void UI::displaySeparator() const {
    drawLine(DEFAULT_WIDTH, '-');
}

// Center text within a given width
std::string UI::centerText(const std::string& content, int width) const {
    int padding = (width - static_cast<int>(content.length())) / 2;
    if (padding < 0) {
        padding = 0;
    }

    std::string result(padding, ' ');
    result += content;
    result.resize(width, ' ');

    return result;
}

// Pad text to a specific width
std::string UI::padText(const std::string& content, int width, Alignment align) const {
    if (static_cast<int>(content.length()) >= width) {
        return content.substr(0, width);
    }

    std::string result;
    int padding = width - static_cast<int>(content.length());

    switch (align) {
        case Alignment::LEFT:
            result = content + std::string(padding, ' ');
            break;
        case Alignment::RIGHT:
            result = std::string(padding, ' ') + content;
            break;
        default:
            result = centerText(content, width);
            break;
    }

    return result;
}

// Truncate text to fit within width
std::string UI::truncatedText(const std::string& content, int width, bool addEllipsis) const {
    if (static_cast<int>(content.length()) <= width) {
        return content;
    }

    if (addEllipsis && width > 3) {
        return content.substr(0, width - 3) + "...";
    }

    return content.substr(0, width);
}

// Helper: Convert string to uppercase
std::string UI::toUpper(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// Helper: Convert string to lowercase
std::string UI::toLower(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Helper: Trim whitespace
std::string UI::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}
