// Book header

#ifndef LIBRARY_MANAGEMENT_SYSTEM_BOOK_H
#define LIBRARY_MANAGEMENT_SYSTEM_BOOK_H

#include <string>
#include <utility>              // for std::move()

class Book {
private:
    std::string isbn;                // ISBN number, key
    std::string title;               // Book Title
    std::string author;              // Book Author
    std::string publisher;           // Book Publisher
    std::string genre;               // One of the default 5 genres

    int totalCopies{};                // Book Total Copies Number
    int availableCopies{};            // Book Available Copies Number
    bool isReserved{};                // true = Reserved, false = Unreserved

public:
    // Constructor
    Book(std::string isbn, std::string title, std::string author, std::string publisher, std::string genre,
     int totalCopies = 0, int availableCopies = 0, bool isReserved = false)
    : isbn(std::move(isbn)), title(std::move(title)), author(std::move(author)), publisher(std::move(publisher)),
      genre(std::move(genre)), totalCopies(totalCopies), availableCopies(availableCopies), isReserved(isReserved) {}
    Book() = default;

    // Getters and Setters
    std::string getISBN() const;
    std::string getTitle() const;
    std::string getPublisher() const;
    std::string getAuthor() const;
    std::string getGenre() const;
    int getTotalCopies() const;
    int getAvailableCopies() const;
    bool canBorrow() const;

    void borrowBook();
    void returnBook();
    void setReserved(bool status);

    // CSV function
    std::string toCSV() const;
    static Book fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_BOOK_H