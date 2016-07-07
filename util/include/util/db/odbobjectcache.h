/*
 * odbobjectcache.h
 *
 *  Created on: Jul 26, 2013
 *      Author: ezoltbo
 */

#ifndef CC_UTIL_ODBOBJECTCACHE_H
#define CC_UTIL_ODBOBJECTCACHE_H

#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <mutex>
#include <memory>

#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <odb/database.hxx>

#include <util/exceptions.h>

namespace cc 
{
namespace util 
{

template <typename Key, typename Value, typename Hash=std::hash<Key>>
class OdbObjectCache
{
private:
  class BucketType
  {
  private:
    typedef std::pair<Key, Value>         BucketValue;
    typedef std::list<BucketValue>        BucketData;
    typedef typename BucketData::iterator BucketIterator;

    BucketData                            data;
    mutable boost::shared_mutex           mutex;

    BucketIterator findEntryFor(const Key& key)
    {
      return std::find_if(data.begin(), data.end(),
        [&](const BucketValue& item) {return item.first == key;});
    }

  public:
    Value valueFor(const Key& key)
    {
      boost::shared_lock<boost::shared_mutex> lock(mutex);
      const auto foundEntry = findEntryFor(key);

      if (foundEntry == data.end())
        throw ItemNotFound();

      return foundEntry->second;
    }

    void insertToCache(const Key& key, const Value& value)
    {
      std::unique_lock<boost::shared_mutex> lock(mutex);
      const auto foundEntry = findEntryFor(key);

      if (foundEntry != data.end())
        return;

      data.push_back(BucketValue(key, value));
    }

    Value getOrInsert(odb::database* db, const Key& key, Value& value)
    {
      std::unique_lock<boost::shared_mutex> lock(mutex);
      const auto foundEntry = findEntryFor(key);

      if (foundEntry == data.end())
      {
        db->persist(value);
        data.push_back(BucketValue(key, value));
        return value;
      }
      else
      {
        return foundEntry->second;
      }
    }

    void removeMapping(const Key& key)
    {
      std::unique_lock<boost::shared_mutex> lock(mutex);
      const auto foundEntry = findEntryFor(key);
      if (foundEntry != data.end())
      {
        data.erase(foundEntry);
      }
    }

  };

  odb::database *database;
  std::vector<std::unique_ptr<BucketType>> buckets;
  Hash hasher;

  BucketType& getBucket(const Key& key) const
  {
    const std::size_t bucketIndex = hasher(key) % buckets.size();

    return *buckets[bucketIndex];
  }

public:
  typedef Key key_type;
  typedef Value mapped_type;
  typedef Hash hash_type;

  OdbObjectCache(odb::database *db,
                 unsigned numBuckets = 127,
                 const Hash& hasher_ = Hash()) :
    database(db), buckets(numBuckets), hasher(hasher_)
  {
    for (auto& bucket : buckets)
    {
      bucket.reset(new BucketType());
    }
  }

  OdbObjectCache(const OdbObjectCache& other) = delete;
  OdbObjectCache& operator=(const OdbObjectCache& other) = delete;

  Value valueFor(const Key& key) const
  {
    return getBucket(key).valueFor(key);
  }

  void insertToCache(const Key& key, const Value& value)
  {
    getBucket(key).insertToCache(key, value);
  }

  Value getOrInsert(const Key& key, Value& value)
  {
    return getBucket(key).getOrInsert(database, key, value);
  }

  void removeMapping(const Key& key)
  {
    getBucket(key).removeMapping(key);
  }

};

} // util
} // cc

#endif /* CC_UTIL_ODBOBJECTCACHE_H */
