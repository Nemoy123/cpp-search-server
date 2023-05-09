#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
static string query;
int document_count_=0;
    
string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    document_count_= result;
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};







class SearchServer {
public:
   
    
    
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

	set<string> ParseQueryFindMinus (const string& text) const {
       set <string> minuswords;
            for (const string& word : SplitIntoWordsNoStop(text)) {
            
                if (IsMinusWord(word)) { //  заполняем множество минус слова
                    
                   minuswords.insert(word.substr(1)); // substr (1) слово без -
                }     
        }
        return minuswords;
    }
	
	
	 //В методе AddDocument переберите слова документа, кроме стоп-слов. 
     // В множество документов, соответствующих очередному слову, вставьте id текущего документа.
	

    void AddDocument(int document_id, const string& document) {
        // map<string, map<int, double>> word_to_document_freqs_
        const vector<string> words = SplitIntoWordsNoStop(document);
        double TF=0;
        int TFk=0;
        
        
        for (auto& word:words){
            TFk = count(words.begin(), words.end(), word);
            TF = static_cast <double> (TFk) / words.size();
            word_to_document_freqs_[word].insert({document_id, TF});
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        // const set<string> minus_words = ParseQueryFindMinus (raw_query);
        
        const set<string> minus_words_in_query = ParseQueryFindMinus (raw_query);
        const set<string> query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words,word_to_document_freqs_);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    // struct DocumentContent {
    //     int id = 0;
    //    vector<string> words;
    //  };
    // vector<DocumentContent> documents_;
    
   // map <string, set<int>> documents_; // string слова поиска set int это Id документов
    map<string, map<int, double>> word_to_document_freqs_;
    
    set<string> stop_words_;
	set<string> minus_words_in_query;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    set<string> ParseQuery(const string& text) const {
        set<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (!IsMinusWord(word)) {
                query_words.insert(word);
            }    
        }
        return query_words;
    }
    bool IsMinusWord(const string& word) const {
         if (word[0] == '-') { return true;}
		 else {return false;}
	}
   
    
    
     //методе FindAllDocuments сначала вычислите релевантность документов, 
     //в которые входят плюс-слова запроса. Для этого используйте map<int, int>, 
     //где ключ — id документа, а значение — количество плюс-слов, которые в этом документе 
     //встречаются. Затем исключите из получившегося map те документы, в которых встретилось 
     //хотя бы одно минус-слово. Оставшиеся элементы map скопируйте в результирующий vector<Document>.
    

    vector<Document> FindAllDocuments(const set<string>& query_words, const map<string,                                             map<int, double>>& word_to_document_freqs_) const {
         // map<string, map<int, double>> word_to_document_freqs_;
        // set<string> minus_words_in_query; // множество минус слов
		int idf_count_word =0;
        double IDF = 0;
        
        double Relevance=0;
       // double tf2old=0;
        
        vector<Document> matched_documents;
       // накопим результат в map
        map <int, double> resultIDFTF; // id документа и TF сумма по всем словам в нем
        map <int, double> plus_document;  
        map <int, double> idset; // множество ID и TF по конкретному слову запроса
                
        for (const string& word:query_words) {
            // взять множество id документов по данному слову, 
            // проверив наличие слова в словаре, иначе можем схватить исключение
            if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
                idset=word_to_document_freqs_.at(word);
            
                idf_count_word=idset.size(); // количество документов где есть слово
                IDF = log(static_cast<double>(document_count_)/static_cast<double>(idf_count_word)); 
                           
                for (const auto&[id2, tf2]:idset){
                        Relevance = resultIDFTF[id2];
                        Relevance += IDF*tf2;
                        resultIDFTF[id2] = Relevance;
                    
                   
                }
            }
        }
        
        for (const string& MinusWord:minus_words_in_query){
            //убрать исключения от использования at, если такого слова в словаре нет
            if (word_to_document_freqs_.find(MinusWord) != word_to_document_freqs_.end()) {
                idset = word_to_document_freqs_.at(MinusWord);
                for (const auto& [id, relevance] : idset) {
                    // убрать из выдачи поиска - обнулить релевантность по ИД
                    // plus_document.insert({id, 0});
                    resultIDFTF.erase(id);
                }
            }    
        }
       
        for (const auto& [id, rel] : resultIDFTF ) {
            // добавляем релевантность данного слова в выходной вектор
             matched_documents.push_back ({id, rel});  
        }
     return matched_documents;
    }

    
    
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
	

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
	//query = ReadLine();
	//search_server.ParseQueryFindMinus (query);
    
	return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}