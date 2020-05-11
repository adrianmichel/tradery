/*
   Copyright (C) 2018-2020 Adrian Michel

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

// to generate compiler warnings with location, that are clickable in the
// compiler output window ex: #pragma message(__LOC__"Need to do exception
// handling")

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__) ") : "

#define __TODO__ __LOC__ "warning Todo: "
#define todo __TODO__

#ifdef MISC_EXPORTS
#define MISC_API __declspec(dllexport)
#else
#define MISC_API __declspec(dllimport)
#endif

/** @file
 *  \brief Contains various classes, typedefs, defines
 */

#include <sstream>
#include <vector>
#include <math.h>
#include <list>
#include <iostream>
#include <memory>
#include <assert.h>
#include <map>
#include <set>
#include <algorithm>
#include "stringconv.h"

#include "rpc.h"
#include "rpcdce.h"

#include "strings.h"
#include "stringformat.h"
#include "sharedptr.h"
#include "datetime.h"
#include <mutex>

namespace tradery {
/**\defgroup Uitilities Utility definitions
 * @{
 */

/** \defgroup MinMax Min and Max
 * Definitions of min and max template functions with 2 respective 3 operators
 * @{
 */
template <class T>
const T& max2(const T& x, const T& y) {
  return x > y ? x : y;
}
template <class T>
const T& min2(const T& x, const T& y) {
  return x < y ? x : y;
}
template <class T>
const T& max3(const T& x, const T& y, const T& z) {
  return max2(x, max2(y, z));
}
template <class T>
const T& min3(const T& x, const T& y, const T& z) {
  return min2(x, min2(y, z));
}
//@}

/**
 * a unique id, currently implemented as a string
 */

class UniqueIdException {
 private:
  const std::string _id;

 public:
  UniqueIdException(const char* id) : _id(id) {}

  UniqueIdException(const std::string& id) : _id(id) {}

  const std::string& id() const { return _id; }
};

class MISC_API UniqueId {
 private:
  std::string id;

 public:
  UniqueId();
  UniqueId(const std::string& id);
  UniqueId(const char* id);

  UniqueId(const TCHAR* id);
  UniqueId(const UniqueId& id);

  operator const std::string() const;
  operator const std::wstring() const;
  static UniqueId generate();

  const std::string& str() const;
  bool operator<(const UniqueId& other) const;
  bool operator==(const UniqueId& other) const;
  const UniqueId& operator=(const UniqueId& id);
  operator const std::string& () { return id; }
};

inline std::ostream& operator<<(std::ostream& os, UniqueId id) {
  os << id.str();
  return os;
}

class UniqueIdVector : public std::vector<UniqueId> {
 public:
  UniqueIdVector(const std::vector<std::string>& ids) {
    *this = ids;
  }

  UniqueIdVector() {}
};

using UniqueIdPtrVector = std::vector<const UniqueId*>;

/**
 * This mutex is used internally by print and dump calls.
 *
 * It is made available so the user code can synchronize its own threads with
 * these functions if necessary
 *
 * @see std::wstring
 * @see PositionsManager::dump
 */
extern MISC_API std::mutex m;
extern MISC_API std::mutex m_debug;

//@}



template< typename T > using PtrVector = std::vector< std::shared_ptr< T > >;

/**
 * A vector of pointers to constant objects
 *
 * A regular STL vector of pointers does not delete the pointers in the
 * destructor. At the same time it is not a good idea to use smart pointers in a
 * vector because of assignment and copy issues so PtrVector implements a vector
 * of regular pointers whose destructor deletes all pointers
 */
template <class T>
class ConstPtrVector : public PtrVector<const T> {};

// note: erase has not been overriden, so it will not delete the pointer!
template <class T, class U>
class PtrMap : public std::map<T, std::shared_ptr< U> > {
 private:
   using PtrMapBase = std::map<T, std::shared_ptr< U >>;

