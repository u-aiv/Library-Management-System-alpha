#ifndef LIBRARY_MANAGEMENT_SYSTEM_TRANSACTION_H
#define LIBRARY_MANAGEMENT_SYSTEM_TRANSACTION_H

#include <string>

class Transaction {
private:
    std::string transactionID;      // Transaction ID (auto generated)
    std::string userID;             // User's ID
    std::string isbn;               // Book's ISBN
    std::string borrowDate;         // Borrow Date (YYYY-MM-DD)
    std::string dueDate;            // Due Date (YYYY-MM-DD)
    std::string returnDate;         // Returned Date (YYYY-MM-DD)
    int renewCount{};               // The count of renewal
    double fine{};                  // The amount of fine
    bool isReturned{};              // true = Returned, false = Unreturned

public:
    // Constructor
    Transaction(std::string transactionID, std::string userID, std::string isbn,
                std::string borrowDate, std::string dueDate, std::string returnDate,
                int renewCount = 0, double fine = 0.0, bool isReturned = false):
                transactionID(std::move(transactionID)), userID(std::move(userID)),
                isbn(std::move(isbn)), borrowDate(std::move(borrowDate)),
                dueDate(std::move(dueDate)), returnDate(std::move(returnDate)),
                renewCount(renewCount), fine(fine), isReturned(isReturned){}
    Transaction() = default;

    // Getters and Setters
    std::string getTransactionID() const;
    std::string getUserID() const;
    std::string getISBN() const;
    std::string getBorrowDate() const;
    std::string getDueDate() const;
    std::string getReturnDate() const;
    int getRenewCount() const;
    double getFine() const;
    bool haveReturned() const;

    // Business Logic Methods
    bool isOverdue() const;
    double calculateFine() const;
    bool canRenew() const;
    void renewBook();
    void returnBook();

    // Utility
    std::string toCSV() const;
    static Transaction fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_TRANSACTION_H
