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
#pragma warning(disable : 4482)

#include <float.h>
#include <fstream>
#include <algorithm>
#include "series.h"
#include "core.h"
#include "log.h"

class DateRange {
 protected:
  Date _from;
  Date _to;

 public:
  DateRange() : _from(NegInfinityDate()), _to(PosInfinityDate()) {}
  Date from() const { return _from; }
  Date to() const { return _to; }
  // returns duration in years
  double duration() const {
    DateDuration dd = _to - _from;
    // should be redone to take into account odd years
    return dd >= 0 ? ((double)dd.days()) / 365 : 0;
  }

  std::string toString() const {
    return _from.toString() + " - " + _to.toString();
  }
};

// calculates the equity date range from the session parameters
// this range must be finite (so -infinity or +infinity are not possible)
// and for this it uses info from the session time range and
// available data to generate this finite range
//
// Also, equity is calculated with a resolution of 1 day, so the range is
// date based
class EquityDateRange : public DateRange {
 public:
  EquityDateRange(const SessionInfo& si) {
    const RuntimeParams* rp = si.runtimeParams();
    assert(rp != 0);

    _from = rp->getRange()->from().date();
    _to = rp->getRange()->to().date();

    LOG(log_info, "Date range: ", _from.toString(), "-", _to.toString());

    assert(_from <= _to);

    tradery::SymbolsIteratorPtr it(si.symbolsIterator());
    assert(it != 0);

    for (SymbolConstPtr symbol = it->getFirst(); symbol.get() != 0; symbol = it->getNext()) {
      // get the data for the symbol
      BarsPtr data = si.getData(symbol->symbol());

      // there should be data for symbol, as we have position(s)

      // we'll assume the data is bars
      // todo: should handle other types
      //
      Bars bars;

      try {
        bars = Bars(dynamic_cast<const BarsAbstr*>(data.get()));
      }
      catch (const std::bad_cast&) {
        // dynamic cast failed, it should be bars.
        assert(false);
      }

      if (bars && bars.size() > 0) {
        // calculate the first date for which we need to calculate equity (it's
        // the min first date of all symbols for wich we have positions, or if
        // the range is earlier than that, it will be end of the range note: if
        // there are positions, _from and _to will be valid dates, if not they
        // will be a not_a_date, in that case equity will not be calculated
        if (_from.is_neg_infinity()) {
          _from = bars.time(0).date();
        }
        else {
          _from = std::min< Date >(_from, bars.time(0).date());
        }

        // calculate the last date for which we need to calculate equity (it's
        // the max last date of all symbols for which we have positions)
        if (_to.is_pos_infinity()) {
          _to = bars.time(bars.size() - 1).date();
        }
        else {
          _to = std::max< Date >(_to, bars.time(bars.size() - 1).date());
        }
      }
    }

    // todo: handle the case where the from is later then the local time, and
    // the to is +infinity in that case we end up with a range with from > to
    // for now just return 0 as duration for such a range, but is should a
    // better job at handling it
    if (_from.is_neg_infinity()) {
      _from = Date("01/01/1960");
    }

    if (_to.is_pos_infinity()) {
      _to = LocalTimeSec().date();
    }
  }
};

/* @cond */
namespace tradery {
/* @endcond */

class DataNotAvailableForSymbolException {};

/**
 * Abstract base class for statistics transformation classes.
 *
 * Classes derived from StatsToFormat will implement method to generate usually
 * a text based version of the statistics, to html, csv etc.
 */
class StatsToFormat {
 protected:
  std::ostream& _os;

 public:
  StatsToFormat(std::ostream& os) : _os(os) {}
  virtual ~StatsToFormat() {}
  virtual void subtitle(const std::string& st) const = 0;
  virtual void header(const DateRange& dateRange) const = 0;
  virtual void footer() const = 0;
  virtual void row(const std::string& name, double all, double longs, double shorts, double bh, bool minMax = true, bool pct = false, unsigned int precision = 2) const = 0;
  virtual void row(const std::string& name, const Date& all, const Date& longs, const Date& shorts, const Date& bh) const = 0;
};

/**
 * Generates a html stream version of the statistics
 */
class StatsToHTML : public StatsToFormat {
 private:
  mutable unsigned __int64 _count;

 public:
  StatsToHTML(std::ostream& os) : StatsToFormat(os), _count(0) {
    LOG(log_info, "[StatsToHTML constructor]");
  }
  void subtitle(const std::string& st) const override {
    _os << "<tr class=\"subheader\"><td colspan=\"5\">" << st << "</td></tr>" << std::endl << "<tr>" << std::endl;
    _count = 0;
  }

  void header(const DateRange& range) const override {
    _os << "<table class=\"statsTable\">" << std::endl;
    _os << "<tr class=\"h\"> <td class=\"h\"></td><td class=\"h\">Total stats</td> <td class=\"h\">Long stats</td> <td class=\"h\">Short "
           "stats</td> <td class=\"h\">Buy & Hold stats</td> </tr>" << std::endl;
    _os << "<tr class=\"d1\"><td class=\"c\">Range</td><td class=\"c\" colspan=\"4\" align=\"center\">"
        << range.from().to_simple_string() << " - " << range.to().to_simple_string() << "</td></tr>";
    _count = 0;
  }

  void footer() const override { _os << "</table>" << std::endl; }

  static std::string getColor(double value, double maxVal, double minVal, bool maxMin) {
    std::string style = value >= 0 ? "style=\"color:blue;" : "style=\"color:red;";
    if (maxMin && value > 0 && value == maxVal || value < 0 && value == minVal) {
      style += "font-weight:bold;";
    }

    style += "\">";
    return style;
  }

  void row(const std::string& name, double all, double longs, double shorts, double bh, bool maxMin = true, bool pct = false, unsigned int precision = 2) const override {
    double maxVal = max3(max2(all, longs), shorts, bh);
    double minVal = min3(min2(all, longs), shorts, bh);

    _os << "<tr class=\"" << (_count % 2 ? "d0\"" : "d1\"") << ">" << std::endl;
    _os << "\t<td class=\"c\">" << name << "</td>" << std::endl;
    _os << "\t<td class=\"c\" " << getColor(all, maxVal, minVal, maxMin) << std::fixed << std::setprecision(precision) << all << (pct ? " %" : "") << "</td>" << std::endl;
    _os << "\t<td class=\"c\" " << getColor(longs, maxVal, minVal, maxMin) << std::fixed << std::setprecision(precision) << longs << (pct ? " %" : "") << "</td>" << std::endl;
    _os << "\t<td class=\"c\" " << getColor(shorts, maxVal, minVal, maxMin) << std::fixed << std::setprecision(precision) << shorts << (pct ? " %" : "") << "</td>" << std::endl;
    _os << "\t<td class=\"c\" " << getColor(bh, maxVal, minVal, maxMin) << std::fixed << std::setprecision(precision) << bh << (pct ? " %" : "") << "</td>" << std::endl;
    _os << "</tr>" << std::endl;
    _count++;
  }

