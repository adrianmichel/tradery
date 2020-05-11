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
#include "positions.h"
#include "system.h"
#include "datamanager.h"
#include "scheduler.h"
#include "bars.h"
#include "ticks.h"
#include <switch.h>


BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
     break;
    case DLL_THREAD_ATTACH:
     break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

PositionsManagerAbstr::PositionsManagerAbstrPtr PositionsManagerAbstr::create(PositionsContainer::PositionsContainerPtr posList, DateTime startTrades, DateTime endTrades,
    Slippage* slippage, Commission* commission) {
  return std::dynamic_pointer_cast< PositionsManagerAbstr >( std::make_shared< PositionsManagerImpl>(posList, startTrades, endTrades, slippage, commission));
}

Scheduler* Scheduler::create(RunEventHandler* runEventHandler) {
  return new SchedulerImpl(runEventHandler);
}

PositionsContainer::PositionsContainerPtr PositionsContainer::create() {
  return std::dynamic_pointer_cast< PositionsContainer> ( std::make_shared< PositionsContainerImpl>());
}

DataManager* DataManager::create(unsigned int cacheSize) {
  return new DataManagerImpl(cacheSize);
}

BarsPtr tradery::createBars(const std::string& dataSourceName, const std::string& symbol, BarsAbstr::Type type, unsigned int resolution, DateTimeRangePtr range,
                            ErrorHandlingMode errorHandlingMode) {
  return std::make_shared< BarsImpl >(dataSourceName, symbol, type, resolution, range, errorHandlingMode);
}

Ticks* tradery::createTicks(const std::string& dataSourceName, const std::string& symbol, const Range* range) {
  return new TicksImpl(dataSourceName, symbol, range);
}

ErrorEvent::ErrorEvent(Types type, Category category, const std::string& systemName, const std::string& message, const std::string& symbol)
    : _type(type),
      _systemName(systemName),
      _message(message),
      _symbol(symbol),
      _category(category) {}

const std::string ErrorEvent::toString() const {
  return tradery::format( "Error type: ", _type, " - ", _message, " - system: \"", _systemName, "\", symbol: \"", _symbol, "\"");
}

Session::Session(RunEventHandler* runEventHandler)
    : _defScheduler(std::make_shared< SchedulerImpl >(runEventHandler)) {}

Session::~Session() {}

void Session::addRunnable(Runnable* runnable, PositionsVector& pos, ErrorEventSink* errorSink, DataInfoIteratorPtr  dataInfoIterator,
                          SignalHandler* signalHandler, RunnableRunInfoHandler* runnableRunInfoHandler, Slippage* slippage, Commission* commission,
                          chart::ChartManager* chartManager, const tradery::ExplicitTrades* explicitTrades) {
  _defScheduler->addRunnable(runnable, dataInfoIterator, pos, errorSink, signalHandler, runnableRunInfoHandler, slippage, commission, chartManager, explicitTrades);
}

void tradery::Session::addRunnable(Runnable* runnable, PositionsVector& pos, ErrorEventSink* errorSink, DataInfoIteratorPtr dataInfoIterator,
    const std::vector<SignalHandler*>& signalHandlers, RunnableRunInfoHandler* runnableRunInfoHandler, Slippage* slippage,
    Commission* commission, chart::ChartManager* chartManager, const tradery::ExplicitTrades* explicitTrades) {
  _defScheduler->addRunnable(runnable, dataInfoIterator, pos, errorSink, signalHandlers, runnableRunInfoHandler, slippage, commission, chartManager, explicitTrades);
}

std::string Signal::signalTypeAsString(SignalType type) {
  static const Switch<  SignalType, std::string > signalName{
    {
      { BUY_AT_MARKET, "Buy at market"},
      { BUY_AT_CLOSE, "Buy at close"},
      { BUY_AT_STOP, "Buy at stop"},
      { BUY_AT_LIMIT, "Buy at limit"},
      { SELL_AT_MARKET, "Sell at market"},
      { SELL_AT_CLOSE, "Sell at close"},
      { SELL_AT_STOP, "Sell at stop"},
      { SELL_AT_LIMIT, "Sell at limit"},
      { SHORT_AT_MARKET, "Short at market"},
      { SHORT_AT_CLOSE, "Short at close"},
      { SHORT_AT_STOP, "Short at stop"},
      { SHORT_AT_LIMIT, "Short at limit"},
      { COVER_AT_MARKET, "Cover at market"},
      { COVER_AT_CLOSE, "Cover at close"},
      { COVER_AT_STOP, "Cover at stop"},
      { COVER_AT_LIMIT, "Cover at limit"}
    }
  };

  std::optional< std::string > name = signalName[type];
  assert(name);
  return *name;
}

extern SeriesCache* _cache;

CORE_API void tradery::init(unsigned int cacheSize) {
  TA_RetCode retCode;
  retCode = TA_Initialize();

  if (retCode == TA_SUCCESS) {
    LOG(log_info, "TA-LIB correctly initialized.");
  }
  else {
    LOG(log_error, "Error initializing TA-LIB: ", retCode);
  }

  // the data cache is disabled for now
  _cache = new SeriesCache(100, false);
  _dataManager = new DataManagerImpl(cacheSize);
}

