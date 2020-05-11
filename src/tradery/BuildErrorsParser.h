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

// to handle:
// C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\include\excpt.h(23) :
// fatal error C1189: #error :  ERROR: Only Win32 target supported!
#include <log.h>

class BuildErrorsParser {
 private:
  class BuildEvent {
   public:
    enum Type { warning, error };

   private:
    const std::string _fileName;
    const Type _type;
    const unsigned long _line;
    const std::string _errorCode;
    const std::string _message;

   private:
    unsigned long toLine(const std::string& line) {
      std::istringstream i(line);

      unsigned long l;

      i >> l;
      return l;
    }

    Type toType(const std::string& type) {
      return type == "warning" ? Type::warning : Type::error;
    }

   public:
    BuildEvent(const std::string& fileName, const std::string& line, const std::string& type, const std::string& errorCode,
               const std::string& message)
        : _fileName(fileName), _type(toType(type)), _line(toLine(line)), _errorCode(errorCode), _message(message) {
      LOG(log_info, "error code: ", errorCode);
    }

    std::string toString() const {
      std::ostringstream o;

      // have to decrement the line number to account for the first line in the
      // h file which is the system class macro
      o << "<td>" << (_line) << "</td><td>" << (_type == warning ? "Warning" : "Error") << "</td><td>" << _message << "</td>" << std::endl;
      return o.str();
    }

    unsigned long line() const { return _line; }
  };

  using BuildEventPtr = std::shared_ptr<BuildEvent>;

  class BuildEvents : public std::vector<BuildEventPtr> {
   public:
    unsigned long write(std::ofstream& ofs, bool onePerLine, unsigned long max) const {
      std::ostringstream os;
      os << "<table class=\"list_table\">" << std::endl << "<tr><th>Line</th><th>Type</th><th>Message</th></tr>" << std::endl;

      unsigned long lastLine = 0;
      unsigned long count = 0;
      bool empty = true;
      for (__super::const_iterator i = __super::begin(); i != __super::end() && count < max; i++, count++) {
        BuildEventPtr event = *i;
        LOG(log_info, "Event: ", event->toString());
        if (event->line() != lastLine) {
          if (empty) {
            ofs << os.str();
          }
          empty = false;
          ofs << "<tr class=\"" << (count % 2 ? "d0\"" : "d1\"") << ">" << std::endl;
          ofs << event->toString();
          ofs << "</tr>" << std::endl;
          if (onePerLine) {
            lastLine = event->line();
          }
        }
      }
      if (!empty) {
        ofs << "</table>" << std::endl;
      }

      return count;
    }
  };

  class SystemBuildEvents {
   private:
    BuildEvents _events;
    const std::string _systemName;

   public:
    SystemBuildEvents(const std::string& name) : _systemName(name) {}

    void add(const BuildEventPtr event) { _events.push_back(event); }

    void write(std::ofstream& ofs, bool onePerLine, unsigned long max) const {
      if (_events.size() > 0) {
        ofs << "<br>Messages for system <b>" << (_systemName.empty() ? "[unnamed]" : _systemName) << "</b><br>" << std::endl;

        _events.write(ofs, onePerLine, max);
      }
    }

    size_t eventCount() const { return _events.size(); }
  };

  using SystemBuildEventsPtr = std::shared_ptr<SystemBuildEvents>;

  class SystemsBuildEvents : public std::vector<SystemBuildEventsPtr> {
   public:
    SystemBuildEventsPtr add(const std::string& systemName) {
      SystemBuildEventsPtr sbe = std::make_shared< SystemBuildEvents >(systemName);
      __super::push_back(sbe);
      return sbe;
    }

    size_t eventCount() const {
      size_t count = 0;

      for (auto i : *this) {
        count += i->eventCount();
      }

      return count;
    }

    bool hasEvents() const { return eventCount() > 0; }

    void write(std::ofstream& ofs, bool onePerLine, unsigned long max) const {
      if (hasEvents()) {
        ofs << "<H2>Build Messages</H2>" << std::endl;

        for (auto i : *this) {
          i->write(ofs, onePerLine, max);
        }
      }
    }
  };

 private:
  std::istream& _is;
  const std::string& classNameKeyword;
  const std::string& systemNameKeyword;
  SystemsBuildEvents _events;

 public:
  BuildErrorsParser(std::istream& is,const Configuration& config)
      : _is(is), systemNameKeyword(config.getSystemNameKeyword()),
        classNameKeyword(config.getClassNameKeyword()) {
    LOG(log_info, "******** START BUILD ERRORS ********");
    parse();
    LOG(log_info, "******* END BUILD ERRORS ******");
  }

  std::string getClassName(const std::string& line) {
    const boost::regex rx(classNameKeyword + "=(.*)");
    std::string::const_iterator start, end;
    start = line.begin();
    end = line.end();

    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;

    if (boost::regex_search(start, end, what, rx, flags)) {
      return what[1];
    }
    else {
      return std::string();
    }
  }

  bool getSystemName(const std::string& line, std::string& systemName) {
    const boost::regex rx(systemNameKeyword + "=(.*)");
    std::string::const_iterator start, end;
    start = line.begin();
    end = line.end();

    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;

    if (boost::regex_search(start, end, what, rx, flags)) {
      systemName = what[1];
      return true;
    }
    else {
      return false;
    }
  }

