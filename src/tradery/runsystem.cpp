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

#include "stdafx.h"

#include "runsystem.h"
#include "document.h"
#include <nlohmann\json.hpp>

#include "runtime_stats_impl.h"



class XPositionHandlerException {
 private:
  const std::string _fileName;

 public:
  XPositionHandlerException(const std::string& fileName)
      : _fileName(fileName) {}

  const std::string& fileName() const { return _fileName; }
  const std::string message() const {
    return "Could not open positions file for writing \""s + _fileName + "\"";
  }
};

// will update session real-time stats
class XSignalHandler : public SignalHandler {
 private:
  mutable std::mutex _mx;
  RuntimeStats& _signalsCounter;

  SignalVector _signals;

 private:
 public:
  XSignalHandler(RuntimeStats& signalCounter)
      : SignalHandler(Info()), _signalsCounter(signalCounter) {}

  virtual void signal(SignalPtr _signal) {
    std::scoped_lock lock(_mx);

    assert(_signal);

    _signalsCounter.incSignals();
    _signals.push_back(_signal);
  }

  unsigned int processedSignalsCount() {
    unsigned int count = 0;
    for (SignalVector::size_type n = 0; n < _signals.size(); ++n) {
      if (_signals[n]->isEnabled()) ++count;
    }
    return count;
  }
};

class XRunnableRunInfoHandler : public RunnableRunInfoHandler {
 private:
  mutable std::mutex _m;
  mutable Timer _runTimer;
  RuntimeStats& _runsCounter;

  unsigned int _count;

 public:
  XRunnableRunInfoHandler(RuntimeStats& runsCounter)
      : _count(0), _runsCounter(runsCounter) {}

  // a runnable status event has been received
  virtual void status(const RunnableRunInfo& status) {
    std::scoped_lock lock(_m);
    _runsCounter.incTotalRuns();
    if (status.errors()) {
      _runsCounter.incErrorRuns();
    }
    _count++;
    LOG(log_debug, (status.errors() ? "!" : "+"), "[", status.threadName(), ":", status.cpuNumber(), "] ", status.status(), " on \"", status.symbol(), "\"");
    _runTimer.restart();
    _runsCounter.incTotalBarCount(status.dataUnitCount());
    _runsCounter.setMessage("Running \""s + status.status() + "\" on \"" + status.symbol() + "\"");
    _runsCounter.setStatus(RuntimeStatus::RUNNING);
  }

  bool timeout(unsigned __int64 runTimeout) const {
    std::scoped_lock lock(_m);
    return _runTimer.elapsed() > runTimeout;
  }

  bool exceededBarCount(unsigned __int64 maxTotalBarCount) const {
    std::scoped_lock lock(_m);
    return maxTotalBarCount > 0 && _runsCounter.getTotalBarCount() > maxTotalBarCount;
  }

  unsigned __int64 totalBarCount() { return _runsCounter.getTotalBarCount(); }
};

class XOutputSink : public OutputSink {
 private:
  ofstream _ofs;
  const std::string _fileName;

 public:
  XOutputSink(const std::string& fileName) : _fileName(fileName) {
    if (!_fileName.empty()) {
      _ofs.open(_fileName.c_str());
    }

    if (!_ofs) {
      // todo: error, throw exception
    }
  }
  virtual void print(const std::string& str) {
    if (_ofs) _ofs << str;
  }
  virtual void printLine(const std::string& str) {
    if (_ofs) _ofs << str << "\n";
  }

  virtual void clear() {}

  virtual void print(Control ctrl) {
    if (_ofs) {
      switch (ctrl) {
        case def:
        case black:
        case blue:
        case green:
        case red:
        case yellow:
        case purple:
        case gray:

        case bold:
        case not_bold:
        case italic:
        case not_italic:
        case underline:
        case not_underline:
        case reset:
          break;

        case tradery::endl:
          _ofs << "\n";
          break;
        default:
          break;
      }
    }
  }
};

class ErrorFile {
 public:
  ErrorFile(const std::string& fileName, const std::string& message, const std::string& caption) {
    std::ofstream errorsFile(fileName.c_str(), ios_base::out | ios_base::app);
    if (errorsFile) {
      LOG(log_debug, "dumping errors to error file");

      if (!caption.empty()) {
        errorsFile << "<h2>" << caption << "</h2>" << std::endl;
      }

      errorsFile << message;
    }
    else {
      LOG(log_debug, "error - can't open the errors file for writing: ", fileName);
    }
  }
};

RunSystem::RunSystem(const Configuration& config) : m_config(config) {}

void RunSystem::saveTradesDescriptionFile( const PositionsContainer& pos ) const {
  if (!m_config.tradesFile().empty()) {
    std::ofstream tradesFile( m_config.tradesFile().c_str());

    if (!tradesFile) {
      LOG(log_error, "error - can't open the trades file for writing: "s + m_config.tradesFile());
    }
    else {
      LOG(log_debug, m_config.getSessionId(), "Trades desc file: ", m_config.getTradesDescriptionFile());
      std::ofstream tradesDescFile(m_config.getTradesDescriptionFile().c_str());
      if (tradesDescFile) {
        PositionsContainerToHTML toHtml(pos, tradesFile, tradesDescFile, m_config.getLinesPerPage());
      }
      else {
        LOG(log_debug, m_config.getSessionId(), "error - can't open the trades desc file for writing: ", m_config.getTradesDescriptionFile());
      }
    }
  }
}

