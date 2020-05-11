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

#include <charthandler.h>
#include <set>
#include <iomanip>

#include <log.h>
#include <path.h>

using namespace tradery::chart;
using std::ostream;

std::string getUniqueFileName();

// inline std::string elementEnd( const std::string

class Attribute {
 private:
  std::string _name;
  std::string _value;

 public:
  Attribute(const std::string& name, const std::string& value)
      : _name(name), _value(value) {}

  Attribute(const std::string& name, size_t value)
    : _name(name), _value( std::to_string(value) ){}

  std::string str() const {
    return tradery::format(" ", _name, "=\"", _value, "\"");
  }

  const Attribute(const Attribute& attribute)
      : _name(attribute._name), _value(attribute._value) {}
};

class Attributes : public std::vector<Attribute> {
 public:
  void add(const std::string& name, const std::string& value) {
    __super::push_back(Attribute(name, value));
  }

  void add(const std::string& name, size_t value) {
    __super::push_back(Attribute(name, value));
  }

  void add(const Attribute& attr) { __super::push_back(attr); }

  Attributes& operator<<(const Attribute& attr) {
    add(attr);
    return *this;
  }

  operator std::string() const {
    std::ostringstream os;

    for (auto attribute : *this) {
      os << attribute.str();
    }

    return os.str();
  }
};

class XMLSerializer {
 private:
  ostream& _os;
  const std::string _elementName;

 public:
  virtual ~XMLSerializer() {
    // closing tag
    closingTag();
  }

  XMLSerializer(ostream& os, const std::string& elementName, const Attributes& attributes, bool hdr = false)
      : _os(os), _elementName(elementName) {
    if (hdr) header();

    _os << "<" << _elementName;
    _os << (std::string)attributes;
    _os << ">";
  }

  XMLSerializer(ostream& os, const std::string& elementName, const std::string& attributeName, const std::string& attributeValue, bool hdr = false)
      : _os(os), _elementName(elementName) {
    if (hdr) header();

    _os << "<" << _elementName;
    attribute(attributeName, attributeValue);
    _os << ">";
  }

  XMLSerializer(ostream& os, const std::string& elementName, bool hdr = false)
      : _os(os), _elementName(elementName) {
    if (hdr) {
      header();
    }

    _os << "<" << _elementName << ">";
  }

 private:
  void header() {
    _os << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << std::endl;
  }
  void closingTag() { _os << "</" << _elementName << ">" << std::endl; }
  void attribute(const std::string& attributeName, const std::string& attributeValue) {
    if (!attributeName.empty()) {
      _os << " " << attributeName << "=\"" << attributeValue << "\"";
    }
  }

 public:
  operator std::ostream&() { return _os; }
};

inline std::ostream& operator<<(std::ostream& os, XMLSerializer& serializer) {
  os << serializer;
  return os;
}

class ChartSerializer : public XMLSerializer {
 public:
  ChartSerializer(ostream& os) : XMLSerializer(os, "chart") {}
};

class XMLCDATASerializer : public XMLSerializer {
 private:
  void toCDATA(const std::string& data) {
    (ostream&)(*this) << "<![CDATA[" << data << "]]>";
  }

 public:
  XMLCDATASerializer(ostream& os, const std::string& elementName, const std::string& data, const std::string& attribute, const std::string& attributeValue)
      : XMLSerializer(os, elementName, attribute, attributeValue, false) {
    toCDATA(data);
  }

  XMLCDATASerializer(ostream& os, const std::string& elementName, const std::string& data, const Attributes& attributes)
      : XMLSerializer(os, elementName, attributes, false) {
    toCDATA(data);
  }

  XMLCDATASerializer(ostream& os, const std::string& elementName, const std::string& data)
      : XMLSerializer(os, elementName, false) {
    toCDATA(data);
  }
};

class PositionsSerializer : public PositionHandler {
 private:
  ostream& _os;

 public:
   PositionsSerializer(ostream& os);
   virtual void onPosition(tradery::Position pos);
};

