// MenuHandler.cpp Implementation

#include "MenuHandler.h"
#include "../managers/BookManager.h"
#include "../managers/MemberManager.h"
#include "../managers/TransactionManager.h"
#include "../managers/ReservationManager.h"
#include "../managers/RecommendationManager.h"
#include "../managers/BackupManager.h"
#include "../managers/ReportManager.h"
#include "../models/Book.h"
#include "../models/Member.h"
#include "../authentication/auth.h"
#include "../config/Config.h"
#include "UI.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <sstream>
#include <map>

// Constructor
MenuHandler::MenuHandler(BookManager &bm, MemberManager &mm,
    TransactionManager &tm, ReservationManager& rsm, RecommendationManager& rcm, UI &ui)
    : bookManager(bm), memberManager(mm), transactionManager(tm), reservationManager(rsm), recommendationManager(rcm), ui(ui),
      currentUser(nullptr), isRunning(false) {}

// Destructor
MenuHandler::~MenuHandler() {
    if (currentUser != nullptr) {
        logout();
    }
}

// Public Methods

bool MenuHandler::login() {
    clearScreen();
    displayWelcomeScreen();

    std::string memberID = promptForInput("Enter member ID ('q' to quit): ", false);
    if (memberID.empty() || memberID == "q" || memberID == "Q") {
        return false;
    }

    std::string password = promptForInput("Enter password: ", false);
    if (password.empty()) {
        return false;
    }

    currentUser = memberManager.authenticateUser(memberID, password);

    if (currentUser == nullptr) {
        displayMessage("Invalid credentials. Please try again!", "error");
        pauseScreen();
        return false;
    }

    if (currentUser->isExpired()) {
        displayMessage("Your account is expired. Please contact the administrator.", "error");
        currentUser = nullptr;
        pauseScreen();
        return false;
    }

    displayMessage("Logged in. Welcome, " + currentUser->getName() + " !", "success");
    return true;
}

bool MenuHandler::logout() {
    if (currentUser != nullptr) {
        displayMessage("Logging out... Goodbye, " + currentUser->getName() + " !", "info");
        currentUser = nullptr;
        pauseScreen();
        return true;
    }
    return false;
}

void MenuHandler::run() {
    isRunning = true;

    while (isRunning) {
        if (currentUser == nullptr) {
            if (!login()) {
                isRunning = false;
                break;
            }
        }
        clearScreen();
        if (currentUser->getAdmin()) {
            displayAdminMenu();
        } else {
            displayMemberMenu();
        }
    }
}

void MenuHandler::exit() {
    isRunning = false;
}

bool MenuHandler::isApplicationRunning() const {
    return isRunning;
}

Member* MenuHandler::getCurrentUser() const {
    return currentUser;
}

// Menu Display Methods

