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

#include <sharedptr.h>
#include <misc.h>
#include <traderytypes.h>

#define MACRO(x) "${" #x "}"

#define SYSTEM_CLASS_PREFIX "System_"s

class TradingSystem {
private:
  const tradery::UniqueId m_id;
  const std::string m_code;

 public:
  TradingSystem( const std::string& code );
  std::string getClassName() const { return SYSTEM_CLASS_PREFIX + boost::replace_all_copy(getId(), "-", "_"); }
  std::string getId() const { return m_id.str(); }
  const std::string& getCode() const { return m_code; }

  std::string generateClass();
  std::string generateInsert() const;
};

class FileTradingSystem : public TradingSystem {
private:
  std::string getCodeFromFile(const std::string& file);

public:
  FileTradingSystem(const std::string& file);
};

class TradingSystems : public std::vector<TradingSystem> {
 public:
   TradingSystems(){}

   TradingSystems(const TradingSystems& systems) 
     : std::vector< TradingSystem >( systems ){}

   TradingSystems(const std::vector<TradingSystem>& systems)
     : std::vector< TradingSystem >( systems ){}
};