class WebComponent {
 private:
  const std::string& _path;

 protected:
  WebComponent(const std::string& path) : _path(path) {}

  //    const std::string& path() { return _path; }

  void serializeSeries(ostream& os, const std::string& name, const Series& series, const std::string& fnPostfix);

  void serializePositions(ostream& os, const std::string& name, const PositionsPtrVector& pos, const std::string& fnPostfix) {
    std::string fileName = addFSlash(_path) + name + fnPostfix + ".txt";

    std::ofstream of(fileName.c_str());

    if (!of) {
      // todo: handle error
    }
    else {
      PositionsSerializer ps(of);
      if (!pos.hasEnablePositions()) {
        LOG(log_info, "No positions");
      }
      else {
        for (auto p : pos) {
          p->forEachConst(ps);
        }
        XMLCDATASerializer x(os, name, fileName);
      }
    }
  }
};

class WebPositionsComponent : public WebComponent, public chart::Component {
 private:
  WithPositions& _wp;
  const bool _reduced;

 public:
  WebPositionsComponent(const std::string& name, const std::string& path, WithPositions& wp, bool reduced)
      : _wp(wp), WebComponent(path), Chart::Component(name), _reduced(reduced) {}

 protected:
  void serialize(ostream& os) {
    if (_wp.positions().hasEnablePositions()) {
      Attributes attrs;
      attrs << Attribute("type", "positions") << Attribute("name", __super::getName());
      XMLSerializer y(os, "component", attrs);

      // ohlcv + timestamp
      std::string postFix = getUniqueFileName();
      __super::serializePositions(os, "positions", _wp.positions(), postFix);
    }
    else {
      return;
    }
  }
};

class WebBarsComponent : public WebComponent, public chart::Component {
 private:
  WithBars& _wb;
  const bool _reduced;

 public:
  WebBarsComponent(const std::string& name, const std::string& path, WithBars& wb, bool reduced)
      : _wb(wb), WebComponent(path), chart::Component(name), _reduced(reduced) {
  }

 private:
 protected:
  void serialize(ostream& os) {
    try {
      if (_wb.bars()) {
        Attributes attrs;
        attrs << Attribute("type", "bars") << Attribute("name", __super::getName());
        XMLSerializer y(os, "component", attrs);

        /*
// ohlcv + timestamp
std::string postFix = getUniqueFileName();
__super::serializeSeries( os, "open", _wb.bars().openSeries(), postFix );
__super::serializeSeries( os, "high", _wb.bars().highSeries(), postFix );
__super::serializeSeries( os, "low", _wb.bars().lowSeries(), postFix );
__super::serializeSeries( os, "close", _wb.bars().closeSeries(), postFix );
__super::serializeSeries( os, "volume", _wb.bars().volumeSeries(), postFix );
        */
      }
      else {
        LOG(log_info, "bars is 0");
        return;
      }
    }
    catch (const std::bad_cast&) {
      LOG(log_info, "bad cast in bars()");
      assert(false);
    }
  }

 public:
};

class WebLine : public chart::Line, public WebComponent {
 private:
  const bool _reduced;

 public:
  WebLine(const std::string& name, const Series& series, const std::string& path, const Color& color, bool reduced)
      : chart::Line(name, series, color), WebComponent(path), _reduced(reduced) {}

  virtual void serialize(ostream& os) {
    if (_reduced) {
      return;
    }

    try {
      Attributes attrs;
      attrs.add("type", "series");
      attrs.add("name", __super::getName());
      attrs.add("color", __super::getColor().rgbAsString());
      attrs.add("transparency", __super::getColor().transparencyAsString());

      XMLSerializer y(os, "component", attrs);

      // ohlcv + timestamp
      std::string postFix = getUniqueFileName();
      __super::serializeSeries(os, "series", __super::series(), postFix);
    }
    catch (const std::bad_cast& e) {
      LOG(log_error, "bad cast in bars: ", e.what());
      assert(false);
    }
  }
};

