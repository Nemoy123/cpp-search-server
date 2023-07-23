#pragma once
#include <iostream>
#include <vector>
#include <stdint.h>
#include "document.h"
#include "search_server.h"

template <typename Type>
class Stack {
public:
    void Push(const Type& element);
    void Pop();
    const Type& Peek() const;
    Type& Peek();
    void Print() const;
    std::size_t Size() const;
    bool IsEmpty() const;
   
private:
    std::vector<Type> elements_;
};

template <typename Type>
void Stack<Type>::Push(const Type& element) {
        elements_.push_back(element);
}
template <typename Type>
void Stack<Type>::Pop() {
        elements_.pop_back();
}
template <typename Type>
const Type& Stack<Type>::Peek() const {
        return elements_.back();
}
template <typename Type>
Type& Stack<Type>::Peek() {
        return elements_.back();
}
template <typename Type>
void Stack<Type>::Print() const {
        PrintRange(elements_.begin(), elements_.end());
}
template <typename Type>
std::size_t Stack<Type>::Size() const {
        return elements_.size();
}
template <typename Type>
bool Stack<Type>::IsEmpty() const {
        return elements_.empty();
}


template <typename Type>
class deque {
public:
    void Push(const Type& element);
    void Pop();
    Type& Front();
    size_t Size() const;
    bool IsEmpty() const;
 
private:
    Stack<Type> stack1_;
    Stack<Type> stack2_;
};

template <typename Type>
void deque<Type>::Push(const Type& element) {
        while (!stack2_.IsEmpty()) {
            stack1_.Push(stack2_.Peek());
            stack2_.Pop();
        }
        stack1_.Push(element);
       
    }
    
template <typename Type>
void deque<Type>::Pop() {
        
        if (stack1_.IsEmpty()) {stack2_.Pop();}
        else if (stack2_.IsEmpty()) {stack1_.Pop();}
        
}

template <typename Type>
    Type& deque<Type>::Front() {
        while (!stack1_.IsEmpty()) {
            stack2_.Push(stack1_.Peek());
            stack1_.Pop();
        }
        
        return stack2_.Peek();
}
    
template <typename Type>
    size_t deque<Type>::Size() const {
       if (stack1_.IsEmpty()) {return stack2_.Size();}
       else {return stack1_.Size();}
      
}

template <typename Type>
bool deque<Type>::IsEmpty() const {
          
          return stack1_.IsEmpty() && stack2_.IsEmpty();
}

class RequestQueue {
public:
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    void DeleteOld ();
    void PlusEpmty (const std::string& raw_query, const std::vector<Document>& search_results);
    int GetNoResultRequests() const;
   
private:
    struct QueryResult {
        int time_request;
        std::string query;  
    };
    const SearchServer& search_server_Queue;
    deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int time_=0;
    int count_=0;
    
    public: 
    
    explicit RequestQueue(const SearchServer& search_server) : search_server_Queue(search_server) { }
}; 


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        
      return search_server_Queue.FindTopDocuments(raw_query, document_predicate);  
}



