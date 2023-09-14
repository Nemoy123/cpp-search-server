#include "process_queries.h"

#include <execution>
#include <utility>
#include <string_view>
#include <deque>

using namespace std;

std::vector<std::vector<Document>> ProcessQueries( const SearchServer& search_server, const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> result(queries.size());
    
    std::transform ( std::execution::par, queries.cbegin(), queries.cend(), result.begin(),    
              [&search_server](const std::string& q) { 
                
                return search_server.FindTopDocuments(q); } );
    return result;          
}

std::deque<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {

  std::vector<std::vector<Document>> result(queries.size());
    std::deque<Document> res_finish;
    std::transform ( std::execution::par, queries.cbegin(), queries.cend(), result.begin(),    
              [&search_server](const std::string& q) { 
                
                return search_server.FindTopDocuments(q); } );              
    for (const auto& res : result) {
      for (const auto& doc : res) {
        res_finish.push_back(doc);
      }
    }
    return res_finish;
}