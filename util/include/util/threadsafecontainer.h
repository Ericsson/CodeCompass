#ifndef CC_UTIL_THREADSAFECONTAINER_H
#define CC_UTIL_THREADSAFECONTAINER_H

#include <mutex>

namespace cc
{
namespace util
{

/**
 * This template class implements a set-like thread-safe container.
 * @tparam Container A set-like container which has insert() method.
 * @tparam T The contained elements' type.
 */
template <typename Container, typename T = typename Container::value_type>
class ThreadSafeSet
{
public:
  /**
   * This function inserts an element to the container.
   * @return If the insertion was successful (i.e. the container didn't contain
   * the element before) then the function returns true.
   */
  bool insert(const T& item_)
  {
    std::lock_guard<std::mutex> guard(_mutex);
    return _container.insert(item_).second;
  }

  /**
   * Removes all items from the container.
   */
  void clear()
  {
    _container.clear();
  }

private:
  Container _container;
  std::mutex _mutex;
};

/**
 * This template class implements a map-like thread-safe container.
 * @tparam Container A map-like container.
 */
template <
  typename Container,
  typename Key = typename Container::key_type,
  typename Value = typename Container::mapped_type>
class ThreadSafeMap
{
public:
  /**
   * This function inserts a key-value pair in a thread safe way.
   * @return If the insertion was successful (i.e. the container didn't contain
   * the key before) then the function returns true.
   */
  bool insert(const std::pair<const Key, Value>& item_)
  {
    std::lock_guard<std::mutex> guard(_mutex);
    return _container.insert(item_).second;
  }

  /**
   * Returns a reference to the mapped value of the element with key equivalent
   * to the key_. If no such element exists, an exception of type
   * std::out_of_range is thrown.
   */
  const Value& at(const Key& key_) const
  {
    std::lock_guard<std::mutex> guard(_mutex);
    return _container.at(key_);
  }

  /**
   * This function returns true if the container contains an element with the
   * given key.
   */
  bool contains(const Key& key_) const
  {
    return _container.find(key_) != _container.end();
  }

  /**
   * Removes all elements from the container.
   */
  void clear()
  {
    _container.clear();
  }

private:
  Container _container;
  mutable std::mutex _mutex;
};

}
}

#endif
