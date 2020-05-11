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

class SimpleCommission : public ClonableImpl<Commission, SimpleCommission>,
                         public Commission {
 private:
  const double _commission;

 private:
  double init(const std::vector<std::string>* params) {
    if (params != 0) {
      double commission;
      std::istringstream i((*params)[0]);
      i >> commission;
      return commission;
    }
    else {
      return 0;
    }
  }

 public:
  // constructor - takes a bool indicating whether it will be applied to an
  // account that is charged an API fee
  SimpleCommission(const std::vector<std::string>* params = 0)
      : _commission(init(params)), Commission(Info("56EF85F7-2F49-4a8b-8F67-35292E67AA84", "Simple commission", "Simple commission, just returns the value")) {}

  // calculates the commission
  double getValue(unsigned long shares, double price) const override {
    return _commission;
  }
};

// commission plugin
class SimpleCommissionPlugin : public SimplePlugin<Commission> {
 public:
  void init() override { insert<SimpleCommission>(); }

  SimpleCommissionPlugin()
      : SimplePlugin<Commission>(Info("52C4DB04-DDD9-480b-9FEE-9169E83A6A4E", "Sample commission plugin",
        "Sample commission plugin - for the moment just an Interactive Brokers commission model")) {}
};