  void row(const std::string& name, const Date& all, const Date& longs, const Date& shorts, const Date& bh) const override {
    _os << "<tr class=\"" << (_count % 2 ? "d0\"" : "d1\"") << ">" << std::endl;
    _os << "\t<td class=\"c\">" << name << "</td>" << std::endl;
    _os << "\t<td class=\"c\">" << (all.is_not_a_date() ? "" : all.to_simple_string()) << "</td>" << std::endl;
    _os << "\t<td class=\"c\">" << (longs.is_not_a_date() ? "" : longs.to_simple_string()) << "</td>" << std::endl;
    _os << "\t<td class=\"c\">" << (shorts.is_not_a_date() ? "" : shorts.to_simple_string()) << "</td>" << std::endl;
    _os << "\t<td class=\"c\">" << (bh.is_not_a_date() ? "" : bh.to_simple_string()) << "</td>" << std::endl;
    _os << "</tr>" << std::endl;
    _count++;
  }
};

/**
 * Generates a csv version of the statistics
 */
class StatsToCSV : public StatsToFormat {
 public:
  StatsToCSV(std::ostream& os) : StatsToFormat(os) {}
  void subtitle(const std::string& st) const override {
    _os << std::endl << st << std::endl;
  }
  void header(const DateRange& dateRange) const override {
    _os << ",Total stats,Long stats,Short stats,Buy & Hold stats" << std::endl;
    _os << "Date Range," << dateRange.from().to_simple_string() << "," << dateRange.to().to_simple_string() << std::endl;
  }
  void footer() const override {}
  void row(const std::string& name, double all, double longs, double shorts, double bh, bool minMax = true, bool pct = false, unsigned int precision = 2) const override {
    _os << name << "," << std::fixed << std::setprecision(precision) << all
        << (pct ? "%" : "") << "," << std::fixed << std::setprecision(precision)
        << longs << (pct ? " %" : "") << "," << std::fixed
        << std::setprecision(precision) << shorts << (pct ? " %" : "") << ","
        << std::fixed << std::setprecision(precision) << bh << (pct ? " %" : "")
        << "," << std::endl;
  }

  void row(const std::string& name, const Date& all, const Date& longs, const Date& shorts, const Date& bh) const override {
    _os << name << "," << (all.is_not_a_date() ? "" : all.to_simple_string())
        << "," << (longs.is_not_a_date() ? "" : longs.to_simple_string()) << ","
        << (shorts.is_not_a_date() ? "" : shorts.to_simple_string()) << ","
        << (bh.is_not_a_date() ? "" : bh.to_simple_string()) << ","
        << std::endl;
  }
};

/**
 * Interface for a class that can get get the current price
 *
 * Current price in this context is the closing price on the last available bar.
 *
 * This price could be used for example when calculating statistics that involve
 * open positions, or for buy & hold stats.
 */
class CurrentPriceSource {
 public:
  virtual ~CurrentPriceSource() {}

  /**
   * Get the current price for a symbol.
   *
   * If data for the symbol is not available, throw a
   * DataNotAvailableForSymbolException.
   *
   * @param symbol The symbol for which the data is requested
   *
   * @return The value of the current price
   * @exception DataNotAvailableForSymbolException
   *                   Thrown in case data for the symbol is not available
   */
  virtual double get(const std::string& symbol) const = 0;
};

/**
 * Calculates and exposes slippage related statistics
 *
 * It is normally used as a component of the global Stats class
 *
 * @see Stats
 */
class SlippageStats : public PositionHandler {
 private:
  double _totalOpenSlippage;
  double _totalCloseSlippage;

 public:
  SlippageStats() { reset(); }

  /**
   * Total absolute slippage value incurred while opening positions
   *
   * It is calculated by adding all the slippage values for the opening side of
   * all positions
   *
   * @return Total open slippage value
   */
  double totalOpenSlippage() const { return _totalOpenSlippage; }
  /**
   * Total absolute slippage value incurred while closing positions
   *
   * It is calculated by adding all the slippage values for the closing side of
   * all closed positions For open positions, close slippage is 0
   *
   * @return Total close slippage value
   */
  double totalCloselippage() const { return _totalCloseSlippage; }
  /**
   * Total slippage, open and close, for all positions
   *
   * It is calculated by adding open and close slippage for all positions
   *
   * For open commission. close slippage is 0
   *
   * @return Total slippage value
   */
  double totalSlippage() const {
    return _totalOpenSlippage + _totalCloseSlippage;
  }

  /* @cond */
  virtual void onPosition( Position pos) override;
  /* @endcond */

  /**
   * Resets the slippage stats
   */
  void reset() {
    _totalOpenSlippage = 0;
    _totalCloseSlippage = 0;
  }

  /**
   * Operator+ adds a slippage stats object to the current slippage stats and
   * puts the result in a new SlippageStats object
   *
   * Returns the new SlippageStats object by value
   *
   * @param slippageStats
   *               The second SlippageStats operand of the + operation
   *
   * @return The new SlippageStats object containing the result of the addition
   */
  SlippageStats operator+(const SlippageStats& slippageStats) const {
    SlippageStats temp;

    temp._totalCloseSlippage = _totalCloseSlippage + slippageStats._totalCloseSlippage;
    temp._totalOpenSlippage = _totalOpenSlippage + slippageStats._totalOpenSlippage;

    return temp;
  }
};

/**
 * Calculates and exposes commission related statistics
 *
 * It is normally used as a component of the global Stats class
 *
 * @see Stats
 */
class CommissionStats : public PositionHandler {
 private:
  double _totalOpenCommission;
  double _totalCloseCommission;

 public:
  CommissionStats() { reset(); }

  /**
   * Total absolute commission value incurred while opening positions
   *
   * It is calculated by adding all the commission values for the opening side
   * of all positions
   *
   * @return Total open commission value
   */
  double totalOpenCommission() const { return _totalOpenCommission; }
  /**
   * Total absolute commission value incurred while closing trades
   *
   * It is calculated by adding all the commission values for the closing side
   * of all closed positions For open positions, close commission is 0
   *
   * @return Total close commission value
   */
  double totalCloseCommission() const { return _totalCloseCommission; }
  /**
   * Total commission, open and close, for all positions
   *
   * It is calculated by adding open and close commission for all positions
   *
   * For open positions. close commission is 0
   *
   * @return Total slippage value
   */
  double totalCommission() const {
    return _totalCloseCommission + _totalOpenCommission;
  }

  /* @cond */
  virtual void onPosition(Position pos);
  /* @endcond */

