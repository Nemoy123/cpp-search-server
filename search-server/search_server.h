#pragma once
#include <iostream>
#include <set>
#include <map>
#include <algorithm>
#include "read_input_functions.h"
#include <cmath>
#include <stdexcept>
#include "string_processing.h"
#include "document.h"
#include <numeric>
#include <iterator>
#include <execution>
#include <utility>
#include <variant>
#include <string_view>
#include <execution>
#include <mutex>
//
using std::string_literals::operator""s;
using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) : base_(bucket_count) {}

    Access operator[](const Key& key) {
       auto* nbase = &(base_[static_cast<size_t>(key) % (base_.size())]);
       return {std::lock_guard ((*nbase).value_mutex_), (*nbase).base_map_[key]};
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for ( auto& base : base_) {
            std::lock_guard lock3 (base.value_mutex_);
            result.insert(base.base_map_.begin(),base.base_map_.end());  
        }
        return result;
    }

    void erase (Key key) {
        auto* nbase = &(base_[static_cast<size_t>(key) % (base_.size())]);
        std::lock_guard ((*nbase).value_mutex_);
        (*nbase).base_map_.erase (key);
    }

private:

    class One_map {
        public:
        std::mutex value_mutex_;
        std::map<Key, Value> base_map_;
    };

    std::vector <One_map> base_;
 
};


const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);
    explicit SearchServer(const std::string& stop_words_text);
    explicit SearchServer();
    
    void AddDocument(int document_id, const std::string_view document, DocumentStatus status,const std::vector<int>& ratings);
  
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments( const std::string_view raw_query,DocumentPredicate document_predicate) const;
    std::vector<Document> FindTopDocuments( const std::string_view raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments( const std::string_view raw_query) const;
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::execution::sequenced_policy, const std::string_view raw_query,DocumentPredicate document_predicate) const;
    std::vector<Document> FindTopDocuments(std::execution::sequenced_policy, const std::string_view raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(std::execution::sequenced_policy, const std::string_view raw_query) const;
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::execution::parallel_policy, const std::string_view raw_query, DocumentPredicate document_predicate) const;
    std::vector<Document> FindTopDocuments(std::execution::parallel_policy, const std::string_view raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(std::execution::parallel_policy, const std::string_view raw_query) const;

    int GetDocumentCount() const; 
    
    const std::map<std::string_view, double> GetWordFrequencies(int document_id) const;
    std::set <int>::const_iterator begin() const; 
    std::set <int>::const_iterator end() const;
    
    
    
    void RemoveDocument(std::execution::parallel_policy, int document_id);
    void RemoveDocument(int document_id);
    void RemoveDocument(std::execution::sequenced_policy, int document_id);
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query,int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::parallel_policy, const std::string_view raw_query,int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::sequenced_policy, const std::string_view raw_query,int document_id) const;
  

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const std::set<std::string, std::less<>>  stop_words_; 
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    std::map<int, std::map<std::string_view, double>> id_doc_string_freqs; 
    std::map <int, std::set <std::string_view, std::less<>>> id_doc_with_set_words_;
    std::set<std::string, std::less<>> server_words_;
    
    bool IsStopWord(const std::string_view word) const;
    static bool IsValidWord(const std::string_view word);
    
    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);
    
    
    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(const std::string_view text) const;
    
    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };
    
    Query ParseQuery(const std::string_view text) const;
    Query ParseQuery(std::execution::parallel_policy, const std::string_view text) const;
    
       
    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::parallel_policy, const Query& query, DocumentPredicate document_predicate) const;
    
};




 template <typename StringContainer>
 SearchServer::SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  
    {
        if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw std::invalid_argument("Some of stop words are invalid"s);
        }
   }

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const {
        const auto query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (std::abs(lhs.relevance - rhs.relevance) < std::numeric_limits<double>::epsilon()) {
                     return lhs.rating > rhs.rating;
                 } else {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::execution::sequenced_policy, const std::string_view raw_query, DocumentPredicate document_predicate) const {
    return FindTopDocuments(raw_query, document_predicate);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
        DocumentPredicate document_predicate) const {
        std::map<int, double> document_to_relevance;
        for (const auto word : query.plus_words) {
            std::string word2{word};
            if (word_to_document_freqs_.count(word2) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word2);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word2)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const auto word : query.minus_words) {
            std::string word2{word};
            if (word_to_document_freqs_.count(word2) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word2)) {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::execution::parallel_policy, const std::string_view raw_query, DocumentPredicate document_predicate) const {
     const auto query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(std::execution::par, query, document_predicate);
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (std::abs(lhs.relevance - rhs.relevance) < std::numeric_limits<double>::epsilon()) {
                     return lhs.rating > rhs.rating;
                 } else {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
}

template <typename DocumentPredicate>
    std::vector<Document> SearchServer::FindAllDocuments(std::execution::parallel_policy, const Query& query, DocumentPredicate document_predicate) const {
        
        ConcurrentMap <int, double> document_to_relevance (150);

        for_each( std::execution::par, query.plus_words.begin(), query.plus_words.end(),
        [&](const auto word) {
                std::string word2{word};
                if (word_to_document_freqs_.count(word2) != 0) {
                        
                    
                    const double inverse_document_freq = ComputeWordInverseDocumentFreq(word2);
                    for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word2)) {
                        const auto& document_data = documents_.at(document_id);
                        if (document_predicate(document_id, document_data.status, document_data.rating)) {
                           
                            document_to_relevance[document_id].ref_to_value += (term_freq * inverse_document_freq);
                        }
                    }
                }
            }
        );

       
        for_each ( std::execution::par, query.minus_words.begin(), query.minus_words.end(),
            [&](const auto word) {
                std::string word2{word};
                if (word_to_document_freqs_.count(word2) != 0) {
                    for (const auto [document_id, _] : word_to_document_freqs_.at(word2)) {
                        document_to_relevance.erase(document_id);
                    } 
                }   
            } );

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance.BuildOrdinaryMap()) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }