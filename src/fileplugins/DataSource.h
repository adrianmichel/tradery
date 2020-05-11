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

#include <boost/algorithm/string.hpp>

#include <datasource.h>

//#define FILE_DS_DUMP

enum Format {
  format1,  // mm/dd/yyyy,h:m:s,o,h,l,c,v
  format2,  // yymmdd,hhmm,o,h,l,c,v
  format3,  // mm/dd/yyyy,o,h,l,c,v
  format4,  // yyyymmdd,o,h,l,c,v
  no_format
};

class TimeException {
 private:
  const std::string _time;

 public:
  TimeException(const std::string& time) : _time(time) {}

  const std::string time() const { return _time; }
};

class DataFileException {};

inline bool is_nl(std::ifstream::char_type c) {
  return c == TCHAR('\n') || c == TCHAR('\r');
}

inline __int64 fileSize(std::istream& file) {
  file.seekg(0, ios_base::end);
  return file.tellg();
}

// get the position in the file of the beginning of a line which contains pos

class PosLine {
 private:
  std::string _line;
  __int64 _pos;

 public:
  PosLine() : _pos(-1) {}

  PosLine(const std::string& line, __int64 pos) : _line(line), _pos(pos) {}

  const PosLine& operator=(const PosLine& pos) {
    if (this != &pos) {
      _line = pos.line();
      _pos = pos.pos();
    }
    return *this;
  }

  const std::string& line() const { return _line; }
  __int64 pos() const { return _pos; }

  operator bool() const { return _pos >= 0; }

  std::string toString() const {
    return "line: "s + _line + ", pos: " + std::to_string( _pos );
  }
};

// skips nl chars starting at current position going back
// stops when current char is not nl, or beginning of file
inline bool skipNLInc(std::istream& file) {
  for (std::istream::int_type c = file.peek(); is_nl(c) && !file.eof(); file.seekg(1, ios_base::cur), c = file.peek());
  return !file.eof();
}

inline void skipNLDec(std::istream& file) {
  for (std::istream::int_type c = file.peek(); is_nl(c) && file.tellg() > 0; file.seekg(-1, ios_base::cur), c = file.peek());
}

inline bool findFirstLineChar(std::istream& file) {
  skipNLDec(file);
  for (std::istream::int_type c = file.peek(); !is_nl(c) && file.tellg() > 0; file.seekg(-1, ios_base::cur), c = file.peek());
  return skipNLInc(file);
}

inline const PosLine getCrtLine(__int64 pos, std::istream& file) {
  if (pos >= fileSize(file)) {
    return PosLine();
  }

  // set file pointer at pos
  file.seekg(pos);

  if (file.fail()) {
    return PosLine();
  }

  if (findFirstLineChar(file)) {
    std::string str;
    __int64 pos = file.tellg();
    std::getline(file, str);
    boost::trim(str);
    return PosLine(str, pos);
  }
  else {
    return PosLine();
  }
}

inline const PosLine getPrevLine(__int64 pos, std::istream& file) {
  PosLine pl = getCrtLine(pos, file);

  return pl ? getCrtLine(pl.pos() - 1, file) : pl;
}

inline const PosLine getNextLine(__int64 pos, std::istream& file) {
  file.seekg(pos);

  if (file.fail()) {
    return PosLine();
  }

  bool eol = false;

  while (!file.eof()) {
    pos = file.tellg();
    std::istream::int_type c = file.get();

    if (is_nl(c)) {
      eol = true;
    }
    else if (eol) {
      return getCrtLine(pos, file);
    }
  }
  return PosLine();
}

inline const PosLine getFirstLine(std::istream& file) {
  file.seekg(0);

  if (file.fail()) {
    return PosLine();
  }

  std::string str;

  std::getline(file, str);

  return PosLine(str, 0);
}

inline const PosLine getLastLine(std::istream& file) {
  return getCrtLine(fileSize(file) - 1, file);
}

class PosDateTime {
 private:
  DateTime _dateTime;
  __int64 _pos;
  __int64 _lineSize;

 public:
  PosDateTime(const DateTime& dateTime, __int64 pos, __int64 lineSize)
      : _dateTime(dateTime), _pos(pos), _lineSize(lineSize) {}

  PosDateTime(const PosDateTime& dateTime)
      : _dateTime(dateTime.dateTime()), _pos(dateTime.pos()) {}

  PosDateTime() : _pos(-1), _lineSize(-1) {}

  bool operator>(const PosDateTime& posDateTime) const {
    return _dateTime > posDateTime.dateTime();
  }

  bool operator>=(const PosDateTime& posDateTime) const {
    return _dateTime >= posDateTime.dateTime();
  }

  bool operator==(const PosDateTime& posDateTime) const {
    return _dateTime == posDateTime.dateTime();
  }

  bool operator!=(const PosDateTime& posDateTime) const {
    return _dateTime != posDateTime.dateTime();
  }

  bool operator<(const PosDateTime& posDateTime) const {
    return _dateTime < posDateTime.dateTime();
  }

  bool operator>(const DateTime& dateTime) const {
    return _dateTime > dateTime;
  }

  bool operator>=(const DateTime& dateTime) const {
    return _dateTime >= dateTime;
  }

  bool operator==(const DateTime& dateTime) const {
    return _dateTime == dateTime;
  }

  bool operator<(const DateTime& dateTime) const {
    return _dateTime < dateTime;
  }

  bool operator!=(const DateTime& dateTime) const {
    return _dateTime != dateTime;
  }

  operator const DateTime&() const { return _dateTime; }

  operator bool() const { return _pos >= 0 || _lineSize >= 0; }

  const PosDateTime& operator=(const PosDateTime& posDateTime) {
    if (this != &posDateTime) {
      _dateTime = posDateTime.dateTime();
      _pos = posDateTime.pos();
    }
    return *this;
  }

  const DateTime& dateTime() const { return _dateTime; }
  __int64 pos() const { return _pos; }
  double posAsDouble() const { return (double)_pos; }

  std::string toString() const {
    return "date: "s + _dateTime.date().toString() + ", pos: " + std::to_string( _pos );
  }

  __int64 lineSize() const { return _lineSize; }
};

using FilePositionInfoBase = std::pair<unsigned __int64, unsigned __int64>;

class FilePositionInfo : public FilePositionInfoBase {
 public:
  FilePositionInfo() : FilePositionInfoBase(0, 0) {}

  FilePositionInfo(unsigned __int64 start, unsigned __int64 count)
      : FilePositionInfoBase(start, count) {}

  unsigned __int64 start() const { return __super::first; }
  unsigned __int64 count() const { return __super::second; }

  bool operator!() const { return FilePositionInfoBase::second > 0; }
};

using FilePositionInfoPtr = std::shared_ptr<FilePositionInfo>;

class Header : public Tokenizer {
 public:
  Header(const std::string& line) : Tokenizer(line, ", \t") {}
};

/**
 * File data source - derived from DataSource
 *
 * @see DataSource
 */
class FileDataSource : public tradery::DataSource {
 private:
  ErrorHandlingMode _errorHandlingMode;

  const PosDateTime timeStamp(__int64 pos, std::istream& file) const {
    assert(pos >= 0);
    PosLine pl(getCrtLine(pos, file));
    try {
      BarPtr bar(BarPtr(parseBarLine(pl.line())));
      return PosDateTime(bar->time(), pl.pos(), pl.line().length());
    }
    catch (...) {
      // if the bar couldn't be created, return an emtpy PosDateTime
      // signaling that this was a line with no data
      return PosDateTime();
    }
  }

  const PosDateTime firstTimeStamp(std::istream& file) const {
    // look for the first line that contains a bar
    for (PosLine pos = getCrtLine(0, file); pos; pos = getNextLine(pos.pos(), file)) {
      PosDateTime pdt = timeStamp(pos.pos(), file);
      if (pdt) {
        return pdt;
      }
    }
    return PosDateTime();
  }

  const PosDateTime lastTimeStamp(std::istream& file) const {
    return timeStamp(fileSize(file) - 1, file);
  }

  PosDateTime getCandidate(PosDateTime& begin, PosDateTime& end, std::istream& file) const {
    assert(begin);
    assert(end);
    assert(begin <= end);

    __int64 newPos = (_int64)(begin.posAsDouble() + (end.posAsDouble() - begin.posAsDouble()) / 2.0);
    return timeStamp(newPos, file);
  }

  PosDateTime next(PosDateTime candidate, std::istream& file) const {
    assert(candidate);
    PosLine next = getNextLine(candidate.pos(), file);
    return next ? timeStamp(next.pos(), file) : PosDateTime();
  }

  PosDateTime prev(PosDateTime candidate, std::istream& file) const {
    assert(candidate);
    PosLine prev = getPrevLine(candidate.pos(), file);
    return prev ? timeStamp(prev.pos(), file) : PosDateTime();
  }

  // returns -1 if td > end time in the file
  __int64 findStart(const DateTime td, std::istream& file) const {

    PosDateTime begin(firstTimeStamp(file));  // first timestamp
    // couldn't find a start date, which means couldn't find any dates at all
    // so probably the data is in the wrong format
    if (!begin) {
      throw DataFileException();
    }
    PosDateTime end(lastTimeStamp(file));  // last timestamp

    if (begin >= td) {
      return 0;
    }
    else if (end < td) {
      return -1;
    }
    else {
      while (true) {
        PosDateTime candidate = getCandidate(begin, end, file);
        if (candidate == begin) {
          candidate = next(candidate, file);

          if (candidate == end) {
            return end.pos();
          }
        } else if (candidate == end) {
          candidate = prev(candidate, file);
          if (candidate == begin) {
            return end.pos();
          }
        }

        if (candidate < td) {
          begin = candidate;
        }
        else if (candidate > td) {
          end = candidate;
        }
        else {
          return candidate.pos();
        }
      }
    }
  }

 private:
  /**
   * Makes a Bars bars collection from a file
   *
   * pure virtual method of the base class, implemented here for file data
   * sources
   *
   * @param dir    The path
   * @param symbol The symbol
   * @param ext    The extension
   * @param range  A range for which to get the data
   * @return A pointer to a Bars object containing the data
   * @exception DataSourceException
   *                   Thrown if there is any data source exception
   */
  DataXPtr makeBars(const std::string& symbol, const std::string& ext, DateTimeRangePtr range) const;
  //  DataX* makeTicks( const std::wstring& symbol, const std::wstring& ext,
  //  const Range* range ) const throw( DataSourceException );

  const std::string getFileStamp(const std::string& fileName) const;
  virtual void fileNotFoundErrorHandler(const std::string& symbol, const std::string& fileName) const {
    throw DataSourceException(OPENING_BARS_FILE_ERROR, "Could not open data file for symbol \""s + symbol + "\", " + fileName, name());
  }

 public:
  virtual ~FileDataSource() {}

 protected:
  //  void parseTicks( tradery::Addable< Tick >* ticks, t_ifstream& _file, const
  //  Range* range ) const throw ( BarException );

  /**
   * Parse the file and populate the BarsIAddable with bars
   *
   * @param bars
   * @param _file
   * @param range
   * @exception BarException
   */
  inline FilePositionInfo FileDataSource::parseBars(tradery::Addable<Bar>* bars, std::istream& _file, DateTimeRangePtr range, const std::string& symbol) const {
    std::string str;

    __int64 startPos = 0;
    __int64 endPos = fileSize(_file);

    if (range) {
      // todo - this shouldn't be a dynamic cast, should work for all ranges
      startPos = findStart(range->from(), _file);

      if (startPos < 0) {
        return FilePositionInfo();
      }
      else {
        assert(timeStamp(startPos, _file) >= range->from());
        _file.clear();
        _file.seekg(startPos);

        do {
          endPos = _file.tellg();
          std::getline(_file, str);
          BarPtr pBar;

          if ((pBar = BarPtr(parseBarLine(str))).get()) {
            if (*range > *pBar) {
              continue;
            }
            else if (*range < *pBar) {
              break;
            }
            else {
              bars->add(*pBar);
            }
          }
        } while (!_file.eof());

        // we only need to get if it's not eof - if it's eof, we alreay have
        // that value from the top of the function and besides, tellg returns -1
        // for eof
        //	  COUT << _T( "endpos2: " ) << endPos << std::endl;
      }
    }
    else {
      do {
        std::getline(_file, str);
        BarPtr pBar;
        size_t count = 0;

        if ((pBar = BarPtr(parseBarLine(str))).get()) {
          bars->add(*pBar);
        }
      } while (!_file.eof());
    }
    return FilePositionInfo(startPos, endPos - startPos);
  }
  static bool isCommentLine(const std::string& str) {
    return str.at(0) == '$' || str.at(0) == '#' || str.length() > 1 && str.at(0) == '/' && str.at(1) == '/';
  }

