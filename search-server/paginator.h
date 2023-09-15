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
    private:
    std::vector <IteratorRange<It>> pages_p_;
    public:
    
    size_t page_number = 0;
    size_t size_page = 0;
        
    Paginator (It range_begin, It range_end, size_t s) {
        PageConsruct (range_begin, range_end, s);
    }
    
    size_t size() const {
        return size_page;
    } 
    auto begin() const { return pages_p_.begin(); }
    auto end() const { return pages_p_.end(); }
    
    void PageConsruct (It range_begin, It range_end, size_t s) {
            It test_step;
           // bool start = true;
            It begin = range_begin;
            It middle;
            for (auto iter = range_begin; iter != range_end; ++iter) {
                                          
                if (iter+1 != range_end) {
                    middle = iter;}
                else { pages_p_.push_back(IteratorRange<It>(begin, range_end)); 
                        return;
                }
                    
                if (distance (range_begin, middle) == (s - 1)) {
                    pages_p_.push_back(IteratorRange<It>(begin, middle+1));
                    begin = iter;
                }
        }         
    }

};

template <typename It>
std::ostream &operator <<(std::ostream &out, IteratorRange<It> range) {
        
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
