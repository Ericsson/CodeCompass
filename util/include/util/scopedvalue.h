#ifndef CC_UTIL_SCOPEDVALUE_H
#define CC_UTIL_SCOPEDVALUE_H

#include <memory>

#define DECLARE_SCOPED_TYPE(type) \
private: \
  type(const type&) = delete; \
  type(type&&) = delete; \
  type& operator=(const type&) = delete; \
  type& operator=(type&&) = delete; \


namespace cc
{
namespace util
{

  /**
   * @brief Scoped value manager.
   *
   * Temporarily stores the given value in a variable upon construction,
   * and restores its original value upon destruction.
   *
   * @tparam TValue The type of the variable and value to store.
   */
  template<typename TValue>
  class ScopedValue final
  {
    DECLARE_SCOPED_TYPE(ScopedValue)
    
  private:
    TValue& _storage;
    TValue _oldValue;

  public:
    ScopedValue(TValue& storage_, TValue&& newValue_) :
      _storage(storage_),
      _oldValue(std::move(_storage))
    {
      _storage = std::forward<TValue>(newValue_);
    }

    ~ScopedValue()
    {
      _storage = std::move(_oldValue);
    }
  };

} // util
} // cc

#endif // CC_UTIL_SCOPEDVALUE_H
