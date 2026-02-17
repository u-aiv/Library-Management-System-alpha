#ifndef LIBRARY_MANAGEMENT_SYSTEM_TRANSACTIONMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_TRANSACTIONMANAGER_H

#include "../models/Transaction.h"
#include "../utils/FileHandler.h"
#include <string>
#include <vector>

// Forward declarations to avoid extra includes in header
class MemberManager;
class BookManager;

class TransactionManager {
private:
    std::vector<Transaction> transactions;
    std::string filePath;
    FileHandler fileHandler;

    // Data Persistence
    void loadFromFile();
    void saveToFile();

    // For Batch Operations
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // Helper: Check autoSave flag to decide whether need to save
    void saveIfNeeded();

    // Helper: Generate Transaction ID
    std::string generateTransactionID() const;

    // Helper: Count active transactions for a member
    int getActiveCountForMember(const std::string& memberID) const;

public:
    // Constructor
    explicit TransactionManager(const std::string& filePath = "../data/transactions.csv");

    // CRUD
    bool addTransaction(const Transaction& transaction);
    bool updateTransaction(const Transaction& transaction);
    bool deleteTransaction(const std::string& transactionID);

    // Search Functions
    Transaction* findByTransactionID(const std::string& transactionID);
    std::vector<const Transaction*> findByMemberID(const std::string& memberID);
    std::vector<const Transaction*> findByISBN(const std::string& isbn);
    std::vector<const Transaction*> findByBorrowDate(const std::string& borrowDate);
    std::vector<const Transaction*> findByDueDate(const std::string& dueDate);
    std::vector<const Transaction*> findByReturnDate(const std::string& returnDate);
    std::vector<const Transaction*> findActiveTransactions();
    std::vector<const Transaction*> findOverdueTransactions();

    // Transaction Functions
    std::string borrowBook(const std::string& memberID, const std::string& isbn);
    std::string borrowBook(MemberManager& memberManager, BookManager& bookManager,
                           const std::string& memberID, const std::string& isbn);
    bool returnBook(const std::string& transactionID);
    bool returnBook(BookManager& bookManager, const std::string& transactionID);
    bool returnBook(const std::string& memberID, const std::string& isbn);
    bool returnBook(BookManager& bookManager, const std::string& memberID, const std::string& isbn);
    bool renewBook(const std::string& transactionID);
    bool renewBook(const std::string& memberID, const std::string& isbn);

    // History & Reports
    std::vector<const Transaction*> getMemberHistory(const std::string& memberID);
    std::vector<const Transaction*> getActiveTransactions(const std::string& memberID);
    std::vector<const Transaction*> getOverdueTransactions();

    // Getters
    const std::vector<Transaction>& getAllTransactions() const;
    int getTotalTransactions() const;
    int getActiveTransactionsCount() const;
    int getOverdueTransactionsCount() const;

    // Utility
    void reload();          // Reload from file
    void clearCache();      // Clear file handler cache
    bool isTransactionIDExists(const std::string& transactionID) const;

    // Batch Operation (RAII)
    class BatchOperation {
    private:
        TransactionManager* transactionManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(TransactionManager& tmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_TRANSACTIONMANAGER_H
