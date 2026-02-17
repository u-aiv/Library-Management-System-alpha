// TransactionManager.cpp Implementation

#include "TransactionManager.h"
#include "BookManager.h"
#include "MemberManager.h"
#include "../config/Config.h"
#include "../utils/DateUtils.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

// Constructor
TransactionManager::TransactionManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "TransactionID,MemberID,ISBN,BorrowDate,DueDate,ReturnDate,RenewCount,Fine,IsReturned"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
}

// private: Helper: Load transaction data from file
void TransactionManager::loadFromFile() {
    transactions.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // Skip header (line 1)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                transactions.push_back(Transaction::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("Failed to load transactions file: " + std::string(e.what()));
    }
}

// private: Helper: Save transaction data to file
void TransactionManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("TransactionID,MemberID,ISBN,BorrowDate,DueDate,ReturnDate,RenewCount,Fine,IsReturned");

    // Add all transactions
    for (const auto& transaction : transactions) {
        lines.push_back(transaction.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to save transactions file: " + std::string(e.what()));
    }
}

// private: Helper: Check autoSave flag to decide whether need to save
void TransactionManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool TransactionManager::isAutoSaveEnabled() const {
    return autoSave;
}

// Helper: Check autoSave flag to decide whether need to save
void TransactionManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// private: Helper: Generate Transaction ID
std::string TransactionManager::generateTransactionID() const {
    std::string currentDate = DateUtils::getCurrentDate();
    std::string year = currentDate.substr(0, 4);
    int month = std::stoi(currentDate.substr(5, 2));
    int season = (month - 1) / 3 + 1;

    std::string prefix = "T" + year + std::to_string(season);
    int maxSeq = 0;
    for (const auto& transaction : transactions) {
        const std::string& id = transaction.getTransactionID();
        if (id.rfind(prefix, 0) != 0) {
            continue;
        }
        if (id.size() < prefix.size() + 5) {
            continue;
        }

        std::string suffix = id.substr(prefix.size());
        bool allDigits = std::all_of(suffix.begin(), suffix.end(),
            [](unsigned char c) { return std::isdigit(c); });
        if (!allDigits) {
            continue;
        }
        int seq = std::stoi(suffix);
        if (seq > maxSeq) {
            maxSeq = seq;
        }
    }

    std::ostringstream oss;
    oss << prefix << std::setw(5) << std::setfill('0') << (maxSeq + 1);
    return oss.str();
}

int TransactionManager::getActiveCountForMember(const std::string& memberID) const {
    int count = 0;
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID && !transaction.haveReturned()) {
            count++;
        }
    }
    return count;
}

// Add a new Transaction
bool TransactionManager::addTransaction(const Transaction& transaction) {
    // Check if TransactionID already exists
    if (isTransactionIDExists(transaction.getTransactionID())) {
        return false;
    }
    transactions.push_back(transaction);
    saveIfNeeded();
    return true;
}

// Update existing transaction
bool TransactionManager::updateTransaction(const Transaction& transaction) {
    Transaction *existingTransaction = findByTransactionID(transaction.getTransactionID());

    if (existingTransaction == nullptr) {
        return false;
    }
    *existingTransaction = transaction;
    saveIfNeeded();
    return true;
}

// Delete transaction by transactionID
bool TransactionManager::deleteTransaction(const std::string& transactionID) {
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [&](const Transaction& trans) { return trans.getTransactionID() == transactionID; });

    if (it != transactions.end()) {
        transactions.erase(it);
        saveIfNeeded();
        return true;
    }
    return false;
}

// Find transaction by transactionID
Transaction* TransactionManager::findByTransactionID(const std::string& transactionID) {
    for (auto& transaction : transactions) {
        if (transaction.getTransactionID() == transactionID) {
            return &transaction;
        }
    }
    return nullptr;
}

