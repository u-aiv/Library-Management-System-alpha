#ifndef LIBRARY_MANAGEMENT_SYSTEM_RESERVATION_H
#define LIBRARY_MANAGEMENT_SYSTEM_RESERVATION_H

#include <string>

class Reservation {
private:
    std::string reservationID;          // Reservation ID (auto generated)
    std::string memberID;               // Member's ID who reserved
    std::string isbn;                   // Book's ISBN which is reserved
    std::string reservationDate;        // Reservation Date
    bool isActive{};                    // Is available?

public:
    // Constructor
    Reservation(std::string reservationID, std::string memberID, std::string isbn,
                std::string reservationDate, bool isActive = true) :
                reservationID(std::move(reservationID)), memberID(std::move(memberID)),
                isbn(std::move(isbn)), reservationDate(std::move(reservationDate)),
                isActive(isActive){}
    Reservation() = default;

    // Getters and Setters
    std::string getReservationID() const;
    std::string getMemberID() const;
    std::string getISBN() const;
    std::string getReservationDate() const;
    bool getIsActive() const;

    // Business Logic
    void cancelReservation();

    // Utility
    std::string toCSV() const;
    static Reservation fromCSV(const std::string& csvLine);
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_RESERVATION_H