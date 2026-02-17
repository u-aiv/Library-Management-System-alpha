// ReservationManager.cpp Implementation

#include "ReservationManager.h"
#include "MemberManager.h"
#include "BookManager.h"
#include "../utils/DateUtils.h"
#include "../utils/Validator.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>


// Constructor
ReservationManager::ReservationManager(const std::string& filePath)
    : filePath(filePath), fileHandler() {

    if (!fileHandler.isFileExist(filePath)) {
        fileHandler.createFileIfNotExist(filePath);
        std::vector<std::string> fileHeader = {
            "ReservationID,MemberID,ISBN,ReservationDate,IsActive"
        };
        fileHandler.writeCSV(filePath, fileHeader);
    }
    loadFromFile();
    buildQueues();
}

// private: Helper: Load reservation data from file
void ReservationManager::loadFromFile() {
    reservations.clear();

    try {
        auto lines = fileHandler.readCSV(filePath);

        // Skip header (line 1)
        for (size_t i = 1; i < lines.size(); i++) {
            if (!lines[i].empty()) {
                reservations.push_back(Reservation::fromCSV(lines[i]));
            }
        }
    }
    catch (std::exception& e) {
        throw std::runtime_error("Failed to load reservations file: " + std::string(e.what()));
    }
}

// private: Helper: Save reservation data to file
void ReservationManager::saveToFile() {
    std::vector<std::string> lines;

    lines.emplace_back("ReservationID,MemberID,ISBN,ReservationDate,IsActive");

    // Add all reservations
    for (const auto& reservation : reservations) {
        lines.push_back(reservation.toCSV());
    }

    try {
        fileHandler.writeCSV(filePath, lines);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to save reservations file: " + std::string(e.what()));
    }
}

// private: Helper: Check autosave flag to decide whether need to save
void ReservationManager::setAutoSave(bool enable) {
    autoSave = enable;
}

bool ReservationManager::isAutoSaveEnabled() const {
    return autoSave;
}

// Helper: Check autoSave flag to decide whether need to save
void ReservationManager::saveIfNeeded() {
    if (autoSave) {
        saveToFile();
    }
}

// Build queues from loaded reservations
void ReservationManager::buildQueues() {
    reservationQueues.clear();

    // Sort reservations by date for each ISBN
    std::map<std::string,std::vector<Reservation*> > isbnGroups;

    for (auto& reservation : reservations) {
        if (reservation.getIsActive()) {
            isbnGroups[reservation.getISBN()].push_back(&reservation);
        }
    }

    // Build ordered queue for each ISBN
    for (auto& pair : isbnGroups) {
        const auto& isbn = pair.first;
        auto& reservationGroup = pair.second;

        // Sort by reservation date (FIFO)
        std::sort(reservationGroup.begin(), reservationGroup.end(),
            [](const Reservation* a, const Reservation* b) {
                return a->getReservationDate() < b->getReservationDate();
            });

        // Add to queue in order
        for (const auto& reservation : reservationGroup) {
            reservationQueues[isbn].push_back(reservation->getReservationID());
        }
    }
}

// Add a reservation to the queue for its ISBN
void ReservationManager::addToQueue(const std::string& isbn, const std::string& reservationID) {
    reservationQueues[isbn].push_back(reservationID);
}

// Remove a reservation from the queue
bool ReservationManager::removeFromQueue(const std::string &isbn, const std::string &reservationID) {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end()) {
        return false;
    }

    auto& queue = it->second;
    auto queueIt = std::find(queue.begin(), queue.end(), reservationID);

    if (queueIt != queue.end()) {
        queue.erase(queueIt);

        // Remove the ISBN entry if queue is empty now
        if (queue.empty()) {
            reservationQueues.erase(it);
        }
        return true;
    }
    return false;
}

// Rebuild queue for a specific ISBN (after bulk operations)
void ReservationManager::rebuildQueueForISBN(const std::string &isbn) {
    reservationQueues.erase(isbn);

    // Collect active reservations for this ISBN
    std::vector<Reservation*> isbnReservations;
    for (auto& reservation : reservations) {
        if (reservation.getISBN() == isbn && reservation.getIsActive()) {
            isbnReservations.push_back(&reservation);
        }
    }

    // Sort by date
    std::sort(isbnReservations.begin(), isbnReservations.end(),
        [](const Reservation* a, const Reservation* b) {
            return a->getReservationDate() < b->getReservationDate();
        });

    // Rebuild queue
    for (const auto* reservation : isbnReservations) {
        reservationQueues[isbn].push_back(reservation->getReservationID());
    }
}

