#include "test_example_functions.h"

void AddDocument (SearchServer& server, int doc_id, std::string text, DocumentStatus status, std::vector <int> ratings) {
   
    server.AddDocument(doc_id, text, status, ratings);
}