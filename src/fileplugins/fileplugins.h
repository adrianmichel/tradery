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

#include <log.h>
#include "Charts.h"
#include "resource.h"  // main symbols



constexpr auto DATASOURCE_FORMAT1_NAME = "Data source plugin format 1";
constexpr auto DATASOURCE_FORMAT3_NAME = "Data source plugin format 3";

// Cfileplugins1App
// See fileplugins1.cpp for the implementation of this class
//

// we want the each data source instance to have a different id, now that all
// runs are inside the same process to avoid id collisions in data manager.
const Info dataSourceInfoFormat1("6188C38F-78FB-470c-A90D-C03FE37D5DEC", DATASOURCE_FORMAT1_NAME, "");
const Info dataSourceInfoFormat1NewId(DATASOURCE_FORMAT1_NAME, "");
const Info dataSourceInfoFormat3("3F8D0DAA-C11E-452c-A097-20127C0673E0", DATASOURCE_FORMAT3_NAME, "");
const Info dataSourceInfoFormat3NewId(DATASOURCE_FORMAT3_NAME, "");
const Info symbolsSourceInfo("E32C975A-ECE1-4e7f-BB49-A604F2EE8083", "Symbols Source plugin - symbols file specified dynamically", "");
const Info statsInfo("4B6632DE-CD7B-43c6-932B-13D098E1E287", "Stats plugin", "Implemented as a signal handler plugin, using only the session notifications to calculate the stats");

class FileSignalHandlerException : public SignalHandlerException {
 public:
  FileSignalHandlerException(const std::string& fileName)
      : SignalHandlerException( __super::name(), std::string("Could not open signals file for writing \"" + fileName + "\"")) {}
};

// the hierarchy is a little messed up:
// The stats handler is a signal handler, but only because there is
// no basic stats handler, so this is just to use the plug-in events
//
// Now we have the actual signal handler also part of this stat handler, so
// the file signal handler will be derived from statshandler to use the basic
// signal methods, and the file stats handler will be derived from the file
// signal handler
//
// they have in common then a few things:
// they are all plug-ins
// they require processing of the end event
// the stats plugin will do the position sizing for positions as well
// as for signal
class FileSignalsHandler : public StatsHandler {
 private:
  std::string _csvFileName;
  std::string m_rawCsvFileName;
  std::string m_sessionId;
  // this is the file containing the description for multi-page presentation
  // of the html file
  std::string _descFileName;
  mutable std::mutex _mx;
  bool _empty;
  size_t _linesPerPage;
  unsigned int _disabledCount;
  // if true, it will generate output, no output otherwise
  bool _outputSignals;

  SignalVector _signals;

  std::string csvHeader() {
    std::ostringstream header;
    header << Signal::csvHeaderLine() << std::endl;
    return header.str();
  }

  std::string csvLine(SignalPtr signal) {
    assert(signal);
    std::ostringstream line;
    line << signal->toCSVString() << std::endl;
    return line.str();
  }

  // this is the number of signals excluding the disabled ones
  size_t count() const { return _signals.size() - _disabledCount; }

  void saveCSVFile(const std::string& file) {
    if (!file.empty()) {
      std::ofstream csvFile(file);
      if (!csvFile) {
        LOG(log_error, "Could not open signals csv file for writing: ", _csvFileName);
      }
      else {
        csvFile << csvHeader();
        unsigned int n = 0;
        for (auto signal : _signals) {
          if (signal->isEnabled()) {
            assert(signal);
            assert(signal->shares() > 0);
            csvFile << csvLine(signal);
          }
        }
      }
    }
  }


  /**
   * Method when a signal of any type is triggered by a trading system
   * associated with this signal handler
   *
   * The default implementation ignores the signal
   *
   * @param _signal Reference to a Signal object, which contains the information
   * about the triggered signal
   */
  virtual void signal(SignalPtr _signal) {
    std::scoped_lock lock(_mx);

    assert(_signal);

    _signals.push_back(_signal);
  }