 public:
  virtual ~PtrMap() { }

};

/**
 * A map of an arbitrary type T to pointers to constant objects of type U
 *
 * A regular STL map of pointers does not delete the pointers in the destructor.
 * At the same time it is not a good idea to use smart pointers because of
 * assignment and copy issues so PtrMap implements a map of regular pointers
 * whose destructor deletes all pointers
 */
template <class T, class U>
class ConstPtrMap : public PtrMap<T, const U> {};

template <class T>
class PtrSet : public std::set<T*> {
 private:
  using PtrSetBase = std::set<T*>;

 public:
  virtual ~PtrSet() { deleteAll(); }

  void deleteAll() {
    for (auto i : *this) delete i;
  }

  void clear() {
    deleteAll();
    PtrSetBase::clear();
  }
};

template <class T>
class ConstPtrSet : public PtrSet<const T> {};

/** \brief a vector of strings
 *
 * Can be used as a base class for a polymorphic multiply derived class, because
 * of the virtual destructor
 */

using StrVectorBase = std::vector<std::string>;

class StrVector : public StrVectorBase {
 public:
  StrVector(const StrVectorBase& v) : StrVectorBase(v) {}

  StrVector() {}

  virtual ~StrVector() {}

  StrVector& operator=(const StrVector& sv) {
    if (this != &sv) __super::operator=(sv);

    return *this;
  }
};

using StrSet = std::set<std::wstring>;

//@}
// end Pointers


/**
 * Generic Info that all plug-ins and plug-in configurations, as well as other
 * classes must have.
 *
 * Info consists of a UniqueId, a name and a description
 */
class Info {
 private:
  const UniqueId _id;
  const std::string _name;
  const std::string _description;

 public:
  /**
   * Constructor taking 3 arguments: UniqueId, and the name and description,
   * which can be any arbitrary strings
   *
   * @param id     the unique id
   * @param name   The name
   * @param description
   *               The description
   */
  Info(const UniqueId& id, const std::string& name, const std::string& description)
      : _id(id), _name(name), _description(description) {}

  Info(const std::string& name, const std::string& description)
      : _name(name), _description(description) {}
  /**
   * Copy constructor - takes another Info as argument
   *
   * @param info   The source Info for the copy operation
   */
  Info(const Info& info)
      : _id(info.id()), _name(info.name()), _description(info.description()) {}
  /**
   * default constructor - generates a new unique id and sets the name and
   * description to empty strings
   */
  Info() {}

  /**
   * Returns the unique id
   *
   * @return the unique id
   */
  const UniqueId& id() const { return _id; }
  /**
   * Returns the name
   *
   * @return the name
   */
  const std::string& name() const { return _name; }
  /**
   * Returns the description
   *
   * @return the description
   */
  const std::string& description() const { return _description; }

  std::string toString() const {
    return tradery::format("id: %1%, name: %2%, desc: %3%", id().str(), name(), description());
  }
};

using InfoPtr = std::shared_ptr<Info>;
using UniqueIdPtr = std::shared_ptr<UniqueId>;

/**\defgroup Diagnostic Diagnostic classes
 * Definitions of error handling related classes
 *
 * Because trading systems run in a multithreaded environment, and because C++
 * exceptions cannot be thrown across different threads an signalnate mechanism
 * for error handling must be used for signaling error conditions in trading
 * systems.
 *
 * Trading systems signal error conditions by sending error events to an error
 * sink.
 *
 *
 * @{
 */

/**
 * Control characters for string attributes
 *
 * @see OutputBuffer
 * @see OutputSink
 */
enum Control {
  // colors
  def,
  black,
  blue,
  green,
  red,
  yellow,
  purple,
  gray,

  // attributes
  bold,
  not_bold,
  italic,
  not_italic,
  underline,
  not_underline,
  reset,
  endl
};

class OutputBuffer;

/**
 * Abstract base class for output sinks
 *
 * An output sink is a class that can print formatted strings. The various
 * plug-ins can use the output sink that they receive to print out diagnostic
 * messages.
 *
 *
 * @see Control
 * @see OutputBuffer
 */
class OutputSink {
 private:
  mutable std::mutex _mx;

