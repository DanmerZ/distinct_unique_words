#pragma once

#ifdef BOOST_FOUND

#include <boost/unordered/concurrent_flat_set.hpp>

template<typename T>
using ThreadSafeHashSet = boost::concurrent_flat_set<T>;

#else

template<typename T>
class ThreadSafeHashSet
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

#endif