 protected:
  FileSignalsHandler(const Info& info, const std::vector<std::string>& strings)
      : StatsHandler(info), _disabledCount(0) {
    // csv signals
    _csvFileName = strings[3];
    // signal description (?)
    _descFileName = strings[4];
    // signals per html page
    istringstream lpp(strings[5]);
    lpp >> _linesPerPage;
    // raw signals csv file name
    m_rawCsvFileName = strings[6];
    // session id
    m_sessionId = strings[7];

    // we can test just one, because all the other files will be the same
    _outputSignals = !_csvFileName.empty();
  }

  SignalVector& getSignals() { return _signals; }

  void sessionEnd() {
    saveCSVFile( m_rawCsvFileName);
    signalsSizing();
    saveCSVFile(_csvFileName);
  }

 private:
  void signalSizing(SignalPtr signal) {
    assert(signal);
    unsigned int newShares;
    if (signal->isEntryPosition()) {
      if (signal->applySignalSizing()) {
        const EquityCurve& ec = equityCurve();
        const Equity* eq = ec.getEquity(signal->time().date());

        // not sure yet if that's true, we'll see at runtime...
        assert(eq != 0);

        const PositionSizingParams* ps = sessionInfo().runtimeParams()->positionSizing();
        assert(ps != 0);

        const BarsPtr data = sessionInfo().getData(signal->symbol());
        // we got a signal, so there must be data
        assert(data);
        assert(data->size() > 0);

        Bars bars(dynamic_cast<const BarsAbstr*>(data.get()));

        // the signal should be on the last available bar
        assert(signal->time() == bars.time(data->size() - 1));

        // if the signal has price, use that for sizing, otherwise, use the
        // current close
        double price = signal->hasPrice() ? signal->price() : bars.close(data->size() - 1);
        double volume = bars.volume(data->size() - 1);
        assert(price > 0);

        // for signals, we don't care about the max number of positions - it
        // will be sorted out when the orders are filled

        // now adjust the signal size according to pos sizing params

        switch (ps->posSizeType()) {
          case PosSizeType::system_defined:
            // no adjustment, using the position size set by the system
            newShares = signal->shares();
            break;
          case PosSizeType::shares:
            // setting the new number of shares
            newShares = (unsigned int)ps->posSize();
            break;
          case PosSizeType::size:
            newShares = (unsigned int)round(ps->posSize() / price);
            break;
          case PosSizeType::pctEquity:
            newShares = (unsigned int)round((eq->getAll().getTotal() * ps->posSize() / 100) / price);
            break;
          case PosSizeType::pctCash:
            newShares = (unsigned int)round((eq->getAll().getCash() * ps->posSize() / 100) / price);
            break;
          default:
            assert(false);
        }

        switch (ps->posSizeLimitType()) {
          case PosSizeLimitType::none:
            break;
          case PosSizeLimitType::pctVolume:
            newShares = min(newShares, (unsigned int)round(ps->posSizeLimit() / 100 * volume));
            break;
          case PosSizeLimitType::limit:
            if (price * newShares > ps->posSizeLimit()) {
              newShares = (size_t)(ps->posSizeLimit() / price);
            }
            break;
          default:
            assert(false);
            break;
        }

        signal->setShares(newShares);
      }
    } else {
      // all exit signal are for the same number of shares as the position they
      // are supposed to exit unless the position is disabled
      Position pos = signal->position();
      if (pos.isEnabled()) {
        newShares = signal->position().getShares();
        signal->setShares(newShares);
      }
      else {
        signal->disable();
        ++_disabledCount;
      }
    }
  }

  void signalsSizing() {
    for (auto signal : _signals) {
      signalSizing(signal);
    }
  }
};

class FileStatsHandler : public FileSignalsHandler {
 private:
  std::string _statsCSV;
  std::string _statsHTML;
  std::string _eqCurveBase;

 public:
  FileStatsHandler(const std::vector<std::string>& strings)
      : FileSignalsHandler(statsInfo, strings) {
    // stats csv
    _statsCSV = strings[0];
    // stats htm
    _statsHTML = strings[1];
    // eqcurve
    _eqCurveBase = strings[2];
  }

  ~FileStatsHandler() {}

  void toHTML() {
    LOG(log_debug, "1");
    std::ofstream ofs(_statsHTML.c_str());
    LOG(log_debug, "2");

    if (ofs) toFormat(StatsToHTML(ofs));
  }

