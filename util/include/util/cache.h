/*
 * cache.h
 *
 *  Created on: Jul 29, 2013
 *      Author: ezoltbo
 */

#ifndef UTIL_CACHE_H_
#define UTIL_CACHE_H_

#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <mutex>
#include <memory>
#include <atomic>

#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/optional.hpp>

namespace cc 
{
namespace util 
{

/**
 * Thread-safe implementation of an object cache
 * It is basically an associative data structure
 * You can store an object for a given unique key
 *
 * When there are too many elements in the cache it
 * shrinks automatically
 */
template <typename Key, typename Value, typename Hash=std::hash<Key>>
class SharedCache
{
private:
  class BucketType
  {
  private:
    struct BucketValue
    {
      BucketValue(Key k, Value v) :
        key(std::move(k)), value(std::move(v))
      { }

      Key key;
      Value value;
      std::atomic<unsigned int> busyness = 1;
    };

    typedef std::list<BucketValue>        BucketData;
    typedef typename BucketData::iterator BucketIterator;

    BucketData                            data;
    mutable boost::shared_mutex           mutex;

    BucketIterator findEntryFor(const Key& key)
    {
      return std::find_if(data.begin(), data.end(),
        [&](const BucketValue& item) {return item.key == key;});
    }

    void purgeCache()
    {
      unsigned sum = 0;
      for (const auto& bucketValue : data)
      {
        sum += bucketValue.busyness.load(std::memory_order_relaxed);
      }

      // in C++11 std::list::size is O(1) complexity
      double avg = static_cast<double>(sum) / static_cast<double>(data.size());

      auto iter = data.begin();
      while (iter != data.end())
      {
        // relaxed should be enough because of the
        // previous load(acquire)
        if (iter->busyness.load(std::memory_order_relaxed) < avg)
        {
          iter = data.erase(iter);
        }
        else
        {
          data->busyness.exchange(1, std::memory_order_relaxed);
          ++iter;
        }
      }
    }

  public:
    boost::optional<Value> valueFor(const Key& key)
    {
      boost::shared_lock<boost::shared_mutex> lock(mutex);
      const auto foundEntry = findEntryFor(key);

      if (foundEntry == data.end())
        return {};

      foundEntry->busyness.fetch_add(1, std::memory_order_relaxed);
      return {foundEntry->second};
    }

    void addOrUpdate(const Key& key, const Value& value, unsigned bucketLoad)
    {
      std::unique_lock<boost::shared_mutex> lock(mutex);
      const auto foundEntry = findEntryFor(key);

      if (foundEntry != data.end())
      {
        foundEntry->second = value;
        return;
      }

      if (data.size() > bucketLoad)
      {
        purgeCache();
      }

      data.emplace_back(key, value);
    }

    void remove(const Key& key)
    {
      std::unique_lock<boost::shared_mutex> lock(mutex);

      const auto foundEntry = findEntryFor(key);
      if (foundEntry != data.end())
      {
        data.erase(foundEntry);
      }
    }

  };

  std::vector<std::unique_ptr<BucketType>>  buckets;
  unsigned                                  bucketLoad;
  Hash                                      hasher;

  BucketType& getBucket(const Key& key) const
  {
    const std::size_t bucketIndex = hasher(key) % buckets.size();

    return *buckets[bucketIndex];
  }

public:
  typedef Key    key_type;
  typedef Value  mapped_type;
  typedef Hash   hash_type;

  SharedCache(unsigned numBuckets_ = 127,
        unsigned bucketLoad_ = 5,
        const Hash& hasher_ = Hash()) :
    buckets(numBuckets_), bucketLoad(bucketLoad_), hasher(hasher_)
  {
    for (auto& bucket : buckets)
    {
      bucket.reset(new BucketType());
    }
  }

  SharedCache(const SharedCache& other) = delete;
  SharedCache& operator=(const SharedCache& other) = delete;

  boost::optional<Value> valueFor(const Key& key) const
  {
    return getBucket(key).valueFor(key);
  }

  void addOrUpdate(const Key& key, const Value& value)
  {
    getBucket(key).addOrUpdate(key, value, bucketLoad);
  }

  void remove(const Key& key)
  {
    getBucket(key).remove(key);
  }

};

} // util
} // cc


#endif /* UTIL_CACHE_H_ */