  void parse() {
    std::string line;
#ifdef REGEX_TRACE
    LOG(log_info, "<start parsing errors");
#endif
    SystemBuildEventsPtr sbe;
    std::string className;

    do {
      std::getline(_is, line);
      LOG(log_info, line);

     // line = _T(
      // "c:\\dev\\home\\webapp\\output\\968e732d-bcf3-11da-a768-00904baca583\\96d3975f-bcf3-11da-a768-00904baca583.h(8)
      // : error C2146: syntax error : missing ';' before identifier 'series'"
      // ); line = _T(
      // "c:\\dev\\home\\webapp\\output\\84c062bd-bcf4-11da-a768-00904baca583\\8507e949-bcf4-11da-a768-00904baca583.h(8)
      // : error C2146: syntax error : missing ';' before identifier 'series'"
      // );
      if (!line.empty()) {
        std::string systemName;
        if (getSystemName(line, systemName)) {
          sbe = _events.add(systemName);
          continue;
        }

        std::string x = getClassName(line);
        if (!x.empty()) {
          LOG(log_info, "Class name: ", x);
          className = x;
          continue;
        }
        if (!className.empty()) {
          parseLine(line, className, sbe);
        }
      }
    } while (!_is.eof());
#ifdef REGEX_TRACE
    LOG(log_info, ">end parsing errors");
#endif
  }

  void cleanKeyword(std::string& line, const std::string& keyword) {
    std::string::size_type ix = line.find(keyword);
    if (ix != std::string::npos) {
      line.erase(ix, keyword.length());
    }
  }

  void replaceKeyword(std::string& line, const std::string& keyword, const std::string& replacement) {
    std::string::size_type ix = line.find(keyword);
    if (ix != std::string::npos) {
      line.replace(ix, keyword.length(), replacement);
    }
  }

  // error message after paranthesis must be cleaned
  // c:\dev\home\webapp\output\c0b4cebd-2c90-be9c-042e-4e06d07e7903\41a305b4-3422-c50a-c640-0928a4400a22.h(24)
  // : error C2958: the left parenthesis '(' found at
  // 'c:\dev\home\webapp\output\c0b4cebd-2c90-be9c-042e-4e06d07e7903\41a305b4-3422-c50a-c640-0928a4400a22.h(20)'
  // was not matched correctly
  // c:\dev\sim\net2.0\systemsplugin1\CalendarSystem.h(58) : error C2958: the
  // left bracket '[' found at
  // 'c:\dev\sim\net2.0\systemsplugin1\calendarsystem.h(58)' was not matched
  // correctly

  void cleanError2958(std::string& line) {
    static const boost::regex error2958regex("(.*at )\'.*\\((\\d+)\\)\'(.*)");
    std::string::const_iterator start, end;
    start = line.begin();
    end = line.end();

    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;

    if (boost::regex_search(start, end, what, error2958regex, flags)) {
      line = what[1] + "line " + what[2] + what[3];
    }
  }

  // c:\dev\sim\net2.0\runtimeproj\runtimeproj.cpp(64) : fatal error C1075: end
  // of file found before the left brace '{' at
  // 'c:\dev\home\webapp\output\5663deaf-c263-5a17-df4a-c2a46a345911\defines.h(9)'
  // was matched
  void cleanError1075(std::string& line) {
    // same regex as 2958
    cleanError2958(line);
  }

  void cleanError1189(std::string& line) {
    line = "A closing bracket or paranthesis is likely missing";
  }

  void cleanLine(std::string& line, const std::string& className, unsigned int code) {
    LOG(log_info, "className: ", className);
    switch (code) {
      case 1075:
        cleanError1075(line);
        break;
      case 2958:
        cleanError2958(line);
        break;
      case 1189:
        cleanError1189(line);
        break;
      default:
        break;
    }

    cleanKeyword(line, className + "::");
    cleanKeyword(line, "tradery::BarSystem<T>::");
    cleanKeyword(line, "tradery::");
    replaceKeyword(line, "macro", "identifier");
    cleanKeyword(line, ", even with argument-dependent lookup");
  }
  // c:\dev\home\webapp\output\5663deaf-c263-5a17-df4a-c2a46a345911\81f8f22a-7eca-eefc-4831-544bf5690a06.h(40)
  // : error C2065: 'asdfasdf' : undeclared identifier

  void systemName(const std::wstring& line) {}

  void parseLine(const std::string& line, const std::string& className,
                 SystemBuildEventsPtr sbe) {
    try {

      // expression: (.+)\((\d+\) : (\w+) (C\d+): (.+)
      static const boost::regex CompilerErrorRegex(R"!!((.+)\((\d+)\) : ((?:\w| )+) (C\d+): (.+))!!");
      std::string::const_iterator start, end;
      start = line.begin();
      end = line.end();

      boost::match_results<std::string::const_iterator> what;
      boost::match_flag_type flags = boost::match_default;

      if (boost::regex_search(start, end, what, CompilerErrorRegex, flags)) {
        std::string l(what[5]);
        std::string code(what[4]);
        code = code.substr(1, what[4].length() - 1);

        cleanLine(l, className, atoi(code.c_str()));

        LOG(log_info, "Error match: ", what[1], "\n", 
                                      what[2], "\n",
                                      what[3], "\n",
                                      what[4], "\n",
                                      what[5]);

        sbe->add(std::make_shared< BuildEvent >(what[1], what[2], what[3], what[4], l));
      }
    }
    catch (const boost::regex_error& e) {
      LOG(log_error, "Regex error: ", e.what());
      assert(false);
    }
  }

  void write(ofstream& ofs, bool onePerLine, unsigned long max = 100) const {
    _events.write(ofs, onePerLine, max);
  }
};