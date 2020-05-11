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

class IBCommission : public ClonableImpl<Commission, IBCommission>,
                     public Commission {
 private:
  const bool _apiFee;

 public:
  // constructor - takes a bool indicating whether it will be applied to an
  // account that is charged an API fee
  IBCommission(const std::vector<std::string>* params = 0)
      : _apiFee(false),
        Commission(Info("12601F2C-5C44-45bb-8646-0AA07816EDE6",
                        "Interactive Brokers commission",
                        "Implements the current commission schedule for "
                        "Interactive brokers, which is dependednt on the "
                        "number of shares traded")) {}

  // calculates the commission
  virtual double getValue(unsigned long shares, double price) const {
    // IB commission in the US has three components
    double commission1 = 0;
    double commission2 = 0;
    double commission3 = 0;

    unsigned long shares1;
    unsigned long shares2;

    if (shares > 500) {
      // if the number of shares is > 500
      // the first component is calculated on the first 500 shares
      // and the second, on everything above 500
      shares1 = 500;
      shares2 = shares - 500;
    } else {
      // if less than 500 shares, it's all component 1
      shares1 = shares;
      shares2 = 0;
    }

    // component 1 is $0.01 * shares of component 1
    commission1 = 0.01 * shares1;

    // there is a minimum of $1
    if (commission1 < 1) commission1 = 1;

    // there is a max of 2% of the value of the trade
    if (commission1 > shares * price * 0.02)
      commission1 = shares * price * 0.02;

    // component 2 is $0.005 * shares above 500
    commission2 = 0.005 * shares2;

    // there is a max of 2% of the value of the trade
    if (commission2 > shares * price * 0.02)
      commission2 = shares * price * 0.02;

    if (_apiFee) {
      // if there is an API fee, add $0.003 for each share of the order
      commission3 = 0.03 * shares;

      // thre is a max of 2% of the value of the trade
      if (commission3 > shares * price * 0.02)
        commission3 = shares * price * 0.02;
    }

    // return total commission for the order
    return commission1 + commission2 + commission3;
  }
};