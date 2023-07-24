#include "remove_duplicates.h"


void RemoveDuplicates (SearchServer& search_server) {
    
    std::set <std::string> set_from_test_id_map = {};
    std::set <int> duplicates={};
    std::map <std::set<std::string>, int> nabor_slov_vseh_documents; // int id, set слова
    for (int testing_id : search_server) {
        
        const std::map <std::string, double>& test_id_map = search_server.GetWordFrequencies(testing_id);
        
        for (const auto& [word, number] : test_id_map) {
            set_from_test_id_map.insert (word);
        }
        
        if (nabor_slov_vseh_documents.count(set_from_test_id_map) > 0) {
            duplicates.insert (testing_id);
        }
        else {
            nabor_slov_vseh_documents.insert ({set_from_test_id_map, testing_id});
        }
        set_from_test_id_map.clear();
         
    }
    for (auto duplicate : duplicates) {
        
        std::cout << "Found duplicate document id "s << duplicate << std::endl;
        search_server.RemoveDocument (duplicate);
    }

}