 public:
  virtual ~OutputSink() {}

  /**
   * Method for printing a string
   *
   * Must be implemented by a concrete OutputSink.
   *
   * Although this method can be called directly by the user code, it is
   * recommended that all output be done using an OutputBuffer, which ensures
   * that output from different threads does not get mixed up (see the print(
   * OutputBuffer& ) method )
   *
   * @param str    The string to print
   *
   * @see OutputBuffer
   */
  virtual void print(const std::string& str) = 0;
  /**
   * Method for printing a string
   *
   * Must be implemented by a concrete OutputSink.
   *
   * Although this method can be called directly by the user code, it is
   * recommended that all output be done using an OutputBuffer, which ensures
   * that output from different threads does not get mixed up (see the print(
   * OutputBuffer& ) method )
   *
   * @param str    The string to print
   *
   * @see OutputBuffer
   */
  virtual void printLine(const std::string& str) = 0;
  /**
   * Prints a control character, i.e. all strings printed after this control
   * character have the characteristic set by the control character, such as
   * bold, italics, underlined, color.
   *
   * Must be implemented by a concrete output sink.
   *
   * Although this method can be called directly by the user code, it is
   * recommended that all output be done using an OutputBuffer, which ensures
   * that output from different threads does not get mixed up (see the print(
   * OutputBuffer& ) method ).
   *
   * @param ctrl   The control character
   *
   * @see OutputBuffer
   * @see Control
   */
  virtual void print(Control ctrl) = 0;

  /**
   * Sends the contents of the output buffer to the output sink.
   *
   * This method can be used safely from different threads to output information
   * to the output sink, as the whole buffer is printed at once.
   *
   * The output buffer is cleared upon return of this function.
   *
   * Example:
   *
   * \code
   * OutputSink& os( getOuputSink() ); // get an output sink somehow
   * OutputBuffer ob; // create a buffer
   * os.print( ob << "This is a test string" << blue << " and this part is blue"
   * << bold << " and this part is also bold" << endl ); \endcode
   *
   * @param ob     The OutputBuffer to be sent to the OutputSink
   *
   * @see OutputBuffer
   */
  void print(OutputBuffer& ob);
  /**
   * Sends the contents of the output buffer to the output sink.
   *
   * This method can be used safely from different threads to output information
   * to the output sink, as the whole buffer is printed at once.
   *
   * The output buffer is cleared upon return of this function.
   *
   * An end of line character is appended to the buffer.
   *
   * Example:
   *
   * \code
   * OutputSink& os( getOuputSink() ); // get an output sink somehow
   * OutputBuffer ob; // create a buffer
   * os.print( ob << "This is a test string" << blue << " and this part is blue"
   * << bold << " and this part is also bold" ); \endcode
   *
   *
   * @param ob     The OutputBuffer to be sent to the OutputSink
   *
   * @see OutputBuffer
   */
  void printLine(OutputBuffer& ob);

  /**
   * Clears the OutputSink window.
   *
   * Must be implemented by the concrete OutputSink class.
   *
   * This should only be called in extreme cases by the user code, as multiple
   * threads send their output to the same OutputSink
   */
  virtual void clear() = 0;
};

/**
 * Buffer for output strings and control characters
 *
 * Use in conjunction with OutputSink to print formatted output in a thread safe
 * manner.
 *
 * First the user sends all the strings and control characters to the buffer
 * using the << operator. Then the one of the OutputSink::print methods is
 * called
 *
 * Example:
 *
 * \code
 * OutputSink& os( getOuputSink() ); // get an output sink somehow
 * OutputBuffer ob; // create a buffer
 * os.print( ob << "This is a test string" << blue << " and this part is blue"
 * << bold << " and this part is also bold" ); \endcode
 *
 *
 */
class OutputBuffer {
  friend OutputSink;

 private:
  class Element {
   public:
    virtual ~Element() {}
    virtual void print(OutputSink& os) = 0;
  };

