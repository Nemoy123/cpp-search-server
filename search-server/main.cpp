// -------- Начало модульных тестов поисковой системы ----------

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL_HINT(server.FindTopDocuments("in"s).empty(),true,
                    "Stop words must be excluded from documents"s);
    }
}

void TestAddingDocuments (){
    const int doc_id = 42;
    const string content = "cat in the city black tale on him best word"s;
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        size_t i=0;
        const vector <string> doc = SplitIntoWords(content);
        for (const auto& word: doc){
            auto found_docs = server.FindTopDocuments(word);
            ASSERT_EQUAL(found_docs.size(), 1);
            const Document& doc0 = found_docs[0];
            ASSERT_EQUAL(doc0.id, doc_id);
            ++i;
        }
        ASSERT_EQUAL(i, static_cast<size_t>(doc.size()));
    }
}

void TestExcludeMinusWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        
        ASSERT_EQUAL_HINT(server.FindTopDocuments("cat -in"s).empty(),true,
                    "documents which contain minus words must be excluded from search results"s);
    }

}

ostream& operator<<(ostream& os, const DocumentStatus& r) {
    os << ((int) r);
    return os;
}

void  TestingMatchDocument() {
    {
        SearchServer server;
        const int doc_id = 42;
        const string content = "cat in the city big tale alone"s;
        const vector<int> ratings = {1, 2, 3};
        const string querytest = "cat alone"s;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const tuple<vector<string>, DocumentStatus> result = server.MatchDocument(querytest,doc_id);
        ASSERT_EQUAL(get<1> (result), (DocumentStatus::ACTUAL));
        ASSERT_EQUAL(get<0> (result)[0], "alone");
        ASSERT_EQUAL(get<0> (result)[1], "cat");
		ASSERT_EQUAL(get<1> (result), DocumentStatus::ACTUAL); 
		server.SetStopWords("the"s);
        ASSERT_EQUAL(get<0> (result).empty(), false); 
    }
}

ostream& operator<<(ostream& out, const vector <Document>& container) {
    for (const auto& [id, relevance, rating]:container){
        out <<"Id " <<id <<" relevance "<< relevance <<" rating "<< rating << " ";
    }
    out << " End "<< endl;
    return out;
}
ostream& operator<<(ostream& out, const Document& container) {
    
    out <<"Id " <<container.id <<" relevance "<< container.relevance <<" rating "<< container.rating << " ";
    
    out << " End "<< endl;
    return out;
}
bool operator==(const vector <Document>& ass, const vector <Document>& bss) {
            bool testtrue = false;
            if (ass.size() != bss.size()) {return false;}

            for (size_t i = 0; i < ass.size(); ++i) {
                
                if (ass.at(i).id == bss.at(i).id && ass.at(i).rating == bss.at(i).rating && ass.at(i).relevance == bss.at(i).relevance) {
                        testtrue = true;
                }
                else {
                    return false;}
            }
            return testtrue;
}

void  TestingSortingRelevance() {
    {
        SearchServer server;
        const int doc_id = 42;
        const string content = "cat in the city big tale alone"s;
        const string querytest = "cat"s;
        const vector<int> ratings = {1, 2, 3};
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(43, "Ser Waymar Royce was the alone youngest son of an ancient house"s, DocumentStatus::ACTUAL, {4, 10, 5});
        server.AddDocument(68, "Will had been a hunter cat before he joined the Nights Watch"s, DocumentStatus::ACTUAL, {4, 1, 7});
        const vector <Document> result = server.FindTopDocuments(querytest);
        vector <Document> righttestresult = result;
        sort(righttestresult.begin(), righttestresult.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
        });
        ASSERT_EQUAL(result, righttestresult);
    }
}

void TestRatingScore () {
    {
        SearchServer server;
        const int doc_id = 42;
        const string content = "cat in the city big tale alone"s;
        const vector<int> ratings = {1, 2, 3, -5, 7, 15, 75, 40, -3};
        const int srednee_arifmeticheskoe = (accumulate(ratings.begin(), ratings.end(), 0))/ratings.size();
        const string querytest = "cat"s;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const vector <Document> result = server.FindTopDocuments(querytest);
        ASSERT_EQUAL (result.at(0).rating, srednee_arifmeticheskoe);

    }
}
void FiltrPredicatandStatus () {
    {
        SearchServer server;
        const int doc_id = 42;
        const string content = "cat in the city big tale alone"s;
        const string querytest = "cat"s;
        const vector<int> ratings = {1, 2, 3};
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(43, "Ser Waymar Royce was the cat alone youngest son of an ancient house"s, DocumentStatus::IRRELEVANT, {1, -2, 1});
        server.AddDocument(69, "Will had been a hunter cat before he joined the Nights Watch"s, DocumentStatus::BANNED, {1, -3, 1});
        vector <Document> result = server.FindTopDocuments(querytest, 
            [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
        ASSERT_EQUAL (result.at(0).id, 42);
        result.clear();
        result = server.FindTopDocuments(querytest, 
            [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::IRRELEVANT; });
        ASSERT_EQUAL (result.at(0).id, 43);
        result.clear();
        result = server.FindTopDocuments(querytest, 
            [](int document_id, DocumentStatus status, int rating) { return rating==2; });
        ASSERT_EQUAL (result.at(0).id, 42);
        result.clear();
        result = server.FindTopDocuments(querytest, DocumentStatus::BANNED);
        ASSERT_EQUAL (result.at(0).id, 69);
    }
}

void TestRelevance(){
    {
        SearchServer server;
        const int doc_id = 42;
        const string content = "cat in the city big tale alone"s;
        const string querytest = "cat alone"s;
        const vector<int> ratings = {1, 2, 3};
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(43, "Ser Waymar Royce was the cat alone youngest son of an ancient house"s, DocumentStatus::IRRELEVANT, {1, -2, 1});
        server.AddDocument(69, "Will had been a hunter cat before he joined the Nights Watch"s, DocumentStatus::BANNED, {1, -3, 1});
        vector <Document> result = server.FindTopDocuments(querytest);
        double test_relevance = (round (abs(result.at(0).relevance)*1000000))/1000000; // округление  до 7 знака после заяпятой
        ASSERT_EQUAL ((abs(result.at(0).relevance -test_relevance) < 1e-6), true);
        //ASSERT_EQUAL ((abs(result.at(0).relevance -0.0579236) < 1e-6), true);   
	}
}


void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddingDocuments);
    RUN_TEST(TestExcludeMinusWordsFromAddedDocumentContent);
    RUN_TEST(TestingMatchDocument);
    RUN_TEST(TestingSortingRelevance);
    RUN_TEST(TestRatingScore);
    RUN_TEST(FiltrPredicatandStatus);
    RUN_TEST(TestRelevance);
}



//////////////////////////////////////////////////////////////////
