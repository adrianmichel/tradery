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

class Screener1 : public BarSystem<Screener1> {
 public:
  Screener1(void) {}

  ~Screener1(void) {}

  void run() {
    // if volume on the last bar (which is barsCount() - 1 )
    // is 1.1 times the volume of the previous bar - barsCount() - 2
    // and (the && operator)
    // if volume of the previous bar is 1.1 times
    // the volume 2 bars ago,
    // then print the current default symbol in the output window

    if (volume(barsCount() - 1) > 1.1 * volume(barsCount() - 2) &&
        volume(barsCount() - 2) > 1.1 * volume(barsCount() - 3))
      PRINT_LINE(defSymbol());
  }
};