class WebPane : public tradery::chart::PaneAbstr {
 private:
  const std::string _path;
  const bool _reduced;

 public:
  WebPane(const std::string& name, const Color& background, const std::string& path, bool def, bool reduced)
      : _path(path), PaneAbstr(name, def, background), _reduced(reduced) {
  }

  virtual void serialize(ostream& os) {
    // only serialize if not reduced
    bool def(__super::isDefault());
    if (!_reduced || def) {
      Attributes attrs;

      attrs.add("name", __super::getName());
      attrs.add("default", def ? "true" : "false");
      if (!_reduced) {
        // only draw the background if not reduced - in reduced state, we only
        // show the basic info, no system specific stuff like background etc
        attrs.add("background", __super::background().rgbAsString());
        attrs.add("transparency", __super::background().transparencyAsString());
      }

      //!!! XMLserializer serializes on destructor, so careful with the scoping
      //!!!!
      XMLSerializer x(os, "pane", attrs);
      __super::serializeComponents(os);
    }
    else {
      __super::serializeComponents(os);
    }
  }

  virtual void drawSeries(const std::string& name, const Series& series, const Color& color = Color()) {
    // only draw if not reduced
    if (!_reduced) {
      if (__super::componentCount() > 5) {
        throw ChartException("Too many components in pane: "s + __super::getName());
      }
      else {
        __super::add(std::make_shared< WebLine >(name, series, _path, color, _reduced));
      }
    }
  }
};

class WebBarsPane : public WebPane {
 public:
  WebBarsPane(const std::string& name, const Color& background, const std::string& path, bool def, WithBars& wb, WithPositions& wp, bool reduced = false)
      : WebPane(name, background, path, def, reduced) {
    add(std::make_shared< WebBarsComponent >("", path, wb, reduced));
    add(std::make_shared< WebPositionsComponent >("", path, wp, reduced));
  }
};

// a chart with a bars pane
class WebBarsChart : public tradery::chart::BarsChart {
 private:
  const std::string _path;
  const bool _reduced;

 public:
  WebBarsChart(const std::string& name, const std::string& symbol, const std::string& path, bool reduced = false)
      : BarsChart(name, symbol), _path(path), _reduced(reduced) {
  }

  Pane createPane(const std::string& name, const Color& background = Color()) {
    if (__super::getPaneCount() > 10) {
      throw ChartException("Too many chart panes");
    }

    PaneAbstrPtr pane(std::make_shared< WebPane >(name, background, path(), false, _reduced));
    __super::add(pane);
    return Pane(pane);
  }

  virtual Pane createDefaultPane(const std::string& name, const Color& background = Color()) {
    PaneAbstrPtr pane(std::make_shared< WebBarsPane >(name, background, path(), true, *this, *this, _reduced));
    __super::addDefaultPane(pane);
    return Pane(pane);
  }

  void serialize(ostream& os) {
    const Bars bars = __super::bars();

    if (bars != 0) {
      Attributes attrs;

      attrs.add("symbol", __super::getSymbol());
      attrs.add("name", __super::getName());

      // need to get the unsync parameters that refers to the actual or
      // otherwise the chart will be wrong if the bars has been synced
      attrs.add("datacount", bars.unsyncSize());

      // need to get the real
      std::pair<DateTime, DateTime> startEnd = bars.unsyncStartEnd();

      LOG(log_debug, "range: ", startEnd.first.to_simple_string(), " - ", startEnd.second.to_simple_string());

      Date start = startEnd.first.date();
      LOG(log_debug, "start date: ", start.to_simple_string());
      if (!start.is_not_a_date()) {
        attrs.add("startDate", tradery::format(start.month(), "/", start.day(), "/", start.year()));
      }
      else {
        attrs.add("startDate", "");
      }

      Date end = startEnd.second.date();
      if (!start.is_not_a_date()) {
        attrs.add("endDate", tradery::format( start.month(), "/", start.day(), "/", start.year()));
      }
      else {
        attrs.add("endDate", "");
      }
      XMLSerializer x(os, "chart", attrs, true);

      if (tradery::chart::BarsChart::getError()) {
        LOG(log_error, "serialize, error message:", tradery::chart::BarsChart::getErrorMessage());
        XMLSerializer z(os, "error");
        os << tradery::chart::BarsChart::getErrorMessage();
      }

      os << bars.locationInfoToXML();

      tradery::chart::BarsChart::serializeComponents(os);
    }
    else {
      LOG(log_error, "writing chart error - no data for symbol: ", tradery::chart::BarsChart::getSymbol());
      XMLSerializer z(os, "error");
      os << "No data for symbol: " << tradery::chart::BarsChart::getSymbol();
    }
  }

