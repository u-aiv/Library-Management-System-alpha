// RecommendationManager.cpp Implementation

#include "RecommendationManager.h"
#include <algorithm>
#include <cmath>

// Constructor
RecommendationManager::RecommendationManager(
    const std::string &bookPath,
    const std::string &memberPath,
    const std::string &transactionPath) :
        bookManager(bookPath),
        memberManager(memberPath),
        transactionManager(transactionPath) {}

// Genre -> Index mapping
std::unordered_map<std::string, size_t> RecommendationManager::buildGenreIndex(const std::vector<Book>& books) const {
    std::unordered_map<std::string, size_t> index;
    size_t next = 0;
    for (const auto& book : books) {
        const std::string& genre = book.getGenre();
        if (index.find(genre) == index.end()) {
            index[genre] = next++;
        }
    }
    return index;
}

// Build Member Preference/History Vector
std::vector<double> RecommendationManager::buildMemberVector(
    const Member& member,
    const std::vector<Transaction>& transactions,
    std::unordered_map<std::string, Book>& bookByISBN,
    std::unordered_map<std::string, size_t>& genreIndex) const {
    std::vector<double> vec(genreIndex.size(), 0.0);

    // Preferences provide a strong signal for cold-start users
    // High weight for explicit preferences
    const std::vector<std::string> preferences = member.getPreference();
    for (const auto& pref : preferences) {
        auto it = genreIndex.find(pref);
        if (it != genreIndex.end()) {
            vec[it->second] += 2.0;
        }
    }

    // Borrow history provides collaborative signal
    // Add weight for past borrows
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() != member.getMemberID()) {      // skip other members
            continue;
        }
        auto bookIt = bookByISBN.find(transaction.getISBN());
        if (bookIt == bookByISBN.end()) {
            continue;
        }
        const std::string& genre = bookIt->second.getGenre();
        auto indexIt = genreIndex.find(genre);
        if (indexIt != genreIndex.end()) {
            vec[indexIt->second] += 1.0;
        }
    }
    return vec;
}

// Cosine Similarity for KNN
double RecommendationManager::cosineSimilarity(
    const std::vector<double> &lhs,
    const std::vector<double> &rhs) {
    if (lhs.empty() || rhs.empty() || lhs.size() != rhs.size()) {
        return 0.0;
    }

    double dot = 0.0;       // dot product
    double normLhs = 0.0;   // norm of lhs
    double normRhs = 0.0;   // norm of rhs
    for (size_t i = 0; i < lhs.size(); ++i) {
        dot += lhs[i] * rhs[i];
        normLhs += lhs[i] * lhs[i];
        normRhs += rhs[i] * rhs[i];
    }
    if (normLhs <= 0 || normRhs <= 0) {
        return 0.0;
    }

    return dot / (std::sqrt(normLhs) * std::sqrt(normRhs));
}

// Collect borrowed ISBNs
std::unordered_set<std::string> RecommendationManager::getMemberBorrowedISBNs(
    const std::string& memberID,
    const std::vector<Transaction>& transactions) const {
    std::unordered_set<std::string> borrowed;
    for (const auto& transaction : transactions) {
        if (transaction.getUserID() == memberID) {
            borrowed.insert(transaction.getISBN());
        }
    }
    return borrowed;
}

// Count ISBN popularity
std::unordered_map<std::string, int> RecommendationManager::buildISBNPopularity(
    const std::vector<Transaction>& transactions) const {
    std::unordered_map<std::string, int> countByISBN;
    for (const auto& transaction : transactions) {
        countByISBN[transaction.getISBN()]++;
    }
    return countByISBN;
}

