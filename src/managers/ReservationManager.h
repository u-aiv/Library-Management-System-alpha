#ifndef LIBRARY_MANAGEMENT_SYSTEM_RESERVATIONMANAGER_H
#define LIBRARY_MANAGEMENT_SYSTEM_RESERVATIONMANAGER_H

#include "../models/Reservation.h"
#include "../utils/FileHandler.h"
#include <string>
#include <vector>
#include <map>
#include <deque>

class ReservationManager {
private:
    std::vector<Reservation> reservations;
    std::string filePath;
    FileHandler fileHandler;

    // Reservation queues per ISBN (FIFO ordering)
    std::map<std::string, std::deque<std::string> > reservationQueues;

    // Data Persistence
    // Helper: Load reservation data from file
    void loadFromFile();

    // Helper: Save reservation data to file
    void saveToFile();

    // For Batch Operations
    bool autoSave = true;
    void setAutoSave(bool enable = true);
    bool isAutoSaveEnabled() const;

    // Helper: Check autosave flag to decide whether need to save
    void saveIfNeeded();

    // Queue Management Helpers
    void buildQueues();     // Build queues from loaded reservations
    void addToQueue(const std::string& isbn, const std::string& reservationID);
    bool removeFromQueue(const std::string& isbn, const std::string& reservationID);
    void rebuildQueueForISBN(const std::string& isbn);

public:
    // Constructor
    explicit ReservationManager(const std::string& filePath = "../data/reservations.csv");

    // CRUD
    bool addReservation(const Reservation& reservation);
    bool updateReservation(const Reservation& reservation);
    bool deleteReservation(const Reservation& reservation);

    // Search Functions
    Reservation* findByReservationID(const std::string &reservationID);
    std::vector<const Reservation*> findByMemberID(const std::string& memberID);
    std::vector<const Reservation*> findByISBN(const std::string& isbn);
    std::vector<const Reservation*> findByReservationDate(const std::string& reservationDate);
    std::vector<const Reservation*> findActiveReservations();

    // Reservation Functions
    std::string reserveBook(const std::string& memberID, const std::string& isbn);
    std::string cancelReservation(const std::string& reservationID);

    // Queue Management Functions
    std::string processNextReservation(const std::string& isbn);
    std::string getNextInQueue(const std::string& isbn) const;
    int getQueuePosition(const std::string& reservationID) const;
    int getQueueLength(const std::string& isbn) const;
    std::vector<std::string> getQueueForISBN(const std::string& isbn) const;
    bool hasActiveReservations(const std::string& isbn) const;

    // Getters
    const std::vector<Reservation>& getAllReservations() const;
    int getTotalReservations() const;
    int getActiveReservations() const;

    // Utility
    void reload();          // Reload from file
    void clearCache();      // Clear file handler cache
    bool isReservationIDExists(const std::string& reservationID) const;

    // Batch Operation (RAII)
    class BatchOperation {
    private:
        ReservationManager* reservationManager;
        bool originalAutoSave;
        bool active;

    public:
        explicit BatchOperation(ReservationManager& rmgr);
        ~BatchOperation();

        BatchOperation(BatchOperation&&) noexcept;
        BatchOperation(const BatchOperation&) = delete;
        BatchOperation& operator=(const BatchOperation&) = delete;
        BatchOperation& operator=(BatchOperation&&) = delete;
    };

    BatchOperation beginBatch();
};

#endif //LIBRARY_MANAGEMENT_SYSTEM_RESERVATIONMANAGER_H