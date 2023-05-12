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

	 //В методе AddDocument переберите слова документа, кроме стоп-слов. 
     // В множество документов, соответствующих очередному слову, вставьте id текущего документа.
	

    void AddDocument(int document_id, const string& document) {
        // map<string, map<int, double>> word_to_document_freqs_
        const vector<string> words = SplitIntoWordsNoStop(document);
        double TF=0;
            
        
        for (const string& word : words){
           
          
            TF = static_cast <double> (1) / static_cast <double> (words.size());
          
           word_to_document_freqs_[word][document_id] += TF;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        // const set<string> minus_words = ParseQueryFindMinus (raw_query);
        
       // const set<string> minus_words_in_query = ParseQueryFindMinus (raw_query);
        const Query QueryPM = ParseQuery(raw_query);
        vector<Document> matched_documents = FindAllDocuments(QueryPM);

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
    
    map<string, map<int, double>> word_to_document_freqs_;
    
    set<string> stop_words_;
	

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };


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



    Query ParseQuery(const string& text) const {
        Query Qu;
        
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (!IsMinusWord(word)) {
                Qu.plus_words.insert(word);
            } 
            else {
                Qu.minus_words.insert(word.substr(1)); 
            }   
        }
        return Qu;
    }
    bool IsMinusWord(const string& word) const {
          return (word[0] == '-');
		 
	}
    double FindIDF (const string& word) const {
        if (word.empty()) {return 0;}
        double x = document_count_;
        double y = word_to_document_freqs_.at(word).size();
    
    return log(x/y);
    }
    
    
     //методе FindAllDocuments сначала вычислите релевантность документов, 
     //в которые входят плюс-слова запроса. Для этого используйте map<int, int>, 
     //где ключ — id документа, а значение — количество плюс-слов, которые в этом документе 
     //встречаются. Затем исключите из получившегося map те документы, в которых встретилось 
     //хотя бы одно минус-слово. Оставшиеся элементы map скопируйте в результирующий vector<Document>.
    

    vector<Document> FindAllDocuments(const Query& Query_) const {
         // map<string, map<int, double>> word_to_document_freqs_;
        		
        double IDF = 0;
        double Relevance=0;
              
        vector<Document> matched_documents;
       // накопим результат в map
        map <int, double> resultIDFTF; // id документа и TF сумма по всем словам в нем
       
                
        for (const string& word:Query_.plus_words) {
            // взять множество id документов по данному слову, 
            // проверив наличие слова в словаре, иначе можем схватить исключение
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            
            IDF = FindIDF(word); 
            for (const auto&[id, tf]:(word_to_document_freqs_.at(word))){
                Relevance = resultIDFTF[id];
                Relevance += IDF*tf;
                resultIDFTF[id] = Relevance;
            }
        }
    
        
        for (const string& MinusWord : Query_.minus_words){
            
            //убрать исключения от использования at, если такого слова в словаре нет
            if (word_to_document_freqs_.count(MinusWord) == 0) {
                continue;
            }
            for (const auto [id, _] : word_to_document_freqs_.at(MinusWord)) {
                
                
               resultIDFTF.erase(id);
            }
               
        }
       
        for (const auto [id, rel] : resultIDFTF ) {
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