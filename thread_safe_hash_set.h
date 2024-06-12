#pragma once

#ifdef BOOST_FOUND

#include <boost/unordered/concurrent_flat_set.hpp>

template<typename T>
using ThreadSafeHashSet = boost::concurrent_flat_set<T>;

#else

#include <memory>
#include <map>
#include <unordered_set>
#include <string>

template<typename T>
class MutexHashSet
{
public:
    void insert(const T& s)
    {
        std::lock_guard<std::mutex> lk(m_);
        set_.insert(s);
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lk(m_);
        return set_.size();
    }

private:
    mutable std::mutex m_;
    std::unordered_set<T> set_;
};

template<>
class MutexHashSet<std::string>
{
public:
    MutexHashSet()
    {
        for (char c = 'a'; c <= 'z'; c++)
        {   
            alphabet_storage_.emplace(c, MutexSet{});
        }
    }

    void insert(const std::string& s)
    {
        if (s.empty() || (s[0] < 'a' || s[0] > 'z'))
        {
            std::lock_guard<std::mutex> lk(general_storage_mutex_);
            general_storage_.insert(s);
            return;
        }

        auto& mutex_set = alphabet_storage_.at(s[0]);
        
        auto& mut = *mutex_set.mut_ptr_;
        {
            std::lock_guard<std::mutex> lk(mut);
            mutex_set.hash_set_.insert(s);
        }
    }

    std::size_t size() const
    {
        std::size_t sum = 0u;
        for (const auto& [key, value] : alphabet_storage_)
        {
            auto& mut = *value.mut_ptr_;
            {
                std::lock_guard<std::mutex> lk(mut);
                sum += value.hash_set_.size();
            }
        }

        std::lock_guard<std::mutex> general_lock(general_storage_mutex_);
        return sum + general_storage_.size();
    }

private: 

    struct MutexSet
    {
        std::shared_ptr<std::mutex> mut_ptr_ = std::make_shared<std::mutex>();
        std::unordered_set<std::string> hash_set_;
    };
    
    std::map<char, MutexSet> alphabet_storage_;

    mutable std::mutex general_storage_mutex_;
    std::unordered_set<std::string> general_storage_;
};

template<typename T>
using ThreadSafeHashSet = MutexHashSet<T>;

#endif