// Main recommendation method
std::vector<Book> RecommendationManager::recommendForMember(
    const std::string& memberID,
    int topN,
    int kNeighbors,
    bool availableOnly) {
    if (topN <= 0) {        // Handle invalid topN
        return {};
    }


    Member* targetMember = memberManager.findMemberByID(memberID);        // Find member
    if (targetMember == nullptr) {
        return {};
    }

    // loading all books, transactions, members
    const std::vector<Book> allBooks = bookManager.getAllBooks();
    const std::vector<Transaction>& allTransactions = transactionManager.getAllTransactions();
    const std::vector<Member>& allMembers = memberManager.getAllMembers();

    std::unordered_map<std::string, Book> bookByISBN;       // ISBN -> Book mapping
    bookByISBN.reserve(allBooks.size());
    for (const auto& book : allBooks) {
        bookByISBN[book.getISBN()] = book;
    }

    std::unordered_map<std::string, size_t> genreIndex = buildGenreIndex(allBooks);
    if (genreIndex.empty()) {
        return {};
    }

    const std::vector<double> targetVec = buildMemberVector(
        *targetMember, allTransactions, bookByISBN, genreIndex);

    // Neighbor ID and similarity
    std::vector<std::pair<std::string, double> > neighborScore;
    neighborScore.reserve(allMembers.size());
    for (const auto& member : allMembers) {
        if (member.getMemberID() == targetMember->getMemberID()) {
            continue;
        }

        const std::vector<double> vec = buildMemberVector(
            member, allTransactions, bookByISBN, genreIndex);
        double similarity = cosineSimilarity(vec, targetVec);
        if (similarity > 0.0) {
            neighborScore.emplace_back(member.getMemberID(), similarity);
        }
    }

    // Sort by similarity
    std::sort(neighborScore.begin(), neighborScore.end(),
    [](const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs)
    {return lhs.second > rhs.second;});

    if (kNeighbors > 0 && static_cast<size_t>(kNeighbors) < neighborScore.size()) {
        neighborScore.resize(static_cast<size_t>(kNeighbors));      // reserve the first _k_ neighbors
    }

    const std::unordered_set<std::string> borrowedByTarget = getMemberBorrowedISBNs(memberID, allTransactions);
    const std::unordered_map<std::string, int> bookPopularity = buildISBNPopularity(allTransactions);

    std::unordered_map<std::string, double> candidateScores;

    for (const auto& neighbor : neighborScore) {
        const std::string& neighborID = neighbor.first;
        double neighborSimilarity = neighbor.second;
        for (const auto& transaction : allTransactions) {
            if (transaction.getUserID() != neighborID) {
                continue;
            }
            const std::string& neighborISBN = transaction.getISBN();
            if (borrowedByTarget.find(neighborISBN) != borrowedByTarget.end()) {
                continue;
            }
            candidateScores[transaction.getISBN()] += neighborSimilarity;
        }
    }

    // Fallback: content-based scoring using target vector if no neighbors
    if (candidateScores.empty()) {      // no collaborative candidates
        for (const auto& book : allBooks) {
            const std::string& isbn = book.getISBN();
            if (borrowedByTarget.find(isbn) != borrowedByTarget.end()) {
                continue;
            }
            auto idxIt = genreIndex.find(book.getGenre());
            if (idxIt == genreIndex.end()) {
                continue;
            }

            double base = targetVec[idxIt->second];     // base score from preference
            int pop = 0;                                // popularity count
            auto popIt = bookPopularity.find(isbn);
            if (popIt != bookPopularity.end()) {
                pop = popIt->second;
            }
            candidateScores[isbn] = base + (0.1 * static_cast<double>(pop));
        }
    } else {                                            // have collaborative candidates
        for (auto& entry: candidateScores) {
            auto popIt = bookPopularity.find(entry.first);
            if (popIt != bookPopularity.end()) {
                entry.second += 0.05 * static_cast<double>(popIt->second);
            }
        }
    }

    std::vector<std::pair<std::string, double> > scoredISBNs;       // vector of scored ISBNs
    scoredISBNs.reserve(candidateScores.size());
    for (const auto& entry: candidateScores) {
        scoredISBNs.emplace_back(entry);
    }

    std::sort(scoredISBNs.begin(), scoredISBNs.end(),               // sort candidates by score
        [](const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs) {
        if (lhs.second != rhs.second) {
            return lhs.second > rhs.second;
        }
            return lhs.first < rhs.first; });       // tie-break by isbn

    std::vector<Book> resultBooks;
    for (const auto& score : scoredISBNs) {
        if (static_cast<int>(resultBooks.size()) >= topN) {
            break;
        }
        auto bookIt = bookByISBN.find(score.first);
        if (bookIt == bookByISBN.end()) {
            continue;
        }
        if (availableOnly && !bookIt->second.canBorrow()) {
            continue;
        }
        resultBooks.emplace_back(bookIt->second);
    }
    return resultBooks;
}