  /**
   * Resets the slippage stats
   */
  void reset() {
    _totalOpenCommission = 0;
    _totalCloseCommission = 0;
  }

  /**
   * Operator+ adds a commission stats object to the current commission stats
   * and puts the result in a new CommissionStats object
   *
   * Returns the new CommissionStats object by value
   *
   * @param commissionStats
   *               The second CommissionStats  operand of the + operation
   *
   * @return The new CommissionStats  object containing the result of the
   * addition
   */
  CommissionStats operator+(const CommissionStats& commissionStats) const {
    CommissionStats temp;

    temp._totalCloseCommission = _totalCloseCommission + commissionStats._totalCloseCommission;
    temp._totalOpenCommission = _totalOpenCommission + commissionStats._totalOpenCommission;

    return temp;
  }
};

/**
 * Calculates statistics related to positions: counts, ratios, commission,
 * slippage etc
 *
 * It is used as part of the Stats class
 *
 * It is also a base class for more specialized positions stats clsses:
 * OpenPosStats (for open positions only) and ClosedPosStats (for closed
 * positions only)
 *
 * Note: when calculating position gain stats, gain for a close position is the
 * difference (with the right sign for either longs or shorts) between the exit
 * and entry costs for that position, including commission.
 *
 * For an open position, the gain is calculated as the most recent price
 * available for that symbol minus the position entry price, including
 * commsssion
 *
 * Percentage gain(loss) for a position is defined as gain(loss)/entry cost *
 * 100
 *
 *
 * @see Stats
 * @see CommissionStats
 * @see SlippageStats
 * @see OpenPosStats
 * @see ClosedPosStats
 */
class PosStats {
 protected:
  size_t _count;

  // closed positions stats
  size_t _winningCount;
  size_t _losingCount;
  size_t _neutralCount;

  double _gain;

  double _maxGainPerPos;
  double _maxLossPerPos;

  double _maxPctGainPerPos;
  double _maxPctLossPerPos;

  // this is on a position basis, meaning the sume of all percentage gains for
  // all positions
  double _totalPctGain;
  double _annualizedPctGain;

  double _totalOpenCost;
  double _totalCloseCost;

  // adding the gain of all winning positions
  double _totalGain;
  // adding the loss of all losing positions
  double _totalLoss;

  CommissionStats _commissionStats;
  SlippageStats _slippageStats;

  double _initialCapital;
  double _endingCapital;

 protected:
  void calc(Position pos, double gain, double pctGain);

 public:
  PosStats() { reset(); }

  void setInitialCapital(double initialCapital) {
    _initialCapital = initialCapital;
  }

  void setEndingCapital(double endingCapital) {
    _endingCapital = endingCapital;
  }

  double initialCapital() const { return _initialCapital; }
  double endingCapital() const { return _endingCapital; }

  /**
   * Resets the position stats
   */
  void reset() {
    _count = 0;
    _winningCount = 0;
    _losingCount = 0;
    _neutralCount = 0;
    _gain = 0;
    _maxGainPerPos = 0;
    _maxLossPerPos = 0;
    _maxPctGainPerPos = 0;
    _maxPctLossPerPos = 0;
    _totalOpenCost = 0;
    _totalCloseCost = 0;
    _totalPctGain = 0;

    _commissionStats.reset();
    _slippageStats.reset();
    _annualizedPctGain = 0.0;
    _totalLoss = _totalGain = 0;
  }

  /**
   * The total number of positions
   *
   * @return
   */
  size_t count() const { return _count; }
  /**
   * The number of positions whose gain is strictly positive.
   *
   * @return The number of winning positions
   * @see Position::gain
   */
  size_t winningCount() const { return _winningCount; }
  /**
   * The number of positions whose gain is strictly negativ.
   *
   * @return The number of losing positions
   * @see Position::gain
   */
  size_t losingCount() const { return _losingCount; }
  /**
   * The number of positions whose gain is 0.
   *
   * @return The number of neutral positions
   * @see Position::gain
   */
  size_t neutralCount() const { return _neutralCount; }
  /**
   * Winning positions as a percentage of the total number of positions
   *
   * @return Percentage of winning positions
   */
  double pctWinning() const {
    double d = ((double)_winningCount) / ((double)_count) * 100.0;
    return _count > 0 ? ((double)_winningCount) / ((double)_count) * 100.0 : 0;
  }
  /**
   * Losing positions as a percentage of the total number of positions
   *
   * @return Percentage of winning positions
   */
  double pctLosing() const {
    return _count > 0 ? ((double)_losingCount) / ((double)_count) * 100.0 : 0;
  }
  /**
   * Neutral positions as a percentage of the total number of positions
   *
   * @return Percentage of winning positions
   */
  double pctNeutral() const {
    return _count > 0 ? ((double)_neutralCount) / ((double)_count) * 100.0 : 0;
  }

  /**
   * Total gain/loss, calculated as the sum of the gains of all individual
   * closed positions
   *
   * @return The total gain
   * @see Position::gain
   */
  double gainLoss() const { return _gain; }
  double pctGainLoss() const {
    assert(_initialCapital > 0);
    LOG(log_info, "In pctGainLoss, initialcap: ", _initialCapital, ", gain: ", _gain);
    return _gain / _initialCapital * 100;
  }
  /**
   * The maximum gain for any position
   *
   * If there was no winning position, this function will return 0
   *
   * @return The maximum gain or 0 if no winning position
   * @see Position::gain
   */
  double maxGainPerPos() const { return _maxGainPerPos; }
  /**
   * The maximum loss for any position
   *
   * The maximum loss will be a negative value or 0 if there was no losing
   * position
   *
   * @return The maximum loss or 0 if no losing position
   * @see Position::gain
   */
  double maxLossPerPos() const { return _maxLossPerPos; }
  /**
   * The maximum percentage gain for any position
   *
   * It will be a positive or 0 value, if there were no winning positions
   *
   * @return The maximum percentage gain
   * @see Position::gain
   */
  double maxPctGainPerPos() const { return _maxPctGainPerPos; }
  /**
   * The maximum percentage loss for any position
   *
   * It will be a negative or 0 value, if there were no losing positions
   *
   * @return The maximum loss
   * @see Position::gain
   */
  double maxPctLossPerPos() const { return _maxPctLossPerPos; }
  /**
   * The average gain/loss for all positions
   *
   * Defined as the sum of all gain/loss values for all position / number of
   * positions
   *
   * @return The average gain/loss
   */
  double averageGainLossPerPos() const {
    return _count > 0 ? _gain / (double)_count : 0;
  }
  /**
   * The average percentage gain/loss for all positions
   *
   * Defined as the sum of all percentage gain/loss values for all position /
   * number of positions
   *
   * @return The average percentage gain/loss
   */
  double averagePctGainLossPerPos() const {
    return _count > 0 ? _totalPctGain / (double)_count : 0;
  }