void RunSystem::saveTradesCSVFile(const PositionsContainer& pos) const {
  if (!m_config.tradesCSVFile().empty()) {
    LOG(log_debug, m_config.getSessionId(), "Creating trades csv file: ", m_config.tradesCSVFile());
    std::ofstream tradesCSVFile(m_config.tradesCSVFile().c_str());

    if (!tradesCSVFile) {
      LOG(log_error, m_config.getSessionId(), "error - can't open the trades CSV file for writing");
    }
    else {
      PositionsContainerToCSV toCSV(pos, tradesCSVFile);
    }
  }
}

void RunSystem::saveErrorsFile(const ErrorEventSinkImpl& errsink) const {
  if (!errsink.empty()) {
    LOG(log_debug, m_config.getSessionId(), "Creating errors file: ", m_config.errorsFile());
    std::fstream ef(m_config.errorsFile().c_str(), ios_base::out | ios_base::app);
    if (!ef) {
      DWORD dw = GetLastError();
      LOG(log_error, m_config.getSessionId(), "error ", dw, ", can't open the errors file for writing: ", m_config.errorsFile());
    }
    else {
      LOG(log_debug, m_config.getSessionId(), "dumping errors to error file");
      errsink.toHTML(ef);
    }
  }
}

void RunSystem::saveSymbolTimeoutError() const {
  std::fstream errorsFile(m_config.errorsFile().c_str(), ios_base::out | ios_base::app);
  if (!errorsFile) {
    LOG(log_error, m_config.getSessionId(), "error - can't open the errors file for writing");
  }
  else {
    LOG(log_debug, m_config.getSessionId(), "writing symbol timed out to error file");
    errorsFile << "symbol processing timed out - processing canceled";
  }
}

void RunSystem::saveMaxBarsExceededError(long totalBarCount) const {
  int n = 3;
  std::fstream errorsFile;
  for (errorsFile.open(m_config.errorsFile().c_str(), ios_base::out | ios_base::app);
    !errorsFile && n > 0;
    n--, errorsFile.open(m_config.errorsFile().c_str(), ios_base::out | ios_base::app))
    Sleep(10);

  if (!errorsFile) {
    LOG(log_error, m_config.getSessionId(), "error - can't open the errors file for writing");
  }
  else {
    LOG(log_debug, m_config.getSessionId(), "Writing \"total number of bars exceeded\" error to errors file");
    errorsFile << "Session exceeded maximum number of bars allowed: " << m_config.maxTotalBarCount() << ", " << totalBarCount << std::endl;
  }
}

