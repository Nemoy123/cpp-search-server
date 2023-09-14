#include "document.h"
#include "search_server.h"
#include <utility>
#include <execution>
#include <deque>
#include <unordered_set>
#include <string_view>
#include <execution>

SearchServer::SearchServer(const std::string& stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text))  
    {}
SearchServer::SearchServer() {}


 void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus status,
                     const std::vector<int>& ratings) {

        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw std::invalid_argument("Invalid document_id"s);
        }
        
        const auto words = SplitIntoWordsNoStop(document);

        const double inv_word_count = 1.0 / words.size();
        for (const auto word : words) {
            std::string word_to_string{word};
            auto iter = server_words_.insert(word_to_string);
            std::string_view word_to_view {*(iter).first};
            word_to_document_freqs_[word_to_view][document_id] += inv_word_count;
            id_doc_string_freqs[document_id][word_to_view] = inv_word_count;
            id_doc_with_set_words_[document_id].insert(word_to_view);
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        document_ids_.insert(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}


std::vector<Document> SearchServer::FindTopDocuments(std::execution::sequenced_policy, 
                                                    const std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, status);
}

std::vector<Document> SearchServer::FindTopDocuments(std::execution::sequenced_policy, 
                                                    const std::string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(std::execution::parallel_policy, 
                                                    const std::string_view raw_query, DocumentStatus status) const {
       return FindTopDocuments(std::execution::par, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
}

std::vector<Document> SearchServer::FindTopDocuments(std::execution::parallel_policy, 
                                                    const std::string_view raw_query) const {
    return FindTopDocuments(std::execution::par, raw_query, DocumentStatus::ACTUAL);
}


int SearchServer::GetDocumentCount() const {
        return documents_.size();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view raw_query,int document_id) const {
        


        if (document_id < 0 || documents_.count(document_id) == 0) {
            throw std::out_of_range("Invalid document_id");
        }
        if (!IsValidWord(raw_query)) {
           throw std::invalid_argument("Invalid query");
        }   

        const auto query = ParseQuery(raw_query);
        std::vector<std::string_view> matched_words;

        for (const auto word : query.minus_words) {
            //std::string word2{word};
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                return {matched_words, documents_.at(document_id).status};
            }
        }

        for (const auto word : query.plus_words) {
           
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        
        return {matched_words, documents_.at(document_id).status};
    }

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::sequenced_policy, 
                                                                    const std::string_view raw_query,int document_id) const {
        if (document_id < 0 || documents_.count(document_id) == 0) {
            throw std::out_of_range("Invalid document_id");
        }
        if (!IsValidWord(raw_query)) {
           throw std::invalid_argument("Invalid query");
        }  
return MatchDocument (raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::parallel_policy, 
          
                                                                    const std::string_view raw_query,int document_id) const {
        if (document_id < 0 || documents_.count(document_id) == 0) {
            throw std::out_of_range("Invalid document_id");
        }
        if (!IsValidWord(raw_query)) {
           throw std::invalid_argument("Invalid query");
        }                                                                


        auto query = ParseQuery(std::execution::par, raw_query);
        
        std::vector<std::string_view> matched_words(query.plus_words.size());
        
        if (!std::none_of ( std::execution::par, query.minus_words.begin(), query.minus_words.end(), 
         [this, document_id, &matched_words](auto& n)
         { 
            std::string word2{n};
            return (word_to_document_freqs_.at(word2).count(document_id)==0 ) ? false : true;
            })  )
        { matched_words.clear(); 
          return {matched_words, documents_.at(document_id).status};  
        }
    
       auto last_it = std::copy_if ( std::execution::par, std::begin(query.plus_words), std::end(query.plus_words), std::begin(matched_words), 
        [this, &document_id](auto& n)
         { 
                std::string word2{n};
               return word_to_document_freqs_.at(word2).count(document_id);  
                
         });
        matched_words.erase (last_it, matched_words.end());
        std::sort(std::execution::par, matched_words.begin(),  matched_words.end());
        matched_words.erase(std::unique(std::execution::par, matched_words.begin(),  matched_words.end()),  matched_words.end());
         

        return {matched_words, documents_.at(document_id).status};
}
 

bool SearchServer::IsStopWord(const std::string_view word) const {
        return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string_view word) {
        return std::none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view text) const {
        std::vector<std::string_view> words;
        for (const std::string_view word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                std::string conv {word};
                throw std::invalid_argument("Word "s + conv + " is invalid"s);
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
        
        return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string_view text) const {
        if (text.empty()) {
            throw std::invalid_argument("Query word is empty"s);
        }
        auto word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            std::string conv {text};
            throw std::invalid_argument("Query word "s + conv + " is invalid");
        }

        return {word, is_minus, IsStopWord(word)};
}

 SearchServer::Query SearchServer::ParseQuery(const std::string_view text) const {
        Query result;
        for (const std::string_view word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    
                    result.minus_words.push_back(query_word.data);
                    
                } else {
                    
                    result.plus_words.push_back(query_word.data);
                    
                }
            }
        }
        std::sort(result.minus_words.begin(), result.minus_words.end());
        result.minus_words.erase(std::unique(result.minus_words.begin(), result.minus_words.end()), result.minus_words.end());
        std::sort(result.plus_words.begin(), result.plus_words.end());
        result.plus_words.erase(std::unique(result.plus_words.begin(), result.plus_words.end()), result.plus_words.end());
        return result;
}

 SearchServer::Query SearchServer::ParseQuery(std::execution::parallel_policy, const std::string_view text) const {
        Query result;
        for (const auto word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    
                    result.minus_words.push_back(query_word.data);
                } else {
                    
                    result.plus_words.push_back(query_word.data);
                }
            }
        }

        return result;
}