 private:
  /**
   * Parse one line
   *
   * Each line contains a bar
   *
   * @param str
   * @param lineCount
   * @return
   * @exception DataSourceException
   */
  virtual const tradery::Bar* parseBarLine(const std::string& str) const = 0;

 private:
  // the root to which we add the relative path
  const std::string _path;
  const std::string _ext;
  Format _format;
  bool _flatData;

 protected:
  // flat data is when all the data is in the path directory,
  // if flat is false, data is in a hierarchy of dirs under path (the rules are
  // in the FileName class
  FileDataSource(const Info& info, const std::string& path, const std::string& ext, Format format, bool flatData = true, ErrorHandlingMode errorHandlingMode = fatal)
      : DataSource(info),
        _path(path),
        _ext(ext),
        _format(format),
        _flatData(flatData),
        _errorHandlingMode(errorHandlingMode) {}

 public:
  static FileDataSource* make(const Info& info, const std::string& path, const std::string& ext, Format format, bool flatData = true, ErrorHandlingMode errorHandlingMode = fatal);
  // _extraInfo is extension
  DataXPtr FileDataSource::getData(const DataInfo* dataInfo, DateTimeRangePtr range) const {
    assert(dataInfo != 0);

    return makeBars(dataInfo->symbol().symbol(), _ext, range);
  }

  //  const tradery::Tick* parseTickLine( const std::wstring& str ) const throw
  //  ( DataSourceException );

  virtual bool isConsistent(const std::string& stamp, const Symbol& symbol, DateTimeRangePtr range) const {
    return getFileStamp(FileName(_flatData).makePath(_path, symbol.symbol(), addExtension(symbol.symbol(), _ext))) == stamp;
  }

  const std::string& dataPath() const { return _path; }
  const std::string& extension() const { return _ext; }
  Format format() const { return _format; }
};