  class StringElement : public Element {
   private:
    std::string _str;

   public:
    StringElement(const std::string& str) : _str(str) {}
    virtual void print(OutputSink& os) { os.print(_str); }
  };

  class ControlElement : public Element {
   private:
    Control _ctrl;

   public:
    ControlElement(Control ctrl) : _ctrl(ctrl) {}
    virtual void print(OutputSink& os) { os.print(_ctrl); }
  };

  std::vector<std::shared_ptr<Element> > _buffer;

 public:
  /**
   * Appends a string to the buffer
   *
   * @param str    string to append
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(const std::string& str) {
    _buffer.push_back(std::make_shared< StringElement >(str));
    return *this;
  }
  OutputBuffer& operator<<(const std::wstring& str) {
    return *this << tradery::ws2s(str);
  }
  /**
   * Appends a single character to the buffer
   *
   * @param str    The character to append
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(const char* str) {
    return *this << std::string(str);
  }
  /**
   * Appends the contents of a stringstream to the buffer
   *
   * @param tos    The stringstream
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(const std::ostringstream& tos) {
    return *this << tos.str();
  }
  /**
   * Appends the string representation of an int to the buffer
   *
   * @param n      The int value to be appended
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(int n) {
    std::ostringstream os;
    os << n;
    return *this << os;
  }
  /**
   * Appends the string representation of a long to the buffer
   *
   * @param l      The long value to be added to the buffer
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(long l) {
    std::ostringstream os;
    os << l;
    return *this << os;
  }
  /**
   * Appends the string representation of an unsigned int to the buffer
   *
   * @param un     The unsigned int value to be added to the buffer
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(unsigned int un) {
    std::ostringstream os;
    os << un;
    return *this << os;
  }
  /**
   * Appends the string representation of an unsigned long to the buffer
   *
   * @param ul     The unsigned long value to be appended
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(unsigned long ul) {
    std::ostringstream os;
    os << ul;
    return *this << os;
  }
  /**
   * Appends the string representation of a bool to the buffer
   *
   * @param b      The bool value to be appended
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(bool b) { return *this << (b ? "true" : "false"); }
  /**
   * Appends the string representation of a double to the buffer
   *
   * @param d      The double value to be appended
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(double d) {
    std::ostringstream os;
    os << d;
    return *this << os;
  }
  /**
   * Addes a control character to the buffer
   *
   * @param control The control character to be appended
   *
   * @return a reference to the current buffer to enable chained calls
   */
  OutputBuffer& operator<<(Control control) {
    _buffer.push_back(std::make_shared< ControlElement >(control));
    return *this;
  }

 private:
  void print(OutputSink& os) {
    for (auto v : _buffer) {
      v->print(os);
    }

    _buffer.clear();
  }
};

/* @cond */
inline void OutputSink::print(OutputBuffer& ob) {
  ob << reset;
  std::scoped_lock lock(_mx);
  ob.print(*this);
}

inline void OutputSink::printLine(OutputBuffer& ob) { print(ob << endl); }

inline void printLine(OutputSink* os, OutputBuffer& ob) {
  assert(os != 0);
  os->printLine(ob);
}

/* @endcond */

//@}

/**
 * Abstract base for classes that are to receive run events.
 *
 * It is also base for the SessionEventHandler class, which defines additional
 * session specific events
 *
 * The assumption that RunEventHandler implementation must make is that these
 * events may be sent from a thread that is different than the one that was used
 * to create the handler object, therefore the user code must make sure that
 * multi-threading issues such as concurrent access to shared data, GUI
 * multithreaded issues etc. are dealt with appropriately.
 *
 * @see SessionEventHandler
 */
class RunEventHandler {
 public:
  virtual ~RunEventHandler() {}

  /**
   * Called when the run is about to start
   */
  virtual void runStarted() = 0;
  /**
   * Called after a cancel request has been received
   */
  virtual void runCanceled() = 0;
  /**
   * Called after a run has ended
   */
  virtual void runEnded() = 0;
};

class Range;
class DataRequester;

class VersionException {
 private:
  const std::string _message;