  /**
   * Returns a reference to the CommissionStats objects associated with the
   * current PosStats
   *
   * @return Reference to the CommissionStats
   */
  const CommissionStats& commissionStats() const { return _commissionStats; }
  /**
   * Returns a reference to the SlippageStats objects associated with the
   * current PosStats
   *
   * @return Reference to the SlippageStats
   */
  const SlippageStats& slippageStats() const { return _slippageStats; }

  /**
   * Calculates the average commission per position
   *
   * It is defined as the total commission (open and close)/positions count .
   *
   * @return Average commission per position
   */
  double averageCommissionPerPos() const {
    return _count > 0 ? _commissionStats.totalCommission() / (double)_count : 0;
  }
  /**
   * Calculates the average slippage per position
   *
   * It is defined as the total slippage (open and close)/positions count .
   *
   * @return Average commission per position
   */
  double averageSlippagePerPos() const {
    return _count > 0 ? _slippageStats.totalSlippage() / (double)_count : 0;
  }

  double averageGainPerWinningPos() const {
    return winningCount() > 0 ? _totalGain / winningCount() : 0;
  }
  double averageLossPerLosingPos() const {
    return losingCount() > 0 ? _totalLoss / losingCount() : 0;
  }

  double expectancy() const {
    LOG(log_info, "total loss: ", _totalLoss, ", total gain: ", _totalGain);
    LOG(log_info, "pct winning: ", pctWinning(), ", avg gain per pos: ", averageGainPerWinningPos(), ", pct losing: ", pctLosing(), ", avg loss per pos: ", averageLossPerLosingPos());
    // use + between the two because loss is negative, so they get subtracted in
    // the end as they should
    return pctWinning() / 100.0 * averageGainPerWinningPos() +
           pctLosing() / 100.0 * averageLossPerLosingPos();
  }

  /**
   * operator+ which will generate a new PosStats by adding the current values
   * to another PosStats object
   *
   * @param posStats The second term of the + operation
   *
   * @return The new sum PosStats instance
   */
  PosStats operator+(const PosStats& posStats) const {
    PosStats temp;

    assert(_initialCapital == posStats._initialCapital);

    temp._initialCapital = _initialCapital;
    temp._count = _count + posStats.count();
    temp._winningCount = _winningCount + posStats.winningCount();
    temp._losingCount = _losingCount + posStats.losingCount();
    temp._neutralCount = _neutralCount + posStats.neutralCount();
    temp._gain = _gain + posStats.gainLoss();
    temp._maxGainPerPos = std::max< double >(_maxGainPerPos, posStats.maxGainPerPos());
    temp._maxLossPerPos = std::min< double >(_maxLossPerPos, posStats.maxLossPerPos());
    temp._maxPctGainPerPos = std::max< double >(_maxPctGainPerPos, posStats.maxPctGainPerPos());
    temp._maxPctLossPerPos = std::min< double >(_maxPctLossPerPos, posStats.maxPctLossPerPos());
    temp._totalOpenCost = _totalOpenCost + posStats._totalOpenCost;
    temp._totalCloseCost = _totalCloseCost + posStats._totalCloseCost;
    temp._totalPctGain = _totalPctGain + posStats._totalPctGain;
    temp._commissionStats = _commissionStats + posStats._commissionStats;
    temp._slippageStats = _slippageStats + posStats._slippageStats;
    temp._totalLoss = _totalLoss + posStats._totalLoss;
    temp._totalGain = _totalGain + posStats._totalGain;

    return temp;
  }

  void calculateAnnualizedPctGain(const DateRange& dr) {
    LOG(log_debug, "range: ", dr.toString());
    LOG(log_debug, "initial capital: ", _initialCapital);
    LOG(log_debug, "duration: ", dr.duration());

    assert(dr.duration() >= 0);
    _annualizedPctGain = dr.duration() > 0 ? (pow(_endingCapital / _initialCapital, 1.0 / dr.duration()) - 1) * 100.0 : 0;
    LOG(log_info, "annualized gain: ", _annualizedPctGain);
  }

  double annualizedPctGain() const { return _annualizedPctGain; }
};

/**
 * Specialized PosStats class, which will calculate position statistics only for
 * open positions
 *
 * @see PosStats
 * @see ClosedPosStats
 */
class OpenPosStats : public PosStats {
 public:
  /* @cond */
  virtual void onPosition(Position pos, const CurrentPriceSource& cpr);
  /* @endcond */
};

/**
 * Specialized PosStats class, which will calculate position statistics only for
 * closed positions
 *
 * @see PosStats
 * @see OpenPosStats
 */
class ClosedPosStats : public PosStats {
 public:
  /* @cond */
  virtual void onPosition(Position pos);
  /* @endcond */
};

/**
 * Contains statistics for a collection of positions.
 *
 * Includes
 * - positions statistics (total number of trades, number of winning trades etc)
 * - performance statistics (total gain, average gain, etc.)
 * - slippage statistics (average slippage etc)
 * - commission statistics (average commission etc)
 *
 * This class only contains the accessor methods, while the actual calculation
 * is done by the derived class StatsCalculator
 *
 * @see PositionEqualPredicate
 * @see PositionsContainer
 * @see PositionHandler
 */
class CORE_API Stats {
 protected:
  OpenPosStats _openPosStats;
  ClosedPosStats _closedPosStats;
  PosStats _allPosStats;
  DateRange _dateRange;
  double _traderyScore;

  double _initialCapital;
  double _endingCapital;

 public:
  Stats() : _initialCapital(0), _endingCapital(0), _traderyScore(0) {}

  virtual ~Stats() {}

  void setInitialCapital(double initialCapital) {
    _initialCapital = initialCapital;
    _openPosStats.setInitialCapital(initialCapital);
    _closedPosStats.setInitialCapital(initialCapital);
    _allPosStats.setInitialCapital(initialCapital);
  }
  void setEndingCapital(double endingCapital) {
    LOG(log_info, endingCapital);
    _endingCapital = endingCapital;
    _openPosStats.setEndingCapital(endingCapital);
    _closedPosStats.setEndingCapital(endingCapital);
    _allPosStats.setEndingCapital(endingCapital);
    _allPosStats.calculateAnnualizedPctGain(_dateRange);
  }

  const PosStats& openPosStats() const { return _openPosStats; }
  const PosStats& closePosStats() const { return _closedPosStats; }
  const PosStats& allPosStats() const { return _allPosStats; }

  double pctWinningClosed() const {
    return _allPosStats.count() > 0 ? (double)_closedPosStats.winningCount() / (double)_allPosStats.count() * 100.0 : 0;
  }
  double pctLosingClosed() const {
    return _allPosStats.count() > 0 ? (double)_closedPosStats.losingCount() / (double)_allPosStats.count() * 100.0 : 0;
  }
  double pctNeutralClosed() const {
    return _allPosStats.count() > 0 ? (double)_closedPosStats.neutralCount() / (double)_allPosStats.count() * 100.0 : 0;
  }