// Find transactions by memberID
std::vector<const Transaction*> TransactionManager::findByMemberID(const std::string& memberID) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Find transactions by ISBN
std::vector<const Transaction*> TransactionManager::findByISBN(const std::string& isbn) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getISBN() == isbn) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Find transactions by BorrowDate
std::vector<const Transaction*> TransactionManager::findByBorrowDate(const std::string& borrowDate) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getBorrowDate() == borrowDate) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Find transactions by DueDate
std::vector<const Transaction*> TransactionManager::findByDueDate(const std::string& dueDate) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getDueDate() == dueDate) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Find transactions by ReturnDate
std::vector<const Transaction*> TransactionManager::findByReturnDate(const std::string& returnDate) {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (transaction.getReturnDate() == returnDate) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Find active transactions
std::vector<const Transaction*> TransactionManager::findActiveTransactions() {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned()) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Find overdue transactions
std::vector<const Transaction*> TransactionManager::findOverdueTransactions() {
    std::vector<const Transaction*> results;

    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned() && transaction.isOverdue()) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Borrow a book
std::string TransactionManager::borrowBook(const std::string& memberID, const std::string& isbn) {
    MemberManager memberManager(Config::MEMBERS_FILE);
    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr || member->isExpired()) {
        return "0";
    }
    if (getActiveCountForMember(memberID) >= member->getMaxBooksAllowed()) {
        return "0";
    }

    BookManager bookManager(Config::BOOKS_FILE);
    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr || !book->canBorrow()) {
        return "0";
    }

    std::string currentDate = DateUtils::getCurrentDate();
    std::string dueDate = DateUtils::addDays(currentDate, 14);
    std::string transactionID = generateTransactionID();

    Transaction transaction(transactionID, memberID, isbn, currentDate, dueDate, "", 0, 0.0, false);
    if (!addTransaction(transaction)) {
        return "0";
    }

    if (!bookManager.borrowBook(isbn)) {
        deleteTransaction(transactionID);
        return "0";
    }

    return transactionID;
}

std::string TransactionManager::borrowBook(MemberManager& memberManager, BookManager& bookManager,
                                           const std::string& memberID, const std::string& isbn) {
    Member* member = memberManager.findMemberByID(memberID);
    if (member == nullptr || member->isExpired()) {
        return "0";
    }
    if (getActiveCountForMember(memberID) >= member->getMaxBooksAllowed()) {
        return "0";
    }

    Book* book = bookManager.findBookByISBN(isbn);
    if (book == nullptr || !book->canBorrow()) {
        return "0";
    }

    std::string currentDate = DateUtils::getCurrentDate();
    std::string dueDate = DateUtils::addDays(currentDate, 14);
    std::string transactionID = generateTransactionID();

    Transaction transaction(transactionID, memberID, isbn, currentDate, dueDate, "", 0, 0.0, false);
    if (!addTransaction(transaction)) {
        return "0";
    }

    if (!bookManager.borrowBook(isbn)) {
        deleteTransaction(transactionID);
        return "0";
    }

    return transactionID;
}

// Return a book with transactionID
bool TransactionManager::returnBook(const std::string& transactionID) {
    Transaction* transaction = findByTransactionID(transactionID);
    if (transaction == nullptr || transaction->haveReturned()) {
        return false;
    }

    BookManager bookManager(Config::BOOKS_FILE);
    if (!bookManager.returnBook(transaction->getISBN())) {
        return false;
    }

    transaction->returnBook();
    saveIfNeeded();
    return true;
}

bool TransactionManager::returnBook(BookManager& bookManager, const std::string& transactionID) {
    Transaction* transaction = findByTransactionID(transactionID);
    if (transaction == nullptr || transaction->haveReturned()) {
        return false;
    }

    if (!bookManager.returnBook(transaction->getISBN())) {
        return false;
    }

    transaction->returnBook();
    saveIfNeeded();
    return true;
}