void MenuHandler::displayWelcomeScreen() {
    ui.displayHeader("Library Management System");
    std::cout << "\n";
    std::cout << "  ╔════════════════════════════════════════════╗\n";
    std::cout << "  ║     WELCOME TO THE LIBRARY SYSTEM          ║\n";
    std::cout << "  ║                                            ║\n";
    std::cout << "  ║  Your gateway to knowledge and learning    ║\n";
    std::cout << "  ╚════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void MenuHandler::displayMemberMenu() {
    ui.displayHeader("Member Menu: " + currentUser->getName());

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  BOOK OPERATIONS                        │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  1. Search Books                        │\n";
    std::cout << "│  2. Borrow Book                         │\n";
    std::cout << "│  3. Return Book                         │\n";
    std::cout << "│  4. Renew Book                          │\n";
    std::cout << "│  5. Reserve Book                        │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  ACCOUNT INFORMATION                    │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  6. View Borrowed Books                 │\n";
    std::cout << "│  7. View Borrowing History              │\n";
    std::cout << "│  8. View Reservations                   │\n";
    std::cout << "│  9. View Recommendations                │\n";
    std::cout << "│ 10. View Profile                        │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  0. Logout                              │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Please enter you choice: ", 0, 11);
    switch (choice) {
        case 1: handleSearchBooks(); break;
        case 2: handleBorrowBook(); break;
        case 3: handleReturnBook(); break;
        case 4: handleRenewBook(); break;
        case 5: handleReserveBook(); break;
        case 6: handleViewBorrowedBooks(); break;
        case 7: handleViewBorrowingHistory(); break;
        case 8: handleViewReservations(); break;
        case 9: handleViewRecommendations(); break;
        case 10: handleViewProfile(); break;
        case 0: logout(); break;
        default: {
            displayMessage("Invalid choice. Please try again.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::displayAdminMenu() {
    ui.displayHeader("Admin Menu: " + currentUser->getName());

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│             ADMINISTRATION              │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  1. Manage Books                        │\n";
    std::cout << "│  2. Manage Members                      │\n";
    std::cout << "│  3. Manage Transactions                 │\n";
    std::cout << "│  4. View Overdue Books                  │\n";
    std::cout << "│  5. Generate Reports                    │\n";
    std::cout << "│  6. Backup & Restore                    │\n";
    std::cout << "│  7. System Settings                     │\n";
    std::cout << "├─────────────────────────────────────────┤\n";
    std::cout << "│  0. Logout                              │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Please enter your choice: ", 0, 7);
    switch (choice) {
        case 1: handleManageBooks(); break;
        case 2: handleManageMembers(); break;
        case 3: handleManageTransactions(); break;
        case 4: handleViewOverdueBooks(); break;
        case 5: handleGenerateReports(); break;
        case 6: handleBackupRestore(); break;
        case 7: handleSystemSettings(); break;
        case 0: logout(); break;
        default: {
            displayMessage("Invalid choice. Please try again.", "error");
            pauseScreen();
            break;
        }
    }
}

// Menu Handlers for MEMBER

void MenuHandler::handleSearchBooks() {
    clearScreen();
    ui.displayHeader("Search Books: ");

    std::cout << "\n";
    std::cout << "Search by:\n";
    std::cout << "  1. Title\n";
    std::cout << "  2. Author\n";
    std::cout << "  3. ISBN\n";
    std::cout << "  4. Category\n";
    std::cout << "  5. Publisher\n";
    std::cout << "  0. Back\n\n";

    int choice = promptForInt("Please enter your choice: ", 0, 5);
    switch (choice) {
        case 1: performBookSearch("title"); break;
        case 2: performBookSearch("author"); break;
        case 3: performBookSearch("isbn"); break;
        case 4: performBookSearch("genre"); break;
        case 5: performBookSearch("publisher"); break;
        case 0: return;
        default: {
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleBorrowBook() {
    clearScreen();
    ui.displayHeader("Borrow Books: ");

    std::string isbn = promptForInput("Enter ISBN of the book to borrow: ");
    if (isbn.empty()) {
        return;
    }

    Book* book = bookManager.findBookByISBN(isbn);
    if (!book) {
        displayMessage("Book not found. Please try again.", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    if (!confirmAction("Do you want to borrow this book?")) {
        return;
    }

    if (transactionManager.borrowBook(memberManager, bookManager, currentUser->getMemberID(), isbn) != "0") {
        displayMessage("Book borrowed successfully!", "success");
    } else {
        displayMessage("Failed to borrow book. Please check availability or your borrowing limit.", "error");
    }
    pauseScreen();
}

void MenuHandler::handleReturnBook() {
    clearScreen();
    ui.displayHeader("Return Books: ");

    // Display current borrowed books
    auto borrowedBook = transactionManager.getActiveTransactions(currentUser->getMemberID());

    if (borrowedBook.empty()) {
        displayMessage("You have no books to return.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\nYour borrowed books:\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Due Date" << "\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& transaction : borrowedBook) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        if (book) {
            std::cout << std::left << std::setw(15) << transaction->getISBN()
                      << std::setw(35) << book->getTitle()
                      << std::setw(15) << transaction->getBorrowDate()
                      << std::setw(15) << transaction->getDueDate() << "\n";
        }
    }
    std::cout << std::string(80, '-') << "\n\n";

    std::string isbn = promptForInput("Enter ISBN of the book to return: ");
    if (isbn.empty()) {
        return;
    }

    if (transactionManager.returnBook(bookManager, currentUser->getMemberID(), isbn)) {
        displayMessage("Book returned successfully!", "success");
    } else {
        displayMessage("Failed to return book. Please check the ISBN.", "error");
    }
    pauseScreen();
}

void MenuHandler::handleRenewBook() {
    clearScreen();
    ui.displayHeader("Renew Book");

    auto borrowedBook = transactionManager.getActiveTransactions(currentUser->getMemberID());

    if (borrowedBook.empty()) {
        displayMessage("You have no books to renew.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n";
    std::cout << "Your borrowed books:\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(15) << "Due Date"
              << std::setw(15) << "Renewals" << "\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& transaction : borrowedBook) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        if (book) {
            std::cout << std::left << std::setw(15) << transaction->getISBN()
                      << std::setw(35) << book->getTitle()
                      << std::setw(15) << transaction->getDueDate()
                      << std::setw(15) << transaction->getRenewCount() << "\n";
        }
    }
    std::cout << std::string(80, '-') << "\n\n";

    std::string isbn = promptForInput("Enter ISBN of the book to renew: ");
    if (isbn.empty()) return;

    if (transactionManager.renewBook(currentUser->getMemberID(), isbn)) {
        displayMessage("Book renewed successfully!", "success");
    } else {
        displayMessage("Failed to renew book. You may have reached the maximum renewal limit.", "error");
    }
    pauseScreen();
}

void MenuHandler::handleReserveBook() {
    clearScreen();
    ui.displayHeader("Reserve Book");

    std::string isbn = promptForInput("Enter ISBN of the book to reserve: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("Book not found.", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    int currentQueueLength = reservationManager.getQueueLength(isbn);
    if (currentQueueLength > 0) {
        std::cout << "\n";
        std::cout << "⚠ RESERVATION QUEUE INFO:\n";
        std::cout << "  Current queue length: " << currentQueueLength << " people waiting\n";
        std::cout << "  Your position will be: " << (currentQueueLength + 1) << "\n\n";
    }

    if (book->canBorrow()) {
        displayMessage("This book is currently available. You can borrow it directly instead of reserving.", "info");
        if (!confirmAction("Do you still want to reserve this book?")) {
            return;
        }
    }

    // Check if member already has an active reservation for this book
    auto memberReservations = reservationManager.findByMemberID(currentUser->getMemberID());
    for (const auto* reservation : memberReservations) {
        if (reservation->getISBN() == isbn && reservation->getIsActive()) {
            displayMessage("You already have an active reservation for this book.", "error");

            int position = reservationManager.getQueuePosition(reservation->getReservationID());
            if (position > 0) {
                std::cout << "Your current position in queue: " << position << "\n";
            }
            pauseScreen();
            return;
        }
    }

    if (!confirmAction("Do you want to reserve for this book?")) {
        return;
    }

    std::string reservationID = reservationManager.reserveBook(currentUser->getMemberID(), isbn);

    if (reservationID == "0") {
        displayMessage("Failed to reserve book. Please try again or contact the administrator.", "error");
    } else {
        displayMessage("Book reserved successfully! Reservation ID: " + reservationID, "success");

        // Show queue information
        int position = reservationManager.getQueuePosition(reservationID);
        int queueLength = reservationManager.getQueueLength(isbn);

        std::cout << "\nReservation Details:\n";
        std::cout << "  Your position in queue: " << position << " of " << queueLength << "\n";

        if (position == 1) {
            std::cout << "   You're first in line! You'll be notified when the book is available.\n";
        } else {
            std::cout << "  Estimated wait: " << (position - 1) << " borrowing period(s)\n";
        }
    }

    pauseScreen();
}

void MenuHandler::handleViewBorrowedBooks() {
    clearScreen();
    ui.displayHeader("Currently Borrowed Books");

    auto borrowedBooks = transactionManager.getActiveTransactions(currentUser->getMemberID());

    if (borrowedBooks.empty()) {
        displayMessage("You have no borrowed books.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(30) << "Title"
              << std::setw(20) << "Author"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Due Date"
              << std::setw(5) << "Fine" << "\n";
    std::cout << std::string(100, '=') << "\n";

    double totalFine = 0.0;
    for (const auto& transaction : borrowedBooks) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        if (book) {
            double fine = transaction->calculateFine();
            totalFine += fine;

            std::cout << std::left << std::setw(15) << transaction->getISBN()
                      << std::setw(30) << book->getTitle().substr(0, 28)
                      << std::setw(20) << book->getAuthor().substr(0, 18)
                      << std::setw(15) << transaction->getBorrowDate()
                      << std::setw(15) << transaction->getDueDate()
                      << std::setw(5) << std::fixed << std::setprecision(2) << fine << "\n";
        }
    }
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Total Fine Due: $" << std::fixed << std::setprecision(2) << totalFine << "\n";
    std::cout << std::string(100, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewBorrowingHistory() {
    clearScreen();
    ui.displayHeader("Borrowing History");

    auto history = transactionManager.getMemberHistory(currentUser->getMemberID());

    if (history.empty()) {
        displayMessage("No borrowing history found.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(110, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(30) << "Title"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Return Date"
              << std::setw(10) << "Status"
              << std::setw(10) << "Fine" << "\n";
    std::cout << std::string(110, '=') << "\n";

    for (const auto& transaction : history) {
        Book* book = bookManager.findBookByISBN(transaction->getISBN());
        std::string title = book ? book->getTitle().substr(0, 28) : "Unknown";

        std::cout << std::left << std::setw(15) << transaction->getISBN()
                  << std::setw(30) << title
                  << std::setw(15) << transaction->getBorrowDate()
                  << std::setw(15) << (transaction->getReturnDate().empty() ? "N/A" : transaction->getReturnDate())
                  << std::setw(10) << (transaction->haveReturned() ? "Returned" : "Active")
                  << std::setw(10) << std::fixed << std::setprecision(2) << transaction->getFine() << "\n";
    }
    std::cout << std::string(110, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewReservations() {
    clearScreen();
    ui.displayHeader("My Reservations");

    auto myReservations = reservationManager.findByMemberID(currentUser->getMemberID());

    if (myReservations.empty()) {
        displayMessage("You have no reservations.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(140, '=') << "\n";
    std::cout << std::left << std::setw(15) << "Reservation ID"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "Book Title"
              << std::setw(18) << "Reserved Date"
              << std::setw(12) << "Status"
              << std::setw(10) << "Position"
              << std::setw(10) << "In Queue"
              << std::setw(15) << "Available"
              << std::setw(15) << "Action" << "\n";
    std::cout << std::string(140, '=') << "\n";

    int activeCount = 0;
    int nextInLineCount = 0;

    for (const auto* reservation : myReservations) {
        Book* book = bookManager.findBookByISBN(reservation->getISBN());
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "Unknown";
        std::string status = reservation->getIsActive() ? "Active" : "Cancelled";

        // Get queue information
        std::string positionStr = "-";
        std::string queueLengthStr = "-";
        std::string availableStr = "-";

        if (reservation->getIsActive()) {
            activeCount++;

            // Get queue position
            int position = reservationManager.getQueuePosition(reservation->getReservationID());
            int queueLength = reservationManager.getQueueLength(reservation->getISBN());

            if (position > 0) {
                if (position == 1) {
                    positionStr = "→ NEXT!";
                    nextInLineCount++;
                } else {
                    positionStr = std::to_string(position);
                }
                queueLengthStr = std::to_string(queueLength);
            }

            // Check if book is available
            if (book) {
                availableStr = std::to_string(book->getAvailableCopies()) + "/" + std::to_string(book->getTotalCopies());
            }
        }

        std::cout << std::left << std::setw(15) << reservation->getReservationID()
                  << std::setw(15) << reservation->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation->getReservationDate()
                  << std::setw(12) << status
                  << std::setw(10) << positionStr
                  << std::setw(10) << queueLengthStr
                  << std::setw(15) << availableStr
                  << std::setw(15) << (reservation->getIsActive() ? "Can Cancel" : "-") << "\n";
    }
    std::cout << std::string(140, '=') << "\n";
    std::cout << "Total Reservations: " << myReservations.size()
              << " (Active: " << activeCount;

    if (nextInLineCount > 0) {
        std::cout << ", Next in line: " << nextInLineCount;
    }
    std::cout << ")\n";
    std::cout << std::string(140, '=') << "\n\n";

    // Helpful information
    if (activeCount > 0) {
        std::cout << "Queue Position Guide:\n";
        std::cout << "   • Position 1 (YOU ARE NEXT!) = Book will be held for you when available\n";
        std::cout << "   • Higher positions = More people ahead in queue\n";
        // std::cout << "   • You'll be notified when you become next in line\n\n";
    }

    // Option to cancel a reservation
    if (activeCount > 0) {
        if (confirmAction("Would you like to cancel a reservation?")) {
            std::string reservationID = promptForInput("Enter Reservation ID to cancel: ");
            if (!reservationID.empty()) {
                std::string result = reservationManager.cancelReservation(reservationID);
                if (result != "0") {
                    displayMessage("Reservation cancelled successfully! Queue positions updated for others.", "success");
                } else {
                    displayMessage("Failed to cancel reservation. Please check the Reservation ID.", "error");
                }
            }
        }
    }

    pauseScreen();
}

void MenuHandler::handleViewRecommendations() {
    clearScreen();
    ui.displayHeader("Book Recommendations");

    std::cout << "\nPersonalized Book Recommendations for " << currentUser->getName() << "\n\n";

    // Configuration for recommendation algorithm KNN
    const int TOP_N = 5;              // Number of recommendations to show
    const int K_NEIGHBORS = 5;        // Number of similar users to consider
    const bool AVAILABLE_ONLY = false; // Show all books (user can reserve unavailable ones)

    std::cout << "Analyzing your reading preferences and finding similar readers...\n";
    std::cout << "   Using collaborative filtering with " << K_NEIGHBORS << " nearest neighbors\n\n";

    // Use the KNN algorithm
    std::vector<Book> recommendations = recommendationManager.recommendForMember(
        currentUser->getMemberID(),
        TOP_N,
        K_NEIGHBORS,
        AVAILABLE_ONLY
    );

    if (recommendations.empty()) {
        displayMessage("No recommendations available at the moment.", "info");
        std::cout << "\nThis could be because:\n";
        std::cout << "  • You're a new member (start borrowing books to get personalized recommendations)\n";
        std::cout << "  • No similar reading patterns found\n";
        std::cout << "  • All recommended books have already been borrowed by you\n\n";

        std::cout << "Tip: Update your reading preferences in your profile to get better recommendations!\n";
        pauseScreen();
        return;
    }

    std::cout << "Top " << recommendations.size() << " Recommendations for You:\n\n";
    std::cout << std::string(125, '=') << "\n";
    std::cout << std::left << std::setw(5) << "#"
              << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(25) << "Author"
              << std::setw(20) << "Genre"
              << std::setw(12) << "Status"
              << std::setw(8) << "Copies"
              << std::setw(5) << "⭐" << "\n";
    std::cout << std::string(125, '=') << "\n";

    int rank = 1;
    for (const auto& book : recommendations) {
        std::string status;
        std::string copiesInfo;

        if (book.canBorrow()) {
            status = " Available";
            copiesInfo = std::to_string(book.getAvailableCopies()) + "/" +
                        std::to_string(book.getTotalCopies());
        } else {
            status = " Reserved";
            int queueLength = reservationManager.getQueueLength(book.getISBN());
            copiesInfo = "Queue:" + std::to_string(queueLength);
        }

        // Recommendation strength indicator (top 5 get stars)
        std::string stars = (rank <= 5) ? "⭐" : "";

        std::cout << std::left << std::setw(5) << rank
                  << std::setw(15) << book.getISBN()
                  << std::setw(35) << book.getTitle().substr(0, 33)
                  << std::setw(25) << book.getAuthor().substr(0, 23)
                  << std::setw(20) << book.getGenre().substr(0, 18)
                  << std::setw(12) << status
                  << std::setw(8) << copiesInfo
                  << std::setw(5) << stars << "\n";
        rank++;
    }
    std::cout << std::string(125, '=') << "\n";

    std::cout << "\n⭐ = Highly recommended (top 5)\n";
    std::cout << " = Available for immediate borrowing\n";
    std::cout << " = Can be reserved (you'll be added to queue)\n\n";

    // Show statistics about recommendations
    int availableCount = 0;
    int reservedCount = 0;
    std::map<std::string, int> genreCount;

    for (const auto& book : recommendations) {
        if (book.canBorrow()) {
            availableCount++;
        } else {
            reservedCount++;
        }
        genreCount[book.getGenre()]++;
    }

    std::cout << "Recommendation Breakdown:\n";
    std::cout << "   Total recommendations: " << recommendations.size() << "\n";
    std::cout << "   Available now:        " << availableCount << "\n";
    std::cout << "   Requires reservation: " << reservedCount << "\n";

    if (!genreCount.empty()) {
        std::cout << "   Top genres suggested: ";
        int shown = 0;
        for (const auto& pair : genreCount) {
            if (shown > 0) std::cout << ", ";
            std::cout << pair.first << " (" << pair.second << ")";
            if (++shown >= 3) break;
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    pauseScreen();
}

void MenuHandler::handleViewProfile() {
    clearScreen();
    ui.displayHeader("My Profile");

    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "Member ID:       " << currentUser->getMemberID() << "\n";
    std::cout << "Name:            " << currentUser->getName() << "\n";
    std::cout << "Member Type:     " << (currentUser->getAdmin() ? "Administrator" : "Member") << "\n";
    std::cout << "Account Status:  " << (currentUser->isExpired() ? "Expired" : "Active") << "\n";
    std::cout << "Registration:    " << currentUser->getRegistrationDate() << "\n";
    std::cout << std::string(50, '=') << "\n";

    // Display borrowing statistics
    auto activeTransactions = transactionManager.getActiveTransactions(currentUser->getMemberID());
    auto history = transactionManager.getMemberHistory(currentUser->getMemberID());

    std::cout << "\nBorrowing Statistics:\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "Currently Borrowed: " << activeTransactions.size() << "\n";
    std::cout << "Total Borrowed:     " << history.size() << "\n";
    std::cout << std::string(50, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleManageBooks() {
    clearScreen();
    ui.displayHeader("Manage Books");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. Add New Book                        │\n";
    std::cout << "│  2. Update Book Information             │\n";
    std::cout << "│  3. Delete Book                         │\n";
    std::cout << "│  4. View All Books                      │\n";
    std::cout << "│  0. Back                                │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 4);

    switch (choice) {
        case 1: handleAddBook(); break;
        case 2: handleUpdateBook(); break;
        case 3: handleDeleteBook(); break;
        case 4: handleViewAllBooks(); break;
        case 0: return;
        default: {
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleManageMembers() {
    clearScreen();
    ui.displayHeader("Manage Members");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. Add New Member                      │\n";
    std::cout << "│  2. Update Member Information           │\n";
    std::cout << "│  3. Delete Member                       │\n";
    std::cout << "│  4. View All Members                    │\n";
    std::cout << "│  0. Back                                │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 4);

    switch (choice) {
        case 1: handleAddMember(); break;
        case 2: handleUpdateMember(); break;
        case 3: handleDeleteMember(); break;
        case 4: handleViewAllMembers(); break;
        case 0: return;
        default: {
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleManageTransactions() {
    clearScreen();
    ui.displayHeader("Manage Transactions");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. View All Transactions               │\n";
    std::cout << "│  2. View Active Transactions            │\n";
    std::cout << "│  3. Manual Return                       │\n";
    std::cout << "│  0. Back                                │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 3);

    switch (choice) {
        case 1: handleViewAllTransactions(); break;
        case 2: handleViewActiveTransactions(); break;
        case 3: handleManualReturn(); break;
        case 0: return;
        default: {
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleViewOverdueBooks() {
    clearScreen();
    ui.displayHeader("Overdue Books");

    auto overdueTransactions = transactionManager.getOverdueTransactions();

    if (overdueTransactions.empty()) {
        displayMessage("No overdue books found.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(12) << "Member ID"
              << std::setw(20) << "Member Name"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "Book Title"
              << std::setw(15) << "Due Date"
              << std::setw(12) << "Days Late"
              << std::setw(10) << "Fine" << "\n";
    std::cout << std::string(120, '=') << "\n";

    double totalFines = 0.0;
    for (const auto& transaction : overdueTransactions) {
        Member* member = memberManager.findMemberByID(transaction->getUserID());
        Book* book = bookManager.findBookByISBN(transaction->getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "Unknown";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "Unknown";
        double fine = transaction->calculateFine();
        totalFines += fine;

        std::cout << std::left << std::setw(12) << transaction->getUserID()
                  << std::setw(20) << memberName
                  << std::setw(15) << transaction->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(15) << transaction->getDueDate()
                  << "$" << std::setw(9) << std::fixed << std::setprecision(2) << fine << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "Total Overdue Books: " << overdueTransactions.size() << "\n";
    std::cout << "Total Fines: $" << std::fixed << std::setprecision(2) << totalFines << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleGenerateReports() {
    clearScreen();
    ui.displayHeader("Generate Reports");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. Inventory Report                    │\n";
    std::cout << "│  2. Member Report                       │\n";
    std::cout << "│  3. Transaction Report                  │\n";
    std::cout << "│  4. Statistics Report                   │\n";
    std::cout << "│  0. Back                                │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 5);

    switch (choice) {
        case 1: handleGenerateInventoryReport(); break;
        case 2: handleGenerateMemberReport(); break;
        case 3: handleGenerateTransactionReport(); break;
        case 4: handleGenerateStatisticsReport(); break;
        case 0: return;
        default: {
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleBackupRestore() {
    clearScreen();
    ui.displayHeader("Backup & Restore");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. Backup Data                         │\n";
    std::cout << "│  2. Restore Data                        │\n";
    std::cout << "│  0. Back                                │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 2);

    switch (choice) {
        case 1: handleBackupData(); break;
        case 2: handleRestoreData(); break;
        case 0: return;
        default: {
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleSystemSettings() {
    clearScreen();
    ui.displayHeader("System Settings");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. Change UI Mode                      │\n";
    std::cout << "│  2. Change Borrow Period                │\n";
    std::cout << "│  3. Change Fine Settings                │\n";
    std::cout << "│  0. Back                                │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 3);

    switch (choice) {
        case 1: handleChangeUIMode(); break;
        case 2: handleChangeBorrowPeriod(); break;
        case 3: handleChangeFineSettings(); break;
        case 0: return;
        default: {
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            break;
        }
    }
}

void MenuHandler::handleAddBook() {
    clearScreen();
    ui.displayHeader("Add New Book");

    std::string isbn = promptForInput("Enter ISBN: ");
    if (isbn.empty()) return;

    // Check if book already exists
    if (bookManager.findBookByISBN(isbn) != nullptr) {
        displayMessage("A book with this ISBN already exists.", "error");
        pauseScreen();
        return;
    }

    std::string title = promptForInput("Enter Title: ");
    if (title.empty()) return;

    std::string author = promptForInput("Enter Author: ");
    if (author.empty()) return;

    std::string publisher = promptForInput("Enter Publisher: ");
    if (publisher.empty()) return;

    std::string genre = promptForInput("Enter Genre: ");
    if (genre.empty()) return;

    int totalCopies = promptForInt("Enter Total Copies: ", 1, 1000);

    // Create new book with correct constructor parameters
    // Book(isbn, title, author, publisher, genre, totalCopies, availableCopies, isReserved)
    Book* newBook = new Book(isbn, title, author, publisher, genre, totalCopies, totalCopies, false);

    if (bookManager.addBook(*newBook)) {
        displayMessage("Book added successfully!", "success");
    } else {
        displayMessage("Failed to add book.", "error");
        delete newBook;
    }

    pauseScreen();
}

void MenuHandler::handleUpdateBook() {
    clearScreen();
    ui.displayHeader("Update Book Information");

    std::string isbn = promptForInput("Enter ISBN of the book to update: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("Book not found.", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    std::cout << "\nEnter new values (press Enter to keep current value):\n\n";

    // Get new values or keep existing ones
    std::string newTitle = promptForInput("Title [" + book->getTitle() + "]: ");
    std::string title = newTitle.empty() ? book->getTitle() : newTitle;

    std::string newAuthor = promptForInput("Author [" + book->getAuthor() + "]: ");
    std::string author = newAuthor.empty() ? book->getAuthor() : newAuthor;

    std::string newPublisher = promptForInput("Publisher [" + book->getPublisher() + "]: ");
    std::string publisher = newPublisher.empty() ? book->getPublisher() : newPublisher;

    std::string newGenre = promptForInput("Genre [" + book->getGenre() + "]: ");
    std::string genre = newGenre.empty() ? book->getGenre() : newGenre;

    Book updatedBook(
        book->getISBN(),
        title,
        author,
        publisher,
        genre,
        book->getTotalCopies(),
        book->getAvailableCopies(),
        false
    );

    if (bookManager.updateBook(updatedBook)) {
        displayMessage("Book updated successfully!", "success");
        // refresh memory and cache
        bookManager.clearCache();
        bookManager.reload();
    } else {
        displayMessage("Failed to update book.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleDeleteBook() {
    clearScreen();
    ui.displayHeader("Delete Book");

    std::string isbn = promptForInput("Enter ISBN of the book to delete: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("Book not found.", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    if (!confirmAction("Are you sure you want to delete this book?")) {
        return;
    }

    if (bookManager.deleteBook(isbn)) {
        displayMessage("Book deleted successfully!", "success");
    } else {
        displayMessage("Failed to delete book. It may have active borrowings.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleViewAllBooks() {
    clearScreen();
    ui.displayHeader("All Books in Library");

    //if user manually modified the CSV outside the program
    bookManager.clearCache();
    bookManager.reload();
    auto allBooks = bookManager.getAllBooks();

    if (allBooks.empty()) {
        displayMessage("No books in the library.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(25) << "Author"
              << std::setw(20) << "Genre"
              << std::setw(10) << "Total"
              << std::setw(10) << "Available" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto& book : allBooks) {
        std::cout << std::left << std::setw(15) << book.getISBN()
                  << std::setw(35) << book.getTitle().substr(0, 33)
                  << std::setw(25) << book.getAuthor().substr(0, 23)
                  << std::setw(20) << book.getGenre().substr(0, 18)
                  << std::setw(10) << book.getTotalCopies()
                  << std::setw(10) << book.getAvailableCopies() << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "Total Books: " << allBooks.size() << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleAddMember() {
    clearScreen();
    ui.displayHeader("Add New Member");

    std::string name = promptForInput("Enter Name: ");
    if (name.empty()) return;

    std::string phone = promptForInput("Enter Phone Number: ");
    if (phone.empty()) return;

    std::string password = promptForInput("Enter Password: ");
    if (password.empty()) return;

    // Get genre preferences (up to 5)
    std::vector<std::string> preferences;
    std::cout << "\nEnter reading preferences (genres, up to 5):\n";
    std::cout << "Available genres: Fiction, Non-Fiction, Science, History, Biography\n";
    std::cout << "Press Enter with no input to finish.\n\n";

    for (int i = 0; i < 5; i++) {
        std::string pref = promptForInput("Genre " + std::to_string(i + 1) + ": ");
        if (pref.empty()) break;
        preferences.push_back(pref);
    }

    std::cout << "\nMember Type:\n";
    std::cout << "  1. Regular Member\n";
    std::cout << "  2. Administrator\n";
    int typeChoice = promptForInt("Enter choice: ", 1, 2);
    bool isAdmin = (typeChoice == 2);

    auto generateNextMemberID = [&](bool adminFlag) -> std::string {
        const std::string prefix = adminFlag ? Config::ADMIN_ID_PREFIX : Config::MEMBER_ID_PREFIX;
        const std::string currentDate = DateUtils::getCurrentDate();
        const std::string year = currentDate.substr(0, 4);
        int month = std::stoi(currentDate.substr(5, 2));
        int season = (month - 1) / 3 + 1;
        int maxSeq = 0;

        const auto& allMembers = memberManager.getAllMembers();
        for (const auto& m : allMembers) {
            const std::string id = m.getMemberID();
            if (id.empty() || id[0] != prefix[0]) {
                continue;
            }

            if (id.size() != 9) {
                continue;
            }
            if (id.substr(1, 4) != year) {
                continue;
            }
            if (!std::isdigit(static_cast<unsigned char>(id[5]))) {
                continue;
            }
            int idSeason = id[5] - '0';
            if (idSeason != season) {
                continue;
            }
            bool seqDigits = true;
            for (size_t i = 6; i < id.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(id[i]))) {
                    seqDigits = false;
                    break;
                }
            }
            if (!seqDigits) {
                continue;
            }
            int seq = std::stoi(id.substr(6));
            if (seq > maxSeq) {
                maxSeq = seq;
            }
        }

        std::ostringstream oss;
        oss << prefix << year << season
            << std::setw(3) << std::setfill('0') << (maxSeq + 1);
        return oss.str();
    };

    std::string memberID = generateNextMemberID(isAdmin);

    // Generate dates
    std::string registrationDate = DateUtils::getCurrentDate();
    std::string expiryDate = DateUtils::addDays(registrationDate, 365); // 1 year membership

    // Hash the password
    std::string passwordHash = auth::hashPassword(password);

    // Get max books allowed
    Config& config = Config::getInstance();
    int maxBooksAllowed = config.getDefaultMaxBooks();

    // Create Member with correct constructor:
    Member* newMember = new Member(
        memberID,
        name,
        phone,
        preferences,
        registrationDate,
        expiryDate,
        maxBooksAllowed,
        isAdmin,
        passwordHash
    );

    if (memberManager.addMember(*newMember)) {
        displayMessage("Member added successfully!", "success");
        std::cout << "\nMembership Details:\n";
        std::cout << "  Member ID:        " << memberID << "\n";
        std::cout << "  Name:             " << name << "\n";
        std::cout << "  Type:             " << (isAdmin ? "Administrator" : "Regular Member") << "\n";
        std::cout << "  Registration:     " << registrationDate << "\n";
        std::cout << "  Expiry:           " << expiryDate << "\n";
        std::cout << "  Max Books:        " << maxBooksAllowed << "\n";
        std::cout << "  Preferences:      ";
        if (preferences.empty()) {
            std::cout << "None\n";
        } else {
            for (size_t i = 0; i < preferences.size(); i++) {
                std::cout << preferences[i];
                if (i < preferences.size() - 1) std::cout << ", ";
            }
            std::cout << "\n";
        }
    } else {
        displayMessage("Failed to add member.", "error");
        delete newMember;
    }

    pauseScreen();
}

void MenuHandler::handleUpdateMember() {
    clearScreen();
    ui.displayHeader("Update Member Information");

    std::string memberID = promptForInput("Enter Member ID to update: ");
    if (memberID.empty()) return;

    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr) {
        displayMessage("Member not found.", "error");
        pauseScreen();
        return;
    }

    std::cout << "\nCurrent Information:\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "Member ID: " << member->getMemberID() << "\n";
    std::cout << "Name:      " << member->getName() << "\n";
    std::cout << "Phone:     " << member->getPhoneNumber() << "\n";
    std::cout << std::string(50, '-') << "\n\n";

    std::cout << "Enter new values (press Enter to keep current value):\n\n";

    // Get new values or keep existing ones
    std::string newName = promptForInput("Name [" + member->getName() + "]: ");
    std::string name = newName.empty() ? member->getName() : newName;

    std::string newPhone = promptForInput("Phone [" + member->getPhoneNumber() + "]: ");
    std::string phone = newPhone.empty() ? member->getPhoneNumber() : newPhone;

    // Member class is immutable - create new instance with updated values
    Member updatedMember(
        member->getMemberID(),
        name,
        phone,
        member->getPreference(),
        member->getRegistrationDate(),
        member->getExpiryDate(),
        member->getMaxBooksAllowed(),
        member->getAdmin(),
        member->getPasswordHash()
    );

    if (memberManager.updateMember(updatedMember)) {
        displayMessage("Member updated successfully!", "success");
    } else {
        displayMessage("Failed to update member.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleDeleteMember() {
    clearScreen();
    ui.displayHeader("Delete Member");

    std::string memberId = promptForInput("Enter Member ID to delete: ");
    if (memberId.empty()) return;

    Member* member = memberManager.findMemberByID(memberId);
    if (member == nullptr) {
        displayMessage("Member not found.", "error");
        pauseScreen();
        return;
    }

    std::cout << "\nMember Information:\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "Member ID: " << member->getMemberID() << "\n";
    std::cout << "Name:      " << member->getName() << "\n";
    std::cout << "Phone:     " << member->getPhoneNumber() << "\n";
    std::cout << std::string(50, '-') << "\n\n";

    if (!confirmAction("Are you sure you want to delete this member?")) {
        return;
    }

    if (memberManager.deleteMember(memberId)) {
        displayMessage("Member deleted successfully!", "success");
    } else {
        displayMessage("Failed to delete member. Member may have active borrowings.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleViewAllMembers() {
    clearScreen();
    ui.displayHeader("All Members");

    auto allMembers = memberManager.getAllMembers();

    if (allMembers.empty()) {
        displayMessage("No members found.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << std::left << std::setw(12) << "Member ID"
              << std::setw(25) << "Name"
              << std::setw(30) << "Email"
              << std::setw(15) << "Phone"
              << std::setw(10) << "Type"
              << std::setw(8) << "Status" << "\n";
    std::cout << std::string(100, '=') << "\n";

    for (const auto& member : allMembers) {
        std::cout << std::left << std::setw(12) << member.getMemberID()
                  << std::setw(25) << member.getName().substr(0, 23)
                  << std::setw(15) << member.getPhoneNumber()
                  << std::setw(10) << (member.getAdmin() ? "Admin" : "Member")
                  << std::setw(8) << (member.isExpired() ? "Expired" : "Active") << "\n";
    }
    std::cout << std::string(100, '=') << "\n";
    std::cout << "Total Members: " << allMembers.size() << "\n";
    std::cout << std::string(100, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewAllTransactions() {
    clearScreen();
    ui.displayHeader("All Transactions");

    auto allTransactions = transactionManager.getAllTransactions();

    if (allTransactions.empty()) {
        displayMessage("No transactions found.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "Transaction ID"
              << std::setw(12) << "Member ID"
              << std::setw(15) << "ISBN"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Due Date"
              << std::setw(15) << "Return Date"
              << std::setw(10) << "Status"
              << std::setw(8) << "Fine" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto& transaction : allTransactions) {
        std::cout << std::left << std::setw(15) << transaction.getTransactionID()
                  << std::setw(12) << transaction.getUserID()
                  << std::setw(15) << transaction.getISBN()
                  << std::setw(15) << transaction.getBorrowDate()
                  << std::setw(15) << transaction.getDueDate()
                  << std::setw(15) << (transaction.getReturnDate().empty() ? "N/A" : transaction.getReturnDate())
                  << std::setw(10) << (transaction.haveReturned() ? "Returned" : "Active")
                  << "$" << std::setw(7) << std::fixed << std::setprecision(2) << transaction.getFine() << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "Total Transactions: " << allTransactions.size() << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewActiveTransactions() {
    clearScreen();
    ui.displayHeader("Active Transactions");

    auto activeTransactions = transactionManager.findActiveTransactions();
    if (activeTransactions.empty()) {
        displayMessage("No active transactions found.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(110, '=') << "\n";
    std::cout << std::left << std::setw(12) << "Member ID"
              << std::setw(20) << "Member Name"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "Book Title"
              << std::setw(15) << "Borrow Date"
              << std::setw(15) << "Due Date"
              << std::setw(8) << "Fine" << "\n";
    std::cout << std::string(110, '=') << "\n";

    for (const auto& transaction : activeTransactions) {
        Member* memberByTransaction = memberManager.findMemberByID(transaction->getUserID());
        Book* book = bookManager.findBookByISBN(transaction->getISBN());

        std::string memberName = memberByTransaction ? memberByTransaction->getName().substr(0, 18) : "Unknown";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "Unknown";

        std::cout << std::left << std::setw(12) << transaction->getUserID()
                  << std::setw(20) << memberName
                  << std::setw(15) << transaction->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(15) << transaction->getBorrowDate()
                  << std::setw(15) << transaction->getDueDate()
                  << "$" << std::setw(7) << std::fixed << std::setprecision(2) << transaction->calculateFine() << "\n";
    }
    std::cout << std::string(110, '=') << "\n";
    std::cout << "Total Active Transactions: " << activeTransactions.size() << "\n";
    std::cout << std::string(110, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleManualReturn() {
    clearScreen();
    ui.displayHeader("Manual Return");

    std::string memberID = promptForInput("Enter Member ID: ");
    if (memberID.empty()) return;

    std::string isbn = promptForInput("Enter ISBN: ");
    if (isbn.empty()) return;

    if (confirmAction("Process return for Member " + memberID + ", Book " + isbn + "?")) {
        if (transactionManager.returnBook(memberID, isbn)) {
            displayMessage("Book returned successfully!", "success");
        } else {
            displayMessage("Failed to process return. Please verify the details.", "error");
        }
    }

    pauseScreen();
}

void MenuHandler::handleGenerateInventoryReport() {
    clearScreen();
    ui.displayHeader("Generate Inventory Report");

    std::cout << "\nGenerating inventory report...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateInventoryReport(true)) {
        displayMessage("Inventory report generated successfully!", "success");
        std::cout << "\n✓ Report saved to: " << Config::REPORTS_DIR << "\n";
        std::cout << "  Check the reports directory for the detailed inventory report.\n";
    } else {
        displayMessage("Failed to generate inventory report.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleGenerateMemberReport() {
    clearScreen();
    ui.displayHeader("Generate Member Report");

    std::cout << "\nGenerating member report...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateMemberReport(true)) {
        displayMessage("Member report generated successfully!", "success");
        std::cout << "\n✓ Report saved to: " << Config::REPORTS_DIR << "\n";
        std::cout << "  Check the reports directory for the detailed member report.\n";
    } else {
        displayMessage("Failed to generate member report.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleGenerateTransactionReport() {
    clearScreen();
    ui.displayHeader("Generate Transaction Report");

    std::cout << "\nGenerating transaction report...\n";

    int topN = promptForInt("How many top transactions to include? (10-100): ", 10, 100);
    if (topN == -1) return;

    std::cout << "\nGenerating report with top " << topN << " transactions...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateTransactionReport(topN, true)) {
        displayMessage("Transaction report generated successfully!", "success");
        std::cout << "\n✓ Report saved to: " << Config::REPORTS_DIR << "\n";
        std::cout << "  Included top " << topN << " transactions.\n";
    } else {
        displayMessage("Failed to generate transaction report.", "error");
    }

    pauseScreen();
}


void MenuHandler::handleGenerateStatisticsReport() {
    clearScreen();
    ui.displayHeader("Generate Statistics Report");

    std::cout << "\nGenerating comprehensive statistics report...\n";
    std::cout << "   This includes all reports: Summary, Inventory, Members,\n";
    std::cout << "   Transactions, Reservations, and Top Borrowed Books.\n\n";

    int topN = promptForInt("Number of top items to include (10-50): ", 10, 50);
    if (topN == -1) return;

    std::cout << "\nGenerating all reports...\n\n";

    ReportManager reportManager(
        Config::BOOKS_FILE,
        Config::MEMBERS_FILE,
        Config::TRANSACTIONS_FILE,
        Config::RESERVATIONS_FILE,
        Config::REPORTS_DIR
    );

    if (reportManager.generateAllReports(topN, true)) {
        displayMessage("All statistical reports generated successfully!", "success");
        std::cout << "\n✓ Reports saved to: " << Config::REPORTS_DIR << "\n";
        std::cout << "\nGenerated reports:\n";
        std::cout << "  • Summary Report\n";
        std::cout << "  • Inventory Report\n";
        std::cout << "  • Member Report\n";
        std::cout << "  • Transaction Report (top " << topN << ")\n";
        std::cout << "  • Reservation Report\n";
        std::cout << "  • Top Borrowed Books (top " << topN << ")\n";
    } else {
        displayMessage("Failed to generate some or all reports.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleBackupData() {
    clearScreen();
    ui.displayHeader("Backup System Data");

    std::string description = promptForInput("Enter backup description (optional): ");
    if (description.empty()) {
        description = "Manual backup";
    }

    std::cout << "\nCreating backup...\n";
    std::cout << "   Description: " << description << "\n\n";

    BackupManager backupManager;

    if (backupManager.backupData(description)) {
        BackupInfo latestBackup = backupManager.getLatestBackup();

        displayMessage("Backup created successfully!", "success");
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════╗\n";
        std::cout << "║           BACKUP COMPLETED                     ║\n";
        std::cout << "╠════════════════════════════════════════════════╣\n";
        std::cout << "║  Backup ID:   " << std::left << std::setw(33) << latestBackup.backupID << "║\n";
        std::cout << "║  Time:        " << std::left << std::setw(33) << latestBackup.backupTime << "║\n";
        std::cout << "║  Description: " << std::left << std::setw(33) << latestBackup.description.substr(0, 32) << "║\n";
        std::cout << "║  Status:      " << std::left << std::setw(35) << (latestBackup.isValid ? "✓ Valid" : "✗ Invalid") << "║\n";
        std::cout << "╚════════════════════════════════════════════════╝\n";
    } else {
        displayMessage("Failed to create backup. Check error messages above.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleRestoreData() {
    clearScreen();
    ui.displayHeader("Restore System Data");

    BackupManager backupManager;

    if (!backupManager.hasValidBackups()) {
        displayMessage("No valid backups available to restore.", "warning");
        pauseScreen();
        return;
    }

    auto backups = backupManager.listBackups();

    std::cout << "\nAvailable Backups:\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(5) << "#"
              << std::setw(25) << "Backup ID"
              << std::setw(22) << "Time"
              << std::setw(40) << "Description"
              << std::setw(10) << "Status" << "\n";
    std::cout << std::string(120, '=') << "\n";

    int index = 1;
    for (const auto& backup : backups) {
        std::cout << std::left << std::setw(5) << index++
                  << std::setw(25) << backup.backupID
                  << std::setw(22) << backup.backupTime
                  << std::setw(40) << backup.description.substr(0, 38)
                  << std::setw(10) << (backup.isValid ? "✓ Valid" : "✗ Invalid") << "\n";
    }
    std::cout << std::string(120, '=') << "\n\n";

    std::cout << "WARNING: Restoring will OVERWRITE all current data!\n";
    std::cout << "   Make sure to backup current data before restoring.\n\n";

    if (!confirmAction("Do you want to proceed with restoration?")) {
        return;
    }

    std::string backupId = promptForInput("Enter Backup ID to restore: ");
    if (backupId.empty()) return;

    // Verify backup exists and is valid
    BackupInfo selectedBackup = backupManager.getBackupInfo(backupId);
    if (selectedBackup.backupID.empty()) {
        displayMessage("Backup ID not found.", "error");
        pauseScreen();
        return;
    }

    if (!selectedBackup.isValid) {
        displayMessage("Selected backup is invalid or corrupted.", "error");
        pauseScreen();
        return;
    }

    std::cout << "\nRestoring backup: " << selectedBackup.description << "\n";
    std::cout << "From: " << selectedBackup.backupTime << "\n\n";

    if (!confirmAction("Final confirmation - restore this backup?")) {
        return;
    }

    std::cout << "\nRestoring data...\n";

    if (backupManager.restoreData(backupId)) {
        displayMessage("Data restored successfully!", "success");
        std::cout << "\nIMPORTANT: Please restart the application for changes to take effect.\n";
    } else {
        displayMessage("Failed to restore data. Check error messages above.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleChangeUIMode() {
    clearScreen();
    ui.displayHeader("Change UI Mode");

    Config& config = Config::getInstance();

    std::cout << "\nCurrent UI Mode: " << (config.isAdvancedUIMode() ? "Advanced" : "Simple") << "\n\n";

    std::cout << "Available Modes:\n";
    std::cout << "  1. Simple Mode  - Basic menus and minimal interface\n";
    std::cout << "  2. Advanced Mode - Enhanced UI with more features\n";
    std::cout << "  0. Cancel\n\n";

    int choice = promptForInt("Select UI mode: ", 0, 2);

    if (choice == 0) return;

    bool newMode = (choice == 2);

    if (newMode == config.isAdvancedUIMode()) {
        displayMessage("UI mode is already set to this option.", "info");
        pauseScreen();
        return;
    }

    config.setAdvancedUIMode(newMode);
    config.saveSettings();

    displayMessage("UI mode changed successfully!", "success");
    std::cout << "New mode: " << (newMode ? "Advanced" : "Simple") << "\n";
    std::cout << "\nNote: Some changes may require application restart.\n";

    pauseScreen();
}

void MenuHandler::handleChangeBorrowPeriod() {
    clearScreen();
    ui.displayHeader("Change Borrow Period");

    Config& config = Config::getInstance();

    std::cout << "\nCurrent Borrow Period: " << config.getBorrowPeriodDays() << " days\n\n";

    std::cout << "Enter new borrow period (7-90 days): ";
    int newPeriod = promptForInt("Days: ", 7, 90);

    if (newPeriod == -1) return;  // User cancelled

    if (newPeriod == config.getBorrowPeriodDays()) {
        displayMessage("Borrow period is already set to this value.", "info");
        pauseScreen();
        return;
    }

    if (!confirmAction("Change borrow period to " + std::to_string(newPeriod) + " days?")) {
        return;
    }

    config.setBorrowPeriodDays(newPeriod);
    config.saveSettings();

    displayMessage("Borrow period updated successfully!", "success");
    std::cout << "New period: " << newPeriod << " days\n";
    std::cout << "\nNote: This affects new borrowings only. Existing loans unchanged.\n";

    pauseScreen();
}

void MenuHandler::handleChangeFineSettings() {
    clearScreen();
    ui.displayHeader("Change Fine Settings");

    Config& config = Config::getInstance();

    std::cout << "\nCurrent Fine Settings:\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << "Fine per day: $" << std::fixed << std::setprecision(2) << config.getFinePerDay() << "\n";
    std::cout << "Maximum fine: $" << std::fixed << std::setprecision(2) << config.getMaxFine() << "\n";
    std::cout << std::string(40, '-') << "\n\n";

    std::cout << "What would you like to change?\n";
    std::cout << "  1. Fine per day\n";
    std::cout << "  2. Maximum fine\n";
    std::cout << "  3. Both\n";
    std::cout << "  0. Cancel\n\n";

    int choice = promptForInt("Enter choice: ", 0, 3);

    if (choice == 0) return;

    double newFinePerDay = config.getFinePerDay();
    double newMaxFine = config.getMaxFine();

    if (choice == 1 || choice == 3) {
        newFinePerDay = promptForDouble("Enter new fine per day ($0.00-$50.00): ");
        if (newFinePerDay < 0 || newFinePerDay > 50.0) {
            displayMessage("Invalid fine amount. Must be between $0.00 and $50.00.", "error");
            pauseScreen();
            return;
        }
    }

    if (choice == 2 || choice == 3) {
        newMaxFine = promptForDouble("Enter new maximum fine ($0.00-$500.00): ");
        if (newMaxFine < 0 || newMaxFine > 500.0) {
            displayMessage("Invalid fine amount. Must be between $0.00 and $500.00.", "error");
            pauseScreen();
            return;
        }
    }

    // Validation: max fine should be >= fine per day
    if (newMaxFine < newFinePerDay) {
        displayMessage("Maximum fine cannot be less than daily fine.", "error");
        pauseScreen();
        return;
    }

    std::cout << "\nNew settings:\n";
    std::cout << "  Fine per day: $" << std::fixed << std::setprecision(2) << newFinePerDay << "\n";
    std::cout << "  Maximum fine: $" << std::fixed << std::setprecision(2) << newMaxFine << "\n\n";

    if (!confirmAction("Apply these fine settings?")) {
        return;
    }

    config.setFinePerDay(newFinePerDay);
    config.setMaxFine(newMaxFine);
    config.saveSettings();

    displayMessage("Fine settings updated successfully!", "success");
    std::cout << "\nNote: New fines apply to future overdue calculations.\n";

    pauseScreen();
}

// Reservation Management Handlers

void MenuHandler::handleManageReservations() {
    clearScreen();
    ui.displayHeader("Manage Reservations");

    std::cout << "\n";
    std::cout << "┌─────────────────────────────────────────┐\n";
    std::cout << "│  1. View All Reservations               │\n";
    std::cout << "│  2. View Active Reservations            │\n";
    std::cout << "│  3. Add Reservation                     │\n";
    std::cout << "│  4. Update Reservation                  │\n";
    std::cout << "│  5. Cancel Reservation                  │\n";
    std::cout << "│  6. Search Reservations                 │\n";
    std::cout << "│  0. Back                                │\n";
    std::cout << "└─────────────────────────────────────────┘\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 6);

    switch (choice) {
        case 1: handleViewAllReservation(); break;
        case 2: handleViewActiveReservation(); break;
        case 3: handleAddReservation(); break;
        case 4: handleUpdateReservation(); break;
        case 5: handleDeleteReservation(); break;
        case 6: handleSearchReservations(); break;
        case 0: return;
        default: displayMessage("Invalid choice.", "error"); pauseScreen(); break;
    }
}

void MenuHandler::handleViewAllReservation() {
    clearScreen();
    ui.displayHeader("All Reservations");

    const auto& allReservations = reservationManager.getAllReservations();

    if (allReservations.empty()) {
        displayMessage("No reservations found.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "Reservation ID"
              << std::setw(12) << "Member ID"
              << std::setw(20) << "Member Name"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "Book Title"
              << std::setw(18) << "Reservation Date"
              << std::setw(10) << "Status" << "\n";
    std::cout << std::string(120, '=') << "\n";

    int activeCount = 0;
    for (const auto& reservation : allReservations) {
        Member* member = memberManager.findMemberByID(reservation.getMemberID());
        Book* book = bookManager.findBookByISBN(reservation.getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "Unknown";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "Unknown";

        if (reservation.getIsActive()) {
            activeCount++;
        }

        std::cout << std::left << std::setw(15) << reservation.getReservationID()
                  << std::setw(12) << reservation.getMemberID()
                  << std::setw(20) << memberName
                  << std::setw(15) << reservation.getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation.getReservationDate()
                  << std::setw(10) << (reservation.getIsActive() ? "Active" : "Cancelled") << "\n";
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "Total Reservations: " << allReservations.size()
              << " (Active: " << activeCount << ", Cancelled: " << (allReservations.size() - activeCount) << ")\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleViewActiveReservation() {
    clearScreen();
    ui.displayHeader("Active Reservations");

    auto activeReservations = reservationManager.findActiveReservations();

    if (activeReservations.empty()) {
        displayMessage("No active reservations found.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\n" << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "Reservation ID"
              << std::setw(12) << "Member ID"
              << std::setw(20) << "Member Name"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "Book Title"
              << std::setw(18) << "Reservation Date";
    std::cout << std::string(120, '=') << "\n";

    for (const auto* reservation : activeReservations) {
        Member* member = memberManager.findMemberByID(reservation->getMemberID());
        Book* book = bookManager.findBookByISBN(reservation->getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "Unknown";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "Unknown";

        std::cout << std::left << std::setw(15) << reservation->getReservationID()
                  << std::setw(12) << reservation->getMemberID()
                  << std::setw(20) << memberName
                  << std::setw(15) << reservation->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation->getReservationDate();
    }
    std::cout << std::string(120, '=') << "\n";
    std::cout << "Total Active Reservations: " << activeReservations.size() << "\n";
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

void MenuHandler::handleAddReservation() {
    clearScreen();
    ui.displayHeader("Add Reservation");

    std::string memberID = promptForInput("Enter Member ID: ");
    if (memberID.empty()) return;

    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr) {
        displayMessage("Member not found.", "error");
        pauseScreen();
        return;
    }

    std::cout << "\nMember: " << member->getName() << " (" << memberID << ")\n\n";

    std::string isbn = promptForInput("Enter ISBN: ");
    if (isbn.empty()) return;

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr) {
        displayMessage("Book not found.", "error");
        pauseScreen();
        return;
    }

    displayBookDetails(book);

    // Check for existing active reservation
    auto existingReservations = reservationManager.findByMemberID(memberID);
    for (const auto* reservation : existingReservations) {
        if (reservation->getISBN() == isbn && reservation->getIsActive()) {
            displayMessage("This member already has an active reservation for this book.", "error");
            pauseScreen();
            return;
        }
    }

    if (!confirmAction("Create reservation for this member?")) {
        return;
    }

    std::string reservationID = reservationManager.reserveBook(memberID, isbn);

    if (reservationID == "0") {
        displayMessage("Failed to create reservation.", "error");
    } else {
        displayMessage("Reservation created successfully! ID: " + reservationID, "success");
    }

    pauseScreen();
}

void MenuHandler::handleDeleteReservation() {
    clearScreen();
    ui.displayHeader("Cancel Reservation");

    std::string reservationID = promptForInput("Enter Reservation ID to cancel: ");
    if (reservationID.empty()) return;

    Reservation* reservation = reservationManager.findByReservationID(reservationID);
    if (reservation == nullptr) {
        displayMessage("Reservation not found.", "error");
        pauseScreen();
        return;
    }

    // Display reservation details
    Member* member = memberManager.findMemberByID(reservation->getMemberID());
    Book* book = bookManager.findBookByISBN(reservation->getISBN());

    std::cout << "\nReservation Details:\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << "Reservation ID:   " << reservation->getReservationID() << "\n";
    std::cout << "Member:           " << (member ? member->getName() : "Unknown")
              << " (" << reservation->getMemberID() << ")\n";
    std::cout << "Book:             " << (book ? book->getTitle() : "Unknown") << "\n";
    std::cout << "ISBN:             " << reservation->getISBN() << "\n";
    std::cout << "Reservation Date: " << reservation->getReservationDate() << "\n";
    std::cout << "Status:           " << (reservation->getIsActive() ? "Active" : "Cancelled") << "\n";
    std::cout << std::string(60, '-') << "\n\n";

    if (!reservation->getIsActive()) {
        displayMessage("This reservation is already cancelled.", "warning");
        pauseScreen();
        return;
    }

    if (!confirmAction("Cancel this reservation?")) {
        return;
    }

    std::string result = reservationManager.cancelReservation(reservationID);

    if (result != "0") {
        displayMessage("Reservation cancelled successfully!", "success");
    } else {
        displayMessage("Failed to cancel reservation.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleUpdateReservation() {
    clearScreen();
    ui.displayHeader("Update Reservation");

    std::string reservationID = promptForInput("Enter Reservation ID to update: ");
    if (reservationID.empty()) return;

    Reservation* reservation = reservationManager.findByReservationID(reservationID);
    if (reservation == nullptr) {
        displayMessage("Reservation not found.", "error");
        pauseScreen();
        return;
    }

    // Display current reservation details
    Member* member = memberManager.findMemberByID(reservation->getMemberID());
    Book* book = bookManager.findBookByISBN(reservation->getISBN());

    std::cout << "\n";
    std::cout << "Current Reservation Details:\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "Reservation ID:   " << reservation->getReservationID() << "\n";
    std::cout << "Member:           " << (member ? member->getName() : "Unknown")
              << " (" << reservation->getMemberID() << ")\n";
    std::cout << "Book:             " << (book ? book->getTitle() : "Unknown") << "\n";
    std::cout << "ISBN:             " << reservation->getISBN() << "\n";
    std::cout << "Reservation Date: " << reservation->getReservationDate() << "\n";
    std::cout << "Status:           " << (reservation->getIsActive() ? "Active" : "Cancelled") << "\n";

    // Show queue position if active
    if (reservation->getIsActive()) {
        int position = reservationManager.getQueuePosition(reservationID);
        int queueLength = reservationManager.getQueueLength(reservation->getISBN());
        std::cout << "Queue Position:   " << position << " of " << queueLength << "\n";
    }
    std::cout << std::string(60, '=') << "\n\n";

    // Update options
    std::cout << "What would you like to update?\n";
    std::cout << "  1. Change Member ID\n";
    std::cout << "  2. Change Book (ISBN)\n";
    std::cout << "  3. Toggle Status (Active/Cancelled)\n";
    std::cout << "  0. Cancel\n\n";

    int choice = promptForInt("Enter choice: ", 0, 3);

    if (choice == 0) return;

    std::string newMemberID = reservation->getMemberID();
    std::string newISBN = reservation->getISBN();
    bool newIsActive = reservation->getIsActive();

    switch (choice) {
        case 1: {
            // Change member
            std::string inputMemberID = promptForInput("Enter new Member ID: ");
            if (inputMemberID.empty()) return;

            Member* newMember = memberManager.findMemberByID(inputMemberID);
            if (newMember == nullptr) {
                displayMessage("Member not found.", "error");
                pauseScreen();
                return;
            }

            newMemberID = inputMemberID;
            std::cout << "\n✓ Will change member to: " << newMember->getName() << "\n";
            break;
        }
        case 2: {
            // Change book
            std::string inputISBN = promptForInput("Enter new ISBN: ");
            if (inputISBN.empty()) return;

            Book* newBook = bookManager.findBookByISBN(inputISBN);
            if (newBook == nullptr) {
                displayMessage("Book not found.", "error");
                pauseScreen();
                return;
            }

            newISBN = inputISBN;
            std::cout << "\n✓ Will change book to: " << newBook->getTitle() << "\n";
            break;
        }
        case 3: {
            // Toggle status
            newIsActive = !reservation->getIsActive();
            std::cout << "\n✓ Will change status to: " << (newIsActive ? "Active" : "Cancelled") << "\n";
            break;
        }
        default:
            return;
    }

    if (!confirmAction("\nApply these changes?")) {
        return;
    }

    // Create updated reservation (Reservation is immutable)
    Reservation updatedReservation(
        reservation->getReservationID(),
        newMemberID,
        newISBN,
        reservation->getReservationDate(),
        newIsActive
    );

    if (reservationManager.updateReservation(updatedReservation)) {
        displayMessage("Reservation updated successfully!", "success");

        // Show what changed
        std::cout << "\nChanges applied:\n";
        if (newMemberID != reservation->getMemberID()) {
            std::cout << "  • Member ID: " << reservation->getMemberID() << " → " << newMemberID << "\n";
        }
        if (newISBN != reservation->getISBN()) {
            std::cout << "  • ISBN: " << reservation->getISBN() << " → " << newISBN << "\n";
        }
        if (newIsActive != reservation->getIsActive()) {
            std::cout << "  • Status: " << (reservation->getIsActive() ? "Active" : "Cancelled")
                      << " → " << (newIsActive ? "Active" : "Cancelled") << "\n";
        }
    } else {
        displayMessage("Failed to update reservation.", "error");
    }

    pauseScreen();
}

void MenuHandler::handleSearchReservations() {
    clearScreen();
    ui.displayHeader("Search Reservations");

    std::cout << "\nSearch by:\n";
    std::cout << "  1. Reservation ID\n";
    std::cout << "  2. Member ID\n";
    std::cout << "  3. ISBN\n";
    std::cout << "  4. Reservation Date\n";
    std::cout << "  0. Back\n\n";

    int choice = promptForInt("Enter your choice: ", 0, 4);

    std::vector<const Reservation*> results;

    switch (choice) {
        case 1: {
            std::string reservationID = promptForInput("Enter Reservation ID: ");
            if (!reservationID.empty()) {
                Reservation* res = reservationManager.findByReservationID(reservationID);
                if (res != nullptr) {
                    results.push_back(res);
                }
            }
            break;
        }
        case 2: {
            std::string memberId = promptForInput("Enter Member ID: ");
            if (!memberId.empty()) {
                results = reservationManager.findByMemberID(memberId);
            }
            break;
        }
        case 3: {
            std::string isbn = promptForInput("Enter ISBN: ");
            if (!isbn.empty()) {
                results = reservationManager.findByISBN(isbn);
            }
            break;
        }
        case 4: {
            std::string date = promptForInput("Enter Reservation Date (YYYY-MM-DD): ");
            if (!date.empty()) {
                results = reservationManager.findByReservationDate(date);
            }
            break;
        }
        case 0:
            return;
        default:
            displayMessage("Invalid choice.", "error");
            pauseScreen();
            return;
    }

    if (results.empty()) {
        displayMessage("No reservations found.", "info");
        pauseScreen();
        return;
    }

    clearScreen();
    ui.displayHeader("Search Results");

    std::cout << "\nFound " << results.size() << " reservation(s):\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "Reservation ID"
              << std::setw(12) << "Member ID"
              << std::setw(20) << "Member Name"
              << std::setw(15) << "ISBN"
              << std::setw(30) << "Book Title"
              << std::setw(18) << "Reservation Date"
              << std::setw(10) << "Status" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto* reservation : results) {
        Member* member = memberManager.findMemberByID(reservation->getMemberID());
        Book* book = bookManager.findBookByISBN(reservation->getISBN());

        std::string memberName = member ? member->getName().substr(0, 18) : "Unknown";
        std::string bookTitle = book ? book->getTitle().substr(0, 28) : "Unknown";

        std::cout << std::left << std::setw(15) << reservation->getReservationID()
                  << std::setw(12) << reservation->getMemberID()
                  << std::setw(20) << memberName
                  << std::setw(15) << reservation->getISBN()
                  << std::setw(30) << bookTitle
                  << std::setw(18) << reservation->getReservationDate()
                  << std::setw(10) << (reservation->getIsActive() ? "Active" : "Cancelled") << "\n";
    }
    std::cout << std::string(120, '=') << "\n";

    pauseScreen();
}

// Utility Functions

std::string MenuHandler::promptForInput(const std::string& prompt, bool canCancel) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);

    // Trim whitespace
    input.erase(0, input.find_first_not_of(" \t\n\r"));
    input.erase(input.find_last_not_of(" \t\n\r") + 1);

    if (canCancel && (input == "q" || input == "Q" || input == "cancel")) {
        return "";
    }

    return input;
}

int MenuHandler::promptForInt(const std::string& prompt, int min, int max, bool canCancel) {
    while (true) {
        std::string input = promptForInput(prompt, canCancel);

        if (input.empty() && canCancel) {
            return -1;
        }

        try {
            int value = std::stoi(input);
            if (value >= min && value <= max) {
                return value;
            }
            displayMessage("Please enter a number between " + std::to_string(min) +
                         " and " + std::to_string(max) + ".", "error");
        } catch (...) {
            displayMessage("Invalid input. Please enter a valid number.", "error");
        }
    }
}

double MenuHandler::promptForDouble(const std::string& prompt, bool canCancel) {
    while (true) {
        std::string input = promptForInput(prompt, canCancel);

        if (input.empty() && canCancel) {
            return -1.0;
        }

        try {
            double value = std::stod(input);
            if (value >= 0) {
                return value;
            }
            displayMessage("Please enter a positive number.", "error");
        } catch (...) {
            displayMessage("Invalid input. Please enter a valid number.", "error");
        }
    }
}

bool MenuHandler::confirmAction(const std::string& message) {
    std::cout << "\n" << message << " (y/n): ";
    std::string input;
    std::getline(std::cin, input);

    return (input == "y" || input == "Y" || input == "yes" || input == "Yes" || input == "YES");
}

void MenuHandler::displayMessage(const std::string& message, const std::string& type) {
    std::cout << "\n";
    if (type == "success") {
        std::cout << "SUCCESS: " << message << "\n";
    } else if (type == "error") {
        std::cout << "ERROR: " << message << "\n";
    } else if (type == "warning") {
        std::cout << "WARNING: " << message << "\n";
    } else {
        std::cout << "INFO: " << message << "\n";
    }
    std::cout << "\n";
}

void MenuHandler::pauseScreen() {
    std::cout << "\nPress Enter to continue...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');     // Discard all characters in buffer
    // std::cin.get();
}

void MenuHandler::clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Search Helper Functions

void MenuHandler::performBookSearch(const std::string& searchType) {
    std::string query = promptForInput("Enter search term: ");
    if (query.empty()) return;

    std::vector<const Book*> results;

    if (searchType == "title") {
        results = bookManager.findByTitle(query);
    } else if (searchType == "author") {
        results = bookManager.findByAuthor(query);
    } else if (searchType == "isbn") {
        Book* book = bookManager.findBookByISBN(query);
        if (book != nullptr) {
            results.push_back(book);
        }
    } else if (searchType == "genre") {
        results = bookManager.findByGenre(query);
    } else if (searchType == "publisher") {
        results = bookManager.findByPublisher(query);
    }

    displaySearchResults(results);
}

void MenuHandler::displaySearchResults(const std::vector<const class Book*>& results) {
    clearScreen();
    ui.displayHeader("Search Results");

    if (results.empty()) {
        displayMessage("No books found matching your search criteria.", "info");
        pauseScreen();
        return;
    }

    std::cout << "\nFound " << results.size() << " book(s):\n";
    std::cout << std::string(120, '=') << "\n";
    std::cout << std::left << std::setw(15) << "ISBN"
              << std::setw(35) << "Title"
              << std::setw(25) << "Author"
              << std::setw(20) << "Category"
              << std::setw(10) << "Available" << "\n";
    std::cout << std::string(120, '=') << "\n";

    for (const auto& book : results) {
        std::cout << std::left << std::setw(15) << book->getISBN()
                  << std::setw(35) << book->getTitle().substr(0, 33)
                  << std::setw(25) << book->getAuthor().substr(0, 23)
                  << std::setw(20) << book->getGenre().substr(0, 18)
                  << std::setw(10) << book->getAvailableCopies() << "\n";
    }
    std::cout << std::string(120, '=') << "\n";

    std::cout << "\nEnter ISBN to view details (or press Enter to go back): ";
    std::string isbn;
    std::getline(std::cin, isbn);

    if (!isbn.empty()) {
        Book* selectedBook = bookManager.findBookByISBN(isbn);
        if (selectedBook != nullptr) {
            displayBookDetails(selectedBook);
        }
    }
}

void MenuHandler::displayBookDetails(Book* book) {
    clearScreen();
    ui.displayHeader("Book Details");

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "ISBN:           " << book->getISBN() << "\n";
    std::cout << "Title:          " << book->getTitle() << "\n";
    std::cout << "Author:         " << book->getAuthor() << "\n";
    std::cout << "Publisher:      " << book->getPublisher() << "\n";
    std::cout << "Category:       " << book->getGenre() << "\n";
    std::cout << "Total Copies:   " << book->getTotalCopies() << "\n";
    std::cout << "Available:      " << book->getAvailableCopies() << "\n";
    std::cout << "Status:         " << (book->canBorrow() ? "Available" : "Not Available") << "\n";

    // Show reservation queue information
    int queueLength = reservationManager.getQueueLength(book->getISBN());
    if (queueLength > 0) {
        std::cout << "\n--- RESERVATION QUEUE ---\n";
        std::cout << "People waiting: " << queueLength << "\n";

        if (currentUser && !currentUser->getAdmin()) {
            auto userReservations = reservationManager.findByMemberID(currentUser->getMemberID());
            for (const auto* res : userReservations) {
                if (res->getISBN() == book->getISBN() && res->getIsActive()) {
                    int position = reservationManager.getQueuePosition(res->getReservationID());
                    std::cout << "Your position:  " << position;
                    if (position == 1) {
                        std::cout << " (NEXT IN LINE!)";
                    }
                    std::cout << "\n";
                    break;
                }
            }
        }
    }

    std::cout << std::string(60, '=') << "\n";

    pauseScreen();
}
