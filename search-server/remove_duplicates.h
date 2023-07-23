#pragma once
#include <iostream>
#include "search_server.h"

void RemoveDuplicates(SearchServer& search_server);
std::set <int> TestDuplicates (const int document_id, const SearchServer& search_server);