 public:
  VersionException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

constexpr auto VERSION_SEPARATOR = ".";
constexpr auto VERSION_SEPARATORS = ".,";

/**
 * Implements the concept of version
 *
 * Has a major number, minor number and build number
 *
 * It is passed to the module by the method modInit to indicate the current
 * GuiApp version. Needs to be returned by the module to indicate which tradery
 * version it was written for.
 *
 * Not used currently, but will be used to make future versions of tradery
 * back compatible with older versions
 */
class MISC_API Version {
 private:
  unsigned int m_major;
  unsigned int m_minor;
  unsigned int m_revision;
  unsigned int m_build;

  void parse(const char* version);

 public:
  /**
   * Default constructor - sets the current version from the package version
   * string
   */
  Version();
  /**
   * Constructor - takes the 4 values that define a version as parameters
   *
   * @param major
   * @param minor
   * @param build
   * @param revision
   */
  Version(unsigned int major, unsigned int minor, unsigned int revision, unsigned int build);

  /**
   * Constructor from string. Ex: "2.0.25.10"
   * accepts anything in between 1 and 4 fields:
   * 1
   * 1.2
   * 1.2.3
   * 1.2.3.4
   */
  Version(const std::string& version);
  Version(const char* version);

  /**
   * Returns the major number
   *
   * @return the major number
   */
  unsigned int major() const { return m_major; }

  /**
   * Returns the minor number
   *
   * @return the minor number
   */
  unsigned int minor() const { return m_minor; }

  /**
   * returns the revision number
   *
   * @return the revision number
   */
  unsigned int revision() const { return m_build; }
  /**
   * returns the build number
   *
   * @return the build number
   */
  unsigned int build() const { return m_build; }

  const std::string toString() const {
    return tradery::format( m_major, '.', m_minor, '.', m_revision, '.', m_build );
  }
};

/**
 * abstract base for classes that can be cloned
 * used for plug-in configurations that are cloned in order to be instantiated
 * The T template parameter should be the plugin type (Runnable, Slippage)
 */
template <class PluginType>
class Clonable {
 public:
  virtual ~Clonable() {}
  // params contains strings passed to the plugin configuration at runtime
  virtual std::shared_ptr<PluginType> clone(const std::vector<std::string>* params) const = 0;
};

/**
 * specific cloneable configuration classes will derive from this class
 * T is the plugin type (Runnable, etc), U is the actual type of the
 * configuration, derived from T For example <Runnable, System1>
 */
template <class PluginType, class U>
class ClonableImpl : public Clonable<PluginType> {
 public:
  virtual std::shared_ptr<PluginType> clone(const std::vector<std::string>* params) const {
    return std::make_shared<U>(params);
  }
};

#pragma warning(default : 4800)
/**
 * Diagnostic class - counts instances of a class as they are created and
 * destroyed -
 *
 * can be used to make sure that all instances of a class are destroyed, or to
 * see how many instances there are at any given moment
 *
 * By default _os is initialized with std::cout, but it can be assigned another
 * reference at runtime
 */
class ObjCount {
 private:
  MISC_API static int _objCount;
  MISC_API static std::wostream& _os;
  MISC_API static int _totalObjects;

  const std::string _name;

 public:
  ObjCount(const std::string& name = "") : _name(name) {
    _objCount++;
    _totalObjects++;
    std::ostringstream o;
    o << _name << " constructor, count: " << _objCount
      << ", total objects: " << _totalObjects << std::endl;
  }

  virtual ~ObjCount() {
    _objCount--;
    std::ostringstream o;
    o << _name << " destructor, count: " << _objCount << std::endl;
  }
};

/**
 * Generates file name for a symbol file.
 *
 * Handles flat data (all symbols files in the same directory), or hierarchical
 * data (symbol files in nested directories).
 *
 * By default, this class generates the file name, but derived classes can also
 * generate the necessary subdirectories, by overrideing the the virtual method
 * createDirectory.
 *
 * In case of nested directories, there is a base dir, under which 2 levels are
 * created: The first is the first char of the symbol ( with transformation for
 * invalid file characters), the second level being the second character of the
 * symbol. In case of one char symbols, the two levels are both the symbol.
 */
class FileName {
 private:
  const bool _flatData;

