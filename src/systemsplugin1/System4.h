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

/**
 * Illustrates shows that even a fatal error will not crash the framework or
 * affect any other systems at runtime This system tries to dreference a null
 * pointer, thus triggering a null pointer exception
 */
class System4 : public BarSystem<System4> {
 public:
  System4(const std::vector<std::string>* params = 0)
      : BarSystem<System4>(
            Info("4CC8D610-756F-457e-A0E5-468306E8F23A",
                 "System 4 - intentional \"Access violation error\"",
                 "Illustrates how usually fatal errors (such as access "
                 "violation) in one system will not crash the framework or "
                 "affect the other running systems")) {}

  virtual void onBar(Index bar) {
    // define a pointer to a char, and setting it to point to address 0
    char* p = 0;
    // trying to set the char at address 0 to 'a' will trigger a system
    // exception
    *p = 'a';
  }

  virtual void run() {
    // run the data handler for each bar
    FOR_EACH_BAR(0);
  }
};
