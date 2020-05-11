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

/*
IMPORTANT DISCLAIMER

These sample trading systems are included for illustrative purpose only.

Using them with real money can lead to substantial financial losses.

DO NOT USE FOR REAL MONEY TRADING OR INVESTMENT DECISIONS.
*/

#pragma once

using namespace tradery;

/**
 * System1 illustrates some of the techniques that are emplyed in writing
 * trading systems for the SimLib framework
 *
 * It is not intended as a system to be used in real trading, and most likely,
 * it is not profitable
 *
 * System11 is derived from
 * - BarSystem, indicating that this is a system that expects bar data
 * - OpenPOsotionHandler, which enables it to receive notifications for each
 * open position when looping through a collection of open positions, by calling
 * the method forEachOpenPosition for example
 * - BarHandler, which allows it to receive notifications on each bar of data
 * when looping methods such as forEachBar are called
 * - OrderFilter, which allows it to receive notifications before each order
 * method is executed
 *
 * Each of these base classes defines one more more pure virtual methods that
 * have to be implemented in System1 and which will receive the respective
 * notifications.
 *
 *
 * @see BarSystem
 * @see OpenPositionsHandler
 * @see BarHandler
 * @see OrderFilter
 */
SYSTEM_BEGIN("System 1 - basic techniques",
             "This system illustrates a few basic techniques for writing a "
             "trading sytem class using the SimLib Framework")

private:
// time based holding period, indicates how many bars a position will be held -
// it will be used with installTimeBasedExit
unsigned int _bars;
// profit target - indicates what level of profit needs to be reached before
// closing a position - will be used with installProfitTarget
double _profitTarget;
// this is pointer to a user defined series. The user still needs to create it
// in init or elsewhere by calling Series::create( size )
// The user doesn't need to delete or deallocate this or any other series - the
// framework takes care of it
SeriesConstPtr series;
// series containing the simple moving average, will be initialized in the init
// method
SeriesConstPtr sma;

private:
// this method is declared in OpenPositionHandler class.
// it will be called for each  open position when the method forEachOpenPosition
// is called further down in this case it doesn't do anything, but positions can
// be closed for example, or used in any other way in the logic of the system
virtual bool onOpenPosition(Position* pos, const Bars& bars, size_t bar) {
  // do something here with the position
  return true;
}

public:
//  constructor of the System1 class
// does one time initialization of the class, which whill hold for the life of
// the class, indicated by the "const" qualifier associated with the variables.
// Per run intialization should be done in the method "init".

// this is called by the framework before running the system.
// used to do initializations for each run on a symbol
virtual bool init(const std::wstring& symbol) {
  _bars = 2;
  _profitTarget = 5;

  PRINT_LINE(red << name() << _T( " in init, on " ) << getSymbol());

  // registers itself as an order filter with the default positions
  // an order filter could have been implemented as a separate class,
  // which would have allowed it to be used by other systems as well
  // onBuyAtLimit will be called for every buyAtLimit call
  getDefPositions()->registerOrderFilter(this);

  // calculates the 10 bars SMA series. this will be calculated on each run
  sma = getDefVolumeSeries().SMA(10);

  // create two temporary series
  SeriesPtr tr = CREATE_SERIES(getDefBars()->size()) SeriesPtr atr =
      CREATE_SERIES(getDefBars()->size())

      // the true range series are not useful in this case,
      // they are just calculated to show it can be done, and how to
      // calculate the true range series values
      for (unsigned int bar = 1; bar < getDefBars()->size(); bar++) {
    double h = getDefHigh(bar);
    double l = getDefLow(bar);
    double c = getDefClose(bar);
    double pc = getDefClose(bar - 1);

    (*tr)[bar] = max3<double>(h - l, pc - h < 0 ? h - pc : pc - h,
                              pc - l < 0 ? l - pc : pc - l) /
                 c;
  }

  // create a new series trsma - 3 bars SMA of the true range
  SeriesConstPtr trsma = tr->SMA(3);

  // calculates the average true range
  (*atr)[3] = (*trsma)[3];
  for (bar = 3; bar < getDefBars()->size(); bar++) {
    double x = (*tr)[bar];
    double prev = (*atr)[bar - 1];
    (*atr)[bar] = (prev * (3 - 1) + x) / 3;
  }

  // get the EMA of the close with period 4 bars
  // the series of limit prices will be the close_ema * 0.95
  SeriesConstPtr close_ema = getDefCloseSeries().EMA(4);

  // calculate a new series - each value is 95% of the close_ema calculated
  // above using overloaded operator *
  series = 0.95 * (*close_ema);

  // install the exit condtions:
  // 5% profit target
  INSTALL_PROFIT_TARGET(_profitTarget)
  INSTALL_TIME_BASED_EXIT(_bars)

  // signal that init was performed succesfully
  // a false return would signal some error and would stop the run
  return true;
}