  double pctWinningOpen() const {
    return _allPosStats.count() > 0 ? (double)_openPosStats.winningCount() / (double)_allPosStats.count() * 100.0 : 0;
  }
  double pctLosingOpen() const {
    return _allPosStats.count() > 0 ? (double)_openPosStats.losingCount() / (double)_allPosStats.count() * 100.0 : 0;
  }
  double pctNeutralOpen() const {
    return _allPosStats.count() > 0 ? (double)_openPosStats.neutralCount() / (double)_allPosStats.count() * 100.0 : 0;
  }

  //  double endingCapital() const { return _initialCapital + realizedGain(); }

  void reset() {
    _openPosStats.reset();
    _closedPosStats.reset();
    _allPosStats.reset();
  }

  void setDateRange(const DateRange& dateRange) { _dateRange = dateRange; }
  const DateRange& dateRange() const { return _dateRange; }

  void calculateScore(double pctExposure, double ulcerIndex) {
    double apg = _allPosStats.annualizedPctGain();
    double sign = apg > 0 ? 1.0 : -1.0;
    _traderyScore = apg * (1 - sign * pctExposure / 100) * (1.00 - sign * std::min< double >(ulcerIndex, 20.0) / 20.0);

    LOG(log_info, "score: ", _traderyScore, ", APG: ", apg, ", exposure: ", pctExposure, ", ulcer index: ", ulcerIndex);
  }

  double getScore() const { return _traderyScore; }
};
/**
 * Calculates statistics for a collection of positions
 *
 * The calculation can be done on the entire collection of positions, or on a
 * subset whose elements meet a certain criteria, as specified by the
 * PositionEqualPredicate, therefore the values calculated could be different
 * for the same PositionsContainer, depending on the actual predicate used.
 *
 * Note: the StatsCalculator class is derived from PositionsHandler, as the
 * PositionsContainer is traversed using one of the forEach method.
 */
class CORE_API StatsCalculator : public Stats, public PositionHandler {
 private:
  const CurrentPriceSource& _cpr;

 private:
  /* @cond */
  void onPosition(Position pos);

  /* @endcond */

 public:
  /**
   * Constructor - takes a CurrentPriceSource reference as parameter
   *
   * The CurrentPriceSource will be used to calculate stats for open positions
   *
   * @param currentPriceRequester
   */
  StatsCalculator(const CurrentPriceSource& currentPriceSource);
  /**
   * Calculates the statistics on a subset of a PositionsContainer, as defined
   * by the PositionEqualPredicate.
   *
   * The PositionEqualPredicate defines which positions are to be considered
   * when calculating the statistics. For example if its operator== method
   * returns true for short positions only, than the statistics will by
   * calculated on short positions only.
   *
   * @param positions The positions container on which to calculate the
   * statistics
   * @param predicate The predicate indicating which positions are to be
   * consdered when calculating the statistics
   *
   * @see PositionEqualPredicate
   * @see PositionsContainer
   */
  void calculate(PositionsContainer& positions, const PositionEqualPredicate& predicate);
  /**
   * Calculates statistics on long positions only.
   *
   * @param positions The PositionsContainer for which the stats are to be
   * calculated
   *
   * @see PositionsContainer
   */
  void calculateLong(PositionsContainer& positions);
  /**
   * Calculates statistics on short positions only.
   *
   * @param positions The PositionsContainer for which the stats are to be
   * calculated
   *
   * @see PositionsContainer
   */
  void calculateShort(PositionsContainer& positions);
  /**
   * Calculates the statistics on all positions in a PositionsContainer.
   *
   * @param positions The positions container on which to calculate the
   * statistics
   *
   * @see PositionEqualPredicate
   * @see PositionsContainer
   */
  void calculateAll(PositionsContainer& positions);
};

class ProcessPosition {
 private:
  Position _pos;

 protected:
  ProcessPosition(Position pos) : _pos(pos) {}

 public:
  const Position& get() const { return _pos; }
  Position& get() { return _pos; }

  PositionId id() const { return _pos.getId(); }
  virtual bool lower(ProcessPosition& pos) const = 0;
  virtual DateTime time() const = 0;
  virtual OrderType type() const = 0;

  bool operator<(ProcessPosition& pos) const {
    OrderType otherType = pos.type();
    OrderType crtType = type();

    if (time() < pos.time()) {
      return true;
    }
    else if (time() > pos.time()) {
      return false;
    }
    else if (crtType != otherType) {
      if (crtType == market_order || otherType == close_order) {
        return true;
      }

      if (crtType == close_order || otherType == market_order) {
        return false;
      }
      else {
        return lower(pos);
      }
    }
    else {
      return lower(pos);
    }
  }

  virtual bool entry() const = 0;
};

class OpenPosition : public ProcessPosition {
 public:
  OpenPosition(Position pos) : ProcessPosition(pos) {}

  DateTime time() const { return __super::get().getEntryTime(); }

  OrderType type() const { return __super::get().getEntryOrderType(); }

  bool lower(ProcessPosition& pos) const override { return id() == pos.id(); }
  bool entry() const override { return true; }
};

class ClosePosition : public ProcessPosition {
 public:
  ClosePosition(Position pos) : ProcessPosition(pos) {}

  DateTime time() const { return __super::get().getCloseTime(); }

  OrderType type() const { return __super::get().getExitOrderType(); }

  bool lower(ProcessPosition& pos) const override { return id() < pos.id(); }

  bool entry() const override { return false; }
};

class ProcessPositionPtr : public std::shared_ptr<ProcessPosition> {
 public:
  ProcessPositionPtr(ProcessPosition* p) : std::shared_ptr<ProcessPosition>(p) {}

  bool operator<(ProcessPositionPtr p) const { return **this < *p; }
};

using ProcessPositionsBase = std::multiset<ProcessPositionPtr>;
class ProcessPositions : public ProcessPositionsBase {
 private:
  mutable ProcessPositionsBase::iterator _i;
  mutable ProcessPositionsBase::const_iterator _ci;

 public:
  void insertEntry(Position pos) {
    __super::insert(ProcessPositionPtr(new OpenPosition(pos)));
  }

  void insertExit(Position pos) {
    assert(pos.isClosed());
    __super::insert(ProcessPositionPtr(new ClosePosition(pos)));
  }

  ProcessPosition* getFirst() {
    _i = __super::begin();

    return getNext();
  }

  ProcessPosition* getNext() {
    while (_i != __super::end()) {
      ProcessPosition* pp = (_i++)->get();
      if (pp->get().isEnabled()) {
        return pp;
      }
    }
    return 0;
  }

