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

class SimpleSlippage : public Slippage, public ClonableImpl<Slippage, SimpleSlippage> {
 private:
  double _slippage;

 public:
  SimpleSlippage(const std::vector<std::string>* params = 0)
      : Slippage(Info("6B4C1ADB-3C98-416a-A026-78494EE08729", "A simple slippage configuration",
                      "A simple slippage, with the slippage value (in %) passed in the params first element")) {
    if (params != 0) {
      std::istringstream i((*params)[0]);
      i >> _slippage;
    }
    else {
      _slippage = 0;
    }
  }

  ~SimpleSlippage(void) override {}

  double getValue(unsigned long shares, unsigned long volume, double price) const override {
    return price * _slippage / 100.0;
  }
};

////////////////////////////////////////////////////////////////
// slippage plugin
class SimpleSlippagePlugin : public SimplePlugin<Slippage> {
 public:
  void init() override { insert<SimpleSlippage>(); }

  SimpleSlippagePlugin()
      : SimplePlugin<Slippage>(Info("3FD34BD4-68CB-4ee3-B3F8-A79FFC8F6506", "Simple slippage plugin",
                                    "Simple slippage plugin - for the moment just one very simple configuration")) {}
};