// the run method - this is called by the framework after a succesful
// initialization it just does forEachBar which in turn calls dataHandler for
// all the available bars
virtual void run() { FOR_EACH_BAR(10) }

// cleanup for the current run on the current symbol
// in this case nothing needs to be done, all variables are cleaned up
// automatically by the framework
virtual void cleanup() {}

// this is the buy at limit dataHandler - it is called before buy at limit is
// submitted and gives the system a last chance to change it or even not submit
// it at all
// the return will indicate how many shares this order should be actually placed
// for. If 0, no order will be submitted
unsigned int onBuyAtLimit(size_t bar, unsigned int shares, double price) const {
  unsigned int sh;
  //		outputSink() << OutputSink::Color::red << name() <<
  // OutputSink::Control::reset  << _T( " in onBuyAtLimit" ) <<
  // OutputSink::Control::endl;
  // in this case we are just doing some money maanagemt as well as some
  // filtering based on previous close prices

  // if it closed lower the previous two bars
  // and the total value traded was higher than $1.2mil
  // and the price of the order is lower than the last close
  // then buy at limit the number of shares calculated by getShares

  // IMPORTANT: since buyAtLimit is called on bar + 1 (see below), we have to
  // look at the last bar prior to that, so we use bar - 1, otherwise this will
  // trigger a bar out of range error for the last bar available so instead bar,
  // we use bar - 1 (which is the last bar before the bar on which the buy would
  // happen) etc.
  if ((getDefClose(bar - 1) < getDefClose(bar - 2)) &&
      (getDefClose(bar - 2) < getDefClose(bar - 3)) &&
      (price * (*sma)[bar - 1] > 1200000) && (price < getDefClose(bar - 1)))
    sh = getShares(bar - 1, price);
  else
    // otherwise don't place the order
    sh = 0;
  return sh;
}

// this implements a simple money management scheme and calculates the number of
// shares by making sure the the position value is alwyas $5000
unsigned int getShares(size_t bar, double price) const {
  return (unsigned int)(5000 / price);
}

// called for each bar
virtual void dataHandler(const Bars& bars, size_t bar) {
  // apply auto stops - so check for profit target and holding period in our
  // case
  APPLY_AUTO_STOPS;
  // get the limit price for the current bar
  double buyLimitPrice = (*series)[bar];

  //    obx << getDefBars()->getTime( bar ).to_simple_string() << "-" <<
  //    buyLimitPrice << ", ";
  // buy at limit on the next bar (it should always be next bar or the system
  // would be peeking into the future!) use a default value of 1000 shares - it
  // will be re-calculated by onBuyAtLimit
  BUY_AT_LIMIT(buyLimitPrice, 1000, "buy at limit")

  //    outputSink().printLine( obx );

  // call this method just to show how it's done - the onOpenPosition will be
  // called for each open position
  FOR_EACH_OPEN_POSITION
}

SYSTEM_END