  const ProcessPosition* getFirst() const {
    _ci = __super::begin();

    return getNext();
  }

  const ProcessPosition* getNext() const {
    while (_ci != __super::end()) {
      ProcessPosition* pp = (_ci++)->get();
      if (pp->get().isEnabled()) {
        return pp;
      }
    }
    return 0;
  }
};

using ProcessPositionsPtr = std::shared_ptr<ProcessPositions>;

class DateToProcessPositions : public tradery::PositionHandler {
 private:
  using DPPMap = std::map<Date, ProcessPositionsPtr>;

  DPPMap _map;

 private:
  void insertPosition(Position pos) {
    DPPMap::iterator i = _map.find(pos.getEntryDate());
    if (i == _map.end()) {
      i = _map.insert(DPPMap::value_type(pos.getEntryDate(), std::make_shared< ProcessPositions >())).first;
    }

    ProcessPositionsPtr pp = i->second;

    pp->insertEntry(pos);

    if (pos.isClosed()) {
      i = _map.find(pos.getCloseDate());
      if (i == _map.end()) {
        i = _map.insert(DPPMap::value_type(pos.getCloseDate(), std::make_shared< ProcessPositions >())).first;
      }

      ProcessPositionsPtr pp1 = i->second;

      pp1->insertExit(pos);
    }
  }

 public:
  DateToProcessPositions(const PositionsContainer& pc) {
    pc.forEachConst(*this);
  }

  virtual void onPosition(Position pos) override { insertPosition(pos); }

  ProcessPosition* getFirst(Date& date) {
    DPPMap::iterator i = _map.find(date);
    if (i != _map.end()) {
      return i->second->getFirst();
    }
    else {
      return 0;
    }
  }

  ProcessPosition* getNext(Date& date) {
    DPPMap::iterator i = _map.find(date);
    if (i != _map.end()) {
      return i->second->getNext();
    }
    else {
      return 0;
    }
  }
};

//#define EQOUT 1

class Eq {
 private:
  double _cash;
  double _total;

 public:
  Eq(double cash = 0) : _cash(cash), _total(cash) {}

 public:
  void adjustEntry(Position pos) { _cash -= pos.getEntryCost(); }

  void adjustExit(Position pos, double price) {
    if (pos.isLong()) {
      _cash += pos.getCloseIncome();
    }
    else {
      _cash += pos.getEntryCost() + pos.getGain();
    }

    _total += pos.getCloseIncome(price);
  }

  void adjust(Position pos, double adj) { _total += adj; }

  Eq& operator+=(const Eq& eq) {
    _total += eq.getTotal();
    _cash += eq.getCash();

    return *this;
  }

  double getCash() const { return _cash; }
  double getTotal() const { return _total; }
};

class Equity {
 private:
  Eq _all;
  Eq _sh;
  Eq _lg;

 public:
  Equity(double cash = 0) : _all(cash), _sh(cash), _lg(cash) {}

  Equity(const Equity& eq) : _all(eq._all), _sh(eq._sh), _lg(eq._lg) {}

  const Eq& getAll() const { return _all; }
  const Eq& getShort() const { return _sh; }
  const Eq& getLong() const { return _lg; }

  // called on entry, closePrice is the closing price on the entry bar
  void adjustEntry(Position pos) {
    _all.adjustEntry(pos);
    pos.isLong() ? _lg.adjustEntry(pos) : _sh.adjustEntry(pos);
  }

  void adjustExit(Position pos, double price) {
    _all.adjustExit(pos, price);
    pos.isLong() ? _lg.adjustExit(pos, price) : _sh.adjustExit(pos, price);
  }

  void adjust(Position pos, double adj) {
    _all.adjust(pos, adj);
    pos.isLong() ? _lg.adjust(pos, adj) : _sh.adjust(pos, adj);
  }

  void dump(std::ostream& os) {
  }

  Equity& operator+=(const Equity& eq) {
    _all += eq._all;
    _sh += eq._sh;
    _lg += eq._lg;

    return *this;
  }
};

// returns the last bar - 1 if the position spans more than one bar, otherwise
// it returns the last bar
#define LAST_BAR_INDEX(pos) \
  (pos.getCloseBar() - (pos.getDuration() > 0 ? 1 : 0))

using EquityCurveBase = std::map<Date, Equity>;

/**
 * An equity curve collection, implemented as map <Date
 * - Equity>
 *
 */
class EquityCurve : public EquityCurveBase {
 private:
  mutable std::vector<double> _total;
  mutable std::vector<double> _short;
  mutable std::vector<double> _long;
  mutable std::vector<double> _cash;
  //  const PositionsEntries _entries;
  const SessionInfo& _si;
  // used to count the currently open positions
  DateToProcessPositions _dpp;
  const DateRange& _edr;
  const bool _doPosSizing;

  unsigned int _openPosCount;
  std::shared_ptr<const PositionEqualPredicate> _pred;

  // used to calculate exposure: 1 - total cash / total equity
  // the total amount of equity over the whole period
  //
  Eq _allSum;
  Eq _shortSum;
  Eq _longSum;

  void onExitPosition(Position pos, const BarsAbstr* bars) {
    assert(pos.isClosed());
    --_openPosCount;
    // for positions opened and closed on the same bar, use the close of the
    // same bar,
    // for others, use the close of the previous bar
    Bar b = bars->getBar(LAST_BAR_INDEX(pos));
    Equity& ec = get(pos.getCloseDate());
    ec.adjustExit(pos, b.getClose());
  }

