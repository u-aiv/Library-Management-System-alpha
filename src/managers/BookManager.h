#ifndef LIBRARY_MANAGEMENT_SYSTEM_BOOKMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_BOOKMANAGER_H

#include "../models/Book.h"
#include "../utils/FileHandler.h"
#include <string>
#include <vector>

class BookManager {
private:
    std::vector<Book> books;
    std::string filePath;
    FileHandler fileHandler;

    // Data Persistence
    // Helper: Load books data from file
    void loadFromFile();

    // Helper: Save books data to file
    void saveToFile();

    // For Batch Operations
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // Helper: Check autoSave flag to decide whether need to save
    void saveIfNeeded();


public:
    // Constructor
    explicit BookManager(const std::string& filePath = "../data/books.csv");

    // CRUD (Create, Read, Update, Delete)
    bool addBook(const Book& book);
    bool updateBook(const Book& book);
    bool deleteBook(const std::string& isbn);

    // Search Functions
    Book* findBookByISBN(const std::string &isbn);
    std::vector<const Book*> findByTitle(const std::string& title, int matchMode = 0) const;
    std::vector<const Book*> findByAuthor(const std::string& author, int matchMode = 0) const;
    std::vector<const Book*> findByPublisher(const std::string& publisher, int matchMode = 0) const;
    std::vector<const Book*> findByGenre(const std::string& genre, int matchMode = 0) const;
    std::vector<const Book*> findAvailableBooks() const;

    // Borrow / Return Operations
    bool borrowBook(const std::string& isbn);
    bool returnBook(const std::string& isbn);

    // Getters
    std::vector<Book> getAllBooks() const;
    int getTotalBooks() const;
    int getAvailableCount() const;
    // std::vector<Book*> getAvailableBooks() const;

    // Utility
    void reload();          // Reload from file
    void clearCache();      // Clear file handler cache
    bool isISBNExists(const std::string& isbn) const;

    // Batch Operation (RAII)
    class BatchOperation {
    private:
        BookManager* bookManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(BookManager& bmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_BOOKMANAGER_H