  TCHAR transformInvalidFileChars(TCHAR c) const {
    if (c == _TCHAR('\\'))
      return _TCHAR('a');
    else if (c == _TCHAR('/'))
      return _TCHAR('b');
    else if (c == _TCHAR(':'))
      return _TCHAR('c');
    else if (c == _TCHAR('*'))
      return _TCHAR('d');
    else if (c == _TCHAR('?'))
      return _TCHAR('e');
    else if (c == _TCHAR('\"'))
      return _TCHAR('f');
    else if (c == _TCHAR('<'))
      return _TCHAR('g');
    else if (c == _TCHAR('>'))
      return _TCHAR('h');
    else if (c == _TCHAR('|'))
      return _TCHAR('i');
    else
      return c;
  }

 public:
  FileName(bool flatData) : _flatData(flatData) {}

  virtual ~FileName() {}

  // path is the root path, symbol is used to generated nested subdirs using its
  // first and second char depending on the "flatData" member, and filename is
  // the name part including the extension
  const std::string makePath(const std::string& p, const std::string& symbol, const std::string& fileName) const {
    std::ostringstream path;
    path << addFSlash(p);

    if (!_flatData) {
      std::string first(1, transformInvalidFileChars(symbol[0]));
      std::string second(1, symbol.length() == 1 ? symbol[0] : transformInvalidFileChars(symbol[1]));

      path << first;

      createDir(path.str());

      path << "\\" << second;

      createDir(path.str());

      path << "\\";
    }

    path << fileName;

    return path.str();
  }

  virtual void createDir(const std::string& path) const {}
};


class ThreadAlgorithm {
 private:
  unsigned int m_mask;

 public:
  ThreadAlgorithm() : m_mask(2) {}

  ThreadAlgorithm(unsigned int mask) : m_mask(mask) {}

  bool oneSystemInMultipleThreds() const { return (m_mask & 0x01) != 0; }
  bool processorAffinity() const { return (m_mask & 0x02) != 0; }
};

inline std::string timeStamp(bool fracSeconds, char timeSeparator = ':') {
  const DateTime time = LocalTimeSec();
  const DateTime timeFrac = LocalTimeSubSec();

  long frac = timeFrac.time_of_day().fractional_seconds();

  std::string timeStr = time.to_simple_string();

  for (std::string::size_type n = 0; n < timeStr.length(); n++) {
    if (timeStr.at(n) == ':') {
      timeStr.replace(n, 1, std::string(1, timeSeparator));
    }
  }

  if (fracSeconds) {
    return timeStr + '.' + tradery::format( frac, "000" ).substr(0, 3);
  }
  else {
    return timeStr;
  }
}

MISC_API std::vector<std::string> cmdLineSplitter(const std::string& line);

}  // end namespace tradery
// utility macro - define a basic exception class
#define BASIC_EXCEPTION(name)                                            \
  \
class name : public std::exception\
{\
\
public : name(const std::string& message) :                              \
                 std::exception(message.c_str()){} std::string message() \
                     const {return __super::what();                      \
  }                                                                      \
  \
}                                                                     \
  ;

#define BASIC_EXCEPTION_DEF_CONSTRUCTOR(name) \
  \
BASIC_EXCEPTION(name##Base)                   \
  \
class name : public name##Base\
{              \
    \
public : name() : name##Base(""){}            \
  \
};

////////////////////
///
inline double round(double x) {
  double intpart;
  double frac = modf(x, &intpart);
  return frac >= 0.5 || (frac < 0 && frac > -0.5) ? ceil(x) : floor(x);
}

//@}

// using namespace tradery