  void toCSV() {
    std::ofstream ofs(_statsCSV.c_str());
    if (ofs) toFormat(StatsToCSV(ofs));
  }

  void eqCurveToChart() {
    const EquityCurve& ec(__super::equityCurve());

    if (ec.size() > 0) {
      const EquityCurve& bhec(__super::bhEquityCurve());

      const DrawdownCurve& totalDC(__super::totalDrawdownCurve());
      const DrawdownCurve& longDC(__super::longDrawdownCurve());
      const DrawdownCurve& shortDC(__super::shortDrawdownCurve());
      const DrawdownCurve& bhDC(__super::bhDrawdownCurve());

      AllChart ac(ec, bhec, totalDC, longDC, shortDC, bhDC, _eqCurveBase);
    }
  }

  void eqCurveToCSV() const {
    const EquityCurve& ec(__super::equityCurve());
    const EquityCurve& bhec(__super::bhEquityCurve());

    const DrawdownCurve& totalDC(__super::totalDrawdownCurve());
    const DrawdownCurve& longDC(__super::longDrawdownCurve());
    const DrawdownCurve& shortDC(__super::shortDrawdownCurve());
    const DrawdownCurve& bhDC(__super::bhDrawdownCurve());

    XLabels labels(ec);

    if (ec.getSize() > 0) {
      std::ofstream os((_eqCurveBase + ".csv").c_str());

      os << "Date,Total,Long,Short,Cash,Buy & Hold,Total dd,Long dd,Short "
            "dd,Buy & Hold dd,Total dd pct,Long dd pct,Short dd pct,B&H dd "
            "pct,Total dd days,Long dd days,Short dd days,B&H dd days"
         << std::endl;

      const double* total = ec.getTotal();
      const double* sh = ec.getShort();
      const double* lg = ec.getLong();
      const double* cash = ec.getCash();
      const double* bh = bhec.getTotal();

      for (size_t i = 0; i < ec.getSize(); i++) {
        os << labels[i] << "," << (total != 0 ? total[i] : 0) << ","
           << (lg != 0 ? lg[i] : 0) << "," << (sh != 0 ? sh[i] : 0) << ","
           << (cash != 0 ? cash[i] : 0) << "," << (bh != 0 ? bh[i] : 0) << ","
           <<

            (totalDC.getDDArray() != 0 ? totalDC.getDDArray()[i] : 0) << ","
           << (longDC.getDDArray() != 0 ? longDC.getDDArray()[i] : 0) << ","
           << (shortDC.getDDArray() != 0 ? shortDC.getDDArray()[i] : 0) << ","
           << (bhDC.getDDArray() != 0 ? bhDC.getDDArray()[i] : 0) << "," <<

            (totalDC.getDDPercentArray() != 0 ? totalDC.getDDPercentArray()[i]
                                              : 0)
           << "%,"
           << (longDC.getDDPercentArray() != 0 ? longDC.getDDPercentArray()[i]
                                               : 0)
           << "%,"
           << (shortDC.getDDPercentArray() != 0 ? shortDC.getDDPercentArray()[i]
                                                : 0)
           << "%,"
           << (bhDC.getDDPercentArray() != 0 ? bhDC.getDDPercentArray()[i] : 0)
           << "%," <<

            (totalDC.getBarsArray() != 0 ? totalDC.getBarsArray()[i] : 0) << ","
           << (longDC.getBarsArray() != 0 ? longDC.getBarsArray()[i] : 0) << ","
           << (shortDC.getBarsArray() != 0 ? shortDC.getBarsArray()[i] : 0)
           << "," << (bhDC.getBarsArray() != 0 ? bhDC.getBarsArray()[i] : 0)
           << "," << std::endl;
      }
    }
  }

  void eqCurveToHTML() {}
  void toFormat(const StatsToFormat& format) const;