double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const {
        std::string word2 {word};
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word2).size());
}

std::set <int>::const_iterator SearchServer::begin() const {return this->document_ids_.cbegin();}
std::set <int>::const_iterator SearchServer::end() const {return this->document_ids_.cend();}

const std::map<std::string_view, double> SearchServer::GetWordFrequencies(int document_id) const {
    
    const auto it = (id_doc_string_freqs).find (document_id);
    
        return {(*it).second.begin(), (*it).second.end()};
}

void SearchServer::RemoveDocument(int document_id) {
    
    auto it = std::find (document_ids_.begin(), document_ids_.end(), document_id);
    if (it != document_ids_.end()) {document_ids_.erase (it);}
    id_doc_string_freqs.erase (document_id);
    documents_.erase (document_id);
    for (auto& word:word_to_document_freqs_) {
        (word.second).erase (document_id);
    }
    
    id_doc_with_set_words_.erase(document_id);

}

void SearchServer::RemoveDocument(std::execution::sequenced_policy, int document_id){
    SearchServer::RemoveDocument(document_id);
}


void SearchServer::RemoveDocument(std::execution::parallel_policy, int document_id){
    if (document_ids_.find(document_id) == document_ids_.end()) {return;}
   
    std::vector <std::string_view> vector_ptr (id_doc_with_set_words_.at(document_id).size());
        
    std::transform ( std::execution::par, id_doc_with_set_words_.at(document_id).begin(), 
                    id_doc_with_set_words_.at(document_id).end(), vector_ptr.begin(),    
              [](auto& q) { return q; } );
    
   
    std::for_each ( std::execution::par, vector_ptr.begin(), vector_ptr.end(), [this, document_id](auto n){
       
        word_to_document_freqs_.at(n)[document_id] = {};
    });
    
    
    document_ids_.erase (std::find (std::execution::par, document_ids_.begin(), document_ids_.end(), document_id));
    id_doc_string_freqs.erase (document_id);
    documents_.erase (document_id);
    id_doc_with_set_words_.erase(document_id);
    
}