// Process the next reservation in queue when book becomes available
std::string ReservationManager::processNextReservation(const std::string& isbn) {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end() || it->second.empty()) {
        return "";      // No reservations in queue
    }

    std::string nextReservationID = it->second.front();
    return nextReservationID;
}

// Get the next reservation ID in queue without removing it
std::string ReservationManager::getNextInQueue(const std::string& isbn) const {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end() || it->second.empty()) {
        return "";
    }
    return it->second.front();
}

// Get a position of a reservation in its queue (1-indexed)
int ReservationManager::getQueuePosition(const std::string& reservationID) const {
    const Reservation* res = nullptr;
    for (const auto& reservation : reservations) {
        if (reservation.getReservationID() == reservationID) {
            res = &reservation;
            break;
        }
    }

    if (res == nullptr || !res->getIsActive()) {
        return -1;      // Not found or inactive
    }

    // Find position in queue
    auto it = reservationQueues.find(res->getISBN());
    if (it == reservationQueues.end()) {
        return -1;
    }

    const auto& queue = it->second;
    auto queueIt = std::find(queue.begin(), queue.end(), reservationID);

    if (queueIt == queue.end()) {
        return -1;
    }

    return static_cast<int>(std::distance(queue.begin(), queueIt)) + 1;
}

// Get the length of the queue for a specific ISBN
int ReservationManager::getQueueLength(const std::string &isbn) const {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end()) {
        return 0;
    }
    return static_cast<int>(it->second.size());
}

// Get all reservationIDs in queue for an ISBN (ordered)
std::vector<std::string> ReservationManager::getQueueForISBN(const std::string &isbn) const {
    auto it = reservationQueues.find(isbn);
    if (it == reservationQueues.end()) {
        return {};
    }
    return {it->second.begin(), it->second.end()};
}

// Check if there are active reservation for an ISBN
bool ReservationManager::hasActiveReservations(const std::string &isbn) const {
    auto it = reservationQueues.find(isbn);
    return it != reservationQueues.end() && !it->second.empty();
}

// Add a new Reservation
bool ReservationManager::addReservation(const Reservation& reservation) {
    // Check if ReservationID already exists
    if (isReservationIDExists(reservation.getReservationID())) {
        return false;
    }
    reservations.push_back(reservation);

    if (reservation.getIsActive()) {
        addToQueue(reservation.getISBN(), reservation.getReservationID());
    }

    if (autoSave) {
        saveToFile();
    }
    return true;
}

// Update existing reservation
bool ReservationManager::updateReservation(const Reservation& reservation) {
    Reservation *existingReservation = findByReservationID(reservation.getReservationID());

    if (existingReservation == nullptr) {
        return false;
    }

    bool wasActive = existingReservation->getIsActive();
    bool isActive = reservation.getIsActive();
    std::string isbn = existingReservation->getISBN();

    *existingReservation = reservation;

    // Update queue based on status change
    if (wasActive && !isActive) {
        removeFromQueue(isbn, reservation.getReservationID());
    } else if (!wasActive && isActive) {
        addToQueue(isbn, reservation.getReservationID());
    }

    if (autoSave) {
        saveToFile();
    }
    return true;
}

// Delete Reservation
bool ReservationManager::deleteReservation(const Reservation& reservation) {
    auto it = std::find_if(reservations.begin(), reservations.end(),
        [&](const Reservation& res) { return res.getReservationID() == reservation.getReservationID(); });

    if (it != reservations.end()) {
        if (it->getIsActive()) {
            removeFromQueue(it->getISBN(), reservation.getReservationID());
        }

        reservations.erase(it);
        if (autoSave) {
            saveToFile();
        }
        return true;
    }
    return false;
}

// Find reservation by ReservationID
Reservation* ReservationManager::findByReservationID(const std::string &reservationID) {
    for (auto& reservation : reservations) {
        if (reservation.getReservationID() == reservationID) {
            return &reservation;
        }
    }
    return nullptr;
}

// Find reservations by MemberID
std::vector<const Reservation*> ReservationManager::findByMemberID(const std::string& memberID) {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getMemberID() == memberID) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// Find reservations by ISBN
std::vector<const Reservation*> ReservationManager::findByISBN(const std::string& isbn) {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getISBN() == isbn) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// Find reservations by ReservationDate
std::vector<const Reservation*> ReservationManager::findByReservationDate(const std::string& reservationDate) {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getReservationDate() == reservationDate) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// Find active reservations
std::vector<const Reservation*> ReservationManager::findActiveReservations() {
    std::vector<const Reservation*> results;

    for (const auto& reservation : reservations) {
        if (reservation.getIsActive()) {
            results.push_back(&reservation);
        }
    }
    return results;
}

// Reserve books
std::string ReservationManager::reserveBook(const std::string& memberID, const std::string& isbn) {
    MemberManager memberManager;
    Member* member = memberManager.findMemberByID(memberID);
    BookManager bookManager;
    Book* book = bookManager.findBookByISBN(isbn);

    if (!member || !book) {
        return "0";
    }
    if (member->isExpired()) {
        return "0";
    }

    // Avoid duplicate active reservations for the same member and ISBN
    for (const auto& reservation : reservations) {
        if (reservation.getIsActive() &&
            reservation.getMemberID() == memberID &&
            reservation.getISBN() == isbn) {
            return "0";
        }
    }

    std::string currentDate = DateUtils::getCurrentDate();
    std::string year = currentDate.substr(0,4);
    int month = std::stoi(currentDate.substr(5,2));
    int season = (month - 1) / 3 + 1;

    std::string prefix = "R" + year + std::to_string(season);
    int maxSeq = 0;
    for (const auto& reservation : reservations) {
        std::string id = reservation.getReservationID();
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
    std::string reservationID = oss.str();

    Reservation reservation(reservationID, memberID, isbn, currentDate, true);
    if (!addReservation(reservation)) {
        return "0";
    }

    // mark book as reserved
    Book updatedBook = *book;
    updatedBook.setReserved(true);
    if (!bookManager.updateBook(updatedBook)) {
        return "0";
    }

    return reservationID;
}

// Cancel reservation
std::string ReservationManager::cancelReservation(const std::string& reservationID) {
    Reservation* reservation = findByReservationID(reservationID);
    if (!reservation || !reservation->getIsActive()) {
        return "0";
    }

    std::string isbn = reservation->getISBN();
    removeFromQueue(isbn, reservationID);
    reservation->cancelReservation();
    saveIfNeeded();

    BookManager bookManager;
    Book* book = bookManager.findBookByISBN(isbn);
    if (!book) {
        return "0";
    }

    // Check if there are still active reservations for this ISBN
    bool hasActiveForISBN = hasActiveReservations(isbn);

    Book updatedBook = *book;
    updatedBook.setReserved(hasActiveForISBN);
    if (!bookManager.updateBook(updatedBook)) {
        return "0";
    }

    return reservationID;
}

// Get all reservations
const std::vector<Reservation>& ReservationManager::getAllReservations() const {
    return reservations;
}

// Get total number of reservations
int ReservationManager::getTotalReservations() const {
    return static_cast<int>(reservations.size());
}

// Get count of active reservations
int ReservationManager::getActiveReservations() const {
    int count = 0;
    for (const auto& reservation : reservations) {
        if (reservation.getIsActive()) {
            count++;
        }
    }
    return count;
}

// Reload from file
void ReservationManager::reload() {
    loadFromFile();
    buildQueues();
}

// Clear file handler cache
void ReservationManager::clearCache() {
    fileHandler.clearCache();
}

// Check if ReservationID exists
bool ReservationManager::isReservationIDExists(const std::string& reservationID) const {
    return std::find_if(reservations.begin(), reservations.end(),
        [&](const Reservation& reservation) { return reservation.getReservationID() == reservationID; }) != reservations.end();
}

// Batch Operations (RAII)
ReservationManager::BatchOperation::BatchOperation(ReservationManager& rmgr) :
                    reservationManager(&rmgr), originalAutoSave(rmgr.autoSave), active(true) {
    rmgr.setAutoSave(false);
}

ReservationManager::BatchOperation::BatchOperation(BatchOperation&& other) noexcept :
    reservationManager(other.reservationManager), originalAutoSave(other.originalAutoSave),
    active(other.active) {
    other.active = false;
}

ReservationManager::BatchOperation::~BatchOperation() {
    if (!active) {
        return;
    }

    try {
        reservationManager->saveToFile();
        reservationManager->buildQueues();
        reservationManager->setAutoSave(originalAutoSave);
    } catch (...) {
        std::cerr << "Error when trying to save reservations during batch operations." << std::endl;
    }
}

ReservationManager::BatchOperation ReservationManager::beginBatch() {
    return BatchOperation(*this);
}