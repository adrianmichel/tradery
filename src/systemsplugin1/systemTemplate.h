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

class SystemTemplate : public BarSystem<SystemTemplate> {
 public:
  SystemTemplate(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<SystemTemplate>(Info("665C0513-4388-4db2-9346-24A4482A123F",
                                       "System template", "")) {}

 private:
  //************************************
  // Optional
  // systemwide variables, if any, here
  //***********************************

  bool init(const String& symbol) {
    std::cout << "SystemTemplate::init" << std::endl;
    //************************************
    // Optional
    // intialization of systemwide variables
    // or other resources used by the system here
    //************************************

    return true;
  }

  void run() {
    //************************************
    // replace 10 with the number of bars
    // the system should wait before starting
    // the processing
    //************************************
    FOR_EACH_BAR(10);
  }

  void onBar(Index bar) {
    APPLY_AUTO_STOPS(bar);
    FOR_EACH_OPEN_POSITION(bar);

    //************************************
    // position entry rules here
    //************************************
  }

  bool onOpenPosition(tradery::Position pos, Index bar) {
    //************************************
    // position exit rules here
    //************************************
    return true;
  }

  void cleanup() {
    //************************************
    // Optional
    // variable or resource cleanup here
    //************************************
  }
};