 protected:
  const std::string& path() const { return _path; }
};

class WebChartManagerSerializer : public XMLSerializer {};

class WebChartManager : public chart::ChartManager {
 private:
  const std::string _chartRootPath;
  const std::string _chartsDescriptionFile;
  const bool _reduced;

 private:
  WebBarsChart* createWebChart(const std::string& name, const std::string& symbol);

 public:
  // reduced is used when we only want to display price bars and position, no
  // lines, panes etc. this is used for multi-system run
  WebChartManager(const std::string& name, const std::string& symbolsToChartFile, const std::string& chartRootPath, 
    const std::string& chartsDescriptionFile, bool reduced = false);

  const std::string& chartRootPath() const { return _chartRootPath; }

  virtual void serialize() {
    LOG(log_info, "Web chart manager, serializing charts, opening chart description file: ", _chartsDescriptionFile);

    if (_chartsDescriptionFile.empty()) {
      return;
    }

    std::ofstream os(_chartsDescriptionFile.c_str());
    if (os) {
      // ensure that the "charts" directory exists
      Path{ _chartRootPath }.createDirectories();
      // create the header and root element "charts"
      XMLSerializer y(os, "charts", true);
      Charts& charts(__super::getCharts());
      for (auto i : charts) {
        ChartPtr chart = i.second;
        if (!chart->enabled()) {
          continue;
        }
        std::string symbol = i.first;
        // todo: this test (hasPositions) should be implemented in a base class
        // or in chart, and not here
        bool hasPositions = chart->getPositions().hasEnablePositions();
        // todo: handle invalid path characters in symbol name
        // the description for a symbol is <path>\symbol.xml
        std::string fileName = addFSlash(_chartRootPath) + chart->getSymbol() + ".xml";
        ofstream symbolDescFile(fileName.c_str());
        if (!symbolDescFile) {
          LOG(log_info, "WebChartManager:;serialize Can't create description file for writing: ", fileName);
        }
        else {
          // write the file name in the manager file

          // create an element "chart" with the attribute the symbol, and the
          // value the file name that contains the actual description of the
          // chart
          Attributes attrs;

          attrs.add("symbol", symbol);
          attrs.add("positions", hasPositions);
          XMLCDATASerializer x(os, "chart", fileName, attrs);
          // now finalize the actual chart

          __super::runtimeStats()->step(__super::getTotalPct() /
                                        ((double)charts.size()));
          __super::runtimeStats()->setMessage(
              std::string("Generating chart(s) for \"") + symbol + "\"");
          __super::runtimeStats()->setStatus(RuntimeStatus::RUNNING);

          std::ofstream ofs(fileName.c_str());

          if (ofs) {
            chart->serialize(ofs);
          }
          else {
            // todo - handle file open error
          }
        }
      }
    }
    else {
      // could not open chart description file for writing
      LOG(log_info, "could not open chart description file for writing");
      throw ChartManagerException( "could not open chart description file for writing");
    }
    LOG(log_info, "Web chart manager, exiting serialize");
  }
};

// turns a collection of bar data into a csv string
inline std::ostream& operator<<(std::ostream& os, Bars bars) {
  for (unsigned int n = 0; n < bars.size(); n++) {
    Bar bar(bars.getBar(n));
  }

  return os;
}