  /**
   * Used internally by EquityCurve to calculate the equity values for each
   * position/date as a result of this, each date equity will be the delta from
   * the previous date
   */
  void onEntryPosition(Position pos, const BarsAbstr* bars) {
#ifdef EQOUT
      LOG( ( log_debug, "onEntryPosition: " << pos.getSymbol() );
#endif
      assert( pos );
      assert( bars != 0 );

      ++_openPosCount;

      // calculate for the whole duration of the position, the end bar exclusive
      // if it's still open, to the most recent bar
      size_t endBar = pos.isClosed() ? LAST_BAR_INDEX( pos ) : bars->size() - 1;

      double prevClose =  0;

      // for all bars the position was open
      for( size_t n = pos.getEntryBar(); n <= endBar; n++ )
      {
        // get current bar
        Bar b = bars->getBar(n);
        // get current bar time stamp
        Date d = b.time().date();

  #ifdef EQOUT
        LOG(log_info, d.to_simple_string() << "-");
  #endif
        Equity& eq = get(d);

        // process the entry
        if (n == pos.getEntryBar()) {
          eq.adjustEntry(pos);
          // increase the equity with the amount the position is worth at the end
          // of the bar, but only if the position was held at least one bar
          eq.adjust(pos, pos.getGain(b.getClose()));
        }
        else {
          eq.adjust(pos, pos.getGain(prevClose, b.getClose()));
        }

        prevClose = b.getClose();
      }
  }

  /**
   * Calculates the final equity values, from the delta's
   * calculated by the position handler
   *
   * @param from
   * @param to
   * @param initialCapital
   */
  void calculate(double initialCapital) {
#ifdef EQOUT
    LOG(log_info, "**** eq calculation *****");
#endif
    assert(_si.runtimeParams()->positionSizing() != 0);

    // for all the dates in the range
    // todo: only use the dates for which there was data
    //

    Equity prevEquity(initialCapital);

    for (Date d = _edr.from(); d <= _edr.to(); d = d + Days(1)) {
      // get current equity
      Equity& ec = get(d);
      ec += prevEquity;
      // if no value for the current day, create a new equity entry with the
      // most recent value

      for (ProcessPosition* pp = _dpp.getFirst(d); pp != 0; pp = _dpp.getNext(d)) {
        Position pos = pp->get();
        assert(pos);

        // the position should be valid

        BarsPtr data = _si.getData(pos.getSymbol());

        const BarsAbstr* bars = 0;

        try {
          // we'll assume the data is bars
          // todo: should handle other types
          bars = dynamic_cast<const BarsAbstr*>(data.get());
          // there should be data for the symbol - we have a position
          assert(bars != 0);
          assert(bars->size() > 0);

        }
        catch (const std::bad_cast&) {
          assert(false);
        }

        if (pp->entry()) {
          // do position sizing and equity curve processing for each entry/exit,
          // in order.
          if (_doPosSizing && pos.applyPositionSizing()) {
            if (!posSizing(pos, bars, ec)) {
              continue;
            }
          }

          onEntryPosition(pos, bars);

        }
        else {
#ifdef EQOUT
          LOG(log_info, d.to_simple_string() << "-");
#endif
          onExitPosition(pp->get(), bars);
        }
      }

      _allSum += ec.getAll();
      _shortSum += ec.getShort();
      _longSum += ec.getLong();

      prevEquity = ec;
    }
  }

  // gets an equity at a certain date. If there isn't any, it creates a new one
  // based on the current eq point
  Equity& get(const Date& date) {
    Equity* eq = getEquity(date);
    if (eq == 0) {
      eq = insert(date, Equity());
      assert(eq != 0);
    }

    return *eq;
  }

  // returns false if the position has been disabled and doesn't need further
  // processing true if the position is still active, and needs further
  // processing.
  bool posSizing(Position pos, const BarsAbstr* bars, const Equity& ec) {
    const PositionSizingParams& ps(*_si.runtimeParams()->positionSizing());

    // there can be more than the allowed number of positions, if there were
    // explicit trades, on which we don't apply position sizing - they are
    // taken unconditionally

    // first do max number of pos - if over limit, disable the position
    if (!ps.maxOpenPos().unlimited() && _openPosCount >= ps.maxOpenPos().get()) {
      pos.disable();
      return false;
    }

    size_t newShares;

    // now adjust the position size according to pos sizing params
    switch (ps.posSizeType()) {
      case PosSizeType::system_defined:
        // no adjustment, using the position size set by the system
        newShares = pos.getShares();
        break;
      case PosSizeType::shares:
        // setting the new number of shares
        newShares = (size_t)round(ps.posSize());
        break;
      case PosSizeType::size:
        newShares = (size_t)round(ps.posSize() / pos.getEntryPrice());
        break;
      case PosSizeType::pctEquity:
        newShares = (size_t)round((ec.getAll().getTotal() * ps.posSize() / 100) / pos.getEntryPrice());
        break;
      case PosSizeType::pctCash:
        newShares = (size_t)round((ec.getAll().getCash() * ps.posSize() / 100) / pos.getEntryPrice());
        break;
      default:
        assert(false);
    }

    // now make sure the size meets the limit requirements
    switch (ps.posSizeLimitType()) {
      case PosSizeLimitType::none:
        break;
      case PosSizeLimitType::pctVolume: {
          size_t bar = pos.getEntryBar() == 0 ? 0 : pos.getEntryBar() - 1;

          size_t maxShares = (size_t)(ps.posSizeLimit() / 100 * bars->volume(bar));

          if (newShares > maxShares) {
            newShares = maxShares;
          }
        } 
        break;
      case PosSizeLimitType::limit:
        if (pos.getEntryPrice() * newShares > ps.posSizeLimit()) {
          newShares = (size_t)(ps.posSizeLimit() / pos.getEntryPrice());
        }
        break;
      default:
        assert(false);
        break;
    }

#ifdef EQOUT
    LOG(log_info, "entry cost: " << pos.getEntryCost(newShares) /*_T( ", cash: " ) << ec.getCash() << */);
#endif

    // then make sure the available cash doesn't go below 0
    if (pos.getEntryCost(newShares) > ec.getAll().getCash()) {
#ifdef EQOUT
      LOG(log_info, "position disabled");
#endif
      pos.disable();
      return false;
    }
    else {
    // position taken
#ifdef EQOUT
      LOG(log_info, "position taken");
#endif
      pos.setShares(newShares);
      return true;
    }
  }

  /**
   * Gets the equity for a specific date.
   *
   * @param date
   *
   * @return Pointer to Equity if found, 0 if not
   */
  Equity* getEquity(const Date& date) {
    iterator i = __super::find(date);

    return i == end() ? 0 : &(i->second);
  }

  double getCash(const Date& date) {
    Equity* eq = getEquity(date);
    assert(eq != 0);
    return eq->getAll().getCash();
  }

  /**
   * Adds equity for a date for which there is no equity
   *
   * Precondition: no equity for that date
   *
   * @param date   The date for which to add equity
   * @param eq
   *
   * @return Pointer to the added equity
   */
  Equity* insert(const Date& date, const Equity& eq) {
    assert(__super::find(date) == end());
    // todo: handle this case in release mode

    __super::insert(__super::value_type(date, eq));
    return &((*this)[date]);
  }

 public:
  const Equity* getEquity(const Date& date) const {
    const_iterator i = __super::find(date);
    return i == end() ? 0 : &(i->second);
  }

  /**
   * Constructs an EquityCurve objec for a collection of positions.
   *
   * The predicate can be used to limit the positions according to any rule:
   * short only, long only, open only etc.
   *
   * @param si     Session info used to calculate initial capital, dates etc
   * @param pc     The positions collection
   * @param pred   Predicate used to filter positions for which to calculate
   * equity
   */
  EquityCurve(const DateRange& edr, const SessionInfo& si, PositionsContainer& pc, bool doPosSizing)
      : _edr(edr),
        _si(si),
        _dpp(pc),
        _openPosCount(0),
        _doPosSizing(doPosSizing) {
    // needs to be sorted by entry time so we can do position counting
    calculate(si.runtimeParams()->positionSizing()->initialCapital());
  }

 private:
  const double* getAsArray(std::vector<double>& v, const Eq& (Equity::*f)() const, double (Eq::*g)() const) const {
    if (v.size() < __super::size()) {
      for (auto i : *this) {
        v.push_back(((i.second.*f)().*g)());
      }
    }

    return reinterpret_cast<double*>(&(v.front()));
  }

 public:
  const double* getTotal() const {
    return getAsArray(_total, &Equity::getAll, &Eq::getTotal);
  }

  const double* getLong() const {
    return getAsArray(_long, &Equity::getLong, &Eq::getTotal);
  }

  const double* getShort() const {
    return getAsArray(_short, &Equity::getShort, &Eq::getTotal);
  }

  const double* getCash() const {
    return getAsArray(_cash, &Equity::getAll, &Eq::getCash);
  }

  double getEndingTotalEquity() const {
    if (empty()) {
      return _si.runtimeParams()->positionSizing()->initialCapital();
    }
    else {
      return (*rbegin()).second.getAll().getTotal();
    }
  }

  double getEndingLongEquity() const {
    if (empty()) {
      return 0;
    }
    else {
      return (*rbegin()).second.getLong().getTotal();
    }
  }

  double getEndingShortEquity() const {
    if (empty()) {
      return 0;
    }
    else {
      return (*rbegin()).second.getShort().getTotal();
    }
  }

  double getTotalPctExposure() const {
    if (_allSum.getTotal() == 0)
      return 0;
    else {
      return (1 - _allSum.getCash() / _allSum.getTotal()) * 100.0;
    }
  }

  double getShortPctExposure() const {
    if (_allSum.getTotal() == 0) {
      return 0;
    }
    else {
      return ((_shortSum.getTotal() - _shortSum.getCash()) / _allSum.getTotal()) * 100.0;
    }
  }

  double getLongPctExposure() const {
    if (_allSum.getTotal() == 0) {
      return 0;
    }
    else {
      return ((_longSum.getTotal() - _longSum.getCash()) / _allSum.getTotal()) * 100.0;
    }
  }

  /**
   * Gets the number of elements in the equity curve
   *
   * @return
   */
  size_t getSize() const { return __super::size(); }
};

/**
 * Drawdown curve associated to an EquityCurve
 *
 * Calculates: absolute value, percentage value and duration,
 * for each equity point, max drawdown, max drawdown percent,
 * max drawdown date, max drawdown days
 */
class DrawdownCurve {
 private:
  mutable std::vector<double> _dd;
  mutable std::vector<double> _ddPercent;
  mutable std::vector<double> _ddBars;