void RunSystem::run() {
  try {
    LOG(log_debug, m_config.getSessionId(), " Start runsystem");
    std::wstring str;

    FileRuntimeStats runtimeStats( m_config.runtimeStatsFile());

    LOG(log_debug, m_config.getSessionId(), " slippage value: ", m_config.defSlippageValue());
    LOG(log_debug, m_config.getSessionId(), " slippage id: ", m_config.defSlippageId());

    LOG(log_debug, m_config.getSessionId(), " stats file: ", m_config.statsFile());
    Document document(m_config);

    std::shared_ptr<XSignalHandler> sh(std::make_shared< XSignalHandler >(runtimeStats));

    XRunnableRunInfoHandler rih(runtimeStats);
    XOutputSink os(m_config.outputFile());
    PositionsVector pv;
    std::shared_ptr<ErrorEventSinkImpl> errsink(std::make_shared< ErrorEventSinkImpl >(runtimeStats));
    LOG(log_debug, m_config.getSessionId(), " RunSystem::RunSystem - starting sessioin");
    TASession session(document, sh.get(), errsink.get(), &rih, &runtimeStats, os);
    session.start(pv);

    try {
      // this has to be called after start, as the symbol source is loaded then,
      // otherwise we'll get an exception
      runtimeStats.setTotalSymbols( session.defSymbolsCount() * m_config.getTradingSystemCount());
    }
    catch (const SymbolsSourceNotAvailableException&) {
      LOG(log_debug, m_config.getSessionId(), "SymbolsSourceNotAvailableException");
    }

    bool symbolTimedOut = false;
    bool maxTotalBarCountExceeded = false;
    Timer runtimeStatsTimer;
    // create the initial output stats file so the user doesn't see a blanc
    // screen
    runtimeStats.outputStats();

    while (session.isRunning()) {
      // check for timeout if haven't received the sessionEnded notification.
      // timeout in this context means a system that takes more than the
      // specified time on one symbol, and it's meant to prevent taking up too
      // much resources either accidentally, or in purpose sessionEnded signals
      // that all the systems have been run on all symbols, and now we may be
      // processing sessionEnded in various plugin config. The stats plugin
      // config may take a long time, so don't do timeout on it the reason there
      // is a timeout on symbols, is because the user code may contain infinite
      // loops, or other blocking elements and the session must have a way out
      // of this. when in plugins however, we control more or less things todo:
      // what if the user overrides the virtual sessionEnded for the system
      // plugin that is being run? Disable this!
      if (m_config.symbolTimeout() > 0 && rih.timeout(m_config.symbolTimeout()) && !session.sessionEndedReceived()) {
        // if timeout, just exit the loop and the process. This will crash the
        // session thread,
        LOG(log_debug, m_config.getSessionId(), " symbol timed out, in while loop");
        symbolTimedOut = true;
        break;
      }

      if (rih.exceededBarCount(m_config.maxTotalBarCount())) {
        LOG(log_debug, m_config.getSessionId(), " Session exceeded max number of bars allowed: ", m_config.maxTotalBarCount(), " exceeded");
        maxTotalBarCountExceeded = true;
        // cancel session than continue the loop waiting for the session to end
        session.cancel();
      }
      // save new stats every 1 seconds
      if (runtimeStatsTimer.elapsed() > 1) {
        runtimeStats.setRawTrades(session.runTradesCount());
        runtimeStats.outputStats();
        runtimeStatsTimer.restart();
      }
      Sleep(50);
    }

    // this gets all the positions in one container. It does a copy of
    // positions, so not very efficient
    PositionsContainer* posp(pv.getAllPositions());
    assert(posp != 0);
    PositionsContainer& pos(*posp);
    // sort positions so they look good in the list
    pos.sortByEntryTime();
    // create the final output stats file, to show in the stats page
    // but first make sure we have the right number of trades, as they are not
    // event based, but they need to be requested
    runtimeStats.setRawTrades(session.runTradesCount());
    runtimeStats.setProcessedTrades(pos.enabledCount());
    runtimeStats.setProcessedSignals(sh->processedSignalsCount());
    runtimeStats.setMessage("Session complete");
    runtimeStats.setStatus(RuntimeStatus::ENDED);
    runtimeStats.outputStats();

    LOG(log_info, m_config.getSessionId(), runtimeStats.to_json());

    saveTradesDescriptionFile(pos);
    saveTradesCSVFile(pos);
    saveErrorsFile(*errsink);

    if (symbolTimedOut) {
      saveSymbolTimeoutError();
    }

    if (maxTotalBarCountExceeded) {
      saveMaxBarsExceededError(rih.totalBarCount());
    }

    LOG(log_debug, m_config.getSessionId(), "\n*****\n*** Processing ended with no errors ***\n*****\n");
  }
  catch (const DocumentException& e) {
    ErrorFile(m_config.errorsFile(), e.message(), "Session error");

    LOG(log_error, m_config.getSessionId(), " DocumentException: ", e.message());
    throw RunSystemException(system_run_error, e.message());
  }
  catch (const PluginTreeException& e) {
    LOG(log_error, m_config.getSessionId(), " PluginTreeException: ", e.message());
    throw RunSystemException(system_run_error, e.message());
  }
  catch (const RunProcessException& e) {
    LOG(log_error, m_config.getSessionId(), " RunProcessException: ", e.message());
    throw RunSystemException(system_run_error, e.message());
  }
  catch (const SignalHandlerException& e) {
    LOG(log_error, m_config.getSessionId(), " SignalHandlerException: ", e.message());
    throw RunSystemException(system_run_error, e.message());
  }
  catch (const XPositionHandlerException& e) {
    LOG(log_error, m_config.getSessionId(), " XPositionHandlerException: ", e.message());
    throw RunSystemException(system_run_error, e.message());
  }
  catch (const TASessionException& e) {
    LOG(log_error, m_config.getSessionId(), " TASessionException: ", e.message());
    throw RunSystemException(system_run_error, e.message());
  }
  catch (const CoreException & e) {
    LOG(log_error, m_config.getSessionId(), " CoreException: ", e.message());
    throw RunSystemException(system_run_error, e.message());
  }
  catch( const ChartManagerException& e ){
    LOG(log_error, m_config.getSessionId(), " CoreException: ", e.what());
    throw RunSystemException(system_run_error, e.what());
  }
  catch (const std::exception& e) {
    ErrorFile(m_config.errorsFile(), "Internal server error", "Server error");
    LOG(log_error, m_config.getSessionId(), " exception: ", e.what());
    throw RunSystemException(system_run_error, "Unknown error");
  }
  catch (const IdNotFoundException & e)  {
    ErrorFile(m_config.errorsFile(), "Id not found: ", e.message());
    LOG(log_error, m_config.getSessionId(), " Id not found: ", e.message() );
  }
  catch (...) {
    ErrorFile(m_config.errorsFile(), "Internal server error", "Server error");
    LOG(log_error, m_config.getSessionId(), " Unknown exception");
    throw RunSystemException(system_run_error, "Unknown error");
  }
}