// base for file data sources which have 7 fields: "date, time, open, high, low,
// close, volume" separated by ",", space or tab.
class FileDataSourceFormat7FieldsBase : public FileDataSource {
 public:
  FileDataSourceFormat7FieldsBase(const Info& info, const std::string& path, const std::string& ext, Format format, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSource(info, path, ext, format, flatData, errorHandlingMode) {}

 protected:
  virtual DateTime parseDate(const std::string& date, const std::string& time) const = 0;
  const tradery::Bar* parseBarLine(const std::string& str) const override;
};

// format 1 has 7 fields, date, time: m/d/y, h:m:s
class FileDataSourceFormat1 : public FileDataSourceFormat7FieldsBase {
 public:
  FileDataSourceFormat1(const Info& info, const std::string& path, const std::string& ext, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat7FieldsBase(info, path, ext, format1, flatData, errorHandlingMode) {}

 protected:
  DateTime parseDate(const std::string& date, const std::string& time) const;
};

// format 2 has 7 fields, date, time: yymmdd, hhmm
class FileDataSourceFormat2 : public FileDataSourceFormat7FieldsBase {
 public:
  FileDataSourceFormat2(const Info& info, const std::string& path, const std::string& ext, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat7FieldsBase(info, path, ext, format2, flatData, errorHandlingMode) {}

 protected:
  DateTime parseDate(const std::string& date, const std::string& time) const;
};

// base for file data sources which have 6 fields: "date, open, high, low,
// close, volume" separated by ",", space or tab. This is normally end of day
// data (time is 0)
class FileDataSourceFormat6FieldsBase : public FileDataSource {
 public:
  FileDataSourceFormat6FieldsBase(const Info& info, const std::string& path, const std::string& ext, Format format, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSource(info, path, ext, format, flatData, errorHandlingMode) {}

 protected:
  virtual DateTime parseDate(const std::string& date) const = 0;
  const tradery::Bar* parseBarLine(const std::string& str) const override;
};

// format has 5 fields, and data is: m/d/y, time is implied to be 0:0:0
class FileDataSourceFormat3 : public FileDataSourceFormat6FieldsBase {
 public:
  FileDataSourceFormat3(const Info& info, const std::string& path, const std::string& ext, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat6FieldsBase(info, path, ext, format3, flatData, errorHandlingMode) {}

 protected:
  DateTime parseDate(const std::string& date) const override;
};

class FileDataSourceFormat4 : public FileDataSourceFormat6FieldsBase {
 public:
  FileDataSourceFormat4(const Info& info, const std::string& path, const std::string& ext, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat6FieldsBase(info, path, ext, format4, flatData, errorHandlingMode) {}

 protected:
  DateTime parseDate(const std::string& date) const override;
};

inline FileDataSource* FileDataSource::make(const Info& info, const std::string& path, const std::string& ext, Format format, bool flatData, ErrorHandlingMode errorHandlingMode) {
  switch (format) {
    case format1:
      return new FileDataSourceFormat1(info, path, ext, flatData, errorHandlingMode);
      break;
    case format2:
      return new FileDataSourceFormat2(info, path, ext, flatData, errorHandlingMode);
      break;
    case format3:
      return new FileDataSourceFormat3(info, path, ext, flatData, errorHandlingMode);
      break;
    case format4:
      return new FileDataSourceFormat4(info, path, ext, flatData, errorHandlingMode);
      break;
    default:
      throw DataSourceException(DATA_SOURCE_FORMAT_ERROR, "Unknown data format", info.name());
      break;
  }
}

inline DateTime FileDataSourceFormat1::parseDate(const std::string& date, const std::string& time) const {
  Tokenizer tokens(time, ":");
  Tokenizer::iterator i = tokens.begin();
  i = tokens.begin();

  unsigned int hour = atoi((*i++).c_str());
  unsigned int min = atoi((*i++).c_str());
  unsigned int sec = atoi((*i++).c_str());

  if ((hour > 23) || (min > 59) || (sec > 59)) {
    throw TimeException(time);
  }

  return DateTime(Date(date, us), TimeDuration(hour, min, sec, 0));
}

inline DateTime FileDataSourceFormat2::parseDate(
    const std::string& date, const std::string& time) const {
  // TODO - check data format, throw exception if wrong

  unsigned int hour = atoi(time.substr(0, 2).c_str());
  unsigned int min = atoi(time.substr(2, 2).c_str());
  unsigned int sec = 0;

  if ((hour > 23) || (min > 59) || (sec > 59)) {
    throw TimeException(time);
  }

  return DateTime(Date(date, iso, false), TimeDuration(hour, min, sec, 0));
}

/**
 * Parse one line
 *
 * Each line contains a bar
 *
 * @param str
 * @param lineCount
 * @return
 * @exception DataSourceException
 */
inline const tradery::Bar* FileDataSourceFormat7FieldsBase::parseBarLine(const std::string& str) const {
  if (str.empty()) {
    return 0;
  }
  // comment line starts with // or # or $
  if (isCommentLine(str)) {
    return 0;
  }

  // TODO - check data format

  Tokenizer tokens(str, ", \t");

  // TODO: check that is valid
  // TODO: prepare for futures (more fields - open interest)
  if (tokens.size() != 6) {
    //    throw DataSourceException();
  }
  char* p;

  return new Bar(parseDate(tokens[0], tokens[1]), strtod(tokens[2].c_str(), &p),
                 strtod(tokens[3].c_str(), &p), strtod(tokens[4].c_str(), &p),
                 strtod(tokens[5].c_str(), &p), atol(tokens[6].c_str()));
}

inline const tradery::Bar* FileDataSourceFormat6FieldsBase::parseBarLine(
    const std::string& str) const {
  if (str.empty()) return 0;
  // comment line starts with // or #
  if (isCommentLine(str)) return 0;

  // TODO - check data format
  Tokenizer tokens(str, ",");

  // TODO: check that is valid
  // TODO: prepare for futures (more fields - open interest)
  if (tokens.size() != 6) {
    //    throw DataSourceException();
  }
  char* p;
  return new Bar(parseDate(tokens[0]), strtod(tokens[1].c_str(), &p),
                 strtod(tokens[2].c_str(), &p), strtod(tokens[3].c_str(), &p),
                 strtod(tokens[4].c_str(), &p), atol(tokens[5].c_str()));
}

inline DateTime FileDataSourceFormat3::parseDate(const std::string& date) const {
  return DateTime(Date(date, us), TimeDuration(0, 0, 0, 0));
}

inline DateTime FileDataSourceFormat4::parseDate(const std::string& date) const {
  return DateTime(Date(date, iso, ""), TimeDuration(0, 0, 0, 0));
}

inline DataSource::DataXPtr FileDataSource::makeBars(const std::string& symbol, const std::string& ext, DateTimeRangePtr range) const {
// create an empty collection of bars
// use a smart pointer (BarsIPtr) in case there is an exception, and the
// collection needs to be deleted
// TODO: determine the type and resolution, now hardcoded
//
  BarsPtr bars = tradery::createBars(name(), symbol, tradery::BarsAbstr::Type::stock, 24 * 3600, range, _errorHandlingMode);

  std::string fileName(FileName(_flatData).makePath(_path, symbol, addExtension(symbol, ext)));

  std::ifstream _file;
  _file.open(fileName.c_str(), ios_base::in | ios_base::binary);
  if (!_file) {
    // error opening the file - throw exception
    fileNotFoundErrorHandler(symbol, fileName);
    return 0;
  }
  else {
    try {
      // parse the file and populate the bars collection with bars
      FilePositionInfo p = parseBars(bars.get(), _file, range, symbol);
      bars->setDataLocationInfo(tradery::makeDataFileLocationInfo(fileName, p.start(), p.count()));

      if ( bars->size() == 0) {
        DataSourceException e = DataSourceException(DATA_ERROR, "No data available in the requested range for symbol: \""s + symbol + "\"", name());
        throw e;
      }

      //		parseBars( bars.get(), _file, range, symbol );
      // release and return the pointer
      // have to release so it won't be deleted by the smart pointer
      return DataXPtr(std::make_shared< DataX >(bars, getFileStamp(fileName)));
    }
    catch (const DataFileException) {
      throw DataSourceException(DATA_SOURCE_ERROR, "Could not find any valid date in data file, likely due to data wrong format ", name());
    }
    catch (const BarException&) {
      // retthrow bar exception, it will be caught in scheduler
      throw;
    }
    catch (const DateException& e) {
      throw DataSourceException(DATE_STRING_ERROR, symbol + " - " + e.message(), name());
    }
    catch (const TimeException& e) {
      throw DataSourceException(TIME_STRING_ERROR, symbol + ", " + e.time(), name());
    }
    catch (const DataSourceException & e) {
      throw e;
    }
    catch (...) {
      throw DataSourceException(DATA_SOURCE_FORMAT_ERROR, symbol, name());
    }
  }
}

inline const std::string FileDataSource::getFileStamp(const std::string& fileName) const {
  OutputBuffer ob;

  ob << "File data source: " << name() << ", session: " << sessionName() << ", getting file stamp";
  //  outputSink().printLine( ob );
  HANDLE file = CreateFile(s2ws(fileName).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  FILETIME creation;
  FILETIME lastAccess;
  FILETIME lastWrite;

  GetFileTime(file, &creation, &lastAccess, &lastWrite);

  CloseHandle(file);

  return tradery::format(lastWrite.dwHighDateTime, lastWrite.dwLowDateTime);
}
