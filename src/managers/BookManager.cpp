// BookManager.h Instance

#include "BookManager.h"
#include "../utils/FileHandler.h"
#include <algorithm>
#include <iostream>
#include <utility>

// Constructor
BookManager::BookManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "ISBN,Title,Author,Publisher,Genre,TotalCopies,AvailableCopies,IsReserved"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
}

// private: Helper: Load books data from file
void BookManager::loadFromFile() {
    books.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // Skip header (line 1)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                books.push_back(Book::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("Failed to load books file: " + std::string(e.what()));
    }
}

// private: Helper: Save books data to file
void BookManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("ISBN,Title,Author,Publisher,Genre,TotalCopies,AvailableCopies,IsReserved");

    // Add all books
    for (const auto& book : books) {
        lines.push_back(book.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to save books file: " + std::string(e.what()));
    }
}

// private: Helper: Check autoSave flag to decide whether need to save
void BookManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// Add a new book
bool BookManager::addBook(const Book& book) {
    // Check if ISBN already exists
    if (isISBNExists(book.getISBN())) {
        return false;
    }
    books.push_back(book);
    saveIfNeeded();
    return true;
}

// Delete book by ISBN
bool BookManager::deleteBook(const std::string& isbn) {
    auto it = std::find_if(books.begin(), books.end(),
        [&](const Book& book) { return book.getISBN() == isbn; });

    if (it != books.end()) {
        books.erase(it);
        saveIfNeeded();
        return true;
    }
    return false;
}

// Update existing bool
bool BookManager::updateBook(const Book& book) {
    Book *existingBook = findBookByISBN(book.getISBN());

    if (existingBook == nullptr) {
        return false;
    }
    *existingBook = book;
    saveIfNeeded();
    return true;
}

// Find book by ISBN
Book* BookManager::findBookByISBN(const std::string &isbn) {
    for (auto& book : books) {
        if (book.getISBN() == isbn) {
            return &book;
        }
    }
    return nullptr;
}

// Find books Template
// matchMode = 0 --> exact matching (case-sensitive) (default)
// matchMode = 1 --> fuzzy matching (case-insensitive)
template<typename Getter>
std::vector<const Book*> findByField(
    const std::vector<Book>& books,
    const std::string& key,
    Getter getter,
    int matchMode = 0
    ) {
        if (matchMode != 0 && matchMode != 1) {
            throw std::runtime_error("Invalid match mode");
        }

        std::vector<const Book*> results;

        if (matchMode == 0) {
            for (const auto& book : books) {
                if ((book.*getter)() == key) {
                    results.push_back(&book);
                }
            }
        }
        else {
            std::string lowerKey = key;
            std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

            for (const auto& book : books) {
                std::string bookKey = (book.*getter)();
                std::transform(bookKey.begin(), bookKey.end(), bookKey.begin(), ::tolower);

                if (bookKey.find(lowerKey) != std::string::npos)
                    results.push_back(&book);
            }
        }
        return results;
}

// Find book by title
std::vector<const Book*> BookManager::findByTitle(const std::string& title, int matchMode) const {
    return findByField(books, title,&Book::getTitle , matchMode);
}

// Find book by author
std::vector<const Book*> BookManager::findByAuthor(const std::string& author, int matchMode) const {
    return findByField(books, author, &Book::getAuthor, matchMode);
}

// Find book by publisher
std::vector<const Book*> BookManager::findByPublisher(const std::string& publisher, int matchMode) const {
    return findByField(books, publisher, &Book::getPublisher, matchMode);
}

// Find book by genre
std::vector<const Book*> BookManager::findByGenre(const std::string& genre, int matchMode) const {
    return findByField(books, genre, &Book::getGenre, matchMode);
}

// Find available book
std::vector<const Book*> BookManager::findAvailableBooks() const {
    std::vector<const Book*> results;

    for (const auto& book : books) {
        if (book.canBorrow()) {
            results.push_back(&book);
        }
    }
    return results;
}

// Borrow a book
bool BookManager::borrowBook(const std::string& isbn) {         // -> TransactionManager
    auto* book = findBookByISBN(isbn);

    if (book == nullptr) {
        return false;       // Book not found
    }

    if (!book->canBorrow()) {
        return false;       // Book unavailable
    }

    book->borrowBook();
    saveIfNeeded();
    return true;
}

// Return a book
bool BookManager::returnBook(const std::string& isbn) {         // -> TransactionManager
    auto* book = findBookByISBN(isbn);

    if (book == nullptr) {
        return false;       // Book not found
    }

    book->returnBook();
    saveIfNeeded();
    return true;
}

// Get all books
std::vector<Book> BookManager::getAllBooks() const {
    return books;
}

// Get total number of books
int BookManager::getTotalBooks() const {
    return static_cast<int>(books.size());
}

// Get count of available books
int BookManager::getAvailableCount() const {
    int count = 0;
    for (const auto& book : books) {
        if (book.canBorrow()) {
            count++;
        }
    }
    return count;
}

// Reload from file
void BookManager::reload() {
    loadFromFile();
}

// Clear file handler cache
void BookManager::clearCache() {
    fileHandler.clearCache();
}

// Check if ISBN exists
bool BookManager::isISBNExists(const std::string& isbn) const {
    return std::find_if(books.begin(), books.end(),
        [&](const Book& book) { return book.getISBN() == isbn; }) != books.end();
}

// Batch Operations (RAII)
void BookManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool BookManager::isAutoSaveEnabled() const {
    return autoSave;
}

BookManager::BatchOperation::BatchOperation(BookManager& bmgr) :
                    bookManager(&bmgr), originalAutoSave(bmgr.autoSave), active(true) {
    bmgr.setAutoSave(false);
}

BookManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    bookManager(other.bookManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

BookManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        bookManager->saveToFile();
        bookManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "Error when trying to save book during batch operations." << std::endl;
    }
}

BookManager::BatchOperation BookManager::beginBatch() {
    return BatchOperation(*this);
}