  void sessionEnded(PositionsContainer& positions) {
    LOG(log_debug, "1");

    RuntimeStats& rts = *sessionInfo().runtimeStats();
    std::cout << "Calculating equity date range" << std::endl;
    rts.setMessage("Calculating equity date range");
    rts.setStatus(RuntimeStatus::RUNNING);
    EquityDateRange edr(__super::sessionInfo());
    rts.step(getDateRangeStep());

    if (sessionInfo().runtimeParams()->statsEnabled() ||
        sessionInfo().runtimeParams()->equityCurveEnabled() ||
        sessionInfo().runtimeParams()->tradesEnabled()){
    // the eq curve calculation is necessary if we have to calc stats or trades
    // (we have to apply pos sizing), so we have to do that if the stats are
    // enabled
      std::cout << "Calculating equity curve" << std::endl;
      __super::calcEqCurve(edr, positions);
    }

    // only do transformations if the eq curve is enabled
    if (!_eqCurveBase.empty()) {
      LOG(log_debug, "before Eq curve to chart");
      eqCurveToChart();
      LOG(log_debug, "before Eq curve to CSV");
      eqCurveToCSV();
      LOG(log_debug, "before Eq curve to html");
      eqCurveToHTML();
      LOG(log_debug, "Eq curve ended");
      //			PriceChart pc;
    }

    if (sessionInfo().runtimeParams()->statsEnabled()) {
      LOG(log_debug, "Calculating stats");
      __super::calcStats(edr, positions);
      __super::calcScore();
    }

    if (!_statsHTML.empty() || !_statsCSV.empty()) {
      // only calculate stats if necessary

      LOG(log_debug, "saving stats as html");
      toHTML();
      LOG(log_debug, "saving stats as csv");
      toCSV();
      LOG(log_debug, "done with stats");
    }

    __super::sessionEnd();
  }
};

class FileStatsPlugin : public SignalHandlerPlugin {
 public:
  FileStatsPlugin()
      : SignalHandlerPlugin(Info("5B776120-3459-4527-8722-8530B4EDBBD8", "", "")) {}

  virtual InfoPtr first() const {
    return std::make_shared< Info >(statsInfo);
  }

  virtual InfoPtr next() const { return InfoPtr(); }

  virtual std::shared_ptr<SignalHandler> get( const UniqueId& id, const std::vector<std::string>* createStrings = 0) {
    LOG(log_info, "getting the signal handler ", id.str());

    if (createStrings == 0) {
      throw PluginException("File stats plugin", "The createStrings parameter must not be null for this plugin");
    }

    if (id == statsInfo.id()) {
      LOG(log_info, "making the signal handler ", id.str(), ", create string: ", (*createStrings)[0]);
      return std::make_shared< FileStatsHandler>(*createStrings);
    }
    else {
      return 0;
    }
  }

  virtual bool canCreate() const { return false; }

  virtual std::shared_ptr<SignalHandler> create(
      const std::vector<std::string>* createStrings =  0) {
    return 0;
  }

  virtual bool canEdit(const UniqueId& id) const { return false; }

  virtual std::shared_ptr<SignalHandler> edit(const UniqueId& id) {
    return 0;
  }

  virtual bool canRemove(const UniqueId& id) const { return false; }

  virtual void remove(const UniqueId& id) {}

  virtual bool hasWindow(const UniqueId& id) const { return false; }
};

class FileDataSourceFormat1ForWeb : public FileDataSourceFormat1 {
 private:
  // this is to show a "sanitized" error message in a web environment
  virtual void fileNotFoundErrorHandler(const std::string& symbol, const std::string& fileName) const {
    throw DataSourceException(OPENING_BARS_FILE_ERROR, "No data for symbol \""s + symbol + "\"", name());
  }

 public:
  FileDataSourceFormat1ForWeb(const std::string& createString, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat1(dataSourceInfoFormat1NewId, createString, "csv", flatData, errorHandlingMode) {}
};

class FileDataSourceFormat3ForWeb : public FileDataSourceFormat3 {
 private:
  // this is to show a "sanitized" error message in a web environment
  virtual void fileNotFoundErrorHandler(const std::string& symbol, const std::string& fileName) const {
    throw DataSourceException(OPENING_BARS_FILE_ERROR, "No data for symbol \""s + symbol + "\"", name());
  }

 public:
  FileDataSourceFormat3ForWeb(const std::string& createString, bool flatData, ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat3(dataSourceInfoFormat3NewId, createString, "csv", flatData, errorHandlingMode) {}
};

class FileDataSourcePlugin : public DataSourcePlugin {
 private:
  std::vector<std::shared_ptr<Info> > _configs;
  mutable std::vector<std::shared_ptr<Info> >::const_iterator _i;