CORE_API void tradery::uninit() {
  delete _cache;
  delete _dataManager;
  TA_Shutdown();
}

extern DataManager* _dataManager;

CORE_API void tradery::registerDataSource(DataSource* dataSource) {
  assert(_dataManager != 0);
  assert(dataSource != 0);
  _dataManager->addDataSource(dataSource);
}

CORE_API bool tradery::unregisterDataSource(DataSource* dataSource) {
  assert(_dataManager != 0);
  assert(dataSource != 0);
  return _dataManager->removeDataSource(dataSource);
}

CORE_API bool tradery::unregisterDataSource(const UniqueId& dataSourceId) {
  assert(_dataManager != 0);
  return _dataManager->removeDataSource(dataSourceId);
}

CORE_API void tradery::setDataCacheSize(unsigned int cacheSize) {
  _dataManager->setCacheSize(cacheSize);
}

CORE_API void Session::run(bool asynch, unsigned int threads, bool cpuAffinity, DateTimeRangePtr range, DateTime startTradesDateTime) {
  _defScheduler->run(asynch, threads, cpuAffinity, range, startTradesDateTime);
}
/**
 * Indicates the running scheduler running status.
 *
 * @return true if any system thread is running, false if no system threads are
 * running
 */
CORE_API bool Session::isRunning() const { return _defScheduler->isRunning(); }

/**
 * Indicates the running scheduler canceling status.
 *
 * @return true if any system thread is running, false if no system threads are
 * running
 */
CORE_API bool Session::isCanceling() const {
  return _defScheduler->isCanceling();
}

/**
 * Cancels the current run synchronously, i.e. the call will return only after
 * there are no more runnables running
 *
 * The cancel works by telling all threads to return after the current runnable
 * has completed its run, so depending on the system, the data etc, the cancel
 * process may not be instantaneous
 *
 * @see runAsync
 */
CORE_API void Session::cancelSync() const { _defScheduler->cancelSync(); }
/**
 * Cancels the current run asynchrnously, i.e. it returns immediately,
 * without waiting for the runnables stop.
 *
 * The cancel works by telling all threads to return after the current runnable
 * has completed its run, so depending on the system, the data etc, the cancel
 * process may not be instantaneous
 *
 * When using this call, the user will usually have to check the running status
 * using isRunning before performing some other action on the SchedulerI object
 *
 * @see isRunning
 */
CORE_API void Session::cancelAsync() const { _defScheduler->cancelAsync(); }

CORE_API void Session::resetRunnables() { _defScheduler->resetRunnables(); }

CORE_API const std::string Signal::csvHeaderLine() {
  return "Symbol,Signal date/time,Shares,Side,Type,Price,Name,System id, System name, Position id";
}



CORE_API const std::string Signal::toCSVString() const {
  static const Switch< tradery::Signal::SignalType, std::string > signalAsCSVString {
    {
      { Signal::SignalType::BUY_AT_MARKET, "Buy,Market,"s },
      { Signal::SignalType::BUY_AT_STOP, "Buy,Stop,"s },
      { Signal::SignalType::BUY_AT_LIMIT, "Buy,Limit,"s},
      { Signal::SignalType::BUY_AT_CLOSE, "Buy,Close,"s},
      { Signal::SignalType::SELL_AT_MARKET, "Sell,Market,"s},
      { Signal::SignalType::SELL_AT_STOP, "Sell,Stop,"s},
      { Signal::SignalType::SELL_AT_LIMIT, "Sell,Limit,"s},
      { Signal::SignalType::SELL_AT_CLOSE, "Sell,Close,"s},
      { Signal::SignalType::SHORT_AT_MARKET, "Short,Market,"s},
      { Signal::SignalType::SHORT_AT_STOP, "Short,Stop,"s},
      { Signal::SignalType::SHORT_AT_LIMIT, "Short,Limit,"s},
      { Signal::SignalType::SHORT_AT_CLOSE, "Short,Close,"s},
      { Signal::SignalType::COVER_AT_MARKET, "Cover,Market,"s},
      { Signal::SignalType::COVER_AT_STOP, "Cover,Stop,"s},
      { Signal::SignalType::COVER_AT_LIMIT, "Cover,Limit,"s},
      { Signal::SignalType::COVER_AT_CLOSE, "Cover,Close,"s}
    }
  };

  std::optional< std::string > signal = signalAsCSVString[type()];
  assert(signal);

  return tradery::format( symbol(), ',', time().to_simple_string(), ',', shares(), ',', *signal, price(), ',',
    name(), ',', systemId(), ',', systemName(), ",", position() ? position().getId() : 0);
}

CORE_API DataRequester* tradery::getDataRequester() { return _dataManager; }

// this has to be created here because this is where it will get deleted
// if this is created in the fileplugin dll, it crashes at deletion
// most likely because the 2 dlls have different options, and also possibly
// because of creation and deletion in and from different heaps.
CORE_API DataLocationInfoPtr tradery::makeDataFileLocationInfo( const std::string& fileName, __int64 startPos, __int64 count) {
  return std::make_shared< DataFileLocationInfo >(fileName, startPos, count);
}
