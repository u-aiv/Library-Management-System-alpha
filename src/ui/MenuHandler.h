#ifndef LIBRARY_MANAGEMENT_SYSTEM_MENUHANDLER_H
#define LIBRARY_MANAGEMENT_SYSTEM_MENUHANDLER_H

#include <string>
#include <vector>

// Forward declarations to avoid circular dependencies
class BookManager;
class MemberManager;
class TransactionManager;
class ReservationManager;
class RecommendationManager;
class BackupManager;
class ReportManager;
class Member;
class UI;

class MenuHandler {
private:
    // Manager references
    BookManager& bookManager;
    MemberManager& memberManager;
    TransactionManager& transactionManager;
    ReservationManager& reservationManager;
    RecommendationManager& recommendationManager;
    UI& ui;

    // Current user session
    Member* currentUser;
    bool isRunning;

    // Menu display helpers
    void displayWelcomeScreen();
    void displayMemberMenu();
    void displayAdminMenu();

    // Member menu handlers
    void handleSearchBooks();
    void handleBorrowBook();
    void handleReturnBook();
    void handleRenewBook();
    void handleReserveBook();
    void handleViewBorrowedBooks();
    void handleViewBorrowingHistory();
    void handleViewReservations();
    void handleViewProfile();

    // Admin menu handlers
    void handleManageBooks();
    void handleManageMembers();
    void handleManageTransactions();
    void handleViewOverdueBooks();
    void handleManageReservations();
    void handleGenerateReports();
    void handleBackupRestore();
    void handleSystemSettings();

    // Sub-menu handlers for admin - books
    void handleAddBook();
    void handleUpdateBook();
    void handleDeleteBook();
    void handleViewAllBooks();

    // Sub-menu handlers for admin - members
    void handleAddMember();
    void handleUpdateMember();
    void handleSearchReservations();
    void handleDeleteMember();
    void handleViewAllMembers();

    // Sub-menu handlers for admin - transactions
    void handleViewAllTransactions();
    void handleViewActiveTransactions();
    void handleManualReturn();

    // Sub-menu handlers for admin - reservations
    void handleAddReservation();
    void handleUpdateReservation();
    void handleDeleteReservation();
    void handleViewAllReservation();
    void handleViewActiveReservation();

    // Sub-menu handlers for admin - recommendations
    void handleViewRecommendations();

    // Sub-menu handlers for admin - reports
    void handleGenerateInventoryReport();
    void handleGenerateMemberReport();
    void handleGenerateTransactionReport();
    void handleGenerateOverdueReport();
    void handleGenerateStatisticsReport();

    // Sub-menu handlers for admin - backup/restore
    void handleBackupData();
    void handleRestoreData();

    // Sub-menu handlers for admin - system settings
    void handleChangeUIMode();
    void handleChangeBorrowPeriod();
    void handleChangeFineSettings();

    // Utility functions
    std::string promptForInput(const std::string& prompt, bool canCancel = true);
    int promptForInt(const std::string& prompt, int min, int max, bool canCancel = true);
    double promptForDouble(const std::string& prompt, bool canCancel = true);
    bool confirmAction(const std::string& message);
    void displayMessage(const std::string& message, const std::string& type = "info");
    void pauseScreen();
    void clearScreen();

    // Search helpers
    void performBookSearch(const std::string& searchType);
    void displaySearchResults(const std::vector<const class Book*>& results);
    void displayBookDetails(Book* book);

public:
    // Constructor
    MenuHandler(BookManager& bm, MemberManager& mm,
                TransactionManager& tm, ReservationManager& rsm,
                RecommendationManager& rcm, UI& ui);

    // Destructor
    ~MenuHandler();

    bool login();
    bool logout();
    void run();
    void exit();
    bool isApplicationRunning() const;

    // Get current logged-in user
    Member* getCurrentUser() const;
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_MENUHANDLER_H
