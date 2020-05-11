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

#include "stdafx.h"

#include "System.h"
#include "resource.h"
#include <exceptions.h>
#include "res\system.h"

TradingSystem::TradingSystem(const std::string& code)
  : m_code(code){
}

std::string TradingSystem::generateClass() {
  std::string systemTemplate = SYSTEM;

  std::string code = systemTemplate;

  boost::replace_all(code, MACRO(SYSTEM_UUID), getId());
  boost::replace_all(code, MACRO(SYSTEM_CLASS_NAME), getClassName());
  boost::replace_all(code, MACRO(SYSTEM_DB_ID), getId());
  boost::replace_all(code, MACRO(SYSTEM_CODE), getCode());

  return code;
}

std::string TradingSystem::generateInsert() const {
  std::stringstream ss;
  ss << "\tinsert< " << getClassName() << " >();\\" << std::endl;
  return ss.str();
}

std::string FileTradingSystem::getCodeFromFile(const std::string& systemFile) {
  std::ifstream file(systemFile);
  if (!file.is_open()) {
    throw tradery::SystemException(COULD_NOT_LOAD_SYSTEM_FILE, "Could not open system file: "s + systemFile);
  }

  stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

FileTradingSystem::FileTradingSystem(const std::string& systemFile)
  : TradingSystem( getCodeFromFile(systemFile) ){
}

