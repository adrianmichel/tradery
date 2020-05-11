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

class SampleSlippage : public Slippage,
                       public ClonableImpl<Slippage, SampleSlippage> {
 public:
  SampleSlippage(const std::vector<std::string>* params = 0)
      : Slippage(Info("37124D71-5B28-4516-B07F-2835AA818AD6",
                      "A simple slippage",
                      "A simple implementation meant to illustrate the "
                      "createion of a slippage plugin")) {}

  virtual ~SampleSlippage(void) {}

  // just return a const value for now
  virtual double getValue(unsigned long shares, unsigned long volume,
                          double price) const {
    return 0.01;
  }
};
