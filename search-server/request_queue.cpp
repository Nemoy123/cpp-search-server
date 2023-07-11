#include "request_queue.h"

   
int RequestQueue::GetNoResultRequests() const {
        return count_;
   }    

void RequestQueue::PlusEpmty (const std::string& raw_query, const std::vector<Document>& search_results) {
       
        if (search_results.size()==0) {
            ++count_;
            QueryResult empty;
            empty.query = raw_query;
            empty.time_request = time_;
            requests_.Push(empty);
            
        } 
        else {                                         

        }    
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
        
        ++time_; // прибавить время
        DeleteOld (); // удалить записи с разницей во времени time - min_in_day_
        const auto search_results = search_server_Queue.FindTopDocuments(raw_query);
        PlusEpmty (raw_query, search_results);

        return search_server_Queue.FindTopDocuments(raw_query);
    }

void RequestQueue::DeleteOld () {
      
       if (!requests_.IsEmpty()) {
                while (((time_ - min_in_day_)) >=                                             requests_.Front().time_request) {    
               
                    --count_;
                    requests_.Pop();
            }
        }
}