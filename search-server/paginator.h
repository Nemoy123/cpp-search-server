#pragma once

template<typename It>
class IteratorRange {
    public:
    IteratorRange(It begin, It end) : begin_page(begin), end_page(end)
    {}
    IteratorRange(It begin, size_t size) : begin_page(begin), end_page(begin+size)
    {}
    IteratorRange() {}
    ~IteratorRange()
    {}
    It begin_page;
    It end_page;

    It begin() const { return begin_page; }
    It end() const { return end_page; }
    size_t size() const { return end_page-begin_page; }

};


template<typename It>
class Paginator {
    public:
    vector <IteratorRange<It>> pages_p;
    size_t page_number = 0;
    size_t size_page = 0;
        
    Paginator (It range_begin, It range_end, size_t s) {
        Page_Consruct (range_begin, range_end, s);
    }
    
    size_t size() const {
        return size_page;
    } 
    auto begin() const { return pages_p.begin(); }
    auto end() const { return pages_p.end(); }
    
    void Page_Consruct (It range_begin, It range_end, size_t s) {
             // накопитель итераторов
            It test_step;
            bool start = true;
            It begin;
            It middle;
            for (auto iter = range_begin; iter != range_end; ++iter) {
                
                if (start == true) {
                    begin = iter;
                    start = false;
                }
                
                if (iter+1 != range_end) {
                    middle = iter;}
                else { pages_p.push_back(IteratorRange<It>(begin, range_end)); 
                        return;
                }
                    
                if (distance (range_begin, middle) == (s-1)) {
                    
                    pages_p.push_back(IteratorRange<It>(begin, middle+1));
                    start = true;
                }
        }         
    }

};



template <typename It>
ostream &operator <<(ostream &out, IteratorRange<It> range) {
        
             for (auto doc = range.begin(); doc != range.end(); ++doc) {
                
                 out << "{ "s
                 << "document_id = "s << (*doc).id << ", "s
                 << "relevance = "s << (*doc).relevance << ", "s
                 << "rating = "s << (*doc).rating << " }"s;
             }
        
        return out;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator <typename Container::const_iterator> (begin(c), end(c), page_size);
}
