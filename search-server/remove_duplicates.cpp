#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::set <int> duplicates={};
    
    for (const int document_id : search_server) {
        std::vector <int> temp = search_server.TestDuplicates(document_id);
        duplicates.insert (temp.begin(), temp.end());
    }
    for (auto duplicat : duplicates) {
        std::cout << "Found duplicate document id "s<< duplicat << std::endl;
        search_server.RemoveDocument (duplicat);
    }
}