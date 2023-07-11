#pragma once
#include <iostream>
#include <vector>
#include "document.h"
#include "search_server.h"

template <typename Type>
class Stack {
public:
     void Push(const Type& element) {
        elements_.push_back(element);
    }
    void Pop() {
        elements_.pop_back();
    }
    const Type& Peek() const {
        return elements_.back();
    }
    Type& Peek() {
        return elements_.back();
    }
    void Print() const {
        PrintRange(elements_.begin(), elements_.end());
    }
    std::size_t Size() const {
        return elements_.size();
    }
    bool IsEmpty() const {
        return elements_.empty();
    }
    
    
    

private:
    std::vector<Type> elements_;
};

template <typename Type>
class deque {
public:
    void Push(const Type& element) {
        while (!stack2_.IsEmpty()) {
            stack1_.Push(stack2_.Peek());
            stack2_.Pop();
        }
        stack1_.Push(element);
       
    }
    void Pop() {
        
        if (stack1_.IsEmpty()) {stack2_.Pop();}
        else if (stack2_.IsEmpty()) {stack1_.Pop();}
        //else if (stack1_.IsEmpty()&&stack2_.IsEmpty()) {continue;}
    }
    Type& Front() {
        while (!stack1_.IsEmpty()) {
            stack2_.Push(stack1_.Peek());
            stack1_.Pop();
        }
        
        return stack2_.Peek();
    }
    size_t Size() const {
       if (stack1_.IsEmpty()) {return stack2_.Size();}
       else {return stack1_.Size();}
      
    }
    bool IsEmpty() const {
          
          return stack1_.IsEmpty() && stack2_.IsEmpty();
    }

private:
    Stack<Type> stack1_;
    Stack<Type> stack2_;
};



class RequestQueue {
public:
    
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        
      return search_server_Queue.FindTopDocuments(raw_query, document_predicate);  
    }

   std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status) {
                
    return search_server_Queue.FindTopDocuments(raw_query, status);
   }

    std::vector<Document> AddFindRequest(const std::string& raw_query);
    
    void DeleteOld ();
    
    void PlusEpmty (const std::string& raw_query, const std::vector<Document>& search_results);    
    
   int GetNoResultRequests() const;
   
private:
    
    struct QueryResult {
        // определите, что должно быть в структуре 
        int time_request;
        std::string query;  
    };
    const SearchServer& search_server_Queue;
    deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int time_=0;
    int count_=0;
    // возможно, здесь вам понадобится что-то ещё
 
    public: 
    //конструктор
    explicit RequestQueue(const SearchServer& search_server) : search_server_Queue(search_server) { }
      
    
    
}; 
