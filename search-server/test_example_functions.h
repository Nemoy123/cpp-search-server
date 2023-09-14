#pragma once
#include "search_server.h"

void AddDocument (SearchServer& server, int doc_id, std::string text, DocumentStatus status, std::vector <int> ratings);