 public:
  FileDataSourcePlugin()
      : DataSourcePlugin(Info("C44EB64E-42A6-48ed-8C6C-3604C5B468DA", "", "")) {
    _configs.push_back(std::make_shared< Info >(dataSourceInfoFormat1));
    _configs.push_back(std::make_shared< Info >(dataSourceInfoFormat3));
  }

  InfoPtr first() const override {
    _i = _configs.begin();
    return std::make_shared< Info>(**_i);
  }

  InfoPtr next() const override {
    ++_i;
    return _i == _configs.end() ? InfoPtr() : std::make_shared< Info >(**_i);
  }

  std::shared_ptr<DataSource> get(const UniqueId& id, const std::vector<std::string>* createStrings = 0) override {
    // TODO: make sure createString != 0, if it is throw exception
    if (createStrings == 0) {
      throw PluginException("File data source plugin 1", "The createStrings parameter must not be null for this plugin");
    }

    ErrorHandlingMode mode(fatal);

    if ((*createStrings).size() > 1) {
      // second element of vector is the error handling mode
      if ((*createStrings)[1] == ERROR_HANDLING_MODE_FATAL) {
        mode = fatal;
      }
      else if ((*createStrings)[1] == ERROR_HANDLING_MODE_WARNING) {
        mode = warning;
      }
      else if ((*createStrings)[1] == ERROR_HANDLING_MODE_IGNORE) {
        mode = ErrorHandlingMode::ignore;
      }
      else {
        throw PluginException("File data source plugin 1", "Unknown error hanlding mode string");
      }
    }

    // passing false, indicating that the data files are in various
    // subdirectories, not all in one dir (flat)
    if (id == dataSourceInfoFormat1.id()) {
      return std::make_shared< FileDataSourceFormat1ForWeb >((*createStrings)[0], false, mode);
    }
    else if (id == dataSourceInfoFormat3.id()) {
      return std::make_shared< FileDataSourceFormat3ForWeb >((*createStrings)[0], false, mode);
    }
    else {
      return 0;
    }
  }

  bool canCreate() const override { return false; }

  std::shared_ptr<DataSource> create(const std::vector<std::string>* createStrings = 0) override {
    return 0;
  }

  bool canEdit(const UniqueId& id) const override { return false; }

  std::shared_ptr<DataSource> edit(const UniqueId& id) override {
    return 0;
  }

  bool canRemove(const UniqueId& id) const override { return false; }

  void remove(const UniqueId& id) override {}

  bool hasWindow(const UniqueId& id) const override { return false; }
};

class FileSymbolsSourcePlugin : public SymbolsSourcePlugin {
 public:
  FileSymbolsSourcePlugin()
      : SymbolsSourcePlugin(Info("6267C82E-EF65-4b89-AC47-2B25CAEE70B0", "", "")) {}

  InfoPtr first() const override {
    return std::make_shared< Info >(symbolsSourceInfo);
  }

  InfoPtr next() const override { return InfoPtr(); }

  std::shared_ptr<SymbolsSource> get(const UniqueId& id, const std::vector<std::string>* createStrings = 0) override {
    if (createStrings == 0) {
      throw PluginException("File symbols source plugin 1", "The createStrings parameter must not be null for this plugin");
    }

    // TODO: make sure createString != 0, if it is throw exception
    if (id == symbolsSourceInfo.id()) {
      return std::make_shared< FileSymbolsSource >(symbolsSourceInfo, (*createStrings)[0]);
    }
    else {
      return 0;
    }
  }

  bool canCreate() const override { return false; }

  std::shared_ptr<SymbolsSource> create(const std::vector<std::string>* createStrings = 0) override {
    return 0;
  }

  bool canEdit(const UniqueId& id) const override { return false; }

  std::shared_ptr<SymbolsSource> edit(const UniqueId& id) override {
    return 0;
  }

  bool canRemove(const UniqueId& id) const override { return false; }

  void remove(const UniqueId& id) override {}

  bool hasWindow(const UniqueId& id) const override { return false; }
};