  mutable double _maxDrawdown;
  mutable double _maxDrawdownPct;
  mutable Date _maxDrawdownDate;
  mutable Date _maxDrawdownPctDate;
  mutable unsigned int _maxDrawdownDays;

  double _retracementSqSum;
  __int64 _retracementCount;

 public:
  /**
   * Constructs a DrawdownCurve from an EquityCurve
   *
   * @param eq     The EquityCurve source
   */
  DrawdownCurve(const EquityCurve& ec, const Eq& (Equity::*f)() const)
      : _maxDrawdown(0), _maxDrawdownPct(0), _maxDrawdownDays(0), _retracementSqSum(0), _retracementCount(0) {
    // last max equity is the lowest possible value initially
    double lastMaxEquity = -FLT_MIN;
    // number of days is 0
    unsigned int days = 0;

    // for all the equity values in the equity curve
    for (auto i : ec ){
      // get the current date and equity
      const Date d(i.first);
      const Equity& eq(i.second);

      if ((eq.*f)().getTotal() >= lastMaxEquity) {
        // if the total is higher than the last max, this is a new equity high
        lastMaxEquity = (eq.*f)().getTotal();
        // reset the number of days
        days = 0;

        // the current drawdown is then 0, and bars 0
        _dd.push_back(0);
        _ddPercent.push_back(0);
        _ddBars.push_back(0);
        //        std::cout<< "x" << ", ";
      }
      else {
        // if it's not a new equty high
        // get the drawdown value, which is the difference between the last max
        // and current equity
        double dd = (eq.*f)().getTotal() - lastMaxEquity;
        // get the dd percentag
        double ddPct = lastMaxEquity == 0 ? 0 : dd / lastMaxEquity * 100;

        // see if this is the highest dd so far
        // if yes, get the date too
        // use < because dd is a negative value
        if (dd < _maxDrawdown) {
          _maxDrawdown = dd;
          _maxDrawdownDate = d;
        }

        // see if this is the highest dd pct so far
        // if yes, get the date too
        // use < because dd is a negative value
        if (ddPct < _maxDrawdownPct) {
          _maxDrawdownPct = ddPct;
          _maxDrawdownPctDate = d;
        }

        // see if this is the highest dd duration so far
        // use > because number of days is a + value
        if (days > _maxDrawdownDays) {
          _maxDrawdownDays = days;
        }

        // store the current date values for the dd, dd pct and days
        _dd.push_back(dd);
        _ddPercent.push_back(ddPct);
        _ddBars.push_back(days);
        //       std::cout<< bars << _T( ", " );
        // inc days
        //

        // calculate retracement square for ulcer index
        _retracementCount++;
        double retracement = dd / lastMaxEquity;
        _retracementSqSum += retracement * retracement;

        days++;
      }
    }
  }

  double* getDDArray() const { return &(_dd.front()); }
  double* getDDPercentArray() const { return &(_ddPercent.front()); }
  double* getBarsArray() const { return &(_ddBars.front()); }
  double maxValue() const { return _maxDrawdown; }
  double maxPct() const { return _maxDrawdownPct; }
  unsigned int maxDrawdownDays() const { return _maxDrawdownDays; }
  const Date& maxDate() const { return _maxDrawdownDate; }
  const Date& maxPctDate() const { return _maxDrawdownPctDate; }
  double ulcerIndex() const {
    return _retracementCount > 0 ? sqrt(_retracementSqSum / _retracementCount) * 100.0 : 0;
  }
};

class TotalDrawdownCurve : public DrawdownCurve {
 public:
  TotalDrawdownCurve(const EquityCurve& ec)
      : DrawdownCurve(ec, &Equity::getAll) {}
};

class LongDrawdownCurve : public DrawdownCurve {
 public:
  LongDrawdownCurve(const EquityCurve& ec)
      : DrawdownCurve(ec, &Equity::getLong) {}
};

class ShortDrawdownCurve : public DrawdownCurve {
 public:
  ShortDrawdownCurve(const EquityCurve& ec)
      : DrawdownCurve(ec, &Equity::getShort) {}
};

}  // namespace tradery
