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

class DipBuyer public BarSystem<DipBuyer> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  DipBuyer(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<DipBuyer>(
            Info("50BF8555-934E-4e31-A68B-A76F69ECF021", "DipBuyer - ", ""))

            {} {
   public:
    ~DipBuyer(void) {}

    int holdingBars;
    int profitTarget;
    double dip;
    double jump;

    virtual bool init(const std::wstring& symbol) {
      holdingBars = 3;
      profitTarget = 5;
      // dip factor is 5%
      dip = 5;
      jump = 10;

      INSTALL_TIME_BASED_EXIT(holdingBars);
      INSTALL_PROFIT_TARGET(profitTarget);
      return true;
    }

    // the run method - this is called by the framework after a succesful
    // initialization it just does forEachBar which in turn calls dataHandler
    // for all the available bars
    virtual void run() { FOR_EACH_BAR(5); }

    // called for each bar
    virtual void onBar(INDEX bar) {
      // apply auto stops - so check for profit target and holding period in our
      // case
      APPLY_AUTO_STOPS(bar);

      BUY_AT_LIMIT(bar + 1, (1 - dip / 100) * close(bar), 1000,
                   "Buy at limit on dip");
      SHORT_AT_LIMIT(bar + 1, (1 + jump / 100) * close(bar), 1000,
                     "Short at limit on jump");

      //    FOR_EACH_OPEN_POSITION( bar );
    }

    /*	virtual bool onOpenPosition(Position pos, INDEX bar )
            {
        if( QQV.crossUnder( bar, QQVMedianCloseLong ) )
          SELL_AT_LIMIT( bar + 1, pos, high( bar ), "Sell at limit" );
        else if( QQV.crossUnder( bar, QQVMedianCloseShort ) )
          COVER_AT_LIMIT( bar + 1, pos, low( bar ), "Cover at limit" );

        return true;
            }
    */
  };
