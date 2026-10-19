#ifndef CC_UTIL_HASH_H
#define CC_UTIL_HASH_H

#include <cstdint>
#include <string>
#include <sstream>

#include <boost/version.hpp>
#if BOOST_VERSION >= 106800 /* 1.68.0 */
#include <boost/uuid/detail/sha1.hpp>
#else
#include <boost/uuid/sha1.hpp>
#endif

namespace cc
{
namespace util
{

inline std::uint64_t fnvHash(const std::string& data_)
{
  std::uint64_t hash = 14695981039346656037ULL;

  std::size_t len = data_.length();
  for (std::size_t i = 0; i < len; ++i)
  {
    hash ^= static_cast<std::uint64_t>(data_[i]);
    hash *= static_cast<std::uint64_t>(1099511628211ULL);
  }

  return hash;
}

inline std::string sha1Hash(const std::string& data_)
{
  using namespace boost::uuids::detail;

  sha1 hasher;
  boost::uuids::detail::sha1::digest_type digest;

  hasher.process_bytes(data_.c_str(), data_.size());
  hasher.get_digest(digest);

  std::stringstream ss;
  ss.setf(std::ios::hex, std::ios::basefield);
  ss.width(8);
  ss.fill('0');

  // To ensure correct output, especially for newer Boost versions where digest might be treated as 20 bytes,
  // we explicitly cast the relevant 4 bytes into a uint32_t.
  // For older Boost, digest[i] is already a uint32_t.
  // For newer Boost, digest is a uint8_t[20]. We need to reconstruct the 32-bit values.
#if BOOST_VERSION >= 108600 /* 1.86.0 */
  for (int i = 0; i < 5; ++i)
  {
    uint32_t part = (static_cast<uint32_t>(digest[i * 4]) << 24) |
                    (static_cast<uint32_t>(digest[i * 4 + 1]) << 16) |
                    (static_cast<uint32_t>(digest[i * 4 + 2]) << 8) |
                     static_cast<uint32_t>(digest[i * 4 + 3]);
    ss << part;
  }
#else
  for (int i = 0; i < 5; ++i)
    ss << digest[i];
#endif

  return ss.str();
}

} // util
} // cc

#endif // CC_UTIL_HASH_H
