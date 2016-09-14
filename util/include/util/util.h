#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

// library includes
#include <cxxabi.h>
#include <typeinfo>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include <memory>
#include <stdint.h>

#include <odb/database.hxx>

#include <boost/shared_ptr.hpp>

// case insensitive LIKE operator
#ifdef DATABASE_PGSQL
#define SQL_ILIKE "ILIKE"
#else
#define SQL_ILIKE "LIKE"
#endif

// regex search
#ifdef DATABASE_PGSQL
#define SQL_REGEX "~*"
#else
#define SQL_REGEX "REGEXP"
#endif

// namespace
namespace cc
{
namespace util
{

enum FileType
{
  UNKNOWN,
  HEADER,
  SOURCE,
  OBJECT,
  BINARY
};

class TooLongException : public std::runtime_error {
public:
  TooLongException(const std::string& s) : std::runtime_error(s) {}
};

std::shared_ptr<odb::database> createDatabase(const std::string& connStr);

uint64_t fnvHash(const std::string& data);

std::string generateSha1Hash(const std::string& data_);

/**
 * \brief Null functions for the standard template library
 *
 * To use shared_ptr with ordered stl containers, like set, you often need to
 * create an other shared_ptr to compare the elements to it. This instance needs
 * not to delete the object, because, an other shared_ptr already ownes that.
 * These functions can be used there, and at other places, where you don't want
 * the default action to take place.
 *
 */
template<typename Param, typename Result> class UnaryNullFunction : public std::unary_function<Param, Result>
{
public:
  Result operator () (Param) {return Result();}
};

template<typename Left, typename Right, typename Result> class BinaryNullFunction : public std::binary_function<Left, Right, Result>
{
public:
  Result operator () (Left, Right) {return Result();}
};

/**
 * \brief Release deleter for std::shared_ptr
 *
 * There is no release function for std::shared_ptr, maybe, due to the problem
 * of defining what exactly release should do. However, you may want to
 * remove the ownership of the shared pointer, to manage the object manually.
 * These custom deleters can accomplish that feat.
 *
 */

template<typename Type> class ReleaseDelete : public std::unary_function<Type *,void>
{
public:
  // constructor
  ReleaseDelete() : _released(false) {}
  // function call operator deletes
  void operator () (Type * t) {if (!_released) delete t;}
  // function to release object
  void release() {_released = true;}

private:
  // stores released state
  bool _released;
};

template<typename Type> class ReleaseDelete<Type []> : public std::unary_function<Type *,void>
{
public:
  // constructor
  ReleaseDelete() : _released(false) {}
  // function call operator deletes
  void operator () (Type * t) {if (!_released) delete [] t;}
  // function to release object
  void release() {_released = true;}

private:
  // stores released state
  bool _released;
};

/**
 * \brief No delete for std::shared_ptr
 *
 * This deleter can be used by those smart pointers which shouldn't
 * delete the managed object when they run out of scope.
 */
struct NoDelete
{
  template<typename T>
  void operator()(T*) const {}
};

/**
 * \brief I/O related exception
 *
 * C++ has no default exception class for I/O related errors, iosteam
 * throws ios_base::failure sometime, but in my opinion the real I/O
 * errors (e.g some file is missing) should be speparated from logic
 * errors in the input, so this class is for pure I/O errors
 *
 */
class IOException : public std::exception
{
public:
  explicit IOException(const std::string & what_ = "IOException") : _msg(what_) {}
  ~IOException() throw() {}
  const char * what() const throw() {return _msg.c_str();}

private:
  std::string _msg;
};

/**
 * \brief Data processing related exceptions
 *
 * While IOException is only for external (I/O) related errors, this class is
 * used for logical errors in the data stream, be the source either an external
 * file, or an internal string.
 *
 */
class DataException : public std::exception
{
public:
  explicit DataException(const std::string & what_ = "DataException") : _msg(what_) {}
  ~DataException() throw() {}
  const char * what() const throw() {return _msg.c_str();}

private:
  std::string _msg;
};

/**
 * \brief Exception to throw when execution has reached a normally unreachable spot.
 *
 * When writing algorithms and programs, there are allways places, witch cannot
 * be reached theoritically. But sometimes people make mistakes, and the
 * program execution wanders into such a spot. When this happens, throw this
 * exception to make sure the error won't be unnoticed.
 *
 */
class UnreachableException : public std::logic_error
{
public:
  explicit UnreachableException(const std::string & what_ = "UnreachableException") : std::logic_error(what_) {}
};

/**
 * \brief Exception to throw when a pointer, that should point to an existing
 * object is NULL.
 *
 * One of the most probable runtime errors. It could be grouped with other
 * errors as well, but I think it's nice to give a separate class to it.
 */
class NullPointerException : public std::runtime_error
{
public:
  explicit NullPointerException(const std::string & what_ = "NullPointerException") : std::runtime_error(what_) {}
};


  bool isExtension(const std::string& path, const std::string& ext);
  std::string getFilename(const std::string& path);
  std::string getExtension(const std::string& path);
  std::string getPathAndFileWithoutExtension(const std::string& path);
  std::string getPath(const std::string& path);
  bool isCppFile(const std::string& path_);
  FileType getFileType(const std::string& path, FileType defaultType = FileType::UNKNOWN);
  bool isFileExist(const std::string& fn_);

/**
 * Monotonic timer using milliseconds
 */
long long int getTickCount();

/**
 * Surrounds the given string with the specified character. It is useful for
 * for quoting command line parameters of a shell execution.
 *
 * @param str_ the original string.
 * @param quoteCh_ quote character.
 * @return quoted string.
 */
inline std::string quoteString(
  const std::string& str_,
  const char quoteCh_ = '\'')
{
  return quoteCh_ + str_ + quoteCh_;
}

/**
 * Returns the demangled name of the type described by the given type info.
 *
 * @param info_ a type info.
 * @return the pretty name of the type.
 */
inline std::string getTypeName(const std::type_info& info_)
{
  int status = 0;
  std::unique_ptr<char[], void (*)(void*)> result(
    abi::__cxa_demangle(info_.name(), 0, 0, &status), std::free);

  return result.get() ? std::string(result.get()) : "##error##";
}

/**
 * Returns the template argument's demangled name.
 *
 * @return the pretty name of the T type.
 */
template <typename T>
inline std::string getTypeName()
{
  return getTypeName(typeid(T));
}

template <typename T1, typename T2>
bool isPrefix(const T1& prefix, const T2& where)
{
  return std::mismatch(prefix.begin(), prefix.end(), where.begin()).first == prefix.end();
}

template<typename T>
boost::shared_ptr<T> make_shared_ptr(std::shared_ptr<T>& ptr)
{
  return boost::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}

template<typename T>
std::shared_ptr<T> make_shared_ptr(boost::shared_ptr<T>& ptr)
{
  return std::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}
} // util
} // cc

#endif