// Return a book with memberID and ISBN
bool TransactionManager::returnBook(const std::string& memberID, const std::string& isbn) {
    const Transaction *transaction = nullptr;
    for (const auto& t : transactions) {
        if ( t.getUserID() == memberID &&
             t.getISBN() == isbn &&
             !t.haveReturned()) {
            transaction = &t;
            break;
        }
    }

    if (transaction == nullptr) {
        return false;
    }

    return returnBook(transaction->getTransactionID());
}

bool TransactionManager::returnBook(BookManager& bookManager, const std::string& memberID, const std::string& isbn) {
    const Transaction *transaction = nullptr;
    for (const auto& t : transactions) {
        if ( t.getUserID() == memberID &&
             t.getISBN() == isbn &&
             !t.haveReturned()) {
            transaction = &t;
            break;
        }
    }

    if (transaction == nullptr) {
        return false;
    }

    return returnBook(bookManager, transaction->getTransactionID());
}

// Renew a book with transactionID
bool TransactionManager::renewBook(const std::string& transactionID) {
    Transaction* transaction = findByTransactionID(transactionID);
    if (transaction == nullptr || transaction->haveReturned()) {
        return false;
    }

    if (!transaction->canRenew()) {
        return false;
    }

    transaction->renewBook();
    saveIfNeeded();
    return true;
}

bool TransactionManager::renewBook(const std::string& memberID, const std::string& isbn) {
    const Transaction *transaction = nullptr;
    for (const auto& t : transactions) {
        if ( t.getUserID() == memberID &&
             t.getISBN() == isbn &&
             !t.haveReturned()) {
            transaction = &t;
            break;
             }
    }

    if (transaction == nullptr) {
        return false;
    }

    return renewBook(transaction->getTransactionID());
}

// Get member borrowing history
std::vector<const Transaction*> TransactionManager::getMemberHistory(const std::string& memberID) {
    return findByMemberID(memberID);
}

// Get active transactions for a member
std::vector<const Transaction*> TransactionManager::getActiveTransactions(const std::string& memberID) {
    std::vector<const Transaction*> results;
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID && !transaction.haveReturned()) {
            results.push_back(&transaction);
        }
    }
    return results;
}

// Get all overdue transactions
std::vector<const Transaction*> TransactionManager::getOverdueTransactions() {
    return findOverdueTransactions();
}

// Get all transactions
const std::vector<Transaction>& TransactionManager::getAllTransactions() const {
    return transactions;
}

// Get total number of transactions
int TransactionManager::getTotalTransactions() const {
    return static_cast<int>(transactions.size());
}

// Get count of active transactions
int TransactionManager::getActiveTransactionsCount() const {
    int count = 0;
    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned()) {
            count++;
        }
    }
    return count;
}

// Get count of overdue transactions
int TransactionManager::getOverdueTransactionsCount() const {
    int count = 0;
    for (const auto& transaction : transactions) {
        if (!transaction.haveReturned() && transaction.isOverdue()) {
            count++;
        }
    }
    return count;
}

// Reload from file
void TransactionManager::reload() {
    loadFromFile();
}

// Clear file handler cache
void TransactionManager::clearCache() {
    fileHandler.clearCache();
}

// Check if TransactionID exists
bool TransactionManager::isTransactionIDExists(const std::string& transactionID) const {
    return std::find_if(transactions.begin(), transactions.end(),
        [&](const Transaction& transaction) { return transaction.getTransactionID() == transactionID; }) != transactions.end();
}

// Batch Operations (RAII)
TransactionManager::BatchOperation::BatchOperation(TransactionManager& tmgr) :
                    transactionManager(&tmgr), originalAutoSave(tmgr.autoSave), active(true) {
    tmgr.setAutoSave(false);
}

TransactionManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    transactionManager(other.transactionManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

TransactionManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        transactionManager->saveToFile();
        transactionManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "Error when trying to save transactions during batch operations." << std::endl;
    }
}

TransactionManager::BatchOperation TransactionManager::beginBatch() {
    return BatchOperation(*this);
}
