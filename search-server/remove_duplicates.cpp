#include "remove_duplicates.h"


std::set <int> TestDuplicates (const int document_id, const SearchServer& search_server) {
    const std::map <std::string, double>& find_words = search_server.GetWordFrequencies(document_id);
    std::set <std::string> test_set = {};
    std::set <int> result = {};
    std::set <std::string> set_from_test_id_map;
    for (const auto& [word, number] : find_words){
        test_set.insert (word);
    }
    for (const int testing_id : search_server) {
        if (testing_id == document_id || result.count(testing_id) > 0) {continue;}
        set_from_test_id_map.clear();
        
        const std::map <std::string, double>& test_id_map = search_server.GetWordFrequencies(testing_id);
        
        for (const auto& [word, number] : test_id_map) {
            set_from_test_id_map.insert (word);
        }
        if (test_set == set_from_test_id_map) {
            result.insert (testing_id);
        }
    }
    return result;

}

void RemoveDuplicates(SearchServer& search_server) {
    std::set <int> duplicates={};
    
    for (const int document_id : search_server) {
        if (duplicates.count(document_id) > 0 ) {continue;}
        //const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {

        std::set <int> temp = TestDuplicates(document_id, search_server);
        duplicates.insert (temp.begin(), temp.end());
    }
    for (auto duplicat : duplicates) {
        
        std::cout << "Found duplicate document id "s<< duplicat << std::endl;
        search_server.RemoveDocument (duplicat);
    }
}


