#pragma once
#include <string>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) : base_(bucket_count) {}

    Access operator[](const Key& key) {
       auto* nbase = &(base_[static_cast<size_t>(key) % (base_.size())]);
       return {std::lock_guard ((*nbase).value_mutex_), (*nbase).base_map_[key]};
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for ( auto& base : base_) {
            std::lock_guard lock3 (base.value_mutex_);
            result.insert(base.base_map_.begin(),base.base_map_.end());  
        }
        return result;
    }

    void erase (Key key) {
        auto* nbase = &(base_[static_cast<size_t>(key) % (base_.size())]);
        std::lock_guard ((*nbase).value_mutex_);
        (*nbase).base_map_.erase (key);
    }

private:

    class OneMap {
        public:
        std::mutex value_mutex_;
        std::map<Key, Value> base_map_;
    };

    std::vector <OneMap> base_;
 
};